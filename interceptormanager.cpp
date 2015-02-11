#include "interceptormanager.h"
#include "macros.h"
#include "webpage.h"
#include "networkreply.h"
#include "webkitbrowser.h"
#include "core.h"

#include <QtNetwork/QNetworkRequest>
#include <QDebug>

using namespace yasem;

InterceptorManager::InterceptorManager(WebPage *parent): QNetworkAccessManager(parent)
{
    //logger = (AbstractLoggerPlugin*)PluginManager::getInstance()->getByRole("logger");
    STUB();
    connect(this, &InterceptorManager::finished, this, &InterceptorManager::replyFinished);

    webServerHost = "http://127.0.0.1";
    webServerPort = Core::instance()->settings()->value("web-server/port", 9999).toInt();
}

QNetworkReply* InterceptorManager::createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    //DEBUG() << QString("Loading URL (before): %1").arg(request.url().toString());

    QUrl url;
    if(!request.url().toString().startsWith("data:"))
    {
        url = page->handleUrl(request.url());

        QString urlString = url.url().trimmed();

        QString rootDir = page->webView()->browser->browserRootDir();

        if(urlString.startsWith("file://") && (!(urlString.startsWith("file://" + rootDir)) || urlString.contains(".php")))
        {
            urlString = urlString.replace(QString("file://%1").arg(rootDir), QString("%1:%2").arg(webServerHost).arg(webServerPort));
        }

        url.setUrl(urlString);
    }
    else
        url = request.url().toString();

    //DEBUG() << "Loading URL:" << request.url() << "->" << url;
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, page->userAgentForUrl(QUrl()));
    req.setRawHeader("Accept", "Accept:text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");

    //TODO: Remove or fix patch for those who doesn't send valid Content-Type header
    //************** PATCH *********************************
    if((op == PostOperation) && !req.hasRawHeader("Content-Type"))
    {
        req.setRawHeader("Content-Type", "application/json");
    }
    //******************************************************

    QNetworkReply* real = QNetworkAccessManager::createRequest(op, req, outgoingData);
    real->ignoreSslErrors();

    connect(real, &QNetworkReply::metaDataChanged, [=]() {
        //WARN() << "metadata" << real->url() << real->errorString();
    });

    connect(real, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
    return real;
}

bool InterceptorManager::addInterceptorEntry(NetworkInterceptorEntry* entry)
{
     entryList.append(entry);
     return true;
}

void InterceptorManager::setPage(WebPage *page)
{
    this->page = page;
}

void InterceptorManager::onError(QNetworkReply::NetworkError err)
{
    ERROR() << "err" << err;
}


void InterceptorManager::replyFinished(QNetworkReply *reply)
{
    //DEBUG() << "URL" << reply->url();
    //DEBUG() << reply->readAll();
    //reply->reset();
}

void InterceptorManager::onMetadataChanged(NetworkReply *reply)
{
}


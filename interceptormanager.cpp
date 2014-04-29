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


    //connect(this, &QNetworkReply::metaDataChanged, this, &InterceptorManager::onMetadataChanged);
}

QNetworkReply* InterceptorManager::createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    //DEBUG(QString("Loading URL (before): %1").arg(request.url().toString()));
    QUrl url = page->handleUrl(request.url());

    QString urlString = url.url().trimmed();

    QString rootDir = page->webView()->browser->browserRootDir();


    if(urlString.startsWith("file://") && (!(urlString.startsWith("file://" + rootDir)) || urlString.contains(".php")))
    {

        urlString = urlString.replace(QString("file://%1").arg(rootDir), QString("%1:%2").arg(webServerHost).arg(webServerPort));
    }

    url.setUrl(urlString);

    //DEBUG(QString("Loading URL: %1").arg(urlString));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, page->userAgentForUrl(QUrl()));

    QNetworkReply* real = QNetworkAccessManager::createRequest(op, req, outgoingData);

    Q_ASSERT(real != 0);

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

void InterceptorManager::onMetadataChanged()
{

}

void InterceptorManager::replyFinished(QNetworkReply *reply)
{

}

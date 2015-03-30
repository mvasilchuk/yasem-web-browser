#include "interceptormanager.h"
#include "macros.h"
#include "webpage.h"
#include "networkreply.h"
#include "webkitbrowserplugin.h"
#include "core.h"
#include "browserpluginobject.h"
#include "pluginmanager.h"

#include <QtNetwork/QNetworkRequest>
#include <QDebug>

using namespace yasem;

InterceptorManager::InterceptorManager(WebPage *parent): QNetworkAccessManager(parent)
{
    //logger = (AbstractLoggerPlugin*)PluginManager::getInstance()->getByRole("logger");
    //STUB();
    //connect(this, &InterceptorManager::finished, this, &InterceptorManager::replyFinished);

    webServerHost = "http://127.0.0.1";
    webServerPort = Core::instance()->settings()->value("web-server/port", 9999).toInt();
    m_browserPlugin = dynamic_cast<BrowserPluginObject*>(PluginManager::instance()->getByRole(ROLE_BROWSER));
}

QNetworkReply* InterceptorManager::createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    //DEBUG() << QString("Loading URL (before): %1").arg(request.url().toString());

    QUrl url;
    if(!request.url().toString().startsWith("data:"))
    {
        url = page->handleUrl(request.url());

        QString urlString = url.url().trimmed();

        // TODO: Need to get url, not from browser but from webview
        QString rootDir = m_browserPlugin->browserRootDir();

        if(urlString.startsWith("file://") && (!(urlString.startsWith("file://" + rootDir)) || urlString.contains(".php")))
        {
            urlString = urlString.replace(QString("file://%1").arg(rootDir), QString("%1:%2").arg(webServerHost).arg(webServerPort));
        }

        url.setUrl(urlString);
    }
    else
        url = request.url().toString();

    //DEBUG() << "Loading URL:" << request.url() << "->" << url;
    QNetworkRequest req = request;
    req.setHeader(QNetworkRequest::UserAgentHeader, page->userAgentForUrl(QUrl()));

    QNetworkReply* real = QNetworkAccessManager::createRequest(op, req, outgoingData);
    real->ignoreSslErrors();

    connect(real, &QNetworkReply::encrypted, [=]() {
        WARN() << "encrypted" << real->url();
    });

    connect(real, &QNetworkReply::sslErrors, [=](const QList<QSslError> &errors) {
        WARN() << "sslErrors" << real->url();
    });

    connect(real, &QNetworkReply::uploadProgress, [=](qint64 bytesSent, qint64 bytesTotal) {
        //WARN() << "uploadProgress" << real->url() << bytesSent << bytesTotal;
    });

    connect(real, &QNetworkReply::downloadProgress, [=](qint64 bytesReceived, qint64 bytesTotal) {
        //WARN() << "downloadProgress" << real->url() << bytesReceived << bytesTotal;
    });

    connect(real, &QNetworkReply::metaDataChanged, [=]() {
        //WARN() << "metadata" << real->url();
    });

    connect(real, &QNetworkReply::finished, [=]() {
        //WARN() << "finished" << real->url() << real->errorString();
    });

    connect(real, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), [=](QNetworkReply::NetworkError err) {
        ERROR() << "err" << real->url() << err << real->errorString();
    });

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

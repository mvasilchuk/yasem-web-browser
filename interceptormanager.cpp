#include "interceptormanager.h"
#include "macros.h"
#include "qtwebpage.h"
#include "networkreply.h"
#include "webkitbrowserplugin.h"
#include "core.h"
#include "browser.h"
#include "pluginmanager.h"
#include "statistics.h"
#include "networkstatistics.h"
#include "yasemsettings.h"
#include "configuration_items.h"

#include <QtNetwork/QNetworkRequest>
#include <QDebug>
#include <QElapsedTimer>
#include <QAbstractNetworkCache>

using namespace yasem;

InterceptorManager::InterceptorManager(QtWebPage *parent):
    QNetworkAccessManager(parent)
{
    webServerHost = "http://127.0.0.1";
    webServerPort = SDK::Core::instance()->settings()->value("web-server/port", 9999).toInt();

    SDK::ConfigContainer* network_statistics = SDK::__get_config_item<SDK::ConfigContainer*>(QStringList() << SETTINGS_GROUP_OTHER << NETWORK_STATISTICS);
    m_statistics_enabled = network_statistics->findItemByKey(NETWORK_STATISTICS_ENABLED)->value().toBool();
    m_slow_request_timeout = network_statistics->findItemByKey(NETWORK_STATISTICS_SLOW_REQ_TIMEOUT)->value().toInt();

    if(m_statistics_enabled)
        initNetworkStatisticGathering();
}

void InterceptorManager::initNetworkStatisticGathering()
{
    SDK::NetworkStatistics* network_statistics = SDK::Core::instance()->statistics()->network();
    connect(this, &InterceptorManager::request_started,         network_statistics, &SDK::NetworkStatistics::incTotalCount);
    connect(this, &InterceptorManager::request_succeeded,       network_statistics, &SDK::NetworkStatistics::intSuccessfulCount);
    connect(this, &InterceptorManager::request_started,         network_statistics, &SDK::NetworkStatistics::incPendingConnection);
    connect(this, &InterceptorManager::request_finished,        network_statistics, &SDK::NetworkStatistics::decPendingConnections);
    connect(this, &InterceptorManager::request_failed,          network_statistics, &SDK::NetworkStatistics::incFailedCount);
    connect(this, &InterceptorManager::slow_request_detected,   network_statistics, &SDK::NetworkStatistics::incTooSlowConnections);
}

SDK::Config* InterceptorManager::settings()
{
    return SDK::Core::instance()->yasem_settings();
}

QNetworkReply* InterceptorManager::createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    //DEBUG() << QString("Loading URL (before): %1").arg(request.url().toString());

    QUrl url;
    if(!request.url().toString().startsWith("data:"))
    {
        url = m_page->handleUrl(request.url());

        QString urlString = url.url().trimmed();

        // TODO: Need to get url, not from browser but from webview
        QString rootDir = SDK::Browser::instance()->browserRootDir();

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
    req.setHeader(QNetworkRequest::UserAgentHeader, m_page->userAgentForUrl(QUrl()));

    QNetworkReply* real = QNetworkAccessManager::createRequest(op, req, outgoingData);
    real->ignoreSslErrors();

    bool marked_as_slow = false;
    QElapsedTimer* elapsed_timer = new QElapsedTimer();
    elapsed_timer->start();

    emit request_started();

    connect(real, &QNetworkReply::encrypted, [=]() {
        WARN() << "encrypted" << real->url();
        emit connection_encrypted(real->url().toString());
    });

    connect(real, &QNetworkReply::sslErrors, [=](const QList<QSslError> &errors) {
        WARN() << "sslErrors" << real->url();
        emit encryption_error(real->url().toString(), errors);
    });

    connect(real, &QNetworkReply::uploadProgress, [=, &marked_as_slow](qint64 bytesSent, qint64 bytesTotal) {
        //WARN() << "uploadProgress" << real->url() << bytesSent << bytesTotal;
        if(elapsed_timer->elapsed() > m_slow_request_timeout && !marked_as_slow)
            slow_request_detected();
    });

    connect(real, &QNetworkReply::downloadProgress, [=, &marked_as_slow](qint64 bytesReceived, qint64 bytesTotal) {
        //WARN() << "downloadProgress" << real->url() << bytesReceived << bytesTotal;
        if((elapsed_timer->elapsed() > m_slow_request_timeout) && !marked_as_slow)
            slow_request_detected();
    });

    connect(real, &QNetworkReply::metaDataChanged, [=]() {
        //WARN() << "metadata" << real->url();
    });

    connect(real, &QNetworkReply::finished, [=]() {
        //WARN() << "finished" << real->url() << real->errorString();
        if(real->error() == QNetworkReply::NoError)
            emit request_succeeded();
        else
            emit request_failed();

        emit request_finished();

        delete elapsed_timer;
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

void InterceptorManager::setPage(QtWebPage *page)
{
    this->m_page = page;
}

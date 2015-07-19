#ifndef INTERCEPTORMANAGER_H
#define INTERCEPTORMANAGER_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QList>
#include <QSslError>

namespace yasem {

namespace SDK {
class Browser;
class Config;
}

class QtWebPage;
class NetworkInterceptorEntry;
class NetworkReply;

class InterceptorManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
   InterceptorManager(QtWebPage *parent = 0);

   QNetworkReply *createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData);
   bool addInterceptorEntry(NetworkInterceptorEntry *entry);
   void setPage(QtWebPage* page);

public slots:
   void initNetworkStatisticGathering();
protected:
   QList<NetworkInterceptorEntry*> entryList;
   QtWebPage* page;
   SDK::Browser* m_browserPlugin;
   SDK::Config* m_settings;
   bool m_statistics_enabled;
   int m_slow_request_timeout;

   QString webServerHost;
   int webServerPort;

signals:
   void request_started();
   void request_finished();
   void request_succeeded();
   void request_failed();
   void slow_request_detected();
   void connection_encrypted(const QString& url);
   void encryption_error(const QString& url, const QList<QSslError> &errors);
};

}


#endif // INTERCEPTORMANAGER_H

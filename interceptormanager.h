#ifndef INTERCEPTORMANAGER_H
#define INTERCEPTORMANAGER_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QList>

namespace yasem {

namespace SDK {
class BrowserPluginObject;
class YasemSettings;
}

class WebPage;
class NetworkInterceptorEntry;
class NetworkReply;

class InterceptorManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
   InterceptorManager(WebPage *parent = 0);

   QNetworkReply *createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData);
   bool addInterceptorEntry(NetworkInterceptorEntry *entry);
   void setPage(WebPage* page);

public slots:
   void initNetworkStatisticGathering();
protected:
   QList<NetworkInterceptorEntry*> entryList;
   WebPage* page;
   SDK::BrowserPluginObject* m_browserPlugin;
   SDK::YasemSettings* m_settings;
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
};

}


#endif // INTERCEPTORMANAGER_H

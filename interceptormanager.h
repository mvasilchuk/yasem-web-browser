#ifndef INTERCEPTORMANAGER_H
#define INTERCEPTORMANAGER_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QList>

namespace yasem {
class WebPage;
class BrowserPluginObject;
class NetworkInterceptorEntry;
class NetworkReply;
class YasemSettings;

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
   BrowserPluginObject* m_browserPlugin;
   YasemSettings* m_settings;
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

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

class InterceptorManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
   InterceptorManager(WebPage *parent = 0);

   QNetworkReply *createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData);
   bool addInterceptorEntry(NetworkInterceptorEntry *entry);
   void setPage(WebPage* page);

protected:
   QList<NetworkInterceptorEntry*> entryList;
   WebPage* page;
   BrowserPluginObject* m_browserPlugin;

   QString webServerHost;
   int webServerPort;
};

}


#endif // INTERCEPTORMANAGER_H

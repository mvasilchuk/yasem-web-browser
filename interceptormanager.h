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
   typedef QPair<QByteArray, QByteArray> RawHeaderPair;

   QNetworkReply *createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData);
   bool addInterceptorEntry(NetworkInterceptorEntry *entry);
   void setPage(WebPage* page);

protected:
   QList<NetworkInterceptorEntry*> entryList;
   WebPage* page;
   BrowserPluginObject* m_browserPlugin;

   QString webServerHost;
   int webServerPort;

public slots:

   void onError(QNetworkReply::NetworkError err);
   void replyFinished(QNetworkReply*);
   void onMetadataChanged(NetworkReply* reply);
};

}


#endif // INTERCEPTORMANAGER_H

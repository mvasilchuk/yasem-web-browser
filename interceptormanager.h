#ifndef INTERCEPTORMANAGER_H
#define INTERCEPTORMANAGER_H

#include "networkinterceptorentry.h"

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QList>

namespace yasem {
class WebPage;

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

   QString webServerHost;
   int webServerPort;

public slots:

   void onMetadataChanged();
   void replyFinished(QNetworkReply*);



};

}


#endif // INTERCEPTORMANAGER_H

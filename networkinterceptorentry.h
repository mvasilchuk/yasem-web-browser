#ifndef NETWORKINTERCEPTORENTRY_H
#define NETWORKINTERCEPTORENTRY_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

class QNetworkRequest;
class QIODevice;

class NetworkInterceptorEntry : public QObject
{
    Q_OBJECT
public:
    virtual ~NetworkInterceptorEntry(){}
signals:


public slots:
    virtual QNetworkReply *processRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice *outgoingData) = 0;
    virtual bool canProcessRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice *outgoingData) = 0;


};

#endif // NETWORKINTERCEPTORENTRY_H

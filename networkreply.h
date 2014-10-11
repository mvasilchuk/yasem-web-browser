#ifndef NETWORKREPLY_H
#define NETWORKREPLY_H

#include <QObject>
#include <QNetworkReply>

class NetworkReply : public QNetworkReply
{
    Q_OBJECT
public:
    explicit NetworkReply(QObject *parent = 0);

    void setRawHeaderPublic(const QByteArray &headerName, const QByteArray &value);
    void setHeaderPublic(QNetworkRequest::KnownHeaders header, const QVariant &value);
    void setAttributePublic(QNetworkRequest::Attribute code, const QVariant &value);
signals:

};

#endif // NETWORKREPLY_H

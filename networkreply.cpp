#include "networkreply.h"

NetworkReply::NetworkReply(QObject *parent) :
    QNetworkReply(parent)
{
}

void NetworkReply::setRawHeaderPublic(const QByteArray &headerName, const QByteArray &value)
{
    QNetworkReply::setRawHeader(headerName, value);
}

void NetworkReply::setHeaderPublic(QNetworkRequest::KnownHeaders header, const QVariant &value)
{
    QNetworkReply::setHeader(header, value);
}

void NetworkReply::setAttributePublic(QNetworkRequest::Attribute code, const QVariant &value)
{
    QNetworkReply::setAttribute(code, value);
}

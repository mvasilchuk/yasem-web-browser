#ifndef WEBVIEWQML_H
#define WEBVIEWQML_H

#include <QWebView>
#include <QPushButton>


#include "QtQuick/QQuickItem"

namespace yasem {

class WebViewQml : public QQuickItem
{
    Q_OBJECT
public:
    explicit WebViewQml(QQuickItem * parent = 0);
    ~WebViewQml();

signals:

private:

};

}

#endif // WEBVIEWQML_H

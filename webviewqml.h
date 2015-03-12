#ifndef WEBVIEWQML_H
#define WEBVIEWQML_H

#include <QtQuick/QQuickPaintedItem>
#include <QEvent>
#include <QGraphicsWebView>
#include <QPainter>

namespace yasem {

class BrowserPluginObject;
class WebView;
class WebPage;

class WebViewQml : public QQuickPaintedItem
{
    Q_OBJECT
public:
    explicit WebViewQml(QQuickItem * parent = 0);
    virtual ~WebViewQml();

protected:
    BrowserPluginObject* browser;
    WebView* webView;
    WebPage* page;

public:
    void paint(QPainter *painter);
    void setWebView(WebView* webView);


protected:
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);

public:
    bool event(QEvent *);


    // QQmlParserStatus interface
public:
    void componentComplete();
};

}

#endif // WEBVIEWQML_H

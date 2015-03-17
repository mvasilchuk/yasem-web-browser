#ifndef WEBPAGE_H
#define WEBPAGE_H

#include "enums.h"
#include "abstractwebpage.h"

#include <QWidget>
#include <QWebPage>
#include <QWebInspector>

namespace yasem
{

class StbPluginObject;
class WebView;
class WebPluginFactory;
class InterceptorManager;

class WebPage : public QWebPage, public virtual AbstractWebPage
{
    Q_OBJECT
public:
    explicit WebPage(WebView *parent = 0);
    WebView* parent;
    StbPluginObject* m_stb_plugin;

    void  javaScriptAlert ( QWebFrame * frame, const QString & msg );
    bool  javaScriptConfirm ( QWebFrame * frame, const QString & msg );
    void  javaScriptConsoleMessage ( const QString & message, int lineNumber, const QString & sourceID );
    bool  javaScriptPrompt ( QWebFrame * frame, const QString & msg, const QString & defaultValue, QString * result );
    void setUserAgent(const QString &userAgent);

    WebView* webView();
public:
    InterceptorManager* interceptor;
public slots:
    bool load(const QUrl &url);
    void close();
    virtual bool event(QEvent*);

    bool stb(StbPluginObject* plugin);
    StbPluginObject* stb();
    QUrl handleUrl(QUrl url);
    void recreateObjects();
    void resetPage();
    void showWebInspector();

    bool receiveKeyCode(RC_KEY keyCode);
    void evalJs(const QString &js);

protected slots:
    void attachJsStbApi();

protected:
    QString defaultUserAgent;
    QString customUserAgent;
    WebPluginFactory* pluginFactory;
    QWebInspector m_web_inspector;

public:
     bool isChildWindow();
    // QWebPage interface
    QString userAgentForUrl(const QUrl &url) const;
public:
    void triggerAction(WebAction action, bool checked);


    // AbstractWebPage interface
public:
    void setVieportSize(QSize new_size);
    QSize getVieportSize();

    // AbstractWebPage interface
public:
    qreal scale();

    // AbstractWebPage interface
public:
    QRect getPageRect();

};

}

#endif // WEBPAGE_H

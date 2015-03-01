#ifndef WEBPAGE_H
#define WEBPAGE_H

#include "macros.h"
#include "webview.h"
#include "stbplugin.h"
#include "abstractwebpage.h"
#include "interceptormanager.h"
#include "webpluginfactory.h"
#include "customkeyevent.h"

#include <QWidget>
#include <QWebPage>
#include <QWebInspector>

namespace yasem
{

class WebPage : public QWebPage, public virtual AbstractWebPage
{
    Q_OBJECT
public:
    explicit WebPage(WebView *parent = 0);
    WebView* parent;
    StbPlugin* stbPlugin;


    void  javaScriptAlert ( QWebFrame * frame, const QString & msg );
    bool  javaScriptConfirm ( QWebFrame * frame, const QString & msg );
    void  javaScriptConsoleMessage ( const QString & message, int lineNumber, const QString & sourceID );
    bool  javaScriptPrompt ( QWebFrame * frame, const QString & msg, const QString & defaultValue, QString * result );
    void setUserAgent(const QString &userAgent);

    WebView* webView();
public:
    InterceptorManager* interceptor;
public slots:    
    void close();
    virtual bool event(QEvent*);

    bool stb(StbPlugin* plugin);
    StbPlugin* stb();
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
    QWebInspector webInspector;


public:
     bool isChildWindow();
    // QWebPage interface
    QString userAgentForUrl(const QUrl &url) const;
public:
    void triggerAction(WebAction action, bool checked);

};

}

#endif // WEBPAGE_H

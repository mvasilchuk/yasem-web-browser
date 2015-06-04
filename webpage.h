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
class BrowserPluginObject;
class WebPage : public QWebPage, public virtual AbstractWebPage
{
    Q_OBJECT
public:
    explicit WebPage(WebView *parent = 0);

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

    QColor getChromaKey()  const;
    void setChromaKey(QColor color);

    QColor getChromaMask()  const;
    void setChromaMask(QColor color);

    float getOpacity()  const;
    void setOpacity(float opacity) ;

    bool isChromaKeyEnabled()  const;
    void setChromaKeyEnabled(bool enabled);

    void reset();
    bool openWindow(const QString &url, const QString &params, const QString &name);


protected slots:
    void attachJsStbApi();

protected:
    WebView* parent;
    StbPluginObject* m_stb_plugin;
    BrowserPluginObject* m_browser;
    QString defaultUserAgent;
    QString customUserAgent;
    WebPluginFactory* pluginFactory;
    QWebInspector m_web_inspector;

    QColor m_chromakey;
    QColor m_chromamask;
    float m_opacity; // 0.0 - 1.0
    bool m_chromakey_enabled;

public:
     bool isChildWindow();
    // QWebPage interface
    QString userAgentForUrl(const QUrl &url) const;
public:
    void triggerAction(WebAction action, bool checked);


    // AbstractWebPage interface
public:
    void setPageViewportSize(QSize new_size);
    QSize getVieportSize();

    // AbstractWebPage interface
public:
    qreal scale();

    // AbstractWebPage interface
public:
    QRect getPageRect();


    // QWebPage interface
protected:
    virtual QWebPage *createWindow(WebWindowType type);

    // AbstractWebPage interface
public slots:
    virtual void execKeyEvent(const QString &action, int code, Qt::KeyboardModifiers mods, const QString &symbol);

    // AbstractWebPage interface
public slots:
    virtual QWidget *widget();

signals:
    void closed();
};

}

#endif // WEBPAGE_H

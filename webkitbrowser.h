#ifndef WEBKITBROWSER_H
#define WEBKITBROWSER_H

#include "plugin.h"
#include "browserplugin.h"
#include "webview.h"
#include "webpage.h"
#include "browserkeyevent.h"

#include "webkitbrowser_global.h"

#include <QtWebKitWidgets/QWebView>
#include <QtWebKitWidgets/QWebFrame>

namespace yasem
{

class WEBKITBROWSERSHARED_EXPORT WebkitBrowser: public QObject, public BrowserPlugin, public Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.mvas.yasem.WebkitBrowserPlugin/1.0" FILE "metadata.json")
    Q_INTERFACES(yasem::Plugin yasem::BrowserPlugin)

    Q_CLASSINFO("author", "Maxim Vasilchuk")
    Q_CLASSINFO("description", "WebkitBrowser plugin for YASEM")
public:
    WebkitBrowser();

    virtual PLUGIN_ERROR_CODES initialize();
    virtual PLUGIN_ERROR_CODES deinitialize();

    virtual void parent(QWidget *parent);
    virtual QWidget* parent();
    virtual bool load(const QUrl &url);
    virtual void evalJs(const QString &js);
    virtual void scale(qreal scale);
    virtual qreal scale();

    virtual QWidget *widget();

    virtual void rect(const QRect &rect);
    virtual QRect rect();
    void resize(QResizeEvent* = 0);
    virtual void stb(StbPlugin* stbPlugin) ;
    virtual StbPlugin* stb();

    virtual void raise();
    void show();
    void hide();

    //virtual void setTransparentColor(QPalette palette);



protected:
    QRect browserRect;
    float browserScale;
    StbPlugin* stbPlugin;
    QUrl indexUrl;
    QString rootDir;
    QHash<RC_KEY, BrowserKeyEvent*> keyEventValues;
    WebView* activeWebView;
    QList<WebView*> webViewList;
    bool isFullscreen;
    // BrowserPlugin interface


public:


    // BrowserPlugin interface
public:
    void setInnerSize(int width, int height);
    void setInnerSize(const QSize &size);

    // BrowserPlugin interface
public:
    QUrl url();
    virtual QString browserRootDir();
    void setUserAgent(const QString &userAgent);
    void addFont(const QString &fileName);
    void registerKeyEvent(RC_KEY rc_key, int keyCode, int which, bool alt = false, bool ctrl = false, bool shift = false);
    void registerKeyEvent(RC_KEY rc_key, int keyCode, int which, int keyCode2, int which2, bool alt = false, bool ctrl = false, bool shift = false) ;
    void clearKeyEvents();
    bool receiveKeyCode(RC_KEY keyCode);
    WebView *getWebView();
    void setWebView(WebView* view);
    void addWebView(WebView* view);
    void removeWebView(WebView* view);
    QList<WebView*> getWebViewList();

    void fullscreen(bool setFullscreen);
    bool fullscreen();

    void passEvent(QEvent *event);
    void setupMousePositionHandler(const QObject *receiver, const char* method);
protected slots:

    void moveEvent(QMoveEvent *event);


    // Plugin interface
public:
    void register_dependencies();
    void register_roles();
public slots:
    void setOpacity(qint32 alpha);
    qint32 getOpacity();
};

}

#endif // WEBKITBROWSER_H

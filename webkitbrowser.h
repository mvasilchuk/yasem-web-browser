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
#include <QQuickItem>

namespace yasem
{
class GuiPlugin;

class WEBKITBROWSERSHARED_EXPORT WebkitBrowser: public QObject, public BrowserPlugin, public Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.mvas.yasem.WebkitBrowserPlugin/1.0" FILE "metadata.json")
    Q_INTERFACES(yasem::Plugin yasem::BrowserPlugin)

    Q_CLASSINFO("author", "Maxim Vasilchuk")
    Q_CLASSINFO("description", "WebkitBrowser plugin for YASEM")
public:
    WebkitBrowser(QObject * parent = 0);

    virtual PLUGIN_ERROR_CODES initialize();
    virtual PLUGIN_ERROR_CODES deinitialize();

    virtual void setParentWidget(QWidget *parent);
    virtual QWidget* getParentWidget();

    bool load(const QUrl &url);
    bool load(const QString &url);

    virtual void scale(qreal scale);
    virtual qreal scale();

    virtual QWidget *widget();

    Q_INVOKABLE void rect(const QRect &rect);
    Q_INVOKABLE void rect(int x, int y, int width, int height);
    virtual QRect rect();
    void resize(QResizeEvent* = 0);
    virtual void stb(StbPlugin* stbPlugin) ;
    virtual StbPlugin* stb();

    virtual void raise();
    void show();
    void hide();

signals:
    void initialized();

protected:
    GuiPlugin* guiPlugin;
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
    QHash<RC_KEY, BrowserKeyEvent*> getKeyEventValues();



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

    // BrowserPlugin interface
public:
    AbstractWebPage *getFirstPage();
    void createNewPage();
};

}

#endif // WEBKITBROWSER_H

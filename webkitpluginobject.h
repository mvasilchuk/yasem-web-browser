#ifndef WEBBROWSERPLUGINOBJECT_H
#define WEBBROWSERPLUGINOBJECT_H

#include "browserpluginobject.h"

#include <QRect>
#include <QUrl>
#include <QMoveEvent>

namespace yasem
{
class GuiPluginObject;
class StbPluginObject;
class WebView;
class BrowserKeyEvent;

class WebkitPluginObject: public BrowserPluginObject
{
    Q_OBJECT
public:
    WebkitPluginObject(Plugin* plugin);
    virtual ~WebkitPluginObject();

    // AbstractPluginObject interface
public:
    PluginObjectResult init();
    PluginObjectResult deinit();

    virtual void setParentWidget(QWidget *parent);
    virtual QWidget* getParentWidget();

    virtual void scale(qreal scale);
    virtual qreal scale();

    virtual QWidget *widget();

    Q_INVOKABLE void rect(const QRect &rect);
    Q_INVOKABLE void rect(int x, int y, int width, int height);
    virtual QRect rect();
    void resize(QResizeEvent* = 0);
    void stb(StbPluginObject* m_stb_plugin) ;
    StbPluginObject* stb();

    void show();
    void hide();

    //virtual void componentComplete();

    //virtual void setTransparentColor(QPalette palette);
protected:
    GuiPluginObject* guiPlugin;
    QRect browserRect;
    float browserScale;
    StbPluginObject* m_stb_plugin;
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

    // BrowserPlugin interface
public:
    AbstractWebPage *getFirstPage();
    AbstractWebPage* createNewPage(bool child = false);

    // BrowserPlugin interface
public:
    AbstractWebPage *getActiveWebPage();
};
}

#endif // WEBBROWSERPLUGINOBJECT_H

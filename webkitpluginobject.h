#ifndef WEBBROWSERPLUGINOBJECT_H
#define WEBBROWSERPLUGINOBJECT_H

#include "browser.h"

#include <QRect>
#include <QUrl>
#include <QMoveEvent>
#include <QSharedPointer>

namespace yasem
{

class WebView;
class BrowserKeyEvent;

namespace SDK {
class GUI;
class StbPluginObject;
}

class WebkitPluginObject: public SDK::Browser
{
    Q_OBJECT
public:
    WebkitPluginObject(SDK::Plugin* plugin);
    virtual ~WebkitPluginObject();

    // AbstractPluginObject interface
public:
    SDK::PluginObjectResult init();
    SDK::PluginObjectResult deinit();

    virtual void setParentWidget(QWidget *parent);
    virtual QWidget* getParentWidget();

    virtual void scale(qreal scale);
    virtual qreal scale();

    virtual QWidget *widget();

    Q_INVOKABLE void rect(const QRect &rect);
    Q_INVOKABLE void rect(int x, int y, int width, int height);
    virtual QRect rect();
    void resize(QResizeEvent* = 0);
    void stb(SDK::StbPluginObject* m_stb_plugin) ;
    SDK::StbPluginObject* stb();

    void show();
    void hide();

    //virtual void componentComplete();

    //virtual void setTransparentColor(QPalette palette);
protected:
    SDK::GUI* m_gui;
    QRect browserRect;
    float browserScale;
    SDK::StbPluginObject* m_stb_plugin;
    QString rootDir;
    QHash<SDK::GUI::RcKey, QSharedPointer<BrowserKeyEvent>> m_key_events;
    WebView* m_active_web_view;
    QList<WebView*> webViewList;
    bool isFullscreen;
    // BrowserPlugin interface


public:
    QHash<SDK::GUI::RcKey, QSharedPointer<BrowserKeyEvent>> getKeyEventValues();


    // BrowserPlugin interface
public:
    QUrl url() const;
    virtual QString browserRootDir() const;
    void setUserAgent(const QString &userAgent);
    void addFont(const QString &fileName);
    virtual void registerKeyEvent(SDK::GUI::RcKey rc_key, int keyCode);
    virtual void registerKeyEvent(SDK::GUI::RcKey rc_key, int keyCode, int which, bool alt = false, bool ctrl = false, bool shift = false);
    virtual void registerKeyEvent(SDK::GUI::RcKey rc_key, int keyCode, int which, int keyCode2, int which2, bool alt = false, bool ctrl = false, bool shift = false) ;
    virtual void clearKeyEvents();
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
    SDK::WebPage* getFirstPage();
    SDK::WebPage* createNewPage(bool child = false, bool visible = true);

    // BrowserPlugin interface
public:
    SDK::WebPage* getActiveWebPage();

    // Browser interface
public slots:
    virtual void showDeveloperTools();
};
}

#endif // WEBBROWSERPLUGINOBJECT_H

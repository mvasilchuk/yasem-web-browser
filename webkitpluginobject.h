#ifndef WEBBROWSERPLUGINOBJECT_H
#define WEBBROWSERPLUGINOBJECT_H

#include "browser.h"

#include <QRect>
#include <QUrl>
#include <QMoveEvent>
#include <QSharedPointer>

class QLayout;

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

    Q_INVOKABLE void rect(const QRect &rect);
    Q_INVOKABLE void rect(int x, int y, int width, int height);
    virtual QRect rect();
    void resize(QResizeEvent* = 0);
    void stb(SDK::StbPluginObject* m_stb_plugin) ;
    SDK::StbPluginObject* stb();


    //virtual void componentComplete();

    //virtual void setTransparentColor(QPalette palette);
protected:
    SDK::GUI* m_gui;
    QRect browserRect;
    float browserScale;
    SDK::StbPluginObject* m_stb_plugin;
    QString rootDir;
    QHash<SDK::GUI::RcKey, QSharedPointer<BrowserKeyEvent>> m_key_events;

    bool isFullscreen;
    QHash<int, SDK::WebPage*> m_pages;
    QLayout* m_layout;
    QWidget* m_parent_widget;
    // BrowserPlugin interface


public:
    QHash<SDK::GUI::RcKey, QSharedPointer<BrowserKeyEvent>> getKeyEventValues();
    void printRegisteredKeys();


    // BrowserPlugin interface
public:
    virtual QString browserRootDir() const;
    void setUserAgent(const QString &userAgent);
    void addFont(const QString &fileName);
    virtual void registerKeyEvent(SDK::GUI::RcKey rc_key, int keyCode);
    virtual void registerKeyEvent(SDK::GUI::RcKey rc_key, int keyCode, int which, bool alt = false, bool ctrl = false, bool shift = false);
    virtual void registerKeyEvent(SDK::GUI::RcKey rc_key, int keyCode, int which, int keyCode2, int which2, bool alt = false, bool ctrl = false, bool shift = false) ;
    virtual void clearKeyEvents();

    void fullscreen(bool setFullscreen);
    bool fullscreen();

    void setupMousePositionHandler(const QObject *receiver, const char* method);
protected slots:

    void moveEvent(QMoveEvent *event);

    // BrowserPlugin interface
public:
    SDK::WebPage* getFirstPage();
    SDK::WebPage* createNewPage(const int page_id = -1, bool visible = true);

    // BrowserPlugin interface
public:
    SDK::WebPage* getMainWebPage() const;

    // Browser interface
public slots:
    virtual void showDeveloperTools();

    void passEvent(QEvent *event);
public:
    QHash<int, SDK::WebPage*> pages() const;
    QUrl url() const;
    void setLayout(QLayout *layout);
    QLayout *layout() const;

    virtual void addPage(SDK::WebPage* page);
    virtual void removePage(SDK::WebPage* page);

protected slots:
    void printWidgetStack();
    void repaintWebViews();
};
}

#endif // WEBBROWSERPLUGINOBJECT_H

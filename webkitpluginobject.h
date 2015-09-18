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
    SDK::PluginObjectResult init() Q_DECL_OVERRIDE;
    SDK::PluginObjectResult deinit() Q_DECL_OVERRIDE;

    virtual void setParentWidget(QWidget *parent) Q_DECL_OVERRIDE;
    virtual QWidget* getParentWidget() Q_DECL_OVERRIDE;

    virtual void scale(qreal scale) Q_DECL_OVERRIDE;
    virtual qreal scale() Q_DECL_OVERRIDE;

    Q_INVOKABLE void rect(const QRect &rect) Q_DECL_OVERRIDE;
    Q_INVOKABLE void rect(int x, int y, int width, int height);
    virtual QRect rect() Q_DECL_OVERRIDE;
    void resize(QResizeEvent* = 0) Q_DECL_OVERRIDE;
    void stb(SDK::StbPluginObject* m_stb_plugin) Q_DECL_OVERRIDE;
    SDK::StbPluginObject* stb() Q_DECL_OVERRIDE;


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
    virtual QString browserRootDir() const Q_DECL_OVERRIDE;
    void setUserAgent(const QString &userAgent) Q_DECL_OVERRIDE;
    void addFont(const QString &fileName) Q_DECL_OVERRIDE;
    virtual void registerKeyEvent(SDK::GUI::RcKey rc_key, int keyCode) Q_DECL_OVERRIDE;
    virtual void registerKeyEvent(SDK::GUI::RcKey rc_key, int keyCode,
                                  int which, bool alt = false,
                                  bool ctrl = false, bool shift = false) Q_DECL_OVERRIDE;
    virtual void registerKeyEvent(SDK::GUI::RcKey rc_key, int keyCode,
                                  int which, int keyCode2, int which2,
                                  bool alt = false, bool ctrl = false,
                                  bool shift = false) Q_DECL_OVERRIDE;
    virtual void clearKeyEvents() Q_DECL_OVERRIDE;

    void fullscreen(bool setFullscreen) Q_DECL_OVERRIDE;
    bool fullscreen() Q_DECL_OVERRIDE;

    void setupMousePositionHandler(const QObject *receiver, const char* method) Q_DECL_OVERRIDE;
protected slots:

    void moveEvent(QMoveEvent *event);

    // BrowserPlugin interface
public:
    SDK::WebPage* getFirstPage();
    SDK::WebPage* createNewPage(const int page_id = -1, bool visible = true) Q_DECL_OVERRIDE;

    // BrowserPlugin interface
public:
    SDK::WebPage* getMainWebPage() const Q_DECL_OVERRIDE;

    // Browser interface
public slots:
    virtual void showDeveloperTools() Q_DECL_OVERRIDE;

    void passEvent(QEvent *event);
public:
    QHash<int, SDK::WebPage*> pages() const Q_DECL_OVERRIDE;
    QUrl url() const;
    void setLayout(QLayout *layout) Q_DECL_OVERRIDE;
    QLayout *layout() const Q_DECL_OVERRIDE;

    virtual void addPage(SDK::WebPage* page) Q_DECL_OVERRIDE;
    virtual void removePage(SDK::WebPage* page) Q_DECL_OVERRIDE;

protected slots:
    void printWidgetStack();
    void repaintWebViews();
};
}

#endif // WEBBROWSERPLUGINOBJECT_H

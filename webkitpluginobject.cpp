#include "webkitpluginobject.h"

#include "webpluginfactory.h"
#include "webpluginfactoryimpl.h"
#include "pluginmanager.h"
#include "stbpluginobject.h"
#include "qtwebpage.h"
#include "browserkeyevent.h"
#include "gui.h"
#include "webview.h"
#include "macros.h"

#include <QFile>
#include <QDir>
#include <QPalette>
#include <QFontDatabase>
#include <QResizeEvent>
#include <QMoveEvent>
#include <QGraphicsOpacityEffect>
#include <QLinearGradient>
#include <QWebSecurityOrigin>

using namespace yasem;

WebkitPluginObject::WebkitPluginObject(SDK::Plugin* plugin):
    SDK::Browser(plugin),
    m_gui(NULL)
{
}

WebkitPluginObject::~WebkitPluginObject()
{
    STUB();
    clearKeyEvents();
}

SDK::PluginObjectResult yasem::WebkitPluginObject::init()
{
    return SDK::PLUGIN_OBJECT_RESULT_OK;
}

SDK::PluginObjectResult yasem::WebkitPluginObject::deinit()
{
    return SDK::PLUGIN_OBJECT_RESULT_OK;
}

void yasem::WebkitPluginObject::setParentWidget(QWidget *parent)
{
    this->setParent(parent);
}

QWidget *yasem::WebkitPluginObject::getParentWidget()
{
     return static_cast<QWidget*>(this->parent());
}

void WebkitPluginObject::scale(qreal scale)
{
    this->browserScale = scale;
}

qreal WebkitPluginObject::scale()
{
    return this->browserScale;
}

QWidget *WebkitPluginObject::widget()
{
    return getWebView();
}

void WebkitPluginObject::rect(const QRect &rect)
{
    this->browserRect = rect;
}

void WebkitPluginObject::rect(int x, int y, int width, int height)
{
    rect(QRect(x, y, width, height));
}

QRect WebkitPluginObject::rect()
{
    return this->browserRect;
}

SDK::StbPluginObject* WebkitPluginObject::stb()
{
    return this->m_stb_plugin;
}

void WebkitPluginObject::show()
{
    activeWebView->show();
}

void WebkitPluginObject::hide()
{
    activeWebView->hide();
}

QHash<SDK::GUI::RcKey, QSharedPointer<BrowserKeyEvent>> WebkitPluginObject::getKeyEventValues()
{
    return m_key_events;
}

QUrl WebkitPluginObject::url()
{
    return activeWebView->url();
}

QString WebkitPluginObject::browserRootDir()
{
    return this->rootDir;
}

void WebkitPluginObject::setUserAgent(const QString &userAgent)
{
    QtWebPage* p = (QtWebPage*)activeWebView->page();

    qDebug() << "Using User Agent" << userAgent;

    p->setUserAgent(userAgent);
}

void WebkitPluginObject::addFont(const QString &fileName)
{
    QFile fontFile(fileName);
    QFontDatabase::addApplicationFontFromData(fontFile.readAll());
}


void WebkitPluginObject::stb(SDK::StbPluginObject* stbPlugin)
{
    this->m_stb_plugin = stbPlugin;
    foreach(QObject* child, webViewList)
    {
        WebView* childView = qobject_cast<WebView*>(child);
        if(childView != NULL)
        {
            static_cast<QtWebPage*>(childView->page())->stb(stbPlugin);
        }
    }
}

void WebkitPluginObject::resize(QResizeEvent* event)
{
    Q_UNUSED(event);
    moveEvent(0);
}

void WebkitPluginObject::moveEvent ( QMoveEvent * event )
{
    Q_UNUSED(event)

    if(!m_gui)
    {
        m_gui = SDK::GUI::instance();
        if(!m_gui) return;
    }

    foreach(QWidget* child, webViewList)
    {
        WebView* vChild = dynamic_cast<WebView*>(child);
        if(vChild != NULL)
        {
            vChild->resizeView(m_gui->widgetRect());
        }

        else qWarning() << "child warn:" << child;
    }
}

void WebkitPluginObject::registerKeyEvent(SDK::GUI::RcKey rc_key, int keyCode)
{
    registerKeyEvent(rc_key, keyCode, keyCode);
}

void WebkitPluginObject::registerKeyEvent(SDK::GUI::RcKey rc_key, int keyCode, int which, bool alt, bool ctrl, bool shift)
{
    DEBUG() << "registerKeyEvent {key:"
            << SDK::GUI::instance()->getRcKeyName(rc_key)
            << ", code:"
            << keyCode
            << ", which:"
            << which << ", alt:"
            << alt << ", ctrl:"
            << ctrl << ", shift:"
            << shift << "}";

    m_key_events[rc_key] = QSharedPointer<BrowserKeyEvent>(new BrowserKeyEvent(keyCode, which, alt, ctrl, shift));
}

void WebkitPluginObject::registerKeyEvent(SDK::GUI::RcKey rc_key, int keyCode, int which, int keyCode2, int which2, bool alt, bool ctrl, bool shift)
{
    m_key_events[rc_key] = QSharedPointer<BrowserKeyEvent>(new BrowserKeyEvent(keyCode, which, keyCode2, which2, alt, ctrl, shift));
}

void WebkitPluginObject::clearKeyEvents()
{
    m_key_events.clear();
}

WebView *WebkitPluginObject::getWebView()
{
    return activeWebView;
}

void WebkitPluginObject::setWebView(WebView *webView)
{
    activeWebView = webView;
}

void WebkitPluginObject::addWebView(WebView* view)
{
   webViewList.append(view);
}

void WebkitPluginObject::removeWebView(WebView* view)
{
    webViewList.removeOne(view);
}

QList<WebView *> WebkitPluginObject::getWebViewList()
{
    return webViewList;
}

void WebkitPluginObject::fullscreen(bool setFullscreen)
{
    isFullscreen = setFullscreen;
}

bool WebkitPluginObject::fullscreen()
{
    return isFullscreen;
}

void WebkitPluginObject::passEvent(QEvent *event)
{
    QCoreApplication::sendEvent(activeWebView, event);
}

void WebkitPluginObject::setupMousePositionHandler(const QObject *receiver, const char *method)
{
    connect(activeWebView, SIGNAL(mousePositionChanged(int)), receiver, method, Qt::DirectConnection);
}

SDK::WebPage* WebkitPluginObject::getFirstPage()
{
    return dynamic_cast<SDK::WebPage*>(activeWebView->page());
}

SDK::WebPage* WebkitPluginObject::createNewPage(bool child)
{
    WebView* webView = new WebView(getParentWidget());
    QtWebPage* page = new QtWebPage(webView);

    if(!child) // If it's not a child view
    {
        page->setObjectName("Main web page");
        setWebView(webView);
#ifdef USE_REAL_TRANSPARENCY
    connect(this, &WebkitPluginObject::topWidgetChanged, webView, &WebView::fullUpdate);
#else
    connect(this, &WebkitPluginObject::topWidgetChanged, webView, &WebView::updateTopWidget);
#endif //USE_REAL_TRANSPARENCY
    }
    else
    {
        page->setObjectName("Child web page");
    }

    webView->setPage(page);
    webView->setViewportSize(QSize(1280, 720));
    webView->show();
    webView->raise();
    fullscreen(false);

    addWebView(webView);
    return dynamic_cast<SDK::WebPage*>(webView->page());
}

/*
void WebkitBrowser::componentComplete()
{
    // Call superclass method to set CustomElement() parent
    QQuickItem::componentComplete();
}*/


SDK::WebPage* WebkitPluginObject::getActiveWebPage()
{
    return dynamic_cast<SDK::WebPage*>(activeWebView->page());
}

void yasem::WebkitPluginObject::showDeveloperTools()
{
    ((QtWebPage*)getActiveWebPage())->showWebInspector();
}

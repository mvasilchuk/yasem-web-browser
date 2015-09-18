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
#include <QLayout>

using namespace yasem;

WebkitPluginObject::WebkitPluginObject(SDK::Plugin* plugin):
    SDK::Browser(plugin),
    m_gui(NULL),
    m_stb_plugin(NULL),
    m_layout(0)
{
    connect(this, &WebkitPluginObject::pageCountChanged, this, &WebkitPluginObject::printWidgetStack);
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
    m_parent_widget = parent;

    //TODO: Move this to a slot for SDK::PluginManager::pluginInitialized() signal
    connect(SDK::GUI::instance(), &SDK::GUI::topWidgetChanged, this, &WebkitPluginObject::repaintWebViews);
}

QWidget *yasem::WebkitPluginObject::getParentWidget()
{
     return m_parent_widget;
}

void WebkitPluginObject::scale(qreal scale)
{
    this->browserScale = scale;
}

qreal WebkitPluginObject::scale()
{
    return this->browserScale;
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


QHash<SDK::GUI::RcKey, QSharedPointer<BrowserKeyEvent>> WebkitPluginObject::getKeyEventValues()
{
    return m_key_events;
}

void WebkitPluginObject::printRegisteredKeys()
{
    DEBUG() << "Registered keys:";
    int keyEnumIndex = SDK::GUI::staticMetaObject.indexOfEnumerator("RcKey");
    for(SDK::GUI::RcKey key: m_key_events.keys())
    {
        DEBUG() << "    ->"
                << SDK::GUI::staticMetaObject.enumerator(keyEnumIndex).valueToKey(key)
                << m_key_events.value(key)->toJsonString();
    }
}

QUrl WebkitPluginObject::url() const
{
    return getMainWebPage()->getURL();
}

QString WebkitPluginObject::browserRootDir() const
{
    return url().toString(QUrl::RemoveFilename | QUrl::StripTrailingSlash | QUrl::RemoveQuery);
}

void WebkitPluginObject::setUserAgent(const QString &userAgent)
{
    QtWebPage* p = (QtWebPage*)getMainWebPage();

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
    foreach(const int id, pages().keys())
    {
        SDK::WebPage* page = pages().value(id);
        dynamic_cast<QtWebPage*>(page)->stb(stbPlugin);
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

    repaintWebViews();
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


void WebkitPluginObject::fullscreen(bool setFullscreen)
{
    isFullscreen = setFullscreen;
}

bool WebkitPluginObject::fullscreen()
{
    return isFullscreen;
}


void WebkitPluginObject::setupMousePositionHandler(const QObject *receiver, const char *method)
{
    connect(getMainWebPage()->widget(), SIGNAL(mousePositionChanged(int)), receiver, method, Qt::DirectConnection);
}

SDK::WebPage* WebkitPluginObject::getFirstPage()
{
    return dynamic_cast<SDK::WebPage*>(getMainWebPage());
}

SDK::WebPage* WebkitPluginObject::createNewPage(const int page_id, bool visible)
{
    Q_ASSERT_X(m_layout, "WebKitPluginObject::createNewPage", "Layout not set!");

    WebView* webView = new WebView(getParentWidget());
    m_layout->addWidget(webView);
    QtWebPage* page = new QtWebPage(webView);
    if(page_id > -1)
        page->setId(page_id);
    else
        page->setId(nextPageId());


    page->setVisibilityState(QWebPage::VisibilityStateHidden);
    page->setObjectName(QString("Web page ").append(QString::number(page->getId())));
    webView->setObjectName(QString("Web view ").append(QString::number(page->getId())));
    page->setupInterceptor();

    addPage(page);

    connect(page, &QtWebPage::raised, this, &WebkitPluginObject::printWidgetStack);
    connect(page, &QtWebPage::showed, this, &WebkitPluginObject::printWidgetStack);
    connect(page, &QtWebPage::hidden, this, &WebkitPluginObject::printWidgetStack);
    connect(page, &QtWebPage::closed, this, &WebkitPluginObject::repaintWebViews);

    if(getMainWebPage())
    {
        QtWebPage* p = (QtWebPage*)getMainWebPage();
        page->stb(p->stb());
    }

    webView->setPage(page);
    webView->setViewportSize(QSize(1280, 720));
    if(visible)
    {
        page->show();
        page->raise();
    }
    else
        page->hide();
    fullscreen(false);

    DEBUG() << "Page id" << page->getId();

    return page;
}

/*
void WebkitBrowser::componentComplete()
{
    // Call superclass method to set CustomElement() parent
    QQuickItem::componentComplete();
}*/


SDK::WebPage* WebkitPluginObject::getMainWebPage() const
{
    SDK::WebPage* page = pages().value(1);
    Q_ASSERT(page);
    return page;
}

void yasem::WebkitPluginObject::showDeveloperTools()
{
    ((QtWebPage*)getMainWebPage())->showWebInspector();
}

QHash<int, SDK::WebPage*> WebkitPluginObject::pages() const
{
    return m_pages;
}

void WebkitPluginObject::setLayout(QLayout *layout)
{
    m_layout = layout;
}

QLayout *WebkitPluginObject::layout() const
{
    return m_layout;
}

void WebkitPluginObject::addPage(SDK::WebPage *page)
{
    m_pages.insert(page->getId(), page);
    emit pageCountChanged();
}

void WebkitPluginObject::removePage(SDK::WebPage *page)
{
    m_pages.remove(page->getId());
    emit pageCountChanged();
}

void WebkitPluginObject::printWidgetStack()
{
    SDK::GUI::instance()->widgetStack();
}

void WebkitPluginObject::repaintWebViews()
{
    if(!m_gui)
    {
        m_gui = SDK::GUI::instance();
        if(!m_gui) return;
    }

    foreach(const int id, pages().keys())
    {
        QtWebPage* page = dynamic_cast<QtWebPage*>(pages().value(id));
        page->webView()->resizeView(m_gui->widgetRect());
    }
}

void WebkitPluginObject::passEvent(QEvent *event)
{
    QCoreApplication::sendEvent(getMainWebPage()->widget(), event);
}

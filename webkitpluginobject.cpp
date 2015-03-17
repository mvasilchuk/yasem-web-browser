#include "webkitpluginobject.h"

#include "webpluginfactory.h"
#include "webpluginfactoryimpl.h"
#include "pluginmanager.h"
#include "stbpluginobject.h"
#include "webpage.h"
#include "browserkeyevent.h"
#include "webviewqml.h"
#include "guipluginobject.h"
#include "webview.h"

#include <QWebSecurityOrigin>
#include <QFile>
#include <QDir>
#include <QPalette>
#include <QFontDatabase>
#include <QResizeEvent>
#include <QMoveEvent>
#include <QGraphicsOpacityEffect>
#include <QLinearGradient>

using namespace yasem;

WebkitPluginObject::WebkitPluginObject(Plugin* plugin):
    BrowserPluginObject(plugin),
    m_stb_plugin(NULL)
{
    guiPlugin = NULL;
}

WebkitPluginObject::~WebkitPluginObject()
{

}



PluginObjectResult yasem::WebkitPluginObject::init()
{
    return PLUGIN_OBJECT_RESULT_OK;
}

PluginObjectResult yasem::WebkitPluginObject::deinit()
{
    return PLUGIN_OBJECT_RESULT_OK;
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
    return activeWebView;
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

StbPluginObject *WebkitPluginObject::stb()
{
    return this->m_stb_plugin;
}

void WebkitPluginObject::raise()
{
    activeWebView->raise();
}

void WebkitPluginObject::show()
{
    activeWebView->show();
}

void WebkitPluginObject::hide()
{
    activeWebView->hide();
}

QHash<RC_KEY, BrowserKeyEvent *> WebkitPluginObject::getKeyEventValues()
{
    return keyEventValues;
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
    WebPage* p = (WebPage*)activeWebView->page();

    qDebug() << "Using User Agent" << userAgent;

    p->setUserAgent(userAgent);
}

void WebkitPluginObject::addFont(const QString &fileName)
{
    QFile fontFile(fileName);
    QFontDatabase::addApplicationFontFromData(fontFile.readAll());
}

void WebkitPluginObject::stb(StbPluginObject *stbPlugin)
{
    this->m_stb_plugin = stbPlugin;
    foreach(QObject* child, webViewList)
    {
        WebView* childView = qobject_cast<WebView*>(child);
        if(childView != NULL)
        {
            ((WebPage*)childView->page())->stb(stbPlugin);
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
    if(!guiPlugin) {
        guiPlugin = dynamic_cast<GuiPluginObject*>(PluginManager::instance()->getByRole(ROLE_GUI));
        if(!guiPlugin) return;
    }

    foreach(QWidget* child, webViewList)
    {
        WebView* vChild = qobject_cast<WebView*>(child);
        if(vChild != NULL)
        {
            vChild->resizeView(guiPlugin->widgetRect());
        }

        else qWarning() << "child warn:" << child;
    }
}


void WebkitPluginObject::registerKeyEvent(RC_KEY rc_key, int keyCode, int which, bool alt, bool ctrl, bool shift)
{
    DEBUG() << "registerKeyEvent {key:"
            << Core::instance()->getKeycodeHashes().key(rc_key)
            << ", code:"
            << keyCode
            << ", which:"
            << which << ", alt:"
            << alt << ", ctrl:"
            << ctrl << ", shift:"
            << shift << "}";

    keyEventValues[rc_key] = new BrowserKeyEvent(keyCode, which, alt, ctrl, shift);
}

void WebkitPluginObject::registerKeyEvent(RC_KEY rc_key, int keyCode, int which, int keyCode2, int which2, bool alt, bool ctrl, bool shift)
{

    keyEventValues[rc_key] = new BrowserKeyEvent(keyCode, which, keyCode2, which2, alt, ctrl, shift);
}

void WebkitPluginObject::clearKeyEvents()
{
    keyEventValues.clear();
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


void WebkitPluginObject::setOpacity(qint32 alpha)
{
    qDebug() << "setOpacity" << alpha;
    foreach(QWidget* child, webViewList)
    {
        WebView* vChild = qobject_cast<WebView*>(child);
        if(vChild != NULL)
        {
            //vChild->setStyleSheet("background: transparent");
            //QGraphicsOpacityEffect * effect = new QGraphicsOpacityEffect(vChild);
            //effect->setOpacity((float)alpha / 255);
            //effect->setOpacity(0.5);

            //QLinearGradient alphaGradient(child->rect().topLeft(), child->rect().bottomLeft());
            /*alphaGradient.setColorAt(0.12, Qt::transparent);
            alphaGradient.setColorAt(0.20, Qt::black);
            alphaGradient.setColorAt(0.8, Qt::black);
            alphaGradient.setColorAt(0.9, Qt::transparent);

            effect->setOpacityMask(QBrush(alphaGradient));*/
            //effect->setOpacity((float)alpha / 255);
            //effect->setOpacityMask(QBrush(QColor( 128, 128, 128, 128)));
            //vChild->setGraphicsEffect(effect);
        }

        else qWarning() << "child warn:" << child;
    }
}

qint32 WebkitPluginObject::getOpacity()
{
    return 100;
}


AbstractWebPage *WebkitPluginObject::getFirstPage()
{
    return (WebPage*)activeWebView->page();
}

AbstractWebPage* WebkitPluginObject::createNewPage()
{
    activeWebView = new WebView(NULL, this);
    WebPage* page = new WebPage(activeWebView);
    page->setObjectName("Main web page");
    activeWebView->setPage(page);
    activeWebView->setViewportSize(QSize(1280, 720));
    webViewList.append(activeWebView);
    qDebug() << "wtf child:" << webViewList;

    fullscreen(false);

    return page;
}

/*
void WebkitBrowser::componentComplete()
{
    // Call superclass method to set CustomElement() parent
    QQuickItem::componentComplete();
}*/




AbstractWebPage *WebkitPluginObject::getActiveWebPage()
{
    return dynamic_cast<AbstractWebPage*>(activeWebView->page());
}

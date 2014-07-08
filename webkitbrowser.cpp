#include "core.h"

#include "webkitbrowser.h"
#include "webpluginfactory.h"
#include "webpluginfactoryimpl.h"
#include "pluginmanager.h"
#include "stbplugin.h"
#include "webpage.h"
#include "browserkeyevent.h"

#include <QWebSecurityOrigin>
#include <QFile>
#include <QDir>
#include <QPalette>
#include <QFontDatabase>
#include <QResizeEvent>
#include <QMoveEvent>

using namespace yasem;

WebkitBrowser::WebkitBrowser()
{
    is_last_state_fullscreen = false;
}

PLUGIN_ERROR_CODES WebkitBrowser::initialize()
{
    activeWebView = new WebView(NULL, this);
    activeWebView->setPage(new WebPage(activeWebView));
    webViewList.append(activeWebView);
    qDebug() << "wtf child:" << webViewList;

    this->innerSize = QSize(1280, 720);
    fullscreen(false);

    //TODO: fix

    stbPlugin = NULL;



    return PLUGIN_ERROR_NO_ERROR;
}

void WebkitBrowser::setupMousePositionHandler(const QObject *receiver, const char* method)
{
    connect(activeWebView, SIGNAL(mousePositionChanged(int)), receiver, method, Qt::DirectConnection);
}

PLUGIN_ERROR_CODES WebkitBrowser::deinitialize()
{
    STUB();
    return PLUGIN_ERROR_NO_ERROR;
}

void WebkitBrowser::parent(QWidget *parent)
{
    activeWebView->setParent(parent);
    normalWindowRect = parent->geometry();
}

QWidget *WebkitBrowser::parent()
{
    Q_ASSERT(activeWebView);
    return activeWebView->parentWidget();
}

bool WebkitBrowser::load(const QUrl &url)
{
    this->indexUrl = url;

    if(indexUrl.toString().startsWith("file://"))
    {
        QFile file(indexUrl.toString().replace("file://", ""));
        QFileInfo info(file);
        this->rootDir = info.absoluteDir().path();
    }
    else
    {
        this->rootDir = indexUrl.url(QUrl::RemoveFilename);
    }

    ((WebPage*)activeWebView->page())->resetPage();

    activeWebView->load(url);
    return true;
}

void WebkitBrowser::evalJs(const QString &js)
{
    Q_ASSERT(activeWebView);
    activeWebView->page()->mainFrame()->evaluateJavaScript(js);
}

void WebkitBrowser::scale(qreal scale)
{
    this->browserScale = scale;
}

qreal WebkitBrowser::scale()
{
    return this->browserScale;
}

QWidget *WebkitBrowser::widget()
{
    Q_ASSERT(activeWebView);
    return activeWebView;
}

void WebkitBrowser::rect(const QRect &rect)
{
    this->browserRect = rect;
    //resize();
}

QRect WebkitBrowser::rect()
{
    return this->browserRect;
}

StbPlugin *WebkitBrowser::stb()
{
    Q_ASSERT(this->stbPlugin);
    return this->stbPlugin;
}

void WebkitBrowser::raise()
{
    activeWebView->raise();
}

void WebkitBrowser::show()
{
    activeWebView->show();
}

void WebkitBrowser::hide()
{
    activeWebView->hide();
}

void WebkitBrowser::setInnerSize(int width, int height)
{
    setInnerSize(QSize(width, height));
}

void WebkitBrowser::setInnerSize(const QSize &size)
{
    Q_ASSERT(size.width() != -1 && size.height() != -1);
    this->innerSize = size;
    foreach(QObject* child, webViewList)
    {
        WebView* childView = qobject_cast<WebView*>(child);
        if(childView != NULL)
        {
            WebPage* childPage = (WebPage*)childView->page();
            childPage->setViewportSize(size);
            DEBUG() << "viewport" << childPage->viewportSize();
        }
    }
    resize();
}

QUrl WebkitBrowser::url()
{
    Q_ASSERT(activeWebView);
    return activeWebView->url();
}

QString WebkitBrowser::browserRootDir()
{
    return this->rootDir;
}

void WebkitBrowser::setUserAgent(const QString &userAgent)
{
    WebPage* p = (WebPage*)activeWebView->page();

    qDebug() << "Using User Agent" << userAgent;

    p->setUserAgent(userAgent);
}

void WebkitBrowser::addFont(const QString &fileName)
{
    QFile fontFile(fileName);
    QFontDatabase::addApplicationFontFromData(fontFile.readAll());
}



void WebkitBrowser::stb(StbPlugin *stbPlugin)
{
    this->stbPlugin = stbPlugin;
    foreach(QObject* child, webViewList)
    {
        WebView* childView = qobject_cast<WebView*>(child);
        if(childView != NULL)
        {
            ((WebPage*)childView->page())->stb(stbPlugin);
        }
    }
}

void WebkitBrowser::resize(QResizeEvent* event)
{   
    Q_UNUSED(event);
    moveEvent(0);
}

void WebkitBrowser::moveEvent ( QMoveEvent * event )
{
    QWidget *parentWidget = parent();

    if(is_last_state_fullscreen && normalWindowRect.width() > 0)
    {
        browserRect = normalWindowRect;
    }

    float scale = (float)innerSize.width() / innerSize.height();
    browserScale = (float)parentWidget->width() / (float)parentWidget->height();

    if(fullscreen())
    {
        if(!is_last_state_fullscreen)
        {
            normalWindowRect = browserRect;
        }
        is_last_state_fullscreen = true;
        browserRect = parentWidget->rect();
    }
    else
    {
        is_last_state_fullscreen = false;
        if(scale > browserScale)
        {
            int containerHeight = parentWidget->height() - browserRect.height();
            if(containerHeight < 0) containerHeight = 0;

            browserRect.setLeft(0);
            browserRect.setTop(containerHeight / 2);
            browserRect.setWidth(parentWidget->width());
            browserRect.setHeight(parentWidget->width() / scale);
        }
        else
        {
            int containerWidth = parentWidget->width() - browserRect.width();
            if(containerWidth < 0) containerWidth = 0;

            browserRect.setLeft(containerWidth / 2);
            browserRect.setTop(0);
            browserRect.setHeight(parentWidget->height());
            browserRect.setWidth(parentWidget->height() * scale);
        }
    }

    browserScale = (qreal)browserRect.width() / innerSize.width();

    QRect actualRect(0, 0, browserRect.width(), browserRect.height());
    foreach(QWidget* child, webViewList)
    {
        WebView* vChild = qobject_cast<WebView*>(child);
        if(vChild != NULL)
        {
            vChild->setZoomFactor(browserScale);
            vChild->page()->setActualVisibleContentRect(actualRect);
        }

        else qWarning() << "child warn:" << child;
    }


    widget()->setGeometry(browserRect);
    //widget()->update();
}


void WebkitBrowser::registerKeyEvent(RC_KEY rc_key, int keyCode, int which, bool alt, bool ctrl, bool shift)
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

void WebkitBrowser::registerKeyEvent(RC_KEY rc_key, int keyCode, int which, int keyCode2, int which2, bool alt, bool ctrl, bool shift)
{

    keyEventValues[rc_key] = new BrowserKeyEvent(keyCode, which, keyCode2, which2, alt, ctrl, shift);
}

void WebkitBrowser::clearKeyEvents()
{
    keyEventValues.clear();
}


bool WebkitBrowser::receiveKeyCode(RC_KEY keyCode)
{
    STUB() << Core::instance()->getKeycodeHashes().key(keyCode); //int)keyCode;

    if(!keyEventValues.contains(keyCode))
    {
        qDebug() << "Key code not registered:" << keyCode << keyEventValues;
        return false;
    }

    BrowserKeyEvent* keyEvent = keyEventValues[keyCode];
    if(keyEvent != NULL)
    {
        QString str = keyEvent->toString();
        evalJs(QString("javascript: %1").arg(str));
        return true;
    }
    return false;
}

WebView *WebkitBrowser::getWebView()
{
    return activeWebView;
}

void WebkitBrowser::setWebView(WebView *webView)
{
    Q_ASSERT(webView != 0);
    activeWebView = webView;
}

void WebkitBrowser::addWebView(WebView* view)
{
   webViewList.append(view);
}

void WebkitBrowser::removeWebView(WebView* view)
{
    webViewList.removeOne(view);
}

QList<WebView *> WebkitBrowser::getWebViewList()
{
    return webViewList;
}

void WebkitBrowser::fullscreen(bool setFullscreen)
{
    isFullscreen = setFullscreen;
}

bool WebkitBrowser::fullscreen()
{
    return isFullscreen;
}

void WebkitBrowser::passEvent(QEvent *event)
{
    QCoreApplication::sendEvent(activeWebView, event);
}

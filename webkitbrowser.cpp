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

using namespace yasem;

WebkitBrowser::WebkitBrowser()
{
}

PLUGIN_ERROR_CODES WebkitBrowser::initialize()
{
    activeWebView = new WebView(NULL, this);
    activeWebView->setPage(new WebPage(activeWebView));
    webViewList.append(activeWebView);
    qDebug() << "wtf child:" << webViewList;

    this->innerSize = QSize(1280, 720);

    //TODO: fix

    stbPlugin = NULL;

    return PLUGIN_ERROR_NO_ERROR;
}

PLUGIN_ERROR_CODES WebkitBrowser::deinitialize()
{
    STUB();
    return PLUGIN_ERROR_NO_ERROR;
}

void WebkitBrowser::parent(QWidget *parent)
{
    activeWebView->setParent(parent);
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
    resize();
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
            qDebug() << "viewport" << childPage->viewportSize();
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
    STUB();
    Q_UNUSED(event);
    QWidget *parentWidget = parent();
    float scale = (float)innerSize.width() / innerSize.height();
    browserScale = (float)parentWidget->width() / (float)parentWidget->height();

    //qDebug() << "start zoom:" << browserScale << scale;

    if(scale > browserScale)
    {
        browserRect.setWidth(parentWidget->width());
        browserRect.setHeight(parentWidget->width() / scale);
        browserRect.setLeft(0);
        browserRect.setTop((parentWidget->height() - browserRect.height()) / 2);
    }
    else
    {
        browserRect.setHeight(parentWidget->height());
        browserRect.setWidth(parentWidget->height() * scale);
        browserRect.setLeft((parentWidget->width() - browserRect.width()) / 2);
        browserRect.setTop(0);
    }

    DEBUG(QString("scale: %1 -> %2").arg(scale).arg(browserScale));
    DEBUG(QString("parent:[%1, %2]").arg(parentWidget->width()).arg(parentWidget->height()));
    DEBUG(QString("browserRect:[%1, %2, %3, %4]").arg(browserRect.left()).arg(browserRect.top()).arg(browserRect.width()).arg(browserRect.height()));

    browserScale = (qreal)browserRect.width() / innerSize.width();

    //qDebug() << "zoom: " << browserScale;

    qDebug() << "children:" << webViewList;

    QRect actualRect(0, 0, browserRect.width(), browserRect.height());
    foreach(QWidget* child, webViewList)
    {
        WebView* vChild = qobject_cast<WebView*>(child);
        if(vChild != NULL)
        {
            qDebug() << "child:" << vChild;
            vChild->setGeometry(browserRect);
            vChild->setZoomFactor(browserScale);


            vChild->page()->setActualVisibleContentRect(actualRect);
        }
        else qWarning() << "child warn:" << child;
    }
}


void WebkitBrowser::registerKeyEvent(RC_KEY rc_key, int keyCode, int which, bool alt, bool ctrl, bool shift)
{
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
    STUB_WITH_PARAMS(QVariant::fromValue((int)keyCode));

    if(!keyEventValues.contains(keyCode)) return false;
    //else qDebug() << keyEventValues[keyCode]->keyCode;

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

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
#include <QGraphicsOpacityEffect>
#include <QLinearGradient>

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
    moveEvent(0);
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
    Q_UNUSED(event)

    QWidget *parentWidget = parent();

    //Core::printCallStack();

    float w_ratio = (float)innerSize.width() / parentWidget->width();
    float h_ratio = (float)innerSize.height() / parentWidget->height();

    int width;
    int height;

    if(w_ratio > h_ratio)
    {
        width = parentWidget->width();
        height = (int)((float)innerSize.height() / w_ratio);
    }
    else
    {
        height = parentWidget->height();
        width = (int)((float)innerSize.width() / h_ratio);
    }


    int left =  (int)(((float)parentWidget->width() - width) / 2);
    int top = (int)(((float)parentWidget->height() - height) / 2);

    browserRect.setLeft(left);
    browserRect.setTop(top);
    browserRect.setWidth(width);
    browserRect.setHeight(height);

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
    STUB() << Core::instance()->getKeycodeHashes().key(keyCode) << keyCode; //int)keyCode;

    if(!keyEventValues.contains(keyCode))
    {
        qDebug() << "Key code not registered:" <<  QString("0x").append(QString::number(keyCode, 16)) << keyEventValues;
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


void yasem::WebkitBrowser::register_dependencies()
{
    add_dependency(ROLE_GUI);
}

void yasem::WebkitBrowser::register_roles()
{
    register_role(ROLE_BROWSER);
}

void WebkitBrowser::setOpacity(qint32 alpha)
{
    qDebug() << "setOpacity" << alpha;
    foreach(QWidget* child, webViewList)
    {
        WebView* vChild = qobject_cast<WebView*>(child);
        if(vChild != NULL)
        {
            //QGraphicsOpacityEffect * effect = new QGraphicsOpacityEffect(vChild);
            //effect->setOpacity((float)alpha / 255);

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
    GuiPlugin* gui = dynamic_cast<GuiPlugin*>(PluginManager::instance()->getByRole(PluginRole::ROLE_GUI));
    gui->repaintGui();
}
;
qint32 WebkitBrowser::getOpacity()
{
    return 100;
}

#include "webview.h"
#include "webkitbrowser.h"

#include "pluginmanager.h"
#include "webpage.h"
#include <QWebFrame>
#include <QWebSecurityOrigin>

#include <QKeyEvent>
#include <QPalette>
#include <QPainter>

using namespace yasem;

WebView::WebView(QWidget *parent, WebkitBrowser* browser) :
    QWebView(parent)
{
    setObjectName("WebView");
    this->browser = browser;
    gui = dynamic_cast<GuiPlugin*>(PluginManager::instance()->getByRole("gui"));

    connect(this, &WebView::loadStarted, this, &WebView::onLoadStarted);
    connect(this, &WebView::loadProgress, this, &WebView::onLoadProgress);
    connect(this, &WebView::loadFinished, this, &WebView::onLoadFinished);
    connect(this, &WebView::titleChanged, this, &WebView::onTitleChanged);
    connect(this, &WebView::statusBarMessage, this, &WebView::onStatusBarMessage);
    connect(this, &WebView::linkClicked, this, &WebView::onLinkClicked);
    connect(this, &WebView::selectionChanged, this, &WebView::onSelectionChanged);
    connect(this, &WebView::iconChanged, this, &WebView::onIconChanged);
    connect(this, &WebView::urlChanged, this, &WebView::onUrlChanged);



    //connect(this, &WebView::invalidateWebView, gui, &AbstractOutputPlugin::invalidateWebView);

    this->setFocusPolicy(Qt::StrongFocus);

    loadFixes();

    QPalette palette = this->palette();
    palette.setBrush(QPalette::Base, Qt::transparent);
    page()->setPalette(palette);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    //setAttribute(Qt::WA_NoSystemBackground, true);
    setStyleSheet("background: transparent;");

}

void WebView::keyPressEvent(QKeyEvent *event)
{
    //emit debug(QString("WebView::keyPressEvent(%1)").arg(event->key()));
    QWebView::keyPressEvent(event);
}

void WebView::keyReleaseEvent(QKeyEvent *event)
{
    //emit debug(QString("WebView::keyReleaseEvent(%1)").arg(event->key()));
    QWebView::keyReleaseEvent(event);
}

bool WebView::event(QEvent *event)
{
    /*QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if(keyEvent != NULL && keyEvent->key() > 0)
    {
        emit log(QString("Got key event: %1").arg(keyEvent->key()));
        return false;
    }*/
    /*if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if(keyEvent != NULL)
        {
            //emit debug(QString("key event: %1:%2:%3").arg(keyEvent->key()).arg(keyEvent->nativeScanCode()).arg(keyEvent->nativeVirtualKey()));
        }
        else
        {
            //emit debug("NULL KEY EVENT!");
        }
    }*/
    return QWebView::event(event);
}

void WebView::loadFixes()
{
    STUB();
    webObjectsFix = loadFix("webobjects.js");
    xmlHttpRequestFix = loadFix("XMLHttpRequest.js");
}

QString WebView::loadFix(const QString &name)
{
    STUB();
    QFile res(QString(":/webkitbrowser/js/fix/").append(name));
    res.open(QIODevice::ReadOnly|QIODevice::Text);

    return res.readAll();
}

QWebView *WebView::createWindow(QWebPage::WebWindowType type)
{
    STUB();
    //emit debug(QString("WebView::createWindow(%1)").arg(type));
    Q_UNUSED(type);

    WebView *webView = new WebView(this->parentWidget(), browser);
    WebPage *webPage = new WebPage(webView);
    webPage->stb(qobject_cast<WebPage*>(this->page())->stb());
    webView->setAttribute(Qt::WA_DeleteOnClose, true);
    webView->setPage(webPage);
    webView->show();
    webView->setFocus();
    webView->setGeometry(browser->rect());
    webView->setZoomFactor(browser->scale());
    webView->setStyleSheet("background-color: white;");
    browser->addWebView(webView);
    browser->setWebView(webView);

    //webView->repaint();

    connect(webPage, &WebPage::windowCloseRequested, [=]() {
        browser->removeWebView(webView);
        //Q_ASSERT(webPage->view());
        webPage->view()->close();
        qDebug() << "close window";

        WebView* last = static_cast<WebView*>(browser->getWebViewList().last());
        Q_ASSERT(last);

        browser->setWebView(last);
        browser->resize();

    });


    //emit setTopLevelWebView(webView);

    return webView;
}


void WebView::onLoadStarted()
{
    STUB();
    triggered = false;

}

void WebView::onLoadProgress(int progress)
{
    DEBUG() << "onLoadProgress(" << progress << ")";
    if(progress > 10 && !triggered)
    {
        Q_ASSERT(page());
        Q_ASSERT(browser);
        Q_ASSERT(browser->stb());

        /* Fix for Samsung API objects
         * Some objects may not have type attribute, so QWebView cannot load this objects and it is required
         * to add this attribute manually.
         * The other fix is for object that are not visible;
        */

        page()->mainFrame()->evaluateJavaScript(webObjectsFix);
        browser->stb()->applyFixes();
        triggered = true;
    }
}

void WebView::onLoadFinished(bool finished)
{
    DEBUG() << "onLoadFinished(" << finished << ")";
}

void WebView::onTitleChanged(const QString &title)
{
    DEBUG() << "onTitleChanged(" << title << ")";
}

void WebView::onStatusBarMessage(const QString &text)
{
    DEBUG() << "onStatusBarMessage(" << text << ")";
}

void WebView::onLinkClicked(const QUrl &url)
{
    DEBUG() << "onLinkClicked(" << url << ")";
}

void WebView::onSelectionChanged()
{
    //DEBUG(QString("onSelectionChanged()"));
    //findText(""); // Disabling selection
}

void WebView::onIconChanged()
{
    DEBUG() << "onIconChanged()";
}

void WebView::onUrlChanged(const QUrl &url)
{
    DEBUG() << QString("onUrlChanged(%1)").arg(url.toString());

    #if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
        QWebSecurityOrigin origin(url);
        origin.addAccessWhitelistEntry("http", "", QWebSecurityOrigin::AllowSubdomains);
        origin.addAccessWhitelistEntry("qrc", "", QWebSecurityOrigin::AllowSubdomains);
        origin.addAccessWhitelistEntry("http", "www.youtube.com", QWebSecurityOrigin::AllowSubdomains);
        this->page()->mainFrame()->securityOrigin().allOrigins().append(origin);
    #else
        WARN(QString("Cross-origin resource sharing (CORS) is only available since Qt 5.2. CORS will be disabled!"));
    #endif

    triggered = false;
    //emit invalidateWebView();
}

void WebView::paintEvent(QPaintEvent *e)
{
    /*QPainter p(this);
    p.fillRect(e->rect(), Qt::transparent);
    p.setPen(Qt::NoPen);
    p.setBrush(gradient);
    p.setOpacity(0.6);
    p.drawRoundedRect(rect(), 100, 100);
    p.end();*/
    QWebView::paintEvent(e);
}


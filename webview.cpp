#include "webview.h"
#include "webkitbrowser.h"
#include "profilemanager.h"

#include "pluginmanager.h"
#include "webpage.h"
#include <QWebFrame>
#include <QWebSecurityOrigin>

#include <QKeyEvent>
#include <QPalette>
#include <QPaintEngine>
#include <QPainter>
#include <QBackingStore>
#include <QGraphicsOpacityEffect>

using namespace yasem;

WebView::WebView(QWidget *parent, WebkitBrowser* browser) :
    QWebView(parent)
{
    setObjectName("WebView");
    this->browser = browser;
    gui = dynamic_cast<GuiPlugin*>(PluginManager::instance()->getByRole(ROLE_GUI));

    rendering_started = false;
    m_is_context_menu_valid = false;

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

    setMouseTracking(true);
    readSettings();

    setStyleSheet("background: transparent");

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(showContextMenu(const QPoint&)));
}

void WebView::setupContextMenu()
{
    m_contextMenu = new QMenu(this);

    // Context menu shouldn't be transparent like a browser
    m_contextMenu->setStyleSheet("background: none");

    for(QMenu* submenu: gui->getMenuItems())
    {
        m_contextMenu->addMenu(submenu);
    }
    m_contextMenu->addSeparator();

    m_backToPreviousPageAction = new QAction(tr("Back"), m_contextMenu);
    connect(m_backToPreviousPageAction, &QAction::triggered, []() {
        ProfileManager::instance()->backToPreviousProfile();
    });
    m_contextMenu->addAction(m_backToPreviousPageAction);


    m_openWebInspectorAction = new QAction(tr("Open Developer Tools"), m_contextMenu);
    connect(m_openWebInspectorAction, &QAction::triggered, [=]() {
        ((WebPage*)this->page())->showWebInspector();
    });
    m_contextMenu->addAction(m_openWebInspectorAction);


}

void WebView::showContextMenu(const QPoint &pos)
{
    if(!m_is_context_menu_valid)
        setupContextMenu();

    if(ProfileManager::instance()->canGoBack())
        m_backToPreviousPageAction->setVisible(true);
    else
        m_backToPreviousPageAction->setVisible(false);

    m_contextMenu->exec(mapToGlobal(pos));
}

void WebView::readSettings()
{
    QSettings* settings = Core::instance()->settings();
    settings->beginGroup("WebBrowser");
    mouseBorderThreshold = settings->value("mouse_border_threshold", 50).toInt();
    settings->endGroup();
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

void WebView::mouseMoveEvent(QMouseEvent *e)
{
    //qDebug() << e;
    int position = MOUSE_POSITION::MIDDLE;

    int y_pos = e->pos().y();
    int x_pos = e->pos().x();
    int height = this->height();
    int width = this->width();

    if(y_pos < (mouseBorderThreshold))
        position |= MOUSE_POSITION::TOP;
    else if(y_pos > height - mouseBorderThreshold)
        position |= MOUSE_POSITION::BOTTOM;

    if(x_pos < mouseBorderThreshold)
        position |= MOUSE_POSITION::LEFT;
    else if(x_pos > width - mouseBorderThreshold)
        position |= MOUSE_POSITION::RIGHT;

    emit mousePositionChanged(position);

    QWebView::mouseMoveEvent(e);
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

    WebView *webView = new WebView(NULL, browser);
    webView->setObjectName("Popup web view");
    WebPage *webPage = new WebPage(webView);
    webPage->setObjectName("Popup web page");

    //webPage->stb(qobject_cast<WebPage*>(this->page())->stb());
    webView->setAttribute(Qt::WA_DeleteOnClose, true);
    webView->setPage(webPage);
    webView->show();
    webView->setFocus();

    browser->addWebView(webView);
    browser->setWebView(webView);

    if(type == QWebPage::WebModalDialog)
        webView->setWindowModality(Qt::ApplicationModal);


    connect(webPage, &WebPage::windowCloseRequested, [=]() {
        browser->removeWebView(webView);
        webPage->view()->close();

        WebView* last = static_cast<WebView*>(browser->getWebViewList().last());
        //Q_ASSERT(last);
        browser->setWebView(last);
        browser->resize();

    });

    //browser->resize();
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

        DEBUG() << "Applying fix for <object> tags";
        page()->mainFrame()->evaluateJavaScript(webObjectsFix);
        DEBUG() << "Applying other fixes";
        browser->stb()->applyFixes();
        triggered = true;
    }
}

void WebView::onLoadFinished(bool finished)
{
    DEBUG() << "onLoadFinished(" << finished << ")";
    browser->resize();
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

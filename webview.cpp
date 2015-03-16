#include "webview.h"
#include "webkitpluginobject.h"
#include "profilemanager.h"
#include "guipluginobject.h"
#include "stbpluginobject.h"

#include "pluginmanager.h"
#include "webpage.h"
#include <QWebFrame>
#include <QWebSecurityOrigin>

#include <QKeyEvent>
#include <QPalette>
#include <QPaintEngine>
#include <QPainter>
#include <QBackingStore>
#include <QApplication>
#include <QBitmap>
//#include <QGraphicsOpacityEffect>

using namespace yasem;

WebView::WebView(QWidget *parent, WebkitPluginObject* browser) :
    QWebView(parent),
    m_browser_object(browser)
{
    setObjectName("WebView");
    gui = dynamic_cast<GuiPluginObject*>(PluginManager::instance()->getByRole(ROLE_GUI));

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
    //setAttribute(Qt::WA_NoSystemBackground);
    //setAttribute(Qt::WA_TranslucentBackground);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(showContextMenu(const QPoint&)));

    this->setAttribute(Qt::WA_OpaquePaintEvent, false);
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

void WebView::resizeView(const QRect &containerRect)
{
    DEBUG() << "WebView::resizeView" << containerRect;
    float w_ratio = (float)m_viewportSize.width() / containerRect.width();
    float h_ratio = (float)m_viewportSize.height() / containerRect.height();

    int width;
    int height;

    if(w_ratio > h_ratio)
    {
        width = containerRect.width();
        height = (int)((float)m_viewportSize.height() / w_ratio);
    }
    else
    {
        height = containerRect.height();
        width = (int)((float)m_viewportSize.width() / h_ratio);
    }

    int left =  (int)(((float)containerRect.width() - width) / 2);
    int top = (int)(((float)containerRect.height() - height) / 2);

    m_viewRect.setLeft(left);
    m_viewRect.setTop(top);
    m_viewRect.setWidth(width);
    m_viewRect.setHeight(height);

    m_pageScale = (qreal)m_viewRect.width() / m_viewportSize.width();

    //QRect actualRect(0, 0, m_viewRect.width(), m_viewRect.height());
    setZoomFactor(m_pageScale);
    setGeometry(m_viewRect);
    //page()->setActualVisibleContentRect(m_viewRect);
}

void WebView::setViewportSize(QSize newSize)
{
    DEBUG() << "WebView::setViewportSize";
    m_viewportSize = newSize;
}

QSize WebView::getViewportSize()
{
    return m_viewportSize;
}

qreal WebView::getScale()
{
    return m_pageScale;
}

QRect WebView::getRect()
{
    return m_viewRect;
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

void WebView::mouseMoveEvent(QMouseEvent *e)
{
    //DEBUG() << e;
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

    WebView *webView = new WebView(NULL, m_browser_object);
    webView->setObjectName("Popup web view");
    WebPage *webPage = new WebPage(webView);
    webPage->setObjectName("Popup web page");

    //webPage->stb(qobject_cast<WebPage*>(this->page())->stb());
    webView->setAttribute(Qt::WA_DeleteOnClose, true);
    webView->setPage(webPage);
    webView->show();
    webView->setFocus();

    m_browser_object->addWebView(webView);
    m_browser_object->setWebView(webView);

    if(type == QWebPage::WebModalDialog)
        webView->setWindowModality(Qt::ApplicationModal);


    connect(webPage, &WebPage::windowCloseRequested, [=]() {
        m_browser_object->removeWebView(webView);
        webPage->view()->close();

        WebView* last = static_cast<WebView*>(m_browser_object->getWebViewList().last());
        m_browser_object->setWebView(last);
        m_browser_object->resize();

    });

    //browser->resize();
    return webView;
}

void WebView::setBrowser(WebkitPluginObject *browser)
{
    this->m_browser_object = browser;
}

void WebView::setDefaultBrowser()
{
    setBrowser(dynamic_cast<WebkitPluginObject*>(PluginManager::instance()->getByRole(ROLE_BROWSER)));
}

void WebView::qmlInit()
{
    DEBUG() << "webview qml init";
    setDefaultBrowser();
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
        /* Fix for Samsung API objects
         * Some objects may not have type attribute, so QWebView cannot load this objects and it is required
         * to add this attribute manually.
         * The other fix is for objects that are not visible;
        */


        if(page() != NULL && m_browser_object->stb() != NULL)
        {
            DEBUG() << "Applying fix for <object> tags";
            page()->mainFrame()->evaluateJavaScript(webObjectsFix);
            DEBUG() << "Applying other fixes";
            m_browser_object->stb()->applyFixes();
            triggered = true;
        }
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
    m_browser_object->resize();

    //emit invalidateWebView();
}

/**
 * @brief yasem::WebView::paintEvent
 * @param event
 *
 * This code was partially copy/pasted from Qt's qtwebkit/Source/WebKit/qt/WidgetApi/qwebview.cpp
 */
void yasem::WebView::paintEvent(QPaintEvent *event)
{

    if (!page())
        return;
#ifdef QWEBKIT_TIME_RENDERING
    QTime time;
    time.start();
#endif

    QWebFrame *frame = page()->mainFrame();
    QPainter painter(this);


    //painter.setCompositionMode(QPainter::CompositionMode_Source);

    //painter.fillRect(event->rect(), QColor(0, 0, 255, 128));
    //painter.setCompositionMode(QPainter::CompositionMode_Xor);

    //QImage image(size(), QImage::Format_ARGB32_Premultiplied);
    QImage image(size(), QImage::Format_ARGB32_Premultiplied);
    QPainter imagePainter(&image);
    imagePainter.setRenderHints(renderHints());
    frame->render(&imagePainter, event->region());
    imagePainter.end();

    QImage mask2 = image.createMaskFromColor(qRgba(0xE0, 0xFF, 0xFF, 0xFF), Qt::MaskInColor);

    QImage mask(size(), QImage::Format_ARGB32_Premultiplied);
    //mask.fill(qRgba(0xE0, 0xFF, 0xFF, 0xFF));
    mask.fill(Qt::black);


    //painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
    //painter.fillRect(image.rect(), QColor(0xE0, 0xFF, 0xFF, 0x88));

   // painter.end();

    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
   //painter.drawImage(QPoint(event->rect().left(), event->rect().top()), image, event->rect());
    painter.drawImage(QPoint(event->rect().left(), event->rect().top()), mask2, event->rect());
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    painter.fillRect(image.rect(), QColor(0xE0, 0xFF, 0xFF, 0x00));
    painter.setCompositionMode(QPainter::CompositionMode_DestinationAtop);
    painter.drawImage(QPoint(event->rect().left(), event->rect().top()), image, event->rect());


    /*painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

        // Outter Rect
        painter.fillRect(QRect(0,0,100,100), QBrush(Qt::green));

        // Inner Rect
        painter.fillRect(QRect(20,20,40,40), QBrush(Qt::transparent));*/

    //

    //painter.drawImage(QPoint(event->rect().left(), event->rect().top()), mask2, event->rect());

    //painter.setCompositionMode(QPainter::CompositionMode_Xor);
    //painter.fillRect(image.rect(), Qt::black);

    //painter.setCompositionMode(QPainter::CompositionMode_Xor);


    ////

    //painter.drawImage(QPoint(event->rect().left(), event->rect().top()), mask, event->rect());
    //
   // painter.drawImage(QPoint(event->rect().left(), event->rect().top()), mask2, event->rect());
    //

    //painter.setCompositionMode(QPainter::CompositionMode_Difference);

    //mask2.save("/tmp/temp_mask2.png");

    //painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    //painter.fillRect(event->rect(), Qt::transparent);

    //image.save("/tmp/temp_image.png");
    //mask.save("/tmp/temp_mask1.png");


    //pixmap.save("/tmp/temp_pixmap.png");


    /*QPixmap temp(size());
    temp.fill(Qt::transparent);

    QPainter p(&temp);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.drawImage(QPoint(event->rect().left(), event->rect().top()), image, event->rect());
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    //p.fillRect(temp.rect(), QColor(0x00, 0x00, 0x00, 0x20));
    p.fillRect(temp.rect(), QColor(0xE0, 0xFF, 0xFF, 0x20));
    p.end();
    painter.drawPixmap(QPoint(event->rect().left(), event->rect().top()), temp, event->rect());*/


#ifdef    QWEBKIT_TIME_RENDERING
    int elapsed = time.elapsed();
    qDebug() << "paint event on " << ev->region() << ", took to render =  " << elapsed;
#endif
}



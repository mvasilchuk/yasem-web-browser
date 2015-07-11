#include "webview.h"
#include "webkitpluginobject.h"
#include "profilemanager.h"
#include "guipluginobject.h"
#include "stbpluginobject.h"
#include "mediaplayerpluginobject.h"
#include "statistics.h"
#include "networkstatistics.h"

#include "pluginmanager.h"
#include "webpage.h"
#include <QWebFrame>

#include <QKeyEvent>
#include <QPalette>
#include <QPaintEngine>
#include <QPainter>
#include <QBackingStore>
#include <QApplication>
#include <QBitmap>
//#include <QGraphicsOpacityEffect>

using namespace yasem;



WebView::WebView(QWidget *parent) :
    QWebView(parent),
    m_browser(__get_plugin<SDK::BrowserPluginObject*>(SDK::ROLE_BROWSER)),
    m_allow_repaint(true),
    m_allow_transparency(true),
    m_skip_full_render(false)
{
    setObjectName("WebView");
    gui = __get_plugin<SDK::GuiPluginObject*>(SDK::ROLE_GUI);
    m_player = __get_plugin<SDK::MediaPlayerPluginObject*>(SDK::ROLE_MEDIA);

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
#ifndef USE_REAL_TRANSPARENCY
    setStyleSheet("background: transparent");
#endif // USE_REAL_TRANSPARENCY

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(showContextMenu(const QPoint&)));

#ifdef USE_REAL_TRANSPARENCY
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAutoFillBackground(false);

    QPalette pal = palette();
    pal.setBrush(QPalette::Base, Qt::transparent);
    pal.setBrush(QPalette::Background, Qt::transparent);
    setPalette(pal);

    connect(m_player, &MediaPlayerPluginObject::started, this, &WebView::fullUpdate);
    connect(m_player, &MediaPlayerPluginObject::stopped, this, &WebView::fullUpdate);
    connect(m_player, &MediaPlayerPluginObject::paused, this, &WebView::fullUpdate);
#endif //USE_REAL_TRANSPARENCY
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
        SDK::ProfileManager::instance()->backToPreviousProfile();
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

    if(SDK::ProfileManager::instance()->canGoBack())
        m_backToPreviousPageAction->setVisible(true);
    else
        m_backToPreviousPageAction->setVisible(false);

    m_contextMenu->exec(mapToGlobal(pos));
}

void WebView::resizeView(const QRect &containerRect)
{
    DEBUG() << "WebView::resizeView" << objectName() << containerRect;
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

    setZoomFactor(m_pageScale);
    setGeometry(m_viewRect);
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
    QSettings* settings = SDK::Core::instance()->settings();
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
    int position = SDK::MOUSE_POSITION::MIDDLE;

    int y_pos = e->pos().y();
    int x_pos = e->pos().x();
    int height = this->height();
    int width = this->width();

    if(y_pos < (mouseBorderThreshold))
        position |= SDK::MOUSE_POSITION::TOP;
    else if(y_pos > height - mouseBorderThreshold)
        position |= SDK::MOUSE_POSITION::BOTTOM;

    if(x_pos < mouseBorderThreshold)
        position |= SDK::MOUSE_POSITION::LEFT;
    else if(x_pos > width - mouseBorderThreshold)
        position |= SDK::MOUSE_POSITION::RIGHT;

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
    WebView *webView = new WebView();
    WebPage *newWeb = new WebPage(webView);
    webView->setPage(newWeb);
    webView->show();
    return webView;
}


void WebView::onLoadStarted()
{
    STUB();
    triggered = false;
}

void WebView::onLoadProgress(int progress)
{
    show(); // Fix for child windows
    setFocus();

    DEBUG() << "onLoadProgress(" << progress << ")";

    if(progress > 10 && !triggered)
    {
        /* Fix for Samsung API objects
         * Some objects may not have type attribute, so QWebView cannot load this objects and it is required
         * to add this attribute manually.
         * The other fix is for objects that are not visible;
        */

        WebPage* _page = dynamic_cast<WebPage*>(page());
        if(_page != NULL)
        {
            DEBUG() << "Applying fix for <object> tags";
            _page->mainFrame()->evaluateJavaScript(webObjectsFix);
            DEBUG() << "Applying other fixes";
            _page->stb()->applyFixes();
            triggered = true;
        }
    }
}


void WebView::onLoadFinished(bool finished)
{
    DEBUG() << "onLoadFinished(" << finished << ")";
    SDK::Core::instance()->statistics()->network()->print();
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

    triggered = false;
    m_browser->resize();
    raise();
}

#define RECT_STR(rect) QString("[%1:%2 %3:%4]").arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height())

/**
 * @brief yasem::WebView::paintEvent
 * @param event
 *
 * This code was partially copy/pasted from Qt's qtwebkit/Source/WebKit/qt/WidgetApi/qwebview.cpp
 *
 * FIXME: This method consumes a lot of CPU. Need to be rewritten.
 * TODO: Move this method into GUI plugin
 */
void yasem::WebView::paintEvent(QPaintEvent *event)
{
#ifdef USE_REAL_TRANSPARENCY
    if (!page() || !m_allow_repaint)
        return;
#define QWEBKIT_TIME_RENDERING

#ifdef QWEBKIT_TIME_RENDERING
    QTime full_time;
    full_time.start();
#endif
    //DEBUG() << "paint" << event->isAccepted() << event->spontaneous() << sender();

    WebPage* w_page = dynamic_cast<WebPage*>(page());
    qreal page_opacity = w_page->getOpacity();
    qreal video_opacity = m_player->getOpacity();
    QRect pixmap_rect = m_render_pixmap.rect();
    QPainter painter(this);

    if(!m_skip_full_render)
    {
        QWebFrame *frame = page()->mainFrame();

        QColor chroma_color = QColor::fromRgb(w_page->getChromaKey().rgb() & w_page->getChromaMask().rgb());

        m_render_pixmap.fill(Qt::transparent);

        QPainter pixmapPainter(&m_render_pixmap);
        frame->render(&pixmapPainter);
        pixmapPainter.end();

        if(w_page->isChromaKeyEnabled())
        {
            QBitmap mask = m_render_pixmap.createMaskFromColor(chroma_color, Qt::MaskInColor);
            m_render_pixmap.setMask(mask);
        }
    }

    m_skip_full_render = false;

#ifdef QWEBKIT_TIME_RENDERING
    QTime player_render;
    player_render.start();
#endif

    m_video_frame = m_player->render();
    QPoint video_pos = m_player->getWidgetPos();
    QRect video_rect = m_video_frame.rect();

#ifdef QWEBKIT_TIME_RENDERING
    int player_render_time = player_render.elapsed();

#endif

    // Video widget is moved to be under webview, so now we should reset its rect position
    if(m_player->isFullscreen())
    {
        video_pos = QPoint(0, 0);
    }

    m_skip_full_render = false;

    if(m_browser_object->getTopWidget() == WebkitPluginObject::TOP_WIDGET_BROWSER)
    {
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.setOpacity(video_opacity);
        painter.drawPixmap(video_pos, m_video_frame, video_rect);

        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.setOpacity(page_opacity);
        painter.drawPixmap(QPoint(0, 0), m_render_pixmap, pixmap_rect);
    }
    else
    {
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.setOpacity(page_opacity);
        painter.drawPixmap(QPoint(0, 0), m_render_pixmap, pixmap_rect);

        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.setOpacity(video_opacity);
        painter.drawPixmap(video_pos, m_video_frame, video_rect);
    }

#ifdef    QWEBKIT_TIME_RENDERING
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawText(QPoint(30, 30), QString("WebView: %1 %2 %3")
                     .arg(RECT_STR(rect()))
                     .arg(RECT_STR(event->region().boundingRect()))
                     .arg(full_time.elapsed()));

    int x = m_video_frame.rect().x();
    int y = m_video_frame.rect().y();
    int width = m_video_frame.rect().width();
    int height = m_video_frame.rect().height();
    painter.drawText(QPoint(30, 50), QString("Video rect: [%1:%2] [%3x%4], render time: %5").arg(x).arg(y).arg(width).arg(height).arg(player_render_time));

    painter.drawText(QPoint(30, 70), QString("Top widget: %1")
                     .arg(m_browser_object->getTopWidget() == WebkitPluginObject::TOP_WIDGET_BROWSER ? "browser" : "player" ));
    //qDebug() << "paint event on " << event->region() << ", took to render =  " << elapsed;
#endif

    painter.end();
#else
    QWebView::paintEvent(event);
#endif //USE_REAL_TRANSPARENCY
}


void yasem::WebView::resizeEvent(QResizeEvent *event)
{
    move(m_viewRect.left(), m_viewRect.top());
    QWebView::resizeEvent(event);
}

void WebView::fullUpdate()
{
    m_skip_full_render = false;
    repaint(rect());
}

void WebView::updateTopWidget()
{
    SDK::BrowserPluginObject* browser = __get_plugin<SDK::BrowserPluginObject*>(SDK::ROLE_BROWSER);
    Q_ASSERT(browser);
    switch(browser->getTopWidget())
    {
        case SDK::BrowserPluginObject::TOP_WIDGET_BROWSER:
        {
            DEBUG() << "raising browser";
            raise();
            break;
        }
        case SDK::BrowserPluginObject::TOP_WIDGET_PLAYER:
        {
            DEBUG() << "raising player";
            m_player->widget()->raise();
            break;
        }
        default: {
            DEBUG() << "raising unknown";
            break;
        }
    }
}

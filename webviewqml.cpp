#include "webviewqml.h"
#include "webview.h"
#include "pluginmanager.h"
#include "browserpluginobject.h"
#include "webpage.h"

#include <QPoint>
#include <QApplication>

using namespace yasem;

WebViewQml::WebViewQml(QQuickItem * parent):
    QQuickPaintedItem(parent)
{
    browser = dynamic_cast<BrowserPluginObject*>(PluginManager::instance()->getByRole(ROLE_BROWSER));
    browser->setUseQml(true);

    page = dynamic_cast<WebPage*>(browser->createNewPage());
    webView = page->webView();
    webView->moveToThread(qApp->thread());

    //webView = new QWebView();
    //webView->setFocusPolicy(Qt::StrongFocus);

    setFlag(ItemHasContents, true);
    setFlag(ItemClipsChildrenToShape, true);
    //setFlag(ItemAcceptsDrops, true);
    setFlag(ItemAcceptsInputMethod, true);
    setFlag(ItemIsFocusScope, true);

    //setOpaquePainting(true);
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);

    setFocus(true);
}

WebViewQml::~WebViewQml()
{
    if(webView != NULL)
        delete webView;
}

void yasem::WebViewQml::componentComplete()
{
    QQuickItem::componentComplete();

    //QRect rect = this->boundingRect().toRect();
    QRectF rect(x(), y(), width(), height());
    webView->resizeView(rect.toRect());
    setFocus(true);
}


void yasem::WebViewQml::paint(QPainter *painter)
{
    QRect rect = webView->getRect();
    QPoint point(rect.left(), rect.top());
    QRegion region(0, 0, rect.width(), rect.height());

    painter->beginNativePainting();
    webView->render(painter, point, region);
    painter->endNativePainting();
}

void WebViewQml::setWebView(WebView *webView)
{
    this->webView = webView;
}


void yasem::WebViewQml::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if(webView != NULL)
        webView->resizeView(newGeometry.toRect());
}

bool WebViewQml::event(QEvent *event)
{
   /*
    MouseButtonPress = 2,                   // mouse button pressed
    MouseButtonRelease = 3,                 // mouse button released
    MouseButtonDblClick = 4,                // mouse button double click
    MouseMove = 5,                          // mouse move
    KeyPress = 6,                           // key pressed
    KeyRelease = 7,                         // key released
    FocusIn = 8,                            // keyboard focus received
    FocusOut = 9,                           // keyboard focus lost
    FocusAboutToChange = 23,                // keyboard focus is about to be lost
    Enter = 10,                             // mouse enters widget
    Leave = 11,                             // mouse leaves widget
    Paint = 12,                             // paint widget
    Wheel = 31,                             // wheel event
    Clipboard = 40,                         // internal clipboard event
    DragEnter = 60,                         // drag moves into widget
    DragMove = 61,                          // drag moves in widget
    DragLeave = 62,                         // drag leaves or is cancelled
    Drop = 63,                              // actual drop
    */
    //if(event->type() != QEvent::MetaCall && event->type() != QEvent::HoverMove)
    //    DEBUG() << event;

    switch(event->type())
    {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::Wheel:
        {
            QRect rect = webView->geometry();
            QMouseEvent* old_event = static_cast<QMouseEvent*>(event);
            QPoint localPos = old_event->pos();
            localPos.setX(localPos.x() - rect.left());
            localPos.setY(localPos.y() - rect.top());

            QMouseEvent* new_event = new QMouseEvent(old_event->type(),
                                                     localPos,
                                                     old_event->button(),
                                                     old_event->buttons(),
                                                     old_event->modifiers());

            DEBUG() << "New event" << new_event;
            QApplication::sendEvent(webView, new_event);
            break;
        }
        /*case QEvent::HoverEnter:
        case QEvent::HoverLeave:
        case QEvent::HoverMove:
        {
            QRect rect = webView->geometry();

            QHoverEvent* old_event = static_cast<QHoverEvent*>(event);
            QPointF pos = old_event->posF();
            pos.setX(pos.x() - rect.left());
            pos.setY(pos.y() - rect.top());

            QEvent::Type t = QEvent::None;

            switch(event->type())
            {
                case QEvent::HoverEnter: { t = QEvent::Enter; break; }
                case QEvent::HoverLeave: { t = QEvent::Leave; break; }
                //case QEvent::HoverMove: { t = QEvent::MouseMove; break; }
            }

            QMouseEvent* new_event = new QMouseEvent(
                        t,
                        pos,
                        Qt::NoButton,
                        { Qt::NoButton },
                        { Qt::NavigationModeCursorAuto} );
            QApplication::postEvent(webView, new_event);
            break;
        }*/
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::Clipboard:
        case QEvent::Speech:
        {
            QApplication::sendEvent(webView, event);
            break;
        }
        case QEvent::FocusIn:
        case QEvent::FocusOut:
        {
            QFocusEvent* focus = static_cast<QFocusEvent*>(event);
            QApplication::sendEvent(webView, focus);
            return true;
            break;
        }

        default: {
            break;
        }
    }

    event->setAccepted(true);
    return QQuickPaintedItem::event(event);
}

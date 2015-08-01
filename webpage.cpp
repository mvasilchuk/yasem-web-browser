#include "qtwebpage.h"
#include "stbpluginobject.h"
#include "webpluginfactoryimpl.h"
#include "pluginmanager.h"
#include "profilemanager.h"
#include "gui.h"
#include "interceptormanager.h"
#include "webview.h"
#include "browserkeyevent.h"
#include "webkitpluginobject.h"
#include "abstracthttpproxy.h"
#include "macros.h"

#include "cmd_line.h"
#include "stbprofile.h"

#include <QWebInspector>
#include <QWebFrame>
#include <QKeyEvent>
#include <QMessageBox>
#include <QFileInfo>
#include <QNetworkProxy>
#include <QWebSecurityOrigin>

using namespace yasem;

QtWebPage::QtWebPage(WebView *parent) :
    QWebPage(parent),
    m_browser(SDK::__get_plugin<SDK::Browser*>(SDK::ROLE_BROWSER)),
    m_chromakey(QColor(0, 0, 0)),
    m_chromamask(QColor(0xFF, 0xFF, 0xFF)),
    m_opacity(1.0),
    m_chromakey_enabled(true),
    m_interceptor(NULL)
{
    this->parent = parent;
    this->setObjectName("WebPage");
    this->stb(NULL);

    defaultUserAgent = "Mozilla/5.0 (%Platform%%Security%%Subplatform%) AppleWebKit/%WebKitVersion% (KHTML, like Gecko) %AppVersion Safari/%WebKitVersion%";
    customUserAgent = "";

    settings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls,true);
    settings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls,true);
    settings()->setAttribute(QWebSettings::FrameFlatteningEnabled,true);
    settings()->setAttribute(QWebSettings::PluginsEnabled,true);
    //settings()->setAttribute(QWebSettings::JavaEnabled,true); //Not using as isn't supported by Qt yet
    settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);
    settings()->setAttribute(QWebSettings::JavascriptCanCloseWindows, true);
    settings()->setAttribute(QWebSettings::SpatialNavigationEnabled, true);
    settings()->setAttribute(QWebSettings::LinksIncludedInFocusChain, true);
    settings()->setAttribute(QWebSettings::LocalStorageEnabled, true);
    settings()->setAttribute(QWebSettings::AcceleratedCompositingEnabled, true);
    settings()->setAttribute(QWebSettings::Accelerated2dCanvasEnabled, true);
    settings()->enablePersistentStorage();

    //settings()->setAttribute(QWebSettings::LinksIncludedInFocusChain, true);

    mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

    pluginFactory = WebPluginFactory::setInstance(new WebPluginFactoryImpl());

    connect(this->mainFrame(), &QWebFrame::javaScriptWindowObjectCleared, this, &QtWebPage::attachJsStbApi);

    settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);


    m_web_inspector.setPage(this);
    m_web_inspector.setGeometry(QRect(0, 0, 1000, 800));

    if(QCoreApplication::arguments().contains(CMD_LINE_DEVELOPER_TOOLS))
        showWebInspector();

    setForwardUnsupportedContent(true);
    connect(this, &QtWebPage::unsupportedContent, []( QNetworkReply * reply ){
        qDebug() << "FIXME: Unsupported content" << reply;
        delete reply;
    });
}

WebView *QtWebPage::webView()
{
    return parent;
}

bool QtWebPage::isChildWindow()
{
    return this->objectName() == "Popup web page";
}

void QtWebPage::attachJsStbApi()
{
    STUB();

    recreateObjects();
    if(m_stb_plugin)
    {
        QHash<QString, QObject*> list = m_stb_plugin->getStbApiList();
        QHashIterator<QString, QObject*> iterator(list);
        //if(list.size() == 0)
        //    ERROR("STB Object is empty or  not initialized!!!");
        while(iterator.hasNext())
        {
            iterator.next();
            QString name = iterator.key();
            QObject* object = iterator.value();

            if(object == NULL)
            {
                ERROR() << "JS API object is not an instance of QObject";
                continue;
            }
            DEBUG() << "Inserting API:" << this << object->metaObject()->className();

            this->mainFrame()->addToJavaScriptWindowObject(name, object);
        }
    }
    DEBUG() << "JS API attached";
}

void QtWebPage::setupInterceptor()
{
    if(m_interceptor)
        delete m_interceptor;
    m_interceptor = new InterceptorManager(this);
    m_interceptor->setPage(this);

    this->setNetworkAccessManager(m_interceptor);

    connect(this, &QtWebPage::load_started, m_browser, &SDK::Browser::page_loading_started);
    connect(m_interceptor, &InterceptorManager::connection_encrypted, m_browser, &SDK::Browser::connection_encrypted);
    connect(m_interceptor, &InterceptorManager::encryption_error, m_browser, &SDK::Browser::encryption_error);
}

void QtWebPage::triggerAction(QWebPage::WebAction action, bool checked)
{
    if(action == QWebPage::Back)
    {
        SDK::ProfileManager::instance()->backToPreviousProfile();
    }
    else
        QWebPage::triggerAction(action, checked);
}

void QtWebPage::javaScriptAlert ( QWebFrame * frame, const QString & msg )
{
    Q_UNUSED(frame);
    LOG() << "[JS ALERT]:" << msg;
}

bool QtWebPage::javaScriptConfirm ( QWebFrame * frame, const QString & msg )
{
    Q_UNUSED(frame);
    LOG() << "[JS CONFIRM]:" << msg;

    const QString title = QObject::tr("Confirm action");

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this->parent, title, msg,
                                QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
    return reply == QMessageBox::Yes;
}

void QtWebPage::javaScriptConsoleMessage ( const QString & message, int lineNumber, const QString & sourceID )
{
    LOG() << qPrintable(QString("[JS MESSAGE] (%1: %2): %3").arg(sourceID).arg(lineNumber).arg(message));
}

bool QtWebPage::javaScriptPrompt ( QWebFrame * frame, const QString & msg, const QString & defaultValue, QString * result )
{
    Q_UNUSED(frame);
    LOG() << QString("WebPage::javaScriptPrompt(%1, %2, %3)").arg(msg).arg(defaultValue).arg(*result);
    return false;
}

bool QtWebPage::event(QEvent *event)
{
    bool result = false;
    if(event->type() == QEvent::KeyPress)
    {
        if(!stb())
        {
            qWarning() << "No STB API found!";
            return false;
        }
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        Qt::KeyboardModifiers modifiers = keyEvent->modifiers();
        bool hasShift = (modifiers & Qt::ShiftModifier) == Qt::ShiftModifier;
        bool hasCtrl = (modifiers & Qt::ControlModifier) == Qt::ControlModifier;
        //bool hasAlt = (modifiers & Qt::AltModifier) == Qt::AltModifier;
        //bool hasMeta = (modifiers & Qt::MetaModifier) == Qt::MetaModifier;


        switch(keyEvent->key())
        {
            case Qt::Key_Left:      {
                if(hasShift)
                    result = receiveKeyCode(SDK::GUI::RC_KEY_REWIND);
                else
                    result = receiveKeyCode(SDK::GUI::RC_KEY_LEFT);
                break;
            }
            case Qt::Key_Right:     {
                if(hasShift)
                    result = receiveKeyCode(SDK::GUI::RC_KEY_FAST_FORWARD);
                else
                    result = receiveKeyCode(SDK::GUI::RC_KEY_RIGHT);
                break;
            }
            case Qt::Key_Up:        {
                    result = receiveKeyCode(SDK::GUI::RC_KEY_UP);
                break;
            }
            case Qt::Key_Down:      {
                result = receiveKeyCode(SDK::GUI::RC_KEY_DOWN);
                break;
            }

            case Qt::Key_Return:
            case Qt::Key_Enter:     result = receiveKeyCode(SDK::GUI::RC_KEY_OK);        break;

            case Qt::Key_Home:
            case Qt::Key_Back:      result = receiveKeyCode(SDK::GUI::RC_KEY_BACK);      break;
            case Qt::Key_Escape:    result = receiveKeyCode(SDK::GUI::RC_KEY_EXIT);      break;

            case Qt::Key_F1:        result = receiveKeyCode(SDK::GUI::RC_KEY_RED);       break;
            case Qt::Key_F2:        result = receiveKeyCode(SDK::GUI::RC_KEY_GREEN);     break;
            case Qt::Key_F3:        result = receiveKeyCode(SDK::GUI::RC_KEY_YELLOW);    break;
            case Qt::Key_F4:        result = receiveKeyCode(SDK::GUI::RC_KEY_BLUE);      break;

            case Qt::Key_Tab:       result = receiveKeyCode(SDK::GUI::RC_KEY_MENU);      break;
            case Qt::Key_PageUp:    result = receiveKeyCode(SDK::GUI::RC_KEY_PAGE_UP);   break;
            case Qt::Key_PageDown:  result = receiveKeyCode(SDK::GUI::RC_KEY_PAGE_DOWN); break;
            case Qt::Key_Control:   result = receiveKeyCode(SDK::GUI::RC_KEY_INFO);      break;

            case Qt::Key_0:         result = receiveKeyCode(SDK::GUI::RC_KEY_NUMBER_0);  break;
            case Qt::Key_1:         result = receiveKeyCode(SDK::GUI::RC_KEY_NUMBER_1);  break;
            case Qt::Key_2:         result = receiveKeyCode(SDK::GUI::RC_KEY_NUMBER_2);  break;
            case Qt::Key_3:         result = receiveKeyCode(SDK::GUI::RC_KEY_NUMBER_3);  break;
            case Qt::Key_4:         result = receiveKeyCode(SDK::GUI::RC_KEY_NUMBER_4);  break;
            case Qt::Key_5:         result = receiveKeyCode(SDK::GUI::RC_KEY_NUMBER_5);  break;
            case Qt::Key_6:         result = receiveKeyCode(SDK::GUI::RC_KEY_NUMBER_6);  break;
            case Qt::Key_7:         result = receiveKeyCode(SDK::GUI::RC_KEY_NUMBER_7);  break;
            case Qt::Key_8:         result = receiveKeyCode(SDK::GUI::RC_KEY_NUMBER_8);  break;
            case Qt::Key_9:         result = receiveKeyCode(SDK::GUI::RC_KEY_NUMBER_9);  break;

            case Qt::Key_Menu:      result = receiveKeyCode(SDK::GUI::RC_KEY_MENU);      break;

            case Qt::Key_VolumeDown:    result = receiveKeyCode(SDK::GUI::RC_KEY_VOLUME_DOWN);    break;
            case Qt::Key_VolumeUp:      result = receiveKeyCode(SDK::GUI::RC_KEY_VOLUME_UP);      break;
            case Qt::Key_VolumeMute:    result = receiveKeyCode(SDK::GUI::RC_KEY_MUTE);           break;

            case Qt::Key_MediaTogglePlayPause:      result = receiveKeyCode(SDK::GUI::RC_KEY_PLAY_PAUSE);    break;
            case Qt::Key_MediaPlay:                 result = receiveKeyCode(SDK::GUI::RC_KEY_PLAY);          break;
            case Qt::Key_MediaPause:                result = receiveKeyCode(SDK::GUI::RC_KEY_PAUSE);         break;
            case Qt::Key_MediaStop:     result = receiveKeyCode(SDK::GUI::RC_KEY_STOP);          break;
            case Qt::Key_MediaPrevious: result = receiveKeyCode(SDK::GUI::RC_KEY_REWIND);        break;
            case Qt::Key_MediaNext:     result = receiveKeyCode(SDK::GUI::RC_KEY_FAST_FORWARD);  break;
            case Qt::Key_Info:          result = receiveKeyCode(SDK::GUI::RC_KEY_INFO);  break;

            case Qt::Key_F11:
            {
                    SDK::GUI::instance()->setFullscreen(!SDK::GUI::instance()->getFullscreen());
                    break;
            }


            default:
            {
                if(!keyEvent->text().isEmpty())
                {
                    //result = receiveKeyCode(RC_KEY_FAST_FORWARD);
                }
                else
                    qWarning() << "No keycode found: %1" <<  QString("0x").append(QString::number(keyEvent->key(), 16));
            }
        }
    }
    else if(event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

        switch(mouseEvent->button())
        {
            //case Qt::LeftButton:    result = receiveKeyCode(RC_KEY_OK);        break;
            //case Qt::RightButton:   result = receiveKeyCode(RC_KEY_BACK);      break;
            default:
            {
                //WARN() << "No keycode found:" << mouseEvent->button();
                return QWebPage::event(event);
                break;
            }
        }

    }

    event->setAccepted(result);

    return result || QWebPage::event(event);
}

bool QtWebPage::receiveKeyCode(SDK::GUI::RcKey keyCode)
{
    STUB() << SDK::GUI::instance()->getRcKeyName(keyCode) << keyCode; //int)keyCode;

    WebkitPluginObject* browser = dynamic_cast<WebkitPluginObject*>(m_browser);

    BrowserKeyEvent* keyEvent = browser->getKeyEventValues()[keyCode];

    if(keyEvent == NULL)
    {
        qDebug() << "Key code not registered:" <<  QString("0x").append(QString::number(keyCode, 16)) <<  browser->getKeyEventValues();
        return false;
    }
    else
    {
        QString str = keyEvent->toString();
        evalJs(QString("javascript: %1").arg(str));
        return true;
    }
    return false;
}



void QtWebPage::evalJs(const QString &js)
{
    mainFrame()->evaluateJavaScript(js);
}

QColor QtWebPage::getChromaKey() const
{
    return m_chromakey;
}

void QtWebPage::setChromaKey(QColor color)
{
    m_chromakey = color;
}

QColor QtWebPage::getChromaMask() const
{
    return m_chromamask;
}

void QtWebPage::setChromaMask(QColor color)
{
    m_chromamask = color;
}

void QtWebPage::setOpacity(float opacity)
{
    DEBUG() << "================= opacity" << opacity;
    m_opacity = opacity;
}

float QtWebPage::getOpacity()  const
{
    return m_opacity;
}

bool QtWebPage::isChromaKeyEnabled() const
{
    return m_chromakey_enabled;
}

void QtWebPage::setChromaKeyEnabled(bool enabled)
{
    m_chromakey_enabled = enabled;
}

void QtWebPage::reset()
{
    m_chromakey_enabled = true;
    m_chromakey = QColor(0, 0, 0);
    m_chromamask = QColor(0xFF, 0xFF, 0xFF);
    m_opacity = 1.0;
}

bool QtWebPage::stb(SDK::StbPluginObject *plugin)
{
    STUB();
    this->m_stb_plugin = plugin;
    return true;
}

SDK::StbPluginObject *QtWebPage::stb()
{
    return this->m_stb_plugin;
}

void QtWebPage::setUserAgent(const QString &userAgent)
{
    STUB() << userAgent;
    customUserAgent = userAgent;
}

QUrl QtWebPage::handleUrl(QUrl url)
{
    SDK::StbPluginObject* stbPlugin = stb();
    if(stbPlugin != NULL)
    {
        return stbPlugin->handleUrl(url);
    }

    return url;
}

void QtWebPage::recreateObjects()
{
    STUB();

    SDK::Profile* profile = SDK::ProfileManager::instance()->getActiveProfile();

    if(profile)
    {
        SDK::StbPluginObject* stbPlugin = profile->getProfilePlugin();
        stb(stbPlugin);
        stbPlugin->initObject(this);
    }
    else
        qWarning() << "Profile not found!";

}

void QtWebPage::resetPage()
{
    STUB();
    pluginFactory->getPluginList().clear();
    SDK::Profile* profile =  SDK::ProfileManager::instance()->getActiveProfile();

    if(profile)
    {
        SDK::StbPluginObject* stbPlugin = profile->getProfilePlugin();
        if(stbPlugin)
        {
            pluginFactory->addPlugin(stbPlugin);
            setPluginFactory(pluginFactory);
            pluginFactory->refreshPlugins();
        }
        else
            qWarning() << "STB plugin not found!";
    }
    else
    {
        qWarning() << "Profile not found!";
    }
}

void QtWebPage::showWebInspector()
{
    m_web_inspector.show();
}

QString QtWebPage::userAgentForUrl(const QUrl & url) const
{
    Q_UNUSED(url);
    QString ua = customUserAgent != "" ? customUserAgent : defaultUserAgent;

    //qDebug() << "ua:" << ua;
    return ua;
}

void QtWebPage::close()
{
    this->parent->close();
    emit closed();
}


void QtWebPage::setPageViewportSize(QSize new_size)
{
    webView()->setViewportSize(new_size);
}

QSize QtWebPage::getVieportSize()
{
    return webView()->getViewportSize();
}


qreal QtWebPage::scale()
{
    return webView()->getScale();
}


QRect QtWebPage::getPageRect()
{
    return webView()->getRect();
}


bool QtWebPage::load(const QUrl &url)
{
    DEBUG() << "load:" << url;
    /*if(url.toString().startsWith("file://"))
    {
        QFile file(url.toString().replace("file://", ""));
        QFileInfo info(file);
        this->rootDir = info.absoluteDir().path();
    }*/

#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
    QWebSecurityOrigin origin(url);
    origin.addAccessWhitelistEntry("http", "", QWebSecurityOrigin::AllowSubdomains);
    origin.addAccessWhitelistEntry("https", "", QWebSecurityOrigin::AllowSubdomains);
    origin.addAccessWhitelistEntry("qrc", "", QWebSecurityOrigin::AllowSubdomains);
    origin.addAccessWhitelistEntry("http", "www.youtube.com", QWebSecurityOrigin::AllowSubdomains);
    origin.addAccessWhitelistEntry("https", "www.youtube.com", QWebSecurityOrigin::AllowSubdomains);
    origin.addAccessWhitelistEntry(url.scheme(), url.host(), QWebSecurityOrigin::AllowSubdomains);
    mainFrame()->securityOrigin().allOrigins().append(origin);
#else
    WARN(QString("Cross-origin resource sharing (CORS) is only available since Qt 5.2. CORS will be disabled!"));
#endif

    resetPage();
    setupInterceptor();
    int max_rps = SDK::ProfileManager::instance()->getActiveProfile()->get(CONFIG_LIMIT_MAX_REQUESTS, "0").toInt();
    if(max_rps > 0)
    {
        SDK::HttpProxy* proxy = SDK::__get_plugin<SDK::HttpProxy*>(SDK::ROLE_HTTP_PROXY);
        if(proxy != NULL)
        {
            if(proxy->isRunning())
                proxy->stopServer();
            proxy->setMaxRequestPerSecond(max_rps);
            proxy->startServer();
            m_interceptor->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, proxy->hostName(), proxy->port()));
        }
        else
            WARN() << "Http proxy not found! Requests will not be limited!";

    }
    else
    {
        SDK::HttpProxy* proxy = SDK::__get_plugin<SDK::HttpProxy*>(SDK::ROLE_HTTP_PROXY);
        if(proxy != NULL)
        {
            proxy->stopServer();
        }
        m_interceptor->setProxy(QNetworkProxy());
    }
    webView()->load(url);
    emit load_started(url.toString());
    return true;
}

QWebPage *QtWebPage::createWindow(WebWindowType type)
{
    STUB();
    QtWebPage* page = dynamic_cast<QtWebPage*>(m_browser->createNewPage(true));
    page->setObjectName("Child window");
    page->parent->setObjectName("Child Web View");
    return page;
}



bool QtWebPage::openWindow(const QString &url, const QString &params = "", const QString &name = "")
{
    evalJs(QString("setTimeout(function(){window.open('%1', '%2', '%3');}, 1)")
                    .arg(url)
                    .arg(name)
                    .arg(params));
    return true;
}


void QtWebPage::execKeyEvent(const QString &action, int code, Qt::KeyboardModifiers mods, const QString &symbol)
{
    QEvent::Type type = QEvent::KeyPress;
    QKeyEvent* event = new QKeyEvent(type, code, mods, symbol);

    qApp->postEvent(this, event);
}


QWidget *QtWebPage::widget()
{
    return webView();
}

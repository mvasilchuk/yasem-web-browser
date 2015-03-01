#include "webpage.h"
#include "stbplugin.h"
#include "webpluginfactoryimpl.h"
#include "pluginmanager.h"
#include "profilemanager.h"
#include "webkitbrowser.h"

#include "cmd_line.h"

#include <QWebInspector>
#include <QWebFrame>
#include <QKeyEvent>
#include <QMessageBox>

using namespace yasem;

WebPage::WebPage(WebView *parent) :
    QWebPage(parent)
{
    this->parent = parent;
    this->setObjectName("WebPage");
    this->stb(NULL);

    defaultUserAgent = "Mozilla/5.0 (%Platform%%Security%%Subplatform%) AppleWebKit/%WebKitVersion% (KHTML, like Gecko) %AppVersion Safari/%WebKitVersion%";
    customUserAgent = "";

    interceptor = new InterceptorManager(this);
    interceptor->setPage(this);
    this->setNetworkAccessManager(interceptor);

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
    settings()->enablePersistentStorage();

    //settings()->setAttribute(QWebSettings::LinksIncludedInFocusChain, true);

    mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

    pluginFactory = WebPluginFactory::setInstance(new WebPluginFactoryImpl());

    connect(this->mainFrame(), &QWebFrame::javaScriptWindowObjectCleared, this, &WebPage::attachJsStbApi);

    settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);


    webInspector.setPage(this);
    webInspector.setGeometry(QRect(0, 0, 1000, 800));

    if(QCoreApplication::arguments().contains(CMD_LINE_DEVELOPER_TOOLS))
        webInspector.show();

    setForwardUnsupportedContent(true);
    connect(this, &WebPage::unsupportedContent, []( QNetworkReply * reply ){
        qDebug() << "FIXME: Unsupported content" << reply;
        delete reply;
    });
}

WebView *WebPage::webView()
{
    return parent;
}

bool WebPage::isChildWindow()
{
    return this->objectName() == "Popup web page";
}

void WebPage::attachJsStbApi()
{
    STUB();

    recreateObjects();
    if(stbPlugin)
    {
        QHash<QString, QObject*> list = stbPlugin->getStbApiList();
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

void WebPage::triggerAction(QWebPage::WebAction action, bool checked)
{
    if(action == QWebPage::Back)
    {
        ProfileManager::instance()->backToPreviousProfile();
    }
    else
        QWebPage::triggerAction(action, checked);
}

void WebPage::javaScriptAlert ( QWebFrame * frame, const QString & msg )
{
    Q_UNUSED(frame);
    LOG() << "[JS ALERT]:" << msg;
}

bool WebPage::javaScriptConfirm ( QWebFrame * frame, const QString & msg )
{
    Q_UNUSED(frame);
    LOG() << "[JS CONFIRM]:" << msg;

    const QString title = QObject::tr("Confirm action");

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this->parent, title, msg,
                                QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
    return reply == QMessageBox::Yes;
}

void WebPage::javaScriptConsoleMessage ( const QString & message, int lineNumber, const QString & sourceID )
{
    LOG() << qPrintable(QString("[JS MESSAGE] (%1: %2): %3").arg(sourceID).arg(lineNumber).arg(message));
}

bool WebPage::javaScriptPrompt ( QWebFrame * frame, const QString & msg, const QString & defaultValue, QString * result )
{
    Q_UNUSED(frame);
    LOG() << QString("WebPage::javaScriptPrompt(%1, %2, %3)").arg(msg).arg(defaultValue).arg(*result);
    return false;
}

bool WebPage::event(QEvent *event)
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
        //bool hasCtrl = (modifiers & Qt::ControlModifier) == Qt::ControlModifier;
        //bool hasAlt = (modifiers & Qt::AltModifier) == Qt::AltModifier;
        //bool hasMeta = (modifiers & Qt::MetaModifier) == Qt::MetaModifier;


        switch(keyEvent->key())
        {
            case Qt::Key_Left:      {
                if(hasShift)
                    result = receiveKeyCode(RC_KEY_REWIND);
                else
                    result = receiveKeyCode(RC_KEY_LEFT);
                break;
            }
            case Qt::Key_Right:     {
                if(hasShift)
                    result = receiveKeyCode(RC_KEY_FAST_FORWARD);
                else
                    result = receiveKeyCode(RC_KEY_RIGHT);
                break;
            }
            case Qt::Key_Up:        {
                    result = receiveKeyCode(RC_KEY_UP);
                break;
            }
            case Qt::Key_Down:      {
                result = receiveKeyCode(RC_KEY_DOWN);
                break;
            }

            case Qt::Key_Return:
            case Qt::Key_Enter:     result = receiveKeyCode(RC_KEY_OK);        break;

            case Qt::Key_Home:      result = receiveKeyCode(RC_KEY_BACK);      break;
            case Qt::Key_Escape:    result = receiveKeyCode(RC_KEY_EXIT);      break;

            case Qt::Key_F1:        result = receiveKeyCode(RC_KEY_RED);       break;
            case Qt::Key_F2:        result = receiveKeyCode(RC_KEY_GREEN);     break;
            case Qt::Key_F3:        result = receiveKeyCode(RC_KEY_YELLOW);    break;
            case Qt::Key_F4:        result = receiveKeyCode(RC_KEY_BLUE);      break;

            case Qt::Key_Tab:       result = receiveKeyCode(RC_KEY_MENU);      break;
            case Qt::Key_PageUp:    result = receiveKeyCode(RC_KEY_PAGE_UP);   break;
            case Qt::Key_PageDown:  result = receiveKeyCode(RC_KEY_PAGE_DOWN); break;
            case Qt::Key_Control:   result = receiveKeyCode(RC_KEY_INFO);      break;

            case Qt::Key_0:         result = receiveKeyCode(RC_KEY_NUMBER_0);  break;
            case Qt::Key_1:         result = receiveKeyCode(RC_KEY_NUMBER_1);  break;
            case Qt::Key_2:         result = receiveKeyCode(RC_KEY_NUMBER_2);  break;
            case Qt::Key_3:         result = receiveKeyCode(RC_KEY_NUMBER_3);  break;
            case Qt::Key_4:         result = receiveKeyCode(RC_KEY_NUMBER_4);  break;
            case Qt::Key_5:         result = receiveKeyCode(RC_KEY_NUMBER_5);  break;
            case Qt::Key_6:         result = receiveKeyCode(RC_KEY_NUMBER_6);  break;
            case Qt::Key_7:         result = receiveKeyCode(RC_KEY_NUMBER_7);  break;
            case Qt::Key_8:         result = receiveKeyCode(RC_KEY_NUMBER_8);  break;
            case Qt::Key_9:         result = receiveKeyCode(RC_KEY_NUMBER_9);  break;

            case Qt::Key_F11:
            {
                result = true;
                parent->gui->setFullscreen(!parent->gui->getFullscreen());
                break;
            }

            case Qt::Key_VolumeDown:    result = receiveKeyCode(RC_KEY_VOLUME_DOWN);  break;
            case Qt::Key_VolumeUp:      result = receiveKeyCode(RC_KEY_VOLUME_UP);    break;
            case Qt::Key_VolumeMute:    result = receiveKeyCode(RC_KEY_MUTE);        break;

            case Qt::Key_MediaTogglePlayPause:      result = receiveKeyCode(RC_KEY_PLAY_PAUSE);    break;
            case Qt::Key_MediaPlay:                 result = receiveKeyCode(RC_KEY_PLAY);          break;
            case Qt::Key_MediaPause:                result = receiveKeyCode(RC_KEY_PAUSE);         break;
            case Qt::Key_MediaStop:                 result = receiveKeyCode(RC_KEY_STOP);          break;
            case Qt::Key_MediaPrevious:             result = receiveKeyCode(RC_KEY_REWIND);        break;
            case Qt::Key_MediaNext:                 result = receiveKeyCode(RC_KEY_FAST_FORWARD);  break;

            default:
            {
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

    return result ? false: QWebPage::event(event);
}

bool WebPage::receiveKeyCode(RC_KEY keyCode)
{
    STUB() << Core::instance()->getKeycodeHashes().key(keyCode) << keyCode; //int)keyCode;

    WebkitBrowser* browser = parent->browser;
    Q_ASSERT(browser);

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



void WebPage::evalJs(const QString &js)
{
    mainFrame()->evaluateJavaScript(js);
}

bool WebPage::stb(StbPlugin *plugin)
{
    STUB();
    this->stbPlugin = plugin;
    return true;
}

StbPlugin *WebPage::stb()
{
    return this->stbPlugin;
}

void WebPage::setUserAgent(const QString &userAgent)
{
    STUB() << userAgent;
    customUserAgent = userAgent;
}

QUrl WebPage::handleUrl(QUrl url)
{
    StbPlugin* stbPlugin = stb();
    if(stbPlugin != NULL)
    {
        return stbPlugin->handleUrl(url);
    }

    return url;
}

void WebPage::recreateObjects()
{
    STUB();

    Profile* profile = ProfileManager::instance()->getActiveProfile();

    if(profile)
    {
        Plugin* obj = profile->getProfilePlugin();
        StbPlugin* stbPlugin = dynamic_cast<StbPlugin*>(obj);
        stb(stbPlugin);
        stbPlugin->init(this);
    }
    else
        qWarning() << "Profile not found!";

}

void WebPage::resetPage()
{
    STUB();
    pluginFactory->getPluginList().clear();
    Profile* profile =  ProfileManager::instance()->getActiveProfile();

    if(profile)
    {
        StbPlugin* stbPlugin = profile->getProfilePlugin();
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

void WebPage::showWebInspector()
{
    webInspector.show();
}

QString WebPage::userAgentForUrl(const QUrl & url) const
{
    Q_UNUSED(url);
    QString ua = customUserAgent != "" ? customUserAgent : defaultUserAgent;

    //qDebug() << "ua:" << ua;
    return ua;
}

void yasem::WebPage::close()
{
    this->parent->close();
}

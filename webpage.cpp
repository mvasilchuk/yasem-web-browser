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
            LOG() << "Inserting API:" << object->metaObject()->className();

            this->mainFrame()->addToJavaScriptWindowObject(name, object);
        }
    }
}

void WebPage::triggerAction(QWebPage::WebAction action, bool checked)
{
    if(action == QWebPage::Back)
    {
        ProfileManager::instance()->backToPreviousProifile();
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
    LOG() << QString("[JS MESSAGE] (%1: %2): %3").arg(sourceID).arg(lineNumber).arg(message);
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


        switch(keyEvent->key())
        {
            case Qt::Key_Left:      result = parent->browser->receiveKeyCode(RC_KEY_LEFT);      break;
            case Qt::Key_Right:     result = parent->browser->receiveKeyCode(RC_KEY_RIGHT);     break;
            case Qt::Key_Up:        result = parent->browser->receiveKeyCode(RC_KEY_UP);        break;
            case Qt::Key_Down:      result = parent->browser->receiveKeyCode(RC_KEY_DOWN);      break;

            case Qt::Key_Return:
            case Qt::Key_Enter:     result = parent->browser->receiveKeyCode(RC_KEY_OK);        break;

            case Qt::Key_Home:      result = parent->browser->receiveKeyCode(RC_KEY_BACK);      break;
            case Qt::Key_Escape:    result = parent->browser->receiveKeyCode(RC_KEY_EXIT);      break;

            case Qt::Key_F1:        result = parent->browser->receiveKeyCode(RC_KEY_RED);       break;
            case Qt::Key_F2:        result = parent->browser->receiveKeyCode(RC_KEY_GREEN);     break;
            case Qt::Key_F3:        result = parent->browser->receiveKeyCode(RC_KEY_YELLOW);    break;
            case Qt::Key_F4:        result = parent->browser->receiveKeyCode(RC_KEY_BLUE);      break;

            case Qt::Key_Tab:       result = parent->browser->receiveKeyCode(RC_KEY_MENU);      break;
            case Qt::Key_PageUp:    result = parent->browser->receiveKeyCode(RC_KEY_PAGE_UP);   break;
            case Qt::Key_PageDown:  result = parent->browser->receiveKeyCode(RC_KEY_PAGE_DOWN); break;
            case Qt::Key_Control:   result = parent->browser->receiveKeyCode(RC_KEY_INFO);      break;

            case Qt::Key_0:         result = parent->browser->receiveKeyCode(RC_KEY_NUMBER_0);  break;
            case Qt::Key_1:         result = parent->browser->receiveKeyCode(RC_KEY_NUMBER_1);  break;
            case Qt::Key_2:         result = parent->browser->receiveKeyCode(RC_KEY_NUMBER_2);  break;
            case Qt::Key_3:         result = parent->browser->receiveKeyCode(RC_KEY_NUMBER_3);  break;
            case Qt::Key_4:         result = parent->browser->receiveKeyCode(RC_KEY_NUMBER_4);  break;
            case Qt::Key_5:         result = parent->browser->receiveKeyCode(RC_KEY_NUMBER_5);  break;
            case Qt::Key_6:         result = parent->browser->receiveKeyCode(RC_KEY_NUMBER_6);  break;
            case Qt::Key_7:         result = parent->browser->receiveKeyCode(RC_KEY_NUMBER_7);  break;
            case Qt::Key_8:         result = parent->browser->receiveKeyCode(RC_KEY_NUMBER_8);  break;
            case Qt::Key_9:         result = parent->browser->receiveKeyCode(RC_KEY_NUMBER_9);  break;

            case Qt::Key_F11:
            {
                result = true;
                parent->gui->setFullscreen(!parent->gui->getFullscreen());
                break;
            }
            default:
            {
                qWarning() << "No keycode found: %1" << keyEvent->key();
            }
        }
    }
    else if(event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

        switch(mouseEvent->button())
        {
            //case Qt::LeftButton:    result = parent->browser->receiveKeyCode(RC_KEY_OK);        break;
            //case Qt::RightButton:   result = parent->browser->receiveKeyCode(RC_KEY_BACK);      break;
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
        stbPlugin->init();
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



QString WebPage::userAgentForUrl(const QUrl & url) const
{
    Q_UNUSED(url);
    QString ua = customUserAgent != "" ? customUserAgent : defaultUserAgent;

    //qDebug() << "ua:" << ua;
    return ua;
}

/*QSize WebPage::sizeHint()
{
    return parent->size();
}*/






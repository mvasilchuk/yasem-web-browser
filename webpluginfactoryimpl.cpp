#include "webpluginfactoryimpl.h"
#include "macros.h"
#include "plugin.h"
#include "stbpluginobject.h"

#include <QDebug>
#include <QWidget>


using namespace yasem;

WebPluginFactoryImpl::WebPluginFactoryImpl(QtWebPage *parent)
{
    Q_UNUSED(parent)
    this->page = parent;
}

bool WebPluginFactoryImpl::addPlugin(SDK::StbPluginObject *plugin)
{
    STUB();
    pluginList.append(plugin);
    return true;
}

QObject *WebPluginFactoryImpl::create(const QString &mimeType, const QUrl &url, const QStringList &argumentNames, const QStringList &argumentValues) const
{
    qDebug() << "WebPluginFactory::create: " << mimeType << url << argumentNames << argumentValues;

    if(mimeType == "application/x-shockwave-flash")
    {
        qDebug() << "Flash player is not available yet!";
        return new QWidget();
    }

    QString classid = "";

    for(int index= 0; index < argumentNames.length(); index++)
    {
        if(argumentNames.at(index) == "classid")
        {
            classid = argumentValues.at(index);
            break;
        }
    }

    /*if(classid == "")
    {
        qDebug() << "WebObject not found!" << mimeType << url << argumentNames << argumentValues;
        return NULL;
    }*/

    QWidget* webObject = getWebObjectByMimeType(mimeType, classid);

    qDebug() << "webObject: " << webObject ;

    if (webObject == NULL)
    {
        qDebug() << "Web object not found for mime type " << mimeType << " classid " << classid;
        return new QWidget();
    }

    return webObject;
}

QList<QWebPluginFactory::Plugin> WebPluginFactoryImpl::plugins() const
{
    return QList<QWebPluginFactory::Plugin>();
    STUB();

    QList<QWebPluginFactory::Plugin> list;

    foreach(SDK::StbPluginObject* plugin, pluginList)
    {
        QWebPluginFactory::Plugin wPlugin;
        wPlugin.name = plugin->m_web_name;
        wPlugin.description = plugin->m_description;
        wPlugin.mimeTypes = QList<MimeType>();

        foreach(SDK::WebObjectInfo info, plugin->getWebObjects())
        {
            QWebPluginFactory::MimeType mimeType;
            mimeType.name = info.mimeType;
            mimeType.description = info.description;
            mimeType.fileExtensions = info.fileExtensions;

            wPlugin.mimeTypes << mimeType;
            qDebug() << info.mimeType << info.description << info.fileExtensions << info.webObject;
        }

        list.append(wPlugin);
    }

    return list;
}

void WebPluginFactoryImpl::refreshPlugins()
{
    STUB();
}

bool WebPluginFactoryImpl::extension(QWebPluginFactory::Extension extension, const QWebPluginFactory::ExtensionOption *option, QWebPluginFactory::ExtensionReturn *output)
{
    STUB();
    return true;
}

bool WebPluginFactoryImpl::supportsExtension(QWebPluginFactory::Extension extension) const
{
    STUB();
    return true;
}

QWidget* WebPluginFactoryImpl::getWebObjectByMimeType(const QString &mimeType, const QString &classid = "") const
{
    //STATIC_STUB();
    //qDebug() << "getWebObjectByMimeType:" << mimeType;

    foreach(SDK::StbPluginObject* pluginObject, pluginList)
    {
        foreach(SDK::WebObjectInfo info, pluginObject->getWebObjects())
        {
            if(info.mimeType.compare(mimeType, Qt::CaseInsensitive) == 0 &&  info.classid == classid)
            {
                qDebug() <<  QString("Found mime type '%1' classid '%2' in '%3'")
                             .arg(mimeType)
                             .arg(classid)
                             .arg(pluginObject->plugin()->getId());
                QWidget* webObject;
                if(info.widgetFactory != NULL)
                    webObject = info.widgetFactory();
                else
                    webObject = info.webObject;
                return webObject;
            }
        }
    }

    qDebug() << "No object found:" << mimeType << classid;
    return NULL;
}



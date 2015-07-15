#ifndef WEBPLUGINFACTORYIMPL_H
#define WEBPLUGINFACTORYIMPL_H

#include "webpluginfactory.h"

#include <QObject>
#include <QList>

namespace yasem
{
class QtWebPage;

namespace SDK {
class StbPluginObject;
}

class WebPluginFactoryImpl : public WebPluginFactory
{
    Q_OBJECT
public:
    WebPluginFactoryImpl(QtWebPage *parent = 0);
public slots:
    virtual bool addPlugin(SDK::StbPluginObject *plugin);
    virtual QObject *create(const QString &mimeType,
                       const QUrl &url,
                       const QStringList &argumentNames,
                       const QStringList &argumentValues) const;
    virtual QList<QWebPluginFactory::Plugin> plugins() const;

protected:
    QtWebPage* page;

    // QWebPluginFactory interface
public:
    void refreshPlugins();
    bool extension(Extension extension, const ExtensionOption *option, ExtensionReturn *output);
    bool supportsExtension(Extension extension) const;

    virtual QWidget* getWebObjectByMimeType(const QString &mimeType, const QString &classid) const;

};

}

#endif // WEBPLUGINFACTORYIMPL_H

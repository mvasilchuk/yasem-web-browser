#ifndef WEBPLUGINFACTORY_H
#define WEBPLUGINFACTORY_H

#include <QObject>
#include <QList>
#include <QWebPluginFactory>

namespace yasem
{
class StbPluginObject;

class WebPluginFactory : public QWebPluginFactory
{
    Q_OBJECT
public:
    static WebPluginFactory* setInstance(WebPluginFactory* inst = 0)
    {
        static WebPluginFactory* instance = inst;// Guaranteed to be destroyed.
        return instance;
    }

    static WebPluginFactory* instance()
    {
       return setInstance();
    }

    QList<StbPluginObject*> getPluginList() {
        return pluginList;
    }

    virtual bool addPlugin(StbPluginObject *plugin) = 0;

protected:
    QList<StbPluginObject*> pluginList;
    WebPluginFactory() {}
private:

            // Constructor? (the {} brackets) are needed here.
    // Dont forget to declare these two. You want to make sure they
    // are unaccessable otherwise you may accidently get copies of
    // your singleton appearing.
    WebPluginFactory(WebPluginFactory const&);              // Don't Implement
    void operator=(WebPluginFactory const&); // Don't implement

    // QWebPluginFactory interface
public:
    virtual QList<Plugin> plugins() const = 0;
    virtual void refreshPlugins()  = 0;
    virtual QObject *create(const QString &mimeType, const QUrl &, const QStringList &argumentNames, const QStringList &argumentValues) const  = 0 ;
    virtual bool extension(Extension extension, const ExtensionOption *option, ExtensionReturn *output) = 0;
    virtual bool supportsExtension(Extension extension) const = 0;
    virtual QWidget* getWebObjectByMimeType(const QString &mimeType, const QString &classid) const = 0;
};

}

#endif // WEBPLUGINFACTORY_H

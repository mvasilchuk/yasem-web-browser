#ifndef WEBKITBROWSER_H
#define WEBKITBROWSER_H

#include "plugin.h"
#include "browser.h"

#include "webkitbrowser_global.h"

namespace yasem
{

class WEBKITBROWSERSHARED_EXPORT WebkitBrowser: public SDK::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.mvas.yasem.WebkitBrowserPlugin/1.0" FILE "metadata.json")
    Q_INTERFACES(yasem::SDK::Plugin)

    Q_CLASSINFO("author", "Maxim Vasilchuk")
    Q_CLASSINFO("description", "Webkit browser")
    Q_CLASSINFO("version", MODULE_VERSION)
    Q_CLASSINFO("revision", GIT_VERSION)
public:
    explicit WebkitBrowser(QObject * parent = 0);
    virtual ~WebkitBrowser();

public:
    void register_dependencies();
    void register_roles();
};

}

#endif // WEBKITBROWSER_H

#include "core.h"

#include "webkitbrowserplugin.h"
#include "webkitpluginobject.h"

using namespace yasem;

WebkitBrowser::WebkitBrowser(QObject * parent):
    Plugin(parent)
{
    DEBUG() << "registering QML type";
    setMultithreading(false);

#ifdef USE_QML_WIDGETS
    qmlRegisterType<WebViewQml>("com.mvas.yasem.WebViewQml", 1, 0, "WebViewQml");
#endif
}

WebkitBrowser::~WebkitBrowser()
{

}

void WebkitBrowser::register_dependencies()
{
    //add_dependency(ROLE_GUI);
    add_dependency(PluginDependency(ROLE_MEDIA, true, true));
}

void WebkitBrowser::register_roles()
{
    register_role(ROLE_BROWSER, new WebkitPluginObject(this));
}



#include "core.h"

#include "webkitbrowserplugin.h"
#include "webkitpluginobject.h"

using namespace yasem;

WebkitBrowser::WebkitBrowser(QObject * parent):
    SDK::Plugin(parent)
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
    add_dependency({SDK::ROLE_MEDIA, true, true});
}

void WebkitBrowser::register_roles()
{
    register_role(SDK::ROLE_BROWSER, new WebkitPluginObject(this));
}



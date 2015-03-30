#include "core.h"

#include "webkitbrowserplugin.h"
#include "webkitpluginobject.h"

using namespace yasem;

WebkitBrowser::WebkitBrowser(QObject * parent):
    Plugin(parent)
{
    DEBUG() << "registering QML type";

#ifdef USE_QML_WIDGETS
    qmlRegisterType<WebViewQml>("com.mvas.yasem.WebViewQml", 1, 0, "WebViewQml");
#endif
}

WebkitBrowser::~WebkitBrowser()
{

}

void WebkitBrowser::register_dependencies()
{
    add_dependency(ROLE_GUI);
    add_dependency(ROLE_MEDIA);
}

void WebkitBrowser::register_roles()
{
    register_role(ROLE_BROWSER, new WebkitPluginObject(this));
}



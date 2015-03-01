#include "webviewqml.h"
#include "webview.h"
#include "pluginmanager.h"
#include "webkitbrowser.h"

using namespace yasem;

WebViewQml::WebViewQml(QQuickItem * parent): QQuickItem(parent)
{
    //WebView* vView = new WebView(NULL, dynamic_cast<WebkitBrowser*>(PluginManager::instance()->getByRole(ROLE_BROWSER)));
    //vView = new QWebView();

}

WebViewQml::~WebViewQml()
{

}




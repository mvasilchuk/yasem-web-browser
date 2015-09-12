#ifndef WEBPAGE_H
#define WEBPAGE_H

#include "enums.h"
#include "webpage.h"
#include "gui.h"

#include <QWidget>
#include <QWebPage>
#include <QWebInspector>

namespace yasem
{
class WebView;
class WebPluginFactory;
class InterceptorManager;

namespace SDK {
class StbPluginObject;
class Browser;
}

class QtWebPage : public QWebPage, public SDK::WebPage
{
    Q_OBJECT
public:
    explicit QtWebPage(WebView *m_parent = 0);
    virtual ~QtWebPage();

    void  javaScriptAlert ( QWebFrame * frame, const QString & msg );
    bool  javaScriptConfirm ( QWebFrame * frame, const QString & msg );
    void  javaScriptConsoleMessage ( const QString & message, int lineNumber, const QString & sourceID );
    bool  javaScriptPrompt ( QWebFrame * frame, const QString & msg, const QString & defaultValue, QString * result );
    void setUserAgent(const QString &userAgent);

    WebView* webView() const;

public slots:
    bool load(const QUrl &url);
    void close();
    virtual bool event(QEvent*);

    bool stb(SDK::StbPluginObject* plugin);
    SDK::StbPluginObject* stb();
    QUrl handleUrl(QUrl url);
    void recreateObjects();
    void resetPage();
    void showWebInspector();

    bool receiveKeyCode(SDK::GUI::RcKey keyCode);
    void evalJs(const QString &js);

    QColor getChromaKey()  const;
    void setChromaKey(QColor color);

    QColor getChromaMask()  const;
    void setChromaMask(QColor color);

    float getOpacity()  const;
    void setOpacity(float opacity) ;

    bool isChromaKeyEnabled()  const;
    void setChromaKeyEnabled(bool enabled);

    void reset();
    bool openWindow(const QString &url, const QString &params, const QString &name);


protected slots:
    void attachJsStbApi();
    void setupInterceptor();

protected:
    WebView* m_parent;
    SDK::StbPluginObject* m_stb_plugin;
    QString defaultUserAgent;
    QString customUserAgent;
    WebPluginFactory* pluginFactory;
    QWebInspector m_web_inspector;

    QColor m_chromakey;
    QColor m_chromamask;
    float m_opacity; // 0.0 - 1.0
    bool m_chromakey_enabled;

    InterceptorManager* m_interceptor;

public:
     bool isChildWindow();
    QString userAgentForUrl(const QUrl &url) const;
    void triggerAction(WebAction action, bool checked);

    void setPageViewportSize(QSize new_size);
    QSize getVieportSize();

    qreal scale();
    QRect getPageRect();


    // QWebPage interface
protected:
    virtual QWebPage *createWindow(WebWindowType type);

public slots:
    virtual void execKeyEvent(const QString &action, int code, Qt::KeyboardModifiers mods, const QString &symbol);
    virtual QWidget *widget() const;

signals:
    void closed();
    void load_started(const QString& url);

    // WebPage interface
public slots:
    QString getTitle() const;
    QUrl getURL() const;
    QString getRootDir() const;

    // WebPage interface
public slots:
    void move(int x, int y);
    void resize(int width, int height);
    void show();
    void hide();
    void raise();
    void setStyleSheet(const QString &stylesheet);



};

}

#endif // WEBPAGE_H

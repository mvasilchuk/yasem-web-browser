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

    void  javaScriptAlert ( QWebFrame * frame, const QString & msg ) Q_DECL_OVERRIDE;
    bool  javaScriptConfirm ( QWebFrame * frame, const QString & msg ) Q_DECL_OVERRIDE;
    void  javaScriptConsoleMessage ( const QString & message, int lineNumber, const QString & sourceID ) Q_DECL_OVERRIDE;
    bool  javaScriptPrompt ( QWebFrame * frame, const QString & msg, const QString & defaultValue, QString * result ) Q_DECL_OVERRIDE;
    void setUserAgent(const QString &userAgent);

    WebView* webView() const;

public slots:
    bool load(const QUrl &url) Q_DECL_OVERRIDE;
    void close() Q_DECL_OVERRIDE;
    virtual bool event(QEvent*) Q_DECL_OVERRIDE;

    bool stb(SDK::StbPluginObject* plugin);
    SDK::StbPluginObject* stb();
    QUrl handleUrl(QUrl url);
    void recreateObjects();
    void resetPage();
    void showWebInspector();

    bool receiveKeyCode(SDK::GUI::RcKey keyCode);
    void evalJs(const QString &js) Q_DECL_OVERRIDE;

    QColor getChromaKey() const Q_DECL_OVERRIDE;
    void setChromaKey(QColor color) Q_DECL_OVERRIDE;

    QColor getChromaMask() const Q_DECL_OVERRIDE;
    void setChromaMask(QColor color) Q_DECL_OVERRIDE;

    float getOpacity() const Q_DECL_OVERRIDE;
    void setOpacity(float opacity) Q_DECL_OVERRIDE;

    bool isChromaKeyEnabled() const Q_DECL_OVERRIDE;
    void setChromaKeyEnabled(bool enabled) Q_DECL_OVERRIDE;

    void reset() Q_DECL_OVERRIDE;
    int openWindow(const QString &url, const QString &params, const QString &name) Q_DECL_OVERRIDE;

    void setupInterceptor();


protected slots:
    void attachJsStbApi();

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
    QString userAgentForUrl(const QUrl &url) const Q_DECL_OVERRIDE;
    void triggerAction(WebAction action, bool checked) Q_DECL_OVERRIDE;

    void setPageViewportSize(QSize new_size) Q_DECL_OVERRIDE;
    QSize getVieportSize() Q_DECL_OVERRIDE;

    qreal scale() Q_DECL_OVERRIDE;
    QRect getPageRect() Q_DECL_OVERRIDE;


    // QWebPage interface
protected:
    virtual QWebPage *createWindow(WebWindowType type) Q_DECL_OVERRIDE;

public slots:
    virtual void execKeyEvent(const QString &action, int code, Qt::KeyboardModifiers mods, const QString &symbol) Q_DECL_OVERRIDE;
    virtual QWidget *widget() const Q_DECL_OVERRIDE;

signals:
    void showed();
    void hidden();
    void raised();
    void closed();
    void load_started(const QString& url);

    // WebPage interface
public slots:
    QString getTitle() const Q_DECL_OVERRIDE;
    QUrl getURL() const Q_DECL_OVERRIDE;
    QString getRootDir() const Q_DECL_OVERRIDE;

    // WebPage interface
public slots:
    void move(int x, int y) Q_DECL_OVERRIDE;
    void resize(int width, int height) Q_DECL_OVERRIDE;
    void show() Q_DECL_OVERRIDE;
    void hide() Q_DECL_OVERRIDE;
    void raise() Q_DECL_OVERRIDE;
    void setStyleSheet(const QString &stylesheet) Q_DECL_OVERRIDE;

};

}

#endif // WEBPAGE_H

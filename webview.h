#ifndef WEBVIEW_H
#define WEBVIEW_H

#include "guiplugin.h"

#include <QObject>
#include <QWebView>
#include <QLinearGradient>
#include <QMenu>
#include <QAction>
#include <QQuickItem>

namespace yasem
{
class WebkitBrowser;
class WebView : public QWebView
{
    Q_OBJECT
public:
    explicit WebView(QWidget *parent = 0, WebkitBrowser* browser = 0);
    QWebView* createWindow(QWebPage::WebWindowType type);
    GuiPlugin* gui;
    WebkitBrowser* browser;

public slots:

    Q_INVOKABLE void setBrowser(WebkitBrowser* browser);
    Q_INVOKABLE void setDefaultBrowser();
    Q_INVOKABLE void qmlInit();

    void onLoadStarted();
    void onLoadProgress(int progress);
    void onLoadFinished(bool);
    void onTitleChanged(const QString& title);
    void onStatusBarMessage(const QString& text);
    void onLinkClicked(const QUrl&);
    void onSelectionChanged();
    void onIconChanged();
    void onUrlChanged(const QUrl&);

    void mouseMoveEvent(QMouseEvent *e);
    void readSettings();

    void showContextMenu(const QPoint &pos);

protected:

    void setupContextMenu();
    virtual void keyPressEvent(QKeyEvent*);
    virtual void keyReleaseEvent(QKeyEvent*);

    void loadFixes();
    QString loadFix(const QString &name);

    bool triggered;

    // Some fixed
    QString webObjectsFix;
    QString xmlHttpRequestFix;

    bool rendering_started;
    bool m_is_context_menu_valid;

    int mouseBorderThreshold;

    QMenu* m_contextMenu;
    QAction* m_backToPreviousPageAction;
    QAction* m_openWebInspectorAction;

signals:
    void mousePositionChanged(int position);
};

}

#endif // WEBVIEW_H

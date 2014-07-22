#ifndef WEBVIEW_H
#define WEBVIEW_H

#include "guiplugin.h"

#include <QObject>
#include <QWebView>
#include <QLinearGradient>

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

    void onLoadStarted();
    void onLoadProgress(int progress);
    void onLoadFinished(bool);
    void onTitleChanged(const QString& title);
    void onStatusBarMessage(const QString& text);
    void onLinkClicked(const QUrl&);
    void onSelectionChanged();
    void onIconChanged();
    void onUrlChanged(const QUrl&);


    void paintEvent(QPaintEvent *e);

    void mouseMoveEvent(QMouseEvent *e);
    void readSettings();
protected:
    virtual void keyPressEvent(QKeyEvent*);
    virtual void keyReleaseEvent(QKeyEvent*);
    virtual bool event(QEvent *event);

    void loadFixes();
    QString loadFix(const QString &name);

    bool triggered;

    // Some fixed
    QString webObjectsFix;
    QString xmlHttpRequestFix;

    bool rendering_started;

    int mouseBorderThreshold;

signals:
    void mousePositionChanged(int position);
};

}

#endif // WEBVIEW_H

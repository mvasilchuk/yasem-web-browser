#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QObject>
#include <QWebView>
#include <QLinearGradient>
#include <QMenu>
#include <QAction>
#include <QPixmap>

class QPainter;

namespace yasem
{
class WebkitPluginObject;

namespace SDK {
class GUI;
class MediaPlayer;
class Browser;
}

class WebView : public QWebView
{
    Q_OBJECT
public:
    explicit WebView(QWidget *parent = 0);
    virtual QWebView* createWindow(QWebPage::WebWindowType type);

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

    void mouseMoveEvent(QMouseEvent *e);
    void readSettings();

    void showContextMenu(const QPoint &pos);
    void resizeView(const QRect& containerRect);
    void setViewportSize(QSize newSize);
    QSize getViewportSize();
    qreal getScale();
    QRect getRect();

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

    SDK::Browser* m_browser;

    QMenu* m_contextMenu;
    QAction* m_backToPreviousPageAction;
    QAction* m_openWebInspectorAction;

    QRect m_viewRect;
    QSize m_viewportSize;
    qreal m_pageScale;

    QPixmap m_video_frame;

signals:
    void mousePositionChanged(int position);

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *);

    bool m_allow_repaint;
    bool m_allow_transparency;
    SDK::MediaPlayer* m_player;
    bool m_skip_full_render;

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *);

public slots:
    void fullUpdate();
    void updateTopWidget();
};

}

#endif // WEBVIEW_H

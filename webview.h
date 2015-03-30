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
class GuiPluginObject;
class MediaPlayerPluginObject;

class WebView : public QWebView
{
    Q_OBJECT
public:
    explicit WebView(QWidget *parent = 0, WebkitPluginObject* browser_object = 0);
    QWebView* createWindow(QWebPage::WebWindowType type);
    GuiPluginObject* gui;
    WebkitPluginObject* m_browser_object;

public slots:

    Q_INVOKABLE void setBrowser(WebkitPluginObject* browser_object);
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

    QMenu* m_contextMenu;
    QAction* m_backToPreviousPageAction;
    QAction* m_openWebInspectorAction;

    QRect m_viewRect;
    QSize m_viewportSize;
    qreal m_pageScale;

    QPixmap m_render_pixmap;
    QPixmap m_video_frame;

protected slots:
    void onPlayerRendered();

signals:
    void mousePositionChanged(int position);

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *);

    bool m_allow_repaint;
    bool m_allow_transparency;
    MediaPlayerPluginObject* m_player;
    bool m_skip_full_render;



    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *);

public slots:
    void fullUpdate();
};

}

#endif // WEBVIEW_H

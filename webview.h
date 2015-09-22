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
class QtWebPage;

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
    virtual ~WebView();
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

    void updatePosition(int x, int y);
    void resize(int width, int height);

    void setId(const QString& id);
    QString getId() const;
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

    QRect m_view_rect;
    QSize m_viewport_size;
    QRect m_window_rect;
    qreal m_pageScale;

    QPixmap m_video_frame;

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *);

    bool m_allow_repaint;
    bool m_allow_transparency;
    bool m_skip_full_render;
    QString m_id;

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *);

public slots:
    void fullUpdate();
};

}

#endif // WEBVIEW_H

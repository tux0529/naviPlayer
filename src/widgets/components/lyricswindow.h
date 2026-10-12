/**
 * @file    lyricswindow.h
 * @brief   独立歌词窗口：置顶、透明、两行（当前句 + 下一句）
 *
 * 交互：鼠标划过窗口时淡出隐藏（不拦截下方窗口点击），鼠标离开后自动显示；
 *      可拖拽调整位置。
 */
#ifndef NAVIPLAYER_LYRICSWINDOW_H
#define NAVIPLAYER_LYRICSWINDOW_H

#include <QWidget>
#include <QPoint>

#include "core/qt_compat.h"
#include "core/lyricsmanager.h"

class QLabel;
class QTimer;
class QPropertyAnimation;

class LyricsWindow : public QWidget
{
    Q_OBJECT
public:
    explicit LyricsWindow(QWidget *parent = nullptr);

    void setLyricsManager(LyricsManager *manager);
    void toggleVisible();
    void applyStyle(const LyricsStyle &style);

    /** 进入/退出"拖动调整位置"模式：期间不自动淡出，拖完自动结束 */
    void setPositionAdjustEnabled(bool enabled);
    bool isPositionAdjustEnabled() const { return m_adjusting; }
    /** 彻底关闭（主窗口退出前调用，避免独立顶层窗口挡住应用退出） */
    void closeLyrics();

signals:
    /** 拖动调整位置完成（通知设置页把按钮恢复成未按下） */
    void positionAdjustFinished();

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(NaviEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onLinesChanged(const QString &current, const QString &next);
    void onStyleChanged(const LyricsStyle &style);
    void onHoverTimer();
    void onPollCursor();
    /** 窗口刷新过一帧后，再把歌词文字显示出来（避免合成层残影） */
    void restorePendingText();

private:
    void fadeTo(qreal opacity);
    /** 清空白内容并刷新一帧，之后再由 restorePendingText() 填回文字 */
    void prepareContentForShow();
    /** 当前是否处于“完全透明”（等同于隐藏）状态 */
    bool isFadedOut() const;

    QLabel *m_currentLabel = nullptr;
    QLabel *m_nextLabel = nullptr;
    QTimer *m_hoverTimer = nullptr;
    QTimer *m_pollTimer = nullptr;
    QPropertyAnimation *m_fade = nullptr;
    LyricsManager *m_lyrics = nullptr;

    bool m_visibleState = false;
    bool m_cursorInside = false;
    bool m_dragging = false;
    bool m_dragMoved = false;
    bool m_adjusting = false;               // 拖动调整位置模式
    bool m_visibleBeforeAdjust = false;     // 进入调整模式前窗口是否可见
    QPoint m_dragOffset;
    qreal m_fadeTarget = -1.0;      // 当前淡入淡出的目标值，避免重复启动动画

    // 显示流程：先清空 → 刷新一帧 → 再填文字（每次隐藏后再显示都要走）
    QString m_pendingCurrent;
    QString m_pendingNext;
    bool m_restoreAfterPaint = false;
};

#endif // NAVIPLAYER_LYRICSWINDOW_H

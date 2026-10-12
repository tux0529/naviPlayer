/**
 * @file    playbackqueuepanel.h
 * @brief   播放队列面板：QTableView + 播放模式切换 + 删除/清空
 */
#ifndef NAVIPLAYER_PLAYBACKQUEUEPANEL_H
#define NAVIPLAYER_PLAYBACKQUEUEPANEL_H

#include <QWidget>
#include <QStyledItemDelegate>

#include "core/playbackqueue.h"

class QTableView;
class QLabel;
class QPushButton;

/** 队列封面列绘制器 */
class CoverDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CoverDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    /** 队列行用图：优先专辑封面（同一专辑只取一次），缺失时回退单曲封面 */
    static QString coverIdForIndex(const QModelIndex &index);
    /** 绘制尺寸（逻辑像素） */
    static int coverSize() { return 40; }
};

class PlaybackQueuePanel : public QWidget
{
    Q_OBJECT
public:
    explicit PlaybackQueuePanel(QWidget *parent = nullptr);
    void setQueue(PlaybackQueue *queue);

signals:
    void playRequested(int index);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void updateModeButton();
    void removeSelected();

private:
    /** 按专辑去重后请求封面（同一专辑只下一张图） */
    void requestCovers();

private:
    PlaybackQueue *m_queue = nullptr;
    QTableView *m_view = nullptr;
    QPushButton *m_modeButton = nullptr;
    QPushButton *m_clearButton = nullptr;
    QPushButton *m_removeButton = nullptr;
    QLabel *m_countLabel = nullptr;
};

#endif // NAVIPLAYER_PLAYBACKQUEUEPANEL_H

#ifndef PLAYQUEUEWIDGET_H
#define PLAYQUEUEWIDGET_H

#include "track.h"
#include <QWidget>

namespace Ui {
class PlayQueueWidget;
}

class AnchorItemDelegate;
class PlaybackQueue;

class PlayQueueWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlayQueueWidget(PlaybackQueue *playQueue, QWidget *parent = nullptr);
    ~PlayQueueWidget();

    void setPlaylist(PlaybackQueue *playQueue);

signals:
    void linkActivated(const QString &);
protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private slots:
    void onCurrentIndexChanged(int index);
    void onTrackCountChanged(int count);
    void onRowDoubleClicked(const QModelIndex &index);
    void onClearButtonClicked();
    void onPlaybackModeButtonClicked();
    void onMoveUpButtonClicked();
    void onMoveDownButtonClicked();
    void onRemoveButtonClicked();

private:
    Ui::PlayQueueWidget *ui;

    PlaybackQueue *m_playbackQueue;

    AnchorItemDelegate *m_anchorItemDelegate;

    void initForm();
    void resizeTableView();
};

#endif // PLAYQUEUEWIDGET_H

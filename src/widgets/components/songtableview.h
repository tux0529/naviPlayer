/**
 * @file    songtableview.h
 * @brief   歌曲表格视图：双击播放 + 右键菜单（立即播放/下一首/收藏/加入播放列表/下载/详情）
 */
#ifndef NAVIPLAYER_SONGTABLEVIEW_H
#define NAVIPLAYER_SONGTABLEVIEW_H

#include <QTableView>
#include <QPoint>

#include "models/song.h"

class SongTableModel;

class SongTableView : public QTableView
{
    Q_OBJECT
public:
    explicit SongTableView(QWidget *parent = nullptr);

    void setSongModel(SongTableModel *model);
    SongTableModel *songModel() const { return m_model; }

    /** 当前选中/右键定位的歌曲 */
    Song selectedSong() const;
    void setContextMenuEnabled(bool enabled);

signals:
    void songActivated(const Song &song);                       // 双击 / 立即播放
    void playNextRequested(const Song &song);
    void favoriteToggled(const Song &song, bool favorite);
    void addToPlaylistRequested(const Song &song, const QString &playlistId);
    void downloadRequested(const Song &song);
    void detailsRequested(const Song &song);

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private slots:
    void showContextMenu(const QPoint &pos);

private:
    SongTableModel *m_model = nullptr;
    bool m_menuEnabled = true;
};

#endif // NAVIPLAYER_SONGTABLEVIEW_H

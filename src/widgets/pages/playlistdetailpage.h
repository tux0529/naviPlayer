/**
 * @file    playlistdetailpage.h
 * @brief   播放列表详情：歌曲列表支持拖动排序并保存回服务端
 */
#ifndef NAVIPLAYER_PLAYLISTDETAILPAGE_H
#define NAVIPLAYER_PLAYLISTDETAILPAGE_H

#include "widgets/pagebase.h"

#include <QVector>

#include "models/playlist.h"
#include "models/song.h"

class QLabel;
class QPushButton;
class SongsListView;

class PlaylistDetailPage : public PageBase
{
    Q_OBJECT
public:
    explicit PlaylistDetailPage(QWidget *parent = nullptr);

    void setPlaylist(const Playlist &playlist);
    Playlist playlist() const { return m_playlist; }

    void refresh() override;
    void retranslate() override;

signals:
    void playAllRequested(const QVector<Song> &songs);
    void addToQueueRequested(const QVector<Song> &songs);
    void songActivated(const Song &song);
    void playNextRequested(const Song &song);
    void favoriteToggled(const Song &song, bool favorite);
    void addToPlaylistRequested(const Song &song, const QString &playlistId);
    void downloadRequested(const Song &song);
    void detailsRequested(const Song &song);

private slots:
    void onPlaylistReady(const Playlist &playlist);
    void onOrderChanged(const QVector<Song> &songs);
    void onSaveOrder();

private:
    QLabel *m_titleLabel = nullptr;
    QLabel *m_metaLabel = nullptr;
    QPushButton *m_playAllButton = nullptr;
    QPushButton *m_addQueueButton = nullptr;
    QPushButton *m_saveOrderButton = nullptr;
    SongsListView *m_songsView = nullptr;

    Playlist m_playlist;
};

#endif // NAVIPLAYER_PLAYLISTDETAILPAGE_H

/**
 * @file    songspage.h
 * @brief   全部歌曲 / 收藏歌曲：本地 SQLite 优先，后台刷新
 */
#ifndef NAVIPLAYER_SONGSLISTPAGE_H
#define NAVIPLAYER_SONGSLISTPAGE_H

#include "widgets/pagebase.h"

#include <QVector>
#include <QStringList>
#include <QDateTime>

#include "models/song.h"
#include "models/album.h"

class SongsListView;

class SongsPage : public PageBase
{
    Q_OBJECT
public:
    enum class Source { All, Favorite };
    Q_ENUM(Source)

    explicit SongsPage(QWidget *parent = nullptr);
    void setSource(Source source);
    Source source() const { return m_source; }

    /** 更新某首歌的收藏状态显示 */
    void setFavorite(const QString &songId, bool favorite);
    /** 高亮正在播放的歌曲 */
    void setPlayingSongId(const QString &songId);

    void refresh() override;
    void retranslate() override;

signals:
    void songActivated(const Song &song);
    void playNextRequested(const Song &song);
    void favoriteToggled(const Song &song, bool favorite);
    void addToPlaylistRequested(const Song &song, const QString &playlistId);
    void downloadRequested(const Song &song);
    void detailsRequested(const Song &song);

private slots:
    void onAlbumReady(const Album &album, const QVector<Song> &songs);

private:
    void loadLocal();
    void requestRemote();
    void startAlbumScanFallback();
    void requestNextAlbum();

    SongsListView *m_view = nullptr;
    Source m_source = Source::All;
    QStringList m_pendingAlbumIds;
    bool m_scanning = false;
    QDateTime m_lastRemoteFetch;
};

#endif // NAVIPLAYER_SONGSLISTPAGE_H

/**
 * @file    albumdetailpage.h
 * @brief   专辑详情页：封面信息 + 操作按钮 + 歌曲列表（多 CD 先按碟号再按曲目号排序）
 */
#ifndef NAVIPLAYER_ALBUMDETAILPAGE_H
#define NAVIPLAYER_ALBUMDETAILPAGE_H

#include "widgets/pagebase.h"

#include <QVector>

#include "models/album.h"
#include "models/artist.h"
#include "models/song.h"

class CoverLabel;
class QLabel;
class QPushButton;
class SongsListView;

class AlbumDetailPage : public PageBase
{
    Q_OBJECT
public:
    explicit AlbumDetailPage(QWidget *parent = nullptr);

    void setAlbum(const Album &album);
    Album album() const { return m_album; }
    QVector<Song> songs() const { return m_songs; }

    void refresh() override;
    void retranslate() override;

signals:
    void playAllRequested(const QVector<Song> &songs);
    void addToQueueRequested(const QVector<Song> &songs);
    void addToNextRequested(const QVector<Song> &songs);
    void favoriteToggled(const Album &album, bool favorite);
    void artistClicked(const Artist &artist);

    void songActivated(const Song &song);
    void playNextRequested(const Song &song);
    void songFavoriteToggled(const Song &song, bool favorite);
    void addToPlaylistRequested(const Song &song, const QString &playlistId);
    void downloadRequested(const Song &song);
    void detailsRequested(const Song &song);

private slots:
    void onAlbumReady(const Album &album, const QVector<Song> &songs);

private:
    void updateHeader();
    static void sortSongs(QVector<Song> &songs);

    CoverLabel *m_cover = nullptr;
    QLabel *m_titleLabel = nullptr;
    QPushButton *m_artistButton = nullptr;
    QLabel *m_metaLabel = nullptr;
    QPushButton *m_addQueueButton = nullptr;
    QPushButton *m_addNextButton = nullptr;
    QPushButton *m_favoriteButton = nullptr;
    SongsListView *m_songsView = nullptr;

    Album m_album;
    QVector<Song> m_songs;
};

#endif // NAVIPLAYER_ALBUMDETAILPAGE_H

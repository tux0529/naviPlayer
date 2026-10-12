/**
 * @file    searchpage.h
 * @brief   搜索结果页：歌曲 / 专辑 / 艺术家 三个标签页
 */
#ifndef NAVIPLAYER_SEARCHPAGE_H
#define NAVIPLAYER_SEARCHPAGE_H

#include "widgets/pagebase.h"

#include <QVector>

#include "models/song.h"
#include "models/album.h"
#include "models/artist.h"

class QTabWidget;
class SongsListView;
class AlbumsView;
class ArtistsView;

class SearchPage : public PageBase
{
    Q_OBJECT
public:
    explicit SearchPage(QWidget *parent = nullptr);

    void search(const QString &query);
    QString query() const { return m_query; }

    void refresh() override;
    void retranslate() override;

signals:
    void songActivated(const Song &song);
    void playNextRequested(const Song &song);
    void favoriteToggled(const Song &song, bool favorite);
    void addToPlaylistRequested(const Song &song, const QString &playlistId);
    void downloadRequested(const Song &song);
    void detailsRequested(const Song &song);
    void albumClicked(const Album &album);
    void artistClicked(const Artist &artist);
    void playAlbumRequested(const Album &album);

private:
    QTabWidget *m_tabs = nullptr;
    SongsListView *m_songsView = nullptr;
    AlbumsView *m_albumsView = nullptr;
    ArtistsView *m_artistsView = nullptr;
    QString m_query;
};

#endif // NAVIPLAYER_SEARCHPAGE_H

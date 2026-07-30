#ifndef FAVORITESWIDGET_H
#define FAVORITESWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include "../models/song.h"
#include "../models/album.h"
#include "../models/artist.h"
#include "../subsonicapi.h"
#include "../cachemanager.h"
#include "../databasemanager.h"

class SongListWidget;
class AlbumWidget;
class ArtistWidget;

class FavoritesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FavoritesWidget(QWidget *parent = nullptr);

    void setSubsonicApi(SubsonicApi *api);
    void setCacheManager(CacheManager *cache);
    void setDatabaseManager(DatabaseManager *db);

    void refresh();

signals:
    void songSelected(const Song &song);
    void songFavoriteClicked(const Song &song);
    void albumSelected(const Album &album);
    void artistSelected(const Artist &artist);

public slots:
    void onStarredReceived(const QVector<Song> &songs,
                            const QVector<Album> &albums,
                            const QVector<Artist> &artists);

private:
    void setupUI();

    QTabWidget *m_favoritesTab;
    SongListWidget *m_songFavorites;
    AlbumWidget *m_albumFavorites;
    ArtistWidget *m_artistFavorites;

    SubsonicApi *m_subsonicApi;
    CacheManager *m_cacheManager;
    DatabaseManager *m_dbManager;
};

#endif // FAVORITESWIDGET_H

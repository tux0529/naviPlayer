#ifndef MEDIAHELPER_H
#define MEDIAHELPER_H

#include "album.h"
#include "albumobject.h"
#include "track.h"
#include "genre.h"
#include "artist.h"
#include "artistobject.h"
#include "playlist.h"
#include "playlistobject.h"

#include "config.h"

#include <QObject>
#include <QMutex>

class QNetworkAccessManager;

class MediaHelper : public QObject
{
    Q_OBJECT

public:

    enum AlbumType { Newest=0, Frequent, Recent, Random, Highest, Starred };

    enum CoverType { AlbumCover=0, ArtistCover, TrackCover, PlayListCover };

    enum CoverSize { Cover=0, Poster, Icon };

public:

    static MediaHelper *Instance()
    {
        static QMutex mutex;

        if (!s_self) {
            QMutexLocker locker(&mutex);

            if (!s_self) {
                s_self = new MediaHelper;
            }
        }

        return s_self;
    }

    static const QIcon &getAlbumIcon(const QString &albumId);

    AlbumList getAlbumList(AlbumType type, int size = 20, int offset = 0);
    AlbumObject getAlbum(const QString &albumId);

    void getArtists();
    ArtistObject getArtist(const QString &id);

    void getGenres();
    void getFavorites();

    QList<PlayList> getPlaylists();
    PlayListObject getPlaylist(const QString &listId);

    QString getCoverArt(const QString &id, MediaHelper::CoverType type=AlbumCover, MediaHelper::CoverSize size = Cover);

    QString getSongUrlbyId(const QString &id);

    /* https://www.redtux.cn:619/rest/scrobble?
     * u=redtux&
     * t=23d9356afe2a8e45f0220c06f50e086e&
     * s=f1f8be&
     * f=json&
     * v=1.8.0&
     * c=NavidromeUI&
     * id=e26a97b5d31c3412a28a89e7e6b665eb&
     * submission=false
     * time=1734662183068
     * submission=true
     * */
    void scrobble(const QString &id, bool submission = true);

    bool ping(Config::Server &srv);

    static GenreList s_genreList;
    static ArtistList s_artistList;
    static ArtistList s_favorArtists;
    static AlbumList s_favorAlbums;
    static TrackList s_favorTracks;
    static int s_albumCount;
    static int s_albumPageSize;

private:
    explicit MediaHelper(QObject *parent = nullptr);
    QString AlbumTypeToString(AlbumType type) const;
    QString questStringBuilder(const QString &command);
    bool getRoundRectPixmap(const QString &filePath);
    QString durationToString(int dur);

    static MediaHelper *s_self;    //对象自身
    QNetworkAccessManager *m_manager;
    QString m_host;
    QString m_authString;

    QMap<QString, QIcon> m_albumIconMap;

};

#endif // MEDIAHELPER_H


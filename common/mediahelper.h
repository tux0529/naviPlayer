#ifndef MEDIAHELPER_H
#define MEDIAHELPER_H

#include "album.h"
#include "albumobject.h"
#include "track.h"
#include "genre.h"
#include "artist.h"
#include "playlist.h"

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

    AlbumList getAlbumList(AlbumType type, int size = 20, int offset = 0);
    QString getCoverArt(const QString &id, CoverType type=AlbumCover, int size = 0);
    void getArtists();
    void getGenres();
    void getFavorites();
    QList<PlayList> getPlaylists();
    TrackList getPlaylist(const QString &listId);

    AlbumObject getAlbum(const QString &albumId);

    QString getSongUrlbyId(const QString &id);

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

};

#endif // MEDIAHELPER_H


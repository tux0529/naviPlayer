#ifndef SONG_H
#define SONG_H

#include <QString>
#include <QMetaType>

class Song
{
public:
    Song();

    QString id() const { return m_id; }
    void setId(const QString &id) { m_id = id; }

    QString title() const { return m_title; }
    void setTitle(const QString &title) { m_title = title; }

    QString artist() const { return m_artist; }
    void setArtist(const QString &artist) { m_artist = artist; }

    QString album() const { return m_album; }
    void setAlbum(const QString &album) { m_album = album; }

    QString albumId() const { return m_albumId; }
    void setAlbumId(const QString &albumId) { m_albumId = albumId; }

    QString artistId() const { return m_artistId; }
    void setArtistId(const QString &artistId) { m_artistId = artistId; }

    int track() const { return m_track; }
    void setTrack(int track) { m_track = track; }

    int year() const { return m_year; }
    void setYear(int year) { m_year = year; }

    QString genre() const { return m_genre; }
    void setGenre(const QString &genre) { m_genre = genre; }

    qint64 duration() const { return m_duration; }
    void setDuration(qint64 duration) { m_duration = duration; }

    qint64 size() const { return m_size; }
    void setSize(qint64 size) { m_size = size; }

    QString suffix() const { return m_suffix; }
    void setSuffix(const QString &suffix) { m_suffix = suffix; }

    QString contentType() const { return m_contentType; }
    void setContentType(const QString &contentType) { m_contentType = contentType; }

    QString coverArtId() const { return m_coverArtId; }
    void setCoverArtId(const QString &coverArtId) { m_coverArtId = coverArtId; }

    QString path() const { return m_path; }
    void setPath(const QString &path) { m_path = path; }

    bool isFavorite() const { return m_isFavorite; }
    void setFavorite(bool favorite) { m_isFavorite = favorite; }

private:
    QString m_id;
    QString m_title;
    QString m_artist;
    QString m_album;
    QString m_albumId;
    QString m_artistId;
    int m_track;
    int m_year;
    QString m_genre;
    qint64 m_duration;
    qint64 m_size;
    QString m_suffix;
    QString m_contentType;
    QString m_coverArtId;
    QString m_path;
    bool m_isFavorite;
};

Q_DECLARE_METATYPE(Song)

#endif // SONG_H

#ifndef ALBUM_H
#define ALBUM_H

#include <QString>

class Album
{
public:
    Album();

    QString id() const { return m_id; }
    void setId(const QString &id) { m_id = id; }

    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    QString artist() const { return m_artist; }
    void setArtist(const QString &artist) { m_artist = artist; }

    QString artistId() const { return m_artistId; }
    void setArtistId(const QString &artistId) { m_artistId = artistId; }

    QString coverArtId() const { return m_coverArtId; }
    void setCoverArtId(const QString &coverArtId) { m_coverArtId = coverArtId; }

    int songCount() const { return m_songCount; }
    void setSongCount(int songCount) { m_songCount = songCount; }

    int duration() const { return m_duration; }
    void setDuration(int duration) { m_duration = duration; }

    int year() const { return m_year; }
    void setYear(int year) { m_year = year; }

    QString genre() const { return m_genre; }
    void setGenre(const QString &genre) { m_genre = genre; }

    bool isFavorite() const { return m_isFavorite; }
    void setFavorite(bool favorite) { m_isFavorite = favorite; }

private:
    QString m_id;
    QString m_name;
    QString m_artist;
    QString m_artistId;
    QString m_coverArtId;
    int m_songCount;
    int m_duration;
    int m_year;
    QString m_genre;
    bool m_isFavorite;
};

#endif // ALBUM_H

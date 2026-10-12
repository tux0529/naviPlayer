/**
 * @file    album.h
 * @brief   专辑数据模型
 */
#ifndef NAVIPLAYER_ALBUM_H
#define NAVIPLAYER_ALBUM_H

#include <QString>
#include <QMetaType>
#include <QJsonObject>

class Album
{
public:
    Album() = default;

    QString id() const { return m_id; }
    void setId(const QString &v) { m_id = v; }

    QString name() const { return m_name; }
    void setName(const QString &v) { m_name = v; }

    QString artist() const { return m_artist; }
    void setArtist(const QString &v) { m_artist = v; }

    QString artistId() const { return m_artistId; }
    void setArtistId(const QString &v) { m_artistId = v; }

    QString coverArtId() const { return m_coverArtId; }
    void setCoverArtId(const QString &v) { m_coverArtId = v; }

    int songCount() const { return m_songCount; }
    void setSongCount(int v) { m_songCount = v; }

    int duration() const { return m_duration; }
    void setDuration(int v) { m_duration = v; }

    int year() const { return m_year; }
    void setYear(int v) { m_year = v; }

    QString genre() const { return m_genre; }
    void setGenre(const QString &v) { m_genre = v; }

    QString created() const { return m_created; }
    void setCreated(const QString &v) { m_created = v; }

    int playCount() const { return m_playCount; }
    void setPlayCount(int v) { m_playCount = v; }

    bool isFavorite() const { return m_isFavorite; }
    void setFavorite(bool v) { m_isFavorite = v; }

    int discCount() const { return m_discCount; }
    void setDiscCount(int v) { m_discCount = v; }

    bool isValid() const { return !m_id.isEmpty(); }

    QJsonObject toJson() const;
    static Album fromJson(const QJsonObject &obj);

private:
    QString m_id;
    QString m_name;
    QString m_artist;
    QString m_artistId;
    QString m_coverArtId;
    int m_songCount = 0;
    int m_duration = 0;
    int m_year = 0;
    QString m_genre;
    QString m_created;
    int m_playCount = 0;
    bool m_isFavorite = false;
    int m_discCount = 1;
};

Q_DECLARE_METATYPE(Album)

#endif // NAVIPLAYER_ALBUM_H

/**
 * @file    artist.h
 * @brief   艺术家数据模型
 */
#ifndef NAVIPLAYER_ARTIST_H
#define NAVIPLAYER_ARTIST_H

#include <QString>
#include <QMetaType>
#include <QJsonObject>

class Artist
{
public:
    Artist() = default;

    QString id() const { return m_id; }
    void setId(const QString &v) { m_id = v; }

    QString name() const { return m_name; }
    void setName(const QString &v) { m_name = v; }

    int albumCount() const { return m_albumCount; }
    void setAlbumCount(int v) { m_albumCount = v; }

    QString coverArtId() const { return m_coverArtId; }
    void setCoverArtId(const QString &v) { m_coverArtId = v; }

    bool isFavorite() const { return m_isFavorite; }
    void setFavorite(bool v) { m_isFavorite = v; }

    int playCount() const { return m_playCount; }
    void setPlayCount(int v) { m_playCount = v; }

    bool isValid() const { return !m_id.isEmpty(); }

    QJsonObject toJson() const;
    static Artist fromJson(const QJsonObject &obj);

private:
    QString m_id;
    QString m_name;
    int m_albumCount = 0;
    QString m_coverArtId;
    bool m_isFavorite = false;
    int m_playCount = 0;
};

Q_DECLARE_METATYPE(Artist)

#endif // NAVIPLAYER_ARTIST_H

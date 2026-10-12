#include "album.h"

QJsonObject Album::toJson() const
{
    QJsonObject o;
    o[QStringLiteral("id")] = m_id;
    o[QStringLiteral("name")] = m_name;
    o[QStringLiteral("artist")] = m_artist;
    o[QStringLiteral("artistId")] = m_artistId;
    o[QStringLiteral("coverArtId")] = m_coverArtId;
    o[QStringLiteral("songCount")] = m_songCount;
    o[QStringLiteral("duration")] = m_duration;
    o[QStringLiteral("year")] = m_year;
    o[QStringLiteral("genre")] = m_genre;
    o[QStringLiteral("created")] = m_created;
    o[QStringLiteral("playCount")] = m_playCount;
    o[QStringLiteral("isFavorite")] = m_isFavorite;
    o[QStringLiteral("discCount")] = m_discCount;
    return o;
}

Album Album::fromJson(const QJsonObject &obj)
{
    Album a;
    a.setId(obj.value(QStringLiteral("id")).toString());
    a.setName(obj.value(QStringLiteral("name")).toString());
    a.setArtist(obj.value(QStringLiteral("artist")).toString());
    a.setArtistId(obj.value(QStringLiteral("artistId")).toString());
    a.setCoverArtId(obj.value(QStringLiteral("coverArtId")).toString());
    a.setSongCount(obj.value(QStringLiteral("songCount")).toInt());
    a.setDuration(obj.value(QStringLiteral("duration")).toInt());
    a.setYear(obj.value(QStringLiteral("year")).toInt());
    a.setGenre(obj.value(QStringLiteral("genre")).toString());
    a.setCreated(obj.value(QStringLiteral("created")).toString());
    a.setPlayCount(obj.value(QStringLiteral("playCount")).toInt());
    a.setFavorite(obj.value(QStringLiteral("isFavorite")).toBool());
    a.setDiscCount(obj.value(QStringLiteral("discCount")).toInt(1));
    return a;
}

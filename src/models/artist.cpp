#include "artist.h"

QJsonObject Artist::toJson() const
{
    QJsonObject o;
    o[QStringLiteral("id")] = m_id;
    o[QStringLiteral("name")] = m_name;
    o[QStringLiteral("albumCount")] = m_albumCount;
    o[QStringLiteral("coverArtId")] = m_coverArtId;
    o[QStringLiteral("isFavorite")] = m_isFavorite;
    o[QStringLiteral("playCount")] = m_playCount;
    return o;
}

Artist Artist::fromJson(const QJsonObject &obj)
{
    Artist a;
    a.setId(obj.value(QStringLiteral("id")).toString());
    a.setName(obj.value(QStringLiteral("name")).toString());
    a.setAlbumCount(obj.value(QStringLiteral("albumCount")).toInt());
    a.setCoverArtId(obj.value(QStringLiteral("coverArtId")).toString());
    a.setFavorite(obj.value(QStringLiteral("isFavorite")).toBool());
    a.setPlayCount(obj.value(QStringLiteral("playCount")).toInt());
    return a;
}

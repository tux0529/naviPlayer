#include "song.h"

#include <QJsonValue>

QJsonObject Song::toJson() const
{
    QJsonObject o;
    o[QStringLiteral("id")] = m_id;
    o[QStringLiteral("title")] = m_title;
    o[QStringLiteral("artist")] = m_artist;
    o[QStringLiteral("artistId")] = m_artistId;
    o[QStringLiteral("album")] = m_album;
    o[QStringLiteral("albumId")] = m_albumId;
    o[QStringLiteral("track")] = m_track;
    o[QStringLiteral("discNumber")] = m_discNumber;
    o[QStringLiteral("year")] = m_year;
    o[QStringLiteral("genre")] = m_genre;
    o[QStringLiteral("duration")] = m_duration;
    o[QStringLiteral("size")] = static_cast<double>(m_size);
    o[QStringLiteral("suffix")] = m_suffix;
    o[QStringLiteral("contentType")] = m_contentType;
    o[QStringLiteral("coverArtId")] = m_coverArtId;
    o[QStringLiteral("path")] = m_path;
    o[QStringLiteral("bitRate")] = m_bitRate;
    o[QStringLiteral("sampleRate")] = m_sampleRate;
    o[QStringLiteral("channels")] = m_channels;
    o[QStringLiteral("playCount")] = m_playCount;
    o[QStringLiteral("created")] = m_created;
    o[QStringLiteral("isFavorite")] = m_isFavorite;
    return o;
}

Song Song::fromJson(const QJsonObject &obj)
{
    Song s;
    s.setId(obj.value(QStringLiteral("id")).toString());
    s.setTitle(obj.value(QStringLiteral("title")).toString());
    s.setArtist(obj.value(QStringLiteral("artist")).toString());
    s.setArtistId(obj.value(QStringLiteral("artistId")).toString());
    s.setAlbum(obj.value(QStringLiteral("album")).toString());
    s.setAlbumId(obj.value(QStringLiteral("albumId")).toString());
    s.setTrack(obj.value(QStringLiteral("track")).toInt());
    s.setDiscNumber(obj.value(QStringLiteral("discNumber")).toInt(1));
    s.setYear(obj.value(QStringLiteral("year")).toInt());
    s.setGenre(obj.value(QStringLiteral("genre")).toString());
    s.setDuration(obj.value(QStringLiteral("duration")).toInt());
    s.setSize(static_cast<qint64>(obj.value(QStringLiteral("size")).toDouble()));
    s.setSuffix(obj.value(QStringLiteral("suffix")).toString());
    s.setContentType(obj.value(QStringLiteral("contentType")).toString());
    s.setCoverArtId(obj.value(QStringLiteral("coverArtId")).toString());
    s.setPath(obj.value(QStringLiteral("path")).toString());
    s.setBitRate(obj.value(QStringLiteral("bitRate")).toInt());
    s.setSampleRate(obj.value(QStringLiteral("sampleRate")).toInt());
    s.setChannels(obj.value(QStringLiteral("channels")).toInt());
    s.setPlayCount(obj.value(QStringLiteral("playCount")).toInt());
    s.setCreated(obj.value(QStringLiteral("created")).toString());
    s.setFavorite(obj.value(QStringLiteral("isFavorite")).toBool());
    return s;
}

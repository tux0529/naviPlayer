#include "playlist.h"

#include <QJsonArray>

QJsonObject Playlist::toJson() const
{
    QJsonObject o;
    o[QStringLiteral("id")] = m_id;
    o[QStringLiteral("name")] = m_name;
    o[QStringLiteral("comment")] = m_comment;
    o[QStringLiteral("owner")] = m_owner;
    o[QStringLiteral("songCount")] = m_songCount;
    o[QStringLiteral("duration")] = m_duration;
    o[QStringLiteral("public")] = m_public;
    o[QStringLiteral("created")] = m_created;
    o[QStringLiteral("changed")] = m_changed;
    o[QStringLiteral("coverArtId")] = m_coverArtId;

    QJsonArray arr;
    for (const Song &s : m_songs)
        arr.append(s.toJson());
    o[QStringLiteral("songs")] = arr;
    return o;
}

Playlist Playlist::fromJson(const QJsonObject &obj)
{
    Playlist p;
    p.setId(obj.value(QStringLiteral("id")).toString());
    p.setName(obj.value(QStringLiteral("name")).toString());
    p.setComment(obj.value(QStringLiteral("comment")).toString());
    p.setOwner(obj.value(QStringLiteral("owner")).toString());
    p.setSongCount(obj.value(QStringLiteral("songCount")).toInt());
    p.setDuration(obj.value(QStringLiteral("duration")).toInt());
    p.setPublic(obj.value(QStringLiteral("public")).toBool());
    p.setCreated(obj.value(QStringLiteral("created")).toString());
    p.setChanged(obj.value(QStringLiteral("changed")).toString());
    p.setCoverArtId(obj.value(QStringLiteral("coverArtId")).toString());

    QVector<Song> songs;
    const QJsonArray arr = obj.value(QStringLiteral("songs")).toArray();
    songs.reserve(arr.size());
    for (const QJsonValue &v : arr)
        songs.append(Song::fromJson(v.toObject()));
    p.setSongs(songs);
    return p;
}

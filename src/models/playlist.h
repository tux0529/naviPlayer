/**
 * @file    playlist.h
 * @brief   播放列表数据模型（服务端歌单）
 */
#ifndef NAVIPLAYER_PLAYLIST_H
#define NAVIPLAYER_PLAYLIST_H

#include <QString>
#include <QMetaType>
#include <QJsonObject>
#include <QVector>

#include "models/song.h"

class Playlist
{
public:
    Playlist() = default;

    QString id() const { return m_id; }
    void setId(const QString &v) { m_id = v; }

    QString name() const { return m_name; }
    void setName(const QString &v) { m_name = v; }

    QString comment() const { return m_comment; }
    void setComment(const QString &v) { m_comment = v; }

    QString owner() const { return m_owner; }
    void setOwner(const QString &v) { m_owner = v; }

    int songCount() const { return m_songCount; }
    void setSongCount(int v) { m_songCount = v; }

    int duration() const { return m_duration; }
    void setDuration(int v) { m_duration = v; }

    bool isPublic() const { return m_public; }
    void setPublic(bool v) { m_public = v; }

    QString created() const { return m_created; }
    void setCreated(const QString &v) { m_created = v; }

    QString changed() const { return m_changed; }
    void setChanged(const QString &v) { m_changed = v; }

    QString coverArtId() const { return m_coverArtId; }
    void setCoverArtId(const QString &v) { m_coverArtId = v; }

    QVector<Song> songs() const { return m_songs; }
    void setSongs(const QVector<Song> &v) { m_songs = v; m_songCount = v.size(); }

    bool isValid() const { return !m_id.isEmpty(); }

    QJsonObject toJson() const;
    static Playlist fromJson(const QJsonObject &obj);

private:
    QString m_id;
    QString m_name;
    QString m_comment;
    QString m_owner;
    int m_songCount = 0;
    int m_duration = 0;
    bool m_public = false;
    QString m_created;
    QString m_changed;
    QString m_coverArtId;
    QVector<Song> m_songs;
};

Q_DECLARE_METATYPE(Playlist)

#endif // NAVIPLAYER_PLAYLIST_H

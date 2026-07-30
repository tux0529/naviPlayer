#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QString>
#include <QVector>
#include "song.h"

class Playlist
{
public:
    enum PlayMode {
        Sequential,
        Loop,
        Random
    };

    Playlist();

    QString id() const { return m_id; }
    void setId(const QString &id) { m_id = id; }

    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    QString comment() const { return m_comment; }
    void setComment(const QString &comment) { m_comment = comment; }

    QVector<Song> songs() const { return m_songs; }
    void setSongs(const QVector<Song> &songs) { m_songs = songs; }
    void addSong(const Song &song) { m_songs.append(song); }
    void removeSong(int index);

    int songCount() const { return m_songs.count(); }

    PlayMode playMode() const { return m_playMode; }
    void setPlayMode(PlayMode mode) { m_playMode = mode; }

private:
    QString m_id;
    QString m_name;
    QString m_comment;
    QVector<Song> m_songs;
    PlayMode m_playMode;
};

#endif // PLAYLIST_H

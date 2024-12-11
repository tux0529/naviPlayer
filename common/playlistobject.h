#ifndef PLAYLISTOBJECT_H
#define PLAYLISTOBJECT_H

#include "playlist.h"
#include "track.h"

class PlayListObject : public PlayList
{
public:
    PlayListObject();

    const QString &modfiyTime() const { return this->m_modfiyTime;}
    void setModfiyTime( const QString &modfiyTime);

    const QString &owner() const { return this->m_owner;}
    void setOwner( const QString &owner);

    void addTrack(const Track &tk);
    void addTracks(const TrackList &tl);

    const Track &at(int index);
    const TrackList &tracks();

private:

    QString m_modfiyTime;

    QString m_owner;

    TrackList m_trackList;
};

#endif // PLAYLISTOBJECT_H

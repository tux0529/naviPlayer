#include "playlistobject.h"

PlayListObject::PlayListObject() {}

void PlayListObject::setModfiyTime(const QString &modfiyTime)
{
    this->m_modfiyTime = modfiyTime;
}

void PlayListObject::setOwner(const QString &owner)
{
    this->m_owner = owner;
}

void PlayListObject::addTrack(Track &tk)
{
    m_trackList << tk;
}

void PlayListObject::addTracks(TrackList &tl)
{
    m_trackList << tl;
}

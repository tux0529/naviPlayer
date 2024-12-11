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

void PlayListObject::addTrack(const Track &tk)
{
    m_trackList << tk;
}

void PlayListObject::addTracks(const TrackList &tl)
{
    m_trackList << tl;
}

const Track &PlayListObject::at(int index)
{
    return m_trackList.at(index);
}

const TrackList &PlayListObject::tracks()
{
    return this->m_trackList;
}

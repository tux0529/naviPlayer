#include "albumobject.h"

AlbumObject::AlbumObject() {}

void AlbumObject::addTrack(Track &s)
{
    m_trackList << s;
}

void AlbumObject::addTracks(TrackList &sl)
{
    m_trackList << sl;
}

const Track &AlbumObject::at(int index)
{
    return m_trackList.at(index);
}

const TrackList &AlbumObject::tracks()
{
    return this->m_trackList;
}

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

Track AlbumObject::at(int index)
{
    if(index < 0 || index >= m_trackList.size()){
        Track tk;
        return tk;
    }
    return m_trackList.at(index);
}

const TrackList &AlbumObject::tracks()
{
    return this->m_trackList;
}

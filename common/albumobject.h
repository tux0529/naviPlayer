#ifndef ALBUMOBJECT_H
#define ALBUMOBJECT_H

#include "album.h"
#include "track.h"

class AlbumObject : public Album
{
public:
    AlbumObject();

    QString genre() const{ return this->m_genre; }
    void setGenre(const QString &genre) { this->m_genre = genre; }

    int year() const{ return this->m_year; }
    void setYear(int year) { this->m_year = year; }

    int songCount() const{ return this->m_trackList.size(); }

    void addTrack(Track &s);
    void addTracks(TrackList &sl);

    Track at(int index);
    const TrackList &tracks();

private:

    QString m_genre;

    int m_year;

    TrackList m_trackList;
};

#endif // ALBUMOBJECT_H

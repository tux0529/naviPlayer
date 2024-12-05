#ifndef TRACK_H
#define TRACK_H

#include "mediabase.h"


class Track : public MediaBase
{
public:
    Track();
    ~Track();

    const QString &album() const{ return this->m_album; }
    void setAlbum(const QString &album) { this->m_album = album; }

    const QString &artist() const{ return this->m_artist; }
    void setArtist(const QString &artist) { this->m_artist = artist; }

    QString track() const{ return this->m_track; }
    void setTrack(const QString &track) { this->m_track = track; }

    const QString &duration() const{ return this->m_duration; }
    void setDuration(const QString &duration) { this->m_duration = duration; }

    int bitRate() const{ return this->m_bitRate; }
    void setBitRate(int bitRate) { this->m_bitRate = bitRate; }

    QString albumId() const{ return this->m_albumId; }
    void setAlbumId(const QString &albumId) { this->m_albumId = albumId; }

    QString artistId() const{ return this->m_artistId; }
    void setArtistId(const QString &artistId) { this->m_artistId = artistId; }

    QString cacheFilepath() const{ return this->m_cacheFilepath; }
    void setCacheFilepath(const QString &cacheFilepath) { this->m_cacheFilepath = cacheFilepath; }


private:
    QString m_album;
    QString m_artist;
    QString m_track;
    QString m_duration;
    int m_bitRate;
    QString m_albumId;
    QString m_artistId;

    QString m_cacheFilepath;
};

typedef QList<Track> TrackList;

#endif // TRACK_H


#ifndef ALBUM_H
#define ALBUM_H

#include <QFileInfo>
#include "mediabase.h"

class Album : public MediaBase
{
public:
    Album();
    ~Album();

    const QString &artist() const{ return this->m_artist; }
    void setArtist(const QString &artist) { this->m_artist = artist; }

    const QString &artistId() const{ return this->m_artistId; }
    void setArtistId(const QString &artistId) { this->m_artistId = artistId; }

    int songCount() const{ return this->m_songCount; }
    void setSongCount(int songCount) { this->m_songCount = songCount; }

    const QString &duration() const{ return this->m_duration; }
    void setDuration(const QString &duration) { this->m_duration = duration; }

private:
    QString m_artist;
    QString m_artistId;

    int m_songCount;
    QString m_duration;

};


typedef QList<Album> AlbumList;


#endif // ALBUM_H

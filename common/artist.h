#ifndef ARTIST_H
#define ARTIST_H

#include "mediabase.h"

class Artist : public MediaBase
{
public:
    Artist(const QString &id, const QString &name, int albumCount);

    int albumCount() const{ return this->m_albumCount; }

    void setIcon(const QString &coverPath);
    const QIcon &icon() const { return this->m_icon; };

private:

    int m_albumCount;
    QIcon m_icon;

};

typedef QList<Artist> ArtistList;

#endif // ARTIST_H

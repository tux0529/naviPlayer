#ifndef ARTIST_H
#define ARTIST_H

#include "mediabase.h"

class Artist : public MediaBase
{
public:
    Artist(const QString &id, const QString &name, const QString &cover, int albumCount);

    int albumCount() const{ return this->m_albumCount; }


private:

    int m_albumCount;

};

typedef QList<Artist> ArtistList;

#endif // ARTIST_H

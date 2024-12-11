#ifndef ARTISTOBJECT_H
#define ARTISTOBJECT_H

#include "artist.h"
#include "album.h"

class ArtistObject : public Artist
{
public:
    ArtistObject();
    ~ArtistObject();

    void addAlbum(const Album &a);
    void addAlbums(const AlbumList &al);

    const Album &at(int index);
    const AlbumList &albums();

private:
    AlbumList m_albumList;
};

#endif // ARTISTOBJECT_H

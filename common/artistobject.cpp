#include "artistobject.h"

ArtistObject::ArtistObject() {}

ArtistObject::~ArtistObject()
{

}

void ArtistObject::addAlbum(const Album &a)
{
    m_albumList << a;
}

void ArtistObject::addAlbums(const AlbumList &al)
{
    m_albumList << al;
}

const Album &ArtistObject::at(int index)
{
    return m_albumList.at(index);
}

const AlbumList &ArtistObject::albums()
{
    return this->m_albumList;
}

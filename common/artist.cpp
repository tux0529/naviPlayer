#include "artist.h"

Artist::Artist()
{

}

Artist::Artist(const QString &id, const QString &name, int albumCount)
{
    this->setId(id);
    this->setName(name);

    this->m_albumCount = albumCount;
}

Artist::~Artist()
{

}

void Artist::setAlbumCount(int count)
{
    this->m_albumCount = count;
}

void Artist::setIcon(const QString &coverPath)
{
    m_icon.addFile(coverPath);
}


#include "artist.h"

Artist::Artist(const QString &id, const QString &name, const QString &cover, int albumCount)
{
    this->setId(id);
    this->setName(name);
    this->setCoverId(cover);

    this->m_albumCount = albumCount;
}


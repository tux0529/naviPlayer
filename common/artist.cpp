#include "artist.h"

Artist::Artist(const QString &id, const QString &name, int albumCount)
{
    this->setId(id);
    this->setName(name);

    this->m_albumCount = albumCount;
}

void Artist::setIcon(const QString &coverPath)
{
    m_icon.addFile(coverPath);
}


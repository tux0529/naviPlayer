#include "genre.h"

Genre::Genre(const QString &name, int songCount, int albumCount)
{
    this->m_name = name;
    this->m_songCount = songCount;
    this->m_albumCount = albumCount;
}

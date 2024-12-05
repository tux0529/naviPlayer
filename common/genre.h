#ifndef GENRE_H
#define GENRE_H

#include <QString>

class Genre
{
public:
    Genre(const QString &name, int songCount, int albumCount);

    QString name() const{ return this->m_name; }
    int songCount() const{ return this->m_songCount; }
    int albumCount() const{ return this->m_albumCount; }

private:
    QString m_name;
    int m_songCount;
    int m_albumCount;
};

typedef QList<Genre> GenreList;

#endif // GENRE_H

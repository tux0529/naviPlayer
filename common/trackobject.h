#ifndef TRACKOBJECT_H
#define TRACKOBJECT_H

#include "track.h"

class TrackObject : public Track
{
public:
    TrackObject();

    QString parent() const{ return this->m_parent; }
    void setParent(const QString &parent) { this->m_parent = parent; }

    QString genre() const{ return this->m_genre; }
    void setGenre(const QString &genre) { this->m_genre = genre; }

    QString suffix() const{ return this->m_suffix; }
    void setSuffix(const QString &suffix) { this->m_suffix = suffix; }

    int size() const{ return this->m_size; }
    void setSize(int size) { this->m_size = size; }

private:
    QString m_parent;

    QString m_genre;
    QString m_suffix;
    int m_size;

};

#endif // TRACKOBJECT_H

#include "mediabase.h"

MediaBase::MediaBase() {}

void MediaBase::setId(const QString &id)
{
    this->m_id = id;
}

void MediaBase::setName(const QString &name)
{
    this->m_name = name;
}

void MediaBase::setCoverId(const QString &coverId)
{
    this->m_coverId = coverId;
}

void MediaBase::setArtDirpath(const QString &dirpath)
{
    this->m_artDirpath = dirpath;
}

QString MediaBase::coverPath() const
{
    return this->m_artDirpath + "cover";
}

QString MediaBase::posterPath() const
{
    return this->m_artDirpath + "poster";
}

QString MediaBase::iconPath() const
{
    return this->m_artDirpath + "icon";
}

void MediaBase::setIcon(const QString &coverPath)
{
    m_icon.addFile(coverPath);
}

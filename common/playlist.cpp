#include "playlist.h"

PlayList::PlayList()
{

}

void PlayList::setSongCount(int songCount)
{
    this->m_songCount = songCount;
}

void PlayList::setDuration(int duration)
{
    this->m_duration = duration;
}

void PlayList::setIcon(const QString &coverPath)
{
    m_icon.addFile(coverPath);
}

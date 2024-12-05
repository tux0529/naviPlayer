#ifndef PLAYLIST_H
#define PLAYLIST_H

#include "mediabase.h"

class PlayList : public MediaBase
{
public:
    explicit PlayList();

    int songCount() const { return this->m_songCount;}
    void setSongCount( int songCount);

    int duration() const { return this->m_duration;}
    void setDuration( int duration);

private:

    int m_songCount;

    int m_duration;

};


#endif // PLAYLIST_H


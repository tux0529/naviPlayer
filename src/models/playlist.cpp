#include "playlist.h"

Playlist::Playlist()
    : m_playMode(Sequential)
{
}

void Playlist::removeSong(int index)
{
    if (index >= 0 && index < m_songs.size()) {
        m_songs.removeAt(index);
    }
}

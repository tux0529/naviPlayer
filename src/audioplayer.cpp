#include "audioplayer.h"
#include <QUrl>
#include <QDebug>
#include <QRandomGenerator>

AudioPlayer::AudioPlayer(QObject *parent)
    : QObject(parent)
    , m_state(Stopped)
    , m_currentIndex(-1)
    , m_playMode(Sequential)
{
    m_decoder = new FFmpegAudioDecoder(this);
    connect(m_decoder, &FFmpegAudioDecoder::stateChanged,
            this, &AudioPlayer::onDecoderStateChanged);
    connect(m_decoder, &FFmpegAudioDecoder::positionChanged,
            this, &AudioPlayer::onDecoderPositionChanged);
    connect(m_decoder, &FFmpegAudioDecoder::durationChanged,
            this, &AudioPlayer::onDecoderDurationChanged);
    connect(m_decoder, &FFmpegAudioDecoder::errorOccurred,
            this, &AudioPlayer::onDecoderError);
    connect(m_decoder, &FFmpegAudioDecoder::finished,
            this, &AudioPlayer::onDecoderFinished);
}

AudioPlayer::~AudioPlayer()
{
    stop();
}

void AudioPlayer::play(const Song &song, const QString &streamUrl)
{
    if (m_currentSong.id() == song.id() && m_state == Paused) {
        resume();
        return;
    }

    emitTrackAboutToChange();
    stop();

    m_currentSong = song;
    m_playlist.clear();
    m_streamUrls.clear();
    m_playlist.append(song);
    m_streamUrls.append(streamUrl);
    m_currentIndex = 0;

    emit playlistChanged(m_playlist);
    emit currentIndexChanged(m_currentIndex);

    if (m_decoder->open(streamUrl)) {
        m_decoder->play();
    }
}

void AudioPlayer::playPlaylist(const QVector<Song> &songs, const QStringList &streamUrls, int startIndex)
{
    if (songs.size() != streamUrls.size()) {
        qDebug() << "Song count and stream URL count mismatch!";
        return;
    }

    if (songs.isEmpty() || startIndex < 0 || startIndex >= songs.size()) {
        return;
    }

    stop();

    m_playlist = songs;
    m_streamUrls = streamUrls;
    m_currentIndex = startIndex;

    if (m_playMode == Shuffle) {
        generateShuffleIndices();
    }

    emit playlistChanged(m_playlist);
    emit currentIndexChanged(m_currentIndex);

    playSongAt(m_currentIndex);
}

void AudioPlayer::restoreSession(const QVector<Song> &songs, const QStringList &streamUrls,
                                int startIndex, qint64 position, PlayMode playMode)
{
    if (songs.size() != streamUrls.size() || songs.isEmpty()
            || startIndex < 0 || startIndex >= songs.size()) {
        return;
    }

    stop();

    m_playlist = songs;
    m_streamUrls = streamUrls;
    m_currentIndex = startIndex;
    m_playMode = playMode;

    if (m_playMode == Shuffle) {
        generateShuffleIndices();
    }

    m_currentSong = m_playlist[startIndex];

    emit playlistChanged(m_playlist);
    emit currentIndexChanged(m_currentIndex);
    emit playModeChanged(m_playMode);

    // 以暂停状态从指定进度恢复，等待用户继续播放
    m_decoder->playPaused(m_streamUrls[startIndex], position);
}

void AudioPlayer::emitTrackAboutToChange()
{
    if (!m_currentSong.id().isEmpty()) {
        emit trackAboutToChange(m_currentSong.id(), m_decoder->position());
    }
}

void AudioPlayer::addToPlaylist(const Song &song, const QString &streamUrl)
{
    m_playlist.append(song);
    m_streamUrls.append(streamUrl);
    
    if (m_playMode == Shuffle) {
        generateShuffleIndices();
    }
    
    emit playlistChanged(m_playlist);
}

void AudioPlayer::addToNext(const QVector<Song> &songs, const QStringList &streamUrls)
{
    if (songs.isEmpty() || songs.size() != streamUrls.size()) {
        return;
    }

    int insertPos = m_playlist.size();
    if (hasValidCurrentIndex()) {
        insertPos = m_currentIndex + 1;
    }

    for (int i = 0; i < songs.size(); ++i) {
        m_playlist.insert(insertPos + i, songs[i]);
        m_streamUrls.insert(insertPos + i, streamUrls[i]);
    }

    if (m_playMode == Shuffle) {
        generateShuffleIndices();
    }

    emit playlistChanged(m_playlist);
    emit currentIndexChanged(m_currentIndex);
}

void AudioPlayer::removeFromPlaylist(int index)
{
    if (index >= 0 && index < m_playlist.size()) {
        m_playlist.removeAt(index);
        m_streamUrls.removeAt(index);
        
        if (index < m_currentIndex) {
            m_currentIndex--;
        } else if (index == m_currentIndex) {
            if (m_playlist.isEmpty()) {
                m_currentIndex = -1;
                stop();
            } else if (m_currentIndex >= m_playlist.size()) {
                m_currentIndex = m_playlist.size() - 1;
                playSongAt(m_currentIndex);
            } else {
                playSongAt(m_currentIndex);
            }
        }
        
        if (m_playMode == Shuffle) {
            generateShuffleIndices();
        }
        
        emit playlistChanged(m_playlist);
        emit currentIndexChanged(m_currentIndex);
    }
}

void AudioPlayer::clearPlaylist()
{
    stop();
    m_playlist.clear();
    m_streamUrls.clear();
    m_currentIndex = -1;
    m_shuffleIndices.clear();
    emit playlistChanged(m_playlist);
    emit currentIndexChanged(m_currentIndex);
}

void AudioPlayer::playNext()
{
    if (m_playlist.isEmpty() || !hasValidCurrentIndex()) {
        return;
    }

    int nextIndex = -1;
    switch (m_playMode) {
    case Sequential:
        if (m_currentIndex < m_playlist.size() - 1) {
            nextIndex = m_currentIndex + 1;
        }
        break;
    case RepeatOne:
        nextIndex = m_currentIndex;
        break;
    case RepeatAll:
        if (m_currentIndex < m_playlist.size() - 1) {
            nextIndex = m_currentIndex + 1;
        } else {
            nextIndex = 0;
        }
        break;
    case Shuffle:
        if (!m_shuffleIndices.isEmpty()) {
            int currentShufflePos = m_shuffleIndices.indexOf(m_currentIndex);
            if (currentShufflePos < m_shuffleIndices.size() - 1) {
                nextIndex = m_shuffleIndices[currentShufflePos + 1];
            } else {
                nextIndex = m_shuffleIndices[0];
            }
        }
        break;
    }

    if (nextIndex != -1) {
        playSongAt(nextIndex);
    }
}

void AudioPlayer::playPrevious()
{
    if (m_playlist.isEmpty() || !hasValidCurrentIndex()) {
        return;
    }

    int prevIndex = -1;
    switch (m_playMode) {
    case Sequential:
        if (m_currentIndex > 0) {
            prevIndex = m_currentIndex - 1;
        }
        break;
    case RepeatOne:
        prevIndex = m_currentIndex;
        break;
    case RepeatAll:
        if (m_currentIndex > 0) {
            prevIndex = m_currentIndex - 1;
        } else {
            prevIndex = m_playlist.size() - 1;
        }
        break;
    case Shuffle:
        if (!m_shuffleIndices.isEmpty()) {
            int currentShufflePos = m_shuffleIndices.indexOf(m_currentIndex);
            if (currentShufflePos > 0) {
                prevIndex = m_shuffleIndices[currentShufflePos - 1];
            } else {
                prevIndex = m_shuffleIndices.last();
            }
        }
        break;
    }

    if (prevIndex != -1) {
        playSongAt(prevIndex);
    }
}

void AudioPlayer::playSongAt(int index)
{
    if (index < 0 || index >= m_playlist.size() || index >= m_streamUrls.size()) {
        return;
    }

    emitTrackAboutToChange();
    stop();

    m_currentIndex = index;
    m_currentSong = m_playlist[index];
    emit currentIndexChanged(m_currentIndex);

    if (m_decoder->open(m_streamUrls[index])) {
        m_decoder->play();
    }
}

void AudioPlayer::updateSongSource(const QString &songId, const QString &source)
{
    if (songId.isEmpty() || source.isEmpty()) {
        return;
    }

    for (int i = 0; i < m_playlist.size() && i < m_streamUrls.size(); ++i) {
        if (m_playlist[i].id() == songId) {
            m_streamUrls[i] = source;
        }
    }
}

void AudioPlayer::pause()
{
    if (m_state == Playing) {
        m_decoder->pause();
    }
}

void AudioPlayer::resume()
{
    if (m_state == Paused) {
        m_decoder->play();
    }
}

void AudioPlayer::stop()
{
    m_decoder->stop();
    m_currentSong = Song();
}

void AudioPlayer::seek(qint64 position)
{
    m_decoder->seek(position);
}

void AudioPlayer::setVolume(int volume)
{
    m_decoder->setVolume(volume);
}

int AudioPlayer::volume() const
{
    return m_decoder->volume();
}

void AudioPlayer::setPlayMode(PlayMode mode)
{
    if (m_playMode != mode) {
        m_playMode = mode;

        if (m_playMode == Shuffle && !m_playlist.isEmpty()) {
            generateShuffleIndices();
        }

        emit playModeChanged(m_playMode);
    }
}

AudioPlayer::PlayMode AudioPlayer::playMode() const
{
    return m_playMode;
}

void AudioPlayer::nextPlayMode()
{
    PlayMode newMode;
    switch (m_playMode) {
    case Sequential:
        newMode = RepeatOne;
        break;
    case RepeatOne:
        newMode = RepeatAll;
        break;
    case RepeatAll:
        newMode = Shuffle;
        break;
    case Shuffle:
        newMode = Sequential;
        break;
    }
    setPlayMode(newMode);
}

void AudioPlayer::generateShuffleIndices()
{
    m_shuffleIndices.clear();
    for (int i = 0; i < m_playlist.size(); ++i) {
        m_shuffleIndices.append(i);
    }

    // Shuffle using Fisher-Yates algorithm
    for (int i = m_shuffleIndices.size() - 1; i > 0; --i) {
        int j = QRandomGenerator::global()->bounded(i + 1);
        m_shuffleIndices.swapItemsAt(i, j);
    }
}

bool AudioPlayer::hasValidCurrentIndex() const
{
    return m_currentIndex >= 0
            && m_currentIndex < m_playlist.size()
            && m_currentIndex < m_streamUrls.size();
}

qint64 AudioPlayer::position() const
{
    return m_decoder->position();
}

qint64 AudioPlayer::duration() const
{
    return m_decoder->duration();
}

void AudioPlayer::setState(State state)
{
    if (m_state != state) {
        m_state = state;
        emit stateChanged(m_state);
    }
}

void AudioPlayer::onDecoderStateChanged(bool playing)
{
    if (playing) {
        setState(Playing);
    } else if (m_decoder->isPaused()) {
        setState(Paused);
    } else {
        setState(Stopped);
    }
}

void AudioPlayer::onDecoderPositionChanged(qint64 position)
{
    emit positionChanged(position);
}

void AudioPlayer::onDecoderDurationChanged(qint64 duration)
{
    emit durationChanged(duration);
}

void AudioPlayer::onDecoderError(const QString &error)
{
    emit errorOccurred(error);
}

void AudioPlayer::onDecoderFinished()
{
    // 播放下一首
    playNext();
}

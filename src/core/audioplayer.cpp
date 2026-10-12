#include "core/audioplayer.h"

#include "core/cachemanager.h"
#include "core/appsettings.h"
#include "core/databasemanager.h"

#include <QAudioFormat>
#include <QDebug>

namespace {
constexpr int kRingCapacity = 1024 * 1024;      // 1MB 环形缓冲（约 5.8 秒 44.1k/立体声/S16）
constexpr int kTickIntervalMs = 100;
// 起播前至少预填充的字节数与最长等待时间
constexpr int kPrebufferBytes = 64 * 1024;
constexpr int kPrebufferTimeoutMs = 4000;
constexpr int kPrebufferTickMs = 50;
}

AudioPlayer::AudioPlayer(CacheManager *cache, AppSettings *settings, DatabaseManager *db,
                         QObject *parent)
    : QObject(parent)
    , m_cache(cache)
    , m_settings(settings)
    , m_db(db)
{
    m_decoder = new FFmpegAudioDecoder(this);
    connect(m_decoder, &FFmpegAudioDecoder::opened, this, &AudioPlayer::onDecoderOpened);
    connect(m_decoder, &FFmpegAudioDecoder::finished, this, &AudioPlayer::onDecoderFinished);
    connect(m_decoder, &FFmpegAudioDecoder::errorOccurred, this, &AudioPlayer::errorOccurred);

    m_tickTimer = new QTimer(this);
    connect(m_tickTimer, &QTimer::timeout, this, &AudioPlayer::onTick);

    m_prebufferTimer = new QTimer(this);
    m_prebufferTimer->setInterval(kPrebufferTickMs);
    connect(m_prebufferTimer, &QTimer::timeout, this, &AudioPlayer::onPrebufferTick);

    m_drainTimer = new QTimer(this);
    m_drainTimer->setInterval(150);
    connect(m_drainTimer, &QTimer::timeout, this, [this]() {
        if (!m_ring || !m_sink)
            return;
        if (m_ring->bufferedBytes() > 0)
            return;
        m_drainTimer->stop();
        handleTrackFinished();
    });

    if (m_cache) {
        connect(m_cache, &CacheManager::songCached, this, &AudioPlayer::onSongCached);
        connect(m_cache, &CacheManager::downloadProgress, this, &AudioPlayer::onDownloadProgress);
        connect(m_cache, &CacheManager::downloadFinished, this, &AudioPlayer::onDownloadFinished);
    }

    if (m_settings) {
        m_volume = m_settings->volume();
    }
}

AudioPlayer::~AudioPlayer()
{
    teardownCurrent();
}

void AudioPlayer::setQueue(PlaybackQueue *queue)
{
    m_queue = queue;
    if (m_queue) {
        m_queue->setPlayMode(static_cast<PlaybackQueue::PlayMode>(m_settings ? m_settings->playMode() : 1));
        connect(m_queue, &PlaybackQueue::playModeChanged, this, [this](PlaybackQueue::PlayMode mode) {
            if (m_settings)
                m_settings->setPlayMode(static_cast<int>(mode));
        });
    }
}

void AudioPlayer::setState(State state)
{
    if (m_state == state)
        return;
    m_state = state;
    Q_EMIT stateChanged(m_state);
}

void AudioPlayer::setVolume(int volume)
{
    m_volume = qBound(0, volume, 100);
    if (m_sink)
        m_sink->setVolume(static_cast<qreal>(m_volume) / 100.0);
    if (m_settings)
        m_settings->setVolume(m_volume);
    Q_EMIT volumeChanged(m_volume);
}

void AudioPlayer::teardownCurrent()
{
    m_tickTimer->stop();
    m_drainTimer->stop();
    m_prebufferTimer->stop();
    m_prebufferElapsedMs = 0;

    if (m_sink) {
        m_sink->stop();
        delete m_sink;
        m_sink = nullptr;
    }

    if (m_decoder) {
        m_decoder->stop();
        // 等待解码线程真正退出，避免其继续写入已释放的环形缓冲
        m_decoder->waitFinished(1000);
        // 必须先解绑：否则解码器会一直持有已释放的环形缓冲，
        // 下一次 teardownCurrent() 调用 decoder->stop() 时触发野指针崩溃
        m_decoder->setRingBuffer(nullptr);
    }

    if (m_ring) {
        m_ring->close();
        m_ring->deleteLater();
        m_ring = nullptr;
    }

    m_positionMs = 0;
    m_durationMs = 0;
    m_startOffsetMs = 0;
}

void AudioPlayer::stop()
{
    teardownCurrent();
    ++m_generation;
    m_currentSong = Song();
    setState(State::Stopped);
    Q_EMIT currentSongChanged(m_currentSong);
}

void AudioPlayer::beginPlayback(const Song &song, qint64 startPositionMs)
{
    if (!song.isValid())
        return;

    teardownCurrent();
    ++m_generation;
    m_scrobbled = false;

    m_currentSong = song;
    m_positionMs = startPositionMs;
    m_startOffsetMs = startPositionMs;
    m_durationMs = song.duration() > 0 ? (qint64)song.duration() * 1000 : 0;
    Q_EMIT currentSongChanged(song);
    Q_EMIT durationChanged(m_durationMs);
    Q_EMIT positionChanged(m_positionMs);

    if (m_db)
        m_db->addHistory(song);

    const QString localPath = m_cache ? m_cache->cachedSongPath(song.id()) : QString();
    if (!localPath.isEmpty()) {
        startDecoding(localPath, startPositionMs);
        return;
    }

    // 未缓存：先下载到本地缓存再解码（后台线程进行，界面显示缓冲进度）
    if (!m_cache) {
        Q_EMIT errorOccurred(QStringLiteral("缓存模块不可用，无法播放"));
        return;
    }

    setState(State::Buffering);
    Q_EMIT bufferProgressChanged(0);
    m_pendingSongId = song.id();
    m_cache->cacheSong(song);
}

void AudioPlayer::startDecoding(const QString &localPath, qint64 startPositionMs)
{
    m_decodeGeneration = m_generation;
    m_ring = new AudioRingBuffer(kRingCapacity, this);
    m_ring->open(QIODevice::ReadWrite | QIODevice::Unbuffered);

    m_decoder->setRingBuffer(m_ring);
    m_decoder->start(localPath, startPositionMs);
}

void AudioPlayer::onSongCached(const QString &songId, const QString &path)
{
    if (songId != m_pendingSongId)
        return;
    m_pendingSongId.clear();

    if (m_currentSong.id() != songId)
        return;

    startDecoding(path, m_startOffsetMs);
}

void AudioPlayer::onDownloadFinished(const QString &songId, const QString &path, bool ok)
{
    Q_UNUSED(path)
    if (ok || songId.isEmpty())
        return;
    if (songId != m_pendingSongId && songId != m_currentSong.id())
        return;

    // 下载失败：结束缓冲状态，避免界面一直停在"缓冲中"
    m_pendingSongId.clear();
    teardownCurrent();
    setState(State::Stopped);
    Q_EMIT errorOccurred(QStringLiteral("歌曲下载失败，无法播放"));
}

void AudioPlayer::onDownloadProgress(const QString &songId, qint64 received, qint64 total)
{
    if (songId != m_pendingSongId)
        return;
    if (total <= 0)
        return;
    const int percent = static_cast<int>(received * 100 / total);
    Q_EMIT bufferProgressChanged(qBound(0, percent, 100));
}

void AudioPlayer::onDecoderOpened(bool ok, qint64 durationMs)
{
    if (m_generation != m_decodeGeneration)
        return;     // 上一个解码线程的迟到信号

    if (!ok) {
        Q_EMIT errorOccurred(QStringLiteral("解码失败"));
        setState(State::Stopped);
        return;
    }

    m_durationMs = durationMs;
    m_bitRate = m_decoder->bitRate();
    Q_EMIT durationChanged(m_durationMs);
    emitMetadata();

    // 等缓冲积累到一定量再开设备，避免首帧就欠载（startOutput 会同步拉取数据）
    startPrebuffering();
}

void AudioPlayer::startPrebuffering()
{
    m_prebufferElapsedMs = 0;
    if (!m_ring || !m_prebufferTimer) {
        startOutput();
        setState(State::Playing);
        m_tickTimer->start(kTickIntervalMs);
        return;
    }
    setState(State::Buffering);
    m_prebufferTimer->start();
}

void AudioPlayer::onPrebufferTick()
{
    if (!m_ring || !m_decoder) {
        m_prebufferTimer->stop();
        return;
    }

    m_prebufferElapsedMs += kPrebufferTickMs;
    const bool enough = m_ring->bufferedBytes() >= kPrebufferBytes;
    if (!enough && m_prebufferElapsedMs < kPrebufferTimeoutMs && !m_ring->endOfStream())
        return;

    m_prebufferTimer->stop();
    startOutput();
    setState(State::Playing);
    m_tickTimer->start(kTickIntervalMs);
}

void AudioPlayer::startOutput()
{
    if (m_sink) {
        m_sink->stop();
        delete m_sink;
        m_sink = nullptr;
    }

    QAudioFormat format;
    naviConfigurePcmFormat(format, FFmpegAudioDecoder::kOutputSampleRate,
                           FFmpegAudioDecoder::kOutputChannels);

    m_sink = new NaviAudioOutput(format, this);
    m_sink->setVolume(static_cast<qreal>(m_volume) / 100.0);
    m_sink->setBufferSize(FFmpegAudioDecoder::kOutputSampleRate * FFmpegAudioDecoder::kOutputChannels * 2);
    m_sink->start(m_ring);
}

void AudioPlayer::emitMetadata()
{
    Song song = m_currentSong;
    song.setBitRate(m_bitRate);
    song.setSampleRate(FFmpegAudioDecoder::kOutputSampleRate);
    song.setChannels(FFmpegAudioDecoder::kOutputChannels);
    Q_EMIT metadataReady(song, m_bitRate, FFmpegAudioDecoder::kOutputSampleRate,
                         FFmpegAudioDecoder::kOutputChannels);
}

void AudioPlayer::onTick()
{
    if (!m_sink || m_state != State::Playing)
        return;

    const qint64 processed = m_sink->processedUSecs() / 1000;
    qint64 pos = m_startOffsetMs + processed;
    if (m_durationMs > 0)
        pos = qMin(pos, m_durationMs);
    if (pos == m_positionMs)
        return;

    m_positionMs = pos;
    Q_EMIT positionChanged(pos);

    maybeScrobble(false);
}

void AudioPlayer::maybeScrobble(bool completed)
{
    if (m_scrobbled || !m_currentSong.isValid() || m_state == State::Stopped)
        return;
    if (m_positionMs <= 0)
        return;

    // 播完即上报；未播完则播放过半才上报（避免切歌刷播放量）
    if (!completed && m_durationMs > 0 && m_positionMs < m_durationMs / 2)
        return;

    m_scrobbled = true;
    Q_EMIT songPlayed(m_currentSong);
}

void AudioPlayer::onDecoderFinished()
{
    if (m_generation != m_decodeGeneration)
        return;     // 上一个解码线程的迟到信号（切歌时避免误判为播完）

    // 等待缓冲播放完毕再切歌
    if (m_ring && m_sink && m_ring->bufferedBytes() > 0) {
        m_drainTimer->start();
        return;
    }
    handleTrackFinished();
}

void AudioPlayer::handleTrackFinished()
{
    // 整首播完：上报播放记录
    maybeScrobble(true);

    if (!m_queue || m_queue->isEmpty()) {
        stop();
        return;
    }

    const int next = m_queue->nextIndex(true);
    if (next < 0) {
        stop();
        return;
    }

    m_queue->setCurrentIndex(next);
    beginPlayback(m_queue->songAt(next), 0);
}

void AudioPlayer::seek(qint64 positionMs)
{
    if (m_state == State::Stopped || !m_decoder || !m_ring)
        return;

    if (m_sink)
        m_sink->stop();

    m_ring->resetForSeek();
    m_decoder->seek(positionMs);

    m_startOffsetMs = positionMs;
    m_positionMs = positionMs;
    Q_EMIT positionChanged(positionMs);

    if (m_sink)
        m_sink->start(m_ring);

    if (m_state == State::Paused) {
        // 暂停状态下跳转保持暂停
        if (m_sink)
            m_sink->suspend();
    }
}

void AudioPlayer::pause()
{
    if (m_state != State::Playing)
        return;
    if (m_sink)
        m_sink->suspend();
    m_decoder->pause();
    m_tickTimer->stop();
    setState(State::Paused);
}

void AudioPlayer::resume()
{
    if (m_state != State::Paused)
        return;
    m_decoder->resume();
    if (m_sink)
        m_sink->resume();
    m_tickTimer->start(kTickIntervalMs);
    setState(State::Playing);
}

void AudioPlayer::restoreState(const Song &song, qint64 positionMs)
{
    if (!song.isValid())
        return;

    teardownCurrent();
    ++m_generation;
    m_scrobbled = false;

    m_currentSong = song;
    m_positionMs = qMax<qint64>(0, positionMs);
    m_startOffsetMs = m_positionMs;
    m_durationMs = song.duration() > 0 ? static_cast<qint64>(song.duration()) * 1000 : 0;

    setState(State::Stopped);
    Q_EMIT currentSongChanged(m_currentSong);
    Q_EMIT durationChanged(m_durationMs);
    Q_EMIT positionChanged(m_positionMs);
}

void AudioPlayer::togglePlayPause()
{
    if (m_state == State::Playing)
        pause();
    else if (m_state == State::Paused)
        resume();
    else if (m_state == State::Stopped && m_currentSong.isValid() && m_positionMs > 0) {
        // 上次会话恢复的进度：从这里续播
        beginPlayback(m_currentSong, m_positionMs);
    } else if (m_queue && !m_queue->isEmpty()) {
        playQueueAt(m_queue->currentIndex() >= 0 ? m_queue->currentIndex() : 0);
    }
}

void AudioPlayer::playSong(const Song &song)
{
    if (!song.isValid())
        return;

    if (m_queue) {
        const int existing = m_queue->indexOfSong(song.id());
        if (existing >= 0) {
            playQueueAt(existing);
            return;
        }
        m_queue->insertNext(song);
        const int idx = m_queue->indexOfSong(song.id());
        if (idx >= 0) {
            playQueueAt(idx);
            return;
        }
    }

    beginPlayback(song, 0);
}

void AudioPlayer::playQueueAt(int index)
{
    if (!m_queue || index < 0 || index >= m_queue->size())
        return;
    m_queue->setCurrentIndex(index);
    beginPlayback(m_queue->songAt(index), 0);
}

void AudioPlayer::playSongs(const QVector<Song> &songs, int startIndex)
{
    if (!m_queue || songs.isEmpty())
        return;
    m_queue->clear();
    m_queue->append(songs);
    const int idx = qBound(0, startIndex, m_queue->size() - 1);
    playQueueAt(idx);
}

void AudioPlayer::next()
{
    if (!m_queue || m_queue->isEmpty())
        return;
    const int next = m_queue->nextIndex(false);
    if (next < 0)
        stop();
    else
        playQueueAt(next);
}

void AudioPlayer::previous()
{
    if (!m_queue || m_queue->isEmpty())
        return;
    const int prev = m_queue->previousIndex();
    if (prev < 0)
        stop();
    else
        playQueueAt(prev);
}

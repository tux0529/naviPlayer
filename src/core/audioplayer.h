/**
 * @file    audioplayer.h
 * @brief   播放器状态机：ffmpeg 解码 -> 环形缓冲 -> Qt 音频输出，驱动播放队列
 */
#ifndef NAVIPLAYER_AUDIOPLAYER_H
#define NAVIPLAYER_AUDIOPLAYER_H

#include <QObject>
#include <QString>
#include <QTimer>

#include "models/song.h"
#include "core/qt_compat.h"
#include "core/audio_ringbuffer.h"
#include "core/ffmpegaudiodecoder.h"
#include "core/playbackqueue.h"

class CacheManager;
class AppSettings;
class DatabaseManager;

class AudioPlayer : public QObject
{
    Q_OBJECT
public:
    enum class State { Stopped, Playing, Paused, Buffering };
    Q_ENUM(State)

    explicit AudioPlayer(CacheManager *cache, AppSettings *settings, DatabaseManager *db,
                         QObject *parent = nullptr);
    ~AudioPlayer() override;

    void setQueue(PlaybackQueue *queue);
    PlaybackQueue *queue() const { return m_queue; }

    State state() const { return m_state; }
    Song currentSong() const { return m_currentSong; }
    qint64 position() const { return m_positionMs; }
    qint64 duration() const { return m_durationMs; }
    int volume() const { return m_volume; }

    void setVolume(int volume);
    void seek(qint64 positionMs);

    /** 立即播放单曲（已在队列中则跳到该曲，否则插到当前曲之后播放） */
    void playSong(const Song &song);
    /** 播放队列指定项 */
    void playQueueAt(int index);
    /** 用一组歌曲替换队列并从 startIndex 开始播放 */
    void playSongs(const QVector<Song> &songs, int startIndex = 0);

    void next();
    void previous();
    void pause();
    void resume();
    void togglePlayPause();
    void stop();

    /** 恢复上次会话：装载当前曲与进度，但不开始播放 */
    void restoreState(const Song &song, qint64 positionMs);

signals:
    void stateChanged(State state);
    void currentSongChanged(const Song &song);
    void positionChanged(qint64 positionMs);
    void durationChanged(qint64 durationMs);
    void volumeChanged(int volume);
    void bufferProgressChanged(int percent);
    void metadataReady(const Song &song, int bitRate, int sampleRate, int channels);
    void errorOccurred(const QString &message);
    /** 一首歌已播放到应统计的程度（用于向服务器 scrobble 上报） */
    void songPlayed(const Song &song);

private slots:
    void onDecoderOpened(bool ok, qint64 durationMs);
    void onDecoderFinished();
    void onSongCached(const QString &songId, const QString &path);
    void onDownloadFinished(const QString &songId, const QString &path, bool ok);
    void onDownloadProgress(const QString &songId, qint64 received, qint64 total);
    void onTick();
    /** 解码开始后等待环形缓冲预填充，再启动音频设备 */
    void onPrebufferTick();

private:
    void setState(State state);
    void beginPlayback(const Song &song, qint64 startPositionMs = 0);
    void startDecoding(const QString &localPath, qint64 startPositionMs);
    void startPrebuffering();
    void startOutput();
    /** 播放过半或播完时上报一次（每首歌只上报一次） */
    void maybeScrobble(bool completed);
    void teardownCurrent();
    void handleTrackFinished();
    void emitMetadata();

    CacheManager *m_cache = nullptr;
    AppSettings *m_settings = nullptr;
    DatabaseManager *m_db = nullptr;
    PlaybackQueue *m_queue = nullptr;

    FFmpegAudioDecoder *m_decoder = nullptr;
    AudioRingBuffer *m_ring = nullptr;
    NaviAudioOutput *m_sink = nullptr;
    QTimer *m_tickTimer = nullptr;
    QTimer *m_drainTimer = nullptr;
    QTimer *m_prebufferTimer = nullptr;
    int m_prebufferElapsedMs = 0;
    // 播放世代号：切歌/停止时自增，用于忽略上一个解码线程迟到的信号
    int m_generation = 0;
    int m_decodeGeneration = -1;
    bool m_scrobbled = false;

    Song m_currentSong;
    QString m_pendingSongId;
    qint64 m_durationMs = 0;
    qint64 m_positionMs = 0;
    qint64 m_startOffsetMs = 0;
    int m_volume = 80;
    State m_state = State::Stopped;

    int m_bitRate = 0;
    int m_sampleRate = 44100;
    int m_channels = 2;
};

#endif // NAVIPLAYER_AUDIOPLAYER_H

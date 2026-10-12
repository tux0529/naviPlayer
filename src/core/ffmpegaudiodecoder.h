/**
 * @file    ffmpegaudiodecoder.h
 * @brief   ffmpeg 音频解码：后台线程解码 -> 写入 AudioRingBuffer
 *
 * 兼容 ffmpeg 4.4 / 5.1 / 6.1 / 7.1（通过 core/ffmpeg_compat.h）。
 * 输出统一为 44.1kHz / 立体声 / S16，与 QAudioFormat 保持一致。
 */
#ifndef NAVIPLAYER_FFMPEGAUDIODECODER_H
#define NAVIPLAYER_FFMPEGAUDIODECODER_H

#include <QObject>
#include <QPointer>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QString>

#include "core/audio_ringbuffer.h"
#include "core/ffmpeg_compat.h"

/** 解码工作线程 */
class DecodeThread : public QThread
{
    Q_OBJECT
public:
    explicit DecodeThread(QObject *parent = nullptr);
    ~DecodeThread() override;

    void configure(const QString &source, qint64 startPositionMs, AudioRingBuffer *buffer,
                   quint64 token);
    void requestSeek(qint64 positionMs);
    void requestStop();
    void setPaused(bool paused);
    bool isPaused() const;
    /** 取下一个解码任务编号（用于丢弃旧线程迟到的信号） */
    quint64 nextToken();

signals:
    void opened(bool ok, qint64 durationMs, int bitRate, int sampleRate, int channels,
                quint64 token);
    void streamFinished(quint64 token);
    void errorOccurred(const QString &message);
    void embeddedLyricsFound(const QString &lyrics);

protected:
    void run() override;

private:
    bool openInput();
    void closeInput();
    void decodeLoop();
    bool handleSeek();
    void collectEmbeddedLyrics();
    bool writeToBuffer(const uint8_t *data, int size);

    QString m_source;
    qint64 m_startPositionMs = 0;
    // 用 QPointer：环形缓冲被销毁后自动置空，避免野指针
    QPointer<AudioRingBuffer> m_ring;
    quint64 m_token = 0;

    mutable QMutex m_mutex;
    QWaitCondition m_pauseCondition;
    bool m_stopRequested = false;
    bool m_paused = false;
    qint64 m_seekTarget = -1;

    AVFormatContext *m_formatCtx = nullptr;
    AVCodecContext *m_codecCtx = nullptr;
    SwrContext *m_swrCtx = nullptr;
    NaviCodecPtr m_codec = nullptr;
    int m_audioStream = -1;
};

/** 解码器门面：供 AudioPlayer 使用 */
class FFmpegAudioDecoder : public QObject
{
    Q_OBJECT
public:
    explicit FFmpegAudioDecoder(QObject *parent = nullptr);
    ~FFmpegAudioDecoder() override;

    void setRingBuffer(AudioRingBuffer *buffer);
    /** 打开并启动解码（结果为 opened() 信号） */
    void start(const QString &source, qint64 startPositionMs = 0);
    void pause();
    void resume();
    void stop();
    void seek(qint64 positionMs);
    /** 等待解码线程退出（毫秒），避免缓冲区被提前释放 */
    bool waitFinished(int timeoutMs = 1000);

    qint64 duration() const { return m_durationMs; }
    int bitRate() const { return m_bitRate; }
    int sampleRate() const { return m_outputSampleRate; }
    int channels() const { return m_outputChannels; }
    bool isDecoding() const { return m_decoding; }

    static constexpr int kOutputSampleRate = 44100;
    static constexpr int kOutputChannels = 2;

    /** 解析音频文件内嵌歌词（阻塞，应在工作线程调用） */
    static QString extractEmbeddedLyrics(const QString &filePath);

signals:
    void opened(bool ok, qint64 durationMs);
    void finished();
    void errorOccurred(const QString &message);
    void embeddedLyricsFound(const QString &lyrics);

private slots:
    void onThreadOpened(bool ok, qint64 durationMs, int bitRate, int sampleRate, int channels,
                        quint64 token);

private:
    DecodeThread *m_thread = nullptr;
    // 用 QPointer：环形缓冲被销毁后自动置空，避免野指针
    QPointer<AudioRingBuffer> m_ring;
    quint64 m_token = 0;
    qint64 m_durationMs = 0;
    int m_bitRate = 0;
    int m_outputSampleRate = kOutputSampleRate;
    int m_outputChannels = kOutputChannels;
    bool m_decoding = false;
};

#endif // NAVIPLAYER_FFMPEGAUDIODECODER_H

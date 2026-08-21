#ifndef FFMPEGAUDIODECODER_H
#define FFMPEGAUDIODECODER_H

#include <QObject>
#include <QString>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QTimer>
#include "qt_compat.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libswresample/swresample.h>
}

class DecodeThread : public QThread
{
    Q_OBJECT
public:
    DecodeThread(const QString &url, QIODevice *audioDevice, QObject *parent = nullptr,
                 qint64 startPositionMs = 0, bool startPaused = false);
    ~DecodeThread();

    void stop();
    void pause();
    void resume();

signals:
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void finished();
    void errorOccurred(const QString &error);

protected:
    void run() override;

private:
    QString m_url;
    QIODevice *m_audioDevice;
    QMutex m_mutex;
    QWaitCondition m_waitCondition;
    bool m_isStopped;
    bool m_isPaused;
    qint64 m_startPositionMs;
    bool m_startPaused;
};

class FFmpegAudioDecoder : public QObject
{
    Q_OBJECT

public:
    explicit FFmpegAudioDecoder(QObject *parent = nullptr);
    ~FFmpegAudioDecoder();

    bool open(const QString &url);
    void close();
    void play();
    // 以暂停状态从指定位置启动播放，用于恢复播放进度
    void playPaused(const QString &url, qint64 startPositionMs);
    void pause();
    void resume();
    void stop();
    void seek(qint64 positionMs);
    void setVolume(int volume);
    int volume() const;

    bool isPlaying() const;
    bool isPaused() const;
    qint64 position() const;
    qint64 duration() const;

signals:
    void stateChanged(bool playing);
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void errorOccurred(const QString &error);
    void finished();

private slots:
    void updatePlaybackPosition();

private:
    void cleanup();
    void setupAudioOutput();

    QString m_currentUrl;
    DecodeThread *m_decodeThread;

    NaviAudioOutput *m_audioSink;
    QIODevice *m_audioDevice;
    QAudioFormat m_audioFormat;

    bool m_isPlaying;
    bool m_isPaused;
    qint64 m_currentPosition;
    qint64 m_totalDuration;
    int m_volume;
    quint64 m_playbackToken;
    // 恢复播放进度场景下，进度由解码线程 pts 驱动而非 audioSink 进度
    bool m_positionFromPts;

    QTimer *m_positionTimer;
};

#endif // FFMPEGAUDIODECODER_H

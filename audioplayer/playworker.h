#ifndef PLAYWORKER_H
#define PLAYWORKER_H

#include <QObject>
#include <QThread>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QAudioDeviceInfo>

#include "audioplayer.h"

extern "C"{
#include "libavcodec/avcodec.h"
//#include "libavfilter/avfilter.h"
#include "libavformat/avformat.h"
//#include "libavutil/avutil.h"
//#include "libavutil/ffversion.h"
#include "libswresample/swresample.h"
//#include "libswscale/swscale.h"
//#include "libpostproc/postprocess.h"
}



#define DEFAULT_OUT_SAMPLE_RATE 48000
#define DEFAULT_OUT_SAMPLE_SIZE 2
#define DEFAULT_OUT_CHANNEL_COUNT 2


class PlayWorker : public QObject
{
    Q_OBJECT

    enum controlFlag{

        controlNone ,

        controlStop ,

        controlPause,

        controlResume,

        controlPlay,

        controlSeek

    };


public:
    PlayWorker(QAudioOutput *audioOutput = nullptr, QIODevice *audioDevice = nullptr);
    ~PlayWorker();

    void setAudioDevice(QAudioOutput *audioOutput, QIODevice *audioDevice);

public slots:
    void play(const QString &filename = "");
    void stop();
    void pause();
    void seek(qint64 pos = -1);

    void testSlot(){
        m_mediaStatus = AudioPlayer::EndOfMedia;
        emit mediaStatusChanged(AudioPlayer::EndOfMedia);
        emit stateChanged(AudioPlayer::StoppedState);
    }

signals:
    void ERROR(AudioPlayer::Error error, const QString &msg);
    void durationChanged(qint64 duration);
    void positionChanged(qint64 position);

    void mediaStatusChanged(AudioPlayer::MediaStatus status);
    void stateChanged(AudioPlayer::State state);

private:
    void initAudioDevice(int sampleRate, int sampleSize, int channelCount);


    void decodeAudio();
    void freeAVMem();

    AVFormatContext *m_fmtCtx;
    AVCodecContext *m_codecCtx;
    SwrContext *m_swr_ctx;
    AVPacket *m_pkt;
    AVFrame *m_frame;

    uint8_t *m_audio_out_buffer;

    QAudioOutput *m_audioOutput;
    QIODevice *m_audioDevice;

    bool m_audioFormatSupported;

    AudioPlayer::MediaStatus m_mediaStatus;

    QString m_filename;

    volatile controlFlag m_flag;

    volatile int m_pauseStateWaitTimes;

    qint64 m_seekPosition;
    qint64 m_emitPosition;
    qint64 m_currentPosition;

};

#endif // PLAYWORKER_H

#include "audioplayer.h"
#include "playworker.h"
#include "playbackqueue.h"

#ifndef TEST
#include "../common/mediahelper.h"
#endif

#include <QDebug>
#include <QApplication>

AudioPlayer::AudioPlayer(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<AudioPlayer::MediaStatus>("AudioPlayer::MediaStatus");
    qRegisterMetaType<AudioPlayer::State>("AudioPlayer::State");
    qRegisterMetaType<AudioPlayer::State>("AudioPlayer::Error");

    initAudioDevice(DEFAULT_OUT_SAMPLE_RATE, DEFAULT_OUT_SAMPLE_SIZE, DEFAULT_OUT_CHANNEL_COUNT);

    m_worker = new PlayWorker(m_audioOutput, m_audioDevice);

    //m_worker->setAudioDevice(m_audioOutput, m_audioDevice);

    m_playThread = new QThread(this);

    m_worker->moveToThread(m_playThread);

    connect(m_playThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(this, SIGNAL(mediaChanged(const QString &) ), m_worker, SLOT( play(const QString &) ) );
    connect(this, &AudioPlayer::workerPauseSig, m_worker, &PlayWorker::pause );
    connect(this, &AudioPlayer::workerStopSig , m_worker, &PlayWorker::stop );
    connect(this, &AudioPlayer::seekPosition , m_worker, &PlayWorker::seek );

    connect(m_worker, SIGNAL( durationChanged(qint64) ), this, SLOT( onDurationChanged(qint64) ) );
    connect(m_worker, SIGNAL( positionChanged(qint64) ), this, SLOT( onPositionChanged(qint64) ) );
    connect(m_worker, SIGNAL( mediaStatusChanged(AudioPlayer::MediaStatus) ),
            this, SLOT( onMediaStatusChanged(AudioPlayer::MediaStatus) ) );
    connect(m_worker, SIGNAL( stateChanged(AudioPlayer::State) ),
            this, SLOT( onStateChanged(AudioPlayer::State) ) );
    connect(m_worker, SIGNAL( ERROR(AudioPlayer::Error, const QString &) ),
            this, SLOT( onError(AudioPlayer::Error, const QString &) ) );

    m_playThread->start();

    this->setVolume(100);
}

void AudioPlayer::initAudioDevice(int sampleRate, int sampleSize, int channelCount)
{
    QAudioFormat format;
    format.setCodec("audio/pcm");
    format.setSampleRate(sampleRate);
    format.setSampleSize(sampleSize * 8);
    format.setChannelCount(channelCount);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setByteOrder(QAudioFormat::LittleEndian);

    //get the default output audio device
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());

    if (! info.isFormatSupported(format)){
        format = info.nearestFormat(format);
#ifdef G_DEBUG
        foreach(int count,info.supportedChannelCounts())
        {
            qDebug()<<"supportedChannelCounts:"<<count;
        }

        foreach(int count,info.supportedSampleRates())
        {
            qDebug()<<"supportedSampleRates:"<<count;
        }

        foreach(int count,info.supportedSampleSizes())
        {
            qDebug()<<"supportedSampleSizes:"<<count;
        }

#endif
    }
    m_audioOutput = new QAudioOutput(format);
    m_audioOutput->setBufferSize(300000);
    this->setVolume(100);
    m_audioDevice = m_audioOutput->start();

}

AudioPlayer::~AudioPlayer()
{
    stop();
    QThread::msleep( 120 );

    m_audioOutput->stop();
    m_audioDevice->close();

    m_audioOutput->deleteLater();
    m_audioDevice->deleteLater();

    m_worker->deleteLater();

    if(m_playThread){
        m_playThread->quit();
        m_playThread->wait();
        m_playThread->deleteLater();
    }
}

void AudioPlayer::pause()
{
    qDebug() << "AudioPlayer::pause:Thread:" << this->thread();


    emit workerPauseSig();
}

void AudioPlayer::stop()
{
    emit workerStopSig();
}

void AudioPlayer::play(const QString &filename )
{
    QString url = filename;

    if(url.isEmpty() ){
        if(m_state == AudioPlayer::StoppedState){

            const Track &trk = m_playbackQueue->currentTrack();
#ifndef TEST
            url = trk.cacheFilepath().isEmpty()?
                      MediaHelper::Instance()->getSongUrlbyId(trk.id()) : trk.cacheFilepath();
#else
            url = trk.cacheFilepath();
#endif

        }
    }
    qDebug() << "AudioPlayer::play:fimename" << url;
    emit mediaChanged(url);
}

void AudioPlayer::playAudio(int index)
{

    const Track &trk = m_playbackQueue->track(index);
#ifndef TEST
    QString url = trk.cacheFilepath().isEmpty()? MediaHelper::Instance()->getSongUrlbyId(trk.id()) : trk.cacheFilepath();
#else
    QString url = trk.cacheFilepath();
#endif

    qDebug() << "AudioPlayer::playAudio" << url;

    if( m_state == PlayingState){
        m_worker->stop();
    }
    //m_worker->play(url);
    emit mediaChanged( url);
}

void AudioPlayer::onPositionChanged(qint64 position)
{
    this->m_position = position;
    emit positionChanged(position);
}

void AudioPlayer::onDurationChanged(qint64 duration)
{
    this->m_duration = duration;
    emit durationChanged(duration);
}

void AudioPlayer::onMediaStatusChanged(AudioPlayer::MediaStatus status)
{
    m_mediaStatus = status;
    if (status == AudioPlayer::EndOfMedia){
        MediaHelper::Instance()->scrobble(m_playbackQueue->currentTrackId(), true);
        m_playbackQueue->next();
    }
    else if(status == AudioPlayer::LoadedMedia){
        MediaHelper::Instance()->scrobble(m_playbackQueue->currentTrackId(), false);
    }
    emit mediaStatusChanged( status);
}

void AudioPlayer::onStateChanged(State state)
{
    if(state == AudioPlayer::PausedState ){
        m_audioOutput->suspend();
    }
    if(state == AudioPlayer::PlayingState && m_state == AudioPlayer::PausedState){
        m_audioOutput->resume();
    }
    m_state = state;
    emit stateChanged(state);
}

void AudioPlayer::onTrackCountChanged(int count)
{
    Q_UNUSED(count);
    if(m_state == AudioPlayer::StoppedState && m_mediaStatus == AudioPlayer::EndOfMedia){
        m_playbackQueue->next();
    }
}

void AudioPlayer::setVolume(int volume)
{
    m_volume = volume;
    qreal linearVolume = QAudio::convertVolume(volume / qreal(100),
                                               QAudio::LogarithmicVolumeScale,
                                               QAudio::LinearVolumeScale);

    m_audioOutput->setVolume(linearVolume);
}

void AudioPlayer::setMuted(bool muted)
{
    if(muted)
        m_audioOutput->setVolume(0);
    else
        m_audioOutput->setVolume(((float)m_volume/100));
}

void AudioPlayer::setPosition(qint64 position)
{
    emit seekPosition(position);
}

void AudioPlayer::setPlaybackQueue(PlaybackQueue *queue)
{
    this->m_playbackQueue = queue;

    connect(m_playbackQueue, SIGNAL(trackCountChanged(int) ), this, SLOT( onTrackCountChanged(int) ) );
    connect(m_playbackQueue, SIGNAL(currentIndexChanged(int) ), this, SLOT( playAudio(int) ) );
}

void AudioPlayer::onError(Error error, const QString &msg)
{
    m_error = error;
    m_errorString = msg;
//    Config::G_Debug(error, msg);
    emit ERROR(error);
}


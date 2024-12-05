#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include "track.h"
#include <QObject>
#include <QThread>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QAudioDeviceInfo>

class PlayWorker;
class PlaybackQueue;

class AudioPlayer : public QObject
{
    Q_OBJECT

public:
    enum Error {

        NoError = 0 ,
        ResourceError ,      //A media resource couldn't be resolved.
        FormatError,         //The format of a media resource not supported
        NetworkError,        //A network error occurred

        /* There are not the appropriate permissions to play a media resource.
         */
        AccessDeniedError,

        UnknownError
    };

    enum MediaStatus {

        UnknownMediaStatus = 0 ,

        //The is no current media. The player is in the StoppedState.
        NoMedia ,

        //The current media is being loaded. The player may be in any state.
        LoadingMedia,

        //The current media has been loaded. The player is in the StoppedState.
        LoadedMedia,

        /* Playback of the current media has stalled due to insufficient
         * buffering or some other temporary interruption. The player is
         * in the PlayingState or PausedState.
         */
        StalledMedia,

        /* The player is buffering data but has enough data buffered for
         * playback to continue for the immediate future. The player is
         * in the PlayingState or PausedState.
         */
        BufferingMedia,

        /* The player has fully buffered the current media. The player is
         * in the PlayingState or PausedState.
         * */
        BufferedMedia,

        /* Playback has reached the end of the current media.
         * The player is in the StoppedState.
         * */
        EndOfMedia,

        //The current media cannot be played. The player is in the StoppedState.
        InvalidMedia
    };

    enum State {

        /* The media player is not playing content, playback will
         * begin from the start of the current track.
         * */
        StoppedState = 0 ,

        //The media player is currently playing content.
        PlayingState ,

        /* The media player has paused playback, playback of the current track
         * will resume from the position the player was paused at.
         * */
        PausedState
    };

public:
    explicit AudioPlayer(QObject *parent = nullptr);
    ~AudioPlayer();

    qint64 duration() const{ return this->m_duration; }
    AudioPlayer::Error error() const{ return this->m_error; }
    QString errorString() const { return this->m_errorString; }
    bool isMuted() const { return this->m_muted; }
    bool isSeekable() const { return true; }
    AudioPlayer::MediaStatus mediaStatus() const { return this->m_mediaStatus; }
    qint64 position() const {  return this->m_position; }
    AudioPlayer::State state() const { return this->m_state; }
    int volume() const { return this->m_volume; }

    PlaybackQueue *playbackQueue() const { return this->m_playbackQueue; }

public slots:
    void pause();
    void play(const QString &filename = "");
    void playAudio(const Track&);
    void setMuted(bool muted);
    void setPosition(qint64 position);
    void setVolume(int volume);
    void stop();

    void setAudio(Track trk);
    void setPlaybackQueue(PlaybackQueue *queue);

    void onError(AudioPlayer::Error error, const QString &msg);

    void onMediaStatusChanged(AudioPlayer::MediaStatus status);
    void onStateChanged(AudioPlayer::State state);


private slots:
    void onDurationChanged(qint64 duration);
    void onPositionChanged(qint64 position);

    void onTrackCountChanged(int count);

signals:
    void audioAvailableChanged(bool available);
    void bufferStatusChanged(int percentFilled);
    //void currentMediaChanged(const QMediaContent &media);
    void customAudioRoleChanged(const QString &role);
    void durationChanged(qint64 duration);
    void ERROR(AudioPlayer::Error error) ;
    void mediaStatusChanged(AudioPlayer::MediaStatus status);
    void mutedChanged(bool muted);
    void playbackRateChanged(qreal rate);
    void positionChanged(qint64 position);
    void seekableChanged(bool seekable);
    void stateChanged(AudioPlayer::State state);
    void volumeChanged(int volume);

    void mediaChanged(const QString &media);

    void workerPauseSig();
    void workerStopSig();
    void seekPosition(qint64 pos);

private:

    void initAudioDevice(int sampleRate, int sampleSize, int channelCount);

    QThread *m_playThread;

    PlayWorker *m_worker;

    PlaybackQueue *m_playbackQueue;

    QAudioOutput *m_audioOutput;
    QIODevice *m_audioDevice;

    Track m_currentAudio;

    AudioPlayer::Error m_error;
    QString m_errorString;
    AudioPlayer::MediaStatus m_mediaStatus;
    AudioPlayer::State m_state;

    qint64 m_duration;
    qint64 m_position;

    int m_volume;
    bool m_muted;
};

#endif // AUDIOPLAYER_H

#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>
#include <QBuffer>
#include "models/song.h"
#include "ffmpegaudiodecoder.h"

class AudioPlayer : public QObject
{
    Q_OBJECT

public:
    enum State {
        Stopped,
        Playing,
        Paused,
        Buffering
    };

    enum PlayMode {
        Sequential,
        RepeatOne,
        RepeatAll,
        Shuffle
    };

    explicit AudioPlayer(QObject *parent = nullptr);
    ~AudioPlayer();

    void play(const Song &song, const QString &streamUrl);
    void pause();
    void resume();
    void stop();
    void seek(qint64 position);
    void setVolume(int volume);
    int volume() const;

    void setPlayMode(PlayMode mode);
    PlayMode playMode() const;
    void nextPlayMode();

    void playPlaylist(const QVector<Song> &songs, const QStringList &streamUrls, int startIndex = 0);
    void addToPlaylist(const Song &song, const QString &streamUrl);
    void addToNext(const QVector<Song> &songs, const QStringList &streamUrls);
    void removeFromPlaylist(int index);
    void clearPlaylist();
    void playNext();
    void playPrevious();
    void playSongAt(int index);
    void updateSongSource(const QString &songId, const QString &source);

    State state() const { return m_state; }
    qint64 position() const;
    qint64 duration() const;
    Song currentSong() const { return m_currentSong; }
    QVector<Song> playlist() const { return m_playlist; }
    int currentIndex() const { return m_currentIndex; }

signals:
    void stateChanged(State state);
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void bufferProgressChanged(int percent);
    void errorOccurred(const QString &error);
    void playlistChanged(const QVector<Song> &songs);
    void currentIndexChanged(int index);
    void playModeChanged(PlayMode mode);

private slots:
    void onDecoderStateChanged(bool playing);
    void onDecoderPositionChanged(qint64 position);
    void onDecoderDurationChanged(qint64 duration);
    void onDecoderError(const QString &error);
    void onDecoderFinished();

private:
    void setState(State state);
    void generateShuffleIndices();
    bool hasValidCurrentIndex() const;

    FFmpegAudioDecoder *m_decoder;
    State m_state;
    Song m_currentSong;
    QVector<Song> m_playlist;
    QStringList m_streamUrls;
    int m_currentIndex;
    PlayMode m_playMode;
    QVector<int> m_shuffleIndices;
};

#endif // AUDIOPLAYER_H

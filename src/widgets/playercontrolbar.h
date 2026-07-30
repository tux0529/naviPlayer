#ifndef PLAYERCONTROLBAR_H
#define PLAYERCONTROLBAR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QString>
#include <QByteArray>
#include <QToolButton>
#include "../audioplayer.h"
#include "../models/song.h"

class SubsonicApi;
class CacheManager;

class PlayerControlBar : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerControlBar(QWidget *parent = nullptr);

    void setAudioPlayer(AudioPlayer *player);
    void setSubsonicApi(SubsonicApi *api);
    void setCacheManager(CacheManager *cache);
    void setMuted(bool muted, int volumeBeforeMute);
    void setVolume(int volume);
    void setSongFavorite(bool favorite);

public slots:
    void onStateChanged(AudioPlayer::State state);
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onSongChanged(const Song &song);

signals:
    void playClicked();
    void pauseClicked();
    void stopClicked();
    void nextClicked();
    void previousClicked();
    void volumeChanged(int volume);
    void muteChanged(bool muted, int volumeBeforeMute);
    void seekRequested(qint64 position);
    void songFavoriteClicked(const Song &song);

private slots:
    void onPlayPauseClicked();
    void onSliderMoved(int position);
    void onVolumeSliderMoved(int volume);
    void onCoverArtReceived(const QString &coverArtId, const QByteArray &data);
    void onVolumeIconClicked();
    void onSongFavoriteClicked();

private:
    void setupUI();
    QString formatTime(qint64 milliseconds);
    void updateCoverArt(const Song &song);
    void updateVolumeIcon(bool animated);
    void updateSongFavoriteIcon(bool animated);

    AudioPlayer *m_audioPlayer;
    SubsonicApi *m_subsonicApi;
    CacheManager *m_cacheManager;
    Song m_currentSong;
    QString m_currentCoverArtId;

    QLabel *m_coverArtLabel;
    QLabel *m_songTitleLabel;
    QLabel *m_artistAlbumLabel;
    QPushButton *m_previousButton;
    QPushButton *m_playPauseButton;
    QPushButton *m_nextButton;
    QSlider *m_progressSlider;
    QLabel *m_currentTimeLabel;
    QLabel *m_totalTimeLabel;
    QSlider *m_volumeSlider;
    QToolButton *m_volumeButton;
    QToolButton *m_songFavoriteButton;
    bool m_songIsFavorite;
    bool m_isMuted;
    int m_volumeBeforeMute;
};

#endif // PLAYERCONTROLBAR_H

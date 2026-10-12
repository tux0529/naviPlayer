/**
 * @file    controlbar.h
 * @brief   顶部固定播放控制栏：返回、上一首/播放暂停/下一首、正在播放、音量、搜索、歌词、队列
 */
#ifndef NAVIPLAYER_CONTROLBAR_H
#define NAVIPLAYER_CONTROLBAR_H

#include <QWidget>

#include "core/audioplayer.h"
#include "models/song.h"

namespace Ui {
class ControlBar;
}

class QLabel;
class QPushButton;
class QSlider;
class QToolButton;
class VolumePopup;

class ControlBar : public QWidget
{
    Q_OBJECT

public:
    explicit ControlBar(QWidget *parent = nullptr);
    ~ControlBar();

    void setSystemButtonWidget(QWidget *widget);
    QList<QWidget *> getHitTestVisibleWidget();

    void setPlayer(AudioPlayer *player);
    void applyTheme(bool dark);

    bool isDarkTheme() const { return m_dark; }

signals:
    void backRequested();
    void previousRequested();
    void nextRequested();
    void playPauseRequested();
    void seekRequested(qint64 positionMs);
    void volumeChanged(int volume);
    void searchRequested(const QString &text);
    void lyricsToggled();
    void queueToggled();

private slots:
    void onStateChanged(AudioPlayer::State state);
    void onCurrentSongChanged(const Song &song);
    void onPositionChanged(qint64 positionMs);
    void onDurationChanged(qint64 durationMs);
    void onVolumeChanged(int volume);
    void onSliderPressed();
    void onSliderReleased();
    void onVolumeButtonClicked();
    void onCoverReady(const QString &coverArtId, int size, const QPixmap &pixmap);

private:
    void updatePlayButton(AudioPlayer::State state);
    void updateNowPlaying(const Song &song);
    static QString formatTime(qint64 ms);

    Ui::ControlBar *ui;
    AudioPlayer *m_player = nullptr;
    VolumePopup *m_volumePopup = nullptr;
    bool m_sliderPressed = false;
    bool m_dark = true;
    Song m_currentSong;
};

#endif // NAVIPLAYER_CONTROLBAR_H

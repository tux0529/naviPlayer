#include "playercontrolbar.h"
#include "../subsonicapi.h"
#include "../cachemanager.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStyle>
#include <QPixmap>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>

namespace {
constexpr int kPlayerCoverSize = 48;
constexpr int kPlayerCoverRequestSize = 64;

QString playerThumbCacheKey(const QString &coverArtId)
{
    return QStringLiteral("player_thumb_%1").arg(coverArtId);
}
}

PlayerControlBar::PlayerControlBar(QWidget *parent)
    : QWidget(parent)
    , m_audioPlayer(nullptr)
    , m_subsonicApi(nullptr)
    , m_cacheManager(nullptr)
    , m_volumeButton(nullptr)
    , m_songFavoriteButton(nullptr)
    , m_songIsFavorite(false)
    , m_isMuted(false)
    , m_volumeBeforeMute(80)
{
    setupUI();
}

void PlayerControlBar::setupUI()
{
    const int controlBarHeight = 96;
    setFixedHeight(controlBarHeight);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    setStyleSheet(R"(
        PlayerControlBar {
            background-color: #2b2b2b;
            border-top: 1px solid #3d3d3d;
        }
        QLabel {
            color: #ffffff;
        }
        QPushButton {
            background-color: transparent;
            color: #ffffff;
            border: none;
            padding: 8px;
            font-size: 16px;
        }
        QPushButton:hover {
            background-color: #3d3d3d;
            border-radius: 4px;
        }
        QSlider::groove:horizontal {
            background-color: #3d3d3d;
            height: 4px;
            border-radius: 2px;
        }
        QSlider::handle:horizontal {
            background-color: #ffffff;
            width: 12px;
            height: 12px;
            margin: -4px 0;
            border-radius: 6px;
        }
        QSlider::sub-page:horizontal {
            background-color: #1db954;
            border-radius: 2px;
        }
    )");

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(16);

    // Left section: Cover art and song info
    QHBoxLayout *leftLayout = new QHBoxLayout();
    leftLayout->setSpacing(12);

    m_coverArtLabel = new QLabel(this);
    m_coverArtLabel->setFixedSize(kPlayerCoverSize, kPlayerCoverSize);
    m_coverArtLabel->setStyleSheet("background-color: #3d3d3d; border-radius: 4px;");
    m_coverArtLabel->setAlignment(Qt::AlignCenter);
    m_coverArtLabel->setText("🎵");

    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(2);

    m_songTitleLabel = new QLabel("未播放", this);
    m_songTitleLabel->setStyleSheet("font-weight: bold; font-size: 13px;");

    m_artistAlbumLabel = new QLabel("-", this);
    m_artistAlbumLabel->setStyleSheet("color: #999999; font-size: 11px;");

    infoLayout->addWidget(m_songTitleLabel);
    infoLayout->addWidget(m_artistAlbumLabel);

    leftLayout->addWidget(m_coverArtLabel);
    leftLayout->addLayout(infoLayout);

    // Center section: Controls and progress
    QVBoxLayout *centerLayout = new QVBoxLayout();
    centerLayout->setSpacing(8);

    QHBoxLayout *controlsLayout = new QHBoxLayout();
    controlsLayout->setSpacing(8);
    controlsLayout->setAlignment(Qt::AlignCenter);

    m_previousButton = new QPushButton("⏮", this);
    m_playPauseButton = new QPushButton("▶", this);
    m_nextButton = new QPushButton("⏭", this);

    m_previousButton->setFixedSize(32, 32);
    m_playPauseButton->setFixedSize(32, 32);
    m_nextButton->setFixedSize(32, 32);

    controlsLayout->addWidget(m_previousButton);
    controlsLayout->addWidget(m_playPauseButton);
    controlsLayout->addWidget(m_nextButton);

    QHBoxLayout *progressLayout = new QHBoxLayout();
    progressLayout->setSpacing(8);

    m_currentTimeLabel = new QLabel("0:00", this);
    m_currentTimeLabel->setStyleSheet("font-size: 11px; color: #999999;");
    m_currentTimeLabel->setFixedWidth(40);

    m_progressSlider = new QSlider(Qt::Horizontal, this);
    m_progressSlider->setRange(0, 100);
    m_progressSlider->setFixedHeight(16);

    m_totalTimeLabel = new QLabel("0:00", this);
    m_totalTimeLabel->setStyleSheet("font-size: 11px; color: #999999;");
    m_totalTimeLabel->setFixedWidth(40);

    progressLayout->addWidget(m_currentTimeLabel);
    progressLayout->addWidget(m_progressSlider);
    progressLayout->addWidget(m_totalTimeLabel);

    centerLayout->addLayout(controlsLayout);
    centerLayout->addLayout(progressLayout);

    // Right section: Volume
    QHBoxLayout *rightLayout = new QHBoxLayout();
    rightLayout->setSpacing(8);

    m_songFavoriteButton = new QToolButton(this);
    m_songFavoriteButton->setText("♡");
    m_songFavoriteButton->setFixedSize(28, 28);
    m_songFavoriteButton->setToolTip("收藏/取消收藏");
    m_songFavoriteButton->setEnabled(false);
    m_songFavoriteButton->setStyleSheet(R"(
        QToolButton {
            background-color: transparent;
            border: none;
            padding: 4px;
            font-size: 16px;
        }
        QToolButton:hover:enabled {
            background-color: #3d3d3d;
            border-radius: 4px;
        }
        QToolButton:disabled {
            color: #666666;
        }
    )");

    m_volumeButton = new QToolButton(this);
    m_volumeButton->setText("🔊");
    m_volumeButton->setFixedSize(28, 28);
    m_volumeButton->setToolTip("静音/取消静音");
    m_volumeButton->setStyleSheet(R"(
        QToolButton {
            background-color: transparent;
            border: none;
            padding: 4px;
            font-size: 16px;
        }
        QToolButton:hover {
            background-color: #3d3d3d;
            border-radius: 4px;
        }
    )");

    m_volumeSlider = new QSlider(Qt::Horizontal, this);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(80);
    m_volumeSlider->setFixedWidth(100);

    rightLayout->addWidget(m_songFavoriteButton);
    rightLayout->addWidget(m_volumeButton);
    rightLayout->addWidget(m_volumeSlider);

    mainLayout->addLayout(leftLayout, 1);
    mainLayout->addLayout(centerLayout, 2);
    mainLayout->addLayout(rightLayout, 1);

    connect(m_playPauseButton, &QPushButton::clicked,
            this, &PlayerControlBar::onPlayPauseClicked);
    connect(m_previousButton, &QPushButton::clicked,
            this, [this]() { emit previousClicked(); });
    connect(m_nextButton, &QPushButton::clicked,
            this, [this]() { emit nextClicked(); });
    connect(m_progressSlider, &QSlider::sliderMoved,
            this, &PlayerControlBar::onSliderMoved);
    connect(m_volumeSlider, &QSlider::sliderMoved,
            this, &PlayerControlBar::onVolumeSliderMoved);
    connect(m_volumeButton, &QToolButton::clicked,
            this, &PlayerControlBar::onVolumeIconClicked);
    connect(m_songFavoriteButton, &QToolButton::clicked,
            this, &PlayerControlBar::onSongFavoriteClicked);
}

void PlayerControlBar::setAudioPlayer(AudioPlayer *player)
{
    m_audioPlayer = player;
    if (!m_audioPlayer) {
        return;
    }

    if (!m_isMuted) {
        m_volumeSlider->setValue(m_audioPlayer->volume());
    }
    updateVolumeIcon(false);
}

void PlayerControlBar::setMuted(bool muted, int volumeBeforeMute)
{
    m_isMuted = muted;
    if (volumeBeforeMute > 0) {
        m_volumeBeforeMute = volumeBeforeMute;
    }
    updateVolumeIcon(false);
}

void PlayerControlBar::setVolume(int volume)
{
    m_volumeSlider->setValue(qBound(0, volume, 100));
    if (!m_isMuted) {
        updateVolumeIcon(false);
    }
}

void PlayerControlBar::setSongFavorite(bool favorite)
{
    m_songIsFavorite = favorite;
    updateSongFavoriteIcon(false);
}

void PlayerControlBar::setSubsonicApi(SubsonicApi *api)
{
    if (m_subsonicApi) {
        disconnect(m_subsonicApi, &SubsonicApi::coverArtReceived,
                   this, &PlayerControlBar::onCoverArtReceived);
    }

    m_subsonicApi = api;
    if (m_subsonicApi) {
        connect(m_subsonicApi, &SubsonicApi::coverArtReceived,
                this, &PlayerControlBar::onCoverArtReceived);
    }
}

void PlayerControlBar::setCacheManager(CacheManager *cache)
{
    m_cacheManager = cache;
}

void PlayerControlBar::onStateChanged(AudioPlayer::State state)
{
    switch (state) {
    case AudioPlayer::Playing:
        m_playPauseButton->setText("⏸");
        break;
    case AudioPlayer::Paused:
        m_playPauseButton->setText("▶");
        break;
    case AudioPlayer::Stopped:
        m_playPauseButton->setText("▶");
        break;
    default:
        break;
    }
}

void PlayerControlBar::onPositionChanged(qint64 position)
{
    m_currentTimeLabel->setText(formatTime(position));
    if (!m_progressSlider->isSliderDown()) {
        m_progressSlider->setValue(static_cast<int>(position));
    }
}

void PlayerControlBar::onDurationChanged(qint64 duration)
{
    m_totalTimeLabel->setText(formatTime(duration));
    m_progressSlider->setMaximum(static_cast<int>(duration));
}

void PlayerControlBar::onSongChanged(const Song &song)
{
    m_currentSong = song;
    if (song.id().isEmpty()) {
        m_songTitleLabel->setText("未播放");
        m_artistAlbumLabel->setText("-");
        m_currentCoverArtId.clear();
        m_coverArtLabel->setPixmap(QPixmap());
        m_coverArtLabel->setText("🎵");
        m_songFavoriteButton->setEnabled(false);
        m_songIsFavorite = false;
        updateSongFavoriteIcon(false);
        return;
    }

    m_songTitleLabel->setText(song.title());
    m_artistAlbumLabel->setText(song.artist() + " - " + song.album());
    m_songFavoriteButton->setEnabled(true);
    m_songIsFavorite = song.isFavorite();
    updateSongFavoriteIcon(false);
    updateCoverArt(song);
}

void PlayerControlBar::updateCoverArt(const Song &song)
{
    m_currentCoverArtId = song.coverArtId();
    m_coverArtLabel->setPixmap(QPixmap());
    m_coverArtLabel->setText("🎵");

    if (m_currentCoverArtId.isEmpty()) {
        return;
    }

    if (m_cacheManager) {
        QPixmap cached = m_cacheManager->getCoverArt(playerThumbCacheKey(m_currentCoverArtId));
        if (!cached.isNull()) {
            m_coverArtLabel->setText(QString());
            m_coverArtLabel->setPixmap(cached);
            return;
        }
    }

    if (m_subsonicApi) {
        m_subsonicApi->getCoverArt(m_currentCoverArtId, kPlayerCoverRequestSize);
    }
}

void PlayerControlBar::onCoverArtReceived(const QString &coverArtId, const QByteArray &data)
{
    if (coverArtId.isEmpty() || coverArtId != m_currentCoverArtId) {
        return;
    }

    QPixmap pixmap;
    if (!pixmap.loadFromData(data)) {
        return;
    }

    QPixmap scaled = pixmap.scaled(m_coverArtLabel->size(),
                                   Qt::KeepAspectRatioByExpanding,
                                   Qt::SmoothTransformation);

    m_coverArtLabel->setText(QString());
    m_coverArtLabel->setPixmap(scaled);

    if (m_cacheManager) {
        m_cacheManager->putCoverArt(playerThumbCacheKey(coverArtId), scaled);
    }
}

void PlayerControlBar::onPlayPauseClicked()
{
    if (m_audioPlayer) {
        if (m_audioPlayer->state() == AudioPlayer::Playing) {
            m_audioPlayer->pause();
            emit pauseClicked();
        } else {
            m_audioPlayer->resume();
            emit playClicked();
        }
    }
}

void PlayerControlBar::onSliderMoved(int position)
{
    if (m_audioPlayer) {
        emit seekRequested(position);
    }
}

void PlayerControlBar::onVolumeSliderMoved(int volume)
{
    if (!m_audioPlayer) {
        return;
    }

    if (m_isMuted && volume > 0) {
        m_isMuted = false;
        m_volumeBeforeMute = volume;
        emit muteChanged(false, m_volumeBeforeMute);
        updateVolumeIcon(true);
    }

    if (!m_isMuted) {
        m_volumeBeforeMute = volume;
    }

    m_audioPlayer->setVolume(m_isMuted ? 0 : volume);
    emit volumeChanged(volume);
    updateVolumeIcon(false);
}

QString PlayerControlBar::formatTime(qint64 milliseconds)
{
    qint64 seconds = milliseconds / 1000;
    qint64 minutes = seconds / 60;
    seconds = seconds % 60;
    return QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
}

void PlayerControlBar::onVolumeIconClicked()
{
    if (!m_audioPlayer) {
        return;
    }

    if (!m_isMuted) {
        const int current = m_volumeSlider->value();
        if (current > 0) {
            m_volumeBeforeMute = current;
        }
        m_isMuted = true;
        m_audioPlayer->setVolume(0);
        emit muteChanged(true, m_volumeBeforeMute);
        updateVolumeIcon(true);
        return;
    }

    m_isMuted = false;
    int restore = m_volumeBeforeMute;
    if (restore <= 0) {
        restore = 80;
        m_volumeBeforeMute = restore;
    }

    m_volumeSlider->setValue(restore);
    m_audioPlayer->setVolume(restore);
    emit volumeChanged(restore);
    emit muteChanged(false, m_volumeBeforeMute);
    updateVolumeIcon(true);
}

void PlayerControlBar::updateVolumeIcon(bool animated)
{
    if (!m_volumeButton) {
        return;
    }

    const QString target = m_isMuted ? QStringLiteral("🔇") : QStringLiteral("🔊");
    if (!animated || !m_volumeButton->graphicsEffect()) {
        m_volumeButton->setText(target);
        if (!m_volumeButton->graphicsEffect()) {
            auto *effect = new QGraphicsOpacityEffect(m_volumeButton);
            effect->setOpacity(1.0);
            m_volumeButton->setGraphicsEffect(effect);
        }
        return;
    }

    auto *effect = qobject_cast<QGraphicsOpacityEffect *>(m_volumeButton->graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(m_volumeButton);
        effect->setOpacity(1.0);
        m_volumeButton->setGraphicsEffect(effect);
    }

    QPropertyAnimation *fadeOut = new QPropertyAnimation(effect, "opacity", m_volumeButton);
    fadeOut->setDuration(150);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::OutCubic);

    connect(fadeOut, &QPropertyAnimation::finished, this, [this, effect, target]() {
        m_volumeButton->setText(target);
        QPropertyAnimation *fadeIn = new QPropertyAnimation(effect, "opacity", m_volumeButton);
        fadeIn->setDuration(150);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->setEasingCurve(QEasingCurve::OutCubic);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    });

    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
}

void PlayerControlBar::onSongFavoriteClicked()
{
    if (m_currentSong.id().isEmpty()) {
        return;
    }
    emit songFavoriteClicked(m_currentSong);
}

void PlayerControlBar::updateSongFavoriteIcon(bool animated)
{
    if (!m_songFavoriteButton) {
        return;
    }

    const QString target = m_songIsFavorite ? QStringLiteral("♥") : QStringLiteral("♡");
    if (!animated || !m_songFavoriteButton->graphicsEffect()) {
        m_songFavoriteButton->setText(target);
        if (!m_songFavoriteButton->graphicsEffect()) {
            auto *effect = new QGraphicsOpacityEffect(m_songFavoriteButton);
            effect->setOpacity(1.0);
            m_songFavoriteButton->setGraphicsEffect(effect);
        }
        return;
    }

    auto *effect = qobject_cast<QGraphicsOpacityEffect *>(m_songFavoriteButton->graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(m_songFavoriteButton);
        effect->setOpacity(1.0);
        m_songFavoriteButton->setGraphicsEffect(effect);
    }

    QPropertyAnimation *fadeOut = new QPropertyAnimation(effect, "opacity", m_songFavoriteButton);
    fadeOut->setDuration(150);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::OutCubic);

    connect(fadeOut, &QPropertyAnimation::finished, this, [this, effect, target]() {
        m_songFavoriteButton->setText(target);
        QPropertyAnimation *fadeIn = new QPropertyAnimation(effect, "opacity", m_songFavoriteButton);
        fadeIn->setDuration(150);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->setEasingCurve(QEasingCurve::OutCubic);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    });

    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
}

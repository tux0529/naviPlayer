#include "controlbar.h"
#include "ui_controlbar.h"

#include "core/subsonicapi.h"
#include "core/cachemanager.h"
#include "core/appcontext.h"
#include "widgets/components/volumepopup.h"

#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QSlider>
#include <QToolButton>
#include <QStyle>

namespace {
constexpr int kPlayerCoverSize = 42;
constexpr int kPlayerCoverRequestSize = 128;

QString playerThumbCacheKey(const QString &coverArtId)
{
    return QStringLiteral("player_thumb_%1").arg(coverArtId);
}

/** 正在播放的缩略图用专辑封面（同一专辑只取一张图） */
QString playerCoverId(const Song &song)
{
    AppContext *ctx = AppContext::instance();
    return ctx ? ctx->coverArtIdForSong(song) : song.coverArtId();
}

/** 只给左上、左下两个角倒角（右上角、右下角保持直角） */
QPainterPath leftRoundedPath(const QRectF &rect, qreal radius)
{
    const qreal r = qMin(radius, qMin(rect.width(), rect.height()) / 2.0);
    const qreal x = rect.left();
    const qreal y = rect.top();
    const qreal w = rect.width();
    const qreal h = rect.height();

    QPainterPath path;
    path.moveTo(x + r, y);
    path.lineTo(x + w, y);                                  // 上边 → 右上角（直角）
    path.lineTo(x + w, y + h);                              // 右边 → 右下角（直角）
    path.lineTo(x + r, y + h);                              // 下边
    path.arcTo(QRectF(x, y + h - 2 * r, 2 * r, 2 * r), 270, -90);   // 左下角
    path.lineTo(x, y + r);
    path.arcTo(QRectF(x, y, 2 * r, 2 * r), 180, -90);               // 左上角
    path.closeSubpath();
    return path;
}

QPixmap roundedPixmap(const QPixmap &source, int size, int radius)
{
    if (source.isNull())
        return source;

    QPixmap scaled = source.scaled(size, size, Qt::KeepAspectRatioByExpanding,
                                   Qt::SmoothTransformation);
    const QPoint offset((scaled.width() - size) / 2, (scaled.height() - size) / 2);
    scaled = scaled.copy(offset.x(), offset.y(), size, size);

    QPixmap target(size, size);
    target.fill(Qt::transparent);
    QPainter painter(&target);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setClipPath(leftRoundedPath(QRectF(0, 0, size, size), radius));
    painter.drawPixmap(0, 0, scaled);
    return target;
}
}

ControlBar::ControlBar(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ControlBar)
{
    ui->setupUi(this);
    setFixedHeight(52);
    setAttribute(Qt::WA_StyledBackground, true);

    ui->nowplayingCoverLabel->setFixedSize(kPlayerCoverSize, kPlayerCoverSize);
    ui->playbackProgressSlider->setRange(0, 0);
    ui->searchEdit->setPlaceholderText(QStringLiteral("搜索歌曲、专辑、艺术家"));

    connect(ui->prevPageButton, &QToolButton::clicked, this, &ControlBar::backRequested);
    connect(ui->prevSongButton, &QToolButton::clicked, this, &ControlBar::previousRequested);
    connect(ui->playButton, &QToolButton::clicked, this, &ControlBar::playPauseRequested);
    connect(ui->nextSongButton, &QToolButton::clicked, this, &ControlBar::nextRequested);
    connect(ui->lyricsButton, &QToolButton::clicked, this, &ControlBar::lyricsToggled);
    connect(ui->playbackButton, &QToolButton::clicked, this, &ControlBar::queueToggled);
    connect(ui->volumeButton, &QToolButton::clicked, this, &ControlBar::onVolumeButtonClicked);
    connect(ui->searchEdit, &QLineEdit::returnPressed, this, [this]() {
        Q_EMIT searchRequested(ui->searchEdit->text().trimmed());
    });

    connect(ui->playbackProgressSlider, &QSlider::sliderPressed, this, &ControlBar::onSliderPressed);
    connect(ui->playbackProgressSlider, &QSlider::sliderReleased, this, &ControlBar::onSliderReleased);

    m_volumePopup = new VolumePopup(this);
    connect(m_volumePopup, &VolumePopup::volumeChanged, this, &ControlBar::volumeChanged);

    applyTheme(true);

    AppContext *ctx = AppContext::instance();
    if (ctx && ctx->cache()) {
        connect(ctx->cache(), &CacheManager::coverReady, this, &ControlBar::onCoverReady);
    }
}

ControlBar::~ControlBar()
{
    delete ui;
}

void ControlBar::setSystemButtonWidget(QWidget *widget)
{
    ui->winbuttonSpacer->changeSize(16, 20, QSizePolicy::Fixed, QSizePolicy::Preferred);
    ui->controlBarHLayout->addWidget(widget);
}

QList<QWidget *> ControlBar::getHitTestVisibleWidget()
{
    return ui->playerControlWidget->findChildren<QWidget *>();
}

void ControlBar::setPlayer(AudioPlayer *player)
{
    if (m_player) {
        disconnect(m_player, nullptr, this, nullptr);
    }
    m_player = player;
    if (!m_player)
        return;

    connect(m_player, &AudioPlayer::stateChanged, this, &ControlBar::onStateChanged);
    connect(m_player, &AudioPlayer::currentSongChanged, this, &ControlBar::onCurrentSongChanged);
    connect(m_player, &AudioPlayer::positionChanged, this, &ControlBar::onPositionChanged);
    connect(m_player, &AudioPlayer::durationChanged, this, &ControlBar::onDurationChanged);
    connect(m_player, &AudioPlayer::volumeChanged, this, &ControlBar::onVolumeChanged);

    onStateChanged(m_player->state());
    onCurrentSongChanged(m_player->currentSong());
    m_volumePopup->setVolume(m_player->volume());
}

void ControlBar::applyTheme(bool dark)
{
    m_dark = dark;
    const QString theme = dark ? QStringLiteral("dark") : QStringLiteral("light");

    auto setIcon = [theme](QToolButton *button, const QString &name) {
        const QString path = QStringLiteral(":/theme/%1/%2.svg").arg(theme, name);
        QIcon icon;
        if (QFile::exists(path))
            icon.addFile(path);
        button->setIcon(icon);
    };

    setIcon(ui->prevPageButton, QStringLiteral("ctrl_prevPageButton"));
    setIcon(ui->prevSongButton, QStringLiteral("ctrl_prevSongButton"));
    setIcon(ui->nextSongButton, QStringLiteral("ctrl_nextSong"));
    setIcon(ui->volumeButton, QStringLiteral("ctrl_volume"));
    setIcon(ui->lyricsButton, QStringLiteral("ctrl_lyrics"));
    setIcon(ui->playbackButton, QStringLiteral("ctrl_playbackqueue"));
    updatePlayButton(m_player ? m_player->state() : AudioPlayer::State::Stopped);
}

void ControlBar::updatePlayButton(AudioPlayer::State state)
{
    const QString theme = m_dark ? QStringLiteral("dark") : QStringLiteral("light");
    const QString name = state == AudioPlayer::State::Playing
                             ? QStringLiteral("ctrl_pauseButton")
                             : QStringLiteral("ctrl_playButton");
    const QString path = QStringLiteral(":/theme/%1/%2.svg").arg(theme, name);
    QIcon icon;
    if (QFile::exists(path))
        icon.addFile(path);
    else
        icon.addFile(QStringLiteral(":/theme/dark/%1.svg").arg(name));
    ui->playButton->setIcon(icon);

    const QString tooltip = state == AudioPlayer::State::Playing ? QStringLiteral("暂停")
                                                                 : QStringLiteral("播放");
    ui->playButton->setToolTip(tooltip);
}

void ControlBar::onStateChanged(AudioPlayer::State state)
{
    updatePlayButton(state);

    QString status;
    switch (state) {
    case AudioPlayer::State::Playing:   status = QStringLiteral("正在播放"); break;
    case AudioPlayer::State::Paused:    status = QStringLiteral("已暂停"); break;
    case AudioPlayer::State::Buffering: status = QStringLiteral("缓冲中…"); break;
    case AudioPlayer::State::Stopped:   status = QStringLiteral("已停止"); break;
    }
    if (!m_currentSong.isValid())
        ui->nowplayingAlbumLabel->setText(status);
}

QString ControlBar::formatTime(qint64 ms)
{
    if (ms <= 0)
        return QStringLiteral("00:00");
    const qint64 total = ms / 1000;
    const int minutes = static_cast<int>(total / 60);
    const int seconds = static_cast<int>(total % 60);
    return QStringLiteral("%1:%2").arg(minutes, 2, 10, QLatin1Char('0'))
                                  .arg(seconds, 2, 10, QLatin1Char('0'));
}

void ControlBar::updateNowPlaying(const Song &song)
{
    if (!song.isValid()) {
        ui->nowplayingTitleLabel->setText(QStringLiteral("未在播放"));
        ui->nowplayingAlbumLabel->setText(QString());
        ui->nowplayingCoverLabel->setPixmap(QPixmap());
        return;
    }

    ui->nowplayingTitleLabel->setText(song.title());
    ui->nowplayingAlbumLabel->setText(QStringLiteral("%1 · %2").arg(song.artist(), song.album()));
    ui->nowplayingTitleLabel->setToolTip(song.title());

    // 封面：优先本地缓存（用专辑封面，缺失时回退单曲封面）
    CacheManager *cache = AppContext::instance() ? AppContext::instance()->cache() : nullptr;
    const QString coverId = playerCoverId(song);
    if (cache && !coverId.isEmpty()) {
        QPixmap pm = cache->cachedCover(coverId, kPlayerCoverRequestSize);
        if (!pm.isNull()) {
            ui->nowplayingCoverLabel->setPixmap(roundedPixmap(pm, kPlayerCoverSize, 4));
        } else {
            cache->requestCover(coverId, kPlayerCoverRequestSize);
            ui->nowplayingCoverLabel->setPixmap(QPixmap());
        }
    }
}

void ControlBar::onCoverReady(const QString &coverArtId, int size, const QPixmap &pixmap)
{
    Q_UNUSED(size)
    if (!m_currentSong.isValid() || coverArtId != playerCoverId(m_currentSong))
        return;
    ui->nowplayingCoverLabel->setPixmap(roundedPixmap(pixmap, kPlayerCoverSize, 4));
}

void ControlBar::onCurrentSongChanged(const Song &song)
{
    m_currentSong = song;
    updateNowPlaying(song);
    ui->playbackProgressSlider->setValue(0);
}

void ControlBar::onDurationChanged(qint64 durationMs)
{
    ui->playbackProgressSlider->setRange(0, static_cast<int>(durationMs));
}

void ControlBar::onPositionChanged(qint64 positionMs)
{
    if (m_sliderPressed)
        return;
    ui->playbackProgressSlider->blockSignals(true);
    ui->playbackProgressSlider->setValue(static_cast<int>(positionMs));
    ui->playbackProgressSlider->blockSignals(false);

    const QString total = formatTime(ui->playbackProgressSlider->maximum());
    ui->nowplayingAlbumLabel->setToolTip(QStringLiteral("%1 / %2").arg(formatTime(positionMs), total));
}

void ControlBar::onVolumeChanged(int volume)
{
    m_volumePopup->setVolume(volume);
}

void ControlBar::onSliderPressed()
{
    m_sliderPressed = true;
}

void ControlBar::onSliderReleased()
{
    m_sliderPressed = false;
    Q_EMIT seekRequested(ui->playbackProgressSlider->value());
}

void ControlBar::onVolumeButtonClicked()
{
    if (m_volumePopup->isVisible()) {
        m_volumePopup->close();
        return;
    }
    const QPoint pos = ui->volumeButton->mapToGlobal(
        QPoint(0, ui->volumeButton->height() + 4));
    m_volumePopup->move(pos);
    m_volumePopup->show();
}

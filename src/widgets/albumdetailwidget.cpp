#include "albumdetailwidget.h"
#include "songlistwidget.h"
#include "../subsonicapi.h"
#include "../cachemanager.h"
#include <QPixmap>
#include <QScrollArea>
#include <QSpacerItem>
#include <QDebug>
#include <QStyle>
#include <QPainter>
#include <QPainterPath>

namespace {
QPixmap roundedPixmap(const QPixmap &pixmap, const QSize &targetSize, qreal radius)
{
    if (pixmap.isNull() || !targetSize.isValid()) {
        return QPixmap();
    }

    QPixmap scaled = pixmap.scaled(targetSize,
                                   Qt::KeepAspectRatioByExpanding,
                                   Qt::SmoothTransformation);
    QPixmap rounded(targetSize);
    rounded.fill(Qt::transparent);

    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPainterPath path;
    path.addRoundedRect(QRectF(QPointF(0, 0), QSizeF(targetSize)), radius, radius);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, scaled);

    return rounded;
}
}

AlbumDetailWidget::AlbumDetailWidget(QWidget *parent)
    : QWidget(parent)
    , m_subsonicApi(nullptr)
    , m_cacheManager(nullptr)
    , m_albumIsFavorite(false)
    , m_albumFavoriteButton(nullptr)
{
    setupUI();
}

void AlbumDetailWidget::setSubsonicApi(SubsonicApi *api)
{
    m_subsonicApi = api;
    connect(m_subsonicApi, &SubsonicApi::coverArtReceived,
            this, &AlbumDetailWidget::onCoverArtReceived);
}

void AlbumDetailWidget::setCacheManager(CacheManager *cache)
{
    m_cacheManager = cache;
}

void AlbumDetailWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // Back button
    m_backButton = new QPushButton("← 返回", this);
    m_backButton->setStyleSheet(R"(
        QPushButton {
            background-color: #3a3a3a;
            color: #ffffff;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #4a4a4a;
        }
    )");
    connect(m_backButton, &QPushButton::clicked,
            this, &AlbumDetailWidget::onBackClicked);
    mainLayout->addWidget(m_backButton);

    // Album info container
    QWidget *infoWidget = new QWidget(this);
    QHBoxLayout *infoLayout = new QHBoxLayout(infoWidget);
    infoLayout->setSpacing(20);

    // Album cover
    m_coverLabel = new QLabel(this);
    m_coverLabel->setFixedSize(256, 256);
    m_coverLabel->setStyleSheet(R"(
        background-color: #3a3a3a;
        border-radius: 8px;
    )");
    m_coverLabel->setAlignment(Qt::AlignCenter);
    m_coverLabel->setText("📀");
    infoLayout->addWidget(m_coverLabel);

    // Album details
    QWidget *detailsWidget = new QWidget(this);
    QVBoxLayout *detailsLayout = new QVBoxLayout(detailsWidget);
    detailsLayout->setSpacing(12);

    m_titleLabel = new QLabel("专辑标题", this);
    m_titleLabel->setStyleSheet("color: #ffffff; font-size: 28px; font-weight: bold;");
    m_titleLabel->setWordWrap(true);

    m_artistLabel = new QLabel("艺术家", this);
    m_artistLabel->setStyleSheet("color: #cccccc; font-size: 18px;");

    m_infoLabel = new QLabel("信息", this);
    m_infoLabel->setStyleSheet("color: #aaaaaa; font-size: 14px;");

    m_playButton = new QPushButton("▶ 播放专辑", this);
    m_playButton->setStyleSheet(R"(
        QPushButton {
            background-color: #1db954;
            color: #ffffff;
            border: none;
            padding: 12px 32px;
            border-radius: 24px;
            font-size: 16px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #1ed760;
        }
    )");
    connect(m_playButton, &QPushButton::clicked,
            this, &AlbumDetailWidget::onPlayClicked);

    m_addToPlaylistButton = new QPushButton(this);
    m_addToPlaylistButton->setToolTip("添加到播放队列");
    m_addToPlaylistButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    m_addToPlaylistButton->setIconSize(QSize(18, 18));
    m_addToPlaylistButton->setFixedSize(44, 44);
    m_addToPlaylistButton->setStyleSheet(R"(
        QPushButton {
            background-color: #3a3a3a;
            color: #ffffff;
            border: none;
            border-radius: 22px;
        }
        QPushButton:hover {
            background-color: #4a4a4a;
        }
    )");
    connect(m_addToPlaylistButton, &QPushButton::clicked,
            this, &AlbumDetailWidget::onAddToPlaylistClicked);

    m_addToNextButton = new QPushButton(this);
    m_addToNextButton->setToolTip("添加到下一首播放");
    m_addToNextButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    m_addToNextButton->setIconSize(QSize(18, 18));
    m_addToNextButton->setFixedSize(44, 44);
    m_addToNextButton->setStyleSheet(R"(
        QPushButton {
            background-color: #3a3a3a;
            color: #ffffff;
            border: none;
            border-radius: 22px;
        }
        QPushButton:hover {
            background-color: #4a4a4a;
        }
    )");
    connect(m_addToNextButton, &QPushButton::clicked,
            this, &AlbumDetailWidget::onAddToNextClicked);

    m_albumFavoriteButton = new QToolButton(this);
    m_albumFavoriteButton->setFixedSize(44, 44);
    m_albumFavoriteButton->setToolTip("收藏/取消收藏专辑");
    m_albumFavoriteButton->setStyleSheet(R"(
        QToolButton {
            background-color: #3a3a3a;
            color: #ffffff;
            border: none;
            border-radius: 22px;
            font-size: 18px;
            font-weight: bold;
        }
        QToolButton:hover:enabled {
            background-color: #4a4a4a;
        }
        QToolButton:disabled {
            background-color: #2b2b2b;
            color: #666666;
        }
    )");
    m_albumFavoriteButton->setEnabled(false);
    connect(m_albumFavoriteButton, &QToolButton::clicked, this, [this]() {
        if (!m_currentAlbum.id().isEmpty()) {
            emit albumFavoriteClicked(m_currentAlbum);
        }
    });

    QHBoxLayout *actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(12);
    actionLayout->addWidget(m_playButton);
    actionLayout->addWidget(m_albumFavoriteButton);
    actionLayout->addWidget(m_addToPlaylistButton);
    actionLayout->addWidget(m_addToNextButton);
    actionLayout->addStretch();

    detailsLayout->addWidget(m_titleLabel);
    detailsLayout->addWidget(m_artistLabel);
    detailsLayout->addWidget(m_infoLabel);
    detailsLayout->addLayout(actionLayout);
    detailsLayout->addStretch();

    infoLayout->addWidget(detailsWidget);
    infoLayout->addStretch();

    mainLayout->addWidget(infoWidget);

    // Song list
    m_songListWidget = new SongListWidget(this);
    connect(m_songListWidget, &SongListWidget::songSelected,
            this, &AlbumDetailWidget::songSelected);
    connect(m_songListWidget, &SongListWidget::songFavoriteClicked,
            this, &AlbumDetailWidget::songFavoriteClicked);
    mainLayout->addWidget(m_songListWidget, 1);
}

void AlbumDetailWidget::setAlbum(const Album &album, const QVector<Song> &songs)
{
    m_currentAlbum = album;
    m_songs = songs;
    bool hasSongFavorite = false;
    for (const Song &song : songs) {
        if (song.isFavorite()) {
            hasSongFavorite = true;
            break;
        }
    }
    setAlbumFavorite(album.isFavorite() || hasSongFavorite);
    m_albumFavoriteButton->setEnabled(!album.id().isEmpty());

    // Update UI
    m_titleLabel->setText(album.name());
    m_artistLabel->setText(album.artist());
    m_infoLabel->setText(QString("%1 首歌曲").arg(songs.size()));
    m_songListWidget->setSongs(songs);

    // Load album cover
    loadAlbumCover();
}

void AlbumDetailWidget::clear()
{
    m_currentAlbum = Album();
    m_songs.clear();
    m_titleLabel->setText("专辑标题");
    m_artistLabel->setText("艺术家");
    m_infoLabel->setText("信息");
    m_coverLabel->setText("📀");
    setAlbumFavorite(false);
    m_albumFavoriteButton->setEnabled(false);
    m_songListWidget->clear();
}

void AlbumDetailWidget::setAlbumFavorite(bool favorite)
{
    m_albumIsFavorite = favorite;
    if (m_albumFavoriteButton) {
        m_albumFavoriteButton->setText(m_albumIsFavorite ? "♥" : "♡");
    }
}

void AlbumDetailWidget::loadAlbumCover()
{
    QString coverArtId = m_currentAlbum.coverArtId();
    if (coverArtId.isEmpty()) {
        return;
    }

    // Check cache first
    if (m_cacheManager) {
        QPixmap cached = m_cacheManager->getCoverArt(coverArtId);
        if (!cached.isNull()) {
            m_coverLabel->setPixmap(roundedPixmap(cached, m_coverLabel->size(), 8.0));
            return;
        }
    }

    // Request from API
    if (m_subsonicApi) {
        m_subsonicApi->getCoverArt(coverArtId);
    }
}

void AlbumDetailWidget::onCoverArtReceived(const QString &coverArtId, const QByteArray &data)
{
    if (coverArtId != m_currentAlbum.coverArtId()) {
        return;
    }

    QPixmap pixmap;
    if (pixmap.loadFromData(data)) {
        QPixmap scaled = roundedPixmap(pixmap, m_coverLabel->size(), 8.0);
        m_coverLabel->setPixmap(scaled);

        if (m_cacheManager) {
            m_cacheManager->putCoverArt(coverArtId, scaled);
        }
    }
}

void AlbumDetailWidget::onPlayClicked()
{
    if (!m_songs.isEmpty()) {
        emit playAlbum(m_songs);
    }
}

void AlbumDetailWidget::onAddToPlaylistClicked()
{
    if (!m_songs.isEmpty()) {
        emit addSongsToPlaylist(m_songs);
    }
}

void AlbumDetailWidget::onAddToNextClicked()
{
    if (!m_songs.isEmpty()) {
        emit addAlbumToNext(m_songs);
    }
}

void AlbumDetailWidget::onBackClicked()
{
    emit backRequested();
}

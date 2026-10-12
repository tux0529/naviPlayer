#include "albumdetailpage.h"

#include "widgets/components/coverlabel.h"
#include "widgets/pages/songslistview.h"
#include "core/appcontext.h"
#include "core/subsonicapi.h"
#include "core/databasemanager.h"

#include <algorithm>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

AlbumDetailPage::AlbumDetailPage(QWidget *parent)
    : PageBase(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(10);

    // ---- 头部 ----
    auto *header = new QHBoxLayout();
    header->setSpacing(16);

    m_cover = new CoverLabel(this);
    m_cover->setFixedSize(160, 160);
    m_cover->setRadius(8);

    auto *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(6);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName(QStringLiteral("albumDetailTitle"));
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(titleFont.pointSize() + 8);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setWordWrap(true);

    m_artistButton = new QPushButton(this);
    m_artistButton->setObjectName(QStringLiteral("albumDetailArtist"));
    m_artistButton->setFlat(true);
    m_artistButton->setCursor(Qt::PointingHandCursor);

    m_metaLabel = new QLabel(this);
    m_metaLabel->setObjectName(QStringLiteral("albumDetailMeta"));

    auto *buttonLayout = new QHBoxLayout();
    m_addQueueButton = new QPushButton(QStringLiteral("添加到队列"), this);
    m_addNextButton = new QPushButton(QStringLiteral("下一首播放"), this);
    m_favoriteButton = new QPushButton(QStringLiteral("收藏"), this);
    buttonLayout->addWidget(m_addQueueButton);
    buttonLayout->addWidget(m_addNextButton);
    buttonLayout->addWidget(m_favoriteButton);
    buttonLayout->addStretch();

    infoLayout->addWidget(m_titleLabel);
    infoLayout->addWidget(m_artistButton);
    infoLayout->addWidget(m_metaLabel);
    infoLayout->addStretch();
    infoLayout->addLayout(buttonLayout);

    header->addWidget(m_cover);
    header->addLayout(infoLayout, 1);

    m_songsView = new SongsListView(this);
    m_songsView->setPagingEnabled(false);

    layout->addLayout(header);
    layout->addWidget(m_songsView, 1);

    connect(m_addQueueButton, &QPushButton::clicked, this, [this]() {
        if (!m_songs.isEmpty())
            Q_EMIT addToQueueRequested(m_songs);
    });
    connect(m_addNextButton, &QPushButton::clicked, this, [this]() {
        if (!m_songs.isEmpty())
            Q_EMIT addToNextRequested(m_songs);
    });
    connect(m_favoriteButton, &QPushButton::clicked, this, [this]() {
        const bool favorite = !m_album.isFavorite();
        m_album.setFavorite(favorite);
        updateHeader();
        Q_EMIT favoriteToggled(m_album, favorite);
    });
    connect(m_artistButton, &QPushButton::clicked, this, [this]() {
        Artist artist;
        artist.setId(m_album.artistId());
        artist.setName(m_album.artist());
        Q_EMIT artistClicked(artist);
    });

    connect(m_songsView, &SongsListView::songActivated, this, &AlbumDetailPage::songActivated);
    connect(m_songsView, &SongsListView::playNextRequested, this, &AlbumDetailPage::playNextRequested);
    connect(m_songsView, &SongsListView::favoriteToggled, this,
            &AlbumDetailPage::songFavoriteToggled);
    connect(m_songsView, &SongsListView::addToPlaylistRequested, this,
            &AlbumDetailPage::addToPlaylistRequested);
    connect(m_songsView, &SongsListView::downloadRequested, this, &AlbumDetailPage::downloadRequested);
    connect(m_songsView, &SongsListView::detailsRequested, this, &AlbumDetailPage::detailsRequested);

    AppContext *ctx = AppContext::instance();
    if (ctx && ctx->api()) {
        connect(ctx->api(), &SubsonicApi::albumReady, this, &AlbumDetailPage::onAlbumReady);
    }
}

void AlbumDetailPage::sortSongs(QVector<Song> &songs)
{
    std::sort(songs.begin(), songs.end(), [](const Song &a, const Song &b) {
        if (a.discNumber() != b.discNumber())
            return a.discNumber() < b.discNumber();
        return a.track() < b.track();
    });
}

void AlbumDetailPage::setAlbum(const Album &album)
{
    m_album = album;
    m_songs.clear();
    updateHeader();

    AppContext *ctx = AppContext::instance();
    if (ctx && ctx->api() && ctx->api()->isConfigured())
        ctx->api()->getAlbum(album.id());
}

void AlbumDetailPage::updateHeader()
{
    m_titleLabel->setText(m_album.name());
    m_artistButton->setText(m_album.artist().isEmpty() ? QStringLiteral("未知艺术家")
                                                       : m_album.artist());
    m_cover->setCoverArt(m_album.coverArtId(), 160);
    m_cover->setPlaceholderText(m_album.name());

    QStringList meta;
    if (m_album.year() > 0)
        meta.append(QString::number(m_album.year()));
    if (!m_album.genre().isEmpty())
        meta.append(m_album.genre());
    if (m_album.songCount() > 0)
        meta.append(QStringLiteral("%1 首").arg(m_album.songCount()));
    m_metaLabel->setText(meta.join(QStringLiteral(" · ")));

    m_favoriteButton->setText(m_album.isFavorite() ? QStringLiteral("已收藏")
                                                   : QStringLiteral("收藏"));
}

void AlbumDetailPage::onAlbumReady(const Album &album, const QVector<Song> &songs)
{
    if (album.id() != m_album.id())
        return;

    m_album = album;
    m_songs = songs;
    sortSongs(m_songs);
    m_songsView->setSongs(m_songs);
    updateHeader();
}

void AlbumDetailPage::refresh()
{
    AppContext *ctx = AppContext::instance();
    if (!ctx || !ctx->api() || !ctx->api()->isConfigured() || !m_album.isValid())
        return;
    ctx->api()->getAlbum(m_album.id());
}

void AlbumDetailPage::retranslate()
{
    m_addQueueButton->setText(QStringLiteral("添加到队列"));
    m_addNextButton->setText(QStringLiteral("下一首播放"));
    updateHeader();
}

#include "playlistdetailpage.h"

#include "widgets/pages/songslistview.h"
#include "core/appcontext.h"
#include "core/subsonicapi.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

PlaylistDetailPage::PlaylistDetailPage(QWidget *parent)
    : PageBase(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(10);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName(QStringLiteral("playlistDetailTitle"));
    QFont font = m_titleLabel->font();
    font.setPointSize(font.pointSize() + 8);
    font.setBold(true);
    m_titleLabel->setFont(font);

    m_metaLabel = new QLabel(this);

    auto *buttonLayout = new QHBoxLayout();
    m_playAllButton = new QPushButton(QStringLiteral("播放全部"), this);
    m_addQueueButton = new QPushButton(QStringLiteral("添加到队列"), this);
    m_saveOrderButton = new QPushButton(QStringLiteral("保存顺序"), this);
    buttonLayout->addWidget(m_playAllButton);
    buttonLayout->addWidget(m_addQueueButton);
    buttonLayout->addWidget(m_saveOrderButton);
    buttonLayout->addStretch();

    m_songsView = new SongsListView(this);
    m_songsView->setPagingEnabled(false);
    m_songsView->setDragReorderEnabled(true);

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_metaLabel);
    layout->addLayout(buttonLayout);
    layout->addWidget(m_songsView, 1);

    connect(m_playAllButton, &QPushButton::clicked, this, [this]() {
        Q_EMIT playAllRequested(m_songsView->songs());
    });
    connect(m_addQueueButton, &QPushButton::clicked, this, [this]() {
        Q_EMIT addToQueueRequested(m_songsView->songs());
    });
    connect(m_saveOrderButton, &QPushButton::clicked, this, &PlaylistDetailPage::onSaveOrder);
    connect(m_songsView, &SongsListView::orderChanged, this, &PlaylistDetailPage::onOrderChanged);

    connect(m_songsView, &SongsListView::songActivated, this, &PlaylistDetailPage::songActivated);
    connect(m_songsView, &SongsListView::playNextRequested, this, &PlaylistDetailPage::playNextRequested);
    connect(m_songsView, &SongsListView::favoriteToggled, this, &PlaylistDetailPage::favoriteToggled);
    connect(m_songsView, &SongsListView::addToPlaylistRequested, this,
            &PlaylistDetailPage::addToPlaylistRequested);
    connect(m_songsView, &SongsListView::downloadRequested, this, &PlaylistDetailPage::downloadRequested);
    connect(m_songsView, &SongsListView::detailsRequested, this, &PlaylistDetailPage::detailsRequested);

    AppContext *ctx = AppContext::instance();
    if (ctx && ctx->api())
        connect(ctx->api(), &SubsonicApi::playlistReady, this, &PlaylistDetailPage::onPlaylistReady);
}

void PlaylistDetailPage::setPlaylist(const Playlist &playlist)
{
    m_playlist = playlist;
    m_titleLabel->setText(playlist.name());
    m_metaLabel->setText(QStringLiteral("%1 首").arg(playlist.songCount()));

    if (playlist.songs().isEmpty()) {
        AppContext *ctx = AppContext::instance();
        if (ctx && ctx->api() && ctx->api()->isConfigured())
            ctx->api()->getPlaylist(playlist.id());
    } else {
        m_songsView->setSongs(playlist.songs());
    }
}

void PlaylistDetailPage::onPlaylistReady(const Playlist &playlist)
{
    if (playlist.id() != m_playlist.id())
        return;
    m_playlist = playlist;
    m_titleLabel->setText(playlist.name());
    m_metaLabel->setText(QStringLiteral("%1 首").arg(playlist.songCount()));
    m_songsView->setSongs(playlist.songs());
}

void PlaylistDetailPage::onOrderChanged(const QVector<Song> &songs)
{
    m_playlist.setSongs(songs);
}

void PlaylistDetailPage::onSaveOrder()
{
    AppContext *ctx = AppContext::instance();
    if (!ctx || !ctx->api() || !ctx->api()->isConfigured())
        return;

    const QVector<Song> songs = m_songsView->songs();
    QStringList ids;
    QList<int> indexesToRemove;
    for (int i = 0; i < songs.size(); ++i) {
        ids.append(songs.at(i).id());
        indexesToRemove.append(i);
    }

    ctx->api()->updatePlaylist(m_playlist.id(), m_playlist.name(), m_playlist.comment(),
                               m_playlist.isPublic(), ids, indexesToRemove);
}

void PlaylistDetailPage::refresh()
{
    AppContext *ctx = AppContext::instance();
    if (ctx && ctx->api() && ctx->api()->isConfigured() && m_playlist.isValid())
        ctx->api()->getPlaylist(m_playlist.id());
}

void PlaylistDetailPage::retranslate()
{
    m_playAllButton->setText(QStringLiteral("播放全部"));
    m_addQueueButton->setText(QStringLiteral("添加到队列"));
    m_saveOrderButton->setText(QStringLiteral("保存顺序"));
}

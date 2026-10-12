#include "searchpage.h"

#include "widgets/pages/songslistview.h"
#include "widgets/pages/albumsview.h"
#include "widgets/pages/artistsview.h"
#include "core/appcontext.h"
#include "core/subsonicapi.h"

#include <QTabWidget>
#include <QVBoxLayout>

SearchPage::SearchPage(QWidget *parent)
    : PageBase(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_tabs = new QTabWidget(this);

    m_songsView = new SongsListView(this);
    m_songsView->setPagingEnabled(false);

    m_albumsView = new AlbumsView(this);
    m_albumsView->setSource(AlbumsView::Source::All);

    m_artistsView = new ArtistsView(this);
    m_artistsView->setSource(ArtistsView::Source::All);

    m_tabs->addTab(m_songsView, QStringLiteral("歌曲"));
    m_tabs->addTab(m_albumsView, QStringLiteral("专辑"));
    m_tabs->addTab(m_artistsView, QStringLiteral("艺术家"));

    layout->addWidget(m_tabs);

    connect(m_songsView, &SongsListView::songActivated, this, &SearchPage::songActivated);
    connect(m_songsView, &SongsListView::playNextRequested, this, &SearchPage::playNextRequested);
    connect(m_songsView, &SongsListView::favoriteToggled, this, &SearchPage::favoriteToggled);
    connect(m_songsView, &SongsListView::addToPlaylistRequested, this,
            &SearchPage::addToPlaylistRequested);
    connect(m_songsView, &SongsListView::downloadRequested, this, &SearchPage::downloadRequested);
    connect(m_songsView, &SongsListView::detailsRequested, this, &SearchPage::detailsRequested);
    connect(m_albumsView, &AlbumsView::albumClicked, this, &SearchPage::albumClicked);
    connect(m_albumsView, &AlbumsView::playAlbumRequested, this, &SearchPage::playAlbumRequested);
    connect(m_artistsView, &ArtistsView::artistClicked, this, &SearchPage::artistClicked);

    AppContext *ctx = AppContext::instance();
    if (ctx && ctx->api()) {
        connect(ctx->api(), &SubsonicApi::searchReady, this,
                [this](const QVector<Artist> &artists, const QVector<Album> &albums,
                       const QVector<Song> &songs) {
                    m_songsView->setSongs(songs);
                    m_albumsView->setSource(AlbumsView::Source::All);
                    m_albumsView->setAlbums(albums);
                    m_artistsView->setArtists(artists);
                });
    }
}

void SearchPage::search(const QString &query)
{
    m_query = query;
    AppContext *ctx = AppContext::instance();
    if (!ctx || !ctx->api() || !ctx->api()->isConfigured() || query.trimmed().isEmpty())
        return;
    ctx->api()->search3(query.trimmed(), 20, 20, 100);
}

void SearchPage::refresh()
{
    if (!m_query.isEmpty())
        search(m_query);
}

void SearchPage::retranslate()
{
    m_tabs->setTabText(0, QStringLiteral("歌曲"));
    m_tabs->setTabText(1, QStringLiteral("专辑"));
    m_tabs->setTabText(2, QStringLiteral("艺术家"));
}

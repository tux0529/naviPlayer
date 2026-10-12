#include "songspage.h"

#include "widgets/pages/songslistview.h"
#include "models/album.h"
#include "core/appcontext.h"
#include "core/subsonicapi.h"
#include "core/databasemanager.h"
#include "core/appsettings.h"
#include "core/asynctask.h"

#include <QDateTime>
#include <QTimer>
#include <QVBoxLayout>

namespace {
constexpr int kScanAlbumLimit = 300;
}

SongsPage::SongsPage(QWidget *parent)
    : PageBase(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_view = new SongsListView(this);
    layout->addWidget(m_view);

    connect(m_view, &SongsListView::songActivated, this, &SongsPage::songActivated);
    connect(m_view, &SongsListView::playNextRequested, this, &SongsPage::playNextRequested);
    connect(m_view, &SongsListView::favoriteToggled, this, &SongsPage::favoriteToggled);
    connect(m_view, &SongsListView::addToPlaylistRequested, this, &SongsPage::addToPlaylistRequested);
    connect(m_view, &SongsListView::downloadRequested, this, &SongsPage::downloadRequested);
    connect(m_view, &SongsListView::detailsRequested, this, &SongsPage::detailsRequested);

    AppContext *ctx = AppContext::instance();
    if (ctx && ctx->api()) {
        connect(ctx->api(), &SubsonicApi::songsReady, this, [this](const QVector<Song> &songs) {
            if (m_source != Source::All)
                return;
            if (!songs.isEmpty()) {
                m_view->setSongs(songs);
                m_lastRemoteFetch = QDateTime::currentDateTime();
                return;
            }
            // 空查询搜索无结果：退化为按专辑逐个拉取
            startAlbumScanFallback();
        });
        // “全部歌曲”走 search3，其结果由 searchReady 返回
        connect(ctx->api(), &SubsonicApi::searchReady, this,
                [this](const QVector<Artist> &, const QVector<Album> &,
                       const QVector<Song> &songs) {
                    if (m_source != Source::All)
                        return;
                    if (songs.isEmpty()) {
                        startAlbumScanFallback();
                        return;
                    }
                    m_view->setSongs(songs);
                    m_lastRemoteFetch = QDateTime::currentDateTime();
                });
        connect(ctx->api(), &SubsonicApi::albumReady, this, &SongsPage::onAlbumReady);
    }
    if (ctx) {
        connect(ctx, &AppContext::favoritesChanged, this, [this]() {
            if (m_source == Source::Favorite)
                loadLocal();
        });
    }
}

void SongsPage::setSource(Source source)
{
    m_source = source;
}

void SongsPage::loadLocal()
{
    DatabaseManager *db = AppContext::instance() ? AppContext::instance()->database() : nullptr;
    if (!db)
        return;

    Async::run(this, [this, db, source = m_source]() -> QVector<Song> {
                   return source == Source::Favorite ? db->favoriteSongs() : db->allSongs();
               },
               [this](const QVector<Song> &songs) {
                   if (!songs.isEmpty())
                       m_view->setSongs(songs);
               });
}

void SongsPage::requestRemote()
{
    AppContext *ctx = AppContext::instance();
    if (!ctx || !ctx->api() || !ctx->api()->isConfigured())
        return;

    if (m_source == Source::Favorite) {
        ctx->refreshFavorites();
        return;
    }

    AppSettings *settings = ctx->settings();
    const int ttlMinutes = settings ? settings->pageCacheTtlMinutes() : 30;
    if (m_lastRemoteFetch.isValid()
        && m_lastRemoteFetch.secsTo(QDateTime::currentDateTime()) < ttlMinutes * 60) {
        return;
    }

    // 先尝试空查询搜索（多数服务器可返回全量）
    ctx->api()->search3(QString(), 0, 0, 500);

    // 部分服务器对空查询返回空结果：超时后退化为按专辑逐个拉取
    QTimer::singleShot(6000, this, [this]() {
        if (m_source == Source::All && m_view->songs().isEmpty() && !m_scanning)
            startAlbumScanFallback();
    });
}

void SongsPage::startAlbumScanFallback()
{
    AppContext *ctx = AppContext::instance();
    if (!ctx || !ctx->database() || m_scanning)
        return;

    const QVector<Album> albums = ctx->database()->allAlbums();
    if (albums.isEmpty()) {
        // 还没有专辑缓存，先拉专辑列表
        if (ctx->api() && ctx->api()->isConfigured())
            ctx->api()->getAlbumList2(SubsonicApi::AlbumListType::AlphabeticalByName, 500, 0);
        return;
    }

    m_pendingAlbumIds.clear();
    for (int i = 0; i < albums.size() && i < kScanAlbumLimit; ++i)
        m_pendingAlbumIds.append(albums.at(i).id());

    m_scanning = true;
    requestNextAlbum();
}

void SongsPage::requestNextAlbum()
{
    AppContext *ctx = AppContext::instance();
    if (!ctx || !ctx->api() || m_pendingAlbumIds.isEmpty()) {
        m_scanning = false;
        m_lastRemoteFetch = QDateTime::currentDateTime();
        return;
    }
    const QString id = m_pendingAlbumIds.takeFirst();
    ctx->api()->getAlbum(id);
}

void SongsPage::onAlbumReady(const Album &album, const QVector<Song> &songs)
{
    if (m_source != Source::All)
        return;

    if (!m_scanning) {
        // 非扫描流程：仅入库（AppContext 已处理）
        return;
    }

    Q_UNUSED(album)

    QVector<Song> merged = m_view->songs();
    for (const Song &s : songs) {
        bool exists = false;
        for (const Song &existing : merged) {
            if (existing.id() == s.id()) {
                exists = true;
                break;
            }
        }
        if (!exists)
            merged.append(s);
    }
    m_view->setSongs(merged);

    requestNextAlbum();
}

void SongsPage::setFavorite(const QString &songId, bool favorite)
{
    m_view->setFavorite(songId, favorite);
}

void SongsPage::setPlayingSongId(const QString &songId)
{
    m_view->setPlayingSongId(songId);
}

void SongsPage::refresh()
{
    loadLocal();
    requestRemote();
}

void SongsPage::retranslate()
{
    m_view->retranslate();
}

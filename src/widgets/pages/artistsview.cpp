#include "artistsview.h"

#include "core/appcontext.h"
#include "core/subsonicapi.h"
#include "core/databasemanager.h"
#include "core/appsettings.h"
#include "core/cachemanager.h"
#include "core/asynctask.h"

#include <QHeaderView>
#include <QPixmap>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVBoxLayout>

ArtistTableModel::ArtistTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int ArtistTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_artists.size();
}

int ArtistTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : ColCount;
}

QVariant ArtistTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_artists.size())
        return QVariant();

    const Artist &artist = m_artists.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case ColName:       return artist.name();
        case ColAlbumCount: return artist.albumCount() > 0
                                    ? QStringLiteral("%1 张专辑").arg(artist.albumCount())
                                    : QString();
        case ColFavorite:   return artist.isFavorite() ? QStringLiteral("♥") : QStringLiteral("♡");
        default:            return QVariant();
        }
    case Qt::DecorationRole: {
        if (index.column() != ColAvatar)
            return QVariant();
        CacheManager *cache = AppContext::instance() ? AppContext::instance()->cache() : nullptr;
        if (!cache || artist.coverArtId().isEmpty())
            return QVariant();
        const QPixmap pm = cache->cachedCover(artist.coverArtId(), 40);
        if (pm.isNull()) {
            cache->requestCover(artist.coverArtId(), 40);
            return QVariant();
        }
        return pm.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    case Qt::TextAlignmentRole:
        if (index.column() == ColFavorite || index.column() == ColAvatar)
            return static_cast<int>(Qt::AlignCenter);
        return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
    case ArtistRole:
        return QVariant::fromValue(artist);
    case CoverArtIdRole:
        return artist.coverArtId();
    default:
        return QVariant();
    }
}

QVariant ArtistTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();
    switch (section) {
    case ColAvatar:     return QStringLiteral("头像");
    case ColName:       return QStringLiteral("艺术家");
    case ColAlbumCount: return QStringLiteral("专辑数");
    case ColFavorite:   return QStringLiteral("♥");
    default:            return QVariant();
    }
}

void ArtistTableModel::setArtists(const QVector<Artist> &artists)
{
    beginResetModel();
    m_artists = artists;
    endResetModel();
}

Artist ArtistTableModel::artistAt(int row) const
{
    if (row < 0 || row >= m_artists.size())
        return Artist();
    return m_artists.at(row);
}

void ArtistTableModel::setFavorite(const QString &artistId, bool favorite)
{
    for (int i = 0; i < m_artists.size(); ++i) {
        if (m_artists.at(i).id() == artistId) {
            m_artists[i].setFavorite(favorite);
            Q_EMIT dataChanged(index(i, 0), index(i, ColCount - 1));
            return;
        }
    }
}

ArtistsView::ArtistsView(QWidget *parent)
    : PageBase(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 8);

    m_model = new ArtistTableModel(this);
    m_proxy = new QSortFilterProxyModel(this);
    m_proxy->setSourceModel(m_model);
    m_proxy->setSortCaseSensitivity(Qt::CaseInsensitive);

    m_view = new QTableView(this);
    m_view->setModel(m_proxy);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->setAlternatingRowColors(true);
    m_view->setShowGrid(false);
    m_view->verticalHeader()->setVisible(false);
    m_view->setSortingEnabled(true);
    m_view->horizontalHeader()->setSectionResizeMode(ArtistTableModel::ColName, QHeaderView::Stretch);
    m_view->setColumnWidth(ArtistTableModel::ColAvatar, 56);
    m_view->setColumnWidth(ArtistTableModel::ColAlbumCount, 100);
    m_view->setColumnWidth(ArtistTableModel::ColFavorite, 48);
    m_view->verticalHeader()->setDefaultSectionSize(48);

    layout->addWidget(m_view);

    connect(m_view, &QTableView::doubleClicked, this, &ArtistsView::onDoubleClicked);
    connect(m_view, &QTableView::clicked, this, &ArtistsView::onClicked);

    AppContext *ctx = AppContext::instance();
    if (ctx && ctx->api()) {
        connect(ctx->api(), &SubsonicApi::artistsReady, this, [this](const QVector<Artist> &artists) {
            if (m_source != Source::All)
                return;
            m_loading = false;
            m_lastRemoteFetch = QDateTime::currentDateTime();
            m_model->setArtists(artists);
        });
    }
    if (ctx) {
        connect(ctx, &AppContext::favoritesChanged, this, [this]() {
            if (m_source == Source::Favorite)
                loadLocal();
        });
    }
}

void ArtistsView::setSource(Source source)
{
    m_source = source;
}

void ArtistsView::onDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    const QModelIndex src = m_proxy->mapToSource(index);
    Q_EMIT artistClicked(m_model->artistAt(src.row()));
}

void ArtistsView::onClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    const QModelIndex src = m_proxy->mapToSource(index);
    if (src.column() != ArtistTableModel::ColFavorite)
        return;
    const Artist artist = m_model->artistAt(src.row());
    m_model->setFavorite(artist.id(), !artist.isFavorite());
    Q_EMIT favoriteToggled(artist, !artist.isFavorite());
}

void ArtistsView::loadLocal()
{
    DatabaseManager *db = AppContext::instance() ? AppContext::instance()->database() : nullptr;
    if (!db)
        return;

    Async::run(this, [this, db, source = m_source]() -> QVector<Artist> {
        return source == Source::Favorite ? db->favoriteArtists() : db->allArtists();
    }, [this](const QVector<Artist> &artists) {
        if (artists.isEmpty())
            return;
        m_model->setArtists(artists);
    });
}

void ArtistsView::requestRemote()
{
    AppContext *ctx = AppContext::instance();
    if (!ctx || !ctx->api() || !ctx->api()->isConfigured())
        return;
    if (m_loading)
        return;

    AppSettings *settings = ctx->settings();
    const int ttlMinutes = settings ? settings->pageCacheTtlMinutes() : 30;
    if (m_lastRemoteFetch.isValid()
        && m_lastRemoteFetch.secsTo(QDateTime::currentDateTime()) < ttlMinutes * 60) {
        return;
    }

    m_loading = true;
    if (m_source == Source::Favorite) {
        ctx->refreshFavorites();
        m_loading = false;
    } else {
        ctx->api()->getArtists();
    }
}

void ArtistsView::setArtists(const QVector<Artist> &artists)
{
    m_model->setArtists(artists);
}

void ArtistsView::refresh()
{
    loadLocal();
    requestRemote();
}

void ArtistsView::retranslate()
{
    m_model->setArtists(m_model->artists());
}

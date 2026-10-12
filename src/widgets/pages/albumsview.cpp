#include "albumsview.h"

#include "widgets/components/albumcard.h"
#include "core/appcontext.h"
#include "core/subsonicapi.h"
#include "core/databasemanager.h"
#include "core/appsettings.h"
#include "core/asynctask.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QTimer>
#include <algorithm>

namespace {
constexpr int kCardSpacing = 16;
constexpr int kMinCardWidth = 150;
constexpr int kFetchPageSize = 500;
constexpr int kMaxAlbums = 3000;
}

AlbumsView::AlbumsView(QWidget *parent)
    : PageBase(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 12, 16, 12);
    mainLayout->setSpacing(8);

    m_gridWidget = new QWidget(this);
    m_grid = new QGridLayout(m_gridWidget);
    m_grid->setContentsMargins(0, 0, 0, 0);
    m_grid->setHorizontalSpacing(kCardSpacing);
    m_grid->setVerticalSpacing(kCardSpacing);
    m_grid->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(m_gridWidget);

    m_emptyLabel = new QLabel(QStringLiteral("暂无专辑"), this);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setVisible(false);

    // 分页条
    auto *pageBar = new QHBoxLayout();
    m_prevButton = new QPushButton(QStringLiteral("上一页"), this);
    m_nextButton = new QPushButton(QStringLiteral("下一页"), this);
    m_pageLabel = new QLabel(QStringLiteral("第 1 / 1 页"), this);
    m_pageLabel->setAlignment(Qt::AlignCenter);

    pageBar->addStretch();
    pageBar->addWidget(m_prevButton);
    pageBar->addWidget(m_pageLabel);
    pageBar->addWidget(m_nextButton);
    pageBar->addStretch();

    mainLayout->addWidget(scroll, 1);
    mainLayout->addWidget(m_emptyLabel, 1);
    mainLayout->addLayout(pageBar);

    connect(m_prevButton, &QPushButton::clicked, this, &AlbumsView::onPrevPage);
    connect(m_nextButton, &QPushButton::clicked, this, &AlbumsView::onNextPage);

    AppContext *ctx = AppContext::instance();
    if (ctx && ctx->api()) {
        connect(ctx->api(), &SubsonicApi::albumListReady, this,
                [this](const QString &type, int offset, const QVector<Album> &albums) {
                    // 只处理本页发起的全量拉取（按名称排序）
                    if (m_source != Source::All || type != QLatin1String("alphabeticalByName"))
                        return;

                    m_loading = false;
                    m_lastRemoteFetch = QDateTime::currentDateTime();

                    if (offset == 0)
                        m_albums = albums;
                    else
                        m_albums.append(albums);

                    rebuildPage();

                    // 还有更多专辑则继续拉取下一页
                    if (albums.size() == kFetchPageSize && m_albums.size() < kMaxAlbums
                        && AppContext::instance()->api()
                        && AppContext::instance()->api()->isConfigured()) {
                        m_loading = true;
                        AppContext::instance()->api()->getAlbumList2(
                            SubsonicApi::AlbumListType::AlphabeticalByName, kFetchPageSize,
                            offset + kFetchPageSize);
                    }
                });
        connect(ctx->api(), &SubsonicApi::artistReady, this,
                [this](const Artist &artist, const QVector<Album> &albums) {
                    if (m_source != Source::Artist || artist.id() != m_artist.id())
                        return;
                    m_loading = false;
                    m_albums = albums;
                    m_lastRemoteFetch = QDateTime::currentDateTime();
                    rebuildPage();
                });
    }
    if (ctx) {
        connect(ctx, &AppContext::favoritesChanged, this, [this]() {
            if (m_source == Source::Favorite)
                loadLocal();
        });
    }
}

void AlbumsView::setSource(Source source)
{
    m_source = source;
    m_page = 0;
}

void AlbumsView::setArtist(const Artist &artist)
{
    m_artist = artist;
    m_page = 0;
    // 切换艺术家：清掉上一个艺术家的数据，并让远端缓存时间失效（TTL 不能跨艺术家复用）
    m_albums.clear();
    m_lastRemoteFetch = QDateTime();
    m_loading = false;
    rebuildPage();
}

int AlbumsView::albumsPerPage() const
{
    AppSettings *settings = AppContext::instance() ? AppContext::instance()->settings() : nullptr;
    return settings ? qMax(1, settings->albumsPerPage()) : 40;
}

int AlbumsView::pageCount() const
{
    const int perPage = albumsPerPage();
    return qMax(1, (m_albums.size() + perPage - 1) / perPage);
}

void AlbumsView::clearCards()
{
    qDeleteAll(m_cards);
    m_cards.clear();
    QLayoutItem *item = nullptr;
    while ((item = m_grid->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
}

void AlbumsView::rebuildPage()
{
    clearCards();

    const int perPage = albumsPerPage();
    const int start = m_page * perPage;
    const int end = qMin(start + perPage, m_albums.size());

    const int availableWidth = m_gridWidget->width() > 0 ? m_gridWidget->width() : width();
    const int columns = qMax(2, (availableWidth + kCardSpacing) / (kMinCardWidth + kCardSpacing));
    const int cardWidth = qMax(kMinCardWidth, (availableWidth - (columns - 1) * kCardSpacing) / columns);

    int row = 0;
    int col = 0;
    for (int i = start; i < end; ++i) {
        const Album &album = m_albums.at(i);
        auto *card = new AlbumCard(m_gridWidget);
        card->setAlbum(album, cardWidth - 24);
        connect(card, &AlbumCard::albumClicked, this, &AlbumsView::albumClicked);
        connect(card, &AlbumCard::artistClicked, this, &AlbumsView::artistClicked);
        connect(card, &AlbumCard::playClicked, this, &AlbumsView::playAlbumRequested);

        m_grid->addWidget(card, row, col);
        m_cards.append(card);

        if (++col >= columns) {
            col = 0;
            ++row;
        }
    }

    m_emptyLabel->setVisible(m_albums.isEmpty());
    updatePageLabel();
}

void AlbumsView::updatePageLabel()
{
    m_pageLabel->setText(QStringLiteral("第 %1 / %2 页（共 %3 张专辑）")
                             .arg(m_page + 1)
                             .arg(pageCount())
                             .arg(m_albums.size()));
    m_prevButton->setEnabled(m_page > 0);
    m_nextButton->setEnabled(m_page + 1 < pageCount());
}

void AlbumsView::onPrevPage()
{
    if (m_page <= 0)
        return;
    --m_page;
    rebuildPage();
}

void AlbumsView::onNextPage()
{
    if (m_page + 1 >= pageCount())
        return;
    ++m_page;
    rebuildPage();
}

void AlbumsView::loadLocal()
{
    DatabaseManager *db = AppContext::instance() ? AppContext::instance()->database() : nullptr;
    if (!db)
        return;

    const QString artistId = m_artist.id();
    const QString artistName = m_artist.name();
    Async::run(this, [this, db, source = m_source, artistId, artistName]() -> QVector<Album> {
        switch (source) {
        case Source::Favorite:
            return db->favoriteAlbums();
        case Source::Artist: {
            QVector<Album> result;
            const QVector<Album> all = db->allAlbums();
            for (const Album &a : all) {
                if ((!artistId.isEmpty() && a.artistId() == artistId)
                    || (!artistName.isEmpty() && a.artist() == artistName)) {
                    result.append(a);
                }
            }
            return result;
        }
        case Source::All:
        default:
            return db->allAlbums();
        }
    }, [this](const QVector<Album> &albums) {
        if (albums.isEmpty())
            return;
        m_albums = albums;
        rebuildPage();
    });
}

void AlbumsView::requestRemote()
{
    AppContext *ctx = AppContext::instance();
    if (!ctx || !ctx->api() || !ctx->api()->isConfigured())
        return;

    if (m_loading)
        return;

    // TTL 控制：避免频繁请求
    AppSettings *settings = ctx->settings();
    const int ttlMinutes = settings ? settings->pageCacheTtlMinutes() : 30;
    if (m_lastRemoteFetch.isValid()
        && m_lastRemoteFetch.secsTo(QDateTime::currentDateTime()) < ttlMinutes * 60) {
        return;
    }

    m_loading = true;

    switch (m_source) {
    case Source::Favorite:
        ctx->refreshFavorites();
        m_loading = false;
        break;
    case Source::Artist:
        ctx->api()->getArtist(m_artist.id());
        break;
    case Source::All:
    default:
        ctx->api()->getAlbumList2(SubsonicApi::AlbumListType::AlphabeticalByName, kFetchPageSize, 0);
        break;
    }
}

void AlbumsView::setAlbums(const QVector<Album> &albums)
{
    m_albums = albums;
    m_page = 0;
    rebuildPage();
}

void AlbumsView::refresh()
{
    loadLocal();
    QTimer::singleShot(60, this, [this]() { requestRemote(); });
}

void AlbumsView::retranslate()
{
    m_prevButton->setText(QStringLiteral("上一页"));
    m_nextButton->setText(QStringLiteral("下一页"));
    m_emptyLabel->setText(QStringLiteral("暂无专辑"));
    updatePageLabel();
}

void AlbumsView::resizeEvent(QResizeEvent *event)
{
    PageBase::resizeEvent(event);
    if (!m_albums.isEmpty())
        rebuildPage();
}

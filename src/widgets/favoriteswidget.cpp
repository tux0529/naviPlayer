#include "favoriteswidget.h"
#include "songlistwidget.h"
#include "albumwidget.h"
#include "artistwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

FavoritesWidget::FavoritesWidget(QWidget *parent)
    : QWidget(parent)
    , m_subsonicApi(nullptr)
    , m_cacheManager(nullptr)
    , m_dbManager(nullptr)
{
    setupUI();
}

void FavoritesWidget::setSubsonicApi(SubsonicApi *api)
{
    m_subsonicApi = api;
    if (m_albumFavorites) {
        m_albumFavorites->setSubsonicApi(api);
    }
    if (m_artistFavorites) {
        m_artistFavorites->setSubsonicApi(api);
    }
}

void FavoritesWidget::setCacheManager(CacheManager *cache)
{
    m_cacheManager = cache;
    if (m_albumFavorites) {
        m_albumFavorites->setCacheManager(cache);
    }
    if (m_artistFavorites) {
        m_artistFavorites->setCacheManager(cache);
    }
}

void FavoritesWidget::setDatabaseManager(DatabaseManager *db)
{
    m_dbManager = db;
}

void FavoritesWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);

    // Header
    QWidget *headerArea = new QWidget(this);
    QHBoxLayout *headerLayout = new QHBoxLayout(headerArea);
    headerLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *titleLabel = new QLabel("我的收藏", this);
    titleLabel->setStyleSheet(R"(
        color: #ffffff;
        font-size: 24px;
        font-weight: bold;
    )");
    headerLayout->addWidget(titleLabel);

    QPushButton *refreshBtn = new QPushButton("刷新", this);
    refreshBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #3a80b3;
            color: #ffffff;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: #4a90c3;
        }
    )");
    connect(refreshBtn, &QPushButton::clicked, this, &FavoritesWidget::refresh);
    headerLayout->addWidget(refreshBtn);
    headerLayout->addStretch();

    mainLayout->addWidget(headerArea);

    // Favorites tab widget
    m_favoritesTab = new QTabWidget(this);
    m_favoritesTab->setStyleSheet(R"(
        QTabWidget::pane {
            background-color: #2d2d30;
            border: none;
            border-radius: 8px;
        }
        QTabBar::tab {
            background-color: #3d3d3f;
            color: #aaaaaa;
            padding: 10px 20px;
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
            margin-right: 4px;
        }
        QTabBar::tab:selected {
            background-color: #2d2d30;
            color: #ffffff;
        }
        QTabBar::tab:hover:!selected {
            background-color: #4d4d4f;
            color: #ffffff;
        }
    )");

    // Songs tab
    m_songFavorites = new SongListWidget(this);
    m_favoritesTab->addTab(m_songFavorites, "歌曲");

    // Albums tab
    m_albumFavorites = new AlbumWidget(this);
    m_favoritesTab->addTab(m_albumFavorites, "专辑");

    // Artists tab
    m_artistFavorites = new ArtistWidget(this);
    m_favoritesTab->addTab(m_artistFavorites, "艺术家");

    mainLayout->addWidget(m_favoritesTab);

    // Connections
    connect(m_songFavorites, &SongListWidget::songSelected, this, &FavoritesWidget::songSelected);
    connect(m_songFavorites, &SongListWidget::songFavoriteClicked, this, &FavoritesWidget::songFavoriteClicked);
    connect(m_albumFavorites, &AlbumWidget::albumSelected, this, &FavoritesWidget::albumSelected);
    connect(m_artistFavorites, &ArtistWidget::artistSelected, this, &FavoritesWidget::artistSelected);
}

void FavoritesWidget::refresh()
{
    if (m_subsonicApi) {
        m_subsonicApi->getStarred();
    }
}

void FavoritesWidget::onStarredReceived(const QVector<Song> &songs,
                                          const QVector<Album> &albums,
                                          const QVector<Artist> &artists)
{
    m_songFavorites->setSongs(songs);
    m_albumFavorites->setAlbums(albums);
    m_artistFavorites->setArtists(artists);
}

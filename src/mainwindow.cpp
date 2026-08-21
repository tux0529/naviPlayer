#include "mainwindow.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QMessageBox>
#include <QDateTime>
#include <QLabel>
#include <QMap>
#include <QSet>
#include <QTimer>
#include <QCloseEvent>

namespace {
constexpr qint64 kDefaultCacheTtlSecs = 600;
constexpr int kHomeSectionTimeoutMs = 12000;

bool isExpiredByTtl(qint64 lastUpdatedSecs, qint64 ttlSecs)
{
    if (lastUpdatedSecs <= 0) {
        return true;
    }
    return (QDateTime::currentSecsSinceEpoch() - lastUpdatedSecs) > ttlSecs;
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_homeWidget(nullptr)
    , m_artistAlbumsWidget(nullptr)
    , m_artistAlbumsPage(nullptr)
    , m_albumDetailReturnPage(nullptr)
    , m_albumPrevButton(nullptr)
    , m_albumNextButton(nullptr)
    , m_artistAlbumsBackButton(nullptr)
    , m_albumPageLabel(nullptr)
    , m_artistAlbumsLabel(nullptr)
    , m_albumPageSize(24)
    , m_currentAlbumPage(0)
    , m_requestedAlbumPage(0)
    , m_hasNextAlbumPage(false)
    , m_pendingAlbumRequestMode(None)
    , m_artistCacheLastUpdatedSecs(0)
    , m_artistRequestInFlight(false)
    , m_cacheTtlSecs(kDefaultCacheTtlSecs)
{
    // Initialize managers
    m_subsonicApi = new SubsonicApi(this);
    m_dbManager = new DatabaseManager(this);
    m_audioPlayer = new AudioPlayer(this);
    m_cacheManager = new CacheManager(this);

    // Initialize database
    if (!m_dbManager->init()) {
        QMessageBox::critical(this, "错误", "无法初始化数据库！");
    } else {
        m_cacheManager->init();
    }

    setupUI();
    setupConnections();
    loadSettings();
    restorePlaybackSession();

    // Check if server is configured
    if (!m_subsonicApi->isConfigured()) {
        onSettingsTriggered();
    } else {
        m_navigationWidget->setCurrentItem(NavigationWidget::Home);
        m_contentStack->setCurrentWidget(m_homeWidget);
        showHome();
    }
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    setWindowTitle("naviPlayer - Subsonic 音乐播放器");
    setMinimumSize(1024, 768);
    setStyleSheet(R"(
        QMainWindow {
            background-color: #1e1e1e;
        }
    )");

    // Create central widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Create splitter for navigation and content
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);

    // Navigation widget
    m_navigationWidget = new NavigationWidget(this);
    m_navigationWidget->setMinimumWidth(200);
    m_navigationWidget->setMaximumWidth(300);
    splitter->addWidget(m_navigationWidget);

    // Content stack
    m_contentStack = new QStackedWidget(this);

    // Create pages
    m_homeWidget = new HomeWidget(this);
    m_homeWidget->setSubsonicApi(m_subsonicApi);
    m_homeWidget->setCacheManager(m_cacheManager);

    // Favorites page
    m_favoritesWidget = new FavoritesWidget(this);
    m_favoritesWidget->setSubsonicApi(m_subsonicApi);
    m_favoritesWidget->setCacheManager(m_cacheManager);
    m_favoritesWidget->setDatabaseManager(m_dbManager);

    // Albums page
    m_albumsPage = new QWidget(this);
    QVBoxLayout *albumsLayout = new QVBoxLayout(m_albumsPage);
    QLabel *albumsLabel = new QLabel("专辑", this);
    albumsLabel->setStyleSheet("color: #ffffff; font-size: 24px; font-weight: bold; padding: 16px;");
    m_albumWidget = new AlbumWidget(this);
    m_albumWidget->setSubsonicApi(m_subsonicApi);
    m_albumWidget->setCacheManager(m_cacheManager);
    QHBoxLayout *albumPaginationLayout = new QHBoxLayout();
    albumPaginationLayout->setContentsMargins(16, 0, 16, 16);

    m_albumPrevButton = new QPushButton("上一页", this);
    m_albumPrevButton->setEnabled(false);
    m_albumPrevButton->setStyleSheet(R"(
        QPushButton {
            background-color: #3d3d3d;
            color: #ffffff;
            border: none;
            border-radius: 4px;
            padding: 8px 14px;
        }
        QPushButton:hover:enabled {
            background-color: #4d4d4d;
        }
        QPushButton:disabled {
            color: #777777;
            background-color: #2b2b2b;
        }
    )");

    m_albumPageLabel = new QLabel("第 1 页", this);
    m_albumPageLabel->setStyleSheet("color: #cccccc; font-size: 13px;");

    m_albumNextButton = new QPushButton("下一页", this);
    m_albumNextButton->setEnabled(false);
    m_albumNextButton->setStyleSheet(m_albumPrevButton->styleSheet());

    albumPaginationLayout->addWidget(m_albumPrevButton);
    albumPaginationLayout->addStretch();
    albumPaginationLayout->addWidget(m_albumPageLabel);
    albumPaginationLayout->addStretch();
    albumPaginationLayout->addWidget(m_albumNextButton);

    albumsLayout->addWidget(albumsLabel);
    albumsLayout->addWidget(m_albumWidget);
    albumsLayout->addLayout(albumPaginationLayout);

    // Artists page
    m_artistsPage = new QWidget(this);
    QVBoxLayout *artistsLayout = new QVBoxLayout(m_artistsPage);
    QLabel *artistsLabel = new QLabel("艺术家", this);
    artistsLabel->setStyleSheet("color: #ffffff; font-size: 24px; font-weight: bold; padding: 16px;");
    m_artistWidget = new ArtistWidget(this);
    m_artistWidget->setSubsonicApi(m_subsonicApi);
    m_artistWidget->setCacheManager(m_cacheManager);
    artistsLayout->addWidget(artistsLabel);
    artistsLayout->addWidget(m_artistWidget);

    // Artist albums page
    m_artistAlbumsPage = new QWidget(this);
    QVBoxLayout *artistAlbumsLayout = new QVBoxLayout(m_artistAlbumsPage);
    artistAlbumsLayout->setContentsMargins(0, 0, 0, 0);
    artistAlbumsLayout->setSpacing(0);

    QHBoxLayout *artistAlbumsHeaderLayout = new QHBoxLayout();
    artistAlbumsHeaderLayout->setContentsMargins(16, 16, 16, 16);
    artistAlbumsHeaderLayout->setSpacing(12);

    m_artistAlbumsBackButton = new QPushButton("返回", this);
    m_artistAlbumsBackButton->setStyleSheet(m_albumPrevButton->styleSheet());
    m_artistAlbumsLabel = new QLabel("艺术家专辑", this);
    m_artistAlbumsLabel->setStyleSheet("color: #ffffff; font-size: 24px; font-weight: bold;");

    artistAlbumsHeaderLayout->addWidget(m_artistAlbumsBackButton, 0, Qt::AlignLeft);
    artistAlbumsHeaderLayout->addWidget(m_artistAlbumsLabel);
    artistAlbumsHeaderLayout->addStretch();

    m_artistAlbumsWidget = new AlbumWidget(this);
    m_artistAlbumsWidget->setSubsonicApi(m_subsonicApi);
    m_artistAlbumsWidget->setCacheManager(m_cacheManager);

    artistAlbumsLayout->addLayout(artistAlbumsHeaderLayout);
    artistAlbumsLayout->addWidget(m_artistAlbumsWidget);

    // Search page
    m_searchWidget = new SearchWidget(this);
    m_searchWidget->setSubsonicApi(m_subsonicApi);
    m_searchWidget->setCacheManager(m_cacheManager);

    // Album detail page
    m_albumDetailWidget = new AlbumDetailWidget(this);
    m_albumDetailWidget->setSubsonicApi(m_subsonicApi);
    m_albumDetailWidget->setCacheManager(m_cacheManager);

    // Playlist page
    m_playlistWidget = new PlaylistWidget(this);

    // Settings page
    m_settingsWidget = new SettingsWidget(this);

    // Add all pages to the stack
    m_contentStack->addWidget(m_homeWidget);
    m_contentStack->addWidget(m_favoritesWidget);
    m_contentStack->addWidget(m_albumsPage);
    m_contentStack->addWidget(m_artistsPage);
    m_contentStack->addWidget(m_artistAlbumsPage);
    m_contentStack->addWidget(m_searchWidget);
    m_contentStack->addWidget(m_albumDetailWidget);
    m_contentStack->addWidget(m_playlistWidget);
    m_contentStack->addWidget(m_settingsWidget);

    splitter->addWidget(m_contentStack);
    splitter->setStretchFactor(1, 1);

    // Player control bar
    m_playerControlBar = new PlayerControlBar(this);
    m_playerControlBar->setAudioPlayer(m_audioPlayer);
    m_playerControlBar->setSubsonicApi(m_subsonicApi);
    m_playerControlBar->setCacheManager(m_cacheManager);

    mainLayout->addWidget(splitter);
    mainLayout->addWidget(m_playerControlBar);

    // Menu bar
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    QMenu *fileMenu = menuBar->addMenu("文件");
    QAction *exitAction = fileMenu->addAction("退出");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);

    // Status bar
    QStatusBar *statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    m_statusLabel = new QLabel("就绪");
    m_statusLabel->setStyleSheet("color: #ffffff;");
    statusBar->addWidget(m_statusLabel);
    statusBar->showMessage("就绪");
}

void MainWindow::setupConnections()
{
    // Navigation
    connect(m_navigationWidget, &NavigationWidget::itemSelected,
            this, &MainWindow::onNavigationItemSelected);
    connect(m_settingsWidget, &SettingsWidget::saveRequested,
            this, &MainWindow::onSettingsSaved);
    connect(m_cacheManager, &CacheManager::songCached,
            this, &MainWindow::onSongCached);
    connect(m_cacheManager, &CacheManager::songCacheSizeChanged,
            m_settingsWidget, &SettingsWidget::setCurrentCacheSize);
    connect(m_homeWidget, &HomeWidget::albumSelected,
            this, [this](const Album &album) {
        m_albumDetailReturnPage = m_homeWidget;
        showAlbumDetail(album);
    });
    connect(m_homeWidget, &HomeWidget::playAlbumRequested,
            this, [this](const Album &album) {
        if (!m_subsonicApi->isConfigured()) {
            QMessageBox::warning(this, "提示", "请先配置 Subsonic 服务器！");
            return;
        }
        m_pendingAlbumRequestMode = PlayFromStart;
        m_pendingAlbumId = album.id();
        m_statusLabel->setText(QString("正在加载 %1...").arg(album.name()));
        m_subsonicApi->getAlbum(album.id());
    });

    // Favorites widget
    connect(m_favoritesWidget, &FavoritesWidget::songSelected,
            this, &MainWindow::onSongSelected);
    connect(m_favoritesWidget, &FavoritesWidget::songFavoriteClicked,
            this, &MainWindow::onSongFavoriteClicked);
    connect(m_favoritesWidget, &FavoritesWidget::albumSelected,
            this, [this](const Album &album) {
        m_albumDetailReturnPage = m_favoritesWidget;
        showAlbumDetail(album);
    });

    // Album widget
    connect(m_albumWidget, &AlbumWidget::albumSelected,
            this, [this](const Album &album) {
        m_albumDetailReturnPage = m_albumsPage;
        showAlbumDetail(album);
    });
    connect(m_albumWidget, &AlbumWidget::playAlbumRequested,
            this, [this](const Album &album) {
        if (!m_subsonicApi->isConfigured()) {
            QMessageBox::warning(this, "提示", "请先配置 Subsonic 服务器！");
            return;
        }
        m_pendingAlbumRequestMode = PlayFromStart;
        m_pendingAlbumId = album.id();
        m_statusLabel->setText(QString("正在加载 %1...").arg(album.name()));
        m_subsonicApi->getAlbum(album.id());
    });
    connect(m_albumPrevButton, &QPushButton::clicked,
            this, [this]() {
        requestAlbumPage(m_currentAlbumPage - 1);
    });
    connect(m_albumNextButton, &QPushButton::clicked,
            this, [this]() {
        requestAlbumPage(m_currentAlbumPage + 1);
    });

    // Artist widget
    connect(m_artistWidget, &ArtistWidget::artistSelected,
            this, [this](const Artist &artist) {
        m_currentArtist = artist;
        m_statusLabel->setText(QString("正在加载 %1 的专辑...").arg(artist.name()));
        m_subsonicApi->getArtist(artist.id());
    });
    connect(m_artistWidget, &ArtistWidget::artistFavoriteClicked,
            this, &MainWindow::onArtistFavoriteClicked);
    connect(m_artistWidget, &ArtistWidget::artistSearchChanged,
            this, [this](const QString &keyword) {
        m_dbManager->logEvent("artist.search",
                              QMap<QString, QString>{
                                  { "keyword", keyword }
                              });
    });
    connect(m_artistWidget, &ArtistWidget::artistSortChanged,
            this, [this](int column, Qt::SortOrder order) {
        m_dbManager->logEvent("artist.sort",
                              QMap<QString, QString>{
                                  { "column", QString::number(column) },
                                  { "order", order == Qt::AscendingOrder ? "asc" : "desc" }
                              });
    });
    connect(m_artistAlbumsWidget, &AlbumWidget::albumSelected,
            this, [this](const Album &album) {
        m_albumDetailReturnPage = m_artistAlbumsPage;
        showAlbumDetail(album);
    });
    connect(m_artistAlbumsWidget, &AlbumWidget::playAlbumRequested,
            this, [this](const Album &album) {
        if (!m_subsonicApi->isConfigured()) {
            QMessageBox::warning(this, "提示", "请先配置 Subsonic 服务器！");
            return;
        }
        m_pendingAlbumRequestMode = PlayFromStart;
        m_pendingAlbumId = album.id();
        m_statusLabel->setText(QString("正在加载 %1...").arg(album.name()));
        m_subsonicApi->getAlbum(album.id());
    });
    connect(m_artistAlbumsBackButton, &QPushButton::clicked,
            this, [this]() {
        m_contentStack->setCurrentWidget(m_artistsPage);
    });

    // Search widget
    connect(m_searchWidget, &SearchWidget::songSelected,
            this, &MainWindow::onSongSelected);
    connect(m_searchWidget, &SearchWidget::songFavoriteClicked,
            this, &MainWindow::onSongFavoriteClicked);
    connect(m_searchWidget, &SearchWidget::albumSelected,
            this, [this](const Album &album) {
        m_albumDetailReturnPage = m_searchWidget;
        showAlbumDetail(album);
    });

    // Audio player
    connect(m_audioPlayer, &AudioPlayer::stateChanged,
            m_playerControlBar, &PlayerControlBar::onStateChanged);
    connect(m_audioPlayer, &AudioPlayer::positionChanged,
            m_playerControlBar, &PlayerControlBar::onPositionChanged);
    connect(m_audioPlayer, &AudioPlayer::durationChanged,
            m_playerControlBar, &PlayerControlBar::onDurationChanged);
    connect(m_audioPlayer, &AudioPlayer::currentIndexChanged,
            this, [this](int index) {
        if (index >= 0) {
            m_playerControlBar->onSongChanged(m_audioPlayer->currentSong());
            m_playerControlBar->setSongFavorite(isSongStarred(m_audioPlayer->currentSong().id()));
            ensureSongCached(m_audioPlayer->currentSong());
        } else {
            m_playerControlBar->onSongChanged(Song());
            m_playerControlBar->setSongFavorite(false);
        }
    });
    connect(m_playerControlBar, &PlayerControlBar::seekRequested,
            this, &MainWindow::onSeekRequested);
    connect(m_playerControlBar, &PlayerControlBar::previousClicked,
            this, [this]() { m_audioPlayer->playPrevious(); });
    connect(m_playerControlBar, &PlayerControlBar::nextClicked,
            this, [this]() { m_audioPlayer->playNext(); });
    connect(m_playerControlBar, &PlayerControlBar::volumeChanged,
            this, [this](int volume) {
        m_dbManager->saveConfig("volume", volume);
    });
    connect(m_playerControlBar, &PlayerControlBar::muteChanged,
            this, [this](bool muted, int volumeBeforeMute) {
        m_dbManager->saveConfig("muted", muted ? 1 : 0);
        m_dbManager->saveConfig("volumeBeforeMute", volumeBeforeMute);
        m_dbManager->logEvent("player.mute_toggle",
                              QMap<QString, QString>{
                                  { "muted", muted ? "1" : "0" },
                                  { "volume_before_mute", QString::number(volumeBeforeMute) }
                              });
    });
    connect(m_playerControlBar, &PlayerControlBar::songFavoriteClicked,
            this, &MainWindow::onSongFavoriteClicked);

    // Album detail widget
    connect(m_albumDetailWidget, &AlbumDetailWidget::songSelected,
            this, &MainWindow::onSongSelected);
    connect(m_albumDetailWidget, &AlbumDetailWidget::songFavoriteClicked,
            this, &MainWindow::onSongFavoriteClicked);
    connect(m_albumDetailWidget, &AlbumDetailWidget::albumFavoriteClicked,
            this, &MainWindow::onAlbumFavoriteClicked);
    connect(m_albumDetailWidget, &AlbumDetailWidget::backRequested,
            this, [this]() {
        m_contentStack->setCurrentWidget(m_albumDetailReturnPage ? m_albumDetailReturnPage : m_albumsPage);
    });
    connect(m_albumDetailWidget, &AlbumDetailWidget::playAlbum,
            this, [this](const QVector<Song> &songs) {
        if (!songs.isEmpty()) {
            QStringList streamUrls;
            for (const Song &song : songs) {
                streamUrls.append(resolvePlaybackSource(song));
            }
            m_audioPlayer->playPlaylist(songs, streamUrls, 0);
        }
    });
    connect(m_albumDetailWidget, &AlbumDetailWidget::addSongsToPlaylist,
            this, [this](const QVector<Song> &songs) {
        for (const Song &song : songs) {
            QString streamUrl = resolvePlaybackSource(song);
            m_audioPlayer->addToPlaylist(song, streamUrl);
        }
    });
    connect(m_albumDetailWidget, &AlbumDetailWidget::addAlbumToNext,
            this, [this](const QVector<Song> &songs) {
        if (songs.isEmpty()) {
            return;
        }

        QStringList streamUrls;
        for (const Song &song : songs) {
            streamUrls.append(resolvePlaybackSource(song));
        }
        m_audioPlayer->addToNext(songs, streamUrls);
    });

    // Playlist widget
    connect(m_playlistWidget, &PlaylistWidget::playSongAtIndex,
            this, [this](int index) {
        if (index >= 0 && index < m_audioPlayer->playlist().size()) {
            m_audioPlayer->playSongAt(index);
        }
    });
    connect(m_playlistWidget, &PlaylistWidget::removeSongs,
            this, [this](const QVector<int> &indices) {
        for (int i = indices.size() - 1; i >= 0; --i) {
            m_audioPlayer->removeFromPlaylist(indices[i]);
        }
    });
    connect(m_playlistWidget, &PlaylistWidget::clearPlaylist,
            this, [this]() {
        m_audioPlayer->clearPlaylist();
    });
    connect(m_playlistWidget, &PlaylistWidget::playModeChanged,
            this, [this](PlaylistWidget::PlayMode mode) {
        AudioPlayer::PlayMode audioMode = static_cast<AudioPlayer::PlayMode>(mode);
        m_audioPlayer->setPlayMode(audioMode);
    });

    // Audio player playlist signals
    connect(m_audioPlayer, &AudioPlayer::playlistChanged,
            this, [this](const QVector<Song> &songs) {
        m_playlistWidget->setSongs(songs);
    });
    connect(m_audioPlayer, &AudioPlayer::currentIndexChanged,
            this, [this](int index) {
        m_playlistWidget->setCurrentIndex(index);
    });
    connect(m_audioPlayer, &AudioPlayer::playModeChanged,
            this, [this](AudioPlayer::PlayMode mode) {
        PlaylistWidget::PlayMode playlistMode = static_cast<PlaylistWidget::PlayMode>(mode);
        m_playlistWidget->setPlayMode(playlistMode);
    });
    // 切歌前保存当前播放会话（含上一首进度），防止异常退出丢失进度
    connect(m_audioPlayer, &AudioPlayer::trackAboutToChange,
            this, [this](const QString &songId, qint64 position) {
        Q_UNUSED(songId)
        Q_UNUSED(position)
        savePlaybackSession();
    });

    // Subsonic API
    connect(m_subsonicApi, &SubsonicApi::pingFinished,
            this, &MainWindow::onPingFinished);
    connect(m_subsonicApi, &SubsonicApi::artistsReceived,
            this, &MainWindow::onArtistsReceived);
    connect(m_subsonicApi, &SubsonicApi::artistsFailed,
            this, &MainWindow::onArtistsFailed);
    connect(m_subsonicApi, &SubsonicApi::artistAlbumsReceived,
            this, &MainWindow::onArtistAlbumsReceived);
    connect(m_subsonicApi, &SubsonicApi::albumsReceived,
            this, &MainWindow::onAlbumsReceived);
    connect(m_subsonicApi, &SubsonicApi::albumsFailed,
            this, &MainWindow::onAlbumsFailed);
    connect(m_subsonicApi, &SubsonicApi::albumListReceived,
            this, &MainWindow::onHomeAlbumListReceived);
    connect(m_subsonicApi, &SubsonicApi::albumListFailed,
            this, &MainWindow::onHomeAlbumListFailed);
    connect(m_subsonicApi, &SubsonicApi::albumReceived,
            this, &MainWindow::onAlbumReceived);
    connect(m_subsonicApi, &SubsonicApi::songsReceived,
            this, &MainWindow::onSongsReceived);
    connect(m_subsonicApi, &SubsonicApi::searchResultsReceived,
            this, [this](const QVector<Artist> &artists, const QVector<Album> &albums, const QVector<Song> &songs) {
        m_searchWidget->onSearchResultsReceived(artists, albums, songs);
    });
    connect(m_subsonicApi, &SubsonicApi::starredReceived,
            this, &MainWindow::onStarredReceived);
    connect(m_subsonicApi, &SubsonicApi::errorOccurred,
            this, &MainWindow::onApiError);
}

void MainWindow::loadSettings()
{
    QString serverUrl = m_dbManager->loadConfig("serverUrl").toString();
    QString username = m_dbManager->loadConfig("username").toString();
    QString password = m_dbManager->loadConfig("password").toString();
    const int cacheSizeMb = m_dbManager->loadConfig("songCacheSizeMb", 512).toInt();
    const QString cacheDirectory = m_dbManager->loadConfig("songCachePath").toString();
    const int albumPageSize = m_dbManager->loadConfig("albumPageSize", 24).toInt();
    const QVariant legacyCacheTtl = m_dbManager->loadConfig("homeSectionTtlMinutes");
    const int cacheTtlMinutes = m_dbManager->loadConfig("cacheTtlMinutes",
                                                        legacyCacheTtl.isValid() ? legacyCacheTtl : 10).toInt();
    const int volume = m_dbManager->loadConfig("volume", 80).toInt();
    const bool muted = m_dbManager->loadConfig("muted", 0).toInt() != 0;
    const int volumeBeforeMute = m_dbManager->loadConfig("volumeBeforeMute", volume).toInt();

    m_subsonicApi->setServerUrl(serverUrl);
    m_subsonicApi->setUsername(username);
    m_subsonicApi->setPassword(password);
    m_cacheManager->setMaxSongCacheSizeMb(cacheSizeMb);
    m_cacheManager->setSongCacheDirectory(cacheDirectory);
    m_albumPageSize = qBound(10, albumPageSize, 50);
    m_cacheTtlSecs = qMax<qint64>(60, static_cast<qint64>(cacheTtlMinutes) * 60);
    m_audioPlayer->setVolume(muted ? 0 : qBound(0, volume, 100));
    m_playerControlBar->setMuted(muted, volumeBeforeMute);
    m_playerControlBar->setVolume(qBound(0, muted ? volumeBeforeMute : volume, 100));

    m_settingsWidget->setServerUrl(serverUrl);
    m_settingsWidget->setUsername(username);
    m_settingsWidget->setPassword(password);
    m_settingsWidget->setCacheSizeMb(cacheSizeMb);
    m_settingsWidget->setCacheTtlMinutes(qMax(1, cacheTtlMinutes));
    m_settingsWidget->setAlbumPageSize(m_albumPageSize);
    m_settingsWidget->setSongCacheDirectory(cacheDirectory);
    m_settingsWidget->setCurrentCacheSize(m_cacheManager->songCacheSize());

    if (m_subsonicApi->isConfigured()) {
        refreshStarred();
    }
}

void MainWindow::saveSettings()
{
    m_dbManager->saveConfig("serverUrl", m_subsonicApi->serverUrl());
    m_dbManager->saveConfig("username", m_subsonicApi->username());
    m_dbManager->saveConfig("password", m_subsonicApi->password());
    m_dbManager->saveConfig("songCacheSizeMb", m_cacheManager->maxSongCacheSizeMb());
    m_dbManager->saveConfig("songCachePath", m_cacheManager->songCacheDirectory());
    m_dbManager->saveConfig("albumPageSize", m_albumPageSize);
    m_dbManager->saveConfig("cacheTtlMinutes",
                            static_cast<int>(qMax<qint64>(1, m_cacheTtlSecs / 60)));
}

void MainWindow::savePlaybackSession()
{
    const QVector<Song> playlist = m_audioPlayer->playlist();
    const int currentIndex = m_audioPlayer->currentIndex();
    const qint64 position = m_audioPlayer->position();
    const int playMode = static_cast<int>(m_audioPlayer->playMode());
    m_dbManager->savePlaybackSession(playlist, currentIndex, position, playMode);
}

void MainWindow::restorePlaybackSession()
{
    if (!m_subsonicApi->isConfigured()) {
        return;
    }

    QVector<Song> songs;
    int currentIndex = -1;
    qint64 position = 0;
    int playMode = 0;
    if (!m_dbManager->loadPlaybackSession(&songs, &currentIndex, &position, &playMode)) {
        return;
    }

    QStringList streamUrls;
    streamUrls.reserve(songs.size());
    for (const Song &song : songs) {
        streamUrls.append(resolvePlaybackSource(song));
    }

    m_audioPlayer->restoreSession(songs, streamUrls, currentIndex, position,
                                  static_cast<AudioPlayer::PlayMode>(playMode));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    savePlaybackSession();
    saveSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::onNavigationItemSelected(NavigationWidget::NavigationItem item)
{
    switch (item) {
    case NavigationWidget::Home:
        m_contentStack->setCurrentWidget(m_homeWidget);
        showHome();
        break;
    case NavigationWidget::Favorites:
        m_contentStack->setCurrentWidget(m_favoritesWidget);
        showFavorites();
        break;
    case NavigationWidget::Albums:
        m_contentStack->setCurrentWidget(m_albumsPage);
        showAlbums();
        break;
    case NavigationWidget::Artists:
        m_contentStack->setCurrentWidget(m_artistsPage);
        showArtists();
        break;
    case NavigationWidget::Search:
        m_contentStack->setCurrentWidget(m_searchWidget);
        showSearch();
        break;
    case NavigationWidget::Playlist:
        m_contentStack->setCurrentWidget(m_playlistWidget);
        showPlaylist();
        break;
    case NavigationWidget::Settings:
        m_contentStack->setCurrentWidget(m_settingsWidget);
        showSettings();
        break;
    }
}

void MainWindow::showHome()
{
    if (!m_subsonicApi->isConfigured()) {
        m_homeWidget->resetSections();
        return;
    }

    refreshHomeSections();
}

void MainWindow::showPlaylist()
{
    // Playlist is already populated by m_playlistWidget
}

void MainWindow::onPlaylistRemoveSong(int index)
{
    m_audioPlayer->removeFromPlaylist(index);
}

void MainWindow::onSongSelected(const Song &song)
{
    onPlaySong(song);
}

void MainWindow::onPlaySong(const Song &song)
{
    if (!m_subsonicApi->isConfigured()) {
        QMessageBox::warning(this, "提示", "请先配置 Subsonic 服务器！");
        return;
    }

    QString streamUrl = resolvePlaybackSource(song);
    m_audioPlayer->play(song, streamUrl);
    m_playerControlBar->onSongChanged(song);
    ensureSongCached(song);

    // Add to history
    m_dbManager->addToHistory(song);
}

void MainWindow::onSeekRequested(qint64 position)
{
    m_audioPlayer->seek(position);
}

void MainWindow::onSettingsTriggered()
{
    m_navigationWidget->setCurrentItem(NavigationWidget::Settings);
    m_contentStack->setCurrentWidget(m_settingsWidget);
    showSettings();
}

void MainWindow::onSettingsSaved(const QString &serverUrl, const QString &username, const QString &password,
                                 int cacheSizeMb, int cacheTtlMinutes, int albumPageSize, const QString &cacheDirectory)
{
    const QString oldServerUrl = m_subsonicApi->serverUrl();
    const int oldAlbumPageSize = m_albumPageSize;

    m_subsonicApi->setServerUrl(serverUrl);
    m_subsonicApi->setUsername(username);
    m_subsonicApi->setPassword(password);

    const bool serverUrlChanged = (oldServerUrl != m_subsonicApi->serverUrl());

    m_cacheManager->setMaxSongCacheSizeMb(cacheSizeMb);
    m_cacheManager->setSongCacheDirectory(cacheDirectory);
    m_albumPageSize = qBound(10, albumPageSize, 50);
    m_cacheTtlSecs = qMax<qint64>(60, static_cast<qint64>(cacheTtlMinutes) * 60);
    saveSettings();
    m_settingsWidget->setCurrentCacheSize(m_cacheManager->songCacheSize());
    m_subsonicApi->ping();

    if (oldAlbumPageSize != m_albumPageSize) {
        m_albumPageCache.clear();
        m_albumPageInFlight.clear();
        m_currentAlbumPage = 0;
        m_requestedAlbumPage = 0;
        m_hasNextAlbumPage = false;
        updateAlbumPaginationControls();
        if (m_contentStack->currentWidget() == m_albumsPage && m_subsonicApi->isConfigured()) {
            requestAlbumPage(0);
        }
    }

    if (serverUrlChanged) {
        m_audioPlayer->clearPlaylist();

        m_pendingAlbumRequestMode = None;
        m_pendingAlbumId.clear();

        m_homeInFlightTypes.clear();
        m_homeLastSuccessTs.clear();
        m_albumPageCache.clear();
        m_albumPageInFlight.clear();
        m_artistCache.clear();
        m_artistCacheLastUpdatedSecs = 0;
        m_artistRequestInFlight = false;

        m_homeWidget->resetSections();
        m_albumWidget->setAlbums(QVector<Album>());
        m_artistWidget->setArtists(QVector<Artist>());
        if (m_artistAlbumsWidget) {
            m_artistAlbumsWidget->setAlbums(QVector<Album>());
        }

        m_currentAlbumPage = 0;
        m_requestedAlbumPage = 0;
        m_hasNextAlbumPage = false;
        updateAlbumPaginationControls();

        if (m_contentStack->currentWidget() == m_albumDetailWidget
                || m_contentStack->currentWidget() == m_artistAlbumsPage) {
            m_contentStack->setCurrentWidget(m_albumsPage);
            m_navigationWidget->setCurrentItem(NavigationWidget::Albums);
        }

        showHome();
        refreshStarred();
        showFavorites();
        showArtists();
        requestAlbumPage(0);

        m_statusLabel->setText("服务器地址已变更，已刷新页面并清空播放队列");
        return;
    }

    refreshStarred();
    m_statusLabel->setText("设置已保存");
}

void MainWindow::showFavorites()
{
    if (m_subsonicApi->isConfigured()) {
        m_favoritesWidget->refresh();
    }
}

void MainWindow::showAlbums()
{
    if (m_subsonicApi->isConfigured()) {
        requestAlbumPage(m_currentAlbumPage);
    }
}

void MainWindow::showArtists()
{
    if (m_subsonicApi->isConfigured()) {
        if (!m_artistCache.isEmpty()) {
            applyArtistCache();
        }
        requestArtistsRefresh(!m_artistCache.isEmpty());
    }
}

void MainWindow::showSearch()
{
    // Search widget handles UI
}

void MainWindow::showSettings()
{
    m_settingsWidget->setCurrentCacheSize(m_cacheManager->songCacheSize());
}

void MainWindow::onArtistsReceived(const QVector<Artist> &artists)
{
    qDebug() << "Received" << artists.size() << "artists";
    m_artistRequestInFlight = false;
    m_artistCache = artists;
    m_artistCacheLastUpdatedSecs = QDateTime::currentSecsSinceEpoch();
    if (m_contentStack->currentWidget() == m_artistsPage) {
        applyArtistCache();
    }
    m_statusLabel->setText(QString("已加载 %1 位艺术家").arg(artists.size()));
}

void MainWindow::onArtistsFailed(const QString &error)
{
    m_artistRequestInFlight = false;
    if (m_artistCache.isEmpty()) {
        m_statusLabel->setText(QString("艺术家加载失败: %1").arg(error));
    } else {
        m_statusLabel->setText("艺术家缓存已保留，后台刷新失败");
    }
}

void MainWindow::onArtistAlbumsReceived(const Artist &artist, const QVector<Album> &albums)
{
    m_currentArtist = artist;
    m_artistAlbumsLabel->setText(QString("%1 的专辑").arg(artist.name()));
    m_artistAlbumsWidget->setAlbums(albums);
    m_contentStack->setCurrentWidget(m_artistAlbumsPage);
    m_statusLabel->setText(QString("已加载 %1 的 %2 张专辑").arg(artist.name()).arg(albums.size()));
}

void MainWindow::onAlbumsReceived(int offset, int size, const QVector<Album> &albums)
{
    qDebug() << "Received" << albums.size() << "albums";
    const int pageSize = size > 0 ? size : m_albumPageSize;
    const int pageIndex = pageSize > 0 ? offset / pageSize : 0;
    m_albumPageInFlight.remove(pageIndex);

    if (albums.isEmpty() && pageIndex > 0) {
        m_hasNextAlbumPage = false;
        updateAlbumPaginationControls();
        m_statusLabel->setText("已到最后一页");
        return;
    }

    AlbumPageCacheEntry entry;
    entry.albums = albums;
    entry.lastUpdatedSecs = QDateTime::currentSecsSinceEpoch();
    entry.hasNextPage = (albums.size() >= m_albumPageSize);
    m_albumPageCache.insert(pageIndex, entry);

    if (m_contentStack->currentWidget() == m_albumsPage
            && (pageIndex == m_currentAlbumPage || pageIndex == m_requestedAlbumPage)) {
        m_currentAlbumPage = pageIndex;
        applyAlbumPageCache(pageIndex);
    }

    m_statusLabel->setText(QString("第 %1 页，已加载 %2 张专辑")
                           .arg(pageIndex + 1)
                           .arg(albums.size()));

    if (albums.size() >= m_albumPageSize) {
        requestAlbumPageRefresh(pageIndex + 1, true);
    }
}

void MainWindow::onAlbumsFailed(int offset, int size, const QString &error)
{
    const int pageSize = size > 0 ? size : m_albumPageSize;
    const int pageIndex = pageSize > 0 ? offset / pageSize : 0;
    m_albumPageInFlight.remove(pageIndex);

    if (m_albumPageCache.contains(pageIndex)) {
        m_statusLabel->setText(QString("第 %1 页缓存已保留，后台刷新失败").arg(pageIndex + 1));
        return;
    }

    if (pageIndex == m_requestedAlbumPage) {
        m_albumPageLabel->setText(QString("第 %1 页").arg(pageIndex + 1));
        updateAlbumPaginationControls();
    }
    m_statusLabel->setText(QString("专辑加载失败: %1").arg(error));
}

void MainWindow::onHomeAlbumListReceived(const QString &type, const QVector<Album> &albums)
{
    HomeWidget::SectionType sectionType = HomeWidget::RecentAlbums;
    QString sectionTitle;
    if (!mapHomeSectionType(type, &sectionType, &sectionTitle)) {
        return;
    }

    m_homeInFlightTypes.remove(type);
    m_homeLastSuccessTs.insert(type, QDateTime::currentSecsSinceEpoch());

    const QVector<Album> filteredAlbums = filterHomeAlbums(albums);
    m_homeWidget->setSectionAlbums(sectionType, filteredAlbums);
    m_statusLabel->setText(QString("%1已加载 %2 张专辑").arg(sectionTitle).arg(filteredAlbums.size()));
}

void MainWindow::onHomeAlbumListFailed(const QString &type, const QString &error)
{
    HomeWidget::SectionType sectionType = HomeWidget::RecentAlbums;
    QString sectionTitle;
    if (!mapHomeSectionType(type, &sectionType, &sectionTitle)) {
        return;
    }

    m_homeInFlightTypes.remove(type);

    const bool hasCache = m_homeLastSuccessTs.contains(type);
    if (!hasCache) {
        m_homeWidget->setSectionMessage(sectionType,
                                        error.isEmpty() ? "加载失败，请稍后重试" : QString("加载失败：%1").arg(error),
                                        true);
    }
    m_statusLabel->setText(QString("%1加载失败").arg(sectionTitle));
}

void MainWindow::onAlbumReceived(const Album &album, const QVector<Song> &songs)
{
    qDebug() << "Album received:" << album.name() << "with" << songs.size() << "songs";
    if (m_pendingAlbumRequestMode != None && album.id() != m_pendingAlbumId) {
        return;
    }

    if (m_pendingAlbumRequestMode == PlayFromStart) {
        m_pendingAlbumRequestMode = None;
        m_pendingAlbumId.clear();

        if (songs.isEmpty()) {
            return;
        }

        QStringList streamUrls;
        streamUrls.reserve(songs.size());
        for (const Song &song : songs) {
            streamUrls.append(resolvePlaybackSource(song));
        }
        m_audioPlayer->playPlaylist(songs, streamUrls, 0);
        return;
    }

    m_pendingAlbumRequestMode = None;
    m_pendingAlbumId.clear();

    m_currentAlbum = album;
    m_albumDetailWidget->setAlbum(album, songs);
    bool hasSongStarred = false;
    for (const Song &song : songs) {
        if (song.isFavorite() || isSongStarred(song.id())) {
            hasSongStarred = true;
            break;
        }
    }
    m_albumDetailWidget->setAlbumFavorite(isAlbumStarred(album.id()) || hasSongStarred);
    m_contentStack->setCurrentWidget(m_albumDetailWidget);
}

void MainWindow::onSongsReceived(const QVector<Song> &songs)
{
    Q_UNUSED(songs);
}

void MainWindow::onPingFinished(bool success, const QString &error)
{
    if (success) {
        qDebug() << "Ping successful";
        m_statusLabel->setText("服务器连接成功");
    } else {
        qDebug() << "Ping failed:" << error;
        m_statusLabel->setText(QString("服务器连接失败: %1").arg(error));
        QMessageBox::warning(this, "连接失败", QString("无法连接到服务器: %1").arg(error));
    }
}

void MainWindow::onApiError(const QString &error)
{
    qDebug() << "API error:" << error;
    m_statusLabel->setText(QString("错误: %1").arg(error));
    updateAlbumPaginationControls();
    QMessageBox::warning(this, "API 错误", error);
}

void MainWindow::showAlbumDetail(const Album &album)
{
    qDebug() << "Showing album detail for:" << album.name();
    m_pendingAlbumRequestMode = ShowDetail;
    m_pendingAlbumId = album.id();
    m_subsonicApi->getAlbum(album.id());
}

void MainWindow::requestAlbumPage(int pageIndex)
{
    if (!m_subsonicApi->isConfigured() || pageIndex < 0) {
        return;
    }

    m_requestedAlbumPage = pageIndex;
    const bool hasCache = m_albumPageCache.contains(pageIndex);
    if (hasCache) {
        m_currentAlbumPage = pageIndex;
        applyAlbumPageCache(pageIndex);
        requestAlbumPageRefresh(pageIndex, true);
        return;
    }

    m_albumPrevButton->setEnabled(false);
    m_albumNextButton->setEnabled(false);
    m_albumPageLabel->setText(QString("第 %1 页，加载中...").arg(pageIndex + 1));
    requestAlbumPageRefresh(pageIndex, false);
}

void MainWindow::refreshHomeSections()
{
    const struct Request {
        const char *type;
        HomeWidget::SectionType sectionType;
    } requests[] = {
        { "recent", HomeWidget::RecentAlbums },
        { "newest", HomeWidget::NewestAlbums },
        { "frequent", HomeWidget::FrequentAlbums },
        { "random", HomeWidget::RandomAlbums }
    };

    for (const Request &request : requests) {
        const QString type = QString::fromLatin1(request.type);
        if (m_homeInFlightTypes.contains(type)) {
            continue;
        }

        const bool hasCache = m_homeLastSuccessTs.contains(type);
        const bool stale = !hasCache || isCacheExpired(m_homeLastSuccessTs.value(type));
        if (!stale) {
            continue;
        }

        if (!hasCache) {
            m_homeWidget->setSectionLoading(request.sectionType, true);
        }

        m_homeInFlightTypes.insert(type);
        m_subsonicApi->getAlbumListByType(type, 20, 0);

        QTimer::singleShot(kHomeSectionTimeoutMs, this, [this, type, request]() {
            if (!m_homeInFlightTypes.contains(type)) {
                return;
            }

            m_homeInFlightTypes.remove(type);
            const bool hasCache = m_homeLastSuccessTs.contains(type);
            if (!hasCache) {
                m_homeWidget->setSectionMessage(request.sectionType, "加载超时，请稍后重试", true);
            }
        });
    }

    m_statusLabel->setText("正在刷新主页内容...");
}

bool MainWindow::isCacheExpired(qint64 lastUpdatedSecs) const
{
    return isExpiredByTtl(lastUpdatedSecs, m_cacheTtlSecs);
}

void MainWindow::applyAlbumPageCache(int pageIndex)
{
    const auto it = m_albumPageCache.constFind(pageIndex);
    if (it == m_albumPageCache.constEnd()) {
        return;
    }

    m_currentAlbumPage = pageIndex;
    m_hasNextAlbumPage = it->hasNextPage;
    m_albumWidget->setAlbums(it->albums);
    updateAlbumPaginationControls();
}

void MainWindow::requestAlbumPageRefresh(int pageIndex, bool silentIfCached)
{
    if (!m_subsonicApi->isConfigured() || pageIndex < 0 || m_albumPageInFlight.contains(pageIndex)) {
        return;
    }

    const auto it = m_albumPageCache.constFind(pageIndex);
    const bool hasCache = it != m_albumPageCache.constEnd();
    if (hasCache && !isCacheExpired(it->lastUpdatedSecs)) {
        return;
    }

    m_albumPageInFlight.insert(pageIndex);
    if (hasCache && silentIfCached) {
        m_statusLabel->setText(QString("第 %1 页缓存已命中，后台刷新中...").arg(pageIndex + 1));
    }
    m_subsonicApi->getAlbums2(m_albumPageSize, pageIndex * m_albumPageSize);
}

void MainWindow::applyArtistCache()
{
    m_artistWidget->setArtists(m_artistCache);
    m_artistWidget->setFavoriteArtistIds(m_starredArtistIds);
}

void MainWindow::requestArtistsRefresh(bool silentIfCached)
{
    if (!m_subsonicApi->isConfigured() || m_artistRequestInFlight) {
        return;
    }

    const bool hasCache = !m_artistCache.isEmpty();
    if (hasCache && !isCacheExpired(m_artistCacheLastUpdatedSecs)) {
        return;
    }

    m_artistRequestInFlight = true;
    if (hasCache && silentIfCached) {
        m_statusLabel->setText("艺术家缓存已命中，后台刷新中...");
    }
    m_subsonicApi->getArtists();
}

QVector<Album> MainWindow::filterHomeAlbums(const QVector<Album> &albums) const
{
    QVector<Album> filtered;
    filtered.reserve(qMin(albums.size(), 20));

    QSet<QString> seenAlbums;
    for (const Album &album : albums) {
        const QString id = album.id().trimmed();
        const QString name = album.name().trimmed();
        const QString artist = album.artist().trimmed();
        const QString uniqueKey = !id.isEmpty() ? id : QString("%1|%2").arg(name, artist);

        if (uniqueKey.trimmed().isEmpty() || seenAlbums.contains(uniqueKey)) {
            continue;
        }

        seenAlbums.insert(uniqueKey);
        filtered.append(album);
        if (filtered.size() >= 20) {
            break;
        }
    }

    return filtered;
}

bool MainWindow::mapHomeSectionType(const QString &type, HomeWidget::SectionType *sectionType, QString *sectionTitle) const
{
    if (!sectionType) {
        return false;
    }

    if (type == "recent") {
        *sectionType = HomeWidget::RecentAlbums;
        if (sectionTitle) {
            *sectionTitle = "最近播放的专辑";
        }
        return true;
    }
    if (type == "newest") {
        *sectionType = HomeWidget::NewestAlbums;
        if (sectionTitle) {
            *sectionTitle = "最新添加的专辑";
        }
        return true;
    }
    if (type == "frequent") {
        *sectionType = HomeWidget::FrequentAlbums;
        if (sectionTitle) {
            *sectionTitle = "播放最多的专辑";
        }
        return true;
    }
    if (type == "random") {
        *sectionType = HomeWidget::RandomAlbums;
        if (sectionTitle) {
            *sectionTitle = "随机专辑";
        }
        return true;
    }

    return false;
}

void MainWindow::updateAlbumPaginationControls()
{
    m_albumPrevButton->setEnabled(m_currentAlbumPage > 0);
    m_albumNextButton->setEnabled(m_hasNextAlbumPage);
    m_albumPageLabel->setText(QString("第 %1 页").arg(m_currentAlbumPage + 1));
}

QString MainWindow::resolvePlaybackSource(const Song &song) const
{
    const QString cachedPath = m_cacheManager->cachedSongPath(song.id(), song.suffix());
    if (!cachedPath.isEmpty()) {
        return cachedPath;
    }
    return m_subsonicApi->getStreamUrl(song.id());
}

void MainWindow::ensureSongCached(const Song &song)
{
    if (song.id().isEmpty()) {
        return;
    }

    if (!m_cacheManager->cachedSongPath(song.id(), song.suffix()).isEmpty()) {
        m_audioPlayer->updateSongSource(song.id(), m_cacheManager->cachedSongPath(song.id(), song.suffix()));
        return;
    }

    m_cacheManager->cacheSong(song.id(), m_subsonicApi->getStreamUrl(song.id()), song.suffix());
}

void MainWindow::onSongCached(const QString &songId, const QString &localPath)
{
    m_audioPlayer->updateSongSource(songId, localPath);
    m_settingsWidget->setCurrentCacheSize(m_cacheManager->songCacheSize());
}

void MainWindow::refreshStarred()
{
    if (!m_subsonicApi->isConfigured()) {
        return;
    }
    m_subsonicApi->getStarred();
}

bool MainWindow::isSongStarred(const QString &songId) const
{
    if (songId.isEmpty()) {
        return false;
    }
    return m_starredSongIds.contains(songId);
}

void MainWindow::onStarredReceived(const QVector<Song> &songs, const QVector<Album> &albums, const QVector<Artist> &artists)
{
    m_starredSongIds.clear();
    m_starredAlbumIds.clear();
    m_starredArtistIds.clear();
    m_albumIdsFromStarredSongs.clear();

    for (const Song &song : songs) {
        if (!song.id().isEmpty()) {
            m_starredSongIds.insert(song.id());
        }
        if (!song.albumId().isEmpty()) {
            m_albumIdsFromStarredSongs.insert(song.albumId());
        }
    }
    for (const Album &album : albums) {
        if (!album.id().isEmpty()) {
            m_starredAlbumIds.insert(album.id());
        }
    }
    for (const Artist &artist : artists) {
        if (!artist.id().isEmpty()) {
            m_starredArtistIds.insert(artist.id());
        }
    }

    if (m_favoritesWidget) {
        m_favoritesWidget->onStarredReceived(songs, albums, artists);
    }

    if (m_artistWidget) {
        m_artistWidget->setFavoriteArtistIds(m_starredArtistIds);
    }

    if (m_audioPlayer) {
        m_playerControlBar->setSongFavorite(isSongStarred(m_audioPlayer->currentSong().id()));
    }

    if (m_contentStack->currentWidget() == m_albumDetailWidget && !m_currentAlbum.id().isEmpty()) {
        const bool derivedFromSongs = m_albumIdsFromStarredSongs.contains(m_currentAlbum.id());
        m_albumDetailWidget->setAlbumFavorite(isAlbumStarred(m_currentAlbum.id()) || derivedFromSongs);
    }
}

void MainWindow::onSongFavoriteClicked(const Song &song)
{
    if (!m_subsonicApi->isConfigured() || song.id().isEmpty()) {
        return;
    }

    const bool shouldStar = !isSongStarred(song.id());
    if (shouldStar) {
        m_starredSongIds.insert(song.id());
        m_subsonicApi->star(QStringList() << song.id());
    } else {
        m_starredSongIds.remove(song.id());
        m_subsonicApi->unstar(QStringList() << song.id());
    }

    m_playerControlBar->setSongFavorite(shouldStar);
    m_dbManager->logEvent("song.favorite_toggle",
                          QMap<QString, QString>{
                              { "song_id", song.id() },
                              { "star", shouldStar ? "1" : "0" }
                          });
    refreshStarred();
}

bool MainWindow::isAlbumStarred(const QString &albumId) const
{
    if (albumId.isEmpty()) {
        return false;
    }
    return m_starredAlbumIds.contains(albumId);
}

bool MainWindow::isArtistStarred(const QString &artistId) const
{
    if (artistId.isEmpty()) {
        return false;
    }
    return m_starredArtistIds.contains(artistId);
}

void MainWindow::onAlbumFavoriteClicked(const Album &album)
{
    if (!m_subsonicApi->isConfigured() || album.id().isEmpty()) {
        return;
    }

    const bool shouldStar = !isAlbumStarred(album.id());
    if (shouldStar) {
        m_starredAlbumIds.insert(album.id());
        m_subsonicApi->star(QStringList(), QStringList() << album.id(), QStringList());
    } else {
        m_starredAlbumIds.remove(album.id());
        m_subsonicApi->unstar(QStringList(), QStringList() << album.id(), QStringList());
    }

    const bool derivedFromSongs = m_albumIdsFromStarredSongs.contains(album.id());
    m_albumDetailWidget->setAlbumFavorite(shouldStar || derivedFromSongs);
    m_dbManager->logEvent("album.favorite_toggle",
                          QMap<QString, QString>{
                              { "album_id", album.id() },
                              { "star", shouldStar ? "1" : "0" },
                              { "derived_from_songs", derivedFromSongs ? "1" : "0" }
                          });
    refreshStarred();
}

void MainWindow::onArtistFavoriteClicked(const Artist &artist)
{
    if (!m_subsonicApi->isConfigured() || artist.id().isEmpty()) {
        return;
    }

    const bool shouldStar = !isArtistStarred(artist.id());
    if (shouldStar) {
        m_starredArtistIds.insert(artist.id());
        m_subsonicApi->star(QStringList(), QStringList(), QStringList() << artist.id());
    } else {
        m_starredArtistIds.remove(artist.id());
        m_subsonicApi->unstar(QStringList(), QStringList(), QStringList() << artist.id());
    }

    if (m_artistWidget) {
        m_artistWidget->setFavoriteArtistIds(m_starredArtistIds);
    }
    m_dbManager->logEvent("artist.favorite_toggle",
                          QMap<QString, QString>{
                              { "artist_id", artist.id() },
                              { "star", shouldStar ? "1" : "0" }
                          });
    refreshStarred();
}

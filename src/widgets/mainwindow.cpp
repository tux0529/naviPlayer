/**
 * @file    mainwindow.cpp
 * @brief   MainWindow 实现
 */
#include "mainwindow.h"

#include "controlbar.h"
#include "navwidget.h"
#include "titlebar.h"

// ---- Pages -----
#include "settingspage.h"
#include "pages/homepage.h"
#include "pages/albumsview.h"
#include "pages/artistsview.h"
#include "pages/songspage.h"
#include "pages/albumdetailpage.h"
#include "pages/artistdetailpage.h"
#include "pages/playlistspage.h"
#include "pages/playlistdetailpage.h"
#include "pages/cachedsongspage.h"
#include "pages/searchpage.h"

// ---- Components ----
#include "components/playbackqueuepanel.h"
#include "components/lyricswindow.h"
#include "components/songdetailsdialog.h"

// ---- Core ----
#include "core/appcontext.h"
#include "core/audioplayer.h"
#include "core/playbackqueue.h"
#include "core/subsonicapi.h"
#include "core/cachemanager.h"
#include "core/appsettings.h"
#include "core/lyricsmanager.h"

#ifndef Q_OS_MAC
#include "widgets.common/windowbutton.h"
#endif

#include <QCloseEvent>
#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QGuiApplication>
#include <QResizeEvent>
#include <QScreen>
#include <QSettings>
#include <QSplitter>
#include <QSplitterHandle>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWindow>
#include <QStackedWidget>
#include <QTranslator>
#include <QMessageBox>
#include <QTimer>

static const char *kSettingsGeometry  = "window/geometry";
static const char *kSettingsState     = "window/splitterState";
static const char *kSettingsMaximized = "window/maximized";

// 启动网络检测：首次可能赶上局域网权限/网络尚未就绪，ping 失败要重试
constexpr int kStartupPingIntervalMs  = 3000;
constexpr int kStartupPingMaxAttempts = 4;

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    currentTheme = Dark;

    // 1) 初始化 主窗口（内部会构建中央 UI）
    setupWindowKitAgent();

    // 2) 初始化主内容区的页面
    buildPages();
    connectPages();
    connectPlayer();

    m_stackedWidget->setCurrentIndex(PageHome);

    setWindowTitle(QStringLiteral("NaviPlayer"));
    setWindowIcon(QIcon(":/icons/naviPlayer.png"));

    resize(1180, 760);
    setMinimumSize(QSize(720, 420));

    // 恢复上次窗口大小/位置与分栏状态（必须在 resize 之后，否则会被默认尺寸覆盖）
    loadSettings();

    AppSettings *settings = AppContext::instance() ? AppContext::instance()->settings() : nullptr;
    applyTheme(settings && settings->theme() == AppSettings::Theme::Light ? Light : Dark);

    // 默认选中首页导航
    if (QPushButton *homeButton =
            m_navWidget->findChild<QPushButton *>(QStringLiteral("homeButton"))) {
        homeButton->setChecked(true);
    }

    // 启动后恢复上次会话 + 加载首页数据
    QTimer::singleShot(300, this, [this]() {
        restoreSessionState();
        if (m_homePage)
            m_homePage->refresh();
        AppContext *ctx = AppContext::instance();
        if (ctx) {
            ctx->refreshFavorites();
            ctx->refreshPlaylists();
            }

        // 启动网络检测：局域网权限/网络可能还没就绪，稍等再 ping，失败会重试几次
        QTimer::singleShot(1200, this, [this]() { runStartupNetworkCheck(); });
    });
}

void MainWindow::runStartupNetworkCheck()
{
    if (m_startupNetworkChecked)
        return;

    AppContext *ctx = AppContext::instance();
    if (!ctx || !ctx->api() || !ctx->api()->isConfigured()) {
        concludeStartupNetworkCheck(false);
        return;
    }

    if (m_startupPingAttempts >= kStartupPingMaxAttempts) {
        concludeStartupNetworkCheck(false);
        return;
    }

    ++m_startupPingAttempts;
    ctx->api()->ping();
    QTimer::singleShot(kStartupPingIntervalMs, this, &MainWindow::runStartupNetworkCheck);
}

MainWindow::~MainWindow()
{
    AppContext *ctx = AppContext::instance();
    if (ctx)
        ctx->shutdown();
}

/* ---------------- 关闭/大小事件 ---------------- */

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    saveSessionState();

    // 歌词窗口是 parent=nullptr 的独立顶层窗口：只要它还可见，
    // Qt 就不会因为"最后一个窗口关闭"而退出，所以退出前必须先关掉它
    if (m_lyricsWindow) {
        m_lyricsWindow->closeLyrics();
        m_lyricsWindow->deleteLater();
        m_lyricsWindow = nullptr;
    }

    QWidget::closeEvent(event);
}

/* ---------------- QWindowKit 接入 ---------------- */

void MainWindow::setupWindowKitAgent()
{
    m_windowAgent = new QWK::WidgetWindowAgent(this);
    const bool ok = m_windowAgent->setup(this);
    if (!ok) {
        qWarning("[MainWindow] QWK::WidgetWindowAgent::setup() failed.");
        return;
    }

    buildCentralUi();

    // 将整个窗口作为拖拽区：用户在空白背景区域拖拽即可移动窗口。
    m_windowAgent->setTitleBar(this);
    const auto handles = m_splitter->findChildren<QSplitterHandle *>();
    for (auto *h : handles) {
        m_windowAgent->setHitTestVisible(h, true);
    }
    m_windowAgent->setHitTestVisible(m_navWidget, true);
    m_windowAgent->setHitTestVisible(m_stackedWidget, true);
    m_windowAgent->setHitTestVisible(m_queuePanel, true);

    const auto www = m_controlBar->getHitTestVisibleWidget();
    for (auto *w : www) {
        m_windowAgent->setHitTestVisible(w, true);
    }
}

/* ---------------- UI 结构 ---------------- */

void MainWindow::buildCentralUi()
{
    m_rootLayout = new QVBoxLayout(this);
    m_rootLayout->setContentsMargins(0, 0, 0, 0);
    m_rootLayout->setSpacing(0);

    // 水平 Splitter（直接填满整个窗口）
    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->setObjectName(QStringLiteral("mainSplitter"));
    m_splitter->setChildrenCollapsible(false);
    m_splitter->setHandleWidth(1);
    m_splitter->setOpaqueResize(true);

    // 构建左侧界面
    QWidget *leftPanel  = new QWidget(this);
    leftPanel->setObjectName(QStringLiteral("leftPanel"));
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setObjectName(QStringLiteral("leftLayout"));
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);
    m_titleBar = new TitleBar(this);
    m_titleBar->setObjectName(QStringLiteral("titleBar"));
    installEventFilter(m_titleBar);
    m_navWidget = new NavWidget(this);
    m_navWidget->setObjectName(QStringLiteral("navWidget"));
    connect(m_navWidget, &NavWidget::buttonClicked, this, &MainWindow::onNavButtonClicked);
    leftLayout->addWidget(m_titleBar, 0);
    leftLayout->addWidget(m_navWidget, 1);

    // 构建右侧面板
    QWidget *rightPanel = new QWidget(this);
    rightPanel->setObjectName(QStringLiteral("rightPanel"));
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setObjectName(QStringLiteral("rightLayout"));
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    m_controlBar = new ControlBar(this);
    m_controlBar->setObjectName(QStringLiteral("controlBar"));

    // 系统按钮（非 macOS）
#ifndef Q_OS_MAC
    QWidget *systemButtonWidget = new QWidget(this);
    systemButtonWidget->setObjectName(QStringLiteral("systemButtonWidget"));

    QHBoxLayout *systemButtonHLayout = new QHBoxLayout();
    systemButtonHLayout->setObjectName(QStringLiteral("systemButtonHLayout"));
    systemButtonHLayout->setContentsMargins(0, 0, 0, 0);
    systemButtonHLayout->setSpacing(0);

    auto minButton = new QWK::WindowButton();
    minButton->setObjectName(QStringLiteral("min-button"));
    minButton->setProperty("system-button", true);
    minButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    auto maxButton = new QWK::WindowButton();
    maxButton->setCheckable(true);
    maxButton->setObjectName(QStringLiteral("max-button"));
    maxButton->setProperty("system-button", true);
    maxButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    auto closeButton = new QWK::WindowButton();
    closeButton->setObjectName(QStringLiteral("close-button"));
    closeButton->setProperty("system-button", true);
    closeButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    systemButtonHLayout->addWidget(minButton);
    systemButtonHLayout->addWidget(maxButton);
    systemButtonHLayout->addWidget(closeButton);

    QVBoxLayout *systemButtonWidgetLayout = new QVBoxLayout(systemButtonWidget);
    systemButtonWidgetLayout->setObjectName(QStringLiteral("systemButtonWidgetLayout"));
    systemButtonWidgetLayout->setContentsMargins(0, 0, 0, 0);
    systemButtonWidgetLayout->setSpacing(0);
    systemButtonWidgetLayout->addLayout(systemButtonHLayout);
    systemButtonWidgetLayout->addItem(
        new QSpacerItem(20, 5, QSizePolicy::Minimum, QSizePolicy::Expanding));
    systemButtonWidget->setLayout(systemButtonWidgetLayout);

    m_windowAgent->setSystemButton(QWK::WindowAgentBase::Minimize, minButton);
    m_windowAgent->setSystemButton(QWK::WindowAgentBase::Maximize, maxButton);
    m_windowAgent->setSystemButton(QWK::WindowAgentBase::Close, closeButton);

    m_controlBar->setSystemButtonWidget(systemButtonWidget);
    connect(minButton, &QAbstractButton::clicked, this, &QWidget::showMinimized);
    connect(maxButton, &QAbstractButton::clicked, this, [this, maxButton](bool max) {
        if (max) {
            showMaximized();
            maxButton->setChecked(true);
        } else {
            showNormal();
            maxButton->setChecked(false);
        }
    });
    connect(closeButton, &QWK::WindowButton::clicked, this, &QWidget::close);
#endif

    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->setObjectName(QStringLiteral("stackedWidget"));

    // 播放队列面板（可折叠）
    m_queuePanel = new PlaybackQueuePanel(this);
    m_queuePanel->setObjectName(QStringLiteral("queuePanel"));
    m_queuePanel->setVisible(false);

    m_contentSplitter = new QSplitter(Qt::Horizontal, this);
    m_contentSplitter->setObjectName(QStringLiteral("contentSplitter"));
    m_contentSplitter->setChildrenCollapsible(false);
    m_contentSplitter->addWidget(m_stackedWidget);
    m_contentSplitter->addWidget(m_queuePanel);
    m_contentSplitter->setStretchFactor(0, 3);
    m_contentSplitter->setStretchFactor(1, 1);
    m_contentSplitter->setHandleWidth(1);

    // 歌词悬浮窗
    m_lyricsWindow = new LyricsWindow(nullptr);

    // 底部状态提示（连接状态 / 错误）
    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName(QStringLiteral("statusLabel"));
    m_statusLabel->setContentsMargins(8, 4, 8, 4);
    m_statusLabel->setVisible(false);
    m_statusTimer = new QTimer(this);
    m_statusTimer->setSingleShot(true);
    connect(m_statusTimer, &QTimer::timeout, this, [this]() {
        m_statusLabel->setVisible(false);
    });

    rightLayout->addWidget(m_controlBar, 0);
    rightLayout->addWidget(m_contentSplitter, 1);
    rightLayout->addWidget(m_statusLabel, 0);

    // 最小宽度保护
    leftPanel->setMinimumWidth(kLeftMinWidth);
    rightPanel->setMinimumWidth(kRightMinWidth);

    m_splitter->addWidget(leftPanel);
    m_splitter->addWidget(rightPanel);

    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 3);

    m_rootLayout->addWidget(m_splitter, 1);
}

void MainWindow::buildPages()
{
    // 主页
    m_homePage = new HomePage(this);
    m_homePage->setObjectName(QStringLiteral("homePage"));
    m_stackedWidget->addWidget(m_homePage);

    // 专辑页
    m_albumsPage = new AlbumsView(this);
    m_albumsPage->setObjectName(QStringLiteral("albumsPage"));
    m_albumsPage->setSource(AlbumsView::Source::All);
    m_stackedWidget->addWidget(m_albumsPage);

    // 艺术家页
    m_artistsPage = new ArtistsView(this);
    m_artistsPage->setObjectName(QStringLiteral("artistsPage"));
    m_artistsPage->setSource(ArtistsView::Source::All);
    m_stackedWidget->addWidget(m_artistsPage);

    // 全部歌曲
    m_songsPage = new SongsPage(this);
    m_songsPage->setObjectName(QStringLiteral("songsPage"));
    m_songsPage->setSource(SongsPage::Source::All);
    m_stackedWidget->addWidget(m_songsPage);

    // 收藏歌曲
    m_favSongsPage = new SongsPage(this);
    m_favSongsPage->setObjectName(QStringLiteral("favSongsPage"));
    m_favSongsPage->setSource(SongsPage::Source::Favorite);
    m_stackedWidget->addWidget(m_favSongsPage);

    // 收藏专辑
    m_favAlbumsPage = new AlbumsView(this);
    m_favAlbumsPage->setObjectName(QStringLiteral("favAlbumsPage"));
    m_favAlbumsPage->setSource(AlbumsView::Source::Favorite);
    m_stackedWidget->addWidget(m_favAlbumsPage);

    // 收藏艺术家
    m_favArtistsPage = new ArtistsView(this);
    m_favArtistsPage->setObjectName(QStringLiteral("favArtistsPage"));
    m_favArtistsPage->setSource(ArtistsView::Source::Favorite);
    m_stackedWidget->addWidget(m_favArtistsPage);

    // 播放列表
    m_playlistsPage = new PlaylistsPage(this);
    m_playlistsPage->setObjectName(QStringLiteral("playlistsPage"));
    m_stackedWidget->addWidget(m_playlistsPage);

    // 缓存歌曲
    m_cachedSongsPage = new CachedSongsPage(this);
    m_cachedSongsPage->setObjectName(QStringLiteral("cachedSongsPage"));
    m_stackedWidget->addWidget(m_cachedSongsPage);

    // 设置页
    m_settingsPage = new SettingsPage(this);
    m_settingsPage->setObjectName(QStringLiteral("settingsPage"));
    m_stackedWidget->addWidget(m_settingsPage);

    // 搜索结果
    m_searchPage = new SearchPage(this);
    m_searchPage->setObjectName(QStringLiteral("searchPage"));
    m_stackedWidget->addWidget(m_searchPage);

    // 艺术家详情
    m_artistDetailPage = new ArtistDetailPage(this);
    m_artistDetailPage->setObjectName(QStringLiteral("artistDetailPage"));
    m_stackedWidget->addWidget(m_artistDetailPage);

    // 专辑详情
    m_albumDetailPage = new AlbumDetailPage(this);
    m_albumDetailPage->setObjectName(QStringLiteral("albumDetailPage"));
    m_stackedWidget->addWidget(m_albumDetailPage);

    // 播放列表详情
    m_playlistDetailPage = new PlaylistDetailPage(this);
    m_playlistDetailPage->setObjectName(QStringLiteral("playlistDetailPage"));
    m_stackedWidget->addWidget(m_playlistDetailPage);
}

void MainWindow::connectPages()
{
    AppContext *ctx = AppContext::instance();

    // ---------- 主页 ----------
    connect(m_homePage, &HomePage::albumClicked, this, &MainWindow::showAlbumDetail);
    connect(m_homePage, &HomePage::artistClicked, this, &MainWindow::showArtistDetail);
    connect(m_homePage, &HomePage::playAlbumRequested, this, &MainWindow::onAlbumPlayRequested);

    // ---------- 专辑页 / 收藏专辑 ----------
    for (AlbumsView *view : {m_albumsPage, m_favAlbumsPage}) {
        connect(view, &AlbumsView::albumClicked, this, &MainWindow::showAlbumDetail);
        connect(view, &AlbumsView::artistClicked, this, &MainWindow::showArtistDetail);
        connect(view, &AlbumsView::playAlbumRequested, this, &MainWindow::onAlbumPlayRequested);
    }

    // ---------- 艺术家页 / 收藏艺术家 ----------
    for (ArtistsView *view : {m_artistsPage, m_favArtistsPage}) {
        connect(view, &ArtistsView::artistClicked, this, &MainWindow::showArtistDetail);
        connect(view, &ArtistsView::favoriteToggled, this, [this](const Artist &artist, bool fav) {
            AppContext::instance()->setArtistFavorite(artist, fav);
        });
    }

    // ---------- 艺术家详情 ----------
    connect(m_artistDetailPage, &ArtistDetailPage::albumClicked, this, &MainWindow::showAlbumDetail);
    connect(m_artistDetailPage, &ArtistDetailPage::playAlbumRequested, this,
            &MainWindow::onAlbumPlayRequested);
    connect(m_artistDetailPage, &ArtistDetailPage::favoriteToggled, this,
            [this](const Artist &artist, bool fav) {
                AppContext::instance()->setArtistFavorite(artist, fav);
            });

    // ---------- 专辑详情 ----------
    connect(m_albumDetailPage, &AlbumDetailPage::artistClicked, this, &MainWindow::showArtistDetail);
    connect(m_albumDetailPage, &AlbumDetailPage::addToQueueRequested, this,
            [this](const QVector<Song> &songs) {
                AppContext::instance()->queue()->append(songs);
            });
    connect(m_albumDetailPage, &AlbumDetailPage::addToNextRequested, this,
            [this](const QVector<Song> &songs) {
                AppContext::instance()->queue()->insertNext(songs);
            });
    connect(m_albumDetailPage, &AlbumDetailPage::favoriteToggled, this,
            [this](const Album &album, bool fav) {
                AppContext::instance()->setAlbumFavorite(album, fav);
            });
    connect(m_albumDetailPage, &AlbumDetailPage::playAllRequested, this, [this](const QVector<Song> &songs) {
        playSongs(songs, 0);
    });
    connect(m_albumDetailPage, &AlbumDetailPage::songActivated, this, [this](const Song &song) {
        AppContext::instance()->player()->playSong(song);
    });
    connect(m_albumDetailPage, &AlbumDetailPage::playNextRequested, this, [this](const Song &song) {
        AppContext::instance()->queue()->insertNext(song);
    });
    connect(m_albumDetailPage, &AlbumDetailPage::songFavoriteToggled, this,
            [this](const Song &song, bool fav) {
                AppContext::instance()->setSongFavorite(song, fav);
            });
    connect(m_albumDetailPage, &AlbumDetailPage::addToPlaylistRequested, this,
            [this](const Song &song, const QString &playlistId) {
                AppContext::instance()->api()->updatePlaylist(playlistId, QString(), QString(),
                                                              false, {song.id()});
            });
    connect(m_albumDetailPage, &AlbumDetailPage::downloadRequested, this, &MainWindow::downloadSong);
    connect(m_albumDetailPage, &AlbumDetailPage::detailsRequested, this, &MainWindow::showSongDetails);

    // ---------- 歌曲页 / 收藏歌曲 ----------
    for (SongsPage *page : {m_songsPage, m_favSongsPage}) {
        connect(page, &SongsPage::songActivated, this, [this](const Song &song) {
            AppContext::instance()->player()->playSong(song);
        });
        connect(page, &SongsPage::playNextRequested, this, [this](const Song &song) {
            AppContext::instance()->queue()->insertNext(song);
        });
        connect(page, &SongsPage::favoriteToggled, this, [this](const Song &song, bool fav) {
            AppContext::instance()->setSongFavorite(song, fav);
            m_songsPage->setFavorite(song.id(), fav);
            m_favSongsPage->setFavorite(song.id(), fav);
        });
        connect(page, &SongsPage::addToPlaylistRequested, this,
                [this](const Song &song, const QString &playlistId) {
                    AppContext::instance()->api()->updatePlaylist(playlistId, QString(), QString(),
                                                                  false, {song.id()});
                });
        connect(page, &SongsPage::downloadRequested, this, &MainWindow::downloadSong);
        connect(page, &SongsPage::detailsRequested, this, &MainWindow::showSongDetails);
    }

    // ---------- 播放列表 ----------
    connect(m_playlistsPage, &PlaylistsPage::playlistOpened, this, &MainWindow::showPlaylistDetail);

    connect(m_playlistDetailPage, &PlaylistDetailPage::playAllRequested, this,
            [this](const QVector<Song> &songs) { playSongs(songs, 0); });
    connect(m_playlistDetailPage, &PlaylistDetailPage::addToQueueRequested, this,
            [this](const QVector<Song> &songs) {
                AppContext::instance()->queue()->append(songs);
            });
    connect(m_playlistDetailPage, &PlaylistDetailPage::songActivated, this, [this](const Song &song) {
        AppContext::instance()->player()->playSong(song);
    });
    connect(m_playlistDetailPage, &PlaylistDetailPage::playNextRequested, this,
            [this](const Song &song) { AppContext::instance()->queue()->insertNext(song); });
    connect(m_playlistDetailPage, &PlaylistDetailPage::favoriteToggled, this,
            [this](const Song &song, bool fav) {
                AppContext::instance()->setSongFavorite(song, fav);
            });
    connect(m_playlistDetailPage, &PlaylistDetailPage::addToPlaylistRequested, this,
            [this](const Song &song, const QString &playlistId) {
                AppContext::instance()->api()->updatePlaylist(playlistId, QString(), QString(),
                                                              false, {song.id()});
            });
    connect(m_playlistDetailPage, &PlaylistDetailPage::downloadRequested, this,
            &MainWindow::downloadSong);
    connect(m_playlistDetailPage, &PlaylistDetailPage::detailsRequested, this,
            &MainWindow::showSongDetails);

    // ---------- 缓存歌曲 ----------
    connect(m_cachedSongsPage, &CachedSongsPage::playAllRequested, this,
            [this](const QVector<Song> &songs) { playSongs(songs, 0); });
    connect(m_cachedSongsPage, &CachedSongsPage::songActivated, this, [this](const Song &song) {
        AppContext::instance()->player()->playSong(song);
    });
    connect(m_cachedSongsPage, &CachedSongsPage::playNextRequested, this, [this](const Song &song) {
        AppContext::instance()->queue()->insertNext(song);
    });
    connect(m_cachedSongsPage, &CachedSongsPage::favoriteToggled, this,
            [this](const Song &song, bool fav) {
                AppContext::instance()->setSongFavorite(song, fav);
            });
    connect(m_cachedSongsPage, &CachedSongsPage::addToPlaylistRequested, this,
            [this](const Song &song, const QString &playlistId) {
                AppContext::instance()->api()->updatePlaylist(playlistId, QString(), QString(),
                                                              false, {song.id()});
            });
    connect(m_cachedSongsPage, &CachedSongsPage::downloadRequested, this, &MainWindow::downloadSong);
    connect(m_cachedSongsPage, &CachedSongsPage::detailsRequested, this,
            &MainWindow::showSongDetails);

    // ---------- 搜索页 ----------
    connect(m_searchPage, &SearchPage::songActivated, this, [this](const Song &song) {
        AppContext::instance()->player()->playSong(song);
    });
    connect(m_searchPage, &SearchPage::playNextRequested, this, [this](const Song &song) {
        AppContext::instance()->queue()->insertNext(song);
    });
    connect(m_searchPage, &SearchPage::favoriteToggled, this, [this](const Song &song, bool fav) {
        AppContext::instance()->setSongFavorite(song, fav);
    });
    connect(m_searchPage, &SearchPage::addToPlaylistRequested, this,
            [this](const Song &song, const QString &playlistId) {
                AppContext::instance()->api()->updatePlaylist(playlistId, QString(), QString(),
                                                              false, {song.id()});
            });
    connect(m_searchPage, &SearchPage::downloadRequested, this, &MainWindow::downloadSong);
    connect(m_searchPage, &SearchPage::detailsRequested, this, &MainWindow::showSongDetails);
    connect(m_searchPage, &SearchPage::albumClicked, this, &MainWindow::showAlbumDetail);
    connect(m_searchPage, &SearchPage::artistClicked, this, &MainWindow::showArtistDetail);
    connect(m_searchPage, &SearchPage::playAlbumRequested, this, &MainWindow::onAlbumPlayRequested);

    // ---------- 设置页 ----------
    connect(m_settingsPage, &SettingsPage::themeChanged, this, [this](bool dark) {
        applyTheme(dark ? Dark : Light);
    });
    connect(m_settingsPage, &SettingsPage::languageChanged, this, &MainWindow::applyLanguage);

    // 歌词窗口：拖动调整位置
    if (m_lyricsWindow) {
        connect(m_settingsPage, &SettingsPage::lyricsPositionAdjustRequested, m_lyricsWindow,
                &LyricsWindow::setPositionAdjustEnabled);
        connect(m_lyricsWindow, &LyricsWindow::positionAdjustFinished, m_settingsPage,
                [this]() { m_settingsPage->setLyricsPositionAdjusting(false); });
    }

    Q_UNUSED(ctx)
}

void MainWindow::connectPlayer()
{
    AppContext *ctx = AppContext::instance();
    if (!ctx)
        return;

    AudioPlayer *player = ctx->player();
    PlaybackQueue *queue = ctx->queue();
    if (!player || !queue)
        return;

    m_controlBar->setPlayer(player);
    m_queuePanel->setQueue(queue);
    if (ctx->lyrics())
        m_lyricsWindow->setLyricsManager(ctx->lyrics());

    connect(m_controlBar, &ControlBar::backRequested, this, &MainWindow::goBack);
    connect(m_controlBar, &ControlBar::previousRequested, player, &AudioPlayer::previous);
    connect(m_controlBar, &ControlBar::nextRequested, player, &AudioPlayer::next);
    connect(m_controlBar, &ControlBar::playPauseRequested, player, &AudioPlayer::togglePlayPause);
    connect(m_controlBar, &ControlBar::seekRequested, player, &AudioPlayer::seek);
    connect(m_controlBar, &ControlBar::volumeChanged, player, &AudioPlayer::setVolume);
    connect(m_controlBar, &ControlBar::searchRequested, this, &MainWindow::onSearchRequested);
    connect(m_controlBar, &ControlBar::lyricsToggled, this, &MainWindow::toggleLyricsWindow);
    connect(m_controlBar, &ControlBar::queueToggled, this, &MainWindow::toggleQueuePanel);

    connect(m_queuePanel, &PlaybackQueuePanel::playRequested, player, &AudioPlayer::playQueueAt);

    connect(player, &AudioPlayer::currentSongChanged, this, [this, ctx](const Song &song) {
        m_songsPage->setPlayingSongId(song.id());
        m_favSongsPage->setPlayingSongId(song.id());
        if (ctx->lyrics())
            ctx->lyrics()->loadForSong(song);
    });

    connect(player, &AudioPlayer::positionChanged, this, [ctx](qint64 positionMs) {
        if (ctx->lyrics())
            ctx->lyrics()->updatePosition(positionMs);
    });

    connect(player, &AudioPlayer::errorOccurred, this, [this](const QString &message) {
        qWarning("[Player] %s", qPrintable(message));
        showStatusMessage(QStringLiteral("播放失败：%1").arg(message));
    });

    // 服务器错误 / 连接状态提示
    connect(ctx->api(), &SubsonicApi::errorOccurred, this, [this](const QString &message) {
        showStatusMessage(QStringLiteral("服务器请求失败：%1").arg(message));
    });
    connect(ctx->api(), &SubsonicApi::pingResult, this, [this](bool ok, const QString &message) {
        concludeStartupNetworkCheck(ok);     // 启动检测以 ping 结论为准
        showStatusMessage(ok ? QStringLiteral("已连接服务器（API %1）").arg(message)
                            : QStringLiteral("无法连接服务器：%1").arg(message),
                         5000);
    });

    // 断网：启动检测阶段弹窗选择；运行中只在底部提示
    connect(ctx->api(), &SubsonicApi::networkUnreachable, this,
            &MainWindow::onNetworkUnreachable);
    connect(ctx->api(), &SubsonicApi::connectionRestored, this, [this]() {
        concludeStartupNetworkCheck(true);
        showStatusMessage(QStringLiteral("网络连接已恢复"), 5000);
        // 恢复后补拉一次当前页数据
        if (auto *page = qobject_cast<PageBase *>(m_stackedWidget->currentWidget()))
            page->refresh();
    });

    // 播放整张专辑：等服务端返回歌曲列表后再入队播放
    connect(ctx->api(), &SubsonicApi::albumReady, this,
            [this](const Album &album, const QVector<Song> &songs) {
                if (m_pendingPlayAlbumId.isEmpty() || album.id() != m_pendingPlayAlbumId)
                    return;
                m_pendingPlayAlbumId.clear();
                if (!songs.isEmpty())
                    playSongs(songs, 0);
            });
}

/* ---------------- 导航 ---------------- */

void MainWindow::onNavButtonClicked(QPushButton *btn)
{
    if (!btn)
        return;

    const QString name = btn->objectName();
    if (name == QLatin1String("homeButton"))            navigateTo(PageHome);
    else if (name == QLatin1String("albumListButton"))  navigateTo(PageAlbums);
    else if (name == QLatin1String("artistButton"))     navigateTo(PageArtists);
    else if (name == QLatin1String("songButton"))       navigateTo(PageSongs);
    else if (name == QLatin1String("favSongButton"))    navigateTo(PageFavSongs);
    else if (name == QLatin1String("favAlbumButton"))   navigateTo(PageFavAlbums);
    else if (name == QLatin1String("favArtistButton"))  navigateTo(PageFavArtists);
    else if (name == QLatin1String("playlistButton"))   navigateTo(PagePlaylists);
    else if (name == QLatin1String("cachedSongButton")) navigateTo(PageCachedSongs);
    else if (name == QLatin1String("settingButton"))    navigateTo(PageSettings);
}

void MainWindow::navigateTo(Page page, bool addToHistory)
{
    if (m_stackedWidget->currentIndex() == page && page == m_currentPage) {
        // 同一页面再次点击：刷新数据
        if (auto *p = qobject_cast<PageBase *>(m_stackedWidget->currentWidget()))
            p->refresh();
        return;
    }

    if (addToHistory)
        m_pageHistory.push(m_currentPage);

    m_currentPage = page;
    m_stackedWidget->setCurrentIndex(page);

    if (auto *p = qobject_cast<PageBase *>(m_stackedWidget->currentWidget()))
        p->refresh();
}

void MainWindow::goBack()
{
    if (m_pageHistory.isEmpty())
        return;
    const int previous = m_pageHistory.pop();
    navigateTo(static_cast<Page>(previous), false);

    // 同步左侧导航选中态
    QPushButton *target = nullptr;
    switch (previous) {
    case PageHome:        target = m_navWidget->findChild<QPushButton *>(QStringLiteral("homeButton")); break;
    case PageAlbums:      target = m_navWidget->findChild<QPushButton *>(QStringLiteral("albumListButton")); break;
    case PageArtists:     target = m_navWidget->findChild<QPushButton *>(QStringLiteral("artistButton")); break;
    case PageSongs:       target = m_navWidget->findChild<QPushButton *>(QStringLiteral("songButton")); break;
    case PageFavSongs:    target = m_navWidget->findChild<QPushButton *>(QStringLiteral("favSongButton")); break;
    case PageFavAlbums:   target = m_navWidget->findChild<QPushButton *>(QStringLiteral("favAlbumButton")); break;
    case PageFavArtists:  target = m_navWidget->findChild<QPushButton *>(QStringLiteral("favArtistButton")); break;
    case PagePlaylists:   target = m_navWidget->findChild<QPushButton *>(QStringLiteral("playlistButton")); break;
    case PageCachedSongs: target = m_navWidget->findChild<QPushButton *>(QStringLiteral("cachedSongButton")); break;
    case PageSettings:    target = m_navWidget->findChild<QPushButton *>(QStringLiteral("settingButton")); break;
    default: break;
    }
    if (target)
        target->setChecked(true);
}

void MainWindow::showAlbumDetail(const Album &album)
{
    if (!album.isValid())
        return;
    m_albumDetailPage->setAlbum(album);
    navigateTo(PageAlbumInfo);
}

void MainWindow::showArtistDetail(const Artist &artist)
{
    if (!artist.isValid())
        return;
    m_artistDetailPage->setArtist(artist);
    navigateTo(PageArtistInfo);
}

void MainWindow::showPlaylistDetail(const Playlist &playlist)
{
    if (!playlist.isValid())
        return;
    m_playlistDetailPage->setPlaylist(playlist);
    navigateTo(PagePlaylistInfo);
}

/* ---------------- 交互动作 ---------------- */

void MainWindow::toggleQueuePanel()
{
    m_queuePanel->setVisible(!m_queuePanel->isVisible());
}

void MainWindow::toggleLyricsWindow()
{
    m_lyricsWindow->toggleVisible();
}

void MainWindow::playSongs(const QVector<Song> &songs, int startIndex)
{
    AppContext *ctx = AppContext::instance();
    if (ctx && ctx->player() && !songs.isEmpty())
        ctx->player()->playSongs(songs, startIndex);
}

void MainWindow::onNetworkUnreachable(const QString &message)
{
    // 启动检测尚未有结论：只记录，不提示也不弹窗。
    // macOS 局域网权限/网络可能在首帧还没就绪，此时失败多半是暂时的，ping 重试会再确认
    if (!m_startupNetworkChecked) {
        if (m_pendingOfflineMessage.isEmpty())
            m_pendingOfflineMessage = message;
        return;
    }

    // 运行中（尤其是播放中）只在底部提示，不打断用户；下次启动会重新检测
    showStatusMessage(QStringLiteral("网络已断开（离线提示模式）：%1").arg(message), 10000);
}

void MainWindow::concludeStartupNetworkCheck(bool online)
{
    if (m_startupNetworkChecked)
        return;
    m_startupNetworkChecked = true;

    if (online) {
        // 之前失败过、后来才连上：补一次刷新，把首帧没拉到的数据补回来
        if (!m_pendingOfflineMessage.isEmpty()) {
            m_pendingOfflineMessage.clear();
            if (auto *page = qobject_cast<PageBase *>(m_stackedWidget->currentWidget()))
                page->refresh();
        }
        return;
    }
    if (!m_pendingOfflineMessage.isEmpty())
        showOfflineChoiceDialog(m_pendingOfflineMessage);
}

void MainWindow::showOfflineChoiceDialog(const QString &message)
{
    AppContext *ctx = AppContext::instance();
    const int cached = ctx && ctx->cache() ? ctx->cache()->cachedSongs().size() : 0;

    QMessageBox box(this);
    box.setWindowTitle(QStringLiteral("无法连接服务器"));
    box.setIcon(QMessageBox::Warning);
    box.setText(QStringLiteral("启动时无法连接到服务器：%1").arg(message));
    box.setInformativeText(cached > 0
                               ? QStringLiteral("你可以离线播放已缓存的 %1 首歌曲，或退出应用。\n"
                                                "选择离线播放后，运行期间断网只在底部提示，下次启动会重新检测。").arg(cached)
                               : QStringLiteral("当前没有已缓存歌曲，离线模式下无法播放。\n"
                                                "下次启动会重新检测网络状态。"));

    QPushButton *offlineButton = box.addButton(QStringLiteral("离线播放"), QMessageBox::AcceptRole);
    QPushButton *quitButton = box.addButton(QStringLiteral("退出应用"), QMessageBox::RejectRole);
    box.setDefaultButton(cached > 0 ? offlineButton : quitButton);
    offlineButton->setEnabled(cached > 0);
    box.exec();

    if (box.clickedButton() == offlineButton)
        startOfflinePlayback();
    else
        QCoreApplication::quit();
}

void MainWindow::startOfflinePlayback()
{
    AppContext *ctx = AppContext::instance();
    const QVector<Song> songs = ctx && ctx->cache() ? ctx->cache()->cachedSongs() : QVector<Song>();

    navigateTo(PageCachedSongs);
    if (!songs.isEmpty())
        playSongs(songs, 0);
    else
        showStatusMessage(QStringLiteral("没有已缓存歌曲，无法离线播放"));
}

void MainWindow::onAlbumPlayRequested(const Album &album)
{
    if (!album.isValid())
        return;

    AppContext *ctx = AppContext::instance();
    if (!ctx || !ctx->api())
        return;

    m_pendingPlayAlbumId = album.id();
    ctx->api()->getAlbum(album.id());
}

void MainWindow::onSearchRequested(const QString &text)
{
    if (text.isEmpty())
        return;
    m_searchPage->search(text);
    navigateTo(PageSearch);
}

void MainWindow::downloadSong(const Song &song)
{
    if (!song.isValid())
        return;

    AppContext *ctx = AppContext::instance();
    if (!ctx || !ctx->cache())
        return;

    const QString suggested = QStringLiteral("%1 - %2.%3")
                                  .arg(song.artist().isEmpty() ? QStringLiteral("未知") : song.artist(),
                                       song.title(),
                                       song.suffix().isEmpty() ? QStringLiteral("mp3") : song.suffix());
    const QString dest = QFileDialog::getSaveFileName(this, QStringLiteral("保存歌曲"), suggested);
    if (dest.isEmpty())
        return;

    ctx->cache()->downloadSongTo(song, dest);
}

void MainWindow::showSongDetails(const Song &song)
{
    if (!song.isValid())
        return;
    auto *dialog = new SongDetailsDialog(song, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

/* ---------------- 状态提示 ---------------- */

void MainWindow::showStatusMessage(const QString &message, int timeoutMs)
{
    if (!m_statusLabel)
        return;
    m_statusLabel->setText(message);
    m_statusLabel->setVisible(!message.isEmpty());
    if (m_statusTimer) {
        m_statusTimer->stop();
        if (!message.isEmpty() && timeoutMs > 0)
            m_statusTimer->start(timeoutMs);
    }
}

/* ---------------- 主题与语言 ---------------- */

void MainWindow::applyTheme(Theme theme)
{
    loadStyleSheet(theme);
    const bool dark = theme == Dark;
    if (m_navWidget)
        m_navWidget->applyTheme(dark);
    if (m_controlBar)
        m_controlBar->applyTheme(dark);
}

void MainWindow::applyLanguage(const QString &language)
{
    if (!m_translator) {
        m_translator = new QTranslator(qApp);
    } else {
        qApp->removeTranslator(m_translator);
    }

    if (language == QLatin1String("system")) {
        for (PageBase *page : m_stackedWidget->findChildren<PageBase *>())
            page->retranslate();
        return;
    }

    if (m_translator->load(QStringLiteral(":/i18n/naviPlayer_%1").arg(language))) {
        qApp->installTranslator(m_translator);
    }

    for (PageBase *page : m_stackedWidget->findChildren<PageBase *>())
        page->retranslate();
}

/* ---------------- 应用样式 ---------------- */

void MainWindow::loadStyleSheet(Theme theme)
{
    if (!styleSheet().isEmpty() && theme == currentTheme)
        return;
    currentTheme = theme;

    if (QFile qss(theme == Dark ? QStringLiteral(":/theme/dark-style.qss")
                                : QStringLiteral(":/theme/light-style.qss"));
        qss.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setStyleSheet(QString::fromUtf8(qss.readAll()));

#ifdef Q_OS_OSX
        // blur effect 透明窗体应用毛玻璃样式。
        if (m_windowAgent->setWindowAttribute(QStringLiteral("blur-effect"),
                                             theme == Dark ? QStringLiteral("dark")
                                                           : QStringLiteral("light"))) {
            setProperty("custom-style", true);
        }
#endif
    }
}

/* ---------------- 监控键盘输入 ---------------- */

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched && watched->isWidgetType()) {
        if (event->type() == QEvent::KeyPress) {
            auto *ke = static_cast<QKeyEvent *>(event);
            if (ke->key() == Qt::Key_Space) {
                // 空格键控制播放/暂停
                AppContext *ctx = AppContext::instance();
                if (ctx && ctx->player())
                    ctx->player()->togglePlayPause();
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

/* ---------------- QSettings 记忆 ---------------- */

void MainWindow::loadSettings()
{
    QSettings s;

    const bool wasMax = s.value(kSettingsMaximized, false).toBool();
    const QByteArray geo = s.value(kSettingsGeometry).toByteArray();

    if (!geo.isEmpty()) {
        restoreGeometry(geo);
    } else {
        QScreen *screen = nullptr;
        if (windowHandle() && windowHandle()->screen()) {
            screen = windowHandle()->screen();
        } else {
            screen = QGuiApplication::primaryScreen();
        }
        if (screen) {
            const QRect available = screen->availableGeometry();
            move(available.center() - rect().center());
        }
    }

    const QByteArray st = s.value(kSettingsState).toByteArray();
    if (!st.isEmpty() && m_splitter) {
        m_splitter->restoreState(st);
    }

    if (wasMax)
        showMaximized();
}

void MainWindow::saveSettings()
{
    QSettings s;
    s.setValue(kSettingsMaximized, isMaximized());
    s.setValue(kSettingsGeometry, saveGeometry());
    if (m_splitter) {
        s.setValue(kSettingsState, m_splitter->saveState());
    }
    s.sync();
}

void MainWindow::saveSessionState()
{
    AppContext *ctx = AppContext::instance();
    if (!ctx || !ctx->queue() || !ctx->player())
        return;

    SessionState state;
    state.queue = ctx->queue()->songs();
    state.currentIndex = ctx->queue()->currentIndex();
    state.playMode = static_cast<int>(ctx->queue()->playMode());
    state.positionMs = ctx->player()->position();

    if (m_lyricsWindow && m_lyricsWindow->isVisible()) {
        state.lyricsVisible = true;
        state.lyricsPos = m_lyricsWindow->pos();
    }

    ctx->saveSession(state);
}

void MainWindow::restoreSessionState()
{
    AppContext *ctx = AppContext::instance();
    if (!ctx || !ctx->queue() || !ctx->player())
        return;

    SessionState state;
    if (!ctx->loadSession(&state))
        return;

    if (!state.queue.isEmpty()) {
        ctx->queue()->clear();
        ctx->queue()->append(state.queue);
        ctx->queue()->setPlayMode(static_cast<PlaybackQueue::PlayMode>(state.playMode));

        const int index = qBound(0, state.currentIndex, state.queue.size() - 1);
        ctx->queue()->setCurrentIndex(index);
        // 只装载不自动播放：按播放键时从上次进度续播
        ctx->player()->restoreState(ctx->queue()->songAt(index), state.positionMs);
    }

    if (state.lyricsVisible && m_lyricsWindow) {
        if (!m_lyricsWindow->isVisible())
            m_lyricsWindow->toggleVisible();
        m_lyricsWindow->move(state.lyricsPos);
    }
}

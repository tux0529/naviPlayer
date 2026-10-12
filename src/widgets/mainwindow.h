/**
 * @file    mainwindow.h
 * @brief   主窗口：QWindowKit 无边框 + 左侧导航 + 顶部控制栏 + 页面栈（含返回栈）
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QStack>

// QWindowKit
#include <QWKWidgets/widgetwindowagent.h>

#include "models/album.h"
#include "models/artist.h"
#include "models/song.h"
#include "models/playlist.h"

QT_BEGIN_NAMESPACE
class QSplitter;
class QVBoxLayout;
class QStackedWidget;
class QTranslator;
class QPushButton;
class QLabel;
class QTimer;
QT_END_NAMESPACE

class NavWidget;
class ControlBar;
class TitleBar;

// ----Pages----
class SettingsPage;
class HomePage;
class AlbumsView;
class AlbumDetailPage;
class ArtistsView;
class ArtistDetailPage;
class SongsPage;
class PlaylistsPage;
class PlaylistDetailPage;
class CachedSongsPage;
class SearchPage;
class PlaybackQueuePanel;
class LyricsWindow;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    enum Theme {
        Dark,
        Light,
    };
    Q_ENUM(Theme)

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:

    enum Page {
        PageHome = 0,
        PageAlbums,
        PageArtists,
        PageSongs,
        PageFavSongs,
        PageFavAlbums,
        PageFavArtists,
        PagePlaylists,
        PageCachedSongs,
        PageSettings,
        PageSearch,
        PageArtistInfo,
        PageAlbumInfo,
        PagePlaylistInfo
    };

    // --- 初始化步骤 ---
    void setupWindowKitAgent();
    void buildCentralUi();
    void buildPages();
    void connectPages();
    void connectPlayer();

    void loadSettings();
    void saveSettings();
    /** 退出前保存会话（播放队列/进度/歌词窗口状态） */
    void saveSessionState();
    /** 启动时恢复上一次会话 */
    void restoreSessionState();

    // --- 导航 ---
    void navigateTo(Page page, bool addToHistory = true);
    void goBack();
    void showAlbumDetail(const Album &album);
    void showArtistDetail(const Artist &artist);
    void showPlaylistDetail(const Playlist &playlist);

    // --- 交互 ---
    void toggleQueuePanel();
    void toggleLyricsWindow();
    void playSongs(const QVector<Song> &songs, int startIndex = 0);
    /** 离线模式：跳到缓存歌曲页并播放已缓存歌曲 */
    void startOfflinePlayback();
    void downloadSong(const Song &song);
    void showSongDetails(const Song &song);

    void applyTheme(Theme theme);
    void applyLanguage(const QString &language);
    /** 底部短暂提示（连接状态、错误信息等） */
    void showStatusMessage(const QString &message, int timeoutMs = 8000);

private slots:
    void onNavButtonClicked(QPushButton *btn);
    void onSearchRequested(const QString &text);
    void onAlbumPlayRequested(const Album &album);
    /** 网络断开：让用户选择退出应用或离线播放 */
    void onNetworkUnreachable(const QString &message);
    /** 启动网络检测（ping，失败按间隔重试若干次） */
    void runStartupNetworkCheck();
    /** 结束启动网络检测：只有“检测期间确实没连上”才弹窗 */
    void concludeStartupNetworkCheck(bool online);
    void showOfflineChoiceDialog(const QString &message);

private:
    // QWindowKit agent
    QWK::WidgetWindowAgent *m_windowAgent = nullptr;

    // 内部控件
    QVBoxLayout   *m_rootLayout = nullptr;
    QSplitter     *m_splitter   = nullptr;
    QSplitter     *m_contentSplitter = nullptr;
    NavWidget  *m_navWidget  = nullptr;
    ControlBar  *m_controlBar = nullptr;
    TitleBar  *m_titleBar = nullptr;
    QStackedWidget  *m_stackedWidget = nullptr;

    // 浮动/附加控件
    PlaybackQueuePanel *m_queuePanel = nullptr;
    LyricsWindow *m_lyricsWindow = nullptr;
    QTranslator *m_translator = nullptr;
    QLabel *m_statusLabel = nullptr;
    QTimer *m_statusTimer = nullptr;

    // Pages
    SettingsPage *m_settingsPage = nullptr;
    HomePage *m_homePage = nullptr;
    AlbumsView *m_albumsPage = nullptr;
    AlbumsView *m_favAlbumsPage = nullptr;
    ArtistsView *m_artistsPage = nullptr;
    ArtistsView *m_favArtistsPage = nullptr;
    SongsPage *m_songsPage = nullptr;
    SongsPage *m_favSongsPage = nullptr;
    AlbumDetailPage *m_albumDetailPage = nullptr;
    ArtistDetailPage *m_artistDetailPage = nullptr;
    PlaylistsPage *m_playlistsPage = nullptr;
    PlaylistDetailPage *m_playlistDetailPage = nullptr;
    CachedSongsPage *m_cachedSongsPage = nullptr;
    SearchPage *m_searchPage = nullptr;

    // 网络状态：启动检测窗口结束后，断网只在底部提示
    bool m_startupNetworkChecked = false;
    // 启动检测期间收到的断网消息（等 ping 结论再决定是否弹窗）
    QString m_pendingOfflineMessage;
    int m_startupPingAttempts = 0;

    // 导航历史
    QStack<int> m_pageHistory;
    int m_currentPage = PageHome;
    QString m_pendingPlayAlbumId;

    // 左右面板最小宽度
    static constexpr int kLeftMinWidth  = 200;
    static constexpr int kRightMinWidth = 720;

    void loadStyleSheet(Theme theme);

    Theme currentTheme{};
};

#endif // MAINWINDOW_H

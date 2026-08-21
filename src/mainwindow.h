#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QHash>
#include <QSet>
#include "subsonicapi.h"
#include "databasemanager.h"
#include "audioplayer.h"
#include "cachemanager.h"
#include "widgets/navigationwidget.h"
#include "widgets/playercontrolbar.h"
#include "widgets/songlistwidget.h"
#include "widgets/albumwidget.h"
#include "widgets/artistwidget.h"
#include "widgets/albumdetailwidget.h"
#include "widgets/playlistwidget.h"
#include "widgets/searchwidget.h"
#include "widgets/favoriteswidget.h"
#include "widgets/homewidget.h"
#include "widgets/settingswidget.h"
#include "models/song.h"
#include "models/album.h"
#include "models/artist.h"

class QCloseEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onNavigationItemSelected(NavigationWidget::NavigationItem item);
    void onSongSelected(const Song &song);
    void onPlaySong(const Song &song);
    void onSeekRequested(qint64 position);
    void onSettingsTriggered();
    void onSettingsSaved(const QString &serverUrl, const QString &username, const QString &password,
                         int cacheSizeMb, int cacheTtlMinutes, int albumPageSize, const QString &cacheDirectory);
    void onArtistsReceived(const QVector<Artist> &artists);
    void onArtistsFailed(const QString &error);
    void onArtistAlbumsReceived(const Artist &artist, const QVector<Album> &albums);
    void onAlbumsReceived(int offset, int size, const QVector<Album> &albums);
    void onAlbumsFailed(int offset, int size, const QString &error);
    void onHomeAlbumListReceived(const QString &type, const QVector<Album> &albums);
    void onHomeAlbumListFailed(const QString &type, const QString &error);
    void onSongsReceived(const QVector<Song> &songs);
    void onAlbumReceived(const Album &album, const QVector<Song> &songs);
    void onStarredReceived(const QVector<Song> &songs, const QVector<Album> &albums, const QVector<Artist> &artists);
    void onPingFinished(bool success, const QString &error);
    void onApiError(const QString &error);
    void onPlaylistRemoveSong(int index);
    void onSongCached(const QString &songId, const QString &localPath);
    void onSongFavoriteClicked(const Song &song);
    void onAlbumFavoriteClicked(const Album &album);
    void onArtistFavoriteClicked(const Artist &artist);

private:
    enum AlbumRequestMode {
        None,
        ShowDetail,
        PlayFromStart
    };

    void setupUI();
    void setupConnections();
    void loadSettings();
    void saveSettings();
    void restorePlaybackSession();
    void savePlaybackSession();
    void showHome();
    void showFavorites();
    void showAlbums();
    void showArtists();
    void showSearch();
    void showSettings();
    void showAlbumDetail(const Album &album);
    void showPlaylist();
    void requestAlbumPage(int pageIndex);
    void refreshHomeSections();
    QVector<Album> filterHomeAlbums(const QVector<Album> &albums) const;
    bool mapHomeSectionType(const QString &type, HomeWidget::SectionType *sectionType, QString *sectionTitle = nullptr) const;
    void updateAlbumPaginationControls();
    bool isCacheExpired(qint64 lastUpdatedSecs) const;
    void applyAlbumPageCache(int pageIndex);
    void requestAlbumPageRefresh(int pageIndex, bool silentIfCached);
    void applyArtistCache();
    void requestArtistsRefresh(bool silentIfCached);
    QString resolvePlaybackSource(const Song &song) const;
    void ensureSongCached(const Song &song);
    void refreshStarred();
    bool isSongStarred(const QString &songId) const;
    bool isAlbumStarred(const QString &albumId) const;
    bool isArtistStarred(const QString &artistId) const;

    struct AlbumPageCacheEntry {
        QVector<Album> albums;
        qint64 lastUpdatedSecs = 0;
        bool hasNextPage = false;
    };

    SubsonicApi *m_subsonicApi;
    DatabaseManager *m_dbManager;
    AudioPlayer *m_audioPlayer;
    CacheManager *m_cacheManager;

    NavigationWidget *m_navigationWidget;
    PlayerControlBar *m_playerControlBar;
    QStackedWidget *m_contentStack;
    HomeWidget *m_homeWidget;
    AlbumWidget *m_albumWidget;
    ArtistWidget *m_artistWidget;
    AlbumWidget *m_artistAlbumsWidget;
    AlbumDetailWidget *m_albumDetailWidget;
    PlaylistWidget *m_playlistWidget;
    SearchWidget *m_searchWidget;
    FavoritesWidget *m_favoritesWidget;
    SettingsWidget *m_settingsWidget;
    QLabel *m_statusLabel;
    QWidget *m_albumsPage;
    QWidget *m_artistsPage;
    QWidget *m_artistAlbumsPage;
    QWidget *m_albumDetailReturnPage;
    QPushButton *m_albumPrevButton;
    QPushButton *m_albumNextButton;
    QPushButton *m_artistAlbumsBackButton;
    QLabel *m_albumPageLabel;
    QLabel *m_artistAlbumsLabel;
    int m_albumPageSize;
    int m_currentAlbumPage;
    int m_requestedAlbumPage;
    bool m_hasNextAlbumPage;
    Album m_currentAlbum;
    Artist m_currentArtist;
    AlbumRequestMode m_pendingAlbumRequestMode;
    QString m_pendingAlbumId;
    QSet<QString> m_starredSongIds;
    QSet<QString> m_starredAlbumIds;
    QSet<QString> m_starredArtistIds;
    QSet<QString> m_albumIdsFromStarredSongs;

    QSet<QString> m_homeInFlightTypes;
    QHash<QString, qint64> m_homeLastSuccessTs;
    QHash<int, AlbumPageCacheEntry> m_albumPageCache;
    QSet<int> m_albumPageInFlight;
    QVector<Artist> m_artistCache;
    qint64 m_artistCacheLastUpdatedSecs;
    bool m_artistRequestInFlight;
    qint64 m_cacheTtlSecs;
};

#endif // MAINWINDOW_H

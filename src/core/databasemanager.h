/**
 * @file    databasemanager.h
 * @brief   SQLite 本地存储：配置、收藏、播放历史、列表缓存、歌曲缓存索引、会话
 *
 * 线程模型：QSqlDatabase 连接按线程惰性创建（每线程独立连接名），
 * 创建过程由互斥锁保护，因此可以从工作线程安全调用本类的写操作。
 */
#ifndef NAVIPLAYER_DATABASEMANAGER_H
#define NAVIPLAYER_DATABASEMANAGER_H

#include <QObject>
#include <QMutex>
#include <QString>
#include <QVector>
#include <QSqlDatabase>

#include "models/song.h"
#include "models/album.h"
#include "models/artist.h"
#include "models/playlist.h"

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager() override;

    bool init();
    QString databasePath() const { return m_dbPath; }

    // ---------------- config ----------------
    void saveConfig(const QString &key, const QString &value);
    QString loadConfig(const QString &key) const;
    void removeConfig(const QString &key);

    // ---------------- favorites ----------------
    void addFavorite(const QString &type, const Song &song);
    void addFavorite(const QString &type, const QJsonObject &payload);
    void removeFavorite(const QString &type, const QString &id);
    bool isFavorite(const QString &type, const QString &id) const;
    QVector<Song> favoriteSongs() const;
    QVector<Album> favoriteAlbums() const;
    QVector<Artist> favoriteArtists() const;
    /** 用服务端返回整体替换某类收藏 */
    void replaceFavorites(const QString &type, const QJsonArray &items);

    // ---------------- history ----------------
    void addHistory(const Song &song);
    QVector<Song> history(int limit = 100) const;
    void clearHistory();
    /** 最近播放过的专辑（按最近播放时间去重） */
    QVector<Album> recentPlayedAlbums(int limit = 20) const;
    /** 播放次数最多的专辑（本地播放历史统计） */
    QVector<Album> mostPlayedAlbums(int limit = 20) const;

    // ---------------- 列表缓存（启动优先显示） ----------------
    void saveSongs(const QVector<Song> &songs);
    QVector<Song> allSongs() const;
    int songCount() const;
    void clearSongsCache();

    void saveAlbums(const QVector<Album> &albums);
    QVector<Album> allAlbums() const;
    Album albumById(const QString &albumId) const;
    /** 按专辑 id 取专辑封面 id（空表示本地无该专辑） */
    QString albumCoverArtId(const QString &albumId) const;
    int albumCount() const;
    void clearAlbumsCache();

    void saveArtists(const QVector<Artist> &artists);
    QVector<Artist> allArtists() const;
    int artistCount() const;
    void clearArtistsCache();

    // ---------------- 页面 / 主页缓存（TTL） ----------------
    void savePageCache(const QString &key, const QString &payload);
    QString loadPageCache(const QString &key, int ttlMinutes) const;
    void clearPageCache();

    // ---------------- 本地歌曲缓存索引 ----------------
    void saveCachedSong(const Song &song, const QString &localPath, qint64 fileSize);
    void removeCachedSong(const QString &songId);
    void clearCachedSongs();
    QVector<Song> cachedSongs() const;
    QString cachedSongPath(const QString &songId) const;
    qint64 cachedSongsTotalSize() const;
    /** 按最后访问时间升序返回（LRU 淘汰用） */
    QStringList cachedSongIdsByLastAccess() const;
    void touchCachedSong(const QString &songId);

    // ---------------- 播放列表（服务端镜像） ----------------
    void savePlaylists(const QVector<Playlist> &playlists);
    QVector<Playlist> playlists() const;
    void removePlaylist(const QString &id);

    // ---------------- 播放会话 ----------------
    void saveSession(const QString &payload);
    QString loadSession() const;

private:
    QSqlDatabase connection() const;
    bool execQuery(QSqlQuery &query) const;
    void createTables(QSqlDatabase &db);

    QString m_dbPath;
    mutable QMutex m_mutex;
};

#endif // NAVIPLAYER_DATABASEMANAGER_H

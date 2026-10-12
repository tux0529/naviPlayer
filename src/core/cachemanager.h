/**
 * @file    cachemanager.h
 * @brief   本地缓存：封面图（内存 LRU + 磁盘）与歌曲文件（磁盘 + 容量上限 LRU 淘汰）
 */
#ifndef NAVIPLAYER_CACHEMANAGER_H
#define NAVIPLAYER_CACHEMANAGER_H

#include <QObject>
#include <QCache>
#include <QPixmap>
#include <QString>
#include <QHash>
#include <QSet>

#include "models/song.h"

class DatabaseManager;
class SubsonicApi;
class AppSettings;
class QFile;
class QNetworkReply;

class CacheManager : public QObject
{
    Q_OBJECT
public:
    explicit CacheManager(DatabaseManager *db, SubsonicApi *api, AppSettings *settings,
                          QObject *parent = nullptr);

    // ---------------- 封面 ----------------
    /** 同步查询缓存（内存 -> 磁盘），未命中返回 null pixmap */
    QPixmap cachedCover(const QString &coverArtId, int size) const;
    /** 异步获取：命中缓存立即发信号，否则走网络 */
    void requestCover(const QString &coverArtId, int size);
    void saveCover(const QString &coverArtId, int size, const QByteArray &data);
    /** 页面切换时取消离屏请求，减少无效网络与解码 */
    void cancelPendingCovers();
    void clearCoverCache();

    // ---------------- 歌曲 ----------------
    bool isSongCached(const QString &songId) const;
    QString cachedSongPath(const QString &songId) const;
    /** 后台缓存歌曲（已缓存则直接返回） */
    void cacheSong(const Song &song);
    /** 下载到用户指定路径（右键“下载该歌曲”） */
    void downloadSongTo(const Song &song, const QString &destination);
    void removeCachedSong(const QString &songId);
    void clearCachedSongs();
    QVector<Song> cachedSongs() const;
    qint64 cacheSizeBytes() const;
    /** 超过容量上限时删除最久未访问的缓存 */
    void enforceCacheLimit();

    QString songCacheDir() const;
    void setSongCacheDir(const QString &dir);

signals:
    void coverReady(const QString &coverArtId, int size, const QPixmap &pixmap);
    void songCached(const QString &songId, const QString &path);
    void downloadFinished(const QString &songId, const QString &localPath, bool ok);
    void downloadProgress(const QString &songId, qint64 received, qint64 total);
    void cacheSizeChanged(qint64 bytes);

private slots:
    void onCoverArtReceived(const QString &coverArtId, const QByteArray &data);

private:
    struct PendingDownload {
        Song song;
        QString tempPath;
        QString finalPath;
        bool userDownload = false;
        QFile *file = nullptr;
        qint64 received = 0;
    };

    QString coverFilePath(const QString &coverArtId, int size) const;
    QString songFilePath(const QString &songId, const QString &suffix) const;
    static QString hashId(const QString &id);
    void startDownload(const Song &song, const QString &destPath, bool userDownload);

    DatabaseManager *m_db = nullptr;
    SubsonicApi *m_api = nullptr;
    AppSettings *m_settings = nullptr;

    // 封面内存缓存（key: "<id>@<size>"）
    mutable QCache<QString, QPixmap> m_coverMemory;
    QString m_coverDir;
    QString m_songDir;

    QHash<QNetworkReply *, PendingDownload> m_downloads;
    QSet<QString> m_downloadingIds;
    QHash<QString, QSet<int>> m_coverRequests;
};

#endif // NAVIPLAYER_CACHEMANAGER_H

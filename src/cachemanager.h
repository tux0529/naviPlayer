#ifndef CACHEMANAGER_H
#define CACHEMANAGER_H

#include <QObject>
#include <QPixmap>
#include <QCache>
#include <QHash>

class QNetworkAccessManager;
class QNetworkReply;
class QFile;

class CacheManager : public QObject
{
    Q_OBJECT

public:
    explicit CacheManager(QObject *parent = nullptr);
    ~CacheManager();

    bool init();

    QPixmap getCoverArt(const QString &coverArtId);
    void putCoverArt(const QString &coverArtId, const QPixmap &pixmap);
    void clearCoverArtCache();

    qint64 cacheSize() const;
    qint64 songCacheSize() const;
    void clearAllCache();
    QString cachedSongPath(const QString &songId, const QString &suffix = QString()) const;
    void cacheSong(const QString &songId, const QString &streamUrl, const QString &suffix = QString());
    void clearSongCache();
    void setMaxSongCacheSizeMb(int sizeMb);
    int maxSongCacheSizeMb() const;
    void setSongCacheDirectory(const QString &directory);
    QString songCacheDirectory() const;

signals:
    void songCached(const QString &songId, const QString &localPath);
    void songCacheSizeChanged(qint64 bytes);

private slots:
    void onSongDownloadReadyRead();
    void onSongDownloadFinished();

private:
    struct SongDownload;

    QString cacheDirectory() const;
    QString coverArtFilePath(const QString &coverArtId) const;
    QString audioCacheDirectory() const;
    QString songCacheBaseName(const QString &songId) const;
    QString songFilePath(const QString &songId, const QString &suffix) const;
    QString normalizeAudioSuffix(const QString &suffix) const;
    void cleanupOldCacheFiles();
    void enforceSongCacheLimit();
    void touchFile(const QString &filePath) const;

    QCache<QString, QPixmap> m_memoryCache;
    QString m_cachePath;
    QString m_songCachePath;
    QNetworkAccessManager *m_networkManager;
    QHash<QNetworkReply *, SongDownload *> m_songDownloads;
    QHash<QString, QNetworkReply *> m_songDownloadsById;
    int m_maxSongCacheSizeMb;
};

#endif // CACHEMANAGER_H

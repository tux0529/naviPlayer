#include "cachemanager.h"
#include <QStandardPaths>
#include <QDir>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QFile>
#include <QDateTime>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>

struct CacheManager::SongDownload
{
    QString songId;
    QString targetPath;
    QFile *file = nullptr;
};

CacheManager::CacheManager(QObject *parent)
    : QObject(parent)
    , m_memoryCache(50)
    , m_songCachePath()
    , m_networkManager(new QNetworkAccessManager(this))
    , m_maxSongCacheSizeMb(512)
{
}

CacheManager::~CacheManager()
{
}

bool CacheManager::init()
{
    m_cachePath = cacheDirectory();
    QDir dir(m_cachePath);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "Failed to create cache directory:" << m_cachePath;
            return false;
        }
    }

    QString coverArtPath = m_cachePath + QDir::separator() + "cover_art";
    QDir coverArtDir(coverArtPath);
    if (!coverArtDir.exists()) {
        coverArtDir.mkpath(".");
    }

    QDir audioDir(audioCacheDirectory());
    if (!audioDir.exists()) {
        audioDir.mkpath(".");
    }

    cleanupOldCacheFiles();
    return true;
}

QString CacheManager::cacheDirectory() const
{
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QDir::separator() + "naviPlayer";
}

QString CacheManager::coverArtFilePath(const QString &coverArtId) const
{
    QString hash = QString::fromUtf8(QCryptographicHash::hash(coverArtId.toUtf8(), QCryptographicHash::Sha256).toHex());
    return m_cachePath + QDir::separator() + "cover_art" + QDir::separator() + hash + ".png";
}

QString CacheManager::audioCacheDirectory() const
{
    const QString overridePath = QDir::cleanPath(m_songCachePath.trimmed());
    if (!overridePath.isEmpty()) {
        return overridePath;
    }
    return m_cachePath + QDir::separator() + "audio";
}

QString CacheManager::songCacheBaseName(const QString &songId) const
{
    return QString::fromUtf8(QCryptographicHash::hash(songId.toUtf8(), QCryptographicHash::Sha256).toHex());
}

QString CacheManager::normalizeAudioSuffix(const QString &suffix) const
{
    QString normalized = suffix.trimmed().toLower();
    if (normalized.isEmpty()) {
        normalized = "audio";
    }
    normalized.remove(QRegularExpression(QStringLiteral("[^a-z0-9]")));
    if (normalized.isEmpty()) {
        normalized = "audio";
    }
    return normalized;
}

QString CacheManager::songFilePath(const QString &songId, const QString &suffix) const
{
    return audioCacheDirectory() + QDir::separator()
            + songCacheBaseName(songId) + "." + normalizeAudioSuffix(suffix);
}

QPixmap CacheManager::getCoverArt(const QString &coverArtId)
{
    if (coverArtId.isEmpty()) {
        return QPixmap();
    }

    QPixmap *cachedPixmap = m_memoryCache.object(coverArtId);
    if (cachedPixmap) {
        return *cachedPixmap;
    }

    QString filePath = coverArtFilePath(coverArtId);
    if (QFile::exists(filePath)) {
        QPixmap pixmap(filePath);
        if (!pixmap.isNull()) {
            m_memoryCache.insert(coverArtId, new QPixmap(pixmap));
            return pixmap;
        }
    }

    return QPixmap();
}

void CacheManager::putCoverArt(const QString &coverArtId, const QPixmap &pixmap)
{
    if (coverArtId.isEmpty() || pixmap.isNull()) {
        return;
    }

    m_memoryCache.insert(coverArtId, new QPixmap(pixmap));

    QString filePath = coverArtFilePath(coverArtId);
    pixmap.save(filePath, "PNG");
}

void CacheManager::clearCoverArtCache()
{
    m_memoryCache.clear();

    QString coverArtPath = m_cachePath + QDir::separator() + "cover_art";
    QDir dir(coverArtPath);
    if (dir.exists()) {
        dir.removeRecursively();
        dir.mkpath(".");
    }
}

qint64 CacheManager::cacheSize() const
{
    qint64 totalSize = 0;
    QDir dir(m_cachePath);
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QFileInfo &fileInfo : fileInfoList) {
        if (fileInfo.isDir()) {
            QDir subDir(fileInfo.absoluteFilePath());
            QFileInfoList subFileInfoList = subDir.entryInfoList(QDir::Files);
            for (const QFileInfo &subFileInfo : subFileInfoList) {
                totalSize += subFileInfo.size();
            }
        } else {
            totalSize += fileInfo.size();
        }
    }

    return totalSize;
}

qint64 CacheManager::songCacheSize() const
{
    qint64 totalSize = 0;
    QDir dir(audioCacheDirectory());
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo &fileInfo : fileInfoList) {
        totalSize += fileInfo.size();
    }
    return totalSize;
}

void CacheManager::clearAllCache()
{
    clearCoverArtCache();
    clearSongCache();
}

void CacheManager::cleanupOldCacheFiles()
{
    QString coverArtPath = m_cachePath + QDir::separator() + "cover_art";
    QDir dir(coverArtPath);
    if (!dir.exists()) {
        return;
    }

    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files, QDir::Time | QDir::Reversed);

    qint64 totalSize = 0;
    const qint64 maxSize = 100 * 1024 * 1024;

    for (const QFileInfo &fileInfo : fileInfoList) {
        totalSize += fileInfo.size();
        if (totalSize > maxSize) {
            QFile::remove(fileInfo.absoluteFilePath());
        }
    }

    enforceSongCacheLimit();
}

QString CacheManager::cachedSongPath(const QString &songId, const QString &suffix) const
{
    if (songId.isEmpty()) {
        return QString();
    }

    const QString exactPath = songFilePath(songId, suffix);
    if (QFile::exists(exactPath)) {
        touchFile(exactPath);
        return exactPath;
    }

    const QString prefix = songCacheBaseName(songId) + ".";
    QDir dir(audioCacheDirectory());
    QFileInfoList matches = dir.entryInfoList(QStringList() << (prefix + "*"), QDir::Files | QDir::NoDotAndDotDot);
    if (!matches.isEmpty()) {
        const QString path = matches.first().absoluteFilePath();
        touchFile(path);
        return path;
    }

    return QString();
}

void CacheManager::cacheSong(const QString &songId, const QString &streamUrl, const QString &suffix)
{
    if (songId.isEmpty() || streamUrl.isEmpty() || m_maxSongCacheSizeMb <= 0) {
        return;
    }

    if (!cachedSongPath(songId, suffix).isEmpty()) {
        emit songCacheSizeChanged(songCacheSize());
        return;
    }

    if (m_songDownloadsById.contains(songId)) {
        return;
    }

    SongDownload *download = new SongDownload();
    download->songId = songId;
    download->targetPath = songFilePath(songId, suffix);

    QFileInfo targetInfo(download->targetPath);
    QDir().mkpath(targetInfo.absolutePath());

    download->file = new QFile(download->targetPath + ".part");
    if (!download->file->open(QIODevice::WriteOnly)) {
        delete download->file;
        delete download;
        return;
    }

    QNetworkReply *reply = m_networkManager->get(QNetworkRequest(QUrl(streamUrl)));
    reply->setProperty("songId", songId);
    m_songDownloads.insert(reply, download);
    m_songDownloadsById.insert(songId, reply);

    connect(reply, &QNetworkReply::readyRead, this, &CacheManager::onSongDownloadReadyRead);
    connect(reply, &QNetworkReply::finished, this, &CacheManager::onSongDownloadFinished);
}

void CacheManager::clearSongCache()
{
    const auto replies = m_songDownloads.keys();
    for (QNetworkReply *reply : replies) {
        if (reply) {
            reply->abort();
        }
    }

    QDir dir(audioCacheDirectory());
    if (dir.exists()) {
        dir.removeRecursively();
        dir.mkpath(".");
    }
    emit songCacheSizeChanged(songCacheSize());
}

void CacheManager::setMaxSongCacheSizeMb(int sizeMb)
{
    m_maxSongCacheSizeMb = qMax(0, sizeMb);
    enforceSongCacheLimit();
    emit songCacheSizeChanged(songCacheSize());
}

int CacheManager::maxSongCacheSizeMb() const
{
    return m_maxSongCacheSizeMb;
}

void CacheManager::setSongCacheDirectory(const QString &directory)
{
    QString normalized = QDir::cleanPath(directory.trimmed());
    if (!normalized.isEmpty()) {
        QFileInfo info(normalized);
        if (!info.isAbsolute()) {
            normalized = QDir(cacheDirectory()).filePath(normalized);
            normalized = QDir::cleanPath(normalized);
        }
    }

    if (m_songCachePath == normalized) {
        return;
    }

    const auto replies = m_songDownloads.keys();
    for (QNetworkReply *reply : replies) {
        if (reply) {
            reply->abort();
        }
    }

    m_songDownloads.clear();
    m_songDownloadsById.clear();

    m_songCachePath = normalized;

    QDir audioDir(audioCacheDirectory());
    if (!audioDir.exists()) {
        audioDir.mkpath(".");
    }

    enforceSongCacheLimit();
    emit songCacheSizeChanged(songCacheSize());
}

QString CacheManager::songCacheDirectory() const
{
    return audioCacheDirectory();
}

void CacheManager::onSongDownloadReadyRead()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    SongDownload *download = m_songDownloads.value(reply, nullptr);
    if (!reply || !download || !download->file) {
        return;
    }

    download->file->write(reply->readAll());
}

void CacheManager::onSongDownloadFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    SongDownload *download = m_songDownloads.take(reply);
    if (!reply || !download) {
        return;
    }

    m_songDownloadsById.remove(download->songId);

    if (download->file) {
        download->file->write(reply->readAll());
        download->file->close();
    }

    const QString tempPath = download->targetPath + ".part";
    if (reply->error() == QNetworkReply::NoError && download->file && QFileInfo::exists(tempPath)) {
        QFile::remove(download->targetPath);
        if (QFile::rename(tempPath, download->targetPath)) {
            touchFile(download->targetPath);
            enforceSongCacheLimit();
            emit songCached(download->songId, download->targetPath);
            emit songCacheSizeChanged(songCacheSize());
        } else {
            QFile::remove(tempPath);
        }
    } else {
        QFile::remove(tempPath);
    }

    delete download->file;
    delete download;
    reply->deleteLater();
}

void CacheManager::enforceSongCacheLimit()
{
    QDir dir(audioCacheDirectory());
    if (!dir.exists()) {
        return;
    }

    const qint64 maxSizeBytes = qint64(m_maxSongCacheSizeMb) * 1024 * 1024;
    if (maxSizeBytes <= 0) {
        QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
        for (const QFileInfo &fileInfo : files) {
            QFile::remove(fileInfo.absoluteFilePath());
        }
        return;
    }

    QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time | QDir::Reversed);
    qint64 totalSize = 0;
    for (const QFileInfo &fileInfo : files) {
        totalSize += fileInfo.size();
        if (totalSize > maxSizeBytes) {
            QFile::remove(fileInfo.absoluteFilePath());
        }
    }
}

void CacheManager::touchFile(const QString &filePath) const
{
    QFile file(filePath);
    if (file.open(QIODevice::ReadWrite)) {
        file.setFileTime(QDateTime::currentDateTime(), QFileDevice::FileAccessTime);
        file.setFileTime(QDateTime::currentDateTime(), QFileDevice::FileModificationTime);
    }
}

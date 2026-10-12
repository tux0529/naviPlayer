#include "cachemanager.h"

#include "core/databasemanager.h"
#include "core/subsonicapi.h"
#include "core/appsettings.h"
#include "core/asynctask.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QDateTime>
#include <QSslError>

namespace {

QString safeSuffix(const QString &suffix)
{
    QString s;
    for (const QChar c : suffix) {
        if (c.isLetterOrNumber())
            s.append(c.toLower());
    }
    return s.isEmpty() ? QStringLiteral("mp3") : s;
}

}

CacheManager::CacheManager(DatabaseManager *db, SubsonicApi *api, AppSettings *settings,
                           QObject *parent)
    : QObject(parent)
    , m_db(db)
    , m_api(api)
    , m_settings(settings)
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    m_coverDir = base + QStringLiteral("/naviplayer/coverart");
    m_songDir = base + QStringLiteral("/naviplayer/audio");
    QDir().mkpath(m_coverDir);
    QDir().mkpath(m_songDir);

    m_coverMemory.setMaxCost(300);      // 最多 300 张封面常驻内存

    if (m_api) {
        connect(m_api, &SubsonicApi::coverArtReady, this, &CacheManager::onCoverArtReceived);
    }
}

QString CacheManager::hashId(const QString &id)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(id.toUtf8(), QCryptographicHash::Sha256).toHex().left(40));
}

// ------------------------------------------------------------------- 封面
QString CacheManager::coverFilePath(const QString &coverArtId, int size) const
{
    return QStringLiteral("%1/%2_%3.png").arg(m_coverDir, hashId(coverArtId)).arg(size);
}

QPixmap CacheManager::cachedCover(const QString &coverArtId, int size) const
{
    if (coverArtId.isEmpty())
        return QPixmap();

    const QString key = QStringLiteral("%1@%2").arg(coverArtId).arg(size);
    if (QPixmap *cached = m_coverMemory.object(key))
        return *cached;

    QString path = coverFilePath(coverArtId, size);
    if (!QFile::exists(path)) {
        // 回退：用原图缩放，避免同一封面重复下载
        path = coverFilePath(coverArtId, 0);
        if (!QFile::exists(path))
            return QPixmap();
    }

    QPixmap pixmap;
    if (!pixmap.load(path))
        return QPixmap();

    if (size > 0 && pixmap.width() > size) {
        pixmap = pixmap.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    m_coverMemory.insert(key, new QPixmap(pixmap), 1);
    return pixmap;
}

void CacheManager::saveCover(const QString &coverArtId, int size, const QByteArray &data)
{
    if (coverArtId.isEmpty() || data.isEmpty())
        return;

    QPixmap pixmap;
    if (!pixmap.loadFromData(data))
        return;

    QDir().mkpath(m_coverDir);
    QFile f(coverFilePath(coverArtId, size));
    if (f.open(QIODevice::WriteOnly)) {
        f.write(data);
        f.close();
    }

    m_coverMemory.insert(QStringLiteral("%1@%2").arg(coverArtId).arg(size), new QPixmap(pixmap), 1);
}

void CacheManager::requestCover(const QString &coverArtId, int size)
{
    if (coverArtId.isEmpty())
        return;

    const QPixmap hit = cachedCover(coverArtId, size);
    if (!hit.isNull()) {
        Q_EMIT coverReady(coverArtId, size, hit);
        return;
    }

    if (!m_api || !m_api->isConfigured())
        return;

    auto &sizes = m_coverRequests[coverArtId];
    const bool firstRequest = sizes.isEmpty();
    sizes.insert(size);
    if (firstRequest)
        m_api->getCoverArt(coverArtId, size > 0 ? size : 300);
}

void CacheManager::onCoverArtReceived(const QString &coverArtId, const QByteArray &data)
{
    const QSet<int> sizes = m_coverRequests.take(coverArtId);

    Async::run(this, [this, coverArtId, data, sizes]() -> QPixmap {
        // 保存原图，后续任何尺寸都可本地缩放复用
        QDir().mkpath(m_coverDir);
        QFile f(coverFilePath(coverArtId, 0));
        if (f.open(QIODevice::WriteOnly)) {
            f.write(data);
            f.close();
        }

        QPixmap original;
        original.loadFromData(data);
        if (original.isNull())
            return QPixmap();

        for (int size : sizes) {
            if (size <= 0 || size >= original.width())
                continue;
            const QPixmap scaled = original.scaled(size, size, Qt::KeepAspectRatio,
                                                   Qt::SmoothTransformation);
            QFile sf(coverFilePath(coverArtId, size));
            if (sf.open(QIODevice::WriteOnly)) {
                scaled.save(&sf, "PNG");
                sf.close();
            }
            m_coverMemory.insert(QStringLiteral("%1@%2").arg(coverArtId).arg(size),
                                 new QPixmap(scaled), 1);
        }
        m_coverMemory.insert(QStringLiteral("%1@0").arg(coverArtId), new QPixmap(original), 1);
        return original;
    }, [this, coverArtId, sizes](const QPixmap &original) {
        if (original.isNull())
            return;
        for (int size : sizes) {
            if (size <= 0 || size >= original.width()) {
                Q_EMIT coverReady(coverArtId, size, original);
            } else {
                Q_EMIT coverReady(coverArtId, size,
                                  original.scaled(size, size, Qt::KeepAspectRatio,
                                                  Qt::SmoothTransformation));
            }
        }
    });
}

void CacheManager::cancelPendingCovers()
{
    if (m_api)
        m_api->cancelPendingCoverArts();
    m_coverRequests.clear();
}

void CacheManager::clearCoverCache()
{
    m_coverMemory.clear();
    QDir dir(m_coverDir);
    const QStringList files = dir.entryList(QDir::Files);
    for (const QString &f : files)
        QFile::remove(dir.filePath(f));
}

// ------------------------------------------------------------------- 歌曲
QString CacheManager::songCacheDir() const
{
    return m_songDir;
}

void CacheManager::setSongCacheDir(const QString &dir)
{
    if (dir.isEmpty())
        return;
    m_songDir = dir;
    QDir().mkpath(m_songDir);
}

QString CacheManager::songFilePath(const QString &songId, const QString &suffix) const
{
    return QStringLiteral("%1/%2.%3").arg(m_songDir, hashId(songId), safeSuffix(suffix));
}

bool CacheManager::isSongCached(const QString &songId) const
{
    if (songId.isEmpty() || !m_db)
        return false;
    const QString path = m_db->cachedSongPath(songId);
    return !path.isEmpty() && QFile::exists(path);
}

QString CacheManager::cachedSongPath(const QString &songId) const
{
    if (!m_db)
        return QString();
    const QString path = m_db->cachedSongPath(songId);
    if (path.isEmpty() || !QFile::exists(path))
        return QString();
    m_db->touchCachedSong(songId);
    return path;
}

void CacheManager::cacheSong(const Song &song)
{
    if (!song.isValid())
        return;
    if (m_settings && m_settings->maxSongCacheMb() <= 0)
        return;
    if (isSongCached(song.id()))
        return;
    startDownload(song, songFilePath(song.id(), song.suffix()), false);
}

void CacheManager::downloadSongTo(const Song &song, const QString &destination)
{
    if (!song.isValid() || destination.isEmpty())
        return;
    startDownload(song, destination, true);
}

void CacheManager::startDownload(const Song &song, const QString &destPath, bool userDownload)
{
    if (!m_api || !m_api->isConfigured() || !song.isValid())
        return;
    if (m_downloadingIds.contains(song.id()))
        return;

    QDir().mkpath(QFileInfo(destPath).absolutePath());
    if (QFile::exists(destPath) && !userDownload) {
        // 已存在则直接登记
        if (m_db)
            m_db->saveCachedSong(song, destPath, QFileInfo(destPath).size());
        Q_EMIT songCached(song.id(), destPath);
        return;
    }

    const QUrl url = m_api->streamUrl(song.id(), m_settings ? m_settings->maxBitRate() : 0);
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = m_api->network()->get(request);
    if (!reply)
        return;

    PendingDownload pd;
    pd.song = song;
    pd.tempPath = destPath + QStringLiteral(".part");
    pd.finalPath = destPath;
    pd.userDownload = userDownload;
    pd.file = new QFile(pd.tempPath);
    if (!pd.file->open(QIODevice::WriteOnly)) {
        delete pd.file;
        reply->abort();
        reply->deleteLater();
        return;
    }

    m_downloads.insert(reply, pd);
    m_downloadingIds.insert(song.id());

    connect(reply, &QNetworkReply::sslErrors, this, [reply](const QList<QSslError> &) {
        reply->ignoreSslErrors();
    });

    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
        if (!m_downloads.contains(reply))
            return;
        PendingDownload &pd = m_downloads[reply];
        if (pd.file)
            pd.file->write(reply->readAll());
    });

    connect(reply, &QNetworkReply::downloadProgress, this,
            [this, reply, song](qint64 received, qint64 total) {
                Q_EMIT downloadProgress(song.id(), received, total);
            });

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (!m_downloads.contains(reply))
            return;
        PendingDownload pd = m_downloads.take(reply);
        m_downloadingIds.remove(pd.song.id());

        const bool failed = reply->error() != QNetworkReply::NoError;
        if (pd.file) {
            pd.file->write(reply->readAll());
            pd.file->close();
            delete pd.file;
            pd.file = nullptr;
        }

        if (failed) {
            qWarning().noquote() << "[CacheManager] 下载失败:" << pd.song.title()
                                 << reply->errorString();
            QFile::remove(pd.tempPath);
            reply->deleteLater();
            Q_EMIT downloadFinished(pd.song.id(), QString(), false);
            return;
        }

        if (QFile::exists(pd.finalPath))
            QFile::remove(pd.finalPath);
        QFile::rename(pd.tempPath, pd.finalPath);

        if (pd.userDownload) {
            Q_EMIT downloadFinished(pd.song.id(), pd.finalPath, true);
        } else {
            if (m_db)
                m_db->saveCachedSong(pd.song, pd.finalPath, QFileInfo(pd.finalPath).size());
            Q_EMIT songCached(pd.song.id(), pd.finalPath);
            Q_EMIT cacheSizeChanged(cacheSizeBytes());
            enforceCacheLimit();
        }

        reply->deleteLater();
    });
}

void CacheManager::removeCachedSong(const QString &songId)
{
    const QString path = m_db ? m_db->cachedSongPath(songId) : QString();
    if (!path.isEmpty())
        QFile::remove(path);
    if (m_db)
        m_db->removeCachedSong(songId);
    Q_EMIT cacheSizeChanged(cacheSizeBytes());
}

void CacheManager::clearCachedSongs()
{
    QDir dir(m_songDir);
    for (const QString &f : dir.entryList(QDir::Files))
        QFile::remove(dir.filePath(f));
    if (m_db)
        m_db->clearCachedSongs();
    Q_EMIT cacheSizeChanged(0);
}

QVector<Song> CacheManager::cachedSongs() const
{
    return m_db ? m_db->cachedSongs() : QVector<Song>();
}

qint64 CacheManager::cacheSizeBytes() const
{
    return m_db ? m_db->cachedSongsTotalSize() : 0;
}

void CacheManager::enforceCacheLimit()
{
    if (!m_db || !m_settings)
        return;

    const qint64 limit = (qint64)m_settings->maxSongCacheMb() * 1024 * 1024;
    qint64 total = cacheSizeBytes();
    if (limit <= 0 || total <= limit)
        return;

    QStringList ids = m_db->cachedSongIdsByLastAccess();
    for (const QString &id : ids) {
        if (total <= limit)
            break;
        const QString path = m_db->cachedSongPath(id);
        if (!path.isEmpty()) {
            const qint64 size = QFileInfo(path).size();
            QFile::remove(path);
            total -= size;
        }
        m_db->removeCachedSong(id);
    }
    Q_EMIT cacheSizeChanged(cacheSizeBytes());
}

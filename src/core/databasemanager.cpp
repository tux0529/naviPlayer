#include "databasemanager.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSqlError>
#include <QSqlQuery>
#include <QMap>
#include <QStringList>
#include <QStandardPaths>
#include <QThread>
#include <QDateTime>
#include <QVariant>

#include <algorithm>

namespace {

QString threadConnectionName()
{
    return QStringLiteral("naviplayer_db_%1")
        .arg(reinterpret_cast<quintptr>(QThread::currentThread()));
}

QString jsonToString(const QJsonObject &obj)
{
    return QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

QJsonObject stringToJson(const QString &text)
{
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError)
        return QJsonObject();
    return doc.object();
}

qint64 nowMs()
{
    return QDateTime::currentMSecsSinceEpoch();
}

}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager() = default;

bool DatabaseManager::init()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    m_dbPath = dir + QStringLiteral("/naviplayer.db");

    QSqlDatabase db = connection();
    if (!db.isOpen()) {
        qWarning("[DatabaseManager] 无法打开数据库: %s", qPrintable(db.lastError().text()));
        return false;
    }
    createTables(db);
    return true;
}

QSqlDatabase DatabaseManager::connection() const
{
    const QString name = threadConnectionName();
    {
        QMutexLocker locker(&m_mutex);
        if (QSqlDatabase::contains(name)) {
            QSqlDatabase db = QSqlDatabase::database(name);
            if (!db.isOpen())
                db.open();
            return db;
        }
    }

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), name);
    db.setDatabaseName(m_dbPath);
    if (!db.open()) {
        qWarning("[DatabaseManager] open failed: %s", qPrintable(db.lastError().text()));
        return db;
    }
    db.exec(QStringLiteral("PRAGMA journal_mode=WAL;"));
    db.exec(QStringLiteral("PRAGMA synchronous=NORMAL;"));
    db.exec(QStringLiteral("PRAGMA foreign_keys=ON;"));
    return db;
}

bool DatabaseManager::execQuery(QSqlQuery &query) const
{
    if (!query.exec()) {
        qWarning("[DatabaseManager] SQL 失败: %s | %s [driver=%s db=%s]",
                 qPrintable(query.lastQuery()),
                 qPrintable(query.lastError().text()),
                 qPrintable(query.lastError().driverText()),
                 qPrintable(query.lastError().databaseText()));
        return false;
    }
    return true;
}

void DatabaseManager::createTables(QSqlDatabase &db)
{
    QSqlQuery q(db);

    // 兼容旧版本遗留数据库：表已存在但缺少必需列时先删除，随后重建
    const QMap<QString, QStringList> requiredColumns = {
        {QStringLiteral("config"),       {QStringLiteral("key"), QStringLiteral("value")}},
        {QStringLiteral("favorites"),    {QStringLiteral("type"), QStringLiteral("id"), QStringLiteral("payload")}},
        {QStringLiteral("history"),      {QStringLiteral("song_id"), QStringLiteral("payload"), QStringLiteral("played_at")}},
        {QStringLiteral("songs"),        {QStringLiteral("id"), QStringLiteral("payload"), QStringLiteral("album_id")}},
        {QStringLiteral("albums"),       {QStringLiteral("id"), QStringLiteral("payload"), QStringLiteral("artist_id")}},
        {QStringLiteral("artists"),      {QStringLiteral("id"), QStringLiteral("payload"), QStringLiteral("album_count")}},
        {QStringLiteral("page_cache"),   {QStringLiteral("key"), QStringLiteral("payload"), QStringLiteral("updated_at")}},
        {QStringLiteral("cached_songs"), {QStringLiteral("song_id"), QStringLiteral("local_path"), QStringLiteral("last_access")}},
        {QStringLiteral("playlists"),    {QStringLiteral("id"), QStringLiteral("payload"), QStringLiteral("updated_at")}},
        {QStringLiteral("session"),      {QStringLiteral("id"), QStringLiteral("payload")}},
    };

    const QStringList existing = db.tables();
    for (auto it = requiredColumns.constBegin(); it != requiredColumns.constEnd(); ++it) {
        if (!existing.contains(it.key()))
            continue;

        QStringList columns;
        QSqlQuery info(db);
        info.exec(QStringLiteral("PRAGMA table_info(%1)").arg(it.key()));
        while (info.next())
            columns.append(info.value(1).toString());

        bool compatible = true;
        for (const QString &column : it.value()) {
            if (!columns.contains(column)) {
                compatible = false;
                break;
            }
        }

        if (!compatible) {
            qWarning("[DatabaseManager] 表 %s 结构不兼容，重建", qPrintable(it.key()));
            QSqlQuery drop(db);
            drop.exec(QStringLiteral("DROP TABLE %1").arg(it.key()));
        }
    }

    const char *statements[] = {
        // 配置
        "CREATE TABLE IF NOT EXISTS config ("
        " key TEXT PRIMARY KEY,"
        " value TEXT)",

        // 收藏（type: song / album / artist）
        "CREATE TABLE IF NOT EXISTS favorites ("
        " type TEXT NOT NULL,"
        " id TEXT NOT NULL,"
        " title TEXT,"
        " artist TEXT,"
        " album TEXT,"
        " cover_art_id TEXT,"
        " payload TEXT,"
        " added_at INTEGER,"
        " PRIMARY KEY (type, id))",

        // 播放历史
        "CREATE TABLE IF NOT EXISTS history ("
        " rowid INTEGER PRIMARY KEY AUTOINCREMENT,"
        " song_id TEXT,"
        " title TEXT,"
        " artist TEXT,"
        " album TEXT,"
        " cover_art_id TEXT,"
        " payload TEXT,"
        " played_at INTEGER)",
        "CREATE INDEX IF NOT EXISTS idx_history_time ON history(played_at)",

        // 歌曲缓存（服务端全量镜像）
        "CREATE TABLE IF NOT EXISTS songs ("
        " id TEXT PRIMARY KEY,"
        " title TEXT,"
        " artist TEXT,"
        " album TEXT,"
        " album_id TEXT,"
        " artist_id TEXT,"
        " track INTEGER,"
        " disc_number INTEGER,"
        " duration INTEGER,"
        " cover_art_id TEXT,"
        " is_favorite INTEGER DEFAULT 0,"
        " payload TEXT,"
        " updated_at INTEGER)",
        "CREATE INDEX IF NOT EXISTS idx_songs_album ON songs(album_id)",

        // 专辑缓存
        "CREATE TABLE IF NOT EXISTS albums ("
        " id TEXT PRIMARY KEY,"
        " name TEXT,"
        " artist TEXT,"
        " artist_id TEXT,"
        " cover_art_id TEXT,"
        " song_count INTEGER,"
        " duration INTEGER,"
        " year INTEGER,"
        " genre TEXT,"
        " is_favorite INTEGER DEFAULT 0,"
        " payload TEXT,"
        " updated_at INTEGER)",

        // 艺术家缓存
        "CREATE TABLE IF NOT EXISTS artists ("
        " id TEXT PRIMARY KEY,"
        " name TEXT,"
        " album_count INTEGER,"
        " cover_art_id TEXT,"
        " is_favorite INTEGER DEFAULT 0,"
        " payload TEXT,"
        " updated_at INTEGER)",

        // 页面 / 主页缓存（带时间戳用于 TTL）
        "CREATE TABLE IF NOT EXISTS page_cache ("
        " key TEXT PRIMARY KEY,"
        " payload TEXT,"
        " updated_at INTEGER)",

        // 本地音频缓存索引
        "CREATE TABLE IF NOT EXISTS cached_songs ("
        " song_id TEXT PRIMARY KEY,"
        " title TEXT,"
        " artist TEXT,"
        " album TEXT,"
        " album_id TEXT,"
        " artist_id TEXT,"
        " duration INTEGER,"
        " cover_art_id TEXT,"
        " local_path TEXT,"
        " file_size INTEGER,"
        " added_at INTEGER,"
        " last_access INTEGER,"
        " payload TEXT)",

        // 播放列表镜像
        "CREATE TABLE IF NOT EXISTS playlists ("
        " id TEXT PRIMARY KEY,"
        " name TEXT,"
        " song_count INTEGER,"
        " duration INTEGER,"
        " payload TEXT,"
        " updated_at INTEGER)",

        // 播放会话
        "CREATE TABLE IF NOT EXISTS session ("
        " id INTEGER PRIMARY KEY CHECK (id = 1),"
        " payload TEXT)",
    };

    for (const char *sql : statements) {
        q.exec(QString::fromUtf8(sql));
    }
}

// ------------------------------------------------------------------ config
void DatabaseManager::saveConfig(const QString &key, const QString &value)
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("INSERT INTO config (key, value) VALUES (?, ?) "
                             "ON CONFLICT(key) DO UPDATE SET value=excluded.value"));
    q.addBindValue(key);
    q.addBindValue(value);
    execQuery(q);
}

QString DatabaseManager::loadConfig(const QString &key) const
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT value FROM config WHERE key = ?"));
    q.addBindValue(key);
    if (!execQuery(q))
        return QString();
    if (q.next())
        return q.value(0).toString();
    return QString();
}

void DatabaseManager::removeConfig(const QString &key)
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("DELETE FROM config WHERE key = ?"));
    q.addBindValue(key);
    execQuery(q);
}

// -------------------------------------------------------------- favorites
void DatabaseManager::addFavorite(const QString &type, const Song &song)
{
    QJsonObject o = song.toJson();
    o[QStringLiteral("id")] = song.id();
    addFavorite(type, o);
}

void DatabaseManager::addFavorite(const QString &type, const QJsonObject &payload)
{
    const QString id = payload.value(QStringLiteral("id")).toString();
    if (id.isEmpty())
        return;

    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("INSERT INTO favorites (type, id, title, artist, album, cover_art_id, payload, added_at) "
                             "VALUES (?, ?, ?, ?, ?, ?, ?, ?) "
                             "ON CONFLICT(type, id) DO UPDATE SET "
                             " title=excluded.title, artist=excluded.artist, album=excluded.album,"
                             " cover_art_id=excluded.cover_art_id, payload=excluded.payload"));
    q.addBindValue(type);
    q.addBindValue(id);
    q.addBindValue(payload.value(QStringLiteral("title")).toString(payload.value(QStringLiteral("name")).toString()));
    q.addBindValue(payload.value(QStringLiteral("artist")).toString());
    q.addBindValue(payload.value(QStringLiteral("album")).toString());
    q.addBindValue(payload.value(QStringLiteral("coverArtId")).toString());
    q.addBindValue(jsonToString(payload));
    q.addBindValue(nowMs());
    execQuery(q);
}

void DatabaseManager::removeFavorite(const QString &type, const QString &id)
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("DELETE FROM favorites WHERE type = ? AND id = ?"));
    q.addBindValue(type);
    q.addBindValue(id);
    execQuery(q);
}

bool DatabaseManager::isFavorite(const QString &type, const QString &id) const
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT 1 FROM favorites WHERE type = ? AND id = ? LIMIT 1"));
    q.addBindValue(type);
    q.addBindValue(id);
    if (!execQuery(q))
        return false;
    return q.next();
}

QVector<Song> DatabaseManager::favoriteSongs() const
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT payload FROM favorites WHERE type='song' ORDER BY added_at DESC"));
    QVector<Song> result;
    if (!execQuery(q))
        return result;
    while (q.next()) {
        const Song s = Song::fromJson(stringToJson(q.value(0).toString()));
        if (s.isValid()) {
            Song copy = s;
            copy.setFavorite(true);
            result.append(copy);
        }
    }
    return result;
}

QVector<Album> DatabaseManager::favoriteAlbums() const
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT payload FROM favorites WHERE type='album' ORDER BY added_at DESC"));
    QVector<Album> result;
    if (!execQuery(q))
        return result;
    while (q.next()) {
        Album a = Album::fromJson(stringToJson(q.value(0).toString()));
        if (a.isValid()) {
            a.setFavorite(true);
            result.append(a);
        }
    }
    return result;
}

QVector<Artist> DatabaseManager::favoriteArtists() const
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT payload FROM favorites WHERE type='artist' ORDER BY added_at DESC"));
    QVector<Artist> result;
    if (!execQuery(q))
        return result;
    while (q.next()) {
        Artist a = Artist::fromJson(stringToJson(q.value(0).toString()));
        if (a.isValid()) {
            a.setFavorite(true);
            result.append(a);
        }
    }
    return result;
}

void DatabaseManager::replaceFavorites(const QString &type, const QJsonArray &items)
{
    QSqlDatabase db = connection();
    db.transaction();
    {
        QSqlQuery del(db);
        del.prepare(QStringLiteral("DELETE FROM favorites WHERE type = ?"));
        del.addBindValue(type);
        execQuery(del);
    }
    for (const QJsonValue &v : items) {
        const QJsonObject o = v.toObject();
        if (o.isEmpty())
            continue;
        QJsonObject copy = o;
        if (copy.value(QStringLiteral("id")).toString().isEmpty())
            continue;
        addFavorite(type, copy);
    }
    db.commit();
}

// ---------------------------------------------------------------- history
void DatabaseManager::addHistory(const Song &song)
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("INSERT INTO history (song_id, title, artist, album, cover_art_id, payload, played_at) "
                             "VALUES (?, ?, ?, ?, ?, ?, ?)"));
    q.addBindValue(song.id());
    q.addBindValue(song.title());
    q.addBindValue(song.artist());
    q.addBindValue(song.album());
    q.addBindValue(song.coverArtId());
    q.addBindValue(jsonToString(song.toJson()));
    q.addBindValue(nowMs());
    execQuery(q);

    // 只保留最近 500 条
    QSqlQuery trim(db);
    trim.prepare(QStringLiteral("DELETE FROM history WHERE rowid NOT IN "
                                "(SELECT rowid FROM history ORDER BY played_at DESC LIMIT 500)"));
    execQuery(trim);
}

QVector<Song> DatabaseManager::history(int limit) const
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT payload FROM history ORDER BY played_at DESC LIMIT ?"));
    q.addBindValue(limit);
    QVector<Song> result;
    if (!execQuery(q))
        return result;
    while (q.next()) {
        const Song s = Song::fromJson(stringToJson(q.value(0).toString()));
        if (s.isValid())
            result.append(s);
    }
    return result;
}

void DatabaseManager::clearHistory()
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("DELETE FROM history"));
    execQuery(q);
}

QVector<Album> DatabaseManager::recentPlayedAlbums(int limit) const
{
    // 最近播放：按时间倒序扫描历史，对专辑去重
    const QVector<Song> songs = history(500);
    QStringList ids;
    for (const Song &s : songs) {
        if (s.albumId().isEmpty() || ids.contains(s.albumId()))
            continue;
        ids.append(s.albumId());
        if (ids.size() >= limit)
            break;
    }

    QVector<Album> result;
    for (const QString &id : ids) {
        const Album a = albumById(id);
        if (a.isValid())
            result.append(a);
    }
    return result;
}

QVector<Album> DatabaseManager::mostPlayedAlbums(int limit) const
{
    // 播放最多：统计历史中各专辑出现次数
    const QVector<Song> songs = history(5000);
    QHash<QString, int> counts;
    for (const Song &s : songs) {
        if (s.albumId().isEmpty())
            continue;
        counts[s.albumId()] += 1;
    }

    QVector<QPair<QString, int>> ranked;
    ranked.reserve(counts.size());
    for (auto it = counts.constBegin(); it != counts.constEnd(); ++it)
        ranked.append(qMakePair(it.key(), it.value()));
    std::sort(ranked.begin(), ranked.end(),
              [](const QPair<QString, int> &a, const QPair<QString, int> &b) {
                  return a.second > b.second;
              });

    QVector<Album> result;
    for (const auto &item : ranked) {
        if (result.size() >= limit)
            break;
        const Album a = albumById(item.first);
        if (a.isValid())
            result.append(a);
    }
    return result;
}

// ------------------------------------------------------------ songs cache
void DatabaseManager::saveSongs(const QVector<Song> &songs)
{
    if (songs.isEmpty())
        return;
    QSqlDatabase db = connection();
    db.transaction();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("INSERT INTO songs (id, title, artist, album, album_id, artist_id, track,"
                             " disc_number, duration, cover_art_id, is_favorite, payload, updated_at) "
                             "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
                             "ON CONFLICT(id) DO UPDATE SET "
                             " title=excluded.title, artist=excluded.artist, album=excluded.album,"
                             " album_id=excluded.album_id, artist_id=excluded.artist_id, track=excluded.track,"
                             " disc_number=excluded.disc_number, duration=excluded.duration,"
                             " cover_art_id=excluded.cover_art_id, payload=excluded.payload,"
                             " updated_at=excluded.updated_at"));
    const qint64 ts = nowMs();
    for (const Song &s : songs) {
        if (!s.isValid())
            continue;
        q.addBindValue(s.id());
        q.addBindValue(s.title());
        q.addBindValue(s.artist());
        q.addBindValue(s.album());
        q.addBindValue(s.albumId());
        q.addBindValue(s.artistId());
        q.addBindValue(s.track());
        q.addBindValue(s.discNumber());
        q.addBindValue(s.duration());
        q.addBindValue(s.coverArtId());
        q.addBindValue(s.isFavorite() ? 1 : 0);
        q.addBindValue(jsonToString(s.toJson()));
        q.addBindValue(ts);
        if (!q.exec()) {
            qWarning("[DatabaseManager] 保存歌曲失败: %s", qPrintable(q.lastError().text()));
        }
    }
    db.commit();
}

QVector<Song> DatabaseManager::allSongs() const
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT payload FROM songs"));
    QVector<Song> result;
    if (!execQuery(q))
        return result;
    while (q.next()) {
        const Song s = Song::fromJson(stringToJson(q.value(0).toString()));
        if (s.isValid())
            result.append(s);
    }
    return result;
}

int DatabaseManager::songCount() const
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT COUNT(*) FROM songs"));
    if (!execQuery(q) || !q.next())
        return 0;
    return q.value(0).toInt();
}

void DatabaseManager::clearSongsCache()
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("DELETE FROM songs"));
    execQuery(q);
}

// ------------------------------------------------------------ albums cache
void DatabaseManager::saveAlbums(const QVector<Album> &albums)
{
    if (albums.isEmpty())
        return;
    QSqlDatabase db = connection();
    db.transaction();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("INSERT INTO albums (id, name, artist, artist_id, cover_art_id, song_count,"
                             " duration, year, genre, is_favorite, payload, updated_at) "
                             "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
                             "ON CONFLICT(id) DO UPDATE SET "
                             " name=excluded.name, artist=excluded.artist, artist_id=excluded.artist_id,"
                             " cover_art_id=excluded.cover_art_id, song_count=excluded.song_count,"
                             " duration=excluded.duration, year=excluded.year, genre=excluded.genre,"
                             " payload=excluded.payload, updated_at=excluded.updated_at"));
    const qint64 ts = nowMs();
    for (const Album &a : albums) {
        if (!a.isValid())
            continue;
        q.addBindValue(a.id());
        q.addBindValue(a.name());
        q.addBindValue(a.artist());
        q.addBindValue(a.artistId());
        q.addBindValue(a.coverArtId());
        q.addBindValue(a.songCount());
        q.addBindValue(a.duration());
        q.addBindValue(a.year());
        q.addBindValue(a.genre());
        q.addBindValue(a.isFavorite() ? 1 : 0);
        q.addBindValue(jsonToString(a.toJson()));
        q.addBindValue(ts);
        if (!q.exec()) {
            qWarning("[DatabaseManager] 保存专辑失败: %s", qPrintable(q.lastError().text()));
        }
    }
    db.commit();
}

QVector<Album> DatabaseManager::allAlbums() const
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT payload FROM albums"));
    QVector<Album> result;
    if (!execQuery(q))
        return result;
    while (q.next()) {
        const Album a = Album::fromJson(stringToJson(q.value(0).toString()));
        if (a.isValid())
            result.append(a);
    }
    return result;
}

Album DatabaseManager::albumById(const QString &albumId) const
{
    if (albumId.isEmpty())
        return Album();

    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT payload FROM albums WHERE id = ?"));
    q.addBindValue(albumId);
    if (!execQuery(q) || !q.next())
        return Album();
    return Album::fromJson(stringToJson(q.value(0).toString()));
}

QString DatabaseManager::albumCoverArtId(const QString &albumId) const
{
    if (albumId.isEmpty())
        return QString();

    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT cover_art_id FROM albums WHERE id = ?"));
    q.addBindValue(albumId);
    if (!execQuery(q) || !q.next())
        return QString();
    return q.value(0).toString();
}

int DatabaseManager::albumCount() const
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT COUNT(*) FROM albums"));
    if (!execQuery(q) || !q.next())
        return 0;
    return q.value(0).toInt();
}

void DatabaseManager::clearAlbumsCache()
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("DELETE FROM albums"));
    execQuery(q);
}

// ----------------------------------------------------------- artists cache
void DatabaseManager::saveArtists(const QVector<Artist> &artists)
{
    if (artists.isEmpty())
        return;
    QSqlDatabase db = connection();
    db.transaction();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("INSERT INTO artists (id, name, album_count, cover_art_id, is_favorite, payload, updated_at) "
                             "VALUES (?, ?, ?, ?, ?, ?, ?) "
                             "ON CONFLICT(id) DO UPDATE SET "
                             " name=excluded.name, album_count=excluded.album_count,"
                             " cover_art_id=excluded.cover_art_id, payload=excluded.payload,"
                             " updated_at=excluded.updated_at"));
    const qint64 ts = nowMs();
    for (const Artist &a : artists) {
        if (!a.isValid())
            continue;
        q.addBindValue(a.id());
        q.addBindValue(a.name());
        q.addBindValue(a.albumCount());
        q.addBindValue(a.coverArtId());
        q.addBindValue(a.isFavorite() ? 1 : 0);
        q.addBindValue(jsonToString(a.toJson()));
        q.addBindValue(ts);
        if (!q.exec()) {
            qWarning("[DatabaseManager] 保存艺术家失败: %s", qPrintable(q.lastError().text()));
        }
    }
    db.commit();
}

QVector<Artist> DatabaseManager::allArtists() const
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT payload FROM artists"));
    QVector<Artist> result;
    if (!execQuery(q))
        return result;
    while (q.next()) {
        const Artist a = Artist::fromJson(stringToJson(q.value(0).toString()));
        if (a.isValid())
            result.append(a);
    }
    return result;
}

int DatabaseManager::artistCount() const
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT COUNT(*) FROM artists"));
    if (!execQuery(q) || !q.next())
        return 0;
    return q.value(0).toInt();
}

void DatabaseManager::clearArtistsCache()
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("DELETE FROM artists"));
    execQuery(q);
}

// -------------------------------------------------------------- page cache
void DatabaseManager::savePageCache(const QString &key, const QString &payload)
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("INSERT INTO page_cache (key, payload, updated_at) VALUES (?, ?, ?) "
                             "ON CONFLICT(key) DO UPDATE SET payload=excluded.payload, updated_at=excluded.updated_at"));
    q.addBindValue(key);
    q.addBindValue(payload);
    q.addBindValue(nowMs());
    execQuery(q);
}

QString DatabaseManager::loadPageCache(const QString &key, int ttlMinutes) const
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT payload, updated_at FROM page_cache WHERE key = ?"));
    q.addBindValue(key);
    if (!execQuery(q) || !q.next())
        return QString();
    const qint64 updated = q.value(1).toLongLong();
    if (ttlMinutes > 0 && (nowMs() - updated) > (qint64)ttlMinutes * 60 * 1000)
        return QString();
    return q.value(0).toString();
}

void DatabaseManager::clearPageCache()
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("DELETE FROM page_cache"));
    execQuery(q);
}

// ------------------------------------------------------- cached songs index
void DatabaseManager::saveCachedSong(const Song &song, const QString &localPath, qint64 fileSize)
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("INSERT INTO cached_songs (song_id, title, artist, album, album_id, artist_id,"
                             " duration, cover_art_id, local_path, file_size, added_at, last_access, payload) "
                             "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
                             "ON CONFLICT(song_id) DO UPDATE SET "
                             " title=excluded.title, artist=excluded.artist, album=excluded.album,"
                             " album_id=excluded.album_id, artist_id=excluded.artist_id, duration=excluded.duration,"
                             " cover_art_id=excluded.cover_art_id, local_path=excluded.local_path,"
                             " file_size=excluded.file_size, last_access=excluded.last_access,"
                             " payload=excluded.payload"));
    const qint64 ts = nowMs();
    q.addBindValue(song.id());
    q.addBindValue(song.title());
    q.addBindValue(song.artist());
    q.addBindValue(song.album());
    q.addBindValue(song.albumId());
    q.addBindValue(song.artistId());
    q.addBindValue(song.duration());
    q.addBindValue(song.coverArtId());
    q.addBindValue(localPath);
    q.addBindValue(fileSize);
    q.addBindValue(ts);
    q.addBindValue(ts);
    q.addBindValue(jsonToString(song.toJson()));
    execQuery(q);
}

void DatabaseManager::removeCachedSong(const QString &songId)
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("DELETE FROM cached_songs WHERE song_id = ?"));
    q.addBindValue(songId);
    execQuery(q);
}

void DatabaseManager::clearCachedSongs()
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("DELETE FROM cached_songs"));
    execQuery(q);
}

QVector<Song> DatabaseManager::cachedSongs() const
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT payload, local_path, file_size FROM cached_songs ORDER BY last_access DESC"));
    QVector<Song> result;
    if (!execQuery(q))
        return result;
    while (q.next()) {
        Song s = Song::fromJson(stringToJson(q.value(0).toString()));
        if (!s.isValid())
            continue;
        s.setLocalPath(q.value(1).toString());
        s.setSize(q.value(2).toLongLong());
        result.append(s);
    }
    return result;
}

QString DatabaseManager::cachedSongPath(const QString &songId) const
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT local_path FROM cached_songs WHERE song_id = ?"));
    q.addBindValue(songId);
    if (!execQuery(q) || !q.next())
        return QString();
    return q.value(0).toString();
}

qint64 DatabaseManager::cachedSongsTotalSize() const
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT SUM(file_size) FROM cached_songs"));
    if (!execQuery(q) || !q.next())
        return 0;
    return q.value(0).toLongLong();
}

QStringList DatabaseManager::cachedSongIdsByLastAccess() const
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT song_id FROM cached_songs ORDER BY last_access ASC"));
    QStringList ids;
    if (!execQuery(q))
        return ids;
    while (q.next())
        ids.append(q.value(0).toString());
    return ids;
}

void DatabaseManager::touchCachedSong(const QString &songId)
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("UPDATE cached_songs SET last_access = ? WHERE song_id = ?"));
    q.addBindValue(nowMs());
    q.addBindValue(songId);
    execQuery(q);
}

// ---------------------------------------------------------------- playlists
void DatabaseManager::savePlaylists(const QVector<Playlist> &playlists)
{
    QSqlDatabase db = connection();
    db.transaction();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("INSERT INTO playlists (id, name, song_count, duration, payload, updated_at) "
                             "VALUES (?, ?, ?, ?, ?, ?) "
                             "ON CONFLICT(id) DO UPDATE SET name=excluded.name, song_count=excluded.song_count,"
                             " duration=excluded.duration, payload=excluded.payload, updated_at=excluded.updated_at"));
    const qint64 ts = nowMs();
    for (const Playlist &p : playlists) {
        if (!p.isValid())
            continue;
        q.addBindValue(p.id());
        q.addBindValue(p.name());
        q.addBindValue(p.songCount());
        q.addBindValue(p.duration());
        q.addBindValue(jsonToString(p.toJson()));
        q.addBindValue(ts);
        if (!q.exec()) {
            qWarning("[DatabaseManager] 保存播放列表失败: %s", qPrintable(q.lastError().text()));
        }
    }
    db.commit();
}

QVector<Playlist> DatabaseManager::playlists() const
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT payload FROM playlists ORDER BY updated_at DESC"));
    QVector<Playlist> result;
    if (!execQuery(q))
        return result;
    while (q.next()) {
        const Playlist p = Playlist::fromJson(stringToJson(q.value(0).toString()));
        if (p.isValid())
            result.append(p);
    }
    return result;
}

void DatabaseManager::removePlaylist(const QString &id)
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("DELETE FROM playlists WHERE id = ?"));
    q.addBindValue(id);
    execQuery(q);
}

// ------------------------------------------------------------------ session
void DatabaseManager::saveSession(const QString &payload)
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("INSERT INTO session (id, payload) VALUES (1, ?) "
                             "ON CONFLICT(id) DO UPDATE SET payload=excluded.payload"));
    q.addBindValue(payload);
    execQuery(q);
}

QString DatabaseManager::loadSession() const
{
    QSqlDatabase db = connection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT payload FROM session WHERE id = 1"));
    if (!execQuery(q) || !q.next())
        return QString();
    return q.value(0).toString();
}

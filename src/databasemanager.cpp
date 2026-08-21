#include "databasemanager.h"
#include <QStandardPaths>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QUrl>
#include <QUrlQuery>
#include <QUuid>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

namespace {
constexpr const char *kPlaybackSessionKey = "playbackSession";

QJsonObject songToJson(const Song &song)
{
    QJsonObject obj;
    obj["id"] = song.id();
    obj["title"] = song.title();
    obj["artist"] = song.artist();
    obj["album"] = song.album();
    obj["album_id"] = song.albumId();
    obj["artist_id"] = song.artistId();
    obj["track"] = song.track();
    obj["year"] = song.year();
    obj["genre"] = song.genre();
    obj["duration"] = static_cast<double>(song.duration());
    obj["size"] = static_cast<double>(song.size());
    obj["suffix"] = song.suffix();
    obj["content_type"] = song.contentType();
    obj["cover_art_id"] = song.coverArtId();
    obj["path"] = song.path();
    return obj;
}

Song songFromJson(const QJsonObject &obj)
{
    Song song;
    song.setId(obj.value("id").toString());
    song.setTitle(obj.value("title").toString());
    song.setArtist(obj.value("artist").toString());
    song.setAlbum(obj.value("album").toString());
    song.setAlbumId(obj.value("album_id").toString());
    song.setArtistId(obj.value("artist_id").toString());
    song.setTrack(obj.value("track").toInt());
    song.setYear(obj.value("year").toInt());
    song.setGenre(obj.value("genre").toString());
    song.setDuration(static_cast<qint64>(obj.value("duration").toDouble()));
    song.setSize(static_cast<qint64>(obj.value("size").toDouble()));
    song.setSuffix(obj.value("suffix").toString());
    song.setContentType(obj.value("content_type").toString());
    song.setCoverArtId(obj.value("cover_art_id").toString());
    song.setPath(obj.value("path").toString());
    return song;
}
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::init()
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString dbPath = dataPath + QDir::separator() + "naviPlayer.db";
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qWarning() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }

    return createTables();
}

bool DatabaseManager::createTables()
{
    QStringList queries;

    queries << "CREATE TABLE IF NOT EXISTS config ("
               "key TEXT PRIMARY KEY,"
               "value TEXT)";

    queries << "CREATE TABLE IF NOT EXISTS favorites ("
               "id TEXT PRIMARY KEY,"
               "title TEXT,"
               "artist TEXT,"
               "album TEXT,"
               "album_id TEXT,"
               "artist_id TEXT,"
               "track INTEGER,"
               "year INTEGER,"
               "genre TEXT,"
               "duration INTEGER,"
               "size INTEGER,"
               "suffix TEXT,"
               "content_type TEXT,"
               "cover_art_id TEXT,"
               "path TEXT,"
               "added_at INTEGER DEFAULT CURRENT_TIMESTAMP)";

    queries << "CREATE TABLE IF NOT EXISTS history ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "song_id TEXT,"
               "title TEXT,"
               "artist TEXT,"
               "album TEXT,"
               "album_id TEXT,"
               "artist_id TEXT,"
               "track INTEGER,"
               "year INTEGER,"
               "genre TEXT,"
               "duration INTEGER,"
               "size INTEGER,"
               "suffix TEXT,"
               "content_type TEXT,"
               "cover_art_id TEXT,"
               "path TEXT,"
               "played_at INTEGER DEFAULT CURRENT_TIMESTAMP)";

    queries << "CREATE TABLE IF NOT EXISTS play_states ("
               "song_id TEXT PRIMARY KEY,"
               "position INTEGER,"
               "updated_at INTEGER DEFAULT CURRENT_TIMESTAMP)";

    queries << "CREATE TABLE IF NOT EXISTS playlists ("
               "id TEXT PRIMARY KEY,"
               "name TEXT,"
               "comment TEXT,"
               "play_mode INTEGER DEFAULT 0,"
               "created_at INTEGER DEFAULT CURRENT_TIMESTAMP,"
               "updated_at INTEGER DEFAULT CURRENT_TIMESTAMP)";

    queries << "CREATE TABLE IF NOT EXISTS playlist_songs ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "playlist_id TEXT,"
               "song_id TEXT,"
               "title TEXT,"
               "artist TEXT,"
               "album TEXT,"
               "album_id TEXT,"
               "artist_id TEXT,"
               "track INTEGER,"
               "year INTEGER,"
               "genre TEXT,"
               "duration INTEGER,"
               "size INTEGER,"
               "suffix TEXT,"
               "content_type TEXT,"
               "cover_art_id TEXT,"
               "path TEXT,"
               "position INTEGER,"
               "FOREIGN KEY (playlist_id) REFERENCES playlists(id) ON DELETE CASCADE)";

    queries << "CREATE TABLE IF NOT EXISTS events ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "ts INTEGER,"
               "name TEXT,"
               "payload TEXT)";

    for (const QString &query : queries) {
        if (!executeQuery(query)) {
            return false;
        }
    }

    return true;
}

bool DatabaseManager::logEvent(const QString &name, const QMap<QString, QString> &payload)
{
    if (name.trimmed().isEmpty()) {
        return false;
    }

    QUrlQuery query;
    for (auto it = payload.begin(); it != payload.end(); ++it) {
        query.addQueryItem(it.key(), it.value());
    }

    QMap<QString, QVariant> params;
    params[":ts"] = QDateTime::currentSecsSinceEpoch();
    params[":name"] = name.trimmed();
    params[":payload"] = query.toString(QUrl::FullyEncoded);

    return executeQuery("INSERT INTO events (ts, name, payload) VALUES (:ts, :name, :payload)", params);
}

bool DatabaseManager::executeQuery(const QString &query, const QMap<QString, QVariant> &params)
{
    QSqlQuery q;
    if (!q.prepare(query)) {
        qWarning() << "Failed to prepare query:" << q.lastError().text();
        qWarning() << "Query:" << query;
        return false;
    }

    for (auto it = params.begin(); it != params.end(); ++it) {
        q.bindValue(it.key(), it.value());
    }

    if (!q.exec()) {
        qWarning() << "Failed to execute query:" << q.lastError().text();
        qWarning() << "Query:" << query;
        return false;
    }

    return true;
}

bool DatabaseManager::saveConfig(const QString &key, const QVariant &value)
{
    QMap<QString, QVariant> params;
    params[":key"] = key;
    params[":value"] = value.toString();

    QString query = "INSERT OR REPLACE INTO config (key, value) VALUES (:key, :value)";
    return executeQuery(query, params);
}

QVariant DatabaseManager::loadConfig(const QString &key, const QVariant &defaultValue)
{
    QSqlQuery q;
    q.prepare("SELECT value FROM config WHERE key = :key");
    q.bindValue(":key", key);

    if (q.exec() && q.next()) {
        return q.value(0);
    }

    return defaultValue;
}

bool DatabaseManager::addFavorite(const Song &song)
{
    QMap<QString, QVariant> params;
    params[":id"] = song.id();
    params[":title"] = song.title();
    params[":artist"] = song.artist();
    params[":album"] = song.album();
    params[":album_id"] = song.albumId();
    params[":artist_id"] = song.artistId();
    params[":track"] = song.track();
    params[":year"] = song.year();
    params[":genre"] = song.genre();
    params[":duration"] = song.duration();
    params[":size"] = song.size();
    params[":suffix"] = song.suffix();
    params[":content_type"] = song.contentType();
    params[":cover_art_id"] = song.coverArtId();
    params[":path"] = song.path();

    QString query = "INSERT OR REPLACE INTO favorites "
                    "(id, title, artist, album, album_id, artist_id, track, year, genre, "
                    "duration, size, suffix, content_type, cover_art_id, path) "
                    "VALUES (:id, :title, :artist, :album, :album_id, :artist_id, :track, :year, :genre, "
                    ":duration, :size, :suffix, :content_type, :cover_art_id, :path)";

    return executeQuery(query, params);
}

bool DatabaseManager::removeFavorite(const QString &songId)
{
    QMap<QString, QVariant> params;
    params[":id"] = songId;
    return executeQuery("DELETE FROM favorites WHERE id = :id", params);
}

bool DatabaseManager::isFavorite(const QString &songId)
{
    QSqlQuery q;
    q.prepare("SELECT id FROM favorites WHERE id = :id");
    q.bindValue(":id", songId);
    return q.exec() && q.next();
}

QVector<Song> DatabaseManager::getFavorites()
{
    QVector<Song> songs;
    QSqlQuery q;
    q.exec("SELECT id, title, artist, album, album_id, artist_id, track, year, genre, "
           "duration, size, suffix, content_type, cover_art_id, path "
           "FROM favorites ORDER BY added_at DESC");

    while (q.next()) {
        Song song;
        song.setId(q.value(0).toString());
        song.setTitle(q.value(1).toString());
        song.setArtist(q.value(2).toString());
        song.setAlbum(q.value(3).toString());
        song.setAlbumId(q.value(4).toString());
        song.setArtistId(q.value(5).toString());
        song.setTrack(q.value(6).toInt());
        song.setYear(q.value(7).toInt());
        song.setGenre(q.value(8).toString());
        song.setDuration(q.value(9).toLongLong());
        song.setSize(q.value(10).toLongLong());
        song.setSuffix(q.value(11).toString());
        song.setContentType(q.value(12).toString());
        song.setCoverArtId(q.value(13).toString());
        song.setPath(q.value(14).toString());
        song.setFavorite(true);
        songs.append(song);
    }

    return songs;
}

bool DatabaseManager::addToHistory(const Song &song)
{
    QMap<QString, QVariant> params;
    params[":song_id"] = song.id();
    params[":title"] = song.title();
    params[":artist"] = song.artist();
    params[":album"] = song.album();
    params[":album_id"] = song.albumId();
    params[":artist_id"] = song.artistId();
    params[":track"] = song.track();
    params[":year"] = song.year();
    params[":genre"] = song.genre();
    params[":duration"] = song.duration();
    params[":size"] = song.size();
    params[":suffix"] = song.suffix();
    params[":content_type"] = song.contentType();
    params[":cover_art_id"] = song.coverArtId();
    params[":path"] = song.path();

    QString query = "INSERT INTO history "
                    "(song_id, title, artist, album, album_id, artist_id, track, year, genre, "
                    "duration, size, suffix, content_type, cover_art_id, path) "
                    "VALUES (:song_id, :title, :artist, :album, :album_id, :artist_id, :track, :year, :genre, "
                    ":duration, :size, :suffix, :content_type, :cover_art_id, :path)";

    return executeQuery(query, params);
}

QVector<Song> DatabaseManager::getHistory(int limit)
{
    QVector<Song> songs;
    QSqlQuery q;
    q.prepare("SELECT DISTINCT song_id, title, artist, album, album_id, artist_id, track, year, genre, "
              "duration, size, suffix, content_type, cover_art_id, path "
              "FROM history ORDER BY played_at DESC LIMIT :limit");
    q.bindValue(":limit", limit);

    if (q.exec()) {
        while (q.next()) {
            Song song;
            song.setId(q.value(0).toString());
            song.setTitle(q.value(1).toString());
            song.setArtist(q.value(2).toString());
            song.setAlbum(q.value(3).toString());
            song.setAlbumId(q.value(4).toString());
            song.setArtistId(q.value(5).toString());
            song.setTrack(q.value(6).toInt());
            song.setYear(q.value(7).toInt());
            song.setGenre(q.value(8).toString());
            song.setDuration(q.value(9).toLongLong());
            song.setSize(q.value(10).toLongLong());
            song.setSuffix(q.value(11).toString());
            song.setContentType(q.value(12).toString());
            song.setCoverArtId(q.value(13).toString());
            song.setPath(q.value(14).toString());
            songs.append(song);
        }
    }

    return songs;
}

bool DatabaseManager::clearHistory()
{
    return executeQuery("DELETE FROM history");
}

bool DatabaseManager::savePlayState(const QString &songId, qint64 position)
{
    QMap<QString, QVariant> params;
    params[":song_id"] = songId;
    params[":position"] = position;

    QString query = "INSERT OR REPLACE INTO play_states (song_id, position) VALUES (:song_id, :position)";
    return executeQuery(query, params);
}

qint64 DatabaseManager::loadPlayState(const QString &songId)
{
    QSqlQuery q;
    q.prepare("SELECT position FROM play_states WHERE song_id = :song_id");
    q.bindValue(":song_id", songId);

    if (q.exec() && q.next()) {
        return q.value(0).toLongLong();
    }

    return 0;
}

bool DatabaseManager::savePlaybackSession(const QVector<Song> &songs, int currentIndex,
                                          qint64 position, int playMode)
{
    QJsonArray songsArray;
    for (const Song &song : songs) {
        songsArray.append(songToJson(song));
    }

    QJsonObject session;
    session["songs"] = songsArray;
    session["currentIndex"] = currentIndex;
    session["position"] = static_cast<double>(position);
    session["playMode"] = playMode;

    const QByteArray data = QJsonDocument(session).toJson(QJsonDocument::Compact);
    return saveConfig(kPlaybackSessionKey, QString::fromUtf8(data));
}

bool DatabaseManager::loadPlaybackSession(QVector<Song> *songs, int *currentIndex,
                                          qint64 *position, int *playMode)
{
    if (!songs || !currentIndex || !position || !playMode) {
        return false;
    }

    songs->clear();
    *currentIndex = -1;
    *position = 0;
    *playMode = 0;

    const QVariant raw = loadConfig(kPlaybackSessionKey);
    if (!raw.isValid() || raw.toString().isEmpty()) {
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(raw.toString().toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }

    const QJsonObject session = doc.object();

    const QJsonArray songsArray = session.value("songs").toArray();
    QVector<Song> loadedSongs;
    loadedSongs.reserve(songsArray.size());
    for (const QJsonValue &value : songsArray) {
        if (!value.isObject()) {
            continue;
        }
        Song song = songFromJson(value.toObject());
        if (!song.id().isEmpty()) {
            loadedSongs.append(song);
        }
    }

    if (loadedSongs.isEmpty()) {
        return false;
    }

    const int loadedIndex = session.value("currentIndex").toInt(-1);
    if (loadedIndex < 0 || loadedIndex >= loadedSongs.size()) {
        return false;
    }

    *songs = loadedSongs;
    *currentIndex = loadedIndex;
    *position = static_cast<qint64>(session.value("position").toDouble(0.0));
    *playMode = session.value("playMode").toInt(0);
    return true;
}

bool DatabaseManager::savePlaylist(const Playlist &playlist)
{
    QMap<QString, QVariant> params;
    params[":id"] = playlist.id().isEmpty() ? QUuid::createUuid().toString() : playlist.id();
    params[":name"] = playlist.name();
    params[":comment"] = playlist.comment();
    params[":play_mode"] = static_cast<int>(playlist.playMode());

    QString query = "INSERT OR REPLACE INTO playlists (id, name, comment, play_mode, updated_at) "
                    "VALUES (:id, :name, :comment, :play_mode, CURRENT_TIMESTAMP)";

    if (!executeQuery(query, params)) {
        return false;
    }

    executeQuery("DELETE FROM playlist_songs WHERE playlist_id = :playlist_id",
                 { { ":playlist_id", params[":id"] } });

    int position = 0;
    for (const Song &song : playlist.songs()) {
        QMap<QString, QVariant> songParams;
        songParams[":playlist_id"] = params[":id"];
        songParams[":song_id"] = song.id();
        songParams[":title"] = song.title();
        songParams[":artist"] = song.artist();
        songParams[":album"] = song.album();
        songParams[":album_id"] = song.albumId();
        songParams[":artist_id"] = song.artistId();
        songParams[":track"] = song.track();
        songParams[":year"] = song.year();
        songParams[":genre"] = song.genre();
        songParams[":duration"] = song.duration();
        songParams[":size"] = song.size();
        songParams[":suffix"] = song.suffix();
        songParams[":content_type"] = song.contentType();
        songParams[":cover_art_id"] = song.coverArtId();
        songParams[":path"] = song.path();
        songParams[":position"] = position++;

        QString songQuery = "INSERT INTO playlist_songs "
                            "(playlist_id, song_id, title, artist, album, album_id, artist_id, track, year, genre, "
                            "duration, size, suffix, content_type, cover_art_id, path, position) "
                            "VALUES (:playlist_id, :song_id, :title, :artist, :album, :album_id, :artist_id, :track, :year, :genre, "
                            ":duration, :size, :suffix, :content_type, :cover_art_id, :path, :position)";

        if (!executeQuery(songQuery, songParams)) {
            return false;
        }
    }

    return true;
}

bool DatabaseManager::deletePlaylist(const QString &playlistId)
{
    QMap<QString, QVariant> params;
    params[":id"] = playlistId;
    return executeQuery("DELETE FROM playlists WHERE id = :id", params);
}

QVector<Playlist> DatabaseManager::getPlaylists()
{
    QVector<Playlist> playlists;
    QSqlQuery q;
    q.exec("SELECT id, name, comment, play_mode FROM playlists ORDER BY created_at DESC");

    while (q.next()) {
        Playlist playlist;
        playlist.setId(q.value(0).toString());
        playlist.setName(q.value(1).toString());
        playlist.setComment(q.value(2).toString());
        playlist.setPlayMode(static_cast<Playlist::PlayMode>(q.value(3).toInt()));
        playlists.append(playlist);
    }

    return playlists;
}

Playlist DatabaseManager::getPlaylist(const QString &playlistId)
{
    Playlist playlist;
    QSqlQuery q;
    q.prepare("SELECT id, name, comment, play_mode FROM playlists WHERE id = :id");
    q.bindValue(":id", playlistId);

    if (q.exec() && q.next()) {
        playlist.setId(q.value(0).toString());
        playlist.setName(q.value(1).toString());
        playlist.setComment(q.value(2).toString());
        playlist.setPlayMode(static_cast<Playlist::PlayMode>(q.value(3).toInt()));

        QSqlQuery songQuery;
        songQuery.prepare("SELECT song_id, title, artist, album, album_id, artist_id, track, year, genre, "
                          "duration, size, suffix, content_type, cover_art_id, path "
                          "FROM playlist_songs WHERE playlist_id = :playlist_id ORDER BY position");
        songQuery.bindValue(":playlist_id", playlistId);

        if (songQuery.exec()) {
            while (songQuery.next()) {
                Song song;
                song.setId(songQuery.value(0).toString());
                song.setTitle(songQuery.value(1).toString());
                song.setArtist(songQuery.value(2).toString());
                song.setAlbum(songQuery.value(3).toString());
                song.setAlbumId(songQuery.value(4).toString());
                song.setArtistId(songQuery.value(5).toString());
                song.setTrack(songQuery.value(6).toInt());
                song.setYear(songQuery.value(7).toInt());
                song.setGenre(songQuery.value(8).toString());
                song.setDuration(songQuery.value(9).toLongLong());
                song.setSize(songQuery.value(10).toLongLong());
                song.setSuffix(songQuery.value(11).toString());
                song.setContentType(songQuery.value(12).toString());
                song.setCoverArtId(songQuery.value(13).toString());
                song.setPath(songQuery.value(14).toString());
                playlist.addSong(song);
            }
        }
    }

    return playlist;
}

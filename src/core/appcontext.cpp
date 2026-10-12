#include "core/appcontext.h"

#include "core/databasemanager.h"
#include "core/appsettings.h"
#include "core/subsonicapi.h"
#include "core/cachemanager.h"
#include "core/audioplayer.h"
#include "core/playbackqueue.h"
#include "core/lyricsmanager.h"
#include "core/asynctask.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>

AppContext::AppContext(QObject *parent)
    : QObject(parent)
{
}

AppContext *AppContext::instance()
{
    static AppContext *ctx = new AppContext(qApp);
    return ctx;
}

bool AppContext::init()
{
    m_db = new DatabaseManager(this);
    if (!m_db->init()) {
        qWarning("[AppContext] 数据库初始化失败");
        return false;
    }

    m_settings = new AppSettings(m_db, this);
    m_api = new SubsonicApi(this);
    m_cache = new CacheManager(m_db, m_api, m_settings, this);
    m_queue = new PlaybackQueue(this);
    m_player = new AudioPlayer(m_cache, m_settings, m_db, this);
    m_player->setQueue(m_queue);

    // 播放过（播完或播放过半）向服务器上报，用于统计播放次数/最近播放
    connect(m_player, &AudioPlayer::songPlayed, this, [this](const Song &song) {
        if (m_api && m_api->isConfigured() && song.isValid())
            m_api->scrobble(song.id());
    });
    m_lyrics = new LyricsManager(m_settings, m_api, m_cache, this);

    applyServerSettings();

    // 网络数据自动落入本地缓存（后台线程写库）
    connect(m_api, &SubsonicApi::artistsReady, this, [this](const QVector<Artist> &artists) {
        Async::run(this, [this, artists]() -> bool {
            m_db->saveArtists(artists);
            return true;
        }, [](bool) {});
    });

    connect(m_api, &SubsonicApi::albumListReady, this,
            [this](const QString &, int, const QVector<Album> &albums) {
                Async::run(this, [this, albums]() -> bool {
                    m_db->saveAlbums(albums);
                    return true;
                }, [this](bool) { m_albumCoverCache.clear(); });
            });

    connect(m_api, &SubsonicApi::artistReady, this,
            [this](const Artist &artist, const QVector<Album> &albums) {
                Async::run(this, [this, artist, albums]() -> bool {
                    m_db->saveArtists({artist});
                    m_db->saveAlbums(albums);
                    return true;
                }, [this](bool) { m_albumCoverCache.clear(); });
            });

    connect(m_api, &SubsonicApi::albumReady, this,
            [this](const Album &album, const QVector<Song> &songs) {
                Async::run(this, [this, album, songs]() -> bool {
                    m_db->saveAlbums({album});
                    m_db->saveSongs(songs);
                    return true;
                }, [this](bool) { m_albumCoverCache.clear(); });
            });

    connect(m_api, &SubsonicApi::songsReady, this, [this](const QVector<Song> &songs) {
        Async::run(this, [this, songs]() -> bool {
            m_db->saveSongs(songs);
            return true;
        }, [](bool) {});
    });

    connect(m_api, &SubsonicApi::playlistsReady, this, [this](const QVector<Playlist> &playlists) {
        m_playlists = playlists;
        Q_EMIT playlistsChanged();
        Async::run(this, [this, playlists]() -> bool {
            m_db->savePlaylists(playlists);
            return true;
        }, [](bool) {});
    });

    connect(m_api, &SubsonicApi::starredReady, this,
            [this](const QVector<Song> &songs, const QVector<Album> &albums,
                   const QVector<Artist> &artists) {
                Async::run(this, [this, songs, albums, artists]() -> bool {
                    QJsonArray songArr;
                    for (const Song &s : songs)
                        songArr.append(s.toJson());
                    QJsonArray albumArr;
                    for (const Album &a : albums)
                        albumArr.append(a.toJson());
                    QJsonArray artistArr;
                    for (const Artist &a : artists)
                        artistArr.append(a.toJson());

                    m_db->replaceFavorites(QStringLiteral("song"), songArr);
                    m_db->replaceFavorites(QStringLiteral("album"), albumArr);
                    m_db->replaceFavorites(QStringLiteral("artist"), artistArr);
                    m_db->saveSongs(songs);
                    m_db->saveAlbums(albums);
                    m_db->saveArtists(artists);
                    return true;
                }, [this](bool) {
                    Q_EMIT favoritesChanged();
                });
            });

    connect(m_settings, &AppSettings::settingChanged, this, [this](const QString &key) {
        if (key.startsWith(QLatin1String("server/")))
            applyServerSettings();
    });

    Q_EMIT initialized();
    return true;
}

void AppContext::shutdown()
{
    if (m_player)
        m_player->stop();
}

void AppContext::applyServerSettings()
{
    if (!m_api || !m_settings)
        return;
    m_api->setServer(m_settings->serverUrl(), m_settings->username(), m_settings->password());
    m_api->setIgnoreSslErrors(m_settings->ignoreSslErrors());
}

void AppContext::refreshFavorites()
{
    if (m_api && m_api->isConfigured())
        m_api->getStarred();
}

void AppContext::refreshPlaylists()
{
    if (m_api && m_api->isConfigured())
        m_api->getPlaylists();
    else if (m_db)
        m_playlists = m_db->playlists();
}

bool AppContext::isFavorite(const QString &type, const QString &id) const
{
    return m_db ? m_db->isFavorite(type, id) : false;
}

QString AppContext::albumCoverArtId(const QString &albumId) const
{
    if (albumId.isEmpty())
        return QString();

    const auto it = m_albumCoverCache.constFind(albumId);
    if (it != m_albumCoverCache.constEnd())
        return it.value();

    const QString cover = m_db ? m_db->albumCoverArtId(albumId) : QString();
    m_albumCoverCache.insert(albumId, cover);
    return cover;
}

QString AppContext::coverArtIdForSong(const Song &song) const
{
    const QString albumCover = albumCoverArtId(song.albumId());
    return albumCover.isEmpty() ? song.coverArtId() : albumCover;
}

// ------------------------------------------------------------------ 会话
void AppContext::saveSession(const SessionState &state)
{
    if (!m_db)
        return;

    QJsonArray arr;
    for (const Song &s : state.queue)
        arr.append(s.toJson());

    QJsonObject lyrics;
    lyrics[QStringLiteral("visible")] = state.lyricsVisible;
    lyrics[QStringLiteral("x")] = state.lyricsPos.x();
    lyrics[QStringLiteral("y")] = state.lyricsPos.y();

    QJsonObject root;
    root[QStringLiteral("queue")] = arr;
    root[QStringLiteral("currentIndex")] = state.currentIndex;
    root[QStringLiteral("playMode")] = state.playMode;
    root[QStringLiteral("position")] = static_cast<double>(state.positionMs);
    root[QStringLiteral("lyrics")] = lyrics;

    m_db->saveSession(QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact)));
}

bool AppContext::loadSession(SessionState *state) const
{
    if (!m_db || !state)
        return false;

    const QString payload = m_db->loadSession();
    if (payload.isEmpty())
        return false;

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(payload.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return false;

    const QJsonObject root = doc.object();
    for (const QJsonValue &v : root.value(QStringLiteral("queue")).toArray()) {
        const Song s = Song::fromJson(v.toObject());
        if (s.isValid())
            state->queue.append(s);
    }
    state->currentIndex = root.value(QStringLiteral("currentIndex")).toInt(-1);
    state->playMode = root.value(QStringLiteral("playMode")).toInt(1);
    state->positionMs = static_cast<qint64>(root.value(QStringLiteral("position")).toDouble());

    const QJsonObject lyrics = root.value(QStringLiteral("lyrics")).toObject();
    state->lyricsVisible = lyrics.value(QStringLiteral("visible")).toBool();
    state->lyricsPos = QPoint(lyrics.value(QStringLiteral("x")).toInt(),
                              lyrics.value(QStringLiteral("y")).toInt());
    return true;
}

void AppContext::setSongFavorite(const Song &song, bool favorite)
{
    if (!m_db)
        return;

    if (favorite)
        m_db->addFavorite(QStringLiteral("song"), song);
    else
        m_db->removeFavorite(QStringLiteral("song"), song.id());

    if (m_api && m_api->isConfigured()) {
        if (favorite)
            m_api->star({song.id()});
        else
            m_api->unstar({song.id()});
    }

    Q_EMIT favoritesChanged();
}

void AppContext::setAlbumFavorite(const Album &album, bool favorite)
{
    if (!m_db)
        return;

    if (favorite)
        m_db->addFavorite(QStringLiteral("album"), album.toJson());
    else
        m_db->removeFavorite(QStringLiteral("album"), album.id());

    if (m_api && m_api->isConfigured()) {
        if (favorite)
            m_api->star({}, {album.id()});
        else
            m_api->unstar({}, {album.id()});
    }

    Q_EMIT favoritesChanged();
}

void AppContext::setArtistFavorite(const Artist &artist, bool favorite)
{
    if (!m_db)
        return;

    if (favorite)
        m_db->addFavorite(QStringLiteral("artist"), artist.toJson());
    else
        m_db->removeFavorite(QStringLiteral("artist"), artist.id());

    if (m_api && m_api->isConfigured()) {
        if (favorite)
            m_api->star({}, {}, {artist.id()});
        else
            m_api->unstar({}, {}, {artist.id()});
    }

    Q_EMIT favoritesChanged();
}

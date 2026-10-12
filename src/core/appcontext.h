/**
 * @file    appcontext.h
 * @brief   全局上下文：聚合各核心服务，并负责"网络数据 -> 本地 SQLite 缓存"的自动入库
 */
#ifndef NAVIPLAYER_APPCONTEXT_H
#define NAVIPLAYER_APPCONTEXT_H

#include <QObject>
#include <QPoint>
#include <QString>
#include <QVector>
#include <QHash>

#include "models/song.h"
#include "models/album.h"
#include "models/artist.h"
#include "models/playlist.h"

class DatabaseManager;
class AppSettings;
class SubsonicApi;
class CacheManager;
class AudioPlayer;
class PlaybackQueue;
class LyricsManager;

/** 退出时保存、启动时恢复的会话状态 */
struct SessionState
{
    QVector<Song> queue;
    int currentIndex = -1;
    int playMode = 1;                 // PlaybackQueue::PlayMode（默认 RepeatAll）
    qint64 positionMs = 0;
    bool lyricsVisible = false;
    QPoint lyricsPos;
};

class AppContext : public QObject
{
    Q_OBJECT
public:
    static AppContext *instance();

    bool init();
    void shutdown();

    DatabaseManager *database() const { return m_db; }
    AppSettings *settings() const { return m_settings; }
    SubsonicApi *api() const { return m_api; }
    CacheManager *cache() const { return m_cache; }
    AudioPlayer *player() const { return m_player; }
    PlaybackQueue *queue() const { return m_queue; }
    LyricsManager *lyrics() const { return m_lyrics; }

    /** 从服务端刷新收藏（随后发出 favoritesChanged） */
    void refreshFavorites();

    /** 刷新并缓存服务端播放列表（右键菜单“添加到播放列表”用） */
    void refreshPlaylists();
    QVector<Playlist> playlists() const { return m_playlists; }

    void setSongFavorite(const Song &song, bool favorite);
    void setAlbumFavorite(const Album &album, bool favorite);
    void setArtistFavorite(const Artist &artist, bool favorite);

    bool isFavorite(const QString &type, const QString &id) const;

    /** 按专辑 id 取专辑封面 id（带内存缓存，供列表用专辑封面代替单曲封面） */
    QString albumCoverArtId(const QString &albumId) const;
    /** 队列/列表用图：优先专辑封面，缺失时回退单曲封面 */
    QString coverArtIdForSong(const Song &song) const;

    /** 应用服务器配置（设置页保存后调用） */
    void applyServerSettings();

    // ---------------- 会话（退出保存 / 启动恢复） ----------------
    void saveSession(const SessionState &state);
    bool loadSession(SessionState *state) const;

signals:
    void favoritesChanged();
    void playlistsChanged();
    void initialized();

private:
    explicit AppContext(QObject *parent = nullptr);

    DatabaseManager *m_db = nullptr;
    AppSettings *m_settings = nullptr;
    SubsonicApi *m_api = nullptr;
    CacheManager *m_cache = nullptr;
    AudioPlayer *m_player = nullptr;
    PlaybackQueue *m_queue = nullptr;
    LyricsManager *m_lyrics = nullptr;
    QVector<Playlist> m_playlists;
    mutable QHash<QString, QString> m_albumCoverCache;
};

#endif // NAVIPLAYER_APPCONTEXT_H

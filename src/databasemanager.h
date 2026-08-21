#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QMap>
#include <QSqlDatabase>
#include <QVariant>
#include "models/song.h"
#include "models/album.h"
#include "models/artist.h"
#include "models/playlist.h"

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool init();

    bool saveConfig(const QString &key, const QVariant &value);
    QVariant loadConfig(const QString &key, const QVariant &defaultValue = QVariant());

    bool addFavorite(const Song &song);
    bool removeFavorite(const QString &songId);
    bool isFavorite(const QString &songId);
    QVector<Song> getFavorites();

    bool addToHistory(const Song &song);
    QVector<Song> getHistory(int limit = 50);
    bool clearHistory();

    bool savePlayState(const QString &songId, qint64 position);
    qint64 loadPlayState(const QString &songId);

    // 保存/恢复播放会话：播放队列、当前索引、播放进度、播放模式
    bool savePlaybackSession(const QVector<Song> &songs, int currentIndex,
                             qint64 position, int playMode);
    bool loadPlaybackSession(QVector<Song> *songs, int *currentIndex,
                             qint64 *position, int *playMode);

    bool savePlaylist(const Playlist &playlist);
    bool deletePlaylist(const QString &playlistId);
    QVector<Playlist> getPlaylists();
    Playlist getPlaylist(const QString &playlistId);

    bool logEvent(const QString &name, const QMap<QString, QString> &payload = QMap<QString, QString>());

private:
    bool createTables();
    bool executeQuery(const QString &query, const QMap<QString, QVariant> &params = QMap<QString, QVariant>());

    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H

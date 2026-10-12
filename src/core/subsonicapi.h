/**
 * @file    subsonicapi.h
 * @brief   Subsonic REST API 客户端（f=json，全异步）
 *
 * 认证：token+salt（md5(password+salt)），兼容明文密码回退。
 * 参考：https://www.subsonic.org/pages/api.jsp
 */
#ifndef NAVIPLAYER_SUBSONICAPI_H
#define NAVIPLAYER_SUBSONICAPI_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QSet>
#include <QMap>

#include "models/song.h"
#include "models/album.h"
#include "models/artist.h"
#include "models/playlist.h"

struct Genre
{
    QString name;
    int songCount = 0;
    int albumCount = 0;
};
Q_DECLARE_METATYPE(Genre)

class SubsonicApi : public QObject
{
    Q_OBJECT
public:
    enum class RequestType {
        Ping,
        GetArtists,
        GetArtist,
        GetAlbum,
        GetAlbumList2,
        GetSongsByGenre,
        GetGenres,
        GetRandomSongs,
        Search3,
        GetCoverArt,
        GetStarred,
        Star,
        Unstar,
        GetPlaylists,
        GetPlaylist,
        CreatePlaylist,
        UpdatePlaylist,
        DeletePlaylist,
        Scrobble,
        GetLyrics,
    };
    Q_ENUM(RequestType)

    /** 专辑列表类型（getAlbumList2） */
    enum class AlbumListType {
        Random,
        Newest,
        Highest,        // 播放最多
        Frequent,       // 最近播放
        Recent,         // 最近添加（= Newest，保留以兼容不同服务器）
        ByYear,
        ByGenre,
        AlphabeticalByName,
        Starred,
    };
    Q_ENUM(AlbumListType)

    explicit SubsonicApi(QObject *parent = nullptr);

    void setServer(const QString &url, const QString &username, const QString &password);
    void setIgnoreSslErrors(bool ignore);
    bool isConfigured() const;

    QString serverUrl() const { return m_serverUrl; }
    QNetworkAccessManager *network() { return &m_nam; }

    // ---------------- API ----------------
    void ping();
    void getArtists();
    void getArtist(const QString &id);
    void getAlbum(const QString &id);
    void getAlbumList2(AlbumListType type, int size = 20, int offset = 0,
                       const QString &extra = QString());
    void getSongsByGenre(const QString &genre, int count = 50, int offset = 0);
    void getGenres();
    void getRandomSongs(int size = 50);
    void search3(const QString &query, int artistCount = 20, int albumCount = 20, int songCount = 50);
    void getCoverArt(const QString &coverArtId, int size = 300);
    void getStarred();
    void star(const QStringList &songIds, const QStringList &albumIds = {},
              const QStringList &artistIds = {});
    void unstar(const QStringList &songIds, const QStringList &albumIds = {},
                const QStringList &artistIds = {});
    void getPlaylists();
    void getPlaylist(const QString &id);
    void createPlaylist(const QString &name, const QStringList &songIds = {});
    void updatePlaylist(const QString &id, const QString &name = QString(),
                        const QString &comment = QString(), bool isPublic = false,
                        const QStringList &songIdsToAdd = {},
                        const QList<int> &songIndexesToRemove = {});
    void deletePlaylist(const QString &id);
    void scrobble(const QString &songId);
    void getLyrics(const QString &artist, const QString &title);

    /** 构造播放地址 */
    QUrl streamUrl(const QString &songId, int maxBitRate = 0) const;
    QUrl coverArtUrl(const QString &coverArtId, int size = 300) const;

    /** 取消未完成的封面请求（页面切换时释放连接） */
    void cancelPendingCoverArts();

    static QString typeToString(AlbumListType type);

signals:
    void pingResult(bool ok, const QString &message);
    void artistsReady(const QVector<Artist> &artists);
    void artistReady(const Artist &artist, const QVector<Album> &albums);
    void albumReady(const Album &album, const QVector<Song> &songs);
    void albumListReady(const QString &type, int offset, const QVector<Album> &albums);
    void songsReady(const QVector<Song> &songs);
    void genresReady(const QVector<Genre> &genres);
    void searchReady(const QVector<Artist> &artists, const QVector<Album> &albums,
                     const QVector<Song> &songs);
    void coverArtReady(const QString &coverArtId, const QByteArray &data);
    void starredReady(const QVector<Song> &songs, const QVector<Album> &albums,
                      const QVector<Artist> &artists);
    void starResult(bool ok, const QStringList &ids);
    void playlistsReady(const QVector<Playlist> &playlists);
    void playlistReady(const Playlist &playlist);
    void playlistChanged(const QString &playlistId, bool ok);
    void lyricsReady(const QString &artist, const QString &title, const QString &lyrics);
    void errorOccurred(const QString &message);
    /** 网络层不可达（离线上/下线只发一次） */
    void networkUnreachable(const QString &message);
    /** 请求恢复成功（回到在线） */
    void connectionRestored();

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    struct Request {
        RequestType type = RequestType::Ping;
        QString method;
        QVariantMap params;
        int retryCount = 0;
    };

    QUrl buildUrl(const QString &method, const QVariantMap &params) const;
    void sendRequest(RequestType type, const QString &method, const QVariantMap &params);
    void dispatchReply(const Request &req, const QByteArray &data);
    void handleApiError(const QJsonObject &root, const QString &method);
    void retry(const Request &req, const QString &method);

    // 解析
    static Song parseSong(const QJsonObject &o);
    static Album parseAlbum(const QJsonObject &o);
    static Artist parseArtist(const QJsonObject &o);
    static Playlist parsePlaylist(const QJsonObject &o);
    static QJsonObject responseRoot(const QByteArray &data, bool *ok = nullptr);
    static QVector<QJsonObject> objectArray(const QJsonValue &v);

    QString m_serverUrl;
    QString m_username;
    QString m_password;
    bool m_ignoreSslErrors = true;
    /** 服务器不支持 token+salt 时自动回退到明文密码认证 */
    bool m_usePlainPassword = false;
    /** 在线状态：仅在网络层由通变断（或由断变通）时发一次信号 */
    bool m_online = true;

    QNetworkAccessManager m_nam;
    QMap<QNetworkReply *, Request> m_pending;
    QSet<QString> m_pendingCoverIds;
    QSet<QNetworkReply *> m_coverReplies;
};

#endif // NAVIPLAYER_SUBSONICAPI_H

#ifndef SUBSONICAPI_H
#define SUBSONICAPI_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSet>
#include <QXmlStreamReader>
#include "models/song.h"
#include "models/album.h"
#include "models/artist.h"

class SubsonicApi : public QObject
{
    Q_OBJECT

public:
    explicit SubsonicApi(QObject *parent = nullptr);
    ~SubsonicApi();

    void setServerUrl(const QString &url);
    void setUsername(const QString &username);
    void setPassword(const QString &password);

    QString serverUrl() const { return m_serverUrl; }
    QString username() const { return m_username; }
    QString password() const { return m_password; }

    bool isConfigured() const;

    void ping();
    void getArtists();
    void getArtist(const QString &artistId);
    void getAlbums2(int size = 100, int offset = 0);
    void getAlbumListByType(const QString &type, int size = 20, int offset = 0);
    void getAlbum(const QString &albumId);
    void getSongsByGenre(const QString &genre, int count = 100);
    void getStarred();
    void search3(const QString &query, int artistCount = 20, int albumCount = 20, int songCount = 20);
    void getCoverArt(const QString &coverArtId, int size = 0);
    /// 立即中止当前所有未完成的 getCoverArt 请求，为高优先级请求（如 getArtist、getAlbum）腾出连接槽。
    void cancelPendingCoverArts();
    void stream(const QString &songId, int maxBitRate = 0);
    QString getStreamUrl(const QString &songId, int maxBitRate = 0);
    QString getCoverArtUrl(const QString &coverArtId, int size = 0);
    void star(const QStringList &songIds = QStringList(),
              const QStringList &albumIds = QStringList(),
              const QStringList &artistIds = QStringList());
    void unstar(const QStringList &songIds = QStringList(),
                const QStringList &albumIds = QStringList(),
                const QStringList &artistIds = QStringList());

signals:
    void pingFinished(bool success, const QString &error);
    void artistsReceived(const QVector<Artist> &artists);
    void artistsFailed(const QString &error);
    void artistAlbumsReceived(const Artist &artist, const QVector<Album> &albums);
    void albumsReceived(int offset, int size, const QVector<Album> &albums);
    void albumsFailed(int offset, int size, const QString &error);
    void albumListReceived(const QString &type, const QVector<Album> &albums);
    void albumListFailed(const QString &type, const QString &error);
    void albumReceived(const Album &album, const QVector<Song> &songs);
    void songsReceived(const QVector<Song> &songs);
    void starredReceived(const QVector<Song> &songs, const QVector<Album> &albums, const QVector<Artist> &artists);
    void searchResultsReceived(const QVector<Artist> &artists, const QVector<Album> &albums, const QVector<Song> &songs);
    void coverArtReceived(const QString &coverArtId, const QByteArray &data);
    void errorOccurred(const QString &error);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QString buildUrl(const QString &method, const QMap<QString, QString> &params = QMap<QString, QString>()) const;
    QNetworkReply *sendRequest(const QString &method, const QMap<QString, QString> &params = QMap<QString, QString>());
    void parseError(QXmlStreamReader &xml);
    QVector<Artist> parseArtists(QXmlStreamReader &xml);
    QVector<Album> parseAlbums(QXmlStreamReader &xml);
    QVector<Song> parseSongs(QXmlStreamReader &xml);
    Album parseAlbum(QXmlStreamReader &xml);
    Artist parseArtist(QXmlStreamReader &xml);
    Song parseSong(QXmlStreamReader &xml);
    QString md5(const QString &input) const;

    QNetworkAccessManager *m_networkManager;
    QSet<QNetworkReply *> m_pendingCoverArtReplies;
    QString m_serverUrl;
    QString m_username;
    QString m_password;
    QString m_salt;
    QString m_token;
    QString m_clientName;
    QString m_apiVersion;
};

#endif // SUBSONICAPI_H

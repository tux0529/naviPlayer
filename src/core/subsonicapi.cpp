#include "subsonicapi.h"

#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QNetworkRequest>
#include <QRandomGenerator>
#include <QSslError>
#include <QTimer>
#include <QUrlQuery>
#include <QVariant>

namespace {
const char *kApiVersion = "1.16.1";
const char *kClientName = "naviPlayer";

QString randomSalt()
{
    QByteArray bytes;
    bytes.resize(8);
    for (int i = 0; i < bytes.size(); ++i)
        bytes[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    return QString::fromLatin1(bytes.toHex());
}
}

SubsonicApi::SubsonicApi(QObject *parent)
    : QObject(parent)
{
    connect(&m_nam, &QNetworkAccessManager::finished, this, &SubsonicApi::onReplyFinished);
}

void SubsonicApi::setServer(const QString &url, const QString &username, const QString &password)
{
    m_serverUrl = url.trimmed();
    while (m_serverUrl.endsWith(QLatin1Char('/')))
        m_serverUrl.chop(1);
    if (!m_serverUrl.startsWith(QLatin1String("http"), Qt::CaseInsensitive))
        m_serverUrl.prepend(QStringLiteral("http://"));
    m_username = username;
    m_password = password;
}

void SubsonicApi::setIgnoreSslErrors(bool ignore)
{
    m_ignoreSslErrors = ignore;
}

bool SubsonicApi::isConfigured() const
{
    return !m_serverUrl.isEmpty() && !m_username.isEmpty() && !m_password.isEmpty();
}

QString SubsonicApi::typeToString(AlbumListType type)
{
    switch (type) {
    case AlbumListType::Random:              return QStringLiteral("random");
    case AlbumListType::Newest:              return QStringLiteral("newest");
    case AlbumListType::Highest:             return QStringLiteral("highest");
    case AlbumListType::Frequent:            return QStringLiteral("frequent");
    case AlbumListType::Recent:              return QStringLiteral("recent");
    case AlbumListType::ByYear:              return QStringLiteral("byYear");
    case AlbumListType::ByGenre:             return QStringLiteral("byGenre");
    case AlbumListType::AlphabeticalByName:  return QStringLiteral("alphabeticalByName");
    case AlbumListType::Starred:             return QStringLiteral("starred");
    }
    return QStringLiteral("random");
}

QUrl SubsonicApi::buildUrl(const QString &method, const QVariantMap &params) const
{
    const QString base = m_serverUrl + QStringLiteral("/rest/") + method + QStringLiteral(".view");
    QUrl url(base);

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("u"), m_username);
    if (m_usePlainPassword) {
        // 旧版服务器不支持 token 认证时的回退：p=明文密码
        query.addQueryItem(QStringLiteral("p"), m_password);
    } else {
        const QString salt = randomSalt();
        const QString token = QString::fromLatin1(
            QCryptographicHash::hash((m_password + salt).toUtf8(), QCryptographicHash::Md5).toHex());
        query.addQueryItem(QStringLiteral("t"), token);
        query.addQueryItem(QStringLiteral("s"), salt);
    }
    query.addQueryItem(QStringLiteral("v"), QString::fromLatin1(kApiVersion));
    query.addQueryItem(QStringLiteral("c"), QString::fromLatin1(kClientName));
    query.addQueryItem(QStringLiteral("f"), QStringLiteral("json"));

    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        if (it.value().canConvert<QStringList>()) {
            const QStringList list = it.value().toStringList();
            for (const QString &v : list)
                query.addQueryItem(it.key(), v);
        } else if (it.value().canConvert<QVariantList>()) {
            const QVariantList list = it.value().toList();
            for (const QVariant &v : list)
                query.addQueryItem(it.key(), v.toString());
        } else {
            query.addQueryItem(it.key(), it.value().toString());
        }
    }

    url.setQuery(query);
    return url;
}

void SubsonicApi::sendRequest(RequestType type, const QString &method, const QVariantMap &params)
{
    if (!isConfigured()) {
        Q_EMIT errorOccurred(QStringLiteral("服务器未配置"));
        return;
    }

    Request req;
    req.type = type;
    req.method = method;
    req.params = params;

    const QUrl url = buildUrl(method, params);
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("User-Agent", kClientName);

    qDebug().noquote() << "[SubsonicApi] ->" << method
                       << url.toString(QUrl::RemoveQuery)
                       << "params:" << params;

    QNetworkReply *reply = m_nam.get(request);
    if (!reply)
        return;

    if (m_ignoreSslErrors) {
        connect(reply, &QNetworkReply::sslErrors, this, [reply](const QList<QSslError> &) {
            reply->ignoreSslErrors();
        });
    }

    m_pending.insert(reply, req);

    if (type == RequestType::GetCoverArt) {
        const QString id = params.value(QStringLiteral("id")).toString();
        if (!id.isEmpty())
            m_pendingCoverIds.insert(id);
        m_coverReplies.insert(reply);
    }
}

void SubsonicApi::retry(const Request &req, const QString &method)
{
    if (req.retryCount >= 1)
        return;

    Request copy = req;
    copy.retryCount += 1;

    QTimer::singleShot(300, this, [this, copy, method]() {
        const QUrl url = buildUrl(method, copy.params);
        QNetworkRequest request(url);
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                             QNetworkRequest::NoLessSafeRedirectPolicy);
        QNetworkReply *reply = m_nam.get(request);
        if (!reply)
            return;
        if (m_ignoreSslErrors) {
            connect(reply, &QNetworkReply::sslErrors, this, [reply](const QList<QSslError> &) {
                reply->ignoreSslErrors();
            });
        }
        m_pending.insert(reply, copy);
    });
}

// ------------------------------------------------------------------ 请求入口
void SubsonicApi::ping()
{
    sendRequest(RequestType::Ping, QStringLiteral("ping"), {});
}

void SubsonicApi::getArtists()
{
    sendRequest(RequestType::GetArtists, QStringLiteral("getArtists"), {});
}

void SubsonicApi::getArtist(const QString &id)
{
    sendRequest(RequestType::GetArtist, QStringLiteral("getArtist"), {{QStringLiteral("id"), id}});
}

void SubsonicApi::getAlbum(const QString &id)
{
    sendRequest(RequestType::GetAlbum, QStringLiteral("getAlbum"), {{QStringLiteral("id"), id}});
}

void SubsonicApi::getAlbumList2(AlbumListType type, int size, int offset, const QString &extra)
{
    QVariantMap params;
    params[QStringLiteral("type")] = typeToString(type);
    params[QStringLiteral("size")] = size;
    params[QStringLiteral("offset")] = offset;
    if (!extra.isEmpty()) {
        if (type == AlbumListType::ByYear)
            params[QStringLiteral("fromYear")] = extra;
        else if (type == AlbumListType::ByGenre)
            params[QStringLiteral("genre")] = extra;
    }
    sendRequest(RequestType::GetAlbumList2, QStringLiteral("getAlbumList2"), params);
}

void SubsonicApi::getSongsByGenre(const QString &genre, int count, int offset)
{
    QVariantMap params;
    params[QStringLiteral("genre")] = genre;
    params[QStringLiteral("count")] = count;
    params[QStringLiteral("offset")] = offset;
    sendRequest(RequestType::GetSongsByGenre, QStringLiteral("getSongsByGenre"), params);
}

void SubsonicApi::getGenres()
{
    sendRequest(RequestType::GetGenres, QStringLiteral("getGenres"), {});
}

void SubsonicApi::getRandomSongs(int size)
{
    QVariantMap params;
    params[QStringLiteral("size")] = size;
    sendRequest(RequestType::GetRandomSongs, QStringLiteral("getRandomSongs"), params);
}

void SubsonicApi::search3(const QString &query, int artistCount, int albumCount, int songCount)
{
    QVariantMap params;
    params[QStringLiteral("query")] = query;
    params[QStringLiteral("artistCount")] = artistCount;
    params[QStringLiteral("albumCount")] = albumCount;
    params[QStringLiteral("songCount")] = songCount;
    sendRequest(RequestType::Search3, QStringLiteral("search3"), params);
}

void SubsonicApi::getCoverArt(const QString &coverArtId, int size)
{
    if (coverArtId.isEmpty())
        return;
    if (m_pendingCoverIds.contains(coverArtId))
        return;     // 去重：同一封面不重复请求
    QVariantMap params;
    params[QStringLiteral("id")] = coverArtId;
    if (size > 0)
        params[QStringLiteral("size")] = size;
    sendRequest(RequestType::GetCoverArt, QStringLiteral("getCoverArt"), params);
}

void SubsonicApi::getStarred()
{
    sendRequest(RequestType::GetStarred, QStringLiteral("getStarred2"), {});
}

void SubsonicApi::star(const QStringList &songIds, const QStringList &albumIds,
                       const QStringList &artistIds)
{
    QVariantMap params;
    if (!songIds.isEmpty())
        params[QStringLiteral("id")] = songIds;
    if (!albumIds.isEmpty())
        params[QStringLiteral("albumId")] = albumIds;
    if (!artistIds.isEmpty())
        params[QStringLiteral("artistId")] = artistIds;
    sendRequest(RequestType::Star, QStringLiteral("star"), params);
}

void SubsonicApi::unstar(const QStringList &songIds, const QStringList &albumIds,
                         const QStringList &artistIds)
{
    QVariantMap params;
    if (!songIds.isEmpty())
        params[QStringLiteral("id")] = songIds;
    if (!albumIds.isEmpty())
        params[QStringLiteral("albumId")] = albumIds;
    if (!artistIds.isEmpty())
        params[QStringLiteral("artistId")] = artistIds;
    sendRequest(RequestType::Unstar, QStringLiteral("unstar"), params);
}

void SubsonicApi::getPlaylists()
{
    sendRequest(RequestType::GetPlaylists, QStringLiteral("getPlaylists"), {});
}

void SubsonicApi::getPlaylist(const QString &id)
{
    sendRequest(RequestType::GetPlaylist, QStringLiteral("getPlaylist"), {{QStringLiteral("id"), id}});
}

void SubsonicApi::createPlaylist(const QString &name, const QStringList &songIds)
{
    QVariantMap params;
    params[QStringLiteral("name")] = name;
    if (!songIds.isEmpty())
        params[QStringLiteral("songId")] = songIds;
    sendRequest(RequestType::CreatePlaylist, QStringLiteral("createPlaylist"), params);
}

void SubsonicApi::updatePlaylist(const QString &id, const QString &name, const QString &comment,
                                 bool isPublic, const QStringList &songIdsToAdd,
                                 const QList<int> &songIndexesToRemove)
{
    QVariantMap params;
    params[QStringLiteral("playlistId")] = id;
    if (!name.isEmpty())
        params[QStringLiteral("name")] = name;
    if (!comment.isEmpty())
        params[QStringLiteral("comment")] = comment;
    params[QStringLiteral("public")] = isPublic ? QStringLiteral("true") : QStringLiteral("false");
    if (!songIdsToAdd.isEmpty())
        params[QStringLiteral("songIdToAdd")] = songIdsToAdd;
    if (!songIndexesToRemove.isEmpty()) {
        QStringList idx;
        for (int i : songIndexesToRemove)
            idx.append(QString::number(i));
        params[QStringLiteral("songIndexToRemove")] = idx;
    }
    sendRequest(RequestType::UpdatePlaylist, QStringLiteral("updatePlaylist"), params);
}

void SubsonicApi::deletePlaylist(const QString &id)
{
    sendRequest(RequestType::DeletePlaylist, QStringLiteral("deletePlaylist"),
                {{QStringLiteral("id"), id}});
}

void SubsonicApi::scrobble(const QString &songId)
{
    sendRequest(RequestType::Scrobble, QStringLiteral("scrobble"), {{QStringLiteral("id"), songId}});
}

void SubsonicApi::getLyrics(const QString &artist, const QString &title)
{
    QVariantMap params;
    params[QStringLiteral("artist")] = artist;
    params[QStringLiteral("title")] = title;
    sendRequest(RequestType::GetLyrics, QStringLiteral("getLyrics"), params);
}

QUrl SubsonicApi::streamUrl(const QString &songId, int maxBitRate) const
{
    QVariantMap params;
    params[QStringLiteral("id")] = songId;
    if (maxBitRate > 0)
        params[QStringLiteral("maxBitRate")] = maxBitRate;
    return buildUrl(QStringLiteral("stream"), params);
}

QUrl SubsonicApi::coverArtUrl(const QString &coverArtId, int size) const
{
    QVariantMap params;
    params[QStringLiteral("id")] = coverArtId;
    if (size > 0)
        params[QStringLiteral("size")] = size;
    return buildUrl(QStringLiteral("getCoverArt"), params);
}

void SubsonicApi::cancelPendingCoverArts()
{
    for (QNetworkReply *reply : m_coverReplies) {
        if (m_pending.contains(reply)) {
            const Request req = m_pending.take(reply);
            m_pendingCoverIds.remove(req.params.value(QStringLiteral("id")).toString());
        }
        reply->abort();
    }
    m_coverReplies.clear();
}

// ------------------------------------------------------------------ 解析工具
QJsonObject SubsonicApi::responseRoot(const QByteArray &data, bool *ok)
{
    if (ok)
        *ok = false;
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError)
        return QJsonObject();
    if (ok)
        *ok = true;
    return doc.object().value(QStringLiteral("subsonic-response")).toObject();
}

QVector<QJsonObject> SubsonicApi::objectArray(const QJsonValue &v)
{
    QVector<QJsonObject> result;
    if (v.isArray()) {
        const QJsonArray arr = v.toArray();
        for (const QJsonValue &item : arr) {
            if (item.isObject())
                result.append(item.toObject());
        }
    } else if (v.isObject()) {
        result.append(v.toObject());
    }
    return result;
}

Song SubsonicApi::parseSong(const QJsonObject &o)
{
    Song s;
    s.setId(o.value(QStringLiteral("id")).toString());
    s.setTitle(o.value(QStringLiteral("title")).toString());
    s.setArtist(o.value(QStringLiteral("artist")).toString());
    s.setArtistId(o.value(QStringLiteral("artistId")).toString());
    s.setAlbum(o.value(QStringLiteral("album")).toString());
    s.setAlbumId(o.value(QStringLiteral("albumId")).toString());
    s.setTrack(o.value(QStringLiteral("track")).toInt());
    s.setDiscNumber(o.value(QStringLiteral("discNumber")).toInt(1));
    s.setYear(o.value(QStringLiteral("year")).toInt());
    s.setGenre(o.value(QStringLiteral("genre")).toString());
    s.setDuration(o.value(QStringLiteral("duration")).toInt());
    s.setSize(static_cast<qint64>(o.value(QStringLiteral("size")).toDouble()));
    s.setSuffix(o.value(QStringLiteral("suffix")).toString());
    s.setContentType(o.value(QStringLiteral("contentType")).toString());
    s.setCoverArtId(o.value(QStringLiteral("coverArt")).toString());
    s.setPath(o.value(QStringLiteral("path")).toString());
    s.setBitRate(o.value(QStringLiteral("bitRate")).toInt());
    s.setSampleRate(o.value(QStringLiteral("sampleRate")).toInt());
    s.setChannels(o.value(QStringLiteral("channelCount")).toInt());
    s.setPlayCount(o.value(QStringLiteral("playCount")).toInt());
    s.setCreated(o.value(QStringLiteral("created")).toString());
    s.setFavorite(o.value(QStringLiteral("starred")).toBool());
    if (s.title().isEmpty())
        s.setTitle(o.value(QStringLiteral("name")).toString());
    return s;
}

Album SubsonicApi::parseAlbum(const QJsonObject &o)
{
    Album a;
    a.setId(o.value(QStringLiteral("id")).toString());
    a.setName(o.value(QStringLiteral("name")).toString(o.value(QStringLiteral("title")).toString()));
    a.setArtist(o.value(QStringLiteral("artist")).toString());
    a.setArtistId(o.value(QStringLiteral("artistId")).toString());
    a.setCoverArtId(o.value(QStringLiteral("coverArt")).toString());
    a.setSongCount(o.value(QStringLiteral("songCount")).toInt());
    a.setDuration(o.value(QStringLiteral("duration")).toInt());
    a.setYear(o.value(QStringLiteral("year")).toInt());
    a.setGenre(o.value(QStringLiteral("genre")).toString());
    a.setCreated(o.value(QStringLiteral("created")).toString());
    a.setPlayCount(o.value(QStringLiteral("playCount")).toInt());
    a.setFavorite(o.value(QStringLiteral("starred")).toBool());
    const QJsonValue discs = o.value(QStringLiteral("discTitles"));
    a.setDiscCount(discs.isArray() ? qMax(1, discs.toArray().size()) : 1);
    return a;
}

Artist SubsonicApi::parseArtist(const QJsonObject &o)
{
    Artist a;
    a.setId(o.value(QStringLiteral("id")).toString());
    a.setName(o.value(QStringLiteral("name")).toString());
    a.setAlbumCount(o.value(QStringLiteral("albumCount")).toInt());
    a.setCoverArtId(o.value(QStringLiteral("coverArt")).toString());
    a.setPlayCount(o.value(QStringLiteral("playCount")).toInt());
    a.setFavorite(o.value(QStringLiteral("starred")).toBool());
    return a;
}

Playlist SubsonicApi::parsePlaylist(const QJsonObject &o)
{
    Playlist p;
    p.setId(o.value(QStringLiteral("id")).toString());
    p.setName(o.value(QStringLiteral("name")).toString());
    p.setComment(o.value(QStringLiteral("comment")).toString());
    p.setOwner(o.value(QStringLiteral("owner")).toString());
    p.setSongCount(o.value(QStringLiteral("songCount")).toInt());
    p.setDuration(o.value(QStringLiteral("duration")).toInt());
    p.setPublic(o.value(QStringLiteral("public")).toBool());
    p.setCreated(o.value(QStringLiteral("created")).toString());
    p.setChanged(o.value(QStringLiteral("changed")).toString());
    p.setCoverArtId(o.value(QStringLiteral("coverArt")).toString());

    QVector<Song> songs;
    for (const QJsonObject &so : objectArray(o.value(QStringLiteral("entry")))) {
        const Song s = parseSong(so);
        if (s.isValid())
            songs.append(s);
    }
    p.setSongs(songs);
    return p;
}

// ------------------------------------------------------------------ 响应分发
void SubsonicApi::onReplyFinished(QNetworkReply *reply)
{
    if (!reply)
        return;
    if (!m_pending.contains(reply)) {
        reply->deleteLater();
        return;
    }

    const Request req = m_pending.take(reply);
    m_coverReplies.remove(reply);
    const QString coverId = req.params.value(QStringLiteral("id")).toString();
    if (req.type == RequestType::GetCoverArt)
        m_pendingCoverIds.remove(coverId);

    const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (reply->error() != QNetworkReply::NoError) {
        const QString msg = reply->errorString();
        qWarning().noquote() << "[SubsonicApi] <-" << req.method << "网络错误 http=" << httpStatus
                             << "err=" << reply->error() << msg;
        reply->deleteLater();

        const bool transient = reply->error() == QNetworkReply::HostNotFoundError
                               || reply->error() == QNetworkReply::TimeoutError
                               || reply->error() == QNetworkReply::TemporaryNetworkFailureError
                               || reply->error() == QNetworkReply::NetworkSessionFailedError
                               || reply->error() == QNetworkReply::RemoteHostClosedError;
        if (transient && req.retryCount < 1 && req.type != RequestType::GetCoverArt) {
            retry(req, req.method);
            return;
        }
        if (req.type == RequestType::GetCoverArt)
            return;      // 封面失败静默处理

        // 仅“连不上”才算断网：SSL 握手失败、请求被取消等不算网络断开
        const auto err = reply->error();
        const bool sslOrCanceled = err == QNetworkReply::SslHandshakeFailedError
                                   || err == QNetworkReply::OperationCanceledError
                                   || err == QNetworkReply::AuthenticationRequiredError
                                   || err == QNetworkReply::ProxyAuthenticationRequiredError;
        const bool offline = !sslOrCanceled
                             && (err == QNetworkReply::HostNotFoundError
                                 || err == QNetworkReply::ConnectionRefusedError
                                 || err == QNetworkReply::RemoteHostClosedError
                                 || err == QNetworkReply::TimeoutError
                                 || err == QNetworkReply::TemporaryNetworkFailureError
                                 || err == QNetworkReply::NetworkSessionFailedError
                                 || err == QNetworkReply::UnknownNetworkError
                                 || err == QNetworkReply::ProxyNotFoundError
                                 || err == QNetworkReply::ProxyConnectionRefusedError
                                 || err == QNetworkReply::ProxyConnectionClosedError
                                 || err == QNetworkReply::ProxyTimeoutError
                                 || (httpStatus == 0 && err != QNetworkReply::ProtocolUnknownError
                                     && err != QNetworkReply::ProtocolInvalidOperationError
                                     && err != QNetworkReply::ContentNotFoundError
                                     && err != QNetworkReply::ContentAccessDenied
                                     && err != QNetworkReply::InternalServerError));
        if (offline && m_online) {
            m_online = false;
            Q_EMIT networkUnreachable(msg);
        }

        Q_EMIT errorOccurred(msg);
        return;
    }

    const QByteArray data = reply->readAll();
    reply->deleteLater();

    if (!m_online) {
        m_online = true;
        Q_EMIT connectionRestored();
    }

    qDebug().noquote() << "[SubsonicApi] <-" << req.method << "http=" << httpStatus
                       << "bytes=" << data.size()
                       << "head:" << QString::fromUtf8(data.left(120));

    if (req.type == RequestType::GetCoverArt) {
        if (!data.isEmpty() && !coverId.isEmpty())
            Q_EMIT coverArtReady(coverId, data);
        return;
    }

    dispatchReply(req, data);
}

void SubsonicApi::handleApiError(const QJsonObject &root, const QString &method)
{
    const QJsonObject err = root.value(QStringLiteral("error")).toObject();
    const QString msg = err.value(QStringLiteral("message")).toString();
    Q_EMIT errorOccurred(QStringLiteral("%1: %2").arg(method, msg.isEmpty() ? QStringLiteral("未知错误") : msg));
}

void SubsonicApi::dispatchReply(const Request &req, const QByteArray &data)
{
    bool parsed = false;
    const QJsonObject root = responseRoot(data, &parsed);
    if (!parsed || root.isEmpty()) {
        qWarning().noquote() << "[SubsonicApi] JSON 解析失败:" << req.method
                             << "响应片段:" << QString::fromUtf8(data.left(300));
        Q_EMIT errorOccurred(QStringLiteral("响应解析失败: %1").arg(req.method));
        return;
    }

    const QString status = root.value(QStringLiteral("status")).toString();
    if (status != QLatin1String("ok")) {
        const QJsonObject err = root.value(QStringLiteral("error")).toObject();
        const int code = err.value(QStringLiteral("code")).toInt();
        const QString message = err.value(QStringLiteral("message")).toString();
        qWarning().noquote() << "[SubsonicApi] API 错误:" << req.method << "code=" << code
                             << message;

        // 认证方式不被支持（token/salt）时回退到明文密码重试一次
        if (!m_usePlainPassword && req.retryCount == 0
            && (code == 40 || message.contains(QStringLiteral("assword"), Qt::CaseInsensitive)
                || message.contains(QStringLiteral("认证"), Qt::CaseInsensitive)
                || message.contains(QStringLiteral("Token"), Qt::CaseInsensitive))) {
            m_usePlainPassword = true;
            Request copy = req;
            copy.retryCount = 1;
            const QUrl url = buildUrl(req.method, req.params);
            QNetworkRequest request(url);
            request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                                 QNetworkRequest::NoLessSafeRedirectPolicy);
            QNetworkReply *reply = m_nam.get(request);
            if (reply) {
                if (m_ignoreSslErrors) {
                    connect(reply, &QNetworkReply::sslErrors, this,
                            [reply](const QList<QSslError> &) { reply->ignoreSslErrors(); });
                }
                m_pending.insert(reply, copy);
            }
            return;
        }

        handleApiError(root, req.method);
        return;
    }

    switch (req.type) {
    case RequestType::Ping:
        Q_EMIT pingResult(true, root.value(QStringLiteral("version")).toString());
        break;

    case RequestType::GetArtists: {
        QVector<Artist> artists;
        const QJsonObject artistsObj = root.value(QStringLiteral("artists")).toObject();
        const QJsonArray indexes = artistsObj.value(QStringLiteral("index")).toArray();
        for (const QJsonValue &iv : indexes) {
            const QJsonObject io = iv.toObject();
            for (const QJsonValue &av : io.value(QStringLiteral("artist")).toArray()) {
                const Artist a = parseArtist(av.toObject());
                if (a.isValid())
                    artists.append(a);
            }
        }
        Q_EMIT artistsReady(artists);
        break;
    }

    case RequestType::GetArtist: {
        const QJsonObject artistObj = root.value(QStringLiteral("artist")).toObject();
        const Artist artist = parseArtist(artistObj);
        QVector<Album> albums;
        const auto appendAlbums = [&albums](const QJsonValue &value) {
            for (const QJsonObject &ao : objectArray(value)) {
                const Album a = parseAlbum(ao);
                if (a.isValid())
                    albums.append(a);
            }
        };
        appendAlbums(root.value(QStringLiteral("album")));
        if (albums.isEmpty())
            appendAlbums(artistObj.value(QStringLiteral("album")));       // Navidrome 嵌在 artist 下
        if (albums.isEmpty())
            appendAlbums(artistObj.value(QStringLiteral("child")));       // 兼容个别服务器
        for (Album &a : albums) {
            if (a.artistId().isEmpty())
                a.setArtistId(artist.id());
            if (a.artist().isEmpty())
                a.setArtist(artist.name());
        }
        Q_EMIT artistReady(artist, albums);
        break;
    }

    case RequestType::GetAlbum: {
        const Album album = parseAlbum(root.value(QStringLiteral("album")).toObject());
        QVector<Song> songs;
        QJsonValue songsValue = root.value(QStringLiteral("song"));
        if (!songsValue.isArray() && !songsValue.isObject())
            songsValue = root.value(QStringLiteral("child"));      // 兼容个别服务器
        for (const QJsonObject &so : objectArray(songsValue)) {
            const Song s = parseSong(so);
            if (s.isValid())
                songs.append(s);
        }
        if (songs.isEmpty() && root.value(QStringLiteral("album")).isObject()) {
            // 少数实现把歌曲放在 album.song 下
            for (const QJsonObject &so : objectArray(
                     root.value(QStringLiteral("album")).toObject().value(QStringLiteral("song")))) {
                const Song s = parseSong(so);
                if (s.isValid())
                    songs.append(s);
            }
        }
        Q_EMIT albumReady(album, songs);
        break;
    }

    case RequestType::GetAlbumList2: {
        QVector<Album> albums;
        const QJsonObject list = root.value(QStringLiteral("albumList2")).toObject();
        for (const QJsonObject &ao : objectArray(list.value(QStringLiteral("album")))) {
            const Album a = parseAlbum(ao);
            if (a.isValid())
                albums.append(a);
        }
        Q_EMIT albumListReady(req.params.value(QStringLiteral("type")).toString(),
                              req.params.value(QStringLiteral("offset")).toInt(), albums);
        break;
    }

    case RequestType::GetSongsByGenre:
    case RequestType::GetRandomSongs: {
        QVector<Song> songs;
        const QJsonValue container = req.type == RequestType::GetRandomSongs
                                         ? root.value(QStringLiteral("randomSongs"))
                                         : root.value(QStringLiteral("songsByGenre"));
        const QJsonObject obj = container.toObject();
        const QJsonValue songsValue = obj.contains(QStringLiteral("song"))
                                          ? obj.value(QStringLiteral("song"))
                                          : container;
        for (const QJsonObject &so : objectArray(songsValue)) {
            const Song s = parseSong(so);
            if (s.isValid())
                songs.append(s);
        }
        Q_EMIT songsReady(songs);
        break;
    }

    case RequestType::GetGenres: {
        QVector<Genre> genres;
        const QJsonObject obj = root.value(QStringLiteral("genres")).toObject();
        for (const QJsonValue &gv : obj.value(QStringLiteral("genre")).toArray()) {
            const QJsonObject go = gv.toObject();
            Genre g;
            // 新旧服务器字段差异：有的用 name，有的把名称放在 value/文本里
            g.name = go.value(QStringLiteral("name")).toString(go.value(QStringLiteral("value")).toString());
            g.songCount = go.value(QStringLiteral("songCount")).toInt();
            g.albumCount = go.value(QStringLiteral("albumCount")).toInt();
            if (!g.name.isEmpty())
                genres.append(g);
        }
        Q_EMIT genresReady(genres);
        break;
    }

    case RequestType::Search3: {
        const QJsonObject result = root.value(QStringLiteral("searchResult3")).toObject();
        QVector<Artist> artists;
        QVector<Album> albums;
        QVector<Song> songs;
        for (const QJsonObject &o : objectArray(result.value(QStringLiteral("artist")))) {
            const Artist a = parseArtist(o);
            if (a.isValid())
                artists.append(a);
        }
        for (const QJsonObject &o : objectArray(result.value(QStringLiteral("album")))) {
            const Album a = parseAlbum(o);
            if (a.isValid())
                albums.append(a);
        }
        for (const QJsonObject &o : objectArray(result.value(QStringLiteral("song")))) {
            const Song s = parseSong(o);
            if (s.isValid())
                songs.append(s);
        }
        Q_EMIT searchReady(artists, albums, songs);
        break;
    }

    case RequestType::GetStarred: {
        const QJsonObject starred = root.value(QStringLiteral("starred2")).toObject();
        const QJsonObject fallback = root.value(QStringLiteral("starred")).toObject();
        const QJsonObject &src = starred.isEmpty() ? fallback : starred;
        QVector<Song> songs;
        QVector<Album> albums;
        QVector<Artist> artists;
        for (const QJsonObject &o : objectArray(src.value(QStringLiteral("song"))))
            songs.append(parseSong(o));
        for (const QJsonObject &o : objectArray(src.value(QStringLiteral("album"))))
            albums.append(parseAlbum(o));
        for (const QJsonObject &o : objectArray(src.value(QStringLiteral("artist"))))
            artists.append(parseArtist(o));
        Q_EMIT starredReady(songs, albums, artists);
        break;
    }

    case RequestType::Star:
    case RequestType::Unstar:
        Q_EMIT starResult(true, req.params.value(QStringLiteral("id")).toStringList());
        break;

    case RequestType::GetPlaylists: {
        QVector<Playlist> playlists;
        const QJsonObject obj = root.value(QStringLiteral("playlists")).toObject();
        for (const QJsonObject &o : objectArray(obj.value(QStringLiteral("playlist")))) {
            const Playlist p = parsePlaylist(o);
            if (p.isValid())
                playlists.append(p);
        }
        Q_EMIT playlistsReady(playlists);
        break;
    }

    case RequestType::GetPlaylist:
        Q_EMIT playlistReady(parsePlaylist(root.value(QStringLiteral("playlist")).toObject()));
        break;

    case RequestType::CreatePlaylist: {
        const QJsonObject obj = root.value(QStringLiteral("playlist")).toObject();
        Q_EMIT playlistChanged(obj.value(QStringLiteral("id")).toString(), true);
        break;
    }

    case RequestType::UpdatePlaylist:
    case RequestType::DeletePlaylist: {
        QString playlistId = req.params.value(QStringLiteral("playlistId")).toString();
        if (playlistId.isEmpty())
            playlistId = req.params.value(QStringLiteral("id")).toString();
        Q_EMIT playlistChanged(playlistId, true);
        break;
    }

    case RequestType::Scrobble:
        break;

    case RequestType::GetLyrics: {
        const QJsonObject obj = root.value(QStringLiteral("lyrics")).toObject();
        Q_EMIT lyricsReady(req.params.value(QStringLiteral("artist")).toString(),
                           req.params.value(QStringLiteral("title")).toString(),
                           obj.value(QStringLiteral("value")).toString());
        break;
    }

    case RequestType::GetCoverArt:
        break;
    }
}

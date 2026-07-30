#include "subsonicapi.h"
#include <QUrl>
#include <QUrlQuery>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QDebug>
#include <QTimer>

namespace {
bool shouldRetryNetworkError(QNetworkReply::NetworkError error)
{
    switch (error) {
    case QNetworkReply::HostNotFoundError:
    case QNetworkReply::TimeoutError:
    case QNetworkReply::TemporaryNetworkFailureError:
    case QNetworkReply::NetworkSessionFailedError:
    case QNetworkReply::UnknownNetworkError:
        return true;
    default:
        return false;
    }
}
}

SubsonicApi::SubsonicApi(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_clientName("naviPlayer")
    , m_apiVersion("1.16.1")
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &SubsonicApi::onReplyFinished);
}

SubsonicApi::~SubsonicApi()
{
}

void SubsonicApi::setServerUrl(const QString &url)
{
    m_serverUrl = url.trimmed();
    if (!m_serverUrl.isEmpty()
            && !m_serverUrl.startsWith("http://")
            && !m_serverUrl.startsWith("https://")) {
        m_serverUrl.prepend("http://");
    }
    if (!m_serverUrl.endsWith("/")) {
        m_serverUrl += "/";
    }
}

void SubsonicApi::setUsername(const QString &username)
{
    m_username = username;
}

void SubsonicApi::setPassword(const QString &password)
{
    m_password = password;
}

bool SubsonicApi::isConfigured() const
{
    return !m_serverUrl.isEmpty() && !m_username.isEmpty() && !m_password.isEmpty();
}

QString SubsonicApi::md5(const QString &input) const
{
    return QString::fromLatin1(QCryptographicHash::hash(input.toUtf8(), QCryptographicHash::Md5).toHex());
}

QString SubsonicApi::buildUrl(const QString &method, const QMap<QString, QString> &params) const
{
    QUrl url(m_serverUrl + "rest/" + method + ".view");
    QUrlQuery query;

    query.addQueryItem("v", m_apiVersion);
    query.addQueryItem("c", m_clientName);
    query.addQueryItem("u", m_username);

    QByteArray saltBytes(6, 0);
    QRandomGenerator::global()->generate(saltBytes.begin(), saltBytes.end());
    QString salt = saltBytes.toHex();
    QString token = md5(m_password + salt);

    query.addQueryItem("s", salt);
    query.addQueryItem("t", token);

    for (auto it = params.begin(); it != params.end(); ++it) {
        query.addQueryItem(it.key(), it.value());
    }

    url.setQuery(query);
    return url.toString();
}

QNetworkReply *SubsonicApi::sendRequest(const QString &method, const QMap<QString, QString> &params)
{
    if (!isConfigured()) {
        emit errorOccurred("Server not configured");
        return nullptr;
    }

    QUrl url(buildUrl(method, params));
    QNetworkRequest request(url);
    QNetworkReply *reply = m_networkManager->get(request);
    if (reply) {
        QVariantMap paramMap;
        for (auto it = params.begin(); it != params.end(); ++it) {
            paramMap.insert(it.key(), it.value());
        }
        reply->setProperty("requestMethod", method);
        reply->setProperty("requestParams", paramMap);
        reply->setProperty("retryCount", 0);
    }
    return reply;
}

void SubsonicApi::ping()
{
    sendRequest("ping");
}

void SubsonicApi::getArtists()
{
    sendRequest("getArtists");
}

void SubsonicApi::getArtist(const QString &artistId)
{
    QMap<QString, QString> params;
    params["id"] = artistId;
    sendRequest("getArtist", params);
}

void SubsonicApi::getAlbums2(int size, int offset)
{
    QMap<QString, QString> params;
    params["type"] = "alphabeticalByName";
    params["size"] = QString::number(size);
    params["offset"] = QString::number(offset);
    sendRequest("getAlbumList2", params);
}

void SubsonicApi::getAlbumListByType(const QString &type, int size, int offset)
{
    QMap<QString, QString> params;
    params["type"] = type;
    params["size"] = QString::number(size);
    params["offset"] = QString::number(offset);
    sendRequest("getAlbumList2", params);
}

void SubsonicApi::getAlbum(const QString &albumId)
{
    QMap<QString, QString> params;
    params["id"] = albumId;
    sendRequest("getAlbum", params);
}

void SubsonicApi::getSongsByGenre(const QString &genre, int count)
{
    QMap<QString, QString> params;
    params["genre"] = genre;
    params["count"] = QString::number(count);
    sendRequest("getSongsByGenre", params);
}

void SubsonicApi::getStarred()
{
    sendRequest("getStarred");
}

void SubsonicApi::search3(const QString &query, int artistCount, int albumCount, int songCount)
{
    QMap<QString, QString> params;
    params["query"] = query;
    params["artistCount"] = QString::number(artistCount);
    params["albumCount"] = QString::number(albumCount);
    params["songCount"] = QString::number(songCount);
    sendRequest("search3", params);
}

void SubsonicApi::getCoverArt(const QString &coverArtId, int size)
{
    QMap<QString, QString> params;
    params["id"] = coverArtId;
    if (size > 0) {
        params["size"] = QString::number(size);
    }
    sendRequest("getCoverArt", params);
}

void SubsonicApi::stream(const QString &songId, int maxBitRate)
{
    QMap<QString, QString> params;
    params["id"] = songId;
    if (maxBitRate > 0) {
        params["maxBitRate"] = QString::number(maxBitRate);
    }
    sendRequest("stream", params);
}

QString SubsonicApi::getStreamUrl(const QString &songId, int maxBitRate)
{
    QMap<QString, QString> params;
    params["id"] = songId;
    if (maxBitRate > 0) {
        params["maxBitRate"] = QString::number(maxBitRate);
    }
    return buildUrl("stream", params);
}

QString SubsonicApi::getCoverArtUrl(const QString &coverArtId, int size)
{
    QMap<QString, QString> params;
    params["id"] = coverArtId;
    if (size > 0) {
        params["size"] = QString::number(size);
    }
    return buildUrl("getCoverArt", params);
}

void SubsonicApi::star(const QStringList &songIds, const QStringList &albumIds, const QStringList &artistIds)
{
    QMap<QString, QString> params;
    for (const QString &id : songIds) {
        params["id"] = id;
    }
    for (const QString &id : albumIds) {
        params["albumId"] = id;
    }
    for (const QString &id : artistIds) {
        params["artistId"] = id;
    }
    sendRequest("star", params);
}

void SubsonicApi::unstar(const QStringList &songIds, const QStringList &albumIds, const QStringList &artistIds)
{
    QMap<QString, QString> params;
    for (const QString &id : songIds) {
        params["id"] = id;
    }
    for (const QString &id : albumIds) {
        params["albumId"] = id;
    }
    for (const QString &id : artistIds) {
        params["artistId"] = id;
    }
    sendRequest("unstar", params);
}

void SubsonicApi::onReplyFinished(QNetworkReply *reply)
{
    if (!reply) return;

    QString url = reply->url().toString();
    QString method = reply->property("requestMethod").toString();
    QVariantMap paramMap = reply->property("requestParams").toMap();
    int retryCount = reply->property("retryCount").toInt();
    qDebug() << "Received response from:" << url;
    QNetworkReply::NetworkError error = reply->error();

    if (error != QNetworkReply::NoError) {
        QString errorStr = reply->errorString();
        qDebug() << "Network error:" << errorStr;
        const QString albumListType = paramMap.value("type").toString();

        if (method != "getCoverArt" && retryCount < 1 && shouldRetryNetworkError(error)) {
            QMap<QString, QString> retryParams;
            for (auto it = paramMap.begin(); it != paramMap.end(); ++it) {
                retryParams.insert(it.key(), it.value().toString());
            }

            QTimer::singleShot(300, this, [this, method, retryParams, retryCount]() {
                QNetworkReply *retryReply = sendRequest(method, retryParams);
                if (retryReply) {
                    retryReply->setProperty("retryCount", retryCount + 1);
                }
            });

            reply->deleteLater();
            return;
        }

        if (method == "ping") {
            emit pingFinished(false, errorStr);
        } else if (method == "getArtists") {
            emit artistsFailed(errorStr);
        } else if (method == "getAlbumList2" && albumListType == "alphabeticalByName") {
            emit albumsFailed(paramMap.value("offset").toInt(),
                              paramMap.value("size").toInt(),
                              errorStr);
        } else if (method == "getAlbumList2" && albumListType != "alphabeticalByName") {
            emit albumListFailed(albumListType, errorStr);
        } else if (method == "getCoverArt") {
            qDebug() << "Cover art request failed:" << errorStr;
        } else {
            emit errorOccurred(errorStr);
        }
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    qDebug() << "Response data size:" << data.size();

    if (method == "getCoverArt") {
        QUrlQuery query(reply->url());
        QString coverArtId = query.queryItemValue("id");
        emit coverArtReceived(coverArtId, data);
        reply->deleteLater();
        return;
    }

    if (method == "ping") {
        qDebug() << "Ping successful";
        emit pingFinished(true, QString());
        reply->deleteLater();
        return;
    }

    // Print XML response for debugging
    qDebug() << "XML Response:" << QString::fromUtf8(data).left(2000);

    QXmlStreamReader xml(data);
    bool foundSubsonicResponse = false;

    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();

        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == "subsonic-response") {
                foundSubsonicResponse = true;
                // Check status attribute
                QXmlStreamAttributes attributes = xml.attributes();
                QString status = attributes.value("status").toString();
                qDebug() << "Subsonic response status:" << status;
                if (status != "ok") {
                    // Continue reading to find error element
                }
                continue;
            } else if (xml.name() == "error") {
                parseError(xml);
            } else if (xml.name() == "artist" && method == "getArtist") {
                Artist artist = parseArtist(xml);
                QVector<Album> albums;
                while (!xml.atEnd()) {
                    xml.readNext();
                    if (xml.isStartElement() && xml.name() == "album") {
                        albums.append(parseAlbum(xml));
                    } else if (xml.isEndElement() && xml.name() == "artist") {
                        break;
                    }
                }
                emit artistAlbumsReceived(artist, albums);
            } else if (xml.name() == "artists") {
                qDebug() << "Found artists element";
                QVector<Artist> artists = parseArtists(xml);
                qDebug() << "Parsed" << artists.size() << "artists";
                emit artistsReceived(artists);
            } else if (xml.name() == "albumList2") {
                qDebug() << "Found albumList2 element";
                QVector<Album> albums = parseAlbums(xml);
                qDebug() << "Parsed" << albums.size() << "albums";
                const QString albumListType = paramMap.value("type").toString();
                if (albumListType == "alphabeticalByName" || albumListType.isEmpty()) {
                    emit albumsReceived(paramMap.value("offset").toInt(),
                                        paramMap.value("size").toInt(),
                                        albums);
                } else {
                    emit albumListReceived(albumListType, albums);
                }
            } else if (xml.name() == "album") {
                qDebug() << "Found album element";
                Album album = parseAlbum(xml);
                QVector<Song> songs = parseSongs(xml);
                qDebug() << "Album has" << songs.size() << "songs";
                emit albumReceived(album, songs);
            } else if (xml.name() == "songsByGenre") {
                QVector<Song> songs = parseSongs(xml);
                emit songsReceived(songs);
            } else if (xml.name() == "starred") {
                QVector<Song> songs;
                QVector<Album> albums;
                QVector<Artist> artists;
                while (!xml.atEnd()) {
                    xml.readNext();
                    if (xml.isStartElement()) {
                        if (xml.name() == "song") {
                            songs.append(parseSong(xml));
                        } else if (xml.name() == "album") {
                            albums.append(parseAlbum(xml));
                        } else if (xml.name() == "artist") {
                            artists.append(parseArtist(xml));
                        }
                    } else if (xml.isEndElement() && xml.name() == "starred") {
                        break;
                    }
                }
                emit starredReceived(songs, albums, artists);
            } else if (xml.name() == "searchResult3") {
                QVector<Artist> artists;
                QVector<Album> albums;
                QVector<Song> songs;
                while (!xml.atEnd()) {
                    xml.readNext();
                    if (xml.isStartElement()) {
                        if (xml.name() == "artist") {
                            artists.append(parseArtist(xml));
                        } else if (xml.name() == "album") {
                            albums.append(parseAlbum(xml));
                        } else if (xml.name() == "song") {
                            songs.append(parseSong(xml));
                        }
                    } else if (xml.isEndElement() && xml.name() == "searchResult3") {
                        break;
                    }
                }
                emit searchResultsReceived(artists, albums, songs);
            } else {
                qDebug() << "Found element:" << xml.name();
            }
        }
    }

    if (xml.hasError()) {
        qDebug() << "XML error:" << xml.errorString();
        const QString albumListType = paramMap.value("type").toString();
        if (method == "getArtists") {
            emit artistsFailed("XML parsing error: " + xml.errorString());
        } else if (method == "getAlbumList2" && albumListType == "alphabeticalByName") {
            emit albumsFailed(paramMap.value("offset").toInt(),
                              paramMap.value("size").toInt(),
                              "XML parsing error: " + xml.errorString());
        } else if (method == "getAlbumList2" && albumListType != "alphabeticalByName") {
            emit albumListFailed(albumListType, "XML parsing error: " + xml.errorString());
        } else {
            emit errorOccurred("XML parsing error: " + xml.errorString());
        }
    }

    if (!foundSubsonicResponse) {
        qDebug() << "Did not find subsonic-response element";
    }

    reply->deleteLater();
}

void SubsonicApi::parseError(QXmlStreamReader &xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    QString message = attributes.value("message").toString();
    emit errorOccurred(message);
}

QVector<Artist> SubsonicApi::parseArtists(QXmlStreamReader &xml)
{
    QVector<Artist> artists;
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == "index") {
                // Parse index - continue reading inside it
                while (!xml.atEnd()) {
                    xml.readNext();
                    if (xml.isStartElement()) {
                        if (xml.name() == "artist") {
                            Artist a = parseArtist(xml);
                            qDebug() << "Parsed artist:" << a.name() << "ID:" << a.id();
                            artists.append(a);
                        }
                    } else if (xml.isEndElement() && xml.name() == "index") {
                        break;
                    }
                }
            } else if (xml.name() == "artist") {
                Artist a = parseArtist(xml);
                qDebug() << "Parsed artist:" << a.name() << "ID:" << a.id();
                artists.append(a);
            }
        } else if (xml.isEndElement() && xml.name() == "artists") {
            break;
        }
    }
    return artists;
}

QVector<Album> SubsonicApi::parseAlbums(QXmlStreamReader &xml)
{
    QVector<Album> albums;
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == "album") {
                albums.append(parseAlbum(xml));
            }
        } else if (xml.isEndElement() && xml.name() == "albumList2") {
            break;
        }
    }
    return albums;
}

QVector<Song> SubsonicApi::parseSongs(QXmlStreamReader &xml)
{
    QVector<Song> songs;
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == "song" || xml.name() == "child") {
                songs.append(parseSong(xml));
            }
        } else if (xml.isEndElement()) {
            if (xml.name() == "songsByGenre" || xml.name() == "album") {
                break;
            }
        }
    }
    return songs;
}

Artist SubsonicApi::parseArtist(QXmlStreamReader &xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    Artist artist;
    artist.setId(attributes.value("id").toString());
    artist.setName(attributes.value("name").toString());
    artist.setCoverArtId(attributes.value("coverArt").toString());
    artist.setAlbumCount(attributes.value("albumCount").toInt());
    artist.setFavorite(attributes.hasAttribute("starred"));
    return artist;
}

Album SubsonicApi::parseAlbum(QXmlStreamReader &xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    Album album;
    album.setId(attributes.value("id").toString());
    auto sanitizeText = [](const QString &in) -> QString {
        QString out;
        out.reserve(in.size());
        for (QChar c : in) {
            if (c.isNull()) {
                continue;
            }
            ushort u = c.unicode();
            if (u == 0x200B || u == 0xFEFF) {
                continue;
            }
            if (u == 0x00A0) {
                out.append(QChar(' '));
                continue;
            }
            QChar::Category cat = c.category();
            if (cat == QChar::Other_Control
                    || cat == QChar::Other_Format
                    || cat == QChar::Other_Surrogate
                    || cat == QChar::Other_PrivateUse
                    || cat == QChar::Other_NotAssigned) {
                continue;
            }
            out.append(c);
        }
        return out.trimmed();
    };

    QString albumName = sanitizeText(attributes.value("name").toString());
    if (albumName.isEmpty()) {
        albumName = sanitizeText(attributes.value("title").toString());
    }
    if (albumName.isEmpty()) {
        albumName = sanitizeText(attributes.value("album").toString());
    }
    album.setName(albumName);
    album.setArtist(attributes.value("artist").toString());
    album.setArtistId(attributes.value("artistId").toString());
    album.setCoverArtId(attributes.value("coverArt").toString());
    album.setSongCount(attributes.value("songCount").toInt());
    album.setDuration(attributes.value("duration").toInt());
    album.setYear(attributes.value("year").toInt());
    album.setGenre(attributes.value("genre").toString());
    album.setFavorite(attributes.hasAttribute("starred"));
    return album;
}

Song SubsonicApi::parseSong(QXmlStreamReader &xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    Song song;
    song.setId(attributes.value("id").toString());
    song.setTitle(attributes.value("title").toString());
    song.setArtist(attributes.value("artist").toString());
    song.setAlbum(attributes.value("album").toString());
    song.setAlbumId(attributes.value("albumId").toString());
    song.setArtistId(attributes.value("artistId").toString());
    song.setTrack(attributes.value("track").toInt());
    song.setYear(attributes.value("year").toInt());
    song.setGenre(attributes.value("genre").toString());
    song.setDuration(attributes.value("duration").toLongLong());
    song.setSize(attributes.value("size").toLongLong());
    song.setSuffix(attributes.value("suffix").toString());
    song.setContentType(attributes.value("contentType").toString());
    song.setCoverArtId(attributes.value("coverArt").toString());
    song.setPath(attributes.value("path").toString());
    song.setFavorite(attributes.hasAttribute("starred"));
    return song;
}

#include "mediahelper.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDomDocument>
#include <QEventLoop>
#include <QDir>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

MediaHelper *MediaHelper::s_self = nullptr;
GenreList MediaHelper::s_genreList;
ArtistList MediaHelper::s_artistList;
ArtistList MediaHelper::s_favorArtists;
AlbumList MediaHelper::s_favorAlbums;
TrackList MediaHelper::s_favorTracks;
int MediaHelper::s_albumCount = 0;
int MediaHelper::s_albumPageSize = 0;

MediaHelper::MediaHelper(QObject *parent)
    : QObject{parent}
{
    m_manager = new QNetworkAccessManager(this);
    m_host = "";
    m_authString = "";
}

AlbumList MediaHelper::getAlbumList(AlbumType type, int size, int offset)
{
    // https://www.redtux.cn:619/rest/getAlbumList?type=recent&size=20&u=redtux&t=9c27026da53968e4fbe19241b6cc49b0&s=coming&v=1.61.1&c=naviPlayer
    bool isGetData=false;

    QString questString = questStringBuilder("getAlbumList");

    questString += QString("&type=%1&size=%2").arg(s_self->AlbumTypeToString(type)).arg( size);

    if (offset > 0)
        questString += QString("&offset=%1").arg(offset);

    Config::G_Debug(questString);

    QNetworkRequest quest;
    quest.setUrl(QUrl(questString));

    QNetworkReply *reply = m_manager->get(quest);

    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    QString replyString = reply->readAll();
    reply->deleteLater();
    reply = nullptr;

    QDomDocument doc;
    if(!doc.setContent(replyString) ){
        Config::G_Debug("Out put is not XML data");
    }
    else {
        isGetData = true;
    }

    QDomElement root = doc.documentElement();

    QDomNodeList list = root.elementsByTagName("album");

    AlbumList al;

    for (int i = 0; i < list.count(); i++) {
        QDomElement element = list.at(i).toElement();

        Album a;

        a.setId(element.attribute("id"));
        a.setName(element.attribute("name"));
        a.setArtist(element.attribute("artist"));
        a.setArtistId(element.attribute("artistId"));
        a.setCoverId(element.attribute("coverArt"));
        a.setSongCount(element.attribute("songCount").toInt());
        a.setDuration(durationToString(element.attribute("duration").toInt()));

        a.setArtDirpath(s_self->getCoverArt(a.id(), AlbumCover));

        a.setIcon(a.iconPath());

        al << a;

    }
    if(!isGetData)
    {
        Config::G_Debug( QString::fromLocal8Bit("获取数据失败"));
    }

    return al;

}

QString MediaHelper::getCoverArt(const QString &id, CoverType type, int size)
{
    QString coverDirName = "";

    switch (type) {
    case AlbumCover:
        coverDirName += "albums";
        break;
    case ArtistCover:
        coverDirName += "artists";
        break;
    case TrackCover:
        coverDirName += "tracks";
        break;
    case PlayListCover:
        coverDirName += "playlist";
        break;
    default:
        break;
    }

    QString coverDirPath = Config::cacheDir() + coverDirName + QDir::separator() + id + QDir::separator();

    QDir dir(coverDirPath);
    if(!dir.exists())
        dir.mkpath(coverDirPath);

    QString filePath = dir.canonicalPath() + QDir::separator()+ "cover";

    QFile file(filePath);
    if (file.exists())
        return coverDirPath;

    QString questString = questStringBuilder("getCoverArt");

    questString += QString("&id=%1").arg(id);

    if (size > 0)
        questString += QString("&size=%1").arg(size);

    QNetworkRequest quest;
    quest.setUrl(QUrl(questString));

    QNetworkReply *reply = s_self->m_manager->get(quest);

    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    if(!file.open(QIODevice::WriteOnly)){
        Config::G_Debug("File cannot write!");
        return QString();
    }

    file.write( reply->readAll() );
    file.close();

    reply->deleteLater();
    reply = nullptr;

    s_self->getRoundRectPixmap(filePath);

    return coverDirPath;
}

void MediaHelper::getArtists()
{
    QString questString = questStringBuilder("getIndexes");

    QNetworkRequest quest;
    quest.setUrl(QUrl(questString));

    //Config::G_Debug("questString:", questString);

    QNetworkReply *reply = m_manager->get(quest);

    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    QString replyString = reply->readAll();
    reply->deleteLater();
    reply = nullptr;

    QDomDocument doc;
    if(!doc.setContent(replyString) ){
        Config::G_Debug("Out put is not XML data");
    }

    QDomElement root = doc.documentElement();

    QDomNodeList list = root.elementsByTagName("artist");

    for (int i = 0; i < list.count(); i++) {

        QDomElement element = list.at(i).toElement();

        Artist art(element.attribute("id"),
                   element.attribute("name"),
                   element.attribute("coverArt"),
                   element.attribute("albumCount").toInt());


        art.setArtDirpath(s_self->getCoverArt(art.id(), ArtistCover));

        //Config::G_Debug(art.coverPath());
        art.setIcon(art.iconPath());

        MediaHelper::s_albumCount += element.attribute("albumCount").toInt();

        this->s_artistList << art;

    }
    s_albumPageSize = qCeil(((float)MediaHelper::s_albumCount) / Config::s_options.albumCountPerPage);

}

void MediaHelper::getGenres()
{

    QString questString = questStringBuilder("getGenres");

    QNetworkRequest quest;
    quest.setUrl(QUrl(questString));

    Config::G_Debug("questString:", questString);

    QNetworkReply *reply = m_manager->get(quest);

    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    QString replyString = reply->readAll();
    reply->deleteLater();
    reply = nullptr;

    QDomDocument doc;
    if(!doc.setContent(replyString) ){
        Config::G_Debug("Out put is not XML data");
    }

    QDomElement root = doc.documentElement();

    QDomNodeList list = root.elementsByTagName("genre");

    for (int i = 0; i < list.count(); i++) {
        QDomElement element = list.at(i).toElement();

        //MediaHelper::s_albumCount += element.attribute("albumCount").toInt();
        Genre gr(element.text(), element.attribute("songCount").toInt(), element.attribute("albumCount").toInt());
        this->s_genreList << gr;
    }
    //s_albumPageSize = MediaHelper::s_albumCount / Config::s_options.albumCountPerPage;
}

void MediaHelper::getFavorites()
{
    QString questString = questStringBuilder("getStarred");

    QNetworkRequest quest;
    quest.setUrl(QUrl(questString));

    QNetworkReply *reply = m_manager->get(quest);

    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    QString replyString = reply->readAll();
    reply->deleteLater();
    reply = nullptr;

    QDomDocument doc;
    if(!doc.setContent(replyString) ){
        Config::G_Debug("Out put is not XML data");
    }

    QDomElement root = doc.documentElement();

    QDomNodeList list = root.elementsByTagName("artist");

    for (int i = 0; i < list.count(); i++) {
        QDomElement element = list.at(i).toElement();

        Artist art(element.attribute("id"),
                   element.attribute("name"),
                   element.attribute("coverArt"),
                   element.attribute("albumCount").toInt());


        art.setArtDirpath(s_self->getCoverArt(art.id(), ArtistCover));

        art.setIcon(art.iconPath());

        this->s_favorArtists << art;
    }

    list = root.elementsByTagName("album");

    for (int i = 0; i < list.count(); i++) {
        QDomElement element = list.at(i).toElement();

        Album a;

        a.setId(element.attribute("id"));
        a.setName(element.attribute("name"));
        a.setArtist(element.attribute("artist"));
        a.setArtistId(element.attribute("artistId"));
        a.setCoverId(element.attribute("coverArt"));
        a.setSongCount(element.attribute("songCount").toInt());
        a.setDuration(durationToString(element.attribute("duration").toInt()));

        a.setArtDirpath(s_self->getCoverArt(a.id(), AlbumCover));
        a.setIcon(a.iconPath());

        this->s_favorAlbums << a;
    }

    list = root.elementsByTagName("song");

    for (int i = 0; i < list.count(); i++) {
        QDomElement element = list.at(i).toElement();

        Track trk;

        trk.setId(element.attribute("id"));
        trk.setName(element.attribute("title"));
        trk.setAlbum(element.attribute("album"));
        trk.setArtist(element.attribute("artist"));
        trk.setTrack(element.attribute("track"));
        trk.setCoverId(element.attribute("coverArt"));
        trk.setDuration(durationToString(element.attribute("duration").toInt()));
        trk.setBitRate(element.attribute("bitRate").toInt());
        trk.setAlbumId(element.attribute("albumId"));
        trk.setArtistId(element.attribute("artistId"));

        trk.setArtDirpath(s_self->getCoverArt(trk.albumId(), AlbumCover));
        trk.setIcon(trk.iconPath());

        this->s_favorTracks << trk;

    }

}

QList<PlayList> MediaHelper::getPlaylists()
{
    QString questString = questStringBuilder("getPlaylists");

    QNetworkRequest quest;
    quest.setUrl(QUrl(questString));

    //Config::G_Debug("questString:", questString);

    QNetworkReply *reply = m_manager->get(quest);

    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    QString replyString = reply->readAll();
    reply->deleteLater();
    reply = nullptr;

    QDomDocument doc;
    if(!doc.setContent(replyString) ){
        Config::G_Debug("Out put is not XML data");
    }

    QDomElement root = doc.documentElement();

    QDomNodeList list = root.elementsByTagName("playlist");

    QList<PlayList> playLists;

    for (int i = 0; i < list.count(); i++) {

        QDomElement element = list.at(i).toElement();
        /*
        <playlist
         id="b01d162c-f53f-408f-b363-9e3944318997"
         name="Brian.Crain"
         songCount="174"
         duration="40183"
         public="false"
         owner="redtux"
         created="2023-07-29T10:34:40.436498638Z"
         changed="2023-07-29T10:39:22Z"
         coverArt="pl-b01d162c-f53f-408f-b363-9e3944318997_64c4ec5a"/>
        */

        PlayList pl;
        pl.setId(element.attribute("id"));
        pl.setName(element.attribute("name"));
        pl.setSongCount(element.attribute("songCount").toInt());
        pl.setDuration(element.attribute("duration").toInt());
        pl.setCoverId(element.attribute("coverArt"));

        pl.setArtDirpath(s_self->getCoverArt(pl.id(), PlayListCover));

        //Config::G_Debug(art.coverPath());
        pl.setIcon(pl.iconPath());

        playLists << pl;

    }
    return playLists;
}

TrackList MediaHelper::getPlaylist(const QString &listId)
{
    QString questString = questStringBuilder("getPlaylist");

    questString += QString("&id=%1").arg(listId);

    QNetworkRequest quest;
    quest.setUrl(QUrl(questString));

    QNetworkReply *reply = m_manager->get(quest);

    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    QString replyString = reply->readAll();
    reply->deleteLater();
    reply = nullptr;

    QDomDocument doc;
    if(!doc.setContent(replyString) ){
        Config::G_Debug("Out put is not XML data");
    }

    QDomElement root = doc.documentElement();

    QDomNodeList list = root.elementsByTagName("entry");

    TrackList trackLists;

    for (int i = 0; i < list.count(); i++) {

        QDomElement element = list.at(i).toElement();
        /*
<entry
//id="ed823aef832e2b6fe1d6d63c3f2b9d9d"
//parent="05b234e7e0467ddfef55a8133a0d8a8a"
isDir="false"
//title="Wild River"
//album="Northern Sky"
//artist="Brian Crain"
//track="5"
year="2000"
//genre="现代器乐"
//coverArt="mf-ed823aef832e2b6fe1d6d63c3f2b9d9d_64c10e11"

//size="7105454"
contentType="audio/mp4"
//suffix="m4a"
//duration="217"
//bitRate="257"
path="Brian Crain/Northern Sky/05 - Wild River.m4a"
playCount="1"
discNumber="1"
created="2023-07-29T04:46:33.487630991Z"
//albumId="05b234e7e0467ddfef55a8133a0d8a8a"
//artistId="bee8a9731824964a9c4cf05199ce8051"
type="music"
isVideo="false"
played="2023-07-31T08:01:05Z"
bpm="0" comment=""
sortName=""
mediaType="song"
musicBrainzId=""
channelCount="2"
samplingRate="0">
<replayGain trackPeak="1" albumPeak="1"/>
</entry>
        */

        Track tk;
        tk.setId(element.attribute("id"));
        tk.setName(element.attribute("title"));
        tk.setAlbum(element.attribute("album"));
        tk.setArtist(element.attribute("artist"));
        tk.setTrack(element.attribute("track"));
        tk.setCoverId(element.attribute("coverArt"));
        tk.setDuration(durationToString(element.attribute("duration").toInt()));
        tk.setBitRate(element.attribute("bitRate").toInt());
        tk.setAlbumId(element.attribute("albumId"));
        tk.setArtistId(element.attribute("artistId"));

        tk.setArtDirpath(s_self->getCoverArt(tk.albumId(), AlbumCover));

        tk.setIcon(tk.iconPath());

        trackLists << tk;

    }
    return trackLists;
}

AlbumObject MediaHelper::getAlbum(const QString &albumId)
{
    QString questString = questStringBuilder("getAlbum");

    questString += QString("&id=%1").arg(albumId);

    QNetworkRequest quest;
    quest.setUrl(QUrl(questString));

    QNetworkReply *reply = m_manager->get(quest);

    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    QString replyString = reply->readAll();
    reply->deleteLater();
    reply = nullptr;

    QDomDocument doc;
    if(!doc.setContent(replyString) ){
        Config::G_Debug("Out put is not XML data");
    }

    QDomElement root = doc.documentElement();

    QDomElement albumEle = root.firstChildElement("album");

    Config::G_Debug("MediaHelper::getAlbum:Tagname:", albumEle.tagName());

    AlbumObject ao;

    /* <album
     * id="9f19d84a48ccc124bba6529f64a7de2b"
     * name="歌手 2017"
     * artist="Various Artists"
     * artistId="03b645ef2100dfc42fa9785ea3102295"
     * coverArt="al-9f19d84a48ccc124bba6529f64a7de2b_663d3367"
     * songCount="107"
     * duration="32810"
     * playCount="34"
     * created="2024-05-09T20:37:47.779700011Z"
     * year="2017"
     * genre="华语流行"
     * played="2024-11-22T16:47:43.389+08:00"
     * userRating="0"
     * musicBrainzId=""
     * isCompilation="false"
     * sortName="">
     */

    ao.setId(albumEle.attribute("id"));
    ao.setName(albumEle.attribute("name"));
    ao.setCoverId(albumEle.attribute("coverArt"));

    ao.setArtDirpath(s_self->getCoverArt(ao.id(), AlbumCover));
    ao.setIcon(ao.iconPath());

    ao.setArtist(albumEle.attribute("artist"));
    ao.setArtistId(albumEle.attribute("artistId"));
    ao.setDuration(albumEle.attribute("duration"));
    ao.setGenre(albumEle.attribute("genre"));
    ao.setYear(albumEle.attribute("year").toInt());

    QDomNodeList list = albumEle.elementsByTagName("song");

    for (int i = 0; i < list.count(); i++) {

        QDomElement element = list.at(i).toElement();

        /*
<entry
//id="ed823aef832e2b6fe1d6d63c3f2b9d9d"
//parent="05b234e7e0467ddfef55a8133a0d8a8a"
isDir="false"
//title="Wild River"
//album="Northern Sky"
//artist="Brian Crain"
//track="5"
year="2000"
//genre="现代器乐"
//coverArt="mf-ed823aef832e2b6fe1d6d63c3f2b9d9d_64c10e11"
//size="7105454"
contentType="audio/mp4"
//suffix="m4a"
//duration="217"
//bitRate="257"
path="Brian Crain/Northern Sky/05 - Wild River.m4a"
playCount="1"
discNumber="1"
created="2023-07-29T04:46:33.487630991Z"
//albumId="05b234e7e0467ddfef55a8133a0d8a8a"
//artistId="bee8a9731824964a9c4cf05199ce8051"
type="music"
isVideo="false"
played="2023-07-31T08:01:05Z"
bpm="0" comment=""
sortName=""
mediaType="song"
musicBrainzId=""
channelCount="2"
samplingRate="0">
<replayGain trackPeak="1" albumPeak="1"/>
</entry>
        */

        Track tk;
        tk.setId(element.attribute("id"));
        tk.setName(element.attribute("title"));
        tk.setAlbum(element.attribute("album"));
        tk.setArtist(element.attribute("artist"));
        tk.setTrack(element.attribute("track"));
        tk.setCoverId(element.attribute("coverArt"));
        tk.setDuration(durationToString(element.attribute("duration").toInt()));
        tk.setBitRate(element.attribute("bitRate").toInt());
        tk.setAlbumId(element.attribute("albumId"));
        tk.setArtistId(element.attribute("artistId"));

        tk.setArtDirpath(s_self->getCoverArt(tk.albumId(), AlbumCover));

        tk.setIcon(tk.iconPath());

        ao.addTrack(tk);

    }
    return ao;
}

QString MediaHelper::getSongUrlbyId(const QString &id)
{
    QString questString = questStringBuilder("stream");

    questString += QString("&id=%1").arg(id);

    return questString;
}

bool MediaHelper::ping(Config::Server &srv)
{
    QString questString = srv.url + "/rest/ping";
    questString += QString("?u=%1&p=%2&v=1.12.0&c=naviPlayer").arg(srv.user, srv.password);

    QNetworkRequest quest;
    quest.setUrl(QUrl(questString));

    QNetworkReply *reply = m_manager->get(quest);

    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    QString replyString = reply->readAll();
    reply->deleteLater();
    reply = nullptr;

    //Config::G_Debug("replyString", replyString );

    QDomDocument doc;
    if(!doc.setContent(replyString) ){
        Config::G_Debug("Out put is not XML data");
    }

    QDomElement root = doc.documentElement();

    bool ret = root.attribute("status") == "ok";

    if (ret){
        srv.version = root.attribute("version");

        if (srv.version > "1.12.0"){
            srv.useToken = true;
        }
    }

    return ret;

}

QString MediaHelper::AlbumTypeToString(AlbumType type) const
{
    QString ret;
    //Random, Newest, Highest, Frequent, Recent, Starred
    switch (type) {
    case Random:
        ret = "random";
        break;
    case Newest:
        ret = "newest";
        break;
    case Highest:
        ret = "highest";
        break;
    case Frequent:
        ret = "frequent";
        break;
    case Recent:
        ret = "recent";
        break;
    case Starred:
        ret = "starred";
        break;
    default:
        ret = "recent";
        break;
    }

    return ret;
}

QString MediaHelper::questStringBuilder(const QString &command)
{

    if(m_host.isEmpty()){
        m_host = Config::Instance()->currentServer.url;
        m_host.remove(QRegExp("/$"));
        m_host +=  "/rest/";
    }

    if(m_authString.isEmpty()){
        m_authString = "?u=" + Config::Instance()->currentServer.user;
        if(Config::Instance()->currentServer.useToken){
            m_authString += QString("&t=%1&s=%2").arg(Config::Instance()->currentServer.token,
                                                     Config::Instance()->currentServer.salt);
        }
        else{
            m_authString += QString("&p=%1").arg(Config::Instance()->currentServer.password);
        }
        m_authString += QString("&v=%1&c=naviPlayer").arg(Config::Instance()->currentServer.version);
    }

    QString questString = m_host + command + m_authString;

    return questString;
}

bool MediaHelper::getRoundRectPixmap(const QString &filePath)
{
    if (!QFile::exists(filePath)) {
        Config::G_Debug("File not exist: ", filePath);
        return false;
    }

    QPixmap srcPixMap(filePath);

    QString iconPath = filePath.left(filePath.lastIndexOf("/")+1) + "icon";

    QString posterPath = filePath.left(filePath.lastIndexOf("/")+1) + "poster";

    if (QFile::exists(iconPath) && QFile::exists(posterPath)) {
        return true;
    }

    QPixmap posterPixMap = srcPixMap.scaled(POSTER_WIDTH, POSTER_WIDTH,
                                            Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QPixmap posterImage(POSTER_WIDTH, POSTER_WIDTH);
    posterImage.fill(Qt::transparent);

    QPainter painter(&posterImage);

    // 抗锯齿
    painter.setRenderHints(QPainter::Antialiasing, true);
    // 图片平滑处理
    painter.setRenderHints(QPainter::SmoothPixmapTransform, true);
    // 将图片裁剪为圆角
    QPainterPath path;
    path.addRoundedRect(0,0, POSTER_WIDTH, POSTER_WIDTH,
                        POSTER_RADIUS, POSTER_RADIUS);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, POSTER_WIDTH, POSTER_WIDTH,
                       posterPixMap);
    if(posterImage.save(posterPath, "PNG", 60)){
        QPixmap iconPixMap = posterImage.scaled(ICON_WIDTH, ICON_WIDTH,
                                                Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        if(iconPixMap.save(iconPath, "PNG", 60)){
            return true;
        }
    }

    return false;

}

QString MediaHelper::durationToString(int dur)
{
    int hh = dur / 3600;
    int mm = (dur % 3600) / 60;
    int ss = dur % 60;

    QString durString;
    if(hh >0){
        durString = QString::number(hh) + ":";
    }
    durString += QString("%1:%2").arg(mm, 2, 10, QLatin1Char('0') )
                     .arg(ss, 2, 10, QLatin1Char('0'));

    return durString;
}

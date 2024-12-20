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

const QIcon &MediaHelper::getAlbumIcon(const QString &albumId)
{
    if(!MediaHelper::Instance()->m_albumIconMap.contains(albumId)){
        QString iconPath = MediaHelper::Instance()->getCoverArt(albumId, MediaHelper::AlbumCover, MediaHelper::Icon );
        QIcon icon;
        if (!iconPath.isEmpty()){
            icon.addFile(iconPath);
        }
        MediaHelper::Instance()->m_albumIconMap.insert(albumId, icon);
    }
    return MediaHelper::Instance()->m_albumIconMap[albumId];
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
        a.setSongCount(element.attribute("songCount").toInt());
        a.setDuration(durationToString(element.attribute("duration").toInt()));

        s_self->getCoverArt(a.id());

        al << a;

    }
    if(!isGetData)
    {
        Config::G_Debug( QString::fromLocal8Bit("获取数据失败"));
    }

    return al;

}

QString MediaHelper::getCoverArt(const QString &id, MediaHelper::CoverType type, MediaHelper::CoverSize size)
{
    QString coverDirName = "";

    QString coverFileName = "";

    switch (type) {
    case AlbumCover:
        coverDirName = "albums";
        break;
    case ArtistCover:
        coverDirName = "artists";
        break;
    case TrackCover:
        coverDirName = "tracks";
        break;
    case PlayListCover:
        coverDirName = "playlist";
        break;
    default:
        break;
    }

    switch (size) {
    case MediaHelper::Cover:
        coverFileName = "cover";
        break;
    case MediaHelper::Poster:
        coverFileName = "poster";
        break;
    case MediaHelper::Icon:
        coverFileName = "icon";
        break;
    default:
        break;
    }

    QString coverDirPath = Config::cacheDir() + coverDirName + QDir::separator() + id + QDir::separator();

    QString filePath = coverDirPath + coverFileName;

    if(QFile::exists(filePath)){
        return filePath;
    }

    QDir dir(coverDirPath);
    if(!dir.exists())
        dir.mkpath(coverDirPath);

    QString coverFilePath = dir.canonicalPath() + QDir::separator()+ "cover";

    QString questString = questStringBuilder("getCoverArt");

    questString += QString("&id=%1").arg(id);

    QNetworkRequest quest;
    quest.setUrl(QUrl(questString));

    QNetworkReply *reply = s_self->m_manager->get(quest);

    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    QFile file(coverFilePath);

    if(!file.open(QIODevice::WriteOnly)){
        Config::G_Debug("File cannot write!");
        return QString();
    }

    file.write( reply->readAll() );
    file.close();

    reply->deleteLater();
    reply = nullptr;

    if(MediaHelper::Instance()->getRoundRectPixmap(coverFilePath)){
        return filePath;
    }

    return QString();
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
                   element.attribute("albumCount").toInt());

        art.setIcon(MediaHelper::Instance()->getCoverArt(art.id(), MediaHelper::ArtistCover, MediaHelper::Icon));

        MediaHelper::s_albumCount += element.attribute("albumCount").toInt();

        this->s_artistList << art;

    }
    s_albumPageSize = qCeil(((float)MediaHelper::s_albumCount) / Config::s_options.albumCountPerPage);

}

ArtistObject MediaHelper::getArtist(const QString &id)
{

    QString questString = questStringBuilder("getArtist");

    questString += QString("&id=%1").arg(id);

    Config::G_Debug("MediaHelper::getArtist:", questString);

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

    QDomElement artistEle = root.firstChildElement("artist");

    ArtistObject artObject;

    /*
     * <artist
     * id="bee8a9731824964a9c4cf05199ce8051"
     * name="Brian Crain"
     * coverArt="ar-bee8a9731824964a9c4cf05199ce8051_0"
     * albumCount="14"
     * starred="2023-12-30T14:38:07Z"
     * artistImageUrl="https://www.redtux.cn:619/share/img/eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpZCI6ImFyLWJlZThhOTczMTgyNDk2NGE5YzRjZjA1MTk5Y2U4MDUxXzAiLCJpc3MiOiJORCJ9.WlJ3pHUvHF17BkvnehKzX4WpaLRRJVdlIyfcXOz7SpM?size=600">
     */

    artObject.setId(artistEle.attribute("id"));
    artObject.setName(artistEle.attribute("name"));
    artObject.setAlbumCount(artistEle.attribute("albumCount").toInt() );
    artObject.setIcon( MediaHelper::Instance()->getCoverArt(artObject.id(), MediaHelper::ArtistCover, MediaHelper::Icon));

    QDomNodeList list = artistEle.elementsByTagName("album");

    for (int i = 0; i < list.count(); i++) {

        QDomElement element = list.at(i).toElement();
        /*
         * <album
         * id="8fe5deb8d124a87fbf0504d4389977cc"
         * parent="bee8a9731824964a9c4cf05199ce8051"
         * isDir="true"
         * title="Sienna"
         * name="Sienna"
         * album="Sienna"
         * artist="Brian Crain"
         * year="2003"
         * genre="现代器乐"
         * coverArt="al-8fe5deb8d124a87fbf0504d4389977cc_64c10fdc"
         * duration="2753"
         * playCount="30"
         * created="2023-07-29T04:46:34.529530772Z"
         * artistId="bee8a9731824964a9c4cf05199ce8051"
         * songCount="12"
         * isVideo="false"
         * played="2024-11-07T01:50:55.947821163+08:00"
         * bpm="0"
         * comment=""
         * sortName=""
         * mediaType="album"
         * musicBrainzId=""
         * channelCount="0"
         * samplingRate="0">
         */

        Album abm;

        abm.setId(element.attribute("id"));
        abm.setName(element.attribute("title"));
        abm.setArtist(element.attribute("artist"));
        abm.setArtistId(element.attribute("artistId"));
        abm.setSongCount(element.attribute("songCount").toInt());
        abm.setDuration(durationToString(element.attribute("duration").toInt()));

        s_self->getCoverArt(abm.id());

        artObject.addAlbum(abm);

    }
    return artObject;
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
                   element.attribute("albumCount").toInt());

        art.setIcon(MediaHelper::Instance()->getCoverArt(art.id(), MediaHelper::ArtistCover, MediaHelper::Icon));

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
        a.setSongCount(element.attribute("songCount").toInt());
        a.setDuration(durationToString(element.attribute("duration").toInt()));

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
        trk.setDuration(durationToString(element.attribute("duration").toInt()));
        trk.setBitRate(element.attribute("bitRate").toInt());
        trk.setAlbumId(element.attribute("albumId"));
        trk.setArtistId(element.attribute("artistId"));

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

        pl.setIcon( MediaHelper::Instance()->getCoverArt(pl.id(), MediaHelper::PlayListCover, MediaHelper::Icon));

        playLists << pl;

    }
    return playLists;
}

PlayListObject MediaHelper::getPlaylist(const QString &listId)
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

    QDomElement playlistEle = root.firstChildElement("playlist");

    PlayListObject plObject;

    /*
     * <playlist id="b01d162c-f53f-408f-b363-9e3944318997"
     * name="Brian.Crain"
     * songCount="174"
     * duration="40183"
     * public="false"
     * owner="redtux"
     * created="2023-07-29T10:34:40.436498638Z"
     * changed="2023-07-29T10:39:22Z"
     * coverArt="pl-b01d162c-f53f-408f-b363-9e3944318997_64c4ec5a">
     */

    plObject.setId(playlistEle.attribute("id"));
    plObject.setName(playlistEle.attribute("name"));
    plObject.setSongCount(playlistEle.attribute("songCount").toInt());
    plObject.setDuration(playlistEle.attribute("duration").toInt());
    plObject.setOwner(playlistEle.attribute("owner"));
    plObject.setModfiyTime(playlistEle.attribute("changed"));
    plObject.setIcon( MediaHelper::Instance()->getCoverArt(plObject.id(), MediaHelper::PlayListCover, MediaHelper::Icon));

    QDomNodeList list = playlistEle.elementsByTagName("entry");

    for (int i = 0; i < list.count(); i++) {

        QDomElement element = list.at(i).toElement();
        /*
         *
         * <entry
         * //id="ed823aef832e2b6fe1d6d63c3f2b9d9d"
         * //parent="05b234e7e0467ddfef55a8133a0d8a8a"
         * isDir="false"
         * //title="Wild River"
         * //album="Northern Sky"
         * //artist="Brian Crain"
         * //track="5"
         * year="2000"
         * //genre="现代器乐"
         * //coverArt="mf-ed823aef832e2b6fe1d6d63c3f2b9d9d_64c10e11"
         * //size="7105454"
         * contentType="audio/mp4"
         * //suffix="m4a"
         * //duration="217"
         * //bitRate="257"
         * path="Brian Crain/Northern Sky/05 - Wild River.m4a"
         * playCount="1"
         * discNumber="1"
         * created="2023-07-29T04:46:33.487630991Z"
         * //albumId="05b234e7e0467ddfef55a8133a0d8a8a"
         * //artistId="bee8a9731824964a9c4cf05199ce8051"
         * type="music"
         * isVideo="false"
         * played="2023-07-31T08:01:05Z"
         * bpm="0" comment=""
         * sortName=""
         * mediaType="song"
         * musicBrainzId=""
         * channelCount="2"
         * samplingRate="0">
         * <replayGain trackPeak="1" albumPeak="1"/>
         * </entry>
         */

        Track tk;
        tk.setId(element.attribute("id"));
        tk.setName(element.attribute("title"));
        tk.setAlbum(element.attribute("album"));
        tk.setArtist(element.attribute("artist"));
        tk.setTrack(element.attribute("track"));
        tk.setDuration(durationToString(element.attribute("duration").toInt()));
        tk.setBitRate(element.attribute("bitRate").toInt());
        tk.setAlbumId(element.attribute("albumId"));
        tk.setArtistId(element.attribute("artistId"));

        plObject.addTrack(tk);

    }
    return plObject;
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
        tk.setDuration(durationToString(element.attribute("duration").toInt()));
        tk.setBitRate(element.attribute("bitRate").toInt());
        tk.setAlbumId(element.attribute("albumId"));
        tk.setArtistId(element.attribute("artistId"));

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

void MediaHelper::scrobble(const QString &id, bool submission)
{
    QString questString = questStringBuilder("scrobble");
    questString += QString("&id=%1").arg(id);
    if (!submission){
        questString += QString("&submission=%1").arg("false");
    }
    else{
        questString += QString("&time=%1&submission=%2").arg(QDateTime::currentMSecsSinceEpoch()).arg("true");
    }

    Config::G_Debug("MediaHelper::scrobble:questString", questString);

    QNetworkRequest quest;
    quest.setUrl(QUrl(questString));

    QNetworkReply *reply = m_manager->get(quest);

    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    //QString replyString = reply->readAll();
    reply->deleteLater();
    reply = nullptr;
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

    QString iconPath = filePath.left(filePath.lastIndexOf("/")+1) + "icon";

    QString posterPath = filePath.left(filePath.lastIndexOf("/")+1) + "poster";

    if (QFile::exists(iconPath) && QFile::exists(posterPath)) {
        return true;
    }

    QImage srcImage(filePath);

    QImage posterSrcImage = srcImage.scaled(POSTER_WIDTH, POSTER_WIDTH,
                                            Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QImage posterImage(POSTER_WIDTH, POSTER_WIDTH, QImage::Format_ARGB32);

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
    painter.drawImage(0, 0, posterSrcImage);
    painter.end();

    if(posterImage.save(posterPath, "PNG", 60)){
        QImage iconImage = posterImage.scaled(ICON_WIDTH, ICON_WIDTH,
                                                Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        if(iconImage.save(iconPath, "PNG", 60)){
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

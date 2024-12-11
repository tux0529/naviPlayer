#include "config.h"
#include "mediahelper.h"

#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>

#include <QCryptographicHash>
#include <QByteArray>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

Config *Config::self = nullptr;
Config::Options Config::s_options;


Config::Config(QObject *parent) : QObject(parent),
    m_serverSeted(false)
{
    m_dbFileName = "naviPlayer.config";

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    m_appDataDir=  QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QDir::separator() ;
#else
    m_appDataDir = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + QDir::separator() ;
#endif

    QDir dir(m_appDataDir);
    if(!dir.exists()){
        dir.mkpath(m_appDataDir);
    }

    m_dbFilePath = m_appDataDir + m_dbFileName;

    reset();

    if (!QFile::exists(m_dbFilePath)){
        initConfigValue();
    }

    if (!connectConfigDB() ){
        return ;
    }

    loadConfigValue();

}

void Config::loadConfigValue()
{

    QString currentSeverName = this->value("currentSever");

    QSqlQuery query(m_configdb);

    query.exec(QString("SELECT * FROM servers;"));

    while(query.next()){
        Config::Server srv;
        srv.name = query.value(1).toString();
        srv.url = query.value(2).toString();
        srv.user = query.value(3).toString();
        srv.password = query.value(4).toString();
        srv.useToken = ( 1 == query.value(5).toInt());
        srv.version = query.value(6).toString();

        if(srv.useToken){
            srv.salt = "coming";
            QByteArray ba;
            ba.append(srv.password.toUtf8());
            ba.append(srv.salt.toUtf8());

            srv.token = QCryptographicHash::hash(ba, QCryptographicHash::Md5).toHex();

            Config::G_Debug(srv.name, ":Token:",srv.token );

        }

        servers << srv;

        if (currentSeverName == srv.name){
            currentServer = srv;
            m_serverSeted = true;
        }
    }

    query.clear();

    if (m_serverSeted)
        m_serverSeted = MediaHelper::Instance()->ping(currentServer);


    if (m_serverSeted){
        m_cacheDir = m_appDataDir + "cache" + QDir::separator() +currentSeverName + QDir::separator();
        QDir dir(m_cacheDir);
        if(!dir.exists()){
            dir.mkpath(m_cacheDir);
        }
    }

    s_options.albumCountPerPage = this->value("albumCountPerPage").toInt();

}

void Config::initConfigValue(){

    if (!connectConfigDB())
        return;


    QSqlQuery query(m_configdb);

    bool ret = false;

    ret = query.exec("CREATE TABLE config ("
               "name TEXT,"
               "value TEXT,"
               "PRIMARY KEY (name));");

    ret = query.exec("CREATE TABLE servers ("
               "id INTEGER,"
               "name TEXT,"
               "url TEXT,"
               "user TEXT,"
               "password TEXT,"
               "usetoken INTEGER,"
               "version TEXT,"
               "PRIMARY KEY(id AUTOINCREMENT));");

    ret = query.exec("CREATE TABLE playqueue ("
               "id INTEGER,"
               "songid TEXT,"
               "parent TEXT,"
               "title TEXT,"
               "album TEXT,"
               "artist TEXT,"
               "track INTEGER,"
               "genre TEXT,"
               "coverArt TEXT,"
               "size INTEGER,"
               "suffix TEXT,"
               "duration INTEGER,"
               "bitrate INTEGER,"
               "albumID TEXT,"
               "artistID TEXT,"
               "PRIMARY KEY(id AUTOINCREMENT));");

    ret = query.exec("CREATE TABLE cachetracks ("
               "id INTEGER,"
               "songid TEXT,"
               "parent TEXT,"
               "title TEXT,"
               "album TEXT,"
               "artist TEXT,"
               "track INTEGER,"
               "genre TEXT,"
               "coverArt TEXT,"
               "size INTEGER,"
               "suffix TEXT,"
               "duration INTEGER,"
               "bitrate INTEGER,"
               "albumID TEXT,"
               "artistID TEXT,"
               "filepath TEXT,"
               "PRIMARY KEY(id AUTOINCREMENT));");

    ret = query.exec("INSERT INTO servers (name, url, user, password, usetoken, version )"
                     " VALUES ( 'Navidrome@Demo', 'https://demo.navidrome.org/', 'demo', 'demo', 1, '1.16.1');");

    //Config::G_Debug("INSERT DATA servers:",ret );

    ret = query.exec("INSERT INTO config (name, value ) VALUES ( 'volumeValue', '100');");

    ret = query.exec("INSERT INTO config (name, value ) VALUES ( 'currentSever', 'Navidrome@Demo');");

    setValue("albumCountPerPage", "50");

    Config::G_Debug("INSERT DATA config:",ret );
}

bool Config::connectConfigDB()
{
    if (QSqlDatabase::contains("CONFIGDB")) {
        m_configdb = QSqlDatabase::database("CONFIGDB");
    } else {
        m_configdb = QSqlDatabase::addDatabase("QSQLITE", "CONFIGDB");
        m_configdb.setDatabaseName(m_dbFilePath);
        if (!m_configdb.open()) {
            Config::G_Debug("Cannot open database:", m_dbFilePath);
            return false;
        }
    }
    return true;
}

QString Config::value(const QString &key)
{
    QString value = "";

    QSqlQuery query(m_configdb);

    query.exec(QString("SELECT value FROM config WHERE name='%1';").arg(key));

    if (query.next()){
        value = query.value(0).toString();
    }
    query.clear();
    return value;
}

bool Config::setValue(const QString &key, const QString &value)
{
    QSqlQuery query(m_configdb);

    query.exec(QString("SELECT value FROM config WHERE name='%1';").arg(key));

    if (query.size() > 0){
        query.clear();
        return query.exec(QString("UPDATE config SET value='%1' WHERE name='%2';").arg(value, key));
    }

    query.clear();

    query.prepare("INSERT INTO config (name, value) "
                  "VALUES ( :name, :value)");
    query.bindValue(":name", key);
    query.bindValue(":value", value);
    return query.exec();

}

void Config::addServer(Server &srv)
{

    QSqlQuery query(m_configdb);
    query.prepare("INSERT INTO servers (name, url, user, password, usetoken, version )"
                     " VALUES ( :name, :url, :user, :password, :usetoken, :version);");

    query.bindValue(":name", srv.name);
    query.bindValue(":url", srv.url);
    query.bindValue(":user", srv.user);
    query.bindValue(":password", srv.password);
    query.bindValue(":usetoken", srv.useToken ? 1:0);
    query.bindValue(":version", srv.version);

    bool ret = query.exec();

    if (ret)
        servers << srv;
}

Config::Server &Config::getServerByName(const QString &name)
{

    QSqlQuery query(m_configdb);

    query.exec(QString("SELECT * FROM servers;"));

    Config::Server server;
    return server;
}

void Config::reset()
{
    QFile::remove(m_dbFilePath);
    removePath(m_cacheDir);
}

bool Config::removePath(const QString &path)
{
    QFileInfo fileInfo(path);

    if (!fileInfo.exists()){
        return true;
    }

    if (fileInfo.isFile()){
        return QFile::remove(path);
    }

    QDir dir(path);

#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    return dir.removeRecursively();
#else
    QStringList fileList = dir.entryList(QDir::NoDotAndDotDot);
    for(int i =0; i < fileList.size(); ++i){
        G_Debug(fileList.at(i));
        removePath(path + QDir::separator() + fileList.at(i));
    }
    return true;
#endif
}

/*
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "MEMORY");
    db.setDatabaseName(":memory:");
    if (!db.open()) {
        qDebug() << "Cannot open database";
        return;
    }

    QSqlQuery query(db);

    query.exec("CREATE TABLE randomsongs ("
               "id INTEGER,"
               "songid TEXT,"
               "parent TEXT,"
               "title TEXT,"
               "album TEXT,"
               "artist TEXT,"
               "track INTEGER,"
               "genre TEXT,"
               "coverArt TEXT,"
               "size INTEGER,"
               "suffix TEXT,"
               "duration INTEGER,"
               "bitrate INTEGER,"
               "albumID TEXT,"
               "artistID TEXT,"
               "PRIMARY KEY(id AUTOINCREMENT));");

    query.exec("CREATE TABLE playlists ("
               "id INTEGER,"
               "playlistid TEXT,"
               "name TEXT,"
               "songCount INTEGER,"
               "duration INTEGER,"
               "owner TEXT,"
               "created TEXT,"
               "changed TEXT,"
               "coverArt TEXT,"
               "PRIMARY KEY(id AUTOINCREMENT));");

    query.exec("CREATE TABLE albums ("
               "id INTEGER,"
               "albumid TEXT,"
               "parent TEXT,"
               "title TEXT,"
               "name TEXT,"
               "album TEXT,"
               "artist TEXT,"
               "year INTEGER,"
               "genre TEXT,"
               "coverArt TEXT,"
               "duration INTEGER,"
               "playCount INTEGER,"
               "played TEXT,"
               "created TEXT,"
               "artistID TEXT,"
               "songCount INTEGER,"
               "PRIMARY KEY(id AUTOINCREMENT));");

    query.exec("CREATE TABLE artists ("
               "id INTEGER,"
               "artistid TEXT,"
               "name TEXT,"
               "coverArt TEXT,"
               "artistImageUrl TEXT,"
               "PRIMARY KEY(id AUTOINCREMENT));");

    query.exec("CREATE TABLE genres ("
               "id INTEGER,"
               "title TEXT,"
               "songcount INTEGER,"
               "albumcount INTEGER,"
               "PRIMARY KEY(id AUTOINCREMENT));");

    query.exec("CREATE TABLE updatetime ("
               "id INTEGER,"
               "table TEXT,"
               "updatetime INTEGER,"
               "PRIMARY KEY(id AUTOINCREMENT));");

    query.exec("CREATE TABLE config ("
               "id INTEGER,"
               "key TEXT,"
               "value INTEGER,"
               "PRIMARY KEY(id AUTOINCREMENT));");

    query.exec("CREATE TABLE server ("
               "id INTEGER,"
               "url TEXT,"
               "username TEXT,"
               "password TEXT,"
               "PRIMARY KEY(id AUTOINCREMENT));");

*/



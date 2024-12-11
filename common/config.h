#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QVariant>

#include <QMutex>
#include <QSqlDatabase>

#ifdef G_DEBUG
#include <QDebug>
#endif


#define POSTER_WIDTH    180
#define POSTER_RADIUS    19
#define ICON_WIDTH       48



class Config : public QObject
{
    Q_OBJECT

public:

    struct Server{
        QString name;
        QString url;
        QString user;
        QString password;
        QString token;
        QString salt;
        QString version;
        bool useToken=false;
    };

    typedef QList<Server> ServerList;

    struct Options{
        int albumCountPerPage;
    };

    Server currentServer;
    ServerList servers;

    static Options s_options;

    static Config *Instance()
    {
        static QMutex mutex;

        if (!self) {
            QMutexLocker locker(&mutex);

            if (!self) {
                self = new Config;
            }
        }

        return self;
    }

    static QString secsToString(int secs)
    {
        int m = secs / 60;
        int h = m / 60;
        m = m % 60;
        secs = secs % 60;
        if(h > 0) {
            return QString("%1:%2:%3").arg(h).arg(m, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
        }
        return QString("%1:%2").arg(m).arg(secs, 2, 10, QChar('0'));
    }

    static QString msecsToTimeString(qint64 ms)
    {
        int s = ms / 1000;
        return  Instance()->secsToString(s);
    }

    static const QString &appDataDir () { return Instance()->m_appDataDir; }
    static const QString &dbFilePath () { return Instance()->m_dbFilePath; }
    static const QString &cacheDir () { return Instance()->m_cacheDir; }
    static const QSqlDatabase &configDB () {
        return Instance()->m_configdb;
    }

    static void G_Debug(const QVariant &str, const QVariant &str2 = QVariant(), const QVariant &str3=QVariant() ){
#ifdef G_DEBUG
        if(str2.toString().isEmpty() && str3.toString().isEmpty()){
            qDebug() << str.toString();
        }
        else if(str3.toString().isEmpty()){
            qDebug() << str.toString() << str2.toString();
        }
        else{
            qDebug() << str.toString() << str2.toString() << str3.toString();
        }
#endif
    }

    QString value(const QString &key);
    bool setValue(const QString &key, const QString &value);

    void addServer(Config::Server &srv);
    bool setCurrentSever(const QString &serverName);
    Server getServerByName(const QString &name);
    bool serverSeted(){return  Instance()->m_serverSeted;}

signals:
private:
    explicit Config(QObject *parent = nullptr);
    static Config *self;

    bool m_serverSeted;

    QSqlDatabase m_configdb;
    QString m_appDataDir;
    QString m_cacheDir;
    QString m_dbFileName;
    QString m_dbFilePath;

    void loadConfigValue();
    void initConfigValue();
    bool connectConfigDB();
    void reset();
    bool removePath(const QString &path);
};

#endif // CONFIG_H

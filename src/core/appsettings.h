/**
 * @file    appsettings.h
 * @brief   应用设置：以 SQLite config 表为后端，带变更信号
 */
#ifndef NAVIPLAYER_APPSETTINGS_H
#define NAVIPLAYER_APPSETTINGS_H

#include <QObject>
#include <QString>
#include <QVariant>

class DatabaseManager;

class AppSettings : public QObject
{
    Q_OBJECT
public:
    enum class Theme { Dark, Light };
    Q_ENUM(Theme)

    explicit AppSettings(DatabaseManager *db, QObject *parent = nullptr);

    // ---------- 服务器 ----------
    QString serverUrl() const;
    void setServerUrl(const QString &v);

    QString username() const;
    void setUsername(const QString &v);

    QString password() const;
    void setPassword(const QString &v);

    bool ignoreSslErrors() const;
    void setIgnoreSslErrors(bool v);

    int maxBitRate() const;                 // 0 表示不限
    void setMaxBitRate(int v);

    // ---------- 外观 ----------
    Theme theme() const;
    void setTheme(Theme v);

    QString language() const;               // "system" / "zh_CN" / "en_US"
    void setLanguage(const QString &v);

    // ---------- 歌词 ----------
    QString lyricsDirectory() const;
    void setLyricsDirectory(const QString &v);

    QString lyricsFontFamily() const;
    void setLyricsFontFamily(const QString &v);

    int lyricsFontSize() const;
    void setLyricsFontSize(int v);

    QString lyricsHighlightColor() const;
    void setLyricsHighlightColor(const QString &v);

    QString lyricsNormalColor() const;
    void setLyricsNormalColor(const QString &v);

    bool lyricsEnabled() const;
    void setLyricsEnabled(bool v);

    int lyricsOffsetMs() const;
    void setLyricsOffsetMs(int v);

    // ---------- 缓存 ----------
    int maxSongCacheMb() const;             // <=0 表示不缓存
    void setMaxSongCacheMb(int v);

    int maxCoverCacheMb() const;
    void setMaxCoverCacheMb(int v);

    int pageCacheTtlMinutes() const;        // 页面数据 TTL
    void setPageCacheTtlMinutes(int v);

    // ---------- 显示 ----------
    int albumsPerPage() const;
    void setAlbumsPerPage(int v);

    int songsPerPage() const;
    void setSongsPerPage(int v);

    // ---------- 播放器 ----------
    int volume() const;                     // 0-100
    void setVolume(int v);

    int playMode() const;
    void setPlayMode(int v);

signals:
    void settingChanged(const QString &key);
    void themeChanged(Theme theme);
    void languageChanged(const QString &language);

private:
    QString value(const QString &key, const QString &def = QString()) const;
    int intValue(const QString &key, int def) const;
    bool boolValue(const QString &key, bool def) const;
    void setValue(const QString &key, const QVariant &v);

    DatabaseManager *m_db = nullptr;
};

#endif // NAVIPLAYER_APPSETTINGS_H

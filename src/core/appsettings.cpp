#include "appsettings.h"

#include "core/databasemanager.h"

namespace {
const char *kServerUrl      = "server/url";
const char *kUsername       = "server/username";
const char *kPassword       = "server/password";
const char *kIgnoreSsl      = "server/ignoreSslErrors";
const char *kMaxBitRate     = "server/maxBitRate";

const char *kTheme          = "ui/theme";
const char *kLanguage       = "ui/language";

const char *kLyricsDir      = "lyrics/directory";
const char *kLyricsFont     = "lyrics/fontFamily";
const char *kLyricsSize     = "lyrics/fontSize";
const char *kLyricsHiColor  = "lyrics/highlightColor";
const char *kLyricsColor    = "lyrics/normalColor";
const char *kLyricsEnabled  = "lyrics/enabled";
const char *kLyricsOffset   = "lyrics/offsetMs";

const char *kSongCacheMb    = "cache/maxSongCacheMb";
const char *kCoverCacheMb   = "cache/maxCoverCacheMb";
const char *kPageTtl        = "cache/pageTtlMinutes";

const char *kAlbumsPerPage  = "display/albumsPerPage";
const char *kSongsPerPage   = "display/songsPerPage";

const char *kVolume         = "player/volume";
const char *kPlayMode       = "player/playMode";
}

AppSettings::AppSettings(DatabaseManager *db, QObject *parent)
    : QObject(parent)
    , m_db(db)
{
}

QString AppSettings::value(const QString &key, const QString &def) const
{
    if (!m_db)
        return def;
    const QString v = m_db->loadConfig(key);
    return v.isEmpty() ? def : v;
}

int AppSettings::intValue(const QString &key, int def) const
{
    bool ok = false;
    const int v = value(key).toInt(&ok);
    return ok ? v : def;
}

bool AppSettings::boolValue(const QString &key, bool def) const
{
    const QString v = value(key).toLower();
    if (v == QLatin1String("true") || v == QLatin1String("1"))
        return true;
    if (v == QLatin1String("false") || v == QLatin1String("0"))
        return false;
    return def;
}

void AppSettings::setValue(const QString &key, const QVariant &v)
{
    if (m_db)
        m_db->saveConfig(key, v.toString());
    Q_EMIT settingChanged(key);
}

// ---------------- 服务器 ----------------
QString AppSettings::serverUrl() const { return value(kServerUrl, QStringLiteral("https://m.redtux.cn:666")); }
void AppSettings::setServerUrl(const QString &v) { setValue(kServerUrl, v); }

QString AppSettings::username() const { return value(kUsername, QStringLiteral("test")); }
void AppSettings::setUsername(const QString &v) { setValue(kUsername, v); }

QString AppSettings::password() const { return value(kPassword, QStringLiteral("123456")); }
void AppSettings::setPassword(const QString &v) { setValue(kPassword, v); }

bool AppSettings::ignoreSslErrors() const { return boolValue(kIgnoreSsl, true); }
void AppSettings::setIgnoreSslErrors(bool v) { setValue(kIgnoreSsl, v); }

int AppSettings::maxBitRate() const { return intValue(kMaxBitRate, 0); }
void AppSettings::setMaxBitRate(int v) { setValue(kMaxBitRate, v); }

// ---------------- 外观 ----------------
AppSettings::Theme AppSettings::theme() const
{
    return value(kTheme, QStringLiteral("dark")) == QLatin1String("light") ? Theme::Light : Theme::Dark;
}

void AppSettings::setTheme(Theme v)
{
    if (theme() == v)
        return;
    setValue(kTheme, v == Theme::Light ? QStringLiteral("light") : QStringLiteral("dark"));
    Q_EMIT themeChanged(v);
}

QString AppSettings::language() const { return value(kLanguage, QStringLiteral("system")); }
void AppSettings::setLanguage(const QString &v)
{
    if (language() == v)
        return;
    setValue(kLanguage, v);
    Q_EMIT languageChanged(v);
}

// ---------------- 歌词 ----------------
QString AppSettings::lyricsDirectory() const { return value(kLyricsDir); }
void AppSettings::setLyricsDirectory(const QString &v) { setValue(kLyricsDir, v); }

QString AppSettings::lyricsFontFamily() const { return value(kLyricsFont, QStringLiteral("PingFang SC")); }
void AppSettings::setLyricsFontFamily(const QString &v) { setValue(kLyricsFont, v); }

int AppSettings::lyricsFontSize() const { return intValue(kLyricsSize, 22); }
void AppSettings::setLyricsFontSize(int v) { setValue(kLyricsSize, v); }

QString AppSettings::lyricsHighlightColor() const { return value(kLyricsHiColor, QStringLiteral("#1DB954")); }
void AppSettings::setLyricsHighlightColor(const QString &v) { setValue(kLyricsHiColor, v); }

QString AppSettings::lyricsNormalColor() const { return value(kLyricsColor, QStringLiteral("#FFFFFF")); }
void AppSettings::setLyricsNormalColor(const QString &v) { setValue(kLyricsColor, v); }

bool AppSettings::lyricsEnabled() const { return boolValue(kLyricsEnabled, false); }
void AppSettings::setLyricsEnabled(bool v) { setValue(kLyricsEnabled, v); }

int AppSettings::lyricsOffsetMs() const { return intValue(kLyricsOffset, 0); }
void AppSettings::setLyricsOffsetMs(int v) { setValue(kLyricsOffset, v); }

// ---------------- 缓存 ----------------
int AppSettings::maxSongCacheMb() const { return intValue(kSongCacheMb, 2048); }
void AppSettings::setMaxSongCacheMb(int v) { setValue(kSongCacheMb, v); }

int AppSettings::maxCoverCacheMb() const { return intValue(kCoverCacheMb, 256); }
void AppSettings::setMaxCoverCacheMb(int v) { setValue(kCoverCacheMb, v); }

int AppSettings::pageCacheTtlMinutes() const { return intValue(kPageTtl, 30); }
void AppSettings::setPageCacheTtlMinutes(int v) { setValue(kPageTtl, v); }

// ---------------- 显示 ----------------
int AppSettings::albumsPerPage() const { return intValue(kAlbumsPerPage, 40); }
void AppSettings::setAlbumsPerPage(int v) { setValue(kAlbumsPerPage, v); }

int AppSettings::songsPerPage() const { return intValue(kSongsPerPage, 100); }
void AppSettings::setSongsPerPage(int v) { setValue(kSongsPerPage, v); }

// ---------------- 播放器 ----------------
int AppSettings::volume() const { return intValue(kVolume, 80); }
void AppSettings::setVolume(int v) { setValue(kVolume, v); }

int AppSettings::playMode() const { return intValue(kPlayMode, 1); }  // 默认列表循环
void AppSettings::setPlayMode(int v) { setValue(kPlayMode, v); }

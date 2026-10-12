/**
 * @file    lyricsmanager.h
 * @brief   歌词管理：本地同名 .lrc 优先 -> ffmpeg 内嵌歌词 -> 服务器歌词
 */
#ifndef NAVIPLAYER_LYRICSNAMANAGER_H
#define NAVIPLAYER_LYRICSNAMANAGER_H

#include <QObject>
#include <QColor>
#include <QString>
#include <QVector>

#include "models/song.h"

class AppSettings;
class SubsonicApi;
class CacheManager;

struct LyricLine
{
    qint64 timeMs = 0;
    QString text;
};

struct LyricsStyle
{
    QString fontFamily = QStringLiteral("PingFang SC");
    int fontSize = 22;
    QColor highlightColor = QColor(QStringLiteral("#1DB954"));
    QColor normalColor = QColor(QStringLiteral("#FFFFFF"));
    QColor shadowColor = QColor(0, 0, 0, 180);
    bool boldHighlight = true;
};

class LyricsManager : public QObject
{
    Q_OBJECT
public:
    enum class Status { NoLyrics, Loading, Loaded };
    Q_ENUM(Status)

    enum class Source { None, Sidecar, Embedded, Server };
    Q_ENUM(Source)

    explicit LyricsManager(AppSettings *settings, SubsonicApi *api, CacheManager *cache,
                           QObject *parent = nullptr);

    void loadForSong(const Song &song);
    void clear();

    void updatePosition(qint64 positionMs);

    QString currentLine() const;
    QString nextLine() const;
    Status status() const { return m_status; }
    Source source() const { return m_source; }
    bool hasTimestamps() const { return m_hasTimestamps; }

    LyricsStyle style() const { return m_style; }
    void setStyle(const LyricsStyle &style);
    void loadStyleFromSettings();

    int offsetMs() const { return m_offsetMs; }
    void setOffsetMs(int ms);

    Song currentSong() const { return m_song; }

signals:
    void lyricsChanged(Status status);
    void linesChanged(const QString &current, const QString &next);
    void styleChanged(const LyricsStyle &style);

private slots:
    void onApiLyricsReady(const QString &artist, const QString &title, const QString &lyrics);

private:
    bool loadLocalLyrics(const Song &song);
    void tryEmbeddedLyrics(const Song &song);
    void applyLyrics(const QString &text, Source source);
    static QVector<LyricLine> parseLrc(const QString &text);
    int indexForPosition(qint64 ms) const;
    static QString localLyricsPath(const Song &song, const QString &dir);

    AppSettings *m_settings = nullptr;
    SubsonicApi *m_api = nullptr;
    CacheManager *m_cache = nullptr;

    Song m_song;
    QVector<LyricLine> m_lines;
    int m_currentIndex = -1;
    qint64 m_positionMs = 0;
    int m_offsetMs = 0;
    bool m_hasTimestamps = false;
    Status m_status = Status::NoLyrics;
    Source m_source = Source::None;
    LyricsStyle m_style;
};

#endif // NAVIPLAYER_LYRICSNAMANAGER_H

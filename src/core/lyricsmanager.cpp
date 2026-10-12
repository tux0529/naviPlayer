#include "lyricsmanager.h"

#include "core/appsettings.h"
#include "core/subsonicapi.h"
#include "core/cachemanager.h"
#include "core/ffmpegaudiodecoder.h"
#include "core/asynctask.h"
#include "core/qt_compat.h"

#include <algorithm>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTextStream>

namespace {

QString sanitizeFileName(const QString &name)
{
    QString s = name;
    s.replace(QLatin1Char('/'), QLatin1Char('_'));
    s.replace(QLatin1Char('\\'), QLatin1Char('_'));
    s.replace(QLatin1Char(':'), QLatin1Char('_'));
    return s.trimmed();
}

}

LyricsManager::LyricsManager(AppSettings *settings, SubsonicApi *api, CacheManager *cache,
                             QObject *parent)
    : QObject(parent)
    , m_settings(settings)
    , m_api(api)
    , m_cache(cache)
{
    loadStyleFromSettings();

    if (m_api) {
        connect(m_api, &SubsonicApi::lyricsReady, this, &LyricsManager::onApiLyricsReady);
    }
    if (m_settings) {
        m_offsetMs = m_settings->lyricsOffsetMs();
    }
}

void LyricsManager::loadStyleFromSettings()
{
    if (!m_settings)
        return;
    m_style.fontFamily = m_settings->lyricsFontFamily();
    m_style.fontSize = m_settings->lyricsFontSize();
    m_style.highlightColor = QColor(m_settings->lyricsHighlightColor());
    m_style.normalColor = QColor(m_settings->lyricsNormalColor());
}

void LyricsManager::setStyle(const LyricsStyle &style)
{
    if (m_style.fontFamily == style.fontFamily && m_style.fontSize == style.fontSize
        && m_style.highlightColor == style.highlightColor && m_style.normalColor == style.normalColor) {
        return;
    }
    m_style = style;
    Q_EMIT styleChanged(m_style);
}

void LyricsManager::setOffsetMs(int ms)
{
    m_offsetMs = ms;
}

void LyricsManager::clear()
{
    m_song = Song();
    m_lines.clear();
    m_currentIndex = -1;
    m_hasTimestamps = false;
    m_source = Source::None;
    m_status = Status::NoLyrics;
    Q_EMIT lyricsChanged(m_status);
    Q_EMIT linesChanged(QString(), QString());
}

QString LyricsManager::localLyricsPath(const Song &song, const QString &dir)
{
    const QString baseName = QStringLiteral("%1 - %2.lrc")
                                 .arg(sanitizeFileName(song.artist()), sanitizeFileName(song.title()));
    return QDir(dir).filePath(baseName);
}

bool LyricsManager::loadLocalLyrics(const Song &song)
{
    QStringList candidates;

    if (m_settings && !m_settings->lyricsDirectory().isEmpty())
        candidates.append(localLyricsPath(song, m_settings->lyricsDirectory()));

    // 与歌曲文件同目录（本地缓存文件旁）
    if (!song.path().isEmpty()) {
        QFileInfo info(song.path());
        candidates.append(info.absolutePath() + QLatin1Char('/') + info.completeBaseName()
                          + QStringLiteral(".lrc"));
    }

    for (const QString &path : candidates) {
        QFile f(path);
        if (!f.exists() || !f.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;
        QTextStream in(&f);
        in.setCodec("UTF-8");
        const QString text = in.readAll();
        f.close();
        if (text.trimmed().isEmpty())
            continue;
        applyLyrics(text, Source::Sidecar);
        return true;
    }
    return false;
}

void LyricsManager::loadForSong(const Song &song)
{
    clear();
    if (!song.isValid())
        return;

    m_song = song;

    // 1) 本地同名歌词
    if (loadLocalLyrics(song))
        return;

    m_status = Status::Loading;
    Q_EMIT lyricsChanged(m_status);

    // 2) ffmpeg 解析内嵌歌词（需要本地文件；未缓存则先缓存）
    const QString localPath = m_cache ? m_cache->cachedSongPath(song.id()) : QString();
    if (!localPath.isEmpty()) {
        tryEmbeddedLyrics(song);
        return;
    }

    // 3) 服务器歌词（最后兜底）
    if (m_api && m_api->isConfigured()) {
        m_api->getLyrics(song.artist(), song.title());
        return;
    }

    m_status = Status::NoLyrics;
    Q_EMIT lyricsChanged(m_status);
}

void LyricsManager::tryEmbeddedLyrics(const Song &song)
{
    const QString localPath = m_cache ? m_cache->cachedSongPath(song.id()) : QString();
    if (localPath.isEmpty())
        return;

    Async::run(this, [localPath]() -> QString {
        return FFmpegAudioDecoder::extractEmbeddedLyrics(localPath);
    }, [this, song](const QString &lyrics) {
        if (m_song.id() != song.id())
            return;      // 已切歌，丢弃过期结果
        if (!lyrics.trimmed().isEmpty()) {
            applyLyrics(lyrics, Source::Embedded);
            return;
        }
        if (m_api && m_api->isConfigured())
            m_api->getLyrics(song.artist(), song.title());
        else {
            m_status = Status::NoLyrics;
            Q_EMIT lyricsChanged(m_status);
        }
    });
}

void LyricsManager::onApiLyricsReady(const QString &artist, const QString &title,
                                     const QString &lyrics)
{
    if (m_song.artist() != artist || m_song.title() != title)
        return;
    if (lyrics.trimmed().isEmpty()) {
        m_status = Status::NoLyrics;
        Q_EMIT lyricsChanged(m_status);
        return;
    }
    applyLyrics(lyrics, Source::Server);
}

QVector<LyricLine> LyricsManager::parseLrc(const QString &text)
{
    QVector<LyricLine> lines;

    // [mm:ss.xx] 或 [mm:ss:xx]，支持一行多个时间标签
    static const QRegularExpression tagRe(
        QStringLiteral(R"(\[(\d{1,3}):(\d{1,2})(?:[.:](\d{1,3}))?\])"));

    const QStringList rawLines = text.split(QLatin1Char('\n'), NaviKeepEmptyParts);
    for (const QString &raw : rawLines) {
        const QString line = raw.trimmed();
        if (line.isEmpty())
            continue;

        auto it = tagRe.globalMatch(line);
        int lastEnd = -1;
        bool hasTag = false;
        QVector<qint64> times;
        while (it.hasNext()) {
            const QRegularExpressionMatch m = it.next();
            const int minute = m.captured(1).toInt();
            const int second = m.captured(2).toInt();
            QString frac = m.captured(3);
            while (frac.length() < 3)
                frac.append(QLatin1Char('0'));
            const int ms = frac.left(3).toInt();
            times.append((qint64)minute * 60000 + second * 1000 + ms);
            lastEnd = m.capturedEnd();
            hasTag = true;
        }

        if (!hasTag)
            continue;

        QString content = line.mid(lastEnd).trimmed();
        // 去掉 [ti:xxx] 之类元信息
        static const QRegularExpression metaRe(QStringLiteral(R"(\[[a-zA-Z]+:[^\]]*\])"));
        content.remove(metaRe);
        content = content.trimmed();

        for (qint64 t : times)
            lines.append({t, content});
    }

    std::sort(lines.begin(), lines.end(), [](const LyricLine &a, const LyricLine &b) {
        return a.timeMs < b.timeMs;
    });
    return lines;
}

void LyricsManager::applyLyrics(const QString &text, Source source)
{
    const QVector<LyricLine> parsed = parseLrc(text);

    if (parsed.isEmpty()) {
        // 无时间戳的纯文本歌词：作为单行展示
        m_lines.clear();
        const QStringList parts = text.split(QLatin1Char('\n'), NaviSkipEmptyParts);
        for (const QString &p : parts)
            m_lines.append({-1, p.trimmed()});
        m_hasTimestamps = false;
    } else {
        m_lines = parsed;
        m_hasTimestamps = true;
    }

    m_source = source;
    m_status = m_lines.isEmpty() ? Status::NoLyrics : Status::Loaded;
    m_currentIndex = -1;

    Q_EMIT lyricsChanged(m_status);
    updatePosition(m_positionMs);
}

int LyricsManager::indexForPosition(qint64 ms) const
{
    if (m_lines.isEmpty() || !m_hasTimestamps)
        return -1;

    const qint64 target = ms + m_offsetMs;
    int lo = 0;
    int hi = m_lines.size() - 1;
    int result = -1;
    while (lo <= hi) {
        const int mid = (lo + hi) / 2;
        if (m_lines.at(mid).timeMs <= target) {
            result = mid;
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    return result;
}

void LyricsManager::updatePosition(qint64 positionMs)
{
    m_positionMs = positionMs;

    if (m_lines.isEmpty()) {
        if (m_currentIndex != -1) {
            m_currentIndex = -1;
            Q_EMIT linesChanged(QString(), QString());
        }
        return;
    }

    if (!m_hasTimestamps) {
        Q_EMIT linesChanged(m_lines.first().text, QString());
        return;
    }

    const int index = indexForPosition(positionMs);
    if (index == m_currentIndex)
        return;

    m_currentIndex = index;
    const QString current = index >= 0 ? m_lines.at(index).text : QString();
    const QString next = (index >= 0 && index + 1 < m_lines.size()) ? m_lines.at(index + 1).text
                                                                    : QString();
    Q_EMIT linesChanged(current, next);
}

QString LyricsManager::currentLine() const
{
    if (m_currentIndex < 0 || m_currentIndex >= m_lines.size())
        return QString();
    return m_lines.at(m_currentIndex).text;
}

QString LyricsManager::nextLine() const
{
    if (m_currentIndex < 0 || m_currentIndex + 1 >= m_lines.size())
        return QString();
    return m_lines.at(m_currentIndex + 1).text;
}

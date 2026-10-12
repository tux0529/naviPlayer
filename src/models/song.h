/**
 * @file    song.h
 * @brief   歌曲数据模型（Subsonic child 字段的超集）
 */
#ifndef NAVIPLAYER_SONG_H
#define NAVIPLAYER_SONG_H

#include <QString>
#include <QMetaType>
#include <QDataStream>
#include <QJsonObject>

class Song
{
public:
    Song() = default;

    // ---- 基本信息 ----
    QString id() const { return m_id; }
    void setId(const QString &v) { m_id = v; }

    QString title() const { return m_title; }
    void setTitle(const QString &v) { m_title = v; }

    QString artist() const { return m_artist; }
    void setArtist(const QString &v) { m_artist = v; }

    QString artistId() const { return m_artistId; }
    void setArtistId(const QString &v) { m_artistId = v; }

    QString album() const { return m_album; }
    void setAlbum(const QString &v) { m_album = v; }

    QString albumId() const { return m_albumId; }
    void setAlbumId(const QString &v) { m_albumId = v; }

    int track() const { return m_track; }
    void setTrack(int v) { m_track = v; }

    int discNumber() const { return m_discNumber; }
    void setDiscNumber(int v) { m_discNumber = v; }

    int year() const { return m_year; }
    void setYear(int v) { m_year = v; }

    QString genre() const { return m_genre; }
    void setGenre(const QString &v) { m_genre = v; }

    int duration() const { return m_duration; }         // 秒
    void setDuration(int v) { m_duration = v; }

    qint64 size() const { return m_size; }              // 字节
    void setSize(qint64 v) { m_size = v; }

    QString suffix() const { return m_suffix; }
    void setSuffix(const QString &v) { m_suffix = v; }

    QString contentType() const { return m_contentType; }
    void setContentType(const QString &v) { m_contentType = v; }

    QString coverArtId() const { return m_coverArtId; }
    void setCoverArtId(const QString &v) { m_coverArtId = v; }

    QString path() const { return m_path; }
    void setPath(const QString &v) { m_path = v; }

    // ---- 音频技术信息（歌曲详情弹窗用） ----
    int bitRate() const { return m_bitRate; }           // kbps
    void setBitRate(int v) { m_bitRate = v; }

    int sampleRate() const { return m_sampleRate; }     // Hz
    void setSampleRate(int v) { m_sampleRate = v; }

    int channels() const { return m_channels; }
    void setChannels(int v) { m_channels = v; }

    int playCount() const { return m_playCount; }
    void setPlayCount(int v) { m_playCount = v; }

    QString created() const { return m_created; }
    void setCreated(const QString &v) { m_created = v; }

    bool isFavorite() const { return m_isFavorite; }
    void setFavorite(bool v) { m_isFavorite = v; }

    /** 本地缓存文件路径（非空表示已缓存） */
    QString localPath() const { return m_localPath; }
    void setLocalPath(const QString &v) { m_localPath = v; }

    bool isCached() const { return !m_localPath.isEmpty(); }

    /** 显示用时长 mm:ss */
    QString durationText() const
    {
        if (m_duration <= 0)
            return QStringLiteral("--:--");
        const int m = m_duration / 60;
        const int s = m_duration % 60;
        return QStringLiteral("%1:%2").arg(m).arg(s, 2, 10, QLatin1Char('0'));
    }

    /** 显示用文件大小 */
    QString sizeText() const
    {
        if (m_size <= 0)
            return QString();
        double v = static_cast<double>(m_size);
        const char *units[] = {"B", "KB", "MB", "GB"};
        int i = 0;
        while (v >= 1024.0 && i < 3) {
            v /= 1024.0;
            ++i;
        }
        return QStringLiteral("%1 %2").arg(v, 0, 'f', i == 0 ? 0 : 1).arg(QString::fromLatin1(units[i]));
    }

    // ---- 序列化（SQLite / 会话恢复用） ----
    QJsonObject toJson() const;
    static Song fromJson(const QJsonObject &obj);

    bool isValid() const { return !m_id.isEmpty(); }

private:
    QString m_id;
    QString m_title;
    QString m_artist;
    QString m_artistId;
    QString m_album;
    QString m_albumId;
    int m_track = 0;
    int m_discNumber = 1;
    int m_year = 0;
    QString m_genre;
    int m_duration = 0;
    qint64 m_size = 0;
    QString m_suffix;
    QString m_contentType;
    QString m_coverArtId;
    QString m_path;
    int m_bitRate = 0;
    int m_sampleRate = 0;
    int m_channels = 0;
    int m_playCount = 0;
    QString m_created;
    bool m_isFavorite = false;
    QString m_localPath;
};

Q_DECLARE_METATYPE(Song)

#endif // NAVIPLAYER_SONG_H

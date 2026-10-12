/**
 * @file    playbackqueue.h
 * @brief   播放队列：既是 QTableView 的数据源，也是播放模式状态机
 */
#ifndef NAVIPLAYER_PLAYBACKQUEUE_H
#define NAVIPLAYER_PLAYBACKQUEUE_H

#include <QAbstractTableModel>
#include <QVector>
#include <QString>

#include "models/song.h"

class PlaybackQueue : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum class PlayMode { Sequential, RepeatAll, RepeatOne, Shuffle };
    Q_ENUM(PlayMode)

    enum Column {
        ColCover = 0,
        ColTitle,
        ColAlbum,
        ColArtist,
        ColDuration,
        ColCount
    };

    enum Roles {
        SongRole = Qt::UserRole + 1,
        CoverArtIdRole = Qt::UserRole + 2,
        IsCurrentRole = Qt::UserRole + 3,
    };

    explicit PlaybackQueue(QObject *parent = nullptr);

    // ---- QAbstractTableModel ----
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                  const QModelIndex &destinationParent, int destinationChild) override;
    Qt::DropActions supportedDropActions() const override;

    // ---- 队列操作 ----
    void append(const QVector<Song> &songs);
    void append(const Song &song);
    /** 插入到当前播放曲之后（“下一首播放”） */
    void insertNext(const QVector<Song> &songs);
    void insertNext(const Song &song);
    void removeAt(int row);
    void clear();
    void moveItem(int from, int to);

    QVector<Song> songs() const { return m_songs; }
    Song songAt(int row) const;
    int indexOfSong(const QString &songId) const;
    bool isEmpty() const { return m_songs.isEmpty(); }
    int size() const { return m_songs.size(); }

    // ---- 播放位置与模式 ----
    int currentIndex() const { return m_currentIndex; }
    void setCurrentIndex(int index);
    Song currentSong() const;

    PlayMode playMode() const { return m_playMode; }
    void setPlayMode(PlayMode mode);
    PlayMode nextPlayMode();

    /** 计算下一曲索引；-1 表示停止 */
    int nextIndex(bool autoAdvance = true) const;
    int previousIndex() const;

    static QString playModeText(PlayMode mode);
    static QString playModeIconName(PlayMode mode);

signals:
    void currentIndexChanged(int index);
    void playModeChanged(PlayMode mode);
    void queueChanged();

private:
    void regenerateShuffleOrder();

    QVector<Song> m_songs;
    int m_currentIndex = -1;
    PlayMode m_playMode = PlayMode::RepeatAll;
    QVector<int> m_shuffleOrder;
    int m_shufflePos = 0;
};

#endif // NAVIPLAYER_PLAYBACKQUEUE_H

/**
 * @file    songtablemodel.h
 * @brief   歌曲列表模型（QTableView 数据源，支持排序、收藏列、拖动排序）
 */
#ifndef NAVIPLAYER_SONGTABLEMODEL_H
#define NAVIPLAYER_SONGTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <QString>

#include "models/song.h"

class SongTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column {
        ColTrack = 0,
        ColFavorite,
        ColTitle,
        ColArtist,
        ColAlbum,
        ColDuration,
        ColCount
    };

    enum Roles {
        SongRole = Qt::UserRole + 1,
        CoverArtIdRole = Qt::UserRole + 2,
        IsPlayingRole = Qt::UserRole + 3,
    };

    explicit SongTableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                  const QModelIndex &destinationParent, int destinationChild) override;
    Qt::DropActions supportedDropActions() const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    void setSongs(const QVector<Song> &songs);
    void appendSongs(const QVector<Song> &songs);
    void clear();
    QVector<Song> songs() const { return m_songs; }
    Song songAt(int row) const;
    int indexOf(const QString &songId) const;
    void moveItem(int from, int to);

    void setPlayingSongId(const QString &songId);
    QString playingSongId() const { return m_playingSongId; }

    void setFavorite(const QString &songId, bool favorite);

    void setDragEnabled(bool enabled) { m_dragEnabled = enabled; }

signals:
    void songsChanged();

private:
    QVector<Song> m_songs;
    QString m_playingSongId;
    bool m_dragEnabled = false;
};

#endif // NAVIPLAYER_SONGTABLEMODEL_H

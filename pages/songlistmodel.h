#ifndef SONGLISTMODEL_H
#define SONGLISTMODEL_H

#include "track.h"

#include <QAbstractTableModel>

class SongListModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit SongListModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void addRow(Track newSong);

    void clear();


private:
    TrackList m_trackList;
};

#endif // SONGLISTMODEL_H

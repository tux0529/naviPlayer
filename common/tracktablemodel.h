#ifndef TRACKTABLEMODEL_H
#define TRACKTABLEMODEL_H

#include "track.h"

#include <QAbstractTableModel>

class TrackTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit TrackTableModel(QObject *parent = nullptr);
    ~TrackTableModel();

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;


    bool addTrack(const Track &trk);
    bool addTracks(const QList<Track> &items);

    bool removeTrack(int pos);
    bool removeTracks(int start, int end);

    bool clear();


private:
    TrackList m_trackList;
};

#endif // TRACKTABLEMODEL_H

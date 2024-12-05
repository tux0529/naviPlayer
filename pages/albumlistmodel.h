#ifndef ALBUMLISTMODEL_H
#define ALBUMLISTMODEL_H

#include "album.h"
#include "mediahelper.h"

#include <QAbstractTableModel>

class AlbumListModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    AlbumListModel(QObject *parent = nullptr);
    ~AlbumListModel();

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;

    void updateData(MediaHelper::AlbumType type, int size, int offset);
    bool clear();
private:
    AlbumList m_albumList;
};

#endif // ALBUMLISTMODEL_H

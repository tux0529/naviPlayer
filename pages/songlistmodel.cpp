#include "songlistmodel.h"

SongListModel::SongListModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

QVariant SongListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return tr("#");
            break;

        case 1:
            return tr("Title");
            break;

        case 2:
            return tr("Album");
            break;

        case 3:
            return tr("Duration");
            break;

        case 4:
            return tr("Bitrate");
            break;

        default:
            return QVariant();
        }
    }
    return QVariant();
}

int SongListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_trackList.size();

}

int SongListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 5;

}

QVariant SongListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_trackList.size() || index.row() < 0)
        return QVariant();

    const auto &track = m_trackList.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
        if (index.column() == 0)
            return  index.row() + 1;
        else if (index.column() == 1)
            return  QString("");
        else if (index.column() == 2)
            return QString("");
        else if (index.column() == 3)
            return track.duration();
        else if (index.column() == 4)
            return track.bitRate();

        break;
    case Qt::UserRole:
        return track.id();
        break;
    case Qt::TextAlignmentRole:
        return Qt::AlignVCenter ;
        break;
    default:
        break;
    }
    return QVariant();
}


void SongListModel::addRow(Track trk)
{
    beginInsertRows(QModelIndex(), rowCount(QModelIndex()) , rowCount(QModelIndex()));
    m_trackList << trk;
    endInsertRows();
}

void SongListModel::clear()
{
    beginResetModel();
    m_trackList.clear();
    endResetModel();
}

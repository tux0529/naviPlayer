#include "tracktablemodel.h"
#include "mediahelper.h"

TrackTableModel::TrackTableModel(QObject *parent)
    : QAbstractTableModel{parent}
{}

TrackTableModel::~TrackTableModel()
{

}

int TrackTableModel::rowCount(const QModelIndex &parent) const
{

    Q_UNUSED(parent);
    return this->m_trackList.size();
}

int TrackTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant TrackTableModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid())
        return QVariant();

    if (index.row() >= this->m_trackList.size() || index.row() < 0)
        return QVariant();

    const Track &trk = this->m_trackList.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        if (index.column() == 0)
            return  index.row() + 1;
        else if (index.column() == 1)
            return  QString("<style>a{color: #CCCCCC; text-decoration:none;}p{margin: 4px 0px;}</style><p>%1</p> <p><a href='search://artist=%2'>%2</a></p>").arg(trk.name(), trk.artist());
        else if (index.column() == 2)
            return trk.album();
        else if (index.column() == 3)
            return trk.duration();
        break;
    case Qt::UserRole:
        return trk.id();
        break;
    case Qt::TextAlignmentRole:
        return Qt::AlignVCenter ;
        break;
    case Qt::DecorationRole:
        if (index.column() == 1)
            return MediaHelper::getAlbumIcon(trk.albumId());
        break;
    default:
        break;
    }
    return QVariant();
}

QVariant TrackTableModel::headerData(int section, Qt::Orientation orientation, int role) const
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

        default:
            return QVariant();
        }
    }
    return QVariant();
}

bool TrackTableModel::removeRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row)
        m_trackList.removeAt(position);

    endRemoveRows();
    return true;
}

bool TrackTableModel::addTrack(const Track &trk)
{
    beginInsertRows(QModelIndex(), m_trackList.size(), m_trackList.size() );
    this->m_trackList << trk;
    endInsertRows();

    return true;
}

bool TrackTableModel::addTracks(const QList<Track> &items)
{
    int count = m_trackList.size();

    beginInsertRows(QModelIndex(), count, count + items.size() -1 );

    this->m_trackList << items;

    endInsertRows();

    return true;
}

bool TrackTableModel::removeTrack(int pos)
{
    this->removeRows(pos, 1);
    return true;
}

bool TrackTableModel::removeTracks(int start, int end)
{
    this->removeRows(start, end - start + 1);
    return true;
}

bool TrackTableModel::clear()
{
    beginResetModel();
    this->m_trackList.clear();
    endResetModel();
    return true;
}


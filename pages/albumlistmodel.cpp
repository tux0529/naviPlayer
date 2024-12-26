#include "albumlistmodel.h"
#include "config.h"

#include <QIcon>
#include <QMap>

AlbumListModel::AlbumListModel(QObject *parent) : QAbstractTableModel(parent)
{
    updateData(MediaHelper::Newest, 20, 0);
}

AlbumListModel::~AlbumListModel()
{

}

int AlbumListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_albumList.size();
}

int AlbumListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant AlbumListModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_albumList.size() || index.row() < 0)
        return QVariant();

    const Album &album = m_albumList.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        if (index.column() == 0)
            return  index.row() + 1;
        else if (index.column() == 1)
            return  QString("<style>a{color: #CCCCCC; text-decoration:none;}p{margin: 4px 0px;}</style><p>%1</p> <p><a href='search://artist=%2'>%3</a></p>").arg(album.name(), album.artistId(), album.artist());
        else if (index.column() == 2)
            return album.songCount();
        else if (index.column() == 3)
            return album.duration();
        break;
    case Qt::UserRole:
        return album.id();
        break;
    case Qt::TextAlignmentRole:
        return Qt::AlignVCenter ;
        break;
    case Qt::DecorationRole:
        if (index.column() == 1)
            return MediaHelper::getAlbumIcon(album.id());
        break;
    default:
        break;
    }
    return QVariant();
}

QVariant AlbumListModel::headerData(int section, Qt::Orientation orientation, int role) const
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
            return tr("Tracks");
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

Qt::ItemFlags AlbumListModel::flags(const QModelIndex &index) const
{
    return QAbstractTableModel::flags(index);
}

bool AlbumListModel::removeRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row)
        m_albumList.removeAt(position);

    endRemoveRows();
    return true;
}

bool AlbumListModel::clear()
{
    if (m_albumList.size() == 0)
        return true;

    beginRemoveRows(QModelIndex(), 0, rowCount(QModelIndex()));

    m_albumList.clear();

    endRemoveRows();
    return true;
}

void AlbumListModel::updateData(MediaHelper::AlbumType type, int size, int offset)
{
    beginResetModel();
    m_albumList.clear();
    m_albumList = MediaHelper::Instance()->getAlbumList(type, size, offset);
    endResetModel();
}

#include "songtablemodel.h"

#include <QFont>
#include <algorithm>

SongTableModel::SongTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int SongTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_songs.size();
}

int SongTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : ColCount;
}

Qt::ItemFlags SongTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsDropEnabled;

    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (m_dragEnabled)
        f |= Qt::ItemIsDragEnabled;
    return f;
}

Qt::DropActions SongTableModel::supportedDropActions() const
{
    return m_dragEnabled ? Qt::MoveAction : Qt::IgnoreAction;
}

QVariant SongTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_songs.size())
        return QVariant();

    const Song &song = m_songs.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case ColTrack:    return song.discNumber() > 1
                                  ? QStringLiteral("%1-%2").arg(song.discNumber()).arg(song.track())
                                  : QString::number(song.track());
        case ColFavorite: return song.isFavorite() ? QStringLiteral("♥") : QString();
        case ColTitle:    return song.title();
        case ColArtist:   return song.artist();
        case ColAlbum:    return song.album();
        case ColDuration: return song.durationText();
        default:          return QVariant();
        }

    case Qt::TextAlignmentRole:
        if (index.column() == ColDuration || index.column() == ColTrack
            || index.column() == ColFavorite) {
            return static_cast<int>(Qt::AlignCenter);
        }
        return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);

    case Qt::FontRole: {
        if (song.id() == m_playingSongId) {
            QFont f;
            f.setBold(true);
            return f;
        }
        return QVariant();
    }

    case SongRole:
        return QVariant::fromValue(song);
    case CoverArtIdRole:
        return song.coverArtId();
    case IsPlayingRole:
        return song.id() == m_playingSongId;

    default:
        return QVariant();
    }
}

QVariant SongTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    switch (section) {
    case ColTrack:    return QStringLiteral("#");
    case ColFavorite: return QStringLiteral("♥");
    case ColTitle:    return QStringLiteral("标题");
    case ColArtist:   return QStringLiteral("艺术家");
    case ColAlbum:    return QStringLiteral("专辑");
    case ColDuration: return QStringLiteral("时长");
    default:          return QVariant();
    }
}

void SongTableModel::sort(int column, Qt::SortOrder order)
{
    auto comparator = [column, order](const Song &a, const Song &b) {
        bool less = false;
        switch (column) {
        case ColTrack:    less = (a.discNumber() * 1000 + a.track()) < (b.discNumber() * 1000 + b.track()); break;
        case ColFavorite: less = a.isFavorite() && !b.isFavorite(); break;
        case ColTitle:    less = a.title().compare(b.title(), Qt::CaseInsensitive) < 0; break;
        case ColArtist:   less = a.artist().compare(b.artist(), Qt::CaseInsensitive) < 0; break;
        case ColAlbum:    less = a.album().compare(b.album(), Qt::CaseInsensitive) < 0; break;
        case ColDuration: less = a.duration() < b.duration(); break;
        default:          less = false;
        }
        return order == Qt::AscendingOrder ? less : !less;
    };

    beginResetModel();
    std::sort(m_songs.begin(), m_songs.end(), comparator);
    endResetModel();
}

bool SongTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || row < 0 || row + count > m_songs.size())
        return false;
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    m_songs.remove(row, count);
    endRemoveRows();
    Q_EMIT songsChanged();
    return true;
}

bool SongTableModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                              const QModelIndex &destinationParent, int destinationChild)
{
    if (sourceParent.isValid() || destinationParent.isValid())
        return false;
    if (sourceRow < 0 || sourceRow + count > m_songs.size())
        return false;
    if (destinationChild < 0 || destinationChild > m_songs.size())
        return false;

    if (!beginMoveRows(QModelIndex(), sourceRow, sourceRow + count - 1, QModelIndex(),
                       destinationChild))
        return false;

    QVector<Song> moved;
    for (int i = 0; i < count; ++i)
        moved.append(m_songs.takeAt(sourceRow));

    int insertAt = destinationChild;
    if (insertAt > sourceRow)
        insertAt -= count;

    for (int i = 0; i < moved.size(); ++i)
        m_songs.insert(insertAt + i, moved.at(i));

    endMoveRows();
    Q_EMIT songsChanged();
    return true;
}

void SongTableModel::setSongs(const QVector<Song> &songs)
{
    beginResetModel();
    m_songs = songs;
    endResetModel();
    Q_EMIT songsChanged();
}

void SongTableModel::appendSongs(const QVector<Song> &songs)
{
    if (songs.isEmpty())
        return;
    beginInsertRows(QModelIndex(), m_songs.size(), m_songs.size() + songs.size() - 1);
    m_songs.append(songs);
    endInsertRows();
    Q_EMIT songsChanged();
}

void SongTableModel::clear()
{
    beginResetModel();
    m_songs.clear();
    endResetModel();
    Q_EMIT songsChanged();
}

Song SongTableModel::songAt(int row) const
{
    if (row < 0 || row >= m_songs.size())
        return Song();
    return m_songs.at(row);
}

int SongTableModel::indexOf(const QString &songId) const
{
    for (int i = 0; i < m_songs.size(); ++i) {
        if (m_songs.at(i).id() == songId)
            return i;
    }
    return -1;
}

void SongTableModel::moveItem(int from, int to)
{
    if (from == to)
        return;
    moveRows(QModelIndex(), from, 1, QModelIndex(), to > from ? to + 1 : to);
}

void SongTableModel::setPlayingSongId(const QString &songId)
{
    if (m_playingSongId == songId)
        return;
    const int oldRow = indexOf(m_playingSongId);
    m_playingSongId = songId;
    if (oldRow >= 0)
        Q_EMIT dataChanged(index(oldRow, 0), index(oldRow, ColCount - 1));
    const int newRow = indexOf(songId);
    if (newRow >= 0)
        Q_EMIT dataChanged(index(newRow, 0), index(newRow, ColCount - 1));
}

void SongTableModel::setFavorite(const QString &songId, bool favorite)
{
    const int row = indexOf(songId);
    if (row < 0)
        return;
    m_songs[row].setFavorite(favorite);
    Q_EMIT dataChanged(index(row, 0), index(row, ColCount - 1));
}

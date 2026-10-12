#include "core/playbackqueue.h"

#include <algorithm>
#include <QRandomGenerator>
#include <QFont>
#include <QColor>

PlaybackQueue::PlaybackQueue(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int PlaybackQueue::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_songs.size();
}

int PlaybackQueue::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : ColCount;
}

Qt::ItemFlags PlaybackQueue::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsDropEnabled;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
}

Qt::DropActions PlaybackQueue::supportedDropActions() const
{
    return Qt::MoveAction;
}

QVariant PlaybackQueue::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_songs.size())
        return QVariant();

    const Song &song = m_songs.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case ColCover:    return QVariant();
        case ColTitle:    return song.title();
        case ColAlbum:    return song.album();
        case ColArtist:   return song.artist();
        case ColDuration: return song.durationText();
        default:          return QVariant();
        }
    case Qt::TextAlignmentRole:
        if (index.column() == ColDuration)
            return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
        return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
    case SongRole:
        return QVariant::fromValue(song);
    case CoverArtIdRole:
        return song.coverArtId();
    case IsCurrentRole:
        return index.row() == m_currentIndex;
    case Qt::FontRole: {
        if (index.row() == m_currentIndex) {
            QFont f;
            f.setBold(true);
            return f;
        }
        return QVariant();
    }
    default:
        return QVariant();
    }
}

QVariant PlaybackQueue::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();
    if (role != Qt::DisplayRole)
        return QVariant();

    switch (section) {
    case ColCover:    return QStringLiteral("封面");
    case ColTitle:    return QStringLiteral("标题");
    case ColAlbum:    return QStringLiteral("专辑");
    case ColArtist:   return QStringLiteral("艺术家");
    case ColDuration: return QStringLiteral("时长");
    default:          return QVariant();
    }
}

bool PlaybackQueue::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || row < 0 || row + count > m_songs.size())
        return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    m_songs.remove(row, count);
    endRemoveRows();

    if (m_currentIndex >= m_songs.size())
        m_currentIndex = m_songs.size() - 1;
    else if (m_currentIndex > row && m_currentIndex > 0)
        m_currentIndex -= count;

    regenerateShuffleOrder();
    Q_EMIT queueChanged();
    return true;
}

bool PlaybackQueue::moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
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

    const int oldCurrent = m_currentIndex;
    for (int i = 0; i < moved.size(); ++i)
        m_songs.insert(insertAt + i, moved.at(i));

    // 维持当前播放指向同一首歌
    if (oldCurrent >= sourceRow && oldCurrent < sourceRow + count)
        m_currentIndex = insertAt + (oldCurrent - sourceRow);
    else if (oldCurrent > sourceRow && oldCurrent < insertAt)
        m_currentIndex = oldCurrent - count;
    else if (oldCurrent >= insertAt && oldCurrent < sourceRow)
        m_currentIndex = oldCurrent + count;

    endMoveRows();
    regenerateShuffleOrder();
    Q_EMIT queueChanged();
    return true;
}

void PlaybackQueue::append(const QVector<Song> &songs)
{
    if (songs.isEmpty())
        return;
    beginInsertRows(QModelIndex(), m_songs.size(), m_songs.size() + songs.size() - 1);
    m_songs.append(songs);
    endInsertRows();
    regenerateShuffleOrder();
    Q_EMIT queueChanged();
}

void PlaybackQueue::append(const Song &song)
{
    append(QVector<Song>{song});
}

void PlaybackQueue::insertNext(const QVector<Song> &songs)
{
    if (songs.isEmpty())
        return;
    const int at = m_currentIndex >= 0 ? m_currentIndex + 1 : m_songs.size();
    beginInsertRows(QModelIndex(), at, at + songs.size() - 1);
    for (int i = 0; i < songs.size(); ++i)
        m_songs.insert(at + i, songs.at(i));
    endInsertRows();
    regenerateShuffleOrder();
    Q_EMIT queueChanged();
}

void PlaybackQueue::insertNext(const Song &song)
{
    insertNext(QVector<Song>{song});
}

void PlaybackQueue::removeAt(int row)
{
    removeRows(row, 1);
}

void PlaybackQueue::clear()
{
    beginResetModel();
    m_songs.clear();
    m_currentIndex = -1;
    m_shuffleOrder.clear();
    m_shufflePos = 0;
    endResetModel();
    Q_EMIT queueChanged();
}

void PlaybackQueue::moveItem(int from, int to)
{
    if (from == to)
        return;
    moveRows(QModelIndex(), from, 1, QModelIndex(), to > from ? to + 1 : to);
}

Song PlaybackQueue::songAt(int row) const
{
    if (row < 0 || row >= m_songs.size())
        return Song();
    return m_songs.at(row);
}

Song PlaybackQueue::currentSong() const
{
    return songAt(m_currentIndex);
}

int PlaybackQueue::indexOfSong(const QString &songId) const
{
    for (int i = 0; i < m_songs.size(); ++i) {
        if (m_songs.at(i).id() == songId)
            return i;
    }
    return -1;
}

void PlaybackQueue::setCurrentIndex(int index)
{
    if (index == m_currentIndex)
        return;
    const int old = m_currentIndex;
    m_currentIndex = index;

    if (old >= 0 && old < m_songs.size())
        Q_EMIT dataChanged(this->index(old, 0), this->index(old, ColCount - 1));
    if (index >= 0 && index < m_songs.size())
        Q_EMIT dataChanged(this->index(index, 0), this->index(index, ColCount - 1));

    // 同步随机序列游标
    const int pos = m_shuffleOrder.indexOf(index);
    if (pos >= 0)
        m_shufflePos = pos;

    Q_EMIT currentIndexChanged(index);
}

void PlaybackQueue::setPlayMode(PlayMode mode)
{
    if (m_playMode == mode)
        return;
    m_playMode = mode;
    regenerateShuffleOrder();
    Q_EMIT playModeChanged(mode);
}

PlaybackQueue::PlayMode PlaybackQueue::nextPlayMode()
{
    switch (m_playMode) {
    case PlayMode::Sequential: setPlayMode(PlayMode::RepeatAll); break;
    case PlayMode::RepeatAll:  setPlayMode(PlayMode::RepeatOne); break;
    case PlayMode::RepeatOne:  setPlayMode(PlayMode::Shuffle);   break;
    case PlayMode::Shuffle:    setPlayMode(PlayMode::Sequential); break;
    }
    return m_playMode;
}

void PlaybackQueue::regenerateShuffleOrder()
{
    m_shuffleOrder.clear();
    m_shuffleOrder.reserve(m_songs.size());
    for (int i = 0; i < m_songs.size(); ++i)
        m_shuffleOrder.append(i);

    // Fisher-Yates
    for (int i = m_shuffleOrder.size() - 1; i > 0; --i) {
        const int j = QRandomGenerator::global()->bounded(i + 1);
        std::swap(m_shuffleOrder[i], m_shuffleOrder[j]);
    }
    m_shufflePos = m_shuffleOrder.indexOf(m_currentIndex);
    if (m_shufflePos < 0)
        m_shufflePos = 0;
}

int PlaybackQueue::nextIndex(bool autoAdvance) const
{
    if (m_songs.isEmpty())
        return -1;

    switch (m_playMode) {
    case PlayMode::RepeatOne:
        if (autoAdvance)
            return m_currentIndex < 0 ? 0 : m_currentIndex;
        return m_currentIndex;

    case PlayMode::Sequential: {
        const int next = m_currentIndex + 1;
        return next < m_songs.size() ? next : -1;
    }

    case PlayMode::RepeatAll: {
        const int next = m_currentIndex + 1;
        return next < m_songs.size() ? next : 0;
    }

    case PlayMode::Shuffle: {
        if (m_songs.size() == 1)
            return 0;
        int candidate = m_currentIndex;
        int guard = 0;
        while (candidate == m_currentIndex && guard++ < 32)
            candidate = m_shuffleOrder.value((m_shufflePos + 1 + guard) % m_shuffleOrder.size(),
                                             QRandomGenerator::global()->bounded(m_songs.size()));
        return candidate;
    }
    }
    return -1;
}

int PlaybackQueue::previousIndex() const
{
    if (m_songs.isEmpty())
        return -1;

    switch (m_playMode) {
    case PlayMode::RepeatOne:
        return m_currentIndex < 0 ? 0 : m_currentIndex;
    case PlayMode::Shuffle:
        return QRandomGenerator::global()->bounded(m_songs.size());
    case PlayMode::Sequential:
    case PlayMode::RepeatAll: {
        const int prev = m_currentIndex - 1;
        if (prev >= 0)
            return prev;
        return m_playMode == PlayMode::RepeatAll ? m_songs.size() - 1 : -1;
    }
    }
    return -1;
}

QString PlaybackQueue::playModeText(PlayMode mode)
{
    switch (mode) {
    case PlayMode::Sequential: return QStringLiteral("顺序播放");
    case PlayMode::RepeatAll:  return QStringLiteral("列表循环");
    case PlayMode::RepeatOne:  return QStringLiteral("单曲循环");
    case PlayMode::Shuffle:    return QStringLiteral("随机播放");
    }
    return QString();
}

QString PlaybackQueue::playModeIconName(PlayMode mode)
{
    switch (mode) {
    case PlayMode::Sequential: return QStringLiteral("mode_sequential");
    case PlayMode::RepeatAll:  return QStringLiteral("mode_repeat_all");
    case PlayMode::RepeatOne:  return QStringLiteral("mode_repeat_one");
    case PlayMode::Shuffle:    return QStringLiteral("mode_shuffle");
    }
    return QString();
}

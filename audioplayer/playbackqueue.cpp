#include "playbackqueue.h"

#ifndef TEST
#include "../common/mediahelper.h"
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5,10,0))
#include <QRandomGenerator>
#else
#include <QTime>
#endif

PlaybackQueue::PlaybackQueue(QObject *parent)
    : QAbstractTableModel{parent}
    ,m_playbackMode(PlaybackQueue::Sequential)
    ,m_currentIndex(-1)
{

}

PlaybackQueue::~PlaybackQueue()
{
}

int PlaybackQueue::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_trackList.size();
}

int PlaybackQueue::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant PlaybackQueue::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_trackList.size() || index.row() < 0)
        return QVariant();

    const auto &song = m_trackList.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
        if (index.column() == 0){
            return  QString("<span style='color: %2;'>%1</span>").arg( song.track(), index.row() == m_currentIndex ? "#00BB9E" : "#dcdcdc");
        }
        else if (index.column() == 1){
            return  QString("<span style='color: %3;'>%1</span><br/> <a style='color: %3;' href='search://artist=%2'>%2</a>")
                .arg(song.name(), song.artist(), index.row() == m_currentIndex ? "#00BB9E" : "#dcdcdc");
        }
        else if (index.column() == 2)
            return  QString("<span style='color: %2;'>%1</span>").arg( song.duration() ).arg(index.row() == m_currentIndex ? "#00BB9E" : "#dcdcdc");
        else if (index.column() == 3)
            return  QString("<span style='color: %2;'>%1</span>").arg( song.bitRate()).arg(index.row() == m_currentIndex ? "#00BB9E" : "#dcdcdc");
        break;
    case Qt::UserRole:
        return song.id();
        break;
    case Qt::TextAlignmentRole:
        return Qt::AlignVCenter ;
        break;

#ifndef TEST
    case Qt::DecorationRole:
        if (index.column() == 1)
            return MediaHelper::getAlbumIcon(song.albumId());
        break;
#endif
    default:
        break;
    }
    return QVariant();
}

QVariant PlaybackQueue::headerData(int section, Qt::Orientation orientation, int role) const
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
            return tr("Duration");
            break;
        case 3:
            return tr("Bitrate");
            break;

        default:
            return QVariant();
        }
    }
    return QVariant();
}

bool PlaybackQueue::removeRows(int position, int rows, const QModelIndex &index)
{
    beginRemoveRows(index, position, position + rows - 1);

    for (int row = 0; row < rows; ++row)
        m_trackList.removeAt(position);

    endRemoveRows();
    return true;
}

bool PlaybackQueue::insertRows(int position, int rows, const QModelIndex &index)
{
    beginInsertRows(index, position, position + rows - 1);

    for (int row = 0; row < rows; ++row){
        Track trk;
        m_trackList.insert(position, trk);
    }
    endInsertRows();
    return true;
}




PlaybackQueue::PlaybackMode PlaybackQueue::playbackMode() const
{
    return this->m_playbackMode;
}

void PlaybackQueue::setPlaybackMode(PlaybackQueue::PlaybackMode mode)
{
    this->m_playbackMode = mode;
    emit playbackModeChanged(mode);
}

int PlaybackQueue::currentIndex() const
{
    return this->m_currentIndex;
}

const Track &PlaybackQueue::currentTrack() const
{
    return this->m_trackList.at(m_currentIndex);
}

int PlaybackQueue::nextIndex(int steps) const
{
    int next = -1;

    switch (m_playbackMode) {
    case Random:
        next = randomIndex();
        break;
    case Loop:
        next = m_currentIndex + steps;

        if ( next > this->m_trackList.size() - 1 )
            next = next - this->m_trackList.size();

        break;
    default:
        if (m_currentIndex + steps < this->m_trackList.size() )
            next = m_currentIndex + steps;
        break;
    }

    return next;
}

int PlaybackQueue::previousIndex(int steps) const
{
    int prev = -1;

    switch (m_playbackMode) {
    case Random:
        prev= randomIndex();
        break;
    case Loop:
        prev = m_currentIndex - steps;

        if ( prev < 0 )
            prev = prev + this->m_trackList.size();

        break;
    default:
        if ( m_currentIndex - steps > -1 )
            prev = m_currentIndex - steps;
        break;
    }
    return prev;
}

const Track &PlaybackQueue::track(int index) const
{
    return this->m_trackList.at(index);
}

int PlaybackQueue::trackCount() const
{
    return this->m_trackList.size();
}

bool PlaybackQueue::isEmpty() const
{
    return this->m_trackList.isEmpty();
}

bool PlaybackQueue::addTrack(const Track &trk)
{
    //TODO: if audio is exist in QList, then return false;
    int count = m_trackList.size();
    beginInsertRows(QModelIndex(), m_trackList.size(), m_trackList.size() );
    this->m_trackList << trk;
    endInsertRows();
    emit trackInserted(count, count );

    emit trackCountChanged(trackCount());

    if( m_currentIndex == -1 ){
        setCurrentIndex(0);
    }
    return true;
}

bool PlaybackQueue::addTracks(const QList<Track> &items)
{
    //TODO: if audio is exist in QList, then return false;

    int count = m_trackList.size();
    beginInsertRows(QModelIndex(), count, count + items.size() -1 );

    this->m_trackList << items;

    endInsertRows();
    emit trackInserted(count, count + items.size() -1 );

    emit trackCountChanged(trackCount());

    if( m_currentIndex == -1 ){
        setCurrentIndex(0);
    }
    return true;
}

bool PlaybackQueue::insertTrack(int index, Track trk)
{
    beginInsertRows(QModelIndex(), index, index );
    this->m_trackList << trk;
    endInsertRows();
    emit trackInserted(index, index );

    emit trackCountChanged(trackCount());
    return true;
}

bool PlaybackQueue::insertTracks(int index, const QList<Track> &items)
{
    beginInsertRows(QModelIndex(), index, index + items.size() -1 );

    this->m_trackList << items;

    endInsertRows();
    emit trackInserted(index, index + items.size() -1);

    emit trackCountChanged(trackCount());

    return true;

}

bool PlaybackQueue::removeTrack(int pos)
{
    this->removeRows(pos, 1);
    emit trackRemoved(pos, pos);

    emit trackCountChanged(trackCount());
    return true;
}

bool PlaybackQueue::removeTracks(int start, int end)
{
    this->removeRows(start, end - start + 1);
    emit trackRemoved(start, end);

    emit trackCountChanged(trackCount());
    return true;
}

bool PlaybackQueue::clear()
{
    beginResetModel();
    this->m_trackList.clear();
    beginResetModel();
    m_currentIndex = -1;
    return true;
}

bool PlaybackQueue::moveTrack(int from, int to)
{
    beginMoveRows(QModelIndex(), from, from, QModelIndex(), to);
    this->m_trackList.move(from, to);
    endMoveRows();
    return true;
}

void PlaybackQueue::shuffle()
{

}

void PlaybackQueue::loadQueue()
{
    emit loaded();
}

void PlaybackQueue::saveQueue()
{

}

PlaybackQueue::Error PlaybackQueue::error() const
{
    return this->m_error;
}

QString PlaybackQueue::errorString() const
{
    return this->m_errorString;
}

void PlaybackQueue::next()
{
    int idx = this->nextIndex();
    if ( -1 < idx )
        setCurrentIndex(idx) ;
}

void PlaybackQueue::previous()
{
    int idx = this->previousIndex();
    if ( -1 < idx )
        setCurrentIndex(idx) ;
}

void PlaybackQueue::setCurrentIndex(int playqueuePosition)
{
    if (playqueuePosition >= 0 && playqueuePosition < this->m_trackList.size() ){
        this->m_currentIndex = playqueuePosition;
        m_currentTrack = this->m_trackList.at(m_currentIndex);
        emit currentIndexChanged(m_currentIndex);
        emit currentTrackChanged(m_currentTrack);
        emit layoutChanged();
    }
}

int PlaybackQueue::randomIndex() const
{
    int ret = -1;
#if (QT_VERSION >= QT_VERSION_CHECK(5,10,0))
    ret = QRandomGenerator::global()->bounded(this->m_trackList.size());
#else
    qsrand(QTime::currentTime().msec());
    ret = qrand() % this->m_trackList.size();
#endif
    return ret;
}



#ifndef PLAYBACKQUEUE_H
#define PLAYBACKQUEUE_H

#include "track.h"

#include <QAbstractTableModel>


class PlaybackQueue : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(PlaybackQueue::PlaybackMode playbackMode READ playbackMode WRITE setPlaybackMode NOTIFY playbackModeChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)

public:
    enum PlaybackMode {
        //CurrentItemOnce,
        CurrentItemInLoop,
        Sequential,
        Loop,
        Random
    };
    enum Error { NoError, FormatError, FormatNotSupportedError, NetworkError, AccessDeniedError };

    explicit PlaybackQueue(QObject *parent = nullptr);
    ~PlaybackQueue();

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    //Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;
    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;

    PlaybackMode playbackMode() const;
    void setPlaybackMode(PlaybackMode mode);

    int currentIndex() const;
    const QString &currentTrackId() const;
    const Track &currentTrack() const;

    int nextIndex(int steps = 1) const;
    int previousIndex(int steps = 1) const;

    const Track &track(int index) const;

    int trackCount() const;
    bool isEmpty() const;
    //bool isReadOnly() const;

    bool addTrack(const Track &trk);
    bool addTracks(const QList<Track> &items);
    bool insertTrack(int index, Track trk);
    bool insertTracks(int index, const QList<Track> &items);
    bool removeTrack(int pos);
    bool removeTracks(int start, int end);
    bool clear();

    bool moveTrack(int from, int to);

    void loadQueue();

    void saveQueue();

    //void load(const QUrl &location, const char *format = nullptr);

    //bool save(const QUrl &location, const char *format = nullptr);

    Error error() const;
    QString errorString() const;

public slots:

    void next();

    void previous();

    /* Activate media content from playlist at position playlistPosition. */
    void setCurrentIndex(int playqueuePosition);

    /* Shuffle items in the playlist. */
    void shuffle();

signals:

    /* Signal emitted when playlist position changed to position. */
    void currentIndexChanged(int position);

    /* Signal emitted when current audio changes to content. */
    void currentTrackChanged(const Track &trk);

    /*Signal emitted if failed to load playlist. */
    void loadFailed();

    /* Signal emitted when playlist finished loading. */
    void loaded();

    /* This signal is emitted after audio has been changed
     * in the playlist between start and end positions inclusive. */
    void trackChanged(int start, int end);

    /* This signal is emitted after audio has been inserted
     * into the playlist. The new items are those between
     * start and end inclusive. */
    void trackInserted(int start, int end);

    /* This signal is emitted after audio has been removed
     * from the playlist. The removed items are those between
     * start and end inclusive. */
    void trackRemoved(int start, int end);

    /* Signal emitted when playback mode changed to mode. */
    void playbackModeChanged(PlaybackMode mode);

    void trackCountChanged(int count);

private:
    int randomIndex() const;

    PlaybackMode m_playbackMode;
    Error m_error;
    QString m_errorString;
    int m_currentIndex;
    Track m_currentTrack;
    TrackList m_trackList;
};

Q_DECLARE_METATYPE(PlaybackQueue::PlaybackMode)
Q_DECLARE_METATYPE(PlaybackQueue::Error)

#endif // PLAYBACKQUEUE_H

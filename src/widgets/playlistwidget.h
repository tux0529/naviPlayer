#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVector>
#include "../models/song.h"

class PlaylistWidget : public QWidget
{
    Q_OBJECT

public:
    enum PlayMode {
        Sequential,
        RepeatOne,
        RepeatAll,
        Shuffle
    };

    explicit PlaylistWidget(QWidget *parent = nullptr);

    void setSongs(const QVector<Song> &songs);
    void addSong(const Song &song);
    void clear();

    int currentIndex() const;
    void setCurrentIndex(int index);
    void setPlayMode(PlayMode mode);

signals:
    void songSelected(int index);
    void playSongAtIndex(int index);
    void removeSongs(const QVector<int> &indices);
    void clearPlaylist();
    void playModeChanged(PlayMode mode);

private slots:
    void onItemDoubleClicked(QListWidgetItem *item);
    void onRemoveButtonClicked();
    void onClearButtonClicked();
    void onPlayModeButtonClicked();

private:
    void setupUI();
    void updatePlayModeButton();
    void updateItemStyles();
    void restoreSelection();

    QListWidget *m_playlistList;
    QVector<Song> m_songs;
    QPushButton *m_removeButton;
    QPushButton *m_clearButton;
    QPushButton *m_playModeButton;
    QLabel *m_titleLabel;
    PlayMode m_playMode;
    int m_playingIndex;
    int m_selectedIndex;
    int m_pendingSelectedIndex;
};

#endif // PLAYLISTWIDGET_H

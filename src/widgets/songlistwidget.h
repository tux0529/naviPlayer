#ifndef SONGLISTWIDGET_H
#define SONGLISTWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include "../models/song.h"
#include <QVector>

class SongListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SongListWidget(QWidget *parent = nullptr);

    void setSongs(const QVector<Song> &songs);
    void clear();
    QVector<Song> songs() const { return m_songs; }
    int currentRow() const;
    Song currentSong() const;

signals:
    void songSelected(const Song &song);
    void songFavoriteClicked(const Song &song);

private slots:
    void onCellDoubleClicked(int row, int column);
    void onCellClicked(int row, int column);

private:
    void setupUI();

    QTableWidget *m_tableWidget;
    QVector<Song> m_songs;
};

#endif // SONGLISTWIDGET_H

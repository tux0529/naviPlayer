#include "songlistwidget.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QLabel>

SongListWidget::SongListWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void SongListWidget::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(6);
    m_tableWidget->setHorizontalHeaderLabels({"#", "♥", "歌曲", "艺术家", "专辑", "时长"});
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->verticalHeader()->setVisible(false);
    m_tableWidget->setShowGrid(false);
    m_tableWidget->setAlternatingRowColors(true);

    m_tableWidget->setStyleSheet(R"(
        QTableWidget {
            background-color: #1e1e1e;
            color: #ffffff;
            border: none;
            gridline-color: #2d2d2d;
            selection-background-color: #3d3d3d;
            selection-color: #ffffff;
            font-size: 13px;
        }
        QTableWidget::item {
            padding: 8px;
            border: none;
        }
        QTableWidget::item:selected {
            background-color: #3d3d3d;
        }
        QTableWidget::item:hover {
            background-color: #353535;
        }
        QHeaderView::section {
            background-color: #2b2b2b;
            color: #999999;
            padding: 8px;
            border: none;
            border-bottom: 1px solid #3d3d3d;
            font-size: 12px;
            font-weight: bold;
        }
        QTableWidget QTableCornerButton::section {
            background-color: #2b2b2b;
            border: none;
        }
    )");

    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->setColumnWidth(0, 50);
    m_tableWidget->setColumnWidth(1, 40);
    m_tableWidget->setColumnWidth(2, 250);
    m_tableWidget->setColumnWidth(3, 150);
    m_tableWidget->setColumnWidth(4, 150);
    m_tableWidget->setColumnWidth(5, 80);

    layout->addWidget(m_tableWidget);

    connect(m_tableWidget, &QTableWidget::cellClicked,
            this, &SongListWidget::onCellClicked);
    connect(m_tableWidget, &QTableWidget::cellDoubleClicked,
            this, &SongListWidget::onCellDoubleClicked);
}

void SongListWidget::setSongs(const QVector<Song> &songs)
{
    m_songs = songs;
    m_tableWidget->setRowCount(m_songs.size());

    for (int i = 0; i < m_songs.size(); ++i) {
        const Song &song = m_songs[i];

        QTableWidgetItem *trackItem = new QTableWidgetItem(QString::number(song.track()));
        trackItem->setTextAlignment(Qt::AlignCenter);
        m_tableWidget->setItem(i, 0, trackItem);

        QTableWidgetItem *favoriteItem = new QTableWidgetItem(song.isFavorite() ? "♥" : "♡");
        favoriteItem->setTextAlignment(Qt::AlignCenter);
        m_tableWidget->setItem(i, 1, favoriteItem);

        m_tableWidget->setItem(i, 2, new QTableWidgetItem(song.title()));
        m_tableWidget->setItem(i, 3, new QTableWidgetItem(song.artist()));
        m_tableWidget->setItem(i, 4, new QTableWidgetItem(song.album()));

        qint64 seconds = song.duration() / 1000;
        qint64 minutes = seconds / 60;
        seconds = seconds % 60;
        QString duration = QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
        m_tableWidget->setItem(i, 5, new QTableWidgetItem(duration));
    }

    if (!m_songs.isEmpty()) {
        m_tableWidget->setCurrentCell(0, 0);
    } else {
        m_tableWidget->clearSelection();
    }
}

void SongListWidget::clear()
{
    m_songs.clear();
    m_tableWidget->setRowCount(0);
}

int SongListWidget::currentRow() const
{
    return m_tableWidget->currentRow();
}

Song SongListWidget::currentSong() const
{
    int row = currentRow();
    if (row >= 0 && row < m_songs.size()) {
        return m_songs[row];
    }
    return Song();
}

void SongListWidget::onCellDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    if (row >= 0 && row < m_songs.size()) {
        emit songSelected(m_songs[row]);
    }
}

void SongListWidget::onCellClicked(int row, int column)
{
    if (column != 1 || row < 0 || row >= m_songs.size()) {
        return;
    }

    Song song = m_songs[row];
    if (song.id().isEmpty()) {
        return;
    }

    const bool newFavorite = !song.isFavorite();
    song.setFavorite(newFavorite);
    m_songs[row] = song;

    QTableWidgetItem *item = m_tableWidget->item(row, column);
    if (item) {
        item->setText(newFavorite ? "♥" : "♡");
    }

    emit songFavoriteClicked(song);
}

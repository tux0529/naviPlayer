#include "playlistwidget.h"
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <algorithm>

PlaylistWidget::PlaylistWidget(QWidget *parent)
    : QWidget(parent)
    , m_playMode(Sequential)
    , m_playingIndex(-1)
    , m_selectedIndex(-1)
    , m_pendingSelectedIndex(-1)
{
    setupUI();
}

void PlaylistWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // 标题
    m_titleLabel = new QLabel("播放队列", this);
    m_titleLabel->setStyleSheet("color: #ffffff; font-size: 24px; font-weight: bold;");
    mainLayout->addWidget(m_titleLabel);

    // 播放列表
    m_playlistList = new QListWidget(this);
    m_playlistList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_playlistList->setStyleSheet(R"(
        QListWidget {
            background-color: #2b2b2b;
            border: none;
            border-radius: 8px;
            padding: 10px;
        }
        QListWidget::item {
            padding: 8px;
            border-radius: 4px;
            margin-bottom: 2px;
        }
        QListWidget::item:selected {
            background-color: #3a80b3;
        }
        QListWidget::item:hover {
            background-color: #3a80b3;
        }
    )");
    connect(m_playlistList, &QListWidget::itemDoubleClicked, this, &PlaylistWidget::onItemDoubleClicked);
    connect(m_playlistList, &QListWidget::currentRowChanged, this, [this](int row) {
        m_selectedIndex = row;
    });
    mainLayout->addWidget(m_playlistList);

    // 按钮区
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    m_playModeButton = new QPushButton("顺序播放", this);
    m_playModeButton->setStyleSheet(R"(
        QPushButton {
            background-color: #3a80b3;
            color: #ffffff;
            border: none;
            padding: 10px 20px;
            border-radius: 6px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #4a90c3;
        }
    )");
    connect(m_playModeButton, &QPushButton::clicked, this, &PlaylistWidget::onPlayModeButtonClicked);

    m_removeButton = new QPushButton("删除选中", this);
    m_removeButton->setStyleSheet(R"(
        QPushButton {
            background-color: #e74c3c;
            color: #ffffff;
            border: none;
            padding: 10px 20px;
            border-radius: 6px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #c0392b;
        }
    )");
    connect(m_removeButton, &QPushButton::clicked, this, &PlaylistWidget::onRemoveButtonClicked);

    m_clearButton = new QPushButton("清空队列", this);
    m_clearButton->setStyleSheet(R"(
        QPushButton {
            background-color: #95a5a6;
            color: #ffffff;
            border: none;
            padding: 10px 20px;
            border-radius: 6px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #7f8c8d;
        }
    )");
    connect(m_clearButton, &QPushButton::clicked, this, &PlaylistWidget::onClearButtonClicked);

    buttonLayout->addWidget(m_playModeButton);
    buttonLayout->addWidget(m_removeButton);
    buttonLayout->addWidget(m_clearButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);
}

void PlaylistWidget::setSongs(const QVector<Song> &songs)
{
    m_songs = songs;
    m_playlistList->clear();

    for (const Song &song : songs) {
        QString itemText = QString("%1 - %2").arg(song.artist()).arg(song.title());
        QListWidgetItem *item = new QListWidgetItem(itemText);
        m_playlistList->addItem(item);
    }

    restoreSelection();
    updateItemStyles();
}

void PlaylistWidget::addSong(const Song &song)
{
    m_songs.append(song);
    QString itemText = QString("%1 - %2").arg(song.artist()).arg(song.title());
    QListWidgetItem *item = new QListWidgetItem(itemText);
    m_playlistList->addItem(item);
    updateItemStyles();
}

void PlaylistWidget::clear()
{
    m_songs.clear();
    m_playlistList->clear();
    m_playingIndex = -1;
    m_selectedIndex = -1;
    m_pendingSelectedIndex = -1;
}

int PlaylistWidget::currentIndex() const
{
    return m_playlistList->currentRow();
}

void PlaylistWidget::setCurrentIndex(int index)
{
    m_playingIndex = index;
    updateItemStyles();
}

void PlaylistWidget::onItemDoubleClicked(QListWidgetItem *item)
{
    int index = m_playlistList->row(item);
    if (index >= 0 && index < m_songs.size()) {
        emit playSongAtIndex(index);
    }
}

void PlaylistWidget::onRemoveButtonClicked()
{
    QList<QListWidgetItem *> selectedItems = m_playlistList->selectedItems();
    if (selectedItems.isEmpty()) {
        return;
    }

    QVector<int> selectedRows;
    selectedRows.reserve(selectedItems.size());
    for (QListWidgetItem *item : selectedItems) {
        if (!item) {
            continue;
        }
        selectedRows.append(m_playlistList->row(item));
    }

    if (selectedRows.isEmpty()) {
        return;
    }

    std::sort(selectedRows.begin(), selectedRows.end());

    int firstSelectedRow = selectedRows.first();
    int remainingCount = m_playlistList->count() - selectedRows.size();
    if (remainingCount <= 0) {
        m_pendingSelectedIndex = -1;
    } else if (firstSelectedRow > 0) {
        m_pendingSelectedIndex = firstSelectedRow - 1;
    } else {
        m_pendingSelectedIndex = 0;
    }

    emit removeSongs(selectedRows);
}

void PlaylistWidget::onClearButtonClicked()
{
    m_pendingSelectedIndex = -1;
    emit clearPlaylist();
}

void PlaylistWidget::setPlayMode(PlayMode mode)
{
    if (m_playMode != mode) {
        m_playMode = mode;
        updatePlayModeButton();
    }
}

void PlaylistWidget::onPlayModeButtonClicked()
{
    PlayMode newMode;
    switch (m_playMode) {
    case Sequential:
        newMode = RepeatOne;
        break;
    case RepeatOne:
        newMode = RepeatAll;
        break;
    case RepeatAll:
        newMode = Shuffle;
        break;
    case Shuffle:
        newMode = Sequential;
        break;
    }
    m_playMode = newMode;
    updatePlayModeButton();
    emit playModeChanged(m_playMode);
}

void PlaylistWidget::updatePlayModeButton()
{
    QString text;
    switch (m_playMode) {
    case Sequential:
        text = "顺序播放";
        break;
    case RepeatOne:
        text = "单曲循环";
        break;
    case RepeatAll:
        text = "列表循环";
        break;
    case Shuffle:
        text = "随机播放";
        break;
    }
    m_playModeButton->setText(text);
}

void PlaylistWidget::updateItemStyles()
{
    for (int i = 0; i < m_playlistList->count(); ++i) {
        QListWidgetItem *item = m_playlistList->item(i);
        if (!item) {
            continue;
        }

        QFont font = item->font();
        if (i == m_playingIndex) {
            item->setForeground(QColor("#1db954"));
            font.setBold(true);
        } else {
            item->setForeground(QColor("#ffffff"));
            font.setBold(false);
        }
        item->setFont(font);
    }
}

void PlaylistWidget::restoreSelection()
{
    int targetIndex = m_pendingSelectedIndex;
    if (targetIndex < 0) {
        targetIndex = m_selectedIndex;
    }

    if (targetIndex >= 0 && targetIndex < m_playlistList->count()) {
        m_playlistList->setCurrentRow(targetIndex);
        m_selectedIndex = targetIndex;
    } else {
        m_playlistList->clearSelection();
        m_selectedIndex = -1;
    }

    m_pendingSelectedIndex = -1;
}

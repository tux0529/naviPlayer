#include "songslistview.h"

#include "models/songtablemodel.h"
#include "widgets/components/songtableview.h"
#include "core/appcontext.h"
#include "core/appsettings.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <algorithm>

SongsListView::SongsListView(QWidget *parent)
    : PageBase(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 8);
    layout->setSpacing(6);

    m_model = new SongTableModel(this);
    m_view = new SongTableView(this);
    m_view->setSongModel(m_model);
    m_view->horizontalHeader()->setSectionResizeMode(SongTableModel::ColTitle, QHeaderView::Stretch);
    m_view->setColumnWidth(SongTableModel::ColTrack, 48);
    m_view->setColumnWidth(SongTableModel::ColFavorite, 36);
    m_view->setColumnWidth(SongTableModel::ColArtist, 150);
    m_view->setColumnWidth(SongTableModel::ColAlbum, 180);
    m_view->setColumnWidth(SongTableModel::ColDuration, 64);

    auto *pageBar = new QHBoxLayout();
    m_prevButton = new QPushButton(QStringLiteral("上一页"), this);
    m_nextButton = new QPushButton(QStringLiteral("下一页"), this);
    m_pageLabel = new QLabel(QStringLiteral("第 1 / 1 页"), this);
    m_pageLabel->setAlignment(Qt::AlignCenter);
    pageBar->addStretch();
    pageBar->addWidget(m_prevButton);
    pageBar->addWidget(m_pageLabel);
    pageBar->addWidget(m_nextButton);
    pageBar->addStretch();

    layout->addWidget(m_view, 1);
    layout->addLayout(pageBar);

    connect(m_prevButton, &QPushButton::clicked, this, &SongsListView::onPrevPage);
    connect(m_nextButton, &QPushButton::clicked, this, &SongsListView::onNextPage);
    connect(m_model, &SongTableModel::songsChanged, this, &SongsListView::onModelSongsChanged);

    connect(m_view, &SongTableView::songActivated, this, &SongsListView::songActivated);
    connect(m_view, &SongTableView::playNextRequested, this, &SongsListView::playNextRequested);
    connect(m_view, &SongTableView::favoriteToggled, this, &SongsListView::favoriteToggled);
    connect(m_view, &SongTableView::addToPlaylistRequested, this,
            &SongsListView::addToPlaylistRequested);
    connect(m_view, &SongTableView::downloadRequested, this, &SongsListView::downloadRequested);
    connect(m_view, &SongTableView::detailsRequested, this, &SongsListView::detailsRequested);
}

void SongsListView::setPagingEnabled(bool enabled)
{
    m_pagingEnabled = enabled;
    m_page = 0;
    rebuildPage();
}

void SongsListView::setDragReorderEnabled(bool enabled)
{
    m_model->setDragEnabled(enabled);
    m_view->setDragEnabled(enabled);
    m_view->setAcceptDrops(enabled);
    m_view->setDragDropMode(enabled ? QAbstractItemView::InternalMove
                                    : QAbstractItemView::NoDragDrop);
    m_view->setDropIndicatorShown(enabled);
}

void SongsListView::setSortingEnabled(bool enabled)
{
    m_view->setSortingEnabled(enabled);
}

void SongsListView::setSongs(const QVector<Song> &songs)
{
    m_allSongs = songs;
    m_page = 0;
    rebuildPage();
}

QVector<Song> SongsListView::songs() const
{
    return m_allSongs;
}

void SongsListView::appendSongs(const QVector<Song> &songs)
{
    m_allSongs.append(songs);
    rebuildPage();
}

void SongsListView::clear()
{
    m_allSongs.clear();
    m_page = 0;
    m_model->clear();
    updatePageLabel();
}

int SongsListView::songsPerPage() const
{
    AppSettings *settings = AppContext::instance() ? AppContext::instance()->settings() : nullptr;
    return settings ? qMax(1, settings->songsPerPage()) : 100;
}

int SongsListView::pageCount() const
{
    const int perPage = songsPerPage();
    return qMax(1, (m_allSongs.size() + perPage - 1) / perPage);
}

void SongsListView::rebuildPage()
{
    if (!m_pagingEnabled) {
        m_model->setSongs(m_allSongs);
        m_prevButton->setVisible(false);
        m_nextButton->setVisible(false);
        m_pageLabel->setText(QStringLiteral("共 %1 首").arg(m_allSongs.size()));
        return;
    }

    const int perPage = songsPerPage();
    const int start = m_page * perPage;
    const int end = qMin(start + perPage, m_allSongs.size());

    QVector<Song> pageSongs;
    for (int i = start; i < end; ++i)
        pageSongs.append(m_allSongs.at(i));

    m_model->setSongs(pageSongs);
    m_prevButton->setVisible(true);
    m_nextButton->setVisible(true);
    updatePageLabel();
}

void SongsListView::updatePageLabel()
{
    if (!m_pagingEnabled)
        return;
    m_pageLabel->setText(QStringLiteral("第 %1 / %2 页（共 %3 首）")
                             .arg(m_page + 1)
                             .arg(pageCount())
                             .arg(m_allSongs.size()));
    m_prevButton->setEnabled(m_page > 0);
    m_nextButton->setEnabled(m_page + 1 < pageCount());
}

void SongsListView::onPrevPage()
{
    if (m_page <= 0)
        return;
    --m_page;
    rebuildPage();
}

void SongsListView::onNextPage()
{
    if (m_page + 1 >= pageCount())
        return;
    ++m_page;
    rebuildPage();
}

void SongsListView::onModelSongsChanged()
{
    // 拖动排序后同步回全量数据
    if (!m_pagingEnabled) {
        m_allSongs = m_model->songs();
        Q_EMIT orderChanged(m_allSongs);
    }
}

void SongsListView::setPlayingSongId(const QString &songId)
{
    m_model->setPlayingSongId(songId);
}

void SongsListView::setFavorite(const QString &songId, bool favorite)
{
    m_model->setFavorite(songId, favorite);
    for (Song &s : m_allSongs) {
        if (s.id() == songId)
            s.setFavorite(favorite);
    }
}

void SongsListView::refresh()
{
    rebuildPage();
}

void SongsListView::retranslate()
{
    m_prevButton->setText(QStringLiteral("上一页"));
    m_nextButton->setText(QStringLiteral("下一页"));
    updatePageLabel();
}

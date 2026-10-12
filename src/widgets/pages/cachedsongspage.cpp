#include "cachedsongspage.h"

#include "widgets/pages/songslistview.h"
#include "widgets/components/songtableview.h"
#include "core/appcontext.h"
#include "core/cachemanager.h"
#include "core/appsettings.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

CachedSongsPage::CachedSongsPage(QWidget *parent)
    : PageBase(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(8);

    auto *toolbar = new QHBoxLayout();
    m_summaryLabel = new QLabel(this);
    m_playAllButton = new QPushButton(QStringLiteral("播放全部"), this);
    m_playAllButton->setObjectName(QStringLiteral("playAllButton"));
    m_deleteButton = new QPushButton(QStringLiteral("删除选中"), this);
    m_clearButton = new QPushButton(QStringLiteral("清空缓存"), this);
    toolbar->addWidget(m_summaryLabel);
    toolbar->addStretch();
    toolbar->addWidget(m_playAllButton);
    toolbar->addWidget(m_deleteButton);
    toolbar->addWidget(m_clearButton);

    m_songsView = new SongsListView(this);
    m_songsView->setPagingEnabled(false);
    m_songsView->setSortingEnabled(true);

    layout->addLayout(toolbar);
    layout->addWidget(m_songsView, 1);

    connect(m_playAllButton, &QPushButton::clicked, this, &CachedSongsPage::onPlayAll);
    connect(m_deleteButton, &QPushButton::clicked, this, &CachedSongsPage::onDeleteSelected);
    connect(m_clearButton, &QPushButton::clicked, this, &CachedSongsPage::onClearAll);

    connect(m_songsView, &SongsListView::songActivated, this, &CachedSongsPage::songActivated);
    connect(m_songsView, &SongsListView::playNextRequested, this, &CachedSongsPage::playNextRequested);
    connect(m_songsView, &SongsListView::favoriteToggled, this, &CachedSongsPage::favoriteToggled);
    connect(m_songsView, &SongsListView::addToPlaylistRequested, this,
            &CachedSongsPage::addToPlaylistRequested);
    connect(m_songsView, &SongsListView::downloadRequested, this, &CachedSongsPage::downloadRequested);
    connect(m_songsView, &SongsListView::detailsRequested, this, &CachedSongsPage::detailsRequested);
}

void CachedSongsPage::updateSummary(qint64 totalBytes)
{
    AppSettings *settings = AppContext::instance() ? AppContext::instance()->settings() : nullptr;
    const int songs = m_songsView->songs().size();
    const double mb = static_cast<double>(totalBytes) / (1024.0 * 1024.0);
    QString limitText;
    if (settings && settings->maxSongCacheMb() > 0)
        limitText = QStringLiteral(" / 上限 %1 MB").arg(settings->maxSongCacheMb());
    m_summaryLabel->setText(QStringLiteral("共 %1 首，占用 %2 MB%3")
                                .arg(songs)
                                .arg(mb, 0, 'f', 1)
                                .arg(limitText));
    if (m_playAllButton)
        m_playAllButton->setEnabled(songs > 0);
}

void CachedSongsPage::onPlayAll()
{
    const QVector<Song> songs = m_songsView->songs();
    if (songs.isEmpty())
        return;
    Q_EMIT playAllRequested(songs);
}

void CachedSongsPage::onDeleteSelected()
{
    CacheManager *cache = AppContext::instance() ? AppContext::instance()->cache() : nullptr;
    if (!cache)
        return;

    const QModelIndexList selected = m_songsView->view()->selectionModel()->selectedRows();
    if (selected.isEmpty())
        return;

    for (const QModelIndex &index : selected) {
        const Song song = m_songsView->songs().value(index.row());
        if (song.isValid())
            cache->removeCachedSong(song.id());
    }
    refresh();
}

void CachedSongsPage::onClearAll()
{
    if (QMessageBox::question(this, QStringLiteral("清空缓存"),
                              QStringLiteral("确定要清空所有本地缓存歌曲吗？"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }
    CacheManager *cache = AppContext::instance() ? AppContext::instance()->cache() : nullptr;
    if (cache)
        cache->clearCachedSongs();
    refresh();
}

void CachedSongsPage::refresh()
{
    CacheManager *cache = AppContext::instance() ? AppContext::instance()->cache() : nullptr;
    if (!cache)
        return;

    const QVector<Song> songs = cache->cachedSongs();
    m_songsView->setSongs(songs);
    updateSummary(cache->cacheSizeBytes());
}

void CachedSongsPage::retranslate()
{
    m_playAllButton->setText(QStringLiteral("播放全部"));
    m_deleteButton->setText(QStringLiteral("删除选中"));
    m_clearButton->setText(QStringLiteral("清空缓存"));
    refresh();
}

#include "songtableview.h"

#include "models/songtablemodel.h"
#include "core/appcontext.h"
#include "core/databasemanager.h"
#include "core/subsonicapi.h"

#include <QHeaderView>
#include <QMenu>
#include <QMouseEvent>
#include <QAction>

SongTableView::SongTableView(QWidget *parent)
    : QTableView(parent)
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setAlternatingRowColors(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setShowGrid(false);
    setContextMenuPolicy(Qt::CustomContextMenu);
    verticalHeader()->setVisible(false);
    horizontalHeader()->setHighlightSections(false);
    setMouseTracking(true);

    connect(this, &QTableView::customContextMenuRequested, this, &SongTableView::showContextMenu);
}

void SongTableView::setSongModel(SongTableModel *model)
{
    m_model = model;
    QTableView::setModel(model);
}

void SongTableView::setContextMenuEnabled(bool enabled)
{
    m_menuEnabled = enabled;
}

Song SongTableView::selectedSong() const
{
    if (!m_model)
        return Song();
    const QModelIndex index = currentIndex();
    if (!index.isValid())
        return Song();
    return m_model->songAt(index.row());
}

void SongTableView::mouseDoubleClickEvent(QMouseEvent *event)
{
    const QModelIndex index = indexAt(event->pos());
    if (!index.isValid() || !m_model) {
        QTableView::mouseDoubleClickEvent(event);
        return;
    }
    Q_EMIT songActivated(m_model->songAt(index.row()));
    event->accept();
}

void SongTableView::showContextMenu(const QPoint &pos)
{
    if (!m_menuEnabled || !m_model)
        return;

    const QModelIndex index = indexAt(pos);
    if (!index.isValid())
        return;

    const Song song = m_model->songAt(index.row());
    if (!song.isValid())
        return;

    QMenu menu(this);

    QAction *playNow = menu.addAction(QStringLiteral("立即播放"));
    QAction *playNext = menu.addAction(QStringLiteral("下一首播放"));
    menu.addSeparator();
    QAction *favorite = menu.addAction(song.isFavorite() ? QStringLiteral("取消收藏")
                                                         : QStringLiteral("添加到收藏"));
    menu.addSeparator();

    QMenu *playlistMenu = menu.addMenu(QStringLiteral("添加到播放列表"));
    QVector<Playlist> playlists = AppContext::instance()->playlists();
    if (playlists.isEmpty()) {
        playlists = AppContext::instance()->database()->playlists();
    }
    QHash<QAction *, QString> playlistActions;
    for (const Playlist &p : playlists) {
        QAction *action = playlistMenu->addAction(p.name());
        playlistActions.insert(action, p.id());
    }
    if (playlists.isEmpty())
        playlistMenu->setEnabled(false);

    menu.addSeparator();
    QAction *download = menu.addAction(QStringLiteral("下载该歌曲"));
    QAction *details = menu.addAction(QStringLiteral("歌曲详情"));

    QAction *chosen = menu.exec(viewport()->mapToGlobal(pos));
    if (!chosen)
        return;

    if (chosen == playNow) {
        Q_EMIT songActivated(song);
    } else if (chosen == playNext) {
        Q_EMIT playNextRequested(song);
    } else if (chosen == favorite) {
        Q_EMIT favoriteToggled(song, !song.isFavorite());
    } else if (chosen == download) {
        Q_EMIT downloadRequested(song);
    } else if (chosen == details) {
        Q_EMIT detailsRequested(song);
    } else if (playlistActions.contains(chosen)) {
        Q_EMIT addToPlaylistRequested(song, playlistActions.value(chosen));
    }
}

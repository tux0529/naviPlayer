#include "playbackqueuepanel.h"

#include "core/appcontext.h"
#include "core/cachemanager.h"

#include <algorithm>
#include <functional>
#include <QApplication>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QSet>
#include <QShowEvent>
#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>

CoverDelegate::CoverDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QSize CoverDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QSize(option.rect.height(), option.rect.height());
}

QString CoverDelegate::coverIdForIndex(const QModelIndex &index)
{
    const Song song = index.data(PlaybackQueue::SongRole).value<Song>();
    if (!song.isValid())
        return index.data(PlaybackQueue::CoverArtIdRole).toString();

    AppContext *ctx = AppContext::instance();
    return ctx ? ctx->coverArtIdForSong(song) : song.coverArtId();
}

void CoverDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    CacheManager *cache = AppContext::instance() ? AppContext::instance()->cache() : nullptr;
    if (!cache)
        return;

    const QString coverId = coverIdForIndex(index);
    if (coverId.isEmpty())
        return;

    const int size = qMax(24, option.rect.height() - 8);

    QPixmap pm = cache->cachedCover(coverId, size);
    if (pm.isNull())
        return;

    const QRect target(option.rect.center().x() - size / 2,
                       option.rect.center().y() - size / 2, size, size);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->drawPixmap(target, pm.scaled(target.size(), Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation));
    painter->restore();
}

PlaybackQueuePanel::PlaybackQueuePanel(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("playbackQueuePanel"));

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    auto *toolbar = new QHBoxLayout();
    auto *title = new QLabel(QStringLiteral("播放队列"), this);
    QFont titleFont = title->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 2);
    title->setFont(titleFont);

    m_countLabel = new QLabel(QStringLiteral("0 首"), this);

    m_modeButton = new QPushButton(this);
    m_modeButton->setObjectName(QStringLiteral("queueModeButton"));
    m_modeButton->setCursor(Qt::PointingHandCursor);

    m_removeButton = new QPushButton(QStringLiteral("删除"), this);
    m_clearButton = new QPushButton(QStringLiteral("清空"), this);

    toolbar->addWidget(title);
    toolbar->addWidget(m_countLabel);
    toolbar->addStretch();
    toolbar->addWidget(m_modeButton);
    toolbar->addWidget(m_removeButton);
    toolbar->addWidget(m_clearButton);

    m_view = new QTableView(this);
    m_view->setObjectName(QStringLiteral("queueView"));
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->setAlternatingRowColors(true);
    m_view->setShowGrid(false);
    m_view->verticalHeader()->setVisible(false);
    m_view->setDragEnabled(true);
    m_view->setAcceptDrops(true);
    m_view->setDragDropMode(QAbstractItemView::InternalMove);
    m_view->setDropIndicatorShown(true);

    layout->addLayout(toolbar);
    layout->addWidget(m_view, 1);

    connect(m_modeButton, &QPushButton::clicked, this, [this]() {
        if (m_queue)
            m_queue->nextPlayMode();
    });
    connect(m_clearButton, &QPushButton::clicked, this, [this]() {
        if (m_queue)
            m_queue->clear();
    });
    connect(m_removeButton, &QPushButton::clicked, this, &PlaybackQueuePanel::removeSelected);

    // 封面异步到达后重绘队列
    if (CacheManager *cache = AppContext::instance() ? AppContext::instance()->cache() : nullptr) {
        connect(cache, &CacheManager::coverReady, m_view,
                [this](const QString &, int, const QPixmap &) { m_view->viewport()->update(); });
    }
}

void PlaybackQueuePanel::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    requestCovers();
}

void PlaybackQueuePanel::requestCovers()
{
    CacheManager *cache = AppContext::instance() ? AppContext::instance()->cache() : nullptr;
    if (!cache || !m_queue)
        return;

    // 高分屏按设备像素比取样
    const qreal dpr = qApp ? qApp->devicePixelRatio() : 1.0;
    const int fetchSize = qRound(CoverDelegate::coverSize() * qMax<qreal>(1.0, dpr));

    QSet<QString> requested;
    for (int row = 0; row < m_queue->rowCount(); ++row) {
        const QString id = CoverDelegate::coverIdForIndex(m_queue->index(row, PlaybackQueue::ColCover));
        if (id.isEmpty() || requested.contains(id))
            continue;
        requested.insert(id);
        cache->requestCover(id, fetchSize);
    }
}

void PlaybackQueuePanel::setQueue(PlaybackQueue *queue)
{
    m_queue = queue;
    if (!m_queue)
        return;

    m_view->setModel(m_queue);
    m_view->setItemDelegateForColumn(PlaybackQueue::ColCover, new CoverDelegate(m_view));
    m_view->horizontalHeader()->setSectionResizeMode(PlaybackQueue::ColCover, QHeaderView::Fixed);
    m_view->setColumnWidth(PlaybackQueue::ColCover, 56);
    m_view->horizontalHeader()->setSectionResizeMode(PlaybackQueue::ColTitle, QHeaderView::Stretch);
    m_view->setColumnWidth(PlaybackQueue::ColAlbum, 160);
    m_view->setColumnWidth(PlaybackQueue::ColArtist, 120);
    m_view->setColumnWidth(PlaybackQueue::ColDuration, 64);
    m_view->verticalHeader()->setDefaultSectionSize(48);

    connect(m_queue, &PlaybackQueue::queueChanged, this, [this]() {
        m_countLabel->setText(QStringLiteral("%1 首").arg(m_queue ? m_queue->size() : 0));
        requestCovers();
    });
    connect(m_queue, &PlaybackQueue::playModeChanged, this, &PlaybackQueuePanel::updateModeButton);
    connect(m_view, &QTableView::doubleClicked, this, [this](const QModelIndex &index) {
        Q_EMIT playRequested(index.row());
    });

    updateModeButton();
    m_countLabel->setText(QStringLiteral("%1 首").arg(m_queue->size()));
    requestCovers();
}

void PlaybackQueuePanel::updateModeButton()
{
    if (!m_queue)
        return;
    m_modeButton->setText(PlaybackQueue::playModeText(m_queue->playMode()));
    m_modeButton->setToolTip(QStringLiteral("点击切换播放模式"));
}

void PlaybackQueuePanel::removeSelected()
{
    if (!m_queue)
        return;

    QList<int> rows;
    const QModelIndexList selected = m_view->selectionModel()->selectedRows();
    for (const QModelIndex &index : selected)
        rows.append(index.row());

    std::sort(rows.begin(), rows.end(), std::greater<int>());
    for (int row : rows)
        m_queue->removeAt(row);
}

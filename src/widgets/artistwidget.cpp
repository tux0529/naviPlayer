#include "artistwidget.h"

#include "../cachemanager.h"
#include "../subsonicapi.h"

#include <QHeaderView>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QScrollBar>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

namespace {
constexpr int kAvatarSize = 48;
constexpr int kAvatarRequestSize = 64;
constexpr qreal kAvatarRadius = 10.0;
// 只拉取视口上下各预加载一页，避免一上来发 40 个请求把 HTTP 连接池打满
constexpr int kVisiblePreloadMarginRows = 6;
// 防抖：快速滚动时不反复发请求，停下来再批量发
constexpr int kCoverRequestDebounceMs = 120;

QPixmap roundedPixmap(const QPixmap &pixmap, const QSize &targetSize, qreal radius)
{
    if (pixmap.isNull() || !targetSize.isValid()) {
        return QPixmap();
    }

    QPixmap scaled = pixmap.scaled(targetSize,
                                   Qt::KeepAspectRatioByExpanding,
                                   Qt::SmoothTransformation);
    QPixmap rounded(targetSize);
    rounded.fill(Qt::transparent);

    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPainterPath path;
    path.addRoundedRect(QRectF(QPointF(0, 0), QSizeF(targetSize)), radius, radius);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, scaled);

    return rounded;
}

QPixmap defaultAvatarPixmap()
{
    QPixmap pixmap(QSize(kAvatarSize, kAvatarSize));
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPainterPath path;
    path.addRoundedRect(QRectF(QPointF(0, 0), QSizeF(pixmap.size())), kAvatarRadius, kAvatarRadius);
    painter.fillPath(path, QColor("#3a3a3a"));

    QFont font = painter.font();
    font.setPointSize(18);
    painter.setFont(font);
    painter.setPen(QColor("#d0d0d0"));
    painter.drawText(pixmap.rect(), Qt::AlignCenter, QStringLiteral("👤"));

    return pixmap;
}
}

ArtistWidget::ArtistWidget(QWidget *parent)
    : QWidget(parent)
    , m_searchEdit(nullptr)
    , m_tableWidget(nullptr)
    , m_artists()
    , m_favoriteArtistIds()
    , m_subsonicApi(nullptr)
    , m_cacheManager(nullptr)
{
    setupUI();
}

void ArtistWidget::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("搜索艺术家...");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setStyleSheet(R"(
        QLineEdit {
            background-color: #2b2b2b;
            color: #ffffff;
            border: 1px solid #3d3d3d;
            border-radius: 6px;
            padding: 8px 10px;
            font-size: 13px;
        }
    )");

    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(4);
    m_tableWidget->setHorizontalHeaderLabels({QString(), "艺术家", "专辑数", QString()});
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->verticalHeader()->setVisible(false);
    m_tableWidget->setShowGrid(false);
    m_tableWidget->setAlternatingRowColors(true);
    m_tableWidget->setSortingEnabled(true);
    m_tableWidget->setWordWrap(false);
    m_tableWidget->setTextElideMode(Qt::ElideRight);
    m_tableWidget->horizontalHeader()->setSortIndicatorShown(true);

    m_tableWidget->setStyleSheet(R"(
        QTableWidget {
            background-color: #1e1e1e;
            color: #ffffff;
            border: none;
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

    connect(m_searchEdit, &QLineEdit::textChanged, this, &ArtistWidget::onSearchTextChanged);
    connect(m_tableWidget, &QTableWidget::cellDoubleClicked, this, &ArtistWidget::onCellDoubleClicked);
    connect(m_tableWidget, &QTableWidget::cellClicked, this, &ArtistWidget::onCellClicked);
    connect(m_tableWidget->horizontalHeader(), &QHeaderView::sortIndicatorChanged,
            this, [this](int logicalIndex, Qt::SortOrder order) {
        emit artistSortChanged(logicalIndex, order);
        // 排序后视口行全变了，重新按需请求
        QTimer::singleShot(0, this, [this]() { requestVisibleCoverArts(); });
    });
    // 滚动时：先取消排队的低优先级封面请求，再防抖重新请求"当前视口"
    connect(m_tableWidget->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &ArtistWidget::onVerticalScrollValueChanged);

    layout->addWidget(m_searchEdit);
    layout->addWidget(m_tableWidget, 1);

    updateResponsiveLayout();
}

void ArtistWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateResponsiveLayout();
    // 窄窗口视口高度变化也会影响可见行数
    requestVisibleCoverArts();
}

void ArtistWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    // 从别的 Tab 切进来时：重新按当前视口请求，避免旧页留下的 pending 占带宽
    requestVisibleCoverArts();
}

void ArtistWidget::setSubsonicApi(SubsonicApi *api)
{
    if (m_coverArtConnection) {
        disconnect(m_coverArtConnection);
    }

    m_subsonicApi = api;
    if (m_subsonicApi) {
        m_coverArtConnection = connect(m_subsonicApi, &SubsonicApi::coverArtReceived,
                                       this, &ArtistWidget::onCoverArtReceived);
    }
}

void ArtistWidget::setCacheManager(CacheManager *cache)
{
    m_cacheManager = cache;
}

void ArtistWidget::setArtists(const QVector<Artist> &artists)
{
    // 新一批艺术家到来时，旧的封面请求对新表完全没意义，直接中止腾出连接
    cancelPendingCoverArtRequests();
    m_pendingCoverArtIds.clear();

    m_artists = artists;
    m_artistById.clear();
    m_artistById.reserve(artists.size());
    for (const Artist &artist : artists) {
        if (!artist.id().isEmpty()) {
            m_artistById.insert(artist.id(), artist);
        }
    }
    rebuildTable();
}

void ArtistWidget::setFavoriteArtistIds(const QSet<QString> &artistIds)
{
    m_favoriteArtistIds = artistIds;
    updateFavoriteColumn();
}

void ArtistWidget::clear()
{
    // 切页/清空时立刻放弃未完成的封面请求，为下一个页面请求让路
    cancelPendingCoverArtRequests();
    m_pendingCoverArtIds.clear();
    m_artists.clear();
    m_artistById.clear();
    if (m_tableWidget) {
        m_tableWidget->setRowCount(0);
    }
    m_coverArtRowMap.clear();
}

void ArtistWidget::rebuildTable()
{
    if (!m_tableWidget) {
        return;
    }

    m_tableWidget->setSortingEnabled(false);
    m_tableWidget->clearContents();
    m_tableWidget->setRowCount(m_artists.size());
    m_coverArtRowMap.clear();

    const QPixmap placeholder = defaultAvatarPixmap();

    for (int row = 0; row < m_artists.size(); ++row) {
        const Artist &artist = m_artists[row];

        QTableWidgetItem *avatarItem = new QTableWidgetItem();
        avatarItem->setIcon(QIcon(placeholder));
        avatarItem->setTextAlignment(Qt::AlignCenter);
        avatarItem->setData(Qt::UserRole, artist.id());
        // 存一份 coverArtId 到额外 role，onCoverArtReceived 找视觉行时直接基于 avatar 列匹配
        // （跨排序/搜索仍然有效：因为 avatarItem 跟着行走）
        avatarItem->setData(Qt::UserRole + 1, artist.coverArtId());
        m_tableWidget->setItem(row, 0, avatarItem);

        QTableWidgetItem *nameItem = new QTableWidgetItem(artist.name());
        nameItem->setToolTip(artist.name());
        nameItem->setData(Qt::UserRole, artist.id());
        m_tableWidget->setItem(row, 1, nameItem);

        QTableWidgetItem *countItem = new QTableWidgetItem(QString::number(artist.albumCount()));
        countItem->setTextAlignment(Qt::AlignCenter);
        countItem->setData(Qt::EditRole, artist.albumCount());
        countItem->setData(Qt::UserRole, artist.id());
        m_tableWidget->setItem(row, 2, countItem);

        const bool favorite = artist.isFavorite() || m_favoriteArtistIds.contains(artist.id());
        QTableWidgetItem *favoriteItem = new QTableWidgetItem(favorite ? "♥" : "♡");
        favoriteItem->setTextAlignment(Qt::AlignCenter);
        favoriteItem->setData(Qt::UserRole, artist.id());
        m_tableWidget->setItem(row, 3, favoriteItem);

        if (!artist.coverArtId().isEmpty()) {
            m_coverArtRowMap.insert(artist.coverArtId(), row);

            if (m_cacheManager) {
                const QPixmap cached = m_cacheManager->getCoverArt(artist.coverArtId());
                if (!cached.isNull()) {
                    const QPixmap rounded = roundedPixmap(cached, QSize(kAvatarSize, kAvatarSize), kAvatarRadius);
                    avatarItem->setIcon(QIcon(rounded));
                }
            }
        }
    }

    m_tableWidget->setSortingEnabled(true);
    updateResponsiveLayout();
    onSearchTextChanged(m_searchEdit ? m_searchEdit->text() : QString());
    requestVisibleCoverArts();
}

void ArtistWidget::updateFavoriteColumn()
{
    if (!m_tableWidget) {
        return;
    }

    for (int row = 0; row < m_tableWidget->rowCount(); ++row) {
        QTableWidgetItem *nameItem = m_tableWidget->item(row, 1);
        QTableWidgetItem *favItem = m_tableWidget->item(row, 3);
        if (!nameItem || !favItem) {
            continue;
        }
        const QString artistId = nameItem->data(Qt::UserRole).toString();
        favItem->setText(m_favoriteArtistIds.contains(artistId) ? "♥" : "♡");
    }
}

void ArtistWidget::updateResponsiveLayout()
{
    if (!m_tableWidget) {
        return;
    }

    const int width = this->width();
    const bool showAlbumCount = width >= 520;

    m_tableWidget->setColumnHidden(2, !showAlbumCount);
    m_tableWidget->setColumnWidth(0, kAvatarSize + 16);
    m_tableWidget->setColumnWidth(3, 48);
    if (showAlbumCount) {
        m_tableWidget->setColumnWidth(2, 90);
    }
    m_tableWidget->horizontalHeader()->setStretchLastSection(false);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
}

void ArtistWidget::cancelPendingCoverArtRequests()
{
    if (!m_subsonicApi) {
        return;
    }
    m_subsonicApi->cancelPendingCoverArts();
}

void ArtistWidget::requestVisibleCoverArts()
{
    if (!m_subsonicApi || !m_tableWidget) {
        return;
    }

    const int totalRows = m_tableWidget->rowCount();
    if (totalRows <= 0) {
        return;
    }

    // 计算视口可见行范围：先用 (0,0) 和 (viewportHeight-1, 0) 的视觉行夹出可见区
    const int viewportHeight = m_tableWidget->viewport()->height();
    const int firstVisualRow = qBound(0, m_tableWidget->rowAt(0), totalRows - 1);
    // rowAt 若中间有隐藏行会 miss，兜底用二分：把坐标往下探
    int lastVisualRow = m_tableWidget->rowAt(viewportHeight - 1);
    if (lastVisualRow < 0) {
        // 视口底部刚好在空白时尝试用下一个像素找；若仍无则假设可见 1 屏
        lastVisualRow = m_tableWidget->rowAt(viewportHeight);
        if (lastVisualRow < 0) {
            const int rowHeight = qMax(1, m_tableWidget->rowHeight(firstVisualRow));
            lastVisualRow = firstVisualRow + qMax(0, viewportHeight / rowHeight);
        }
    }
    lastVisualRow = qBound(0, lastVisualRow, totalRows - 1);

    const int from = qMax(0, firstVisualRow - kVisiblePreloadMarginRows);
    const int to = qMin(totalRows - 1, lastVisualRow + kVisiblePreloadMarginRows);

    for (int visualRow = from; visualRow <= to; ++visualRow) {
        if (m_tableWidget->isRowHidden(visualRow)) {
            continue;
        }
        const QTableWidgetItem *nameItem = m_tableWidget->item(visualRow, 1);
        if (!nameItem) {
            continue;
        }
        const QString artistId = nameItem->data(Qt::UserRole).toString();
        if (artistId.isEmpty()) {
            continue;
        }
        const Artist artist = m_artistById.value(artistId);
        const QString coverArtId = artist.coverArtId();
        if (coverArtId.isEmpty()) {
            continue;
        }
        // 已经发过请求且未回调，不要再发一次
        if (m_pendingCoverArtIds.contains(coverArtId)) {
            continue;
        }
        // 缓存命中过的：rebuildTable 时已设置，不重新请求；这里检测 item 已经是非占位图的
        // 快速路径：CacheManager 已在上游填过 icon，只需再查一次缓存。
        if (m_cacheManager) {
            const QPixmap cached = m_cacheManager->getCoverArt(coverArtId);
            if (!cached.isNull()) {
                QTableWidgetItem *avatarItem = m_tableWidget->item(visualRow, 0);
                if (avatarItem) {
                    const QPixmap rounded = roundedPixmap(cached,
                                                           QSize(kAvatarSize, kAvatarSize),
                                                           kAvatarRadius);
                    avatarItem->setIcon(QIcon(rounded));
                }
                continue;
            }
        }
        m_pendingCoverArtIds.insert(coverArtId);
        m_subsonicApi->getCoverArt(coverArtId, kAvatarRequestSize);
    }
}

Artist ArtistWidget::artistAtRow(int row) const
{
    if (!m_tableWidget || row < 0 || row >= m_tableWidget->rowCount()) {
        return Artist();
    }
    const QTableWidgetItem *nameItem = m_tableWidget->item(row, 1);
    if (!nameItem) {
        return Artist();
    }
    const QString artistId = nameItem->data(Qt::UserRole).toString();
    if (artistId.isEmpty()) {
        return Artist();
    }
    return m_artistById.value(artistId);
}

void ArtistWidget::onSearchTextChanged(const QString &text)
{
    if (!m_tableWidget) {
        return;
    }

    const QString keyword = text.trimmed();
    emit artistSearchChanged(keyword);
    for (int row = 0; row < m_tableWidget->rowCount(); ++row) {
        QTableWidgetItem *nameItem = m_tableWidget->item(row, 1);
        if (!nameItem) {
            continue;
        }
        const bool match = keyword.isEmpty() || nameItem->text().contains(keyword, Qt::CaseInsensitive);
        m_tableWidget->setRowHidden(row, !match);
    }
    // 搜索结果变了，立刻放弃旧 pending，重新按视口取
    cancelPendingCoverArtRequests();
    m_pendingCoverArtIds.clear();
    QTimer::singleShot(kCoverRequestDebounceMs, this, [this]() {
        requestVisibleCoverArts();
    });
}

void ArtistWidget::onVerticalScrollValueChanged(int /*value*/)
{
    // 滚动途中：用户可能马上还有更大幅度滚动，把已经排队但仍未真正开始下载的封面请求直接 abort，
    // 防止滚到新视口时 getArtist 这种高优先级请求被 40 个封面 reply 堵在 QNAM 后面。
    cancelPendingCoverArtRequests();
    // 防抖，停下来再发当前视口请求
    QTimer::singleShot(kCoverRequestDebounceMs, this, [this]() {
        requestVisibleCoverArts();
    });
}

void ArtistWidget::onCellDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    Artist artist = artistAtRow(row);
    if (artist.id().isEmpty()) {
        return;
    }
    // 关键：立刻中止所有未完成的 getCoverArt，腾出 HTTP 连接槽让 getArtist 立即执行，
    // 否则 QNAM 同主机并发数默认有限，40 个封面请求会把 artist 请求阻塞到队列尾部。
    cancelPendingCoverArtRequests();
    m_pendingCoverArtIds.clear();
    emit artistSelected(artist);
}

void ArtistWidget::onCellClicked(int row, int column)
{
    if (column != 3) {
        return;
    }
    Artist artist = artistAtRow(row);
    if (artist.id().isEmpty()) {
        return;
    }
    emit artistFavoriteClicked(artist);
}

void ArtistWidget::onCoverArtReceived(const QString &coverArtId, const QByteArray &data)
{
    m_pendingCoverArtIds.remove(coverArtId);
    if (!m_tableWidget || coverArtId.isEmpty()) {
        return;
    }

    // 通过 avatar 列 (col 0) 匹配 Qt::UserRole+1 里存的 coverArtId 直接定位视觉行。
    // 优点：跨排序 / 搜索 / 行隐藏仍然稳定命中，O(N) 扫 avatar 项，不依赖构建期的 row。
    int targetRow = -1;
    for (int visualRow = 0; visualRow < m_tableWidget->rowCount(); ++visualRow) {
        const QTableWidgetItem *avatarItem = m_tableWidget->item(visualRow, 0);
        if (!avatarItem) {
            continue;
        }
        if (avatarItem->data(Qt::UserRole + 1).toString() == coverArtId) {
            targetRow = visualRow;
            break;
        }
    }
    if (targetRow < 0) {
        return;
    }

    QPixmap pixmap;
    if (!pixmap.loadFromData(data)) {
        return;
    }

    const QPixmap rounded = roundedPixmap(pixmap, QSize(kAvatarSize, kAvatarSize), kAvatarRadius);
    QTableWidgetItem *avatarItem = m_tableWidget->item(targetRow, 0);
    if (avatarItem) {
        avatarItem->setIcon(QIcon(rounded));
    }

    if (m_cacheManager) {
        m_cacheManager->putCoverArt(coverArtId, pixmap);
    }
}

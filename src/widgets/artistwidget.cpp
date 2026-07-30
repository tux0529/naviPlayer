#include "artistwidget.h"

#include "../cachemanager.h"
#include "../subsonicapi.h"

#include <QHeaderView>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QVBoxLayout>

namespace {
constexpr int kAvatarSize = 48;
constexpr int kAvatarRequestSize = 64;
constexpr qreal kAvatarRadius = 10.0;
constexpr int kCoverArtRequestLimit = 40;

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
    });

    layout->addWidget(m_searchEdit);
    layout->addWidget(m_tableWidget, 1);

    updateResponsiveLayout();
}

void ArtistWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateResponsiveLayout();
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
    m_artists = artists;
    rebuildTable();
}

void ArtistWidget::setFavoriteArtistIds(const QSet<QString> &artistIds)
{
    m_favoriteArtistIds = artistIds;
    updateFavoriteColumn();
}

void ArtistWidget::clear()
{
    m_artists.clear();
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

void ArtistWidget::requestVisibleCoverArts()
{
    if (!m_subsonicApi) {
        return;
    }

    int requested = 0;
    for (auto it = m_coverArtRowMap.begin(); it != m_coverArtRowMap.end(); ++it) {
        if (requested >= kCoverArtRequestLimit) {
            break;
        }
        const QString coverArtId = it.key();
        if (coverArtId.isEmpty()) {
            continue;
        }
        ++requested;
        m_subsonicApi->getCoverArt(coverArtId, kAvatarRequestSize);
    }
}

Artist ArtistWidget::artistAtRow(int row) const
{
    if (row < 0 || row >= m_artists.size()) {
        return Artist();
    }
    return m_artists[row];
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
}

void ArtistWidget::onCellDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    Artist artist = artistAtRow(row);
    if (artist.id().isEmpty()) {
        return;
    }
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
    if (!m_tableWidget || coverArtId.isEmpty()) {
        return;
    }

    const int row = m_coverArtRowMap.value(coverArtId, -1);
    if (row < 0 || row >= m_tableWidget->rowCount()) {
        return;
    }

    QPixmap pixmap;
    if (!pixmap.loadFromData(data)) {
        return;
    }

    const QPixmap rounded = roundedPixmap(pixmap, QSize(kAvatarSize, kAvatarSize), kAvatarRadius);
    QTableWidgetItem *avatarItem = m_tableWidget->item(row, 0);
    if (avatarItem) {
        avatarItem->setIcon(QIcon(rounded));
    }

    if (m_cacheManager) {
        m_cacheManager->putCoverArt(coverArtId, pixmap);
    }
}

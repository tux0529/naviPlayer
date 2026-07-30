#include "albumwidget.h"
#include "../subsonicapi.h"
#include "../cachemanager.h"
#include "../qt_compat.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QFrame>
#include <QScrollArea>
#include <QSpacerItem>
#include <QToolButton>
#include <QStyle>
#include <QSize>
#include <QScrollBar>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>

namespace {
constexpr int kAlbumCoverThumbnailSize = 120;
constexpr qreal kAlbumCoverRadius = 8.0;

QString albumThumbCacheKey(const QString &coverArtId)
{
    return QStringLiteral("album_thumb_%1").arg(coverArtId);
}

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

class HoverCoverWidget : public QWidget
{
public:
    explicit HoverCoverWidget(QWidget *parent = nullptr)
        : QWidget(parent)
        , m_playButton(nullptr)
    {
        setMouseTracking(true);
    }

    void setPlayButton(QToolButton *button)
    {
        m_playButton = button;
        if (m_playButton) {
            m_playButton->setVisible(false);
        }
    }

protected:
    void enterEvent(NaviEnterEvent *event) override
    {
        Q_UNUSED(event);
        if (m_playButton) {
            m_playButton->setVisible(true);
        }
    }

    void leaveEvent(QEvent *event) override
    {
        Q_UNUSED(event);
        if (m_playButton) {
            m_playButton->setVisible(false);
        }
    }

private:
    QToolButton *m_playButton;
};
}

AlbumWidget::AlbumWidget(QWidget *parent)
    : QWidget(parent)
    , m_subsonicApi(nullptr)
    , m_cacheManager(nullptr)
{
    setupUI();
}

void AlbumWidget::setSubsonicApi(SubsonicApi *api)
{
    m_subsonicApi = api;
    connect(m_subsonicApi, &SubsonicApi::coverArtReceived,
            this, &AlbumWidget::onCoverArtReceived);
}

void AlbumWidget::setCacheManager(CacheManager *cache)
{
    m_cacheManager = cache;
}

void AlbumWidget::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet(R"(
        QScrollArea {
            background-color: #1e1e1e;
            border: none;
        }
        QScrollArea > QWidget > QWidget {
            background-color: #1e1e1e;
        }
    )");

    m_contentWidget = new QWidget();
    m_contentWidget->setStyleSheet("background-color: #1e1e1e;");
    m_gridLayout = new QGridLayout(m_contentWidget);
    m_gridLayout->setSpacing(20);
    m_gridLayout->setContentsMargins(20, 20, 20, 20);

    m_scrollArea->setWidget(m_contentWidget);
    layout->addWidget(m_scrollArea);

    connect(m_scrollArea->verticalScrollBar(), &QScrollBar::valueChanged,
            this, [this]() { requestVisibleCovers(); });
    connect(m_scrollArea->horizontalScrollBar(), &QScrollBar::valueChanged,
            this, [this]() { requestVisibleCovers(); });
}

void AlbumWidget::setAlbums(const QVector<Album> &albums)
{
    m_albums = albums;
    m_pendingCoverArtIds.clear();
    updateUI();
}

void AlbumWidget::clear()
{
    m_albums.clear();
    m_coverLabels.clear();
    m_pendingCoverArtIds.clear();
    updateUI();
}

void AlbumWidget::updateUI()
{
    m_coverLabels.clear();

    // Clear existing items
    QLayoutItem *item;
    while ((item = m_gridLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Add new album items
    for (int i = 0; i < m_albums.size(); ++i) {
        const Album &album = m_albums[i];

        // Create album container widget - use ClickableWidget
        ClickableWidget *albumWidget = new ClickableWidget(i, this);
        albumWidget->setFixedSize(180, 250);

        // Connect the click signal
        connect(albumWidget, &ClickableWidget::clicked,
                this, &AlbumWidget::handleAlbumClick);

        QVBoxLayout *albumLayout = new QVBoxLayout(albumWidget);
        albumLayout->setContentsMargins(0, 0, 0, 0);
        albumLayout->setSpacing(8);

        // Album cover
        HoverCoverWidget *coverContainer = new HoverCoverWidget(albumWidget);
        coverContainer->setFixedSize(160, 160);

        QLabel *coverLabel = new QLabel(coverContainer);
        coverLabel->setFixedSize(160, 160);
        coverLabel->move(0, 0);
        coverLabel->setStyleSheet(R"(
            background-color: #3a3a3a;
            border-radius: 8px;
        )");
        coverLabel->setAlignment(Qt::AlignCenter);
        coverLabel->setText("📀");
        coverLabel->setObjectName("cover_" + album.coverArtId());
        coverLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        m_coverLabels.insert(album.coverArtId(), coverLabel);

        QToolButton *playButton = new QToolButton(coverContainer);
        playButton->setText("▶");
        playButton->setFixedSize(44, 44);
        playButton->move((coverContainer->width() - playButton->width()) / 2,
                         (coverContainer->height() - playButton->height()) / 2);
        playButton->setToolTip("播放专辑");
        playButton->setStyleSheet(R"(
            QToolButton {
                background-color: rgba(0, 0, 0, 140);
                border: none;
                border-radius: 22px;
                color: #ffffff;
                font-size: 18px;
                font-weight: bold;
            }
            QToolButton:hover {
                background-color: rgba(0, 0, 0, 180);
            }
        )");
        playButton->raise();
        coverContainer->setPlayButton(playButton);
        int index = i;
        connect(playButton, &QToolButton::clicked, this, [this, index]() {
            if (index >= 0 && index < m_albums.size()) {
                emit playAlbumRequested(m_albums[index]);
            }
        });

        // Album title
        auto sanitizeText = [](const QString &in) -> QString {
            QString out;
            out.reserve(in.size());
            for (QChar c : in) {
                if (c.isNull()) {
                    continue;
                }
                ushort u = c.unicode();
                if (u == 0x200B || u == 0xFEFF) {
                    continue;
                }
                if (u == 0x00A0) {
                    out.append(QChar(' '));
                    continue;
                }
                QChar::Category cat = c.category();
                if (cat == QChar::Other_Control
                        || cat == QChar::Other_Format
                        || cat == QChar::Other_Surrogate
                        || cat == QChar::Other_PrivateUse
                        || cat == QChar::Other_NotAssigned) {
                    continue;
                }
                out.append(c);
            }
            out = out.trimmed();
            return out;
        };

        QString visibleTitleText = sanitizeText(album.name());
        bool hasVisible = false;
        for (QChar c : visibleTitleText) {
            if (c.isLetterOrNumber() || c.isPunct() || c.isSymbol()) {
                hasVisible = true;
                break;
            }
        }
        if (!hasVisible) {
            visibleTitleText = "(未命名专辑)";
        }

        QLabel *titleLabel = new QLabel(albumWidget);
        titleLabel->setTextFormat(Qt::PlainText);
        titleLabel->setStyleSheet("color: #ffffff; background-color: transparent;");
        QFont titleFont = titleLabel->font();
        titleFont.setPointSize(14);
        titleFont.setBold(true);
        titleLabel->setFont(titleFont);
        titleLabel->setWordWrap(false);
        const int labelWidth = 160;
        titleLabel->setFixedWidth(labelWidth);
        titleLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        titleLabel->setFixedHeight(QFontMetrics(titleFont).height() + 2);
        titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        titleLabel->setText(QFontMetrics(titleFont).elidedText(visibleTitleText, Qt::ElideRight, labelWidth));

        // Artist name
        QLabel *artistLabel = new QLabel(album.artist());
        artistLabel->setStyleSheet("color: #aaaaaa; font-size: 12px;");
        artistLabel->setWordWrap(false);
        artistLabel->setFixedWidth(labelWidth);
        artistLabel->setFixedHeight(QFontMetrics(artistLabel->font()).height() + 2);
        artistLabel->setText(QFontMetrics(artistLabel->font()).elidedText(album.artist(), Qt::ElideRight, labelWidth));

        albumLayout->addWidget(coverContainer, 0, Qt::AlignCenter);
        albumLayout->addWidget(titleLabel);
        albumLayout->addWidget(artistLabel);

        // Make the widget clickable
        albumWidget->setCursor(Qt::PointingHandCursor);

        // Add to grid
        m_gridLayout->addWidget(albumWidget, i / 4, i % 4);
    }

    // Add stretch at bottom
    m_gridLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding),
                         m_albums.size() / 4 + 1, 0, 1, 4);

    requestVisibleCovers();
}

void AlbumWidget::loadAlbumCover(QLabel *coverLabel, const QString &coverArtId)
{
    if (coverArtId.isEmpty()) {
        return;
    }

    const QString cacheKey = albumThumbCacheKey(coverArtId);

    // Check cache first
    if (m_cacheManager) {
        QPixmap cached = m_cacheManager->getCoverArt(cacheKey);
        if (!cached.isNull()) {
            coverLabel->setPixmap(roundedPixmap(cached, coverLabel->size(), kAlbumCoverRadius));
            return;
        }
    }

    // Request from API
    if (m_subsonicApi && !m_pendingCoverArtIds.contains(coverArtId)) {
        m_pendingCoverArtIds.insert(coverArtId);
        m_subsonicApi->getCoverArt(coverArtId, kAlbumCoverThumbnailSize);
    }
}

void AlbumWidget::requestVisibleCovers()
{
    for (auto it = m_coverLabels.constBegin(); it != m_coverLabels.constEnd(); ++it) {
        QLabel *coverLabel = it.value();
        if (!coverLabel || !isVisibleInViewport(coverLabel)) {
            continue;
        }
        loadAlbumCover(coverLabel, it.key());
    }
}

bool AlbumWidget::isVisibleInViewport(QWidget *widget) const
{
    if (!widget || !m_scrollArea || !m_scrollArea->viewport()) {
        return false;
    }

    const QRect viewportRect = m_scrollArea->viewport()->rect();
    const QPoint topLeft = widget->mapTo(m_scrollArea->viewport(), QPoint(0, 0));
    const QRect widgetRect(topLeft, widget->size());
    return viewportRect.intersects(widgetRect.adjusted(-40, -40, 40, 40));
}

void AlbumWidget::onCoverArtReceived(const QString &coverArtId, const QByteArray &data)
{
    m_pendingCoverArtIds.remove(coverArtId);

    QPixmap pixmap;
    if (pixmap.loadFromData(data)) {
        QLabel *coverLabel = m_coverLabels.value(coverArtId, nullptr);
        QSize targetSize = coverLabel ? coverLabel->size() : QSize(kAlbumCoverThumbnailSize, kAlbumCoverThumbnailSize);
        QPixmap scaled = roundedPixmap(pixmap, targetSize, kAlbumCoverRadius);
        if (coverLabel) {
            coverLabel->setPixmap(scaled);
        }

        if (m_cacheManager) {
            m_cacheManager->putCoverArt(albumThumbCacheKey(coverArtId), scaled);
        }
    }
}

void AlbumWidget::handleAlbumClick(int index)
{
    qDebug() << "Album clicked at index:" << index;
    if (index >= 0 && index < m_albums.size()) {
        emit albumSelected(m_albums[index]);
    }
}

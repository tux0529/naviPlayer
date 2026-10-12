#include "coverlabel.h"

#include "core/cachemanager.h"
#include "core/appcontext.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QFont>

CoverLabel::CoverLabel(QWidget *parent)
    : QLabel(parent)
{
    setCursor(Qt::PointingHandCursor);
    setMouseTracking(true);
    setAlignment(Qt::AlignCenter);
    setMinimumSize(48, 48);
}

void CoverLabel::setCacheManager(CacheManager *cache)
{
    m_cache = cache;
    if (m_cache && !m_connected) {
        connect(m_cache, &CacheManager::coverReady, this, &CoverLabel::onCoverReady);
        m_connected = true;
    }
}

void CoverLabel::setRadius(int radius)
{
    m_radius = radius;
    update();
}

void CoverLabel::setHoverPlayEnabled(bool enabled)
{
    m_hoverPlayEnabled = enabled;
}

void CoverLabel::setPlaceholderText(const QString &text)
{
    m_placeholderText = text;
    update();
}

void CoverLabel::clear()
{
    m_coverArtId.clear();
    m_pixmap = QPixmap();
    update();
}

void CoverLabel::setCoverArt(const QString &coverArtId, int size)
{
    m_coverArtId = coverArtId;
    m_requestedSize = size;

    if (coverArtId.isEmpty()) {
        m_pixmap = QPixmap();
        update();
        return;
    }

    if (!m_cache)
        m_cache = AppContext::instance()->cache();
    if (!m_cache)
        return;
    setCacheManager(m_cache);

    // 高分屏按设备像素比取样，避免位图被放大后模糊（size 为逻辑显示尺寸）
    const qreal dpr = qApp ? qApp->devicePixelRatio() : 1.0;
    m_fetchSize = qMax(size, qRound(size * qMax<qreal>(1.0, dpr)));

    // 先查本地缓存
    const QPixmap cached = m_cache->cachedCover(coverArtId, m_fetchSize);
    if (!cached.isNull()) {
        m_pixmap = cached;
        update();
        return;
    }

    m_pixmap = QPixmap();
    m_cache->requestCover(coverArtId, m_fetchSize);
    update();
}

void CoverLabel::onCoverReady(const QString &coverArtId, int size, const QPixmap &pixmap)
{
    if (coverArtId != m_coverArtId)
        return;
    if (size != m_fetchSize && size != 0)
        return;
    if (pixmap.isNull())
        return;

    // 保留原始分辨率，由 paintEvent 按控件尺寸缩放（高分屏下才够清晰）
    m_pixmap = pixmap;
    update();
}

bool CoverLabel::playButtonContains(const QPointF &pos) const
{
    if (!m_hoverPlayEnabled)
        return false;
    const QRectF circle(rect().center().x() - 22, rect().center().y() - 22, 44, 44);
    return circle.contains(pos);
}

void CoverLabel::enterEvent(NaviEnterEvent *event)
{
    m_hovered = true;
    update();
    QLabel::enterEvent(event);
}

void CoverLabel::leaveEvent(QEvent *event)
{
    m_hovered = false;
    update();
    QLabel::leaveEvent(event);
}

void CoverLabel::mouseMoveEvent(QMouseEvent *event)
{
    QLabel::mouseMoveEvent(event);
    update();
}

void CoverLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (playButtonContains(naviEventPos(event))) {
            Q_EMIT playClicked();
            event->accept();
            return;
        }
        Q_EMIT clicked();
        event->accept();
        return;
    }
    QLabel::mousePressEvent(event);
}

void CoverLabel::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    update();
}

void CoverLabel::drawPlaceholder(QPainter *painter)
{
    painter->save();
    QLinearGradient gradient(rect().topLeft(), rect().bottomRight());
    gradient.setColorAt(0.0, QColor(35, 35, 40));
    gradient.setColorAt(1.0, QColor(24, 24, 28));
    painter->setBrush(gradient);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(rect(), m_radius, m_radius);

    painter->setPen(QColor(120, 120, 130));
    QFont f = painter->font();
    f.setPointSize(qMax(9, rect().height() / 6));
    painter->setFont(f);
    const QString text = m_placeholderText.isEmpty() ? QStringLiteral("♫") : m_placeholderText;
    painter->drawText(rect(), Qt::AlignCenter, text);
    painter->restore();
}

void CoverLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (m_pixmap.isNull()) {
        drawPlaceholder(&painter);
        return;
    }

    const QSize target = rect().size();
    QPixmap scaled = m_pixmap.scaled(target, Qt::KeepAspectRatioByExpanding,
                                     Qt::SmoothTransformation);
    // 居中裁剪
    const QPoint offset((scaled.width() - target.width()) / 2,
                        (scaled.height() - target.height()) / 2);
    scaled = scaled.copy(offset.x(), offset.y(), target.width(), target.height());

    QPainterPath path;
    path.addRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), m_radius, m_radius);
    painter.save();
    painter.setClipPath(path);
    painter.drawPixmap(rect(), scaled);
    painter.restore();

    // hover 半透明播放按钮
    if (m_hoverPlayEnabled && m_hovered && underMouse()) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 120));
        painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), m_radius, m_radius);

        const QPointF center(rect().center());
        painter.setBrush(QColor(255, 255, 255, 210));
        painter.drawEllipse(center, 22, 22);

        painter.setBrush(QColor(20, 20, 20));
        QPolygonF triangle;
        triangle.append(QPointF(center.x() - 6, center.y() - 9));
        triangle.append(QPointF(center.x() - 6, center.y() + 9));
        triangle.append(QPointF(center.x() + 11, center.y()));
        painter.drawPolygon(triangle);
    }
}

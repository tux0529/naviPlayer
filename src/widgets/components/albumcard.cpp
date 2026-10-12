#include "albumcard.h"
#include "coverlabel.h"

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QTimer>

namespace {
constexpr int kScrollIntervalMs = 320;
// 必须为纯 ASCII：QLatin1String 会把 UTF-8 多字节字符拆成乱码
const QString kScrollSeparator = QStringLiteral("      ");
}

AlbumCard::AlbumCard(QWidget *parent)
    : QFrame(parent)
{
    setObjectName(QStringLiteral("albumCard"));
    setFrameShape(QFrame::NoFrame);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    m_cover = new CoverLabel(this);
    m_cover->setObjectName(QStringLiteral("albumCardCover"));
    m_cover->setHoverPlayEnabled(true);
    m_cover->setFixedSize(m_coverSize, m_coverSize);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName(QStringLiteral("albumCardTitle"));
    m_titleLabel->setWordWrap(false);                 // 单行显示
    m_titleLabel->setAlignment(Qt::AlignCenter);      // 居中
    m_titleLabel->setCursor(Qt::PointingHandCursor);
    m_titleLabel->installEventFilter(this);

    m_artistButton = new QPushButton(this);
    m_artistButton->setObjectName(QStringLiteral("albumCardArtist"));
    m_artistButton->setFlat(true);
    m_artistButton->setCursor(Qt::PointingHandCursor);
    m_artistButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    layout->addWidget(m_cover, 0, Qt::AlignCenter);
    layout->addWidget(m_titleLabel);
    layout->addWidget(m_artistButton);

    m_scrollTimer = new QTimer(this);
    m_scrollTimer->setInterval(kScrollIntervalMs);
    connect(m_scrollTimer, &QTimer::timeout, this, &AlbumCard::onScrollTick);

    connect(m_cover, &CoverLabel::clicked, this, [this]() {
        Q_EMIT albumClicked(m_album);
    });
    connect(m_cover, &CoverLabel::playClicked, this, [this]() {
        Q_EMIT playClicked(m_album);
    });
    connect(m_artistButton, &QPushButton::clicked, this, [this]() {
        Artist artist;
        artist.setId(m_album.artistId());
        artist.setName(m_album.artist());
        Q_EMIT artistClicked(artist);
    });
}

void AlbumCard::setCoverSize(int size)
{
    m_coverSize = size;
    m_cover->setFixedSize(size, size);
    updateTitle();
}

void AlbumCard::setAlbum(const Album &album, int coverSize)
{
    m_album = album;
    m_fullTitle = album.name();
    m_scrollPos = 0;

    if (coverSize > 0)
        setCoverSize(coverSize);

    m_cover->setCoverArt(album.coverArtId(), m_coverSize);
    m_cover->setPlaceholderText(album.name());

    m_titleLabel->setToolTip(m_fullTitle);
    updateTitle();

    m_artistButton->setText(album.artist().isEmpty() ? QStringLiteral("未知艺术家") : album.artist());
    m_artistButton->setToolTip(m_artistButton->text());
}

bool AlbumCard::titleTruncated() const
{
    if (m_fullTitle.isEmpty())
        return false;
    QFontMetrics fm(m_titleLabel->font());
    const int w = m_titleLabel->width() > 0 ? m_titleLabel->width() : m_coverSize;
    return fm.horizontalAdvance(m_fullTitle) > w;
}

void AlbumCard::updateTitle()
{
    if (m_fullTitle.isEmpty()) {
        m_titleLabel->clear();
        return;
    }

    QFontMetrics fm(m_titleLabel->font());
    const int w = m_titleLabel->width() > 0 ? m_titleLabel->width() : m_coverSize;

    if (!m_hovered) {
        // 未悬停：超出宽度直接截断
        m_titleLabel->setText(fm.elidedText(m_fullTitle, Qt::ElideRight, w));
        return;
    }

    // 悬停且被截断：循环滚动显示
    const QString loop = m_fullTitle + kScrollSeparator + m_fullTitle + kScrollSeparator;
    if (m_scrollPos >= m_fullTitle.size() + kScrollSeparator.size())
        m_scrollPos = 0;
    m_titleLabel->setText(fm.elidedText(loop.mid(m_scrollPos), Qt::ElideRight, w));
}

void AlbumCard::onScrollTick()
{
    if (!m_hovered) {
        m_scrollTimer->stop();
        return;
    }
    ++m_scrollPos;
    updateTitle();
}

void AlbumCard::enterEvent(NaviEnterEvent *event)
{
    m_hovered = true;
    if (titleTruncated())
        m_scrollTimer->start();
    QFrame::enterEvent(event);
}

void AlbumCard::leaveEvent(QEvent *event)
{
    m_hovered = false;
    m_scrollTimer->stop();
    m_scrollPos = 0;
    updateTitle();
    QFrame::leaveEvent(event);
}

bool AlbumCard::eventFilter(QObject *watched, QEvent *event)
{
    // 标题点击：与封面同样进入专辑详情
    if (watched == m_titleLabel && event->type() == QEvent::MouseButtonRelease) {
        auto *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton && m_album.isValid()) {
            Q_EMIT albumClicked(m_album);
            return true;
        }
    }
    return QFrame::eventFilter(watched, event);
}

void AlbumCard::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    updateTitle();
}

#include "albumrowscroller.h"
#include "widgets/components/albumcard.h"

#include <QDebug>
#include <QEasingCurve>
#include <QHBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QScroller>
#include <QTimer>
#include <QVBoxLayout>

namespace {
// 与专辑列表（AlbumsView）保持一致：单卡宽度 176、网格间距 16、封面 = 卡宽 - 24
constexpr int kCardWidth = 150;
constexpr int kSpacing = 16;
constexpr int kCoverSize = kCardWidth - 16;
// 标题改为单行后的卡片实测高度（封面 152 + 标题 + 艺术家 + 间距）+ 上下留白
constexpr int kRowHeight = 218;
// 行内容上下留白（构造时 m_rowLayout 的 margins）
constexpr int kRowMargin = 4;

QPushButton *makeArrowButton(const QString &text, const char *objName, QWidget *parent)
{
    auto *btn = new QPushButton(text, parent);
    btn->setObjectName(QLatin1String(objName));
    btn->setProperty("rowArrow", true);   // 供 QSS 统一匹配
    btn->setFixedSize(30, 30);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFocusPolicy(Qt::NoFocus);
    return btn;
}

void deleteRowChildren(QHBoxLayout *layout)
{
    QLayoutItem *item = nullptr;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }
}
}

AlbumRowScroller::AlbumRowScroller(QWidget *parent)
    : QWidget(parent)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(6);

    // ---- 标题行：标题 + 右对齐的左右箭头（固定在右上角，不随列表滚动） ----
    auto *header = new QHBoxLayout;
    header->setContentsMargins(0, 0, 0, 0);
    header->setSpacing(6);

    m_title = new QLabel(this);
    m_title->setObjectName(QStringLiteral("sectionTitle"));
    m_title->setStyleSheet(QStringLiteral("font-size: 15px; font-weight: 600;"));

    m_leftArrow = makeArrowButton(QStringLiteral("‹"), "arrowLeftButton", this);
    m_rightArrow = makeArrowButton(QStringLiteral("›"), "arrowRightButton", this);

    header->addWidget(m_title);
    header->addStretch(1);
    header->addWidget(m_leftArrow);
    header->addWidget(m_rightArrow);
    outer->addLayout(header);

    // ---- 横向滚动区域 ----
    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 由箭头 + 拖拽驱动
    m_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setFixedHeight(kRowHeight);

    m_container = new QWidget;
    m_container->resize(m_scroll->viewport()->width(), kRowHeight);
    m_rowLayout = new QHBoxLayout(m_container);
    m_rowLayout->setContentsMargins(8, 8, 8, 8);
    m_rowLayout->setSpacing(kSpacing);
    m_scroll->setWidget(m_container);
    outer->addWidget(m_scroll);

    // 触摸滑动 + 鼠标拖动滚动（QScroller 会驱动横向滚动条 value）
    // 注意：不拦截鼠标滚轮，滚轮交给外层页面做竖向滚动
    QScroller::grabGesture(m_scroll->viewport(), QScroller::TouchGesture);
    QScroller::grabGesture(m_scroll->viewport(), QScroller::LeftMouseButtonGesture);

    auto *hbar = m_scroll->horizontalScrollBar();
    connect(hbar, &QScrollBar::valueChanged, this, &AlbumRowScroller::updateArrowState);
    connect(hbar, &QScrollBar::rangeChanged, this, &AlbumRowScroller::updateArrowState);
    connect(m_leftArrow, &QPushButton::clicked, this, [this]() { scrollByPage(-1); });
    connect(m_rightArrow, &QPushButton::clicked, this, [this]() { scrollByPage(1); });

    // 延迟重建卡片：切页时同步创建大量卡片会阻塞主线程（音频卡顿），
    // 延迟到下一轮事件循环再创建，先让音频缓冲得到补充。
    m_buildTimer = new QTimer(this);
    m_buildTimer->setSingleShot(true);
    m_buildTimer->setInterval(0);
    connect(m_buildTimer, &QTimer::timeout, this, &AlbumRowScroller::buildCards);

    updateArrowState();
}

void AlbumRowScroller::setTitle(const QString &title)
{
    m_title->setText(title);
}

void AlbumRowScroller::setCoverSize(int size)
{
    m_coverSize = size;
}

void AlbumRowScroller::setAlbums(const QVector<Album> &albums)
{
    // 数据未变化时跳过重建：切页会反复调用 setAlbums，
    // 每次都重建卡片（创建大量 QWidget + 解码封面）会导致主线程阻塞卡顿
    if (m_albums.size() == albums.size()) {
        bool same = true;
        for (int i = 0; i < albums.size(); ++i) {
            if (m_albums.at(i).id() != albums.at(i).id()) {
                same = false;
                break;
            }
        }
        if (same)
            return;
    }

    m_albums = albums;
    // 延迟到下一轮事件循环再重建卡片（重启单发定时器，保证用最新数据）
    m_buildTimer->start();
}

void AlbumRowScroller::buildCards()
{
    clearCards();

    int tallestCard = 0;
    for (const Album &album : m_albums) {
        auto *card = new AlbumCard(m_container);
        card->setFixedWidth(kCardWidth);
        card->setAlbum(album, m_coverSize);
        connect(card, &AlbumCard::albumClicked, this, &AlbumRowScroller::albumClicked);
        connect(card, &AlbumCard::artistClicked, this, &AlbumRowScroller::artistClicked);
        connect(card, &AlbumCard::playClicked, this, &AlbumRowScroller::playClicked);
        m_rowLayout->addWidget(card);
        tallestCard = qMax(tallestCard, card->sizeHint().height());
    }
    m_rowLayout->addStretch(1);

    // 行高按卡片实际高度自适应，但不低于下限 kRowHeight
    // （无数据/卡片尚未布局时容器 sizeHint 会退化成边距高度，不能直接拿来用）
    const int wanted = tallestCard > 0 ? tallestCard + 2 * kRowMargin : kRowHeight;
    m_scroll->setFixedHeight(qMax(kRowHeight, wanted));

    // 回到最左并刷新箭头状态
    m_scroll->horizontalScrollBar()->setValue(0);
    updateArrowState();
}

void AlbumRowScroller::clearCards()
{
    deleteRowChildren(m_rowLayout);
}

void AlbumRowScroller::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateArrowState();
}

void AlbumRowScroller::scrollByPage(int direction)
{
    QScrollBar *hbar = m_scroll->horizontalScrollBar();
    const int page = qMax(hbar->pageStep(), 1); // 一屏 = 可视宽度
    animateScrollTo(hbar->value() + direction * page);
}

void AlbumRowScroller::animateScrollTo(int target)
{
    QScrollBar *hbar = m_scroll->horizontalScrollBar();
    target = qBound(hbar->minimum(), target, hbar->maximum());
    if (target == hbar->value())
        return;

    auto *anim = new QPropertyAnimation(hbar, "value", this);
    anim->setDuration(260);
    anim->setStartValue(hbar->value());
    anim->setEndValue(target);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AlbumRowScroller::updateArrowState()
{
    if (!m_leftArrow || !m_rightArrow || !m_scroll)
        return;
    QScrollBar *hbar = m_scroll->horizontalScrollBar();
    const bool atLeft = hbar->value() <= hbar->minimum();
    const bool atRight = hbar->value() >= hbar->maximum();
    m_leftArrow->setEnabled(!atLeft);
    m_rightArrow->setEnabled(!atRight);
}

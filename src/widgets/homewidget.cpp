#include "homewidget.h"

#include "../cachemanager.h"
#include "../qt_compat.h"
#include "../subsonicapi.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QScrollBar>
#include <QSet>
#include <QStackedWidget>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>

#include <functional>

namespace {
constexpr int kCoverSize = 160;
constexpr int kCardWidth = 180;
constexpr int kCardHeight = 250;
constexpr int kScrollDistance = kCardWidth * 3;
constexpr qreal kCoverRadius = 8.0;

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

QString sanitizeText(const QString &input)
{
    QString output;
    output.reserve(input.size());
    for (QChar c : input) {
        if (c.isNull()) {
            continue;
        }
        const ushort unicode = c.unicode();
        if (unicode == 0x200B || unicode == 0xFEFF) {
            continue;
        }
        if (unicode == 0x00A0) {
            output.append(QChar(' '));
            continue;
        }
        const QChar::Category category = c.category();
        if (category == QChar::Other_Control
                || category == QChar::Other_Format
                || category == QChar::Other_Surrogate
                || category == QChar::Other_PrivateUse
                || category == QChar::Other_NotAssigned) {
            continue;
        }
        output.append(c);
    }

    output = output.trimmed();
    if (output.isEmpty()) {
        return QStringLiteral("(未命名专辑)");
    }
    return output;
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

class HomeAlbumCard : public QWidget
{
public:
    explicit HomeAlbumCard(const Album &album, QWidget *parent = nullptr)
        : QWidget(parent)
        , m_album(album)
        , m_coverContainer(new HoverCoverWidget(this))
        , m_coverLabel(new QLabel(m_coverContainer))
        , m_titleLabel(new QLabel(this))
        , m_artistLabel(new QLabel(this))
        , m_playButton(new QToolButton(m_coverContainer))
        , m_subsonicApi(nullptr)
        , m_cacheManager(nullptr)
        , m_coverRequested(false)
        , m_coverLoaded(false)
    {
        setFixedSize(kCardWidth, kCardHeight);
        setCursor(Qt::PointingHandCursor);

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(8);

        m_coverContainer->setFixedSize(kCoverSize, kCoverSize);

        m_coverLabel->setFixedSize(kCoverSize, kCoverSize);
        m_coverLabel->move(0, 0);
        m_coverLabel->setAlignment(Qt::AlignCenter);
        m_coverLabel->setStyleSheet(R"(
            background-color: #3a3a3a;
            border-radius: 8px;
            color: #d0d0d0;
            font-size: 40px;
        )");
        m_coverLabel->setText("📀");
        m_coverLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

        m_playButton->setText("▶");
        m_playButton->setFixedSize(44, 44);
        m_playButton->move((kCoverSize - m_playButton->width()) / 2,
                           (kCoverSize - m_playButton->height()) / 2);
        m_playButton->setToolTip("播放专辑");
        m_playButton->setCursor(Qt::PointingHandCursor);
        m_playButton->setStyleSheet(R"(
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
        m_coverContainer->setPlayButton(m_playButton);

        QFont titleFont = m_titleLabel->font();
        titleFont.setPointSize(14);
        titleFont.setBold(true);
        m_titleLabel->setFont(titleFont);
        m_titleLabel->setFixedWidth(kCoverSize);
        m_titleLabel->setFixedHeight(QFontMetrics(titleFont).height() + 2);
        m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_titleLabel->setStyleSheet("color: #ffffff; background-color: transparent;");
        m_titleLabel->setText(QFontMetrics(titleFont).elidedText(sanitizeText(album.name()),
                                                                 Qt::ElideRight,
                                                                 kCoverSize));

        m_artistLabel->setFixedWidth(kCoverSize);
        m_artistLabel->setFixedHeight(QFontMetrics(m_artistLabel->font()).height() + 2);
        m_artistLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_artistLabel->setStyleSheet("color: #aaaaaa; font-size: 12px;");
        m_artistLabel->setText(QFontMetrics(m_artistLabel->font()).elidedText(album.artist(),
                                                                               Qt::ElideRight,
                                                                               kCoverSize));

        layout->addWidget(m_coverContainer, 0, Qt::AlignCenter);
        layout->addWidget(m_titleLabel);
        layout->addWidget(m_artistLabel);

        connect(m_playButton, &QToolButton::clicked, this, [this]() {
            if (m_onPlayRequested) {
                m_onPlayRequested(m_album);
            }
        });
    }

    const Album &album() const
    {
        return m_album;
    }

    void setDependencies(SubsonicApi *api, CacheManager *cache)
    {
        m_subsonicApi = api;
        m_cacheManager = cache;
        loadFromCache();
    }

    void setOnActivated(const std::function<void(const Album &)> &callback)
    {
        m_onActivated = callback;
    }

    void setOnPlayRequested(const std::function<void(const Album &)> &callback)
    {
        m_onPlayRequested = callback;
    }

    void ensureCoverRequested()
    {
        if (m_coverLoaded || m_coverRequested || m_album.coverArtId().isEmpty()) {
            return;
        }

        loadFromCache();
        if (m_coverLoaded || !m_subsonicApi) {
            return;
        }

        m_coverRequested = true;
        m_subsonicApi->getCoverArt(m_album.coverArtId(), kCoverSize);
    }

    void applyCoverArt(const QString &coverArtId, const QByteArray &data)
    {
        if (coverArtId != m_album.coverArtId() || data.isEmpty()) {
            return;
        }

        QPixmap pixmap;
        if (!pixmap.loadFromData(data)) {
            return;
        }

        const QPixmap scaled = pixmap.scaled(m_coverLabel->size(),
                                             Qt::KeepAspectRatioByExpanding,
                                             Qt::SmoothTransformation);
        m_coverLabel->setPixmap(roundedPixmap(scaled, m_coverLabel->size(), kCoverRadius));
        m_coverLabel->setText(QString());
        m_coverLoaded = true;
        m_coverRequested = false;

        if (m_cacheManager) {
            m_cacheManager->putCoverArt(albumThumbCacheKey(coverArtId), scaled);
        }
    }

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton && m_onActivated) {
            m_onActivated(m_album);
        }
        QWidget::mousePressEvent(event);
    }

private:
    void loadFromCache()
    {
        if (!m_cacheManager || m_album.coverArtId().isEmpty() || m_coverLoaded) {
            return;
        }

        const QPixmap cached = m_cacheManager->getCoverArt(albumThumbCacheKey(m_album.coverArtId()));
        if (!cached.isNull()) {
            m_coverLabel->setPixmap(roundedPixmap(cached, m_coverLabel->size(), kCoverRadius));
            m_coverLabel->setText(QString());
            m_coverLoaded = true;
        }
    }

    Album m_album;
    HoverCoverWidget *m_coverContainer;
    QLabel *m_coverLabel;
    QLabel *m_titleLabel;
    QLabel *m_artistLabel;
    QToolButton *m_playButton;
    SubsonicApi *m_subsonicApi;
    CacheManager *m_cacheManager;
    bool m_coverRequested;
    bool m_coverLoaded;
    std::function<void(const Album &)> m_onActivated;
    std::function<void(const Album &)> m_onPlayRequested;
};

class HomeSectionView : public QFrame
{
public:
    explicit HomeSectionView(const QString &title, QWidget *parent = nullptr)
        : QFrame(parent)
        , m_titleLabel(new QLabel(title, this))
        , m_leftButton(new QToolButton(this))
        , m_rightButton(new QToolButton(this))
        , m_stateStack(new QStackedWidget(this))
        , m_placeholderLabel(new QLabel(this))
        , m_scrollArea(new QScrollArea(this))
        , m_contentWidget(new QWidget(this))
        , m_contentLayout(new QHBoxLayout(m_contentWidget))
        , m_subsonicApi(nullptr)
        , m_cacheManager(nullptr)
    {
        setStyleSheet(R"(
            QFrame {
                background-color: #252525;
                border-radius: 12px;
            }
            QLabel {
                background-color: transparent;
            }
        )");

        QVBoxLayout *rootLayout = new QVBoxLayout(this);
        rootLayout->setContentsMargins(20, 20, 20, 20);
        rootLayout->setSpacing(16);

        QHBoxLayout *headerLayout = new QHBoxLayout();
        m_titleLabel->setStyleSheet("color: #ffffff; font-size: 18px; font-weight: bold;");

        const QString buttonStyle = QStringLiteral(R"(
            QToolButton {
                background-color: #3d3d3d;
                border: none;
                border-radius: 16px;
                color: #ffffff;
                min-width: 32px;
                min-height: 32px;
            }
            QToolButton:hover:enabled {
                background-color: #4d4d4d;
            }
            QToolButton:disabled {
                background-color: #2d2d2d;
                color: #666666;
            }
        )");

        m_leftButton->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
        m_rightButton->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
        m_leftButton->setCursor(Qt::PointingHandCursor);
        m_rightButton->setCursor(Qt::PointingHandCursor);
        m_leftButton->setStyleSheet(buttonStyle);
        m_rightButton->setStyleSheet(buttonStyle);
        m_leftButton->setEnabled(false);
        m_rightButton->setEnabled(false);

        headerLayout->addWidget(m_titleLabel);
        headerLayout->addStretch();
        headerLayout->addWidget(m_leftButton);
        headerLayout->addWidget(m_rightButton);

        QWidget *placeholderPage = new QWidget(this);
        QVBoxLayout *placeholderLayout = new QVBoxLayout(placeholderPage);
        placeholderLayout->setContentsMargins(0, 12, 0, 12);
        placeholderLayout->addWidget(m_placeholderLabel, 0, Qt::AlignCenter);
        m_placeholderLabel->setAlignment(Qt::AlignCenter);
        m_placeholderLabel->setStyleSheet("color: #a0a0a0; font-size: 13px;");
        placeholderPage->setMinimumHeight(kCardHeight);
        placeholderPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        m_scrollArea->setWidgetResizable(true);
        m_scrollArea->setFrameShape(QFrame::NoFrame);
        m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_scrollArea->setStyleSheet(R"(
            QScrollArea {
                background-color: transparent;
                border: none;
            }
            QScrollArea > QWidget > QWidget {
                background-color: transparent;
            }
        )");
        m_scrollArea->horizontalScrollBar()->setSingleStep(40);
        m_scrollArea->setMinimumHeight(kCardHeight);
        m_scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        m_contentLayout->setContentsMargins(0, 0, 0, 0);
        m_contentLayout->setSpacing(20);
        m_contentLayout->addStretch();
        m_scrollArea->setWidget(m_contentWidget);

        m_stateStack->addWidget(placeholderPage);
        m_stateStack->addWidget(m_scrollArea);
        m_stateStack->setMinimumHeight(kCardHeight);
        m_stateStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        rootLayout->addLayout(headerLayout);
        rootLayout->addWidget(m_stateStack);

        connect(m_leftButton, &QToolButton::clicked, this, [this]() {
            animateScroll(-kScrollDistance);
        });
        connect(m_rightButton, &QToolButton::clicked, this, [this]() {
            animateScroll(kScrollDistance);
        });
        connect(m_scrollArea->horizontalScrollBar(), &QScrollBar::valueChanged, this, [this]() {
            updateScrollButtons();
            ensureVisibleCovers();
        });

        setLoading(true);
    }

    void setDependencies(SubsonicApi *api, CacheManager *cache)
    {
        m_subsonicApi = api;
        m_cacheManager = cache;
        for (HomeAlbumCard *card : m_cards) {
            card->setDependencies(m_subsonicApi, m_cacheManager);
        }
        ensureVisibleCovers();
    }

    void setAlbumActivatedCallback(const std::function<void(const Album &)> &callback)
    {
        m_onAlbumActivated = callback;
        for (HomeAlbumCard *card : m_cards) {
            card->setOnActivated(m_onAlbumActivated);
        }
    }

    void setPlayRequestedCallback(const std::function<void(const Album &)> &callback)
    {
        m_onPlayRequested = callback;
        for (HomeAlbumCard *card : m_cards) {
            card->setOnPlayRequested(m_onPlayRequested);
        }
    }

    void setLoading(bool loading)
    {
        if (loading) {
            m_placeholderLabel->setStyleSheet("color: #a0a0a0; font-size: 13px;");
            m_placeholderLabel->setText("正在加载...");
            m_stateStack->setCurrentIndex(0);
            updateScrollButtons();
        }
    }

    void setMessage(const QString &message, bool isError)
    {
        m_placeholderLabel->setStyleSheet(isError
                                          ? "color: #ff8f8f; font-size: 13px;"
                                          : "color: #a0a0a0; font-size: 13px;");
        m_placeholderLabel->setText(message);
        m_stateStack->setCurrentIndex(0);
        updateScrollButtons();
    }

    void setAlbums(const QVector<Album> &albums)
    {
        m_cards.clear();
        QLayoutItem *item = nullptr;
        while ((item = m_contentLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }

        if (albums.isEmpty()) {
            setMessage("暂无专辑", false);
            return;
        }

        for (const Album &album : albums) {
            HomeAlbumCard *card = new HomeAlbumCard(album, m_contentWidget);
            card->setDependencies(m_subsonicApi, m_cacheManager);
            card->setOnActivated(m_onAlbumActivated);
            card->setOnPlayRequested(m_onPlayRequested);
            m_cards.append(card);
            m_contentLayout->addWidget(card);
        }
        m_contentLayout->addStretch();

        m_stateStack->setCurrentWidget(m_scrollArea);
        m_scrollArea->horizontalScrollBar()->setValue(0);
        updateScrollButtons();
        ensureVisibleCovers();
    }

    void reset()
    {
        setMessage("等待加载", false);
    }

    void handleCoverArtReceived(const QString &coverArtId, const QByteArray &data)
    {
        for (HomeAlbumCard *card : m_cards) {
            card->applyCoverArt(coverArtId, data);
        }
    }

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        QFrame::resizeEvent(event);
        updateScrollButtons();
        ensureVisibleCovers();
    }

private:
    void animateScroll(int delta)
    {
        QScrollBar *scrollBar = m_scrollArea->horizontalScrollBar();
        const int startValue = scrollBar->value();
        const int endValue = qBound(scrollBar->minimum(),
                                    startValue + delta,
                                    scrollBar->maximum());
        if (startValue == endValue) {
            updateScrollButtons();
            return;
        }

        QPropertyAnimation *animation = new QPropertyAnimation(scrollBar, "value", this);
        animation->setDuration(180);
        animation->setStartValue(startValue);
        animation->setEndValue(endValue);
        animation->setEasingCurve(QEasingCurve::OutCubic);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }

    void updateScrollButtons()
    {
        if (m_stateStack->currentWidget() != m_scrollArea) {
            m_leftButton->setEnabled(false);
            m_rightButton->setEnabled(false);
            return;
        }

        QScrollBar *scrollBar = m_scrollArea->horizontalScrollBar();
        m_leftButton->setEnabled(scrollBar->value() > scrollBar->minimum());
        m_rightButton->setEnabled(scrollBar->value() < scrollBar->maximum());
    }

    void ensureVisibleCovers()
    {
        if (m_stateStack->currentWidget() != m_scrollArea) {
            return;
        }

        const QRect viewportRect = m_scrollArea->viewport()->rect().adjusted(-120, 0, 120, 0);
        for (HomeAlbumCard *card : m_cards) {
            const QPoint topLeft = card->mapTo(m_scrollArea->viewport(), QPoint(0, 0));
            const QRect cardRect(topLeft, card->size());
            if (cardRect.intersects(viewportRect)) {
                card->ensureCoverRequested();
            }
        }
    }

    QLabel *m_titleLabel;
    QToolButton *m_leftButton;
    QToolButton *m_rightButton;
    QStackedWidget *m_stateStack;
    QLabel *m_placeholderLabel;
    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QHBoxLayout *m_contentLayout;
    QVector<HomeAlbumCard *> m_cards;
    SubsonicApi *m_subsonicApi;
    CacheManager *m_cacheManager;
    std::function<void(const Album &)> m_onAlbumActivated;
    std::function<void(const Album &)> m_onPlayRequested;
};
}

HomeWidget::HomeWidget(QWidget *parent)
    : QWidget(parent)
    , m_subsonicApi(nullptr)
    , m_cacheManager(nullptr)
{
    setupUI();
}

void HomeWidget::setupUI()
{
    setStyleSheet("background-color: #1e1e1e; color: #ffffff;");

    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet(R"(
        QScrollArea {
            background-color: #1e1e1e;
            border: none;
        }
        QScrollArea > QWidget > QWidget {
            background-color: #1e1e1e;
        }
    )");

    QWidget *contentWidget = new QWidget(scrollArea);
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(24, 24, 24, 24);
    contentLayout->setSpacing(20);

    QLabel *titleLabel = new QLabel("主页", contentWidget);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #ffffff;");

    QLabel *subtitleLabel = new QLabel("聚合浏览最近播放、最新添加、播放最多和随机专辑。", contentWidget);
    subtitleLabel->setStyleSheet("color: #b0b0b0; font-size: 13px;");

    contentLayout->addWidget(titleLabel);
    contentLayout->addWidget(subtitleLabel);

    const struct SectionDefinition {
        SectionType type;
        const char *title;
    } sectionDefinitions[] = {
        { RecentAlbums, "最近播放的专辑" },
        { NewestAlbums, "最新添加的专辑" },
        { FrequentAlbums, "播放最多的专辑" },
        { RandomAlbums, "随机专辑" }
    };

    for (const SectionDefinition &definition : sectionDefinitions) {
        HomeSectionView *sectionWidget = new HomeSectionView(QString::fromUtf8(definition.title), contentWidget);
        sectionWidget->setAlbumActivatedCallback([this](const Album &album) {
            emit albumSelected(album);
        });
        sectionWidget->setPlayRequestedCallback([this](const Album &album) {
            emit playAlbumRequested(album);
        });
        sectionWidget->setDependencies(m_subsonicApi, m_cacheManager);
        m_sections.insert(static_cast<int>(definition.type), sectionWidget);
        contentLayout->addWidget(sectionWidget);
    }

    contentLayout->addStretch();
    scrollArea->setWidget(contentWidget);
    rootLayout->addWidget(scrollArea);
}

QWidget *HomeWidget::section(SectionType type) const
{
    return m_sections.value(static_cast<int>(type), nullptr);
}

void HomeWidget::setSubsonicApi(SubsonicApi *api)
{
    if (m_coverArtConnection) {
        disconnect(m_coverArtConnection);
    }

    m_subsonicApi = api;
    for (QWidget *widget : m_sections) {
        HomeSectionView *sectionWidget = static_cast<HomeSectionView *>(widget);
        if (sectionWidget) {
            sectionWidget->setDependencies(m_subsonicApi, m_cacheManager);
        }
    }

    if (m_subsonicApi) {
        m_coverArtConnection = connect(m_subsonicApi, &SubsonicApi::coverArtReceived,
                                       this, [this](const QString &coverArtId, const QByteArray &data) {
            for (QWidget *widget : m_sections) {
                HomeSectionView *sectionWidget = static_cast<HomeSectionView *>(widget);
                if (sectionWidget) {
                    sectionWidget->handleCoverArtReceived(coverArtId, data);
                }
            }
        });
    }
}

void HomeWidget::setCacheManager(CacheManager *cache)
{
    m_cacheManager = cache;
    for (QWidget *widget : m_sections) {
        HomeSectionView *sectionWidget = static_cast<HomeSectionView *>(widget);
        if (sectionWidget) {
            sectionWidget->setDependencies(m_subsonicApi, m_cacheManager);
        }
    }
}

void HomeWidget::setSectionLoading(SectionType type, bool loading)
{
    HomeSectionView *sectionWidget = static_cast<HomeSectionView *>(section(type));
    if (sectionWidget) {
        sectionWidget->setLoading(loading);
    }
}

void HomeWidget::setSectionAlbums(SectionType type, const QVector<Album> &albums)
{
    HomeSectionView *sectionWidget = static_cast<HomeSectionView *>(section(type));
    if (sectionWidget) {
        sectionWidget->setAlbums(albums);
    }
}

void HomeWidget::setSectionMessage(SectionType type, const QString &message, bool isError)
{
    HomeSectionView *sectionWidget = static_cast<HomeSectionView *>(section(type));
    if (sectionWidget) {
        sectionWidget->setMessage(message, isError);
    }
}

void HomeWidget::resetSections()
{
    for (QWidget *widget : m_sections) {
        HomeSectionView *sectionWidget = static_cast<HomeSectionView *>(widget);
        if (sectionWidget) {
            sectionWidget->reset();
        }
    }
}

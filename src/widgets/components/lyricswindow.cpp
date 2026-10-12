#include "lyricswindow.h"

#include "core/appcontext.h"

#include <QApplication>
#include <QCursor>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QScreen>
#include <QTimer>
#include <QVBoxLayout>

LyricsWindow::LyricsWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
                   | Qt::WindowDoesNotAcceptFocus);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setObjectName(QStringLiteral("lyricsWindow"));
    setWindowTitle(QStringLiteral("歌词"));

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 12, 24, 12);
    layout->setSpacing(6);

    m_currentLabel = new QLabel(this);
    m_currentLabel->setObjectName(QStringLiteral("lyricsCurrent"));
    m_currentLabel->setAlignment(Qt::AlignCenter);
    m_currentLabel->setWordWrap(false);

    m_nextLabel = new QLabel(this);
    m_nextLabel->setObjectName(QStringLiteral("lyricsNext"));
    m_nextLabel->setAlignment(Qt::AlignCenter);
    m_nextLabel->setWordWrap(false);

    layout->addWidget(m_currentLabel);
    layout->addWidget(m_nextLabel);

    m_fade = new QPropertyAnimation(this, "windowOpacity", this);
    m_fade->setDuration(180);

    m_hoverTimer = new QTimer(this);
    m_hoverTimer->setSingleShot(true);
    m_hoverTimer->setInterval(220);
    connect(m_hoverTimer, &QTimer::timeout, this, &LyricsWindow::onHoverTimer);

    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(300);
    connect(m_pollTimer, &QTimer::timeout, this, &LyricsWindow::onPollCursor);

    // 初始位置：屏幕底部居中
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        const QRect available = screen->availableGeometry();
        setGeometry(available.center().x() - 350, available.bottom() - 160, 700, 96);
    } else {
        setGeometry(200, 200, 700, 96);
    }
}

void LyricsWindow::setLyricsManager(LyricsManager *manager)
{
    if (m_lyrics) {
        disconnect(m_lyrics, nullptr, this, nullptr);
    }
    m_lyrics = manager;
    if (!m_lyrics)
        return;

    connect(m_lyrics, &LyricsManager::linesChanged, this, &LyricsWindow::onLinesChanged);
    connect(m_lyrics, &LyricsManager::styleChanged, this, &LyricsWindow::onStyleChanged);
    applyStyle(m_lyrics->style());
    m_pollTimer->start();
}

void LyricsWindow::applyStyle(const LyricsStyle &style)
{
    QFont currentFont(style.fontFamily, style.fontSize, style.boldHighlight ? QFont::Bold
                                                                            : QFont::Normal);
    QFont nextFont(style.fontFamily, qMax(10, style.fontSize - 4));

    m_currentLabel->setFont(currentFont);
    m_nextLabel->setFont(nextFont);

    const QString currentColor = style.highlightColor.name(QColor::HexArgb);
    const QString nextColor = style.normalColor.name(QColor::HexArgb);

    m_currentLabel->setStyleSheet(QStringLiteral("color: %1; background: transparent;").arg(currentColor));
    m_nextLabel->setStyleSheet(QStringLiteral("color: %1; background: transparent;").arg(nextColor));
    update();
}

void LyricsWindow::onLinesChanged(const QString &current, const QString &next)
{
    m_pendingCurrent = current;
    m_pendingNext = next;

    // 窗口处于“先清空、等刷新”阶段、还没显示、或已完全透明（悬停淡出）时不写文字，
    // 等窗口真正刷新过一帧后由 restorePendingText() 统一填进去
    if (!m_visibleState || m_restoreAfterPaint || isFadedOut())
        return;

    m_currentLabel->setText(current);
    m_nextLabel->setText(next);
    // 整窗重绘：paintEvent 会先擦底再画背景
    update();
}

void LyricsWindow::restorePendingText()
{
    if (!m_visibleState)
        return;
    if (m_currentLabel->text() == m_pendingCurrent && m_nextLabel->text() == m_pendingNext)
        return;

    m_currentLabel->setText(m_pendingCurrent);
    m_nextLabel->setText(m_pendingNext);
    update();
}

void LyricsWindow::onStyleChanged(const LyricsStyle &style)
{
    applyStyle(style);
}

void LyricsWindow::toggleVisible()
{
    if (m_visibleState) {
        m_visibleState = false;
        m_pollTimer->stop();
        m_restoreAfterPaint = false;
        fadeTo(0.0);
        QTimer::singleShot(220, this, [this]() {
            if (m_visibleState)
                return;
            hide();
            // 隐藏后清空窗口内容，下次显示时以“空白窗口”起步
            m_currentLabel->clear();
            m_nextLabel->clear();
        });
    } else {
        m_visibleState = true;
        show();
        prepareContentForShow();    // 清空 → 刷新一帧 → 稍后填文字
        fadeTo(1.0);
        m_pollTimer->start();
    }
}

void LyricsWindow::fadeTo(qreal opacity)
{
    // 已经在朝同一目标动画时不要重启：onPollCursor 每 300ms 调用一次，
    // 反复 stop()+start() 会让窗口长期停在半透明状态，把底层残影"按"在屏幕上
    if (m_fade->state() == QPropertyAnimation::Running) {
        if (qFuzzyCompare(m_fadeTarget + 1.0, opacity + 1.0))
            return;
    } else if (qFuzzyCompare(windowOpacity() + 1.0, opacity + 1.0)) {
        m_fadeTarget = opacity;
        return;
    }

    m_fadeTarget = opacity;
    m_fade->stop();
    m_fade->setStartValue(windowOpacity());
    m_fade->setEndValue(opacity);
    m_fade->start();
}

void LyricsWindow::enterEvent(NaviEnterEvent *event)
{
    m_cursorInside = true;
    if (m_visibleState && !m_dragging)
        m_hoverTimer->start();
    QWidget::enterEvent(event);
}

void LyricsWindow::leaveEvent(QEvent *event)
{
    m_cursorInside = false;
    m_hoverTimer->stop();
    QWidget::leaveEvent(event);
}

bool LyricsWindow::isFadedOut() const
{
    return !isVisible() || windowOpacity() <= 0.05;
}

void LyricsWindow::closeLyrics()
{
    // 主窗口退出前必须关掉：它是 parent=nullptr 的独立顶层窗口，
    // 只要它还可见，Qt 就不会因为"最后一个窗口关闭"而退出应用
    m_visibleState = false;
    m_adjusting = false;
    m_pollTimer->stop();
    m_hoverTimer->stop();
    m_fade->stop();
    setWindowOpacity(1.0);
    m_currentLabel->clear();
    m_nextLabel->clear();
    hide();
    close();
}

void LyricsWindow::setPositionAdjustEnabled(bool enabled)
{
    if (enabled == m_adjusting)
        return;

    m_adjusting = enabled;

    if (enabled) {
        m_visibleBeforeAdjust = m_visibleState;
        // 调整期间不自动淡出，并保证窗口可见、完全不透明
        m_hoverTimer->stop();
        m_pollTimer->stop();
        m_visibleState = true;
        show();
        raise();
        prepareContentForShow();
        m_fade->stop();
        setWindowOpacity(1.0);
        setCursor(Qt::SizeAllCursor);
        update();
    } else {
        unsetCursor();
        update();
        if (!m_visibleBeforeAdjust) {
            // 调整前没显示过：结束后恢复隐藏
            m_visibleState = false;
            fadeTo(0.0);
            QTimer::singleShot(220, this, [this]() {
                if (!m_visibleState)
                    hide();
            });
        } else {
            m_visibleState = true;
            fadeTo(1.0);
            m_pollTimer->start();
        }
    }
}

void LyricsWindow::prepareContentForShow()
{
    // 显示/重新出现流程：先清空窗口内所有文字 → 刷新一帧 → 再显示文字
    m_currentLabel->clear();
    m_nextLabel->clear();
    update();
    repaint();                  // 以“空内容”立即刷新一次

    m_restoreAfterPaint = true;
    QTimer::singleShot(250, this, &LyricsWindow::restorePendingText);   // 兜底
}

void LyricsWindow::onHoverTimer()
{
    if (!m_visibleState || m_dragging || m_adjusting)
        return;
    fadeTo(0.0);
    // 淡出完成后同样清空内容（这里只是 opacity=0，窗口并未 hide）
    QTimer::singleShot(220, this, [this]() {
        if (!m_visibleState || windowOpacity() > 0.05)
            return;
        m_currentLabel->clear();
        m_nextLabel->clear();
        repaint();
    });
}

void LyricsWindow::onPollCursor()
{
    if (m_adjusting)
        return;     // 调整位置期间不自动淡出/淡入

    const bool inside = frameGeometry().contains(QCursor::pos());

    if (m_visibleState) {
        if (inside && !m_dragging && windowOpacity() > 0.05) {
            fadeTo(0.0);
        } else if (!inside && windowOpacity() < 0.95) {
            // 从"完全透明"恢复：必须先空窗口刷新一帧，否则文字与窗口一起出现会留下残影
            if (isFadedOut())
                prepareContentForShow();
            fadeTo(1.0);
        }
    }
}

void LyricsWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragMoved = false;
        m_hoverTimer->stop();
        m_dragOffset = event->pos();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void LyricsWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        move(naviEventGlobalPos(event).toPoint() - m_dragOffset);
        m_dragMoved = true;
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void LyricsWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_dragging) {
        m_dragging = false;
        event->accept();

        // 调整模式下真正拖动过 → 结束调整（设置页按钮会自动恢复）
        if (m_adjusting && m_dragMoved) {
            m_dragMoved = false;
            Q_EMIT positionAdjustFinished();
            setPositionAdjustEnabled(false);
        }
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void LyricsWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 透明窗口不会自动擦底：先把整块清成全透明（Source 模式会替换掉旧像素，
    // 包括上一句歌词的字形），否则每次换句都是"叠加"而不是"覆盖"
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    painter.setBrush(QColor(0, 0, 0, 10));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 14, 14);

    // 空白帧已经画完（并会在本次事件循环里 flush 到窗口层），
    // 下一轮事件循环再把歌词文字填回去
    if (m_restoreAfterPaint) {
        m_restoreAfterPaint = false;
        QTimer::singleShot(50, this, &LyricsWindow::restorePendingText);
    }
}

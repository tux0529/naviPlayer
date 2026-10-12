/**
 * @file    qt_compat.h
 * @brief   Qt5.15 / Qt6 统一兼容层
 *
 * 说明：项目需要同时兼容 Qt 5.15 与 Qt 6.x，差异点集中在此文件中以宏/别名收敛，
 *       业务代码一律使用本文件提供的别名与辅助函数，避免散落的 #if。
 */
#ifndef NAVIPLAYER_QT_COMPAT_H
#define NAVIPLAYER_QT_COMPAT_H

#include <QtGlobal>

#include <QAudioFormat>
#include <QEvent>
#include <QPoint>
#include <QPointF>
#include <QRect>
#include <QString>
#include <QStringList>

class QWidget;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#  include <QGuiApplication>
#  include <QScreen>
#else
#  include <QApplication>
#  include <QDesktopWidget>
#endif

// ---------------------------------------------------------------------------
// 音频输出设备：Qt6 为 QAudioSink，Qt5 为 QAudioOutput
// ---------------------------------------------------------------------------
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#  include <QAudioSink>
using NaviAudioOutput = QAudioSink;
#else
#  include <QAudioOutput>
using NaviAudioOutput = QAudioOutput;
#endif

// ---------------------------------------------------------------------------
// 鼠标进入事件：Qt6 为 QEnterEvent，Qt5 仍为 QEvent
// ---------------------------------------------------------------------------
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#  include <QEnterEvent>
using NaviEnterEvent = QEnterEvent;
#else
using NaviEnterEvent = QEvent;
#endif

// ---------------------------------------------------------------------------
// 字符串分割行为枚举：Qt6 使用 Qt::SplitBehaviorFlags，Qt5.15 起也支持 Qt::KeepEmptyParts，
// 但 Qt5.14 及更早使用 QString::SplitBehavior，这里统一为 NaviSkipEmptyParts
// ---------------------------------------------------------------------------
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
constexpr auto NaviSkipEmptyParts = Qt::SkipEmptyParts;
constexpr auto NaviKeepEmptyParts = Qt::KeepEmptyParts;
#else
constexpr auto NaviSkipEmptyParts = QString::SkipEmptyParts;
constexpr auto NaviKeepEmptyParts = QString::KeepEmptyParts;
#endif

/**
 * @brief 统一配置 PCM 16bit 小端音频格式（与 ffmpeg 解码输出保持一致）
 */
inline void naviConfigurePcmFormat(QAudioFormat &format, int sampleRate, int channelCount)
{
    format.setSampleRate(sampleRate);
    format.setChannelCount(channelCount);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    format.setSampleFormat(QAudioFormat::Int16);
#else
    format.setCodec(QStringLiteral("audio/pcm"));
    format.setSampleSize(16);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);
#endif
}

/**
 * @brief 计算每毫秒字节数（用于位置换算）
 */
inline qint64 naviBytesPerMs(int sampleRate, int channelCount)
{
    // 16bit = 2 bytes / sample
    return (qint64)sampleRate * channelCount * 2 / 1000;
}

// ---------------------------------------------------------------------------
// 事件坐标访问：Qt6 的 QMouseEvent::pos() 返回 QPointF，Qt5 返回 QPoint
// ---------------------------------------------------------------------------
class QMouseEvent;
class QWheelEvent;
class QDropEvent;

inline QPointF naviEventPos(const QMouseEvent *event);
inline QPointF naviEventGlobalPos(const QMouseEvent *event);
inline QPointF naviWheelPos(const QWheelEvent *event);
inline QPointF naviDropPos(const QDropEvent *event);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#  include <QMouseEvent>
#  include <QWheelEvent>
#  include <QDropEvent>

inline QPointF naviEventPos(const QMouseEvent *event)
{
    return event ? event->position() : QPointF();
}

inline QPointF naviEventGlobalPos(const QMouseEvent *event)
{
    return event ? event->globalPosition() : QPointF();
}

inline QPointF naviWheelPos(const QWheelEvent *event)
{
    return event ? event->position() : QPointF();
}

inline QPointF naviDropPos(const QDropEvent *event)
{
    return event ? event->position() : QPointF();
}
#else
#  include <QMouseEvent>
#  include <QWheelEvent>
#  include <QDropEvent>

inline QPointF naviEventPos(const QMouseEvent *event)
{
    return event ? QPointF(event->pos()) : QPointF();
}

inline QPointF naviEventGlobalPos(const QMouseEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event ? event->globalPosition() : QPointF();
#else
    return event ? QPointF(event->globalPos()) : QPointF();
#endif
}

inline QPointF naviWheelPos(const QWheelEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event ? event->position() : QPointF();
#else
    return event ? QPointF(event->pos()) : QPointF();
#endif
}

inline QPointF naviDropPos(const QDropEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event ? event->position() : QPointF();
#else
    return event ? QPointF(event->pos()) : QPointF();
#endif
}
#endif // QT_VERSION >= 6

/**
 * @brief 获取窗口所在屏幕的可用区域（替代 Qt6 中移除的 QDesktopWidget）
 */
inline QRect naviAvailableGeometry(const QWidget *widget)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QScreen *screen = widget && widget->windowHandle() ? widget->windowHandle()->screen()
                                                       : QGuiApplication::primaryScreen();
    return screen ? screen->availableGeometry() : QRect();
#else
    QDesktopWidget *desktop = QApplication::desktop();
    if (!desktop)
        return QRect();
    if (widget) {
        const int index = desktop->screenNumber(widget);
        return desktop->availableGeometry(index);
    }
    return desktop->availableGeometry(QApplication::desktop()->primaryScreen());
#endif
}

#endif // NAVIPLAYER_QT_COMPAT_H

#ifndef QT_COMPAT_H
#define QT_COMPAT_H

#include <QtGlobal>
#include <QAudioFormat>
#include <QEvent>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QAudioSink>
#include <QEnterEvent>
using NaviAudioOutput = QAudioSink;
using NaviEnterEvent = QEnterEvent;
#else
#include <QAudioOutput>
using NaviAudioOutput = QAudioOutput;
using NaviEnterEvent = QEvent;
#endif

inline void configurePcm16StereoAudioFormat(QAudioFormat *format)
{
    if (!format) {
        return;
    }

    format->setSampleRate(44100);
    format->setChannelCount(2);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    format->setSampleFormat(QAudioFormat::Int16);
#else
    format->setCodec("audio/pcm");
    format->setSampleSize(16);
    format->setByteOrder(QAudioFormat::LittleEndian);
    format->setSampleType(QAudioFormat::SignedInt);
#endif
}

#endif // QT_COMPAT_H

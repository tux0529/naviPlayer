#include "audio_ringbuffer.h"
#include "core/qt_compat.h"

#include <QtGlobal>
#include <QDebug>
#include <cstring>

namespace {
constexpr int kWaitMs = 100;
// 缓冲不足时单次补零的最大字节数（约 11ms 44.1k/立体声/S16）
constexpr int kSilenceChunk = 2048;
}

AudioRingBuffer::AudioRingBuffer(int capacityBytes, QObject *parent)
    : QIODevice(parent)
    , m_capacity(qMax(4096, capacityBytes))
{
    m_data = new char[m_capacity];
}

AudioRingBuffer::~AudioRingBuffer()
{
    abort();
    delete[] m_data;
}

void AudioRingBuffer::abort()
{
    QMutexLocker locker(&m_mutex);
    m_aborted = true;
    m_notEmpty.wakeAll();
    m_notFull.wakeAll();
}

void AudioRingBuffer::resetForSeek()
{
    QMutexLocker locker(&m_mutex);
    m_head = 0;
    m_count = 0;
    m_endOfStream = false;
    m_notFull.wakeAll();
}

void AudioRingBuffer::setEndOfStream(bool end)
{
    QMutexLocker locker(&m_mutex);
    m_endOfStream = end;
    m_notEmpty.wakeAll();
}

bool AudioRingBuffer::endOfStream() const
{
    QMutexLocker locker(&m_mutex);
    return m_endOfStream;
}

qint64 AudioRingBuffer::bufferedBytes() const
{
    QMutexLocker locker(&m_mutex);
    return m_count;
}

int AudioRingBuffer::bufferedPercent() const
{
    QMutexLocker locker(&m_mutex);
    return static_cast<int>((qint64)m_count * 100 / m_capacity);
}

qint64 AudioRingBuffer::bytesAvailable() const
{
    QMutexLocker locker(&m_mutex);
    return m_count + QIODevice::bytesAvailable();
}

bool AudioRingBuffer::atEnd() const
{
    QMutexLocker locker(&m_mutex);
    return m_count == 0 && m_endOfStream;
}

qint64 AudioRingBuffer::writeData(const char *data, qint64 maxSize)
{
    if (maxSize <= 0)
        return 0;

    qint64 written = 0;
    QMutexLocker locker(&m_mutex);

    while (written < maxSize) {
        while (m_count == m_capacity) {
            if (m_aborted)
                return written > 0 ? written : -1;
            if (!m_notFull.wait(&m_mutex, kWaitMs)) {
                // 等待超时：若仍满且已终止则返回，否则继续等
                if (m_aborted)
                    return written > 0 ? written : -1;
            }
        }

        const int tail = (m_head + m_count) % m_capacity;
        const qint64 space = qMin<qint64>(maxSize - written, m_capacity - m_count);
        const int firstChunk = qMin<qint64>(space, m_capacity - tail);
        memcpy(m_data + tail, data + written, static_cast<size_t>(firstChunk));
        if (space > firstChunk) {
            memcpy(m_data, data + written + firstChunk, static_cast<size_t>(space - firstChunk));
        }

        m_count += static_cast<int>(space);
        written += space;
        m_totalWritten += space;
        m_notEmpty.wakeAll();
    }

    return written;
}

qint64 AudioRingBuffer::readData(char *data, qint64 maxSize)
{
    if (maxSize <= 0)
        return 0;

    QMutexLocker locker(&m_mutex);

    if (m_count == 0) {
        // 读取可能发生在主线程（QAudioOutput::start）或音频线程，
        // 这里只能有限等待：没有数据就输出静音，绝不能一直阻塞调用线程
        if (!m_aborted && !m_endOfStream)
            m_notEmpty.wait(&m_mutex, kWaitMs);
        if (m_count == 0) {
            if (m_aborted || m_endOfStream)
                return 0;
            const qint64 fill = qMin<qint64>(maxSize, kSilenceChunk);
            memset(data, 0, static_cast<size_t>(fill));
            return fill;
        }
    }

    const qint64 toRead = qMin<qint64>(maxSize, m_count);
    const int firstChunk = qMin<qint64>(toRead, m_capacity - m_head);
    memcpy(data, m_data + m_head, static_cast<size_t>(firstChunk));
    if (toRead > firstChunk) {
        memcpy(data + firstChunk, m_data, static_cast<size_t>(toRead - firstChunk));
    }

    m_head = (m_head + static_cast<int>(toRead)) % m_capacity;
    m_count -= static_cast<int>(toRead);
    m_totalRead += toRead;
    m_notFull.wakeAll();

    if (m_count == 0 && !m_underflow) {
        m_underflow = true;
        locker.unlock();
        Q_EMIT underflowChanged(true);
    } else if (m_count > m_capacity / 4 && m_underflow) {
        m_underflow = false;
        locker.unlock();
        Q_EMIT underflowChanged(false);
    }

    return toRead;
}

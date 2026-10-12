/**
 * @file    audio_ringbuffer.h
 * @brief   环形缓冲 QIODevice：解码线程写入，Qt 音频输出设备主动拉取
 *
 * - 写入端满水阻塞（背压），避免内存无限增长
 * - 读取端无水阻塞（有限等待），避免设备提前判定 EOF
 * - seek 时整体清空并重置计数
 */
#ifndef NAVIPLAYER_AUDIO_RINGBUFFER_H
#define NAVIPLAYER_AUDIO_RINGBUFFER_H

#include <QIODevice>
#include <QMutex>
#include <QWaitCondition>
#include <QObject>

class AudioRingBuffer : public QIODevice
{
    Q_OBJECT
public:
    explicit AudioRingBuffer(int capacityBytes, QObject *parent = nullptr);
    ~AudioRingBuffer() override;

    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;
    qint64 bytesAvailable() const override;
    bool isSequential() const override { return true; }
    bool atEnd() const override;
    bool canReadLine() const override { return false; }

    /** 清空缓冲区（seek 或切歌时调用） */
    void resetForSeek();
    /** 标记解码结束（读取端耗尽后返回 EOF） */
    void setEndOfStream(bool end);
    bool endOfStream() const;
    /** 唤醒所有阻塞的读写（stop 时调用） */
    void abort();

    int capacity() const { return m_capacity; }
    qint64 bufferedBytes() const;
    /** 缓冲占比 0~100 */
    int bufferedPercent() const;
    qint64 totalWritten() const { return m_totalWritten; }
    qint64 totalRead() const { return m_totalRead; }

signals:
    /** 缓冲不足（播放可能卡顿）状态变化 */
    void underflowChanged(bool underflow);

private:
    const int m_capacity;
    char *m_data = nullptr;
    int m_head = 0;         // 读取位置
    int m_count = 0;        // 当前字节数
    mutable QMutex m_mutex;
    QWaitCondition m_notEmpty;
    QWaitCondition m_notFull;
    bool m_endOfStream = false;
    bool m_aborted = false;
    qint64 m_totalWritten = 0;
    qint64 m_totalRead = 0;
    bool m_underflow = false;
};

#endif // NAVIPLAYER_AUDIO_RINGBUFFER_H

#include "core/ffmpegaudiodecoder.h"

#include <QDebug>
#include <QFileInfo>

DecodeThread::DecodeThread(QObject *parent)
    : QThread(parent)
{
}

DecodeThread::~DecodeThread()
{
    requestStop();
    wait(3000);
    closeInput();
}

void DecodeThread::configure(const QString &source, qint64 startPositionMs, AudioRingBuffer *buffer,
                             quint64 token)
{
    QMutexLocker locker(&m_mutex);
    m_source = source;
    m_startPositionMs = startPositionMs;
    m_ring = buffer;
    m_token = token;
    // 每次开始新的解码都必须复位状态，否则上一次 stop 残留的标志会让解码循环立即退出
    m_stopRequested = false;
    m_paused = false;
    m_seekTarget = -1;
}

quint64 DecodeThread::nextToken()
{
    static quint64 counter = 0;
    return ++counter;
}

void DecodeThread::requestSeek(qint64 positionMs)
{
    QMutexLocker locker(&m_mutex);
    m_seekTarget = positionMs;
    m_pauseCondition.wakeAll();
}

void DecodeThread::requestStop()
{
    QMutexLocker locker(&m_mutex);
    m_stopRequested = true;
    m_paused = false;
    m_pauseCondition.wakeAll();
}

void DecodeThread::setPaused(bool paused)
{
    QMutexLocker locker(&m_mutex);
    m_paused = paused;
    m_pauseCondition.wakeAll();
}

bool DecodeThread::isPaused() const
{
    QMutexLocker locker(&m_mutex);
    return m_paused;
}

bool DecodeThread::openInput()
{
    m_formatCtx = avformat_alloc_context();
    if (!m_formatCtx) {
        Q_EMIT errorOccurred(QStringLiteral("无法分配 AVFormatContext"));
        return false;
    }

    if (avformat_open_input(&m_formatCtx, m_source.toUtf8().constData(), nullptr, nullptr) < 0) {
        Q_EMIT errorOccurred(QStringLiteral("无法打开音频源: %1").arg(m_source));
        avformat_free_context(m_formatCtx);
        m_formatCtx = nullptr;
        return false;
    }

    if (avformat_find_stream_info(m_formatCtx, nullptr) < 0) {
        Q_EMIT errorOccurred(QStringLiteral("无法获取流信息"));
        closeInput();
        return false;
    }

    // 选择音频流
    m_audioStream = -1;
    for (unsigned i = 0; i < m_formatCtx->nb_streams; ++i) {
        if (m_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            m_audioStream = static_cast<int>(i);
            break;
        }
    }
    if (m_audioStream < 0) {
        Q_EMIT errorOccurred(QStringLiteral("未找到音频流"));
        closeInput();
        return false;
    }

    collectEmbeddedLyrics();

    const AVCodecParameters *par = m_formatCtx->streams[m_audioStream]->codecpar;
    m_codec = avcodec_find_decoder(par->codec_id);
    if (!m_codec) {
        Q_EMIT errorOccurred(QStringLiteral("未找到解码器"));
        closeInput();
        return false;
    }

    m_codecCtx = avcodec_alloc_context3(m_codec);
    if (!m_codecCtx || avcodec_parameters_to_context(m_codecCtx, par) < 0) {
        Q_EMIT errorOccurred(QStringLiteral("无法初始化解码上下文"));
        closeInput();
        return false;
    }

    if (avcodec_open2(m_codecCtx, m_codec, nullptr) < 0) {
        Q_EMIT errorOccurred(QStringLiteral("无法打开解码器"));
        closeInput();
        return false;
    }

    // 重采样：统一输出 44.1kHz / 立体声 / S16
    m_swrCtx = swr_alloc();
    if (!m_swrCtx || naviConfigureSwr(m_swrCtx, m_codecCtx, FFmpegAudioDecoder::kOutputSampleRate,
                                      FFmpegAudioDecoder::kOutputChannels) < 0) {
        Q_EMIT errorOccurred(QStringLiteral("无法初始化重采样器"));
        closeInput();
        return false;
    }

    return true;
}

void DecodeThread::collectEmbeddedLyrics()
{
    static const char *keys[] = {"lyrics", "lyrics-eng", "unsyncedlyrics", "syncedlyrics",
                                 "uslt", "sylt", "comment"};
    for (const char *key : keys) {
        if (m_formatCtx->metadata) {
            const AVDictionaryEntry *entry = av_dict_get(m_formatCtx->metadata, key, nullptr,
                                                         AV_DICT_IGNORE_SUFFIX);
            if (entry && entry->value && strlen(entry->value) > 0) {
                Q_EMIT embeddedLyricsFound(QString::fromUtf8(entry->value));
                return;
            }
        }
        for (unsigned i = 0; i < m_formatCtx->nb_streams; ++i) {
            AVStream *stream = m_formatCtx->streams[i];
            if (!stream->metadata)
                continue;
            const AVDictionaryEntry *entry = av_dict_get(stream->metadata, key, nullptr,
                                                         AV_DICT_IGNORE_SUFFIX);
            if (entry && entry->value && strlen(entry->value) > 0) {
                Q_EMIT embeddedLyricsFound(QString::fromUtf8(entry->value));
                return;
            }
        }
    }
}

void DecodeThread::closeInput()
{
    if (m_swrCtx) {
        swr_free(&m_swrCtx);
        m_swrCtx = nullptr;
    }
    if (m_codecCtx) {
        avcodec_free_context(&m_codecCtx);
        m_codecCtx = nullptr;
    }
    if (m_formatCtx) {
        avformat_close_input(&m_formatCtx);
        m_formatCtx = nullptr;
    }
    m_codec = nullptr;
    m_audioStream = -1;
}

bool DecodeThread::handleSeek()
{
    QMutexLocker locker(&m_mutex);
    const qint64 target = m_seekTarget;
    m_seekTarget = -1;

    if (target < 0 || !m_formatCtx)
        return false;

    const int64_t ts = static_cast<int64_t>(target) * AV_TIME_BASE / 1000;
    int64_t minTs = INT64_MIN;
    int64_t maxTs = ts;
    if (avformat_seek_file(m_formatCtx, -1, minTs, ts, maxTs, 0) < 0)
        return false;

    if (m_codecCtx)
        avcodec_flush_buffers(m_codecCtx);
    return true;
}

bool DecodeThread::writeToBuffer(const uint8_t *data, int size)
{
    if (!m_ring || size <= 0)
        return false;

    int written = 0;
    while (written < size) {
        {
            QMutexLocker locker(&m_mutex);
            if (m_stopRequested)
                return false;
        }
        const int n = static_cast<int>(m_ring->write(reinterpret_cast<const char *>(data) + written,
                                                     size - written));
        if (n <= 0)
            return false;
        written += n;
    }
    return true;
}

void DecodeThread::run()
{
    quint64 token = 0;
    {
        QMutexLocker locker(&m_mutex);
        token = m_token;
    }

    if (!openInput()) {
        Q_EMIT opened(false, 0, 0, 0, 0, token);
        return;
    }

    qint64 durationMs = 0;
    if (m_formatCtx->duration > 0)
        durationMs = m_formatCtx->duration / (AV_TIME_BASE / 1000);

    int bitRate = 0;
    if (m_codecCtx && m_codecCtx->bit_rate > 0)
        bitRate = static_cast<int>(m_codecCtx->bit_rate / 1000);
    else if (m_formatCtx->bit_rate > 0)
        bitRate = static_cast<int>(m_formatCtx->bit_rate / 1000);

    Q_EMIT opened(true, durationMs, bitRate, FFmpegAudioDecoder::kOutputSampleRate,
                  FFmpegAudioDecoder::kOutputChannels, token);

    if (m_startPositionMs > 0) {
        QMutexLocker locker(&m_mutex);
        m_seekTarget = m_startPositionMs;
    }
    if (m_seekTarget >= 0) {
        handleSeek();
        if (m_ring)
            m_ring->resetForSeek();
    }

    decodeLoop();

    closeInput();
    Q_EMIT streamFinished(token);
}

void DecodeThread::decodeLoop()
{
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    if (!packet || !frame) {
        if (packet) av_packet_free(&packet);
        if (frame) av_frame_free(&frame);
        return;
    }

    constexpr int kOutSamples = 8192;
    uint8_t *outData[2] = {nullptr, nullptr};
    const int lineSize = av_samples_alloc(outData, nullptr, FFmpegAudioDecoder::kOutputChannels,
                                          kOutSamples, AV_SAMPLE_FMT_S16, 0);
    if (lineSize < 0 || !outData[0]) {
        av_packet_free(&packet);
        av_frame_free(&frame);
        return;
    }

    const int outBytesPerSample = 2 * FFmpegAudioDecoder::kOutputChannels;
    bool stop = false;

    while (!stop) {
        {
            QMutexLocker locker(&m_mutex);
            if (m_stopRequested)
                break;
            while (m_paused && !m_stopRequested)
                m_pauseCondition.wait(&m_mutex, 100);
            if (m_stopRequested)
                break;
        }

        if (m_seekTarget >= 0) {
            handleSeek();
            if (m_ring)
                m_ring->resetForSeek();
        }

        const int readRet = av_read_frame(m_formatCtx, packet);
        if (readRet < 0) {
            if (readRet == AVERROR_EOF) {
                if (m_ring)
                    m_ring->setEndOfStream(true);
                break;
            }
            msleep(5);
            continue;
        }

        if (packet->stream_index != m_audioStream) {
            av_packet_unref(packet);
            continue;
        }

        const int sendRet = avcodec_send_packet(m_codecCtx, packet);
        av_packet_unref(packet);
        if (sendRet < 0)
            continue;

        while (!stop) {
            const int recvRet = avcodec_receive_frame(m_codecCtx, frame);
            if (recvRet == AVERROR(EAGAIN) || recvRet == AVERROR_EOF)
                break;
            if (recvRet < 0)
                break;

            const int samples = swr_convert(m_swrCtx, outData, kOutSamples,
                                            const_cast<const uint8_t **>(frame->data),
                                            frame->nb_samples);
            av_frame_unref(frame);

            if (samples > 0) {
                if (!writeToBuffer(outData[0], samples * outBytesPerSample)) {
                    stop = true;
                    break;
                }
            }

            QMutexLocker locker(&m_mutex);
            if (m_stopRequested) {
                stop = true;
                break;
            }
            if (m_paused)
                break;      // 回到外层循环等待继续
        }
    }

    if (outData[0])
        av_freep(&outData[0]);
    av_packet_free(&packet);
    av_frame_free(&frame);
}

// ---------------------------------------------------------- FFmpegAudioDecoder
FFmpegAudioDecoder::FFmpegAudioDecoder(QObject *parent)
    : QObject(parent)
{
    m_thread = new DecodeThread(this);

    connect(m_thread, &DecodeThread::opened, this, &FFmpegAudioDecoder::onThreadOpened);
    connect(m_thread, &DecodeThread::streamFinished, this, [this](quint64 token) {
        if (token != m_token)
            return;     // 旧解码线程迟到的结束信号
        m_decoding = false;
        Q_EMIT finished();
    });
    connect(m_thread, &DecodeThread::errorOccurred, this, [this](const QString &msg) {
        Q_EMIT errorOccurred(msg);
    });
    connect(m_thread, &DecodeThread::embeddedLyricsFound, this, [this](const QString &lyrics) {
        Q_EMIT embeddedLyricsFound(lyrics);
    });
}

FFmpegAudioDecoder::~FFmpegAudioDecoder()
{
    stop();
    if (m_thread) {
        m_thread->requestStop();
        m_thread->wait(3000);
    }
}

void FFmpegAudioDecoder::setRingBuffer(AudioRingBuffer *buffer)
{
    m_ring = buffer;
}

void FFmpegAudioDecoder::start(const QString &source, qint64 startPositionMs)
{
    if (!m_thread)
        return;
    if (m_thread->isRunning()) {
        // 上一次解码线程尚未退出：先停止并等待，避免新旧线程共用缓冲区
        m_thread->requestStop();
        m_thread->wait(1000);
    }

    m_token = m_thread->nextToken();
    m_thread->configure(source, startPositionMs, m_ring, m_token);
    m_decoding = true;
    m_thread->start();
}

void FFmpegAudioDecoder::pause()
{
    if (m_thread)
        m_thread->setPaused(true);
}

void FFmpegAudioDecoder::resume()
{
    if (m_thread)
        m_thread->setPaused(false);
}

void FFmpegAudioDecoder::stop()
{
    if (m_thread) {
        m_thread->setPaused(false);
        m_thread->requestStop();
    }
    if (m_ring)
        m_ring->abort();
    m_decoding = false;
}

void FFmpegAudioDecoder::seek(qint64 positionMs)
{
    if (m_thread)
        m_thread->requestSeek(positionMs);
}

bool FFmpegAudioDecoder::waitFinished(int timeoutMs)
{
    if (!m_thread)
        return true;
    if (!m_thread->isRunning())
        return true;
    return m_thread->wait(timeoutMs);
}

void FFmpegAudioDecoder::onThreadOpened(bool ok, qint64 durationMs, int bitRate, int sampleRate,
                                        int channels, quint64 token)
{
    if (token != m_token)
        return;     // 旧解码线程迟到的信号

    m_durationMs = durationMs;
    m_bitRate = bitRate;
    if (sampleRate > 0)
        m_outputSampleRate = sampleRate;
    if (channels > 0)
        m_outputChannels = channels;
    Q_EMIT opened(ok, durationMs);
}

QString FFmpegAudioDecoder::extractEmbeddedLyrics(const QString &filePath)
{
    if (filePath.isEmpty() || !QFileInfo::exists(filePath))
        return QString();

    AVFormatContext *fmt = avformat_alloc_context();
    if (!fmt)
        return QString();

    QString result;
    if (avformat_open_input(&fmt, filePath.toUtf8().constData(), nullptr, nullptr) >= 0) {
        if (avformat_find_stream_info(fmt, nullptr) >= 0) {
            static const char *keys[] = {"lyrics", "lyrics-eng", "unsyncedlyrics",
                                         "syncedlyrics", "uslt", "sylt", "comment"};
            for (const char *key : keys) {
                const AVDictionaryEntry *entry = av_dict_get(fmt->metadata, key, nullptr,
                                                             AV_DICT_IGNORE_SUFFIX);
                if (!entry && fmt->nb_streams > 0)
                    entry = av_dict_get(fmt->streams[0]->metadata, key, nullptr, AV_DICT_IGNORE_SUFFIX);
                if (entry && entry->value && strlen(entry->value) > 0) {
                    result = QString::fromUtf8(entry->value);
                    break;
                }
            }
        }
        avformat_close_input(&fmt);
    } else {
        avformat_free_context(fmt);
    }

    return result;
}

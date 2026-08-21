#include "ffmpegaudiodecoder.h"
#include <QDebug>
#include <QBuffer>
#include <QThread>

// 解码线程实现
DecodeThread::DecodeThread(const QString &url, QIODevice *audioDevice, QObject *parent,
                           qint64 startPositionMs, bool startPaused)
    : QThread(parent)
    , m_url(url)
    , m_audioDevice(audioDevice)
    , m_isStopped(false)
    , m_isPaused(false)
    , m_startPositionMs(startPositionMs)
    , m_startPaused(startPaused)
{
}

DecodeThread::~DecodeThread()
{
    stop();
    wait();
}

void DecodeThread::stop()
{
    QMutexLocker locker(&m_mutex);
    m_isStopped = true;
    m_waitCondition.wakeAll();
}

void DecodeThread::pause()
{
    QMutexLocker locker(&m_mutex);
    m_isPaused = true;
}

void DecodeThread::resume()
{
    QMutexLocker locker(&m_mutex);
    m_isPaused = false;
    m_waitCondition.wakeAll();
}

void DecodeThread::run()
{
    AVFormatContext *formatCtx = nullptr;
    AVCodecContext *codecCtx = nullptr;
    SwrContext *swrCtx = nullptr;
    int audioStreamIndex = -1;

    // 打开输入
    if (avformat_open_input(&formatCtx, m_url.toUtf8().constData(), nullptr, nullptr) < 0) {
        emit errorOccurred("Failed to open audio file");
        return;
    }

    // 查找流信息
    if (avformat_find_stream_info(formatCtx, nullptr) < 0) {
        emit errorOccurred("Failed to find stream info");
        avformat_close_input(&formatCtx);
        return;
    }

    // 查找音频流
    for (unsigned int i = 0; i < formatCtx->nb_streams; i++) {
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = i;
            break;
        }
    }

    if (audioStreamIndex == -1) {
        emit errorOccurred("No audio stream found");
        avformat_close_input(&formatCtx);
        return;
    }

    // 查找解码器
    AVCodecParameters *codecpar = formatCtx->streams[audioStreamIndex]->codecpar;
    const AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);
    if (!codec) {
        emit errorOccurred("Failed to find codec");
        avformat_close_input(&formatCtx);
        return;
    }

    // 创建解码上下文
    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        emit errorOccurred("Failed to allocate codec context");
        avformat_close_input(&formatCtx);
        return;
    }

    // 复制参数
    if (avcodec_parameters_to_context(codecCtx, codecpar) < 0) {
        emit errorOccurred("Failed to copy codec parameters");
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return;
    }

    // 打开解码器
    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
        emit errorOccurred("Failed to open codec");
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return;
    }

    // 设置重采样器
    AVChannelLayout outChLayout = AV_CHANNEL_LAYOUT_STEREO;
    AVSampleFormat outSampleFmt = AV_SAMPLE_FMT_S16;
    int outSampleRate = 44100;

    swrCtx = swr_alloc();
    if (!swrCtx) {
        emit errorOccurred("Failed to allocate resampler");
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return;
    }

    av_opt_set_chlayout(swrCtx, "in_chlayout", &codecCtx->ch_layout, 0);
    av_opt_set_int(swrCtx, "in_sample_rate", codecCtx->sample_rate, 0);
    av_opt_set_sample_fmt(swrCtx, "in_sample_fmt", codecCtx->sample_fmt, 0);
    av_opt_set_chlayout(swrCtx, "out_chlayout", &outChLayout, 0);
    av_opt_set_int(swrCtx, "out_sample_rate", outSampleRate, 0);
    av_opt_set_sample_fmt(swrCtx, "out_sample_fmt", outSampleFmt, 0);

    if (swr_init(swrCtx) < 0) {
        emit errorOccurred("Failed to initialize resampler");
        swr_free(&swrCtx);
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return;
    }

    // 获取并发送总时长
    qint64 totalDuration = (formatCtx->duration / AV_TIME_BASE) * 1000;
    emit durationChanged(totalDuration);

    qint64 currentPosition = 0;

    // 恢复播放进度：seek 到起始位置并刷新解码器
    if (m_startPositionMs > 0) {
        const int64_t seekTargetUs = static_cast<int64_t>(m_startPositionMs) * 1000;
        if (avformat_seek_file(formatCtx, -1, 0, seekTargetUs, seekTargetUs,
                               AVSEEK_FLAG_BACKWARD) >= 0) {
            avcodec_flush_buffers(codecCtx);
            currentPosition = m_startPositionMs;
            emit positionChanged(currentPosition);
        } else {
            qDebug() << "Seek to" << m_startPositionMs << "ms failed, start from beginning";
        }
    }

    // 启动即暂停：用于恢复时停留在指定进度，等待用户继续播放
    if (m_startPaused) {
        QMutexLocker locker(&m_mutex);
        m_isPaused = true;
    }

    AVPacket *pkt = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    // 解码循环
    while (!isInterruptionRequested()) {
        m_mutex.lock();
        if (m_isStopped) {
            m_mutex.unlock();
            break;
        }

        if (m_isPaused) {
            m_waitCondition.wait(&m_mutex);
            m_mutex.unlock();
            continue;
        }
        m_mutex.unlock();

        if (av_read_frame(formatCtx, pkt) < 0) {
            break;
        }

        if (pkt->stream_index == audioStreamIndex) {
            if (avcodec_send_packet(codecCtx, pkt) < 0) {
                break;
            }

            while (avcodec_receive_frame(codecCtx, frame) == 0) {
                // 解码和转换
                uint8_t *outBuffer = nullptr;
                int outSamples = swr_get_out_samples(swrCtx, frame->nb_samples);

                if (av_samples_alloc(&outBuffer, nullptr, 2, outSamples, AV_SAMPLE_FMT_S16, 0) >= 0) {
                    int outSamplesConverted = swr_convert(swrCtx, &outBuffer, outSamples,
                                                          (const uint8_t **)frame->data, frame->nb_samples);

                    if (outSamplesConverted > 0 && m_audioDevice) {
                        int outSize = outSamplesConverted * 2 * sizeof(int16_t);
                        const char *dataPtr = reinterpret_cast<const char *>(outBuffer);
                        qint64 bytesWritten = 0;
                        
                        while (bytesWritten < outSize && !isInterruptionRequested()) {
                            m_mutex.lock();
                            if (m_isStopped) {
                                m_mutex.unlock();
                                break;
                            }
                            if (m_isPaused) {
                                m_waitCondition.wait(&m_mutex);
                                m_mutex.unlock();
                                continue;
                            }
                            m_mutex.unlock();
                            
                            qint64 chunkSize = m_audioDevice->write(dataPtr + bytesWritten, outSize - bytesWritten);
                            if (chunkSize <= 0) {
                                QThread::msleep(10);
                                continue;
                            }
                            bytesWritten += chunkSize;
                        }
                    }

                    av_freep(&outBuffer);
                }

                // 更新当前位置
                qint64 pts = frame->pts;
                if (pts == AV_NOPTS_VALUE) {
                    pts = frame->pkt_dts;
                }

                if (pts != AV_NOPTS_VALUE) {
                    AVRational timebase = formatCtx->streams[audioStreamIndex]->time_base;
                    qint64 ptsMs = (pts * 1000 * timebase.num) / timebase.den;
                    if (ptsMs > currentPosition) {
                        currentPosition = ptsMs;
                        emit positionChanged(currentPosition);
                    }
                }
            }
        }

        av_packet_unref(pkt);
    }

    // 刷新解码器
    avcodec_send_packet(codecCtx, nullptr);
    while (avcodec_receive_frame(codecCtx, frame) == 0 && !isInterruptionRequested()) {
        uint8_t *outBuffer = nullptr;
        int outSamples = swr_get_out_samples(swrCtx, frame->nb_samples);

        if (av_samples_alloc(&outBuffer, nullptr, 2, outSamples, AV_SAMPLE_FMT_S16, 0) >= 0) {
            int outSamplesConverted = swr_convert(swrCtx, &outBuffer, outSamples,
                                                  (const uint8_t **)frame->data, frame->nb_samples);

            if (outSamplesConverted > 0 && m_audioDevice) {
                int outSize = outSamplesConverted * 2 * sizeof(int16_t);
                const char *dataPtr = reinterpret_cast<const char *>(outBuffer);
                qint64 bytesWritten = 0;
                
                while (bytesWritten < outSize && !isInterruptionRequested()) {
                    m_mutex.lock();
                    if (m_isStopped) {
                        m_mutex.unlock();
                        break;
                    }
                    if (m_isPaused) {
                        m_waitCondition.wait(&m_mutex);
                        m_mutex.unlock();
                        continue;
                    }
                    m_mutex.unlock();
                    
                    qint64 chunkSize = m_audioDevice->write(dataPtr + bytesWritten, outSize - bytesWritten);
                    if (chunkSize <= 0) {
                        QThread::msleep(10);
                        continue;
                    }
                    bytesWritten += chunkSize;
                }
            }

            av_freep(&outBuffer);
        }
    }

    // 清理资源
    av_frame_free(&frame);
    av_packet_free(&pkt);
    swr_free(&swrCtx);
    avcodec_free_context(&codecCtx);
    avformat_close_input(&formatCtx);

    emit finished();
}

// 主解码器实现
FFmpegAudioDecoder::FFmpegAudioDecoder(QObject *parent)
    : QObject(parent)
    , m_decodeThread(nullptr)
    , m_audioSink(nullptr)
    , m_audioDevice(nullptr)
    , m_isPlaying(false)
    , m_isPaused(false)
    , m_currentPosition(0)
    , m_totalDuration(0)
    , m_volume(80)
    , m_playbackToken(0)
    , m_positionFromPts(false)
    , m_positionTimer(nullptr)
{
    m_positionTimer = new QTimer(this);
    connect(m_positionTimer, &QTimer::timeout, this, &FFmpegAudioDecoder::updatePlaybackPosition);
}

FFmpegAudioDecoder::~FFmpegAudioDecoder()
{
    stop();
    cleanup();
}

bool FFmpegAudioDecoder::open(const QString &url)
{
    // 先清理旧的播放
    stop();
    cleanup();

    m_currentUrl = url;
    m_currentPosition = 0;
    m_totalDuration = 0;

    return true;
}

void FFmpegAudioDecoder::close()
{
    stop();
    cleanup();
}

void FFmpegAudioDecoder::play()
{
    if (m_isPlaying && !m_isPaused) {
        return;
    }

    if (m_isPaused && m_decodeThread) {
        m_decodeThread->resume();
        m_isPaused = false;
        // 恢复进度场景下进度由 pts 驱动，不轮询 audioSink 进度
        if (!m_positionFromPts) {
            m_positionTimer->start(100);
        }
        emit stateChanged(true);
        return;
    }

    // 开始新的播放
    m_isPlaying = true;
    m_isPaused = false;
    m_positionFromPts = false;
    m_currentPosition = 0;
    const quint64 playbackToken = ++m_playbackToken;

    // 设置音频输出格式
    configurePcm16StereoAudioFormat(&m_audioFormat);

    setupAudioOutput();

    // 创建解码线程
    m_decodeThread = new DecodeThread(m_currentUrl, m_audioDevice, this);
    connect(m_decodeThread, &DecodeThread::positionChanged, this, [this, playbackToken](qint64 pos) {
        if (playbackToken != m_playbackToken) {
            return;
        }
        m_currentPosition = pos;
        emit positionChanged(pos);
    });
    connect(m_decodeThread, &DecodeThread::durationChanged, this, [this, playbackToken](qint64 duration) {
        if (playbackToken != m_playbackToken) {
            return;
        }
        m_totalDuration = duration;
        emit durationChanged(duration);
    });
    connect(m_decodeThread, &DecodeThread::finished, this, [this, playbackToken]() {
        if (playbackToken != m_playbackToken) {
            return;
        }
        m_isPlaying = false;
        m_isPaused = false;
        m_positionTimer->stop();
        emit stateChanged(false);
        emit finished();
    });
    connect(m_decodeThread, &DecodeThread::errorOccurred, this, [this, playbackToken](const QString &error) {
        if (playbackToken != m_playbackToken) {
            return;
        }
        emit errorOccurred(error);
    });

    m_decodeThread->start();
    m_positionTimer->start(100);
    emit stateChanged(true);
}

void FFmpegAudioDecoder::playPaused(const QString &url, qint64 startPositionMs)
{
    stop();
    cleanup();

    m_currentUrl = url;
    m_currentPosition = startPositionMs;
    m_totalDuration = 0;

    m_isPlaying = true;
    m_isPaused = true;
    m_positionFromPts = true;
    const quint64 playbackToken = ++m_playbackToken;

    configurePcm16StereoAudioFormat(&m_audioFormat);
    setupAudioOutput();

    m_decodeThread = new DecodeThread(m_currentUrl, m_audioDevice, this,
                                      startPositionMs, true);
    connect(m_decodeThread, &DecodeThread::positionChanged, this, [this, playbackToken](qint64 pos) {
        if (playbackToken != m_playbackToken) {
            return;
        }
        m_currentPosition = pos;
        emit positionChanged(pos);
    });
    connect(m_decodeThread, &DecodeThread::durationChanged, this, [this, playbackToken](qint64 duration) {
        if (playbackToken != m_playbackToken) {
            return;
        }
        m_totalDuration = duration;
        emit durationChanged(duration);
    });
    connect(m_decodeThread, &DecodeThread::finished, this, [this, playbackToken]() {
        if (playbackToken != m_playbackToken) {
            return;
        }
        m_isPlaying = false;
        m_isPaused = false;
        m_positionFromPts = false;
        m_positionTimer->stop();
        emit stateChanged(false);
        emit finished();
    });
    connect(m_decodeThread, &DecodeThread::errorOccurred, this, [this, playbackToken](const QString &error) {
        if (playbackToken != m_playbackToken) {
            return;
        }
        emit errorOccurred(error);
    });

    m_decodeThread->start();
    // 暂停状态下不轮询 audioSink 进度，进度完全由解码线程 pts 驱动
    emit stateChanged(false);
}

void FFmpegAudioDecoder::pause()
{
    if (m_isPlaying && !m_isPaused && m_decodeThread) {
        m_isPaused = true;
        m_decodeThread->pause();
        m_positionTimer->stop();
        emit stateChanged(false);
    }
}

void FFmpegAudioDecoder::resume()
{
    play();
}

void FFmpegAudioDecoder::stop()
{
    ++m_playbackToken;

    if (m_decodeThread) {
        disconnect(m_decodeThread, nullptr, this, nullptr);
        m_decodeThread->stop();
        m_decodeThread->requestInterruption();
        m_decodeThread->wait();
        delete m_decodeThread;
        m_decodeThread = nullptr;
    }

    if (m_positionTimer) {
        m_positionTimer->stop();
    }

    if (m_audioSink) {
        m_audioSink->stop();
    }

    m_isPlaying = false;
    m_isPaused = false;
    m_positionFromPts = false;
    m_currentPosition = 0;
    emit stateChanged(false);
}

void FFmpegAudioDecoder::seek(qint64 positionMs)
{
    // 暂不实现seek功能
    Q_UNUSED(positionMs);
}

void FFmpegAudioDecoder::setVolume(int volume)
{
    m_volume = qBound(0, volume, 100);
    if (m_audioSink) {
        m_audioSink->setVolume(qreal(m_volume) / 100.0);
    }
}

int FFmpegAudioDecoder::volume() const
{
    return m_volume;
}

bool FFmpegAudioDecoder::isPlaying() const
{
    return m_isPlaying && !m_isPaused;
}

bool FFmpegAudioDecoder::isPaused() const
{
    return m_isPaused;
}

qint64 FFmpegAudioDecoder::position() const
{
    return m_currentPosition;
}

qint64 FFmpegAudioDecoder::duration() const
{
    return m_totalDuration;
}

void FFmpegAudioDecoder::updatePlaybackPosition()
{
    if (!m_isPlaying || m_isPaused || !m_audioSink || m_positionFromPts) {
        return;
    }

    qint64 processedUSecs = m_audioSink->processedUSecs();
    qint64 currentPositionMs = processedUSecs / 1000;

    if (currentPositionMs != m_currentPosition) {
        m_currentPosition = currentPositionMs;
        emit positionChanged(m_currentPosition);
    }
}

void FFmpegAudioDecoder::setupAudioOutput()
{
    if (m_audioSink) {
        delete m_audioSink;
    }

    m_audioSink = new NaviAudioOutput(m_audioFormat, this);
    m_audioSink->setVolume(qreal(m_volume) / 100.0);
    m_audioDevice = m_audioSink->start();
}

void FFmpegAudioDecoder::cleanup()
{
    if (m_audioDevice) {
        m_audioDevice = nullptr;
    }

    if (m_audioSink) {
        delete m_audioSink;
        m_audioSink = nullptr;
    }
}

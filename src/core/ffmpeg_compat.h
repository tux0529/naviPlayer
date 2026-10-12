/**
 * @file    ffmpeg_compat.h
 * @brief   ffmpeg 4.4 / 5.1 / 6.1 / 7.1 统一兼容层
 *
 * 版本边界（以 LIBAVCODEC_VERSION_MAJOR 为准）：
 *   58 -> ffmpeg 4.4   : channels / channel_layout(uint64)，AVCodec* 非 const
 *   59 -> ffmpeg 5.1   : 引入 AVChannelLayout，av_opt_set_chlayout
 *   60 -> ffmpeg 6.1   : 同上，部分旧字段移除
 *   61 -> ffmpeg 7.1   : swr_alloc_set_opts 废弃，必须使用 av_opt_set_chlayout
 *
 * 业务代码只使用本文件提供的 navi* 辅助函数，避免散落版本宏。
 */
#ifndef NAVIPLAYER_FFMPEG_COMPAT_H
#define NAVIPLAYER_FFMPEG_COMPAT_H

#include <QtGlobal>
#include <QString>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/dict.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
}

#if LIBAVCODEC_VERSION_MAJOR >= 59
#  define NAVI_HAVE_CHANNEL_LAYOUT 1
#else
#  define NAVI_HAVE_CHANNEL_LAYOUT 0
#endif

#if LIBAVCODEC_VERSION_MAJOR >= 59
using NaviCodecPtr = const AVCodec *;
#else
using NaviCodecPtr = AVCodec *;
#endif

/**
 * @brief 获取解码上下文声道数
 */
inline int naviCodecChannels(const AVCodecContext *ctx)
{
    if (!ctx)
        return 0;
#if NAVI_HAVE_CHANNEL_LAYOUT
    return ctx->ch_layout.nb_channels;
#else
    return ctx->channels;
#endif
}

/**
 * @brief 获取帧声道数
 */
inline int naviFrameChannels(const AVFrame *frame)
{
    if (!frame)
        return 0;
#if NAVI_HAVE_CHANNEL_LAYOUT
    return frame->ch_layout.nb_channels;
#else
    return frame->channels;
#endif
}

/**
 * @brief 构造输出声道布局（仅支持 1/2 声道，其余降级为立体声）
 */
#if NAVI_HAVE_CHANNEL_LAYOUT
inline AVChannelLayout naviOutputChannelLayout(int channels)
{
    AVChannelLayout layout = {};
    if (channels <= 1)
        layout = AV_CHANNEL_LAYOUT_MONO;
    else
        layout = AV_CHANNEL_LAYOUT_STEREO;
    return layout;
}
#endif

/**
 * @brief 配置 swresample：将解码输出重采样为 S16 指定采样率/声道
 *
 * @param swr        已 swr_alloc() 的上下文（未 init）
 * @param ctx        解码器上下文（提供输入采样率/格式/声道布局）
 * @param outRate    输出采样率
 * @param outChannels 输出声道数（1 或 2）
 * @return 0 成功，否则负的错误码
 */
inline int naviConfigureSwr(SwrContext *swr, AVCodecContext *ctx, int outRate, int outChannels)
{
    if (!swr || !ctx)
        return AVERROR(EINVAL);

    const int outCh = outChannels <= 1 ? 1 : 2;

    av_opt_set_int(swr, "in_sample_rate", ctx->sample_rate, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt", ctx->sample_fmt, 0);
    av_opt_set_int(swr, "out_sample_rate", outRate, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

#if NAVI_HAVE_CHANNEL_LAYOUT
    AVChannelLayout inLayout = ctx->ch_layout;
    if (inLayout.nb_channels <= 0) {
        inLayout = naviOutputChannelLayout(2);
    }
    AVChannelLayout outLayout = naviOutputChannelLayout(outCh);
    av_opt_set_chlayout(swr, "in_chlayout", &inLayout, 0);
    av_opt_set_chlayout(swr, "out_chlayout", &outLayout, 0);
#else
    quint64 inMask = ctx->channel_layout;
    if (inMask == 0) {
        inMask = av_get_default_channel_layout(ctx->channels);
    }
    if (inMask == 0) {
        inMask = AV_CH_LAYOUT_STEREO;
    }
    const quint64 outMask = outCh == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;
    av_opt_set_int(swr, "in_channel_layout", static_cast<qint64>(inMask), 0);
    av_opt_set_int(swr, "out_channel_layout", static_cast<qint64>(outMask), 0);
#endif

    return swr_init(swr);
}

/**
 * @brief 计算重采样后一帧的最大输出字节数
 */
inline int naviSwrOutputBytes(int outRate, int outChannels, int srcNbSamples, int inRate)
{
    if (inRate <= 0 || srcNbSamples <= 0)
        return 0;
    const qint64 samples = (qint64)srcNbSamples * outRate / inRate + 256;
    return static_cast<int>(samples * outChannels * 2);
}

/**
 * @brief 打印 ffmpeg 版本（调试用）
 */
inline QString naviFfmpegVersion()
{
    return QStringLiteral("%1.%2.%3")
        .arg(LIBAVCODEC_VERSION_MAJOR)
        .arg(LIBAVCODEC_VERSION_MINOR)
        .arg(LIBAVCODEC_VERSION_MICRO);
}

#endif // NAVIPLAYER_FFMPEG_COMPAT_H

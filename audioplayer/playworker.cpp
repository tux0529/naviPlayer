#include "playworker.h"
#include "qdebug.h"
#include "config.h"

#include <QTime>
#include <QCoreApplication>

#define MAX_AUDIO_FRAME_SIZE 192000

#define PAUSE_STATE_SLEEP_TIME 100
#define PAUSE_STATE_MAX_WAIT_TIMES 100


PlayWorker::PlayWorker(QAudioOutput *audioOutput, QIODevice *audioDevice):
    m_audioOutput(audioOutput),
    m_audioDevice(audioDevice)
{
    m_mediaStatus = AudioPlayer::NoMedia;

    m_pauseStateWaitTimes = 0;
    m_seekPosition = -1;
    m_currentPosition = 0;
    m_emitPosition = 0;

    //Do global initialization of network libraries. This is optional, and not recommended anymore.
    //avformat_network_init();
}

PlayWorker::~PlayWorker()
{
    if(m_flag != PlayWorker::controlStop){
        this->stop();
    }

}

void PlayWorker::setAudioDevice(QAudioOutput *audioOutput, QIODevice *audioDevice)
{
    this->m_audioOutput = audioOutput;
    this->m_audioDevice = audioDevice;
}

void PlayWorker::play(const QString &filename)
{
    qDebug() << "PlayWorker::play1: fimename" << filename;
    qDebug()<< "PlayWorker::play: m_audioOutput state:" << m_audioOutput->state();

    if (filename.isEmpty() ){
        if(m_seekPosition > -1){
            seek();
            Config::G_Debug("PlayWorker::play:seek", m_seekPosition);
        }
        else{
            m_flag = PlayWorker::controlPlay;
            Config::G_Debug("PlayWorker::play:remuse");
        }
        return;
    }

    this->m_filename = filename;

    if (m_mediaStatus == AudioPlayer::EndOfMedia ) {
        m_flag = PlayWorker::controlPlay;
        decodeAudio();
    }
    else if(m_audioOutput->state() == QAudio::StoppedState){

        if (!m_audioDevice->isOpen())
            m_audioDevice->open(QIODevice::Unbuffered | QIODevice::WriteOnly);

        m_flag = PlayWorker::controlPlay;
        decodeAudio();
    }
    else if (m_audioOutput->state() == QAudio::ActiveState ) {

        m_flag = PlayWorker::controlStop;

        Config::G_Debug("PlayWorker::play: Playing now ,msleep for stop:", 200);
        QThread::msleep( 200 );

        m_flag = PlayWorker::controlPlay;
        decodeAudio();
    }
    else if (m_audioOutput->state() == QAudio::IdleState ) {

        Config::G_Debug("PlayWorker::play: Playing now");
        m_flag = PlayWorker::controlPlay;
        decodeAudio();
    }
}


void PlayWorker::stop()
{
    m_flag = PlayWorker::controlStop;
    QThread::msleep( 100 );
}

void PlayWorker::pause()
{
    m_flag = PlayWorker::controlPause;
}

void PlayWorker::seek(qint64 pos)
{
    if (m_flag == PlayWorker::controlSeek){
        return;
    }

    m_flag = PlayWorker::controlSeek;

    if(pos > -1)
        m_seekPosition = pos;
    else
        decodeAudio();


    Config::G_Debug("PlayWorker::seek: seekPosition:", m_seekPosition);
}

void PlayWorker::decodeAudio()
{

    qint64 duration;

    int error;

    int aStreamIndex = -1;

    m_emitPosition = 0;

    m_currentPosition = 0;

    m_fmtCtx = avformat_alloc_context();
    m_codecCtx = NULL;
    m_swr_ctx = NULL;
    m_pkt = av_packet_alloc();
    m_frame = av_frame_alloc();

    /* Open the input file to read from it. */
    error = avformat_open_input(&m_fmtCtx,m_filename.toLocal8Bit().constData(),NULL,NULL);
    if(error < 0){
        avformat_close_input(&m_fmtCtx);
        emit ERROR(AudioPlayer::ResourceError, "Cannot open input file.");
        return;
    }
    else{
        emit mediaStatusChanged(AudioPlayer::LoadingMedia);
    }

    /* Get information on the input file (number of streams etc.). */
    error = avformat_find_stream_info(m_fmtCtx,NULL);
    if(error < 0){
        avformat_close_input(&m_fmtCtx);
        emit ERROR(AudioPlayer::FormatError, "Cannot find any stream in file.");
        emit mediaStatusChanged(AudioPlayer::InvalidMedia);
        return;
    }

#ifdef G_DEBUG
    //av_dump_format(m_fmtCtx,0,m_filename.toLocal8Bit().constData(),0);
    const AVDictionaryEntry *tag = NULL;
    while ((tag = av_dict_get(m_fmtCtx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))){
        if( "lyrics" == QString(tag->key) ){
            QStringList sl = QString::fromLocal8Bit( tag->value).split("\n");
            for (int i = 0; i < sl.size(); ++i) {
                qDebug() << sl.at(i);
            }
        }
    }
#endif

    /* Find the best audio stream in the input file. */
    aStreamIndex = av_find_best_stream(m_fmtCtx, AVMEDIA_TYPE_AUDIO, -1,-1,NULL,0);

    if(aStreamIndex==-1){

        emit ERROR(AudioPlayer::FormatError, "Cannot find audio stream.");
        emit mediaStatusChanged(AudioPlayer::InvalidMedia);
        avformat_close_input(&m_fmtCtx);
        return;
    }

    const AVStream *aStream = m_fmtCtx->streams[aStreamIndex];

    /* Find a decoder for the audio stream. */
    AVCodecParameters *aCodecPara = aStream->codecpar;
    const AVCodec *codec = avcodec_find_decoder(aCodecPara->codec_id);

    if(!codec){

        emit mediaStatusChanged(AudioPlayer::InvalidMedia);
        emit ERROR(AudioPlayer::FormatError, "Unsurport audio format.");

        avcodec_free_context(&m_codecCtx);
        avformat_close_input(&m_fmtCtx);
        return;
    }

    /* Allocate a new decoding context. */
    m_codecCtx = avcodec_alloc_context3(codec);
    /* Initialize the stream parameters with demuxer information. */
    error = avcodec_parameters_to_context(m_codecCtx,aCodecPara);
    if(error < 0){

        emit ERROR(AudioPlayer::UnknownError, "Cannot alloc codec context.");
        emit mediaStatusChanged(AudioPlayer::UnknownMediaStatus);

        avcodec_free_context(&m_codecCtx);

        avformat_close_input(&m_fmtCtx);
        return;
    }

    /* Open the decoder for the audio stream to use it later. */
    error = avcodec_open2(m_codecCtx,codec,NULL);
    if(error < 0){
        emit ERROR(AudioPlayer::UnknownError, "Cannot open audio codec.");
        emit mediaStatusChanged(AudioPlayer::UnknownMediaStatus);

        avcodec_free_context(&m_codecCtx);

        avformat_close_input(&m_fmtCtx);
        return;
    }

    m_codecCtx->pkt_timebase = aStream->time_base;

    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;

    /* Get the default channel layout for number of channels output. */
    AVChannelLayout out_channel_layout;
    av_channel_layout_default(&out_channel_layout, DEFAULT_OUT_CHANNEL_COUNT);

    /*
     * Create a resampler context for the conversion.
     * Set the conversion parameters.
     */
    error = swr_alloc_set_opts2(&m_swr_ctx,
                                &out_channel_layout,
                                out_sample_fmt,
                                DEFAULT_OUT_SAMPLE_RATE,
                                &m_codecCtx->ch_layout,
                                m_codecCtx->sample_fmt,
                                m_codecCtx->sample_rate,
                                0, NULL);
    if (error < 0) {
        emit ERROR(AudioPlayer::UnknownError, "Could not allocate resample context");
        emit mediaStatusChanged(AudioPlayer::UnknownMediaStatus);

        swr_free(&m_swr_ctx);

        avcodec_free_context(&m_codecCtx);

        avformat_close_input(&m_fmtCtx);

        return;
    }

    error = swr_init(m_swr_ctx);
    if (error < 0) {
        emit ERROR(AudioPlayer::UnknownError, "Could not open resample context");
        emit mediaStatusChanged(AudioPlayer::UnknownMediaStatus);

        swr_free(&m_swr_ctx);

        avcodec_free_context(&m_codecCtx);

        avformat_close_input(&m_fmtCtx);

        return;
    }

    /* Allocate a memory block */
    uint8_t *audio_out_buffer = (uint8_t*)av_malloc(MAX_AUDIO_FRAME_SIZE*2);

    duration = av_q2d(aStream->time_base)*1000*aStream->duration;

    emit durationChanged(duration);
    emit mediaStatusChanged(AudioPlayer::LoadedMedia);
    emit stateChanged(AudioPlayer::PlayingState);

    int sleep_time=0;

    while( 1 ){

        QCoreApplication::processEvents();
        if(m_flag == PlayWorker::controlSeek ){
            int64_t timestamp = m_seekPosition/(double)1000/av_q2d(aStream->time_base);
            error = av_seek_frame(m_fmtCtx, aStreamIndex, timestamp,  AVSEEK_FLAG_BACKWARD);

            if(error < 0){
                m_mediaStatus = AudioPlayer::InvalidMedia;
                emit mediaStatusChanged(AudioPlayer::InvalidMedia);
                freeAVMem();
                emit stateChanged(AudioPlayer::StoppedState);
                return;
            }
            else{
                emit positionChanged(m_seekPosition);
                emit mediaStatusChanged(AudioPlayer::LoadedMedia);
                emit stateChanged(AudioPlayer::PlayingState);
                m_flag = PlayWorker::controlPlay;
                m_currentPosition = m_seekPosition;
                m_seekPosition = -1;

            }
        }

        /* Read the next frame of a stream
         * 0 if OK, < 0 on error or end of file.
         * On error, pkt will be blank (as if it came from av_packet_alloc()).
         */
        error = av_read_frame(m_fmtCtx,m_pkt);

        /* If the end of the input file is reached, stop decoding. */
        if (error == AVERROR_EOF){
            m_mediaStatus = AudioPlayer::EndOfMedia;
            emit mediaStatusChanged(AudioPlayer::EndOfMedia);
            emit stateChanged(AudioPlayer::StoppedState);
            freeAVMem();

            return;
        }
        else if( error < 0 ) {

            emit ERROR(AudioPlayer::ResourceError, "Could not read the frame data.");
            emit mediaStatusChanged(AudioPlayer::InvalidMedia);
            freeAVMem();

            emit stateChanged(AudioPlayer::StoppedState);
            return;
        }

        if(m_pkt->stream_index==aStreamIndex){

            /* Send the audio frame stored in the temporary packet to the decoder.
             * The input audio stream decoder is used to do this. */
            error = avcodec_send_packet(m_codecCtx,m_pkt);
            /* 0	success
             * VERROR(EAGAIN)	input is not accepted in the current state - user must read output with avcodec_receive_frame() (once all output is read, the packet should be resent, and the call will not fail with EAGAIN).
             * AVERROR_EOF	the decoder has been flushed, and no new packets can be sent to it (also returned if more than 1 flush packet is sent)
             * AVERROR(EINVAL)	codec not opened, it is an encoder, or requires flush
             * AVERROR(ENOMEM)	failed to add packet to internal queue, or similar
             * another negative error code	legitimate decoding errors
             * */

            if (error < 0) {
                emit ERROR(AudioPlayer::UnknownError, "Could not send packet for decoding");
                emit mediaStatusChanged(AudioPlayer::InvalidMedia);
                freeAVMem();

                emit stateChanged(AudioPlayer::StoppedState);
                return;
            }

            /* Receive one frame from the decoder. */
            error = avcodec_receive_frame(m_codecCtx,m_frame);
            /* 0                success, a frame was returned
                 * AVERROR(EAGAIN)	output is not available in this state - user must try to send new input
                 * AVERROR_EOF      the codec has been fully flushed, and there will be no more output frames
                 * AVERROR(EINVAL)	codec not opened, or it is an encoder without the AV_CODEC_FLAG_RECON_FRAME flag enabled
                 * other negative error code	legitimate decoding errors
                 * */

            /* If the decoder asks for more data to be able to decode a frame,
                 * return indicating that no data is present. */
            if (error == AVERROR(EAGAIN) || error == AVERROR_EOF) {
                emit ERROR(AudioPlayer::UnknownError, "PlayWorker::decodeAudio : avcodec_receive_frame: %d: Try again!");
                emit mediaStatusChanged(AudioPlayer::InvalidMedia);
                av_packet_unref(m_pkt);
                continue;
            } else if (error < 0) {
                emit ERROR(AudioPlayer::UnknownError, "Error during decoding\n");
                av_packet_unref(m_pkt);
                freeAVMem();

                emit stateChanged(AudioPlayer::StoppedState);
                return;
                /* Default case: Return decoded data. */
            }
            else{

                /* Convert the samples using the resampler.
                 * Returns: number of samples output per channel, negative value on error
                 * */
                int len = swr_convert(m_swr_ctx,
                                      &audio_out_buffer,
                                      MAX_AUDIO_FRAME_SIZE*2,
                                      (const uint8_t**)m_frame->data,
                                      m_frame->nb_samples);
                if(len<=0){
                    emit ERROR(AudioPlayer::UnknownError, "Could not convert input samples");
                    av_packet_unref(m_pkt);
                    continue;
                }

                /* Get the required buffer size for the given audio parameters.
                 * Parameters:  linesize	calculated linesize, may be NULL
                 *              nb_channels	the number of channels
                 *              nb_samples	the number of samples in a single channel
                 *              sample_fmt	the sample format
                 *              align	buffer size alignment (0 = default, 1 = no alignment)
                 * Returns:     required buffer size, or negative error code on failure
                 * */
                int out_size = av_samples_get_buffer_size(0,
                                                          DEFAULT_OUT_CHANNEL_COUNT,
                                                          len,
                                                          out_sample_fmt,
                                                          1);
                //qDebug("buffer size is: %d.",out_size);

                sleep_time = out_size * 1000 /(DEFAULT_OUT_SAMPLE_RATE * DEFAULT_OUT_CHANNEL_COUNT * DEFAULT_OUT_SAMPLE_SIZE);

                while (m_audioOutput->bytesFree() < out_size){

                    QCoreApplication::processEvents();
                    //qDebug()<<"Sleep " << sleep_time << "ms";
                    if ( PlayWorker::controlStop == m_flag){
                        freeAVMem();
                        emit stateChanged(AudioPlayer::StoppedState);
                        return;
                    }
                    else if ( PlayWorker::controlSeek == m_flag){
                        av_packet_unref(m_pkt);
                        break;
                    }
                    else if (PlayWorker::controlPause == m_flag) {
                        emit stateChanged(AudioPlayer::PausedState);
                        while(true){
                            QCoreApplication::processEvents();
                            m_pauseStateWaitTimes++;
                            qDebug() << "pauseState Sleep Times :" <<  m_pauseStateWaitTimes;

                            if(PlayWorker::controlStop == m_flag){
                                freeAVMem();

                                emit stateChanged(AudioPlayer::StoppedState);
                                return;
                            }
                            else if ( PlayWorker::controlSeek == m_flag){
                                av_packet_unref(m_pkt);
                                break;
                            }
                            else if(PlayWorker::controlPlay  == m_flag){
                                emit stateChanged(AudioPlayer::PlayingState);
                                m_pauseStateWaitTimes = 0;

                                break;
                            }
                            else if(m_pauseStateWaitTimes >= PAUSE_STATE_MAX_WAIT_TIMES){
                                m_pauseStateWaitTimes = 0;
                                //m_audioDevice->write((char*)audio_out_buffer, out_size);
                                m_seekPosition = av_q2d(aStream->time_base)*1000*m_frame->pts;
                                freeAVMem();
                                return;
                            }
                            else {
                                QThread::msleep( PAUSE_STATE_SLEEP_TIME );
                            }
                        }
                    }
                    else{
                        QThread::msleep( sleep_time );
                    }
                }

                //qDebug("audioOutput Free size is: %d.",m_audioOutput->bytesFree());
                //qDebug()<<"Time: " << QTime::currentTime().toString("hh:mm:ss.zzz");
                m_audioDevice->write((char*)audio_out_buffer, out_size);

                m_currentPosition = av_q2d(aStream->time_base)*1000*m_frame->pts;

                if ( (m_currentPosition / 100) > ( m_emitPosition / 100)){
                    m_emitPosition = m_currentPosition;
                    emit positionChanged(m_emitPosition);
                }
            }

        }
        av_packet_unref(m_pkt);
    }

    freeAVMem();

    emit stateChanged(AudioPlayer::StoppedState);
}

void PlayWorker::freeAVMem()
{

    av_packet_unref(m_pkt);

    swr_free(&m_swr_ctx);

    av_frame_free(&m_frame);
    av_packet_free(&m_pkt);
    av_freep(&m_pkt);

    avcodec_close(m_codecCtx);
    avcodec_free_context(&m_codecCtx);

    avformat_free_context(m_fmtCtx);
}


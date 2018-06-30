#include "muxer.h"

#ifdef _WIN32
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#endif // _WIN32

#ifdef SYS_LOG_TAG
#undef SYS_LOG_TAG
#define SYS_LOG_TAG "PLShortVideo-FFMuxer"
#endif 

void free_output_stream(output_stream_t* muxer_ptr);

//print AVPacket timestamp
void muxer_log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt);
//copy stream header to coder
int muxer_set_header(AVStream* stream, muxer_t* muxer_ptr, unsigned char* data, int data_len);

void muxer_init_ffmpeg()
{
    //global register ffmpeg
    av_register_all();
    //LOGI("%s init ffmpeg.", __FUNCTION__);
}

muxer_t* muxer_create_file(const char* file_name)
{
    if (!file_name) {
        //LOGE("%s error file_name is NULL!", __FUNCTION__);
        return NULL;
    }
    //LOGI("%s file_name:%s", __FUNCTION__, file_name);
    
    muxer_t* muxer_ptr = (muxer_t*)malloc(sizeof(muxer_t));
    if (!muxer_ptr) {
        //LOGE("%s malloc muxer_t memory failed.", __FUNCTION__);
        return NULL;
    }
    memset(muxer_ptr, 0, sizeof(muxer_t));

    muxer_ptr->file_name = (char* )malloc(strlen(file_name) + 1);
    if (!muxer_ptr->file_name) {
        //LOGE("%s file_name is too long, length is: %d; malloc memory failed.", __FUNCTION__, strlen(file_name));
        free(muxer_ptr);
        return NULL;
    }
    memset(muxer_ptr->file_name, 0, strlen(file_name) + 1);
    memcpy(muxer_ptr->file_name, file_name, strlen(file_name));
    muxer_ptr->file_name[strlen(file_name)] = '\0';

    //allocate the output media context
    avformat_alloc_output_context2(&muxer_ptr->fmt_ctx, NULL, NULL, muxer_ptr->file_name);
    if (!muxer_ptr->fmt_ctx) {
        avformat_alloc_output_context2(&muxer_ptr->fmt_ctx, NULL, "mp4", muxer_ptr->file_name);
    }
    if (!muxer_ptr->fmt_ctx) {
        //LOGE("%s init format context failed.", __FUNCTION__);
        //release muxerinfo
        free(muxer_ptr);
        return NULL;
    }
    //init audio & video codec
    if (!muxer_ptr->fmt_ctx->oformat) {
        //LOGE("%s AVFormatContext output format is null.", __FUNCTION__);
        free(muxer_ptr);
        return NULL;
    }
    muxer_ptr->fmt_ctx->oformat->audio_codec = AV_CODEC_ID_NONE;
    muxer_ptr->fmt_ctx->oformat->video_codec = AV_CODEC_ID_NONE;
    muxer_ptr->output_fmt = muxer_ptr->fmt_ctx->oformat;

    //success
    return muxer_ptr;
}

int muxer_add_video_stream(muxer_t* muxer_ptr, int width, int height,
    int bit_rate, int gop_size, int frame_per_sec, EVideoCodec video_codec_id, int rotate)
{
    //LOGI("%s muxer_ptr:%x, width:%d, height:%d, bit_rate:%d, gop_size:%d, frame_per_sec:%d, video_codec_id:%d, rotate:%d",
        //__FUNCTION__, (unsigned int)muxer_ptr, width, height, bit_rate, gop_size, frame_per_sec, video_codec_id, rotate);

    if (!muxer_ptr || width <= 0 || height <= 0 || bit_rate <= 0 || gop_size <= 0 || frame_per_sec <= 0) {
        return -1;
    }

    //if already add a video stream, release it
    //the current video file allows only have one video stream all the way.
    if (muxer_ptr->video_out_stream) {
        free_output_stream(muxer_ptr->video_out_stream);
    }

    //create video stream
    do {
        muxer_ptr->video_out_stream = (output_stream_t*)malloc(sizeof(output_stream_t));
        if (!muxer_ptr->video_out_stream) {
            //LOGE("%s malloc video out stream memory failed.", __FUNCTION__);
            break;
        }
        memset(muxer_ptr->video_out_stream, 0, sizeof(output_stream_t));

        muxer_ptr->video_out_stream->stream = avformat_new_stream(muxer_ptr->fmt_ctx, NULL);
        if (!muxer_ptr->video_out_stream->stream) {
            //LOGE("Could not allocate stream");
            break;
        }
        muxer_ptr->video_out_stream->stream->id = muxer_ptr->fmt_ctx->nb_streams - 1;
        
        //init codec parameters to set AVCodecContext and AVStream
        //params's extradata will init when set sps & pps
        AVCodecParameters* params = avcodec_parameters_alloc();
        if (!params) {
            break;
        }
        params->codec_type = AVMEDIA_TYPE_VIDEO;
        params->codec_id = AV_CODEC_ID_H264;
        params->width = width;
        params->height = height;
        params->bit_rate = bit_rate;
        //not a must
        AVRational rt = { 0, 1 };
        params->sample_aspect_ratio = rt;
        params->color_primaries = 2;
        params->color_trc = 2;
        params->color_space = 2;
        params->color_space = AV_PIX_FMT_YUV420P;
        params->format = AV_PIX_FMT_YUV420P;
        params->level = params->profile = -99;

        //init codec context
        AVCodecContext *codec_ctx = NULL;
        codec_ctx = avcodec_alloc_context3(NULL);
        if (!codec_ctx) {
            //LOGE("Could not alloc an encoding context");
            return -1;
        }
        codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
        switch (video_codec_id) {
        case EVIDEO_CODEC_H264:
            codec_ctx->codec_id = AV_CODEC_ID_H264;
            break;
        case EVIDEO_CODEC_MPEG4:
            codec_ctx->codec_id = AV_CODEC_ID_MPEG4;
            break;
        }
        //set parameters to AVCodecContext
        int ret = avcodec_parameters_to_context(codec_ctx, params);
        if (0 != ret) {
            //LOGE("%s set codec parameters to codec context failed, error string:%s",
                //__FUNCTION__, av_err2str(ret));
            break;
        }

        codec_ctx->bit_rate = bit_rate;
        //Resolution must be a multiple of two. 
        codec_ctx->width = width;
        codec_ctx->height = height;
        /* timebase: This is the fundamental unit of time (in seconds) in terms
        * of which frame timestamps are represented. For fixed-fps content,
        * timebase should be 1/framerate and timestamp increments should be
        * identical to 1. */
        //used for not video timestamp,only video frame per second
        // muxer_ptr->video_out_stream->stream->time_base = (AVRational) { 1, frame_per_sec };
        // codec_ctx->time_base = muxer_ptr->video_out_stream->stream->time_base;

        //use external video timestamp
        muxer_ptr->video_out_stream->stream->time_base = AV_TIME_BASE_Q;
        codec_ctx->time_base = AV_TIME_BASE_Q;

        codec_ctx->gop_size = gop_size; //emit one intra frame every twelve frames at most
        codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P; 

        muxer_ptr->video_out_stream->codec_context = codec_ctx;

        ret = avcodec_parameters_from_context(
            muxer_ptr->video_out_stream->stream->codecpar,
            muxer_ptr->video_out_stream->codec_context);

        if (0 != ret) {
            //LOGE("%s init codec parameters from codec context failed, error string:%s",
                //__FUNCTION__, av_err2str(ret));
            break;
        }

        //LOGI("%s add video stream success, stream time base:%d:%d, codecContext time base:%d:%d, video stream index:%d, id:%d",
        //__FUNCTION__, muxer_ptr->video_out_stream->stream->time_base.num, muxer_ptr->video_out_stream->stream->time_base.den,
        //muxer_ptr->video_out_stream->codec_context->time_base.num, muxer_ptr->video_out_stream->codec_context->time_base.den,
        //muxer_ptr->video_out_stream->stream->index, muxer_ptr->video_out_stream->stream->id);

        //set rotate
        char rotate_buf[128] = {0};
        snprintf(rotate_buf, 128, "%d", rotate);
        ret = av_dict_set(&muxer_ptr->video_out_stream->stream->metadata, "rotate", rotate_buf, 0);
        if (ret < 0) {
            //LOGE("%s set video rotate %s failed. error str:%s", __FUNCTION__, rotate_buf, av_err2str(ret));
        }

        return muxer_ptr->video_out_stream->stream->index;
    } while(0);

    //failed
    //LOGE("%s failed!", __FUNCTION__);
    if (muxer_ptr->video_out_stream) {
        free_output_stream(muxer_ptr->video_out_stream);
        muxer_ptr->video_out_stream = NULL;
    }
    return -1;
}

int muxer_add_audio_stream(muxer_t* muxer_ptr, int sample_rate, int channel_num,
    int bit_width, int bit_rate, EAudioCodec audio_codec_id)
{
    //LOGI("%s muxer_ptr:%x, sample_rate:%d, channel_num:%d, bit_width:%d, bit_rate:%d, audio_codec_id:%d",
        //__FUNCTION__, (unsigned int)muxer_ptr, sample_rate, channel_num, bit_width, bit_rate, audio_codec_id);

    if (!muxer_ptr || sample_rate <= 0 || channel_num <= 0 || bit_width <= 0 || bit_rate <= 0) {
        return -1;
    }
    //if already add a audio stream, release it
    //the current video file allows only have one audio stream all the way.
    if (muxer_ptr->audio_out_stream) {
        free_output_stream(muxer_ptr->audio_out_stream);
    }
    do {
        muxer_ptr->audio_out_stream = (output_stream_t*)malloc(sizeof(output_stream_t));
        if (!muxer_ptr->audio_out_stream) {
            //LOGE("%s malloc audio out stream memory failed.", __FUNCTION__);
            break;
        }
        memset(muxer_ptr->audio_out_stream, 0, sizeof(output_stream_t));

        muxer_ptr->audio_out_stream->stream = avformat_new_stream(muxer_ptr->fmt_ctx, NULL);
        if (!muxer_ptr->audio_out_stream->stream) {
            //LOGE("Could not allocate stream");
            break;
        }
        muxer_ptr->audio_out_stream->stream->id = muxer_ptr->fmt_ctx->nb_streams - 1;

        //init codec context
        AVCodecContext *codec_ctx = NULL;
        codec_ctx = avcodec_alloc_context3(NULL);
        if (!codec_ctx) {
            //LOGE("Could not alloc an encoding context");
            break;
        }
        //init PCM sample format, here maybe occur exception
        switch (bit_width) {
        case 8:
            codec_ctx->sample_fmt = AV_SAMPLE_FMT_U8;
            break;
        case 16:
            codec_ctx->sample_fmt = AV_SAMPLE_FMT_S16;
            break;
        case 32:
            codec_ctx->sample_fmt = AV_SAMPLE_FMT_S32;
            break;
        }

        codec_ctx->channels = channel_num;
        switch (codec_ctx->channels) {
        case 1:
            codec_ctx->channel_layout = AV_CH_LAYOUT_MONO;
            break;
        case 2:
            codec_ctx->channel_layout = AV_CH_LAYOUT_STEREO;
            break;
        }
        codec_ctx->bit_rate = bit_rate;
        codec_ctx->sample_rate = sample_rate;

        //init audio codec
        AVCodecParameters* params = avcodec_parameters_alloc();
        if (!params) {
            break;
        }
        params->codec_type = AVMEDIA_TYPE_AUDIO;
        params->codec_id = AV_CODEC_ID_AAC;
        params->codec_tag = 0;
        params->format = -1;
        params->bit_rate = bit_rate;
        params->profile = params->level = -99;
        AVRational rt = { 0, 8192 };
        params->sample_aspect_ratio = rt;
        params->color_primaries = 2;
        params->color_trc = 2;
        params->color_space = 2;
        params->channels = codec_ctx->channels;
        params->channel_layout = codec_ctx->channel_layout;
        params->bits_per_coded_sample = bit_width;
        params->bits_per_raw_sample = 0;
        params->sample_rate = sample_rate;
        // params->frame_size = 19200;

        int ret = avcodec_parameters_to_context(codec_ctx, params);
        if (ret != 0) {
            //LOGE("%s set codec parameters to codec context failed, error str:%s",
                //__FUNCTION__, av_err2str(ret));
            break;
        }

        muxer_ptr->audio_out_stream->codec_context = codec_ctx;

        ret = avcodec_parameters_from_context(
            muxer_ptr->audio_out_stream->stream->codecpar,
            muxer_ptr->audio_out_stream->codec_context);
        if (ret != 0) {
            //LOGE("%s set codec parameters from codec context failed, error str:%s",
                //__FUNCTION__, av_err2str(ret));
            break;
        }
        muxer_ptr->audio_out_stream->stream->time_base = (AVRational){1,  sample_rate /* AV_TIME_BASE */};
        muxer_ptr->audio_out_stream->codec_context->time_base = (AVRational){1,  sample_rate /* AV_TIME_BASE */};

        //LOGI("%s add audio stream success, stream time base:%d:%d, codecContext time base:%d:%d, audio stream index:%d, id:%d",
        //__FUNCTION__, muxer_ptr->audio_out_stream->stream->time_base.num, muxer_ptr->audio_out_stream->stream->time_base.den,
        //muxer_ptr->audio_out_stream->codec_context->time_base.num, muxer_ptr->audio_out_stream->codec_context->time_base.den,
        //muxer_ptr->audio_out_stream->stream->index, muxer_ptr->audio_out_stream->stream->id);

        return muxer_ptr->audio_out_stream->stream->index;
    } while (0);

    //failed
    //LOGE("%s failed!", __FUNCTION__);
    if (muxer_ptr->audio_out_stream) {
        free_output_stream(muxer_ptr->audio_out_stream);
        muxer_ptr->audio_out_stream = NULL;
    }
    return -1;
}

int muxer_set_header(AVStream* stream, muxer_t* muxer_ptr, unsigned char* data, int data_len) 
{
    //copy header data to stream codec param
    if (!stream || !stream->codecpar) {
        //LOGE("%s stream or codecpar is null, you must add stream first.", __FUNCTION__);
        return -1;
    }
    if(stream->codecpar->extradata) {
        free(stream->codecpar->extradata);
        stream->codecpar->extradata = NULL;
    }
    stream->codecpar->extradata = (uint8_t *)malloc(data_len);
    memcpy(stream->codecpar->extradata, data, data_len);
    stream->codecpar->extradata_size = data_len;

    return 0;
}

int muxer_set_sps_pps_header(muxer_t* muxer_ptr, unsigned char* data, int data_len)
{
    //LOGI("%s muxer_ptr:%x, sps pps data:%x, data_len:%d",
        //__FUNCTION__, (unsigned int)muxer_ptr, (unsigned int)data, data_len);

    if (!data || data_len <= 0 || !muxer_ptr || !muxer_ptr->video_out_stream) {
        //LOGE("muxer_ptr->video_out_stream is null.");
        return -1;
    }
    FILE* fp = fopen("/sdcard/sps-pps.raw", "wb");
    fwrite(data, 1, data_len, fp);
    fclose(fp);
    //copy sps pps to stream codec param
    return muxer_set_header(muxer_ptr->video_out_stream->stream, muxer_ptr, data, data_len);
}

int muxer_set_esds_header(muxer_t* muxer_ptr, unsigned char* data, int data_len)
{
    //LOGI("%s muxer_ptr:%x, esds data:%x, data_len:%d",
        //__FUNCTION__, (unsigned int)muxer_ptr, (unsigned int)data, data_len);

    if (!data || data_len <= 0 || !muxer_ptr || !muxer_ptr->audio_out_stream) {
        //LOGE("muxer_ptr->audio_out_stream is null.");
        return -1;
    }
    //copy esds to stream codec param
    return muxer_set_header(muxer_ptr->audio_out_stream->stream, muxer_ptr, data, data_len);
}

int muxer_start(muxer_t* muxer_ptr)
{
    //LOGI("%s muxer_ptr:%x", __FUNCTION__, (unsigned int)muxer_ptr);

    if (!muxer_ptr || !muxer_ptr->fmt_ctx || !muxer_ptr->file_name) {
        //LOGE("muxer_info_pt or FormatContext is null.");
        return -1;
    }
    if (!muxer_ptr->file_name) {
        //LOGE("Output file name is null.");
        return -1;
    }
    //dump format, only for debug video format information
    av_dump_format(muxer_ptr->fmt_ctx, 0, muxer_ptr->file_name, 1);

    int ret = 0;
    //open the output file, if needed
    if (!(muxer_ptr->fmt_ctx->flags & AVFMT_NOFILE)) {
        ret = avio_open(&muxer_ptr->fmt_ctx->pb, muxer_ptr->file_name, AVIO_FLAG_WRITE);
        if (ret < 0) {
            //LOGE("Could not open '%s': %s", muxer_ptr->file_name,
                //av_err2str(ret));
            return -1;
        }
    }

    if (av_opt_set(muxer_ptr->fmt_ctx->priv_data, "movflags", "faststart", 0) < 0){
        //LOGE("%s set moov failed.", __FUNCTION__);
    } else {
        //LOGI("%s set moov success.", __FUNCTION__);
    }

    //Write the stream header, if any.
    ret = avformat_write_header(muxer_ptr->fmt_ctx, NULL);
    if (ret < 0) {
        //LOGE("Error occurred when opening output file: %s",
            //av_err2str(ret));
        return -1;
    }

    //success
    return 0;
}

int muxer_write_video_frame(muxer_t* muxer_ptr, unsigned char* pkt_data, int data_len, int is_key_frame, long long pts, long long dts)
{
    //LOGD("%s pkt_data:%x, data_len:%d, is_key_frame:%d, pts:%lld, dts:%lld", __FUNCTION__, (unsigned int)pkt_data, data_len, is_key_frame, pts, dts);
    if (!muxer_ptr || !pkt_data || data_len <= 0) {
        //LOGE("%s muxer_ptr:%x, pkt_data:%x, data_len:%d",
            //__FUNCTION__, (unsigned int)muxer_ptr, (unsigned int)pkt_data, data_len);
        return -1;
    }
    if (!muxer_ptr->video_out_stream) {
        //LOGE("%s video stream ptr is null.", __FUNCTION__);
        return -1;
    }
    //init a packet
    AVPacket pkt = { 0 };
    pkt.stream_index = muxer_ptr->video_out_stream->stream->index;
    pkt.buf = NULL;
    pkt.side_data = NULL;
    pkt.side_data_elems = 0;
    pkt.pos = 0;
    pkt.pts = pts;
    pkt.dts = dts;
    pkt.flags = is_key_frame;
    pkt.data = pkt_data;
    pkt.size = data_len;

    // //if not set pts and dts
    // if (-1 == pts && -1 == dts) {
    //     static long long _pts = 0;
    //     ++_pts;
    //     pkt.pts = _pts;
    //     pkt.dts = _pts;
    //     //default frame_per_sec is 25
    //     muxer_ptr->video_out_stream->codec_context->time_base = (AVRational){1, 25};
    //     muxer_ptr->video_out_stream->stream->time_base = (AVRational){1, 25};
    //     //rescale output packet timestamp values from codec to stream timebase
    //     av_packet_rescale_ts(&pkt, muxer_ptr->video_out_stream->codec_context->time_base,
    //         muxer_ptr->video_out_stream->stream->time_base);
    // }

    muxer_log_packet(muxer_ptr->fmt_ctx, &pkt);

    //Write the compressed frame to the media file.
    int ret = av_interleaved_write_frame(muxer_ptr->fmt_ctx, &pkt);
    if (0 != ret) {
        //failed
        //LOGE("%s write frame failed, error str:%s", __FUNCTION__, av_err2str(ret));
        return -1;
    }
    //success
    return 0;
}

int muxer_write_audio_frame(muxer_t* muxer_ptr, unsigned char* pkt_data, int data_len, long long pts, int samples_in_frame)
{
    //LOGD("%s pkt_data:%x, data_len:%d, pts:%lld, samples_in_frame:%d", __FUNCTION__, (unsigned int)pkt_data, data_len, pts, samples_in_frame);

    if (!muxer_ptr || !pkt_data || data_len <= 0) {
        //LOGE("%s muxer_ptr:%x, pkt_data:%x, data_len:%d, samples_in_frame:%d",
            //__FUNCTION__, (unsigned int)muxer_ptr, (unsigned int)pkt_data, data_len, samples_in_frame);
        return -1;
    }
    if (!muxer_ptr->audio_out_stream || ! muxer_ptr->audio_out_stream->stream) {
        //LOGE("%s failed, audio out stream or stream is null.", __FUNCTION__);
        return -1;
    }
    AVPacket pkt = { 0 };
    pkt.stream_index = muxer_ptr->audio_out_stream->stream->index;
    pkt.buf = NULL;
    pkt.side_data = NULL;
    pkt.side_data_elems = 0;
    pkt.pts = pts / (1000000.0f / muxer_ptr->audio_out_stream->stream->time_base.den);
    pkt.dts = pkt.pts;
    pkt.pos = 0;
    // pkt.duration = samples_in_frame;
    pkt.data = pkt_data;
    pkt.size = data_len;

    // //if not set pts
    // if (-1 == pts) {
    //     static long long sample_count = 0;
    //     sample_count += samples_in_frame;
    //     pts = sample_count;
    //     int64_t pts_ = av_rescale_q(sample_count, (AVRational) { 1, muxer_ptr->audio_out_stream->codec_context->sample_rate },
    //         muxer_ptr->audio_out_stream->codec_context->time_base);
    //     pkt.pts = pts_;
    //     pkt.dts = pts_;
    //     pkt.duration = 1024;    //This value can't achieved , so we give a default value
    // }

    muxer_log_packet(muxer_ptr->fmt_ctx, &pkt);

    //Write the compressed frame to the media file.
    int ret = av_interleaved_write_frame(muxer_ptr->fmt_ctx, &pkt);
    if (0 != ret) {
        //failed
        //LOGE("%s write frame failed, error str:%s", __FUNCTION__, av_err2str(ret));
        return -1;
    }
    //success
    return 0;
}

int muxer_close_file(muxer_t* muxer_ptr)
{
    //LOGI("%s muxer_ptr:%x", __FUNCTION__, (unsigned int)muxer_ptr);

    if (!muxer_ptr || !muxer_ptr->fmt_ctx) {
        //LOGE("muxer_info_pt or FormatContext is null.");
        return -1;
    }
    if (!muxer_ptr->file_name) {
        //LOGE("Output file name is null.");
        return -1;
    }
    /* Write the trailer, if any. The trailer must be written before you
    * close the CodecContexts open when you wrote the header; otherwise
    * av_write_trailer() may try to use memory that was freed on
    * av_codec_close(). */
    av_write_trailer(muxer_ptr->fmt_ctx);

    //Close each codec.
    if (muxer_ptr->video_out_stream) {
        free_output_stream(muxer_ptr->video_out_stream);
    }

    if (muxer_ptr->audio_out_stream) {
        free_output_stream(muxer_ptr->audio_out_stream);
    }

    if (!(muxer_ptr->fmt_ctx->flags & AVFMT_NOFILE))
        //Close the output file.
        avio_closep(&muxer_ptr->fmt_ctx->pb);

    //free the stream
    avformat_free_context(muxer_ptr->fmt_ctx);

    if (muxer_ptr->file_name) {
        free(muxer_ptr->file_name);
    }

    free(muxer_ptr);
    return 0;
}

void free_output_stream(output_stream_t* output_stream_ptr)
{
    if (!output_stream_ptr) {
        return;
    }
    if (output_stream_ptr->codec_context) {
        avcodec_free_context(&(output_stream_ptr->codec_context));
    }
    free(output_stream_ptr);
}

void muxer_log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)
{
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    //LOGI("time_base{%d, %d} pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d",
        //time_base->num, time_base->den,
        //av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
        //av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
        //av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
        //pkt->stream_index);
}


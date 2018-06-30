#include "demuxer.h"
//#include "logger.h"

#ifdef _WIN32
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#endif // _WIN32

#ifdef SYS_LOG_TAG
#undef SYS_LOG_TAG
#define SYS_LOG_TAG "PLShortVideo-FFDeMuxer"
#endif

void demuxer_init_ffmpeg()
{
    //global register ffmpeg
    av_register_all();
    //LOGI("%s init ffmpeg.", __FUNCTION__);
}

demuxer_t * demuxer_open_file(const char * file_name)
{
    //LOGI("%s file_name : %s", __FUNCTION__, file_name);
    if (!file_name) {
        return NULL;
    }
    demuxer_t* demuxer_ptr = NULL;

    do {
        demuxer_ptr = (demuxer_t*)malloc(sizeof(demuxer_t));
        if (!demuxer_ptr) {
            //LOGE("%s malloc muxer_t memory failed.", __FUNCTION__);
            break;
        }
        memset(demuxer_ptr, 0, sizeof(demuxer_t));

        demuxer_ptr->file_name = (char*)malloc(strlen(file_name) + 1);
        if (!demuxer_ptr->file_name) {
            //LOGE("%s file_name is too long, length is: %d; malloc memory failed.", __FUNCTION__, strlen(file_name));
            free(demuxer_ptr);
            break;
        }
        memset(demuxer_ptr->file_name, 0, strlen(file_name) + 1);
        memcpy(demuxer_ptr->file_name, file_name, strlen(file_name));
        demuxer_ptr->file_name[strlen(file_name)] = '\0';
        demuxer_ptr->audio_stream_index = -1;
        demuxer_ptr->video_stream_index = -1;

        int ret = -1;
        if ((ret = avformat_open_input(&demuxer_ptr->fmt_ctx, demuxer_ptr->file_name, 0, 0)) < 0) {
            //LOGE("%s Could not open input file '%s', error str:%s", 
                //__FUNCTION__, demuxer_ptr->file_name, av_err2str(ret));
            break;
        }

        if ((ret = avformat_find_stream_info(demuxer_ptr->fmt_ctx, 0)) < 0) {
            //LOGE("%s Failed to retrieve input stream information.", __FUNCTION__);
            break;
        }
        //dump input file format information (print debug)
        av_dump_format(demuxer_ptr->fmt_ctx, 0, demuxer_ptr->file_name, 0);

        demuxer_ptr->audio_stream_index = av_find_best_stream(demuxer_ptr->fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
        if (demuxer_ptr->audio_stream_index >= 0) {
            //LOGI("%s find audio stream : %d", __FUNCTION__, demuxer_ptr->audio_stream_index);
        } else {
            //LOGI("%s not find audio stream.", __FUNCTION__);
        }
        demuxer_ptr->video_stream_index = av_find_best_stream(demuxer_ptr->fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
        if (demuxer_ptr->video_stream_index >= 0) {
            //LOGI("%s find video stream : %d", __FUNCTION__, demuxer_ptr->video_stream_index);
        } else {
            //LOGI("%s not find video stream.", __FUNCTION__);
        }

        //success
        //LOGI("%s success.", __FUNCTION__);

        return demuxer_ptr;
    } while (0);

    //failed
    //LOGE("%s failed.", __FUNCTION__);

    demuxer_close_file(demuxer_ptr);
    return NULL;
}

int demuxer_get_video_stream_index(demuxer_t * demuxer_ptr)
{
    //LOGI("%s demuxer info ptr:%x", __FUNCTION__, (unsigned int)demuxer_ptr);
    if (!demuxer_ptr) {
        return -1;
    }
    return demuxer_ptr->video_stream_index;
}

int demuxer_get_audio_stream_index(demuxer_t * demuxer_ptr)
{
    //LOGI("%s demuxer info ptr:%x", __FUNCTION__, (unsigned int)demuxer_ptr);
    if (!demuxer_ptr) {
        return -1;
    }
    return demuxer_ptr->audio_stream_index;
}

demuxer_avpacket_t * demuxer_read_next_packet(demuxer_t * demuxer_ptr)
{
    //LOGI("%s demuxer info ptr:%x", __FUNCTION__, (unsigned int)demuxer_ptr);
    if (!demuxer_ptr || !demuxer_ptr->fmt_ctx) {
        //LOGE("%s muxer info ptr or AVFormatContext is null.", __FUNCTION__);
        return NULL;
    }
    //read a packet from file,maybe audio or video
    AVPacket pkt = { 0 };
    while (1) {
        int ret = av_read_frame(demuxer_ptr->fmt_ctx, &pkt);
        if (ret < 0) {
            //LOGE("%s read frame failed, error str : %s", __FUNCTION__, av_err2str(ret));
            return NULL;
        }
        if ((pkt.stream_index != demuxer_ptr->audio_stream_index) & 
            (pkt.stream_index != demuxer_ptr->video_stream_index)) {
            //LOGI("%s read a frame stream index : %d, not belong to audio and video.", __FUNCTION__, pkt.stream_index);
            continue;
        }
        //LOGI("%s read a frame from stream index : %d; pts : %lld, dts : %lld, data size : %d, frame type : %d",
            //__FUNCTION__, pkt.stream_index, pkt.pts, pkt.dts, pkt.size, pkt.flags);

        //copy memory to demuxer_ptr->av_packet from pkt
        if (!demuxer_ptr->av_packet) {
            demuxer_ptr->av_packet = (demuxer_avpacket_t *)malloc(sizeof(demuxer_avpacket_t));
            if (!demuxer_ptr->av_packet) {
                //LOGI("%s malloc demuxer packet memory failed.", __FUNCTION__);
                av_packet_unref(&pkt);
                break;
            }
            memset(demuxer_ptr->av_packet, 0, sizeof(demuxer_avpacket_t));
        }
        if (demuxer_ptr->av_packet->capacity_size < pkt.size) {
            if (demuxer_ptr->av_packet->data) {
                free(demuxer_ptr->av_packet->data);
            }

            demuxer_ptr->av_packet->data = (unsigned char*)malloc(pkt.size);
            if (!demuxer_ptr->av_packet->data) {
                //LOGI("%s malloc demuxer packet data memory failed.", __FUNCTION__);
                av_packet_unref(&pkt);
                demuxer_ptr->av_packet->data = NULL;
                demuxer_ptr->av_packet->capacity_size = 0;
                demuxer_ptr->av_packet->limit_size = 0;
                free(demuxer_ptr->av_packet);
                demuxer_ptr->av_packet = NULL;
                break;
            }
            //record the capacity size
            demuxer_ptr->av_packet->capacity_size = pkt.size;
        }
        //copy memory
        memcpy(demuxer_ptr->av_packet->data, pkt.data, pkt.size);
        demuxer_ptr->av_packet->limit_size = pkt.size;
        demuxer_ptr->av_packet->duration = pkt.duration;
        demuxer_ptr->av_packet->flags = pkt.flags;
        demuxer_ptr->av_packet->pts = pkt.pts;
        demuxer_ptr->av_packet->dts = pkt.dts;
        demuxer_ptr->av_packet->stream_index = pkt.stream_index;

        av_packet_unref(&pkt);
        //success 
        return demuxer_ptr->av_packet;
    }
    //failed
    //LOGE("%s read frame failed.", __FUNCTION__);
    return NULL;
}

extern demuxer_avpacket_t* demuxer_read_next_key_frame(demuxer_t* demuxer_ptr)
{
    //LOGI("%s demuxer info ptr:%x", __FUNCTION__, (unsigned int)demuxer_ptr);
    if (!demuxer_ptr || !demuxer_ptr->fmt_ctx) {
        //LOGE("%s muxer info ptr or AVFormatContext is null.", __FUNCTION__);
        return NULL;
    }
    //read a packet from file,maybe audio or video
    AVPacket pkt = { 0 };
    while (1) {
        int ret = av_read_frame(demuxer_ptr->fmt_ctx, &pkt);
        if (ret < 0) {
            //LOGE("%s read frame failed, error str : %s", __FUNCTION__, av_err2str(ret));
            return NULL;
        }
        if (pkt.stream_index != demuxer_ptr->video_stream_index) {
            //LOGI("%s read a frame stream index : %d, not belong to audio and video.", __FUNCTION__, pkt.stream_index);
            continue;
        }
        // only read vide key frame
        if (pkt.flags != AV_PKT_FLAG_KEY) {
            continue;
        }
        //LOGI("%s read a frame from stream index : %d; pts : %lld, dts : %lld, data size : %d, frame type : %d",
        //__FUNCTION__, pkt.stream_index, pkt.pts, pkt.dts, pkt.size, pkt.flags);

        //copy memory to demuxer_ptr->av_packet from pkt
        if (!demuxer_ptr->av_packet) {
            demuxer_ptr->av_packet = (demuxer_avpacket_t *)malloc(sizeof(demuxer_avpacket_t));
            if (!demuxer_ptr->av_packet) {
                //LOGI("%s malloc demuxer packet memory failed.", __FUNCTION__);
                av_packet_unref(&pkt);
                break;
            }
            memset(demuxer_ptr->av_packet, 0, sizeof(demuxer_avpacket_t));
        }
        if (demuxer_ptr->av_packet->capacity_size < pkt.size) {
            if (demuxer_ptr->av_packet->data) {
                free(demuxer_ptr->av_packet->data);
            }

            demuxer_ptr->av_packet->data = (unsigned char*)malloc(pkt.size);
            if (!demuxer_ptr->av_packet->data) {
                //LOGI("%s malloc demuxer packet data memory failed.", __FUNCTION__);
                av_packet_unref(&pkt);
                demuxer_ptr->av_packet->data = NULL;
                demuxer_ptr->av_packet->capacity_size = 0;
                demuxer_ptr->av_packet->limit_size = 0;
                free(demuxer_ptr->av_packet);
                demuxer_ptr->av_packet = NULL;
                break;
            }
            //record the capacity size
            demuxer_ptr->av_packet->capacity_size = pkt.size;
        }
        //copy memory
        memcpy(demuxer_ptr->av_packet->data, pkt.data, pkt.size);
        demuxer_ptr->av_packet->limit_size = pkt.size;
        demuxer_ptr->av_packet->duration = pkt.duration;
        demuxer_ptr->av_packet->flags = pkt.flags;
        demuxer_ptr->av_packet->pts = pkt.pts;
        demuxer_ptr->av_packet->dts = pkt.dts;
        demuxer_ptr->av_packet->stream_index = pkt.stream_index;

        av_packet_unref(&pkt);
        //success 
        return demuxer_ptr->av_packet;
    }
    //failed
    //LOGE("%s read frame failed.", __FUNCTION__);
    return NULL;
}

int demuxer_get_video_width(demuxer_t * demuxer_ptr)
{
    //LOGI("%s demuxer info ptr : %x", __FUNCTION__, (unsigned int)demuxer_ptr);
    if (!demuxer_ptr || !demuxer_ptr->fmt_ctx || demuxer_ptr->video_stream_index < 0) {
        //LOGI("%s video stream is null.", __FUNCTION__);
        return -1;
    }

    //LOGI("%s get video width : %d", __FUNCTION__, 
        //demuxer_ptr->fmt_ctx->streams[demuxer_ptr->video_stream_index]->codec->width);

    return demuxer_ptr->fmt_ctx->streams[demuxer_ptr->video_stream_index]->codec->width;
}

int demuxer_get_video_height(demuxer_t * demuxer_ptr)
{
    //LOGI("%s demuxer info ptr : %x", __FUNCTION__, (unsigned int)demuxer_ptr);
    if (!demuxer_ptr || !demuxer_ptr->fmt_ctx || demuxer_ptr->video_stream_index < 0) {
        //LOGE("%s video stream is null.", __FUNCTION__);
        return -1;
    }

    //LOGI("%s get video height : %d", __FUNCTION__,
        //demuxer_ptr->fmt_ctx->streams[demuxer_ptr->video_stream_index]->codec->height);

    return demuxer_ptr->fmt_ctx->streams[demuxer_ptr->video_stream_index]->codec->height;
}

long long demuxer_get_video_bitrate(demuxer_t * demuxer_ptr)
{
    //LOGI("%s demuxer info ptr : %x", __FUNCTION__, (unsigned int)demuxer_ptr);
    if (!demuxer_ptr || !demuxer_ptr->fmt_ctx || demuxer_ptr->video_stream_index < 0) {
        //LOGE("%s video stream is null.", __FUNCTION__);
        return -1;
    }

    //LOGI("%s get video bit rate : %lld", __FUNCTION__,
        //demuxer_ptr->fmt_ctx->streams[demuxer_ptr->video_stream_index]->codec->bit_rate);

    return demuxer_ptr->fmt_ctx->streams[demuxer_ptr->video_stream_index]->codec->bit_rate;
}

int demuxer_get_video_frame_rate(demuxer_t * demuxer_ptr)
{
    //LOGI("%s demuxer info ptr : %x", __FUNCTION__, (unsigned int)demuxer_ptr);
    if (!demuxer_ptr || !demuxer_ptr->fmt_ctx || demuxer_ptr->video_stream_index < 0) {
        //LOGE("%s video stream is null.", __FUNCTION__);
        return -1;
    }
    int num = demuxer_ptr->fmt_ctx->streams[demuxer_ptr->video_stream_index]->codec->framerate.num;
    int den = demuxer_ptr->fmt_ctx->streams[demuxer_ptr->video_stream_index]->codec->framerate.den;
    //LOGI("%s get video frame rate : %d", __FUNCTION__, num / den);

    return num / den;
}

int demuxer_get_video_gop_size(demuxer_t * demuxer_ptr)
{
    //LOGI("%s demuxer info ptr : %x", __FUNCTION__, (unsigned int)demuxer_ptr);
    if (!demuxer_ptr || !demuxer_ptr->fmt_ctx || demuxer_ptr->video_stream_index < 0) {
        //LOGE("%s video stream is null.", __FUNCTION__);
        return -1;
    }

    //LOGI("%s get video gop size : %d", __FUNCTION__,
        //demuxer_ptr->fmt_ctx->streams[demuxer_ptr->video_stream_index]->codec->gop_size);

    return demuxer_ptr->fmt_ctx->streams[demuxer_ptr->video_stream_index]->codec->gop_size;
}

int demuxer_get_video_codec_id(demuxer_t * demuxer_ptr)
{
    //LOGI("%s demuxer info ptr : %x", __FUNCTION__, (unsigned int)demuxer_ptr);
    if (!demuxer_ptr || !demuxer_ptr->fmt_ctx || demuxer_ptr->video_stream_index < 0) {
        //LOGE("%s video stream is null.", __FUNCTION__);
        return -1;
    }

    int codec_id = EVIDEO_CODEC_H264;
    switch (demuxer_ptr->fmt_ctx->streams[demuxer_ptr->video_stream_index]->codec->codec_id) {
    case AV_CODEC_ID_H264:
        codec_id = EVIDEO_CODEC_H264;
        break;
    case AV_CODEC_ID_MPEG4:
        codec_id = EVIDEO_CODEC_MPEG4;
        break;
    default:
        codec_id = EVIDEO_CODEC_H264;
        break;
    }

    //LOGI("%s get video codec id : origin : %d, demuxer: %d", __FUNCTION__,
        //demuxer_ptr->fmt_ctx->streams[demuxer_ptr->video_stream_index]->codec->codec_id, codec_id);

    return codec_id;
}

int demuxer_get_video_sps_pps(demuxer_t * demuxer_ptr, unsigned char * buf, int max_size)
{
    //LOGI("%s demuxer info ptr : %x, buf: %x, max_size:%d ", 
        //__FUNCTION__, (unsigned int)demuxer_ptr, (unsigned int)buf, max_size);
    if (!demuxer_ptr || !demuxer_ptr->fmt_ctx || demuxer_ptr->video_stream_index < 0) {
        //LOGE("%s video stream is null.", __FUNCTION__);
        return -1;
    }
    if (!buf || max_size <= 0) {
        return -1;
    }
    if (!demuxer_ptr->fmt_ctx->streams[demuxer_ptr->video_stream_index]->codecpar
        || demuxer_ptr->fmt_ctx->streams[demuxer_ptr->video_stream_index]->codecpar->extradata_size <= 0) {
        //LOGE("%s video stream codec extradata is example.", __FUNCTION__);
        return 0;
    }
    if (max_size < demuxer_ptr->fmt_ctx->streams[demuxer_ptr->video_stream_index]->codecpar->extradata_size) {
        ////LOGE("%s dest buf to small; src buffer size:%d, dest buffer size:%d.", 
            //__FUNCTION__, demuxer_ptr->fmt_ctx->streams[demuxer_ptr->video_stream_index]->codecpar->extradata_size, max_size);
        return -2;
    }
    memcpy(buf, demuxer_ptr->fmt_ctx->streams[demuxer_ptr->video_stream_index]->codecpar->extradata, 
        demuxer_ptr->fmt_ctx->streams[demuxer_ptr->video_stream_index]->codecpar->extradata_size);

    //LOGI("%s get video gop size : %d", __FUNCTION__,
        //demuxer_ptr->fmt_ctx->streams[demuxer_ptr->video_stream_index]->codecpar->extradata_size);

    return demuxer_ptr->fmt_ctx->streams[demuxer_ptr->video_stream_index]->codecpar->extradata_size;
}

int demuxer_get_video_rotate(demuxer_t * demuxer_ptr)
{
    //LOGI("%s demuxer info ptr : %x", __FUNCTION__, (unsigned int)demuxer_ptr);
    if (!demuxer_ptr || !demuxer_ptr->fmt_ctx || demuxer_ptr->audio_stream_index < 0) {
        //LOGE("%s video stream is null.", __FUNCTION__);
        return -1;
    }

    //get rotate
    //LOGI("%s get video stream metadata dict count :%d ", 
        //__FUNCTION__, av_dict_count(demuxer_ptr->fmt_ctx->streams[demuxer_ptr->video_stream_index]->metadata));

    int rotate = 0;
    AVDictionaryEntry *entry = NULL;
    while ((entry = av_dict_get(demuxer_ptr->fmt_ctx->streams[demuxer_ptr->video_stream_index]->metadata, "", entry, AV_DICT_IGNORE_SUFFIX)) != NULL) {
        //LOGI("%s  get video stream dict key value: key:%s, value:%s", __FUNCTION__, entry->key, entry->value);
        if (_stricmp(entry->key, "rotate") == 0) {
            sscanf(entry->value, "%d", &rotate);
            break;
        }
    }
    
    //success
    //LOGI("%s get video rotate : %d", __FUNCTION__, rotate);
    return rotate;
}

int demuxer_get_audio_sample_rate(demuxer_t * demuxer_ptr)
{
    //LOGI("%s demuxer info ptr : %x", __FUNCTION__, (unsigned int)demuxer_ptr);
    if (!demuxer_ptr || !demuxer_ptr->fmt_ctx || demuxer_ptr->audio_stream_index < 0) {
        //LOGE("%s audio stream is null.", __FUNCTION__);
        return -1;
    }

    //LOGI("%s get audio sample rate : %d", __FUNCTION__,
        //demuxer_ptr->fmt_ctx->streams[demuxer_ptr->audio_stream_index]->codec->sample_rate);

    return demuxer_ptr->fmt_ctx->streams[demuxer_ptr->audio_stream_index]->codec->sample_rate;
}

long long demuxer_get_audio_bitrate(demuxer_t * demuxer_ptr)
{
    //LOGI("%s demuxer info ptr : %x", __FUNCTION__, (unsigned int)demuxer_ptr);
    if (!demuxer_ptr || !demuxer_ptr->fmt_ctx || demuxer_ptr->audio_stream_index < 0) {
        //LOGE("%s audio stream is null.", __FUNCTION__);
        return -1;
    }

    //LOGI("%s get audio bit rate : %lld", __FUNCTION__,
        //demuxer_ptr->fmt_ctx->streams[demuxer_ptr->audio_stream_index]->codec->bit_rate);

    return demuxer_ptr->fmt_ctx->streams[demuxer_ptr->audio_stream_index]->codec->bit_rate;
}

int demuxer_get_audio_channel_count(demuxer_t * demuxer_ptr)
{
    //LOGI("%s demuxer info ptr : %x", __FUNCTION__, (unsigned int)demuxer_ptr);
    if (!demuxer_ptr || !demuxer_ptr->fmt_ctx || demuxer_ptr->audio_stream_index < 0) {
        //LOGE("%s audio stream is null.", __FUNCTION__);
        return -1;
    }

    //LOGI("%s get audio channel count : %d", __FUNCTION__,
        //demuxer_ptr->fmt_ctx->streams[demuxer_ptr->audio_stream_index]->codec->channels);

    return demuxer_ptr->fmt_ctx->streams[demuxer_ptr->audio_stream_index]->codec->channels;
}

int demuxer_get_audio_bit_width(demuxer_t * demuxer_ptr)
{
    //LOGI("%s demuxer info ptr : %x", __FUNCTION__, (unsigned int)demuxer_ptr);
    if (!demuxer_ptr || !demuxer_ptr->fmt_ctx || demuxer_ptr->audio_stream_index < 0) {
        //LOGE("%s audio stream is null.", __FUNCTION__);
        return -1;
    }

    //LOGI("%s get audio bit width : %d", __FUNCTION__,
        //demuxer_ptr->fmt_ctx->streams[demuxer_ptr->audio_stream_index]->codec->bits_per_raw_sample);

    return demuxer_ptr->fmt_ctx->streams[demuxer_ptr->audio_stream_index]->codec->bits_per_raw_sample;
}

int demuxer_get_audio_codec_id(demuxer_t * demuxer_ptr)
{
    //LOGI("%s demuxer info ptr : %x", __FUNCTION__, (unsigned int)demuxer_ptr);
    if (!demuxer_ptr || !demuxer_ptr->fmt_ctx || demuxer_ptr->audio_stream_index < 0) {
        //LOGE("%s audio stream is null.", __FUNCTION__);
        return -1;
    }
    int codec_id = EAUDIO_CODEC_AAC;
    switch (demuxer_ptr->fmt_ctx->streams[demuxer_ptr->audio_stream_index]->codec->codec_id) {
    case AV_CODEC_ID_AAC:
        codec_id = EAUDIO_CODEC_AAC;
        break;
    default:
    {
        codec_id = EAUDIO_CODEC_AAC;
        //LOGI("%s input file audio codec id : %d not support.", 
            //__FUNCTION__, demuxer_ptr->fmt_ctx->streams[demuxer_ptr->audio_stream_index]->codec->codec_id);
    }
        break;
    }

    //LOGI("%s get audio codec id, origin codec id: %d, dest codec id: %d", 
        //__FUNCTION__, demuxer_ptr->fmt_ctx->streams[demuxer_ptr->audio_stream_index]->codec->codec_id, codec_id);

    return codec_id;
}

int demuxer_get_audio_esds(demuxer_t * demuxer_ptr, unsigned char * buf, int max_size)
{
    //LOGI("%s demuxer info ptr : %x, buf: %x, max_size:%d ",
        //__FUNCTION__, (unsigned int)demuxer_ptr, (unsigned int)buf, max_size);
    if (!demuxer_ptr || !demuxer_ptr->fmt_ctx || demuxer_ptr->audio_stream_index < 0) {
        //LOGE("%s audio stream is null.", __FUNCTION__);
        return -1;
    }
    if (!buf || max_size <= 0) {
        return -1;
    }
    if (!demuxer_ptr->fmt_ctx->streams[demuxer_ptr->audio_stream_index]->codecpar 
        || demuxer_ptr->fmt_ctx->streams[demuxer_ptr->audio_stream_index]->codecpar->extradata_size <= 0) {
        //LOGE("%s audio stream codec extradata is example.", __FUNCTION__);
        return 0;
    }
    if (max_size < demuxer_ptr->fmt_ctx->streams[demuxer_ptr->audio_stream_index]->codecpar->extradata_size) {
        //LOGE("%s dest buf to small; src buffer size:%d, dest buffer size:%d.",
            //__FUNCTION__, demuxer_ptr->fmt_ctx->streams[demuxer_ptr->audio_stream_index]->codecpar->extradata_size, max_size);
        return -2;
    }
    memcpy(buf, demuxer_ptr->fmt_ctx->streams[demuxer_ptr->audio_stream_index]->codecpar->extradata,
        demuxer_ptr->fmt_ctx->streams[demuxer_ptr->audio_stream_index]->codecpar->extradata_size);

    //LOGI("%s get video gop size : %d", __FUNCTION__,
        //demuxer_ptr->fmt_ctx->streams[demuxer_ptr->audio_stream_index]->codecpar->extradata_size);

    return demuxer_ptr->fmt_ctx->streams[demuxer_ptr->audio_stream_index]->codecpar->extradata_size;
}

int demuxer_close_file(demuxer_t * demuxer_ptr)
{
    //LOGI("%s demuxer info ptr:%x", __FUNCTION__, (unsigned int)demuxer_ptr);
    if (!demuxer_ptr) {
        return -1;
    }
    if (demuxer_ptr->fmt_ctx) {
        avformat_close_input(&demuxer_ptr->fmt_ctx);
        demuxer_ptr->fmt_ctx = NULL;
    }
    if (demuxer_ptr->file_name) {
        free(demuxer_ptr->file_name);
        demuxer_ptr->file_name = NULL;
    }
    if (demuxer_ptr->av_packet) {
        if (demuxer_ptr->av_packet->data) {
            free(demuxer_ptr->av_packet->data);
            demuxer_ptr->av_packet->data = NULL;
        }
        free(demuxer_ptr->av_packet);
        demuxer_ptr->av_packet = NULL;
    }
    free(demuxer_ptr);

    //LOGI("%s success.", __FUNCTION__);
    return 0;
}

/*
*  @example
*  int main(int argc, char **argv)
*  {
*  const char *in_filename, *out_filename;
*  int stream_index = 0;
*  int *stream_mapping = NULL;
*  int stream_mapping_size = 0;
*
*  if (argc < 3) {
*  printf("usage: %s input output\n"
*  "API example program to remux a media file with libavformat and libavcodec.\n"
*  "The output format is guessed according to the file extension.\n"
*  "\n", argv[0]);
*  return 1;
*  }
*
*  in_filename = argv[1];
*  out_filename = argv[2];
*
*  demuxer_init_ffmpeg();
*
*  demuxer_t* pmuxer_t = demuxer_open_file(in_filename);
*  if (!pmuxer_t) {
*  return -1;
*  }
*  int audio_index = demuxer_get_audio_stream_index(pmuxer_t);
*  int video_index = demuxer_get_video_stream_index(pmuxer_t);
*
*  printf("get video params *****************************************************\n");
*  printf("video width:%d\n", demuxer_get_video_width(pmuxer_t));
*  printf("video height:%d\n", demuxer_get_video_height(pmuxer_t));
*  printf("video bit_rate:%lld\n", demuxer_get_video_bitrate(pmuxer_t));
*  printf("video frame rte:%d\n", demuxer_get_video_frame_rate(pmuxer_t));
*  printf("video gop size:%d\n", demuxer_get_video_gop_size(pmuxer_t));
*  printf("video rotate:%d\n", demuxer_get_video_rotate(pmuxer_t));
*  printf("video codec id:%d\n", demuxer_get_video_codec_id(pmuxer_t));
*  unsigned char video_buf[128] = { 0 };
*  printf("video sps pps:%d\n", demuxer_get_video_sps_pps(pmuxer_t, video_buf, 128));
*
*  printf("get audio params *****************************************************\n");
*  printf("audio sample rate:%d\n", demuxer_get_audio_sample_rate(pmuxer_t));
*  printf("audio bitrate:%lld\n", demuxer_get_audio_bitrate(pmuxer_t));
*  printf("audio channel count:%d\n", demuxer_get_audio_channel_count(pmuxer_t));
*  printf("audio bitwith:%d\n", demuxer_get_audio_bit_width(pmuxer_t));
*  printf("audio codec id:%d\n", demuxer_get_audio_codec_id(pmuxer_t));
*
*  unsigned char audio_buf[128] = { 0 };
*  printf("audio esds:%d\n", demuxer_get_audio_esds(pmuxer_t, audio_buf, 128));
*
*  demuxer_avpacket_t* pkt = NULL;
*  while ((pkt = demuxer_read_next_packet(pmuxer_t))) {
*  }
*  demuxer_close_file(pmuxer_t);
*  getchar();
*  return 0;
*  }
*/

#pragma once

//#ifndef __DEMUXER_H__
//#define __DEMUXER_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <libavformat/avformat.h>

#ifdef __cplusplus
}
#endif //__cplusplus

//audio or video packet info struct, return by demuxer_read_next_packet()
typedef struct demuxer_avpacket
{
    int             stream_index;   //stream index(audio or video)
    unsigned char*  data;           //pointer of data memory 
    int             capacity_size;  //data memory total size
    int             limit_size;     //data effective size in all memory
    long long       pts;            //pts
    long long       dts;            //dts (only for video)
    int             flags;          //frame type, 1:key frame.(only for video)
    long long       duration;       //frame duration
} demuxer_avpacket_t;

//create by demuxer_open_file()
typedef struct _demuxer
{
    char*				file_name;
    AVFormatContext*	fmt_ctx;
    int                 audio_stream_index;
    int                 video_stream_index;
    demuxer_avpacket_t* av_packet;
}demuxer_t;

typedef enum DVideoCodec
{
    EVIDEO_CODEC_H264 = 0,
    EVIDEO_CODEC_MPEG4,
}EVideoCodec;

typedef enum DAudioCodec
{
    EAUDIO_CODEC_AAC = 0,
}EAudioCodec;

#if defined __cplusplus
extern "C"
{
#endif // __cplusplus

/* step 1:global init ffmpeg, register demuxers
**/
extern void             demuxer_init_ffmpeg();

/* step 2
* @return success return pointer to demuxer_t, if failed, return NULL pointer.
**/
extern demuxer_t*       demuxer_open_file(const char* file_name);

/* step 3
* @param demuxer_ptr pointer to demuxer_t, create by demuxer_open_file
* @return success:video stream index; failed:-1.
**/
extern int              demuxer_get_video_stream_index(demuxer_t* demuxer_ptr);

/* step 3
* @param demuxer_ptr pointer to demuxer_t, create by demuxer_open_file
* @return success:audio stream index; failed:-1.
**/
extern int              demuxer_get_audio_stream_index(demuxer_t* demuxer_ptr);

/* step 3 : get video params
* @param demuxer_ptr pointer to demuxer_t, create by demuxer_open_file
* @return success:video params value; failed:-1.
**/
extern int              demuxer_get_video_width(demuxer_t* demuxer_ptr);
extern int              demuxer_get_video_height(demuxer_t* demuxer_ptr);
extern long long        demuxer_get_video_bitrate(demuxer_t* demuxer_ptr);
extern int              demuxer_get_video_frame_rate(demuxer_t* demuxer_ptr);
extern int              demuxer_get_video_gop_size(demuxer_t* demuxer_ptr);
extern int              demuxer_get_video_rotate(demuxer_t* demuxer_ptr);
extern int              demuxer_get_video_codec_id(demuxer_t* demuxer_ptr);
extern int              demuxer_get_video_sps_pps(demuxer_t* demuxer_ptr, unsigned char* buf, int max_size);

/* step 3 : get audio params
* @param demuxer_ptr pointer to demuxer_t, create by demuxer_open_file
* @return success:audio params value; failed:-1.
**/
extern int              demuxer_get_audio_sample_rate(demuxer_t* demuxer_ptr);
extern long long        demuxer_get_audio_bitrate(demuxer_t* demuxer_ptr);
extern int              demuxer_get_audio_channel_count(demuxer_t* demuxer_ptr);
extern int              demuxer_get_audio_bit_width(demuxer_t* demuxer_ptr);
extern int              demuxer_get_audio_codec_id(demuxer_t* demuxer_ptr);
extern int              demuxer_get_audio_esds(demuxer_t* demuxer_ptr, unsigned char* buf, int max_size);

/* step 3 : read a media packet, maybe audio or video packet.
* @param demuxer_ptr pointer to demuxer_t, create by demuxer_open_file
* @return success:pointer to demuxer_avpacket_t, failed:return NULL.
**/
extern demuxer_avpacket_t* demuxer_read_next_packet(demuxer_t* demuxer_ptr);

/** read next key frame
*/
extern demuxer_avpacket_t* demuxer_read_next_key_frame(demuxer_t* demuxer_ptr);

/* step 4 : close file & release ffmpeg context
* @param demuxer_ptr pointer to demuxer_t, create by demuxer_open_file
* @return success:0, failed:<0.
**/
extern int              demuxer_close_file(demuxer_t* demuxer_ptr);

#if defined __cplusplus
}
#endif // __cplusplus

//#endif //__DEMUXER_H

#ifndef __MUXER_h
#define __MUXER_h

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <libavutil/opt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

#ifdef __cplusplus
}
#endif //__cplusplus

// a wrapper around a	single output AVStream
typedef struct output_stream {
	AVStream*			stream;
	AVCodecContext*		codec_context;
} output_stream_t;

typedef struct _muxer
{
	char*				file_name;
	AVOutputFormat*		output_fmt;
	AVFormatContext*	fmt_ctx;
	output_stream_t*	video_out_stream;
	output_stream_t*	audio_out_stream;
}muxer_t;

typedef enum EVideoCodec
{
    EVIDEO_CODEC_H264 = 0,
    EVIDEO_CODEC_MPEG4,
}EVideoCodec;

typedef enum EAudioCodec
{
    EAUDIO_CODEC_AAC = 0,
}EAudioCodec;

#if defined __cplusplus
extern "C"
{
#endif // __cplusplus

/* step 0:global init ffmpeg, register muxers
**/
extern void muxer_init_ffmpeg();

/* step 1:create file
* @param file_name file name 
* @return success:pointer to muxer_t; failed:NULL
**/
extern muxer_t* muxer_create_file(const char* file_name);

/* step 2:add video stream
* @param muxer_ptr pointer to muxer_t, create by muxer_create_file
* @param width video width
* @param height video height
* @param bit_rate video bit_rate,unit:bps
* @param gop_size video gop size
* @param frame_per_sec framerate
* @param video_codec_id video codec type, refer to EVideoCodec
* @param rotate video rotate value
* @return success:video stream index; failed:-1
**/
extern int muxer_add_video_stream(muxer_t* muxer_ptr, int width, int height, 
    int bit_rate, int gop_size, int frame_per_sec, EVideoCodec video_codec_id, int rotate);
/* step 2:add audio stream
* @param muxer_ptr pointer to muxer_t, create by muxer_create_file
* @param sample_rate samples per second, unit:hz
* @param channel_num audio channel num, 1 or 2
* @param bit_width PCM value type, such as uint8 is 8 bit, short is 16 bit, float is 32 bit
* @param audio_codec_id video codec type, refer to EAudioCodec
* @return success:audio stream index; failed:-1
**/
extern int muxer_add_audio_stream(muxer_t* muxer_ptr, int sample_rate, 
    int channel_num, int bit_width, int bit_rate, EAudioCodec audio_codec_id);

/* step 3:start write file header
* @param muxer_ptr pointer to muxer_t, create by muxer_create_file
* @return success:0; failed:-1
**/
extern int muxer_start(muxer_t* muxer_ptr);

/* step 4:set sps pps header must before muxer_writeVideoFrame
* @param muxer_ptr pointer to muxer_t, create by muxer_create_file
* @param data pointer to data memory
* @param data_len data size
* @return success:0; failed:-1
**/
extern int muxer_set_sps_pps_header(muxer_t* muxer_ptr, unsigned char* data, int data_len);

/* step 4:set esds header must before muxer_writeAudioFrame
* @param muxer_ptr pointer to muxer_t, create by muxer_create_file
* @param data pointer to data memory
* @param data_len data size
* @return success:0; failed:-1
**/
extern int muxer_set_esds_header(muxer_t* muxer_ptr, unsigned char* data, int data_len);

/* step 5:write video frame to file
* @param muxer_ptr pointer to muxer_t, create by muxer_create_file
* @param pkt_data pointer to data memory
* @param data_len data size
* @param is_key_frame is key frame?1:0
* @param pts presentation time stamp
* @param dts decode time stamp
* @return success:0; failed:-1
**/
extern int muxer_write_video_frame(muxer_t* muxer_ptr, 
    unsigned char* pkt_data, int data_len, int is_key_frame, long long pts, long long dts);

/* step 5:write audio frame to file
* @param muxer_ptr pointer to muxer_t, create by muxer_create_file
* @param pkt_data pointer to data memory
* @param data_len data size
* @param is_key_frame is key frame?1:0
* @param pts presentation time stamp
* @param samples_in_frame samples count in current audio frame
* @return success:0; failed:-1
**/
extern int muxer_write_audio_frame(muxer_t* muxer_ptr, 
    unsigned char* pkt_data, int data_len, long long pts, int samples_in_frame);

/*step 5:write file tail & close file & release *muxer_info_ptr
* @param muxer_ptr pointer to muxer_t, create by muxer_create_file
* @return success:0; failed:-1
**/
extern int muxer_close_file(muxer_t* muxer_ptr);

#if defined __cplusplus
}
#endif // __cplusplus

#endif //__MUXER_h

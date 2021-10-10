#ifndef CASTOR3_PLAYER_H
#define CASTOR3_PLAYER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ite/ith.h"
#include "file_player.h"

#define MTAL_MAX_CSTRING_LEN    1024

typedef struct
{
    int vol_level;
    int prev_file_pos;
    char srcname[MTAL_MAX_CSTRING_LEN]; // the file name of video stream
} MTAL_SPEC;


/* MTAL */
int mtal_pb_init(cb_handler_t callback);
int mtal_pb_exit(void);
int mtal_pb_select_file(MTAL_SPEC *spec);
int mtal_pb_play(void);
int mtal_pb_pause(void);
int mtal_pb_stop(void);
int mtal_pb_play_videoloop(void);
int mtal_pb_get_total_duration(int *totaltime);
int mtal_pb_get_total_duration_ext(int *totaltime, char *filepath);
int mtal_pb_get_current_time(int *currenttime);
int mtal_pb_seekto(int file_pos);
int mtal_pb_slow_fast_play(float speed);
int mtal_pb_get_file_pos(int *file_pos);
int mtal_pb_get_audio_codec_id(char *filepath);
bool mtal_pb_check_fileplayer_playing();
int mtal_pb_InitAVDecodeEnv(void);
int mtal_pb_InitH264DecodeEnv(void);
int mtal_pb_InitAudioDecodeEnv(int samplerate, int num_channels, RTSPCLIENT_AUDIO_CODEC codec_id);
int mtal_pb_h264_decode_from_rtsp(unsigned char *buf, int size, double timestamp);
int mtal_pb_audio_decode_from_rtsp(unsigned char *buf, int size, double timestamp);
int mtal_pb_DeinitAVDecodeEnv(void);

int mtal_pb_set_freerun(int freerun);
int mtal_pb_get_freerun(void);

/* for live streaming */
int mtal_drop_to_align(void);

#ifdef __cplusplus
}
#endif

#endif

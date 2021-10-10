/*
 * iTE castor3 media player
 *
 * @file	castor3player.h
 * @author	Evan Chang
 * @version	1.0.0
 *
 */
#ifndef FILE_PLAYER
#define FILE_PLAYER

//#define MULTI_CHANNEL			1
#define TOTAL_FRAME_BUFFER_CNT 4

typedef enum
{
    PLAYER_EVENT_EOF,
    PLAYER_EVENT_OPEN_FILE_FAIL,
    PLAYER_EVENT_UNSUPPORT_FILE,
    PLAYER_EVENT_CAPTURE_DEV
} PLAYER_EVENT;

typedef void (*cb_handler_t)(PLAYER_EVENT event_id, void *arg);

typedef enum
{
    MODE_NONE,
    ADV_MODE,
    IPCAM_MODE    
} RTSPCLIENT_MODE;

typedef enum
{
    RTSP_CODEC_MPA,
    RTSP_CODEC_MPEG4_AAC,
    RTSP_CODEC_PCMU
} RTSPCLIENT_AUDIO_CODEC;

typedef struct
{
    int width;
    int height;
    int max_framerate;
} RTSP_MEDIA_INFO;

typedef enum
{
    STREAM_EOF,
    STREAM_ERROR,
    STREAM_CONNECT_FAIL,
	STREAM_START_PLAYING,
    STREAM_GET_INFO,
    STREAM_SET_VOLUME
} RTSPCLIENT_EVENT;
typedef void (*cb_RtspHandler_t)(RTSPCLIENT_EVENT event_id, void *arg);

typedef struct ithMediaPlayer
{
    /********************************************************
    * function: init
    *
    * @param
    *   mode: Assign to be Normal/DVR player.
    *
    * @return value
    *   0: Success
    *  <0: Error occur
    ********************************************************/
    int (*init)(cb_handler_t callback);

    /********************************************************
    * function: select
    *
    * @param
    *   file: Assign file name and path.
    *         Ex, A:\mediafile.mp4
    *
    * @return value
    *   0: Success
    *  <0: Error occur
    ********************************************************/
    int (*select)(const char *file, int level);

    /********************************************************
    * function: play
    *
    * @return value
    *   0: Success
    *  <0: Error occur
    ********************************************************/
    int (*play)(void);

    /********************************************************
    * function: pause
    *
    * @return value:
    *   0: pause -> play
    *   1: play  -> pause
    *  <0: Error occur
    ********************************************************/
    int (*pause)(void);

    /********************************************************
    * function: stop
    *
    * @return value
    *   0: Success
    *  <0: Error occur
    ********************************************************/
    int (*stop)(void);

    /********************************************************
    * function: deinit
    *
    * @return value
    *   0: Success
    *  <0: Error occur
    ********************************************************/
    int (*deinit)(void);

    int (*gettotaltime)(int64_t *total_time);
	int (*getcurrenttime)(int64_t *current_time);
    int (*gettotaltime_ext)(int64_t *total_time, char *filepath);
    int (*seekto)(int pos);
    int (*slow_fast_play)(float speed);
    int (*getfilepos)(double *pos);
    int (*getaudioCodecId)(int *codec_id, char *filepath);
    bool(*check_fileplayer_playing)(void);
    /********************************************************
    * function: vol_up
    *
    * @return value
    *   0: Success
    *  <0: Error occur
    ********************************************************/
    int (*vol_up)(void);

    /********************************************************
    * function: vol_down
    *
    * @return value
    *   0: Success
    *  <0: Error occur
    ********************************************************/
    int (*vol_down)(void);

    /********************************************************
    * function: mute
    *
    * @return value
    *   0: mute    -> un-mute
    *   1: un-mute -> mute
    *  <0: Error occur
    ********************************************************/
    int (*mute)(void);

    /********************************************************
    * function: drop_all_input_streams
    *
    * <hint: used only in dvr mode, drops streams from TsSource>
    *
    * @return value
    *   0: Success
    *  <0: Error occur
    ********************************************************/
    int (*drop_all_input_streams)(void);

    void (*InitAVDecodeEnv)(void);
    void (*InitH264DecodeEnv)(void);
    void (*InitAudioDecodeEnv)(int, int, RTSPCLIENT_AUDIO_CODEC);
    int (*h264_decode_from_rtsp)(unsigned char *, int, double);
    int (*audio_decode_from_rtsp)(unsigned char *, int, double);
    void (*DeinitAVDecodeEnv)(void);
    int (*play_videoloop)(void);
} ithMediaPlayer;

/*
 * Media player
 *
 * Do not release pointer, just init/deinit
 *
 */
extern ithMediaPlayer fileplayer;
extern ithMediaPlayer captureplayer;

#endif
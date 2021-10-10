#ifndef LEAF_MEDIASTREAM_H
#define LEAF_MEDIASTREAM_H

#define IPADDR_SIZE 64
#define FILE_SIZE 512
#define bool _Bool

#include "iniparser/dictionary.h"
#include "castor3player.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _RtspStreamStatus {
    RtspStreamNone,
    RtspStreamConnectFail,
    RtspStreamParsingError        
}RtspStreamStatus;

typedef enum _LeafCallDir {
	CallIncoming,  /**< incoming calls*/
	CallOutgoing   /**< outgoing calls*/	
}LeafCallDir;

typedef enum SOUND_CALLBACK_TAG{
    SOUND_START_PLAY,
    SOUND_FINISH_PLAY,
} LeafSoundStateCallback;

typedef enum _LeafCallState{
	LeafCallIdle,					/**<Initial call state */
	LeafCallIncomingReceived, /**<This is a new incoming call */
	LeafCallOutgoingInit, /**<An outgoing call is started */
	LeafCallOutgoingProgress, /**<An outgoing call is in progress */
	LeafCallOutgoingRinging, /**<An outgoing call is ringing at remote end */
	LeafCallOutgoingEarlyMedia, /**<An outgoing call is proposed early media */
	LeafCallConnected, /**<Connected, the call is answered */
	LeafCallStreamsRunning, /**<The media streams are established and running*/
	LeafCallPausing, /**<The call is pausing at the initiative of local end */
	LeafCallPaused, /**< The call is paused, remote end has accepted the pause */
	LeafCallResuming, /**<The call is being resumed by local end*/
	LeafCallRefered, /**<The call is being transfered to another party, resulting in a new outgoing call to follow immediately*/
	LeafCallError, /**<The call encountered an error*/
	LeafCallEnd, /**<The call ended normally*/
	LeafCallPausedByRemote, /**<The call is paused by remote end*/
	LeafCallUpdatedByRemote, /**<The call's parameters are updated, used for example when video is asked by remote */
	LeafCallIncomingEarlyMedia, /**<We are proposing early media to an incoming call */
	LeafCallUpdated, /**<The remote accepted the call update initiated by us */
	LeafCallReleased /**< The call object is no more retained by the core */
} LeafCallState;

typedef enum _LeafPlaySoundCase{
    Normalplay = -1,
    RepeatPlay,
}LeafPlaySoundCase;

typedef struct _sound_conf{
    int ring_level;
    int play_level;
    int rec_level;
}sound_conf_t;

typedef struct _LeafCall
{
    LeafCallDir dir;
    char localip[IPADDR_SIZE]; /* our best guess for local ipaddress for this call */
    time_t start_time; /*time at which the call was initiated*/
    time_t media_start_time; /*time at which it was accepted, media streams established*/
    LeafCallState   state;
    pthread_t Thread;
    sound_conf_t sound_conf;
    dictionary *cfgini;
    int refcnt;
    void * user_pointer;
    int audio_port;
    int video_port;
    struct _AudioStream *audiostream;  /**/
    struct _VideoStream *videostream;
    struct _RingStream *ringstream;
    struct _VoiceMemoRecordStream *voice_memo_record_stream;/**/
    struct _VoiceMemoPlayStream *voice_memo_play_stream;
    struct _MSSndCard * ring_sndcard;   /* the playback sndcard currently used */
    struct _MSSndCard * play_sndcard;   /* the playback sndcard currently used */
    struct _MSSndCard * capt_sndcard; /* the capture sndcard currently used */    
    char *refer_to;
    int up_bw; /*upload bandwidth setting at the time the call is started. Used to detect if it changes during a call */
    int audio_bw;   /*upload bandwidth used by audio */
    bool refer_pending;
    bool media_pending;
    bool audio_muted;
    bool camera_active;
    bool all_muted; /*this flag is set during early medias*/
    bool playing_ringbacktone;
    char videomemo_file[FILE_SIZE];
    char audiomemo_file[FILE_SIZE];
}LeafCall;

typedef void (*FileWriterCallback)(void *arg);
typedef void (*SoundPlayCallback)(int state);

LeafCall* leaf_init(void);
void leaf_init_video_streams (LeafCall *call, unsigned short port);
void leaf_init_audio_streams (LeafCall *call, unsigned short port);
void leaf_start_video_stream(LeafCall *call, const char *addr, int port);
void leaf_start_audio_stream(LeafCall *call, const char *addr, int port);
void leaf_stop_media_streams(LeafCall *call);
int leaf_check_ipcam_stream_status();
bool leaf_get_ipcam_stream_media_info(RTSP_MEDIA_INFO *info);
void leaf_start_ipcam_stream(LeafCall *call, const char *addr, int port);
void leaf_stop_ipcam_stream(LeafCall *call);
int leaf_take_video_snapshot(LeafCall *call, char *file,FileWriterCallback func );
int leaf_show_snapshot(LeafCall *call, int width,int height,char *file);
bool leaf_video_memo_is_recording(LeafCall *call);
int leaf_start_video_memo_record(LeafCall *call, char *file);
void leaf_stop_video_memo_record(LeafCall *call);
bool leaf_video_memo_is_play_finished(void);
int leaf_start_video_memo_playback(LeafCall *call, char *file);
void leaf_stop_video_memo_playback(LeafCall *call);
void leaf_pause_video_memo_playback();
int leaf_get_video_memo_current_time();
int leaf_get_video_memo_total_time();
bool leaf_audio_memo_is_recording(void);
void leaf_start_voice_memo_record(LeafCall *call, char *file);
void leaf_stop_voice_memo_record(LeafCall *call);
//void leaf_start_voice_memo_playback(LeafCall *call, char *file);
//void leaf_stop_voice_memo_playback(LeafCall *call);
int leaf_start_audio_memo_record(LeafCall *call, char *file);
void leaf_stop_audio_memo_record(LeafCall *call);
bool leaf_audio_sound_is_playing(void);
void leaf_start_sound_play(LeafCall *call, char *file, LeafPlaySoundCase playcase,SoundPlayCallback func);
void leaf_stop_sound_play(LeafCall *call);
void leaf_pause_sound_play(LeafCall *call,int pause);
void leaf_set_rec_level(LeafCall *call,int level);
void leaf_set_ring_level(LeafCall *call,int level);
void leaf_set_play_level(LeafCall *call,int level);
double leaf_get_wav_time(char *filename);
void leaf_start_rtsp_stream(const char *addr, int port, char *file);
void leaf_stop_rtsp_stream(void);
static void _check_auto_stop_sound(LeafCall *call);
static void *leaf_background_iterate(LeafCall *call);

#ifdef __cplusplus
}
#endif

#endif

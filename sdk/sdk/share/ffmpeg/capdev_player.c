/*
 * iTE castor3 media player
 *
 * @file    capdev_player.c
 * @author  Benson Lin
 * @version 1.0.0
 */
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <limits.h>
#include "ite/itp.h"
#include "ite/ith_video.h"
#include "ite/audio.h"

#include "isp/mmp_isp.h"
#include "ite/itv.h"
#include "fc_sync.h"

// porting
#include "config.h"
#include "libavutil/avstring.h"
#include "libavutil/colorspace.h"
#include "libavutil/mathematics.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
#include "libavutil/dict.h"
#include "libavutil/parseutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/avassert.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
#include "libavcodec/audioconvert.h"
#include "libavutil/opt.h"
#include "libavcodec/avfft.h"
#include "libswresample/swresample.h"

#if CONFIG_AVFILTER
    #include "libavfilter/avcodec.h"
    #include "libavfilter/avfilter.h"
    #include "libavfilter/avfiltergraph.h"
    #include "libavfilter/buffersink.h"
#endif

//#include "SDL/SDL.h"
//#include <SDL_thread.h>
#include "i2s/i2s.h"

#include "cmdutils.h"

#include <unistd.h>
#include <assert.h>

#include "capture_s/ite_capture.h"
#include "fc_external.h"
#include "file_player.h"

///////////////////////////////////////////////////////////////////////////////////////////
// Definitions and type
///////////////////////////////////////////////////////////////////////////////////////////
#ifdef CFG_LCD_ENABLE
    #define LCD_OUTPUT
#endif

#define SIMPLE_CAPTURE_FUNC 0

#define CaptureDevName CFG_CAPTURE_MODULE_NAME
#define SecondCaptureDevName CFG_SECOND_CAPTURE_MODULE_NAME
#define CaptureDevWidth CFG_CAPTURE_WIDTH
#define CaptureDevHeight CFG_CAPTURE_HEIGHT

//#define AV_SYNC_STC

/* no AV sync correction is done if below the AV sync threshold */
#define AV_SYNC_THRESHOLD            0.01
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD          10.0

//#define DEBUG
#define TEST_BITRATE_IN_RENDER_FRAME 0
#define TEST_BITRATE_WITHOUT_DECODE  0

#define MAX_QUEUE_SIZE               (1280 * 1024) // evaluation under 20 Mbps
#define MIN_FRAMES                   (18)

typedef struct PacketQueue {
    AVPacketList    *first_pkt, *last_pkt;
    int             nb_packets;
    int             size;
    int             abort_request;
    int64_t         lastPts;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
} PacketQueue;

typedef struct PlayerInstance {
    pthread_t       video_tid;
    pthread_t       audio_tid;
    FC_STRC_SYNC    stc;
    // control
    char            filename[256];
    int             audio_disable;
    int             video_disable;
    int             abort_request;
    int             paused;
    int             last_paused;
    int             seek_req;
    int             seek_flags;
    int64_t         seek_pos;
    int64_t         seek_rel;
    int             read_pause_return;
    int             autoexit;
    int             loop;
    unsigned int    exclock, frame_count;
    bool            is_seekto_case;

    AVFormatContext *ic;

    // audio component
    int             audio_stream;
    AVStream        *audio_st;
    PacketQueue     audioq;
    int             audioq_size;
    pthread_mutex_t audioq_mutex;
    pthread_cond_t  audioq_cond;

    AVPacket        audio_pkt_temp;
    AVPacket        audio_pkt;
    double          audio_current_pts;
    double          audio_current_pts_drift;
    int             frame_drops_early;
    int             vol_level;
    // video component
    int             video_stream;
    AVStream        *video_st;
    PacketQueue     videoq;
	float           video_speed;

    // subtitle component
    int             subtitle_stream;
    AVStream        *subtitle_st;
    //PacketQueue subtitleq;

    double          frame_timer;
    double          frame_last_pts;
    double          frame_last_delay;
    double          frame_last_duration;
    double          frame_last_dropped_pts;
    double          frame_last_returned_time;
    double          frame_last_filter_delay;
    int64_t         frame_last_dropped_pos;
    double          video_clock; ///<pts of last decoded frame / predicted pts of next decoded frame
    double          video_current_pts;
    double          video_current_pts_drift;
    int64_t         video_current_pos;
    double          video_start_pos;
    bool            get_first_pkt;
#ifdef AV_SYNC_STC
    bool            get_first_I_frame;
    bool            audio_is_sync;
    bool            first_pkt_is_audio;
#endif

    // Used in test player
    int64_t        total_duration;
    AVStream       *is;
    AVCodecContext *avctx;
    AVCodecContext *videoctx;
    AVCodecContext *audioctx;
    AVCodec        *codec;
} PlayerInstance;

typedef struct PlayerProps {
    pthread_t      read_tid;
    cb_handler_t   callback;

    // control flags
    int            show_status;
    int            av_sync_type;
    int            genpts;
    int            seek_by_bytes;
    int64_t        start_time;
    int            step;
    int            decoder_reorder_pts;
    int            mute;

    int            instCnt;
    PlayerInstance *inst;
} PlayerProps;

enum {
    AV_SYNC_AUDIO_MASTER,   /* default choice */
    AV_SYNC_VIDEO_MASTER,
    AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
};

///////////////////////////////////////////////////////////////////////////////////////////
// global var
//
///////////////////////////////////////////////////////////////////////////////////////////
static AVPacket               flush_pkt;
static pthread_mutex_t        player_mutex;
static sem_t                  sem_pause;
static bool                   is_thread_create     = false;
static int64_t                duration             = AV_NOPTS_VALUE;
static PlayerProps            *global_player_prop  = NULL;
static int64_t                global_video_pkt_pts = AV_NOPTS_VALUE;

///////////////////////////////////////////////////////////////////////////////////////////
// Functions decl
//
///////////////////////////////////////////////////////////////////////////////////////////
static int ithCapdevPlayer_stop(void);
static int ithCapdevPlayer_drop_all_input_streams(void);
static double cal_audio_threshold(PlayerInstance *is);

///////////////////////////////////////////////////////////////////////////////////////////
// Private Functions
//
///////////////////////////////////////////////////////////////////////////////////////////


static void ithCapdevPlayer_FlipLCD(void)
{
	static int         New_state   = 0;
	static int         Old_state   = 0;
	static int         state_count = 0;
	static int         cap_idx     = 0;
	uint8_t            *dbuf       = NULL;
	uint16_t      	   timeOut     = 0;
	ITV_DBUF_PROPERTY  dbufprop    = {0};
	ITE_CAP_VIDEO_INFO outdata     = {0};

  	ithCaptureGetNewFrame(&outdata);

	 while( (dbuf = itv_get_dbuf_anchor()) == NULL)
	{
		//printf("wait to get dbuf!\n");
		usleep(1000);
		timeOut++;
		if (timeOut > 1000)
	    {
	    	printf("can`t get dbuf!!\n");
	        break;
	    }
	}

	if(outdata.IsInterlaced)
		itv_enable_isp_feature(MMP_ISP_DEINTERLACE); 

    dbufprop.src_w    = outdata.OutWidth;
    dbufprop.src_h    = outdata.OutHeight;
    dbufprop.pitch_y  = outdata.PitchY;
    dbufprop.pitch_uv = outdata.PitchUV;
    dbufprop.format   = MMP_ISP_IN_YUV422;
    dbufprop.ya       = outdata.DisplayAddrY;
    dbufprop.ua       = outdata.DisplayAddrU;
    dbufprop.va       = outdata.DisplayAddrV;
   // printf("dbufprop.ya=0x%x,dbufprop.ua=0x%x,dbufprop.va=0x%x,dbufprop.src_w=%d,dbufprop.src_h=%d,dbufprop.pitch_y=%d,dbufprop.pitch_uv=%d\n",dbufprop.ya,dbufprop.ua,dbufprop.va,dbufprop.src_w,dbufprop.src_h,dbufprop.pitch_y,dbufprop.pitch_uv);
    itv_update_dbuf_anchor(&dbufprop);
  
    return;
}


static int packet_queue_put(PacketQueue *q, AVPacket *pkt)
{

}

/* packet queue handling */
static void packet_queue_init(PacketQueue *q)
{

}

static void packet_queue_flush(PacketQueue *q)
{

}

static void packet_queue_end(PacketQueue *q)
{

}

static void packet_queue_abort(PacketQueue *q)
{

}

/* return < 0 if aborted, 0 if no packet and > 0 if packet received. */
static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block)
{
}

void send_event_cap(PLAYER_EVENT event_id, void *arg)
{
    PlayerProps *pprop = global_player_prop;

    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        return;
    }

    if (pprop && pprop->callback)
    {
        pprop->callback(event_id, arg);
    }
}

static int decode_interrupt_cb(void *ctx)
{
    return (global_player_prop && global_player_prop->inst->abort_request);
}

static double synchronize_video(PlayerInstance *is, AVFrame *src_frame, double pts)
{

}

static void video_refresh_timer(PlayerInstance *is, double pts)
{

}

static void video_refresh_timer_for_rtsp_client(PlayerInstance *is, double pts)
{

}

static void render_frame(AVCodecContext *avctx, AVFrame *picture)
{
  
}

static double cal_audio_threshold(PlayerInstance *is)
{
    double threshold = 0;
    return threshold;
}

/* seek in the stream */
static void stream_seek(PlayerInstance *is, int64_t pos, int64_t rel, int seek_by_bytes)
{
  
}

static int video_thread(void *arg)
{
    return 0;
}

static int audio_thread(void *arg)
{
    return 0;
}

static int stream_component_open(PlayerInstance *is, int stream_index, void *opaque)
{
    return 0;
}

static void stream_component_close(PlayerInstance *is, int stream_index)
{
   
}

void *read_thread_cap(void *arg)
{
    PlayerProps     *pprop = (PlayerProps *) arg;
    AVFormatContext *ic    = NULL;
    int             err, ret = 0, i, instance = 0;
    int             st_index[AVMEDIA_TYPE_NB];
    AVPacket        pkt1, *pkt = &pkt1;
    int             eof               = 0;
    int             pkt_in_play_range = 0;
    PlayerInstance  *is;
    double          pts;

    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        ret = -1;
        goto fail;
    }

    is                  = pprop->inst;

    // player instances initial
    is->video_stream    = -1;
    is->audio_stream    = -1;
    is->subtitle_stream = -1;
    is->exclock         = 0;
    is->frame_count     = 0;
    is->total_duration  = 0;

    memset(st_index, -1, sizeof(st_index));

    /* init AVFormatContext */
    ic                              = avformat_alloc_context();
    ic->interrupt_callback.callback = decode_interrupt_cb; // callback when stop avformat

    //err                             = avformat_open_input(&ic, is->filename, NULL, NULL);
    is->ic                          = ic;

    for (;;)
    {
        PlayerInstance  *is = pprop->inst;
        AVFormatContext *ic = is->ic;
        if (is->abort_request)
        {
            printf("should not run here!!!\n");
            break;
        }

        ithCapdevPlayer_FlipLCD();

        if (is->paused)
        {
	        ithCapStop();    
        }

        if (is->videoq.size > MAX_QUEUE_SIZE
            || (is->videoq.nb_packets > MIN_FRAMES
                || is->audioq.nb_packets > MIN_FRAMES))
        {
            /* wait 1 ms */
            usleep(1000);
            continue;
        }

seekto:
        if (is->seek_req)
        {
            int64_t seek_target = is->seek_pos;
            int64_t seek_min    = is->seek_rel > 0 ? seek_target - is->seek_rel + 2 : INT64_MIN;
            int64_t seek_max    = is->seek_rel < 0 ? seek_target - is->seek_rel - 2 : INT64_MAX;

            printf("is->seek_req = true\n");
            av_log(NULL, AV_LOG_DEBUG, "iformat name=%s seeking...\n", is->ic->iformat->name);
            ret = avformat_seek_file(is->ic, -1, seek_min, seek_target, seek_max, is->seek_flags);
            if (ret < 0)
            {
                av_log(NULL, AV_LOG_ERROR, "%s: error while seeking\n", is->ic->filename);
            }
            else
            {
                if (is->audio_stream >= 0)
                {
                    packet_queue_flush(&is->audioq);
                }
                if (is->video_stream >= 0)
                {
                    packet_queue_flush(&is->videoq);
                    avcodec_flush_buffers(is->videoctx);
                }
                if (is->subtitle_stream >= 0)
                {
                    avcodec_flush_buffers(is->subtitle_st->codec);
                }
            }
#ifdef AV_SYNC_STC
            is->get_first_I_frame  = false;
            is->audio_is_sync      = false;
            is->first_pkt_is_audio = false;
#endif
            is->seek_req = 0;
        }


        usleep(1000);
    }
    /* wait until the end */
    while (!pprop->inst->abort_request)
    {
        if(is->seek_req) goto seekto;
        usleep(10000);
    }
    ret = 0;
fail:
    printf("terminate video\n");
    printf("terminate audio\n");
    /* close each stream */
    avio_set_interrupt_cb(NULL);

    if (ret != 0)
        av_log(NULL, AV_LOG_ERROR, "read_thread failed, ret=%d\n", ret);
    else
        av_log(NULL, AV_LOG_ERROR, "read thread %x done\n", pprop->read_tid);

    pthread_exit(NULL);
    return 0;
}

static void stream_close(PlayerProps *pprop)
{   void           *status;
    PlayerInstance *is;

    av_log(NULL, AV_LOG_DEBUG, "stream close\n");

    if ((!pprop) || (!pprop->inst))
        return;

    /* make sure one complete frame is done */
    pprop->inst->abort_request = 1;
    pthread_join(pprop->read_tid, &status);

    /* free all pictures */
    is = pprop->inst;

    if (!is)
        return;

    is->abort_request = 0;
#ifdef CFG_BUILD_ITV
    fc_deinit_sync(&is->stc);
#endif
    free(pprop->inst);
    pprop->inst    = NULL;
    pprop->instCnt = 0;

}

///////////////////////////////////////////////////////////////////////////////////////////
// Public Functions
//
///////////////////////////////////////////////////////////////////////////////////////////
static int video_thread_for_rtsp_client(void *arg)
{
    return 0;
}

static int audio_thread_for_rtsp_client(void *arg)
{
    return 0;
}

void ithCapdevPlayer_InitAVDecodeEnv()
{
  
}

void ithCapdevPlayer_InitH264DecodeEnv()
{
  return 0;
}

void ithCapdevPlayer_InitAudioDecodeEnv(int samplerate, int num_channels, RTSPCLIENT_AUDIO_CODEC codec_id)
{
  return 0;
}

int ithCapdevPlayer_h264_decode_from_rtsp(unsigned char *inputbuf, int inputbuf_size, double timestamp)
{
    return 0;
}

int ithCapdevPlayer_audio_decode_from_rtsp(unsigned char *inputbuf, int inputbuf_size, double timestamp)
{
    return 0;
}

void ithCapdevPlayer_DeinitAVDecodeEnv()
{
    return;
}

int ithCapdevPlayer_init(cb_handler_t callback)
{
    PlayerProps *pprop = NULL;
    PlayerInstance *inst = NULL;
	uint16_t            bSignalStable = 0;
	CaptureModuleDriver IrSensor;

    if (global_player_prop)
        return 1;

    printf("ithCapdevPlayer_init\n");

#ifdef DEBUG
    av_log_set_level(AV_LOG_DEBUG);
#endif

    pprop = (PlayerProps *) calloc(sizeof(char), sizeof(PlayerProps));
    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Initialize player failed\n");
        return -1;
    }

	ithCapInitialize();


	IrSensor = (CaptureModuleDriver)CaptureModuleDriver_GetDevice(CaptureDevName);
	ithCaptureSetModule(IrSensor);

	//usleep(1000 * 1000 * 3);
	bSignalStable = ithCapDeviceIsSignalStable();
	if (!bSignalStable) printf("Capture device not stable!!\n");
	

    // player properties initial, TODO use API later
    pthread_mutex_init(&player_mutex, NULL);
    pprop->callback      = callback;
    pprop->show_status   = 1;
    pprop->seek_by_bytes = -1;
    pprop->start_time    = AV_NOPTS_VALUE;

    inst = (PlayerInstance *) calloc(1, sizeof(PlayerInstance));
    if (!inst)
        av_log(NULL, AV_LOG_ERROR, "not enough memory\n");

    if (pprop->inst)
        free(pprop->inst);
    pprop->inst    = inst;
    pprop->instCnt = 1;

    global_player_prop   = pprop;
    sem_init(&sem_pause, 0, 0);
    is_thread_create     = false;
    return 0;
}

static int ithCapdevPlayer_select_file(const char *filename, int level)
{
	
	printf("%s\n",__func__);
    return 0;
}

static int ithCapdevPlayer_play(void)
{
    PlayerProps    *pprop = global_player_prop;
    PlayerInstance *is    = NULL;
    int            rc;

    pthread_mutex_lock(&player_mutex);
    printf("%s\n",__func__);

    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        pthread_mutex_unlock(&player_mutex);
        return -1;
    }

    is = pprop->inst;
    if (!is)
    {
        av_log(NULL, AV_LOG_ERROR, "No assigned stream in player\n");
        pthread_mutex_unlock(&player_mutex);
        return -1;
    }

    if (!is_thread_create)
    {
        is->get_first_pkt      = false;
#ifdef AV_SYNC_STC
        is->get_first_I_frame  = false;
        is->audio_is_sync      = false;
        is->first_pkt_is_audio = false;
#endif
        is->video_start_pos    = 0;
        is->frame_timer        = 0;
        is->frame_last_delay   = 40e-3;
        is->frame_last_pts     = 0;

        rc                     = pthread_create(&pprop->read_tid, NULL, read_thread_cap, (void *)pprop);       
        if (rc)
        {
            av_log(NULL, AV_LOG_ERROR, "create thread failed %d\n", rc);
            ithCapdevPlayer_stop();
        }

        printf("ith9850CaptureRun\n");
#if SIMPLE_CAPTURE_FUNC
        ithCapControllerSetBT601Href(CaptureDevWidth,CaptureDevHeight,CAP_IN_YUYV);
		//ithCapControllerSetBT601(CaptureDevWidth,CaptureDevHeight,CAP_IN_YUYV);
		//ithCapControllerSetBT601WithoutDE(CaptureDevWidth,CaptureDevHeight,CAP_IN_YUYV,200,(1600+200),0,599,0,599);
		//ithCapControllerSetBT656(CaptureDevWidth,CaptureDevHeight,CAP_IN_UYVY);

#else
		ithCapFire();
#endif
    }
    else
    {
        if (is->paused)
        {
	        ithCapStop();
            is->paused = false;
        }
    }

    is_thread_create = true;
    pthread_mutex_unlock(&player_mutex);
    return 0;
}

/* pause or resume the video */
static int ithCapdevPlayer_pause(void)
{
    PlayerProps    *pprop = global_player_prop;
    PlayerInstance *is    = NULL;

    printf("%s\n",__func__);
    pthread_mutex_lock(&player_mutex);


    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        pthread_mutex_unlock(&player_mutex);
        return -1;
    }

    is = pprop->inst;
    if (!is)
    {
        av_log(NULL, AV_LOG_ERROR, "No assigned stream in player\n");
        pthread_mutex_unlock(&player_mutex);
        return -1;
    }

    if (is->paused)
    {
		ithCapStop();
        is->paused       = false;
    }
    else if (is->get_first_pkt)
    {
#ifdef CFG_AUDIO_ENABLE
        iteAudioPause(1);
#endif
        is->paused = true;
    }

    pthread_mutex_unlock(&player_mutex);
    return 0;
}

static int ithCapdevPlayer_stop(void)
{
    PlayerProps    *pprop = global_player_prop;
    PlayerInstance *is    = NULL;

    printf("ithCapdevPlayer_stop\n");
    pthread_mutex_lock(&player_mutex);

    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        pthread_mutex_unlock(&player_mutex);
        return -1;
    }

    is = pprop->inst;
    if (is && is->paused) //press stop when video is paused
    {
	    ithCapStop();
        is->paused = false;
    }
    stream_close(pprop);
	ithCapStop();
	
    is_thread_create = false;

    /* default set RMI active index to 1 (single instance) */
#ifdef CFG_BUILD_ITV
    itv_flush_dbuf();
#endif

    pthread_mutex_unlock(&player_mutex);
    return 0;
}

static int ithCapdevPlayer_deinit()
{
    PlayerProps *pprop = global_player_prop;

    printf("ithCapdevPlayer_deinit\n");
    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        return -1;
    }
    pthread_mutex_destroy(&player_mutex);

    sem_destroy(&sem_pause);
    global_player_prop = NULL;

    /* Release PlayerProps if any */
    if (pprop && pprop->show_status)
    {
        if (pprop->inst)
            free(pprop->inst);
        free(pprop);
        pprop = NULL;
        printf("\n");
    }

    /* Release video request memory buffers */
    printf("ith9850CaptureTerminate\n");
    ithCapTerminate();
    return 0;
}


static int ithCapdevPlayer_play_videoloop(void)
{
	printf("ithCapdevPlayer_play_videoloop\n");
    PlayerProps    *pprop = global_player_prop;
    PlayerInstance *is    = NULL;
    int            rc;

    pthread_mutex_lock(&player_mutex);

    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        pthread_mutex_unlock(&player_mutex);
        return -1;
    }

    is = pprop->inst;
    if (!is)
    {
        av_log(NULL, AV_LOG_ERROR, "No assigned stream in player\n");
        pthread_mutex_unlock(&player_mutex);
        return -1;
    }

    if (!is_thread_create)
    {
        is->get_first_pkt      = false;
#ifdef AV_SYNC_STC
        is->get_first_I_frame  = false;
        is->audio_is_sync      = false;
        is->first_pkt_is_audio = false;
#endif
        is->video_start_pos    = 0;
        is->frame_timer        = 0;
        is->frame_last_delay   = 40e-3;
        is->frame_last_pts     = 0;
        is->video_speed        = 1;

        rc                     = pthread_create(&pprop->read_tid, NULL, read_thread_cap, (void *)pprop);
        if (rc)
        {
            av_log(NULL, AV_LOG_ERROR, "create thread failed %d\n", rc);
            ithCapdevPlayer_stop();
        }

        printf("ith9850CaptureRun\n");
#if SIMPLE_CAPTURE_FUNC
		ithCapControllerSetBT601Href(CaptureDevWidth,CaptureDevHeight,CAP_IN_YUYV);
		//ithCapControllerSetBT601(CaptureDevWidth,CaptureDevHeight,CAP_IN_YUYV);
		//ithCapControllerSetBT601WithoutDE(CaptureDevWidth,CaptureDevHeight,CAP_IN_YUYV,200,(1600+200),0,599,0,599);
		//ithCapControllerSetBT656(CaptureDevWidth,CaptureDevHeight,CAP_IN_UYVY);
#else
        ithCapFire();
#endif
    }
    else
    {
        if (is->paused)
        {
        	printf("is pause!\n");
			ithCapStop();
            is->paused = false;
        }
    }

    is_thread_create = true;
    pthread_mutex_unlock(&player_mutex);
    return 0;
}


static int ithCapdevPlayer_get_total_duration(int64_t *total_time)
{
    return 0;
}

static int ithCapdevPlayer_get_total_duration_ext(int64_t *total_time, char *filepath)
{
    return 0;
}

static int ithCapdevPlayer_get_current_time(int64_t *current_time)
{
    return 0;
}

static void ithCapdevPlayer_seekto(int pos)
{
	printf("ithCapdevPlayer_seekto\n");
    return ;
}


static void ithCapdevPlayer_slow_fast_play(float speed)
{
    PlayerProps    *pprop = global_player_prop;
    PlayerInstance *is    = NULL;

    pthread_mutex_lock(&player_mutex);

    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        pthread_mutex_unlock(&player_mutex);
        return;
    }

    is = pprop->inst;
    if (!is)
    {
        av_log(NULL, AV_LOG_ERROR, "No assigned stream in player\n");
        pthread_mutex_unlock(&player_mutex);
        return;
    }
    
    if(speed < 0.5)
        speed = 0.5;
    else if(speed > 2)
        speed = 2;
    
    is->video_speed = speed;
    pthread_mutex_unlock(&player_mutex);
    return;
}


static int ithCapdevPlayer_get_file_pos(double *pos)
{
    return 0;
}

static int ithCapdevPlayer_volume_up(void)
{
    return 0;
}

static int ithCapdevPlayer_volume_down(void)
{
    return 0;
}

static int ithCapdevPlayer_mute(void)
{
    return 0;
}

int ithCapdevPlayer_drop_all_input_streams(void)
{
    return 0;
}

#if defined(_MSC_VER)
ithMediaPlayer captureplayer = {
    ithCapdevPlayer_init,
    ithCapdevPlayer_select_file,
    ithCapdevPlayer_play,
    ithCapdevPlayer_pause,
    ithCapdevPlayer_stop,
    ithCapdevPlayer_play_videoloop,
    ithCapdevPlayer_deinit,
    ithCapdevPlayer_get_total_duration,
    ithCapdevPlayer_get_total_duration_ext,
    ithCapdevPlayer_get_current_time,
    ithCapdevPlayer_seekto,
    ithCapdevPlayer_slow_fast_play,
    ithCapdevPlayer_get_file_pos,
    ithCapdevPlayer_volume_up,
    ithCapdevPlayer_volume_down,
    ithCapdevPlayer_mute,
    ithCapdevPlayer_drop_all_input_streams,
    ithCapdevPlayer_InitAVDecodeEnv,
    ithCapdevPlayer_InitH264DecodeEnv,
    ithCapdevPlayer_InitAudioDecodeEnv,
    ithCapdevPlayer_h264_decode_from_rtsp,
    ithCapdevPlayer_audio_decode_from_rtsp,
    ithCapdevPlayer_DeinitAVDecodeEnv
    
};
#else // no defined _MSC_VER
ithMediaPlayer captureplayer = {
    .init                   = ithCapdevPlayer_init,
    .select                 = ithCapdevPlayer_select_file,
    .play                   = ithCapdevPlayer_play,
    .pause                  = ithCapdevPlayer_pause,
    .stop                   = ithCapdevPlayer_stop,
    .play_videoloop			= ithCapdevPlayer_play_videoloop,
    .deinit                 = ithCapdevPlayer_deinit,
    .gettotaltime           = ithCapdevPlayer_get_total_duration,
    .gettotaltime_ext       = ithCapdevPlayer_get_total_duration_ext,
    .getcurrenttime         = ithCapdevPlayer_get_current_time,
    .seekto                 = ithCapdevPlayer_seekto,
    .slow_fast_play         = ithCapdevPlayer_slow_fast_play,
    .getfilepos             = ithCapdevPlayer_get_file_pos,
    .vol_up                 = ithCapdevPlayer_volume_up,
    .vol_down               = ithCapdevPlayer_volume_down,
    .mute                   = ithCapdevPlayer_mute,
    .drop_all_input_streams = ithCapdevPlayer_drop_all_input_streams,
    .InitAVDecodeEnv        = ithCapdevPlayer_InitAVDecodeEnv,
    .InitH264DecodeEnv      = ithCapdevPlayer_InitH264DecodeEnv,
    .InitAudioDecodeEnv     = ithCapdevPlayer_InitAudioDecodeEnv,
    .h264_decode_from_rtsp  = ithCapdevPlayer_h264_decode_from_rtsp,
    .audio_decode_from_rtsp = ithCapdevPlayer_audio_decode_from_rtsp,
    .DeinitAVDecodeEnv      = ithCapdevPlayer_DeinitAVDecodeEnv
   
};
#endif
//ithMediaPlayer *media_player = &captureplayer;

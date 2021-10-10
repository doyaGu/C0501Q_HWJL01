/*
 * iTE castor3 media player
 *
 * @file    castor3player.c
 * @author  Evan Chang
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

#include "fc_external.h"
#include "file_player.h"

///////////////////////////////////////////////////////////////////////////////////////////
// Definitions and type
///////////////////////////////////////////////////////////////////////////////////////////
#ifdef CFG_LCD_ENABLE
    #define LCD_OUTPUT
#endif

//#define AV_SYNC_STC

/* no AV sync correction is done if below the AV sync threshold */
#define AV_SYNC_THRESHOLD            0.01
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD          10.0

//#define DEBUG
#define TEST_BITRATE_IN_RENDER_FRAME 0
#define TEST_BITRATE_WITHOUT_DECODE  0

#if (CFG_CHIP_FAMILY == 9850)
#if (CFG_CHIP_PKG_IT9856 || CFG_CHIP_PKG_IT9854)
#define MAX_QUEUE_SIZE               (2*1024*1024)
#define MIN_FRAMES                   (100)
#else //9852
#define MAX_QUEUE_SIZE               (1280*1024)
#define MIN_FRAMES                   (18)
#endif
#else
#define MAX_QUEUE_SIZE               (2*1024*1024)
#define MIN_FRAMES                   (100)
#endif

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
    bool            is_videospeed_changed;

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
    double          ori_frame_last_pts;
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
    bool                isFilePlayer_thread_created;
    
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
AVPacket               flush_pkt;
pthread_mutex_t        player_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t        video_speed_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t                  sem_pause;
/*static*/ int64_t     duration             = AV_NOPTS_VALUE;
/*static*/ PlayerProps *global_player_prop  = NULL;
int64_t                global_video_pkt_pts = AV_NOPTS_VALUE;

///////////////////////////////////////////////////////////////////////////////////////////
// Functions decl
//
///////////////////////////////////////////////////////////////////////////////////////////
static int ithMediaPlayer_stop(void);
static int ithMediaPlayer_drop_all_input_streams(void);
static double cal_audio_threshold(PlayerInstance *is);

///////////////////////////////////////////////////////////////////////////////////////////
// Private Functions
//
///////////////////////////////////////////////////////////////////////////////////////////
static int packet_queue_put(PacketQueue *q, AVPacket *pkt)
{
    AVPacketList *pkt1;

    /* duplicate the packet */
    if (q->mutex)
    {
        if (pkt != &flush_pkt && av_dup_packet(pkt) < 0)
            return -1;

        pkt1       = av_malloc(sizeof(AVPacketList));
        if (!pkt1)
            return -1;
        pkt1->pkt  = *pkt;
        pkt1->next = NULL;

        pthread_mutex_lock(&q->mutex);

        if (!q->last_pkt)
            q->first_pkt = pkt1;
        else
            q->last_pkt->next = pkt1;
        q->last_pkt = pkt1;
        q->nb_packets++;
        q->size    += pkt1->pkt.size + sizeof(*pkt1);
        q->lastPts  = pkt->pts;
        /* XXX: should duplicate packet data in DV case */
        pthread_cond_signal(&q->cond);
        pthread_mutex_unlock(&q->mutex);
        return 0;
    }
    else
    {
        return -1;
    }
}

/* packet queue handling */
static void packet_queue_init(PacketQueue *q)
{
    memset(q, 0, sizeof(PacketQueue));
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
    //packet_queue_put(q, &flush_pkt);
}

static void packet_queue_flush(PacketQueue *q)
{
    AVPacketList *pkt, *pkt1;
    if (q->mutex)
    {
        pthread_mutex_lock(&q->mutex);
        for (pkt = q->first_pkt; pkt != NULL; pkt = pkt1)
        {
            pkt1 = pkt->next;
            av_free_packet(&pkt->pkt);
            av_freep(&pkt);
        }
        q->last_pkt   = NULL;
        q->first_pkt  = NULL;
        q->nb_packets = 0;
        q->size       = 0;
        pthread_mutex_unlock(&q->mutex);
    }
}

static void packet_queue_end(PacketQueue *q)
{
    if (q->mutex)
    {
        packet_queue_flush(q);
        pthread_mutex_destroy(&q->mutex);
        pthread_cond_destroy(&q->cond);
        memset(q, 0, sizeof(PacketQueue));
    }
}

static void packet_queue_abort(PacketQueue *q)
{
    if (q->mutex)
    {
        pthread_mutex_lock(&q->mutex);
        q->abort_request = 1;
        pthread_cond_signal(&q->cond);
        pthread_mutex_unlock(&q->mutex);
    }
}

/* return < 0 if aborted, 0 if no packet and > 0 if packet received. */
static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block)
{
    AVPacketList *pkt1;
    int          ret;
    if (q->mutex)
    {
        pthread_mutex_lock(&q->mutex);

        for (;;)
        {
            if (q->abort_request)
            {
                ret = -1;
                break;
            }

            pkt1 = q->first_pkt;
            if (pkt1)
            {
                q->first_pkt = pkt1->next;
                if (!q->first_pkt)
                    q->last_pkt = NULL;
                q->nb_packets--;
                q->size -= pkt1->pkt.size + sizeof(*pkt1);
                *pkt     = pkt1->pkt;
                av_free(pkt1);
                ret      = 1;
                break;
            }
            else if (!block)
            {
                ret = 0;
                break;
            }
            else
            {
                av_log(NULL, AV_LOG_DEBUG, "queue empty, condition wait %lld\n", av_gettime());
                pthread_cond_wait(&q->cond, &q->mutex);
                av_log(NULL, AV_LOG_DEBUG, "leave condition wait %lld\n", av_gettime());
            }
        }
        pthread_mutex_unlock(&q->mutex);
        return ret;
    }
    else
    {
        return -1;
    }
}

void send_event(PLAYER_EVENT event_id, void *arg)
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

double synchronize_video(PlayerInstance *is, AVFrame *src_frame, double pts)
{
    double frame_delay;

    if (pts != 0)
    {
        /* if we have pts, set video clock to it */
        is->video_clock = pts;
    }
    else
    {
        /* if we aren't given a pts, set it to the clock */
        pts = is->video_clock;
    }
    /* update the video clock */
    frame_delay      = av_q2d(is->video_st->codec->time_base);
    /* if we are repeating a frame, adjust clock accordingly */
    frame_delay     += src_frame->repeat_pict * (frame_delay * 0.5);
    is->video_clock += frame_delay;
    return pts;
}

void video_refresh_timer(PlayerInstance *is, double pts)
{
    PlayerProps *pprop      = global_player_prop;
    double      actual_delay, delay, sync_threshold, ref_clock, diff, audio_threshold;
    int64_t     stc_time;
    uint32_t    audio_clock = 0;
    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        return;
    }

    //printf("YC: video timestamp = %f\n", pts);
#ifdef AV_SYNC_STC
    pthread_mutex_lock(&video_speed_mutex);
    if (!is->get_first_I_frame || is->is_videospeed_changed || pts < is->ori_frame_last_pts)
    {
        is->is_videospeed_changed = false;
        if (is->first_pkt_is_audio)
        {
    #ifdef CFG_BUILD_ITV
            fc_sync_gettime(&is->stc, &stc_time);
    #endif
            ref_clock = (double)stc_time / 90000;
            diff      = pts - ref_clock;
            if (diff > 0)
                usleep(diff * 1000000);
        }
        is->get_first_I_frame = true;
    #if (CFG_CHIP_FAMILY == 9070)
        audio_threshold       = cal_audio_threshold(is);
        usleep(audio_threshold * 1000000);
    #endif        
    #ifdef CFG_BUILD_ITV
        fc_sync_settime(&is->stc, pts * 90000);
    #endif
        is->frame_last_delay = 0;
        is->frame_last_pts = pts;
        is->frame_timer      = pts;
        is->ori_frame_last_pts = pts;
    }
    else
    {
        if(is->video_speed != 1)
        {
            double interval;
            interval = (pts - is->ori_frame_last_pts)/is->video_speed;
            is->ori_frame_last_pts = pts;
            pts = is->frame_last_pts + interval;
        }
        else
        {
            is->ori_frame_last_pts = pts;
        }
        delay = pts - is->frame_last_pts;
        //printf("YC: delay0 = %f\n", delay);

        is->frame_last_delay = delay;
        is->frame_last_pts   = pts;

    #ifdef CFG_BUILD_ITV
        fc_sync_gettime(&is->stc, &stc_time);
    #endif

        ref_clock      = (double)stc_time / 90000;
    #if defined(CFG_ENABLE_ROTATE) && defined(CFG_AIRCONDITIONER)
        ref_clock -= 0.5;
    #endif
        //diff = pts - ref_clock - is->video_start_pos;
        diff           = pts - ref_clock;
        //printf("YC: diff = %f, pts = %f, ref_clock = %f\n", diff, pts, ref_clock);
        sync_threshold = (delay > AV_SYNC_THRESHOLD) ? delay : AV_SYNC_THRESHOLD;

        if (fabs(diff) < AV_NOSYNC_THRESHOLD)
        {
            if (diff <= -sync_threshold)
            {
                delay = 0;
            }
            else if (diff >= sync_threshold)
            {
                delay = 2 * delay;
            }
        }

        is->frame_timer += delay;

        actual_delay     = is->frame_timer - ref_clock;
        if (actual_delay < 0.005)
        {
            /* Really it should skip the picture instead */
            actual_delay = 0;
        }

        //printf("YC: actual_delay = %f\n", actual_delay);
        usleep(actual_delay * 1000000);
        //usleep(delay * 1000000);
    }
    pthread_mutex_unlock(&video_speed_mutex);
#else  //audio master
    if (is->frame_last_pts == 0 && is->is_seekto_case)
    {
    #ifdef CFG_BUILD_ITV
        fc_sync_settime(&pprop->stc, (int64_t)pts * 90000);
    #endif
        is->video_start_pos = pts;
        is->is_seekto_case  = false;
    }

    delay = pts - is->frame_last_pts;
    //printf("YC: delay0 = %f\n", delay);

    if (delay <= 0 || delay >= 1.0)
    {
        delay = is->frame_last_delay;
    }

    is->frame_last_delay = delay;
    is->frame_last_pts   = pts;

    #ifndef WIN32
    iteAudioGetDecodeTimeV2(&audio_clock);
    #endif
    ref_clock = (double)audio_clock / 1000;
    #if 0
        #ifdef CFG_BUILD_ITV
    fc_sync_gettime(&pprop->stc, &stc_time);
        #endif

    ref_clock = (double)stc_time / 90000;
    #endif

    //    printf("YC: current pts=%f, stc_time = %f, delay_1 = %f\n", pts, ref_clock, delay);

    diff           = pts - ref_clock - is->video_start_pos;
    //printf("YC: diff = %f\n", diff);
    sync_threshold = (delay > AV_SYNC_THRESHOLD) ? delay : AV_SYNC_THRESHOLD;

    if (fabs(diff) < AV_NOSYNC_THRESHOLD)
    {
        if (diff <= -sync_threshold)
        {
            delay = 0;
        }
        else if (diff >= sync_threshold)
        {
            delay = 2 * delay;
        }
    }

    is->frame_timer += delay;

    actual_delay     = is->frame_timer - ref_clock;
    if (actual_delay < 0.005)
    {
        /* Really it should skip the picture instead */
        actual_delay = 0;
    }

    //printf("YC: actual_delay = %f\n", actual_delay);
    usleep(actual_delay * 1000000);
    //usleep(delay * 1000000);
#endif
}

void video_refresh_timer_for_rtsp_client(PlayerInstance *is, double pts)
{
    PlayerProps *pprop      = global_player_prop;
    double      actual_delay, delay, sync_threshold, ref_clock, diff, audio_threshold;
    int64_t     stc_time;
    uint32_t    audio_clock = 0;
    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        return;
    }

    //printf("YC: video timestamp = %f\n", pts);
#ifdef AV_SYNC_STC
    if (!is->get_first_I_frame)
    {
        if (is->first_pkt_is_audio)
        {
    #ifdef CFG_BUILD_ITV
            fc_sync_gettime(&is->stc, &stc_time);
    #endif
            ref_clock       = (double)stc_time / 90000;
            diff            = pts - ref_clock;
            if (diff > 0)
                usleep(diff * 1000000);
    #if (CFG_CHIP_FAMILY == 9070)            
            audio_threshold = cal_audio_threshold(is);
            usleep(audio_threshold * 1000000);
    #endif        
        }
        is->get_first_I_frame = true;
    #ifdef CFG_BUILD_ITV
        fc_sync_settime(&is->stc, pts * 90000);
    #endif
        is->frame_last_delay = 0;
        is->frame_last_pts = is->ori_frame_last_pts = pts;
        is->frame_timer      = pts;
    }
    else
    {
        delay = pts - is->frame_last_pts;
        //printf("YC: delay0 = %f\n", delay);

        if (delay <= 0 || delay >= 1.0)
        {
            delay = is->frame_last_delay;
        }

        is->frame_last_delay = delay;
        is->frame_last_pts = is->ori_frame_last_pts = pts;

    #ifdef CFG_BUILD_ITV
        fc_sync_gettime(&is->stc, &stc_time);
    #endif

        ref_clock      = (double)stc_time / 90000;

        //diff = pts - ref_clock - is->video_start_pos;
        diff           = pts - ref_clock;
        //printf("YC: diff = %f, pts = %f, ref_clock = %f\n", diff, pts, ref_clock);
        sync_threshold = (delay > AV_SYNC_THRESHOLD) ? delay : AV_SYNC_THRESHOLD;

        if (fabs(diff) < AV_NOSYNC_THRESHOLD)
        {
            if (diff <= -sync_threshold)
            {
                delay = 0;
            }
            else if (diff >= sync_threshold)
            {
                delay = 2 * delay;
            }
        }

        is->frame_timer += delay;

        actual_delay     = is->frame_timer - ref_clock;
        if (actual_delay < 0.005)
        {
            /* Really it should skip the picture instead */
            actual_delay = 0;
        }

        //printf("YC: actual_delay = %f\n", actual_delay);
        usleep(actual_delay * 1000000);
        //usleep(delay * 1000000);
    }
#else  //audio master
    if (is->frame_last_pts == 0 && is->is_seekto_case)
    {
    #ifdef CFG_BUILD_ITV
        fc_sync_settime(&pprop->stc, (int64_t)pts * 90000);
    #endif
        is->video_start_pos = pts;
        is->is_seekto_case  = false;
    }

    delay = pts - is->frame_last_pts;
    //printf("YC: delay0 = %f\n", delay);

    if (delay <= 0 || delay >= 1.0)
    {
        delay = is->frame_last_delay;
    }

    is->frame_last_delay = delay;
    is->frame_last_pts   = pts;

    #ifndef WIN32
    iteAudioGetDecodeTimeV2(&audio_clock);
    #endif
    ref_clock = (double)audio_clock / 1000;
    #if 0
        #ifdef CFG_BUILD_ITV
    fc_sync_gettime(&pprop->stc, &stc_time);
        #endif

    ref_clock = (double)stc_time / 90000;
    #endif

    //    printf("YC: current pts=%f, stc_time = %f, delay_1 = %f\n", pts, ref_clock, delay);

    diff           = pts - ref_clock - is->video_start_pos;
    //printf("YC: diff = %f\n", diff);
    sync_threshold = (delay > AV_SYNC_THRESHOLD) ? delay : AV_SYNC_THRESHOLD;

    if (fabs(diff) < AV_NOSYNC_THRESHOLD)
    {
        if (diff <= -sync_threshold)
        {
            //printf("YC:step1\n");
            delay = 0;
        }
        else if (diff >= sync_threshold)
        {
            //printf("YC:step2\n");
            delay = 2 * delay;
        }
        //else
        //    delay = 0.8 * delay;
    }

    is->frame_timer += delay;

    actual_delay     = is->frame_timer - ref_clock;
    if (actual_delay < 0.005)
    {
        /* Really it should skip the picture instead */
        actual_delay = 0;
    }

    //printf("YC: actual_delay = %f\n", actual_delay);
    usleep(actual_delay * 1000000);
    //usleep(delay * 1000000);
#endif
}

void video_refresh_timer_for_videoloop(PlayerInstance *is, double pts)
{
    PlayerProps *pprop      = global_player_prop;
    double      actual_delay, delay, sync_threshold, ref_clock, diff, audio_threshold;
    int64_t     stc_time;
    uint32_t    audio_clock = 0;
    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        return;
    }

    //printf("YC: video timestamp = %f\n", pts);
#ifdef AV_SYNC_STC
    pthread_mutex_lock(&video_speed_mutex);
    if (!is->get_first_I_frame || is->is_videospeed_changed || pts < is->ori_frame_last_pts)
    {
        is->is_videospeed_changed = false;
        if (is->first_pkt_is_audio)
        {
    #ifdef CFG_BUILD_ITV
            fc_sync_gettime(&is->stc, &stc_time);
    #endif
            ref_clock = (double)stc_time / 90000;
            diff      = pts - ref_clock;
            if (diff > 0)
                usleep(diff * 1000000);
        }
        is->get_first_I_frame = true;
    #if (CFG_CHIP_FAMILY == 9070)    
        audio_threshold       = cal_audio_threshold(is);
        usleep(audio_threshold * 1000000);
    #endif    
    #ifdef CFG_BUILD_ITV
        fc_sync_settime(&is->stc, pts * 90000);
    #endif
        is->frame_last_delay = 0;
        is->frame_last_pts = pts;
        is->frame_timer      = pts;
        is->ori_frame_last_pts = pts;
    }
    else
    {
        if(is->video_speed != 1)
        {
            double interval;
            interval = (pts - is->ori_frame_last_pts)/is->video_speed;
            is->ori_frame_last_pts = pts;
            pts = is->frame_last_pts + interval;
        }
        else
        {
            is->ori_frame_last_pts = pts;
        }
        delay = pts - is->frame_last_pts;
        //printf("YC: delay0 = %f\n", delay);

        is->frame_last_delay = delay;
        is->frame_last_pts   = pts;

    #ifdef CFG_BUILD_ITV
        fc_sync_gettime(&is->stc, &stc_time);
    #endif

        ref_clock      = (double)stc_time / 90000;

        //diff = pts - ref_clock - is->video_start_pos;
        diff           = pts - ref_clock;
        //printf("YC: diff = %f, pts = %f, ref_clock = %f\n", diff, pts, ref_clock);
        sync_threshold = (delay > AV_SYNC_THRESHOLD) ? delay : AV_SYNC_THRESHOLD;

        if (fabs(diff) < AV_NOSYNC_THRESHOLD)
        {
            if (diff <= -sync_threshold)
            {
                delay = 0;
            }
            else if (diff >= sync_threshold)
            {
                delay = 2 * delay;
            }
        }

        is->frame_timer += delay;

        actual_delay     = is->frame_timer - ref_clock;
        if (actual_delay < 0.005)
        {
            /* Really it should skip the picture instead */
            actual_delay = 0;
        }

        //printf("YC: actual_delay = %f\n", actual_delay);
        usleep(actual_delay * 1000000);
        //usleep(delay * 1000000);
    }
    pthread_mutex_unlock(&video_speed_mutex);
#else  //audio master
    if (is->frame_last_pts == 0 && is->is_seekto_case)
    {
    #ifdef CFG_BUILD_ITV
        fc_sync_settime(&pprop->stc, (int64_t)pts * 90000);
    #endif
        is->video_start_pos = pts;
        is->is_seekto_case  = false;
    }

    delay = pts - is->frame_last_pts;
    //printf("YC: delay0 = %f\n", delay);

    if (delay <= 0 || delay >= 1.0)
    {
        delay = is->frame_last_delay;
    }

    is->frame_last_delay = delay;
    is->frame_last_pts   = pts;

    #ifndef WIN32
    iteAudioGetDecodeTimeV2(&audio_clock);
    #endif
    ref_clock = (double)audio_clock / 1000;
    #if 0
        #ifdef CFG_BUILD_ITV
    fc_sync_gettime(&pprop->stc, &stc_time);
        #endif

    ref_clock = (double)stc_time / 90000;
    #endif

    //    printf("YC: current pts=%f, stc_time = %f, delay_1 = %f\n", pts, ref_clock, delay);

    diff           = pts - ref_clock - is->video_start_pos;
    //printf("YC: diff = %f\n", diff);
    sync_threshold = (delay > AV_SYNC_THRESHOLD) ? delay : AV_SYNC_THRESHOLD;

    if (fabs(diff) < AV_NOSYNC_THRESHOLD)
    {
        if (diff <= -sync_threshold)
        {
            delay = 0;
        }
        else if (diff >= sync_threshold)
        {
            delay = 2 * delay;
        }
    }

    is->frame_timer += delay;

    actual_delay     = is->frame_timer - ref_clock;
    if (actual_delay < 0.005)
    {
        /* Really it should skip the picture instead */
        actual_delay = 0;
    }

    //printf("YC: actual_delay = %f\n", actual_delay);
    usleep(actual_delay * 1000000);
    //usleep(delay * 1000000);
#endif
}


#if 1
void render_frame(AVCodecContext *avctx, AVFrame *picture)
{
    bool              bprint = true;
    ITV_DBUF_PROPERTY prop   = {0};

    while (1)
    {
        uint8_t *dbuf = NULL;

    #ifdef CFG_BUILD_ITV
        dbuf = itv_get_dbuf_anchor();
    #endif

        if (dbuf != NULL)
        {
            uint32_t rdIndex = picture->opaque ? *(uint32_t *)picture->opaque : 0;
            int      bidx;

            bidx          = rdIndex;

#if !defined(CFG_DOORBELL_INDOOR) && !defined(CFG_DOORBELL_ADMIN)
            if(picture->width != avctx->width)
                picture->width = avctx->width;
            if(picture->height != avctx->height)
                picture->height = avctx->height;
#endif

            prop.src_w    = picture->width;
            prop.src_h    = picture->height;
            prop.ya       = picture->data[0];
            prop.ua       = picture->data[1];
            prop.va       = picture->data[2];
            prop.pitch_y  = picture->linesize[0]; //width; //2048
            prop.pitch_uv = picture->linesize[1]; //width/2; //2048
            prop.bidx     = bidx;
            prop.format   = MMP_ISP_IN_YUV420;

    #ifdef CFG_BUILD_ITV
            itv_update_dbuf_anchor(&prop);
    #endif

            av_log(NULL, AV_LOG_DEBUG, "disp(%d)\n", bidx);
            break;
        }
        else
        {
            if (bprint)
            {
                av_log(NULL, AV_LOG_DEBUG, "get no display buf");
                bprint = false;
            }
            av_log(NULL, AV_LOG_DEBUG, ".");
            usleep(1000);
        }
    }
    av_log(NULL, AV_LOG_DEBUG, "\n");
}
#else
void render_frame(AVCodecContext *avctx, AVFrame *picture)
{
    static i;
    char   filename[1024];
    FILE   *f;
    sprintf(filename, "d:/test/%08d.yuv", i++);

    f = fopen(filename, "wb+");
    if (f)
    {
        int line;
        for (line = 0; line < picture->height; ++line)
            fwrite(picture->data[0] + line * picture->linesize[0], 1, picture->width, f);
        fclose(f);
    }
}
#endif

static double cal_audio_threshold(PlayerInstance *is)
{
    double threshold = 0;

    if (!is || !is->audioctx)
        return 0;

    switch (is->audioctx->codec_id)
    {
    case CODEC_ID_MP2:
    case CODEC_ID_MP3:
        threshold = 0.2;
        break;

    case CODEC_ID_PCM_S16LE:
    case CODEC_ID_PCM_S16BE:
    case CODEC_ID_PCM_U16LE:
    case CODEC_ID_PCM_U16BE:
    case CODEC_ID_PCM_S8:
    case CODEC_ID_PCM_U8:
    case CODEC_ID_PCM_ALAW:
    case CODEC_ID_PCM_MULAW:
    case CODEC_ID_ADPCM_IMA_WAV:
    case CODEC_ID_ADPCM_SWF:
        threshold = 0.2;
        break;

    case CODEC_ID_AAC:
        threshold = 0.2;
        break;

    case CODEC_ID_AC3:
    case CODEC_ID_EAC3:
        threshold = 0.2;
        break;

    case CODEC_ID_WMAV2:
        threshold = 0.2;
        break;

    case CODEC_ID_AMR_NB:
        threshold = 0.2;

    default:
        break;
    }

    return threshold;
}

#if 0
static void flush_audio_hw(PlayerInstance *is)
{
    AVFormatContext *ic          = is->ic;
    int             audio_stream = is->audio_stream;
    int             nAudioEngine = 0;

    iteAudioStop();
    switch (ic->streams[audio_stream]->codec->codec_id)
    {
    case CODEC_ID_MP2:
    case CODEC_ID_MP3:
        nAudioEngine = ITE_MP3_DECODE;
        break;

    case CODEC_ID_PCM_S16LE:
    case CODEC_ID_PCM_S16BE:
    case CODEC_ID_PCM_U16LE:
    case CODEC_ID_PCM_U16BE:
    case CODEC_ID_PCM_S8:
    case CODEC_ID_PCM_U8:
    case CODEC_ID_PCM_ALAW:
    case CODEC_ID_PCM_MULAW:
    case CODEC_ID_ADPCM_IMA_WAV:
    case CODEC_ID_ADPCM_SWF:
        nAudioEngine = ITE_WAV_DECODE;
        break;

    case CODEC_ID_AAC:
        nAudioEngine = ITE_AAC_DECODE;
        break;

    case CODEC_ID_AC3:
    case CODEC_ID_EAC3:
        nAudioEngine = ITE_AC3_DECODE;
        break;

    case CODEC_ID_WMAV2:
        nAudioEngine = ITE_WMA_DECODE;
        break;

    case CODEC_ID_AMR_NB:
        nAudioEngine = ITE_AMR_CODEC;

    default:
        break;
    }
    iteAudioOpenEngine(nAudioEngine);
}

/* get the current audio clock value */
static double get_audio_clock(PlayerInstance *is)
{
    #if 0
    if (is->paused)
    {
        return is->audio_current_pts;
    }
    else
    {
        return is->audio_current_pts_drift + av_gettime() / 1000000.0;
    }
    #else
    return 0;
    #endif
}

/* get the current video clock value */
static double get_video_clock(PlayerInstance *is)
{
    if (is->paused)
    {
        return is->video_current_pts;
    }
    else
    {
        return is->video_current_pts_drift + av_gettime() / 1000000.0;
    }
}

/* get the current external clock value */
static double get_external_clock(PlayerInstance *is)
{
    #if 0
    int64_t ti;
    ti = av_gettime();
    return is->external_clock + ((ti - is->external_clock_time) * 1e-6);
    #else
    return 0;
    #endif
}

/* get the current master clock value */
static double get_master_clock(PlayerInstance *is)
{
    double val;

    if (global_player_prop->av_sync_type == AV_SYNC_VIDEO_MASTER)
    {
        if (is->video_st)
            val = get_video_clock(is);
        else
            val = get_audio_clock(is);
    }
    else if (global_player_prop->av_sync_type == AV_SYNC_AUDIO_MASTER)
    {
        if (is->audio_st)
            val = get_audio_clock(is);
        else
            val = get_video_clock(is);
    }
    else
    {
        val = get_external_clock(is);
    }
    return val;
}
#endif

/* seek in the stream */
static void stream_seek(PlayerInstance *is, int64_t pos, int64_t rel, int seek_by_bytes)
{
    if (!is->seek_req)
    {
        is->seek_pos    = pos;
        is->seek_rel    = rel;
        is->seek_flags &= ~AVSEEK_FLAG_BYTE;
        if (seek_by_bytes)
            is->seek_flags |= AVSEEK_FLAG_BYTE;
        is->seek_req    = 1;
    }
}

static int video_thread(void *arg)
{
    int            ret;
    PlayerInstance *is = (PlayerInstance *) arg;
    //AVPacket pkt1, *pkt = &pkt1;
    double         pts;
    for (;;)
    {
        AVPacket pkt;
        while (is->paused && !is->videoq.abort_request)
        {
            usleep(10000);
        }
        if (is->videoq.abort_request)
        {
            goto the_end;
        }
        ret = packet_queue_get(&is->videoq, &pkt, 0);
        if (ret < 0)
        {
            goto the_end;
        }
        else if (ret > 0)
        {
            int     got_picture = 0, rev = -1;
            AVFrame *frame      = NULL;

            frame = avcodec_alloc_frame();
            //avcodec_get_frame_defaults(frame);
            //frame->opaque        = malloc(sizeof(uint32_t));

            global_video_pkt_pts = pkt.pts;
            rev                  = avcodec_decode_video2(is->videoctx, frame, &got_picture, &pkt);
            if (rev < 0)
                av_log(NULL, AV_LOG_ERROR, "video decode error\n");

            if (pkt.dts == AV_NOPTS_VALUE && frame->opaque1 && *(int64_t *)frame->opaque1 != AV_NOPTS_VALUE)
            {
                pts = *(int64_t *)frame->opaque1;
            }
            else if (pkt.dts != AV_NOPTS_VALUE)
            {
                pts = pkt.dts;
            }
            else
            {
                pts = 0;
            }
            pts *= av_q2d(is->video_st->time_base);

            av_free_packet(&pkt);

            if (got_picture)
            {
                //pts = synchronize_video(is, frame, pts);
                //printf("YC: pts = %f\n", pts);
                video_refresh_timer(is, pts);
                render_frame(is->videoctx, frame);
            }

            //free(frame->opaque);
            av_free(frame);
        }
        else
        {
            usleep(10000);
        }    
    }

the_end:
    //packet_queue_end(&is->videoq);
    if (is->videoctx)
    {
        avcodec_flush_buffers(is->videoctx);
    }
    pthread_exit(NULL);
    return 0;
}

static int audio_thread(void *arg)
{
    int            ret;
    PlayerInstance *is = (PlayerInstance *) arg;

    for (;;)
    {
        AVPacket pkt;
        while (is->paused && !is->audioq.abort_request)
        {
            usleep(10000);
        }
        if (is->audioq.abort_request)
        {
            goto the_end;
        }
        ret = packet_queue_get(&is->audioq, &pkt, 0);
        if (ret < 0)
        {
            goto the_end;
        }
        else if (ret > 0)
        {
            int     got_picture = 0, rev = -1;
            AVFrame *frame      = NULL;
#ifdef AV_SYNC_STC
            double  ref_clock, diff, audio_pts, audio_threshold;
            int64_t stc_time;
            if (!is->get_first_I_frame && !is->first_pkt_is_audio)
            {
                is->audio_is_sync = true;
                audio_pts         = (double)pkt.pts * av_q2d(is->audio_st->time_base);
    #ifdef CFG_BUILD_ITV
                fc_sync_settime(&is->stc, audio_pts * 90000);
    #endif
                is->first_pkt_is_audio = true;
            }
            else if (!is->audio_is_sync)
            {
    #if 0
                if (!is->get_first_I_frame)
                {
                    av_free_packet(&pkt);
                    continue;
                }
    #endif

    #ifdef CFG_BUILD_ITV
                fc_sync_gettime(&is->stc, &stc_time);
    #endif
                ref_clock = (double)stc_time / 90000;
                audio_pts = (double)pkt.pts * av_q2d(is->audio_st->time_base);
                diff      = audio_pts - ref_clock;
                if (diff < 0)
                {
                    av_free_packet(&pkt);
                    continue;
                }
                else if (diff > 0)
                {
                    usleep(diff * 1000000);
                    is->audio_is_sync = true;
                }
                else
                    is->audio_is_sync = true;
            }
#endif

#if !defined(CFG_DOORBELL_INDOOR) && !defined(CFG_DOORBELL_ADMIN)
#ifdef CFG_BUILD_ITV
            fc_sync_gettime(&is->stc, &stc_time);
#endif      
            ref_clock = (double)stc_time / 90000;
            audio_pts = (double)pkt.pts * av_q2d(is->audio_st->time_base);
            diff      = audio_pts - ref_clock;
            //printf("ref_clock = %f, audio_pts = %f, diff = %f\n", ref_clock, audio_pts, diff);
            if(diff > 0)
            {
                usleep(diff * 1000000);
            }
#endif            
            frame         = avcodec_alloc_frame();
            //avcodec_get_frame_defaults(frame);
            //frame->opaque = malloc(sizeof(uint32_t));

            rev           = avcodec_decode_audio4(is->audioctx, frame, &got_picture, &pkt);
            if (rev < 0)
                av_log(NULL, AV_LOG_ERROR, "audio decode error\n");

            av_free_packet(&pkt);
            //free(frame->opaque);
            av_free(frame);
        }
        else
        {
            usleep(10000);
        }    
    }

the_end:
    //packet_queue_end(&is->audioq);
    pthread_exit(NULL);
    return 0;
}

static int video_thread_for_videoloop(void *arg)
{
    int            ret;
    PlayerInstance *is = (PlayerInstance *) arg;
    //AVPacket pkt1, *pkt = &pkt1;
    double         pts;
    for (;;)
    {
        AVPacket pkt;
        while (is->paused && !is->videoq.abort_request)
        {
            usleep(10000);
        }
        if (is->videoq.abort_request)
        {
            goto the_end;
        }
        ret = packet_queue_get(&is->videoq, &pkt, 0);
        if (ret < 0)
        {
            goto the_end;
        }
        else if (ret > 0)
        {
            int     got_picture = 0, rev = -1;
            AVFrame *frame      = NULL;

            frame = avcodec_alloc_frame();
            //avcodec_get_frame_defaults(frame);
            //frame->opaque        = malloc(sizeof(uint32_t));
            global_video_pkt_pts = pkt.pts;
            rev                  = avcodec_decode_video2(is->videoctx, frame, &got_picture, &pkt);
            if (rev < 0)
                av_log(NULL, AV_LOG_ERROR, "video decode error\n");

            if (pkt.dts == AV_NOPTS_VALUE && frame->opaque1 && *(int64_t *)frame->opaque1 != AV_NOPTS_VALUE)
            {
                pts = *(int64_t *)frame->opaque1;
            }
            else if (pkt.dts != AV_NOPTS_VALUE)
            {
                pts = pkt.dts;
            }
            else
            {
                pts = 0;
            }
            pts *= av_q2d(is->video_st->time_base);

            av_free_packet(&pkt);

            if (got_picture)
            {
                if(is->video_speed > 1.2 && is->video_speed <= 1.5)
                {
                    static int count = 0;
                    count++;
                    if(count%4 == 0)
                    {
                        count = 0;
                        av_free(frame);
                        continue;
                    }
                }
                else if(is->video_speed > 1.5)
                {
                    static int count = 0;
                    count++;
                    if(count%2 == 0)
                    {
                        count = 0;
                        av_free(frame);
                        continue;
                    }
                }
                video_refresh_timer_for_videoloop(is, pts);
                render_frame(is->videoctx, frame);
            }

            //free(frame->opaque);
            av_free(frame);
        }
        else
        {
            usleep(10000);
        }    
    }

the_end:
    //packet_queue_end(&is->videoq);
    if (is->videoctx)
    {
        avcodec_flush_buffers(is->videoctx);
    }
    pthread_exit(NULL);
    return 0;
}


static int stream_component_open_for_videoloop(PlayerInstance *is, int stream_index, void *opaque)
{
    AVFormatContext *ic = is->ic;
    AVCodecContext  *avctx;
    AVCodec         *codec;

    if (stream_index < 0 || stream_index >= ic->nb_streams)
        return -1;
    avctx         = ic->streams[stream_index]->codec;
    avctx->opaque = opaque;

    codec         = avcodec_find_decoder(avctx->codec_id);

    if (!codec)
        return -1;

    if (!codec || avcodec_open2(avctx, codec, NULL) < 0)
        return -1;

    ic->streams[stream_index]->discard = AVDISCARD_DEFAULT;
    switch (avctx->codec_type)
    {
    case AVMEDIA_TYPE_AUDIO:
        is->audio_stream = stream_index;
        is->audio_st     = ic->streams[stream_index];
        is->audioctx     = avctx;
#if !defined(WIN32) && defined(CFG_AUDIO_ENABLE)
        i2s_set_direct_volperc(is->vol_level);
#endif
        packet_queue_init(&is->audioq);
        pthread_create(&is->audio_tid, NULL, audio_thread, (void *)is);
        break;

    case AVMEDIA_TYPE_VIDEO:
        if (avctx->width > 1280 || avctx->height > 720)
        {
            av_log(NULL, AV_LOG_ERROR, "%d x %d, the resolution is too large\n", avctx->width, avctx->height);
            return -1;
        }
        is->video_stream = stream_index;
        is->video_st     = ic->streams[stream_index];
        is->avctx        = avctx;
        is->videoctx     = avctx;

        packet_queue_init(&is->videoq);
        pthread_create(&is->video_tid, NULL, video_thread_for_videoloop, (void *)is);
        break;

    case AVMEDIA_TYPE_SUBTITLE:
        is->subtitle_stream = stream_index;
        is->subtitle_st     = ic->streams[stream_index];
        break;

    default:
        break;
    }
    return 0;
}


static int stream_component_open(PlayerInstance *is, int stream_index, void *opaque)
{
    AVFormatContext *ic = is->ic;
    AVCodecContext  *avctx;
    AVCodec         *codec;

    if (stream_index < 0 || stream_index >= ic->nb_streams)
        return -1;
    avctx         = ic->streams[stream_index]->codec;
    avctx->opaque = opaque;

    codec         = avcodec_find_decoder(avctx->codec_id);

    if (!codec)
        return -1;

    if (!codec || avcodec_open2(avctx, codec, NULL) < 0)
        return -1;

    ic->streams[stream_index]->discard = AVDISCARD_DEFAULT;
    switch (avctx->codec_type)
    {
    case AVMEDIA_TYPE_AUDIO:
        is->audio_stream = stream_index;
        is->audio_st     = ic->streams[stream_index];
        is->audioctx     = avctx;
#if !defined(WIN32) && defined(CFG_AUDIO_ENABLE)
        i2s_set_direct_volperc(is->vol_level);
#endif
        packet_queue_init(&is->audioq);
        pthread_create(&is->audio_tid, NULL, audio_thread, (void *)is);
        break;

    case AVMEDIA_TYPE_VIDEO:
#ifndef CFG_ENABLE_ROTATE
        if (avctx->width > 1280 || avctx->height > 720)
        {
            av_log(NULL, AV_LOG_ERROR, "%d x %d, the resolution is too large\n", avctx->width, avctx->height);
            return -1;
        }
#else
        if (avctx->width > 720 || avctx->height > 1280)
        {
            av_log(NULL, AV_LOG_ERROR, "%d x %d, the resolution is too large\n", avctx->width, avctx->height);
            return -1;
        }
#endif        
        is->video_stream = stream_index;
        is->video_st     = ic->streams[stream_index];
        is->avctx        = avctx;
        is->videoctx     = avctx;

        packet_queue_init(&is->videoq);
        pthread_create(&is->video_tid, NULL, video_thread, (void *)is);
        break;

    case AVMEDIA_TYPE_SUBTITLE:
        is->subtitle_stream = stream_index;
        is->subtitle_st     = ic->streams[stream_index];
        break;

    default:
        break;
    }
    return 0;
}

static void stream_component_close(PlayerInstance *is, int stream_index)
{
    AVFormatContext *ic = is->ic;
    AVCodecContext  *avctx;

    if (stream_index < 0 || stream_index >= ic->nb_streams)
        return;
    avctx = ic->streams[stream_index]->codec;

    switch (avctx->codec_type)
    {
    case AVMEDIA_TYPE_AUDIO:
        /* close audio thread */
#ifndef WIN32
        if (is->audio_tid)
#endif
        {
            packet_queue_abort(&is->audioq);
            pthread_join(is->audio_tid, NULL);
            packet_queue_end(&is->audioq);
#ifndef WIN32
            is->audio_tid = 0;
#endif
        }
        break;

    case AVMEDIA_TYPE_VIDEO:
        /* close video thread */
#ifndef WIN32
        if (is->video_tid)
#endif
        {
            packet_queue_abort(&is->videoq);
            pthread_join(is->video_tid, NULL);
            packet_queue_end(&is->videoq);
#ifndef WIN32
            is->video_tid = 0;
#endif
        }
        break;
    }

    ic->streams[stream_index]->discard = AVDISCARD_ALL;
    avcodec_close(avctx);
    switch (avctx->codec_type)
    {
    case AVMEDIA_TYPE_AUDIO:
        is->audio_st     = NULL;
        is->audio_stream = -1;
        break;

    case AVMEDIA_TYPE_VIDEO:
        is->video_st     = NULL;
        is->video_stream = -1;
        break;

    case AVMEDIA_TYPE_SUBTITLE:
        is->subtitle_st     = NULL;
        is->subtitle_stream = -1;
        break;

    default:
        break;
    }
}


void *read_thread_for_videoloop(void *arg)
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

    err                             = avformat_open_input(&ic, is->filename, NULL, NULL);
    is->ic                          = ic;

    if (err < 0)
    {
        char rets[256] = {0};
        av_strerror(err, rets, sizeof(rets));
        av_log(NULL, AV_LOG_ERROR, "Open input error \"%s\" : %s\n", is->filename, rets);
        send_event(PLAYER_EVENT_OPEN_FILE_FAIL, NULL);
        goto fail;
    }

    if (ic->duration != AV_NOPTS_VALUE)
        is->total_duration = ic->duration;

    if (pprop->genpts)
        ic->flags |= AVFMT_FLAG_GENPTS;

    if (ic->pb)
        ic->pb->eof_reached = 0;

    if (pprop->seek_by_bytes < 0)
        pprop->seek_by_bytes = !!(ic->iformat->flags & AVFMT_TS_DISCONT);

    /* if seeking requested, execute it */
    if (pprop->start_time != AV_NOPTS_VALUE)
    {
        int64_t timestamp;

        av_log(NULL, AV_LOG_ERROR, "seek request before start\n");
        timestamp = pprop->start_time;
        /* add the stream start time */
        if (ic->start_time != AV_NOPTS_VALUE)
            timestamp += ic->start_time;
        ret = avformat_seek_file(ic, -1, INT64_MIN, timestamp, INT64_MAX, 0);
        if (ret < 0)
            av_log(NULL, AV_LOG_ERROR, "%s: could not seek to position %0.3f\n", is->filename, (double)timestamp / AV_TIME_BASE);
    }

    /* set for avformat_seek_file */
    for (i = 0; i < ic->nb_streams; i++)
        ic->streams[i]->discard = AVDISCARD_ALL;

    /* find stream id */
    if (!is->video_disable)
        st_index[AVMEDIA_TYPE_VIDEO] = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

    if (!is->audio_disable)
        st_index[AVMEDIA_TYPE_AUDIO] = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, st_index[AVMEDIA_TYPE_VIDEO], NULL, 0);

    if (!is->video_disable)
        st_index[AVMEDIA_TYPE_SUBTITLE] = av_find_best_stream(ic, AVMEDIA_TYPE_SUBTITLE, -1, st_index[AVMEDIA_TYPE_VIDEO], NULL, 0);

    if (pprop->show_status)
        av_dump_format(ic, 0, is->filename, 0);

    /* open the streams */
    if (st_index[AVMEDIA_TYPE_AUDIO] >= 0)
        stream_component_open(is, st_index[AVMEDIA_TYPE_AUDIO], NULL);

    ret = -1;
    if (st_index[AVMEDIA_TYPE_VIDEO] >= 0)
        ret = stream_component_open_for_videoloop(is, st_index[AVMEDIA_TYPE_VIDEO], NULL);

    if (st_index[AVMEDIA_TYPE_SUBTITLE] >= 0)
        stream_component_open(is, st_index[AVMEDIA_TYPE_SUBTITLE], NULL);

#ifdef AV_SYNC_STC
    if (is->video_stream < 0 && is->audio_stream < 0)
#else
    if (is->video_stream < 0 || is->audio_stream < 0)
#endif
    {
        av_log(NULL, AV_LOG_ERROR, "%s: could not open codecs\n", is->filename);
        packet_queue_end(&is->videoq);
        packet_queue_end(&is->audioq);
        send_event(PLAYER_EVENT_UNSUPPORT_FILE, NULL);
        ret = -1;
        goto fail;
    }

    //is->exclock = PalGetClock();

    /* resync TsSource from TsExtractor */
    //ithMediaPlayer_drop_all_input_streams();

    for (;;)
    {
        PlayerInstance  *is = pprop->inst;
        AVFormatContext *ic = is->ic;
        if (is->abort_request)
        {
            break;
        }

        //if (eof) continue;

        if (is->paused)
        {
#ifdef CFG_BUILD_ITV
            fc_sync_pause(&is->stc);
#endif
            sem_wait(&sem_pause);
        }

        if (is->videoq.size + is->audioq.size > MAX_QUEUE_SIZE
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
                    //packet_queue_put(&is->audioq, &flush_pkt);
                    //avcodec_flush_buffers(is->audio_st->codec);
                    //flush_audio_hw(is);
                }
                if (is->video_stream >= 0)
                {
                    //packet_queue_flush(&is->videoq);
                    //packet_queue_put(&is->videoq, &flush_pkt);
                    //avcodec_flush_buffers(is->videoctx);
                }
                if (is->subtitle_stream >= 0)
                {
                    avcodec_flush_buffers(is->subtitle_st->codec);
                }
            }
            is->seek_req = 0;
        }

        ret = av_read_frame(ic, pkt);
        if (ret < 0)
        {
            if (ret == AVERROR_EOF || url_feof(ic->pb))
            {                   
                send_event(PLAYER_EVENT_EOF, NULL);
                eof = 1;
                break;
            }
            if (ic->pb && ic->pb->error)
                break;

            continue;
        }

        if (!is->get_first_pkt)
        {
#ifdef CFG_BUILD_ITV
            fc_init_sync(&is->stc);
            //fc_sync_settime(&pprop->stc, -54000);
#endif
            is->get_first_pkt = true;
        }

#if 0
        if (pkt->pts != AV_NOPTS_VALUE)
            av_log(NULL, AV_LOG_DEBUG, "Frame %d@%d: pts=%lld, dts=%lld, size=%d, data=%x\n", instance, pkt->stream_index, pkt->pts, pkt->dts, pkt->size, pkt->data);
#endif

#if TEST_BITRATE_WITHOUT_DECODE /* input verify */
        {
            av_free_packet(pkt);
            continue;
        }
#endif

        /* check if packet is in play range specified by usr, then q, otherwise discard */
        pkt_in_play_range = duration == AV_NOPTS_VALUE ||
                            (pkt->pts - ic->streams[pkt->stream_index]->start_time) *
                            av_q2d(ic->streams[pkt->stream_index]->time_base) -
                            (double)(pprop->start_time != AV_NOPTS_VALUE ? pprop->start_time : 0) / 1000000
                            <= ((double)duration / 1000000);
        if (pkt->stream_index == is->audio_stream && pkt_in_play_range)
        {
            packet_queue_put(&is->audioq, pkt);
        }
        else if (pkt->stream_index == is->video_stream && pkt_in_play_range)
        {
            packet_queue_put(&is->videoq, pkt);
        }
        else if (pkt->stream_index == is->subtitle_stream && pkt_in_play_range)
        {
            av_log(NULL, AV_LOG_INFO, "pass subtitle packet\n");
            av_free_packet(pkt);
            continue;
        }
        else
        {
            av_free_packet(pkt);
            continue;
        }

        usleep(1000);
    }
    /* wait until the end */
    while (!pprop->inst->abort_request)
    {
        if(is->seek_req) goto seekto;
        usleep(1000);
    }
    ret = 0;
fail:
    //printf("terminate video\n");
    //printf("terminate audio\n");
    /* close each stream */
    if (is->audio_stream >= 0)
        stream_component_close(is, is->audio_stream);
    if (is->video_stream >= 0)
        stream_component_close(is, is->video_stream);
    if (is->subtitle_stream >= 0)
        stream_component_close(is, is->subtitle_stream);
    if (is->ic)
    {
        avformat_close_input(&is->ic);
        is->ic = NULL; /* safety */
    }
    avio_set_interrupt_cb(NULL);

    if (ret != 0)
        av_log(NULL, AV_LOG_ERROR, "read_thread failed, ret=%d\n", ret);
    else
        av_log(NULL, AV_LOG_ERROR, "read thread %x done\n", pprop->read_tid);

    pthread_exit(NULL);
    return 0;
}

void *read_thread(void *arg)
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

    err                             = avformat_open_input(&ic, is->filename, NULL, NULL);
    is->ic                          = ic;

    if (err < 0)
    {
        char rets[256] = {0};
        av_strerror(err, rets, sizeof(rets));
        av_log(NULL, AV_LOG_ERROR, "Open input error \"%s\" : %s\n", is->filename, rets);
        send_event(PLAYER_EVENT_OPEN_FILE_FAIL, NULL);
        goto fail;
    }

    if (ic->duration != AV_NOPTS_VALUE)
        is->total_duration = ic->duration;

    if (pprop->genpts)
        ic->flags |= AVFMT_FLAG_GENPTS;

    if (ic->pb)
        ic->pb->eof_reached = 0;

    if (pprop->seek_by_bytes < 0)
        pprop->seek_by_bytes = !!(ic->iformat->flags & AVFMT_TS_DISCONT);

    /* if seeking requested, execute it */
    if (pprop->start_time != AV_NOPTS_VALUE)
    {
        int64_t timestamp;

        av_log(NULL, AV_LOG_ERROR, "seek request before start\n");
        timestamp = pprop->start_time;
        /* add the stream start time */
        if (ic->start_time != AV_NOPTS_VALUE)
            timestamp += ic->start_time;
        ret = avformat_seek_file(ic, -1, INT64_MIN, timestamp, INT64_MAX, 0);
        if (ret < 0)
            av_log(NULL, AV_LOG_ERROR, "%s: could not seek to position %0.3f\n", is->filename, (double)timestamp / AV_TIME_BASE);
    }

    /* set for avformat_seek_file */
    for (i = 0; i < ic->nb_streams; i++)
        ic->streams[i]->discard = AVDISCARD_ALL;

    /* find stream id */
    if (!is->video_disable)
        st_index[AVMEDIA_TYPE_VIDEO] = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

    if (!is->audio_disable)
        st_index[AVMEDIA_TYPE_AUDIO] = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, st_index[AVMEDIA_TYPE_VIDEO], NULL, 0);

    if (!is->video_disable)
        st_index[AVMEDIA_TYPE_SUBTITLE] = av_find_best_stream(ic, AVMEDIA_TYPE_SUBTITLE, -1, st_index[AVMEDIA_TYPE_VIDEO], NULL, 0);

    if (pprop->show_status)
        av_dump_format(ic, 0, is->filename, 0);

    /* open the streams */
    if (st_index[AVMEDIA_TYPE_AUDIO] >= 0)
        stream_component_open(is, st_index[AVMEDIA_TYPE_AUDIO], NULL);

    ret = -1;
    if (st_index[AVMEDIA_TYPE_VIDEO] >= 0)
        ret = stream_component_open(is, st_index[AVMEDIA_TYPE_VIDEO], NULL);

    if (st_index[AVMEDIA_TYPE_SUBTITLE] >= 0)
        stream_component_open(is, st_index[AVMEDIA_TYPE_SUBTITLE], NULL);

#ifdef AV_SYNC_STC
    if (is->video_stream < 0 && is->audio_stream < 0)
#else
    if (is->video_stream < 0 || is->audio_stream < 0)
#endif
    {
        av_log(NULL, AV_LOG_ERROR, "%s: could not open codecs\n", is->filename);
        packet_queue_end(&is->videoq);
        packet_queue_end(&is->audioq);
        send_event(PLAYER_EVENT_UNSUPPORT_FILE, NULL);
        ret = -1;
        goto fail;
    }

    //is->exclock = PalGetClock();

    /* resync TsSource from TsExtractor */
    //ithMediaPlayer_drop_all_input_streams();

    for (;;)
    {
        PlayerInstance  *is = pprop->inst;
        AVFormatContext *ic = is->ic;
        if (is->abort_request)
        {
            break;
        }

        //if (eof) continue;

        if (is->paused)
        {
#ifdef CFG_BUILD_ITV
            fc_sync_pause(&is->stc);
#endif
            sem_wait(&sem_pause);
        }

        if (is->videoq.size + is->audioq.size > MAX_QUEUE_SIZE
            || (is->videoq.nb_packets > MIN_FRAMES
                || is->audioq.nb_packets > MIN_FRAMES))
        {
            if (is->seek_req) goto seekto;
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
                    //packet_queue_put(&is->audioq, &flush_pkt);
                    //avcodec_flush_buffers(is->audio_st->codec);
                    //flush_audio_hw(is);
                }
                if (is->video_stream >= 0)
                {
                    packet_queue_flush(&is->videoq);
                    //packet_queue_put(&is->videoq, &flush_pkt);
                    //avcodec_flush_buffers(is->videoctx);
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
#if 0
        if (eof)
        {
            is->frame_timer      = 0;
            is->frame_last_delay = 40e-3;
            is->frame_last_pts   = 0;
            fc_deinit_sync(&pprop->stc);
            unsigned long dur                = PalGetDuration(is->exclock);
            float         AvgFramesPerSecond = is->frame_count / (dur / 1000.0);
            av_log(NULL, AV_LOG_DEBUG, "frame = %d, duration = %u ms, AvgFramesPerSecond = %0.2f\n", is->frame_count, dur, AvgFramesPerSecond);

            if (is->video_stream >= 0)
            {
                av_init_packet(pkt);
                pkt->data         = NULL;
                pkt->size         = 0;
                pkt->stream_index = is->video_stream;
            }
            if (is->audio_stream >= 0 &&
                is->audio_st->codec->codec->capabilities & CODEC_CAP_DELAY)
            {
                av_init_packet(pkt);
                pkt->data         = NULL;
                pkt->size         = 0;
                pkt->stream_index = is->audio_stream;
                //packet_queue_put(&is->audioq, pkt);
            }

            if (is->loop != 1 && (!is->loop || --is->loop))
            {
                stream_seek(is, pprop->start_time != AV_NOPTS_VALUE ? pprop->start_time : 0, 0, 0);
            }
            else if (is->autoexit)
            {
                ret = AVERROR_EOF;
                goto fail;
            }

            eof = 0;
            continue;
        }
#endif

        ret = av_read_frame(ic, pkt);
        if (ret < 0)
        {
            if (ret == AVERROR_EOF || url_feof(ic->pb))
            {
                while (is->videoq.nb_packets > 0 || is->audioq.nb_packets > 0)
                {
                    if (is->seek_req) goto seekto;
                    usleep(1000);
                }    
                send_event(PLAYER_EVENT_EOF, NULL);
                eof = 1;
                break;
            }
            if (ic->pb && ic->pb->error)
                break;

            continue;
        }

        if (!is->get_first_pkt)
        {
#ifdef CFG_BUILD_ITV
            fc_init_sync(&is->stc);
            //fc_sync_settime(&pprop->stc, -54000);
#endif
            is->get_first_pkt = true;
        }

#if 0
        if (pkt->pts != AV_NOPTS_VALUE)
            av_log(NULL, AV_LOG_DEBUG, "Frame %d@%d: pts=%lld, dts=%lld, size=%d, data=%x\n", instance, pkt->stream_index, pkt->pts, pkt->dts, pkt->size, pkt->data);
#endif

#if TEST_BITRATE_WITHOUT_DECODE /* input verify */
        {
            av_free_packet(pkt);
            continue;
        }
#endif

        /* check if packet is in play range specified by usr, then q, otherwise discard */
        pkt_in_play_range = duration == AV_NOPTS_VALUE ||
                            (pkt->pts - ic->streams[pkt->stream_index]->start_time) *
                            av_q2d(ic->streams[pkt->stream_index]->time_base) -
                            (double)(pprop->start_time != AV_NOPTS_VALUE ? pprop->start_time : 0) / 1000000
                            <= ((double)duration / 1000000);
        if (pkt->stream_index == is->audio_stream && pkt_in_play_range)
        {
            packet_queue_put(&is->audioq, pkt);
        }
        else if (pkt->stream_index == is->video_stream && pkt_in_play_range)
        {
            packet_queue_put(&is->videoq, pkt);
        }
        else if (pkt->stream_index == is->subtitle_stream && pkt_in_play_range)
        {
            av_log(NULL, AV_LOG_INFO, "pass subtitle packet\n");
            av_free_packet(pkt);
            continue;
        }
        else
        {
            av_free_packet(pkt);
            continue;
        }

        usleep(1000);
    }
    /* wait until the end */
    while (!pprop->inst->abort_request)
    {
        if(is->seek_req) goto seekto;
        usleep(1000);
    }
    ret = 0;
fail:
    //printf("terminate video\n");
    //printf("terminate audio\n");
    /* close each stream */
    if (is->audio_stream >= 0)
        stream_component_close(is, is->audio_stream);
    if (is->video_stream >= 0)
        stream_component_close(is, is->video_stream);
    if (is->subtitle_stream >= 0)
        stream_component_close(is, is->subtitle_stream);
    if (is->ic)
    {
        avformat_close_input(&is->ic);
        is->ic = NULL; /* safety */
    }
    avio_set_interrupt_cb(NULL);

    if (ret != 0)
        av_log(NULL, AV_LOG_ERROR, "read_thread failed, ret=%d\n", ret);
    else
        av_log(NULL, AV_LOG_ERROR, "read thread %x done\n", pprop->read_tid);

    pthread_exit(NULL);
    return 0;
}

static void stream_close(PlayerProps *pprop)
{
    void           *status;
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
    //av_free(is); // implement stop, don't free PlayerProps here
}

///////////////////////////////////////////////////////////////////////////////////////////
// Public Functions
//
///////////////////////////////////////////////////////////////////////////////////////////
static int video_thread_for_rtsp_client(void *arg)
{
    int            ret;
    PlayerProps    *pProp = (PlayerProps *) arg;
    PlayerInstance *is    = pProp->inst;
    double         pts;

    for (;;)
    {
        AVPacket pkt;
        if (is->videoq.abort_request)
        {
            goto the_end;
        }
        ret = packet_queue_get(&is->videoq, &pkt, 0);
        if (ret < 0)
        {
            goto the_end;
        }
        else if (ret > 0)
        {
            int     got_picture = 0, rev = -1;
            AVFrame *frame      = NULL;

            pts           = pkt.timestamp;
            frame         = avcodec_alloc_frame();
            frame->opaque = malloc(sizeof(uint32_t));
            rev           = avcodec_decode_video2(is->videoctx, frame, &got_picture, &pkt);
            if (rev < 0)
                av_log(NULL, AV_LOG_ERROR, "video decode error\n");

            av_free_packet(&pkt);
            if (got_picture)
            {
                //printf("YC: pts = %f\n", pts);
                video_refresh_timer_for_rtsp_client(is, pts);
                render_frame(is->videoctx, frame);
            }

            free(frame->opaque);
            av_free(frame);
        }
        else
            usleep(5000);
    }

the_end:
    //packet_queue_end(&is->videoq);
    if (is->videoctx)
    {
        avcodec_flush_buffers(is->videoctx);
    }
    pthread_exit(NULL);
    return 0;
}

static int audio_thread_for_rtsp_client(void *arg)
{
    int            ret;
    PlayerProps    *pProp = (PlayerProps *) arg;
    PlayerInstance *is    = pProp->inst;

    for (;;)
    {
        AVPacket pkt;
        if (is->audioq.abort_request)
        {
            goto the_end;
        }
        ret = packet_queue_get(&is->audioq, &pkt, 0);
        if (ret < 0)
        {
            goto the_end;
        }
        else if (ret > 0)
        {
            int     got_picture = 0, rev = -1;
            AVFrame *frame      = NULL;
#ifdef AV_SYNC_STC
            double  ref_clock, diff, audio_pts, audio_threshold;
            int64_t stc_time;
            //printf("YC: audio timestamp = %f\n", pkt.timestamp);
            if (!is->get_first_I_frame && !is->first_pkt_is_audio)
            {
                //printf("YC: %s, %d\n", __FUNCTION__, __LINE__);
                is->audio_is_sync = true;
                audio_pts         = (double)pkt.timestamp;
    #ifdef CFG_BUILD_ITV
                fc_sync_settime(&is->stc, audio_pts * 90000);
    #endif
                is->first_pkt_is_audio = true;
            }
            else if (!is->audio_is_sync)
            {
                //printf("YC: %s, %d\n", __FUNCTION__, __LINE__);
                audio_pts       = (double)pkt.timestamp;
                audio_threshold = cal_audio_threshold(is);
                if (audio_pts > audio_threshold)
                {
    #ifdef CFG_BUILD_ITV
                    fc_sync_settime(&is->stc, (audio_pts - audio_threshold) * 90000);
    #endif
                    is->audio_is_sync = true;
                }

    #if 0
                if (!is->get_first_I_frame)
                {
                    av_free_packet(&pkt);
                    continue;
                }
    #endif
    #if 0
        #ifdef CFG_BUILD_ITV
                fc_sync_gettime(&pProp->stc, &stc_time);
        #endif
                ref_clock       = (double)stc_time / 90000;
                audio_pts       = (double)pkt.timestamp;
                diff            = audio_pts - ref_clock;
                audio_threshold = cal_audio_threshold(is);
                printf("YC: %s, %d, diff = %f\n", __FUNCTION__, __LINE__, diff);
                if (diff < 0)
                {
                    if (audio_pts > audio_threshold)
                    {
        #ifdef CFG_BUILD_ITV
                        fc_sync_settime(&pProp->stc, (audio_pts - audio_threshold) * 90000);
        #endif
                        is->audio_is_sync = true;
                    }
                    else
                    {
                        av_free_packet(&pkt);
                        continue;
                    }
                }
                else if (diff > 0)
                {
                    usleep(diff * 1000000);
                    is->audio_is_sync = true;
                }
                else
                    is->audio_is_sync = true;
    #endif
            }
#endif

            frame         = avcodec_alloc_frame();
            frame->opaque = malloc(sizeof(uint32_t));
            rev           = avcodec_decode_audio4(is->audioctx, frame, &got_picture, &pkt);
            if (rev < 0)
                av_log(NULL, AV_LOG_ERROR, "audio decode error\n");

            av_free_packet(&pkt);
            free(frame->opaque);
            av_free(frame);
        }
        else
            usleep(5000);
    }

the_end:
    //packet_queue_end(&is->audioq);
    pthread_exit(NULL);
    return 0;
}

void ithMediaPlayer_InitAVDecodeEnv()
{
    PlayerProps    *pprop = NULL;
    PlayerInstance *inst  = NULL;

    //pthread_mutex_init(&player_mutex, NULL);
    pthread_mutex_lock(&player_mutex);

    if (global_player_prop)
    {
        pthread_mutex_unlock(&player_mutex);
        return 1;
    }

    av_register_all();

    pprop = (PlayerProps *) calloc(sizeof(char), sizeof(PlayerProps));
    inst  = (PlayerInstance *) calloc(1, sizeof(PlayerInstance));

    if (pprop->inst)
        free(pprop->inst);
    pprop->inst        = inst;
    global_player_prop = pprop;

    packet_queue_init(&inst->videoq); //use packet queue
    packet_queue_init(&inst->audioq);
#ifdef AV_SYNC_STC
    pprop->inst->get_first_I_frame = false;
    pprop->inst->audio_is_sync     = false;
#endif
#ifdef CFG_BUILD_ITV
    fc_init_sync(&inst->stc);
#endif
    pthread_create(&inst->video_tid, NULL, video_thread_for_rtsp_client, (void *)pprop);
    pthread_create(&inst->audio_tid, NULL, audio_thread_for_rtsp_client, (void *)pprop);

    pthread_mutex_unlock(&player_mutex);
}

void ithMediaPlayer_InitH264DecodeEnv()
{
    PlayerProps    *pprop         = global_player_prop;
    PlayerInstance *is            = NULL;
    AVCodec        *videoCodec;
    AVCodecContext *video_context = NULL;

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

    if (!(videoCodec = avcodec_find_decoder(CODEC_ID_H264)))
    {
        printf("codec not found!");
        pthread_mutex_unlock(&player_mutex);
        return;
    }
    video_context             = avcodec_alloc_context();
    video_context->codec_id   = CODEC_ID_H264;
    video_context->codec_type = AVMEDIA_TYPE_VIDEO;

    if (avcodec_open(video_context, videoCodec) < 0)
    {
        fprintf(stderr, "could not open codec\n");
        pthread_mutex_unlock(&player_mutex);
        exit(1);
    }
    is->videoctx = video_context;

    pthread_mutex_unlock(&player_mutex);
}

void ithMediaPlayer_InitAudioDecodeEnv(int samplerate, int num_channels, RTSPCLIENT_AUDIO_CODEC codec_id)
{
    PlayerProps    *pprop         = global_player_prop;
    PlayerInstance *is            = NULL;
    AVCodec        *audioCodec;
    AVCodecContext *audio_context = NULL;

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

    audio_context = avcodec_alloc_context();
    switch (codec_id)
    {
    case RTSP_CODEC_MPA:
        audioCodec              = avcodec_find_decoder(CODEC_ID_MP3);
        audio_context->codec_id = CODEC_ID_MP3;
        break;

    case RTSP_CODEC_MPEG4_AAC:
        audioCodec              = avcodec_find_decoder(CODEC_ID_AAC);
        audio_context->codec_id = CODEC_ID_AAC;
        break;

    case RTSP_CODEC_PCMU:
        audioCodec              = avcodec_find_decoder(CODEC_ID_PCM_MULAW);
        audio_context->codec_id = CODEC_ID_PCM_MULAW;
        break;
    }
    audio_context->codec_type  = AVMEDIA_TYPE_AUDIO;
    audio_context->channels    = num_channels;
    audio_context->sample_rate = samplerate;

    if (avcodec_open(audio_context, audioCodec) < 0)
    {
        fprintf(stderr, "could not open codec\n");
        pthread_mutex_unlock(&player_mutex);
        exit(1);
    }
    is->audioctx = audio_context;
    pthread_mutex_unlock(&player_mutex);
}

int ithMediaPlayer_h264_decode_from_rtsp(unsigned char *inputbuf, int inputbuf_size, double timestamp)
{
    PlayerProps    *pprop = global_player_prop;
    PlayerInstance *is;
    AVPacket       packet;

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

    while (is->videoq.size > MAX_QUEUE_SIZE || is->videoq.nb_packets > 8)
    {
        /* wait 1 ms */
        usleep(1000);
    }

    av_init_packet(&packet);
    packet.data      = inputbuf;
    packet.size      = inputbuf_size;
    packet.timestamp = timestamp;
    packet.destruct  = av_destruct_packet;
    packet_queue_put(&is->videoq, &packet);

    pthread_mutex_unlock(&player_mutex);

    return 0;
}

int ithMediaPlayer_audio_decode_from_rtsp(unsigned char *inputbuf, int inputbuf_size, double timestamp)
{
    PlayerProps    *pprop = global_player_prop;
    PlayerInstance *is;
    AVPacket       packet;

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

    while (is->audioq.nb_packets > MIN_FRAMES)
    {
        /* wait 1 ms */
        usleep(1000);
    }

    av_init_packet(&packet);
    packet.data      = inputbuf;
    packet.size      = inputbuf_size;
    packet.timestamp = timestamp;
    packet.destruct  = av_destruct_packet;
    packet_queue_put(&is->audioq, &packet);

    pthread_mutex_unlock(&player_mutex);
    return 0;
}

void ithMediaPlayer_DeinitAVDecodeEnv()
{
    PlayerProps    *pprop = global_player_prop;
    PlayerInstance *is    = pprop->inst;

    pthread_mutex_lock(&player_mutex);

    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        pthread_mutex_unlock(&player_mutex);
        return -1;
    }

#ifndef WIN32
    if (is->video_tid)
#endif
    {
        packet_queue_abort(&is->videoq);
        pthread_join(is->video_tid, NULL);
        packet_queue_end(&is->videoq);
#ifndef WIN32
        is->video_tid = 0;
#endif
    }
#ifndef WIN32
    if (is->audio_tid)
#endif
    {
        packet_queue_abort(&is->audioq);
        pthread_join(is->audio_tid, NULL);
        packet_queue_end(&is->audioq);
#ifndef WIN32
        is->audio_tid = 0;
#endif
    }

    if (is->videoctx->opaque)
    {
        av_free(is->videoctx->opaque);
        is->videoctx->opaque = NULL;
    }
    if (is->videoctx)
    {
        avcodec_close(is->videoctx);
        av_free(is->videoctx);
    }
    if (is->audioctx)
    {
        avcodec_close(is->audioctx);
        av_free(is->audioctx);
    }

#ifdef CFG_BUILD_ITV
    fc_deinit_sync(&is->stc);
#endif

    global_player_prop = NULL;
    if (pprop)
    {
        if (pprop->inst)
            free(pprop->inst);
        free(pprop);
        pprop = NULL;
        printf("\n");
    }
    pthread_mutex_unlock(&player_mutex);
    //pthread_mutex_destroy(&player_mutex);
    return;
}

int ithMediaPlayer_init(cb_handler_t callback)
{
    PlayerProps *pprop = NULL;

    if (global_player_prop)
        return 1;

    /* register all codecs, demux and protocols */
    av_register_all();
    avformat_network_init();

#ifdef DEBUG
    av_log_set_level(AV_LOG_DEBUG);
#endif

    pprop = (PlayerProps *) calloc(sizeof(char), sizeof(PlayerProps));
    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Initialize player failed\n");
        return -1;
    }

    // player properties initial, TODO use API later
    //pthread_mutex_init(&player_mutex, NULL);
    pprop->callback      = callback;
    pprop->show_status   = 1;
    pprop->seek_by_bytes = -1;
    pprop->start_time    = AV_NOPTS_VALUE;

    global_player_prop   = pprop;
    sem_init(&sem_pause, 0, 0);
    return 0;
}

static int ithMediaPlayer_select_file(const char *filename, int level)
{
    PlayerProps *pprop = global_player_prop;

    pthread_mutex_lock(&player_mutex);

    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Null player properties %s\n", __func__);
        pthread_mutex_unlock(&player_mutex);
        return -1;
    }

    if (pprop->instCnt >= 1)
    {
        av_log(NULL, AV_LOG_ERROR, "Maximum stream reach\n");
        pthread_mutex_unlock(&player_mutex);
        return -1;
    }

    if (filename[0] != '\0')
    {
        PlayerInstance *inst = NULL;

        av_log(NULL, AV_LOG_INFO, "Select %s\n", filename);

        inst = (PlayerInstance *) calloc(1, sizeof(PlayerInstance));
        if (!inst)
            av_log(NULL, AV_LOG_ERROR, "not enough memory\n");

        av_strlcpy(inst->filename, filename, strlen(filename) + 1);
        inst->vol_level     = level;
#ifndef CFG_AUDIO_ENABLE
        inst->audio_disable = 1;
#else
        inst->audio_disable = 0;
#endif
        if (pprop->inst)
            free(pprop->inst);
        pprop->inst    = inst;
        pprop->instCnt = 1;
    }
    pthread_mutex_unlock(&player_mutex);
    return 0;
}

static int ithMediaPlayer_play(void)
{
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

    if (!pprop->isFilePlayer_thread_created)
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

        rc                     = pthread_create(&pprop->read_tid, NULL, read_thread, (void *)pprop);
        if (rc)
        {
            av_log(NULL, AV_LOG_ERROR, "create thread failed %d\n", rc);
            ithMediaPlayer_stop();
        }
    }
    else
    {
        if (is->paused)
        {
            is->paused = false;
            sem_post(&sem_pause);
#ifdef CFG_BUILD_ITV
            fc_sync_pause(&is->stc);
#endif
#ifdef CFG_AUDIO_ENABLE
            iteAudioPause(0);
#endif
        }
    }

    pprop->isFilePlayer_thread_created = true;
    pthread_mutex_unlock(&player_mutex);
    return 0;
}

/* pause or resume the video */
static int ithMediaPlayer_pause(void)
{
    PlayerProps    *pprop = global_player_prop;
    PlayerInstance *is    = NULL;

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
        is->paused       = false;
        sem_post(&sem_pause);
#ifdef CFG_BUILD_ITV
        fc_sync_pause(&is->stc);
#endif
#ifdef CFG_AUDIO_ENABLE
        iteAudioPause(0);
#endif
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

static int ithMediaPlayer_stop(void)
{
    PlayerProps    *pprop = global_player_prop;
    PlayerInstance *is    = NULL;

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
        is->paused = false;
        sem_post(&sem_pause);
#ifdef CFG_BUILD_ITV
        fc_sync_pause(&is->stc);
#endif
#ifdef CFG_AUDIO_ENABLE
        iteAudioPause(0);
#endif
    }
    stream_close(pprop);
    pprop->isFilePlayer_thread_created = false;

    /* default set RMI active index to 1 (single instance) */
#ifdef CFG_BUILD_ITV
    itv_flush_dbuf();
#endif

    pthread_mutex_unlock(&player_mutex);
    return 0;
}

static int ithMediaPlayer_play_videoloop(void)
{
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

    if (!pprop->isFilePlayer_thread_created)
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

        rc                     = pthread_create(&pprop->read_tid, NULL, read_thread_for_videoloop, (void *)pprop);
        if (rc)
        {
            av_log(NULL, AV_LOG_ERROR, "create thread failed %d\n", rc);
            ithMediaPlayer_stop();
        }
    }
    else
    {
        if (is->paused)
        {
            is->paused = false;
            sem_post(&sem_pause);
#ifdef CFG_BUILD_ITV
            fc_sync_pause(&is->stc);
#endif
#ifdef CFG_AUDIO_ENABLE
            iteAudioPause(0);
#endif
        }
    }

    pprop->isFilePlayer_thread_created = true;
    pthread_mutex_unlock(&player_mutex);
    return 0;
}

#if 0
static void do_seek(PlayerInstance *is, unsigned short key)
{
    double         incr, pos;
    PlayerInstance *cur_stream  = is;
    unsigned char  direction[6] = {0};

    switch (key)
    {
    case 0:
        incr = 60.0;
        sprintf(direction, "UP");
        break;

    case 1:
        incr = -60.0;
        sprintf(direction, "DOWN");
        break;

    case 2:
        incr = 10.0;
        sprintf(direction, "RIGHT");
        break;

    case 3:
        incr = -10.0;
        sprintf(direction, "LEFT");
        break;
    }

    if (global_player_prop->seek_by_bytes) //there is some problems when using seek by byte, not every demuxer support this.
    {
        int av = -1;
        if (cur_stream->video_stream >= 0 && cur_stream->video_current_pos >= 0)
        {
            pos = cur_stream->video_current_pos;
            av  = 1;
        }
        else if (cur_stream->audio_stream >= 0 && cur_stream->audio_pkt.pos >= 0)
        {
            pos = cur_stream->audio_pkt.pos;
            av  = 2;
        }
        else
        {
            pos = avio_tell(cur_stream->ic->pb);
            av  = 3;
        }

        if (cur_stream->ic->bit_rate)
            incr *= cur_stream->ic->bit_rate / 8.0;
        else
            incr *= 180000.0;

        pos += incr;

        {
            int64_t seek_target = pos + incr;
            int64_t seek_min    = incr > 0 ? seek_target - incr + 2 : INT64_MIN;
            int64_t seek_max    = incr < 0 ? seek_target - incr - 2 : INT64_MAX;
            av_log(NULL, AV_LOG_DEBUG, "seek(%s,%d) bitrate=%d incr=%0.2f pos=%0.2f target=%lld seek_min=%lld seek_max=%lld backrewind=%d\n",
                   direction, av, cur_stream->ic->bit_rate,
                   incr, pos, seek_target, seek_min, seek_max,
                   (seek_target - seek_min > (uint64_t)(seek_max - seek_target) ? AVSEEK_FLAG_BACKWARD : 0));
        }

        stream_seek(cur_stream, pos, incr, 1);
    }
    else
    {
        pos  = get_master_clock(cur_stream);
        pos += incr;
        stream_seek(cur_stream, (int64_t)(pos * AV_TIME_BASE), (int64_t)(incr * AV_TIME_BASE), 0);
    }
}
#endif

static int ithMediaPlayer_deinit()
{
    PlayerProps *pprop = global_player_prop;

    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        return -1;
    }
    //pthread_mutex_destroy(&player_mutex);

    sem_destroy(&sem_pause);
    global_player_prop = NULL;

    /* De-init network */
    avformat_network_deinit();

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
    //ithVideoBufRelease();
    return 0;
}

static int ithMediaPlayer_get_total_duration(int64_t *total_time)
{
    PlayerProps    *pprop = global_player_prop;
    PlayerInstance *is    = NULL;

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

    *total_time = is->total_duration;

    /*
        int64_t current_time = 0;
        //read(ITP_DEVICE_STC, &current_time, sizeof (current_time));
        fc_sync_gettime(&pprop->stc, &current_time);
        printf("YC: current time = %lld\n", current_time);
     */
    pthread_mutex_unlock(&player_mutex);
    return 0;
}

static int ithMediaPlayer_get_total_duration_ext(int64_t *total_time, char *filepath)
{
    int err;
    AVFormatContext *ic = NULL;
    av_register_all();
    avformat_network_init();
    ic = avformat_alloc_context();
    err = avformat_open_input(&ic, filepath, NULL, NULL);
    if (err < 0)
    {
        char rets[256] = {0};
        av_strerror(err, rets, sizeof(rets));
        av_log(NULL, AV_LOG_ERROR, "Open input error \"%s\" : %s\n", filepath, rets);
        *total_time = 0;
    }
    else
        *total_time = ic->duration;
    
    if (ic)
    {
        avformat_close_input(&ic);
        ic = NULL; /* safety */
    }
    avformat_network_deinit();
    return 0;
}

static int ithMediaPlayer_get_current_time(int64_t *current_time)
{
    PlayerProps    *pprop = global_player_prop;
    PlayerInstance *is    = NULL;
    int64_t        ctime;

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

    if (is->get_first_pkt)
    {
        *current_time = is->ori_frame_last_pts*1000000; 
    }
    else
        *current_time = 0;

    pthread_mutex_unlock(&player_mutex);
    return 0;
}

static void ithMediaPlayer_seekto(int pos)
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

#ifndef AV_SYNC_STC
    is->is_seekto_case = true;
#endif
    stream_seek(is, (int64_t)pos * 1000000, 0, 0);
    //#ifdef CFG_BUILD_ITV
    //    fc_sync_settime(&pprop->stc, (int64_t)pos*90000);
    //#endif
    pthread_mutex_unlock(&player_mutex);
    return;
}

static void ithMediaPlayer_slow_fast_play(float speed)
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

    if(is->video_speed != speed)
    {
        pthread_mutex_lock(&video_speed_mutex);
        is->video_speed = speed;
        is->is_videospeed_changed = true;
        pthread_mutex_unlock(&video_speed_mutex);
    }    
    pthread_mutex_unlock(&player_mutex);
    return;
}
static int ithMediaPlayer_get_file_pos(double *pos)
{
    PlayerProps    *pprop = global_player_prop;
    PlayerInstance *is    = NULL;

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

    *pos = is->ori_frame_last_pts;
    //printf("YC: xxxxx pos = %f xxxx\n", *pos);

    pthread_mutex_unlock(&player_mutex);
    return 0;
}

static int ithMediaPlayer_get_audio_codec_id(int *codec_id, char *filepath)
{
    int err;
    AVFormatContext *ic = NULL;
    av_register_all();
    avformat_network_init();
    ic = avformat_alloc_context();
    err = avformat_open_input(&ic, filepath, NULL, NULL);
    if (err < 0)
    {
        char rets[256] = {0};
        av_strerror(err, rets, sizeof(rets));
        av_log(NULL, AV_LOG_ERROR, "Open input error \"%s\" : %s\n", filepath, rets);
        *codec_id = 0;
    }
    else
    {
        int i;
        *codec_id = 0;
        for (i = 0; i < ic->nb_streams; i++)
        {
            AVStream *st = ic->streams[i];
            AVCodecContext *avctx = st->codec;
            if (avctx->codec_type != AVMEDIA_TYPE_AUDIO) continue;
            else
            {
                     *codec_id = (int)avctx->codec_id;
            }
        }
    }
    if (ic)
    {
        avformat_close_input(&ic);
        ic = NULL; /* safety */
    }
    avformat_network_deinit();
    return 0;
}

static bool ithMediaPlayer_check_fileplayer_playing()
{
    PlayerProps    *pprop = global_player_prop;
    if (!pprop)
    {
        //av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        return false;
    }
    return pprop->isFilePlayer_thread_created;
}
static int ithMediaPlayer_volume_up(void)
{
#ifdef CFG_BUILD_I2S
    i2s_volume_up();
#endif
    return 0;
}

static int ithMediaPlayer_volume_down(void)
{
#ifdef CFG_BUILD_I2S
    i2s_volume_down();
#endif
    return 0;
}

static int ithMediaPlayer_mute(void)
{
    PlayerProps *pprop = global_player_prop;
    int         mute   = -1;

    if (!pprop)
    {
        av_log(NULL, AV_LOG_ERROR, "Player not exist\n");
        return -1;
    }
    mute        = !pprop->mute;
    pprop->mute = mute;

#ifdef CFG_BUILD_I2S
    i2s_mute_DAC(mute);
#endif

    return mute;
}

/* hint: Used only in DVR mode, drop streams from TsSource */
//#include "ts_source.h"
int ithMediaPlayer_drop_all_input_streams(void)
{
    return 0;
}

#if defined(_MSC_VER)
ithMediaPlayer fileplayer = {
    ithMediaPlayer_init,
    ithMediaPlayer_select_file,
    ithMediaPlayer_play,
    ithMediaPlayer_pause,
    ithMediaPlayer_stop,
    ithMediaPlayer_play_videoloop,
    ithMediaPlayer_deinit,
    ithMediaPlayer_get_total_duration,
    ithMediaPlayer_get_total_duration_ext,
    ithMediaPlayer_get_current_time,
    ithMediaPlayer_seekto,
    ithMediaPlayer_slow_fast_play,
    ithMediaPlayer_get_file_pos,
    ithMediaPlayer_get_audio_codec_id,
    ithMediaPlayer_check_fileplayer_playing,
    ithMediaPlayer_volume_up,
    ithMediaPlayer_volume_down,
    ithMediaPlayer_mute,
    ithMediaPlayer_drop_all_input_streams,
    ithMediaPlayer_InitAVDecodeEnv,
    ithMediaPlayer_InitH264DecodeEnv,
    ithMediaPlayer_InitAudioDecodeEnv,
    ithMediaPlayer_h264_decode_from_rtsp,
    ithMediaPlayer_audio_decode_from_rtsp,
    ithMediaPlayer_DeinitAVDecodeEnv
};
#else // no defined _MSC_VER
ithMediaPlayer fileplayer = {
    .init                   = ithMediaPlayer_init,
    .select                 = ithMediaPlayer_select_file,
    .play                   = ithMediaPlayer_play,
    .pause                  = ithMediaPlayer_pause,
    .stop                   = ithMediaPlayer_stop,
    .play_videoloop         = ithMediaPlayer_play_videoloop,
    .deinit                 = ithMediaPlayer_deinit,
    .gettotaltime           = ithMediaPlayer_get_total_duration,
    .gettotaltime_ext       = ithMediaPlayer_get_total_duration_ext,
    .getcurrenttime         = ithMediaPlayer_get_current_time,
    .seekto                 = ithMediaPlayer_seekto,
    .slow_fast_play          = ithMediaPlayer_slow_fast_play,
    .getfilepos             = ithMediaPlayer_get_file_pos,
    .getaudioCodecId = ithMediaPlayer_get_audio_codec_id,
    .check_fileplayer_playing = ithMediaPlayer_check_fileplayer_playing,
    .vol_up                 = ithMediaPlayer_volume_up,
    .vol_down               = ithMediaPlayer_volume_down,
    .mute                   = ithMediaPlayer_mute,
    .drop_all_input_streams = ithMediaPlayer_drop_all_input_streams,
    .InitAVDecodeEnv        = ithMediaPlayer_InitAVDecodeEnv,
    .InitH264DecodeEnv      = ithMediaPlayer_InitH264DecodeEnv,
    .InitAudioDecodeEnv     = ithMediaPlayer_InitAudioDecodeEnv,
    .h264_decode_from_rtsp  = ithMediaPlayer_h264_decode_from_rtsp,
    .audio_decode_from_rtsp = ithMediaPlayer_audio_decode_from_rtsp,
    .DeinitAVDecodeEnv      = ithMediaPlayer_DeinitAVDecodeEnv
};
#endif
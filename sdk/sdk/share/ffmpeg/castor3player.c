#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "castor3player.h"
#include "ite/itv.h"

#define SERR()

typedef enum VIDEO_PLAYER_STATE_TAG
{
    VIDEO_PLAYER_STATE_NOT_INITED,
    VIDEO_PLAYER_STATE_STOPPED,
    VIDEO_PLAYER_STATE_PLAYING
} VIDEO_PLAYER_STATE;

//mtal player mutex
pthread_mutex_t    mtal_mutex = PTHREAD_MUTEX_INITIALIZER;
VIDEO_PLAYER_STATE mtal_state = VIDEO_PLAYER_STATE_NOT_INITED;

MTAL_SPEC g_mtal_spec;
static cb_handler_t gcallback;
ithMediaPlayer *media_player = &fileplayer;

/* MTAL */
// MTAL is a video player
int mtal_pb_init(cb_handler_t callback) /* open for later playback */
{
    printf("MTAL# %s +\n", __func__);

    pthread_mutex_lock(&mtal_mutex);
    gcallback = callback;

    switch (mtal_state)
    {
    case VIDEO_PLAYER_STATE_NOT_INITED:
        {
            /* TODO: add playback routine here */
            if (media_player && media_player->init)
            {
                // ...
                int retv = media_player->init(callback);

                if (retv < 0)
                {
                    SERR();
                }
            }
#ifdef CFG_BUILD_ITV            
            itv_flush_dbuf();   // why do this?
#endif            

            // mtal_set_freerun(0);
            // mtal_set_pb_mode(1);
            mtal_state = VIDEO_PLAYER_STATE_STOPPED;
        }
        break;

    default:
        break;
    }

    pthread_mutex_unlock(&mtal_mutex);

    printf("MTAL# %s -\n", __func__);
    return 0;
}

int mtal_pb_exit(void) /* close for termination of playback */
{
    printf("MTAL# %s +\n", __func__);
    pthread_mutex_lock(&mtal_mutex);

    switch (mtal_state)
    {
    case VIDEO_PLAYER_STATE_NOT_INITED:
        break;

    default:
        {
            if (media_player && media_player->deinit)
            {
                // ...
                int retv = media_player->deinit();

                if (retv < 0)
                {
                    SERR();
                }
            }

            memset((void *)&g_mtal_spec, 0, sizeof(MTAL_SPEC));

            mtal_state = VIDEO_PLAYER_STATE_NOT_INITED;
        }
        break;
    }

    pthread_mutex_unlock(&mtal_mutex);
#ifdef WIN32
    //pthread_mutex_destroy(&mtal_mutex);
#endif

    printf("MTAL# %s -\n", __func__);
    return 0;
}

int mtal_pb_select_file(MTAL_SPEC *spec)
{
    ithMediaPlayer *new_player;
    printf("MTAL# %s +\n", __func__);
    pthread_mutex_lock(&mtal_mutex);
    memcpy((void *)&g_mtal_spec, (void *)spec, sizeof(MTAL_SPEC));

#ifdef CFG_CAPTURE_MODULE_ENABLE
    if (strcmp(g_mtal_spec.srcname,"CaptureDev") == 0)
    {        
        new_player = &captureplayer;        
    }
    else
#endif
    {
        new_player = &fileplayer;
    }


    if (new_player != media_player)
    {
        mtal_pb_exit();
        media_player = new_player;
		//mtal_pb_init(gcallback);
        media_player->init(gcallback);
		mtal_state = VIDEO_PLAYER_STATE_STOPPED;
    }

    if (media_player && media_player->select)
    {
        media_player->select(g_mtal_spec.srcname, g_mtal_spec.vol_level);
    }
#if defined(CFG_BUILD_ITV) && !defined(CFG_TEST_VIDEO)    
    itv_set_pb_mode(1);
#endif
    pthread_mutex_unlock(&mtal_mutex);
    printf("MTAL# %s -\n", __func__);
    return 0;
}

int mtal_pb_play(void)
{
    printf("MTAL# %s +\n", __func__);
    pthread_mutex_lock(&mtal_mutex);
    /* mtal routine */
#ifdef CFG_BUILD_ITV    
    itv_flush_dbuf();
#endif

    /* TODO: add playback routine here */
    if (media_player && media_player->play)
    {
        int retv;
        retv = media_player->play();
        if (retv < 0)
        {
            SERR();
        }
    }
    pthread_mutex_unlock(&mtal_mutex);
    printf("MTAL# %s -\n", __func__);
    return 0;
}

int mtal_pb_pause(void)
{
    printf("MTAL# %s +\n", __func__);
    pthread_mutex_lock(&mtal_mutex);
    if (media_player && media_player->pause)
    {
        int retv;
        retv = media_player->pause();
        if (retv < 0)
            return -1;
    }

    pthread_mutex_unlock(&mtal_mutex);
    printf("MTAL# %s -\n", __func__);
    return 0;
}

int mtal_pb_stop(void)
{
   
    int retv;
    pthread_mutex_lock(&mtal_mutex);
    printf("MTAL# %s +\n", __func__);
    switch (mtal_state)
    {
    case VIDEO_PLAYER_STATE_NOT_INITED:
        break;

    default:
        {
            /* TODO: add playback routine here */
            if (media_player && media_player->stop)
            {
#if (CFG_BUILD_ITV) && (CFG_CHIP_FAMILY == 9850)  
                itv_stop_vidSurf_anchor();
#endif            	
                retv = media_player->stop();

                if (retv < 0)
                {
                    SERR();
                }
            }
            /* mtal routine */
#ifdef CFG_BUILD_ITV            
            itv_flush_dbuf();
#ifndef CFG_TEST_VIDEO
            itv_set_pb_mode(0);
#endif
#endif            
            // -
            mtal_state = VIDEO_PLAYER_STATE_STOPPED;
        }
        break;
    }
    pthread_mutex_unlock(&mtal_mutex);
    printf("MTAL# %s -\n", __func__);
    return 0;
}

int mtal_pb_play_videoloop(void)
{
    printf("MTAL# %s +\n", __func__);
    pthread_mutex_lock(&mtal_mutex);
    /* mtal routine */
#ifdef CFG_BUILD_ITV    
    itv_flush_dbuf();
#endif

    /* TODO: add playback routine here */
    if (media_player && media_player->play_videoloop)
    {
        int retv;
        retv = media_player->play_videoloop();
        if (retv < 0)
        {
            SERR();
        }
    }
    pthread_mutex_unlock(&mtal_mutex);
    printf("MTAL# %s -\n", __func__);
    return 0;
}


int mtal_pb_get_total_duration(int *totaltime)
{
    int64_t micro_secs = 0;
#if 0
    int     hours, mins, secs;
#else
    int secs;
#endif

    pthread_mutex_lock(&mtal_mutex);
    if (media_player && media_player->gettotaltime)
    {
        int retv = media_player->gettotaltime(&micro_secs);
        if (retv < 0)
        {
            pthread_mutex_unlock(&mtal_mutex);
            return -1;
        }    
    }
    secs       = (int)(micro_secs / 1000000);
    *totaltime = secs;
#if 0
    mins       = secs / 60;
    secs      %= 60;
    hours      = mins / 60;
    mins      %= 60;

    sprintf(totaltime, "%02d:%02d:%02d", hours, mins, secs);
#endif
    //printf("YC: total time = %lld\n", micro_secs);
    pthread_mutex_unlock(&mtal_mutex);
    return 0;
}

int mtal_pb_get_total_duration_ext(int *totaltime, char *filepath)
{
    int64_t micro_secs = 0;
    int secs;
    pthread_mutex_lock(&mtal_mutex);
    if (media_player && media_player->gettotaltime_ext)
    {
        int retv = media_player->gettotaltime_ext(&micro_secs, filepath);
        if (retv < 0)
            SERR();
    }
    secs = (int)(micro_secs / 1000000);
    *totaltime = secs;
    pthread_mutex_unlock(&mtal_mutex);
    return 0;
}

int mtal_pb_get_current_time(int *currenttime)
{
    int64_t ctimes = 0;
    int     retv;
#if 0
    int     hours, mins, secs;
#else
    int secs;
#endif
    //  printf("MTAL# %s +\n", __func__);

    pthread_mutex_lock(&mtal_mutex);
    if (media_player && media_player->getcurrenttime)
    {
        retv = media_player->getcurrenttime(&ctimes);
        if (retv < 0)
        {
            pthread_mutex_unlock(&mtal_mutex);
            return -1;
        }
    }
    secs         = (int)(ctimes / 1000000);
    *currenttime = secs;
#if 0
    mins         = secs / 60;
    secs        %= 60;
    hours        = mins / 60;
    mins        %= 60;

    sprintf(currenttime, "%02d:%02d:%02d", hours, mins, secs);
#endif
    //printf("YC: total time = %lld\n", micro_secs);
    //  printf("MTAL# %s -\n", __func__);
    pthread_mutex_unlock(&mtal_mutex);
    return 0;
}

int mtal_pb_seekto(int file_pos)
{
    pthread_mutex_lock(&mtal_mutex);
    if (media_player && media_player->seekto)
    {
        int retv = media_player->seekto(file_pos);
        if (retv < 0)
            SERR();
    }

    pthread_mutex_unlock(&mtal_mutex);
    return 0;
}

int mtal_pb_slow_fast_play(float speed)
{
    pthread_mutex_lock(&mtal_mutex);
    if (media_player && media_player->slow_fast_play)
    {
        int retv = media_player->slow_fast_play(speed);
        if (retv < 0)
            SERR();
    }

    pthread_mutex_unlock(&mtal_mutex);
    return 0;
}


int mtal_pb_get_file_pos(int *file_pos)
{
    double pos = 0;
    pthread_mutex_lock(&mtal_mutex);
    if (media_player && media_player->getfilepos)
    {
        int retv = media_player->getfilepos(&pos);
        if (retv < 0)
            SERR();
    }

    *file_pos = (int)pos;
    pthread_mutex_unlock(&mtal_mutex);
    return 0;
}

int mtal_pb_get_audio_codec_id(char *filepath)
{
    int codec_id;
    pthread_mutex_lock(&mtal_mutex);
    if (media_player && media_player->gettotaltime_ext)
    {
        media_player->getaudioCodecId(&codec_id, filepath);
    }
    pthread_mutex_unlock(&mtal_mutex);
    return codec_id;
}

bool mtal_pb_check_fileplayer_playing()
{
    if (media_player && media_player->check_fileplayer_playing)
    {
        return media_player->check_fileplayer_playing();
    }
    return false;
}

int mtal_pb_InitAVDecodeEnv(void)
{
    printf("MTAL# %s +\n", __func__);
    if (media_player && media_player->InitAVDecodeEnv)
    {
        media_player->InitAVDecodeEnv();
    }

    printf("MTAL# %s -\n", __func__);
    return 0;
}

int mtal_pb_InitH264DecodeEnv(void)
{
    printf("MTAL# %s +\n", __func__);
    if (media_player && media_player->InitH264DecodeEnv)
    {
        media_player->InitH264DecodeEnv();
    }

    printf("MTAL# %s -\n", __func__);
    return 0;
}

int mtal_pb_InitAudioDecodeEnv(int samplerate, int num_channels, RTSPCLIENT_AUDIO_CODEC codec_id)
{
    printf("MTAL# %s +\n", __func__);
    if (media_player && media_player->InitAudioDecodeEnv)
    {
        media_player->InitAudioDecodeEnv(samplerate, num_channels, codec_id);
    }

    printf("MTAL# %s -\n", __func__);
    return 0;
}

int mtal_pb_h264_decode_from_rtsp(unsigned char *buf, int size, double timestamp)
{
    //printf("MTAL# %s +\n", __func__);
    if (media_player && media_player->h264_decode_from_rtsp)
    {
        int retv = media_player->h264_decode_from_rtsp(buf, size, timestamp);
        if (retv < 0)
            SERR();
    }
    //printf("MTAL# %s -\n", __func__);
    return 0;
}

int mtal_pb_audio_decode_from_rtsp(unsigned char *buf, int size, double timestamp)
{
    //printf("MTAL# %s +\n", __func__);
    if (media_player && media_player->audio_decode_from_rtsp)
    {
        int retv = media_player->audio_decode_from_rtsp(buf, size, timestamp);
        if (retv < 0)
            SERR();
    }
    //printf("MTAL# %s -\n", __func__);
    return 0;
}

int mtal_pb_DeinitAVDecodeEnv(void)
{
    printf("MTAL# %s +\n", __func__);
    if (media_player && media_player->DeinitAVDecodeEnv)
    {
        media_player->DeinitAVDecodeEnv();
    }

    printf("MTAL# %s -\n", __func__);
    return 0;
}

/* for live streaming */
int mtal_drop_all_input_streams(void)
{
    return media_player->drop_all_input_streams();
}

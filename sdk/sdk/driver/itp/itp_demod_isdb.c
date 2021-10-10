/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Demod functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include <string.h>
#include "itp_cfg.h"
#include "pthread.h"

#include "tsi/mmp_tsi.h"
#include "ts_demuxer/ite_ts_demuxer.h"

#include "itp_demod_thread.h"

//=============================================================================
//                Macro Definition
//=============================================================================
#ifdef DEBUG
    #ifdef _MSC_VER // WIN32
        #define itp_msg(type, string, ...)      ((void)((type & LOG_ZONES) ? printf(string, __VA_ARGS__) : 0))
        #define itp_msg_ex(type, string, ...)   do{ if(type & LOG_ZONES){ \
                                                    printf(string, __VA_ARGS__); \
                                                    printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                                }while(0)

    #else
        #define itp_msg(type, string, args...)      ((void)((type & LOG_ZONES) ? printf(string, ## args) : 0))
        #define itp_msg_ex(type, string, args...)   do{ if(type & LOG_ZONES){ \
                                                        printf(string, ## args); \
                                                        printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                                    }while(0)
    #endif
#else
    #ifdef _MSC_VER // WIN32
        #define itp_msg(type, string, ...)
        #define itp_msg_ex(type, string, ...)
    #else
        #define itp_msg(type, string, args...)
        #define itp_msg_ex(type, string, args...)
    #endif
#endif

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct _demod_ts_ctxt_tag
{
    uint32_t            refCnt;

    uint32_t            demod_total_cnt;

    struct DEMODUSERINFO{
        pthread_t           demod_threads;
        pthread_mutex_t     demod_mutex;
        TSD_HANDLE          *pHTsd;
        struct timeval      lastT;
        void                *privData;
    }demodUserInfo[ITP_MAX_DEMOD_SUPPORT_CNT];

}demod_ts_ctxt;

//=============================================================================
//                Global Data Definition
//=============================================================================
static ITPDemodInfo     demodInfo;
demod_ts_ctxt    g_demodTsCtxt = {0};

//=============================================================================
//                Private Function Definition
//=============================================================================
static void
_demodIsdb_get_clock(
    struct timeval *startT)
{
    gettimeofday(startT, NULL);
}

static uint32_t
_demodIsdb_get_duration(
    struct timeval *startT)
{
    struct timeval currT = {0};
    uint64_t  duration_time = 0;

    gettimeofday(&currT, NULL);
    duration_time = (currT.tv_sec - startT->tv_sec) * 1000;      // sec to ms
    duration_time += ((currT.tv_usec - startT->tv_usec) / 1000); // us to ms
    return (uint32_t)duration_time;
}


static int DemodIsdb_Open(const char* name, int flags, int mode, void* info)
{
    int                 fid = -1;
    TSD_HANDLE          *pHTsd = 0;
    uint32_t            value = 0;
    uint32_t            serivce_idx = 0; //(uint32_t)name;
    uint32_t            demod_idx = 0;
    uint32_t            sound_idx = 0;
    char                *service_name = (char*)(name + 1);
    char                *subname = strchr(name, ':');
    TSD_SRVC_USER_INFO  *pSrvcUserInfo = 0;
    int                 i;
    struct DEMODUSERINFO *pDemodUserInfo = 0;

    service_name = subname ? subname + 1 : (char*)(name + 1);

    value = atoi(name);

    demod_idx   = ((value & 0xFF000000) >> 24);
    sound_idx   = ((value & 0x00FF0000) >> 16);
    serivce_idx = (value & 0xFFFF);

    pDemodUserInfo = &g_demodTsCtxt.demodUserInfo[demod_idx];
    _mutex_lock(pDemodUserInfo->demod_mutex);

    do{
        uint32_t        act_demod_id = 0;

        // change service
        if( (serivce_idx != 0xFFFF) )
        {
            act_demod_id = (uint32_t)pDemodUserInfo->privData;
            TsiSkipBuffer(act_demod_id, false);

            TsiStopCache(act_demod_id);
        }

        fid = (int)act_demod_id;
    }while(0);

    _mutex_unlock(pDemodUserInfo->demod_mutex);
    return fid;

}

static int DemodIsdb_Close(int file, void* info)
{
    uint32_t    demod_idx = (file & 0xFF);

    do{
        uint32_t        act_demod_id = 0;

        if( file < 0 )      break;

        itp_msg_ex(1, ".... demdoe id = %d\n", demod_idx);

        act_demod_id = (uint32_t)g_demodTsCtxt.demodUserInfo[demod_idx].privData;

        TsiSkipBuffer(act_demod_id, true);
    }while(0);

    return 0;
}

static int DemodIsdb_Read(int file, char *ptr, int len, void* info)
{
    int       result = 0;
    uint32_t    demod_idx = (file & 0xFF);
    int         read_size = 0;
    uint32_t    quality = 0;

    // _mutex_lock(g_demodTsCtxt.demod_mutex);

    do{
        struct timeval      *lastT;

        if( file < 0 )      break;

        lastT = &g_demodTsCtxt.demodUserInfo[demod_idx].lastT;

#if 0
        if(1)
        {
            static struct timeval startT;

            if( tsd_dbg_get_duration(&startT) > 60000 )
            {
                tsd_dbg_get_clock(&startT);
                printf("\n=======\n");
                malloc_stats();
            }
        }
#endif

        result = TsiReadBuffer(demod_idx, (uint8_t*)ptr, (unsigned int)len);
        if( !result )       read_size = len;

        if( read_size != len )
            printf(" warning: %s() get wrong size (%d, %d)\n", __FUNCTION__, read_size, len);
    }while(0);

    // _mutex_unlock(g_demodTsCtxt.demod_mutex);
    return read_size;
}

static void DemodIsdb_Init(void)
{
    int                 rc;

    if( g_demodTsCtxt.refCnt == 0 )
    {
        uint32_t             demod_count = 0;
        int                  i, j = 0;
        struct DEMODUSERINFO *pDemodUserInfo = 0;

        // create tsd handle
        pDemodUserInfo = &g_demodTsCtxt.demodUserInfo[demod_count];
        _mutex_init(pDemodUserInfo->demod_mutex);
        pDemodUserInfo->pHTsd = 0;
        // init tsi 1
        mmpTsiInitialize(1); // only tsi 1 support parallel mode

        demod_count++;

#if 0 //(CFG_DEMOD_SUPPORT_COUNT > 1) && (CFG_DEMOD_SUPPORT_COUNT < 3)
    
    #error "only tsi 1 support parallel mode"
        pDemodUserInfo = &g_demodTsCtxt.demodUserInfo[demod_count];
        _mutex_init(pDemodUserInfo->demod_mutex);
        pDemodUserInfo->pHTsd = 0;
        // init tsi 1
        mmpTsiInitialize(demod_count);

        demod_count++;
#endif

        g_demodTsCtxt.demod_total_cnt = demod_count;
        printf(" g_demodTsCtxt.demod_total_cnt=%d\n", g_demodTsCtxt.demod_total_cnt);
        for(i = 0; i < g_demodTsCtxt.demod_total_cnt; i++)
        {
            TSI_THREAD_PARAM    tsiThreadParam = {0};

            if( i < ITP_MAX_DEMOD_SUPPORT_CNT )
            {
                TSD_HANDLE  *pHTsd = g_demodTsCtxt.demodUserInfo[i].pHTsd;
                pthread_attr_t attr;

                TsiSkipBuffer(i, true);
                g_demodTsCtxt.demodUserInfo[i].privData = (void*)i;

                // Create thread
                tsiThreadParam.handle = (uint32_t)pHTsd;
                tsiThreadParam.idx    = 1;
                pthread_attr_init(&attr);
                attr.schedparam.sched_priority = 2;
                rc = pthread_create(&g_demodTsCtxt.demodUserInfo[i].demod_threads, &attr, TsiBufferThread, (void *)&tsiThreadParam);
                if( rc )
                {
                    itp_msg_ex(ZONE_ERROR, "ERROR; pthread_create() fail %d\n", rc);
                }
                usleep(5000);
            }
        }

        demodInfo.ts_country_cnt = 1;

        // send data size to clint
        demodInfo.blockSize = (1 * 1024 * 1024); //(1 << 20); // 2MB

        g_demodTsCtxt.refCnt = 1;
    }
    else
        g_demodTsCtxt.refCnt++;

}

static void DemodIsdb_Terminate(void)
{
    int     i;

    if( g_demodTsCtxt.refCnt == 1 )
    {
        struct DEMODUSERINFO *pDemodUserInfo = 0;

        for(i = 0; i < g_demodTsCtxt.demod_total_cnt; i++)
        {
            pDemodUserInfo = &g_demodTsCtxt.demodUserInfo[i];
            _mutex_lock(pDemodUserInfo->demod_mutex);

            TsiStopThread(i);

            pthread_join(g_demodTsCtxt.demodUserInfo[i].demod_threads, NULL);
            
            mmpTsiTerminate(1);
            
            _mutex_unlock(pDemodUserInfo->demod_mutex);
            _mutex_deinit(pDemodUserInfo->demod_mutex);
        }

        // set frequency/bandwidth
        demodInfo.frequency = 0;
        demodInfo.bandwidth = 0;

        // upnp parameter
        demodInfo.blockSize = 0;

        g_demodTsCtxt.refCnt = 0;
    }
    else
        g_demodTsCtxt.refCnt--;

    return;
}


static int DemodIsdb_Ioctl(int file, unsigned long request, void* ptr, void* info)
{
    int             result = 0;
    uint32_t        demod_idx = (file & 0xFF);
    struct DEMODUSERINFO *pDemodUserInfo = 0;

    pDemodUserInfo = &g_demodTsCtxt.demodUserInfo[demod_idx];

    _mutex_lock(pDemodUserInfo->demod_mutex);

    switch (request)
    {
    case ITP_IOCTL_INIT:
        DemodIsdb_Init();
        return result;
        break;

    case ITP_IOCTL_EXIT:
        DemodIsdb_Terminate();
        break;

    case ITP_IOCTL_RECEIVE_TS:
        // enable tsi
        mmpTsiEnable(demod_idx);
        TsiStartCache(demod_idx);
        break;

    case ITP_IOCTL_SKIP_TS:
        // disable tsi
        mmpTsiDisable(demod_idx);
        TsiStopCache(demod_idx);
        break;

    case ITP_IOCTL_GET_COUNRTY_NAME: // combine with ITP_IOCTL_GET_INFO ??
        *(ITPDemodInfo**)ptr = &demodInfo;
        break;

    case ITP_IOCTL_GET_TS_HANDLE:
        if( ptr )   *(TSD_HANDLE**)ptr = 0;
        break;

    case ITP_IOCTL_GET_INFO:
        {
            int                 j;

            // need default ISDB-T service attribute
            // To Do:

            demodInfo.ts_service_cnt = 5;
            for(j = 0; j < demodInfo.ts_service_cnt; j++)
            {
                demodInfo.ts_service_table[j].service_order_index = j;
                demodInfo.ts_service_table[j].totalSoundTracks    = 1;
                demodInfo.ts_service_table[j].bVideoService       = true;
            }

            *(ITPDemodInfo**)ptr = &demodInfo;
        }
        break;

    default:
        errno = (ITP_DEVICE_DEMOD << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        itp_msg_ex(0, "\tWrong request(%d) in %s() !", request, __FUNCTION__);
        result = -1;
        break;
    }

    _mutex_unlock(pDemodUserInfo->demod_mutex);

    return result;
}

//=============================================================================
//                Public Function Definition
//=============================================================================
const ITPDevice itpDeviceDemod =
{
    ":demod",
    DemodIsdb_Open,
    DemodIsdb_Close,
    DemodIsdb_Read,
    itpWriteDefault,
    itpLseekDefault,
    DemodIsdb_Ioctl,
    NULL
};

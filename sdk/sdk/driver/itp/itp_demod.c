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

#include "ts_demuxer/ite_ts_demuxer.h"
#include "itp_demod_thread.h"

#include "ts_demuxer/ts_debug.h"

//=============================================================================
//				  Macro Definition
//=============================================================================
#ifndef LOG_ZONES
    #define LOG_ZONES   0x1
#endif

#ifndef ZONE_ERROR
    #define ZONE_ERROR      0x1
#endif

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


#if (CFG_DEMOD_OMEGA)
    #define ACTION_DEMOD_ID     TSD_DEMOD_OMEGA
#elif (CFG_DEMOD_IT9135)
    #define ACTION_DEMOD_ID     TSD_DEMOD_IT9135
#elif (CFG_DEMOD_IT9137)
    #define ACTION_DEMOD_ID     TSD_DEMOD_IT9137
#elif (CFG_DEMOD_IT9137_USB)
    #define ACTION_DEMOD_ID     TSD_DEMOD_IT9137_USB
#else 
    #define ACTION_DEMOD_ID     TSD_DEMOD_UNKNOW
#endif

#define DEMOD_SIGNAL_STATUS_TIME_GAP    1500
//=============================================================================
//				  Structure Definition
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
//				  Global Data Definition
//=============================================================================
static ITPDemodInfo     demodInfo;
demod_ts_ctxt    g_demodTsCtxt = {0};

//=============================================================================
//				  Private Function Definition
//=============================================================================
static void
itpDemod_get_clock(
    struct timeval *startT)
{
    gettimeofday(startT, NULL);
}

static uint32_t
itpDemod_get_duration(
    struct timeval *startT)
{
    struct timeval currT = {0};
    uint64_t  duration_time = 0;
    
    gettimeofday(&currT, NULL);
    duration_time = (currT.tv_sec - startT->tv_sec) * 1000;      // sec to ms
    duration_time += ((currT.tv_usec - startT->tv_usec) / 1000); // us to ms
    return (uint32_t)duration_time;
}


static uint32_t    
_export(
    TSD_INFO_REPO   *pTsdInfoRepo, 
    uint8_t         *buf, 
    uint32_t        byteSize)
{
    return 0;
}

#if _MSC_VER
typedef struct _TS_SRC_INPUT_TAG
{
    FILE        *f_ts;
    uint32_t    freq;

}TS_SRC_INPUT;

FILE                    *f_ts_scan = 0;
uint32_t                ts_scan_file_size = 0;
static TS_SRC_INPUT     ts_src_input[2] = {0};
static void
_scan_channel(
    uint32_t            demod_id,  
    ITP_TS_COUNTRY_ID   countryId,
    TSD_SCAN_PARAM      *pScanParm)
{
    int                 i;
    TSD_SCAN_PARAM      scanParm = {0};
    TSD_PRE_SET_INFO    presetInfo = {0};
    TSD_HANDLE          *pHTsd = 0;

    char    *ts_name[2] ={
        //{"./Greece_30_12_09_1st_stream.ts"},
        //{"./CTV_MyLife_tw_0001.ts"},
        {"A:/CTV_MyLife_tw_0001.ts"},
        {"A:/CTV_MyLife_tw_0001.ts"}
    };

    pHTsd = g_demodTsCtxt.demodUserInfo[demod_id].pHTsd;

    for(i = 0; i < 2; i++)
    {
        if( !(ts_src_input[i].f_ts = fopen(ts_name[i], "rb")) )
        {
            printf("open %s fail !\n", ts_name[i]);
            while(1);
        }

        fseek(ts_src_input[i].f_ts, 0, SEEK_END);
        ts_scan_file_size = ftell(ts_src_input[i].f_ts);
        fseek(ts_src_input[i].f_ts, 0, SEEK_SET);

        f_ts_scan = ts_src_input[i].f_ts;

        scanParm.scanFrequency = 533 + i * 30;
        scanParm.bandwidth     = 6000;
        scanParm.countryId     = TSD_COUNTRY_TAIWAN;

        ts_src_input[i].freq = scanParm.scanFrequency;

        tsd_Scan_Channel(pHTsd, TSD_FREQ_SCAN_MANUAL, &scanParm, ((!i)? 0:1));
    }
    return;
}

static int DemodOpen(const char* name, int flags, int mode, void* info)
{
    TSD_HANDLE          *pHTsd = 0;
    uint32_t            value = 0;
    uint32_t            serivce_idx = 0; //(uint32_t)name; 
    uint32_t            demod_idx = 0; 
    char                *service_name = (char*)(name + 1);
    char                *subname = strchr(name, ':');
    TSD_SRVC_USER_INFO  *pSrvcUserInfo = 0;
    int                 i;
    struct DEMODUSERINFO *pDemodUserInfo = 0;
    
    service_name = subname ? subname + 1 : (char*)(name + 1);
    
    value = atoi(name);
    
    demod_idx   = ((value & 0xFF000000) >> 24);
    serivce_idx = (value & 0xFFFF);

    pDemodUserInfo = &g_demodTsCtxt.demodUserInfo[demod_idx];
    _mutex_lock(pDemodUserInfo->demod_mutex);

    if( !pHTsd )
    {
        itp_msg_ex(ZONE_ERROR, "No ts demuxer handle !");
        errno = (ITP_DEVICE_DEMOD << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        serivce_idx = -1;
        goto end;
    }

    tsd_Control(pHTsd, TSD_CTRL_SET_DEMOD_STATUS, (uint32_t*)TSD_DEMOD_STATUS_RUNNING, 0);
    
    // change service
    if( (serivce_idx != 0xFFFF) &&
        (pHTsd->actSrvcIdx != serivce_idx) )
    {
        tsd_Change_Service(pHTsd, serivce_idx, 0);
        for(i = 0; i < 2; i++)
            if( pHTsd->actFreq == ts_src_input[i].freq )
            {
                TsiSkipBuffer(pHTsd->act_demod_id, false);
                f_ts_scan = ts_src_input[i].f_ts;
                break;
            }
    }

end:
    _mutex_unlock(pDemodUserInfo->demod_mutex);
    return (int)pHTsd->act_demod_id;

}
#else // #if _MSC_VER
static void
_scan_channel(
    uint32_t            demod_id,  
    ITP_TS_COUNTRY_ID   countryId,
    TSD_SCAN_PARAM      *pScanParm)
{ 
    TSD_HANDLE          *pHTsd = 0;

    pHTsd = g_demodTsCtxt.demodUserInfo[demod_id].pHTsd;
    
/*
	TSD_SCAN_PARAM		scanParm = {0};

	memcpy(&scanParm, pScanParm, sizeof(TSD_SCAN_PARAM));
	
    switch( countryId )
    {
        case ITP_TS_COUNTRY_AUSTRALIA:      scanParm.countryId = TSD_COUNTRY_AUSTRALIA;   break;
        case ITP_TS_COUNTRY_AUSTRIA:        scanParm.countryId = TSD_COUNTRY_AUSTRIA;     break;
        case ITP_TS_COUNTRY_CHINA:          scanParm.countryId = TSD_COUNTRY_CHINA;       break;
        case ITP_TS_COUNTRY_FRANCE:         scanParm.countryId = TSD_COUNTRY_FRANCE;      break;
        case ITP_TS_COUNTRY_GERMANY:        scanParm.countryId = TSD_COUNTRY_GERMANY;     break;
        case ITP_TS_COUNTRY_GREECE:         scanParm.countryId = TSD_COUNTRY_GREECE;      break;
        case ITP_TS_COUNTRY_HUNGARY:        scanParm.countryId = TSD_COUNTRY_HUNGARY;     break;
        case ITP_TS_COUNTRY_ITALY:          scanParm.countryId = TSD_COUNTRY_ITALY;       break;
        case ITP_TS_COUNTRY_NETHERLANDS:    scanParm.countryId = TSD_COUNTRY_NETHERLANDS;   break;
        case ITP_TS_COUNTRY_POLAND:         scanParm.countryId = TSD_COUNTRY_POLAND;      break;
        case ITP_TS_COUNTRY_PORTUGAL:       scanParm.countryId = TSD_COUNTRY_PORTUGAL;    break;
        case ITP_TS_COUNTRY_RUSSIAN:        scanParm.countryId = TSD_COUNTRY_RUSSIAN;     break;
        case ITP_TS_COUNTRY_SPAIN:          scanParm.countryId = TSD_COUNTRY_SPAIN;       break;
        case ITP_TS_COUNTRY_TAIWAN:         scanParm.countryId = TSD_COUNTRY_TAIWAN;      break;
        case ITP_TS_COUNTRY_UK:             scanParm.countryId = TSD_COUNTRY_UK;          break;
        case ITP_TS_COUNTRY_DENMARK:        scanParm.countryId = TSD_COUNTRY_DENMARK;     break;
        case ITP_TS_COUNTRY_SWEDEN:         scanParm.countryId = TSD_COUNTRY_SWEDEN;      break;
        case ITP_TS_COUNTRY_FINLAND:        scanParm.countryId = TSD_COUNTRY_FINLAND;     break;
        default:
        case ITP_TS_COUNTRY_NONE:           scanParm.countryId = TSD_COUNTRY_UNKNOW;      break;
    }
*/

#if 1
    tsd_Scan_Channel(pHTsd, TSD_FREQ_SCAN_AUTO, pScanParm, 0);
#else
    pScanParm->scanFrequency = 533000;//569000; //533000;
    pScanParm->bandwidth     = 6000;
    tsd_Scan_Channel(pHTsd, TSD_FREQ_SCAN_MANUAL, pScanParm, 0);
#endif

}

static int DemodOpen(const char* name, int flags, int mode, void* info)
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
        pHTsd = pDemodUserInfo->pHTsd;
        
        if( !pHTsd )
        {
            itp_msg_ex(ZONE_ERROR, "No ts demuxer handle !");
            errno = (ITP_DEVICE_DEMOD << ITP_DEVICE_ERRNO_BIT) | __LINE__;
            break;
        }
        
        //tsd_Control(g_demodTsCtxt.pHTsd[demod_idx], TSD_CTRL_SET_DEMOD_STATUS, TSD_DEMOD_STATUS_RUNNING, 0);
        tsd_Control(pHTsd, TSD_CTRL_SET_DEMOD_STATUS, (uint32_t*)TSD_DEMOD_STATUS_RUNNING, 0);
        
        // change service
        if( (serivce_idx != 0xFFFF) &&
            (pHTsd->actSrvcIdx != serivce_idx) )
        {
            TSD_ERR             tsd_rst = TSD_ERR_OK;
            
            TsiSkipBuffer(pHTsd->act_demod_id, false);
            tsd_rst = tsd_Change_Service(pHTsd, serivce_idx, 0);
            if( tsd_rst != TSD_ERR_OK)
            {
                itp_msg_ex(ZONE_ERROR, " change service fail (0x%x) !", tsd_rst);
                errno = (ITP_DEVICE_DEMOD << ITP_DEVICE_ERRNO_BIT) | __LINE__;
                break;
            }
            
            tsd_rst = tsd_Change_SoundTrack(pHTsd, -1, 0);
            if( tsd_rst != TSD_ERR_OK)
            {
                itp_msg_ex(ZONE_ERROR, " change service fail (0x%x) !", tsd_rst);
                //errno = (ITP_DEVICE_DEMOD << ITP_DEVICE_ERRNO_BIT) | __LINE__;
                //break;
            } 
            TsiStopCache(demod_idx); 
        }
        
        fid = (int)pHTsd->act_demod_id;
    }while(0);
    
    _mutex_unlock(pDemodUserInfo->demod_mutex);
    return fid;

}
#endif

static int DemodClose(int file, void* info)
{
    uint32_t    demod_idx = (file & 0xFF);
    TSD_HANDLE  *pHTsd = 0;

    do{
        if( file < 0 )      break;

        itp_msg_ex(1, ".... demdoe id = %d\n", demod_idx);

        pHTsd = g_demodTsCtxt.demodUserInfo[demod_idx].pHTsd;

        TsiSkipBuffer(pHTsd->act_demod_id, true);
        tsd_Control(pHTsd, TSD_CTRL_SET_DEMOD_STATUS, (uint32_t*)TSD_DEMOD_STATUS_IDLE, 0);
        tsd_Control(pHTsd, TSD_CTRL_RESET_ACT_INFO, 0, 0);
    }while(0);
    
    return 0;
}

static int DemodRead(int file, char *ptr, int len, void* info)
{
	int 	  result = 0;
	uint32_t    demod_idx = (file & 0xFF);
	int         read_size = 0;
	uint32_t    quality = 0;
	TSD_HANDLE  *pHTsd = 0;

    // _mutex_lock(g_demodTsCtxt.demod_mutex);

    do{
        struct timeval      *lastT;
        
        if( file < 0 )      break;
        
        pHTsd = g_demodTsCtxt.demodUserInfo[demod_idx].pHTsd;
        lastT = &g_demodTsCtxt.demodUserInfo[demod_idx].lastT;

#if 0
        if( tsd_dbg_get_duration(lastT) > DEMOD_SIGNAL_STATUS_TIME_GAP )
        {
            itpDemod_get_clock(lastT);
            tsd_Control(pHTsd, TSD_CTRL_GET_SIGNAL_STATUS, &quality, 0);
            if( quality < 60 )
                printf("\n\tdemod Weak Signal: %d\n", quality);
        }
#endif

#if 0
        if(0)
        {
            static struct timeval startT;

            if( tsd_dbg_get_duration(&startT) > 60000 )
            {
                tsd_dbg_get_clock(&startT);
                printf("\n=======\n");
                malloc_stats();
            }
        }
        
        if(0)
        {
            static struct timeval startT;

            if( tsd_dbg_get_duration(&startT) > 60000 )
            {
                TSD_INFO_REPO       tsdInfoRepo = {0};
                
                tsd_dbg_get_clock(&startT);
                printf("\n=======\n");
                tsdInfoRepo.tsd_repo_export = _export;
                tsd_Get_Service_Schedule(g_demodTsCtxt.pHTsd, g_demodTsCtxt.pHTsd->actSrvcIdx, &tsdInfoRepo, 0);
            }     
        }   
        
        if(1)
        {
            static struct timeval liveT = {0};
            static struct timeval startT;

            if( !liveT.tv_usec )   tsd_dbg_get_clock(&liveT);

            if( tsd_dbg_get_duration(&startT) > 60000 )
            {
                tsd_dbg_get_clock(&startT);
                printf("\tliveTime: %d sec\n", tsd_dbg_get_duration(&liveT)); 
            }
        }       
#endif

        result = TsiReadBuffer(demod_idx, (uint8_t*)ptr, (unsigned int)len);
        if( !result )       read_size = len;

        if( read_size != len )
            printf(" warning: DemodRead() get wrong size (%d, %d)\n", read_size, len);
    }while(0);

    // _mutex_unlock(g_demodTsCtxt.demod_mutex);
    return read_size; 
}

static void DemodInit(void)
{
    int                 rc;

    if( g_demodTsCtxt.refCnt == 0 )
    {
        uint32_t            demod_count = 0;
        int                 i, j, validCountryCnt;
        TSD_COUNTRY_ID      countryId = TSD_COUNTRY_UNKNOW;
        char                *countryName = 0;
        TSD_PRE_SET_INFO    presetInfo = {0};
        struct DEMODUSERINFO *pDemodUserInfo = 0;
        
        // create tsd handle
        pDemodUserInfo = &g_demodTsCtxt.demodUserInfo[demod_count];
        _mutex_init(pDemodUserInfo->demod_mutex);

        presetInfo.tsRecevier.tsrx_type = TSD_TSRX_TYPE_TSI;
        presetInfo.tsdOutMode   = TSD_OUT_TS_BY_PASS;
        presetInfo.tsdDemodType = ACTION_DEMOD_ID; 
        presetInfo.demod_id     = 0;
      #if defined(CFG_BOARD_ITE_9079_EVB)
        presetInfo.tsi_id       = 1;
      #else
        presetInfo.tsi_id       = 0;
      #endif
        tsd_CreateHandle(&pDemodUserInfo->pHTsd, &presetInfo, 0);
        if( pDemodUserInfo->pHTsd == 0 )  
            printf("create tsd handle fail (demodId=%d, tsiId=%d) !!\n", presetInfo.demod_id, presetInfo.tsi_id);
        else
            demod_count++;
        
#if (CFG_DEMOD_SUPPORT_COUNT > 1) && (CFG_DEMOD_SUPPORT_COUNT < 3)
#if defined(CFG_BOARD_ITE_9079_EVB) && defined(CFG_TS_DEMUX_ENABLE)
    #error "ite 9079 evb not suppout 2-nd demod"
#endif
        pDemodUserInfo = &g_demodTsCtxt.demodUserInfo[demod_count];
        _mutex_init(pDemodUserInfo->demod_mutex);

        presetInfo.tsRecevier.tsrx_type = TSD_TSRX_TYPE_TSI;
        presetInfo.tsdOutMode   = TSD_OUT_TS_BY_PASS;
        presetInfo.tsdDemodType = ACTION_DEMOD_ID; 
        presetInfo.demod_id     = 1;
        presetInfo.tsi_id       = 1;
        tsd_CreateHandle(&pDemodUserInfo->pHTsd, &presetInfo, 0);
        if( pDemodUserInfo->pHTsd == 0 )  
            printf("create tsd handle fail (demodId=%d, tsiId=%d) !!\n", presetInfo.demod_id, presetInfo.tsi_id);
        else
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
                
                // set supported country 
                validCountryCnt = 0;
                for(j = ITP_TS_COUNTRY_AUSTRALIA; j < ITP_TS_COUNTRY_CNT; j++)
                {
                    switch( j )
                    {
                        case ITP_TS_COUNTRY_AUSTRALIA:      countryId = TSD_COUNTRY_AUSTRALIA;   break;
                        case ITP_TS_COUNTRY_AUSTRIA:        countryId = TSD_COUNTRY_AUSTRIA;     break;
                        case ITP_TS_COUNTRY_CHINA:          countryId = TSD_COUNTRY_CHINA;       break;
                        case ITP_TS_COUNTRY_FRANCE:         countryId = TSD_COUNTRY_FRANCE;      break;
                        case ITP_TS_COUNTRY_GERMANY:        countryId = TSD_COUNTRY_GERMANY;     break;
                        case ITP_TS_COUNTRY_GREECE:         countryId = TSD_COUNTRY_GREECE;      break;
                        case ITP_TS_COUNTRY_HUNGARY:        countryId = TSD_COUNTRY_HUNGARY;     break;
                        case ITP_TS_COUNTRY_ITALY:          countryId = TSD_COUNTRY_ITALY;       break;
                        case ITP_TS_COUNTRY_NETHERLANDS:    countryId = TSD_COUNTRY_NETHERLANDS;   break;
                        case ITP_TS_COUNTRY_POLAND:         countryId = TSD_COUNTRY_POLAND;      break;
                        case ITP_TS_COUNTRY_PORTUGAL:       countryId = TSD_COUNTRY_PORTUGAL;    break;
                        case ITP_TS_COUNTRY_RUSSIAN:        countryId = TSD_COUNTRY_RUSSIAN;     break;
                        case ITP_TS_COUNTRY_SPAIN:          countryId = TSD_COUNTRY_SPAIN;       break;
                        case ITP_TS_COUNTRY_TAIWAN:         countryId = TSD_COUNTRY_TAIWAN;      break;
                        case ITP_TS_COUNTRY_UK:             countryId = TSD_COUNTRY_UK;          break;
                        case ITP_TS_COUNTRY_DENMARK:        countryId = TSD_COUNTRY_DENMARK;     break;
                        case ITP_TS_COUNTRY_SWEDEN:         countryId = TSD_COUNTRY_SWEDEN;      break;
                        case ITP_TS_COUNTRY_FINLAND:        countryId = TSD_COUNTRY_FINLAND;     break;
                        default:
                        case ITP_TS_COUNTRY_NONE:           countryId = TSD_COUNTRY_UNKNOW;      break;
                    }
                
                    tsd_Get_CountryName(pHTsd, countryId, &countryName, 0);
                    if( countryName )
                    {
                        demodInfo.ts_country_table[validCountryCnt].countryId = j;
                        strncpy(demodInfo.ts_country_table[validCountryCnt].country_name, countryName, strlen(countryName));
                        validCountryCnt++;
                        //printf("%s, %s\n", countryName, demodInfo.ts_country_table[j].country_name);
                    }
                }

                TsiSkipBuffer(pHTsd->act_demod_id, true);

                // Create thread 
                tsiThreadParam.handle = (uint32_t)pHTsd;
                tsiThreadParam.idx    = i;
#if !(_MSC_VER)
                pthread_attr_init(&attr);
                attr.schedparam.sched_priority = 2;
#endif
                rc = pthread_create(&g_demodTsCtxt.demodUserInfo[i].demod_threads, &attr, TsiBufferThread, (void *)&tsiThreadParam);
                if( rc )
                {
                    itp_msg_ex(ZONE_ERROR, "ERROR; pthread_create() fail %d\n", rc);
                }
                usleep(5000);
            }
        }
        
        demodInfo.ts_country_cnt = validCountryCnt;
        
        // send data size to clint
        demodInfo.blockSize = (1 * 1024 * 1024); //(1 << 20); // 2MB
        
        g_demodTsCtxt.refCnt = 1;
    }
    else
        g_demodTsCtxt.refCnt++;
    
}

static void DemodTerminate(void)
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
            // destroy tsd handle
            tsd_DestroyHandle(&g_demodTsCtxt.demodUserInfo[i].pHTsd); 
            
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


static int DemodIoctl(int file, unsigned long request, void* ptr, void* info)
{
    int             result = 0;
    uint32_t        demod_idx = (file & 0xFF);
    TSD_HANDLE      *pHTsd = 0;
    struct DEMODUSERINFO *pDemodUserInfo = 0;

    pDemodUserInfo = &g_demodTsCtxt.demodUserInfo[demod_idx];
    
    _mutex_lock(pDemodUserInfo->demod_mutex);

    pHTsd = pDemodUserInfo->pHTsd;

    switch (request)
    {
    case ITP_IOCTL_TS_HANDLE_MERGE:
        {
            int k;

            for(k = 1; k < ITP_MAX_DEMOD_SUPPORT_CNT; k++ )
            {
                tsd_Merge_Service_Info(
                        g_demodTsCtxt.demodUserInfo[0].pHTsd,
                        g_demodTsCtxt.demodUserInfo[k].pHTsd,
                        0);
            }

            for(k = 1; k < ITP_MAX_DEMOD_SUPPORT_CNT; k++ )
            {
                tsd_Merge_Service_Info(
                        g_demodTsCtxt.demodUserInfo[k].pHTsd,
                        g_demodTsCtxt.demodUserInfo[0].pHTsd,
                        0);
            }
        }
        break;
    	
    case ITP_IOCTL_SCAN_CHANNEL:
        {
            TSD_SCAN_PARAM	*pUserScanParm = (TSD_SCAN_PARAM*)ptr;
            TSD_SCAN_PARAM	scanParm = {0};

            if( !pUserScanParm )	break;

            itp_msg_ex(1, ".... demdoe id = %d\n", demod_idx);
            
            memcpy(&scanParm, pUserScanParm, sizeof(TSD_SCAN_PARAM));

            if( pHTsd )
            {
                /** 
                 * pause tsi buffer assess in demod_thread
                 * When scan channel, tsi buffer can't be accessed by other.
                 **/
                TsiStopCache(demod_idx); 

                tsd_Control(pHTsd, TSD_CTRL_SET_DEMOD_STATUS, (uint32_t*)TSD_DEMOD_STATUS_RUNNING, 0);
                _scan_channel(demod_idx, -1, &scanParm); // auto scan

                // need to merge service info in two pHTsd
                // To Do:
                
                // start tsi buffer assess in demod_thread 
                TsiStartCache(demod_idx);
            }
        }

        #if 0// test+
        {      
            TSD_SRVC_USER_INFO  *pSrvcUserInfo = 0;
            TSD_TXT_CONV        txtConv = {0};
            int                 j;
            char                utf8_txt[256] = {0};

            unsigned int bytes_used = 0;
            
            demodInfo.ts_service_cnt = (g_demodTsCtxt.pHTsd->totalSrvc > ITP_MAX_TS_SERVICE) ? ITP_MAX_TS_SERVICE : g_demodTsCtxt.pHTsd->totalSrvc;
            for(j = 0; j < demodInfo.ts_service_cnt; j++)
            {
                tsd_Get_ServiceInfo(g_demodTsCtxt.pHTsd, j, &pSrvcUserInfo, 0);
                demodInfo.ts_service_table[j].service_order_index = j;

                txtConv.dvbTxt          = pSrvcUserInfo->serviceName;
                txtConv.dvbTxtLength    = pSrvcUserInfo->nameSize;
                txtConv.utf16Txt        = demodInfo.ts_service_table[j].service_name;
                txtConv.utf16TxtLength  = ITP_MAX_TS_SERVICE_NAME_SIZE;
                tsd_Conv_Text(g_demodTsCtxt.pHTsd, &txtConv, 0);
                utf16le_to_utf8(demodInfo.ts_service_table[j].service_name, utf8_txt, 256, &bytes_used);
                printf("srvcName(%d-th): %s\n", j, utf8_txt);
            }
        }
        #endif // test-        

        break;

    case ITP_IOCTL_INIT:
        DemodInit();
        return result;
        break;

    case ITP_IOCTL_TS_SUSPEND_FIRE:
        if( ptr )
        {
            tsd_Control(pHTsd, TSD_CTRL_DEMOD_SUSPEND_FIRE, (void*)true, 0);
        }
        else
        {
            tsd_Control(pHTsd, TSD_CTRL_DEMOD_SUSPEND_FIRE, (void*)false, 0);
        }
        break;
        
    case ITP_IOCTL_SET_TS_SUSPEND:
        if( ptr )
        {
            // enter suspend
            TsiStopCache(demod_idx);
            tsd_Control(pHTsd, TSD_CTRL_SET_DEMOD_SUSPEND_MODE, (void*)true, 0);
        }
        else
        {
            // wake up
            tsd_Control(pHTsd, TSD_CTRL_SET_DEMOD_SUSPEND_MODE, (void*)false, 0);
            TsiStartCache(demod_idx);
        }
        break;
        
    case ITP_IOCTL_EXIT:
        DemodTerminate();
        break;

    case ITP_IOCTL_RECEIVE_TS:
        tsd_Control(pHTsd, TSD_CTRL_ENABLE_TSI, 0, 0);
        TsiStartCache(demod_idx);
        break;
        
    case ITP_IOCTL_SKIP_TS:
        tsd_Control(pHTsd, TSD_CTRL_DISABLE_TSI, 0, 0); 
        TsiStopCache(demod_idx);
        break;

    case ITP_IOCTL_GET_COUNRTY_NAME: // combine with ITP_IOCTL_GET_INFO ??
        *(ITPDemodInfo**)ptr = &demodInfo;
        break;
    
    case ITP_IOCTL_GET_TS_HANDLE:
        if( ptr )   *(TSD_HANDLE**)ptr = pHTsd;
        break;
        
    case ITP_IOCTL_GET_INFO:
        {
            TSD_SRVC_USER_INFO  *pSrvcUserInfo = 0;
            TSD_TXT_CONV        txtConv = {0};
            int                 j;

            demodInfo.ts_service_cnt = (pHTsd->totalSrvc > ITP_MAX_TS_SERVICE) ? ITP_MAX_TS_SERVICE : pHTsd->totalSrvc;
            for(j = 0; j < demodInfo.ts_service_cnt; j++)
            {
                tsd_Get_ServiceInfo(pHTsd, j, &pSrvcUserInfo, 0);
                if( !pSrvcUserInfo )    
                {
                    printf("pSrvcUserInfo = 0x%x\n", pSrvcUserInfo);
                    continue;
                }
                
                demodInfo.ts_service_table[j].service_order_index = j;
                demodInfo.ts_service_table[j].totalSoundTracks    = (int)pSrvcUserInfo->audioCount;
                demodInfo.ts_service_table[j].bVideoService       = pSrvcUserInfo->bTV;
                
                txtConv.dvbTxt          = pSrvcUserInfo->serviceName;
                txtConv.dvbTxtLength    = pSrvcUserInfo->nameSize;
                txtConv.utf16Txt        = demodInfo.ts_service_table[j].service_name;
                txtConv.utf16TxtLength  = ITP_MAX_TS_SERVICE_NAME_SIZE;
                tsd_Conv_Text(pHTsd, &txtConv, 0);
                #if 0// test+
                {
                    char            utf8_txt[256] = {0};
                    unsigned int    bytes_used = 0;
                    utf16le_to_utf8(txtConv.utf16Txt, utf8_txt, 256, &bytes_used);
                    printf("-srvcName(%d-th): %s\n", j, utf8_txt);
                }
                #endif// test-
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
//				  Public Function Definition
//=============================================================================
const ITPDevice itpDeviceDemod =
{
    ":demod",
    DemodOpen,
    DemodClose,
    DemodRead,
    itpWriteDefault,
    itpLseekDefault,
    DemodIoctl,
    NULL
};

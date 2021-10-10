#ifndef __TS_AIRFILE_H_DHJN00OO_8N1X_6GVO_GGQT_VXFFDQLSMV4V__
#define __TS_AIRFILE_H_DHJN00OO_8N1X_6GVO_GGQT_VXFFDQLSMV4V__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ts_airfile_cfg.h"
#include "ts_airfile_def.h"
#include "ite_ts_airfile.h"
//=============================================================================
//				  Constant Definition
//=============================================================================
#if (CFG_DEMOD_OMEGA)
    #define TSAF_ACTION_DEMOD_ID     TSAF_DEMOD_OMEGA
#elif (CFG_DEMOD_IT9135)
    #define TSAF_ACTION_DEMOD_ID     TSAF_DEMOD_IT9135
#elif (CFG_DEMOD_IT9137)
    #define TSAF_ACTION_DEMOD_ID     TSAF_DEMOD_IT9137
#elif (CFG_DEMOD_IT9137_USB)
    #define TSAF_ACTION_DEMOD_ID     TSAF_DEMOD_IT9137_USB
#else
    #define TSAF_ACTION_DEMOD_ID     TSAF_DEMOD_UNKNOW
#endif

/**
 * ts air file type (by produce)
 **/
typedef enum _TS_AIRFILE_TYPE_ID_TAG
{
    TS_AIRFILE_ID_UNKNOW     = 0,
    TS_AIRFILE_ID_NETTV,
    TS_AIRFILE_ID_GATEWAY,
    TS_AIRFILE_ID_SPLIT_GATEWAY,
    TS_AIRFILE_ID_AGGRE_GATEWAY

}TS_AIRFILE_TYPE_ID;

//=============================================================================
//				  Macro Definition
//=============================================================================
typedef uint32_t (*pFunc_inSrc)(void *argv_0, void *argv_1, void *argv_2, void *argv_3);
typedef void* (*THREAD_ROUTINE_FUNC)(void*);
typedef uint32_t (*GET_TS_STREAM_FUNC)(void *handler, uint8_t **sampleAddr, uint32_t *sampleLength, void *extraData);

//=============================================================================
//				  Structure Definition
//=============================================================================
/**
 * cache buffer mgr, for maintaining read/write pointer
 **/
typedef struct _CACHE_BUF_MGR_TAG
{
    //bool                bSkip;

    // cache thread layer
    pthread_mutex_t     cacheThreadStopMutex;
    bool                bStartCacheThread;

    // control cache
    pthread_mutex_t     cacheStopMutex;
    bool                bStartCache;

    // cache buffer access
    pthread_mutex_t     cacheBufAccessMutex;
    uint32_t            cacheBuf_RPtr;
    uint32_t            cacheBuf_WPtr;
    uint8_t             *pCacheBuf;

    struct timeval      lastT;

    uint32_t            maxCacheBufSize;
}CACHE_BUF_MGR;

/**
 * initialize parameters
 **/
typedef struct _TSAF_INIT_PARAM_TAG
{
    pthread_t           thread;
    THREAD_ROUTINE_FUNC threadFunc;

    GET_TS_STREAM_FUNC  getTsStreamFunc;
    uint32_t            demod_index;
    void                *extra_handle;

    // cache buffer mgr
    CACHE_BUF_MGR       *pCacheBufMgr;

    // the time gap of current get_sample to next get_sample
    uint32_t            waiting_time; // usec

}TSAF_INIT_PARAM;

/**
 * cache buffer control info
 **/
struct _TSAF_CRTL_INFO_TAG;
typedef struct _TSAF_CACHE_INFO_TAG
{
    void*    (*cache_BufThread)(void *args);
    int      (*cache_StopThread)(uint32_t index, struct _TSAF_CRTL_INFO_TAG *pTsafCtrlInfo);
    int      (*cache_Stop)(uint32_t index, struct _TSAF_CRTL_INFO_TAG *pTsafCtrlInfo);
    int      (*cache_Start)(uint32_t index, struct _TSAF_CRTL_INFO_TAG *pTsafCtrlInfo);
    int      (*cache_GetData)(uint32_t index, uint8_t *buffer, uint32_t bufferLength,
                              uint32_t *pRealLength, struct _TSAF_CRTL_INFO_TAG *pTsafCtrlInfo);

    void     *extraData;

}TSAF_CACHE_INFO;

/**
 * single device control info
 **/
typedef struct _TSAF_CRTL_INFO_TAG
{
    pthread_t           thread;
    uint32_t            demod_id;
    void                *privData;

    void                *extra_handle;

    int                 ts_country_cnt;
    TSAF_COUNTRY_INFO   ts_country_table[TSAF_COUNTRY_CNT-1];

    TSAF_SCAN_PARAM     scanParam;

    TSAF_CACHE_INFO     cacheInfo;

    // for usb demod
    bool                bUsbDefaultAccess;

    // cache buffer mgr
    CACHE_BUF_MGR       cacheBufMgr;

    // to integrate with aggregation module
    uint32_t            tsi_index;

}TSAF_CRTL_INFO;

/**
 * type descriptor
 **/
typedef struct _TS_AIRFILE_DESC_TAG
{
    const char        *name;

    struct _TS_AIRFILE_DESC_TAG  *next;
    TS_AIRFILE_TYPE_ID           id;

    void        *privInfo;

    // operator
    TSAF_ERR     (*taf_init)(TSAF_CRTL_INFO *pInfo, TSAF_INIT_PARAM *pInitParam, void *extraData);
    TSAF_ERR     (*taf_deinit)(TSAF_CRTL_INFO *pInfo, void *extraData);

    TSAF_ERR     (*taf_open)(TSAF_CRTL_INFO *pInfo, uint32_t service_index, void *extraData);
    TSAF_ERR     (*taf_close)(TSAF_CRTL_INFO *pInfo, void *extraData);
    int          (*taf_read)(TSAF_CRTL_INFO *pInfo, uint8_t *pDstBuf, int length, uint32_t *pRealLength, void *extraData);
    TSAF_ERR     (*taf_write)(TSAF_CRTL_INFO *pInfo, void *extraData);

    TSAF_ERR     (*taf_get_inSrc_func)(TSAF_CRTL_INFO *pInfo, pFunc_inSrc *ppFunc, void *extraData);

    //TSAF_ERR     (*taf_changeService)(TSAF_CRTL_INFO *pInfo, void *extraData);
    TSAF_ERR     (*taf_scanFreq)(TSAF_CRTL_INFO *pInfo, void *extraData);
    TSAF_ERR     (*taf_control)(TSAF_CRTL_INFO *pInfo, uint32_t cmd, uint32_t *value, void *extraData);


}TS_AIRFILE_DESC;

//=============================================================================
//				  Global Data Definition
//=============================================================================


//=============================================================================
//				  Private Function Definition
//=============================================================================


//=============================================================================
//				  Public Function Definition
//=============================================================================


#ifdef __cplusplus
}
#endif

#endif

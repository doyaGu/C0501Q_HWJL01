
#include <unistd.h>
#include <pthread.h>
#include "ts_airfile.h"

#if (CONFIG_TS_AIRFILE_DESC_SPLIT_GATEWAY_DESC)
#include "ite/ite_risc_ts_demux.h"
#include "iniparser/iniparser.h"
#include "ts_demuxer/ite_ts_demuxer.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define ENABLE_FILE_SIMULATION       0
// #define MAX_DEMOD_SUPPORT_CNT        CFG_DEMOD_SUPPORT_COUNT

#define USB_DEMOD_ACCESS_UNIT           (87 * 188)
#define USB_DEMOD_BUF_SIZE              (USB_DEMOD_ACCESS_UNIT * 160)  // need aligement of USB_DEMOD_ACCESS_UNIT

#define MAX_SUPPORT_CACHE_CNT           CFG_DEMOD_SUPPORT_COUNT

#define TSI_CACHE_BUFFER_SIZE           (2 * 1024 * 1024)
#define CACHE_GET_SAMPLE_TIME_DELAY     10000

#define CACHE_THREAD_TIMER_GAP              30

typedef enum USB_FILL_BUF_STATE_TAG
{
    USB_FILL_BUF_STOP = 0,
    USB_FILL_BUF_START,
    USB_FILL_BUF_STOPPING,
} USB_FILL_BUF_STAGE;

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct _DEMOD_BASE_INFO_TAG
{
    const char          *name;
    bool                bUsed;

    const bool          bUsb_demod;

    const uint32_t      tsi_index;

    uint32_t            startFreq;
    uint32_t            bandwidth;
    const uint32_t      channlTotalCnt;
}DEMOD_BASE_INFO;

typedef struct _USB_TS_BUF_MGR_TAG
{
    uint32_t            demod_index;

    pthread_mutex_t     usbBufMutex;
    bool                bFirstSample;
    bool                bFirstSampleStop;
    uint32_t            usbBuf_RPtr;
    uint32_t            usbBuf_WPtr;
    uint8_t             *pUsbTsBuf;
    uint32_t            bufferSize;
    USB_FILL_BUF_STAGE  bufState;
    CHANNEL_INFO*       ptChannelInfo;
} USB_TS_BUF_MGR;

typedef struct _TSI_TS_BUF_MGR_TAG
{
    uint32_t            demod_index;

    pthread_mutex_t     tsiBufMutex;
    uint32_t            tsiBuf_RPtr;
    uint32_t            tsiBuf_WPtr;
    uint8_t             *pTsiTsBuf;
    uint32_t            bufferSize;
    bool                bStartCacheThread;
    struct timeval      lastT;
    CHANNEL_INFO*       ptChannelInfo;
} TSI_TS_BUF_MGR;

//=============================================================================
//                  Global Data Definition
//=============================================================================
static DEMOD_BASE_INFO  g_demod_base_info[] =
{
    {"InputStream0", false, false, 0,    611000, 6000, 3},
    {"InputStream1", false, false, 1,    611000, 6000, 3},
    {"InputStream2", false, true,  (-1), 611000, 6000, 3},
    {"InputStream3", false, true,  (-1), 611000, 6000, 3},
};

//=============================================================================
//                  Private Function Definition
//=============================================================================
#if _MSC_VER && (ENABLE_FILE_SIMULATION)
#include <stdlib.h>
typedef struct _TS_SRC_INPUT_TAG
{
    FILE        *f_ts;
    uint32_t    fileSize;
    uint32_t    freq;

}TS_SRC_INPUT;

FILE                    *f_ts_scan = 0;
uint32_t                ts_scan_file_size = 0;
static TS_SRC_INPUT     ts_src_input[MAX_DEMOD_SUPPORT_CNT] = {0};
static void
_scan_channel(
    TSD_HANDLE          *pHTsd,
    uint32_t            demod_index)
{
    int                 i;
    TSD_SCAN_PARAM      scanParm = {0};
    static char    *ts_name[MAX_DEMOD_SUPPORT_CNT] ={
        {"A:/CTV_MyLife_tw_0001.ts"},
        #if (MAX_DEMOD_SUPPORT_CNT > 1)
        {"A:/CTV_MyLife_tw_0001.ts"},
        #endif
        #if (MAX_DEMOD_SUPPORT_CNT > 2)
        {"A:/CTV_MyLife_tw_0001.ts"},
        #endif
        #if (MAX_DEMOD_SUPPORT_CNT > 3)
        {"A:/CTV_MyLife_tw_0001.ts"},
        #endif
    };

    if( !(ts_src_input[demod_index].f_ts = fopen(ts_name[demod_index], "rb")) )
    {
        printf("open %s fail !\n", ts_name[demod_index]);
        while(1);
    }

    fseek(ts_src_input[demod_index].f_ts, 0, SEEK_END);
    ts_src_input[demod_index].fileSize = ftell(ts_src_input[demod_index].f_ts);
    fseek(ts_src_input[demod_index].f_ts, 0, SEEK_SET);

    f_ts_scan = ts_src_input[demod_index].f_ts;
    ts_scan_file_size = ts_src_input[demod_index].fileSize;

    scanParm.scanFrequency = 533 + demod_index * 30;
    scanParm.bandwidth     = 6000;
    scanParm.countryId     = TSD_COUNTRY_TAIWAN;

    ts_src_input[demod_index].freq = scanParm.scanFrequency;

    tsd_Scan_Channel(pHTsd, TSD_FREQ_SCAN_MANUAL, &scanParm, 1);
    return;
}
#endif

//=============================================================================
//                  Private Function Definition
//=============================================================================

#define IS_THREAD_RUNNING(index, pBufMgr)     ((pBufMgr)->bStartCacheThread)
#define IS_START_CACHE(index, pBufMgr)        ((pBufMgr)->bStartCache)

static uint32_t
_get_Ts_Stream(
    void        *handle,
    uint8_t     **sampleAddr,
    uint32_t    *sampleLength,
    void        *extraData)
{
    uint32_t    result = 0;
    TSD_HANDLE  *pHTsd = (TSD_HANDLE*)handle;

    TSD_SAMPLE_INFO sampleInfo = {0};

    sampleInfo.bShareBuf = true;
    tsd_Get_Sample(pHTsd, TSD_SAMPLE_TS, &sampleInfo, 0);

    if( sampleAddr )     *sampleAddr    = sampleInfo.sampleAddr;
    if( sampleLength )  {*sampleLength = result = sampleInfo.sampleSize;}

    return result;
}

static void*
_tsi_cache_BufThread(
    void *args)
{
    GET_TS_STREAM_FUNC  getTsStreamFunc = 0;
    TSD_HANDLE          *pHTsd = 0;
    TSAF_CRTL_INFO*     ptCtrlInfo = (TSAF_CRTL_INFO*)args;
    TSI_TS_BUF_MGR      *pTsiCacheBufMgr = (TSI_TS_BUF_MGR*)ptCtrlInfo->privData;

    //------------------------------------------
    // init parameters
    _tsaf_mutex_init(TSAF_MSG_TYPE_TRACE_CACHE_BUF, pTsiCacheBufMgr->tsiBufMutex);

    pTsiCacheBufMgr->bStartCacheThread = true;

    pHTsd = (TSD_HANDLE*)ptCtrlInfo->extra_handle;

    if( !(pTsiCacheBufMgr->pTsiTsBuf = (uint8_t*)malloc(TSI_CACHE_BUFFER_SIZE)) )
    {
        tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " err ! tsi ts cache (size=0x%x) create is FAIL !\n",
                    TSI_CACHE_BUFFER_SIZE);
        return;
    }
    tsaf_get_clock(&pTsiCacheBufMgr->lastT);

    //--------------------------
    // thread running
    while(pTsiCacheBufMgr->bStartCacheThread)
    {
        if( tsaf_get_duration(&pTsiCacheBufMgr->lastT) > CACHE_THREAD_TIMER_GAP )
        {
            struct timeval timeout;
            uint8_t     *pSampleAddr = 0;
            uint32_t    sampleSize = 0;

            tsaf_get_clock(&pTsiCacheBufMgr->lastT);
            tsaf_get_clock(&timeout);

            // cache buffer access
            while( pTsiCacheBufMgr->bStartCacheThread)
            {
                if( tsaf_get_duration(&timeout) > (CACHE_GET_SAMPLE_TIME_DELAY << 2) )
                {
                    //tsaf_msg(1, " cache (%d-th) buffer: No data input (%d ms) !\n", pTsiCacheBufMgr->demod_index, tsaf_get_duration(&timeout));
                }

                //--------------------------------
                // get ts sample
                _get_Ts_Stream((TSD_HANDLE*)ptCtrlInfo->extra_handle, &pSampleAddr, &sampleSize, 0);

                //----------------------------------
                // Copy to cache buffer
                if( (int)sampleSize > 0 )
                {
                    uint32_t    remainderSize = 0;
                    CHANNEL_INFO* ptChannelInfo = (CHANNEL_INFO*) pTsiCacheBufMgr->ptChannelInfo;
                    tsaf_get_clock(&timeout);

                    //tsaf_msg(1, "sampleSize[%d]=%d\n", cache_index, sampleSize);
                    remainderSize = (TSI_CACHE_BUFFER_SIZE - pTsiCacheBufMgr->tsiBuf_WPtr);

                    if( sampleSize > remainderSize )
                    {
                        // swap copy
                        memcpy(pTsiCacheBufMgr->pTsiTsBuf + pTsiCacheBufMgr->tsiBuf_WPtr, pSampleAddr, remainderSize);
                        memcpy(pTsiCacheBufMgr->pTsiTsBuf, pSampleAddr + remainderSize, sampleSize - remainderSize);

                        _tsaf_mutex_lock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, pTsiCacheBufMgr->tsiBufMutex);
                        pTsiCacheBufMgr->tsiBuf_WPtr = (sampleSize - remainderSize);
                        _tsaf_mutex_unlock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, pTsiCacheBufMgr->tsiBufMutex);

                        tsaf_msg(0, "..... cache (%d-th) buf WPtr=0\n", pTsiCacheBufMgr->demod_index);
                    }
                    else
                    {
                        memcpy(pTsiCacheBufMgr->pTsiTsBuf + pTsiCacheBufMgr->tsiBuf_WPtr, pSampleAddr, sampleSize);

                        _tsaf_mutex_lock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, pTsiCacheBufMgr->tsiBufMutex);
                        pTsiCacheBufMgr->tsiBuf_WPtr += sampleSize;
                        _tsaf_mutex_unlock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, pTsiCacheBufMgr->tsiBufMutex);
                    }
                    if (ptChannelInfo)
                    {
                        ptChannelInfo->tsBufferWriteIdx = pTsiCacheBufMgr->tsiBuf_WPtr;
                        ithFlushMemBuffer();

                    }
                }

                usleep(CACHE_GET_SAMPLE_TIME_DELAY);
            }
        }
        else
        {
            usleep(20000);
        }
    }

    //--------------------------
    // reset global parameters
    if( pTsiCacheBufMgr->pTsiTsBuf )
    {
        free(pTsiCacheBufMgr->pTsiTsBuf);
        pTsiCacheBufMgr->pTsiTsBuf = NULL;
    }

    _tsaf_trace_leave(TSAF_MSG_TYPE_TRACE_CACHE_BUF);

    _tsaf_mutex_deinit(TSAF_MSG_TYPE_TRACE_CACHE_BUF, pTsiCacheBufMgr->tsiBufMutex);

    memset(pTsiCacheBufMgr, 0x0, sizeof(CACHE_BUF_MGR));
    _tsaf_thread_exit(NULL);
    return 0;
}

static int
_tsi_cache_StopThread(
    uint32_t            index,
    TSAF_CRTL_INFO      *pTsafCtrlInfo)
{
    TSI_TS_BUF_MGR   *ptTsiTsBufMgr = (TSI_TS_BUF_MGR*)pTsafCtrlInfo->privData;
    _tsaf_trace_enter(TSAF_MSG_TYPE_TRACE_CACHE_BUF, "\n");

    ptTsiTsBufMgr->bStartCacheThread = false;

    _tsaf_trace_leave(TSAF_MSG_TYPE_TRACE_CACHE_BUF);

    return 0;
}

static int
_tsi_cache_Stop(
    uint32_t            index,
    TSAF_CRTL_INFO      *pTsafCtrlInfo)
{
    _tsaf_trace_enter(TSAF_MSG_TYPE_TRACE_CACHE_BUF, "\n");

    _tsaf_trace_leave(TSAF_MSG_TYPE_TRACE_CACHE_BUF);

    return 0;
}

static int
_tsi_cache_Start(
    uint32_t            index,
    TSAF_CRTL_INFO      *pTsafCtrlInfo)
{
    _tsaf_trace_enter(TSAF_MSG_TYPE_TRACE_CACHE_BUF, "\n");

    tsd_Control((TSD_HANDLE*)pTsafCtrlInfo->extra_handle, TSD_CTRL_ENABLE_TSI, 0, 0);

    _tsaf_trace_leave(TSAF_MSG_TYPE_TRACE_CACHE_BUF);
    return 0;
}

static uint32_t
_tsi_cache_GetDataSize(
    uint32_t            index,
    TSAF_CRTL_INFO      *pTsafCtrlInfo)
{
    uint32_t    validSize = 0;
    uint32_t    maxCacheBufSize = TSI_CACHE_BUFFER_SIZE;
    TSI_TS_BUF_MGR  *pTsiCacheBufMgr = (TSI_TS_BUF_MGR*)pTsafCtrlInfo->privData;

    _tsaf_mutex_lock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, pTsiCacheBufMgr->tsiBufMutex);

    if( pTsiCacheBufMgr->tsiBuf_WPtr < pTsiCacheBufMgr->tsiBuf_RPtr )
    {
        validSize = (maxCacheBufSize - pTsiCacheBufMgr->tsiBuf_RPtr) + pTsiCacheBufMgr->tsiBuf_WPtr;
    }
    else
    {
        validSize = pTsiCacheBufMgr->tsiBuf_WPtr - pTsiCacheBufMgr->tsiBuf_RPtr;
    }

    _tsaf_mutex_unlock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, pTsiCacheBufMgr->tsiBufMutex);

    return validSize;
}

static int
_tsi_cache_NoBlockingGetData(
    uint32_t        index,
    uint8_t         *buffer,
    uint32_t        bufferLength,
    uint32_t        *pRealLength,
    TSAF_CRTL_INFO  *pTsafCtrlInfo)
{
    int         ret = (-1);
    uint32_t    ReadRemainderSize = 0;
    uint32_t    maxCacheBufSize = pTsafCtrlInfo->cacheBufMgr.maxCacheBufSize;

    if( pRealLength )   *pRealLength = 0;

    if( _tsi_cache_GetDataSize(index, pTsafCtrlInfo) < bufferLength )
    {
        return ret;
    }

    ReadRemainderSize = (maxCacheBufSize - pTsafCtrlInfo->cacheBufMgr.cacheBuf_RPtr);

    if( ReadRemainderSize < bufferLength )
    {
        // swap copy
        memcpy(buffer, pTsafCtrlInfo->cacheBufMgr.pCacheBuf + pTsafCtrlInfo->cacheBufMgr.cacheBuf_RPtr, ReadRemainderSize);
        memcpy(buffer + ReadRemainderSize, pTsafCtrlInfo->cacheBufMgr.pCacheBuf, bufferLength - ReadRemainderSize);
        pTsafCtrlInfo->cacheBufMgr.cacheBuf_RPtr = (bufferLength - ReadRemainderSize);
    }
    else
    {
        memcpy(buffer, pTsafCtrlInfo->cacheBufMgr.pCacheBuf + pTsafCtrlInfo->cacheBufMgr.cacheBuf_RPtr, bufferLength);
        pTsafCtrlInfo->cacheBufMgr.cacheBuf_RPtr += bufferLength;
    }

    if( pRealLength )   *pRealLength = bufferLength;

    ret = 0;
    return ret;
}



#if defined(CFG_USB_DEMOD_ENABLE) && !defined(_MSC_VER)
////////////////////////////////////////
// Usb demod module access
/////////////////////////////////////////
static inline int
_usb_get_cacheSize(
    USB_TS_BUF_MGR  *ptUsbTsBufMgr)
{
    uint32_t            writePtr = 0;
    int                 cacheSize = 0;

    //_tsaf_mutex_lock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, ptUsbTsBufMgr->usbBufMutex);
    writePtr = ptUsbTsBufMgr->usbBuf_WPtr;
    //_tsaf_mutex_unlock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, ptUsbTsBufMgr->usbBufMutex);

    if( ptUsbTsBufMgr->usbBuf_RPtr > writePtr )
    {
        cacheSize = (int) (((uint32_t)(ptUsbTsBufMgr->pUsbTsBuf) + ptUsbTsBufMgr->bufferSize)
                           - ptUsbTsBufMgr->usbBuf_RPtr
                           + (writePtr - (uint32_t) ptUsbTsBufMgr->pUsbTsBuf));
    }
    else
    {
        cacheSize = writePtr - ptUsbTsBufMgr->usbBuf_RPtr;
    }
    return cacheSize;
}

struct timeval testTime = { 0 };
static void
_UsbDemod_UpdateBuffer(
    uint32_t    index,
    void        *pTsafCtrlInfo)
{
    TSD_SAMPLE_INFO         tSample = {0};
    USB_FILL_BUF_STAGE      curState = USB_FILL_BUF_STOP;
    TSAF_CRTL_INFO          *pCtrlInfo = (TSAF_CRTL_INFO*)pTsafCtrlInfo;
    USB_TS_BUF_MGR          *ptUsbTsBufMgr = 0;
    CHANNEL_INFO* ptChannelInfo = 0;
    struct timeval curTime = { 0 };
    uint32_t duration_time = 0;
    ptUsbTsBufMgr = (USB_TS_BUF_MGR*)pCtrlInfo->privData;
    ptChannelInfo = ptUsbTsBufMgr->ptChannelInfo;
    //_tsaf_mutex_lock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, ptUsbTsBufMgr->usbBufMutex);
    curState = ptUsbTsBufMgr->bufState;
    //_tsaf_mutex_unlock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, ptUsbTsBufMgr->usbBufMutex);

    switch( curState )
    {
        case USB_FILL_BUF_STOPPING:
            ptUsbTsBufMgr->bufState = USB_FILL_BUF_STOP;
            break;

        case USB_FILL_BUF_STOP:
            if( ptUsbTsBufMgr->pUsbTsBuf )      free(ptUsbTsBufMgr->pUsbTsBuf);

            _tsaf_mutex_deinit(TSAF_MSG_TYPE_TRACE_CACHE_BUF, ptUsbTsBufMgr->usbBufMutex);

            free(ptUsbTsBufMgr);
            pCtrlInfo->privData = 0;
            break;

        default:
            if (ptChannelInfo)
            {
                ptChannelInfo->tsBufferWriteIdx = ptUsbTsBufMgr->usbBuf_WPtr;
                ithFlushMemBuffer();
                //printf("usb widx: 0x%X, ridx: 0x%X\n", ptChannelInfo->tsBufferWriteIdx, ptChannelInfo->tsBufferReadIdx);
            }

            if( ptUsbTsBufMgr->bFirstSample == true )
            {
                ptUsbTsBufMgr->usbBuf_WPtr = 0;
                ptUsbTsBufMgr->bFirstSample = false;
            }
            else
            {
                ptUsbTsBufMgr->usbBuf_WPtr += USB_DEMOD_ACCESS_UNIT;
            }

            if( ptUsbTsBufMgr->usbBuf_WPtr == ptUsbTsBufMgr->bufferSize )
            {
                ptUsbTsBufMgr->usbBuf_WPtr = 0;
            }
#if 0
            if (testTime.tv_sec)
            {
                gettimeofday(&curTime, NULL);
                    duration_time = (curTime.tv_sec - testTime.tv_sec) * 1000;      // sec to ms
                    duration_time += ((curTime.tv_usec - testTime.tv_usec) / 1000); // us to ms
            }
            gettimeofday(&testTime, NULL);
#endif
            // Update buffer index for RISC Pes decoding.
            //if (ptChannelInfo)
            //{
            //    ptChannelInfo->tsBufferWriteIdx = ptUsbTsBufMgr->usbBuf_WPtr;
            //    ithFlushMemBuffer();
                //printf("usb widx: 0x%X, ridx: 0x%X\n", ptChannelInfo->tsBufferWriteIdx, ptChannelInfo->tsBufferReadIdx);
            //}

            tSample.bShareBuf         = true;
            tSample.disableMutex      = TSD_DISABLE_MUTEX;
            tSample.bUsbDefaultAccess = pCtrlInfo->bUsbDefaultAccess; // default follow: copy data out
            tSample.sampleAddr        = &ptUsbTsBufMgr->pUsbTsBuf[ptUsbTsBufMgr->usbBuf_WPtr];
            tSample.sampleSize        = USB_DEMOD_ACCESS_UNIT;
            tSample.pfCallback        = _UsbDemod_UpdateBuffer;
            tSample.pCtrlParam        = pCtrlInfo;

            tsd_Get_Sample((TSD_HANDLE*)pCtrlInfo->extra_handle, TSD_SAMPLE_TS, &tSample, 0);
            break;
    }
}

static bool
_UsbDemod_IsFillingBuffer(
    USB_TS_BUF_MGR      *ptUsbTsBufMgr)
{
    bool    bResult = false;

    _tsaf_mutex_lock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, ptUsbTsBufMgr->usbBufMutex);
    if( USB_FILL_BUF_START == ptUsbTsBufMgr->bufState )
    {
        bResult = true;
    }
    _tsaf_mutex_unlock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, ptUsbTsBufMgr->usbBufMutex);
    return bResult;
}

static uint32_t
_UsbDemod_Rx_deInit(
    TSD_TS_RX_INFO      *pRxInfo,
    void                *extraData)
{
    int                 rst = 0;
    TSAF_CRTL_INFO      *pCtrlInfo = (TSAF_CRTL_INFO*)(pRxInfo->privData.usb_demod_info.pDevCtrlInfo);
    USB_TS_BUF_MGR      *ptUsbTsBufMgr = 0;
    ptUsbTsBufMgr = (USB_TS_BUF_MGR*)pCtrlInfo->privData;

    do{
        if( !ptUsbTsBufMgr || !pCtrlInfo )
        {
            tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " err ! usb ts buffer mgr Null Pointer\n");
            break;
        }

        ptUsbTsBufMgr->bufState = USB_FILL_BUF_STOPPING;

    }while(0);

    return rst;
}

static uint32_t
_UsbDemod_Rx_Init(
    TSD_TS_RX_INFO      *pRxInfo,
    void                *extraData)
{
    int                 rst = 0;
    TSAF_CRTL_INFO      *pCtrlInfo = (TSAF_CRTL_INFO*)(pRxInfo->privData.usb_demod_info.pDevCtrlInfo);
    USB_TS_BUF_MGR      *ptUsbTsBufMgr = 0;

    ptUsbTsBufMgr = (USB_TS_BUF_MGR*)pCtrlInfo->privData;

    do{
        if( !ptUsbTsBufMgr || !pCtrlInfo )
        {
            tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " err ! usb ts buffer mgr Null Pointer\n");
            break;
        }

        pCtrlInfo->cacheInfo.extraData = (void*)pRxInfo;

        ptUsbTsBufMgr->bufferSize = USB_DEMOD_BUF_SIZE;
        if( !(ptUsbTsBufMgr->pUsbTsBuf = (uint8_t*)malloc(ptUsbTsBufMgr->bufferSize)) )
        {
            tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " err ! usb ts cache (size=0x%x) create is FAIL !\n",
                        ptUsbTsBufMgr->bufferSize);
            break;
        }
        _tsaf_mutex_init(TSAF_MSG_TYPE_TRACE_CACHE_BUF, ptUsbTsBufMgr->usbBufMutex);

        return rst;
    }while(0);

    _UsbDemod_Rx_deInit(pRxInfo, extraData);
    return (-1);
}

static uint32_t
_UsbDemod_Rx_TurnOn(
    TSD_TS_RX_INFO      *pRxInfo,
    void                *extraData)
{
    int                 rst = 0;
    TSAF_CRTL_INFO      *pCtrlInfo = (TSAF_CRTL_INFO*)(pRxInfo->privData.usb_demod_info.pDevCtrlInfo);
    USB_TS_BUF_MGR      *ptUsbTsBufMgr = 0;
    TSD_SAMPLE_INFO     tSample1 = {0};
    TSD_SAMPLE_INFO     tSample2 = {0}, tSample3 = {0}, tSample4 = {0},tSample5 = {0}, tSample6 = {0}, tSample7 = {0}, tSample8 = {0};

    ptUsbTsBufMgr = (USB_TS_BUF_MGR*)pCtrlInfo->privData;
    _tsaf_mutex_lock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, ptUsbTsBufMgr->usbBufMutex);
    switch( ptUsbTsBufMgr->bufState )
    {
        case USB_FILL_BUF_STOP:
            ptUsbTsBufMgr->bufState = USB_FILL_BUF_START;
            ptUsbTsBufMgr->usbBuf_RPtr = 0;
            ptUsbTsBufMgr->usbBuf_WPtr = USB_DEMOD_ACCESS_UNIT*6;
            //ptUsbTsBufMgr->bFirstSample = true;

            tSample1.bShareBuf    = true;
            tSample1.disableMutex = TSD_DISABLE_MUTEX;
            tSample1.sampleAddr   = &ptUsbTsBufMgr->pUsbTsBuf[0];
            tSample1.sampleSize   = USB_DEMOD_ACCESS_UNIT;

            tSample2.bShareBuf    = true;
            tSample2.disableMutex = TSD_DISABLE_MUTEX;
            tSample2.sampleAddr   = &ptUsbTsBufMgr->pUsbTsBuf[0];
            tSample2.sampleSize   = USB_DEMOD_ACCESS_UNIT;

            tSample3.bShareBuf    = true;
            tSample3.disableMutex = TSD_DISABLE_MUTEX;
            tSample3.sampleAddr   = &ptUsbTsBufMgr->pUsbTsBuf[USB_DEMOD_ACCESS_UNIT*1];
            tSample3.sampleSize   = USB_DEMOD_ACCESS_UNIT;

            tSample4.bShareBuf    = true;
            tSample4.disableMutex = TSD_DISABLE_MUTEX;
            tSample4.sampleAddr   = &ptUsbTsBufMgr->pUsbTsBuf[USB_DEMOD_ACCESS_UNIT*2];
            tSample4.sampleSize   = USB_DEMOD_ACCESS_UNIT;

            tSample5.bShareBuf    = true;
            tSample5.disableMutex = TSD_DISABLE_MUTEX;
            tSample5.sampleAddr   = &ptUsbTsBufMgr->pUsbTsBuf[USB_DEMOD_ACCESS_UNIT*3];
            tSample5.sampleSize   = USB_DEMOD_ACCESS_UNIT;

            tSample6.bShareBuf    = true;
            tSample6.disableMutex = TSD_DISABLE_MUTEX;
            tSample6.sampleAddr   = &ptUsbTsBufMgr->pUsbTsBuf[USB_DEMOD_ACCESS_UNIT*4];
            tSample6.sampleSize   = USB_DEMOD_ACCESS_UNIT;

            tSample7.bShareBuf    = true;
            tSample7.disableMutex = TSD_DISABLE_MUTEX;
            tSample7.sampleAddr   = &ptUsbTsBufMgr->pUsbTsBuf[USB_DEMOD_ACCESS_UNIT*5];
            tSample7.sampleSize   = USB_DEMOD_ACCESS_UNIT;

            tSample8.bShareBuf    = true;
            tSample8.disableMutex = TSD_DISABLE_MUTEX;
            tSample8.sampleAddr   = &ptUsbTsBufMgr->pUsbTsBuf[USB_DEMOD_ACCESS_UNIT*6];
            tSample8.sampleSize   = USB_DEMOD_ACCESS_UNIT;
            //_tsaf_mutex_unlock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, ptUsbTsBufMgr->usbBufMutex);

            pCtrlInfo->bUsbDefaultAccess = true;

            tSample1.bUsbDefaultAccess = pCtrlInfo->bUsbDefaultAccess; // default follow
            tSample1.pfCallback        = _UsbDemod_UpdateBuffer;
            tSample1.pCtrlParam        = pCtrlInfo;

            tSample2.bUsbDefaultAccess = pCtrlInfo->bUsbDefaultAccess; // default follow
            tSample2.pfCallback        = _UsbDemod_UpdateBuffer;
            tSample2.pCtrlParam        = pCtrlInfo;

            tSample3.bUsbDefaultAccess = pCtrlInfo->bUsbDefaultAccess; // default follow
            tSample3.pfCallback        = _UsbDemod_UpdateBuffer;
            tSample3.pCtrlParam        = pCtrlInfo;

            tSample4.bUsbDefaultAccess = pCtrlInfo->bUsbDefaultAccess; // default follow
            tSample4.pfCallback        = _UsbDemod_UpdateBuffer;
            tSample4.pCtrlParam        = pCtrlInfo;

            tSample5.bUsbDefaultAccess = pCtrlInfo->bUsbDefaultAccess; // default follow
            tSample5.pfCallback        = _UsbDemod_UpdateBuffer;
            tSample5.pCtrlParam        = pCtrlInfo;

            tSample6.bUsbDefaultAccess = pCtrlInfo->bUsbDefaultAccess; // default follow
            tSample6.pfCallback        = _UsbDemod_UpdateBuffer;
            tSample6.pCtrlParam        = pCtrlInfo;

            tSample7.bUsbDefaultAccess = pCtrlInfo->bUsbDefaultAccess; // default follow
            tSample7.pfCallback        = _UsbDemod_UpdateBuffer;
            tSample7.pCtrlParam        = pCtrlInfo;

            tSample8.bUsbDefaultAccess = pCtrlInfo->bUsbDefaultAccess; // default follow
            tSample8.pfCallback        = _UsbDemod_UpdateBuffer;
            tSample8.pCtrlParam        = pCtrlInfo;

            tsd_Get_Sample((TSD_HANDLE*)pCtrlInfo->extra_handle, TSD_SAMPLE_TS, &tSample1, 0);
            tsaf_msg_ex(1, "........... start receiving data from usb\n");

            tsd_Get_Sample((TSD_HANDLE*)pCtrlInfo->extra_handle, TSD_SAMPLE_TS, &tSample2, 0);
            tsd_Get_Sample((TSD_HANDLE*)pCtrlInfo->extra_handle, TSD_SAMPLE_TS, &tSample3, 0);
            tsd_Get_Sample((TSD_HANDLE*)pCtrlInfo->extra_handle, TSD_SAMPLE_TS, &tSample4, 0);
            tsd_Get_Sample((TSD_HANDLE*)pCtrlInfo->extra_handle, TSD_SAMPLE_TS, &tSample5, 0);
            tsd_Get_Sample((TSD_HANDLE*)pCtrlInfo->extra_handle, TSD_SAMPLE_TS, &tSample6, 0);
            tsd_Get_Sample((TSD_HANDLE*)pCtrlInfo->extra_handle, TSD_SAMPLE_TS, &tSample7, 0);
            tsd_Get_Sample((TSD_HANDLE*)pCtrlInfo->extra_handle, TSD_SAMPLE_TS, &tSample8, 0);

            _tsaf_mutex_unlock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, ptUsbTsBufMgr->usbBufMutex);
            break;

        default:
            _tsaf_mutex_unlock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, ptUsbTsBufMgr->usbBufMutex);
            break;
    }
    return 0;
}

static uint32_t
_UsbDemod_Rx_GetData(
    TSD_TS_RX_INFO      *pRxInfo,
    uint8_t             **ppSampleAddr,
    uint32_t            *pSampleLength,
    void                *extraData)
{
    // without copy from usb demod
    int                 rst = 0;
    uint32_t            cacheSize = 0;
    uint32_t            writePtr = 0;
    uint32_t            ReadRemainderSize = 0;
    TSAF_CRTL_INFO      *pCtrlInfo = (TSAF_CRTL_INFO*)(pRxInfo->privData.usb_demod_info.pDevCtrlInfo);
    USB_TS_BUF_MGR      *ptUsbTsBufMgr = 0;

    ptUsbTsBufMgr = (USB_TS_BUF_MGR*)pCtrlInfo->privData;

    do{
        if( _UsbDemod_IsFillingBuffer(ptUsbTsBufMgr) == false )       break;

        if( !pSampleLength || !ppSampleAddr )   break;
        else                                    *pSampleLength = 0;

        cacheSize = _usb_get_cacheSize(ptUsbTsBufMgr);

        if( cacheSize == 0 )
        {
            break;
        }

        ReadRemainderSize = (ptUsbTsBufMgr->bufferSize - ptUsbTsBufMgr->usbBuf_RPtr);
        *ppSampleAddr = &ptUsbTsBufMgr->pUsbTsBuf[ptUsbTsBufMgr->usbBuf_RPtr];
        if( cacheSize < ReadRemainderSize )
        {
            *pSampleLength = cacheSize;
            ptUsbTsBufMgr->usbBuf_RPtr += cacheSize;
        }
        else
        {
            *pSampleLength = ReadRemainderSize;
            ptUsbTsBufMgr->usbBuf_RPtr = 0;
        }

    }while(0);

    return rst;
}

////////////////////////////////////////
// Cache buffer access
/////////////////////////////////////////
static int
_usb_cache_Stop(
    uint32_t        index,
    TSAF_CRTL_INFO  *pCtrlInfo)
{
    return 0;
}

static int
_usb_cache_Start(
    uint32_t        index,
    TSAF_CRTL_INFO  *pCtrlInfo)
{
    TSD_TS_RX_INFO      *pRxInfo = (TSD_TS_RX_INFO*)pCtrlInfo->cacheInfo.extraData;
    bool                bUsbDefaultAccess = true;

    _UsbDemod_Rx_TurnOn(pRxInfo, 0);
    return 0;
}

static int
_usb_cache_NoBlockingGetData(
    uint32_t        index,
    uint8_t         *buffer,
    uint32_t        bufferLength,
    uint32_t        *pRealLength,
    TSAF_CRTL_INFO  *pCtrlInfo)
{
    // if fail return -1, success return 0
    int                 ret = (-1);
    USB_TS_BUF_MGR      *ptUsbTsBufMgr = 0;
    int                 cacheSize = 0;

    ptUsbTsBufMgr = (USB_TS_BUF_MGR*)pCtrlInfo->privData;

    if( !pCtrlInfo || !ptUsbTsBufMgr )
    {
        tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " err ! Null Pointer pTsafCtrlInf=0x%x, ptUsbTsBufMgr=0x%x\n",
                    pCtrlInfo, ptUsbTsBufMgr);
        return ret;
    }

    if( pRealLength )   *pRealLength = 0;

    if( false == _UsbDemod_IsFillingBuffer(ptUsbTsBufMgr) ) return ret;

    if( bufferLength )
    {
        uint32_t ReadRemainderSize = 0;

        cacheSize = _usb_get_cacheSize(ptUsbTsBufMgr);

        if( cacheSize < bufferLength )
        {
            return ret;
        }

        ReadRemainderSize = (ptUsbTsBufMgr->bufferSize - ptUsbTsBufMgr->usbBuf_RPtr);
        if( ReadRemainderSize < bufferLength )
        {
            memcpy(buffer, ptUsbTsBufMgr->pUsbTsBuf + ptUsbTsBufMgr->usbBuf_RPtr, ReadRemainderSize);
            memcpy(buffer + ReadRemainderSize, ptUsbTsBufMgr->pUsbTsBuf, bufferLength - ReadRemainderSize);
            ptUsbTsBufMgr->usbBuf_RPtr = (bufferLength - ReadRemainderSize);
        }
        else
        {
            memcpy(buffer, ptUsbTsBufMgr->pUsbTsBuf + ptUsbTsBufMgr->usbBuf_RPtr, bufferLength);
            if (ReadRemainderSize > bufferLength)
            {
                ptUsbTsBufMgr->usbBuf_RPtr += bufferLength;
            }
            else
            {
                ptUsbTsBufMgr->usbBuf_RPtr = 0;
            }
        }

        if( pRealLength )   *pRealLength = bufferLength;

        ret = 0;
        return ret;
    }
    else
    {
        return ret;
    }
}

static const TSD_TS_RX_INFO  g_usbDemodAccessDef =
{
    TSD_TSRX_TYPE_USB,      // uint32_t        tsrx_type;
    {0},                    // union{}privData;
    _UsbDemod_Rx_Init,      // uint32_t (*ts_rx_init)(struct _TSD_TS_RX_INFO_TAG *pRxInfo, void *extraData);
    _UsbDemod_Rx_TurnOn,    // uint32_t (*ts_rx_turn_on)(struct _TSD_TS_RX_INFO_TAG *pRxInfo, void *extraData);
    0,                      // uint32_t (*ts_rx_turn_off)(struct _TSD_TS_RX_INFO_TAG *pRxInfo, void *extraData);
    _UsbDemod_Rx_GetData,   // uint32_t (*ts_rx_get_data)(struct _TSD_TS_RX_INFO_TAG *pRxInfo, uint8_t **sampleAddr, uint32_t *sampleLength, void *extraData);
    _UsbDemod_Rx_deInit,    // uint32_t (*ts_rx_deinit)(struct _TSD_TS_RX_INFO_TAG *pRxInfo, void *extraData);
};

static const TSAF_CACHE_INFO g_usbDemodCacheDef =
{
    0,                            // void*    (*cache_BufThread)(void *args);
    0,                            // int      (*cache_StopThread)(uint32_t index, struct _TSAF_CRTL_INFO_TAG *pTsafCtrlInfo);
    _usb_cache_Stop,              // int      (*cache_Stop)(uint32_t index, struct _TSAF_CRTL_INFO_TAG *pTsafCtrlInfo);
    _usb_cache_Start,             // int      (*cache_Start)(uint32_t index, struct _TSAF_CRTL_INFO_TAG *pTsafCtrlInfo);
    _usb_cache_NoBlockingGetData, // int      (*cache_GetData)(uint32_t index, uint8_t *buffer, uint32_t bufferLength, struct _TSAF_CRTL_INFO_TAG *pTsafCtrlInfo);
    0,                  // void     *privData;
};

#else

static const TSD_TS_RX_INFO  g_usbDemodAccessDef = {0};
static const TSAF_CACHE_INFO g_usbDemodCacheDef  = {0};
#endif

static const TSAF_CACHE_INFO g_tsiDemodCacheDef =
{
    _tsi_cache_BufThread,         // void*    (*cache_BufThread)(void *args);
    _tsi_cache_StopThread,        // int      (*cache_StopThread)(uint32_t index, struct _TSAF_CRTL_INFO_TAG *pTsafCtrlInfo);
    _tsi_cache_Stop,              // int      (*cache_Stop)(uint32_t index, struct _TSAF_CRTL_INFO_TAG *pTsafCtrlInfo);
    _tsi_cache_Start,             // int      (*cache_Start)(uint32_t index, struct _TSAF_CRTL_INFO_TAG *pTsafCtrlInfo);
    _tsi_cache_NoBlockingGetData, // int      (*cache_GetData)(uint32_t index, uint8_t *buffer, uint32_t bufferLength, struct _TSAF_CRTL_INFO_TAG *pTsafCtrlInfo);
    0,                  // void     *privData;
};

static bool
_gateway_GetFreeRiscShareChannelInfo(
    uint32_t       pChannelId,
    CHANNEL_INFO** ptFreeChannel)
{
    //uint32_t i = 0;
    CHANNEL_INFO_ARRAY* ptChannelInfoArray = (CHANNEL_INFO_ARRAY*) iteTsDemux_GetChannelInfoArrayHandle();
    //for (i = 0; i < MAX_CHANNEL_COUNT; i++)
    //    if (!(ptChannelInfoArray->tChannelArray[i].bValidChannel))
    //        break;

    //if (i >= MAX_CHANNEL_COUNT)
    //{
    //    return false;
    //}

    //*pChannelId = i;
    *ptFreeChannel = &ptChannelInfoArray->tChannelArray[pChannelId];
    return true;
}

static bool
_gateway_InsertChannelPid(
    CHANNEL_INFO* ptChannelInfo,
    uint32_t      pid,
    uint32_t      pesSamplekSize,
    uint32_t      pesSampleCount,
    uint32_t*     pesIndex)
{
    uint32_t i = 0;
    PID_INFO* ptPidInfo = 0;
    for (i = 0; i < ptChannelInfo->validPidCount; i++) asm("");

    if (i >= MAX_PID_COUNT_PER_CHANNEL)
    {
        return false;
    }
    ptPidInfo = &ptChannelInfo->tPidInfo[i];

    ptPidInfo->pid = pid;
    ptPidInfo->pOutPesSampleSize = pesSamplekSize;
    ptPidInfo->validPesSampleCount = pesSampleCount;
    if (ptPidInfo->pOutPesBuffer)
        free(ptPidInfo->pOutPesBuffer);
    ptPidInfo->pOutPesBuffer = (uint8_t*) malloc(pesSamplekSize * pesSampleCount);
    if (0 == ptPidInfo->pOutPesBuffer)
    {
        return false;
    }
    ptChannelInfo->validPidCount++;
}



////////////////////////////////////////////////

static TSAF_ERR
gateway_Init(
    TSAF_CRTL_INFO  *pCtrlInfo,
    TSAF_INIT_PARAM *pInitParam,
    void            *extraDat)
{
    TSAF_ERR            result = TSAF_ERR_OK;
    TSD_PRE_SET_INFO    presetInfo = {0};

    do{
        int                 i, rc;
        TSAF_DEMOD_TYPE     act_demod_type = TSAF_ACTION_DEMOD_ID;
        THREAD_ROUTINE_FUNC  _routine_func = 0;

        presetInfo.demod_id       = (uint32_t)(-1);
        pCtrlInfo->ts_country_cnt = (-1);

        presetInfo.tsdOutMode   = TSD_OUT_TS_BY_PASS;

        switch( act_demod_type )
        {
            case TSAF_DEMOD_OMEGA:      presetInfo.tsdDemodType = TSD_DEMOD_OMEGA;    break;
            case TSAF_DEMOD_IT9135:     presetInfo.tsdDemodType = TSD_DEMOD_IT9135;   break;
            case TSAF_DEMOD_IT9137:     presetInfo.tsdDemodType = TSD_DEMOD_IT9137;   break;
            case TSAF_DEMOD_IT9137_USB: presetInfo.tsdDemodType = TSD_DEMOD_IT9137_USB;   break;
        }

        //---------------------------------------
        // auto select un-used demod
        for(i = 0; i < MAX_DEMOD_SUPPORT_CNT; i++)
        {
            if( g_demod_base_info[i].bUsed == false )
            {
                presetInfo.demod_id = i;
                presetInfo.tsi_id   = g_demod_base_info[i].tsi_index;

                g_demod_base_info[i].bUsed = true;
                break;
            }
        }

        if( presetInfo.demod_id == (uint32_t)(-1) )
        {
            tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " err ! No free demod can be used !!");
            break;
        }

        pCtrlInfo->demod_id = pInitParam->demod_index = presetInfo.demod_id;

        if( g_demod_base_info[pCtrlInfo->demod_id].bUsb_demod )
        {
            USB_TS_BUF_MGR      *pUsbBufMgr = 0;

            //---------------------------------------
            // creater usb buffer mgr
            if( pUsbBufMgr = (USB_TS_BUF_MGR*)malloc(sizeof(USB_TS_BUF_MGR)) )
            {
                memset(pUsbBufMgr, 0x0, sizeof(USB_TS_BUF_MGR));
                pUsbBufMgr->demod_index = pCtrlInfo->demod_id;
            }
            else
                tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " demod %d-th malloc USB buf fail !!\n", pCtrlInfo->demod_id);

            pCtrlInfo->privData = (void*)pUsbBufMgr;

            //---------------------------------------
            // set usb demod module access operator
            presetInfo.tsRecevier = g_usbDemodAccessDef;
            presetInfo.tsRecevier.privData.usb_demod_info.pDevCtrlInfo = (void*)pCtrlInfo;

            //---------------------------------------
            // set usb demod cache buffer operator
            pCtrlInfo->cacheInfo = g_usbDemodCacheDef;

            //---------------------------------------
            // create tsd handle
            tsd_CreateHandle((TSD_HANDLE**)&pInitParam->extra_handle, &presetInfo, 0);
            if( pInitParam->extra_handle == 0 )
            {
                result = TSAF_ERR_ALLOCATE_FAIL;
                tsaf_msg_ex(TSAF_MSG_TYPE_ERR, "create tsd handle fail (demodId=%d, tsiId=%d) !!\n", presetInfo.demod_id, presetInfo.tsi_id);
                break;
            }
            pCtrlInfo->extra_handle = pInitParam->extra_handle;
        }
        else
        {
            TSI_TS_BUF_MGR      *ptTsiBufMgr = 0;

            //---------------------------------------
            // creater usb buffer mgr
            if( ptTsiBufMgr = (TSI_TS_BUF_MGR*)malloc(sizeof(TSI_TS_BUF_MGR)) )
            {
                memset(ptTsiBufMgr, 0x0, sizeof(TSI_TS_BUF_MGR));
                ptTsiBufMgr->demod_index = pCtrlInfo->demod_id;

                _tsaf_mutex_init(TSAF_MSG_TYPE_TRACE_CACHE_BUF, ptTsiBufMgr->tsiBufMutex);
            }
            else
            {
                tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " demod %d-th malloc USB buf fail !!\n", pCtrlInfo->demod_id);
            }

            pCtrlInfo->privData = (void*)ptTsiBufMgr;

            //---------------------------------------
            // set tsi demod cache buffer operator
            pCtrlInfo->cacheInfo = g_tsiDemodCacheDef;
            presetInfo.tsRecevier.tsrx_type = TSD_TSRX_TYPE_TSI;

            //---------------------------------------
            // create tsd handle
            tsd_CreateHandle((TSD_HANDLE**)&pInitParam->extra_handle, &presetInfo, 0);
            if( pInitParam->extra_handle == 0 )
            {
                result = TSAF_ERR_ALLOCATE_FAIL;
                tsaf_msg_ex(TSAF_MSG_TYPE_ERR, "create tsd handle fail (demodId=%d, tsiId=%d) !!\n", presetInfo.demod_id, presetInfo.tsi_id);
                break;
            }

            // -------------------------
            // Create thread
            pCtrlInfo->extra_handle = pInitParam->extra_handle;

            rc = pthread_create(&pInitParam->thread, NULL, pCtrlInfo->cacheInfo.cache_BufThread, (void *)pCtrlInfo);
            if( rc )
            {
                tsaf_msg_ex(TSAF_MSG_TYPE_ERR, "ERROR; pthread_create() fail %d\n", rc);
            }
        #ifndef _WIN32
            pCtrlInfo->thread = pInitParam->thread;
        #endif
            usleep(5000);
        }

    }while(0) ;

    return result;
}

static TSAF_ERR
gateway_deInit(
    TSAF_CRTL_INFO  *pCtrlInfo,
    void            *extraDat)
{
    TSAF_ERR     result = TSAF_ERR_OK;

    tsd_Control((TSD_HANDLE*)pCtrlInfo->extra_handle, TSD_CTRL_DEMOD_RESET, 0, 0);
    // destroy tsd handle
    tsd_DestroyHandle((TSD_HANDLE**)&pCtrlInfo->extra_handle);
    g_demod_base_info[pCtrlInfo->demod_id].bUsed = false;

    // destroy thread


    return result;
}

static TSAF_ERR
gateway_Open(
    TSAF_CRTL_INFO  *pCtrlInfo,
    uint32_t        service_index,
    void            *extraDat)
{
    TSAF_ERR     result = TSAF_ERR_OK;

    #if _MSC_VER && (ENABLE_FILE_SIMULATION)
        {
            int   i;

            tsd_Change_Service((TSD_HANDLE*)pCtrlInfo->extra_handle, service_index, 0);
            for(i = 0; i < MAX_DEMOD_SUPPORT_CNT; i++)
                if( ((TSD_HANDLE*)pCtrlInfo->extra_handle)->actFreq == ts_src_input[i].freq )
                {
                    f_ts_scan = ts_src_input[i].f_ts;
                    break;
                }
        }
    #else
        tsd_Control((TSD_HANDLE*)pCtrlInfo->extra_handle, TSD_CTRL_SET_DEMOD_STATUS, (uint32_t*)TSD_DEMOD_STATUS_RUNNING, 0);
    #endif
    return result;
}

static TSAF_ERR
gateway_Close(
    TSAF_CRTL_INFO  *pCtrlInfo,
    void            *extraDat)
{
    TSAF_ERR     result = TSAF_ERR_OK;

    // reset demod status
    tsd_Control((TSD_HANDLE*)pCtrlInfo->extra_handle, TSD_CTRL_SET_DEMOD_STATUS, (uint32_t*)TSD_DEMOD_STATUS_IDLE, 0);
    return result;
}

static int
gateway_readData(
    TSAF_CRTL_INFO  *pCtrlInfo,
    uint8_t         *pDstBuf,
    int             length,
    uint32_t        *pRealLength,
    void            *extraDat)
{
    int         ret = -1;
    uint32_t    real_size = 0;

    if( pCtrlInfo->cacheInfo.cache_GetData )
    {
        ret = pCtrlInfo->cacheInfo.cache_GetData(pCtrlInfo->demod_id, pDstBuf, length, &real_size, pCtrlInfo);
        if( ret < 0)   real_size = 0;
    }

    if( pRealLength )       *pRealLength = real_size;

    return ret;
}

static TSAF_ERR
gateway_get_inSrc_func(
    TSAF_CRTL_INFO  *pCtrlInfo,
    pFunc_inSrc     *ppFunc,
    void            *extraDat)
{
    TSAF_ERR     result = TSAF_ERR_OK;
    return result;
}


static TSAF_ERR
gateway_scanFreq(
    TSAF_CRTL_INFO  *pCtrlInfo,
    void            *extraDat)
{
    TSAF_ERR        result = TSAF_ERR_OK;
    TSD_SCAN_PARAM  tsdScanParam = {0};
    dictionary*     ini;

    //CHANNEL_INFO_ARRAY* ptShareInfo = NULL;

    //read demod info from NOR
    ini = iniparser_load(CFG_PRIVATE_DRIVE ":/gateway.ini");
    if (!ini)
        printf("cannot load ini file: %s\n", CFG_PRIVATE_DRIVE":/gateway.ini");
    else
    {
        g_demod_base_info[0].startFreq = atoi(iniparser_getstring(ini, "ts:freq0", "611000"));;
        g_demod_base_info[0].bandwidth = atoi(iniparser_getstring(ini, "ts:bandwidth0", "6000"));;
        g_demod_base_info[1].startFreq = atoi(iniparser_getstring(ini, "ts:freq1", "611000"));;
        g_demod_base_info[1].bandwidth = atoi(iniparser_getstring(ini, "ts:bandwidth1", "6000"));;
        g_demod_base_info[2].startFreq = atoi(iniparser_getstring(ini, "ts:freq2", "611000"));;
        g_demod_base_info[2].bandwidth = atoi(iniparser_getstring(ini, "ts:bandwidth2", "6000"));;
        g_demod_base_info[3].startFreq = atoi(iniparser_getstring(ini, "ts:freq3", "611000"));;
        g_demod_base_info[3].bandwidth = atoi(iniparser_getstring(ini, "ts:bandwidth3", "6000"));;
    }
    iniparser_freedict(ini);

    switch( pCtrlInfo->scanParam.scan_type )
    {
        case TSAF_SCAN_AUTO:
            //----------------------------------------
            // demod Acquire Channel
            #if _MSC_VER && (ENABLE_FILE_SIMULATION)
                _scan_channel((TSD_HANDLE*)pCtrlInfo->extra_handle, pCtrlInfo->demod_id);
            #else

                tsd_Control((TSD_HANDLE*)pCtrlInfo->extra_handle, TSD_CTRL_SET_DEMOD_STATUS, (uint32_t*)TSD_DEMOD_STATUS_RUNNING, 0);
                tsdScanParam.scanFrequency = g_demod_base_info[pCtrlInfo->demod_id].startFreq;
                tsdScanParam.bandwidth     = g_demod_base_info[pCtrlInfo->demod_id].bandwidth;

                result = tsd_Scan_Channel((TSD_HANDLE*)pCtrlInfo->extra_handle, TSD_FREQ_SCAN_MANUAL, &tsdScanParam, 0);
            #endif

            // disable parsing EPG
            tsd_Control((TSD_HANDLE*)pCtrlInfo->extra_handle, TSD_CTRL_SKIP_EPG_PARSING, (uint32_t*)true, 0);
            printf("total SrvCount : %u\n", ((TSD_HANDLE*)pCtrlInfo->extra_handle)->totalSrvc);
            if (((TSD_HANDLE*)pCtrlInfo->extra_handle)->totalSrvc)
            {
                uint32_t i = 0;
                uint32_t j = 0;
                uint32_t      channelId = pCtrlInfo->demod_id;
                CHANNEL_INFO* ptChannelInfo = 0;

                TSD_SERVICE_PID_INFO tPidInfo = { 0 };

                iteTsDemux_DisableChannel(channelId);

                if (_gateway_GetFreeRiscShareChannelInfo(channelId, &ptChannelInfo))
                {
                    uint8_t *pTsInputBuffer = 0;
                    uint32_t inputBufferSize = 0;
                    uint32_t startBufferReadIndex = 0;
                    // USB

                    if (g_demod_base_info[pCtrlInfo->demod_id].bUsb_demod)
                    {
                        USB_TS_BUF_MGR* ptUsbTsBufMgr = (USB_TS_BUF_MGR*)pCtrlInfo->privData;
                        pTsInputBuffer = ptUsbTsBufMgr->pUsbTsBuf;
                        inputBufferSize = USB_DEMOD_BUF_SIZE;
                        startBufferReadIndex = ptUsbTsBufMgr->usbBuf_WPtr;
                        ptUsbTsBufMgr->ptChannelInfo = ptChannelInfo;
                    }
                    else // TSI
                    {
                        TSI_TS_BUF_MGR* ptTsiTsBufMgr = (TSI_TS_BUF_MGR*)pCtrlInfo->privData;
                        pTsInputBuffer = ptTsiTsBufMgr->pTsiTsBuf;
                        inputBufferSize = TSI_CACHE_BUFFER_SIZE;
                        startBufferReadIndex = ptTsiTsBufMgr->tsiBuf_WPtr;
                        ptTsiTsBufMgr->ptChannelInfo = ptChannelInfo;
                    }

                    if (pTsInputBuffer)
                    {
                        uint32_t pesIndex = 0;
                        ptChannelInfo->pInputTsBuffer = pTsInputBuffer;
                        ptChannelInfo->tsBufferSize = inputBufferSize;
                        ptChannelInfo->tsBufferReadIdx = startBufferReadIndex;
                        ithFlushMemBuffer();

                        for (i = 0; i < ((TSD_HANDLE*)pCtrlInfo->extra_handle)->totalSrvc; i++)
                        {
                            memset(&tPidInfo, 0x0, sizeof(TSD_SERVICE_PID_INFO));
                            tsd_Get_PidInfo((TSD_HANDLE*)pCtrlInfo->extra_handle, i, &tPidInfo);

                            if (tPidInfo.videoType != 2)
                            {
                                if (_gateway_InsertChannelPid(ptChannelInfo, tPidInfo.videoPID, 250 * 1024, 12, &pesIndex))
                                {
                                    printf("pes index: %u\n", pesIndex);
                                }
                            }
                        }

                        for (i = 0; i < ptChannelInfo->validPidCount; i++)
                        {
                            printf("pid: 0x%X, buf: 0x%X\n", ptChannelInfo->tPidInfo[i].pid, ptChannelInfo->tPidInfo[i].pOutPesBuffer);
                        }
                    }
                    iteTsDemux_EnableChannel(channelId);
                }
            }
            break;

        case TSAF_SCAN_MANUAL:
            break;
    }

    return result;
}

static TSAF_ERR
gateway_control(
    TSAF_CRTL_INFO  *pCtrlInfo,
    uint32_t        cmd,
    uint32_t        *value,
    void            *extraDat)
{
    TSAF_ERR     result = TSAF_ERR_OK;

    switch( cmd )
    {
        default:        break;
    }
    return result;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================

TS_AIRFILE_DESC  TS_AIRFILE_DESC_split_gateway_desc =
{
    "ts air file Split GateWay", // char        *name;
    0,                           // struct _TS_AIRFILE_DESC_TAG  *next;
    TS_AIRFILE_ID_SPLIT_GATEWAY, // TS_AIRFILE_TYPE_ID           id;
    0,                           // void        *privInfo;

    // // operator
    gateway_Init,               // int     (*taf_init)(TSAF_CRTL_INFO *pCtrlInfo, TSAF_INIT_PARAM *pInitParam, void *extraData);
    gateway_deInit,             // int     (*taf_deinit)(TSAF_CRTL_INFO *pCtrlInfo, void *extraData);

    gateway_Open,               // int     (*taf_open)(TSAF_CRTL_INFO *pCtrlInfo, void *extraData);
    gateway_Close,              // int     (*taf_close)(TSAF_CRTL_INFO *pCtrlInfo, void *extraData);
    gateway_readData,           // int     (*taf_read)(TSAF_CRTL_INFO *pCtrlInfo, void *extraData);
    0,                          // int     (*taf_write)(TSAF_CRTL_INFO *pCtrlInfo, void *extraData);
    gateway_get_inSrc_func,     // int     (*taf_get_inSrc_func)(TSAF_CRTL_INFO *pCtrlInfo, pFunc_inSrc *ppFunc, void *extraData);
    // gateway_changeServic,       // int     (*taf_changeService)(TSAF_CRTL_INFO *pCtrlInfo, void *extraData);
    gateway_scanFreq,           // int     (*taf_scanFreq)(TSAF_CRTL_INFO *pCtrlInfo, void *extraData);
    gateway_control,            // int     (*taf_control)(TSAF_CRTL_INFO *pCtrlInfo, uint32_t cmd, uint32_t *value, void *extraData);
};
#else

TS_AIRFILE_DESC  TS_AIRFILE_DESC_split_gateway_desc =
{
    "ts air file Split GateWay",      // char        *name;
    0,                          // struct _TS_AIRFILE_DESC_TAG  *next;
    TS_AIRFILE_ID_SPLIT_GATEWAY,      // TS_AIRFILE_TYPE_ID           id;
    0,                          // void        *privInfo;
};
#endif


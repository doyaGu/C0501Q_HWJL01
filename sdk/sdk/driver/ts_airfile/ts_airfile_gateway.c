
#include <unistd.h>
#include <pthread.h>
#include "ts_airfile.h"

#if (CONFIG_TS_AIRFILE_DESC_GATEWAY_DESC)

#include "ts_demuxer/ite_ts_demuxer.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define ENABLE_FILE_SIMULATION       0
// #define MAX_DEMOD_SUPPORT_CNT       4

#define USB_DEMOD_ACCESS_UNIT           (348 * 188)
#define USB_DEMOD_BUF_SIZE              (USB_DEMOD_ACCESS_UNIT * 7 * 25)  // need aligement of USB_DEMOD_ACCESS_UNIT

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

    const uint32_t      startFreq;
    const uint32_t      bandwidth;
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

} USB_TS_BUF_MGR;

//=============================================================================
//                  Global Data Definition
//=============================================================================
static DEMOD_BASE_INFO  g_demod_base_info[] =
{
    {"IamBig",      false, false, 0,    557000, 6000, 3},
    {"IBigerThanU", false, false, 1,    557000, 6000, 3},
    {"bigIsGood",   false, true,  (-1), 557000, 6000, 3},
    {"suckTalk",    false, true,  (-1), 557000, 6000, 1},
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

static void
_UsbDemod_UpdateBuffer(
    uint32_t    index,
    void        *pTsafCtrlInfo)
{
    TSD_SAMPLE_INFO         tSample = {0};
    USB_FILL_BUF_STAGE      curState = USB_FILL_BUF_STOP;
    TSAF_CRTL_INFO          *pCtrlInfo = (TSAF_CRTL_INFO*)pTsafCtrlInfo;
    USB_TS_BUF_MGR          *ptUsbTsBufMgr = 0;

    ptUsbTsBufMgr = (USB_TS_BUF_MGR*)pCtrlInfo->privData;

    _tsaf_mutex_lock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, ptUsbTsBufMgr->usbBufMutex);
    curState = ptUsbTsBufMgr->bufState;
    _tsaf_mutex_unlock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, ptUsbTsBufMgr->usbBufMutex);

    switch( curState )
    {
        case USB_FILL_BUF_STOPPING:
            if (ptUsbTsBufMgr->bFirstSampleStop == true)
            {
                ptUsbTsBufMgr->bFirstSampleStop = false;
                break;
            }
            else
            {
                ptUsbTsBufMgr->bufState = USB_FILL_BUF_STOP;
                break;
            }

        default:
            if( ptUsbTsBufMgr->bFirstSample == true )
            {
                ptUsbTsBufMgr->usbBuf_WPtr = 0;
                ptUsbTsBufMgr->bFirstSample = false;
            }
            else
                ptUsbTsBufMgr->usbBuf_WPtr += USB_DEMOD_ACCESS_UNIT;

            if( ptUsbTsBufMgr->usbBuf_WPtr == ptUsbTsBufMgr->bufferSize )
            {
                ptUsbTsBufMgr->usbBuf_WPtr = 0;
            }
            //printf("usbBuf_WPtr=%d\n", ptUsbTsBufMgr->usbBuf_WPtr);

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

        if( ptUsbTsBufMgr->pUsbTsBuf )      free(ptUsbTsBufMgr->pUsbTsBuf);

        _tsaf_mutex_deinit(TSAF_MSG_TYPE_TRACE_CACHE_BUF, ptUsbTsBufMgr->usbBufMutex);

        free(ptUsbTsBufMgr);
        pCtrlInfo->privData = 0;

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
    TSD_SAMPLE_INFO     tSample2 = {0};

    ptUsbTsBufMgr = (USB_TS_BUF_MGR*)pCtrlInfo->privData;

    _tsaf_mutex_lock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, ptUsbTsBufMgr->usbBufMutex);
    switch( ptUsbTsBufMgr->bufState )
    {
        case USB_FILL_BUF_STOP:
            ptUsbTsBufMgr->bufState = USB_FILL_BUF_START;
            ptUsbTsBufMgr->usbBuf_RPtr = 0;
            ptUsbTsBufMgr->usbBuf_WPtr = 0;
            //ptUsbTsBufMgr->bFirstSample = true;

            tSample1.bShareBuf    = true;
            tSample1.disableMutex = TSD_DISABLE_MUTEX;
            tSample1.sampleAddr   = &ptUsbTsBufMgr->pUsbTsBuf[ptUsbTsBufMgr->usbBuf_WPtr];
            tSample1.sampleSize   = USB_DEMOD_ACCESS_UNIT;

            tSample2.bShareBuf    = true;
            tSample2.disableMutex = TSD_DISABLE_MUTEX;
            tSample2.sampleAddr   = &ptUsbTsBufMgr->pUsbTsBuf[ptUsbTsBufMgr->usbBuf_WPtr];
            tSample2.sampleSize   = USB_DEMOD_ACCESS_UNIT;
            _tsaf_mutex_unlock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, ptUsbTsBufMgr->usbBufMutex);

            pCtrlInfo->bUsbDefaultAccess = true;

            tSample1.bUsbDefaultAccess = pCtrlInfo->bUsbDefaultAccess; // default follow
            tSample1.pfCallback        = _UsbDemod_UpdateBuffer;
            tSample1.pCtrlParam        = pCtrlInfo;

            tSample2.bUsbDefaultAccess = pCtrlInfo->bUsbDefaultAccess; // default follow
            tSample2.pfCallback        = _UsbDemod_UpdateBuffer;
            tSample2.pCtrlParam        = pCtrlInfo;

            tsd_Get_Sample((TSD_HANDLE*)pCtrlInfo->extra_handle, TSD_SAMPLE_TS, &tSample1, 0);
            tsaf_msg_ex(1, "........... start receiving data from usb\n");

            tsd_Get_Sample((TSD_HANDLE*)pCtrlInfo->extra_handle, TSD_SAMPLE_TS, &tSample2, 0);
            break;

        default:
            _tsaf_mutex_unlock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, ptUsbTsBufMgr->usbBufMutex);
            break;
    }
    return 0;
}

static uint32_t
_UsbDemod_Rx_TurnOff(
    TSD_TS_RX_INFO      *pRxInfo,
    void                *extraData)
{
    int                 rst = 0;
    TSAF_CRTL_INFO      *pCtrlInfo = (TSAF_CRTL_INFO*)(pRxInfo->privData.usb_demod_info.pDevCtrlInfo);
    USB_TS_BUF_MGR      *ptUsbTsBufMgr = 0;
    bool                bBreak = false;

    ptUsbTsBufMgr = (USB_TS_BUF_MGR*)pCtrlInfo->privData;

    while (1)
    {
        _tsaf_mutex_lock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, ptUsbTsBufMgr->usbBufMutex);

        switch( ptUsbTsBufMgr->bufState )
        {
            case USB_FILL_BUF_START:
                ptUsbTsBufMgr->bufState = USB_FILL_BUF_STOPPING;
                ptUsbTsBufMgr->bFirstSampleStop = true;
                break;

            case USB_FILL_BUF_STOP:
                ptUsbTsBufMgr->usbBuf_RPtr = ptUsbTsBufMgr->usbBuf_WPtr = 0;
                printf(".....................USB_FILL_BUF_STOP\n");
                bBreak = true;
                break;
        }

        _tsaf_mutex_unlock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, ptUsbTsBufMgr->usbBufMutex);

        if( bBreak == true )        break;

        usleep(1000);
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
            //tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " err ! usb cache buf No Data !!\n");
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

        //printf(" pSampleAddr=0x%x, pSampleLength=%d, %s()[#%d]\n", *ppSampleAddr, *pSampleLength, __FUNCTION__, __LINE__);
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
    TSD_TS_RX_INFO      *pRxInfo = (TSD_TS_RX_INFO*)pCtrlInfo->cacheInfo.extraData;

    //_UsbDemod_Rx_TurnOff(pRxInfo, 0);
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
_usb_cache_GetData(
    uint32_t        index,
    uint8_t         *buffer,
    uint32_t        bufferLength,
    TSAF_CRTL_INFO  *pCtrlInfo)
{
    // if fail return -1, success return 0
    struct timeval      startT;
    int                 ret = (-1);
    uint32_t            cacheSize = 0;
    uint32_t            writePtr = 0;
    USB_TS_BUF_MGR      *ptUsbTsBufMgr = 0;


    ptUsbTsBufMgr = (USB_TS_BUF_MGR*)pCtrlInfo->privData;

    if( !pCtrlInfo || !ptUsbTsBufMgr )
    {
        tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " err ! Null Pointer pTsafCtrlInf=0x%x, ptUsbTsBufMgr=0x%x\n",
                    pCtrlInfo, ptUsbTsBufMgr);
        return ret;
    }

    tsaf_get_clock(&startT);

    do{
        uint32_t validSize = 0;

        if( false == _UsbDemod_IsFillingBuffer(ptUsbTsBufMgr) )      break;

        if( bufferLength )
        {
            uint32_t ReadRemainderSize = 0;
            struct timeval waitT;

            tsaf_get_clock(&waitT);

            while( tsaf_get_duration(&waitT) < 2500 )
            {
                cacheSize = _usb_get_cacheSize(ptUsbTsBufMgr);

                if( cacheSize < bufferLength )
                {
                    usleep(1500);
                }
                else
                    break;
            }

            if( cacheSize < bufferLength )
            {
                tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " usb demod cache buf (cacheSize=%d, requst=%d) too slow \n",
                            cacheSize, bufferLength);
                break;
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

            ret = 0;
            return ret;
        }
    }while( tsaf_get_duration(&startT) < 3500 );

    tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " usb demod wait cache time out !! \n");
    return ret;
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

        if( cacheSize < (bufferLength + USB_DEMOD_ACCESS_UNIT) )
        {
            return ret;
        }

        //printf("usbBuf_RPtr=%d, usbBuf_WPtr=%d\n", ptUsbTsBufMgr->usbBuf_RPtr, ptUsbTsBufMgr->usbBuf_WPtr);
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
    _UsbDemod_Rx_TurnOff,   // uint32_t (*ts_rx_turn_off)(struct _TSD_TS_RX_INFO_TAG *pRxInfo, void *extraData);
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

static uint32_t
_tsi_cache_GetDataSize(
    uint32_t            index,
    TSAF_CRTL_INFO      *pTsafCtrlInfo)
{
    uint32_t    validSize = 0;
    uint32_t    maxCacheBufSize = pTsafCtrlInfo->cacheBufMgr.maxCacheBufSize;

    _tsaf_mutex_lock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, pTsafCtrlInfo->cacheBufMgr.cacheBufAccessMutex);

    if( pTsafCtrlInfo->cacheBufMgr.cacheBuf_WPtr < pTsafCtrlInfo->cacheBufMgr.cacheBuf_RPtr )
    {
        validSize = (maxCacheBufSize - pTsafCtrlInfo->cacheBufMgr.cacheBuf_RPtr) + pTsafCtrlInfo->cacheBufMgr.cacheBuf_WPtr;
    }
    else
    {
        validSize = pTsafCtrlInfo->cacheBufMgr.cacheBuf_WPtr - pTsafCtrlInfo->cacheBufMgr.cacheBuf_RPtr;
    }

    _tsaf_mutex_unlock(TSAF_MSG_TYPE_TRACE_CACHE_BUF, pTsafCtrlInfo->cacheBufMgr.cacheBufAccessMutex);

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
            case TSAF_DEMOD_OMEGA:  presetInfo.tsdDemodType = TSD_DEMOD_OMEGA;    break;
            case TSAF_DEMOD_IT9135: presetInfo.tsdDemodType = TSD_DEMOD_IT9135;   break;
            case TSAF_DEMOD_IT9137: presetInfo.tsdDemodType = TSD_DEMOD_IT9137;   break;
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
        }
        else
        {
            presetInfo.tsRecevier.tsrx_type = TSD_TSRX_TYPE_TSI;
            _routine_func = pInitParam->threadFunc;

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
            // note: tsi module access operator use default
            //       L2 cache buffer operator use default
            pCtrlInfo->cacheInfo.cache_GetData = _tsi_cache_NoBlockingGetData;

            // -------------------------
            // Create thread
            rc = pthread_create(&pInitParam->thread, NULL, _routine_func, (void *)pInitParam);
            if( rc )
            {
                tsaf_msg_ex(TSAF_MSG_TYPE_ERR, "ERROR; pthread_create() fail %d\n", rc);
            }

            pCtrlInfo->thread = pInitParam->thread;

            usleep(5000);
        }

        pCtrlInfo->extra_handle = pInitParam->extra_handle;

    }while(0) ;

    return result;
}

static TSAF_ERR
gateway_deInit(
    TSAF_CRTL_INFO  *pCtrlInfo,
    void            *extraDat)
{
    TSAF_ERR     result = TSAF_ERR_OK;

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

        if( ((TSD_HANDLE*)pCtrlInfo->extra_handle)->actSrvcIdx != service_index )
        {
            // do change service
            //tsaf_msg_ex(1, "change service to %d-th\n", service_index);

            /**
             * usb_demod driver something wrong !!!!!!!!!!!!!!!
             * it can't call DemodCtrl_ResetPidTable()
             **/
            if( !g_demod_base_info[pCtrlInfo->demod_id].bUsb_demod )
                tsd_Change_Service((TSD_HANDLE*)pCtrlInfo->extra_handle, service_index, 0);
        }
    #endif
    return result;
}

static TSAF_ERR
gateway_Close(
    TSAF_CRTL_INFO  *pCtrlInfo,
    void            *extraDat)
{
    TSAF_ERR     result = TSAF_ERR_OK;
    // stop cacne

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
                printf(" \n\n scan channel: %d, %d\n", tsdScanParam.scanFrequency, tsdScanParam.bandwidth);

                tsd_Scan_Channel((TSD_HANDLE*)pCtrlInfo->extra_handle, TSD_FREQ_SCAN_MANUAL, &tsdScanParam, 0);
                //tsdScanParam.countryId = TSD_COUNTRY_TAIWAN;
                //tsd_Scan_Channel(pCtrlInfo->pHTsd, TSD_FREQ_SCAN_AUTO, &tsdScanParam, 0);
            #endif

            // disable parsing EPG
            tsd_Control((TSD_HANDLE*)pCtrlInfo->extra_handle, TSD_CTRL_SKIP_EPG_PARSING, (uint32_t*)true, 0);

            /**
             * usb_demod driver something wrong !!!!!!!!!!!!!!!
             * it can't call DemodCtrl_ResetPidTable()
             **/
            if( !g_demod_base_info[pCtrlInfo->demod_id].bUsb_demod )
                tsd_Change_Service((TSD_HANDLE*)pCtrlInfo->extra_handle, 0, 0);
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

TS_AIRFILE_DESC  TS_AIRFILE_DESC_gateway_desc =
{
    "ts air file GateWay",      // char        *name;
    0,                          // struct _TS_AIRFILE_DESC_TAG  *next;
    TS_AIRFILE_ID_GATEWAY,      // TS_AIRFILE_TYPE_ID           id;
    0,                          // void        *privInfo;

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

TS_AIRFILE_DESC  TS_AIRFILE_DESC_gateway_desc =
{
    "ts air file GateWay",      // char        *name;
    0,                          // struct _TS_AIRFILE_DESC_TAG  *next;
    TS_AIRFILE_ID_GATEWAY,      // TS_AIRFILE_TYPE_ID           id;
    0,                          // void        *privInfo;
};
#endif


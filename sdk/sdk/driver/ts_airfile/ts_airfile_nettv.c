
#include <unistd.h>
#include <pthread.h>
#include "ts_airfile.h"
#include "ts_airfile_def.h"

#if (CONFIG_TS_AIRFILE_DESC_NETTV_DESC)

#include "ts_demuxer/ite_ts_demuxer.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define ENABLE_FILE_SIMULATION       0
// #define MAX_DEMOD_SUPPORT_CNT       2

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
    const uint32_t      tsi_index;

}DEMOD_BASE_INFO;


//=============================================================================
//                  Global Data Definition
//=============================================================================
static DEMOD_BASE_INFO  g_demod_base_info[] =
{
    {"Luffy",   false, 0},
    {"Chopper", false, 1},
};

//=============================================================================
//                  Private Function Definition
//=============================================================================
static TSAF_ERR
nettv_Init(
    TSAF_CRTL_INFO  *pCtrlInfo,
    TSAF_INIT_PARAM *pInitParam,
    void            *extraDat)
{
    TSAF_ERR            result = TSAF_ERR_OK;
    TSD_PRE_SET_INFO    presetInfo = {0};

    do{
        int             i, countryId, validCountryCnt, rc;
        char            *countryName = 0;
        TSAF_DEMOD_TYPE act_demod_type = TSAF_ACTION_DEMOD_ID;

        presetInfo.demod_id       = (uint32_t)(-1);
        pCtrlInfo->ts_country_cnt = (-1);

        // -------------------------
        // create tsd handle
        presetInfo.tsdOutMode   = TSD_OUT_TS_BY_PASS;
        switch( act_demod_type )
        {
            case TSAF_DEMOD_OMEGA:  presetInfo.tsdDemodType = TSD_DEMOD_OMEGA;    break;
            case TSAF_DEMOD_IT9135: presetInfo.tsdDemodType = TSD_DEMOD_IT9135;   break;
            case TSAF_DEMOD_IT9137: presetInfo.tsdDemodType = TSD_DEMOD_IT9137;   break;
        }

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

        tsd_CreateHandle((TSD_HANDLE**)&pInitParam->extra_handle, &presetInfo, 0);
        if( pInitParam->extra_handle == 0 )
        {
            result = TSAF_ERR_ALLOCATE_FAIL;
            tsaf_msg_ex(TSAF_MSG_TYPE_ERR, "create tsd handle fail (demodId=%d, tsiId=%d) !!\n", presetInfo.demod_id, presetInfo.tsi_id);
            break;
        }

        // -------------------------
        // get support country
        validCountryCnt = 0;
        for(countryId = TSD_COUNTRY_AUSTRALIA; countryId < TSD_COUNTRY_CNT; countryId++)
        {
            tsd_Get_CountryName((TSD_HANDLE*)pCtrlInfo->extra_handle, countryId, &countryName, 0);
            if( countryName )
            {
                pCtrlInfo->ts_country_table[validCountryCnt].countryId = countryId;
                strncpy(pCtrlInfo->ts_country_table[validCountryCnt].country_name, countryName, strlen(countryName));
                validCountryCnt++;
                //printf("%s, %s\n", countryName, demodInfo.ts_country_table[j].country_name);
            }
        }
        pCtrlInfo->ts_country_cnt = validCountryCnt;

        // -------------------------
        // Create thread
        rc = pthread_create(&pInitParam->thread, NULL, pInitParam->threadFunc, (void *)pInitParam);
        if( rc )
        {
            tsaf_msg_ex(TSAF_MSG_TYPE_ERR, "ERROR; pthread_create() fail %d\n", rc);
        }

        pCtrlInfo->thread = pInitParam->thread;

        usleep(5000);

    }while(0) ;

    return result;
}

static TSAF_ERR
nettv_deInit(
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
nettv_Open(
    TSAF_CRTL_INFO  *pCtrlInfo,
    uint32_t        service_index,
    void            *extraDat)
{
    // do change service
    TSAF_ERR     result = TSAF_ERR_OK;
    return result;
}

static TSAF_ERR
nettv_Close(
    TSAF_CRTL_INFO  *pCtrlInfo,
    void            *extraDat)
{
    TSAF_ERR     result = TSAF_ERR_OK;
    // stop cacne

    // reset demod status
    tsd_Control((TSD_HANDLE*)pCtrlInfo->extra_handle, TSD_CTRL_SET_DEMOD_STATUS, (uint32_t*)TSD_DEMOD_STATUS_IDLE, 0);


    // tsd_Control(pCtrlInfo->pHTsd, TSD_CTRL_RESET_ACT_INFO, 0, 0);

    return result;
}

static int
nettv_readData(
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
nettv_get_inSrc_func(
    TSAF_CRTL_INFO  *pCtrlInfo,
    pFunc_inSrc     *ppFunc,
    void            *extraDat)
{
    TSAF_ERR     result = TSAF_ERR_OK;
    return result;
}

static TSAF_ERR
nettv_changeServic(
    TSAF_CRTL_INFO  *pCtrlInfo,
    void            *extraDat)
{
    TSAF_ERR     result = TSAF_ERR_OK;
    return result;
}

static TSAF_ERR
nettv_scanFreq(
    TSAF_CRTL_INFO  *pCtrlInfo,
    void            *extraDat)
{
    TSAF_ERR        result = TSAF_ERR_OK;
    TSD_SCAN_PARAM  tsdScanParam = {0};

    tsd_Control((TSD_HANDLE*)pCtrlInfo->extra_handle, TSD_CTRL_SET_DEMOD_STATUS, (uint32_t*)TSD_DEMOD_STATUS_RUNNING, 0);

    switch( pCtrlInfo->scanParam.scan_type )
    {
        case TSAF_SCAN_AUTO:
            //----------------------------------------
            // demod Acquire Channel
            #if _MSC_VER && (ENABLE_FILE_SIMULATION)
                _scan_channel((TSD_HANDLE*)pCtrlInfo->extra_handle, pCtrlInfo->demod_id);
            #else

                tsdScanParam.countryId     = pCtrlInfo->scanParam.country_id;
                tsdScanParam.scanFrequency = 0;
                tsdScanParam.bandwidth     = 0;
                printf(" \n\n scan channel country: %d, freq=%d, bw=%d\n",
                            tsdScanParam.countryId, tsdScanParam.scanFrequency, tsdScanParam.bandwidth);

                tsd_Scan_Channel((TSD_HANDLE*)pCtrlInfo->extra_handle, TSD_FREQ_SCAN_AUTO, &tsdScanParam, 0);
            #endif

            // tsd_Change_Service(pCtrlInfo->pHTsd, 0, 0);
            break;

        case TSAF_SCAN_MANUAL:

                tsdScanParam.scanFrequency = pCtrlInfo->scanParam.start_freq;
                tsdScanParam.bandwidth     = pCtrlInfo->scanParam.bandwidth;
                printf(" \n\n scan channel country: %d, freq=%d, bw=%d\n",
                            tsdScanParam.countryId, tsdScanParam.scanFrequency, tsdScanParam.bandwidth);

                tsd_Scan_Channel((TSD_HANDLE*)pCtrlInfo->extra_handle, TSD_FREQ_SCAN_MANUAL, &tsdScanParam, 0);
            break;
    }
    return result;
}

static TSAF_ERR
nettv_control(
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
TS_AIRFILE_DESC  TS_AIRFILE_DESC_nettv_desc =
{
    "ts air file NetTv",      // char        *name;
    0,                        // struct _TS_AIRFILE_DESC_TAG  *next;
    TS_AIRFILE_ID_NETTV,      // TS_AIRFILE_TYPE_ID           id;
    0,                        // void        *privInfo;

    // // operator
    nettv_Init,               // int     (*taf_init)(TSAF_CRTL_INFO *pCtrlInfo, TSAF_INIT_PARAM *pInitParam, void *extraData);
    nettv_deInit,             // int     (*taf_deinit)(TSAF_CRTL_INFO *pCtrlInfo, void *extraData);

    nettv_Open,               // int     (*taf_open)(TSAF_CRTL_INFO *pCtrlInfo, void *extraData);
    nettv_Close,              // int     (*taf_close)(TSAF_CRTL_INFO *pCtrlInfo, void *extraData);
    nettv_readData,           // int     (*taf_read)(TSAF_CRTL_INFO *pCtrlInfo, void *extraData);
    0,                        // int     (*taf_write)(TSAF_CRTL_INFO *pCtrlInfo, void *extraData);
    nettv_get_inSrc_func,     // int     (*taf_get_inSrc_func)(TSAF_CRTL_INFO *pCtrlInfo, pFunc_inSrc *ppFunc, void *extraData);
    // nettv_changeServic,       // int     (*taf_changeService)(TSAF_CRTL_INFO *pCtrlInfo, void *extraData);
    nettv_scanFreq,           // int     (*taf_scanFreq)(TSAF_CRTL_INFO *pCtrlInfo, void *extraData);
    nettv_control,            // int     (*taf_control)(TSAF_CRTL_INFO *pCtrlInfo, uint32_t cmd, uint32_t *value, void *extraData);
};

#else

TS_AIRFILE_DESC  TS_AIRFILE_DESC_nettv_desc =
{
    "ts air file NetTv",      // char        *name;
    0,                        // struct _TS_AIRFILE_DESC_TAG  *next;
    TS_AIRFILE_ID_NETTV,      // TS_AIRFILE_TYPE_ID           id;
    0,                        // void        *privInfo;
};
#endif

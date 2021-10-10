
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "ts_airfile.h"
#include "ts_airfile_def.h"

#if (CONFIG_TS_AIRFILE_DESC_AGGRE_GATEWAY_DESC) && defined(CFG_RISC_TS_DEMUX_PLUGIN)

// rtsp://192.168.0.104/channel_0.01.airts

#include <fcntl.h>
#include <sys/ioctl.h>
#include "ite/itp.h"
#include "ite/ite_risc_ts_demux.h"
#include "iniparser/iniparser.h"
#include "ts_extractor/ite_ts_extractor.h"
//=============================================================================
//                Constant Definition
//=============================================================================
#define SW_SIMULATION_ON                     0

#define AGGRE_DEF_MAX_PES_SAMPLE_BUF_NUM        6// 6
#define AGGRE_DEF_PES_SAMPLE_BUF_SIZE           (230 << 10) // KB
#define AGGRE_GATEWAY_MAX_SERVICE_PRE_CAMERA    5
#define AGGRE_CMD_PKT_PID                       0x201

#if (CFG_AGGRE_SUPPORT_COUNT > 0)
    #if (CFG_AGGRE_SUPPORT_COUNT > 2)
        #define AGGRE_GATEWAY_MAX_TS_INPUT_CNT         2
    #else
        #define AGGRE_GATEWAY_MAX_TS_INPUT_CNT         CFG_AGGRE_SUPPORT_COUNT // 2
    #endif
#else
    #define AGGRE_GATEWAY_MAX_TS_INPUT_CNT         1
#endif

#if defined(CFG_AGGRE_ENABLE)
    #define AGGRE_GATEWAY_DISABLE_AGGRE      false
#else
    #define AGGRE_GATEWAY_DISABLE_AGGRE      true
#endif

#define INI_FILE_PATH                           CFG_PRIVATE_DRIVE ":/gateway.ini"
//=============================================================================
//                Macro Definition
//=============================================================================
#if (_MSC_VER)
    #define iteTsDemux_DisableChannel(a)    0
    #define ithFlushMemBuffer()             0
    #define iteTsDemux_EnableChannel(a)     0
    static inline CHANNEL_INFO_ARRAY* iteTsDemux_GetChannelInfoArrayHandle()
    {
        static CHANNEL_INFO_ARRAY   chnnl_info_array = {0};
        return &chnnl_info_array;
    }
#endif

#define _SET_DEMOD_ID(aggre_idx, demod_idx)     ((((aggre_idx) & 0xFF) << 16) | (((demod_idx) & 0xFFFF)))
#define _GET_AGGRE_IDX(demod_id)                (((demod_id) >> 16) & 0xFF)
#define _GET_DEMOD_IDX(demod_id)                ((demod_id) & 0xFFFF)
//=============================================================================
//                Structure Definition
//=============================================================================
/**
 * ini file item info
 **/
typedef struct USER_INI_ITEM_T
{
    char    *pItem_name;
    char    *pDef_value;

}USER_INI_ITEM;


/**
 * user arguments
 **/
typedef struct USER_ARG_T
{
#define USER_ARG_THREAD_SETUP      0xAE0

    uint32_t        type;

    union{
        struct{
            TSE_HANDLE      *pHTse;
            bool            *pBCancel_thread;
            uint32_t        port_index;
        }thread_setup;
    }arg;

}USER_ARG;
/**
 * aggregation module attribute for user layer
 **/
typedef struct USER_AGGRE_ATTR_T
{
    TSE_AGGRE_TYPE       aggre_type;
    TSE_AGGRE_BUS_TYPE   bus_type;

    uint32_t             aggre_index;
    uint32_t             i2c_addr;

    uint32_t             tsi_index;

}USER_AGGRE_ATTR;

/**
 * demod module attribute for user layer
 **/
typedef struct USER_DEMOD_ATTR_T
{
    uint32_t            aggre_index;
    bool                bUsed;
    TSE_DEMOD_ATTR      demod_attr;

    uint32_t            frequency;
    uint32_t            bandwidth;

    int                 hLed;

}USER_DEMOD_ATTR;

/**
 * private data
 **/
typedef struct USER_PRIV_DATA_T
{
    uint32_t        demod_port_index;

    bool            bScan_ready;

    TSE_HANDLE      *pHTse;

    bool            bStart_cache;
    bool            bCancel_thread;

    // for RISC demux plug-in
    CHANNEL_INFO    *ptChannelInfo;
    uint8_t         *cache_head_ptr;
    uint8_t         *cache_tail_ptr;
    uint32_t        cache_buf_size;
    uint32_t        cache_write_idx; // offset of start pointer

}USER_PRIV_DATA;

/**
 * ts input cotrol info for mapping tse handle
 **/
typedef struct TS_INPUT_CTRL_INFO_T
{
    uint32_t            ref_cnt;
    pthread_t           thread_handle;

    TSE_HANDLE          *pHTse;
    bool                bCancel_thread;

}TS_INPUT_CTRL_INFO;

/**
 * camera demod attribute
 **/
typedef struct TS_CAM_SRVC_ATTR_T
{
    uint32_t      port_id;
    uint32_t      video_pid;

    uint32_t      width;
    uint32_t      height;
    uint32_t      srvc_buf_size; // demod buffer

    // one service buffer size (srvc_stream_buf_num*srvc_stream_buf_size)
    uint32_t      srvc_stream_buf_num;
    uint32_t      srvc_stream_buf_size;

}TS_CAM_SRVC_ATTR;

//=============================================================================
//                Global Data Definition
//=============================================================================

/**
 * global aggre module attribute
 **/
static USER_AGGRE_ATTR g_def_aggre_attr[AGGRE_GATEWAY_MAX_TS_INPUT_CNT] =
{
#if (SW_SIMULATION_ON)
    {TSE_AGGRE_TYPE_ENDEAVOUR, TSE_AGGRE_BUS_I2C, 0, 0x68, 0},
#else
#define TSE_AGGRE_ATTR_SET(aggre_chip,aggre_bus_type,aggre_index,aggre_i2c_addr,linked_tsi_index)\
                 {aggre_chip, aggre_bus_type, aggre_index, aggre_i2c_addr, linked_tsi_index},

#include "ts_extractor/ts_extractor_attr_set.h"
#endif
};

/**
 * global demod attribute
 **/
static USER_DEMOD_ATTR  g_def_demod_attr[TSE_MAX_DEMOD_PER_AGGRE] =
{
#if (SW_SIMULATION_ON)
    {0, false, {0, TSE_DEMOD_BUS_I2C, TSE_DEMOD_TYPE_UNKNOW, 0x38}, 533000, 6000},
#else
#define TSE_DEMOD_ATTR_SET(demod_chip,attached_aggre_id,demod_index,demod_bus_type,demod_i2c_addr,linked_aggre_port_idx)\
                 {attached_aggre_id, false, {demod_index, demod_bus_type, demod_chip, demod_i2c_addr, linked_aggre_port_idx}, 533000, 6000},

#include "ts_extractor/ts_extractor_attr_set.h"
#endif
};

static const TS_CAM_SRVC_ATTR const g_def_srvc_attr[AGGRE_GATEWAY_MAX_SERVICE_PRE_CAMERA] =
{
#if 0
    {0, 0x641, 1920, 1080, (3669948), 6, (650 <<10)}, {0, 0x651, 720, 480, (3669948), 6, (180 <<10)}, {0, 0x7d1, 1280, 720, (3669948), 6, (550 <<10)}, {0, 0x7d3, 720, 480, (3669948), 6, (180 <<10)}, {0, 0x7d5, 352, 288, (3669948), 6, (80 <<10)},
#else
    // ccHDtv spec v0.11 definition, and final memory setting
    {0, 0x111, 1920, 1080, (2097140), 6, (580 <<10)},
    {0, 0x121, 1280, 720, (2097140), 6, (550 <<10)},
    {0, 0x131, 720, 480, (2097140), 6, (180 <<10)},
    {0, 0x141, 320, 240, (2097140), 6, (80 <<10)},
#endif
};

static TS_INPUT_CTRL_INFO   g_ts_input_ctrl_info[AGGRE_GATEWAY_MAX_TS_INPUT_CNT] = {0};
static bool                 g_bUpdate_ini_file = true;
//=============================================================================
//                Private Function Definition
//=============================================================================
#if (_MSC_VER)
#define CACHE_BUF_SIZE      (300 << 10)
static uint32_t
_file_ts_recv_init(
    TSE_RECV_OPR    *pTs_recv_opr,
    void            *extraData)
{
    uint32_t    result = 0;
    uint32_t    tsi_idx = 0;

    if( pTs_recv_opr )
    {
        uint32_t    index = 0;
        uint8_t     *pTsi_buf = 0;
        FILE        *fp = 0;

        if( !(fp = fopen("./CTV_MyLife_tw_0001.ts", "rb")) )
        {
            printf("open file fail !!");
            while(1);
        }

        //if( !(fp = fopen("./aggre_ts_tag1-2-3-4_length_1.ts", "rb")) ) _err("open file fail !!");

        index = 0;
        if( !(pTsi_buf = tsaf_malloc(CACHE_BUF_SIZE)) )
        {
            printf("malloc fail !!");
            while(1);
        }

        pTs_recv_opr->privData.file.index           = index;
        pTs_recv_opr->privData.file.fin[index]      = (void*)fp;
        pTs_recv_opr->privData.file.pTsi_buf[index] = pTsi_buf;
    }
    return result;
}

static uint32_t
_file_ts_recv_deinit(
    TSE_RECV_OPR    *pTs_recv_opr,
    void            *extraData)
{
    uint32_t    result = 0;
    uint32_t    tsi_idx = 0;

    if( pTs_recv_opr )
    {
        uint32_t    index = pTs_recv_opr->privData.file.index;
        uint8_t     *pTsi_buf = 0;
        FILE        *fp = 0;

        pTsi_buf = pTs_recv_opr->privData.file.pTsi_buf[index];
        if( pTsi_buf )
        {
            free(pTsi_buf);
            pTs_recv_opr->privData.file.pTsi_buf[index] = 0;
        }

        fp = (FILE*)pTs_recv_opr->privData.file.fin[index];
        fclose(fp);
    }
    return result;
}

static uint32_t
_file_ts_recv_get_data(
    TSE_RECV_OPR    *pTs_recv_opr,
    uint8_t         **ppSampleAddr,
    uint32_t        *pSampleSize,
    void            *extraData)
{
    uint32_t    result = 0;

    if( pTs_recv_opr && ppSampleAddr && pSampleSize )
    {
        uint32_t    index = pTs_recv_opr->privData.file.index;
        uint8_t     *pTsi_buf = 0;
        uint32_t    stream_data_size = (rand() % CACHE_BUF_SIZE);
        uint32_t    valid_size = 0;
        FILE        *fp = 0;

        fp = (FILE*)pTs_recv_opr->privData.file.fin[index];
        pTsi_buf = pTs_recv_opr->privData.file.pTsi_buf[index];

        valid_size = fread(pTsi_buf, 1, stream_data_size, fp);
        if( !valid_size )
        {
            fseek(fp, 0, SEEK_SET);
            valid_size = fread(pTsi_buf, 1, stream_data_size, fp);
        }

        (*ppSampleAddr) = pTsi_buf;
        (*pSampleSize)  = valid_size;
    }
    return result;
}

static uint32_t
_reset_ts_recv_opr(
    TSE_RECV_OPR    *pRecv_opr,
    void            *extraData)
{
    uint32_t    result = 0;

    pRecv_opr->ts_recv_type = TSE_RECV_TYPE_FILE;

    pRecv_opr->ts_recv_init     = _file_ts_recv_init;
    pRecv_opr->ts_recv_deinit   = _file_ts_recv_deinit;
    pRecv_opr->ts_recv_turn_on  = 0;
    pRecv_opr->ts_recv_turn_off = 0;
    pRecv_opr->ts_recv_get_data = _file_ts_recv_get_data;

    return result;
}
#else
    #define _reset_ts_recv_opr   0
#endif

static uint32_t
_aggre_set_resolution_with_vpid(
    TSE_USER_ARG    *pUser_arg,
    void            *extraData)
{
    do{
        uint32_t    i = 0, j = 0;
        uint32_t    port_index = 0;

        if( !pUser_arg )        break;

        port_index = pUser_arg->arg.v_info.port_index;

        pUser_arg->arg.v_info.width         = 0;
        pUser_arg->arg.v_info.height        = 0;
        pUser_arg->arg.v_info.srvc_buf_size = 0;

        if( port_index >= TSE_MAX_DEMOD_PER_AGGRE )      break;

        for(i = 0; i < AGGRE_GATEWAY_MAX_SERVICE_PRE_CAMERA; i++)
        {
            if( pUser_arg->arg.v_info.v_pid == g_def_srvc_attr[i].video_pid )
            {
                pUser_arg->arg.v_info.width         = g_def_srvc_attr[i].width;
                pUser_arg->arg.v_info.height        = g_def_srvc_attr[i].height;
                pUser_arg->arg.v_info.srvc_buf_size = g_def_srvc_attr[i].srvc_buf_size;
                break;
            }
        }
    }while(0);

    return 0;
}

static void*
_aggre_extract_proc(
    void* arg)
{
    USER_ARG      *pArg_info = (USER_ARG*)arg;
    TSE_HANDLE    *pHTse = 0;
    bool          *pBCancel_thread = 0;

    pHTse           = pArg_info->arg.thread_setup.pHTse;
    pBCancel_thread = pArg_info->arg.thread_setup.pBCancel_thread;

    while( (*pBCancel_thread) == false )
    {
        tse_Action_Proc(pHTse, 0);

        usleep(20000);
    }

    tsaf_msg_ex(1, " exit thread %s() !", __FUNCTION__);
    pthread_exit(0);
    return 0;
}


static void*
_aggre_update_risc_w_idx(
    void* arg)
{
    TSAF_CRTL_INFO      *ptCtrlInfo = (TSAF_CRTL_INFO*)arg;
    TSE_HANDLE          *pHTse = 0;
    USER_PRIV_DATA      *pUser_priv_data = 0;
    uint32_t            demod_port_index = 0;
    TSE_SAMPLE_INFO     tse_sample_info = {0};

    if( !ptCtrlInfo || !ptCtrlInfo->privData )
    {
        tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " err ! Null pointer (0x%x, 0x%x)!\n",
                    ptCtrlInfo, ptCtrlInfo->privData);
        pthread_exit(0);
        return 0;
    }

    pUser_priv_data = (USER_PRIV_DATA*)ptCtrlInfo->privData;

    pHTse            = pUser_priv_data->pHTse;
    demod_port_index = pUser_priv_data->demod_port_index;

    tse_sample_info.port_idx     = demod_port_index;
    tse_sample_info.customer_idx = 0;
    tse_sample_info.service_idx  = 0;

    tsaf_msg(1, "  --- create %s(port=%d) thread !\n\n", __FUNCTION__, demod_port_index);

    while( pUser_priv_data->bCancel_thread == false )
    {
        if( pUser_priv_data->bStart_cache == false )
        {
            usleep(100000);
            continue;
        }

        tse_sample_info.bufLength = 0;

        tse_Get_Service_Sample(pHTse, &tse_sample_info, 0);
        if( tse_sample_info.bufLength && tse_sample_info.pBufAddr )
        {
            CHANNEL_INFO    *ptChannelInfo = (CHANNEL_INFO*)pUser_priv_data->ptChannelInfo;

            //------------------------------------
            // update risc write index
            if( ptChannelInfo )
            {
                if( (tse_sample_info.pBufAddr + tse_sample_info.bufLength) < pUser_priv_data->cache_tail_ptr )
                    pUser_priv_data->cache_write_idx += tse_sample_info.bufLength;
                else
                    pUser_priv_data->cache_write_idx = 0;

                ptChannelInfo->tsBufferWriteIdx = pUser_priv_data->cache_write_idx;

                //tsaf_msg(1, "  port %d get sampe(0x%x, %d)\n",
                //        demod_port_index,
                //        tse_sample_info.pBufAddr, tse_sample_info.bufLength);

                ithFlushMemBuffer();
            }
        }

        usleep(6000);
    }

    tsaf_msg_ex(1, " exit thread %s(), demod_port_index=%d !", __FUNCTION__, demod_port_index);
    pthread_exit(0);
    return 0;
}

static bool
_aggre_gateway_InsertChannelPid(
    CHANNEL_INFO    *ptChannelInfo,
    uint32_t        pid,
    uint32_t        pesSamplekSize,
    uint32_t        pesSampleCount,
    uint32_t        *pesIndex)
{
    bool        result = false;
    PID_INFO    *ptPidInfo = 0;

    do{
        if( ptChannelInfo->validPidCount >= MAX_PID_COUNT_PER_CHANNEL )
            break;

        ptPidInfo = &ptChannelInfo->tPidInfo[ptChannelInfo->validPidCount];

        ptPidInfo->pid                 = pid;
        ptPidInfo->pOutPesSampleSize   = pesSamplekSize;
        ptPidInfo->validPesSampleCount = pesSampleCount;

        if( ptPidInfo->pOutPesBuffer )      free(ptPidInfo->pOutPesBuffer);

        ptPidInfo->pOutPesBuffer = tsaf_malloc(pesSamplekSize * pesSampleCount);
        if( !ptPidInfo->pOutPesBuffer )     break;

        ptChannelInfo->validPidCount++;
        result = true;

    }while(0);

    return result;
}

static int
_aggre_gateway_cache_StopThread(
    uint32_t            index,
    TSAF_CRTL_INFO      *pTsafCtrlInfo)
{
    USER_PRIV_DATA      *pUser_priv_data = (USER_PRIV_DATA*)pTsafCtrlInfo->privData;

    _tsaf_trace_enter(TSAF_MSG_TYPE_TRACE_CACHE_BUF, "\n");

    do{
        if( !pUser_priv_data )
        {
            tsaf_msg_ex(TSAF_MSG_TYPE_ERR, "Null Pointer !");
            break;
        }

        pUser_priv_data->bCancel_thread = true;
    }while(0);

    _tsaf_trace_leave(TSAF_MSG_TYPE_TRACE_CACHE_BUF);
    return 0;
}

static int
_aggre_gateway_cache_Stop(
    uint32_t            index,
    TSAF_CRTL_INFO      *pTsafCtrlInfo)
{
    USER_PRIV_DATA      *pUser_priv_data = (USER_PRIV_DATA*)pTsafCtrlInfo->privData;

    _tsaf_trace_enter(TSAF_MSG_TYPE_TRACE_CACHE_BUF, "\n");

    do{
        if( !pUser_priv_data )
        {
            tsaf_msg_ex(TSAF_MSG_TYPE_ERR, "Null Pointer !");
            break;
        }

        pUser_priv_data->bStart_cache = false;
        tse_Set_Service_Receive(pUser_priv_data->pHTse, pUser_priv_data->demod_port_index, -1, false, 0);
    }while(0);

    _tsaf_trace_leave(TSAF_MSG_TYPE_TRACE_CACHE_BUF);
    return 0;
}

static int
_aggre_gateway_cache_Start(
    uint32_t            index,
    TSAF_CRTL_INFO      *pTsafCtrlInfo)
{
    USER_PRIV_DATA      *pUser_priv_data = (USER_PRIV_DATA*)pTsafCtrlInfo->privData;

    _tsaf_trace_enter(TSAF_MSG_TYPE_TRACE_CACHE_BUF, "\n");

    do{
        TSE_USER_INFO       tse_user_info = {0};
        uint32_t            demod_num = 0;
        uint32_t            demod_port_idx = 0;
        uint32_t            act_aggre_idx = 0;
        uint32_t            channelId = 0;

        if( !pUser_priv_data )
        {
            tsaf_msg_ex(TSAF_MSG_TYPE_ERR, "Null Pointer !");
            break;
        }

        demod_num      = ARRAY_SIZE(g_def_demod_attr);
        demod_port_idx = _GET_DEMOD_IDX(pTsafCtrlInfo->demod_id);
        act_aggre_idx  = _GET_AGGRE_IDX(pTsafCtrlInfo->demod_id);

        tse_Set_Service_Receive(pUser_priv_data->pHTse, demod_port_idx, 0, true, 0);
        usleep(200000); // wait hTspd create

        { // set cmd service pid
            TSE_CMD_PKT_INFO    tse_cmd_pkt_info = {0};

            tse_cmd_pkt_info.cmd_pkt_pid = AGGRE_CMD_PKT_PID;
            tse_Attach_Cmd_Pkt_Recv(pUser_priv_data->pHTse, demod_port_idx, &tse_cmd_pkt_info, 0);
        }

        tse_Get_Service_Info(pUser_priv_data->pHTse, &tse_user_info, 0);
        if( index != (-1) &&
            tse_user_info.demodInfo[demod_port_idx].total_services )
        {
            uint32_t            i = 0;
            uint32_t            channelId = 0;
            TSE_SERVICE_INFO    *pServiceInfo = 0;
            CHANNEL_INFO_ARRAY  *ptChannelInfoArray = (CHANNEL_INFO_ARRAY*)iteTsDemux_GetChannelInfoArrayHandle();
            CHANNEL_INFO        *ptChannelInfo = 0;
            uint32_t            curBufReadPtr = 0;

            channelId = (demod_num * act_aggre_idx) + demod_port_idx;
            if( channelId >= MAX_CHANNEL_COUNT )
            {
                tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " channel Id(%d) out support number(%d) ! \n",
                            channelId, MAX_CHANNEL_COUNT);
                break;
            }

            iteTsDemux_DisableChannel(channelId);

            pUser_priv_data->cache_write_idx = 0;
            pUser_priv_data->cache_buf_size  = tse_user_info.demodInfo[demod_port_idx].serviceInfo[0].ring_buf_size;
            pUser_priv_data->cache_head_ptr  = tse_user_info.demodInfo[demod_port_idx].serviceInfo[0].pRing_buf;
            pUser_priv_data->cache_tail_ptr  = pUser_priv_data->cache_head_ptr + pUser_priv_data->cache_buf_size;

            //-------------------------------
            //Get free risc share channel info
            pUser_priv_data->ptChannelInfo = ptChannelInfo = &ptChannelInfoArray->tChannelArray[channelId];

            if( tse_user_info.demodInfo[demod_port_idx].serviceInfo[0].pRing_buf )
            {
                uint32_t pesIndex = 0;
                // set ring buf info
                ptChannelInfo->pInputTsBuffer  = tse_user_info.demodInfo[demod_port_idx].serviceInfo[0].pRing_buf; // ring buf start pointer
                ptChannelInfo->tsBufferSize    = tse_user_info.demodInfo[demod_port_idx].serviceInfo[0].ring_buf_size; // ring buf size
                ptChannelInfo->tsBufferReadIdx = pUser_priv_data->cache_write_idx; // offset with start pointer
                ithFlushMemBuffer();

                // set pid info
                for(i = 0; i < tse_user_info.demodInfo[demod_port_idx].total_services; i++)
                {
                    uint32_t        k = 0;
                    uint32_t        pesSamplekSize = 0;
                    uint32_t        pesSampleCount = 0;

                    pServiceInfo = &tse_user_info.demodInfo[demod_port_idx].serviceInfo[i];

                    for(k = 0; k < AGGRE_GATEWAY_MAX_SERVICE_PRE_CAMERA; k++)
                    {
                        if( g_def_srvc_attr[k].video_pid == pServiceInfo->video_pid )
                        {
                            pesSamplekSize = g_def_srvc_attr[k].srvc_stream_buf_size;
                            pesSampleCount = g_def_srvc_attr[k].srvc_stream_buf_num;
                            break;
                        }
                    }

                    if( !pesSamplekSize || !pesSampleCount )
                    {
                        pesSamplekSize = AGGRE_DEF_PES_SAMPLE_BUF_SIZE;
                        pesSampleCount = AGGRE_DEF_MAX_PES_SAMPLE_BUF_NUM;
                    }

                    if( pServiceInfo->videoType != 2 &&
                        _aggre_gateway_InsertChannelPid(ptChannelInfo, pServiceInfo->video_pid,
                                                        pesSamplekSize, pesSampleCount, &pesIndex) )
                        tsaf_msg(1, "pes index: %u\n", pesIndex);
                }

                for(i = 0; i < ptChannelInfo->validPidCount; i++)
                    tsaf_msg(1, "pid: 0x%X, buf: 0x%X\n",
                            ptChannelInfo->tPidInfo[i].pid, ptChannelInfo->tPidInfo[i].pOutPesBuffer);
            }
            iteTsDemux_EnableChannel(channelId);
        }

        pUser_priv_data->bStart_cache = true;
    }while(0);

    _tsaf_trace_leave(TSAF_MSG_TYPE_TRACE_CACHE_BUF);
    return 0;
}


static uint32_t
_aggre_gateway_reset_attr(
    uint32_t    aggre_num,
    uint32_t    demod_num)
{
    uint32_t        result = 0;
    uint32_t        i = 0;

    char            def_value[32] = {0};
    USER_INI_ITEM   ini_setting_aggre[2][2] =
    {
        {{"aggre_0:i2c_addr", ""}, {"aggre_0:tsi_index", ""}},
        {{"aggre_1:i2c_addr", ""}, {"aggre_1:tsi_index", ""}},
    };

    USER_INI_ITEM   ini_setting_demod[8][3] =
    {
        {{"demod_0:aggre_index", ""}, {"demod_0:demod_index", ""}, {"demod_0:i2c_addr", ""}},
        {{"demod_1:aggre_index", ""}, {"demod_1:demod_index", ""}, {"demod_1:i2c_addr", ""}},
        {{"demod_2:aggre_index", ""}, {"demod_2:demod_index", ""}, {"demod_2:i2c_addr", ""}},
        {{"demod_3:aggre_index", ""}, {"demod_3:demod_index", ""}, {"demod_3:i2c_addr", ""}},
        {{"demod_4:aggre_index", ""}, {"demod_4:demod_index", ""}, {"demod_4:i2c_addr", ""}},
        {{"demod_5:aggre_index", ""}, {"demod_5:demod_index", ""}, {"demod_5:i2c_addr", ""}},
        {{"demod_6:aggre_index", ""}, {"demod_6:demod_index", ""}, {"demod_6:i2c_addr", ""}},
        {{"demod_7:aggre_index", ""}, {"demod_7:demod_index", ""}, {"demod_7:i2c_addr", ""}},
    };

    if( g_bUpdate_ini_file == false )      return result;

    do{
        dictionary      *pIni_handle = 0;

        //read demod info from NOR
        pIni_handle = iniparser_load(INI_FILE_PATH);
        if( !pIni_handle )
        {
            tsaf_msg_ex(TSAF_MSG_TYPE_ERR, "cannot load ini file: %s\n", INI_FILE_PATH);
            break;
        }

        if( aggre_num > 2 )
        {
            tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " err, aggre_num (%d) > support (2)\n", aggre_num);
            break;
        }
        if( demod_num > 4 )
        {
            tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " err, demod_num (%d) > support (4)\n", demod_num);
            break;
        }

        // read aggre attr
        for(i = 0; i < aggre_num; i++)
        {
            sprintf(def_value, "%d", g_def_aggre_attr[i].i2c_addr);
            g_def_aggre_attr[i].i2c_addr  =
                atoi(iniparser_getstring(pIni_handle, ini_setting_aggre[i][0].pItem_name, def_value));

            sprintf(def_value, "%d", g_def_aggre_attr[i].tsi_index);
            g_def_aggre_attr[i].tsi_index =
                atoi(iniparser_getstring(pIni_handle, ini_setting_aggre[i][1].pItem_name, def_value));
        }

        // read demod attr
        for(i = 0; i < demod_num; i++)
        {
            sprintf(def_value, "%d", g_def_demod_attr[i].aggre_index);
            g_def_demod_attr[i].aggre_index =
                atoi(iniparser_getstring(pIni_handle, ini_setting_demod[i][0].pItem_name, def_value));

            sprintf(def_value, "%d", g_def_demod_attr[i].demod_attr.demod_idx);
            g_def_demod_attr[i].demod_attr.demod_idx =
                atoi(iniparser_getstring(pIni_handle, ini_setting_demod[i][1].pItem_name, def_value));

            sprintf(def_value, "%u", g_def_demod_attr[i].demod_attr.demod_i2c_addr);
            g_def_demod_attr[i].demod_attr.demod_i2c_addr =
                strtoul(iniparser_getstring(pIni_handle, ini_setting_demod[i][2].pItem_name, def_value), NULL, 16);
        }

        iniparser_freedict(pIni_handle);

        g_bUpdate_ini_file = false;
    }while(0);

#if 0 // show attr msg
    tsaf_msg(1, "\n aggre attr:\n");
    for(i = 0; i < aggre_num; i++)
        tsaf_msg(1, "  chip_type=0x%x, bus_type=0x%x, aggre_index=%d, i2c_addr=0x%x, tsi_index=%d\n",
                g_def_aggre_attr[i].aggre_type, g_def_aggre_attr[i].bus_type,
                g_def_aggre_attr[i].aggre_index, g_def_aggre_attr[i].i2c_addr,
                g_def_aggre_attr[i].tsi_index);
    tsaf_msg(1, "\n demod attr, %s():\n", __FUNCTION__);
    for(i = 0; i < demod_num; i++)
        tsaf_msg(1, "  aggre_index=%d, demod_idx=%d, bus_type=0x%x, demod_chip_type=0x%x, i2c_addr=0x%x\n\n",
                g_def_demod_attr[i].aggre_index,
                g_def_demod_attr[i].demod_attr.demod_idx,
                g_def_demod_attr[i].demod_attr.bus_type,
                g_def_demod_attr[i].demod_attr.demod_type,
                g_def_demod_attr[i].demod_attr.demod_i2c_addr);
#endif

    return result;
}

static uint32_t
_aggre_gateway_reset_freq(
    uint32_t    demod_num)
{
    uint32_t        result = 0;
    uint32_t        i = 0;

    USER_INI_ITEM   ini_setting[4][2] =
    {
        {{"ts:freq0", "611000"}, {"ts:bandwidth0", "6000"}},
        {{"ts:freq1", "611000"}, {"ts:bandwidth1", "6000"}},
        {{"ts:freq2", "611000"}, {"ts:bandwidth2", "6000"}},
        {{"ts:freq3", "611000"}, {"ts:bandwidth3", "6000"}},
    };

    do{
        dictionary      *pIni_handle = 0;

        //read demod info from NOR
        pIni_handle = iniparser_load(INI_FILE_PATH);
        if( !pIni_handle )
        {
            tsaf_msg_ex(TSAF_MSG_TYPE_ERR, "cannot load ini file: %s\n", INI_FILE_PATH);
            break;
        }

        for(i = 0; i < demod_num; i++)
        {
            g_def_demod_attr[i].frequency =
                atoi(iniparser_getstring(pIni_handle, ini_setting[i][0].pItem_name, ini_setting[i][0].pDef_value));

            g_def_demod_attr[i].bandwidth =
                atoi(iniparser_getstring(pIni_handle, ini_setting[i][1].pItem_name, ini_setting[i][1].pDef_value));
        }

        iniparser_freedict(pIni_handle);

    }while(0);

#if 0 // show attr msg
    tsaf_msg(1, "\n demod attr, %s():\n", __FUNCTION__);
    for(i = 0; i < demod_num; i++)
        tsaf_msg(1, "  aggre_index=%d, demod_idx=%d, bus_type=0x%x, demod_chip_type=0x%x, i2c_addr=0x%x, freq=%d, bw=%d\n\n",
                g_def_demod_attr[i].aggre_index,
                g_def_demod_attr[i].demod_attr.demod_idx,
                g_def_demod_attr[i].demod_attr.bus_type,
                g_def_demod_attr[i].demod_attr.demod_type,
                g_def_demod_attr[i].demod_attr.demod_i2c_addr,
                g_def_demod_attr[i].frequency,
                g_def_demod_attr[i].bandwidth);
#endif

    return result;
}

static uint32_t
_aggre_gateway_scan_report(
    TSE_USER_ARG   *pUser_arg,
    void           *extraData)
{
    bool        *pBScan_ready = 0;
    uint32_t    port_index = 0;

    do{
        pBScan_ready = (bool*)pUser_arg->arg.scan.pTunnelInfo[0];

        port_index = pUser_arg->arg.scan.port_index;
        *pBScan_ready = true;

        tsaf_msg(1, " ------ get scan ready (port = %d)\n", port_index);
    }while(0);

    return 0;
}

static const TSAF_CACHE_INFO g_demod_cache_info =
{
    _aggre_update_risc_w_idx,               // void*    (*cache_BufThread)(void *args);
    _aggre_gateway_cache_StopThread,        // int      (*cache_StopThread)(uint32_t index, struct _TSAF_CRTL_INFO_TAG *pTsafCtrlInfo);
    _aggre_gateway_cache_Stop,              // int      (*cache_Stop)(uint32_t index, struct _TSAF_CRTL_INFO_TAG *pTsafCtrlInfo);
    _aggre_gateway_cache_Start,             // int      (*cache_Start)(uint32_t index, struct _TSAF_CRTL_INFO_TAG *pTsafCtrlInfo);
    0,                                      // int      (*cache_GetData)(uint32_t index, uint8_t *buffer, uint32_t bufferLength, struct _TSAF_CRTL_INFO_TAG *pTsafCtrlInfo);
    0,                  // void     *privData;
};
////////////////////////////////////////////////////////////////
static TSAF_ERR
aggre_gateway_Init(
    TSAF_CRTL_INFO  *pCtrlInfo,
    TSAF_INIT_PARAM *pInitParam,
    void            *extraDat)
{
    TSAF_ERR             result = TSAF_ERR_OK;

    do{
        uint32_t            i = 0, j = 0;
        uint32_t            act_aggre_idx = 0;
        uint32_t            act_demod_idx = 0;
        uint32_t            aggre_num = 0;
        uint32_t            demod_num = 0;
        USER_PRIV_DATA      *pUser_priv_data = 0;
        TSE_INIT_PARAM      tse_init_param = {0};

        aggre_num = ARRAY_SIZE(g_def_aggre_attr);
        demod_num = ARRAY_SIZE(g_def_demod_attr);

        //---------------------------------------------
        // reset aggre/demod attribute with *.ini file
        _aggre_gateway_reset_attr(aggre_num, demod_num);

#if USE_EVB_9079
        // 9079 EVB setting
        tse_init_param.tsi_idx = 1;
#else
        //----------------------------------------------
        // search action aggre/demod attribute
        tse_init_param.tsi_idx = pCtrlInfo->tsi_index;
        for(act_aggre_idx = 0; act_aggre_idx < aggre_num; act_aggre_idx++)
        {
            if( g_def_aggre_attr[act_aggre_idx].tsi_index == pCtrlInfo->tsi_index )
                break;
        }
        if( act_aggre_idx == aggre_num ||
            act_aggre_idx >= AGGRE_GATEWAY_MAX_TS_INPUT_CNT )
        {
            tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " wrong tsi index (%d) !", act_aggre_idx);
            break;
        }
#endif
        //----------------------------------------------
        // map user_demod_index to H/W real demod index
        for(act_demod_idx = 0; act_demod_idx < demod_num; act_demod_idx++)
        {
            if( g_def_demod_attr[act_demod_idx].aggre_index == act_aggre_idx &&
                g_def_demod_attr[act_demod_idx].bUsed == false )
                break;
        }

        if( act_demod_idx == demod_num )
        {
            tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " err ! No free demod can be used !!");
            break;
        }

        //----------------------------------------------
        // start init proc
        if( !g_ts_input_ctrl_info[act_aggre_idx].pHTse )
        {
            USER_ARG        arg_info = {0};

            /**
             * When create tse handle, need to initialize all demods, which be attached at the current aggre_chip
             **/

            // set the right attribute info
            tse_init_param.total_demod = 0;
            for(i = 0; i < demod_num; i++)
            {
                if( g_def_demod_attr[i].aggre_index == act_aggre_idx )
                {
                    uint32_t    cur_demod_idx = g_def_demod_attr[i].demod_attr.demod_idx;

                    if( cur_demod_idx < TSE_MAX_DEMOD_PER_AGGRE )
                    {
                        tse_init_param.demod_attr[cur_demod_idx] = g_def_demod_attr[i].demod_attr;
                        tse_init_param.total_demod++;
                    }
                    else
                        tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " demod index (%d) out support (%d) !", cur_demod_idx, TSE_MAX_DEMOD_PER_AGGRE-1);
                }
            }

            tse_init_param.reset_ts_recv_opr = _reset_ts_recv_opr;

            // create tse handle
            tse_init_param.aggre_type         = g_def_aggre_attr[act_aggre_idx].aggre_type;
            tse_init_param.bSkip_aggre        = AGGRE_GATEWAY_DISABLE_AGGRE;
            tse_init_param.aggre_idx          = g_def_aggre_attr[act_aggre_idx].aggre_index;
            tse_init_param.aggre_bus_type     = TSE_AGGRE_BUS_I2C;
            tse_init_param.aggre_i2c_addr     = g_def_aggre_attr[act_aggre_idx].i2c_addr;
            tse_init_param.ts_split_level     = TSE_SPLIT_DEMOD;
            tse_init_param.get_user_info.func = _aggre_set_resolution_with_vpid;

            tse_CreateHandle(&g_ts_input_ctrl_info[act_aggre_idx].pHTse, &tse_init_param, 0);
            if( !g_ts_input_ctrl_info[act_aggre_idx].pHTse )
            {
                tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " create tse hanlde fail !");
                break;
            }

            // create thread
            g_ts_input_ctrl_info[act_aggre_idx].bCancel_thread = false;
            arg_info.type                             = USER_ARG_THREAD_SETUP;
            arg_info.arg.thread_setup.pHTse           = g_ts_input_ctrl_info[act_aggre_idx].pHTse;
            arg_info.arg.thread_setup.pBCancel_thread = (void*)&g_ts_input_ctrl_info[act_aggre_idx].bCancel_thread;

            pthread_create(&g_ts_input_ctrl_info[act_aggre_idx].thread_handle, NULL, _aggre_extract_proc, (void*)&arg_info);
            usleep(10000); // wait thread create ready

            g_ts_input_ctrl_info[act_aggre_idx].ref_cnt = 1;
        }
        else
        {
            // reference cnt
            g_ts_input_ctrl_info[act_aggre_idx].ref_cnt++;
        }

        g_def_demod_attr[act_demod_idx].bUsed = true;
        pCtrlInfo->demod_id = _SET_DEMOD_ID(act_aggre_idx, g_def_demod_attr[act_demod_idx].demod_attr.demod_idx);

        pCtrlInfo->extra_handle = (void*)g_ts_input_ctrl_info[act_aggre_idx].pHTse;

        //---------------------------------
        // allocate private data
        pUser_priv_data = tsaf_malloc(sizeof(USER_PRIV_DATA));
        if( !pUser_priv_data )
        {
            tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " malloc priv data fail !");
            break;
        }

        memset(pUser_priv_data, 0x0, sizeof(USER_PRIV_DATA));
        pUser_priv_data->pHTse            = g_ts_input_ctrl_info[act_aggre_idx].pHTse;
        pUser_priv_data->demod_port_index = g_def_demod_attr[act_demod_idx].demod_attr.demod_idx;

        pCtrlInfo->privData = (void*)pUser_priv_data;

        // set cache operator info
        pCtrlInfo->cacheInfo = g_demod_cache_info;

#ifdef CFG_LED_ENABLE
        {
            char    tmp_str[16] = {0};
            snprintf(tmp_str, 16, ":led:%d", act_demod_idx);

            g_def_demod_attr[act_demod_idx].hLed = open(tmp_str, O_RDONLY);

            ioctl(g_def_demod_attr[act_demod_idx].hLed, ITP_IOCTL_FLICKER, (void*)200);
            // ioctl(g_def_demod_attr[act_demod_idx].hLed, ITP_IOCTL_ON, NULL);
        }
#endif

        pthread_create(&pCtrlInfo->thread, NULL, _aggre_update_risc_w_idx, (void*)pCtrlInfo);
        usleep(10000); // wait thread create ready

    }while(0);

    return result;
}

static TSAF_ERR
aggre_gateway_deInit(
    TSAF_CRTL_INFO  *pCtrlInfo,
    void            *extraDat)
{
    TSAF_ERR     result = TSAF_ERR_OK;

    do{
        uint32_t        i = 0;
        uint32_t        demod_num = 0;
        uint32_t        act_aggre_idx = 0;
        uint32_t        demod_port_idx = 0;

        demod_num      = ARRAY_SIZE(g_def_demod_attr);
        act_aggre_idx  = _GET_AGGRE_IDX(pCtrlInfo->demod_id);
        demod_port_idx = _GET_DEMOD_IDX(pCtrlInfo->demod_id);

        if( g_ts_input_ctrl_info[act_aggre_idx].ref_cnt == 1 )
        {
            // destroy thread
            g_ts_input_ctrl_info[act_aggre_idx].bCancel_thread = true;

            _tsaf_thread_join(&g_ts_input_ctrl_info[act_aggre_idx].thread_handle, 0);

            // destroy tse hanlde
            tse_DestroyHandle(&g_ts_input_ctrl_info[act_aggre_idx].pHTse, 0);

            g_ts_input_ctrl_info[act_aggre_idx].ref_cnt = 0;

            g_bUpdate_ini_file = true; // need to set ??
        }
        else if( g_ts_input_ctrl_info[act_aggre_idx].ref_cnt )
        {
            g_ts_input_ctrl_info[act_aggre_idx].ref_cnt--;

            // set demod_port_index stop recevice
            tse_Set_Service_Receive(g_ts_input_ctrl_info[act_aggre_idx].pHTse,
                                    demod_port_idx, -1, false, 0);
        }

        // free private data
        if( pCtrlInfo->privData )
        {
            free(pCtrlInfo->privData);
            pCtrlInfo->privData = 0;
        }

        for(i = 0; i < demod_num; i++)
        {
            if( g_def_demod_attr[i].aggre_index == act_aggre_idx &&
                g_def_demod_attr[i].demod_attr.demod_idx == demod_port_idx )
            {
                g_def_demod_attr[i].bUsed     = false;
                g_def_demod_attr[i].frequency = -1;
                g_def_demod_attr[i].bandwidth = -1;
                break;
            }
        }

        pCtrlInfo->extra_handle = 0;

    }while(0);

    return result;
}

static TSAF_ERR
aggre_gateway_Open(
    TSAF_CRTL_INFO  *pCtrlInfo,
    uint32_t        service_index,
    void            *extraDat)
{
    TSAF_ERR     result = TSAF_ERR_OK;
    TSE_HANDLE   *pHTse = 0;

    do{
        USER_PRIV_DATA      *pUser_priv_data = 0;
        uint32_t            demod_port_idx = 0;

        pHTse = (TSE_HANDLE*)pCtrlInfo->extra_handle;
        if( !pHTse )    break;

        pUser_priv_data = (USER_PRIV_DATA*)pCtrlInfo->privData;
        if( !pUser_priv_data )      break;

        if( pUser_priv_data->bScan_ready == false )
        {
            tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " tsaf open fail (scan_freq is not ready) !\n");
            break;
        }

        demod_port_idx = _GET_DEMOD_IDX(pCtrlInfo->demod_id);

        // set demod ctrl which one be used
        tse_Control(pHTse, TSE_CTRL_SET_DEMOD_STATUS, TSE_DEMOD_STATUS_RUNNING, (void*)demod_port_idx);

    }while(0);

    return result;
}

static TSAF_ERR
aggre_gateway_Close(
    TSAF_CRTL_INFO  *pCtrlInfo,
    void            *extraDat)
{
    TSAF_ERR     result = TSAF_ERR_OK;
    TSE_HANDLE   *pHTse = 0;

    do{
        USER_PRIV_DATA      *pUser_priv_data = 0;
        uint32_t            demod_port_idx = 0;

        pHTse = (TSE_HANDLE*)pCtrlInfo->extra_handle;
        if( !pHTse )    break;

        pUser_priv_data = (USER_PRIV_DATA*)pCtrlInfo->privData;
        if( !pUser_priv_data )      break;

        if( pUser_priv_data->bScan_ready == false )
        {
            tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " tsaf close fail (scan_freq is not ready) !\n");
            break;
        }

        demod_port_idx = _GET_DEMOD_IDX(pCtrlInfo->demod_id);

        // set demod ctrl which one be un-used
        tse_Control(pHTse, TSE_CTRL_SET_DEMOD_STATUS, TSE_DEMOD_STATUS_IDLE, (void*)demod_port_idx);

    }while(0);

    return result;
}

static int
aggre_gateway_readData(
    TSAF_CRTL_INFO  *pCtrlInfo,
    uint8_t         *pDstBuf,
    int             length,
    uint32_t        *pRealLength,
    void            *extraDat)
{
    //int         ret = -1;
    uint32_t    real_size = 0;
    TSE_HANDLE   *pHTse = 0;

    do{
        USER_PRIV_DATA      *pUser_priv_data = 0;
        TSE_SAMPLE_INFO     sample_info = {0};
        uint32_t            demod_port_idx = 0;
        uint32_t            real_size = 0;

        pHTse = (TSE_HANDLE*)pCtrlInfo->extra_handle;
        if( !pHTse )    break;

        pUser_priv_data = (USER_PRIV_DATA*)pCtrlInfo->privData;
        if( !pUser_priv_data )      break;

        if( pUser_priv_data->bScan_ready == false )
        {
            tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " tsaf read fail (scan_freq is not ready) !\n");
            break;
        }

        demod_port_idx = _GET_DEMOD_IDX(pCtrlInfo->demod_id);

        sample_info.port_idx     = demod_port_idx;
        sample_info.customer_idx = 0;
        sample_info.service_idx  = 0;
        sample_info.bufLength    = length;
        tse_Get_Service_Sample(pHTse, &sample_info, 0);

        if( sample_info.bufLength && sample_info.pBufAddr )
        {
            memcpy(pDstBuf, sample_info.pBufAddr, sample_info.bufLength);
            real_size = sample_info.bufLength;
        }

        if( pRealLength )       *pRealLength = real_size;

    }while(0);

    return 0;
}


static TSAF_ERR
aggre_gateway_scanFreq(
    TSAF_CRTL_INFO  *pCtrlInfo,
    void            *extraDat)
{
    TSAF_ERR        result = TSAF_ERR_OK;
    TSE_HANDLE      *pHTse = 0;

    do{
        TSE_ERR             rst = TSE_ERR_OK;
        USER_PRIV_DATA      *pUser_priv_data = 0;
        TSE_SCAN_PARAM      tse_scan_param = {0};
        TSE_USER_INFO       tse_user_info = {0};
        uint32_t            demod_num = 0;
        uint32_t            demod_port_idx = 0;
        uint32_t            act_aggre_idx = 0;

        struct timeval      curTime = {0};

        demod_num      = ARRAY_SIZE(g_def_demod_attr);
        demod_port_idx = _GET_DEMOD_IDX(pCtrlInfo->demod_id);
        act_aggre_idx  = _GET_AGGRE_IDX(pCtrlInfo->demod_id);

        pHTse = (TSE_HANDLE*)pCtrlInfo->extra_handle;
        if( !pHTse )    break;

        pUser_priv_data = (USER_PRIV_DATA*)pCtrlInfo->privData;
        if( !pUser_priv_data )      break;

        //---------------------------------------
        // read demod info from NOR
        _aggre_gateway_reset_freq(demod_num);

        pUser_priv_data->bScan_ready = false;

        // set demod ctrl which one be used
        tse_Control(pHTse, TSE_CTRL_SET_DEMOD_STATUS, TSE_DEMOD_STATUS_RUNNING, (void*)demod_port_idx);

        //---------------------------------------
        // start scan
        tse_scan_param.demod_idx                                            = demod_port_idx;
        tse_scan_param.channel.single.scanFreq                              = g_def_demod_attr[demod_port_idx].frequency;
        tse_scan_param.channel.single.bandwidth                             = g_def_demod_attr[demod_port_idx].bandwidth;
        tse_scan_param.scan_state_recv.func                                 = _aggre_gateway_scan_report;
        tse_scan_param.scan_state_recv.tse_user_arg.arg.scan.pTunnelInfo[0] = (void*)&pUser_priv_data->bScan_ready;

        rst = tse_Set_Scan_Info(pHTse, &tse_scan_param, 0);
        if( rst != TSE_ERR_OK )
        {
            result = TSAF_ERR_SCAN_FAIL;
            tsaf_msg_ex(1, "scan fail !! \n");
            break;
        }

        //--------------------------
        // pull High LED
#ifdef CFG_LED_ENABLE
        ioctl(g_def_demod_attr[demod_port_idx].hLed, ITP_IOCTL_FLICKER, (void*)200);
        // ioctl(g_def_demod_attr[demod_port_idx].hLed, ITP_IOCTL_ON, 0);
#endif

        //--------------------------
        tsaf_get_clock(&curTime);
        while( pUser_priv_data->bScan_ready == false )
        {
            if( tsaf_get_duration(&curTime) > 5000 )
            {
                //tsaf_get_clock(&curTime);
                result = TSAF_ERR_SCAN_FAIL;
                tsaf_msg_ex(1, "scan fail !! (over 5 sec)\n");
                break;
            }
            usleep(30000);
        }

        if( result != TSAF_ERR_OK )     break;

        tse_Set_Service_Receive(pHTse, demod_port_idx, 0, true, 0);
        usleep(200000); // wait hTspd create
        //-----------------------------------------
        // set risc demux plug-in
        tse_Get_Service_Info(pHTse, &tse_user_info, 0);

        tsaf_msg(1, " %d-th aggre -> %d-th demod port: total service=%d\n",
                act_aggre_idx, demod_port_idx,
                tse_user_info.demodInfo[demod_port_idx].total_services);

        tsaf_msg(1,"\tpRing_buf_H=0x%x, ring_buf_size=%d, pRing_buf_T=0x%x\n",
                tse_user_info.demodInfo[demod_port_idx].serviceInfo[0].pRing_buf,
                tse_user_info.demodInfo[demod_port_idx].serviceInfo[0].ring_buf_size,
                (tse_user_info.demodInfo[demod_port_idx].serviceInfo[0].pRing_buf+
                 tse_user_info.demodInfo[demod_port_idx].serviceInfo[0].ring_buf_size));

#if 1 // show attr msg
        {
            uint32_t    i = 0, j = 0;
            for(i = 0; i < tse_user_info.total_demod; i++)
            {
                printf("\nport %2d ------\n", i);
                for(j = 0; j < tse_user_info.demodInfo[i].total_services; j++)
                {
                    printf("\tservice %2d, vid = 0x%x\n", j,
                            tse_user_info.demodInfo[i].serviceInfo[j].video_pid);
                }
            }
        }
#endif

        if( tse_user_info.demodInfo[demod_port_idx].total_services )
        {
            uint32_t            i = 0;
            uint32_t            channelId = 0;
            TSE_SERVICE_INFO    *pServiceInfo = 0;
            CHANNEL_INFO_ARRAY  *ptChannelInfoArray = (CHANNEL_INFO_ARRAY*)iteTsDemux_GetChannelInfoArrayHandle();
            CHANNEL_INFO        *ptChannelInfo = 0;
            uint32_t            curBufReadPtr = 0;

            channelId = (demod_num * act_aggre_idx) + demod_port_idx;
            if( channelId >= MAX_CHANNEL_COUNT )
            {
                tsaf_msg_ex(TSAF_MSG_TYPE_ERR, " channel Id(%d) out support number(%d) ! \n",
                            channelId, MAX_CHANNEL_COUNT);
                break;
            }

            iteTsDemux_DisableChannel(channelId);

            pUser_priv_data->cache_write_idx = 0;
            pUser_priv_data->cache_buf_size  = tse_user_info.demodInfo[demod_port_idx].serviceInfo[0].ring_buf_size;
            pUser_priv_data->cache_head_ptr  = tse_user_info.demodInfo[demod_port_idx].serviceInfo[0].pRing_buf;
            pUser_priv_data->cache_tail_ptr  = pUser_priv_data->cache_head_ptr + pUser_priv_data->cache_buf_size;

            //-------------------------------
            //Get free risc share channel info
            pUser_priv_data->ptChannelInfo = ptChannelInfo = &ptChannelInfoArray->tChannelArray[channelId];

            if( tse_user_info.demodInfo[demod_port_idx].serviceInfo[0].pRing_buf )
            {
                uint32_t pesIndex = 0;
                // set ring buf info
                ptChannelInfo->pInputTsBuffer  = tse_user_info.demodInfo[demod_port_idx].serviceInfo[0].pRing_buf; // ring buf start pointer
                ptChannelInfo->tsBufferSize    = tse_user_info.demodInfo[demod_port_idx].serviceInfo[0].ring_buf_size; // ring buf size
                ptChannelInfo->tsBufferReadIdx = pUser_priv_data->cache_write_idx; // offset with start pointer
                ithFlushMemBuffer();

                // set pid info
                for(i = 0; i < tse_user_info.demodInfo[demod_port_idx].total_services; i++)
                {
                    uint32_t        k = 0;
                    uint32_t        pesSamplekSize = 0;
                    uint32_t        pesSampleCount = 0;

                    pServiceInfo = &tse_user_info.demodInfo[demod_port_idx].serviceInfo[i];

                    for(k = 0; k < AGGRE_GATEWAY_MAX_SERVICE_PRE_CAMERA; k++)
                    {
                        if( g_def_srvc_attr[k].video_pid == pServiceInfo->video_pid )
                        {
                            pesSamplekSize = g_def_srvc_attr[k].srvc_stream_buf_size;
                            pesSampleCount = g_def_srvc_attr[k].srvc_stream_buf_num;
                            break;
                        }
                    }

                    if( !pesSamplekSize || !pesSampleCount )
                    {
                        pesSamplekSize = AGGRE_DEF_PES_SAMPLE_BUF_SIZE;
                        pesSampleCount = AGGRE_DEF_MAX_PES_SAMPLE_BUF_NUM;
                    }

                    if( pServiceInfo->videoType != 2 &&
                        _aggre_gateway_InsertChannelPid(ptChannelInfo, pServiceInfo->video_pid,
                                                        pesSamplekSize, pesSampleCount, &pesIndex) )
                        tsaf_msg(1, "pes index: %u\n", pesIndex);
                }

                for(i = 0; i < ptChannelInfo->validPidCount; i++)
                    tsaf_msg(1, "pid: 0x%X, buf: 0x%X\n",
                            ptChannelInfo->tPidInfo[i].pid, ptChannelInfo->tPidInfo[i].pOutPesBuffer);
            }
            iteTsDemux_EnableChannel(channelId);

            // enable update RISC write pointer
            _aggre_gateway_cache_Start(-1, pCtrlInfo);

            //--------------------------
            // pull low LED
#ifdef CFG_LED_ENABLE
            ioctl(g_def_demod_attr[demod_port_idx].hLed, ITP_IOCTL_OFF, NULL);
#endif
        }
    }while(0);
    return result;
}

static TSAF_ERR
aggre_gateway_control(
    TSAF_CRTL_INFO  *pCtrlInfo,
    uint32_t        cmd,
    uint32_t        *value,
    void            *extraDat)
{
    TSAF_ERR     result = TSAF_ERR_OK;
    TSE_HANDLE   *pHTse = 0;
    uint32_t     demod_port_idx = 0;

    do{
        pHTse = (TSE_HANDLE*)pCtrlInfo->extra_handle;
        if( !pHTse )    break;

        demod_port_idx = _GET_DEMOD_IDX(pCtrlInfo->demod_id);

        switch( cmd )
        {
            case TSAF_CTRL_GET_CMD_STREAM:
                {
                    TSAF_ARG            *pArg = (TSAF_ARG*)extraDat;
                    TSE_SAMPLE_INFO     sample_info = {0};

                    if( !pArg )     break;

                    sample_info.port_idx     = demod_port_idx;
                    sample_info.service_idx  = TSE_CMD_SRVC_INDEX;
                    sample_info.customer_idx = 0;
                    tse_Get_Service_Sample(pHTse, &sample_info, 0);

                    pArg->args.get_cmd_stream.pStream_buf   = sample_info.pBufAddr;
                    pArg->args.get_cmd_stream.stream_length = sample_info.bufLength;
                }
                break;

            default:        break;
        }
    }while(0);
    return result;
}
//=============================================================================
//                Public Function Definition
//=============================================================================

TS_AIRFILE_DESC  TS_AIRFILE_DESC_aggre_gateway_desc =
{
    "ts air file aggre GateWay", // char        *name;
    0,                           // struct _TS_AIRFILE_DESC_TAG  *next;
    TS_AIRFILE_ID_AGGRE_GATEWAY, // TS_AIRFILE_TYPE_ID           id;
    0,                           // void        *privInfo;

    // // operator
    aggre_gateway_Init,               // int     (*taf_init)(TSAF_CRTL_INFO *pCtrlInfo, TSAF_INIT_PARAM *pInitParam, void *extraData);
    aggre_gateway_deInit,             // int     (*taf_deinit)(TSAF_CRTL_INFO *pCtrlInfo, void *extraData);

    aggre_gateway_Open,               // int     (*taf_open)(TSAF_CRTL_INFO *pCtrlInfo, void *extraData);
    aggre_gateway_Close,              // int     (*taf_close)(TSAF_CRTL_INFO *pCtrlInfo, void *extraData);
    aggre_gateway_readData,           // int     (*taf_read)(TSAF_CRTL_INFO *pCtrlInfo, void *extraData);
    0,                                // int     (*taf_write)(TSAF_CRTL_INFO *pCtrlInfo, void *extraData);
    0,                                // int     (*taf_get_inSrc_func)(TSAF_CRTL_INFO *pCtrlInfo, pFunc_inSrc *ppFunc, void *extraData);
    // aggre_gateway_changeServic,       // int     (*taf_changeService)(TSAF_CRTL_INFO *pCtrlInfo, void *extraData);
    aggre_gateway_scanFreq,           // int     (*taf_scanFreq)(TSAF_CRTL_INFO *pCtrlInfo, void *extraData);
    aggre_gateway_control,            // int     (*taf_control)(TSAF_CRTL_INFO *pCtrlInfo, uint32_t cmd, uint32_t *value, void *extraData);
};
#else

TS_AIRFILE_DESC  TS_AIRFILE_DESC_aggre_gateway_desc =
{
    "ts air file aggre GateWay",      // char        *name;
    0,                          // struct _TS_AIRFILE_DESC_TAG  *next;
    TS_AIRFILE_ID_AGGRE_GATEWAY,      // TS_AIRFILE_TYPE_ID           id;
    0,                          // void        *privInfo;
};
#endif


#include <stdio.h>
#include <string.h>
#include "ts_extractor_defs.h"
#include "ite_ts_extractor.h"
#include "ts_aggre_ctrl.h"
#include "ts_extract.h"
#include "demod_ctrl.h"

#if !(ENABLE_SW_SIMULATION)
    #include "ite/itp.h"
    #include "mmp_tsi.h"
    #include "mmp_iic.h"
#endif
//=============================================================================
//                Constant Definition
//=============================================================================
typedef enum _TSE_STATUS_TAG
{
    TSE_STATUS_IDLE = 0x11,
    TSE_STATUS_BUSY = 0xBB,
    TSE_STATUS_FAIL = 0xFF,

}TSE_STATUS;

#define TSE_TS_PACKET_SIZE          (188)

#if !defined(CFG_AGGRE_NONE)
    #define TSE_ACT_AGGRE_TAG_LENGTH    TSA_TAG_LEN_4
#else
    #define TSE_ACT_AGGRE_TAG_LENGTH    TSA_TAG_LEN_0
#endif

#define TSE_PID_PAT         (0x00)
#define TSE_PID_NIT         (0x10)
#define TSE_PID_SDT         (0x11)
#define TSE_PID_EIT         (0x12)
#define TSE_PID_TDT_TOT     (0x14)

typedef enum TSE_PID_FILTER_INDEX_T
{
    TSE_PID_PAT_INDEX       = 0,
    TSE_PID_SDT_INDEX       = 1,
    TSE_PID_EIT_INDEX       = 2,
    TSE_PID_TOT_TDT_INDEX   = 3,
    TSE_PID_NIT_INDEX       = 4,
    TSE_PID_PMT_INDEX       = 5,  // reserve 1 * TSE_MAX_SERVICE_PRE_DEMOD PMTs
    TSE_PID_VIDEO_INDEX     = 9,  // reserve 1 * TSE_MAX_SERVICE_PRE_DEMOD video PID
    TSE_PID_AUDIO_INDEX     = 13, // reserve 3 * TSE_MAX_SERVICE_PRE_DEMOD audio PID

    TSE_PID_CMD_INDEX_0     = 30,
    TSE_PID_CMD_INDEX_1     = 31,

    TSE_PID_TOTAL_FILTER_COUNT = 32,
}TSE_PID_FILTER_INDEX;

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct TSE_PASSPORT_INFO_T
{
    // Coordinate with aggre module
    TSA_TAG_LEN      tag_len;

    // reg[tag_value3] << 24 | reg[tag_value2] << 16 | reg[tag_value1] << 8 | reg[tag_value0]
    uint32_t        tag_value;

}TSE_PASSPORT_INFO;

typedef struct TSE_DEMOD_INFO_T
{
    uint32_t            freq;
    uint32_t            bandwidth;

    DEMOD_CTRL_HANDLE   *pHDemodCtrl;

}TSE_DEMOD_INFO;

typedef struct _tes_byte_align4 _ITE_TSE_DEV_TAG
{
    TSE_HANDLE          hTse;       // communicate to user

    pthread_mutex_t     tse_mutex;

    TSE_STATUS          tseStatus;  // globle status

    // ts input operator
    bool                bSkip_recv_turn_on;
    TSE_RECV_OPR        recv_opr;

    // aggre handle
    TSA_HANDLE          *pHTsa;

    // demode
    uint32_t            total_demod;
    uint32_t            act_demod_idx;
    TSE_DEMOD_INFO      demod_info[TSE_MAX_DEMOD_PER_AGGRE];


    // ts extrat
    TSE_SPLIT_LEVEL     ts_split_level;
    TSEXT_HANDLE        *pHTsExt;

    // user scan state callback
    TSE_USER_MBOX       scan_state_recv[TSE_MAX_DEMOD_PER_AGGRE];

    // get some info (ex. service resolution) from ap layer
    TSE_USER_MBOX       get_user_info;

    // ccHDTv
    // keep code
    //TSE_CMD_PKT_INFO    *pUser_cmd_pkt[TSE_MAX_DEMOD_PER_AGGRE];

}ITE_TSE_DEV;
//=============================================================================
//                Global Data Definition
//=============================================================================
uint32_t  tseMsgOnFlag = (0x1);

static TSE_PASSPORT_INFO  g_passport_info[4] =
{
    {TSE_ACT_AGGRE_TAG_LENGTH, 0x0},
    {TSE_ACT_AGGRE_TAG_LENGTH, 0x1},
    {TSE_ACT_AGGRE_TAG_LENGTH, 0x2},
    {TSE_ACT_AGGRE_TAG_LENGTH, 0x3}
};
//=============================================================================
//                Private Function Definition
//=============================================================================
#if !(ENABLE_SW_SIMULATION)
static uint32_t
_def_ts_recv_init(
    TSE_RECV_OPR    *pTs_recv_opr,
    void            *extraData)
{
    uint32_t    result = 0;
    uint32_t    tsi_idx = 0;

    if( pTs_recv_opr )
    {
        tsi_idx = pTs_recv_opr->privData.tsi.tsi_index;

        mmpTsiInitialize(tsi_idx);
    }
    return result;
}

static uint32_t
_def_ts_recv_deinit(
    TSE_RECV_OPR    *pTs_recv_opr,
    void            *extraData)
{
    uint32_t    result = 0;
    uint32_t    tsi_idx = 0;

    if( pTs_recv_opr )
    {
        tsi_idx = pTs_recv_opr->privData.tsi.tsi_index;

        mmpTsiDisable(tsi_idx);
        mmpTsiTerminate(tsi_idx);
    }
    return result;
}

static uint32_t
_def_ts_recv_turn_on(
    TSE_RECV_OPR    *pTs_recv_opr,
    void            *extraData)
{
    uint32_t    result = 0;

    if( pTs_recv_opr )
    {
        uint32_t    tsi_idx = pTs_recv_opr->privData.tsi.tsi_index;

        mmpTsiEnable(tsi_idx);
    }
    return result;
}

static uint32_t
_def_ts_recv_turn_off(
    TSE_RECV_OPR    *pTs_recv_opr,
    void            *extraData)
{
    uint32_t    result = 0;

    if( pTs_recv_opr )
    {
        uint32_t    tsi_idx = pTs_recv_opr->privData.tsi.tsi_index;

        mmpTsiDisable(tsi_idx);
    }
    return result;
}

static uint32_t
_def_ts_recv_get_data(
    TSE_RECV_OPR    *pTs_recv_opr,
    uint8_t         **ppSampleAddr,
    uint32_t        *pSampleSize,
    void            *extraData)
{
    uint32_t    result = 0;

    if( pTs_recv_opr && ppSampleAddr && pSampleSize )
    {
        uint32_t    tsi_idx = pTs_recv_opr->privData.tsi.tsi_index;
        uint8_t     *pSampleBuf = 0;
        uint32_t    sampleLength = 0;

        mmpTsiReceive(tsi_idx, &pSampleBuf, &sampleLength);
        //tse_msg(1, "tsi dataSize = %d\n", sampleLength);
        (*ppSampleAddr) = pSampleBuf;
        (*pSampleSize)  = sampleLength;
    }
    return result;
}

static TSE_RECV_OPR def_ts_recv_opr =
{
    TSE_RECV_TYPE_TSI,
    {0},
    _def_ts_recv_init,
    _def_ts_recv_deinit,
    _def_ts_recv_turn_on,
    _def_ts_recv_turn_off,
    _def_ts_recv_get_data,
};
#else /* if !(ENABLE_SW_SIMULATION) */
static TSE_RECV_OPR def_ts_recv_opr = {0};
#endif

static uint32_t
_scan_feedback(
    TSE_USER_ARG   *pUser_arg,
    void           *extraData)
{
    do{
        uint32_t        port_index = 0;
        ITE_TSE_DEV     *pTseDev = 0;
        TSEXT_HANDLE    *pHTsExt = 0;

        if( !pUser_arg )        break;

        port_index = pUser_arg->arg.scan.port_index;
        pTseDev    = (ITE_TSE_DEV*)pUser_arg->arg.scan.pTunnelInfo[0];
        tsExt_Set_Pkt_Proc_Mode(pTseDev->pHTsExt, port_index, TSEXT_PKT_PROC_IDLE, 0);

        // callback to external AP
        if( pTseDev->scan_state_recv[port_index].func )
        {
            TSE_USER_ARG   *pUser_scan_arg = &pTseDev->scan_state_recv[port_index].tse_user_arg;

            pUser_scan_arg->arg.scan.port_index = port_index;
            pTseDev->scan_state_recv[port_index].func(pUser_scan_arg, extraData);
        }

        tse_msg(1, " ------ TSE get scan ready (port = %d)\n", port_index);

        {
            // set demod filter
            uint32_t                srvc_idx = 0;
            TSEXT_PKT_ANAL_INFO     *pAct_pkt_analyzer_info = 0;
            TSEXT_SRVC_PID_INFO     *pCur_srvc_pid_info = 0;
            DEMOD_CTRL_HANDLE       *pHDemodCtrl = 0;

            pAct_pkt_analyzer_info = pTseDev->pHTsExt->pPkt_analyzer_info[port_index];
            pHDemodCtrl            = pTseDev->demod_info[port_index].pHDemodCtrl;

            DemodCtrl_UpdatePidTable(pHDemodCtrl, TSE_PID_PAT, TSE_PID_PAT_INDEX);
            DemodCtrl_UpdatePidTable(pHDemodCtrl, TSE_PID_SDT, TSE_PID_SDT_INDEX);
            DemodCtrl_UpdatePidTable(pHDemodCtrl, TSE_PID_EIT, TSE_PID_EIT_INDEX);
            DemodCtrl_UpdatePidTable(pHDemodCtrl, TSE_PID_TDT_TOT, TSE_PID_TOT_TDT_INDEX);
            DemodCtrl_UpdatePidTable(pHDemodCtrl, TSE_PID_NIT, TSE_PID_NIT_INDEX);

            for(srvc_idx = 0; srvc_idx < TSE_MAX_SERVICE_PRE_DEMOD; srvc_idx++)
            {
                uint32_t    j = 0, aud_num = 0;

                if( srvc_idx >= pAct_pkt_analyzer_info->total_service )
                    break;

                pCur_srvc_pid_info = &pAct_pkt_analyzer_info->pSrvc_pid_info[srvc_idx];

                //--------------------------------------
                // Need to refine: To Do:
                // PMT
                DemodCtrl_UpdatePidTable(pHDemodCtrl, pCur_srvc_pid_info->pmt_pid, TSE_PID_PMT_INDEX + srvc_idx);

                // video
                DemodCtrl_UpdatePidTable(pHDemodCtrl, pCur_srvc_pid_info->videoPID, TSE_PID_VIDEO_INDEX + srvc_idx);

                // audio
                aud_num = ARRAY_SIZE(pCur_srvc_pid_info->audioPID);
                aud_num = (aud_num > TSE_SRVC_MAX_AUD_COUNT) ? TSE_SRVC_MAX_AUD_COUNT : aud_num;
                for(j = 0; j < aud_num; j++)
                {
                    if( j < pCur_srvc_pid_info->audioCount )
                        DemodCtrl_UpdatePidTable(pHDemodCtrl, pCur_srvc_pid_info->audioPID[j],
                                                 TSE_PID_AUDIO_INDEX + srvc_idx + j);
                }
                //--------------------------------------
            }
        }
    }while(0);

    return 0;
}


static uint32_t
_setup_scan_freq(
    ITE_TSE_DEV     *pTseDev)
{
    uint32_t    result = 0;

    do{
        bool                bChannelLock = false;
        uint32_t            demod_idx = pTseDev->act_demod_idx;
        DEMOD_CTRL_HANDLE   *pHDemodCtrl = 0;
        #if (_MSC_VER)
            uint32_t    startT = 0;
        #else
            struct timeval startT = {0};
        #endif

        pHDemodCtrl = pTseDev->demod_info[demod_idx].pHDemodCtrl;

        // If the channel acquisition step is success.
        pHDemodCtrl->frequency = pTseDev->demod_info[demod_idx].freq;
        pHDemodCtrl->bandwith  = pTseDev->demod_info[demod_idx].bandwidth;
        result = DemodCtrl_AcquireChannel(pHDemodCtrl);
        if( result )
        {
            tse_msg_ex(TSE_MSG_ERR, "acquire channel return error: 0x%x\n", result);
            break;
        }

        tse_msg(1, " port %d-> scan freq: %d, bw: %d MHz\n", demod_idx, pHDemodCtrl->frequency, pHDemodCtrl->bandwith/1000);
        tse_get_clock(&startT);
        bChannelLock = DemodCtrl_IsChannelLock(pHDemodCtrl);

        tse_msg(1, "\t\tport %d-> lock channel period: %d ms, bChannelLock = %d\n",
                demod_idx,
                tse_get_duration(&startT), bChannelLock);

#if (ENABLE_SW_SIMULATION)
        bChannelLock = true;
#endif
        if( bChannelLock == false )
        {
            result = (-1);
            break;
        }

        //-------------------------------------
        // reset demode PID filter and enable tsi H/W
        DemodCtrl_ResetPidTable(pHDemodCtrl, false);

    }while(0);

    return result;
}

static uint32_t
_get_video_info(
    TSE_USER_ARG    *pUser_arg,
    void            *extraData)
{
    ITE_TSE_DEV     *pTseDev = 0;

    do{
        uint32_t        port_index = 0;

        if( !pUser_arg )        break;

        port_index = pUser_arg->arg.v_info.port_index;
        pTseDev    = (ITE_TSE_DEV*)pUser_arg->arg.v_info.pTunnelInfo[0];

        // callback to external AP
        if( pTseDev->get_user_info.func )
        {
            TSE_USER_ARG   *pExternal_info_arg = &pTseDev->get_user_info.tse_user_arg;

            pExternal_info_arg->type                  = pUser_arg->type;
            pExternal_info_arg->arg.v_info.port_index = port_index;
            pExternal_info_arg->arg.v_info.v_pid      = pUser_arg->arg.v_info.v_pid;
            pTseDev->get_user_info.func(pExternal_info_arg, extraData);

            tse_msg(0, " ------ TSE get service W/H=(%d, %d), cacsh size=%d, port=%d\n",
                        pExternal_info_arg->arg.v_info.width,
                        pExternal_info_arg->arg.v_info.height,
                        pExternal_info_arg->arg.v_info.srvc_buf_size,
                        port_index);

            pUser_arg->arg.v_info.width         = pExternal_info_arg->arg.v_info.width;
            pUser_arg->arg.v_info.height        = pExternal_info_arg->arg.v_info.height;
            pUser_arg->arg.v_info.srvc_buf_size = pExternal_info_arg->arg.v_info.srvc_buf_size;
        }

    }while(0);

    return 0;
}
//=============================================================================
//                Public Function Definition
//=============================================================================
TSE_ERR
tse_CreateHandle(
    TSE_HANDLE        **pHTse,
    TSE_INIT_PARAM    *pInitParam,
    void              *extraData)
{
    TSE_ERR         result = TSE_ERR_OK;
    ITE_TSE_DEV     *pTseDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSE, "0x%x, 0x%x, 0x%x\n", pHTse, pInitParam, extraData);

    do{
        uint32_t            rst = 0;
        uint32_t            total_demod = 0;
        uint32_t            tag_length = 0;
        uint32_t            i = 0, j = 0;
        TS_AGGR_ID          tsa_type = TS_AGGR_ID_NONE;
        TSA_SETUP_INFO      tsa_setup_info = {0};
        TSA_INIT_PARAM      tsa_init_param = {0};
        TSEXT_INIT_PARAM    tsext_init_param = {0};
        TSEXT_PASSPORT_INFO passport_info = {0};
        TSA_AGR             tsaArg = {0};
        TSE_USER_MBOX       tse_get_info = {0};

        if( *pHTse != 0 )
        {
            tse_msg_ex(TSE_MSG_ERR, " err, Exist tse handle !!");
            result = TSE_ERR_INVALID_PARAMETER;
            break;
        }

        if( !pInitParam )
        {
            tse_msg_ex(TSE_MSG_ERR, " err, Need pre-set info !!");
            result = TSE_ERR_INVALID_PARAMETER;
            break;
        }

        pTseDev = tse_malloc(sizeof(ITE_TSE_DEV));
        if( !pTseDev )
        {
            tse_msg_ex(TSE_MSG_ERR, " err allocate fail !!");
            result = TSE_ERR_ALLOCATE_FAIL;
            break;
        }

        memset(pTseDev, 0x0, sizeof(ITE_TSE_DEV));
        //------------------------------------
        // set init param
        pTseDev->total_demod    = pInitParam->total_demod;
        pTseDev->ts_split_level = pInitParam->ts_split_level;
        pTseDev->get_user_info  = pInitParam->get_user_info;

        //------------------------------------
        // ts input initial
        pTseDev->recv_opr = def_ts_recv_opr;
        pTseDev->recv_opr.privData.tsi.tsi_index = pInitParam->tsi_idx;
        if( pInitParam->reset_ts_recv_opr )
            pInitParam->reset_ts_recv_opr(&pTseDev->recv_opr, extraData);

        if( pTseDev->recv_opr.ts_recv_init )
            pTseDev->recv_opr.ts_recv_init(&pTseDev->recv_opr, 0);

        //------------------------------------
        // init i2c
        #if !(ENABLE_SW_SIMULATION)
        rst = (IIC_PORT_0, IIC_MASTER_MODE, 0, 0, 100 * 1000);
        if( rst )   tse_msg_ex(TSE_MSG_ERR, "mmpIicInitialize() err 0x%x !", rst);

        mmpIicLockModule(IIC_PORT_0);
        rst = mmpIicSetClockRate(IIC_PORT_0, 200* 1024);
        if( rst )   tse_msg(1, "curr iic clock %d !\n\n", rst);
        #endif

        //------------------------------------
        // create aggre handle
        if( pTseDev->total_demod > TSA_MAX_PORT_NUM )
        {
            tse_msg_ex(TSE_MSG_ERR, " err, demod port num (%d) out support (%d) !!",
                            pTseDev->total_demod, TSA_MAX_PORT_NUM);
            break;
        }

        switch( pInitParam->aggre_type )
        {
            case TSE_AGGRE_TYPE_ENDEAVOUR:  tsa_setup_info.aggre_id = TS_AGGR_ID_ENDEAVOUR; break;
        }

        tsa_setup_info.tsa_idx     = pInitParam->aggre_idx;
        tsa_setup_info.tsa_tag_len = TSE_ACT_AGGRE_TAG_LENGTH;
        rst = tsa_CreateHandle(&pTseDev->pHTsa, &tsa_setup_info, 0);
        if( (TSA_ERR)rst != TSA_ERR_OK )
        {
            tse_msg_ex(TSE_MSG_ERR, " err, tsa create fail !!");
            result = TSE_ERR_AGGRE_FAIL;
            break;
        }

        // init aggre module
        switch( pInitParam->aggre_bus_type )
        {
            case TSE_AGGRE_BUS_I2C: tsa_init_param.bus_type = TSA_BUS_I2C;  break;
            case TSE_AGGRE_BUS_USB: tsa_init_param.bus_type = TSA_BUS_USB;  break;
        }

        tsa_init_param.aggre_i2c_addr   = pInitParam->aggre_i2c_addr;
        tsa_init_param.tsa_mode         = TSA_MODE_TAG;
        tsa_init_param.total_demod_port = pTseDev->total_demod;
        for(i = 0; i < pTseDev->total_demod; i++)
        {
            for(j = 0; j < TSE_MAX_DEMOD_PER_AGGRE; j++)
            {
                if( pInitParam->demod_attr[j].demod_idx == i )
                {
                    tsa_init_param.demod_i2c_addr[i]        = pInitParam->demod_attr[j].demod_i2c_addr;
                    tsa_init_param.linked_aggre_port_idx[i] = pInitParam->demod_attr[j].linked_aggre_port_idx;
                    break;
                }
            }

            // ts extract set passport info
            tsa_init_param.tag_value[i] = g_passport_info[i].tag_value;
        }

        tsa_Init(pTseDev->pHTsa, &tsa_init_param, 0);

        //-------------------------------------
        //  create ts extract handle
        switch( pTseDev->ts_split_level )
        {
            case TSE_SPLIT_DEMOD:   tsext_init_param.pkt_split_level = TSEXT_PKT_SPLIT_DEMOD;   break;
            case TSE_SPLIT_SERVICE: tsext_init_param.pkt_split_level = TSEXT_PKT_SPLIT_SERVICE; break;
        }

        tsext_init_param.bBy_Pass_Tss = pInitParam->bSkip_aggre;
        tag_length = (tsext_init_param.bBy_Pass_Tss) ? 0 : pTseDev->pHTsa->tag_length;

        tsext_init_param.act_pkt_size = TSE_TS_PACKET_SIZE + tag_length;
        tsext_init_param.bSpareMem    = pInitParam->bSpareMem;

        tsExt_CreateHandle(&pTseDev->pHTsExt, &tsext_init_param, 0);
        if( !pTseDev->pHTsExt )
        {
            tse_msg_ex(TSE_MSG_ERR, " err, tsExt create fail !!");
            break;
        }

        // set aggre tag and length
        for(i = 0; i < pTseDev->total_demod; i++)
        {
            passport_info.tag_len       = pTseDev->pHTsa->tag_length;
            passport_info.tag_value     = g_passport_info[i].tag_value;

            tsExt_Add_Passport_Info(pTseDev->pHTsExt, &passport_info, 0);
        }

        // get info from user layer
        switch( pTseDev->ts_split_level )
        {
            //case TSE_SPLIT_DEMOD:   tse_get_info.func = 0;                  break;
            default:
            case TSE_SPLIT_SERVICE: tse_get_info.func = _get_video_info;    break;
        }
        tse_get_info.tse_user_arg.type                      = TSE_USER_ARG_TYPE_GET_VIDEO_INFO;
        tse_get_info.tse_user_arg.arg.v_info.pTunnelInfo[0] = (void*)pTseDev;

        pTseDev->pHTsExt->get_info = tse_get_info;

        //------------------------------------
        // create demod hanlde
        for(i = 0; i < pTseDev->total_demod; i++)
        {
            DEMOD_TYPE_ID       demod_type_id = DEMOD_ID_UNKNOW;
            DEMOD_ATTR          demod_attr = {0};
            DEMOD_SETUP_INFO    demod_setupInfo = {0};
            TSA_AGR             tsaArg = {0};

            for(j = 0; j < TSE_MAX_DEMOD_PER_AGGRE; j++)
            {
                if( pInitParam->demod_attr[j].demod_idx == i )
                    break;
            }

            if( j == TSE_MAX_DEMOD_PER_AGGRE )
            {
                tse_msg_ex(TSE_MSG_ERR, " err, wrong demod attribut !!");
                break;
            }

            switch( pInitParam->demod_attr[j].demod_type )
            {
                case TSE_DEMOD_TYPE_IT9135: demod_attr.demod_type = DEMOD_ID_IT9135;   break;
                case TSE_DEMOD_TYPE_IT9137: demod_attr.demod_type = DEMOD_ID_IT9137;   break;
                default:                    demod_attr.demod_type = DEMOD_ID_UNKNOW;   break;
            }
            switch( pInitParam->demod_attr[j].bus_type )
            {
                case TSE_DEMOD_BUS_I2C: demod_attr.bus_type = DEMOD_BUS_I2C; break;
                case TSE_DEMOD_BUS_USB: demod_attr.bus_type = DEMOD_BUS_USB; break;
            }

            demod_attr.demod_idx             = i;
            demod_attr.demod_i2c_addr        = pInitParam->demod_attr[j].demod_i2c_addr;
            demod_attr.linked_aggre_port_idx = pInitParam->demod_attr[j].linked_aggre_port_idx;
            DemodCtrl_CreateHandle(&pTseDev->demod_info[i].pHDemodCtrl, &demod_attr);

            demod_setupInfo.architecture = 2;
            demod_setupInfo.supportType  = 2; // not work
            DemodCtrl_Init(pTseDev->demod_info[i].pHDemodCtrl, &demod_setupInfo);

            // set endeavour tag info
            tsaArg.type                    = TSA_ARG_TYPE_SET_MODE;
            tsaArg.arg.set_mode.aggre_mode = TSA_MODE_TAG;
            tsaArg.arg.set_mode.port_index = i; //pTseDev->demod_info[i].pHDemodCtrl->linked_aggre_port_idx;
            tsaArg.arg.set_mode.tag_value  = g_passport_info[i].tag_value;
            rst = tsa_Set_Aggre_Mode(pTseDev->pHTsa, &tsaArg, 0);
            if( (TSA_ERR)rst != TSA_ERR_OK )
                tse_msg_ex(TSE_MSG_ERR, " tsa %d-th port set mode fail (0x%x)\n", i, result);

            tse_msg(1, " tsa %d-th port set mode \n", i);
        }

        //------------------------------------
        // set i2c
        #if !(ENABLE_SW_SIMULATION)
        rst = mmpIicSetClockRate(IIC_PORT_0, 100* 1024);
        if( rst )   tse_msg(1, "curr iic clock %d !\n", rst);
        mmpIicReleaseModule(IIC_PORT_0);
        #endif

        //------------------------------------
        // create mutex
        _mutex_init(TSE_MSG_TRACE_TSE, pTseDev->tse_mutex);
        (*pHTse) = &pTseDev->hTse;

    }while(0);

    if( result != TSE_ERR_OK )
    {
        pTseDev->tseStatus = TSE_STATUS_FAIL;
        tse_msg_ex(TSE_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _trace_leave(TSE_MSG_TRACE_TSE);
    return result;
}

TSE_ERR
tse_DestroyHandle(
    TSE_HANDLE        **pHTse,
    void              *extraData)
{
    TSE_ERR             result = TSE_ERR_OK;
    ITE_TSE_DEV         *pTseDev = 0;
    pthread_mutex_t     tse_mutex = 0;

    _trace_enter(TSE_MSG_TRACE_TSE, "0x%x\n", pHTse);

    /**
     * Ap layer need to check all threads, which assess this handle, in STOP state.
     * Or system maybe crash.
     **/

    _verify_handle(pHTse, result);
    _verify_handle((*pHTse), result);

    pTseDev = DOWN_CAST(ITE_TSE_DEV, (*pHTse), hTse);

    _mutex_lock(TSE_MSG_TRACE_TSE, pTseDev->tse_mutex);

    _disable_irq();
    if( pTseDev )
    {
        uint32_t    i = 0;

        // -----------------------------
        // destroy handle
        for(i = 0; i < TSE_MAX_DEMOD_PER_AGGRE; i++)
        {
            DemodCtrl_DestroyHandle(&pTseDev->demod_info[i].pHDemodCtrl);
        }

        tsExt_DestroyHandle(&pTseDev->pHTsExt, extraData);

        tsa_Deinit(pTseDev->pHTsa, 0, extraData);
        tsa_DestroyHandle(&pTseDev->pHTsa, extraData);
        // ------------------------------
        // ts input terminate
        if( pTseDev->recv_opr.ts_recv_turn_off )
            pTseDev->recv_opr.ts_recv_turn_off(&pTseDev->recv_opr, extraData);

        if( pTseDev->recv_opr.ts_recv_deinit)
            pTseDev->recv_opr.ts_recv_deinit(&pTseDev->recv_opr, extraData);

        #if !(ENABLE_SW_SIMULATION)
        mmpIicTerminate(IIC_PORT_0);
        #endif

        //-------------------------------
        *pHTse = 0; // notice AP that handle has be destroyed

        tse_mutex = pTseDev->tse_mutex;
        // destroy dev info
        free(pTseDev);

    }

    _mutex_unlock(TSE_MSG_TRACE_TSE, tse_mutex);
    _mutex_deinit(TSE_MSG_TRACE_TSE, tse_mutex);

    _enable_irq();
    _trace_leave(TSE_MSG_TRACE_TSE);
    return result;
}

TSE_ERR
tse_Action_Proc(
    TSE_HANDLE     *pHTse,
    void           *extraData)
{
    TSE_ERR         result = TSE_ERR_OK;
    ITE_TSE_DEV     *pTseDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSE, "0x%x, 0x%x\n", pHTse, extraData);
    _verify_handle(pHTse, result);

    pTseDev = DOWN_CAST(ITE_TSE_DEV, pHTse, hTse);

    _mutex_lock(TSE_MSG_TRACE_TSE, pTseDev->tse_mutex);

    if( pTseDev && pTseDev->tseStatus != TSE_STATUS_FAIL )
    {
        TSEXT_BUF_INFO      tsext_buf_info = {0};
        uint8_t             *pBuf_addr = 0;
        uint32_t            buf_size = 0;

        if( pTseDev->recv_opr.ts_recv_get_data )
        {
            pTseDev->recv_opr.ts_recv_get_data(&pTseDev->recv_opr,
                                               &pBuf_addr, &buf_size, extraData);
        }

        if( pBuf_addr && buf_size )
        {
            tsext_buf_info.pBufAddr  = pBuf_addr;
            tsext_buf_info.bufLength = buf_size;
            tsExt_Extract(pTseDev->pHTsExt, &tsext_buf_info, 0);
        }
    }

    if( result != TSE_ERR_OK )
    {
        pTseDev->tseStatus = TSE_STATUS_FAIL;
        tse_msg_ex(TSE_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSE_MSG_TRACE_TSE, pTseDev->tse_mutex);
    _trace_leave(TSE_MSG_TRACE_TSE);
    return result;
}


TSE_ERR
tse_Set_Scan_Info(
    TSE_HANDLE      *pHTse,
    TSE_SCAN_PARAM  *pScan_param,
    void            *extraData)
{
    TSE_ERR         result = TSE_ERR_OK;
    ITE_TSE_DEV     *pTseDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSE, "0x%x, 0x%x\n", pHTse, extraData);
    _verify_handle(pHTse, result);
    _verify_handle(pScan_param, result);

    pTseDev = DOWN_CAST(ITE_TSE_DEV, pHTse, hTse);

    _mutex_lock(TSE_MSG_TRACE_TSE, pTseDev->tse_mutex);

    if( pTseDev && pTseDev->tseStatus != TSE_STATUS_FAIL )
    {
        do{
            uint32_t        i = 0;
            TSE_USER_MBOX   scan_report = {0};
            TSA_AGR         tsaArg = {0};
            uint32_t        rst = 0;

            scan_report.func                                 = _scan_feedback;
            scan_report.tse_user_arg.type                    = TSE_USER_ARG_TYPE_SCAN_STATUS;
            scan_report.tse_user_arg.arg.scan.pTunnelInfo[0] = (void*)pTseDev;

            if( pScan_param->demod_idx == TSE_ALL_DEMOD_PORT )
            {
                pTseDev->pHTsExt->scan_state_recv = scan_report;

                // all demod scan freq
                for(i = 0; i < TSE_MAX_DEMOD_PER_AGGRE; i++)
                {
                    //-------------------------------------
                    // disable ts aggre
                    tsaArg.type                    = TSA_ARG_TYPE_SET_PORT;
                    tsaArg.arg.set_port.port_index = i; //pTseDev->demod_info[i].pHDemodCtrl->linked_aggre_port_idx;

                    rst = tsa_Disable_Port(pTseDev->pHTsa, &tsaArg, 0);
                    if( (TSA_ERR)rst != TSA_ERR_OK )
                    {
                        result = TSE_ERR_AGGRE_FAIL;
                        tse_msg_ex(TSE_MSG_ERR, " tsa %d-th port enable fail (0x%x)\n", i, result);
                        break;
                    }//*/

                    pTseDev->act_demod_idx           = i;
                    pTseDev->demod_info[i].freq      = pScan_param->channel.all.scanFreq[i];
                    pTseDev->demod_info[i].bandwidth = pScan_param->channel.all.bandwidth[i];
                    pTseDev->scan_state_recv[i]      = pScan_param->scan_state_recv;

                    rst = _setup_scan_freq(pTseDev);
                    if( rst )
                    {
                        result = TSE_ERR_DEMOD_FAIL;
                        tse_msg_ex(TSE_MSG_ERR, " tse scan fail (0x%x), port %d-th !\n", rst, i);
                        continue;
                    }

                    // set ts extract
                    tsExt_Set_Pkt_Proc_Mode(pTseDev->pHTsExt, pTseDev->act_demod_idx, TSEXT_PKT_PROC_ANALYZE, 0);

                    //-------------------------------------
                    // enable ts aggre
                    tsaArg.type                    = TSA_ARG_TYPE_SET_PORT;
                    tsaArg.arg.set_port.port_index = i;//pTseDev->demod_info[i].pHDemodCtrl->linked_aggre_port_idx;
                    rst = tsa_Enable_Port(pTseDev->pHTsa, &tsaArg, 0);
                    if( (TSA_ERR)rst != TSA_ERR_OK )
                    {
                        result = TSE_ERR_AGGRE_FAIL;
                        tse_msg_ex(TSE_MSG_ERR, " tsa %d-th port enable fail (0x%x)\n", i, result);
                        continue;
                    }
                }
            }
            else if( pScan_param->demod_idx < TSE_MAX_DEMOD_PER_AGGRE )
            {
                //-------------------------------------
                // disable ts aggre
                tsaArg.type                    = TSA_ARG_TYPE_SET_PORT;
                tsaArg.arg.set_port.port_index = pScan_param->demod_idx; //pTseDev->demod_info[pScan_param->demod_idx].pHDemodCtrl->linked_aggre_port_idx;

                rst = tsa_Disable_Port(pTseDev->pHTsa, &tsaArg, 0);
                if( (TSA_ERR)rst != TSA_ERR_OK )
                {
                    result = TSE_ERR_AGGRE_FAIL;
                    tse_msg_ex(TSE_MSG_ERR, " tsa %d-th port enable fail (0x%x)\n", i, result);
                    break;
                }//*/

                pTseDev->scan_state_recv[pScan_param->demod_idx] = pScan_param->scan_state_recv;
                pTseDev->pHTsExt->scan_state_recv                = scan_report;

                pTseDev->act_demod_idx                                = pScan_param->demod_idx;
                pTseDev->demod_info[pScan_param->demod_idx].freq      = pScan_param->channel.single.scanFreq;
                pTseDev->demod_info[pScan_param->demod_idx].bandwidth = pScan_param->channel.single.bandwidth;
                rst = _setup_scan_freq(pTseDev);
                if( rst )
                {
                    tsExt_Set_Pkt_Proc_Mode(pTseDev->pHTsExt, pScan_param->demod_idx, TSEXT_PKT_PROC_IDLE, 0);
                    result = TSE_ERR_DEMOD_FAIL;
                    tse_msg_ex(TSE_MSG_ERR, " tse scan fail (0x%x), port %d-th !\n", rst, pScan_param->demod_idx);
                    break;
                }

                // set ts extract
                tsExt_Set_Pkt_Proc_Mode(pTseDev->pHTsExt, pTseDev->act_demod_idx, TSEXT_PKT_PROC_ANALYZE, 0);

                //-------------------------------------
                // enable ts aggre
                tsaArg.type                    = TSA_ARG_TYPE_SET_PORT;
                tsaArg.arg.set_port.port_index = pScan_param->demod_idx; //pTseDev->demod_info[pScan_param->demod_idx].pHDemodCtrl->linked_aggre_port_idx;

                rst = tsa_Enable_Port(pTseDev->pHTsa, &tsaArg, 0);
                if( (TSA_ERR)rst != TSA_ERR_OK )
                {
                    result = TSE_ERR_AGGRE_FAIL;
                    tse_msg_ex(TSE_MSG_ERR, " tsa %d-th port enable fail (0x%x)\n", pScan_param->demod_idx, result);
                    break;
                }
            }
            else
            {
                tse_msg_ex(TSE_MSG_ERR, "err, wrong demod port index (%d, max=%d) !",
                            pScan_param->demod_idx, TSE_MAX_DEMOD_PER_AGGRE-1);
                break;
            }

            // enable ts input  ??????  Should it be set by AP ??????
            if( pTseDev->recv_opr.ts_recv_turn_on &&
                pTseDev->bSkip_recv_turn_on == false )
            {
                pTseDev->recv_opr.ts_recv_turn_on(&pTseDev->recv_opr, extraData);
                pTseDev->bSkip_recv_turn_on = true;
            }

        }while(0);
    }

    if( result != TSE_ERR_OK )
    {
        //pTseDev->tseStatus = TSE_STATUS_FAIL;
        tse_msg_ex(TSE_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSE_MSG_TRACE_TSE, pTseDev->tse_mutex);
    _trace_leave(TSE_MSG_TRACE_TSE);
    return result;
}


TSE_ERR
tse_Set_Service_Receive(
    TSE_HANDLE     *pHTse,
    uint32_t       port_idx,
    uint32_t       service_idx,
    bool           bReceive,
    void           *extraData)
{
    TSE_ERR         result = TSE_ERR_OK;
    ITE_TSE_DEV     *pTseDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSE, "0x%x, 0x%x\n", pHTse, extraData);
    _verify_handle(pHTse, result);

    pTseDev = DOWN_CAST(ITE_TSE_DEV, pHTse, hTse);

    _mutex_lock(TSE_MSG_TRACE_TSE, pTseDev->tse_mutex);

    if( pTseDev && pTseDev->tseStatus != TSE_STATUS_FAIL )
    {
        TSEXT_PKT_ANAL_INFO     *pAct_pkt_analyzer_info = 0;

        do{
            if( port_idx >= pTseDev->total_demod )
            {
                tse_msg_ex(TSE_MSG_ERR, "port index (%d) out range (%d) !", port_idx, pTseDev->total_demod-1);
                break;
            }

            pAct_pkt_analyzer_info = pTseDev->pHTsExt->pPkt_analyzer_info[port_idx];
            if( !pAct_pkt_analyzer_info )
            {
                tse_msg_ex(TSE_MSG_ERR, "%d-th port NO info!", port_idx);
                break;
            }

            if( service_idx != (-1) &&
                service_idx >= pAct_pkt_analyzer_info->total_service )
            {
                tse_msg_ex(TSE_MSG_ERR, "service index (%d) out range (%d) !",
                                service_idx, pAct_pkt_analyzer_info->total_service);
                break;
            }

            if( bReceive == true )
            {
                if( pAct_pkt_analyzer_info->proc_mode == TSEXT_PKT_PROC_IDLE )
                    tsExt_Set_Pkt_Proc_Mode(pTseDev->pHTsExt, port_idx, TSEXT_PKT_PROC_SPLIT, 0);
            }
            else if( bReceive == false && service_idx == (-1) )
            {
                tsExt_Set_Pkt_Proc_Mode(pTseDev->pHTsExt, port_idx, TSEXT_PKT_PROC_IDLE, 0);
                break;
            }

            tsExt_Set_Service_Recv(pTseDev->pHTsExt, port_idx, service_idx, bReceive, extraData);
        }while(0);
    }

    if( result != TSE_ERR_OK )
    {
        pTseDev->tseStatus = TSE_STATUS_FAIL;
        tse_msg_ex(TSE_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSE_MSG_TRACE_TSE, pTseDev->tse_mutex);
    _trace_leave(TSE_MSG_TRACE_TSE);
    return result;
}


TSE_ERR
tse_Get_Service_Info(
    TSE_HANDLE      *pHTse,
    TSE_USER_INFO   *pUser_info,
    void            *extraData)
{
    TSE_ERR         result = TSE_ERR_OK;
    ITE_TSE_DEV     *pTseDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSE, "0x%x, 0x%x\n", pHTse, extraData);
    _verify_handle(pHTse, result);
    _verify_handle(pUser_info, result);

    pTseDev = DOWN_CAST(ITE_TSE_DEV, pHTse, hTse);

    _mutex_lock(TSE_MSG_TRACE_TSE, pTseDev->tse_mutex);

    if( pTseDev && pTseDev->tseStatus != TSE_STATUS_FAIL )
    {
        uint32_t                demod_idx = 0, srvc_idx = 0;
        TSEXT_PKT_ANAL_INFO     *pAct_pkt_analyzer_info = 0;

        pUser_info->total_demod = pTseDev->total_demod;
        for(demod_idx = 0; demod_idx < pTseDev->total_demod; demod_idx++)
        {
            TSE_PORT_INFO           *pCur_port_Info = 0;
            TSE_SERVICE_INFO        *pCur_user_service_Info = 0;
            TSEXT_SRVC_PID_INFO     *pCur_srvc_pid_info = 0;

            pAct_pkt_analyzer_info = pTseDev->pHTsExt->pPkt_analyzer_info[demod_idx];
            if( !pAct_pkt_analyzer_info )   continue;

            pCur_port_Info = &pUser_info->demodInfo[demod_idx];

            pCur_port_Info->total_services = pAct_pkt_analyzer_info->total_service;
            for(srvc_idx = 0; srvc_idx < TSE_MAX_SERVICE_PRE_DEMOD; srvc_idx++)
            {
                uint32_t    j = 0, aud_num = 0;

                if( srvc_idx >= pAct_pkt_analyzer_info->total_service )
                    break;

                pCur_user_service_Info = &pCur_port_Info->serviceInfo[srvc_idx];
                pCur_srvc_pid_info     = &pAct_pkt_analyzer_info->pSrvc_pid_info[srvc_idx];

                aud_num = ARRAY_SIZE(pCur_user_service_Info->audioPID);

                pCur_user_service_Info->pRing_buf     = pCur_srvc_pid_info->pRing_buf;
                pCur_user_service_Info->ring_buf_size = pCur_srvc_pid_info->ring_buf_size;

                pCur_user_service_Info->video_pid  = pCur_srvc_pid_info->videoPID;
                pCur_user_service_Info->videoType  = pCur_srvc_pid_info->videoType;
                pCur_user_service_Info->audioCount = 0;
                for(j = 0; j < aud_num; j++)
                {
                    if( j < pCur_srvc_pid_info->audioCount )
                    {
                        pCur_user_service_Info->audioPID[j] = pCur_srvc_pid_info->audioPID[j];
                        pCur_user_service_Info->audioCount++;
                    }
                }
            }
        }
    }

    if( result != TSE_ERR_OK )
    {
        pTseDev->tseStatus = TSE_STATUS_FAIL;
        tse_msg_ex(TSE_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSE_MSG_TRACE_TSE, pTseDev->tse_mutex);
    _trace_leave(TSE_MSG_TRACE_TSE);
    return result;
}


TSE_ERR
tse_Get_Service_Sample(
    TSE_HANDLE          *pHTse,
    TSE_SAMPLE_INFO     *pSample_info,
    void                *extraData)
{
    TSE_ERR         result = TSE_ERR_OK;
    ITE_TSE_DEV     *pTseDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSE, "0x%x, 0x%x\n", pHTse, extraData);
    _verify_handle(pHTse, result);

    pTseDev = DOWN_CAST(ITE_TSE_DEV, pHTse, hTse);

    /**
    * for performance, mark mutex protect
    **/
    //_mutex_lock(TSE_MSG_TRACE_TSE, pTseDev->tse_mutex);

    if( pTseDev && pTseDev->tseStatus != TSE_STATUS_FAIL )
    {
        TSEXT_SAMPLE_INFO       tsext_sample_info = {0};

        tsext_sample_info.port_idx     = pSample_info->port_idx;
        tsext_sample_info.service_idx  = pSample_info->service_idx;
        tsext_sample_info.customer_idx = pSample_info->customer_idx;
        tsext_sample_info.bufLength    = pSample_info->bufLength;

        tsExt_Get_Sample(pTseDev->pHTsExt, &tsext_sample_info, 0);

        pSample_info->pBufAddr  = tsext_sample_info.pBufAddr;
        pSample_info->bufLength = tsext_sample_info.bufLength;
    }

    if( result != TSE_ERR_OK )
    {
        pTseDev->tseStatus = TSE_STATUS_FAIL;
        tse_msg_ex(TSE_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    //_mutex_unlock(TSE_MSG_TRACE_TSE, pTseDev->tse_mutex);
    _trace_leave(TSE_MSG_TRACE_TSE);
    return result;
}


TSE_ERR
tse_Control(
    TSE_HANDLE      *pHTse,
    TSE_CTRL_CMD    cmd,
    uint32_t        value,
    void            *extraData)
{
    TSE_ERR         result = TSE_ERR_OK;
    ITE_TSE_DEV     *pTseDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSE, "0x%x, 0x%x, 0x%x, 0x%x\n", pHTse, cmd, value, extraData);
    _verify_handle(pHTse, result);

    pTseDev = DOWN_CAST(ITE_TSE_DEV, pHTse, hTse);

    _mutex_lock(TSE_MSG_TRACE_TSE, pTseDev->tse_mutex);

    if( pTseDev && pTseDev->tseStatus != TSE_STATUS_FAIL )
    {
        switch( cmd )
        {
            case TSE_CTRL_SET_DEMOD_STATUS:
                {
                    DEMOD_ENG_STATUS    status = DEMOD_ENG_STATUS_UNKNOW;
                    uint32_t            port_index = (uint32_t)extraData;

                    switch( (TSE_DEMOD_STATUS)value )
                    {
                        case TSE_DEMOD_STATUS_IDLE:     status = DEMOD_ENG_STATUS_IDLE;     break;
                        case TSE_DEMOD_STATUS_RUNNING:  status = DEMOD_ENG_STATUS_RUNNING;  break;
                    }
                    DemodCtrl_Set_Engine_Status(pTseDev->demod_info[port_index].pHDemodCtrl, status);
                }
                break;
        }
    }

    if( result != TSE_ERR_OK )
    {
        pTseDev->tseStatus = TSE_STATUS_FAIL;
        tse_msg_ex(TSE_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSE_MSG_TRACE_TSE, pTseDev->tse_mutex);
    _trace_leave(TSE_MSG_TRACE_TSE);
    return result;
}


TSE_ERR
tse_Attach_Cmd_Pkt_Recv(
    TSE_HANDLE          *pHTse,
    uint32_t            port_index,
    TSE_CMD_PKT_INFO   *pCmd_pkt_info,
    void                *extraData)
{
    TSE_ERR         result = TSE_ERR_OK;
    ITE_TSE_DEV     *pTseDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSE, "0x%x, %d, 0x%x, 0x%x\n", pHTse, port_index, pCmd_pkt_info, extraData);
    _verify_handle(pHTse, result);
    _verify_handle(pCmd_pkt_info, result);

    pTseDev = DOWN_CAST(ITE_TSE_DEV, pHTse, hTse);

    _mutex_lock(TSE_MSG_TRACE_TSE, pTseDev->tse_mutex);

    if( pTseDev && pTseDev->tseStatus != TSE_STATUS_FAIL )
    {
        do{
            TSEXT_CMD_PKT_ATTR      tsext_cmd_pkt_attr= {0};

            if( !pCmd_pkt_info || port_index > (pTseDev->total_demod - 1) )
            {
                tse_msg_ex(TSE_MSG_ERR, " wrong parameters (0x%x, port=%d) !", pCmd_pkt_info, port_index);
                break;
            }

            tsext_cmd_pkt_attr.cmd_pkt_pid = pCmd_pkt_info->cmd_pkt_pid;

            tsExt_Add_Cmd_Pkt_Info(pTseDev->pHTsExt, port_index, &tsext_cmd_pkt_attr, extraData);

            // set pid to demod for filtering
            DemodCtrl_UpdatePidTable(pTseDev->demod_info[port_index].pHDemodCtrl, tsext_cmd_pkt_attr.cmd_pkt_pid, TSE_PID_CMD_INDEX_0);
        }while(0);
    }

    if( result != TSE_ERR_OK )
    {
        pTseDev->tseStatus = TSE_STATUS_FAIL;
        tse_msg_ex(TSE_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSE_MSG_TRACE_TSE, pTseDev->tse_mutex);
    _trace_leave(TSE_MSG_TRACE_TSE);
    return result;
}



TSE_ERR
tse_Set_Ts_Receive(
    TSE_HANDLE     *pHTse,
    bool           bReceive,
    void           *extraData)
{
    TSE_ERR         result = TSE_ERR_OK;
    ITE_TSE_DEV     *pTseDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSE, "0x%x, 0x%x\n", pHTse, extraData);
    _verify_handle(pHTse, result);

    pTseDev = DOWN_CAST(ITE_TSE_DEV, pHTse, hTse);

    _mutex_lock(TSE_MSG_TRACE_TSE, pTseDev->tse_mutex);

    if( pTseDev && pTseDev->tseStatus != TSE_STATUS_FAIL )
    {
        if( bReceive == true )
        {
            //tspd_Srvc_Start_Cache();

            // enable ts input  (all ???????)
            if( pTseDev->recv_opr.ts_recv_turn_on )
                pTseDev->recv_opr.ts_recv_turn_on(&pTseDev->recv_opr, extraData);
        }
        else
        {
            //tspd_Srvc_Stop_Cache();

            // disable ts input (all ???????)
            if( pTseDev->recv_opr.ts_recv_turn_off )
                pTseDev->recv_opr.ts_recv_turn_off(&pTseDev->recv_opr, extraData);
        }
    }

    if( result != TSE_ERR_OK )
    {
        pTseDev->tseStatus = TSE_STATUS_FAIL;
        tse_msg_ex(TSE_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSE_MSG_TRACE_TSE, pTseDev->tse_mutex);
    _trace_leave(TSE_MSG_TRACE_TSE);
    return result;
}



// TSE_ERR
// tse_tamplete(
//     TSE_HANDLE     *pHTse,
//     void           *extraData)
// {
//     TSE_ERR         result = TSE_ERR_OK;
//     ITE_TSE_DEV     *pTseDev = 0;
//
//     _trace_enter(TSE_MSG_TRACE_TSE, "0x%x, 0x%x\n", pHTse, extraData);
//     _verify_handle(pHTse, result);
//
//     pTseDev = DOWN_CAST(ITE_TSE_DEV, pHTse, hTse);
//
//     _mutex_lock(TSE_MSG_TRACE_TSE, pTseDev->tse_mutex);
//
//     if( pTseDev && pTseDev->tseStatus != TSE_STATUS_FAIL )
//     {
//
//     }
//
//     if( result != TSE_ERR_OK )
//     {
//         pTseDev->tseStatus = TSE_STATUS_FAIL;
//         tse_msg_ex(TSE_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
//     }
//
//     _mutex_unlock(TSE_MSG_TRACE_TSE, pTseDev->tse_mutex);
//     _trace_leave(TSE_MSG_TRACE_TSE);
//     return result;
// }


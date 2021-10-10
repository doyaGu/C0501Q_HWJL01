

#include "ts_split_defs.h"
#include "ts_split.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#if (CFG_DEMOD_SUPPORT_COUNT > 0)
    #if (CFG_DEMOD_SUPPORT_COUNT > 3)
        #define TSS_MAX_PKT_ANALYZER_NUM         4
    #else
        #define TSS_MAX_PKT_ANALYZER_NUM         CFG_DEMOD_SUPPORT_COUNT // 4
    #endif
#else
    #define TSS_MAX_PKT_ANALYZER_NUM         1
#endif


typedef enum _TSS_STATUS_TAG
{
    TSS_STATUS_IDLE = 0x11,
    TSS_STATUS_BUSY = 0xBB,
    TSS_STATUS_FAIL = 0xFF,

}TSS_STATUS;

#define PASSPORT_INCR_STEP              3
#define TSS_VALID_SYNC_BYTE             (0x47)

typedef enum TSS_USER_OPR_STATE_T
{
    TSS_USER_OPR_STATE_unknow   = 0,
    TSS_USER_OPR_STATE_running,
}TSS_USER_OPR_STATE;

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct TSS_DEV_T
{
    TSS_HANDLE              hTss;

    TSS_STATUS              tssStatus;

    bool                    bBy_Pass_Tss;

    uint32_t                total_passports;
    uint32_t                max_passport_array;
    TSS_PASSPORT_INFO       *pPassportInfo;

    // for user
    bool                    bInit_user_opr[TSS_MAX_PKT_ANALYZER_NUM];
    TSS_USER_OPR            user_opr[TSS_MAX_PKT_ANALYZER_NUM];

}TSS_DEV;
//=============================================================================
//                Global Data Definition
//=============================================================================
#if !defined(TSS_LOCAL_MACRO_DISABLE)
uint32_t  tssMsgOnFlag = 0x1;
#endif

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
TSE_ERR
tss_CreateHandle(
    TSS_HANDLE        **pHTss,
    TSS_INIT_PARAM    *pInitParam,
    void              *extraData)
{
    TSE_ERR         result = TSS_ERR_OK;
    TSS_DEV         *pTssDev = 0;

    // _trace_enter(TSE_MSG_TRACE_TSS, "0x%x, 0x%x, 0x%x\n", pHTss, pInitParam, extraData);

    do{
        uint32_t        i = 0;

        if( *pHTss != 0 )
        {
            tss_msg_ex(TSS_MSG_ERR, " error, Exist tse handle !!");
            result = TSE_ERR_INVALID_PARAMETER;
            break;
        }

        if( !pInitParam )
        {
            tss_msg_ex(TSS_MSG_ERR, " error, Need pre-set info !!");
            result = TSE_ERR_INVALID_PARAMETER;
            break;
        }

        pTssDev = tss_malloc(sizeof(TSS_DEV));
        if( !pTssDev )
        {
            tss_msg_ex(TSS_MSG_ERR, " error, allocate fail !!");
            result = TSE_ERR_ALLOCATE_FAIL;
            break;
        }

        memset(pTssDev, 0x0, sizeof(TSS_DEV));

        //-------------------------------
        // init paraments
        for(i = 0; i < TSS_MAX_PKT_ANALYZER_NUM; i++)
            pTssDev->bInit_user_opr[i] = true;

        pTssDev->bBy_Pass_Tss   = pInitParam->bBy_Pass_Tss;

        pTssDev->pPassportInfo = tss_malloc(PASSPORT_INCR_STEP*sizeof(TSS_PASSPORT_INFO));
        if( !pTssDev->pPassportInfo )
        {
            tss_msg_ex(TSS_MSG_ERR, " error, allocate fail !!");
            result = TSE_ERR_ALLOCATE_FAIL;
            break;
        }
        pTssDev->max_passport_array = PASSPORT_INCR_STEP;
        memset(pTssDev->pPassportInfo, 0x0, PASSPORT_INCR_STEP*sizeof(TSS_PASSPORT_INFO));


        //-------------------------------
        (*pHTss) = &pTssDev->hTss;

    }while(0);

    if( result != TSS_ERR_OK )
    {
        pTssDev->tssStatus = TSS_STATUS_FAIL;
        tss_msg_ex(TSS_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    // _trace_leave(TSE_MSG_TRACE_TSS);
    return result;
}

TSE_ERR
tss_DestroyHandle(
    TSS_HANDLE        **pHTss,
    void              *extraData)
{
    TSE_ERR             result = TSS_ERR_OK;
    TSS_DEV             *pTssDev = 0;
    // pthread_mutex_t     tss_mutex = 0;

    // _trace_enter(TSE_MSG_TRACE_TSS, "0x%x\n", pHTss);

    /**
     * Ap layer need to check all threads, which assess this handle, in STOP state.
     * Or system maybe crash.
     **/

    _verify_handle(pHTss, result);
    _verify_handle((*pHTss), result);

    pTssDev = DOWN_CAST(TSS_DEV, (*pHTss), hTss);

    // _mutex_lock(TSE_MSG_TRACE_TSS, pTssDev->tss_mutex);

    // _disable_irq();
    if( pTssDev )
    {
        if( pTssDev->pPassportInfo )    free(pTssDev->pPassportInfo);

        // tss_mutex = pTssDev->tss_mutex;
        free(pTssDev);
        *pHTss = 0;
    }

    // _mutex_unlock(TSE_MSG_TRACE_TSS, tss_mutex);
    // _mutex_deinit(TSE_MSG_TRACE_TSS, tss_mutex);

    // _enable_irq();
    // _trace_leave(TSE_MSG_TRACE_TSS);
    return result;
}

TSE_ERR
tss_Attach_User_Operator(
    TSS_HANDLE      *pHTss,
    TSS_USER_OPR    *pUser_opr,
    void            *extraData)
{
    TSE_ERR     result = TSS_ERR_OK;
    TSS_DEV     *pTssDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSS, "0x%x, 0x%x, 0x%x\n", pHTss, pUser_opr, extraData);
    _verify_handle(pHTss, result);
    _verify_handle(pUser_opr, result);

    pTssDev = DOWN_CAST(TSS_DEV, pHTss, hTss);

    // _mutex_lock(TSE_MSG_TRACE_TSS, pTssDev->tss_mutex);

    if( pTssDev && pTssDev->tssStatus != TSS_STATUS_FAIL )
    {
        uint32_t    i = 0;

        do{
            if( pUser_opr->port_index < TSS_MAX_PKT_ANALYZER_NUM )
            {
                pTssDev->user_opr[pUser_opr->port_index] = *(pUser_opr);
                pTssDev->bInit_user_opr[pUser_opr->port_index] = true;
            }
        }while(0);
    }

    if( result != TSS_ERR_OK )
    {
        pTssDev->tssStatus = TSS_STATUS_FAIL;
        tss_msg_ex(TSS_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    // _mutex_unlock(TSE_MSG_TRACE_TSS, pTssDev->tss_mutex);
    _trace_leave(TSE_MSG_TRACE_TSS);
    return result;
}

TSE_ERR
tss_Add_Passport_Info(
    TSS_HANDLE          *pHTss,
    TSS_PASSPORT_INFO   *pPassportInfo,
    void                *extraData)
{
    TSE_ERR         result = TSS_ERR_OK;
    TSS_DEV     *pTssDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSS, "0x%x, 0x%x, 0x%x\n", pHTss, pPassportInfo, extraData);
    _verify_handle(pHTss, result);
    _verify_handle(pPassportInfo, result);

    pTssDev = DOWN_CAST(TSS_DEV, pHTss, hTss);

    // _mutex_lock(TSE_MSG_TRACE_TSS, pTssDev->tss_mutex);

    if( pTssDev && pTssDev->pPassportInfo && pTssDev->tssStatus != TSS_STATUS_FAIL )
    {
        do{
            pTssDev->pPassportInfo[pTssDev->total_passports] = *(pPassportInfo);
            pTssDev->total_passports++;
            if( pTssDev->total_passports == pTssDev->max_passport_array )
            {
                TSS_PASSPORT_INFO       *pNewPassportInfo = 0;
                uint32_t                array_size = pTssDev->max_passport_array + PASSPORT_INCR_STEP;

                pNewPassportInfo = tss_malloc(array_size*sizeof(TSS_PASSPORT_INFO));
                if( !pNewPassportInfo )
                {
                    pTssDev->total_passports--;
                    break;
                }
                memset(pNewPassportInfo, 0x0, array_size*sizeof(TSS_PASSPORT_INFO));
                memcpy(pNewPassportInfo, pTssDev->pPassportInfo,
                       pTssDev->total_passports*sizeof(TSS_PASSPORT_INFO));

                free(pTssDev->pPassportInfo);

                pTssDev->pPassportInfo      = pNewPassportInfo;
                pTssDev->max_passport_array = array_size;
            }
        }while(0);

    }

    if( result != TSS_ERR_OK )
    {
        pTssDev->tssStatus = TSS_STATUS_FAIL;
        tss_msg_ex(TSS_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    // _mutex_unlock(TSE_MSG_TRACE_TSS, pTssDev->tss_mutex);
    _trace_leave(TSE_MSG_TRACE_TSS);
    return result;
}

TSE_ERR
tss_Split(
    TSS_HANDLE      *pHTss,
    TSS_BUF_INFO    *pBuf_info,
    void            *extraData)
{
    TSE_ERR         result = TSS_ERR_OK;
    TSS_DEV     *pTssDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSS, "0x%x, 0x%x\n", pHTss, extraData);
    _verify_handle(pHTss, result);

    pTssDev = DOWN_CAST(TSS_DEV, pHTss, hTss);

    // _mutex_lock(TSE_MSG_TRACE_TSS, pTssDev->tss_mutex);

    if( pTssDev && pTssDev->tssStatus != TSS_STATUS_FAIL )
    {
        do{
            uint32_t    act_idx = (uint32_t)(-1);
            uint32_t    pkt_size = pBuf_info->bufLength;
            uint32_t    tag_len  = 0;
            uint8_t     *pData   = pBuf_info->pBufAddr;
            uint8_t     *pCur    = 0;
            TSS_MBOX    *pUser_create = 0;
            TSS_MBOX    *pUser_destroy = 0;
            TSS_MBOX    *pUser_proc = 0;
            uint32_t    feedback_cmd = TSS_CMD_USER_OPR_IDLE;

            if( pData[0] != TSS_VALID_SYNC_BYTE )
                break;
#if 0
            if( pTssDev->bBy_Pass_Tss == false )
            {
                //----------------------------------------
                // compare all passport info at every packet
                for(act_idx = 0; act_idx < pTssDev->total_passports; act_idx++)
                {
                    if( pTssDev->pPassportInfo[act_idx].tag_len )
                    {
                        uint32_t        cur_tag = 0;

                        // find passport info start addr in one packet
                        pCur = (pData + pkt_size - pTssDev->pPassportInfo[act_idx].tag_len);

                        if( pTssDev->pPassportInfo[act_idx].tag_len == 1 )
                            cur_tag = (*pCur);
                        else
                            cur_tag = (pCur[0] << 24) | (pCur[1] << 16) | (pCur[2] << 8) | pCur[3];
                            //cur_tag = (*((uint32_t*)pCur));

                        if( cur_tag == pTssDev->pPassportInfo[act_idx].tag_value )
                            break;
                    }
                }

                // no passport match
                if( act_idx == pTssDev->total_passports )      break;
            }
            else
                act_idx = 0;
#else
            if( pTssDev->bBy_Pass_Tss == false )
            {
                /**
                 * special paraments setting in aggre module,
                 *      tag_value  = 0, 1, 2 , 3, ...
                 *      tag_length = 4 bytes
                 **/
                if( pTssDev->pPassportInfo && pTssDev->pPassportInfo[0].tag_len )
                {
                    uint32_t        cur_tag = 0;

                    // find passport info start addr in one packet
                    pCur = (pData + pkt_size - pTssDev->pPassportInfo[0].tag_len);
                    //act_idx = (pCur[0]) | (pCur[1] << 8) | (pCur[2] << 16) | (pCur[3] << 24);
                    act_idx = (*((uint32_t*)pCur));
                    //tss_msg(1, " act_idx=%d, ", act_idx);
                    if( act_idx >= TSS_MAX_PKT_ANALYZER_NUM )       break;

                    tag_len = pTssDev->pPassportInfo[act_idx].tag_len;
                }
            }
            else
                act_idx = 0;
#endif
            // -----------------------------------
            // call back to user operator create
            pUser_create  = &pTssDev->user_opr[act_idx].user_create;
            pUser_destroy = &pTssDev->user_opr[act_idx].user_destroy;
            pUser_proc    = &pTssDev->user_opr[act_idx].user_proc;

            if( pTssDev->bInit_user_opr[act_idx] == true &&
                pUser_create->func )
            {
                pUser_create->tss_mbox_arg.arg.user.index        = act_idx;
                pUser_create->tss_mbox_arg.arg.user.feedback_cmd = TSS_CMD_USER_OPR_IDLE;
                pUser_create->func(&pUser_create->tss_mbox_arg, extraData);

                pTssDev->bInit_user_opr[act_idx] = false;
            }

            // -----------------------------------
            // call back to user operator proc
            if( pUser_proc->func )
            {
                pUser_proc->tss_mbox_arg.arg.user.index        = act_idx;
                pUser_proc->tss_mbox_arg.arg.user.feedback_cmd = TSS_CMD_USER_OPR_IDLE;
                pUser_proc->tss_mbox_arg.arg.user.pBuf_addr    = pData;
                pUser_proc->tss_mbox_arg.arg.user.buf_size     = (pkt_size - tag_len);
                pUser_proc->func(&pUser_proc->tss_mbox_arg, extraData);

                feedback_cmd = pUser_proc->tss_mbox_arg.arg.user.feedback_cmd;
            }

            if( (feedback_cmd == TSS_CMD_USER_OPR_DESTROY ||
                 pUser_destroy->tss_mbox_arg.arg.user.feedback_cmd == TSS_CMD_USER_OPR_DESTROY) &&
                pUser_destroy->func )
            {
                // -----------------------------------
                // call back to user operator destroy
                pUser_destroy->tss_mbox_arg.arg.user.index        = act_idx;
                pUser_destroy->tss_mbox_arg.arg.user.feedback_cmd = TSS_CMD_USER_OPR_IDLE;
                pUser_destroy->func(&pUser_destroy->tss_mbox_arg, extraData);

                //------------------------------------
                // reset user_opr
                memset(&pTssDev->user_opr[act_idx], 0x0, sizeof(TSS_USER_OPR));
            }

        }while(0);
    }

    if( result != TSS_ERR_OK )
    {
        pTssDev->tssStatus = TSS_STATUS_FAIL;
        tss_msg_ex(TSS_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    // _mutex_unlock(TSE_MSG_TRACE_TSS, pTssDev->tss_mutex);
    _trace_leave(TSE_MSG_TRACE_TSS);
    return result;
}


// TSE_ERR
// tss_tamplete(
//     TSS_HANDLE     *pHTss,
//     void           *extraData)
// {
//     TSE_ERR         result = TSS_ERR_OK;
//     TSS_DEV     *pTssDev = 0;
//
//     _trace_enter(TSE_MSG_TRACE_TSS, "0x%x, 0x%x\n", pHTss, extraData);
//     _verify_handle(pHTss, result);
//
//     pTssDev = DOWN_CAST(TSS_DEV, pHTss, hTss);
//
//     // _mutex_lock(TSE_MSG_TRACE_TSS, pTssDev->tss_mutex);
//
//     if( pTssDev && pTssDev->tssStatus != TSS_STATUS_FAIL )
//     {
//
//     }
//
//     if( result != TSS_ERR_OK )
//     {
//         pTssDev->tssStatus = TSS_STATUS_FAIL;
//         tss_msg_ex(TSS_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
//     }
//
//     // _mutex_unlock(TSE_MSG_TRACE_TSS, pTssDev->tss_mutex);
//     _trace_leave(TSE_MSG_TRACE_TSS);
//     return result;
// }

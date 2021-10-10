

#include "ts_packet_demux.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define TSPD_CMD_PKT_INCR_STEP      5
#define TSPD_CMD_PKT_BUF_SIZE       (188*1085) // about 200KB
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================
static int
_tspd_sort_pid(
    const void *parg0,
    const void *parg1)
{
    TSPD_CMD_PKT_ATTR   *pInfo_0 = (TSPD_CMD_PKT_ATTR*)parg0;
    TSPD_CMD_PKT_ATTR   *pInfo_1 = (TSPD_CMD_PKT_ATTR*)parg1;

    return ((int)(pInfo_0->cmd_pkt_pid - pInfo_1->cmd_pkt_pid));
}

static int
_tspd_binary_search(
    TSPD_CMD_PKT_ATTR   *pCmd_pkt_attr,
    uint32_t            tail_index,
    uint32_t            act_pid)
{
    int    target_index = (-1);

    do{
        int    max_idx = (-1);
        int    min_idx = 0;
        int    middle_idx = (-1);

        if( !pCmd_pkt_attr )    break;

        max_idx    = (int)tail_index;
        middle_idx = (int)(max_idx >> 1);

        do{
            if( pCmd_pkt_attr[middle_idx].cmd_pkt_pid == act_pid )
            {
                // get target
                target_index = (int)middle_idx;
                break;
            }
            else if( pCmd_pkt_attr[middle_idx].cmd_pkt_pid < act_pid )
                min_idx = middle_idx + 1;
            else
                max_idx = middle_idx - 1;

            middle_idx = (int)((min_idx + max_idx) >> 1);
        }while( min_idx <= max_idx );
    }while(0);

    return target_index;
}



//=============================================================================
//                Public Function Definition
//=============================================================================
TSE_ERR
tspd_CreateHandle(
    TSPD_HANLDE          **ppHTspd,
    TSPD_SETUP_PARAM     *pSetupParam,
    void                 *extraData)
{
    TSE_ERR          result = TSPD_ERR_OK;
    TSPD_HANLDE      *pTspdDev = 0;

    do{
        uint32_t    i = 0;
        uint32_t    srvc_cache_buf_size = 0;

        if( *ppHTspd != 0 )
        {
            tspd_msg_ex(TSPD_MSG_ERR, " error, Exist tspa handle !!");
            result = TSPD_ERR_INVALID_PARAMETER;
            break;
        }

        if( !pSetupParam )
        {
            tspd_msg_ex(TSPD_MSG_ERR, " error, Need pre-set info !!");
            result = TSPD_ERR_INVALID_PARAMETER;
            break;
        }

        if( !pSetupParam->total_service ||
            pSetupParam->total_service > TSPD_MAX_SERVICE_NUM )
        {
            tspd_msg_ex(TSPD_MSG_ERR, " err, total service=%d, max=%d !!",
                        pSetupParam->total_service, pSetupParam->total_service );
            break;
        }

        pTspdDev = tspd_malloc(sizeof(TSPD_HANLDE));
        if( !pTspdDev )
        {
            tspd_msg_ex(TSPD_MSG_ERR, " error, allocate fail !!");
            result = TSPD_ERR_ALLOCATE_FAIL;
            break;
        }

        memset(pTspdDev, 0x0, sizeof(TSPD_HANLDE));
        //---------------------------------
        // init paraments
        pTspdDev->total_service   = pSetupParam->total_service;
        pTspdDev->pkt_demux_level = pSetupParam->pkt_demux_level;
        pTspdDev->bSpareMem       = pSetupParam->bSpareMem;

        //---------------------------------
        // alloc service cache buf
        pTspdDev->pSrvc_attr = tspd_malloc(pTspdDev->total_service * sizeof(TSPD_SRVC_ATTR));
        if( !pTspdDev->pSrvc_attr )
        {
            tspd_msg_ex(TSPD_MSG_ERR, " error, allocate fail !!");
            result = TSPD_ERR_ALLOCATE_FAIL;
            break;
        }
        memset(pTspdDev->pSrvc_attr, 0x0, pTspdDev->total_service * sizeof(TSPD_SRVC_ATTR));

        // ---------------------------------
        // user callback to get user info
        if( pSetupParam->get_user_info.func )
        {
            pSetupParam->get_user_info.user_mbox_arg.arg.set_pid.total_service = pTspdDev->total_service;
            pSetupParam->get_user_info.user_mbox_arg.arg.set_pid.pSrvc_attr    = (void*)pTspdDev->pSrvc_attr;

            pSetupParam->get_user_info.func(&pSetupParam->get_user_info.user_mbox_arg, extraData);
        }

        switch( pTspdDev->pkt_demux_level )
        {
            case TSPD_DEMUX_LEVEL_SRVC_PES: // not implement
                break;

            case TSPD_DEMUX_LEVEL_SERVICE:
                for(i = 0; i < pTspdDev->total_service; i++)
                {
                    if( pTspdDev->pSrvc_attr[i].srvc_cache_buf_size )
                        srvc_cache_buf_size = pTspdDev->pSrvc_attr[i].srvc_cache_buf_size;
                    else
                    {
                        srvc_cache_buf_size = TSPD_SRVC_BUF_SIZE_LEVEL_1;
                        pTspdDev->pSrvc_attr[i].srvc_cache_buf_size = srvc_cache_buf_size;
                    }

                    pTspdDev->pSrvc_attr[i].pSrvc_cache_buf = 0;

                    if( pTspdDev->bSpareMem == false )
                    {

                        pTspdDev->pSrvc_attr[i].pSrvc_cache_buf = tspd_malloc(srvc_cache_buf_size);
                        if( !pTspdDev->pSrvc_attr[i].pSrvc_cache_buf )
                        {
                            tspd_msg_ex(TSPD_MSG_ERR, " error, allocate fail !!");
                            result = TSPD_ERR_ALLOCATE_FAIL;
                            break;
                        }

                        //printf("**** service %d-th, buf size=%d, pBuf=0x%x\n",
                        //    i, pTspdDev->pSrvc_attr[i].srvc_cache_buf_size,  pTspdDev->pSrvc_attr[i].pSrvc_cache_buf);

                        memset(pTspdDev->pSrvc_attr[i].pSrvc_cache_buf, 0x0, srvc_cache_buf_size);
                        // cache buf operator init
                        rb_opt_init(&pTspdDev->pSrvc_attr[i].ring_buf_opr,
                                    (uint32_t)pTspdDev->pSrvc_attr[i].pSrvc_cache_buf,
                                    pTspdDev->pSrvc_attr[i].srvc_cache_buf_size);
                    }
                }
                break;

            case TSPD_DEMUX_LEVEL_DEMOD:
                // demod data all send to 0-th service cache
                if( pTspdDev->pSrvc_attr[0].srvc_cache_buf_size )
                    srvc_cache_buf_size = pTspdDev->pSrvc_attr[0].srvc_cache_buf_size;
                else
                {
                    srvc_cache_buf_size = TSPD_SRVC_BUF_SIZE_LEVEL_3;
                    pTspdDev->pSrvc_attr[0].srvc_cache_buf_size = srvc_cache_buf_size;
                }

                pTspdDev->pSrvc_attr[0].pSrvc_cache_buf = tspd_malloc(srvc_cache_buf_size);
                if( !pTspdDev->pSrvc_attr[0].pSrvc_cache_buf )
                {
                    tspd_msg_ex(TSPD_MSG_ERR, " error, allocate fail !!");
                    result = TSPD_ERR_ALLOCATE_FAIL;
                    break;
                }

                memset(pTspdDev->pSrvc_attr[0].pSrvc_cache_buf, 0x0, srvc_cache_buf_size);
                for(i = 1; i < pTspdDev->total_service; i++)
                {
                    pTspdDev->pSrvc_attr[i].srvc_cache_buf_size = pTspdDev->pSrvc_attr[0].srvc_cache_buf_size;
                    pTspdDev->pSrvc_attr[i].pSrvc_cache_buf     = pTspdDev->pSrvc_attr[0].pSrvc_cache_buf;
                }

                // cache buf operator init
                rb_opt_init(&pTspdDev->pSrvc_attr[0].ring_buf_opr,
                            (uint32_t)pTspdDev->pSrvc_attr[0].pSrvc_cache_buf,
                            pTspdDev->pSrvc_attr[0].srvc_cache_buf_size);
                break;
        }
        // ---------------------------------
        // user callback to set pid and update ring_buf_info
        if( pSetupParam->set_srvc_pid.func )
        {
            pSetupParam->set_srvc_pid.user_mbox_arg.type                      = TSPD_USER_MBOX_TYPE_SET_PID;
            pSetupParam->set_srvc_pid.user_mbox_arg.arg.set_pid.total_service = pTspdDev->total_service;
            pSetupParam->set_srvc_pid.user_mbox_arg.arg.set_pid.pSrvc_attr    = (void*)pTspdDev->pSrvc_attr;

            pSetupParam->set_srvc_pid.func(&pSetupParam->set_srvc_pid.user_mbox_arg, extraData);
        }

        //----------------------------------
        // reset pid mapping talbe (note to un-used)
        for(i = 0; i < TSPD_MAX_PID_NUM; i++)
            TSPD_BIT_SET(&pTspdDev->pid_mapping[i], (TSPD_BIT_FIELD_SIZE-1));

        //----------------------------------
        // update pid mapping table with service pid
        for(i = 0; i < pTspdDev->total_service; i++)
        {
            uint32_t            j = 0, cache_index = 0;
            TSPD_SRVC_ATTR      *pCur_srvc_attr = &pTspdDev->pSrvc_attr[i];

            switch( pTspdDev->pkt_demux_level )
            {
                default:
                case TSPD_DEMUX_LEVEL_SERVICE:  cache_index = i;    break;
                case TSPD_DEMUX_LEVEL_DEMOD:    cache_index = 0;    break;
            }

            TSPD_BIT_SET(&pTspdDev->pid_mapping[pCur_srvc_attr->pat_pid], cache_index); // pat
            TSPD_BIT_CLR(&pTspdDev->pid_mapping[pCur_srvc_attr->pat_pid], (TSPD_BIT_FIELD_SIZE-1));
            TSPD_BIT_SET(&pTspdDev->pid_mapping[pCur_srvc_attr->sdt_pid], cache_index); // sdt
            TSPD_BIT_CLR(&pTspdDev->pid_mapping[pCur_srvc_attr->sdt_pid], (TSPD_BIT_FIELD_SIZE-1));
            TSPD_BIT_SET(&pTspdDev->pid_mapping[pCur_srvc_attr->nit_pid], cache_index); // nit
            TSPD_BIT_CLR(&pTspdDev->pid_mapping[pCur_srvc_attr->nit_pid], (TSPD_BIT_FIELD_SIZE-1));
            TSPD_BIT_SET(&pTspdDev->pid_mapping[pCur_srvc_attr->tdt_tot_pid], cache_index); // tdt, tot
            TSPD_BIT_CLR(&pTspdDev->pid_mapping[pCur_srvc_attr->tdt_tot_pid], (TSPD_BIT_FIELD_SIZE-1));
            TSPD_BIT_SET(&pTspdDev->pid_mapping[pCur_srvc_attr->pmt_pid], cache_index); // pmt
            TSPD_BIT_CLR(&pTspdDev->pid_mapping[pCur_srvc_attr->pmt_pid], (TSPD_BIT_FIELD_SIZE-1));
            TSPD_BIT_SET(&pTspdDev->pid_mapping[pCur_srvc_attr->video_pid], cache_index); // video pid
            TSPD_BIT_CLR(&pTspdDev->pid_mapping[pCur_srvc_attr->video_pid], (TSPD_BIT_FIELD_SIZE-1));
            for(j = 0; j < TSPD_SRVC_MAX_AUD_NUM; j++)
            {
                TSPD_BIT_SET(&pTspdDev->pid_mapping[pCur_srvc_attr->audio_pid[j]], cache_index); // audio pid
                TSPD_BIT_CLR(&pTspdDev->pid_mapping[pCur_srvc_attr->audio_pid[j]], (TSPD_BIT_FIELD_SIZE-1));
            }
        }

        //----------------------------------
        (*ppHTspd) = (result == TSPD_ERR_OK) ? pTspdDev : 0;
    }while(0);

    if( result != TSPD_ERR_OK )
    {
        if( pTspdDev )
        {
            TSPD_HANLDE        *pTmpHTspd = pTspdDev;

            tspd_DestroyHandle(&pTmpHTspd, 0);
        }
        tspd_msg_ex(TSPD_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    return result;
}

TSE_ERR
tspd_DestroyHandle(
    TSPD_HANLDE     **ppHTspd,
    void            *extraData)
{
    TSE_ERR         result = TSPD_ERR_OK;
    TSPD_HANLDE     *pTspdDev = 0;

    _verify_handle(ppHTspd, result);
    _verify_handle((*ppHTspd), result);

    pTspdDev = (*ppHTspd);

    if( pTspdDev )
    {
        uint32_t    i = 0;

        //-----------------------------
        // free service cache buf
        if( pTspdDev->pSrvc_attr )
        {
            switch( pTspdDev->pkt_demux_level )
            {
                case TSPD_DEMUX_LEVEL_SRVC_PES: // not implement
                    break;

                case TSPD_DEMUX_LEVEL_SERVICE:
                    for(i = 0; i < pTspdDev->total_service; i++)
                    {
                        if( pTspdDev->pSrvc_attr[i].pSrvc_cache_buf )
                            free(pTspdDev->pSrvc_attr[i].pSrvc_cache_buf);
                    }
                    break;

                case TSPD_DEMUX_LEVEL_DEMOD:
                    if( pTspdDev->pSrvc_attr[0].pSrvc_cache_buf )
                        free(pTspdDev->pSrvc_attr[0].pSrvc_cache_buf);
                    break;
            }
            free(pTspdDev->pSrvc_attr);
            pTspdDev->pSrvc_attr = 0;
        }

        //-------------------------------------
        // destroy cmd data buffer
        if( pTspdDev->pCmd_pkt_buf )
        {
            free(pTspdDev->pCmd_pkt_buf);
            pTspdDev->pCmd_pkt_buf = 0;
        }

        if( pTspdDev->pCmd_pkt_attr )
        {
            free(pTspdDev->pCmd_pkt_attr);
            pTspdDev->pCmd_pkt_attr = 0;
        }

        //------------------------------
        // free handle
        free(pTspdDev);
        (*ppHTspd) = 0;
    }

    return result;
}

TSE_ERR
tspd_Pkt_Demux(
    TSPD_HANLDE     *pHTspd,
    TSPD_BUF_INFO   *pBuf_info,
    void            *extraData)
{
#define _TSPD_GET_PID(pData)     ((uint32_t)(((pData[1] & 0x1f) << 8) | pData[2]))

    TSE_ERR         result = TSPD_ERR_OK;
    TSPD_HANLDE     *pTspdDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSAD, "0x%x, 0x%x\n", pHTspd, extraData);
    _verify_handle(pHTspd, result);

    pTspdDev = pHTspd;

    do{
        uint8_t     *pData = pBuf_info->pBufAddr;
        uint32_t    pkt_size = pBuf_info->bufLength;
        uint32_t    cur_pid = 0;
        uint32_t    i = 0;

        cur_pid = _TSPD_GET_PID(pData);

        // if pull high, no used
        if( TSPD_BIT_IS_SET(&pTspdDev->pid_mapping[cur_pid], (TSPD_BIT_FIELD_SIZE-1)) )
            break;

        // if pull high, handle cmd packet
        if( TSPD_BIT_IS_SET(&pTspdDev->pid_mapping[cur_pid], (TSPD_BIT_FIELD_SIZE-2)) )
        {
            // send to cmd data buffer
            rb_opt_update_w(&pTspdDev->cmd_pkt_buf_opr, (uint32_t)pData, pkt_size);
            break;
        }

        switch( pTspdDev->pkt_demux_level )
        {
            case TSPD_DEMUX_LEVEL_SRVC_PES:
            case TSPD_DEMUX_LEVEL_SERVICE:
                for(i = 0; i < pTspdDev->total_service; i++)
                {
                    if( TSPD_BIT_IS_SET(&pTspdDev->pid_mapping[cur_pid], i) )
                    {
                        TSPD_SRVC_ATTR      *pCur_srvc_attr = &pTspdDev->pSrvc_attr[i];

                        if( pCur_srvc_attr->bGetPat == false && cur_pid == pCur_srvc_attr->pat_pid )
                            pCur_srvc_attr->bGetPat = true;

                        //if( pTspdDev->pkt_demux_level == TSPD_DEMUX_LEVEL_SRVC_PES )
                        //{
                        //    // not implement
                        //    // neet to cut the pes header
                        //}

                        // copy packet to cache
                        if( pCur_srvc_attr->bStopCache == false &&
                            pCur_srvc_attr->bGetPat == true &&
                            pCur_srvc_attr->pSrvc_cache_buf )
                            rb_opt_update_w(&pCur_srvc_attr->ring_buf_opr, (uint32_t)pData, pkt_size);
                    }
                }
                break;

            case TSPD_DEMUX_LEVEL_DEMOD:
                if( TSPD_BIT_IS_SET(&pTspdDev->pid_mapping[cur_pid], 0) )
                {
                    TSPD_SRVC_ATTR      *pCur_srvc_attr = &pTspdDev->pSrvc_attr[0];

                    // copy packet to cache
                    if( pCur_srvc_attr->bStopCache == false )
                        rb_opt_update_w(&pCur_srvc_attr->ring_buf_opr, (uint32_t)pData, pkt_size);
                }
                break;
        }

    }while(0);

    if( result != TSPD_ERR_OK )
    {
        tspd_msg_ex(TSPD_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }
    // _mutex_unlock(TSE_MSG_TRACE_TSAD, pTseDev->tse_mutex);
    _trace_leave(TSE_MSG_TRACE_TSAD);
    return result;
}


TSE_ERR
tspd_Get_Srvc_Stream(
    TSPD_HANLDE         *pHTspd,
    TSPD_SAMPLE_INFO    *pSample_info,
    void                *extraData)
{
    TSE_ERR         result = TSPD_ERR_OK;
    TSPD_HANLDE     *pTspdDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSAD, "0x%x, 0x%x, 0x%x\n", pHTspd, pSample_info, extraData);
    _verify_handle(pHTspd, result);
    _verify_handle(pSample_info, result);

    pTspdDev = pHTspd;

    do{
        switch( pTspdDev->pkt_demux_level )
        {
            case TSPD_DEMUX_LEVEL_SRVC_PES: // not implement
                break;

            case TSPD_DEMUX_LEVEL_SERVICE:
                if( pSample_info->service_idx < pTspdDev->total_service &&
                    pSample_info->customer_idx < RB_OPT_SUPPORT_NUM )
                {
                    TSPD_SRVC_ATTR      *pCur_srvc_attr = &pTspdDev->pSrvc_attr[pSample_info->service_idx];
                    uint32_t            sample_addr = 0, sample_size = 0;

                    sample_size = pSample_info->bufLength;
                    rb_opt_update_r(&pCur_srvc_attr->ring_buf_opr,
                                    pSample_info->customer_idx, &sample_addr, &sample_size);

                    if( sample_size > ((3*pCur_srvc_attr->srvc_cache_buf_size) >> 2) )
                        tspd_msg(1, " cache(%d-th) size over than 3/4 (total:%d)\n", pSample_info->service_idx, pCur_srvc_attr->srvc_cache_buf_size);

                    pSample_info->pBufAddr  = (uint8_t*)sample_addr;
                    pSample_info->bufLength = sample_size;
                }
                break;

            case TSPD_DEMUX_LEVEL_DEMOD:
                if( pSample_info->customer_idx < RB_OPT_SUPPORT_NUM )
                {
                    TSPD_SRVC_ATTR      *pCur_srvc_attr = &pTspdDev->pSrvc_attr[0];
                    uint32_t            sample_addr = 0, sample_size = 0;

                    sample_size = pSample_info->bufLength;
                    rb_opt_update_r(&pCur_srvc_attr->ring_buf_opr,
                                    pSample_info->customer_idx, &sample_addr, &sample_size);

                    if( sample_size > ((3*pCur_srvc_attr->srvc_cache_buf_size) >> 2) )
                        tspd_msg(1, " cache(%d-th) size over than 3/4 (total:%d)\n", 0, pCur_srvc_attr->srvc_cache_buf_size);

                    pSample_info->pBufAddr  = (uint8_t*)sample_addr;
                    pSample_info->bufLength = sample_size;
                }
                break;
        }
    }while(0);

    if( result != TSPD_ERR_OK )
    {
        tspd_msg_ex(TSPD_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }
    // _mutex_unlock(TSE_MSG_TRACE_TSAD, pTseDev->tse_mutex);
    _trace_leave(TSE_MSG_TRACE_TSAD);
    return result;
}


TSE_ERR
tspd_Get_Cmd_Pkt_Stream(
    TSPD_HANLDE         *pHTspd,
    TSPD_SAMPLE_INFO    *pSample_info,
    void                *extraData)
{
    TSE_ERR         result = TSPD_ERR_OK;
    TSPD_HANLDE     *pTspdDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSAD, "0x%x, 0x%x, 0x%x\n", pHTspd, pSample_info, extraData);
    _verify_handle(pHTspd, result);
    _verify_handle(pSample_info, result);

    pTspdDev = pHTspd;

    do{
        if( pSample_info->customer_idx < RB_OPT_SUPPORT_NUM )
        {
            uint32_t            sample_addr = 0, sample_size = 0;

            sample_size = pSample_info->bufLength;
            rb_opt_update_r(&pTspdDev->cmd_pkt_buf_opr,
                            pSample_info->customer_idx, &sample_addr, &sample_size);

            //if( sample_size > ((3*TSPD_CMD_PKT_BUF_SIZE) >> 2) )
            //    tspd_msg(1, " cache(cmd data) size over than 3/4\n");

            pSample_info->pBufAddr  = (uint8_t*)sample_addr;
            pSample_info->bufLength = sample_size;
        }
    }while(0);

    if( result != TSPD_ERR_OK )
    {
        tspd_msg_ex(TSPD_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }
    // _mutex_unlock(TSE_MSG_TRACE_TSAD, pTseDev->tse_mutex);
    _trace_leave(TSE_MSG_TRACE_TSAD);
    return result;
}


TSE_ERR
tspd_Srvc_Start_Cache(
    TSPD_HANLDE     *pHTspd,
    uint32_t        service_idx,
    void            *extraData)
{
    TSE_ERR         result = TSPD_ERR_OK;
    TSPD_HANLDE     *pTspdDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSAD, "0x%x, 0x%x\n", pHTspd, extraData);
    _verify_handle(pHTspd, result);

    pTspdDev = pHTspd;

    do{
        TSPD_SRVC_ATTR      *pCur_srvc_attr = 0;

        switch( pTspdDev->pkt_demux_level )
        {
            case TSPD_DEMUX_LEVEL_SERVICE:
                if( service_idx < pTspdDev->total_service )
                {
                    pCur_srvc_attr = &pTspdDev->pSrvc_attr[service_idx];

                    pCur_srvc_attr->bStopCache = false;
                    pCur_srvc_attr->bGetPat    = false;

                    if( pTspdDev->bSpareMem == true && !pCur_srvc_attr->pSrvc_cache_buf )
                    {
                        uint32_t        srvc_cache_buf_size = 0;
                        //printf("**** service %d-th, buf size = %d\n", i, srvc_cache_buf_size);
                        srvc_cache_buf_size = pCur_srvc_attr->srvc_cache_buf_size;
                        pCur_srvc_attr->pSrvc_cache_buf = tspd_malloc(srvc_cache_buf_size);
                        if( !pCur_srvc_attr->pSrvc_cache_buf )
                        {
                            tspd_msg_ex(TSPD_MSG_ERR, " error, allocate fail !!");
                            result = TSPD_ERR_ALLOCATE_FAIL;
                            break;
                        }
                        memset(pCur_srvc_attr->pSrvc_cache_buf, 0x0, srvc_cache_buf_size);
                        // cache buf operator init
                        rb_opt_init(&pCur_srvc_attr->ring_buf_opr,
                                    (uint32_t)pCur_srvc_attr->pSrvc_cache_buf,
                                    pCur_srvc_attr->srvc_cache_buf_size);
                    }
                }
                break;

            case TSPD_DEMUX_LEVEL_DEMOD:
                pCur_srvc_attr = &pTspdDev->pSrvc_attr[0];
                pCur_srvc_attr->bStopCache = false;
                break;
        }
    }while(0);

    if( result != TSPD_ERR_OK )
    {
        tspd_msg_ex(TSPD_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }
    // _mutex_unlock(TSE_MSG_TRACE_TSAD, pTseDev->tse_mutex);
    _trace_leave(TSE_MSG_TRACE_TSAD);
    return result;
}


TSE_ERR
tspd_Srvc_Stop_Cache(
    TSPD_HANLDE     *pHTspd,
    uint32_t        service_idx,
    void            *extraData)
{
    TSE_ERR         result = TSPD_ERR_OK;
    TSPD_HANLDE     *pTspdDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSAD, "0x%x, 0x%x\n", pHTspd, extraData);
    _verify_handle(pHTspd, result);

    pTspdDev = pHTspd;

    do{
        TSPD_SRVC_ATTR      *pCur_srvc_attr = 0;

        switch( pTspdDev->pkt_demux_level )
        {
            case TSPD_DEMUX_LEVEL_SERVICE:
                if( service_idx < pTspdDev->total_service )
                {
                    pCur_srvc_attr = &pTspdDev->pSrvc_attr[service_idx];

                    if( pTspdDev->bSpareMem == true && pCur_srvc_attr->pSrvc_cache_buf )
                    {
                        free(pCur_srvc_attr->pSrvc_cache_buf);
                        pCur_srvc_attr->pSrvc_cache_buf = 0;
                    }

                    // cache buf operator init
                    rb_opt_init(&pCur_srvc_attr->ring_buf_opr,
                                (uint32_t)pCur_srvc_attr->pSrvc_cache_buf,
                                pCur_srvc_attr->srvc_cache_buf_size);

                    pCur_srvc_attr->bStopCache = true;
                }
                break;

            case TSPD_DEMUX_LEVEL_DEMOD:
                pCur_srvc_attr = &pTspdDev->pSrvc_attr[0];

                // cache buf operator init
                rb_opt_init(&pCur_srvc_attr->ring_buf_opr,
                            (uint32_t)pCur_srvc_attr->pSrvc_cache_buf,
                            pCur_srvc_attr->srvc_cache_buf_size);

                pCur_srvc_attr->bStopCache = true;
                break;
        }
    }while(0);

    if( result != TSPD_ERR_OK )
    {
        tspd_msg_ex(TSPD_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }
    // _mutex_unlock(TSE_MSG_TRACE_TSAD, pTseDev->tse_mutex);
    _trace_leave(TSE_MSG_TRACE_TSAD);
    return result;
}


TSE_ERR
tspd_Attach_Cmd_Pkt_Info(
    TSPD_HANLDE         *pHTspd,
    TSPD_CMD_PKT_ATTR   *pCmd_pkt_attr,
    void                *extraData)
{
    TSE_ERR         result = TSPD_ERR_OK;
    TSPD_HANLDE     *pTspdDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSAD, "0x%x, 0x%x, 0x%x\n", pHTspd, pCmd_pkt_attr, extraData);
    _verify_handle(pHTspd, result);
    _verify_handle(pCmd_pkt_attr, result);

    pTspdDev = pHTspd;

    do{
        if( !pTspdDev->pCmd_pkt_attr )
        {
            pTspdDev->pCmd_pkt_attr = tspd_malloc(TSPD_CMD_PKT_INCR_STEP*sizeof(TSPD_CMD_PKT_ATTR));
            if( !pTspdDev->pCmd_pkt_attr )
            {
                tspd_msg_ex(TSPD_MSG_ERR, " error, allocate fail !!");
                break;
            }

            pTspdDev->total_cmd_pkt     = 0;
            pTspdDev->max_cmd_pkt_array = TSPD_CMD_PKT_INCR_STEP;
            memset(pTspdDev->pCmd_pkt_attr, 0x0, TSPD_CMD_PKT_INCR_STEP*sizeof(TSPD_CMD_PKT_ATTR));
        }
        else
        {
            int     match_idx = -1;

            // has existed
            match_idx = _tspd_binary_search(pTspdDev->pCmd_pkt_attr, pTspdDev->total_cmd_pkt - 1, pCmd_pkt_attr->cmd_pkt_pid);
            if( match_idx >= 0 )     break;
        }

        pTspdDev->pCmd_pkt_attr[pTspdDev->total_cmd_pkt] = *(pCmd_pkt_attr);
        pTspdDev->total_cmd_pkt++;
        TSPD_BIT_CLR(&pTspdDev->pid_mapping[pCmd_pkt_attr->cmd_pkt_pid], (TSPD_BIT_FIELD_SIZE-1));
        TSPD_BIT_SET(&pTspdDev->pid_mapping[pCmd_pkt_attr->cmd_pkt_pid], (TSPD_BIT_FIELD_SIZE-2));

        //-------------------------------------
        // create cmd data buffer
        if( !pTspdDev->pCmd_pkt_buf )
        {
            pTspdDev->cmd_pkt_buf_size = TSPD_CMD_PKT_BUF_SIZE;
            pTspdDev->pCmd_pkt_buf = tspd_malloc(pTspdDev->cmd_pkt_buf_size);
            if( !pTspdDev->pCmd_pkt_buf )
            {
                tspd_msg_ex(TSPD_MSG_ERR, " error, allocate fail !!");
                break;
            }

            memset(pTspdDev->pCmd_pkt_buf, 0x0, pTspdDev->cmd_pkt_buf_size);
            // cache buf operator init
            rb_opt_init(&pTspdDev->cmd_pkt_buf_opr,
                        (uint32_t)pTspdDev->pCmd_pkt_buf,
                        pTspdDev->cmd_pkt_buf_size);
        }

        //-------------------------------------
        // check array size
        if( pTspdDev->total_cmd_pkt == pTspdDev->max_cmd_pkt_array )
        {
            TSPD_CMD_PKT_ATTR       *pNewCmdPktAttr = 0;
            uint32_t                array_size = pTspdDev->max_cmd_pkt_array + TSPD_CMD_PKT_INCR_STEP;

            pNewCmdPktAttr = tspd_malloc(array_size*sizeof(TSPD_CMD_PKT_ATTR));
            if( !pNewCmdPktAttr )
            {
                pTspdDev->total_cmd_pkt--;
                break;
            }
            memset(pNewCmdPktAttr, 0x0, array_size*sizeof(TSPD_CMD_PKT_ATTR));
            memcpy(pNewCmdPktAttr, pTspdDev->pCmd_pkt_attr,
                   pTspdDev->total_cmd_pkt*sizeof(TSPD_CMD_PKT_ATTR));

            free(pTspdDev->pCmd_pkt_attr);

            pTspdDev->pCmd_pkt_attr     = pNewCmdPktAttr;
            pTspdDev->max_cmd_pkt_array = array_size;
        }

        //---------------------------------------------
        // sort
        qsort((void*)(pTspdDev->pCmd_pkt_attr),
              pTspdDev->total_cmd_pkt,
              sizeof(TSPD_CMD_PKT_ATTR),
              _tspd_sort_pid);

    }while(0);

    if( result != TSPD_ERR_OK )
    {
        tspd_msg_ex(TSPD_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }
    // _mutex_unlock(TSE_MSG_TRACE_TSAD, pTseDev->tse_mutex);
    _trace_leave(TSE_MSG_TRACE_TSAD);
    return result;
}


TSE_ERR
tspd_Detach_Cmd_Pkt_Info(
    TSPD_HANLDE         *pHTspd,
    TSPD_CMD_PKT_ATTR   *pCmd_pkt_attr,
    void                *extraData)
{
    TSE_ERR         result = TSPD_ERR_OK;
    TSPD_HANLDE     *pTspdDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSAD, "0x%x, 0x%x, 0x%x\n", pHTspd, pCmd_pkt_attr, extraData);
    _verify_handle(pHTspd, result);
    _verify_handle(pCmd_pkt_attr, result);

    pTspdDev = pHTspd;

    do{
        uint32_t    i = 0;
        int         match_idx = -1;

        if( !pTspdDev->pCmd_pkt_attr || !pTspdDev->total_cmd_pkt )
            break;

        match_idx = _tspd_binary_search(pTspdDev->pCmd_pkt_attr, pTspdDev->total_cmd_pkt - 1, pCmd_pkt_attr->cmd_pkt_pid);
        // no find and leave
        if( match_idx < 0 )     break;

        //-------------------------------------
        // destroy cmd data buffer, if every cmd packet has cmd_data_buf self.
        // To Do:

        memset(&pTspdDev->pCmd_pkt_attr[i], 0x0, sizeof(TSPD_CMD_PKT_ATTR));

        memcpy(&pTspdDev->pCmd_pkt_attr[i],
               &pTspdDev->pCmd_pkt_attr[i+1],
               (pTspdDev->max_cmd_pkt_array - (i+1))*sizeof(TSPD_CMD_PKT_ATTR));

        pTspdDev->total_cmd_pkt--;

        TSPD_BIT_CLR(&pTspdDev->pid_mapping[pCmd_pkt_attr->cmd_pkt_pid], (TSPD_BIT_FIELD_SIZE-2));
        TSPD_BIT_SET(&pTspdDev->pid_mapping[pCmd_pkt_attr->cmd_pkt_pid], (TSPD_BIT_FIELD_SIZE-1));
    }while(0);

    if( result != TSPD_ERR_OK )
    {
        tspd_msg_ex(TSPD_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }
    // _mutex_unlock(TSE_MSG_TRACE_TSAD, pTseDev->tse_mutex);
    _trace_leave(TSE_MSG_TRACE_TSAD);
    return result;
}

// TSE_ERR
// tspd_template(
//     TSPD_HANLDE     *pHTspd,
//     void            *extraData)
// {
//     TSE_ERR         result = TSPD_ERR_OK;
//     TSPD_HANLDE     *pTspdDev = 0;
//
//     _trace_enter(TSE_MSG_TRACE_TSAD, "0x%x, 0x%x\n", pHTspd, extraData);
//     _verify_handle(pHTspd, result);
//
//     pTspdDev = pHTspd;
//
//     do{
//     }while(0);
//
//     if( result != TSPD_ERR_OK )
//     {
//         tspd_msg_ex(TSPD_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
//     }
//     // _mutex_unlock(TSE_MSG_TRACE_TSAD, pTseDev->tse_mutex);
//     _trace_leave(TSE_MSG_TRACE_TSAD);
//     return result;
// }


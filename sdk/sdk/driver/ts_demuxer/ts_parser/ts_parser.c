

#include "ts.h"
#include "ts_parser.h"
#include "ts_demuxer_defs.h"
#include "ite_ts_demuxer.h"

//=============================================================================
//				  Constant Definition
//=============================================================================
#define TS_CHECK_COUNT_THRESHOLD           (32)

typedef enum _TSP_STATUS_TAG
{
    TSP_STATUS_IDLE = 0x11,
    TSP_STATUS_BUSY = 0xBB,
    TSP_STATUS_FAIL = 0xFF, 

}TSP_STATUS;

/**
 * ts parser input bit stream status
 **/
typedef enum _TSP_BS_STATUS_TAG
{
    TSP_BS_STATUS_SEARCH_PACKET     = 0,
    TSP_BS_STATUS_LESS_THAN_188,
    
}TSP_BS_STATUS;


//=============================================================================
//				  Macro Definition
//=============================================================================


//=============================================================================
//				  Structure Definition
//=============================================================================
typedef struct _BASE_BUF_INFO_TAG
{
    uint8_t     *pBufAddr;
    uint32_t    bufSize;
    
}BASE_BUF_INFO;

typedef struct _ITE_TSP_DEV_TAG
{
    TSP_HANDLE      hTsp;

    TSP_STATUS      tspStatus;  // globle current module status

    pthread_mutex_t     tsp_mutex;

    TS_DEMUX        *tsParser;
    bool            bWaitNit;
    bool            bInScan;
    bool            bCollectEit;

    uint32_t        tsFECLength;

    // for bs buffer incomplete TS packet.
    TSP_BS_STATUS   tspBsStatus;
    uint8_t         incompleteTsPacket[TS_PACKET_SIZE];
    uint32_t        collectedByte;

    TS_SRVC_HANDLE  *pHTsSrvc;  // to link ts_service_info database
    TS_EPG_HANDLE   *pHTsEpg;   // to link ts_epg_info database

    // PES info
    //BASE_BUF_INFO   bufInfo_a;  // pes audio valid buffer info
    //BASE_BUF_INFO   bufInfo_v;  // pes video valid buffer info
    //BASE_BUF_INFO   bufInfo_s;  // pes subtitle valid buffer info
    //BASE_BUF_INFO   bufInfo_t;  // pes teletext valid buffer info
    
    // ts file case
    TSD_CTRL_STATUS tsdCtrlStatus;  // control process status
    uint32_t        videoPid;
    uint32_t        audioPid;
    uint32_t        startOffset;
    uint32_t        startTimeStamp;
    bool            bCalPacketSize; // check 188 or 204 or 208

}ITE_TSP_DEV;


//=============================================================================
//				  Global Data Definition
//=============================================================================

//=============================================================================
//				  Private Function Definition
//=============================================================================
static TSD_ERR
_Pid_Statistics_Callback(
    TS_PID_ANAL_DATA    *newData,
    void                *extraData)
{
    TSD_ERR             result = TSP_ERR_OK;
    TS_PID_ANAL_DATA    *pidData = (TS_PID_ANAL_DATA*)newData;
    TSP_PARSER_INFO     *pTspPrsInfo = (TSP_PARSER_INFO*)extraData;

    // service_order_num just is a serial number, if minus service, maybe make a mistake.
    pTspPrsInfo->pPidStatistics[pTspPrsInfo->pesPidCount].fileServiceIndex = pidData->service_order_num;
    pTspPrsInfo->pPidStatistics[pTspPrsInfo->pesPidCount].pid = pidData->pid;
    pTspPrsInfo->pPidStatistics[pTspPrsInfo->pesPidCount].bVideo = pidData->bVideo;
    pTspPrsInfo->pPidStatistics[pTspPrsInfo->pesPidCount++].pidDataCount = 0;

    return result;
}

static void
_Tsp_PesCallBack(
    TS_DEMUX    *ptDemux,
    PES_INFO    *ptPesInfo)
{
    ITE_TSP_DEV     *pTspDev = DOWN_CAST(ITE_TSP_DEV, ptDemux->pHTsp, hTsp);

    _trace_enter("0x%x, 0x%x\n", ptDemux, ptPesInfo);
    
    if( ptDemux && ptDemux->bOnPesOut == false ) 
    {
        tsp_msg_ex(0, " ts parser skip PES data !!");
        _trace_leave();
        return;
    }
    
    _trace_leave();
    return;
}


static TSD_ERR
_Reset_TsParser(
    ITE_TSP_DEV     *pTspDev,
    bool            bCollectEit,
    TSP_TUNNEL_INFO *pTspTunnelInfo)
{
    TSD_ERR         result = TSP_ERR_OK;

    _trace_enter("0x%x, %d, 0x%x\n", pTspDev, bCollectEit, pTspTunnelInfo);

    if( pTspDev )
    {
        do{
            // need to wait ????
            //-------------------------
            // destroy tspPrsInfo
            if( pTspDev->hTsp.pTspPrsInfo )
            {
                free(pTspDev->hTsp.pTspPrsInfo);
                pTspDev->hTsp.pTspPrsInfo = 0;
            }

            //-------------------------
            // terminate psi parser    
            if( pTspDev->tsParser )     
            {
                TS_Terminate(pTspDev->tsParser);
                pTspDev->tsParser = 0;
            }

            //-------------------------
            // initial psi parser
            pTspDev->bWaitNit       = false;
            pTspDev->bInScan        = true;
            pTspDev->bCollectEit    = bCollectEit;        
            pTspDev->tsParser = TS_Init(pTspDev->bWaitNit, pTspDev->bInScan, pTspDev->bCollectEit);
            if( !pTspDev->tsParser )
            {
                tsp_msg_ex(TSP_MSG_TYPE_ERR, " error, allocate fail !!");
                result = TSP_ERR_ALLOCATE_FAIL;
                break;        
            }

            // for psi parser callback
            pTspDev->tsParser->pHTsSrvc = pTspTunnelInfo->srvc_tunnel_info.handle;
            pTspDev->tsParser->pHTsEpg  = pTspTunnelInfo->epg_tunnel_info.handle;
            pTspDev->tsParser->pHTsp    = (void*)&pTspDev->hTsp;

            // for ts parser internal use
            pTspDev->pHTsSrvc = pTspTunnelInfo->srvc_tunnel_info.handle;
            pTspDev->pHTsEpg  = pTspTunnelInfo->epg_tunnel_info.handle;

            TS_SetPatCallBack(pTspDev->tsParser, tsSrvc_PatCallBack);
            TS_SetPmtCallBack(pTspDev->tsParser, tsSrvc_PmtCallBack);
            TS_SetSdtCallBack(pTspDev->tsParser, tsSrvc_SdtCallBack);

            // for pes parser, run-time input should be single audio/video/subtitle/teletext PID
            pTspDev->tsParser->bOnPesOut = pTspTunnelInfo->bOnPesOut;
            if( pTspDev->tsParser->bOnPesOut == true )
            {
                PID_INFO    pidInfo = {0};

                // audio
                pidInfo.pid                  = pTspTunnelInfo->pesOutBuf_a.pid;
                pidInfo.pesOutSampleSize     = pTspTunnelInfo->pesOutBuf_a.pitch;
                pidInfo.validPesSampleCount  = pTspTunnelInfo->pesOutBuf_a.heigth;
                pidInfo.pOutPesBuffer        = pTspTunnelInfo->pesOutBuf_a.pBufAddr;
                pTspDev->tsParser->pidInfo_a = pidInfo;
                TS_InsertEsPid(pTspDev->tsParser, 0, PES_PID_AUDIO, &pTspDev->tsParser->pidInfo_a, 0);
                TS_SetPid(pTspDev->tsParser, pidInfo.pid, true);

                // video
                pidInfo.pid                  = pTspTunnelInfo->pesOutBuf_v.pid;
                pidInfo.pesOutSampleSize     = pTspTunnelInfo->pesOutBuf_v.pitch;
                pidInfo.validPesSampleCount  = pTspTunnelInfo->pesOutBuf_v.heigth;
                pidInfo.pOutPesBuffer        = pTspTunnelInfo->pesOutBuf_v.pBufAddr;
                pTspDev->tsParser->pidInfo_v = pidInfo;
                TS_InsertEsPid(pTspDev->tsParser, 0, PES_PID_VIDEO, &pTspDev->tsParser->pidInfo_v, 0);
                TS_SetPid(pTspDev->tsParser, pidInfo.pid, true);

                // subtitle
                pidInfo.pid                  = pTspTunnelInfo->pesOutBuf_s.pid;
                pidInfo.pesOutSampleSize     = pTspTunnelInfo->pesOutBuf_s.pitch;
                pidInfo.validPesSampleCount  = pTspTunnelInfo->pesOutBuf_s.heigth;
                pidInfo.pOutPesBuffer        = pTspTunnelInfo->pesOutBuf_s.pBufAddr;
                pTspDev->tsParser->pidInfo_s = pidInfo;
                TS_InsertEsPid(pTspDev->tsParser, 0, PES_PID_SUBTITLE, &pTspDev->tsParser->pidInfo_s, 0);
                TS_SetPid(pTspDev->tsParser, pidInfo.pid, false);

                // teletext
                pidInfo.pid                  = pTspTunnelInfo->pesOutBuf_t.pid;
                pidInfo.pesOutSampleSize     = pTspTunnelInfo->pesOutBuf_t.pitch;
                pidInfo.validPesSampleCount  = pTspTunnelInfo->pesOutBuf_t.heigth;
                pidInfo.pOutPesBuffer        = pTspTunnelInfo->pesOutBuf_t.pBufAddr;
                pTspDev->tsParser->pidInfo_t = pidInfo;
                TS_InsertEsPid(pTspDev->tsParser, 0, PES_PID_TELETEXT, &pTspDev->tsParser->pidInfo_t, 0);
                TS_SetPid(pTspDev->tsParser, pidInfo.pid, false);

                //TS_SetPesCallBack(pTspDev->tsParser, _Tsp_PesCallBack);
            }
            
            pTspDev->tsParser->bReceiveSdt = 1;  // ???????

            //------------------
            /*// Set the valid transport_stream_id to TS demux system
            // ps. this case need to parse NIT and verify the same ts or not.
            TS_SetValidTransportStream(pTspDev->tsParser,
                                       tChannel.transport_stream_id,
                                       tChannel.original_network_id); 
            */
            //------------------
            
            //-------------------------
            // create tspPrsInfo
            pTspDev->hTsp.pTspPrsInfo = tsd_malloc(sizeof(TSP_PARSER_INFO));
            if( !pTspDev->hTsp.pTspPrsInfo )
            {
                tsp_msg_ex(TSP_MSG_TYPE_ERR, " error, allocate fail !!");
                result = TSP_ERR_ALLOCATE_FAIL;
                break;
            }
            
            memset(pTspDev->hTsp.pTspPrsInfo, 0, sizeof(TSP_PARSER_INFO));              
        }while(0);
    }
    
    if( result != TSP_ERR_OK )
    {
        pTspDev->tspStatus = TSP_STATUS_FAIL;
        tsp_msg_ex(TSP_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _trace_leave();
    return result;
}

//=============================================================================
//				  Public Function Definition
//=============================================================================
TSD_ERR
tsp_CreateHandle(
    TSP_HANDLE          **pHTsp,
    bool                bCollectEit,
    TSP_TUNNEL_INFO     *pTspTunnelInfo,
    void                *extraData)
{
    TSD_ERR         result = TSP_ERR_OK;
    ITE_TSP_DEV     *pTspDev = 0;

    _trace_enter("0x%x, %d, 0x%x, 0x%x\n", pHTsp, bCollectEit, pTspTunnelInfo, extraData);
    
    do{
        if( *pHTsp != 0 )
        {
            tsp_msg_ex(TSP_MSG_TYPE_ERR, " error, Exist tsp handle !!");
            result = TSP_ERR_INVALID_PARAMETER;
            break;
        }

        // ------------------------
        // craete dev info
        pTspDev = tsd_malloc(sizeof(ITE_TSP_DEV));
        if( !pTspDev )
        {
            tsp_msg_ex(TSP_MSG_TYPE_ERR, " error, allocate fail !!");
            result = TSP_ERR_ALLOCATE_FAIL;
            break;
        }

        memset((void*)pTspDev, 0x0, sizeof(ITE_TSP_DEV));
        pTspDev->bCalPacketSize = true;
      
        pTspDev->tspStatus = TSP_STATUS_IDLE;

        result = _Reset_TsParser(pTspDev, bCollectEit, pTspTunnelInfo);
        if( result != TSP_ERR_OK )
        {
            tsp_msg_ex(TSP_MSG_TYPE_ERR, " error, init fail !!");
            break;
        }

        // create mutex
        _mutex_init(TSD_MSG_TYPE_TRACE_PARSER, pTspDev->tsp_mutex);
        
        // if not error
        (*pHTsp) = &pTspDev->hTsp;
        
    }while(0);
    
    if( result != TSP_ERR_OK )
    {
        pTspDev->tspStatus = TSP_STATUS_FAIL;
        tsp_msg_ex(TSP_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    } 

    _trace_leave();
    return result;    
}

TSD_ERR
tsp_DestroyHandle(
    TSP_HANDLE  **pHTsp)
{
    TSD_ERR         result = TSP_ERR_OK;
    ITE_TSP_DEV     *pTspDev = DOWN_CAST(ITE_TSP_DEV, (*pHTsp), hTsp);
    pthread_mutex_t tsp_mutex = 0;

    _trace_enter("0x%x\n", pHTsp);

    _verify_handle((*pHTsp), result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_PARSER, pTspDev->tsp_mutex);

    /**
     * avoid broke destroy process in pre-empty multi-task environment
     * (released_handle be used on other function) 
     */
    _disable_irq(); 

    if( pTspDev )
    {
        // reset channel info data to service_info_database
        tsSrvc_Control(pTspDev->pHTsSrvc, TS_SRVC_CTL_SET_CHANNEL_INFO, 0, 0);
            
        //-------------------------
        // destroy tspPrsInfo
        if( pTspDev->hTsp.pTspPrsInfo )
        {
            free(pTspDev->hTsp.pTspPrsInfo);
            pTspDev->hTsp.pTspPrsInfo = 0;
        }
    
        //-------------------------
        // terminate psi parser    
        if( pTspDev->tsParser )     
        {
            TS_Terminate(pTspDev->tsParser);
            pTspDev->tsParser = 0;
        }
        
        *pHTsp = 0;

        tsp_mutex = pTspDev->tsp_mutex;

        free(pTspDev);
    }
    
    _mutex_unlock(TSD_MSG_TYPE_TRACE_PARSER, tsp_mutex);
    // de-init mutex
    _mutex_deinit(TSD_MSG_TYPE_TRACE_PARSER, tsp_mutex);

    _enable_irq();
    _trace_leave();
    return result;
}


TSD_ERR
tsp_ParseStream( 
    TSP_HANDLE      *pHTsp,
    void            *extraData)
{
    TSD_ERR         result = TSP_ERR_OK;
    ITE_TSP_DEV     *pTspDev = DOWN_CAST(ITE_TSP_DEV, pHTsp, hTsp);
    int32_t         remainSize = (int32_t)pTspDev->hTsp.dataSize;
    uint8_t         *pData = 0;

    _trace_enter("0x%x, 0x%x\n", pHTsp, extraData);

    _verify_handle(pHTsp, result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_PARSER, pTspDev->tsp_mutex);

    pData = pTspDev->hTsp.dataBuf;

    if( pTspDev && pTspDev->tspStatus != TSP_STATUS_FAIL )
    {
        if( remainSize )
        {
#if 0
            // ts file case +
            // check packet length 188, 204, or 208
            if( 0 && TSD_CTRL_WAIT_VIDEO_AUDIO == pTspDev->tsdCtrlStatus && 
                pTspDev->bCalPacketSize )
            {
                uint32_t    packet188Cnt = 0, packet204Cnt = 0, packet208Cnt = 0;
                uint32_t    maxCount = TS_CHECK_COUNT_THRESHOLD;
                int32_t     checkLength = remainSize;
                uint8_t     *pCheckData = pData;
                int32_t     jumpLength = 0;
                uint32_t    i = 0;
                
                while( checkLength > 0 )
                {
                    if( *pCheckData == VALID_SYNC_BYTE )
                    {
                        if( *(pCheckData + 188) == VALID_SYNC_BYTE )        { packet188Cnt++; jumpLength = 188; }
                        else if( *(pCheckData + 204) == VALID_SYNC_BYTE )   { packet204Cnt++; jumpLength = 204; }
                        else if( *(pCheckData + 208) == VALID_SYNC_BYTE )   { packet208Cnt++; jumpLength = 208; }
                        else                                                jumpLength = 1;

                        pCheckData += jumpLength;
                        checkLength -= jumpLength;
                    }
                    else
                    {
                        pCheckData++;
                        checkLength--;
                    }
                }

                pTspDev->tsFECLength = 0xFFFFFFFF;
                
                if( packet188Cnt >= maxCount )  { pTspDev->tsFECLength = 0; maxCount = packet188Cnt; }
                if( packet204Cnt >= maxCount )  { pTspDev->tsFECLength = 16; maxCount = packet204Cnt; }
                if( packet208Cnt >= maxCount )  pTspDev->tsFECLength = 20;

                if (pTspDev->tsFECLength == 0xFFFFFFFF)
                    pTspDev->tsFECLength = 0;
                    
                pTspDev->bCalPacketSize = false;
            }
            
            if( 0 && pTspDev->startOffset )
            {
                pData += pTspDev->startOffset;
                remainSize -= pTspDev->startOffset;
                pTspDev->startOffset = 0;
            }
            // ts file case -
#endif

            // decode psi packet and handle bs buf status (less than packet size)
            while( remainSize > 0 )
            {
                switch( pTspDev->tspBsStatus )
                {
                    case TSP_BS_STATUS_SEARCH_PACKET:
                        // chack packet sync byte
                        if( *pData == VALID_SYNC_BYTE )
                        {
                            if( remainSize < TS_PACKET_SIZE )
                                pTspDev->tspBsStatus = TSP_BS_STATUS_LESS_THAN_188;
                            else
                            {
                            #if 0
                                // start parsing packet
                                // ts file case +
                                if( TSD_CTRL_WAIT_VIDEO_AUDIO == pTspDev->tsdCtrlStatus )
                                {
                                    uint32_t          pid = ((pData[1] & 0x1f) << 8) | pData[2];
                                    uint32_t          i = 0;
                                    TSP_PARSER_INFO   *pTspPrsInfo = pTspDev->hTsp.pTspPrsInfo;
                                    
                                    for(i = 0; i < pTspPrsInfo->pesPidCount; i++)
                                    {
                                        if( pid == pTspPrsInfo->pPidStatistics[i].pid )
                                        {
                                            pTspPrsInfo->pPidStatistics[i].pidDataCount++;
                                            break;
                                        }
                                    }
                                }
                                // ts file case -
                                
                                // send channel info to service_info_database
                                tsSrvc_Control(pTspDev->pHTsSrvc, TS_SRVC_CTL_SET_CHANNEL_INFO, 0, &pTspDev->hTsp.tsChnlInfo);
                                
                                // start deocde psi
                                TS_Decode(pTspDev->tsParser, pData);
                                pData += (TS_PACKET_SIZE + pTspDev->tsFECLength);

                                // ts file case +
                                if( remainSize < (int32_t)(TS_PACKET_SIZE + pTspDev->tsFECLength) )
                                    pTspDev->startOffset = TS_PACKET_SIZE + pTspDev->tsFECLength - remainSize;
                                // ts file case -

                                remainSize -= (TS_PACKET_SIZE + pTspDev->tsFECLength);
                            #else
                                // send channel info to service_info_database
                                tsSrvc_Control(pTspDev->pHTsSrvc, TS_SRVC_CTL_SET_CHANNEL_INFO, 0, &pTspDev->hTsp.tsChnlInfo);
                                
                                // start deocde psi
                                TS_Decode(pTspDev->tsParser, pData);
                                pData += (TS_PACKET_SIZE + pTspDev->tsFECLength);

                                remainSize -= (TS_PACKET_SIZE + pTspDev->tsFECLength);
                            #endif
                            }
                        }
                        else
                        {
                            pData++;
                            remainSize--;
                        }                        
                        break;
                        
                    case TSP_BS_STATUS_LESS_THAN_188:
                        if( pTspDev->collectedByte > 0 &&
                            remainSize >= (int32_t)(TS_PACKET_SIZE - pTspDev->collectedByte) )
                        {
                            memcpy(&pTspDev->incompleteTsPacket[pTspDev->collectedByte],
                                pData, TS_PACKET_SIZE - pTspDev->collectedByte);
                                
                            TS_Decode(pTspDev->tsParser, pTspDev->incompleteTsPacket);
                            pData += (TS_PACKET_SIZE + pTspDev->tsFECLength - pTspDev->collectedByte);

                        #if 0
                            // ts file case +
                            if( remainSize < (int32_t)(TS_PACKET_SIZE + pTspDev->tsFECLength - pTspDev->collectedByte) )
                                pTspDev->startOffset = (TS_PACKET_SIZE + pTspDev->tsFECLength - pTspDev->collectedByte) - remainSize;
                            // ts file case -
                        #endif
                        
                            remainSize -= (TS_PACKET_SIZE + pTspDev->tsFECLength - pTspDev->collectedByte);

                            pTspDev->collectedByte = 0;
                            pTspDev->tspBsStatus = TSP_BS_STATUS_SEARCH_PACKET;
                        }
                        else
                        {
                            memcpy(&pTspDev->incompleteTsPacket[pTspDev->collectedByte],
                                pData, remainSize);
                                
                            pTspDev->collectedByte += remainSize;
                            remainSize = 0;
                        }                    
                        break;
                }
            }

            #if 0
            // ts file case +
            if( TSD_CTRL_WAIT_FIRST_TIMESTAMP == pTspDev->tsdCtrlStatus &&
                pTspDev->startTimeStamp )
            {
                // get start time
                pTspDev->tsdCtrlStatus = TSD_CTRL_WAIT_LAST_TIMESTAMP;
                tsSrvc_Set_TsdCtrlStatus(pTspDev->tsdCtrlStatus);
            }
            // ts file case -
            #endif
        }
        else
        {
        #if 0
            // ts file case +
            switch( pTspDev->tsdCtrlStatus )
            {
                case TSD_CTRL_WAIT_VIDEO_AUDIO:
                    {
                        uint32_t          i = 0;
                        uint32_t          maxDataCnt = 0;
                        uint32_t          videoPid = 0;
                        uint32_t          audioPid = 0;
                        uint32_t          fileServiceIdx = (uint32_t)-1;
                        uint32_t          videoPidCnt = 0;
                        uint32_t          thresholdDataCnt = 0;                
                        TSP_PARSER_INFO   *pTspPrsInfo = pTspDev->hTsp.pTspPrsInfo;

                        // check pTspPrsInfo NULL pointer or not
                        if( !pTspPrsInfo )  break;

                        for (i = 0; i < pTspPrsInfo->pesPidCount; i++)
                        {
                            if( pTspPrsInfo->pPidStatistics[i].pidDataCount
                             && pTspPrsInfo->pPidStatistics[i].bVideo )
                            {
                                videoPidCnt++;
                                thresholdDataCnt += pTspPrsInfo->pPidStatistics[i].pidDataCount;
                            }
                        }

                        // what's this ??
                        if( videoPidCnt )   thresholdDataCnt = (thresholdDataCnt / videoPidCnt) * 12 / 10;
                        
                        for (i = 0; i < pTspPrsInfo->pesPidCount; i++)
                        {
                            if( pTspPrsInfo->pPidStatistics[i].pidDataCount > maxDataCnt
                             && pTspPrsInfo->pPidStatistics[i].pidDataCount < thresholdDataCnt )
                            {
                                videoPid = pTspPrsInfo->pPidStatistics[i].pid;
                                maxDataCnt = pTspPrsInfo->pPidStatistics[i].pidDataCount;
                                fileServiceIdx = pTspPrsInfo->pPidStatistics[i].fileServiceIndex;
                            }
                        }
                        
                        if( videoPid )
                        {
                            maxDataCnt = 0;
                            pTspDev->videoPid = videoPid;
                            pTspPrsInfo->activeServiceIndex = fileServiceIdx;
                            for (i = 0; i < pTspPrsInfo->pesPidCount; i++)
                            {
                                if( pTspPrsInfo->pPidStatistics[i].pidDataCount
                                 && false == pTspPrsInfo->pPidStatistics[i].bVideo
                                 && fileServiceIdx == pTspPrsInfo->pPidStatistics[i].fileServiceIndex )
                                {
                                    if( pTspPrsInfo->pPidStatistics[i].pidDataCount > maxDataCnt )
                                    {
                                        audioPid = pTspPrsInfo->pPidStatistics[i].pid;
                                        maxDataCnt = pTspPrsInfo->pPidStatistics[i].pidDataCount;
                                    }
                                }
                            }
                            
                            if( audioPid )      pTspDev->audioPid = audioPid;
                        }

                        if( pTspDev->videoPid )
                        {
                            pTspDev->tsdCtrlStatus = TSD_CTRL_WAIT_FIRST_TIMESTAMP;
                            tsSrvc_Set_TsdCtrlStatus(pTspDev->tsdCtrlStatus);
                        }
                        else
                        {
                            // can't fine video PID, unknow bit stream.
                            free(pTspDev->hTsp.pTspPrsInfo);
                            pTspDev->hTsp.pTspPrsInfo = 0;
                        }                    
                    }
                    break;
                    
                case TSD_CTRL_WAIT_FIRST_TIMESTAMP:
                    // handle timestamp ??
                    pTspDev->tsdCtrlStatus = TSD_CTRL_WAIT_LAST_TIMESTAMP;
                    
                    // sync status with service database
                    tsSrvc_Set_TsdCtrlStatus(pTspDev->tsdCtrlStatus);
                    break;
                    
                case TSD_CTRL_WAIT_LAST_TIMESTAMP:
                    // handle timestamp ??
                    pTspDev->tsdCtrlStatus = TSD_CTRL_NORMAL_MODE;
                    
                    // sync status with service database
                    tsSrvc_Set_TsdCtrlStatus(pTspDev->tsdCtrlStatus);
                    break;
                default:
                    tsp_msg_ex(TSP_MSG_TYPE_ERR, "unknow status (%d) !!", pTspDev->tsdCtrlStatus);
                    break;
            }
            // ts file case -
            #endif
        }
    }
    
    if( result != TSP_ERR_OK )
    {
        pTspDev->tspStatus = TSP_STATUS_FAIL;
        tsp_msg_ex(TSP_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSD_MSG_TYPE_TRACE_PARSER, pTspDev->tsp_mutex);
    _trace_leave();
    return result;
}

TSD_ERR
tsp_GetNextSample(
    TSP_HANDLE          *pHTsp,
    TSP_PES_SAMPLE_INFO *pPesSample,
    void                *extraData)
{
    TSD_ERR         result = TSP_ERR_OK;
    ITE_TSP_DEV     *pTspDev = DOWN_CAST(ITE_TSP_DEV, pHTsp, hTsp);

    _trace_enter("0x%x, 0x%x, 0x%x\n", pHTsp, pPesSample, extraData);

    _verify_handle(pHTsp, result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_PARSER, pTspDev->tsp_mutex);

    if( pTspDev && pTspDev->tspStatus != TSP_STATUS_FAIL )
    {
        uint8_t         *pSampleAddr = 0;
        uint32_t        sampleSize = 0;
        
        // need to do get_ready and set_free
        switch( pPesSample->sampleType )
        {
            case TSD_SAMPLE_VIDEO:
                pPesSample->pSampleAddr = 0;
                pPesSample->sampleSize  = 0;
                if( !pesQm_GetReady(0, PES_PID_VIDEO, (void**)&pSampleAddr, &sampleSize) )
                {
                    pPesSample->pSampleAddr = pSampleAddr;
                    pPesSample->sampleSize  = sampleSize;
                    pesQm_SetFree(0, PES_PID_VIDEO, (void**)&pSampleAddr);
                }
                break;
        
            case TSD_SAMPLE_AUDIO:
                pPesSample->pSampleAddr = 0;
                pPesSample->sampleSize  = 0;
                if( !pesQm_GetReady(0, PES_PID_AUDIO, (void**)&pSampleAddr, &sampleSize) )
                {
                    pPesSample->pSampleAddr = pSampleAddr;
                    pPesSample->sampleSize  = sampleSize;
                    pesQm_SetFree(0, PES_PID_AUDIO, (void**)&pSampleAddr);
                }        
                break;
                
            case TSD_SAMPLE_SUBTITLE:
                pPesSample->pSampleAddr = 0;
                pPesSample->sampleSize  = 0;
                if( !pesQm_GetReady(0, PES_PID_SUBTITLE, (void**)&pSampleAddr, &sampleSize) )
                {
                    pPesSample->pSampleAddr = pSampleAddr;
                    pPesSample->sampleSize  = sampleSize;
                    pesQm_SetFree(0, PES_PID_SUBTITLE, (void**)&pSampleAddr);
                }           
                break;
                
            case TSD_SAMPLE_TELETEXT:
                pPesSample->pSampleAddr = 0;
                pPesSample->sampleSize  = 0;
                if( !pesQm_GetReady(0, PES_PID_TELETEXT, (void**)&pSampleAddr, &sampleSize) )
                {
                    pPesSample->pSampleAddr = pSampleAddr;
                    pPesSample->sampleSize  = sampleSize;
                    pesQm_SetFree(0, PES_PID_TELETEXT, (void**)&pSampleAddr);
                }
                break;            
        }
    }

    if( result != TSP_ERR_OK )
    {
        pTspDev->tspStatus = TSP_STATUS_FAIL;
        tsp_msg_ex(TSP_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }
    
    _mutex_unlock(TSD_MSG_TYPE_TRACE_PARSER, pTspDev->tsp_mutex);
    _trace_leave();
    return result;
}


TSD_ERR
tsp_Control(
    TSP_HANDLE      *pHTsp,
    TSP_CTRL_CMD    cmd,
    uint32_t        *value,
    void            *extraData)
{
    TSD_ERR         result = TSP_ERR_OK;
    ITE_TSP_DEV     *pTspDev = DOWN_CAST(ITE_TSP_DEV, pHTsp, hTsp);

    _trace_enter("0x%x, %d, 0x%x, 0x%x\n", pHTsp, cmd, value, extraData);

    _verify_handle(pHTsp, result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_PARSER, pTspDev->tsp_mutex);

    if( pTspDev && pTspDev->tspStatus != TSP_STATUS_FAIL )
    {
        switch( cmd )
        {
            case TSP_CTRL_RESET:
                result = _Reset_TsParser(pTspDev, ((value)? true : false), (TSP_TUNNEL_INFO*)extraData);
                if( result != TSP_ERR_OK )
                    tsp_msg_ex(TSP_MSG_TYPE_ERR, " error, init fail !!");
                break;

            case TSP_CTRL_SET_TSD_CTL_STATUS:
                pTspDev->tsdCtrlStatus = (TSD_CTRL_STATUS)value;
                // sync status with service database
                tsSrvc_Control(pTspDev->pHTsSrvc,
                               TS_SRVC_CTL_SET_CTL_STATUS,
                               (uint32_t*)pTspDev->tsdCtrlStatus, 0);
                break;

            case TSP_CTRL_ATTACH_PID_STATISTICS_CB:
                // we need this ????????
                {
                    TS_SRVC_PID_STAT_CB_INFO    cbInfo = {0};

                    cbInfo.pStatInfo = (void*)pTspDev->hTsp.pTspPrsInfo; 
                    cbInfo.pfStat_CB = _Pid_Statistics_Callback;
                    
                    // set callbace for statistics PID
                    tsSrvc_Control(pTspDev->pHTsSrvc, 
                        TS_SRVC_CTL_SET_PID_STAT_CB_INFO, 
                        0, &cbInfo);
                }
                break;

            case TSP_CTRL_ATTACH_EPG_HANDLE:
                pTspDev->pHTsEpg = (TS_EPG_HANDLE*)extraData;
                break;
                
            case TSP_CTRL_ATTACH_EIT_CB:
                if( pTspDev->tsParser )
                {
                    TS_SetEitPfCallBack(pTspDev->tsParser, tsEpg_Eit_P_F_CallBack);
                    TS_SetEitSchCallBack(pTspDev->tsParser, tsEpg_Eit_Schedule_CallBack);
                    TS_SetEitSectionFilterCallBack(pTspDev->tsParser, tsEpg_Eit_SectionFilter_CallBack);
                    
                    TS_SetTdtCallBack(pTspDev->tsParser, tsEpg_Tdt_CallBack);
                    TS_SetTotCallBack(pTspDev->tsParser, tsEpg_Tot_CallBack);
                }
                break;

            case TSP_CTRL_CAL_PACKET_SIZE:
                pTspDev->bCalPacketSize = (bool)value;
                break;
                
            case TSP_CTRL_REGEN_EIT_HANDLE:
                if( pTspDev->tsParser )
                    TS_ReCreateEitHandle(pTspDev->tsParser);
                break;
                
            case TSP_CTRL_ENABLE_PID:
                TS_EnablePid(pTspDev->tsParser, (uint32_t)value);
                break;
                
            case TSP_CTRL_DISABLE_PID:
                TS_DisablePid(pTspDev->tsParser, (uint32_t)value);
                break;

            case TSP_CTRL_SET_CHNL_INFO:
                if( extraData )  
                {
                    TS_CHNL_INFO    *pTsChnlInfo = (TS_CHNL_INFO*)extraData;
                    memcpy(&pTspDev->hTsp.tsChnlInfo, pTsChnlInfo, sizeof(TS_CHNL_INFO));
                }
                break;
                
            default:
                result = TSP_ERR_NO_IMPLEMENT;
                break;
        }
    }

    if( result != TSP_ERR_OK && 
        result != TSP_ERR_NO_IMPLEMENT )
    {
        pTspDev->tspStatus = TSP_STATUS_FAIL;
        tsp_msg_ex(TSP_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }
    
    _mutex_unlock(TSD_MSG_TYPE_TRACE_PARSER, pTspDev->tsp_mutex);
    _trace_leave();
    return result;
}


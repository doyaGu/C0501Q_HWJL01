

#include "jpg_defs.h"
#include "jpg_codec.h"
#include "jpg_hw.h"
#include "jpg_common.h"
#include "jpg_extern_link.h"

#if (CONFIG_JPG_CODEC_DESC_DEC_JPG_CMD_DESC)
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct JPG_DEOCDER_TAG
{
    JPG_TRIGGER_MODE    triggerMode;

//    uint8_t             *pSysBsStart;
//    int                 vramBsSize;
//    uint8_t             *vramBsAddr;
//    uint8_t             *vramBsCurr;
//    uint32_t            vramRwSize;

}JPG_DEOCDER;

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
static JPG_ERR
_JPG_Cmd_HW_Update(
    JPG_CODEC_HANDLE    *pHJCodec)
{
    JPG_ERR             result = JPG_ERR_OK;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_DEC_JPG_CMD, "0x%x", pHJCodec);

    do{
        JPG_FRM_COMP        *pJFrmComp = &pHJCodec->jFrmCompInfo;
        JPG_HW_CTRL         *pJHwCtrl = &pHJCodec->jHwCtrl;
        JPG_LINE_BUF_INFO   *pJLineBufInfo = &pHJCodec->jLineBufInfo;
        JPG_HW_BS_CTRL      *pJHwBsCtrl = &pHJCodec->jHwBsCtrl;
        JPG_FRM_SIZE_INFO   *pJFrmSizeInfo = &pHJCodec->jFrmSizeInfo;

        JPG_SetCodecCtrlReg(pJHwCtrl->codecCtrl);

        JPG_SetDriReg(pJFrmComp->restartInterval);

        JPG_SetTableSpecifyReg(pJFrmComp);

        JPG_SetFrmSizeInfoReg(pJFrmSizeInfo);

        JPG_SetLineBufInfoReg(pJLineBufInfo);

        JPG_SetLineBufSliceUnitReg(pJLineBufInfo->sliceNum, pJFrmComp->jFrmInfo[0].verticalSamp);

        // win32
        _jpg_reflash_vram(pHJCodec->pHInJStream,
                          JPG_STREAM_CMD_GET_VRAM_BS_BUF_A,
                          0, 0, &pJHwBsCtrl->addr);

        JPG_SetBitStreamBufInfoReg(pJHwBsCtrl);
        JPG_SetBitstreamReadBytePosReg(pJHwBsCtrl->shiftByteNum);

        JPG_SetSamplingFactorReg(pJFrmComp);

        // set Q table
        //JPG_SetQtableReg(pJFrmComp);

        // set huffman table
        JPG_SetHuffmanCodeCtrlReg(JPG_HUUFFMAN_Y_DC , pJHwCtrl->dcHuffTable[0]);
        JPG_SetHuffmanCodeCtrlReg(JPG_HUUFFMAN_UV_DC, pJHwCtrl->dcHuffTable[1]);
        JPG_SetHuffmanCodeCtrlReg(JPG_HUUFFMAN_Y_AC , pJHwCtrl->acHuffTable[0]);
        JPG_SetHuffmanCodeCtrlReg(JPG_HUUFFMAN_UV_AC, pJHwCtrl->acHuffTable[1]);

        JPG_SetDcHuffmanValueReg(JPG_HUUFFMAN_Y_DC,
                                 (pJHwCtrl->dcHuffTable[0] + 16),
                                  pJHwCtrl->dcHuffW2talCodeLenCnt[0]);

        JPG_SetDcHuffmanValueReg(JPG_HUUFFMAN_UV_DC,
                                 (pJHwCtrl->dcHuffTable[1] + 16),
                                 pJHwCtrl->dcHuffW2talCodeLenCnt[1]);

        JPG_SetDecodeAcHuffmanValueReg(JPG_HUUFFMAN_Y_AC,
                                       (pJHwCtrl->acHuffTable[0] + 16),
                                       pJHwCtrl->acHuffW2talCodeLenCnt[0]);

        JPG_SetDecodeAcHuffmanValueReg(JPG_HUUFFMAN_UV_AC,
                                       (pJHwCtrl->acHuffTable[1] + 16),
                                       pJHwCtrl->acHuffW2talCodeLenCnt[1]);

        JPG_DropHv((JPG_REG)(pHJCodec->ctrlFlag >> 16));

    }while(0);


    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_DEC_JPG_CMD);
    return result;
}

static JPG_ERR
_JPG_Cmd_HW_Wait_Idle(
    uint32_t      timeOutCnt)
{
    JPG_ERR     result = JPG_ERR_OK;
    JPG_REG     bufStatus = 0, hwStatus = 0;
    uint32_t    timeOut = 0;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_DEC, "%d\n", timeOutCnt);

    bufStatus = JPG_GetProcStatusReg();
    while( !(bufStatus & JPG_STATUS_DECODE_COMPLETE) )
    {
        if( bufStatus & JPG_STATUS_DECODE_ERROR )
        {
            result = JPG_ERR_DECODE_IRQ;
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg HW err !! ");
            break;
        }

        hwStatus = JPG_GetEngineStatusReg();
        if( (hwStatus&0xF800) == 0x3000 )
        {
            jpg_sleep(1); // 2
            hwStatus = JPG_GetEngineStatusReg();
            if( (hwStatus&0xF800) == 0x3000 )
            {
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg HW err !! ");
                result = JPG_ERR_BUSY_TIMEOUT;
                break;
            }
        }

        jpg_sleep(1);

        timeOut++;
        if( timeOut > timeOutCnt )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg HW err (bufStatus=0x%x, hwStatus= 0x%x)!! ", bufStatus, hwStatus);
            result = JPG_ERR_BUSY_TIMEOUT;
            break;
        }

        bufStatus = JPG_GetProcStatusReg();
    }

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_DEC);
    return result;
}


static JPG_ERR
jpg_dec_cmd_init(
    JPG_CODEC_HANDLE    *pHJCodec,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_DEC_JPG_CMD, "0x%x, 0x%x\n", pHJCodec, extraData);

    do{
        JPG_STREAM_DESC     *pJStreamDesc = &pHJCodec->pHInJStream->jStreamDesc;

        JPG_PowerUp();

        pHJCodec->ctrlFlag |= JPG_FLAGS_MJPG;
        pHJCodec->ctrlFlag |= JPG_FLAGS_MJPG_FIRST_FRAME;

        if( pJStreamDesc->jHeap_mem )
        {
            // allocate private data
            pHJCodec->privData = pJStreamDesc->jHeap_mem(
                                            pHJCodec->pHInJStream, JPG_HEAP_DEF,
                                            sizeof(JPG_DEOCDER), 0);
            if(pHJCodec->privData)
                memset(pHJCodec->privData, 0x0, sizeof(JPG_DEOCDER));
        }
    }while(0);

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_DEC_JPG_CMD);
    return result;
}

static JPG_ERR
jpg_dec_cmd_deInit(
    JPG_CODEC_HANDLE    *pHJCodec,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;
    JPG_STREAM_DESC     *pJStreamDesc = &pHJCodec->pHInJStream->jStreamDesc;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_DEC_JPG_CMD, "0x%x, 0x%x\n", pHJCodec, extraData);

    JPG_DecPowerDown();

    if( pJStreamDesc->jFree_mem )
    {
        // free private data
        if( pHJCodec->privData )
        {
            pJStreamDesc->jFree_mem(pHJCodec->pHInJStream, JPG_HEAP_DEF, pHJCodec->privData);
            pHJCodec->privData = 0;
        }
    }

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_DEC_JPG_CMD);
    return result;
}


static JPG_ERR
jpg_dec_cmd_setup(
    JPG_CODEC_HANDLE    *pHJCodec,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;


    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_DEC_JPG_CMD, "0x%x, 0x%x\n", pHJCodec, extraData);

    do{
        JCOMM_HANDLE        *pHJComm = (JCOMM_HANDLE*)pHJCodec->pHJComm;
        JPG_FRM_COMP        *pJFrmComp = &pHJCodec->jFrmCompInfo;
        JPG_HW_CTRL         *pJHwCtrl = &pHJCodec->jHwCtrl;
        JPG_LINE_BUF_INFO   *pJLineBufInfo = &pHJCodec->jLineBufInfo;
        JPG_HW_BS_CTRL      *pJHwBsCtrl = &pHJCodec->jHwBsCtrl;
        JPG_STREAM_HANDLE   *pHOutJStream = pHJCodec->pHOutJStream;
        JPG_DEOCDER         *pJDecoder = (JPG_DEOCDER*)pHJCodec->privData;

        // set JPG_COMMAND_TRIGGER mode
        pJDecoder->triggerMode = JPG_COMMAND_TRIGGER;

        // ---------------------------
        // downsample or not
        if( pHJCodec->ctrlFlag & JPG_FLAGS_OUTPUT_RGB565 )
        {
            // set line buffer
            pJLineBufInfo->comp1Addr  = pHJComm->jOutBufInfo[0].pBufAddr;
            pJLineBufInfo->comp1Pitch = pHJComm->jOutBufInfo[0].pitch;
        }

        // ??
        pJLineBufInfo->sliceNum   = ((pJFrmComp->realHeight + 0x7) >> 3);
        //pJLineBufInfo->sliceNum   = (pJFrmComp->realHeight >> 3);

        if( pHJCodec->ctrlFlag & JPG_FLAGS_MJPG_FIRST_FRAME )
        {
            pHJCodec->ctrlFlag &= ~JPG_FLAGS_MJPG_FIRST_FRAME;

            // --------------------------------------------
            // HW control setting
            if( pHJCodec->ctrlFlag & JPG_FLAGS_OUTPUT_RGB565 )
            {
            #if 0
                // MM9070 kill this feature
                if( pJFrmComp->decColorSpace == JPG_COLOR_SPACE_YUV422 )
                {
                    pHJCodec->yuv2RgbMatrix = &yuv2RgbMatrix;
                    pJHwCtrl->codecCtrl = (JPG_OP_DECODE_RGB565 | pJDecoder->triggerMode);
                }
                else
                {
                    jpg_msg(JEG_MSG_TYPE_ERR, "Jpg just support YUV422 to RGB565. ");
                    result = JPG_ERR_INVALID_PARAMETER;
                    break;
                }
            #else
                jpg_msg(JPG_MSG_TYPE_ERR, "9070 Jpg not support YUV to RGB565. ");
                result = JPG_ERR_INVALID_PARAMETER;
                break;
            #endif
            }
            else
            {
                pJHwCtrl->codecCtrl = (JPG_OP_DECODE | pJDecoder->triggerMode);
            }

            pJHwCtrl->codecCtrl |= (pJFrmComp->validComp & JPG_MSK_DEC_COMPONENT_VALID);
            pJHwCtrl->codecCtrl |= (jpgCompCtrl[pJFrmComp->compNum] & JPG_MSK_LINE_BUF_COMPONENT_VALID);
#if 0
            // --------------------------------------------
            // set H-table
                // Y: huffman table
            pJHwCtrl->dcHuffTable[0] = pJFrmComp->huff_DC[0].pHuffmanTable;
            pJHwCtrl->acHuffTable[0] = pJFrmComp->huff_AC[0].pHuffmanTable;
            pJHwCtrl->dcHuffW2talCodeLenCnt[0] = pJFrmComp->huff_DC[0].totalCodeLenCnt;
            pJHwCtrl->acHuffW2talCodeLenCnt[0] = pJFrmComp->huff_AC[0].totalCodeLenCnt;
                // UV: huffman table
            pJHwCtrl->dcHuffTable[1] = pJFrmComp->huff_DC[1].pHuffmanTable;
            pJHwCtrl->acHuffTable[1] = pJFrmComp->huff_AC[1].pHuffmanTable;
            pJHwCtrl->dcHuffW2talCodeLenCnt[1] = pJFrmComp->huff_DC[1].totalCodeLenCnt;
            pJHwCtrl->acHuffW2talCodeLenCnt[1] = pJFrmComp->huff_AC[1].totalCodeLenCnt;

            // --------------------------------------------
            // set Q-table
            pJHwCtrl->qTableY = pJFrmComp->qTable.table[pJFrmComp->jFrmInfo[0].qTableSel];
            if( pJFrmComp->qTable.tableCnt == 1 )
            {
                // only one Q table case
                pJHwCtrl->qTableUv = pJFrmComp->qTable.table[pJFrmComp->jFrmInfo[0].qTableSel];
                pJFrmComp->qTable.tableCnt = 2;
            }
            else
                pJHwCtrl->qTableUv = pJFrmComp->qTable.table[pJFrmComp->jFrmInfo[1].qTableSel];
#endif
            // --------------------------------------------
            // set HW register
            _JPG_Cmd_HW_Update(pHJCodec);
        }

    }while(0);

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_DEC_JPG_CMD);
    return result;
}

static JPG_ERR
jpg_dec_cmd_fire(
    JPG_CODEC_HANDLE    *pHJCodec,
    void                *extraData)
{
    JPG_ERR             result = JPG_ERR_OK;
    JPG_LINE_BUF_INFO   *pJLineBufInfo = &pHJCodec->jLineBufInfo;
    JPG_HW_BS_CTRL      *pJHwBsCtrl = &pHJCodec->jHwBsCtrl;
    JPG_FRM_COMP        *pJFrmComp = &pHJCodec->jFrmCompInfo;
    JCOMM_HANDLE        *pHJComm = (JCOMM_HANDLE*)extraData;
    uint8_t*	WriteBuf = NULL; //uint8_t*	writeBuffer = NULL;
    uint8_t*    mappedSysRam = NULL;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_DEC_JPG_CMD, "0x%x, 0x%x\n", pHJCodec, extraData);

    do{
        //---------------------------------
        // set input bit stream info
        pJHwBsCtrl->addr = pHJCodec->pSysBsBuf;
        pJHwBsCtrl->size = pHJCodec->sysBsBufSize;

        // ----------------------------
        // reset bitstream info
        JPG_SetBitstreamReadBytePosReg(pJHwBsCtrl->shiftByteNum);

        // win32
        _jpg_reflash_vram(pHJCodec->pHInJStream,
                          JPG_STREAM_CMD_GET_VRAM_BS_BUF_A,
                          (uint32_t)pJHwBsCtrl->addr,
                          pJHwBsCtrl->size, &pJHwBsCtrl->addr);

        JPG_SetBitStreamBufInfoReg(pJHwBsCtrl);

        // --------------------------
        // reset line buffer (frame buffer) info
        JPG_SetLineBufInfoReg(pJLineBufInfo);
        //JPG_SetLineBufSliceWriteNumReg((jLineBufInfo->sliceNum/jFrmComp->jFrmInfo[0].verticalSamp));


		JPG_SetTilingTable(pJLineBufInfo, 0, 1);


        // ---------------------------
        // reset Q-table
        JPG_SetQtableReg(pJFrmComp);

        // Flush cache
        WriteBuf = (uint8_t *)itpVmemAlloc((pHJComm->jInBufInfo[pHJComm->actIOBuf_idx].bufLength ) );
        if(WriteBuf)
        {
            mappedSysRam = ithMapVram((uint32_t)WriteBuf, (pHJComm->jInBufInfo[pHJComm->actIOBuf_idx].bufLength) , ITH_VRAM_WRITE);
            memcpy(mappedSysRam, pHJComm->jInBufInfo[pHJComm->actIOBuf_idx].pBufAddr, (pHJComm->jInBufInfo[pHJComm->actIOBuf_idx].bufLength) );
            ithUnmapVram(mappedSysRam, (pHJComm->jInBufInfo[pHJComm->actIOBuf_idx].bufLength) );
            ithFlushDCacheRange(mappedSysRam, (pHJComm->jInBufInfo[pHJComm->actIOBuf_idx].bufLength) );
            ithFlushMemBuffer();
        }

        JPG_LogReg(false);
        JPG_StartReg();

        // ---------------------------
        //set bitstream buffer read size
        JPG_SetBitstreamBufRwSizeReg((pJHwBsCtrl->size + 0x3) & ~0x3);

        JPG_SetBitstreamBufCtrlReg((JPG_REG)(JPG_MSK_BITSTREAM_BUF_RW_END | JPG_MSK_LAST_BITSTREAM_DATA));

        // ---------------------------
        //free Writeback buffer
        if (WriteBuf)
        {
            itpVmemFree((uint32_t)WriteBuf);
        }
    }while(0);

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_DEC_JPG_CMD);
    return result;
}

static JPG_ERR
jpg_dec_cmd_ctrl(
    JPG_CODEC_HANDLE    *pHJCodec,
    uint32_t            cmd,
    uint32_t            *value,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_DEC_JPG_CMD, "0x%x, 0x%x\n", pHJCodec, extraData);

    switch( cmd )
    {
        case JCODEC_CMD_WAIT_IDLE:
            result = _JPG_Cmd_HW_Wait_Idle(JPG_TIMEOUT_COUNT);
            break;

        default:
            result = JPG_ERR_NO_IMPLEMENT;
            break;
    }

    if( result != JPG_ERR_OK &&
        result != JPG_ERR_NO_IMPLEMENT )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_DEC_JPG_CMD);
    return result;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
JPG_CODEC_DESC JPG_CODEC_DESC_dec_jpg_cmd_desc =
{
    "jpg dec no ISP",       // const char              *codecName;
    NULL,                   // struct JPG_CODEC_DESC_TAG  *next;
    JPG_CODEC_DEC_JPG_CMD,  // const JPG_CODEC_TYPE       id;
    jpg_dec_cmd_init,          // JPG_ERR (*init)(struct JPG_CODEC_HANDLE_TAG, void *extraData);
    jpg_dec_cmd_deInit,        // JPG_ERR (*deinit)(struct JPG_CODEC_HANDLE_TAG, void *extraData);
    jpg_dec_cmd_setup,         // JPG_ERR (*setup)(struct JPG_CODEC_HANDLE_TAG, void *extraData);
    jpg_dec_cmd_fire,          // JPG_ERR (*fire)(struct JPG_CODEC_HANDLE_TAG, void *extraData);
    jpg_dec_cmd_ctrl,          // JPG_ERR (*control)(struct JPG_CODEC_HANDLE_TAG, uint32_t cmd, uint32_t *value, void *extraData);
};
#else

JPG_CODEC_DESC JPG_CODEC_DESC_dec_jpg_cmd_desc = {0};
#endif


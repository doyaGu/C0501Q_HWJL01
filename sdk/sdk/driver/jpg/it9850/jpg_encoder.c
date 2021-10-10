#include "jpg_defs.h"
#include "jpg_codec.h"
#include "jpg_hw.h"
#include "jpg_common.h"
#include "jpg_extern_link.h"

#if (CONFIG_JPG_CODEC_DESC_ENCODER_DESC)
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct JPG_ENCODER_TAG
{
    JPG_TRIGGER_MODE        triggerMode;

    uint8_t                 *pSysBsBuf_Cur;  // record the current position in pSysBsBuf

    JPG_BS_RING_BUF_INFO    jBsRingBufInfo;

    uint8_t                 *pYuvEncBuf;
    uint32_t                yuvEncBufSize;

}JPG_ENCODER;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
static JPG_ERR
_JPG_Enc_HW_Update(
    JPG_CODEC_HANDLE    *pHJCodec)
{
    JPG_ERR             result = JPG_ERR_OK;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ENC, "0x%x\n", pHJCodec);

    do{
        JPG_ENCODER         *pJEncoder = (JPG_ENCODER*)pHJCodec->privData;
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

        JPG_SetBitStreamBufInfoReg(pJHwBsCtrl);

        JPG_SetSamplingFactorReg(pJFrmComp);

        // set Q table
        JPG_SetQtableReg(pJFrmComp);

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

        JPG_SetEncodeAcHuffmanValueReg(JPG_HUUFFMAN_Y_AC,
                                       (pJHwCtrl->acHuffTable[0] + 16),
                                       pJHwCtrl->acHuffW2talCodeLenCnt[0]);

        JPG_SetEncodeAcHuffmanValueReg(JPG_HUUFFMAN_UV_AC,
                                       (pJHwCtrl->acHuffTable[1] + 16),
                                       pJHwCtrl->acHuffW2talCodeLenCnt[1]);

        JPG_DropHv((JPG_REG)(pHJCodec->ctrlFlag >> 16));

#if defined(CFG_CHIP_PKG_IT9910)
        JPG_SetTilingMode();
#endif

        JPG_SetTilingTable(pJLineBufInfo, 1, 1);                   
    }while(0);

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ENC);
    return result;
}

static JPG_ERR
_JPG_Enc_HW_Wait_Idle(
    uint32_t      timeOutCnt,
    JPG_REG       *bufStatus)
{
#define JPG_ENC_STOP_STATUS     (JPG_STATUS_BITSTREAM_BUF_FULL | JPG_STATUS_ENCODE_COMPLETE | JPG_STATUS_LINE_BUF_EMPTY)

    JPG_ERR     result = JPG_ERR_OK;
    uint32_t    cnt = 0;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ENC, "%d, 0x%x\n", timeOutCnt, bufStatus);

    *bufStatus = JPG_GetProcStatusReg();
    while( !((*bufStatus) & JPG_ENC_STOP_STATUS) )
    {
        jpg_sleep(1);
        cnt++;
        if( cnt > timeOutCnt )
        {
            result = JPG_ERR_ENC_TIMEOUT;
            JPG_LogReg(true);
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg err 0x%x !", result);
            break;
        }

        *bufStatus = JPG_GetProcStatusReg();
    }

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ENC);
    return result;
}


static JPG_ERR
_JPG_Enc_Copy_To_SysBsBuf(
    JPG_CODEC_HANDLE    *pHJCodec,
    uint32_t            copySize)
{
    JPG_ERR                 result = JPG_ERR_OK;
    uint32_t                resiBsBufSize = 0;
    uint32_t                ringBsBufSize = 0;
    JPG_MEM_MOVE_INFO       jMemInfo = {0};
    JPG_ENCODER             *pJEncoder = (JPG_ENCODER*)pHJCodec->privData;
    JPG_BS_RING_BUF_INFO    *pJBsRingBufInfo = &((JPG_ENCODER*)pHJCodec->privData)->jBsRingBufInfo;
    uint8_t                 *pCur = 0, *pVramBsRingBuf = 0, *pNewBsBufAddr = 0;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ENC, "0x%x, %d\n", pHJCodec, copySize);

    pCur = pJEncoder->pSysBsBuf_Cur;

    pVramBsRingBuf = pJBsRingBufInfo->pBsRingBuf;
    // win32
    _jpg_reflash_vram(pHJCodec->pHInJStream,
                      JPG_STREAM_CMD_GET_VRAM_ENC_RING_BUF,
                      0, 0, &pVramBsRingBuf);

    pNewBsBufAddr = (uint8_t*)((uint32_t)pVramBsRingBuf + pJBsRingBufInfo->rwSize);

    resiBsBufSize = pJBsRingBufInfo->bsRingBufLeng - pJBsRingBufInfo->rwSize;

    copySize &= ~0x1;

    ringBsBufSize = (copySize > resiBsBufSize) ? (copySize - resiBsBufSize) : ringBsBufSize;
    if( ringBsBufSize > 0 )
    {
        /**
         *                              last       wrSize
         *  |<-------------------------->|<--------->|<----copy this segment----->|
         */
        // cpu_invalidate_cache();
        jMemInfo.dstAddr    = 0;
        jMemInfo.srcAddr    = (uint32_t)pNewBsBufAddr;
        jMemInfo.sizeByByte = resiBsBufSize;
        Jpg_Ext_Link_Ctrl(JEL_CTRL_CPU_INVALD_CACHE, 0, (void*)&jMemInfo);

        // move data
        jMemInfo.dstAddr    = (uint32_t)pCur;
        jMemInfo.srcAddr    = (uint32_t)pNewBsBufAddr;
        jMemInfo.sizeByByte = resiBsBufSize;
        Jpg_Ext_Link_Ctrl(JEL_CTRL_HOST_R_MEM, 0, (void*)&jMemInfo);

        /**
         *  last == ringBitstreamBufSize
         *                             last        wrSize
         *  |<--copy this segment------>|<---------->|<------------------------->|
         */
        pVramBsRingBuf = pJBsRingBufInfo->pBsRingBuf;
        // win32
        _jpg_reflash_vram(pHJCodec->pHInJStream,
                          JPG_STREAM_CMD_GET_VRAM_ENC_RING_BUF,
                          0, 0, &pVramBsRingBuf);

        pNewBsBufAddr = pVramBsRingBuf;
        pCur          = (uint8_t *)((uint32_t)pCur + resiBsBufSize);

        // cpu_invalidate_cache();
        jMemInfo.dstAddr    = 0;
        jMemInfo.srcAddr    = (uint32_t)pNewBsBufAddr;
        jMemInfo.sizeByByte = ringBsBufSize;
        Jpg_Ext_Link_Ctrl(JEL_CTRL_CPU_INVALD_CACHE, 0, (void*)&jMemInfo);

        // move data
        jMemInfo.dstAddr    = (uint32_t)pCur;
        jMemInfo.srcAddr    = (uint32_t)pNewBsBufAddr;
        jMemInfo.sizeByByte = ringBsBufSize;
        Jpg_Ext_Link_Ctrl(JEL_CTRL_HOST_R_MEM, 0, (void*)&jMemInfo);

        pJBsRingBufInfo->rwSize = ringBsBufSize;
        pCur += ringBsBufSize;
  	}
    else
    {
        // cpu_invalidate_cache();
        jMemInfo.dstAddr    = 0;
        jMemInfo.srcAddr    = (uint32_t)pNewBsBufAddr;
        jMemInfo.sizeByByte = copySize;
        Jpg_Ext_Link_Ctrl(JEL_CTRL_CPU_INVALD_CACHE, 0, (void*)&jMemInfo);

        // move data
        jMemInfo.dstAddr    = (uint32_t)pCur;
        jMemInfo.srcAddr    = (uint32_t)pNewBsBufAddr;
        jMemInfo.sizeByByte = copySize;
        Jpg_Ext_Link_Ctrl(JEL_CTRL_HOST_R_MEM, 0, (void*)&jMemInfo);

        // update SW bitstream buffer read point
        pJBsRingBufInfo->rwSize = ((pJBsRingBufInfo->rwSize + copySize) % (pJBsRingBufInfo->bsRingBufLeng));
        pCur += copySize;
    }

    pJEncoder->pSysBsBuf_Cur = pCur;

    // set bitstream buffer read size
    JPG_SetBitstreamBufRwSizeReg(copySize);

    // set bitstream buffer read end
    JPG_SetBitstreamBufCtrlReg(JPG_MSK_BITSTREAM_BUF_RW_END);

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ENC);
    return result;
}

static JPG_ERR
_JPG_Enc_Get_CodedData(
    JPG_CODEC_HANDLE    *pHJCodec,
    bool                *bEncIdle)
{
    JPG_ERR             result = JPG_ERR_OK;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ENC, "0x%x, 0x%x\n", pHJCodec, bEncIdle);

    do{
        JPG_ENCODER     *pJEncoder = (JPG_ENCODER*)pHJCodec->privData;
        JPG_REG         hwStatus = 0, bufStatus = 0;
        uint32_t        bsBufValidSize = 0;
        int             sysRamRemainSize = 0;

        *bEncIdle = false;

        //----------------------------------------
        // check system bs buffer full or not
        sysRamRemainSize = pHJCodec->pSysBsBuf + pHJCodec->sysBsBufSize - pJEncoder->pSysBsBuf_Cur;
     	if( sysRamRemainSize < 2 ) // reserve last 2 bytes
        {
            // sys bs buffer full
            result = JPG_ERR_ENC_OVER_BS_BUF;
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg err 0x%x (Need to extend AP bit stream buffer) !", result);
            break;
        }

        //----------------------------------------
        // check H/W status
        result = _JPG_Enc_HW_Wait_Idle(JPG_TIMEOUT_COUNT, &bufStatus);
        if( result != JPG_ERR_OK )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg err 0x%x !", result);
            break;
        }

        // complete or bs_full
        bsBufValidSize = JPG_GetBitStreamValidSizeReg();
       	if( (int)bsBufValidSize > sysRamRemainSize )
        {
            // sys bs buffer full
            result = JPG_ERR_ENC_OVER_BS_BUF;
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg err 0x%x (Need to extend AP bit stream buffer) !", result);
            break;
        }

        // ------------------------------------
        // handle bs buffer full case (it should not be happened),
        if( bufStatus & JPG_STATUS_BITSTREAM_BUF_FULL )
        {
        	printf("bufStatus & JPG_STATUS_BITSTREAM_BUF_FULL\n");
            // Move bitstream in unit of 32 bytes alignment
            // if JPEG_STATUS_ENCODE_COMPLETE, remove all the remained bitstream at once.
            bsBufValidSize = ((bsBufValidSize >> 5) << 5);
        }

        if( bsBufValidSize > 0 )
        {
            // need to move out data which is in bs buffer.
            result = _JPG_Enc_Copy_To_SysBsBuf(pHJCodec, bsBufValidSize);
            if( result != JPG_ERR_OK )
            {
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg err 0x%x !", result);
                break;
            }
        }

        (*bEncIdle) = (bufStatus & JPG_STATUS_LINE_BUF_EMPTY) ? true : (*bEncIdle);

        // --------------------------------------
        // handle encoding process complete
        if( bufStatus & JPG_STATUS_ENCODE_COMPLETE )
        {
            JPG_SetBitstreamBufCtrlReg(JPG_MSK_LAST_BITSTREAM_DATA);
            *bEncIdle = true;
        }
    }while(0);

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ENC);
    return result;
}

static uint8_t*
_JPG_Enc_Clear_Dummy(
    uint8_t     *ptEnd)
{
    int   validLeng = 0;

    while( validLeng < 10 ) // 10 => just follow old version
    {
        ptEnd--;
        if( *ptEnd != 0xFF )
            break;
        validLeng++;
    }

    // add end of image marker
    *(ptEnd + 1) = 0xFF;
    *(ptEnd + 2) = 0xD9;

    ptEnd += 3;

    return ptEnd;
}

static JPG_ERR
_JPG_Enc_Set_Line_buf(
    JPG_CODEC_HANDLE    *pHJCodec)
{
    JPG_ERR             result = JPG_ERR_OK;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ENC, "0x%x\n", pHJCodec);

    do{
        JPG_LINE_BUF_INFO   *pJLineBufInfo = &pHJCodec->jLineBufInfo;
        JPG_SHARE_DATA      *pJShare2Isp = &pHJCodec->jShare2Isp;
        JPG_STREAM_DESC     *pJStreamDesc = &pHJCodec->pHInJStream->jStreamDesc;

        if( !pJShare2Isp->addrY || !pJShare2Isp->addrU || !pJShare2Isp->addrV )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, "line buf Null pointer !!");
            result = JPG_ERR_NULL_POINTER;
            break;
        }

    #ifdef _MSC_VER
        //------------------------------
        // allocate vram line buff
        if( pJStreamDesc->jHeap_mem )
        {
            JPG_ENCODER     *pJEncoder = (JPG_ENCODER*)pHJCodec->privData;
            JPG_FRM_COMP    *pJFrmComp = &pHJCodec->jFrmCompInfo;
            uint32_t        yuvSrcBufSize = 0;
            uint8_t         *pVramYuvBufAddr = 0, *pTmpBufAddr = 0;

            yuvSrcBufSize = ((pJShare2Isp->pitchY * (pJShare2Isp->sliceCount << 3)) << 1);
            pJEncoder->pYuvEncBuf = pJStreamDesc->jHeap_mem(pHJCodec->pHInJStream,
                                                            (uint32_t)JPG_HEAP_ENC_YUV_BUF,
                                                            yuvSrcBufSize, &pJEncoder->yuvEncBufSize);
            // Y
            pTmpBufAddr = pJEncoder->pYuvEncBuf;
            memcpy(pTmpBufAddr, pJShare2Isp->addrY, pJShare2Isp->pitchY * (pJShare2Isp->sliceCount << 3));
            // U
            pTmpBufAddr += (pJShare2Isp->pitchY * (pJShare2Isp->sliceCount << 3));
            memcpy(pTmpBufAddr, pJShare2Isp->addrU,
                   pJShare2Isp->pitchUv * (pJShare2Isp->sliceCount << 3) / pJFrmComp->jFrmInfo[0].verticalSamp);
            // V
            pTmpBufAddr += (pJShare2Isp->pitchUv * (pJShare2Isp->sliceCount << 3) / pJFrmComp->jFrmInfo[0].verticalSamp);
            memcpy(pTmpBufAddr, pJShare2Isp->addrV,
                   pJShare2Isp->pitchUv * (pJShare2Isp->sliceCount << 3) / pJFrmComp->jFrmInfo[0].verticalSamp);

            _jpg_reflash_vram(pHJCodec->pHInJStream, JPG_STREAM_CMD_GET_VRAM_ENC_YUV_BUF,
                              pJEncoder->pYuvEncBuf, yuvSrcBufSize, &pVramYuvBufAddr);

            pJLineBufInfo->comp1Addr = (uint8_t*)pVramYuvBufAddr;
            pJLineBufInfo->comp2Addr = (uint8_t*)(pJLineBufInfo->comp1Addr + (pJShare2Isp->pitchY * (pJShare2Isp->sliceCount << 3)));
            pJLineBufInfo->comp3Addr = (uint8_t*)(pJLineBufInfo->comp2Addr + (pJShare2Isp->pitchUv * (pJShare2Isp->sliceCount << 3) / pJFrmComp->jFrmInfo[0].verticalSamp));
            printf("line buf: y=0x%x,u=0x%x, v=0x%x\n", pJLineBufInfo->comp1Addr, pJLineBufInfo->comp2Addr, pJLineBufInfo->comp2Addr);
        }

    #else
        pJLineBufInfo->comp1Addr   = (uint8_t*)pJShare2Isp->addrY;
        pJLineBufInfo->comp2Addr   = (uint8_t*)pJShare2Isp->addrU;
        pJLineBufInfo->comp3Addr   = (uint8_t*)pJShare2Isp->addrV;
    #endif

        pJLineBufInfo->comp1Pitch  = pJShare2Isp->pitchY;
        pJLineBufInfo->comp23Pitch = pJShare2Isp->pitchUv;
        pJLineBufInfo->sliceNum    = pJShare2Isp->sliceCount;
    }while(0);

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ENC);
    return result;
}

static JPG_ERR
jpg_enc_init(
    JPG_CODEC_HANDLE    *pHJCodec,
    void                *extraData)
{
    JPG_ERR             result = JPG_ERR_OK;
    JPG_STREAM_DESC     *pJStreamDesc = &pHJCodec->pHInJStream->jStreamDesc;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ENC, "0x%x, 0x%x\n", pHJCodec, extraData);

    do{
        JPG_PowerUp();

        //------------------------------
        // allocate memory
        if( pJStreamDesc->jHeap_mem )
        {
            uint32_t                bsRingBufSize = 0;
            JPG_BS_RING_BUF_INFO    *pJBsRingBufInfo = 0;

            // allocate private data
            pHJCodec->privData = pJStreamDesc->jHeap_mem(
                                            pHJCodec->pHInJStream, JPG_HEAP_DEF,
                                            sizeof(JPG_ENCODER), 0);
            if( pHJCodec->privData == 0 )
            {
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " Allocate Line buf fail !! ", __FUNCTION__);
                result = JPG_ERR_ALLOCATE_FAIL;
                break;
            }
            memset(pHJCodec->privData, 0x0, sizeof(JPG_ENCODER));
            pJBsRingBufInfo = &((JPG_ENCODER*)pHJCodec->privData)->jBsRingBufInfo;

            // allocate jpg H/W bs ring buffer
            if( pJStreamDesc->jControl )
                pJStreamDesc->jControl(pHJCodec->pHInJStream,
                                       (uint32_t)JPG_STREAM_CMD_GET_BS_RING_BUF_SIZE,
                                       &bsRingBufSize, 0);

            pJBsRingBufInfo->pBsRingBuf = pJStreamDesc->jHeap_mem(
                                            pHJCodec->pHInJStream, JPG_HEAP_ENC_BS_RING_BUF,
                                            bsRingBufSize, &pJBsRingBufInfo->bsRingBufLeng);
            if( pJBsRingBufInfo->pBsRingBuf == 0 )
            {
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " Allocate bs ring buf fail !! ", __FUNCTION__);
                result = JPG_ERR_ALLOCATE_FAIL;
                break;
            }
        }
    }while(0);

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ENC);
    return result;
}

static JPG_ERR
jpg_enc_deInit(
    JPG_CODEC_HANDLE    *pHJCodec,
    void                *extraData)
{
    JPG_ERR             result = JPG_ERR_OK;
    JPG_STREAM_DESC     *pJStreamDesc = &pHJCodec->pHInJStream->jStreamDesc;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ENC, "0x%x, 0x%x\n", pHJCodec, extraData);

    JPG_EncPowerDown();

    if( pJStreamDesc->jFree_mem )
    {
        if( pHJCodec->privData )
        {
            JPG_BS_RING_BUF_INFO    *pJBsRingBufInfo = 0;
            uint8_t                 *pYuvEncBuf = 0;

            pJBsRingBufInfo = &((JPG_ENCODER*)pHJCodec->privData)->jBsRingBufInfo;

            // free H/W bs ring buffer
            if( pJBsRingBufInfo->pBsRingBuf )
            {
                pJStreamDesc->jFree_mem(pHJCodec->pHInJStream, JPG_HEAP_ENC_BS_RING_BUF, pJBsRingBufInfo->pBsRingBuf);
                pJBsRingBufInfo->pBsRingBuf = 0;
            }

            // free yuv encoded src buf, win32 case
            pYuvEncBuf = ((JPG_ENCODER*)pHJCodec->privData)->pYuvEncBuf;
            if( pYuvEncBuf )
            {
                pJStreamDesc->jFree_mem(pHJCodec->pHInJStream, JPG_HEAP_ENC_YUV_BUF, pYuvEncBuf);
                ((JPG_ENCODER*)pHJCodec->privData)->pYuvEncBuf = 0;
            }

            // free private data
            pJStreamDesc->jFree_mem(pHJCodec->pHInJStream, JPG_HEAP_DEF, pHJCodec->privData);
            pHJCodec->privData = 0;
        }
    }

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ENC);
    return result;
}


static JPG_ERR
jpg_enc_setup(
    JPG_CODEC_HANDLE    *pHJCodec,
    void                *extraData)
{
    JPG_ERR         result = JPG_ERR_OK;
    JPG_ENCODER     *pJEncoder = (JPG_ENCODER*)pHJCodec->privData;
    JPG_HW_CTRL     *pJHwCtrl = &pHJCodec->jHwCtrl;
    JPG_FRM_COMP    *pJFrmComp = &pHJCodec->jFrmCompInfo;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ENC, "0x%x, 0x%x\n", pHJCodec, extraData);

    // set JPG_COMMAND_TRIGGER mode
    pJEncoder->triggerMode = JPG_COMMAND_TRIGGER;

    pJHwCtrl->codecCtrl = (JPG_OP_ENCODE | pJEncoder->triggerMode);
    pJHwCtrl->codecCtrl |= (jpgCompCtrl[pJFrmComp->compNum] & JPG_MSK_LINE_BUF_COMPONENT_VALID);

    do{
        //JPG_LINE_BUF_INFO   *pJLineBufInfo = &pHJCodec->jLineBufInfo;
        JPG_SHARE_DATA      *pJShare2Isp = &pHJCodec->jShare2Isp;
        JPG_HW_BS_CTRL      *pJHwBsCtrl = &pHJCodec->jHwBsCtrl;
        uint8_t             *pVramBsBufAddr = 0;

        // set line buffer information (YUV)
        result = _JPG_Enc_Set_Line_buf(pHJCodec);
        if( result != JPG_ERR_OK )      break;

        // set H/W bit-stream ring buffer
        pVramBsBufAddr = pJEncoder->jBsRingBufInfo.pBsRingBuf;
        // win32
        _jpg_reflash_vram(pHJCodec->pHInJStream, JPG_STREAM_CMD_GET_VRAM_ENC_RING_BUF, 0, 0, &pVramBsBufAddr);


        pJHwBsCtrl->addr = pVramBsBufAddr;
        pJHwBsCtrl->size = pJEncoder->jBsRingBufInfo.bsRingBufLeng;

        // set HW register
        _JPG_Enc_HW_Update(pHJCodec);

        //---------------------------------
        // start fire jpg
        JPG_LogReg(false);

#ifdef CFG_CPU_WB			
		ithFlushDCacheRange(pJHwBsCtrl->addr ,	pJHwBsCtrl->size);
		ithFlushMemBuffer();
#endif

        JPG_StartReg();
    }while(0);

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ENC);
    return result;
}

static JPG_ERR
jpg_enc_fire(
    JPG_CODEC_HANDLE    *pHJCodec,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;
    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ENC, "0x%x, 0x%x\n", pHJCodec, extraData);

    do{
        JPG_ENCODER         *pJEncoder = (JPG_ENCODER*)pHJCodec->privData;
        JPG_MULTI_SECT_INFO *pJMultiSectInfo = &pHJCodec->jMultiSectInfo;
        JPG_LINE_BUF_INFO   *pJLineBufInfo = &pHJCodec->jLineBufInfo;
        JPG_FRM_COMP        *pJFrmComp = &pHJCodec->jFrmCompInfo;
        JPG_FRM_SIZE_INFO   *pJFrmSizeInfo = &pHJCodec->jFrmSizeInfo;
        int                 remainSliceNum = 0, procSliceNum = 0;
        bool                bEncIdle = false, bSkip = false;

       pJEncoder->jBsRingBufInfo.rwSize = 0;

        if( pJMultiSectInfo->bFirst == true )
            pJMultiSectInfo->bFirst = false;
        else
        {
            // update line buf info
            JPG_SetLineBufInfoReg(pJLineBufInfo);
        }

#ifdef CFG_CPU_WB
		ithInvalidateDCacheRange(pHJCodec->pSysBsBuf, pHJCodec->sysBsBufSize);
#endif
        pJEncoder->pSysBsBuf_Cur = pHJCodec->pSysBsBuf;

        // --------------------------------
        // Set slice number
        remainSliceNum = ((pJMultiSectInfo->section_hight + 7) >> 3);
        JPG_SetLineBufSliceUnitReg(remainSliceNum, pJFrmComp->jFrmInfo[0].verticalSamp);  //Ex. 1280 x 1024 encoded frame and 422 has (1024/(1x8)) slice unit.

        // --------------------------------
        // command trigger (while loop not work now)
        while( remainSliceNum > 0 )
        {
            procSliceNum = remainSliceNum; // ENC_SLICE_STEP;

            // set line buffer r/w data size
            JPG_SetLineBufSliceWriteNumReg((JPG_REG)(procSliceNum/pJFrmComp->jFrmInfo[0].verticalSamp));
            // set line buffer write end
            JPG_SetLineBufCtrlReg((JPG_REG)JPG_MSK_LINE_BUF_WRITE_END);

            remainSliceNum -= procSliceNum;

            // set last line buffer data flag
            if( pJMultiSectInfo->bFinished == true )
            {
                JPG_SetLineBufCtrlReg((JPG_REG)JPG_MSK_LAST_ENCODE_DATA);
            }
            jpg_sleep(1); 

            // check encode status (if want to work while loop)
            // 1. check H/W busy or idle => JPG_GetProcStatusReg()
            // 2. bs buffer full => need to move bit stream from bs buffer
                // 2.1. set bitstream buffer read size => JPG_SetBitstreamBufRwSizeReg();
                // 2.2. set bitstream buffer read end => JPG_SetBitstreamBufCtrlReg(JPEG_MSK_BITSTREAM_BUF_RW_END);
        }

        // ----------------------------------------
        // Read valid data in bitstream ring buffer
        while( bEncIdle == false )
        {
            result = _JPG_Enc_Get_CodedData(pHJCodec, &bEncIdle);
            if( result != JPG_ERR_OK )
            {
                jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);
                bSkip = true;
                break;
            }
            jpg_sleep(1); 
            
        }

        if( bSkip == true )     break;

        if( pJMultiSectInfo->bFinished == true )
        {
            // remove dummy bytes (0xFF) and add EOI marker
            pJEncoder->pSysBsBuf_Cur = _JPG_Enc_Clear_Dummy(pJEncoder->pSysBsBuf_Cur);
        }

        pHJCodec->sysValidBsBufSize = (pJEncoder->pSysBsBuf_Cur - pHJCodec->pSysBsBuf);
    }while(0);

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ENC);
    return result;
}

static JPG_ERR
jpg_enc_ctrl(
    JPG_CODEC_HANDLE    *pHJCodec,
    uint32_t            cmd,
    uint32_t            *value,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ENC, "0x%x, 0x%x, 0x%x, 0x%x\n", pHJCodec, cmd, value, extraData);

    switch( cmd )
    {
        default:
            result = JPG_ERR_NO_IMPLEMENT;
            break;
    }

    if( result != JPG_ERR_OK &&
        result != JPG_ERR_NO_IMPLEMENT )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ENC);
    return result;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
JPG_CODEC_DESC JPG_CODEC_DESC_encoder_desc =
{
    "signal jpg enc",       // const char              *codecName;
    NULL,                   // struct JPG_CODEC_DESC_TAG  *next;
    JPG_CODEC_ENC_JPG,      // const JPG_CODEC_TYPE       id;
    jpg_enc_init,          // JPG_ERR (*init)(struct JPG_CODEC_HANDLE_TAG, void *extraData);
    jpg_enc_deInit,        // JPG_ERR (*deinit)(struct JPG_CODEC_HANDLE_TAG, void *extraData);
    jpg_enc_setup,         // JPG_ERR (*setup)(struct JPG_CODEC_HANDLE_TAG, void *extraData);
    jpg_enc_fire,          // JPG_ERR (*fire)(struct JPG_CODEC_HANDLE_TAG, void *extraData);
    jpg_enc_ctrl,          // JPG_ERR (*control)(struct JPG_CODEC_HANDLE_TAG, uint32_t cmd, uint32_t *value, void *extraData);
};
#else

JPG_CODEC_DESC JPG_CODEC_DESC_encoder_desc = {0};
#endif


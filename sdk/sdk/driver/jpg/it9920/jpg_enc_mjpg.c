

#include "jpg_defs.h"
#include "jpg_codec.h"
#include "jpg_hw.h"
#include "jpg_common.h"
#include "jpg_extern_link.h"

#if (CONFIG_JPG_CODEC_DESC_ENC_MJPG_DESC)
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct MJPG_ENCODER_TAG
{
    JPG_TRIGGER_MODE        triggerMode;

    uint8_t                 *pSysBsBuf_Cur;  // record the current position in pSysBsBuf

    JPG_BS_RING_BUF_INFO    jBsRingBufInfo;

}MJPG_ENCODER;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
static JPG_ERR
_MJPG_Enc_HW_Update(
    JPG_CODEC_HANDLE    *pHJCodec)
{
    JPG_ERR             result = JPG_ERR_OK;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ENC_MJPG, "0x%x\n", pHJCodec);

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

    }while(0);

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ENC_MJPG);
    return result;
}

static JPG_ERR
_MJPG_Enc_HW_Wait_Idle(
    uint32_t      timeOutCnt,
    JPG_REG       *bufStatus)
{
#define JPG_ENC_STOP_STATUS     (JPG_STATUS_BITSTREAM_BUF_FULL | JPG_STATUS_ENCODE_COMPLETE | JPG_STATUS_LINE_BUF_EMPTY)

    JPG_ERR     result = JPG_ERR_OK;
    uint32_t    cnt = 0;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ENC_MJPG, "%d, 0x%x\n", timeOutCnt, bufStatus);

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

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ENC_MJPG);
    return result;
}

static JPG_ERR
_MJPG_Enc_Copy_To_SysBsBuf(
    JPG_CODEC_HANDLE    *pHJCodec,
    uint32_t            copySize)
{
    JPG_ERR                 result = JPG_ERR_OK;
    uint32_t                resiBsBufSize = 0;
    uint32_t                ringBsBufSize = 0;
    JPG_MEM_MOVE_INFO       jMemInfo = {0};
    MJPG_ENCODER            *pMjEncoder = (MJPG_ENCODER*)pHJCodec->privData;
    JPG_BS_RING_BUF_INFO    *pJBsRingBufInfo = &((MJPG_ENCODER*)pHJCodec->privData)->jBsRingBufInfo;
    uint8_t                 *pCur = 0, *pVramBsRingBuf = 0, *pNewBsBufAddr = 0;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ENC_MJPG, "0x%x, %d\n", pHJCodec, copySize);

    pCur = pMjEncoder->pSysBsBuf_Cur;

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

    pMjEncoder->pSysBsBuf_Cur = pCur;

    // set bitstream buffer read size
    JPG_SetBitstreamBufRwSizeReg(copySize);

    // set bitstream buffer read end
    JPG_SetBitstreamBufCtrlReg(JPG_MSK_BITSTREAM_BUF_RW_END);

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ENC_MJPG);
    return result;
}

static JPG_ERR
_MJPG_Enc_Get_CodedData(
    JPG_CODEC_HANDLE    *pHJCodec,
    bool                *bEncIdle)
{
    JPG_ERR             result = JPG_ERR_OK;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ENC_MJPG, "0x%x, 0x%x\n", pHJCodec, bEncIdle);

    do{
        MJPG_ENCODER    *pMjEncoder = (MJPG_ENCODER*)pHJCodec->privData;
        JPG_REG         hwStatus = 0, bufStatus = 0;
        uint32_t        bsBufValidSize = 0;
        int             sysRamRemainSize = 0;

        *bEncIdle = false;

        //----------------------------------------
        // check system bs buffer full or not
        sysRamRemainSize = pHJCodec->pSysBsBuf + pHJCodec->sysBsBufSize - pMjEncoder->pSysBsBuf_Cur;
        if( sysRamRemainSize < 2 ) // reserve last 2 bytes
        {
            // sys bs buffer full
            result = JPG_ERR_ENC_OVER_BS_BUF;
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg err 0x%x (Need to extend AP bit stream buffer) !", result);
            break;
        }

        //----------------------------------------
        // check H/W status
        result = _MJPG_Enc_HW_Wait_Idle(JPG_TIMEOUT_COUNT, &bufStatus);
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
            // Move bitstream in unit of 32 bytes alignment
            // if JPEG_STATUS_ENCODE_COMPLETE, remove all the remained bitstream at once.
            bsBufValidSize = ((bsBufValidSize >> 5) << 5);
        }

        if( bsBufValidSize > 0 )
        {
            // need to move out data which is in bs buffer.
            result = _MJPG_Enc_Copy_To_SysBsBuf(pHJCodec, bsBufValidSize);
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

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ENC_MJPG);
    return result;
}

static uint8_t*
_MJPG_Enc_Clear_Dummy(
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
_MJPG_Enc_Set_Line_buf(
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

        pJLineBufInfo->comp1Addr   = (uint8_t*)pJShare2Isp->addrY;
        pJLineBufInfo->comp2Addr   = (uint8_t*)pJShare2Isp->addrU;
        pJLineBufInfo->comp3Addr   = (uint8_t*)pJShare2Isp->addrV;
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
mjpg_enc_init(
    JPG_CODEC_HANDLE    *pHJCodec,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ENC_MJPG, "0x%x, 0x%x\n", pHJCodec, extraData);

    do{
        JPG_STREAM_DESC     *pJStreamDesc = &pHJCodec->pHInJStream->jStreamDesc;

        JPG_PowerUp();

        pHJCodec->ctrlFlag |= JPG_FLAGS_MJPG;
        pHJCodec->ctrlFlag |= JPG_FLAGS_MJPG_FIRST_FRAME;

        if( pJStreamDesc->jHeap_mem )
        {
            uint32_t                bsRingBufSize = 0;
            JPG_BS_RING_BUF_INFO    *pJBsRingBufInfo = 0;

            // allocate private data
            pHJCodec->privData = pJStreamDesc->jHeap_mem(
                                            pHJCodec->pHInJStream, JPG_HEAP_DEF,
                                            sizeof(MJPG_ENCODER), 0);
            if( pHJCodec->privData == 0 )
            {
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " Allocate Line buf fail !! ", __FUNCTION__);
                result = JPG_ERR_ALLOCATE_FAIL;
                break;
            }
            memset(pHJCodec->privData, 0x0, sizeof(MJPG_ENCODER));

            pJBsRingBufInfo = &((MJPG_ENCODER*)pHJCodec->privData)->jBsRingBufInfo;

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

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ENC_MJPG);
    return result;
}

static JPG_ERR
mjpg_enc_deInit(
    JPG_CODEC_HANDLE    *pHJCodec,
    void                *extraData)
{
    JPG_ERR             result = JPG_ERR_OK;
    JPG_STREAM_DESC     *pJStreamDesc = &pHJCodec->pHInJStream->jStreamDesc;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ENC_MJPG, "0x%x, 0x%x\n", pHJCodec, extraData);

    JPG_EncPowerDown();

    if( pJStreamDesc->jFree_mem )
    {
        if( pHJCodec->privData )
        {
            JPG_BS_RING_BUF_INFO    *pJBsRingBufInfo = 0;

            pJBsRingBufInfo = &((MJPG_ENCODER*)pHJCodec->privData)->jBsRingBufInfo;

            // free H/W bs ring buffer
            if( pJBsRingBufInfo->pBsRingBuf )
            {
                pJStreamDesc->jFree_mem(pHJCodec->pHInJStream, JPG_HEAP_ENC_BS_RING_BUF, pJBsRingBufInfo->pBsRingBuf);
                pJBsRingBufInfo->pBsRingBuf = 0;
            }

            // free private data
            pJStreamDesc->jFree_mem(pHJCodec->pHInJStream, JPG_HEAP_DEF, pHJCodec->privData);
            pHJCodec->privData = 0;
        }
    }

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ENC_MJPG);
    return result;
}

static JPG_ERR
mjpg_enc_setup(
    JPG_CODEC_HANDLE    *pHJCodec,
    void                *extraData)
{
    JPG_ERR         result = JPG_ERR_OK;
    MJPG_ENCODER    *pMjEncoder = (MJPG_ENCODER*)pHJCodec->privData;
    JPG_HW_BS_CTRL  *pJHwBsCtrl = &pHJCodec->jHwBsCtrl;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ENC_MJPG, "0x%x, 0x%x\n", pHJCodec, extraData);

    do{
        // set line buffer information (YUV)
        result = _MJPG_Enc_Set_Line_buf(pHJCodec);
        if( result != JPG_ERR_OK )      break;

        if( pHJCodec->ctrlFlag & JPG_FLAGS_MJPG_FIRST_FRAME )
        {
            uint8_t             *pVramBsBufAddr = 0;

            pHJCodec->ctrlFlag &= ~JPG_FLAGS_MJPG_FIRST_FRAME;
            pHJCodec->bSkipPreSetting = true; // do not generate jpg header again

            // set H/W bit-stream ring buffer
            pVramBsBufAddr = pMjEncoder->jBsRingBufInfo.pBsRingBuf;
            // win32
            _jpg_reflash_vram(pHJCodec->pHInJStream, JPG_STREAM_CMD_GET_VRAM_ENC_RING_BUF, 0, 0, &pVramBsBufAddr);

            pJHwBsCtrl->addr = pVramBsBufAddr;
            pJHwBsCtrl->size = pMjEncoder->jBsRingBufInfo.bsRingBufLeng;

            // set HW register
            _MJPG_Enc_HW_Update(pHJCodec);
        }
        else
        {
            JPG_LINE_BUF_INFO   *pJLineBufInfo = &pHJCodec->jLineBufInfo;
            uint32_t            ringBsSize = pMjEncoder->jBsRingBufInfo.bsRingBufLeng;

            JPG_SetBitstreamBufRwSizeReg(ringBsSize & ~0x3);
            //JPG_SetLineBufSliceWriteNumReg(0);

            // update line buf info to H/W
            JPG_SetLineBufInfoReg(pJLineBufInfo);

            // set H/W bit-stream ring buffer
            pJHwBsCtrl->addr = pMjEncoder->jBsRingBufInfo.pBsRingBuf;
            pJHwBsCtrl->size = pMjEncoder->jBsRingBufInfo.bsRingBufLeng;

            // win32
            _jpg_reflash_vram(pHJCodec->pHInJStream, JPG_STREAM_CMD_GET_VRAM_ENC_RING_BUF, 0, 0, &pJHwBsCtrl->addr);

            JPG_SetBitStreamBufInfoReg(pJHwBsCtrl);
        }
    }while(0);

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ENC_MJPG);
    return result;
}

static JPG_ERR
mjpg_enc_fire(
    JPG_CODEC_HANDLE    *pHJCodec,
    void                *extraData)
{
    JPG_ERR             result = JPG_ERR_OK;
    MJPG_ENCODER        *pMjEncoder = (MJPG_ENCODER*)pHJCodec->privData;
    JPG_LINE_BUF_INFO   *pJLineBufInfo = &pHJCodec->jLineBufInfo;
    JPG_FRM_COMP        *pJFrmComp = &pHJCodec->jFrmCompInfo;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ENC_MJPG, "0x%x, 0x%x\n", pHJCodec, extraData);

    do{
        bool    bEncIdle = false, bSkip = false;
        int     procSliceNum = 0;

        // -----------------------------
        // update bistsream buf info
        pMjEncoder->pSysBsBuf_Cur = pHJCodec->pSysBsBuf;

        // --------------------------------
        // Set slice number
        procSliceNum = pJLineBufInfo->sliceNum;
        JPG_SetLineBufSliceUnitReg(procSliceNum, pJFrmComp->jFrmInfo[0].verticalSamp);

        JPG_StartReg();
        // --------------------------------
        // command trigger
            // set line buffer r/w data size
        JPG_SetLineBufSliceWriteNumReg((JPG_REG)(procSliceNum/pJFrmComp->jFrmInfo[0].verticalSamp));
            // set line buffer write end and last line buffer data flag

        JPG_SetLineBufCtrlReg((JPG_REG)(JPG_MSK_LINE_BUF_WRITE_END | JPG_MSK_LAST_ENCODE_DATA));

        // ----------------------------------------
        // Read valid data in bitstream ring buffer
        while( bEncIdle == false )
        {
            result = _MJPG_Enc_Get_CodedData(pHJCodec, &bEncIdle);
            if( result != JPG_ERR_OK )
            {
                jpg_msg_ex(JPG_MSG_TYPE_ERR, "Jpg err 0x%x !", result);
                bSkip = true;
                break;
            }
        }

        if( bSkip == true )     break;

        // remove dummy bytes (0xFF) and add EOI marker
        pMjEncoder->pSysBsBuf_Cur = _MJPG_Enc_Clear_Dummy(pMjEncoder->pSysBsBuf_Cur);

        pHJCodec->sysValidBsBufSize = (pMjEncoder->pSysBsBuf_Cur - pHJCodec->pSysBsBuf);
    }while(0);

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ENC_MJPG);
    return result;
}

static JPG_ERR
mjpg_enc_ctrl(
    JPG_CODEC_HANDLE    *pHJCodec,
    uint32_t            cmd,
    uint32_t            *value,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ENC_MJPG, "0x%x, 0x%x\n", pHJCodec, extraData);

    switch( cmd )
    {
        default:
            result = JPG_ERR_NO_IMPLEMENT;
            break;
    }

    if( result != JPG_ERR_OK &&
        result != JPG_ERR_NO_IMPLEMENT )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ENC_MJPG);
    return result;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
JPG_CODEC_DESC JPG_CODEC_DESC_enc_mjpg_desc =
{
    "motion jpg enc",       // const char              *codecName;
    NULL,                   // struct JPG_CODEC_DESC_TAG  *next;
    JPG_CODEC_ENC_MJPG,     // const JPG_CODEC_TYPE       id;
    mjpg_enc_init,          // JPG_ERR (*init)(struct JPG_CODEC_HANDLE_TAG, void *extraData);
    mjpg_enc_deInit,        // JPG_ERR (*deinit)(struct JPG_CODEC_HANDLE_TAG, void *extraData);
    mjpg_enc_setup,         // JPG_ERR (*setup)(struct JPG_CODEC_HANDLE_TAG, void *extraData);
    mjpg_enc_fire,          // JPG_ERR (*fire)(struct JPG_CODEC_HANDLE_TAG, void *extraData);
    mjpg_enc_ctrl,          // JPG_ERR (*control)(struct JPG_CODEC_HANDLE_TAG, uint32_t cmd, uint32_t *value, void *extraData);
};
#else

JPG_CODEC_DESC JPG_CODEC_DESC_enc_mjpg_desc = {0};
#endif


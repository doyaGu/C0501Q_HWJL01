

#include "jpg_defs.h"
#include "jpg_codec.h"
#include "jpg_hw.h"
#include "jpg_common.h"
#include "jpg_extern_link.h"

#if (CONFIG_JPG_CODEC_DESC_DEC_MJPG_DESC)
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct MJPG_DEOCDER_TAG
{
    JPG_TRIGGER_MODE    triggerMode;

//    uint8_t             *pSysBsStart;
//    int                 vramBsSize;
//    uint8_t             *vramBsAddr;
//    uint8_t             *vramBsCurr;
//    uint32_t            vramRwSize;

}MJPG_DEOCDER;
extern uint16_t  PITCH_ARR[6];


//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
static JPG_ERR
_MJPG_HW_Update(
    JPG_CODEC_HANDLE    *pHJCodec)
{
    JPG_ERR             result = JPG_ERR_OK;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_DEC_MJPG, "0x%x", pHJCodec);

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

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_DEC_MJPG);
    return result;
}

static JPG_ERR
_MJPG_HW_Wait_Idle(
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
            jpg_sleep(2);
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
_MJPG_Wait_Bs_Buf_Empty(
    uint32_t      timeOutCnt)
{
    JPG_ERR     result = JPG_ERR_OK;
    JPG_REG     bufStatus = 0;
    uint32_t    timeOut = 0;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_DEC, "%d\n", timeOutCnt);

    bufStatus = JPG_GetProcStatusReg();
    while( !(bufStatus & (JPG_STATUS_BITSTREAM_BUF_EMPTY | JPG_STATUS_DECODE_COMPLETE)) )
    {
        if( bufStatus & JPG_STATUS_DECODE_ERROR )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg err ! bufStatus = 0x%x\n", bufStatus);
            JPG_LogReg(true);
            result = JPG_ERR_DECODE_IRQ;
            break;
        }

        jpg_sleep(1);

        timeOut++;
        if( timeOut > timeOutCnt )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg timeout ! bufStatus = 0x%x\n", bufStatus);
            JPG_LogReg(true);
            result = JPG_ERR_BS_CONSUME_TIMEOUT;
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
_MJPG_Set_CodedData(
    JPG_CODEC_HANDLE    *pHJCodec,
    uint32_t            *actBsSize,
    bool                *bEnd)
{
    JPG_ERR             result = JPG_ERR_OK;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_DEC, "0x%x, %d, %d\n", pHJCodec, *actBsSize, *bEnd);

    do{
        JPG_HW_BS_CTRL      *pJHwBsCtrl = &pHJCodec->jHwBsCtrl;
        JPG_FRM_COMP        *pJFrmComp = &pHJCodec->jFrmCompInfo;
        JPG_REG             bufStatus = 0;

        *bEnd = false;

        bufStatus = JPG_GetProcStatusReg();
        if( bufStatus & JPG_STATUS_DECODE_COMPLETE )
            *bEnd = true;


        result = _MJPG_Wait_Bs_Buf_Empty(JPG_TIMEOUT_COUNT);
        if( result != JPG_ERR_OK )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg err !!");
            break;
        }

        if( pHJCodec->bLastSection == true && pJFrmComp->bProgressive == false )
        {
	        /*Why needs to rewrite the end of inputBuffer of endMarker(0xFFD9) again, I think it doesn`t necessary, Benson,2016/0315 */
			
            //*(pJHwBsCtrl->preBsBuf + (*actBsSize) - 2) = 0xFF;  
            //*(pJHwBsCtrl->preBsBuf + (*actBsSize) - 1) = 0xD9;
        }

        // set bs buffer addr and length
        pJHwBsCtrl->preBsBuf = pHJCodec->pSysBsBuf;
        pJHwBsCtrl->addr     = pHJCodec->pSysBsBuf;
        pJHwBsCtrl->size     = (*actBsSize);
        JPG_SetBitStreamBufInfoReg(pJHwBsCtrl);

        // set bs buffer read size
        JPG_SetBitstreamBufRwSizeReg((*actBsSize));

        // set bs buffer write end
        JPG_SetBitstreamBufCtrlReg((JPG_REG)JPG_MSK_BITSTREAM_BUF_RW_END);
        *actBsSize = 0;
        if( pHJCodec->bLastSection == true )
        {
            JPG_SetBitstreamBufCtrlReg((JPG_REG)JPG_MSK_LAST_BITSTREAM_DATA);
            *bEnd = true;
            //break;
        }

        bufStatus = JPG_GetProcStatusReg();
        if( bufStatus & JPG_STATUS_DECODE_ERROR )
        {
            result = JPG_ERR_DECODE_IRQ;
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg HW err !! ");
            break;
        }

    }while(0);

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_DEC);
    return result;
}


static JPG_ERR
_MJPG_Set_Less_Line_Buf(
    JPG_CODEC_HANDLE    *pHJCodec)
{
    JPG_ERR             result = JPG_ERR_OK;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_DEC, "0x%x\n", pHJCodec);

    do{
        JPG_LINE_BUF_INFO   *pJLineBufInfo = &pHJCodec->jLineBufInfo;
        JPG_FRM_SIZE_INFO   *pJFrmSizeInfo = &pHJCodec->jFrmSizeInfo;
        JPG_FRM_COMP        *pJFrmComp = &pHJCodec->jFrmCompInfo;
        uint32_t            real_w = pJFrmSizeInfo->realWidth;
        uint32_t            real_h = pJFrmSizeInfo->realHeight;
        int                 tmpSliceNum = 0;
        uint32_t            heightFactor = 1;
        uint16_t            i     =0;
        bool                bSkip = false;

        if( pHJCodec->ctrlFlag & JPG_FLAGS_DEC_DC_ONLY )
        {
            real_w >>= 3;
            real_h >>= 3;
        }
        else if( pHJCodec->ctrlFlag & JPG_FLAGS_DEC_Y_HOR_DOWNSAMPLE )
        {
            real_w >>= 1;
        }

        // 8-alignment       
        if (ithGetDeviceId() == 0x9850 && ithGetRevisionId() > 0 && ithIsTilingModeOn()) 
        {
           pJLineBufInfo->comp1Pitch = (real_w + 0x7) & ~0x7;
           for(i = 0; i< ((sizeof(PITCH_ARR) / sizeof(PITCH_ARR[0]))- 1); i++)
           {
               if( (PITCH_ARR[i] < pJLineBufInfo->comp1Pitch) && (PITCH_ARR[i+1] >= pJLineBufInfo->comp1Pitch) )
                   pJLineBufInfo->comp1Pitch = PITCH_ARR[i+1];
           }  
           pJLineBufInfo->comp23Pitch = pJLineBufInfo->comp1Pitch;           
           pJLineBufInfo->comp1Addr = (uint8_t*)((((uint32_t)pJLineBufInfo->addrAlloc + ((1 << 17) - 1) ) >> 17 ) << 17);
		   //printf("jpg_dec_mjpeg.c pJLineBufInfo->comp23Pitch = %d,pJLineBufInfo->comp1Addr=0x%x\n",pJLineBufInfo->comp23Pitch,pJLineBufInfo->comp1Addr);
                      
#if defined(WIN32)
           // win32
           _jpg_reflash_vram(pHJCodec->pHInJStream,
                             JPG_STREAM_CMD_GET_VRAM_LINE_BUF,
                             0, 0, &pJLineBufInfo->comp1Addr);                
#endif      
           tmpSliceNum = pJLineBufInfo->size - ((uint32_t)pJLineBufInfo->comp1Addr - (uint32_t)pJLineBufInfo->addrAlloc);         
           tmpSliceNum = tmpSliceNum / (1 << 18);
           pJLineBufInfo->sliceNum = tmpSliceNum * ((1 << 17) / (pJLineBufInfo->comp1Pitch * 8));    
           
           pJLineBufInfo->comp2Addr = pJLineBufInfo->comp1Addr + (pJLineBufInfo->sliceNum * pJLineBufInfo->comp1Pitch * 8);
           pJLineBufInfo->comp3Addr = pJLineBufInfo->comp2Addr + (pJLineBufInfo->comp23Pitch >> 1);            
        } 
        else
        {
           pJLineBufInfo->comp1Pitch = (real_w + 0x7) & ~0x7; 
           if(pJLineBufInfo->comp1Pitch  < 2048) pJLineBufInfo->comp1Pitch = 2048;
           
           // long time ago, Isp and Jpg H/W handshaking bug.
           // Now, it should be fixed but we lazily modify.
           tmpSliceNum = pJLineBufInfo->size / (pJLineBufInfo->comp1Pitch << 2);
           
           switch( pJFrmComp->decColorSpace )
           {
               case JPG_COLOR_SPACE_YUV444:
                   pJLineBufInfo->comp23Pitch = pJLineBufInfo->comp1Pitch;
                   heightFactor = 1;
                   pJLineBufInfo->sliceNum = (uint16_t)((tmpSliceNum / 6) & ~0x1);
                   break;
           
               case JPG_COLOR_SPACE_YUV411: // 411 will transfer to 422 for ISP
               case JPG_COLOR_SPACE_YUV422:
                   pJLineBufInfo->comp23Pitch = pJLineBufInfo->comp1Pitch >> 1;
                   heightFactor = 1;
                   pJLineBufInfo->sliceNum = (uint16_t)((tmpSliceNum >> 2) & ~0x1);
                   break;
           
               case JPG_COLOR_SPACE_YUV420:
                   pJLineBufInfo->comp23Pitch = pJLineBufInfo->comp1Pitch >> 1;
                   heightFactor = 2;
                   pJLineBufInfo->sliceNum = (uint16_t)((tmpSliceNum / 3)  & ~0x1);
                   break;
           
               case JPG_COLOR_SPACE_YUV422R:
                   pJLineBufInfo->comp23Pitch = pJLineBufInfo->comp1Pitch;
                   heightFactor = 2;
                   pJLineBufInfo->sliceNum = (uint16_t)((tmpSliceNum >> 2) & ~0x1);
                   break;
           
               default:
                   jpg_msg_ex(JPG_MSG_TYPE_ERR, "Wrong color format (0x%x)!! ", pJFrmComp->decColorSpace);
                   result = JPG_ERR_INVALID_PARAMETER;
                   bSkip = true;
                   break;
           }
           
           if( bSkip == true )     break;
           
           pJLineBufInfo->sliceNum = (pJLineBufInfo->sliceNum > 100) ? 100 : pJLineBufInfo->sliceNum;
           
           pJLineBufInfo->ySliceByteSize = (pJLineBufInfo->comp1Pitch << 3);
           pJLineBufInfo->uSliceByteSize = (pJLineBufInfo->comp23Pitch << 3) / heightFactor;
           pJLineBufInfo->vSliceByteSize = pJLineBufInfo->uSliceByteSize;
           
           // comp1Addr should be the same with addrAlloc
           pJLineBufInfo->comp1Addr = (uint8_t*)(((uint32_t)pJLineBufInfo->addrAlloc + 3) & ~3);
           
           // win32
           _jpg_reflash_vram(pHJCodec->pHInJStream,
                             JPG_STREAM_CMD_GET_VRAM_LINE_BUF,
                             0, 0, &pJLineBufInfo->comp1Addr);
           
           pJLineBufInfo->comp2Addr = pJLineBufInfo->comp1Addr + (pJLineBufInfo->sliceNum * pJLineBufInfo->ySliceByteSize);
           pJLineBufInfo->comp3Addr = pJLineBufInfo->comp2Addr + (pJLineBufInfo->sliceNum * pJLineBufInfo->uSliceByteSize);
        }
        if( pJFrmComp->bSingleChannel == true )
        {
            // Single channel will be processed with YUV444 by H/W
            uint8_t             wrColor = 0x80;
            JPG_MEM_MOVE_INFO   memInfo = {0};
            uint32_t            i, j, size;

            memInfo.sizeByByte = 1;

            size = (pJLineBufInfo->sliceNum * pJLineBufInfo->uSliceByteSize) >> 3;

            // ??? we should do so stupid flow ???
            for(j = 0; j < 8; j++)
            {
                for(i = 0; i < size; i++)
                {
                    memInfo.dstAddr = (uint32_t)pJLineBufInfo->comp2Addr + i + j*size;
                    memInfo.srcAddr = (uint32_t)(&wrColor);
                    Jpg_Ext_Link_Ctrl(JEL_CTRL_HOST_W_MEM, 0, (void*)&memInfo);
                    jpg_sleep(1);
                }
            }

            for(j = 0; j < 8; j++)
            {
                for(i = 0; i < size; i++)
                {
                    memInfo.dstAddr = (uint32_t)pJLineBufInfo->comp3Addr + i + j*size;
                    memInfo.srcAddr = (uint32_t)(&wrColor);
                    Jpg_Ext_Link_Ctrl(JEL_CTRL_HOST_W_MEM, 0, (void*)&memInfo);
                    jpg_sleep(1);
                }
            }
        }

    }while(0);

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_DEC);
    return result;

}


static JPG_ERR
mjpg_dec_init(
    JPG_CODEC_HANDLE    *pHJCodec,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_DEC_MJPG, "0x%x, 0x%x\n", pHJCodec, extraData);

    do{
        JPG_STREAM_DESC     *pJStreamDesc = &pHJCodec->pHInJStream->jStreamDesc;
		JCOMM_HANDLE        *pHJComm = (JCOMM_HANDLE*)extraData;


        // isp init for mjpeg isp handshake.
        if(pHJComm->pJDispInfo->outColorSpace == JPG_COLOR_SPACE_RGB565)
        {
	        result = Jpg_Ext_Link_Ctrl(JEL_CTRL_ISP_INIT, (uint32_t*)JEL_SET_ISP_SHOW_IMAGE, 0);
	        if( result )
	        {
	            jpg_msg_ex(JPG_MSG_TYPE_ERR, "Isp init err 0x%x !!", result);
	            break;
	        }
        }


        JPG_PowerUp();

        pHJCodec->ctrlFlag |= JPG_FLAGS_MJPG;
        pHJCodec->ctrlFlag |= JPG_FLAGS_MJPG_FIRST_FRAME;

        if( pJStreamDesc->jHeap_mem )
        {
            // allocate private data
            pHJCodec->privData = pJStreamDesc->jHeap_mem(
                                            pHJCodec->pHInJStream, JPG_HEAP_DEF,
                                            sizeof(MJPG_DEOCDER), 0);
			if(pHJCodec->privData)
            memset(pHJCodec->privData, 0x0, sizeof(MJPG_DEOCDER));
        }
    }while(0);

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_DEC_MJPG);
    return result;
}

static JPG_ERR
mjpg_dec_deInit(
    JPG_CODEC_HANDLE    *pHJCodec,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;
    JPG_STREAM_DESC     *pJStreamDesc = &pHJCodec->pHInJStream->jStreamDesc;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_DEC_MJPG, "0x%x, 0x%x\n", pHJCodec, extraData);

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

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_DEC_MJPG);
    return result;
}

static JPG_ERR
mjpg_dec_setup(
    JPG_CODEC_HANDLE    *pHJCodec,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;


    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_DEC_MJPG, "0x%x, 0x%x\n", pHJCodec, extraData);

    do{
        JCOMM_HANDLE        *pHJComm = (JCOMM_HANDLE*)pHJCodec->pHJComm;
        JPG_FRM_COMP        *pJFrmComp = &pHJCodec->jFrmCompInfo;
        JPG_HW_CTRL         *pJHwCtrl = &pHJCodec->jHwCtrl;
        JPG_LINE_BUF_INFO   *pJLineBufInfo = &pHJCodec->jLineBufInfo;
        JPG_HW_BS_CTRL      *pJHwBsCtrl = &pHJCodec->jHwBsCtrl;
        JPG_STREAM_HANDLE   *pHOutJStream = pHJCodec->pHOutJStream;
        MJPG_DEOCDER        *pMjDecoder = (MJPG_DEOCDER*)pHJCodec->privData;
     	bool                bMjpgFullScreen = false;

        // set JPG_COMMAND_TRIGGER mode or isp hand shake mode.
        if(pHJComm->pJDispInfo->outColorSpace ==JPG_COLOR_SPACE_RGB565)
        {
	        pMjDecoder->triggerMode = JPG_ISP_TRIGGER;
			//---------------------------------
			// set ISP engine
			// set display info and output buffer info
			bMjpgFullScreen = false; //Powei says don`t fullscreen it.
        	result = Jpg_Ext_Link_Ctrl(JEL_CTRL_ISP_SET_DISP_INFO, (uint32_t*)bMjpgFullScreen, (void*)pHJCodec->pHJComm);
	        if( result )
	        {
	            jpg_msg_ex(JPG_MSG_TYPE_ERR, " err, 0x%x !!", result);
	            break;
	        }
			//Benson add. // set line buffer
			result = _MJPG_Set_Less_Line_Buf(pHJCodec);
			if(result) printf("jpg_dec_mjpeg something wrong!\n");
        }
		else
        	pMjDecoder->triggerMode = JPG_COMMAND_TRIGGER;
        

        // ---------------------------
        // downsample or not
        #if 0
        if( pHJCodec->ctrlFlag & JPG_FLAGS_OUTPUT_RGB565 )
        {
            // set line buffer
            pJLineBufInfo->comp1Addr  = pHJComm->jOutBufInfo[0].pBufAddr;
            pJLineBufInfo->comp1Pitch = pHJComm->jOutBufInfo[0].pitch;
        }
        else
        {
        	//printf("jpg_dec_mjpeg pJFrmComp->decColorSpace=%d\n",pJFrmComp->decColorSpace); //Now go565
            // always output 420
            #if 0
            switch( pJFrmComp->decColorSpace )
            {
                case JPG_COLOR_SPACE_YUV422:
                    pHJCodec->ctrlFlag |= JPG_FLAGS_DEC_UV_VER_DOWNSAMPLE;
                    break;
                case JPG_COLOR_SPACE_YUV422R:
                    pHJCodec->ctrlFlag |= JPG_FLAGS_DEC_UV_HOR_DOWNSAMPLE;
                    break;
                case JPG_COLOR_SPACE_YUV444:
                    pHJCodec->ctrlFlag |= (JPG_FLAGS_DEC_UV_VER_DOWNSAMPLE | JPG_FLAGS_DEC_UV_HOR_DOWNSAMPLE);
                    break;
            }
			#endif
        }
		#endif

        // ??
        if(pHJComm->pJDispInfo->outColorSpace != JPG_COLOR_SPACE_RGB565)  // if Command mode , using it. Benson.
        {
        	pJLineBufInfo->sliceNum   = ((pJFrmComp->realHeight + 0x7) >> 3);
        }

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
            //#else
                jpg_msg(JPG_MSG_TYPE_ERR, "9070 Jpg not support YUV to RGB565. ");
                result = JPG_ERR_INVALID_PARAMETER;
                break;
            #endif
            }
            else
            {
                pJHwCtrl->codecCtrl = (JPG_OP_DECODE | pMjDecoder->triggerMode);
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
            _MJPG_HW_Update(pHJCodec);
        }
        else
        {
            // only update Q-table
            pJHwCtrl->qTableY = pJFrmComp->qTable.table[pJFrmComp->jFrmInfo[0].qTableSel];
            if( pJFrmComp->qTable.tableCnt == 1 )
            {
                // only one Q table case
                pJHwCtrl->qTableUv = pJFrmComp->qTable.table[pJFrmComp->jFrmInfo[0].qTableSel];
                pJFrmComp->qTable.tableCnt = 2;
            }
            else
                pJHwCtrl->qTableUv = pJFrmComp->qTable.table[pJFrmComp->jFrmInfo[1].qTableSel];
        }

    }while(0);

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_DEC_MJPG);
    return result;
}

static JPG_ERR
mjpg_dec_fire(
    JPG_CODEC_HANDLE    *pHJCodec,
    void                *extraData)
{
    JPG_ERR             result = JPG_ERR_OK;
    JPG_LINE_BUF_INFO   *pJLineBufInfo = &pHJCodec->jLineBufInfo;
    JPG_HW_BS_CTRL      *pJHwBsCtrl = &pHJCodec->jHwBsCtrl;
    JPG_FRM_COMP        *pJFrmComp = &pHJCodec->jFrmCompInfo;
	JCOMM_HANDLE        *pHJComm = (JCOMM_HANDLE*)extraData;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_DEC_MJPG, "0x%x, 0x%x\n", pHJCodec, extraData);

    do{

        //---------------------------------
        // set input bit stream info
        pJHwBsCtrl->addr = pHJCodec->pSysBsBuf;
        pJHwBsCtrl->size = pHJCodec->sysBsBufSize;

#ifdef  CFG_CPU_WB
        ithFlushDCacheRange((void*)pJHwBsCtrl->addr, pJHwBsCtrl->size);
        ithFlushMemBuffer();
#endif

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
	
		JPG_SetTilingTable(pJLineBufInfo, 0, 1);

        //JPG_SetLineBufSliceWriteNumReg((jLineBufInfo->sliceNum/jFrmComp->jFrmInfo[0].verticalSamp));

        // ---------------------------
        // reset Q-table
        JPG_SetQtableReg(pJFrmComp);

		//ithFlushDCache(); //Benson 
        //ithFlushMemBuffer(); //Benson

        JPG_LogReg(false);
        JPG_StartReg();

		//for Command trigger mode,mark it temporary. Benson
		if(pHJComm->pJDispInfo->outColorSpace != JPG_COLOR_SPACE_RGB565)
		{
	        // ---------------------------
	        //set bitstream buffer read size
	        JPG_SetBitstreamBufRwSizeReg((pJHwBsCtrl->size + 0x3) & ~0x3);

	        JPG_SetBitstreamBufCtrlReg((JPG_REG)(JPG_MSK_BITSTREAM_BUF_RW_END | JPG_MSK_LAST_BITSTREAM_DATA));
		}else
		{
            uint32_t            actBsSize = 0;
            bool                bEnd = false, bSkip = false;
			//---------------------------------
			// fire isp image process
			//if(1) //Benson
			JPG_FRM_SIZE_INFO   *pJFrmSizeInfo = &pHJCodec->jFrmSizeInfo;
			JPG_SHARE_DATA      *pJShare2Isp = &pHJCodec->jShare2Isp;
			//---------------------------------
			// store buffer information with isp_share_data
			if( pHJCodec->ctrlFlag & JPG_FLAGS_DEC_DC_ONLY )
			{
				pJShare2Isp->width  = (pJFrmSizeInfo->dispWidth >> 3);
				pJShare2Isp->height = (pJFrmSizeInfo->dispHeight >> 3);
			}
			else if( pHJCodec->ctrlFlag & JPG_FLAGS_DEC_Y_HOR_DOWNSAMPLE )
			{
				pJShare2Isp->width  = (pJFrmSizeInfo->dispWidth >> 1);
				pJShare2Isp->height = pJFrmSizeInfo->dispHeight;
			}
			else
			{
				pJShare2Isp->width  = pJFrmSizeInfo->dispWidth;
				pJShare2Isp->height = pJFrmSizeInfo->dispHeight;
			}

				pJShare2Isp->addrY       = (uint32_t)pJLineBufInfo->comp1Addr;
				pJShare2Isp->addrU       = (uint32_t)pJLineBufInfo->comp2Addr;
				pJShare2Isp->addrV       = (uint32_t)pJLineBufInfo->comp3Addr;
				pJShare2Isp->pitchY      = pJLineBufInfo->comp1Pitch;
				pJShare2Isp->pitchUv     = pJLineBufInfo->comp23Pitch;
				pJShare2Isp->sliceCount  = pJLineBufInfo->sliceNum;
				pJShare2Isp->bCMYK       = pHJComm->pJDispInfo->bCMYK;
				pJShare2Isp->colorSpace  = pJFrmComp->decColorSpace;

				//---------------------------------
			    // fire isp data.
				result = Jpg_Ext_Link_Ctrl(JEL_CTRL_ISP_IMG_PROC, 0, (void*)&pHJCodec->jShare2Isp);
				if( result != JPG_ERR_OK )
				{
				   jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);
				   break;
                }

				actBsSize			   = pHJCodec->sysBsBufSize;
				//printf("actBsSize = %d\n",actBsSize);  This is important.

				while( actBsSize > 0 && bEnd == false )
				{
					jpg_sleep(1);
					result = _MJPG_Set_CodedData(pHJCodec, &actBsSize, &bEnd);
					if( result != JPG_ERR_OK )
					{
					    jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg Err !!");
					    bSkip = true;
					    break;
					}
				}

				if( bEnd == true )		pHJCodec->bLastSection = true;

				if( pHJCodec->bLastSection == true )
				{
					result = _MJPG_HW_Wait_Idle(JPG_TIMEOUT_COUNT);
					if( result != JPG_ERR_OK )
					{
					   jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg Err !!");
					   //break;
					}
				}

				result = Jpg_Ext_Link_Ctrl(JEL_CTRL_ISP_WAIT_IDLE, 0, 0);
				if( result != JPG_ERR_OK )
				{
				    jpg_msg_ex(JPG_MSG_TYPE_ERR, " Isp err !! ", result);
				    JPG_LogReg(true);
				    //break;
				}
			
		}
		
    }while(0);

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_DEC_MJPG);
    return result;
}

static JPG_ERR
mjpg_dec_ctrl(
    JPG_CODEC_HANDLE    *pHJCodec,
    uint32_t            cmd,
    uint32_t            *value,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_DEC_MJPG, "0x%x, 0x%x\n", pHJCodec, extraData);

    switch( cmd )
    {
        case JCODEC_CMD_WAIT_IDLE:
            result = _MJPG_HW_Wait_Idle(JPG_TIMEOUT_COUNT);
            break;

        default:
            result = JPG_ERR_NO_IMPLEMENT;
            break;
    }

    if( result != JPG_ERR_OK &&
        result != JPG_ERR_NO_IMPLEMENT )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_DEC_MJPG);
    return result;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
JPG_CODEC_DESC JPG_CODEC_DESC_dec_mjpg_desc =
{
    "motion jpg dec",       // const char              *codecName;
    NULL,                   // struct JPG_CODEC_DESC_TAG  *next;
    JPG_CODEC_DEC_MJPG,     // const JPG_CODEC_TYPE       id;
    mjpg_dec_init,          // JPG_ERR (*init)(struct JPG_CODEC_HANDLE_TAG, void *extraData);
    mjpg_dec_deInit,        // JPG_ERR (*deinit)(struct JPG_CODEC_HANDLE_TAG, void *extraData);
    mjpg_dec_setup,         // JPG_ERR (*setup)(struct JPG_CODEC_HANDLE_TAG, void *extraData);
    mjpg_dec_fire,          // JPG_ERR (*fire)(struct JPG_CODEC_HANDLE_TAG, void *extraData);
    mjpg_dec_ctrl,          // JPG_ERR (*control)(struct JPG_CODEC_HANDLE_TAG, uint32_t cmd, uint32_t *value, void *extraData);
};
#else

JPG_CODEC_DESC JPG_CODEC_DESC_dec_mjpg_desc = {0};
#endif

/*
 * Copyright (c) 2012 ITE Technology Corp. All Rights Reserved.
 */
/** @file mmp_encoder.c
 *
 * @author
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "encoder/mmp_encoder.h"
#include "encoder/encoder_error.h"
#include "encoder/encoder_memory.h"
#include "encoder/encoder_hardware.h"
#include "encoder/encoder_register.h"
#include "encoder/config.h"
#include "encoder/blackbird_ITE.h"
//#include "encoder/blackbird_ITE_0418.h"
#include "encoder/encoder_bitstream.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct AVC_FRAME_RATE_PARA_TAG
{
    MMP_UINT32 FrameRateRes;
    MMP_UINT32 FrameRateDiv_Minus1;
} AVC_FRAME_RATE_PARA;

//=============================================================================
//                              Global Data Definition
//=============================================================================
static void*    gpEncoderSemaphore  = MMP_NULL;
static MMP_BOOL gbInstanceUsed[MAX_INSTANCE_NUM];
static MMP_BOOL gbAddParaSetHdr[MAX_INSTANCE_NUM] = {0};
static MMP_UINT32 gPreStreamBufSelect = 0;
static AVC_FRAME_RATE gInitframeRate[MAX_INSTANCE_NUM];
static AVC_FRAME_RATE_PARA gFrameRate[16] =
{
    {    0,    0}, // 0: forbidden
    {   25,    0}, // 1: 25 fps
    {   50,    0}, // 2: 50 fps
    {   30,    0}, // 3: 30 fps
    {   60,    0}, // 4: 60 fps
    {30000, 1000}, // 5: 29.97 fps   
    {60000, 1000}, // 6: 59.94 fps
    {24000, 1000}, // 7: 23.976 fps
    {   24,    0}, // 8: 24 fps
    {    0,    0}, // 9: forbidden
    {    0,    0}, // 10: forbidden
    {    0,    0}, // 11: forbidden
    {    0,    0}, // 12: forbidden
    {    0,    0}, // 13: forbidden
    {   30,    0}, // 14: 30 fps
    {   60,    0}, // 15: 60 fps     
};

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================

MMP_RESULT
mmpAVCEncodeInit(
    void)
{
    VIDEO_ERROR_CODE    error       = VIDEO_ERROR_SUCCESS;
    MMP_UINT32          size        = 0;
    MMP_UINT32          i           = 0;
    MMP_UINT8*          ptmpBufAdr  = MMP_NULL;

    LOG_ENTER "[mmpAVCEncodeInit]\n" LOG_END

    if (!gpEncoderSemaphore)
    {
        //gpEncoderSemaphore = SYS_CreateSemaphore(1, "MMP_AVCENCODER");        
        gpEncoderSemaphore = 0x13579;
        if (!gpEncoderSemaphore)
        {
            error = ERROR_ENCODER_SEMAPHORE_CREATE;
            goto RETURN_ERROR;
        }
    }
    else
    {
        error = ERROR_ENCODER_INIT_CONFLICT;
        goto RETURN_ERROR;
    }

    /////////////////////////////////////////////
    // step 1. create global buffer
    /////////////////////////////////////////////

    //create memory pool
    if ((error=encoder_memory_Initialize()) != VIDEO_ERROR_SUCCESS)
    {
        LOG_INFO " create memory pool fail\n" LOG_END
        goto RETURN_ERROR;
    }

    // get firmware code buffer
    if ((error=encoder_memory_GetCodeBuffer(&ptmpBufAdr)) == VIDEO_ERROR_SUCCESS)
    {
        // download firmware
        BitLoadFirmware(ptmpBufAdr, bit_code, (sizeof(bit_code)/sizeof(bit_code[0])));

        encoder_hardware_SetBufAddr_Reg(BIT_CODE_BUF_ADDR, ptmpBufAdr);
        LOG_INFO " cod buf 0x%08X\n", ptmpBufAdr LOG_END
    }
    else
        goto RETURN_ERROR;

    // get parameter buffer
    if ((error=encoder_memory_GetParaBuffer(&ptmpBufAdr)) == VIDEO_ERROR_SUCCESS)
    {
        encoder_hardware_SetBufAddr_Reg(BIT_PARA_BUF_ADDR, ptmpBufAdr);
        LOG_INFO " par buf 0x%08X\n", ptmpBufAdr LOG_END
    }
    else
        goto RETURN_ERROR;

    // get temp buffer
    if ((error=encoder_memory_GetTempBuffer(&ptmpBufAdr)) == VIDEO_ERROR_SUCCESS)
    {
        encoder_hardware_SetBufAddr_Reg(BIT_TEMP_BUF_ADDR, ptmpBufAdr);
        LOG_INFO " tmp buf 0x%08X\n", ptmpBufAdr LOG_END
    }
    else
        goto RETURN_ERROR;

    ///////////////////////////////////////////////////////////////////
    // step 2. set init ctrl
    ///////////////////////////////////////////////////////////////////
    VPU_WriteRegister(BIT_BIT_STREAM_CTRL, BS_CTRL_BUF_PIC_RESET | BS_CTRL_DYN_BUF_ENABLE | BS_CTRL_BIG_ENDIAN);
    VPU_WriteRegister(BIT_BIT_STREAM_PARAM, 0);

#ifdef TILED_MODE_MAPPING
    VPU_WriteRegister(BIT_FRAME_MEM_CTRL, FRM_CTRL_TILED_MAP_EN | FRM_CTRL_BURST_WR_BACK_EN | FRM_CTRL_CBCR_INTERLEAVE | FRM_CTRL_BIG_ENDIAN);
#else
    VPU_WriteRegister(BIT_FRAME_MEM_CTRL, FRM_CTRL_BURST_WR_BACK_EN | FRM_CTRL_CBCR_INTERLEAVE | FRM_CTRL_BIG_ENDIAN);
#endif

    VPU_WriteRegister(BIT_AXI_SRAM_USE, AXI_SEC_EN);
    // TODO : interrupt
    //VPU_WriteRegister(BIT_INT_ENABLE, INT_BIT_BIT_BUF_FULL | INT_BIT_PIC_RUN);
    VPU_WriteRegister(BIT_INT_ENABLE,   0);
    VPU_WriteRegister(BIT_INT_CLEAR,    INT_CLEAR);
    VPU_WriteRegister(BIT_BUSY_FLAG,    1); // ?
    VPU_WriteRegister(BIT_CODE_RUN,     1);

    VPU_WriteRegister(BIT_RD_PTR, 0);
    VPU_WriteRegister(BIT_WR_PTR, 0);

    if (!WaitIdle(100))
    {
        error = ERROR_ENCODER_TIME_OUT;
        goto RETURN_ERROR;
    }

    for (i=0; i<MAX_INSTANCE_NUM; i++)
        gbInstanceUsed[i] = MMP_FALSE;

RETURN_ERROR:
    if (error)
        LOG_ERROR " mmpAVCEncodeInit err 0x%08X \n", error LOG_END

    return (MMP_RESULT)error;
}

MMP_RESULT
mmpAVCEncodeOpen(
    AVC_ENCODER* ptEncoder)
{
    VIDEO_ERROR_CODE error = VIDEO_ERROR_SUCCESS;

    MMP_UINT32        bufferIndex = 0;
    MMP_UINT32        bufferCount = 0;
    MMP_UINT8*        ptmpBufAdr = MMP_NULL;
    MMP_UINT32        size = 0;
    MMP_UINT32        i = 0;
    MMP_UINT32        zeroByte = 0x0;
    MMP_UINT32        data;
    MMP_UINT32        SPSLen, PPSLen, offset;
    VIDEO_FRAME_BUFFER frameBuf = {0};
    VIDEO_FRAME_BUFFER tiledfrmBuf = {0};

    LOG_ENTER "[mmpAVCEncodeOpen] inst=%d, width=%d, height=%d \n", ptEncoder->InstCreated, ptEncoder->frameWidth, ptEncoder->frameHeight LOG_END
    
    if (ptEncoder->InstCreated)
    {
        error = ERROR_ENCODER_INSTANCE_CREATED;
        goto RETURN_ERROR;
    }

    // find out available instance
    for (i=0; i<MAX_INSTANCE_NUM; i++)
        if (!gbInstanceUsed[i])
            break;

    if (i==MAX_INSTANCE_NUM)
    {
        error = ERROR_ENCODER_NO_INSTANCE;
        goto RETURN_ERROR;
    }    

    if ((i==0 && (ptEncoder->frameWidth > MAX_FRAME_WIDTH || ptEncoder->frameHeight > MAX_INST0_FRAME_HEIGHT)) ||
        (i==1 && (ptEncoder->frameWidth > MAX_FRAME_WIDTH || ptEncoder->frameHeight > MAX_INST1_FRAME_HEIGHT)) ||
        (i==2 && (ptEncoder->frameWidth > MAX_FRAME_WIDTH || ptEncoder->frameHeight > MAX_INST2_FRAME_HEIGHT)))
    {
        error = ERROR_ENCODER_FRAME_WIDTH_HEIGHT;
        goto RETURN_ERROR;
    }
    
    gbInstanceUsed[i] = MMP_TRUE;
    ptEncoder->InstanceNum = i;
    
#if !defined(MULTIPLE_INSTANCES)
    ptEncoder->streamBufCount = 7;
    ptEncoder->streamBufSize = STREAM_BUF_SIZE;
#else
#if defined(DOOR_PHONE)
    ptEncoder->streamBufCount = 7;
#else
    ptEncoder->streamBufCount = 3;
#endif 
    if (i == 0)
    	ptEncoder->streamBufSize = INST_0_STREAM_BUF_SIZE;
    else if (i == 1)
    	ptEncoder->streamBufSize = INST_1_STREAM_BUF_SIZE;
    else
    	ptEncoder->streamBufSize = INST_2_STREAM_BUF_SIZE;
#endif
        
#if defined(FULL_HD)    
    ptEncoder->sourceBufCount = 5;
#else
    ptEncoder->sourceBufCount = 3;    
#endif    
    ptEncoder->framePitchY = TILED_FRAME_PITCH;
    
    /////////////////////////////////////////////
    // step 1. create buffer
    /////////////////////////////////////////////
    // get work buffer
    if ((error=encoder_memory_GetWorkBuffer(&ptmpBufAdr, ptEncoder->InstanceNum)) == VIDEO_ERROR_SUCCESS)
    {
        ptEncoder->pWorkBufAddr = ptmpBufAdr;
    }
    else
        goto RETURN_ERROR;

    // get bitstream buffer
#if !defined(MULTIPLE_INSTANCES)
    bufferCount = 7;
#else
#if defined(DOOR_PHONE)
    bufferCount = 7;
#else
    bufferCount = 3;
#endif    
#endif
    
    for (bufferIndex=0; bufferIndex<bufferCount; bufferIndex++)
    {
        if ((error=encoder_memory_GetStreamBuffer(&ptmpBufAdr, bufferIndex, ptEncoder->InstanceNum)) == VIDEO_ERROR_SUCCESS)
        {
            ptEncoder->pStreamBufAdr[bufferIndex] = ptmpBufAdr;
        }
        else
            goto RETURN_ERROR;
    }

    // get reconstructed/reference & source buffer
    bufferCount = 2;

    for (bufferIndex=0; bufferIndex<bufferCount; bufferIndex++)
    {
        if ((error=encoder_memory_GetReconBuffer(&frameBuf, bufferIndex, ptEncoder->InstanceNum)) == VIDEO_ERROR_SUCCESS)
        {
            ptEncoder->pReconBufAddrY[bufferIndex] = frameBuf.pAddress_Y;
            ptEncoder->pReconBufAddrU[bufferIndex] = frameBuf.pAddress_U;
            ptEncoder->pReconBufAddrV[bufferIndex] = frameBuf.pAddress_V;
        }
        else
            goto RETURN_ERROR;
    }

    // get source buffer
#if defined(FULL_HD)    
    bufferCount = 5;
#else        
    bufferCount = 3;
#endif    

    for (bufferIndex=0; bufferIndex<bufferCount; bufferIndex++)
    {
        if ((error=encoder_memory_GetSourceBuffer(&frameBuf, &tiledfrmBuf, bufferIndex)) == VIDEO_ERROR_SUCCESS)
        {
            ptEncoder->pSourceBufAdrY[bufferIndex] = frameBuf.pAddress_Y;
            ptEncoder->pSourceBufAdrU[bufferIndex] = frameBuf.pAddress_U;
            ptEncoder->pSourceBufAdrV[bufferIndex] = frameBuf.pAddress_V;

            //clear buffer            
            if (ptEncoder->InstanceNum == 0)
            {
                memset(frameBuf.pAddress_Y, 0, TILED_FRAME_PITCH*MAX_LUM_HEIGHT);
                memset(frameBuf.pAddress_U, 0x80, TILED_FRAME_PITCH*MAX_LUM_HEIGHT/2);                       
            } 
            else if (ptEncoder->InstanceNum == 1)
            {
            	memset(frameBuf.pAddress_Y, 0, TILED_FRAME_PITCH*INST_1_LUM_HEIGHT);
                memset(frameBuf.pAddress_U, 0x80, TILED_FRAME_PITCH*INST_1_LUM_HEIGHT/2);
            }                                   
            
            ptEncoder->pTiledSrcBufAdrY[bufferIndex] = tiledfrmBuf.pAddress_Y;
            ptEncoder->pTiledSrcBufAdrU[bufferIndex] = tiledfrmBuf.pAddress_U;
            ptEncoder->pTiledSrcBufAdrV[bufferIndex] = tiledfrmBuf.pAddress_V;
        }
        else
            goto RETURN_ERROR;
    }

    // get subimage buffer
    bufferCount = 2;

    for (bufferIndex=0; bufferIndex<bufferCount; bufferIndex++)
    {
        if ((error=encoder_memory_GetSubImgBuffer(&ptmpBufAdr, bufferIndex, ptEncoder->InstanceNum)) == VIDEO_ERROR_SUCCESS)
        {
            ptEncoder->pSubImgBufAdr[bufferIndex] = ptmpBufAdr;
        }
        else
            goto RETURN_ERROR;
    }

    // get SPS/PPS header buffer
    if ((error=encoder_memory_GetPSBuffer(&ptmpBufAdr, ptEncoder->InstanceNum)) == VIDEO_ERROR_SUCCESS)
    {
        ptEncoder->pHdrBufAddr[0] = ptmpBufAdr;
        ptEncoder->pHdrBufAddr[1] = (MMP_UINT8 *) ((MMP_UINT32)ptmpBufAdr + 10*1024);
    }
    else
        goto RETURN_ERROR;

    LOG_INFO " wbuf 0x%08X hbuf 0x%08X bbuf 0x%08X\n", ptEncoder->pWorkBufAddr, ptEncoder->pHdrBufAddr[0], ptEncoder->pStreamBufAdr[0] LOG_END
    LOG_INFO " rbuf 0x%08X sbuf 0x%08X ibuf 0x%08X\n", ptEncoder->pReconBufAddrY[0], ptEncoder->pSourceBufAdrY[0], ptEncoder->pSubImgBufAdr[0] LOG_END
    
    // create instance success
    ptEncoder->InstCreated = MMP_TRUE;   

RETURN_ERROR:
    if (error)
        LOG_ERROR " mmpAVCEncodeOpen(%d) err 0x%08X \n", ptEncoder->InstanceNum, error LOG_END

    return (MMP_RESULT)error;
}

MMP_RESULT
mmpAVCEncodeCreateHdr(
    AVC_ENCODER* ptEncoder)
{
    VIDEO_ERROR_CODE error = VIDEO_ERROR_SUCCESS;

    MMP_UINT32        bufferIndex = 0;
    MMP_UINT32        bufferCount = 0;
    MMP_UINT8*        ptmpBufAdr = MMP_NULL;
    MMP_UINT32        size = 0;
    MMP_UINT32        i = 0;
    MMP_UINT32        zeroByte = 0x0;
    MMP_UINT32        data;
    MMP_UINT32        SPSLen, PPSLen, offset;
    VIDEO_FRAME_BUFFER frameBuf = {0};
    VIDEO_FRAME_BUFFER tiledfrmBuf = {0};

    LOG_ENTER "[mmpAVCEncodeCreateHdr] inst=%d, width=%d, height=%d \n", ptEncoder->InstCreated, ptEncoder->frameWidth, ptEncoder->frameHeight LOG_END
    
    printf("[mmpAVCEncodeCreateHdr] inst=%d, width=%d, height=%d \n", ptEncoder->InstCreated, ptEncoder->frameWidth, ptEncoder->frameHeight);
    
    if (!ptEncoder->InstCreated)
    {
        error = ERROR_ENCODER_INSTANCE_CREATED;
        goto RETURN_ERROR;
    }
    
    ptEncoder->streamBufSelect = 0;
    
    ///////////////////////////////////////////////////////////////////
    // step 1. seq_init
    ///////////////////////////////////////////////////////////////////
    encoder_hardware_SetBufAddr_Reg(CMD_ENC_SEQ_BB_START, ptEncoder->pStreamBufAdr[0]);
    VPU_WriteRegister(CMD_ENC_SEQ_BB_SIZE, ptEncoder->streamBufSize / 1024);

    VPU_WriteRegister(CMD_ENC_SEQ_COD_STD, 0); // H.264 standard

    data = ((((ptEncoder->frameWidth+15)>>4)<<4) << 16) | (((ptEncoder->frameHeight+15)>>4)<<4);
    VPU_WriteRegister(CMD_ENC_SEQ_SRC_SIZE, data);        
        
    if (ptEncoder->frameRate == AVC_FRAME_RATE_29_97HZ ||
    	ptEncoder->frameRate == AVC_FRAME_RATE_59_94HZ ||
    	ptEncoder->frameRate == AVC_FRAME_RATE_23_97HZ)
    {    	
        gFrameRate[ptEncoder->frameRate].FrameRateRes = ptEncoder->EnFrameRate *1000;
    } else {    	
    	gFrameRate[ptEncoder->frameRate].FrameRateRes = ptEncoder->EnFrameRate;
    }
    
    data =  (gFrameRate[ptEncoder->frameRate].FrameRateDiv_Minus1) << 16 | (gFrameRate[ptEncoder->frameRate].FrameRateRes & 0xFFFF);
       
    VPU_WriteRegister(CMD_ENC_SEQ_SRC_F_RATE, data);

    gInitframeRate[ptEncoder->InstanceNum] = ptEncoder->frameRate;
    
    data = (ptEncoder->deblkFilterOffsetBeta & 15) << 12 |
           (ptEncoder->deblkFilterOffsetAlpha & 15) << 8 |
           ptEncoder->disableDeblk << 6 |
           ptEncoder->constrainedIntraPredFlag << 5 |
           (ptEncoder->chromaQpOffset & 31);
    VPU_WriteRegister(CMD_ENC_SEQ_264_PARA, data);

    // Now always set single slice
    VPU_WriteRegister(CMD_ENC_SEQ_SLICE_MODE, 0);
    //data = 80 << 2 | 3;
    //VPU_WriteRegister(CMD_ENC_SEQ_SLICE_MODE, data);
    
    if (ptEncoder->gopSize == 0)
    {
        ptEncoder->gopSize = 30;
    }
    VPU_WriteRegister(CMD_ENC_SEQ_GOP_NUM, ptEncoder->gopSize);

    if (ptEncoder->bitRate) { // rate control enabled
        data = (!ptEncoder->enableAutoSkip) << 31 |
               (ptEncoder->initialDelay & 0x7FFF) << 16 |
               (ptEncoder->bitRate & 0x7FFF) << 1 | 1;
        VPU_WriteRegister(CMD_ENC_SEQ_RC_PARA, data);
    } else {
        VPU_WriteRegister(CMD_ENC_SEQ_RC_PARA, 0);
    }

    VPU_WriteRegister(CMD_ENC_SEQ_RC_BUF_SIZE, ptEncoder->vbvBufferSize);
    VPU_WriteRegister(CMD_ENC_SEQ_INTRA_REFRESH, ptEncoder->intraRefresh);

    data = 0;
     if (ptEncoder->rcIntraQp > 0) {
        data = SEQ_INIT_RC_CONST_INTRAQP_EN;
        VPU_WriteRegister(CMD_ENC_SEQ_INTRA_QP, ptEncoder->rcIntraQp);
    } else {
        data = 0;
        VPU_WriteRegister(CMD_ENC_SEQ_INTRA_QP, 0);
    }

    if (ptEncoder->userQpMax) {
        data |= SEQ_INIT_RC_QP_MAXSET_EN;
        VPU_WriteRegister(CMD_ENC_SEQ_RC_QP_MAX, ptEncoder->userQpMax);
    } else {
        VPU_WriteRegister(CMD_ENC_SEQ_RC_QP_MAX, 0);
    }

    if (ptEncoder->userGamma) {
        data |= SEQ_INIT_RC_GAMMASET_EN;
        VPU_WriteRegister(CMD_ENC_SEQ_RC_GAMMA, ptEncoder->userGamma);
    } else {
        VPU_WriteRegister(CMD_ENC_SEQ_RC_GAMMA, 0);
    }

#ifdef USING_HW_SPS_PPS
    data |= SEQ_INIT_AUD_ENABLE;
#endif    
    VPU_WriteRegister(CMD_ENC_SEQ_OPTION, data);

    VPU_WriteRegister(CMD_ENC_SEQ_RC_INTERVAL_MODE, (ptEncoder->MbInterval<<2) | ptEncoder->RcIntervalMode);
    VPU_WriteRegister(CMD_ENC_SEQ_INTRA_WEIGHT, ptEncoder->IntraCostWeight);

    VPU_WriteRegister(CMD_ENC_SEQ_ME_OPTION, (ptEncoder->MEUseZeroPmv << 2) | ptEncoder->MESearchRange);

    encoder_hardware_SetBufAddr_Reg(BIT_WORK_BUF_ADDR, ptEncoder->pWorkBufAddr);

    VPU_WriteRegister(BIT_RD_PTR, 0);
    VPU_WriteRegister(BIT_WR_PTR, 0);

#ifdef TILED_MODE_MAPPING
    VPU_WriteRegister(BIT_FRAME_MEM_CTRL, FRM_CTRL_TILED_MAP_EN | FRM_CTRL_BURST_WR_BACK_EN | FRM_CTRL_CBCR_INTERLEAVE | FRM_CTRL_BIG_ENDIAN);
#else
    VPU_WriteRegister(BIT_FRAME_MEM_CTRL, FRM_CTRL_BURST_WR_BACK_EN | FRM_CTRL_CBCR_INTERLEAVE | FRM_CTRL_BIG_ENDIAN);
#endif

    // issue command
    IssueCommand(ptEncoder->InstanceNum, RUN_CMD_SEQ_INIT);

    if (!WaitIdle(100))
    {
        error = ERROR_ENCODER_TIME_OUT;
        goto RETURN_ERROR;
    }

    if (VPU_ReadRegister(RET_ENC_SEQ_ENC_SUCCESS) & (1<<31))
    {
        error = ERROR_ENCODER_MEMORY_ACCESS_VIOLATION;
        goto RETURN_ERROR;
    }

    if (VPU_ReadRegister(RET_ENC_SEQ_ENC_SUCCESS) == 0)
    {
        error = ERROR_ENCODER_SEQ_INIT_FAILURE;
        goto RETURN_ERROR;
    }

    ///////////////////////////////////////////////////////////////////
    // step 2. set GDI parameters
    ///////////////////////////////////////////////////////////////////
    encoder_hardware_SetGDI_Reg(encoder_memory_GetTiledBaseAddr());

    ///////////////////////////////////////////////////////////////////
    // step 3. set frame buffer control
    ///////////////////////////////////////////////////////////////////
    VPU_WriteRegister(CMD_SET_FRAME_BUF_NUM, 2);
    VPU_WriteRegister(CMD_SET_FRAME_BUF_STRIDE, ptEncoder->framePitchY);
    VPU_WriteRegister(CMD_SET_FRAME_AXI_BIT_ADDR, 0xF8003DE0);
    VPU_WriteRegister(CMD_SET_FRAME_AXI_IPACDC_ADDR, 0xF8007BC0);
    VPU_WriteRegister(CMD_SET_FRAME_AXI_DBKY_ADDR, 0xF8000000);
    VPU_WriteRegister(CMD_SET_FRAME_AXI_DBKC_ADDR, 0xF8001EF0);
    VPU_WriteRegister(CMD_SET_FRAME_AXI_OVL_ADDR, 0xF8018000);
    VPU_WriteRegister(CMD_SET_FRAME_AXI_BTP_ADDR, 0xF801D000);

//#ifdef TILED_MODE_MAPPING
//
//#ifdef MULTIPLE_INSTANCES
//
//VPU_WriteRegister(CMD_SET_FRAME_CACHE_CONFIG, 0x000007E4);
//
//#else

//#ifdef ISP_ONFLY_CACHE_ON
//    if (ptEncoder->bISPOnFly)
//        VPU_WriteRegister(CMD_SET_FRAME_CACHE_CONFIG, 0x000007E4);
//    else	
//#endif  	
//        VPU_WriteRegister(CMD_SET_FRAME_CACHE_CONFIG, 0x000007E6); // magic number ? should try real condition   
//        
//#endif        
//#else
//    VPU_WriteRegister(CMD_SET_FRAME_CACHE_CONFIG, 0x000007E0);
//    
//#endif

    VPU_WriteRegister(CMD_SET_FRAME_CACHE_CONFIG, 0x000007E4);
    
    encoder_hardware_SetBufAddr_Reg(CMD_SET_FRAME_SUBSAMP_A, ptEncoder->pSubImgBufAdr[0]);
    encoder_hardware_SetBufAddr_Reg(CMD_SET_FRAME_SUBSAMP_B, ptEncoder->pSubImgBufAdr[1]);
    encoder_hardware_SetBufAddr_Reg(BIT_WORK_BUF_ADDR, ptEncoder->pWorkBufAddr);
    // issue command
    IssueCommand(ptEncoder->InstanceNum, RUN_CMD_SET_FRM_BUF);

    if (!WaitIdle(100))
    {
        error = ERROR_ENCODER_TIME_OUT;
        goto RETURN_ERROR;
    }

    if (VPU_ReadRegister(RET_SET_FRAME_SUCCESS) & (1<<31))
    {
        error = ERROR_ENCODER_MEMORY_ACCESS_VIOLATION;
        goto RETURN_ERROR;
    }

    if (VPU_ReadRegister(RET_SET_FRAME_SUCCESS) == 0)
    {
        error = ERROR_ENCODER_SET_FRAME_FAILURE;
        goto RETURN_ERROR;
    }
	
#ifdef USING_HW_SPS_PPS
    ///////////////////////////////////////////////////////////////////
    // step 4. create SPS/PPS data
    ///////////////////////////////////////////////////////////////////
    ptmpBufAdr = ptEncoder->pHdrBufAddr[0];

    // clear header buffer
    for (i = 0; i < 32 ; i += 4)
        HOST_WriteBlockMemory((MMP_UINT32) (ptmpBufAdr+i), (MMP_UINT32) &zeroByte, 4);

    // calculate crop size
    ptEncoder->frameCropLeft = 0;    
    ptEncoder->frameCropRight = (((ptEncoder->frameWidth+15)>>4)<<4) - ptEncoder->frameWidth;
    ptEncoder->frameCropTop = 0;
    ptEncoder->frameCropBottom = (((ptEncoder->frameHeight+15)>>4)<<4) - ptEncoder->frameHeight;
        
    // encode SPS header
    data = ENC_SPS_HEADER;
    if (ptEncoder->frameCropLeft > 0 || ptEncoder->frameCropRight > 0 ||
        ptEncoder->frameCropTop > 0 || ptEncoder->frameCropBottom > 0)
        data |= SPS_FRAME_CROPPING_FLAG;

    VPU_WriteRegister(CMD_ENC_HEADER_CODE, data);

    data = ptEncoder->frameCropLeft << 16 | ptEncoder->frameCropRight;
    VPU_WriteRegister(CMD_ENC_HEADER_FRAME_CROP_H, data);

    data = ptEncoder->frameCropTop << 16 | ptEncoder->frameCropBottom;
    VPU_WriteRegister(CMD_ENC_HEADER_FRAME_CROP_V, data);

    encoder_hardware_SetBufAddr_Reg(CMD_ENC_HEADER_BB_START, ptmpBufAdr);
    VPU_WriteRegister(CMD_ENC_HEADER_BB_SIZE, SPS_PPS_HEADER_BUF_SIZE / 1024);

    VPU_WriteRegister(BIT_RD_PTR, (MMP_UINT32) ptmpBufAdr);
    VPU_WriteRegister(BIT_WR_PTR, (MMP_UINT32) ptmpBufAdr);

    IssueCommand(ptEncoder->InstanceNum, RUN_CMD_ENOCDE_HEADER);

    if (!WaitIdle(100))
    {
        error = ERROR_ENCODER_TIME_OUT;
        goto RETURN_ERROR;
    }

    // check SPS size and alignment to 8 bytes boundary
    SPSLen = VPU_ReadRegister(BIT_WR_PTR) -  VPU_ReadRegister(BIT_RD_PTR);

    if (SPSLen & 0x7)
    {
        offset = 8 - (SPSLen & 0x7);
        SPSLen += offset;
        ptmpBufAdr = (MMP_UINT8 *)((MMP_UINT32) ptmpBufAdr + SPSLen);
    }

    // encode PPS header
    data = ENC_PPS_HEADER;
    VPU_WriteRegister(CMD_ENC_HEADER_CODE, data);

    encoder_hardware_SetBufAddr_Reg(CMD_ENC_HEADER_BB_START, ptmpBufAdr);
    VPU_WriteRegister(CMD_ENC_HEADER_BB_SIZE, (SPS_PPS_HEADER_BUF_SIZE - SPSLen) / 1024);

    VPU_WriteRegister(BIT_RD_PTR, (MMP_UINT32) ptmpBufAdr);
    VPU_WriteRegister(BIT_WR_PTR, (MMP_UINT32) ptmpBufAdr);

    IssueCommand(ptEncoder->InstanceNum, RUN_CMD_ENOCDE_HEADER);

    if (!WaitIdle(100))
    {
        error = ERROR_ENCODER_TIME_OUT;
        goto RETURN_ERROR;
    }

    PPSLen = VPU_ReadRegister(BIT_WR_PTR) - VPU_ReadRegister(BIT_RD_PTR);

    if (PPSLen & 0x7)
    {
        offset = 8 - (PPSLen & 0x7);
        PPSLen += offset;
    }

    ptEncoder->ParaSetHdrSize[0] = SPSLen+PPSLen;
    ptEncoder->ParaSetHdrSize[1] = 0;
#else
    if (ptEncoder->bMP4format == MMP_TRUE)
    {
    	encoder_CreateMP4Config(ptEncoder, (MMP_UINT32 *)ptEncoder->pHdrBufAddr[0], 10*1024);
    	encoder_CreateMP4SEI(ptEncoder, (MMP_UINT32 *)ptEncoder->pHdrBufAddr[1], 10*1024);
    }    
    else 
    {
#ifndef SEPARATE_SPS_PPS    	
        // Create AUD, SPS, PPS, SEI NAL for I frame
        encoder_CreateIFrameHdr(ptEncoder, (MMP_UINT32 *)ptEncoder->pHdrBufAddr[0], 10*1024);
        // Create AUD, SEI for P frame
        encoder_CreatePFrameHdr(ptEncoder, (MMP_UINT32 *)ptEncoder->pHdrBufAddr[1], 10*1024);
#else        
        // Create AUD, SPS, PPS, SEI NAL for I frame
        encoder_CreateSPS(ptEncoder, (MMP_UINT32 *)ptEncoder->pHdrBufAddr[0], 10*1024);
        // Create AUD, SEI for P frame
        encoder_CreatePPS(ptEncoder, (MMP_UINT32 *)ptEncoder->pHdrBufAddr[1], 10*1024);
#endif        
    }
#endif    
    // create instance success    
    ptEncoder->framecount = 0;
    gPreStreamBufSelect = 0;

RETURN_ERROR:
    if (error)
        LOG_ERROR " mmpAVCEncodeCreateHdr(%d) err 0x%08X \n", ptEncoder->InstanceNum, error LOG_END

    return (MMP_RESULT)error;
}

MMP_RESULT
mmpAVCEncodeSetFrameRate(
    AVC_ENCODER* ptEncoder)
{
    VIDEO_ERROR_CODE error = VIDEO_ERROR_SUCCESS;
    MMP_UINT32 data;
    MMP_UINT32 i = 0;

    LOG_ENTER "[mmpAVCEncodeSetFrameRate] inst=%d, width=%d, height=%d \n", ptEncoder->InstCreated, ptEncoder->frameWidth, ptEncoder->frameHeight LOG_END
    
    if (!ptEncoder->InstCreated)
    {
        error = ERROR_ENCODER_INSTANCE_CREATED;
        goto RETURN_ERROR;
    }
    
    ///////////////////////////////////////////////////////////////////
    // change frame rate info
    ///////////////////////////////////////////////////////////////////
    if (gInitframeRate[ptEncoder->InstanceNum] != ptEncoder->frameRate)
    {    
        VPU_WriteRegister(CMD_ENC_SEQ_PARA_CHANGE_ENABLE, FRAME_RATE_CHANGE_ENABLE);
               
        data =  (gFrameRate[ptEncoder->frameRate].FrameRateDiv_Minus1) << 16 | gFrameRate[ptEncoder->frameRate].FrameRateRes;
            
        VPU_WriteRegister(CMD_ENC_SEQ_PARA_RC_FRAME_RATE, data);
            
        encoder_hardware_SetBufAddr_Reg(BIT_WORK_BUF_ADDR, ptEncoder->pWorkBufAddr);
            
        IssueCommand(ptEncoder->InstanceNum, RUN_CMD_PARAM_CHANGE);
        
        while (!WaitIdle(0))
        {
           i++;
           if (i > 1000)
           {
           	   error = ERROR_ENCODER_TIME_OUT;       	  
               goto RETURN_ERROR;
           }    
        }
    }
    
    if (ptEncoder->bMP4format == MMP_TRUE)
    {
    	encoder_CreateMP4Config(ptEncoder, (MMP_UINT32 *)ptEncoder->pHdrBufAddr[0], 10*1024);
    	encoder_CreateMP4SEI(ptEncoder, (MMP_UINT32 *)ptEncoder->pHdrBufAddr[1], 10*1024);
    }    
    else 
    {
        // Create AUD, SPS, PPS, SEI NAL for I frame
        encoder_CreateIFrameHdr(ptEncoder, (MMP_UINT32 *)ptEncoder->pHdrBufAddr[0], 10*1024);
        // Create AUD, SEI for P frame
        encoder_CreatePFrameHdr(ptEncoder, (MMP_UINT32 *)ptEncoder->pHdrBufAddr[1], 10*1024);
    }

RETURN_ERROR:
    if (error)
        LOG_ERROR " mmpAVCEncodeSetFrameRate(%d) err 0x%08X \n", ptEncoder->InstanceNum, error LOG_END

    return (MMP_RESULT)error;
}

MMP_RESULT
mmpAVCEncodeFire(
    AVC_ENCODER* ptEncoder)
{
    VIDEO_ERROR_CODE error = VIDEO_ERROR_SUCCESS;

    MMP_UINT32 streamBufSelect;
    MMP_UINT32 sourceBufSelect = ptEncoder->sourceBufSelect;
    MMP_UINT32 offset = 0;

    LOG_ENTER "[mmpAVCEncodeFire] strbuf=%d, srcbuf=%d streamAdr %x\n", streamBufSelect, sourceBufSelect, ptEncoder->pStreamBufAdr[streamBufSelect] LOG_END        

    gbAddParaSetHdr[ptEncoder->InstanceNum] = MMP_FALSE;

    // add SPS/PPS before I frame
    if (ptEncoder->gopSize == 0)
    {
        ptEncoder->gopSize = 30;
    }
    if ((ptEncoder->framecount++ % ptEncoder->gopSize) == 0)
    {
        offset = ptEncoder->ParaSetHdrSize[0];
        gbAddParaSetHdr[ptEncoder->InstanceNum] = MMP_TRUE;
    } else {  	
        offset = ptEncoder->ParaSetHdrSize[1];
    }
       
    if (ptEncoder->bMP4format == MMP_TRUE)    
        offset = 0;
 
 #ifdef SEPARATE_SPS_PPS   
    offset = 0;
 #endif    	
    //streamBufSelect = ptEncoder->streamBufSelect = (ptEncoder->streamBufSelect + 1)
    //                                             % (ptEncoder->streamBufCount);
    
    streamBufSelect = ptEncoder->streamBufSelect;
    
    // picture run command
    VPU_WriteRegister(CMD_ENC_PIC_QS, ptEncoder->PicQS);

    VPU_WriteRegister(CMD_ENC_PIC_SRC_INDEX, sourceBufSelect + 2);
    VPU_WriteRegister(CMD_ENC_PIC_SRC_STRIDE, ptEncoder->framePitchY);
    VPU_WriteRegister(CMD_ENC_PIC_SRC_ADDR_Y, (MMP_UINT32) ptEncoder->pTiledSrcBufAdrY[sourceBufSelect]);
    VPU_WriteRegister(CMD_ENC_PIC_SRC_ADDR_CB, (MMP_UINT32) ptEncoder->pTiledSrcBufAdrU[sourceBufSelect]);
    VPU_WriteRegister(CMD_ENC_PIC_SRC_ADDR_CR, (MMP_UINT32) ptEncoder->pTiledSrcBufAdrV[sourceBufSelect]);

    VPU_WriteRegister(CMD_ENC_PIC_ROT_MODE, 0);
    
    if (gbAddParaSetHdr[ptEncoder->InstanceNum] == MMP_TRUE)
        VPU_WriteRegister(CMD_ENC_PIC_OPTION, 0x2);
    else
        VPU_WriteRegister(CMD_ENC_PIC_OPTION, ptEncoder->forceIPicture << 1 | ptEncoder->skipEncode);
    
    encoder_hardware_SetBufAddr_Reg(CMD_ENC_PIC_BB_START, ptEncoder->pStreamBufAdr[streamBufSelect] + offset);
    VPU_WriteRegister(CMD_ENC_PIC_BB_SIZE, ptEncoder->streamBufSize / 1024); // size in KB
    VPU_WriteRegister(BIT_RD_PTR, (MMP_UINT32) ptEncoder->pStreamBufAdr[streamBufSelect] + offset);
    VPU_WriteRegister(BIT_WR_PTR, (MMP_UINT32) ptEncoder->pStreamBufAdr[streamBufSelect] + offset);
    //VPU_WriteRegister(CMD_ENC_PIC_SUB_FRAME_SYNC, val); ??
    encoder_hardware_SetBufAddr_Reg(BIT_WORK_BUF_ADDR, ptEncoder->pWorkBufAddr);

    IssueCommand(ptEncoder->InstanceNum, RUN_CMD_PIC_RUN);

    return (MMP_RESULT)error;
}

MMP_RESULT
mmpAVCEncodeWait(
    MMP_UINT32 timeout)
{
    VIDEO_ERROR_CODE error = VIDEO_ERROR_SUCCESS;

    LOG_ENTER "[mmpAVCEncodeWait]\n" LOG_END

    if (!WaitIdle(timeout))
    {
        error = ERROR_ENCODER_TIME_OUT;
        goto RETURN_ERROR;
    }

    if (VPU_ReadRegister(RET_ENC_PIC_SUCCESS) & (1<<31))
    {
        error = ERROR_ENCODER_MEMORY_ACCESS_VIOLATION;
        goto RETURN_ERROR;
    }

    if (VPU_ReadRegister(RET_ENC_PIC_SUCCESS) == 0)
    {
        error = ERROR_ENCODER_PIC_RUN_FAILURE;
    }

RETURN_ERROR :
    if (error)
        LOG_ERROR " mmpAVCEncodeWait err 0x%08X \n", error LOG_END

    return (MMP_RESULT)error;
}

MMP_RESULT
mmpAVCEncodeClose(
    AVC_ENCODER* ptEncoder)
{
    VIDEO_ERROR_CODE error = VIDEO_ERROR_SUCCESS;

    LOG_ENTER "[mmpAVCEncodeClose]\n" LOG_END

    if (!ptEncoder->InstCreated)
        return (MMP_RESULT)error;

    if (!WaitIdle(100))
    {
        error = ERROR_ENCODER_TIME_OUT;
        goto end;
    }
    
    IssueCommand(ptEncoder->InstanceNum, RUN_CMD_SEQ_END);

    if (!WaitIdle(100))
    {
        error = ERROR_ENCODER_TIME_OUT;
    }

    //ptEncoder->InstCreated = MMP_FALSE;
    //gbInstanceUsed[ptEncoder->InstanceNum] = MMP_FALSE;

end :
    if (error)
        LOG_ERROR " mmpAVCEncodeClose err 0x%08X \n", error LOG_END

    ptEncoder->framecount = 0;
    
    return (MMP_RESULT)error;
}

MMP_RESULT
mmpAVCEncodeGetStream(
    AVC_ENCODER* ptEncoder,
    MMP_UINT32*  StreamLen,
    MMP_BOOL*    bFrameEnd)
{
    MMP_UINT32 offset = 0;

    LOG_ENTER "[mmpAVCEncodeGetStream]\n" LOG_END

    if (gbAddParaSetHdr[ptEncoder->InstanceNum])
    {
        offset = ptEncoder->ParaSetHdrSize[0];
        ptEncoder->bIFrame = MMP_TRUE;
    } else {    	
        offset = ptEncoder->ParaSetHdrSize[1];
        ptEncoder->bIFrame = MMP_FALSE;
    }

#ifndef SEPARATE_SPS_PPS     
    if (ptEncoder->bMP4format != MMP_TRUE)
    {       
        // if I frame, copy SPS/PPS
        if (ptEncoder->bIFrame)
        {
#if defined(__OPENRTOS__)        
            PalMemcpy(ptEncoder->pStreamBufAdr[ptEncoder->streamBufSelect], ptEncoder->pHdrBufAddr[0], offset);
#else
            {
                MMP_UINT8 * streambuf;
            
                streambuf = (MMP_UINT8 *) PalHeapAlloc(0, offset);
            
                HOST_ReadBlockMemory( (MMP_UINT32) streambuf, (MMP_UINT32) ptEncoder->pHdrBufAddr, offset);
                HOST_WriteBlockMemory((MMP_UINT32) ptEncoder->pStreamBufAdr[ptEncoder->streamBufSelect], (MMP_UINT32) streambuf, offset);
            
                PalHeapFree(0, streambuf);
            }
#endif
        } else {
        	if (offset)
        	    PalMemcpy(ptEncoder->pStreamBufAdr[ptEncoder->streamBufSelect], ptEncoder->pHdrBufAddr[1], offset); 
        }
    } 
    else 
#endif    	
    {
    	offset = 0;
    }

    *StreamLen = (VPU_ReadRegister(BIT_WR_PTR) - VPU_ReadRegister(BIT_RD_PTR)) + offset;

    if (ptEncoder->bMP4format == MMP_TRUE)
    {
    	MMP_UINT8 *tmpBuf = ptEncoder->pStreamBufAdr[ptEncoder->streamBufSelect];
    	MMP_UINT32 tmpLen = *StreamLen - 4;
    	
    	*(tmpBuf)   = (tmpLen & 0xFF000000) >> 24;
    	*(tmpBuf+1) = (tmpLen & 0x00FF0000) >> 16;
    	*(tmpBuf+2) = (tmpLen & 0x0000FF00) >> 8;
    	*(tmpBuf+3) = (tmpLen & 0x000000FF);
    }
        
    LOG_INFO " Stream Len %d %x %x\n",*StreamLen, VPU_ReadRegister(BIT_RD_PTR), VPU_ReadRegister(BIT_WR_PTR) LOG_END

    if (WaitIdle(0))
    {
        *bFrameEnd = MMP_TRUE;
        
        ptEncoder->streamBufSelect = (ptEncoder->streamBufSelect + 1) % (ptEncoder->streamBufCount);
    } else
        *bFrameEnd = MMP_FALSE;
            
    return VIDEO_ERROR_SUCCESS;
}

MMP_RESULT
mmpAVCEncodeReset(
    AVC_ENCODER* ptEncoder)
{
    LOG_ENTER "[mmpAVCEncodeReset]\n" LOG_END

    // TODO
    return (MMP_RESULT) VIDEO_ERROR_SUCCESS;
}

MMP_RESULT
mmpAVCEncodeQuery(
    AVC_ENCODER_QUERY_TYPE  queryType,
    MMP_UINT32*             pValue)
{
    VIDEO_ERROR_CODE error = VIDEO_ERROR_SUCCESS;

    LOG_ENTER "[mmpAVCEncodeQuery] type=%d\n", queryType LOG_END

    switch (queryType)
    {
    case AVC_ENCODER_PICTURE_TYPE:
        *pValue = VPU_ReadRegister(RET_ENC_PIC_TYPE);
        break;

    case AVC_ENCODER_SLICE_NUM:
        *pValue = VPU_ReadRegister(RET_ENC_PIC_SLICE_NUM);
        break;

    case AVC_ENCODER_STREAM_BUF_OVERFLOW:
        *pValue = VPU_ReadRegister(RET_ENC_PIC_FLAG);
        break;

    case AVC_ENCODER_ENGINE_IDLE:
    	  *pValue = (MMP_UINT32) WaitIdle(0);    	
        break;

    case AVC_ENCODER_SEMAPHORE:
        *pValue = (MMP_UINT32) gpEncoderSemaphore;
        break;

    default:
        break;
    }

    return (MMP_RESULT)error;
}

MMP_BOOL
mmpAVCEncodeIsIdle(
    void)
{ 
    LOG_ENTER "[mmpAVCEncodeIsIdle]\n" LOG_END

    if (WaitIdle(0))
        return MMP_TRUE;
    else
    	  return MMP_FALSE;
}

MMP_RESULT
mmpAVCEncodeEnableInterrupt(
    ITHIntrHandler  handler)
{
    VIDEO_ERROR_CODE error = VIDEO_ERROR_SUCCESS;

    /** register interrupt handler to interrupt mgr */
    ithIntrRegisterHandlerIrq(ITH_INTR_CODA, handler, MMP_NULL);
    ithIntrSetTriggerModeIrq(ITH_INTR_CODA, ITH_INTR_EDGE);
    ithIntrSetTriggerLevelIrq(ITH_INTR_CODA, ITH_INTR_HIGH_RISING);
    ithIntrEnableIrq(ITH_INTR_CODA);
    /** enable encoder interrupt */
    VPU_WriteRegister(BIT_INT_ENABLE, INT_BIT_PIC_RUN | INT_BIT_BIT_BUF_FULL);
    VPU_WriteRegister(BIT_INT_CLEAR, INT_CLEAR);

    return (MMP_RESULT)error;
}

MMP_RESULT
mmpAVCEncodeDisableInterrupt(
    void)
{
    VIDEO_ERROR_CODE error = VIDEO_ERROR_SUCCESS;

    //ithIntrDisableIrq(ITH_INTR_SD);
    VPU_WriteRegister(BIT_INT_ENABLE, 0);

    return (MMP_RESULT)error;
}

void
mmpAVCEncodeClearInterrupt(
    void)
{       
    VPU_WriteRegister(BIT_INT_REASON, 0);
    VPU_WriteRegister(BIT_INT_CLEAR, 1);       
}

void
mmpAVCEncodeSWRest(
    void)
{   
    MMP_UINT32 cmd = 0; 
    MMP_UINT32 i = 0;
    
    // Software Reset Trigger
	cmd =  VPU_SW_RESET_BPU_CORE | VPU_SW_RESET_BPU_BUS;
    cmd |= VPU_SW_RESET_VCE_CORE | VPU_SW_RESET_VCE_BUS;
	// If you reset GDI, tiled map should be reconfigured
    //cmd |= VPU_SW_RESET_GDI_CORE | VPU_SW_RESET_GDI_BUS;
	VPU_WriteRegister(BIT_SW_RESET, cmd);
	//VPU_DELAY_US(1);	
	// wait until reset is done
	while(VPU_ReadRegister(BIT_SW_RESET_STATUS) != 0 && i != 10)
	{
		i++;
	}	
	// clear sw reset (not automatically cleared)
	VPU_WriteRegister(BIT_SW_RESET, 0);    
}

//MMP_UINT32
//mmpAVCEncodeGetIntStatus(
//    void)
//{
//    return VPU_ReadRegister(BIT_INT_REASON);
//}

void
mmpAVCEncodePowerUp(
    void)
{       
    VPU_EnableClock();
}

void
mmpAVCEncodePowerDown(
    void)
{       
    WaitIdle(100);
    VPU_DisableClock();     
}

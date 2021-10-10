/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  Memory Stick and Memory Stick Pro common communication functions.
 *
 *  SMSC: S => Spec
 *        M => Memory
 *        S => Stick
 *        C => MS and MSpro common use function
 *
 * @author Irene Lin
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "mspro/config.h"
#include "mspro/ms_type.h"
#include "mspro/ms_hw.h"
#include "mspro/ms_common.h"
#if defined (__OPENRTOS__)
#include "ite/ith.h"
#endif
#if defined(__OR32__)
#include "or32.h"
#define ithInvalidateDCacheRange    or32_invalidate_cache
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================


//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================


//=========================================================
/**
 * Get write protect status.
 */
//=========================================================
MMP_BOOL
SMSC_GetWriteProtectStatus(void)
{
    MMP_INT result = 0;
    MMP_UINT8 value = 0;

    result = SMSC_SetRwRegAddress(0x1, 0x10, 0x01, 0x02);
    if(result)
        goto end;

    MS_SetTpcCmdReg(MS_TPC_READ_REG, Qualet_1_Byte, 0x1);
 
    MS_SetControlReg(MS_MSK_GINT_WAIT_BS0_EN
                    |MS_MSK_INT_2STATE_EN
                    |MS_MSK_INT_CRC_CHECK_EN
                    |MS_MSK_TRANSFER_EN
                    |MS_MSK_MEMORY_STICK_EN);
    
    result = MS_WaitCmdCompleteReg();
    if(result)
        goto end;

    value = (MMP_UINT8)MS_ReadDataReg();

end:
    if(result)
        LOG_ERROR "SMSC_GetWriteProtectStatus() return error code 0x%08X \n", result LOG_END

    return (MMP_BOOL)((value & SMS_MSK_WRITE_PROTECT) ? MMP_TRUE : MMP_FALSE);
}

//=========================================================
/**
 * Get Media Type.
 */
//=========================================================
MMP_INT SMSC_GetMediaType(MS_MEDIA_TYPE* mediaType)
{
    MMP_INT result = 0;

    result = SMSC_SetRwRegAddress(0x1, 0x10, 0x4, 0x4);
    if(result)
        goto end;

    MS_SetTpcCmdReg(MS_TPC_READ_REG, Qualet_1_Byte, 0x4);
    
    MS_SetControlReg(MS_MSK_GINT_WAIT_BS0_EN
                    |MS_MSK_INT_2STATE_EN
                    |MS_MSK_INT_CRC_CHECK_EN
                    |MS_MSK_TRANSFER_EN
                    |MS_MSK_MEMORY_STICK_EN);

    result = MS_WaitCmdCompleteReg();
    if(result)
        goto end;
    
    mediaType->ms_type     = (MMP_UINT8)MS_ReadDataReg();
    MS_ReadDataReg(); // reserved, unused.
    mediaType->ms_category = (MMP_UINT8)MS_ReadDataReg();
    mediaType->ms_class    = (MMP_UINT8)MS_ReadDataReg();

    /** For MS Pro */
    if(mediaType->ms_type == 0x01)
    {
        if(mediaType->ms_category != 0x00)
            goto error_type;

        if(mediaType->ms_class == 0x00)
        {
            mediaType->ms_device_type = MS_CARD_PRO;
            if(SMSC_GetWriteProtectStatus())
            {
                /** Write-Protected Status */
                MSCard.flags |= MS_FLAGS_WRITE_PROTECT;
                LOG_INFO " This MS-Pro card is in Write-Protected status !! \n" LOG_END
            }
        }
        else
        {
            if( (mediaType->ms_class == 0x01) ||
                (mediaType->ms_class == 0x02) ||
                (mediaType->ms_class == 0x03) )
            {
                /** Read Only Type */
                mediaType->ms_device_type = MS_CARD_PRO;
                MSCard.flags |= MS_FLAGS_WRITE_PROTECT;
                LOG_INFO " This is READ ONLY MS-Pro card!! \n" LOG_END
            }
            else
                goto error_type;
        }
        goto end;
    }
    /** For MS */
    else if( (mediaType->ms_type == 0x00) || (mediaType->ms_type == 0xFF) )
    {
        if(mediaType->ms_type == 0x00)
        {
            if( (mediaType->ms_category >= 0x80)/* && (mediaType->ms_category <= 0xFF) */)
                goto error_type;
            if( (mediaType->ms_category >= 0x01) && (mediaType->ms_category <= 0x7F) )
            {
                result = ERROR_MS_MS_IO_EXPANDED_MODULE;
                goto end;
            }

            if( (mediaType->ms_class >= 0x04)/* && (mediaType->ms_class <= 0xFF) */)
                goto error_type;
        }
        else
        {
            if( ((mediaType->ms_category >= 0x80) && (mediaType->ms_category <= 0xFE)) || (mediaType->ms_category == 0x00) )
                goto error_type;
            if( (mediaType->ms_category >= 0x01) && (mediaType->ms_category <= 0x7F) )
            {
                result = ERROR_MS_MS_IO_EXPANDED_MODULE;
                goto end;
            }

            if( ((mediaType->ms_class >= 0x04) && (mediaType->ms_class <= 0xFE)) || (mediaType->ms_class == 0x00) )
                goto error_type;
        }

        mediaType->ms_device_type = MS_CARD;

        if( (mediaType->ms_class >= 0x01) && (mediaType->ms_class <= 0x03) )
        {
            MSCard.flags |= MS_FLAGS_WRITE_PROTECT;
            LOG_INFO " MS card is always in the Write-Protected status!! \n" LOG_END
        }

        if(SMSC_GetWriteProtectStatus())
        {
            MSCard.flags |= MS_FLAGS_WRITE_PROTECT;
            LOG_INFO " MS card's Write-Protect setting is switch to ON!! \n" LOG_END
        }
        goto end;
    }

error_type:
    result = ERROR_MS_MEDIA_TYPE_ERROR;
    goto end;

end:
    if(result)
        LOG_ERROR "SMSC_GetMediaType() return error code 0x%08X, Type=0x%02X, Category=0x%02X, Class=0x%02X \n", result, mediaType->ms_type, mediaType->ms_category, mediaType->ms_class LOG_END

    {
        LOG_INFO " ms_type     = 0x%02X \n", mediaType->ms_type LOG_END
        LOG_INFO " ms_category = 0x%02X \n", mediaType->ms_category LOG_END
        LOG_INFO " ms_class    = 0x%02X \n", mediaType->ms_class LOG_END
        switch(MSCard.Media_type.ms_device_type)
        {
        case MS_CARD_PRO:
            LOG_INFO " Device Type: MS pro \n" LOG_END
            break;
        case MS_CARD:
            LOG_INFO " Device Type: MS Card \n" LOG_END
            break;
        }
    }
    return result;
}

MMP_INT SMSC_SetRwRegAddress(MMP_UINT8 W_len,MMP_UINT8 W_addr,MMP_UINT8 R_len,MMP_UINT8 R_addr)
{
    MMP_INT result = 0;

    MS_SetTpcCmdReg(MS_TPC_SET_REG_ADRS, Qualet_1_Byte, 0x4);
    
    MS_SetControlReg(MS_MSK_GINT_WAIT_BS0_EN
                    |MS_MSK_INT_2STATE_EN
                    |MS_MSK_INT_CRC_CHECK_EN
                    |MS_MSK_TRANSFER_EN
                    |MS_MSK_MEMORY_STICK_EN);

    MS_WriteDataReg(R_addr);
    MS_WriteDataReg(R_len);
    MS_WriteDataReg(W_addr);
    MS_WriteDataReg(W_len);

    result = MS_WaitCmdCompleteReg();
    if(result)
        goto end;

end:
    if(result)
        LOG_ERROR "SMSC_SetRwRegAddress() return error code 0x%08X \n", result LOG_END
    return result;
}

MMP_INT SMSC_InterfaceSwitch(MS_INTERFACE mode)
{
    MMP_INT result = 0;
    MMP_UINT8 value = 0;

    if(MSCard.Media_type.ms_device_type == MS_CARD_PRO)
    {
        if(mode == MS_INTERFACE_PARALLEL)
            value = 0x00;
        else
            value = 0x80;
    }
    else
    {
        if(mode == MS_INTERFACE_PARALLEL)
            value = 0x88;
        else
            value = 0x80;
    }
    #if defined(MS_WORKAROUND)
    MSCard.sysReg = value;
    #endif
    
    result = SMSC_SetRwRegAddress(0x01,0x10,0x08,0x00);
    if(result)
        goto end;

    MS_SetTpcCmdReg(MS_TPC_WRITE_REG, Qualet_1_Byte, MS_TPC_LEN_WRITE_REG(1));	
    
    MS_SetControlReg(MS_MSK_GINT_WAIT_BS0_EN
                    |MS_MSK_INT_2STATE_EN
                    |MS_MSK_INT_CRC_CHECK_EN
                    |MS_MSK_TRANSFER_EN
                    |MS_MSK_MEMORY_STICK_EN);
	
    MS_WriteDataReg(value);
	
    result = MS_WaitCmdCompleteReg();
    if(result)
        goto end;

    if(mode == MS_INTERFACE_PARALLEL)
    {
        MSCard.flags |= MS_FLAGS_PARALLEL_INTERFACE;
        INT_CED   = MS_MSK_INT_CED_P;
        INT_CMDNK = MS_MSK_INT_CMDNK_P;
        INT_ERR   = MS_MSK_INT_ERR_P;
        INT_BREQ  = MS_MSK_INT_BREQ_P;
    }
    else
    {
        MSCard.flags &= ~MS_FLAGS_PARALLEL_INTERFACE;
        INT_CED   = SMS_INT_CED_S;
        INT_CMDNK = SMS_INT_CMDNK_S;
        INT_ERR   = SMS_INT_ERR_S;
        INT_BREQ  = SMS_INT_BREQ_S;
    }

    MS_SetSerialClkDivideReg();

end:
    if(result)
        LOG_ERROR "SMSC_InterfaceSwitch() return error code 0x%08X \n", result LOG_END
    return result;
}


MMP_INT
SMSC_ReadPageDataDma(MMP_UINT8* data)
{
    MMP_INT result = 0;
    
    MS_SetTpcCmdReg(MSP_TPC_READ_LONG_DATA, Qualet_4_Byte, MS_PAGE_SIZE);	

    MS_SetControlReg((MS_READ_BURST_SIZE << MS_SHT_FIFO_OVERRUN_THRE)
                    |MS_MSK_RX_DMA_EN
                    |MS_MSK_GINT_WAIT_BS0_EN
                    |MS_MSK_INT_2STATE_EN
                    |MS_MSK_INT_CRC_CHECK_EN
                    |MS_MSK_TRANSFER_EN
                    |MS_MSK_MEMORY_STICK_EN);

    ithDmaStart(MSCard.dmaCh);

    result = MS_WaitCmdCompleteReg();
    if(result)
        goto end;

    result = MS_WaitDmaComplete();
    if(result)
        goto end;

    #if defined (__OPENRTOS__)
    ithInvalidateDCacheRange(data, MS_PAGE_SIZE);
    #endif

end:
    if(result)
        LOG_ERROR "SMSC_ReadPageDataDma() return error code 0x%08X \n", result LOG_END
    return result;
}

MMP_INT
SMSC_ReadPageDataCpu(MMP_UINT8* data)
{
    MMP_INT result = 0;
    MMP_UINT32 i,j;
    MMP_UINT32 fifoSizeInbyte = MSCard.FIFO_Size*4;
    MMP_UINT32 tmpData = 0;
    
    MS_SetTpcCmdReg(MSP_TPC_READ_LONG_DATA, Qualet_4_Byte, MS_PAGE_SIZE);	

    MS_SetControlReg(MS_MSK_GINT_WAIT_BS0_EN
                    |MS_MSK_INT_2STATE_EN
                    |MS_MSK_INT_CRC_CHECK_EN
                    |MS_MSK_TRANSFER_EN
                    |MS_MSK_MEMORY_STICK_EN);

    for(i=0; i<(MS_PAGE_SIZE/fifoSizeInbyte); i++)
    {
        result = MS_WaitFifoFullReg();
        if(result)
            goto end;
        
        for(j=0; j<MSCard.FIFO_Size; j++, data+=4)
        {
            tmpData = MS_ReadDataReg();
            *(MMP_UINT32*)data = cpu_to_le32(tmpData);
        }
    }

    result = MS_WaitCmdCompleteReg();
    if(result)
        goto end;

end:
    if(result)
        LOG_ERROR "SMSC_ReadPageDataCpu() return error code 0x%08X \n", result LOG_END
    return result;
}

MMP_INT
SMSC_WritePageDataDma(MMP_UINT8* data)
{
    MMP_INT result = 0;
    
    MS_SetTpcCmdReg(MSP_TPC_WRITE_LONG_DATA, Qualet_4_Byte, MS_PAGE_SIZE);

    MS_SetControlReg((MS_WRITE_BURST_SIZE << MS_SHT_FIFO_UNDERRUN_THRE)
                    |MS_MSK_TX_DMA_EN
                    |MS_MSK_GINT_WAIT_BS0_EN
                    |MS_MSK_INT_2STATE_EN
                    |MS_MSK_INT_CRC_CHECK_EN
                    |MS_MSK_TRANSFER_EN
                    |MS_MSK_MEMORY_STICK_EN);

    ithDmaStart(MSCard.dmaCh);

    result = MS_WaitDmaComplete();
    if(result)
        goto end;
    result = MS_WaitCmdCompleteReg();
    if(result)
        goto end;

end:
    if(result)
        LOG_ERROR "SMSC_WritePageDataDma() return error code 0x%08X \n", result LOG_END
    return result;
}

MMP_INT
SMSC_WritePageDataCpu(MMP_UINT8* data)
{
    MMP_INT result = 0;
    MMP_UINT32 i,j;
    MMP_UINT32 fifoSizeInbyte = MSCard.FIFO_Size*4;
    MMP_UINT32 tmpData = 0;
    
    MS_SetTpcCmdReg(MSP_TPC_WRITE_LONG_DATA, Qualet_4_Byte, MS_PAGE_SIZE);
	
    MS_SetControlReg(MS_MSK_GINT_WAIT_BS0_EN
                    |MS_MSK_INT_2STATE_EN
                    |MS_MSK_INT_CRC_CHECK_EN
                    |MS_MSK_TRANSFER_EN
                    |MS_MSK_MEMORY_STICK_EN);
  
    for(i=0; i<(MS_PAGE_SIZE/fifoSizeInbyte); i++)
    {
        result = MS_WaitFifoEmptyReg();
        if(result)
            goto end;
        
        for(j=0; j<MSCard.FIFO_Size; j++, data+=4)
        {
            tmpData = (*(data+3) << 24 |
                       *(data+2) << 16 |
                       *(data+1) << 8  |
                       *(data));
            MS_WriteDataReg(tmpData);
        }
    }
        
    result = MS_WaitCmdCompleteReg();
    if(result)
        goto end;

end:
    if(result)
        LOG_ERROR "SMSC_WritePageDataCpu() return error code 0x%08X \n", result LOG_END
    return result;
}


//=========================================================
/**
 * MS pro Check Status Command.
 */
//=========================================================
/** 
 * Get INT status
 */
MMP_INT
SMSC_GetIntStatus(MMP_UINT32* status)
{
    MMP_INT result = 0;

    MS_SetTpcCmdReg(MS_TPC_GET_INT, Qualet_1_Byte, MS_TPC_LEN_GET_INT);
    
    MS_SetControlReg(MS_MSK_GINT_WAIT_BS0_EN
                    |MS_MSK_INT_2STATE_EN
                    |MS_MSK_INT_CRC_CHECK_EN
                    |MS_MSK_TRANSFER_EN
                    |MS_MSK_MEMORY_STICK_EN);
    
    result = MS_WaitCmdCompleteReg();
    if(result)
        goto end;
    
    (*status) = MS_ReadDataReg();

end:
    if(result)
        LOG_ERROR "SMSC_GetIntStatus() return error code 0x%08X \n", result LOG_END
    return result;
}

/** Get INT status special case for confirm CPU Startup case. */
MMP_INT
SMSC_GetIntStatusEx(MMP_UINT32* status)
{
    MMP_INT    result = 0;

    MS_SetTpcCmdReg(MS_TPC_GET_INT, Qualet_1_Byte, MS_TPC_LEN_GET_INT);

    MS_SetControlReg(/*MS_MSK_GINT_WAIT_BS0_EN
                    |*/MS_MSK_INT_2STATE_EN
                    |MS_MSK_INT_CRC_CHECK_EN
                    |MS_MSK_TRANSFER_EN
                    |MS_MSK_MEMORY_STICK_EN);
    
    result = MS_WaitCmdCompleteReg();
    if(result)
        goto end;

    (*status) = MS_ReadDataReg();

end:
    if(result)
        LOG_ERROR "SMSC_GetIntStatusEx() return error code 0x%08X \n", result LOG_END
    return result;
}


//=========================================================
/**
 * MS get status register1.
 */
//=========================================================
MMP_INT
SMSC_WaitIntStatus(MMP_UINT32* status, MMP_UINT32 mask, MMP_UINT32 sTimeout)
{
    MMP_INT result = 0;

    if(MSCard.flags & MS_FLAGS_PARALLEL_INTERFACE)
    {
        result = MS_GetIntStatusReg(status, sTimeout);
        if(result)
            goto end;

        (*status) &= mask;
    }
    else
    {
        struct timeval startT, endT;

        gettimeofday(&startT, NULL);
        
        (*status) = 0;
        result = SMSC_GetIntStatus(status);
        if(result)
            goto end;
        (*status) &= mask;
        while(!(*status))
        {
            gettimeofday(&endT, NULL);
            if(itpTimevalDiff(&startT, &endT) > (long)sTimeout)
            {
                result = ERROR_MS_WAIT_INT_TIMEOUT_S;
                goto end;
            }
            result = SMSC_GetIntStatus(status);
            if(result)
                goto end;

            (*status) &= mask;
        }
    }

end:
    if(result)
        LOG_ERROR "SMSC_WaitIntStatus() return error code 0x%08X \n", result LOG_END

    return result;
}

#if defined(MS_WORKAROUND)

MMP_INT MSC_WrStart(void)
{
    MMP_INT result = 0;
    
    result = SMSC_SetRwRegAddress(0x01,0x10,0x08,0x00);
    if(result)
        goto end;

    MS_SetTpcCmdReg(MS_TPC_WRITE_REG, Qualet_1_Byte, MS_TPC_LEN_WRITE_REG(1));	
    
    MS_SetControlReg(MS_MSK_GINT_WAIT_BS0_EN
                    |MS_MSK_INT_2STATE_EN
                    |MS_MSK_INT_CRC_CHECK_EN
                    |MS_MSK_TRANSFER_EN
                    |MS_MSK_MEMORY_STICK_EN);

end:
    if(result)
        LOG_ERROR "MSC_WrStart() return error code 0x%08X \n", result LOG_END
    return result;
}
	
MMP_INT MSC_WrEnd(void)
{
    MMP_INT result = 0;

    MS_WriteDataReg(MSCard.sysReg);
	
    result = MS_WaitCmdCompleteReg();
    if(result)
        goto end;

end:
    if(result)
        LOG_ERROR "MSC_WrEnd() return error code 0x%08X \n", result LOG_END
    return result;
}

#endif


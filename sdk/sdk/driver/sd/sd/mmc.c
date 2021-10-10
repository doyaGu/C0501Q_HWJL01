/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  MMC related function.
 *
 * @author Irene Lin
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "config.h"
#include "sd_hw.h"
#include "sd.h"
#if defined (__OPENRTOS__)
#include "ite/ith.h"
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
extern uint8_t sdResp[17];

//=============================================================================
//                              Private Function Definition
//=============================================================================
//=============================================================================
/**
 * MMC4 switch.
 *
 * Send CMD8: send EXT_CSD
 *            From EXT_CSD the host gets the power and speed class of the card.
 *
 * Send CMD6: Switch the mode of operation of the selected card or modifies the EXT_CSD register.
 *
 * Send CMD19: host sends a data pattern with CMD19, card sends a response and latches received 
 *             data pattern internally.
 * Send CMD14: host sends CMD14, the card would sned the reverse pattern of received data.
 */
//=============================================================================
static inline int
MMC4_Switch(SD_CTXT* ctxt)
{
    int result = 0;
    uint8_t extCSD[SD_SECTOR_SIZE] = {0};
    uint8_t* pattern = extCSD;

    //==============================
    // Send CMD8: send EXT_CSD
    //            From EXT_CSD the host gets the power and speed class of the card.
    //==============================
    SD_SetSectorLengthReg(SD_SECTOR_SIZE);
    SD_SetSectorCountReg(1);
#if defined(MMC_WR_TIMING) || defined(SD_WIN32_DMA)
    if(0)
#else
    if((ctxt->flags & SD_FLAGS_DMA_ENABLE) && (ctxt->dmaCh >= 0))
#endif
    {
        SDMMC_DmaRead(ctxt, extCSD, SD_SECTOR_SIZE);
        SDW_SetWrapCtrlReg(ctxt, SDW_DATA_IN, SDW_DMA); /** set smedia wrap control */
        result = SD_SendCmdReg(ctxt, MMC_ADTC_SEND_EXT_CSD, 0, 0, 0, 0, (SD_RESP_TYPE_48|SD_CMD_DATA_IN|SD_MSK_AUTO_SWAP));
        if(result)
            goto end;
        #if !defined (WIN32)
        ithInvalidateDCacheRange(extCSD, SD_SECTOR_SIZE);
        #endif
    }
    else
    {
        SDW_SetWrapCtrlReg(ctxt, SDW_DATA_IN, SDW_NON_DMA); /** set smedia wrap control */
        SD_SendCmdNoWaitReg(ctxt, MMC_ADTC_SEND_EXT_CSD, 0, 0, 0, 0, (SD_RESP_TYPE_48|SD_CMD_DATA_IN|SD_MSK_AUTO_SWAP)); // Irene: Need swap??

        result = SDMMC_ReadData(ctxt, extCSD, SD_SECTOR_SIZE);
        if(result)
            goto end;
    }

    if((extCSD[196] & 0x03) == 0x03)
    {
        if(!(ctxt->flags & SD_FLAGS_CARD_MMC4))
            LOG_WARNING " It is NOT MMC v4.0 but it is MMC4 card type!! \n" LOG_END
        ctxt->flags |= SD_FLAGS_CARD_MMC4;
    }
    else
    {
        if(ctxt->flags & SD_FLAGS_CARD_MMC4)
            LOG_WARNING " It is MMC v4.0 but NOT MMC4 card type!! \n" LOG_END
        ctxt->flags &= ~SD_FLAGS_CARD_MMC4;
    }
    LOG_INFO " MMC4 card type 0x%02X \n", extCSD[196] LOG_END

    #if defined(SD_DUMP_EXT_CSD)
    {
        uint32_t i = 0;
        LOG_DATA " EXT_CSD content: (H->L) \n" LOG_END
        for(i=0; i<512; i+=16)
        {
            LOG_DATA " Byte %02d: %02X %02X %02X %02X %02X %02X %02X %02X -  %02X %02X %02X %02X %02X %02X %02X %02X \n",
                    i, extCSD[i+0], extCSD[i+1], extCSD[i+2], extCSD[i+3], extCSD[i+4], extCSD[i+5], extCSD[i+6], extCSD[i+7]
                     , extCSD[i+8], extCSD[i+9], extCSD[i+10], extCSD[i+11], extCSD[i+12], extCSD[i+13], extCSD[i+14], extCSD[i+15] LOG_END
        }
    }
    #endif

    ctxt->clockDiv = SD_CLK_NORMAL; 
    SD_SetClockDivReg(ctxt->clockDiv);
    SDMMC_WriteDelay(ctxt->flags & SD_FLAGS_SD_HIGH_SPEED);
    if(ctxt->flags & SD_FLAGS_CARD_MMC_HC)
    {
        ctxt->totalSectors = (extCSD[212] |
                               (extCSD[213]<<8) |
                               (extCSD[214]<<16) |
                               (extCSD[215]<<24) );
        LOG_INFO " MMC HC new total sector number = 0x%08X \n", ctxt->totalSectors LOG_END
        if(!ctxt->totalSectors)
        {
            result = ERROR_MMC_HC_TOTAL_SECTORS_ERROR;
            goto end;
        }
    }

    //==============================
    // MMC4 card switch to High speed
    // Send CMD6: Switch the mode of operation of the selected card or modifies the EXT_CSD register.
    //==============================
    if(ctxt->flags & SD_FLAGS_CARD_MMC4)
    {
        /** arg 0x03B90100 */
        if(SD_SendCmdReg(ctxt, MMC_AC_SWITCH, ACCESS_WRITE_BYTE, HS_TIMING_INDEX, MMC_SPEED, 0, (SD_RESP_TYPE_48_BUSY|SD_CMD_NO_DATA)))
        {
            ctxt->flags &= ~SD_FLAGS_CARD_MMC4;
            LOG_WARNING " MMC4 change high speed fail! => disable MMC4 \n" LOG_END
        }
        else
        {
            ctxt->flags |= SD_FLAGS_SD_HIGH_SPEED;
            //LOG_INFO2 " Run with High Speed! \n" LOG_END
        }
    }
    if(ctxt->flags & SD_FLAGS_CARD_MMC4)
    {
        ctxt->clockDiv = SD_CLK_HIGH_SPEED;
		SD_SetHighSpeedReg();
        LOG_INFO2 " Run with High Speed! \n" LOG_END
    }
    else
    {
        ctxt->clockDiv = SD_CLK_NORMAL;
        LOG_INFO2 " Run with Default Mode! \n" LOG_END
    }
    SD_SetClockDivReg(ctxt->clockDiv);

    //==============================
    // MMC4 card Change bus width procedure
    // Send CMD19: test write
    // Send CMD14: test read
    //==============================
    /** set host as 8-bits */
    ctxt->cardBusWidth = 8;
    SD_SetBusWidthReg(ctxt->cardBusWidth);
    SD_CrcBypassReg();
	#if defined(SD_READ_FLIP_FLOP)
    SD_CrcNonFixReg(false);
	#endif

    /** test write: CMD19, test read: CMD14 */
    pattern[0] = 0x55;
    pattern[1] = 0xAA;
    SD_SetSectorLengthReg(SD_SECTOR_SIZE);
    SD_SetSectorCountReg(1);
#if !defined (WIN32)
    if((ctxt->flags & SD_FLAGS_DMA_ENABLE) && (ctxt->dmaCh >= 0))
    {
        /** test write: CMD19 */
        SDMMC_DmaWrite(ctxt, pattern, SD_SECTOR_SIZE);
        SDW_SetWrapCtrlReg(ctxt, SDW_DATA_OUT, SDW_DMA);  /** set smedia wrap control */
        result = SD_SendCmdReg(ctxt, MMC_ADTC_BUSTEST_W, 0, 0, 0, 0, (SD_RESP_TYPE_48|SD_CMD_DATA_OUT|SD_MSK_AUTO_SWAP)); // Irene: Need swap??
        if(result)
            goto end;
        /** test read: CMD14 */
        pattern[0] = 0x0;
        pattern[1] = 0x0;
        SDMMC_DmaRead(ctxt, pattern, SD_SECTOR_SIZE);
        SDW_SetWrapCtrlReg(ctxt, SDW_DATA_IN, SDW_DMA); /** set smedia wrap control */
        result = SD_SendCmdReg(ctxt, MMC_ADTC_BUSTEST_R, 0, 0, 0, 0, (SD_RESP_TYPE_48|SD_CMD_DATA_IN|SD_MSK_AUTO_SWAP)); // Irene: Need swap??
        if(result)
            goto end;
        #if !defined(WIN32)
        ithInvalidateDCacheRange(pattern, SD_SECTOR_SIZE);
        #endif
    }
    else
#endif
    {
        /** test write: CMD19 */
        SDW_SetWrapCtrlReg(ctxt, SDW_DATA_OUT, SDW_NON_DMA);  /** set smedia wrap control */
        SD_SendCmdNoWaitReg(ctxt, MMC_ADTC_BUSTEST_W, 0, 0, 0, 0, (SD_RESP_TYPE_48|SD_CMD_DATA_OUT|SD_MSK_AUTO_SWAP));
        result = SDMMC_WriteData(ctxt, pattern, SD_SECTOR_SIZE);
        if(result)
            goto end;

        /** test read: CMD14 */
        SDW_SetWrapCtrlReg(ctxt, SDW_DATA_IN, SDW_NON_DMA); /** set smedia wrap control */
        SD_SendCmdNoWaitReg(ctxt, MMC_ADTC_BUSTEST_R, 0, 0, 0, 0, (SD_RESP_TYPE_48|SD_CMD_DATA_IN|SD_MSK_AUTO_SWAP));
        result = SDMMC_ReadData(ctxt, pattern, SD_SECTOR_SIZE);
        if(result)
            goto end;
    }
    #if defined(SD_READ_FLIP_FLOP)
    SD_CrcNonFixReg(true);
    #endif

    //==============================
    // Determines the bus line connection or the number of the card I/O through CMD6
    //==============================
    ctxt->cardBusWidth = 1;
    SD_SetBusWidthReg(ctxt->cardBusWidth);
    if((pattern[0]==0xAA) && (pattern[1]==0x55))
    {
        if(SD_SendCmdReg(ctxt, MMC_AC_SWITCH, ACCESS_WRITE_BYTE, BUS_WIDTH_INDEX, MMC_BUS_WIDTH_8, 0, (SD_RESP_TYPE_48_BUSY|SD_CMD_NO_DATA)) == 0)
        {
            ctxt->cardBusWidth = 8;
            LOG_INFO2 " This MMC card run with 8-bits! \n" LOG_END
        }
        else
            LOG_WARNING " MMC 8-bits bus, but set 8 bits bus fail! \n" LOG_END

        ctxt->flags |= SD_FLAGS_MMC_8_BIT_BUS;
    }
    else if(((pattern[0]&0x0F)==0x0A) && ((pattern[1]&0x0F)==0x05))
    {
        if(SD_SendCmdReg(ctxt, MMC_AC_SWITCH, ACCESS_WRITE_BYTE, BUS_WIDTH_INDEX, MMC_BUS_WIDTH_4, 0, (SD_RESP_TYPE_48_BUSY|SD_CMD_NO_DATA)) == 0)
        {
            ctxt->cardBusWidth = 4;
            LOG_INFO2 " This MMC card run with 4-bits! \n" LOG_END
            LOG_INFO2 " pattern[0] = %X, pattern[1] = %X\n", pattern[0], pattern[1] LOG_END
        }
        else
            LOG_WARNING " MMC 4-bits bus, but set 4 bits bus fail! \n" LOG_END

        ctxt->flags |= SD_FLAGS_MMC_4_BIT_BUS;
    }
    else
    {
        LOG_ERROR " pattern[0] = %X, pattern[1] = %X\n", pattern[0], pattern[1] LOG_END
        result = ERROR_MMC_TEST_BUS_FAIL;
        goto end;
    }
    SD_SetBusWidthReg(ctxt->cardBusWidth);

    //SD_ResetIFReg();

end:
    if(result)
    {
        //SD_SetBusWidthReg(1);
        LOG_ERROR " MMC4_Switch() return error code 0x%08X \n", result LOG_END
    }
    return result;
}


//=============================================================================
//                              Public Function Definition
//=============================================================================
//=============================================================================
/**
 * MMC start up function.
 *
 * Send CMD1: asks all cards in idle state to send their operation conditions regsiter contents
 *            in the response on the CMD line.
 */
//=============================================================================
int
MMC_StartUp(SD_CTXT* ctxt)
{
    int result = 0;
    uint32_t lastTick;

    lastTick = itpGetTickCount();
    do
    {
        if(itpGetTickDuration(lastTick) > 600)
        {
            result = ERROR_MMC_SEND_CMD1_TIMEOUT;
            goto end;
        }
        if(!SD_IsCardInserted(ctxt->index))
        {
            result = ERROR_SD_NO_CARD_INSERTED;
            goto end;
        }

        if(SD_SendCmdReg(ctxt, MMC_BCR_SEND_OP_COND, MMC_HOST_HC, 0xFF, 0x80, 0x00, (SD_RESP_TYPE_48|SD_CMD_NO_DATA)))
            continue;

        SD_ReadResponseReg(0x1E);
        LOG_INFO " OCR register: 0x%02X%02X%02X%02X \n", sdResp[4], sdResp[3], sdResp[2], sdResp[1] LOG_END
    } while(!(sdResp[4] & 0x80)); /** HOST repeatedly issues CMD1 until the busy bit is set to 1. */

    if((sdResp[4] & 0x60) == MMC_HOST_HC)
    {
        ctxt->flags |= SD_FLAGS_CARD_MMC_HC;
        LOG_INFO " MMC => SD_FLAGS_CARD_MMC_HC \n" LOG_END
    }

    ctxt->flags |= SD_FLAGS_CARD_MMC;
    LOG_INFO " This card is MMC! \n" LOG_END

end:
    if(result)
        LOG_ERROR " MMC_StartUp() return error code 0x%08X\n", result LOG_END
    return result;
}

//=============================================================================
/**
 * MMC card go into Stand-by State.
 *
 * Send CMD3: Assigns relative address to the card.
 */
//=============================================================================
int
MMC_SetRca(SD_CTXT* ctxt)
{
    int result = 0;

    do
    {
        ctxt->rcaByte2++;
        if(ctxt->rcaByte2 > 10)
        {
            result = ERROR_MMC_SET_RCA_MORE_THAN_10;
            goto end;
        }
        if(SD_SendCmdReg(ctxt, MMC_BCR_SET_RELATIVE_ADDR, 0, ctxt->rcaByte2, 0, 0, (SD_RESP_TYPE_48|SD_CMD_NO_DATA)))
            continue;

        SD_ReadResponseReg(0x1E);
        LOG_DEBUG " MMC Set RCA Status = 0x%08X \n", sdResp[4], sdResp[3], sdResp[2], sdResp[1] LOG_END
        break;
    } while(1);

end:
    if(result)
        LOG_ERROR " MMC_SetRca() return error code 0x%08X \n", result LOG_END
    return result;
}

//=============================================================================
/**
 * MMC set bus width.
 *
 */
//=============================================================================
int
MMC_Switch(SD_CTXT* ctxt)
{
    int result = 0;

    if(ctxt->flags & SD_FLAGS_CARD_MMC4)
    {
        result = MMC4_Switch(ctxt);
        if(result)
            goto end;
    }
    if(!(ctxt->flags & SD_FLAGS_CARD_MMC4))
    {
        ctxt->clockDiv = SD_CLK_NORMAL;
        SD_SetClockDivReg(ctxt->clockDiv);
        SDMMC_WriteDelay(ctxt->flags & SD_FLAGS_SD_HIGH_SPEED);
    }

end:
    if(result)
        LOG_ERROR " MMC_Switch() return error code 0x%08X \n", result LOG_END
    return result;
}


/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  SD relaed function.
 *
 * @author Irene Lin
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "config.h"
#include "sd_hw.h"
#include "sd.h"

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
 * SD switch function.
 *
 * Send CMD6: 
 *  Mode 0 (Check function) is used to query if the card supports a specific function
 *  or functions.
 *  Mode 1 (set functoin) is used to switch the functionality of the card.
 */
//=============================================================================
#define SD_SWITCH_FUN_LEN   64

static int SD_SwitchFunction11(SD_CTXT* ctxt)
{
    int result = 0;
    uint8_t status[SD_SWITCH_FUN_LEN] = {0};
    int i = 0;
    uint32_t tmp = 0;
    uint32_t lastTick;

    lastTick = itpGetTickCount();
    while(1)
    {
        if(itpGetTickDuration(lastTick) > 500)
        {
            result = ERROR_SD_SEND_CMD6_TIMEOUT;
            goto end;
        }
        if(!SD_IsCardInserted(ctxt->index))
        {
            result = ERROR_SD_NO_CARD_INSERTED;
            goto end;
        }

        /** Send CMD6: Mode 0 => check function */
        SD_SetSectorLengthReg(SD_SWITCH_FUN_LEN);
        SD_SetSectorCountReg(1);
        SDW_SetWrapCtrlReg(ctxt, SDW_DATA_IN, SDW_NON_DMA); /** set smedia wrap control */
        if(SD_SendCmdReg(ctxt, SD_ADTC_SWITCH_FUNCTION, SD_FUN_CHECK_MODE, 0xFF, 0xFF, (0xF0|SD_FUN_HIGH_SPEED), (SD_RESP_TYPE_48|SD_CMD_DATA_IN|SD_MSK_AUTO_SWAP)))
            continue;

        for(i=0; i<SD_SWITCH_FUN_LEN; i+=4)
        {
            tmp = SD_ReadDataReg();
            *((uint32_t*)&status[i]) = cpu_to_le32(tmp);
        }

        result = SDW_WaitWrapReadyReg(ctxt);
        if(result)
            goto end;

        #if defined(SD_DUMP_SWITCH_FUN)
        LOG_DATA " SD Switch Function status: H->L \n" LOG_END
        for(i=0; i<SD_SWITCH_FUN_LEN; i+=16)
        {
            LOG_DATA " Byte %02d: %02X %02X %02X %02X %02X %02X %02X %02X -  %02X %02X %02X %02X %02X %02X %02X %02X \n",
                    i, status[i+0], status[i+1], status[i+2], status[i+3], status[i+4], status[i+5], status[i+6], status[i+7]
                     , status[i+8], status[i+9], status[i+10], status[i+11], status[i+12], status[i+13], status[i+14], status[i+15] LOG_END
        }
        #endif

        if((status[0]==0) && (status[1]==0))
        {
            result = ERROR_SD_CMD6_ERROR;
            goto end;
        }
        if(status[29] & 0x01) /** busy */
            continue;

        /** Send CMD6: Mode 1 => set function */
        SD_SetSectorLengthReg(SD_SWITCH_FUN_LEN);
        SD_SetSectorCountReg(1);
        SDW_SetWrapCtrlReg(ctxt, SDW_DATA_IN, SDW_NON_DMA); /** set smedia wrap control */
        if(SD_SendCmdReg(ctxt, SD_ADTC_SWITCH_FUNCTION, SD_FUN_SWITCH_MODE, 0xFF, 0xFF, (0xF0|SD_FUN_HIGH_SPEED), (SD_RESP_TYPE_48|SD_CMD_DATA_IN|SD_MSK_AUTO_SWAP)))
            continue;

        for(i=0; i<SD_SWITCH_FUN_LEN; i+=4)
        {
            tmp = SD_ReadDataReg();
            *((uint32_t*)&status[i]) = cpu_to_le32(tmp);
        }

        result = SDW_WaitWrapReadyReg(ctxt);
        if(result)
            goto end;

        if(status[16] & 0x01) // Irene: ??
            goto end;

        if(status[29] & 0x01) /** busy */
            continue;
    }

end:
    if(result)
        LOG_WARNING " SD_SwitchFunction11() return error code 0x%08X \n", result LOG_END
    return result;
}



//=============================================================================
//                              Public Function Definition
//=============================================================================
//=============================================================================
/**
 * SD start up function.
 *
 * Send CMD8: Sends SD Memory Card interface condition, which includes host supply
 *            voltage information and asks the card whether card supports voltage.
 * Send ACMD41: Sends host capacity support informatin (HCS) and asks the accessed 
 *              card to send its operating condition register (OCR) content in the 
 *              response on the CMD line.
 */
//=============================================================================
int
SD_StartUp(SD_CTXT* ctxt)
{
    int result = 0;
    uint32_t lastTick;

    ctxt->flags |= SD_FLAGS_CARD_SD;

    //============================
    // Send CMD8 
    //============================
    result = SD_SendCmdReg(ctxt, SD_BCR_SEND_IF_COND, 0, 0, 0x01, STUFF_BITS_AA, (SD_RESP_TYPE_48|SD_CMD_NO_DATA));
    if(result)
    {
        LOG_WARNING " Send CMD8 fail! \n" LOG_END 
        result = 0;
    }
    else
    {
        /** Card with compatible Voltage range. */
        LOG_INFO " SD 2.0 or later! \n" LOG_END
        SD_ReadResponseReg(0x6);
        if( ((sdResp[2]&0xF) == 0x01) && 
            (sdResp[1] == STUFF_BITS_AA) )
        {
            ctxt->flags |= SD_FLAGS_SUPPORT_CMD8;
        }
        else
        {
            result = ERROR_SD_SUPPORT_CMD8_BUT_FAIL;
            goto end;
        }
    }

    //============================
    // Send ACMD41 
    //============================
    lastTick = itpGetTickCount();
    do
    {
        if(itpGetTickDuration(lastTick) > 1300)
        {
            result = ERROR_SD_SEND_ACMD41_TIMEOUT;
            goto end;
        }
        if(!SD_IsCardInserted(ctxt->index))
        {
            result = ERROR_SD_NO_CARD_INSERTED;
            goto end;
        }

        if(SD_SendCmdReg(ctxt, SD_AC_APP_CMD, 0, 0, 0, 0, (SD_RESP_TYPE_48|SD_CMD_NO_DATA)))
            continue;

        //============================
        // arg D[23:0] = 0xFF8000, VDD Vlotage window OCR[23:0] 
        // ACMD41 => R3
        //============================
        if(ctxt->flags & SD_FLAGS_SUPPORT_CMD8)
        {
            if(SD_SendCmdReg(ctxt, SD_ACMD_BCR_SEND_OP_COND, SD_HOST_SDHC, 0xFF, 0x80, 0x0, (SD_RESP_TYPE_48|SD_CMD_NO_DATA)))
                continue;
        }
        else
        {
            if(SD_SendCmdReg(ctxt, SD_ACMD_BCR_SEND_OP_COND, 0x0, 0xFF, 0x80, 0x0, (SD_RESP_TYPE_48|SD_CMD_NO_DATA)))
                continue;
        }
        SD_ReadResponseReg(0x10);
    } while(!(sdResp[4] & 0x80)); /** HOST repeatedly issues ACMD41 until the busy bit is set to 1. */

    if( (ctxt->flags & SD_FLAGS_SUPPORT_CMD8) && (sdResp[4] & 0x40) )
    {
        ctxt->flags |= SD_FLAGS_CARD_SDHC;
        LOG_INFO " Ver2.00 or later SDHC! \n" LOG_END
    }
    else if( (ctxt->flags & SD_FLAGS_SUPPORT_CMD8) && !(sdResp[4] & 0x40) )
        LOG_INFO " Ver2.00 or later Standard Capatity SD! \n" LOG_END
    else
        LOG_INFO " Ver1.X Standard Capatity SD! \n" LOG_END

end:
    if(result)
        LOG_ERROR " SD_StartUp() return error code 0x%08X\n", result LOG_END
    return result;
}

//=============================================================================
/**
 * SD card go into Stand-by State.
 *
 * Send CMD3: Ask the card to publish a new relative address (RCA).
 */
//=============================================================================
int
SD_GetRca(SD_CTXT* ctxt)
{
    int result = 0;
    uint32_t lastTick;

    lastTick = itpGetTickCount();
    do
    {
        if(itpGetTickDuration(lastTick) > 600)
        {
            result = ERROR_SD_SEND_RCA_TIMEOUT;
            goto end;
        }
        if(!SD_IsCardInserted(ctxt->index))
        {
            result = ERROR_SD_NO_CARD_INSERTED;
            goto end;
        }
        result = SD_SendCmdReg(ctxt, SD_BCR_SEND_RELATIVE_ADDR, 0, 0, 0, 0, (SD_RESP_TYPE_48|SD_CMD_NO_DATA));
        if(result)
            goto end;
        SD_ReadResponseReg(0x1E);
        if((sdResp[2] & 0xE)==0x2)
        {
            result = ERROR_SD_RCA_STATUS_ERROR;
            LOG_ERROR " D[39:8] = %02X %02X %02X %02X \n", sdResp[4], sdResp[3], sdResp[2], sdResp[1] LOG_END
            goto end;
        }
    } while( (sdResp[4]==0) && (sdResp[3]==0) );

    ctxt->rcaByte3 = sdResp[4];
    ctxt->rcaByte2 = sdResp[3];

end:
    if(result)
        LOG_ERROR " SD_GetRca() return error code 0x%08X \n", result LOG_END
    return result;
}

//=============================================================================
/**
 * SD set the working clock and bus width.
 *
 * Get SCR(ACMD51)
 * SCR provides information on the SD Memory Card's special features that were configured into the given card.
 *
 * Set Bus Width(ACMD6)
 * SCR provides information on the SD Memory Card's special features that were configured into the given card.
 */
//=============================================================================
int
SD_Switch(SD_CTXT* ctxt)
{
    int result = 0;
    uint8_t scr[8] = {0};
    uint32_t tmp = 0;

    ctxt->clockDiv = SD_CLK_NORMAL_INIT;
    SD_SetClockDivReg(ctxt->clockDiv); 
    SDMMC_WriteDelay(ctxt->flags & SD_FLAGS_SD_HIGH_SPEED);

    //============================
    // Get SCR(ACMD51)
    // SCR provides information on the SD Memory Card's special features that were configured into the given card.
    //============================
    result = SD_SendCmdReg(ctxt, SD_AC_APP_CMD, ctxt->rcaByte3, ctxt->rcaByte2, 0, 0, (SD_RESP_TYPE_48|SD_CMD_NO_DATA));
    if(result)
        goto end;
    SD_CrcBypassReg(); // Irene: should we set CRC bypass?
    SD_SetSectorLengthReg(8);
    SD_SetSectorCountReg(1);
    SDW_SetWrapCtrlReg(ctxt, SDW_DATA_IN, SDW_NON_DMA); /** set smedia wrap control */
    result = SD_SendCmdReg(ctxt, SD_ACMD_ADTC_SEND_SCR, 0, 0, 0, 0, (SD_RESP_TYPE_48|SD_CMD_DATA_IN|SD_MSK_AUTO_SWAP));
    if(result)
        goto end;

    tmp = SD_ReadDataReg();
    *((uint32_t*)&scr[0]) = cpu_to_le32(tmp);
    tmp = SD_ReadDataReg();
    *((uint32_t*)&scr[4]) = cpu_to_le32(tmp);
    result = SDW_WaitWrapReadyReg(ctxt);
    if(result)
        goto end;
    LOG_INFO " SD SCR Reg(H->L): %02X %02X %02X %02X %02X %02X %02X %02X \n", scr[0], scr[1], scr[2], scr[3], scr[4], scr[5], scr[6], scr[7] LOG_END
    if((scr[1] & 0x0F) != 0x5)
    {
        LOG_ERROR " (scr[1] & 0x0F) != 0x5 \n" LOG_END
        result = ERROR_SD_BUS_WIDTH_ERROR;
        goto end;
    }
    ctxt->cardBusWidth = 4;

    //============================
    // Set Bus Width(ACMD6)
    // SCR provides information on the SD Memory Card's special features that were configured into the given card.
    //============================
    result = SD_SendCmdReg(ctxt, SD_AC_APP_CMD, ctxt->rcaByte3, ctxt->rcaByte2, 0, 0, (SD_RESP_TYPE_48|SD_CMD_NO_DATA));
    if(result)
        goto end;
    result = SD_SendCmdReg(ctxt, SD_ACMD_AC_SET_BUS_WIDTH, 0, 0, 0, SD_BUS_4BITS, (SD_RESP_TYPE_48|SD_CMD_NO_DATA));
    if(result)
        goto end;
    SD_SetBusWidthReg(ctxt->cardBusWidth);

    //============================
    // Switch Function(CMD6)
    // SD Physical Layer Specification Version > 1.10
    //============================
    if(scr[0] & 0x0F)
    {
        ctxt->flags |= SD_FLAGS_SUPPORT_SWITCH_FUNC;
        if(SD_SwitchFunction11(ctxt))
        {
            /** Switch High Speed fail. */
            ctxt->clockDiv = SD_CLK_NORMAL;
            LOG_INFO2 " SD switch High Speed fail!! \n" LOG_END
        }
        else
        {
            /** Run with High Speed. */
            ctxt->clockDiv = SD_CLK_HIGH_SPEED;
            ctxt->flags |= SD_FLAGS_SD_HIGH_SPEED;
            SD_SetHighSpeedReg();
            LOG_INFO2 " SD run with High Speed!! \n" LOG_END
        }
        SD_SetClockDivReg(ctxt->clockDiv);  
    }

end:
    if(result)
        LOG_ERROR " SD_Switch() return error code 0x%08X \n", result LOG_END
    return result;
}

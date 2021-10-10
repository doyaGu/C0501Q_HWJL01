/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * Used as SD HW configure header file.
 *
 * @author Irene Lin
 */

#ifndef SD_HW_H
#define SD_HW_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "config.h"
#include "sd_reg.h"
#include "sd.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define SDMMC_READ  0
#define SDMMC_WRITE 1

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================
#define SD_CrcBypassReg()                  ithWriteRegMaskA(SD_REG_STS0, SD_MSK_CRC_BYPASS, SD_MSK_CRC_BYPASS)
#define SD_CrcEnableReg()                  ithWriteRegMaskA(SD_REG_STS0, 0x0, SD_MSK_CRC_BYPASS)
#define SD_CrcNonFixReg(x)                 ithWriteRegMaskA(SD_REG_STS0, (x == true) ? SD_MSK_CRC_NON_FIX : 0x0, SD_MSK_CRC_NON_FIX)
#define SD_SetClockDivReg(clkDiv)          ithWriteRegA(SD_REG_CLK_DIV, (clkDiv << 2))
#if defined(SD_NEW_HW)
    #define SD_SetRespTimeoutReg(cardTick) ithWriteRegA(SD_REG_RESP_TIMEOUT, cardTick)
    #define SD_WrapIntrDisable()           ithWriteRegMaskA(SDW_REG_WRAP_STATUS, SDW_MSK_INTR_MASK, SDW_MSK_INTR_MASK)
    #define SD_WrapIntrEnable()            ithWriteRegMaskA(SDW_REG_WRAP_STATUS, 0x0, SDW_MSK_INTR_MASK)
    #if (CFG_CHIP_FAMILY == 9850)
        #define SD_SelectIo(idx)           SD_StorageSelect(idx)
    #else
        #define SD_SelectIo(idx)           do { \
                                               ithWriteRegMaskA(SD_REG_STS0, (idx << 4), (1 << 4)); \
                                               SD_StorageSelect(idx); \
                                           } while (0)
    #endif
#else
    #define SD_SetRespTimeoutReg(x)
    #define SD_WrapIntrDisable()
    #define SD_WrapIntrEnable()
    #define SD_SelectIo(idx)               SD_StorageSelect(idx)
#endif

#if defined(FPGA)
    #define SDMMC_DefaultDelay()
    #define SDMMC_ReadDelay()
    #define SDMMC_WriteDelay()
#else
    #define SDMMC_DefaultDelay()           ithWriteRegMaskA((SD_BASE + 0x008C), 0x0, 0x010000FF)
    #define SDMMC_ReadDelay(hs)            do { \
                                               uint32_t timing = hs ? 0x00000020 : 0x00000000; \
                                               ithWriteRegMaskA((SD_BASE + 0x008C), timing, 0x010000FF); \
                                           } while (0);
    #define SDMMC_WriteDelay(hs)           do { \
                                               uint32_t timing = hs ? 0x00000020 : 0x00000000; \
                                               ithWriteRegMaskA((SD_BASE + 0x008C), timing, 0x010000FF); \
                                           } while (0);
    //#define SD_SetHighSpeedReg()           ithWriteRegMaskA(SD_REG_STS0, SD_MSK_HIGH_SPEED, SD_MSK_HIGH_SPEED)
    #define SD_SetHighSpeedReg()
#endif

//=============================================================================
//							Funtion Declaration
//=============================================================================
void SD_PowerOnReg(int index);

void SD_PowerDownReg(SD_CTXT *ctxt);

/** reset sd card controller */
static inline void SD_ResetIFReg(void)
{
    /* 9070 A0 can't do this, it will trigger interrupt */
    //ithWriteRegMaskA(SD_REG_STS0, SD_MSK_IF_RESET, SD_MSK_IF_RESET);
    //ithWriteRegMaskA(SD_REG_STS0, 0, SD_MSK_IF_RESET);

    /* async reset */
    ithSetRegBitH(ITH_APB_CLK4_REG, ITH_RST_SDIP_BIT);     /* 0x22 D[8] */
    usleep(50);
    ithClearRegBitH(ITH_APB_CLK4_REG, ITH_RST_SDIP_BIT);   /* 0x22 D[8] */
    usleep(50);

#if defined(SD_READ_FLIP_FLOP) && (CFG_CHIP_FAMILY == 9910)
    /** sd data in from flip-flop out */
    ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_HTRAP_REG, 16);
    SD_CrcNonFixReg(true);
#endif
}

/** reset sd card controller + smedia wrapper */
static inline void SDW_SwResetReg(void)
{
    ithWriteRegMaskA(SDW_REG_WRAP_CTRL, SDW_MSK_SW_RESET, SDW_MSK_SW_RESET);
    ithWriteRegMaskA(SDW_REG_WRAP_CTRL, 0x0, SDW_MSK_SW_RESET);
}

static inline void SD_SetClockReg(uint32_t clkSrc, uint32_t clkDiv)
{
    ithWriteRegMaskA(SD_REG_STS0, clkSrc, (SD_MSK_ASYN_CLK_SEL | SD_MSK_CLK_SRC));
    ithWriteRegA(SD_REG_CLK_DIV, clkDiv);
}

static inline void SD_SetBusWidthReg(uint8_t busWidth)
{
    uint32_t value = 0;
    switch (busWidth)
    {
    case 1:
        value = SD_BUS_WIDTH_1BIT;
        break;
    case 4:
        value = SD_BUS_WIDTH_4BIT;
        break;
    case 8:
        value = SD_BUS_WIDTH_8BIT;
        break;
    default:
        value = SD_BUS_WIDTH_1BIT;
        LOG_ERROR " SD_SetBusWidthReg() Invalid Bus Width!!! \n" LOG_END
        break;
    }
    ithWriteRegMaskA(SD_REG_STS0, value, SD_MSK_BUS_WIDTH);
}

static inline void SD_GenClockReg(void)
{
    ithWriteRegMaskA(SD_REG_CTL, SD_MSK_CLK_CTRL, SD_MSK_CLK_CTRL);
    usleep(15 * 1000);
    ithWriteRegMaskA(SD_REG_CTL, 0, SD_MSK_CLK_CTRL);
}

static inline void SD_GetSectorLengthReg(uint32_t *length)
{
#if defined(SD_NEW_HW)
    (*length) = ithReadRegA(SD_REG_LENGTH);
#else
    uint32_t tmpH = 0;
    uint32_t tmpL = 0;
    tmpH       = ithReadRegA(SD_REG_LENGTH_H);
    tmpL       = ithReadRegA(SD_REG_LENGTH_L);
    (*length)  = ((tmpH & 0xFF) << 8) | (tmpL & 0xFF);
#endif
    (*length) += 1;
}

static inline void SD_SetSectorCountReg(uint32_t count)
{
    uint32_t sectorLen = 0;
    ithWriteRegA(SD_REG_SECTOR_COUNT_L, (uint32_t)(count & 0xFF));
    ithWriteRegA(SD_REG_SECTOR_COUNT_H, (uint32_t)((count >> 8) & 0xFF));
    /** for smedia wrap */
    SD_GetSectorLengthReg(&sectorLen);
    ithWriteRegA(SDW_REG_DATA_LEN, (uint32_t)(count * sectorLen));
}

static inline uint32_t SD_GetSectorCountReg(void)
{
    uint32_t tmpH = 0;
    uint32_t tmpL = 0;
    tmpH = ithReadRegA(SD_REG_SECTOR_COUNT_H);
    tmpL = ithReadRegA(SD_REG_SECTOR_COUNT_L);
    return ((tmpH & 0xFF) << 8) | (tmpL & 0xFF);
}

static inline void SD_SetSectorLengthReg(uint32_t length)
{
    length -= 1;
#if defined(SD_NEW_HW)
    ithWriteRegA(SD_REG_LENGTH, length);
#else
    ithWriteRegA(SD_REG_LENGTH_H, (uint32_t)((length >> 8) & 0xFF));
    ithWriteRegA(SD_REG_LENGTH_L, (uint32_t)(length & 0xFF));
#endif
}

static inline uint32_t SD_IsErrorReg(void)
{
    uint32_t reg = ithReadRegA(SD_REG_STS1);
    return (reg & SD_ERROR);
}

#if !defined(SD_NEW_HW)
static inline bool SD_IsCrcErrorReg(void)
{
    uint32_t reg = ithReadRegA(SD_REG_STS1);
    return (reg & SD_MSK_CRC_ERROR) ? true : false;
}
#endif

#if defined(SD_IRQ_ENABLE)
void dma_isr(int ch, void *arg, uint32_t status);
void sd_isr(void *arg);
#endif // #if defined(SD_IRQ_ENABLE)

int SD_SendCmdReg(
    SD_CTXT *ctxt,
    uint8_t command,
    uint8_t arg3,
    uint8_t arg2,
    uint8_t arg1,
    uint8_t arg0,
    uint8_t condition);

void SD_SendCmdNoWaitReg(
    SD_CTXT *ctxt,
    uint8_t command,
    uint8_t arg3,
    uint8_t arg2,
    uint8_t arg1,
    uint8_t arg0,
    uint8_t condition);

int SD_WaitSendCmdReadyReg(SD_CTXT *ctxt);

int SDW_WaitWrapReadyReg(SD_CTXT *ctxt);

void SD_ReadResponseReg(uint16_t readByte);

static inline uint32_t SD_ReadDataReg(void)
{
    return ithReadRegA(SDW_REG_DATA_PORT);
}

static inline void SD_WriteDataReg(uint32_t data)
{
    ithWriteRegA(SDW_REG_DATA_PORT, data);
}

int SD_WaitFifoFullReg(void);

int SD_WaitFifoEmptyReg(void);

static inline void SDW_SetWrapCtrlReg(SD_CTXT *ctxt, uint8_t dir, uint8_t dma)
{
    uint32_t value = SDW_MSK_WRAP_FIRE;
    uint32_t mask  = SDW_MSK_DATA_IN | SDW_MSK_HW_HANDSHAKING | SDW_MSK_WRAP_FIRE;

    if (dir == SDW_DATA_IN)
        value |= SDW_MSK_DATA_IN;
    if (dma == SDW_DMA)
    {
        value       |= SDW_MSK_HW_HANDSHAKING;
        ctxt->flags |= SD_FLAGS_DATA_DMA;
#if defined(SD_NEW_HW) && defined(SD_IRQ_ENABLE)
        ctxt->flags |= SD_FLAGS_WRAP_INTR;
        SD_WrapIntrEnable();
#else
        ctxt->flags |= SD_FLAGS_WRAP; /** check wrap ready by polling */
#endif
    }
    else
    {
        ctxt->flags &= ~SD_FLAGS_DATA_DMA;
        SD_WrapIntrDisable();
        /** if without dma, we will check wrap ready by SDW_WaitWrapReadyReg() after read/write data by PIO */
        //ctxt->flags |= SD_FLAGS_WRAP;
    }

    ithWriteRegMaskA(SDW_REG_WRAP_CTRL, value, mask);
}

static inline bool SD_IsCardInserted(int index)
{
#if defined(SD_DETECT_IRQ)
    extern bool sd0_insert;
    extern bool sd1_insert;
    return (index == 0) ? sd0_insert : sd1_insert;
#else
    return ithCardInserted((index == SD_0) ? ITH_CARDPIN_SD0 : ITH_CARDPIN_SD1);
#endif
}

static inline bool SD_IsCardLocked(int index)
{
    return ithCardLocked((index == SD_0) ? ITH_CARDPIN_SD0 : ITH_CARDPIN_SD1);
}

static inline void SD_PowerReset(int index, int delay_ms)
{
    switch (index)
    {
    case SD_0:
        ithStorageUnSelect(ITH_STOR_SD);
        ithCardPowerOff(ITH_CARDPIN_SD0);
        usleep(delay_ms * 1000);
        ithCardPowerOn(ITH_CARDPIN_SD0);
        break;
    case SD_1:
        ithStorageUnSelect(ITH_STOR_SD1);
        ithCardPowerOff(ITH_CARDPIN_SD1);
        usleep(delay_ms * 1000);
        ithCardPowerOn(ITH_CARDPIN_SD1);
        break;
    }
}

static inline void SD_StorageSelect(int index)
{
    ithStorageSelect((index == SD_0) ? ITH_STOR_SD : ITH_STOR_SD1);
}

void SD_DumpReg(void);

#ifdef __cplusplus
}
#endif

#endif //SD_HW_H
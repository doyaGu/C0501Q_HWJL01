/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * Used as SD/MMC related function header file.
 *
 * @author Irene Lin
 */
#ifndef SD_H
#define SD_H

//=============================================================================
//                              Constant Definition
//=============================================================================
typedef enum SD_FLAGS_TAG
{
    SD_FLAGS_DMA_ENABLE          = (0x00000001 << 0),
    SD_FLAGS_WRITE_PROTECT       = (0x00000001 << 1),
    SD_FLAGS_SUPPORT_CMD8        = (0x00000001 << 2),
    SD_FLAGS_CARD_SDHC           = (0x00000001 << 3),
    SD_FLAGS_CARD_SD             = (0x00000001 << 4),
    SD_FLAGS_CARD_MMC            = (0x00000001 << 5),
    SD_FLAGS_CARD_MMC4           = (0x00000001 << 6),
    SD_FLAGS_SUPPORT_SWITCH_FUNC = (0x00000001 << 7),
    SD_FLAGS_CARD_MMC_HC         = (0x00000001 << 8),
    SD_FLAGS_MMC_8_BIT_BUS       = (0x00000001 << 9),
    SD_FLAGS_MMC_4_BIT_BUS       = (0x00000001 << 10),
    SD_FLAGS_SD_HIGH_SPEED       = (0x00000001 << 11),
    SD_FLAGS_INIT_FAIL           = (0x00000001 << 12),
    SD_FLAGS_INIT_READY          = (0x00000001 << 13),
    SD_FLAGS_SD2                 = (0x00000001 << 14),
    SD_FLAGS_LAST_LEAVE          = (0x00000001 << 15),
    SD_FLAGS_WRAP                = (0x00000001 << 16),
    SD_FLAGS_DATA_IN             = (0x00000001 << 17),
    SD_FLAGS_DATA_DMA            = (0x00000001 << 18),
#if defined(SD_IRQ_ENABLE)
    SD_FLAGS_WRAP_INTR           = (0x00000001 << 24),
    SD_FLAGS_IRQ_SD              = (0x00000001 << 25),
    SD_FLAGS_IRQ_DMA_END         = (0x00000001 << 26),
    SD_FLAGS_IRQ_SD_END          = (0x00000001 << 27),
    SD_FLAGS_IRQ_DMA_ERROR       = (0x00000001 << 28),
    SD_FLAGS_IRQ_SD_ERROR        = (0x00000001 << 29),
    SD_FLAGS_IRQ_WRAP_END        = (0x00000001 << 30),
#endif
    SD_FLAGS_MAX                 = (0x00000001 << 31),
} SD_FLAGS;

#if defined(SD_IRQ_ENABLE)
    #define SD_IRQ_FLAGS (SD_FLAGS_IRQ_SD | \
                          SD_FLAGS_WRAP | \
                          SD_FLAGS_WRAP_INTR | \
                          SD_FLAGS_DATA_DMA | \
                          SD_FLAGS_IRQ_DMA_END | \
                          SD_FLAGS_IRQ_SD_END | \
                          SD_FLAGS_IRQ_DMA_ERROR | \
                          SD_FLAGS_IRQ_SD_ERROR | \
                          SD_FLAGS_IRQ_WRAP_END)
#endif

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct SD_CTXT_TAG
{
    int               index;
    volatile uint32_t flags;
#if defined(SD_IRQ_ENABLE)
    uint32_t          intr_flags; // for interrupt safe
#endif
    uint32_t          totalSectors;
    long              timeout;
    int               dmaCh;

    uint8_t           cardBusWidth;
    uint8_t           clockDiv;
    uint8_t           rcaByte3;
    uint8_t           rcaByte2;
    uint8_t           cid[16];
    uint8_t           cmd;
    volatile uint8_t  intrErr;
    uint8_t           reserved[2];
} SD_CTXT;

//=============================================================================
//							Funtion Declaration
//=============================================================================

/** common.c */
int SDMMC_StartUp(SD_CTXT *ctxt);

int SDMMC_GetCapacity(SD_CTXT *ctxt);

int SDMMC_TransferState(SD_CTXT *ctxt);

int SDMMC_Switch(SD_CTXT *ctxt);

void SDMMC_DmaRead(SD_CTXT *ctxt, uint8_t *data, uint32_t size);

void SDMMC_DmaWrite(SD_CTXT *ctxt, uint8_t *data, uint32_t size);

int SDMMC_ReadData(SD_CTXT *ctxt, uint8_t *data, uint32_t totalSize);

int SDMMC_WriteData(SD_CTXT *ctxt, uint8_t *data, uint32_t totalSize);

int SDMMC_ReadMultiSector(SD_CTXT *ctxt, uint32_t sector, uint32_t count, void *data);

int SDMMC_WriteMultiSector(SD_CTXT *ctxt, uint32_t sector, uint32_t count, void *data);

/** sd.c */
int SD_StartUp(SD_CTXT *ctxt);

int SD_GetRca(SD_CTXT *ctxt);

int SD_Switch(SD_CTXT *ctxt);

/** mmc.c */
int MMC_StartUp(SD_CTXT *ctxt);

int MMC_SetRca(SD_CTXT *ctxt);

int MMC_Switch(SD_CTXT *ctxt);

#endif
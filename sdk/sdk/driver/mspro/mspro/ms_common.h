/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * Used as MS/MS pro common used function header file.
 *
 * @author Irene Lin
 */
#ifndef MS_COMMON_H
#define MS_COMMON_H

#include "mspro/ms_type.h"
#include "mspro/ms_reg.h"

extern MEM_STICK_CARD_STRUCT MSCard;

MMP_BOOL SMSC_GetWriteProtectStatus(void);

MMP_INT SMSC_GetMediaType(MS_MEDIA_TYPE *mediaType);

MMP_INT SMSC_SetRwRegAddress(MMP_UINT8 W_len, MMP_UINT8 W_addr, MMP_UINT8 R_len, MMP_UINT8 R_addr);

MMP_INT SMSC_InterfaceSwitch(MS_INTERFACE mode);

extern MS_READ_PAGEDATA  SMSC_ReadPageData;
extern MS_WRITE_PAGEDATA SMSC_WritePageData;
MMP_INT SMSC_ReadPageDataDma(MMP_UINT8 *data);
MMP_INT SMSC_ReadPageDataCpu(MMP_UINT8 *data);
MMP_INT SMSC_WritePageDataDma(MMP_UINT8 *data);
MMP_INT SMSC_WritePageDataCpu(MMP_UINT8 *data);

MMP_INT SMSC_GetIntStatus(MMP_UINT32 *status);

MMP_INT SMSC_GetIntStatusEx(MMP_UINT32 *status);

MMP_INT SMSC_WaitIntStatus(MMP_UINT32 *status, MMP_UINT32 mask, MMP_UINT32 sTimeout);

#define MS_READ_BURST_SIZE  32
#define MS_WRITE_BURST_SIZE 16

static MMP_INLINE void MSC_ReadDmaPreset(MMP_UINT8 *data)
{
    if (MSCard.dmaCh >= 0)
    {
        if (!(MSCard.flags & MS_FLAGS_LAST_IS_READ))
        {
            ithDmaSetBurst(MSCard.dmaCh, ITH_DMA_BURST_32);
            ithDmaSetSrcAddr(MSCard.dmaCh, MS_AHB_DATA_PORT);
            ithDmaSetRequest(MSCard.dmaCh, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_MS, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM);
            ithDmaSetSrcParams(MSCard.dmaCh, ITH_DMA_WIDTH_32, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
        }
        ithDmaSetDstAddr(MSCard.dmaCh, (uint32_t)data);
        ithDmaSetDstParams(MSCard.dmaCh, dmaWidthMap[((uint32_t)data) & 0x3], ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
        //ithDmaStart(MSCard.dmaCh);

        MSCard.flags |= MS_FLAGS_LAST_IS_READ;
    }
}

static MMP_INLINE void MSC_WriteDmaPreset(MMP_UINT8 *data)
{
    if (MSCard.dmaCh >= 0)
    {
        if (MSCard.flags & MS_FLAGS_LAST_IS_READ)
        {
            ithDmaSetBurst(MSCard.dmaCh, ITH_DMA_BURST_16);
            ithDmaSetDstAddr(MSCard.dmaCh, MS_AHB_DATA_PORT);
            ithDmaSetRequest(MSCard.dmaCh, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_MS);
            ithDmaSetDstParams(MSCard.dmaCh, ITH_DMA_WIDTH_32, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
        }

        ithDmaSetSrcAddr(MSCard.dmaCh, (uint32_t)data);
        ithDmaSetSrcParams(MSCard.dmaCh, dmaWidthMap[((uint32_t)data) & 0x3], ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
        //ithDmaStart(MSCard.dmaCh);

        MSCard.flags &= ~MS_FLAGS_LAST_IS_READ;
    }
}

#if defined(MS_WORKAROUND)
MMP_INT MSC_WrStart(void);
MMP_INT MSC_WrEnd(void);
#endif

#if defined(MS_DMA_IRQ_ENABLE)
void ms_dma_isr(void *arg, MMP_UINT32 status);

extern MMP_EVENT ms_dma_event;
static MMP_INLINE MMP_INT MS_WaitDmaComplete(void)
{
    MMP_INT result;

    result = SYS_WaitEvent(ms_dma_event, 100);
    if (result) /** timeout */
        result = ERROR_MS_IRQ_DMA_TIMEOUT;

    #if 0
    if (result)
    {
        MMP_UINT32 reg = 0;
        AHB_ReadRegister(MS_REG_STATUS, &reg);
        LOG_ERROR "MS_WaitDmaComplete() return error code 0x%08X, 0x14 = 0x%08X \n", result, reg LOG_END
    }
    #endif
    return result;
}

#else // #if defined(MS_DMA_IRQ_ENABLE)

static MMP_INLINE MMP_INT MS_WaitDmaComplete(void)
{
    MMP_INT result     = 0;
    MMP_INT timeoutCnt = 8000;

    while (ithDmaIsBusy(MSCard.dmaCh) && --timeoutCnt);
    if (timeoutCnt <= 0)
        result = ERROR_MS_IRQ_DMA_TIMEOUT;

    #if 0
    if (result)
    {
        MMP_UINT32 reg = 0;
        AHB_ReadRegister(MS_REG_STATUS, &reg);
        LOG_ERROR "MS_WaitDmaComplete() return error code 0x%08X, 0x14 = 0x%08X \n", result, reg LOG_END
    }
    #endif
    return result;
}

#endif // #if defined(MS_DMA_IRQ_ENABLE)

#endif
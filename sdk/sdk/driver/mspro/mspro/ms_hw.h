/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * Used as memory stick HW configure header file.
 *
 * @author Irene Lin
 */

#ifndef MS_HW_H
#define MS_HW_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "mspro/config.h"
#include "mspro/ms_reg.h"
#include "mspro/ms_type.h"

extern MEM_STICK_CARD_STRUCT MSCard;

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//							Funtion Declaration
//=============================================================================
void MS_PowerOnReg(void);

void MS_PowerDownReg(void);

MMP_INT MS_ResetReg(void);

MMP_INT MS_WaitFifoFullReg(void);

MMP_INT MS_WaitFifoEmptyReg(void);

//=============================================================================
/** 0x0014h
 * Is Card Insert?
 */
//=============================================================================
static MMP_INLINE
MMP_BOOL MS_IsCardInsertReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(MS_REG_STATUS, &value);
    return (MMP_BOOL)((value & MS_MSK_CARD_DETECT) ? MMP_FALSE : MMP_TRUE);
}

void MS_SetSerialClkDivideReg(void);

MMP_UINT32 MS_GetFifoSizeReg(void);

MMP_UINT32 MS_GetFifoWidthReg(void);

//=============================================================================
/** 0x0004h
 * Set Control register
 */
//=============================================================================
static MMP_INLINE
void MS_SetControlReg(
    MMP_UINT32 value)
{
#if defined(MS_SONY_2_STATE_ERROR)
    value |= MS_MSK_SLEEP_WAKEUP;
#endif

#if 0
    if ( (MSCard.chipId == CHIP_ID_220) && (MSCard.chipVersion == CHIP_REV_A1) )
    {
        AHB_WriteRegister(MS_REG_CONTROL, value);
    }
    else if ( (MSCard.chipId == CHIP_ID_220) && (MSCard.chipVersion >= CHIP_REV_A2) )
    {
        /** MS_MSK_INT_CRC_CHECK_EN is hardware ECO bit */
        if (MSCard.flags & MS_FLAGS_PARALLEL_INTERFACE)
            AHB_WriteRegister(MS_REG_CONTROL, (value | MS_MSK_INT_CRC_CHECK_EN));
        else
            AHB_WriteRegister(MS_REG_CONTROL, (value & ~MS_MSK_INT_CRC_CHECK_EN));
    }
    else if (MSCard.chipId >= CHIP_ID_230)
#endif
    {
#if defined(MS_IRQ_ENABLE)
        value |= MS_MSK_INT_CMD_COMPLETE_EN;
#endif
        AHB_WriteRegister(MS_REG_CONTROL, value);
    }
}

void MS_ClearInterruptReg(MMP_UINT32 interrupt);

//=============================================================================
/** 0x000Ch
 * Set TPC command.
 */
//=============================================================================
static MMP_INLINE
void MS_SetTpcCmdReg(
    MMP_UINT32 tpcCmd,
    MMP_UINT32 dataSize,
    MMP_UINT32 dataLength)
{
    MMP_UINT32 value = ( tpcCmd |
                         ((dataSize << MS_SHT_CMD_DATA_SIZE) & MS_MSK_CMD_DATA_SIZE) |
                         dataLength );
    if (MSCard.flags & MS_FLAGS_PARALLEL_INTERFACE)
    {
        value |= MS_MSK_PARALLEL;
    }
#if 0
    if (MSCard.chipId >= CHIP_ID_230)
#endif
    {
        if (MSCard.flags & MS_FLAGS_PARALLEL_INTERFACE)
            value |= MS_MSK_CLK_INVERT;
    }
    AHB_WriteRegister(MS_REG_COMMAND, value);
}

//=============================================================================
/** 0x0010h
 * Write DATA register
 */
//=============================================================================
static MMP_INLINE
void MS_WriteDataReg(
    MMP_UINT32 value)
{
    AHB_WriteRegister(MS_REG_DATA, value);
}

//=============================================================================
/** 0x0010h
 * Read DATA register
 */
//=============================================================================
static MMP_INLINE
MMP_UINT32 MS_ReadDataReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(MS_REG_DATA, &value);
    return value;
}

#if defined(MS_IRQ_ENABLE)
void ms_isr(void *data);
void ms_isr2(void *data);
#endif

MMP_INT MS_WaitCmdCompleteReg(void);

MMP_INT MS_GetIntStatusReg(MMP_UINT32 *intStatus, MMP_UINT32 sTimeout);

#ifdef __cplusplus
}
#endif

#endif //MS_HW_H
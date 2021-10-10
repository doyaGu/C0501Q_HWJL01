/*
 * Copyright (c) 2012 ITE Technology Corp. All Rights Reserved.
 */
/** @file encoder_hardware.h
 *
 * @author
 */
 
#ifndef _ENCODER_HARDWARE_H_
#define _ENCODER_HARDWARE_H_

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================

#include "encoder/encoder_error.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
typedef enum VIDEO_HW_STATUS_TAG
{
    VIDEO_HW_STATUS_IDLE = 0,
    VIDEO_HW_STATUS_BUSY = 1,
    VIDEO_HW_STATUS_HALT = 2,
    VIDEO_HW_STATUS_ERROR = 3
} VIDEO_HW_STATUS;

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

void
VPU_WriteRegister(
    MMP_UINT32 destAddress,
    MMP_UINT32 data);
    
MMP_UINT32
VPU_ReadRegister(
    MMP_UINT32 destAddress);
    
void 
BitLoadFirmware(
    MMP_UINT8* codeBase, 
    const MMP_UINT16 *codeWord, 
    MMP_UINT32 codeSize);
    
MMP_BOOL 
WaitIdle(
    MMP_UINT32 timeout);
    
void 
IssueCommand(
    MMP_UINT32 instanceNum, 
    MMP_UINT32 cmd);
    
void
encoder_hardware_SetBufAddr_Reg(
    MMP_UINT32 reg,
    MMP_UINT8* pAddr);

void
encoder_hardware_SetGDI_Reg(
    MMP_UINT32 baseAddr);    

void 
VPU_EnableClock(
    void);

void 
VPU_DisableClock(
    void);
void 
VPU_Reset(
    void);        
#ifdef __cplusplus
}
#endif

#endif //_ENCODER_HARDWARE_H_

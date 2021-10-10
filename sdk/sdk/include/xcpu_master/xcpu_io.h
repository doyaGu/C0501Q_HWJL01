/*
 * Copyright (c) 2013 ITE Technology Corp. All Rights Reserved.
 */
/** @file xcpu_io.h
 *
 *
 * @version 0.1
 */

#ifndef XCPU_IO_H
#define XCPU_IO_H

#include "itx.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
	
//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

#if (!defined(ITX_BUS_TYPE) || (ITX_BUS_TYPE != ITX_BUS_UART))
MMP_UINT16
xCpuIO_ReadRegister(
    MMP_UINT16 addr);

void
xCpuIO_WriteRegister(
    MMP_UINT16 addr,
    MMP_UINT16 data);

void
xCpuIO_ReadMemory(
    MMP_UINT32 destSysRamAddress,
    MMP_UINT32 srcVRamAddress,
    MMP_UINT32 sizeInByte);

void
xCpuIO_WriteMemory(
    MMP_UINT32 destVRamAddress,
    MMP_UINT32 srcSysRamAddress,
    MMP_UINT32 sizeInByte);

void
xCpuIO_ReadMemoryUInt16(
    MMP_UINT32 destSysRamAddress,
    MMP_UINT32 srcVRamAddress,
    MMP_UINT32 sizeInByte);

void
xCpuIO_WriteMemoryUInt16(
    MMP_UINT32 destVRamAddress,
    MMP_UINT32 srcSysRamAddress,
    MMP_UINT32 sizeInByte);

void
xCpuIO_ReadMemoryUInt32(
    MMP_UINT32 destSysRamAddress,
    MMP_UINT32 srcVRamAddress,
    MMP_UINT32 sizeInByte);

void
xCpuIO_WriteMemoryUInt32(
    MMP_UINT32 destVRamAddress,
    MMP_UINT32 srcSysRamAddress,
    MMP_UINT32 sizeInByte);
#endif

#ifdef __cplusplus
}
#endif

#endif


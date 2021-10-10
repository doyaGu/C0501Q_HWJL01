/*
 * Copyright (c) 2007 SMedia technology Corp. All Rights Reserved.
 */
/** @file mmp_tsi.h
 * TS output.
 *
 * @author Steven Hsiao
 * @version 0.1
 */
#ifndef MMP_TSO_H
#define MMP_TSO_H


#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN32_WCE)
	#if defined(TSO_EXPORTS)
		#define TSO_API __declspec(dllexport)
	#else
		#define TSO_API __declspec(dllimport)
	#endif
#else
	#define TSO_API extern
#endif /* defined(_WIN32) || defined(_WIN32_WCE) */

//=============================================================================
//                              Include Files
//=============================================================================
//#include "mmp_types.h"
#include "ite/ith.h"

#define MMP_RESULT      uint32_t
#define MMP_ULONG       uint32_t
#define MMP_UINT        uint32_t
#define MMP_UINT32      uint32_t
#define MMP_UINT16      uint16_t
#define MMP_UINT8       uint8_t
#define MMP_FLOAT       float
#define MMP_INT32       int32_t
#define MMP_INT16       int16_t
#define MMP_INLINE      inline
#define MMP_SUCCESS     0
#define MMP_NULL        0
#define MMP_BOOL        bool
#define MMP_TRUE        true
#define MMP_FALSE       false

// Host
#define HOST_ReadRegister(add, data)               (*data = ithReadRegH(add))
#define HOST_WriteRegister(add, data)              ithWriteRegH(add, data)
#define HOST_WriteRegisterMask(add, data, mark)    ithWriteRegMaskH(add, data, mark)
// AMBA 
#define AHB_ReadRegister(add, data)                 (*data = ithReadRegA(add))
#define AHB_WriteRegister(add, data)                ithWriteRegA(add, data)
#define AHB_WriteRegisterMask(add, data, mark)      ithWriteRegMaskA(add, data, mark)

#define PalSleep(ms)                                usleep((ms*1000))

//=============================================================================
//                              Constant Definition
//=============================================================================
#define MMP_RESULT_SHIFT        16
#define MMP_TSO                 1
/**
 * Result codes
 */
#define MMP_TSO_BAD_PARAM           (MMP_TSO << MMP_RESULT_SHIFT | 0x1)
#define MMP_TSO_OUT_OF_MEM          (MMP_TSO << MMP_RESULT_SHIFT | 0x2)
#define MMP_TSO_IS_DISABLED         (MMP_TSO << MMP_RESULT_SHIFT | 0x3)
#define MMP_TSO_ENGINE_BUSY         (MMP_TSO << MMP_RESULT_SHIFT | 0x4)
#define MMP_TSO_INIT_FAIL           (MMP_TSO << MMP_RESULT_SHIFT | 0x5)

//=============================================================================
//                              Function Declaration
//=============================================================================

MMP_RESULT
mmpTsoInitialize(
    MMP_UINT32 padStart,
    MMP_UINT32 videoPid,
    MMP_FLOAT  pcrClock,
    MMP_UINT32 startPcrValue,
    MMP_UINT8* pExternalBuffer,
    MMP_INT32  bufferSize,
    MMP_BOOL   bInjectPcr);

TSO_API MMP_RESULT
mmpTsoTerminate(
    void);

TSO_API MMP_RESULT
mmpTsoEnable(
    void);

TSO_API MMP_RESULT
mmpTsoDisable(
    void);

TSO_API MMP_RESULT
mmpTsoWrite(
    MMP_UINT8*  pInputBuffer,
    MMP_ULONG   bufferSize);

MMP_RESULT
mmpTsoWriteWithoutCopy(
    MMP_ULONG   bufferSize);

TSO_API MMP_UINT32
mmpTsoGetStatus(
    void);

#ifdef __cplusplus
}
#endif

#endif

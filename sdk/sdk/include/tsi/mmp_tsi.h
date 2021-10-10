/*
 * Copyright (c) 2007 ITE technology Corp. All Rights Reserved.
 */
/** @file mmp_tsi.h
 * Used to receive data through the transport stream interface (TSI).
 *
 * @author I-Chun Lai
 * @version 0.1
 * @example
 *  #define   MAX_READ_SIZE   0xFFEE00
 *
 *  void main()
 *  {
 *      uint8_t*  sysBuf  = MMP_NULL;
 *      uint32_t  totalReadSize = 0;
 *      uint32_t  result;
 *      uint32_t  size = 0;
 *
 *      mmpTsiInitialize();
 *      mmpTsiEnable();
 *
 *      sysBuf = (uint8_t*)SYS_Malloc(MAX_READ_SIZE);
 *
 *      do
 *      {
 *          result = mmpTsiReceive(
 *              (void*)(sysBuf + totalReadSize),
 *              MAX_READ_SIZE - totalReadSize,
 *              &size);
 *
 *          if (result == MMP_SUCCESS && size > 0)
 *          {
 *              totalReadSize += size;
 *              printf("receive %d bytes\n", size);
 *          }
 *      } while(totalReadSize < MAX_READ_SIZE);
 *
 *      mmpTsiDisable();
 *      mmpTsiTerminate();
 *  }
 */
#ifndef MMP_TSI_H
#define MMP_TSI_H


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include "ite/itp.h"

#define TSI_API extern
//=============================================================================
//                              Include Files
//=============================================================================
#ifndef bool
    #define bool uint8_t
#endif
#ifndef false
    #define false 0
#endif
#ifndef true
    #define true 1
#endif
//=============================================================================
//                              Constant Definition
//=============================================================================
#define MMP_RESULT_SHIFT            16
#define MMP_TSI                     1
#define MAX_TSI_COUNT               2

/**
 * Result codes
 */
#define MMP_TSI_BAD_PARAM           (MMP_TSI << MMP_RESULT_SHIFT | 0x1)
#define MMP_TSI_OUT_OF_MEM          (MMP_TSI << MMP_RESULT_SHIFT | 0x2)
#define MMP_TSI_IS_DISABLED         (MMP_TSI << MMP_RESULT_SHIFT | 0x3)
#define MMP_TSI_ENGINE_BUSY         (MMP_TSI << MMP_RESULT_SHIFT | 0x4)
#define MMP_TSP_IS_NONINITED        (MMP_TSI << MMP_RESULT_SHIFT | 0x5)

#define TSI_TOTAL_FILTER_COUNT      (32)
#define TSI_MAX_PMT_FILTER_COUNT    (23)

// The index of PID filter table. However, due to more than one active PMT pids
// in a specific frequency channel, the index of PMT here is just the first PMT
// index.
typedef enum TSI_PID_FILTER_INDEX_TAG
{
    TSI_PID_FILTER_PAT_INDEX        = 0,
    TSI_PID_FILTER_SDT_INDEX,
    TSI_PID_FILTER_EIT_INDEX,
    TSI_PID_FILTER_TOT_TDT_INDEX,
    TSI_PID_FILTER_NIT_INDEX,
    TSI_PID_FILTER_PMT_INDEX,
    TSI_PID_FILTER_TELETEXT_INDEX   = (TSI_TOTAL_FILTER_COUNT - 4),
    TSI_PID_FILTER_SUBTITLE_INDEX   = (TSI_TOTAL_FILTER_COUNT - 3),
    TSI_PID_FILTER_AUDIO_INDEX      = (TSI_TOTAL_FILTER_COUNT - 2),
    TSI_PID_FILTER_VIDEO_INDEX      = (TSI_TOTAL_FILTER_COUNT - 1)
} TSI_PID_FILTER_INDEX;

//=============================================================================
//                              Function Declaration
//=============================================================================

TSI_API uint32_t
mmpTsiInitialize(
    uint32_t tsiId);

TSI_API uint32_t
mmpTsiTerminate(
    uint32_t tsiId);

TSI_API uint32_t
mmpTsiEnable(
    uint32_t tsiId);

TSI_API uint32_t
mmpTsiDisable(
    uint32_t tsiId);

TSI_API uint32_t
mmpTsiReset(
    uint32_t tsiId);

#if 0 //def WIN32
TSI_API uint32_t
mmpTsiReceive(
    uint32_t  tsiId,
    void*       buffer, 
    uint32_t   maxSize,
    uint32_t*  actualSize);
#else
TSI_API uint32_t
mmpTsiReceive(
    uint32_t  tsiId,
    uint8_t** ppOutBuffer, 
    uint32_t*  outSize);
#endif

TSI_API bool
mmpTsiIsPcrInterruptTriggered(
    uint32_t tsiId);

TSI_API uint32_t
mmpTsiGetPcrValue(
    uint32_t tsiId);

TSI_API uint16_t
mmpTsiGetPcrPid(
    uint32_t tsiId);

TSI_API uint32_t
mmpTsiGetPCRCnt(
    uint32_t tsiId);

TSI_API void
mmpTsiClearInterrupt(
    uint32_t tsiId);

TSI_API uint32_t
mmpTsiEnablePcrPidFilter(
    uint32_t tsiId,
    uint16_t pid);

TSI_API uint32_t
mmpTsiDisablePcrPidFilter(
    uint32_t tsiId);

TSI_API uint32_t
mmpTsiEnablePCRCntThreshold(
    uint32_t tsiId,
    uint16_t threshold);

TSI_API uint32_t
mmpTsiDisablePCRCntThreshold(
    uint32_t tsiId);

TSI_API uint32_t
mmpTsiUpdatePidFilter(
    uint32_t              tsiId,
    uint32_t              pid,
    TSI_PID_FILTER_INDEX    pidIndex);

TSI_API uint32_t
mmpTsiResetPidFilter(
    uint32_t  tsiId);


#ifdef __cplusplus
}
#endif

#endif

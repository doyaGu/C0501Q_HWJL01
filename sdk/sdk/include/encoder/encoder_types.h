/*
 * Copyright (c) 2012 ITE Technology Corp. All Rights Reserved.
 */
/** @file encoder_types.h
 *
 * @author
 */

#ifndef _ENCODER_TYPES_H_
#define _ENCODER_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "ite/ith.h"
#include "ite/mmp_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define MMP_ERROR_OFFSET 16     /**< Error offset */

typedef enum MEM_STATUS_TAG
{
    MEM_STATUS_SUCCESS,                             // 0
    MEM_STATUS_ERROR_OVER_MICROP_HEAP_SIZE,         // 1
    MEM_STATUS_ERROR_NONE_FREE_MEMORY,              // 2
    MEM_STATUS_ERROR_ALLOCATE_ZERO_MEMORY,          // 3
    MEM_STATUS_ERROR_ADDRESS_IS_ZERO,               // 4
    MEM_STATUS_ERROR_ADDRESS_NOT_FOUND,             // 5
    MEM_STATUS_ERROR_RELEASE_FREE_MEMORY,           // 6
    MEM_STATUS_ERROR_INITIALIZE_MEMORY,             // 7
    MEM_TOTOAL_STATUS
} MEM_STATUS;

#define PalMemcpy memcpy
#define PalMemset memset
#define PalHeapAlloc(a, size)                  malloc(size)
#define PalHeapFree(a, size)                   free(size)
#define PalSleep(ms)                           usleep((ms * 1000))
#define MMP_Sleep(ms)                          usleep((ms * 1000))

#define MEM_Allocate(size, a)                  itpVmemAlloc(size)
#define MEM_Release(x)                         itpVmemFree((uint32_t)x)
#define HOST_GetVramBaseAddress()              0
#define SYS_Malloc    malloc
#define SYS_Free      free
#define SYS_MemorySet memset

#define HOST_WriteBlockMemory(dst, src, size)  ithWriteVram(dst, (const void *)src, size)
#define HOST_ReadBlockMemory(dst, src, size)   ithReadVram((void *)dst, src, size
#define HOST_WriteRegister(reg, val)           ithWriteRegH(reg, val)
#define HOST_WriteRegisterMask(reg, val, mask) ithWriteRegMaskH(reg, val, mask)
#define HOST_ReadRegister(reg, pval)           (*pval) = ithReadRegH(reg)
#define AHB_WriteRegister(reg, val)            ithWriteRegA(reg, val)
#define AHB_WriteRegisterMask(reg, val, mask)  ithWriteRegMaskA(reg, val, mask)
#define AHB_ReadRegister(reg, pval)            (*pval) = ithReadRegA(reg)
#define HOST_GetVramBaseAddress()              0

#ifdef __cplusplus
}
#endif

#endif //_ENCODER_TYPES_H_
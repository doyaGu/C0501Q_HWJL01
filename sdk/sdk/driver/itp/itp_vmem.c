/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * Video Memory Management functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <malloc.h>
#include "itp_cfg.h"

#ifdef _WIN32

uint32_t itpVmemAlloc(size_t size)
{
    return ithVmemAlloc(ITH_ALIGN_UP(size, 32));
}

uint32_t itpVmemAlignedAlloc(size_t alignment, size_t size)
{
    return ithVmemAlignedAlloc(ITH_ALIGN_UP(alignment, 32), ITH_ALIGN_UP(size, 32));
}

void itpVmemFree(uint32_t addr)
{
    ithVmemFree(addr);
}

void itpVmemStats(void)
{
    ithVmemStats();
}

uint32_t itpWTAlloc(size_t size)
{
    return ithVmemAlloc(size);
}

void itpWTFree(uint32_t addr)
{
    ithVmemFree(addr);
}

void itpWTStats(void)
{
    ithVmemStats();
}

#else

uint32_t itpVmemAlloc(size_t size)
{
    return (uint32_t) memalign(64, ITH_ALIGN_UP(size, 32));
}

uint32_t itpVmemAlignedAlloc(size_t alignment, size_t size)
{
    return (uint32_t) memalign(ITH_ALIGN_UP(alignment, 32), ITH_ALIGN_UP(size, 32));
}

void itpVmemFree(uint32_t addr)
{
    free((void*) addr);
}

void itpVmemStats(void)
{
    malloc_stats();
}

extern uint32_t __wt_base;
#define ITP_WT_BASE (uint32_t)&__wt_base

uint32_t itpWTAlloc(size_t size)
{
#ifdef CFG_CPU_WB
    return ITP_WT_BASE + ithVmemAlloc(size);
#else
    return (uint32_t) memalign(64, size);
#endif
}

void itpWTFree(uint32_t addr)
{
#ifdef CFG_CPU_WB
    ithVmemFree(addr - ITP_WT_BASE);
#else
    free((void*) addr);
#endif
}

void itpWTStats(void)
{
#ifdef CFG_CPU_WB
    ithVmemStats();
#else
    malloc_stats();
#endif
}

#endif // _WIN32

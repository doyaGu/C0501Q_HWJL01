/*
 * Copyright (c) 2011 ITE Technology Corp. All Rights Reserved.
 *
 * @description Used as System Time Clock in front panel control.
 * @file ${Program}/ith_stc.c
 * @author Irene Wang
 * @version 1.0.0
 */
#include "ite/ith.h"

static uint32_t stcBaseCountLo = 0;

uint64_t ithStcGetBaseClock64(STCInfo *pstc_info)
{
    uint32_t baseCount = ithStcGetBaseClock();
    uint64_t tempBaseCount;

    if(baseCount < stcBaseCountLo)
    {
        pstc_info->stcBaseCountHi++;
    }

    stcBaseCountLo = baseCount;
    //printf("[%s] hi=%u lo = %u\n", __FUNCTION__, pstc_info->stcBaseCountHi, stcBaseCountLo);
    tempBaseCount = ((uint64_t)pstc_info->stcBaseCountHi << 32) | stcBaseCountLo;
    tempBaseCount = (tempBaseCount * 9)/10;
    return tempBaseCount;
}


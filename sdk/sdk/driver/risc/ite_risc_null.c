/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * ITE RISC header File
 *
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "ite/ith.h"
#include "ite/itp.h"
#include "stdlib.h"
#include "string.h"
#include "ite/ite_risc.h"

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Enumeration Type Definition
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================
int
iteRiscInit(
    void)
{
    return 0;
}
    
int
iteRiscTerminate(
    void)
{
    return 0;
}

uint8_t*
iteRiscGetTargetMemAddress(
    int loadTarget)
{
    return 0;
}

int
iteRiscLoadData(
    int             loadTarget,
    uint8_t*        pData,
    int             dataSize)
{
    return ITE_RISC_OK_RESULT;
}

void
iteRiscFireCpu(
    int             targetCpu)
{
    return;
}

void
iteRiscResetCpu(
    int             targetCpu)
{
    return;
}

uint32_t
iteRiscGetProgramCounter(
    int         targetCpu)
{
    return;
}
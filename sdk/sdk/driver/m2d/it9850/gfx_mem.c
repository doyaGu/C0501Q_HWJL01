/*
 * Copyright (c) 2014 ITE Corp. All Rights Reserved.
 */
/** @file gfx_mem.c
 *  GFX memory API header file.
 *
 * @author Awin Huang
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include "ite/itp.h"
#include "gfx_mem.h"

//=============================================================================
//                              Compile Option
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================
uint8_t *
gfxVmemAlloc(
    uint32_t sizeInByte)
{
    return (uint8_t *)itpVmemAlloc(sizeInByte);
}

void
gfxVmemFree(
    uint8_t *ptr)
{
    itpVmemFree((uint32_t)ptr);
}

//=============================================================================
//                              Private Function Definition
//=============================================================================
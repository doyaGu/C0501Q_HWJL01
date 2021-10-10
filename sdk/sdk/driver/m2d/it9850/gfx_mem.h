/*
 * Copyright (c) 2014 ITE Corp. All Rights Reserved.
 */
/** @file gfx_mem.h
 *  GFX memory API header file.
 *
 * @author Awin Huang
 * @version 1.0
 */
#ifndef __TEMPLATE_H__
#define __TEMPLATE_H__

#include <stdint.h>

/**
 * DLL export API declaration for Win32.
 */

#ifdef __cplusplus
extern "C" {
#endif

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
//                              Public Function Declaration
//=============================================================================
/**
 * This routine is used to alloc memory.
 *
 * @param sizeInByte    The size of alloc memory.
 *
 * @return a valid pointer of alloc memory.
 */
uint8_t*
gfxVmemAlloc(
    uint32_t    sizeInByte);

/**
 * This routine is used to free memory.
 *
 * @param ptr    The pointer of alloc memory.
 *
 * @return void.
 */
void
gfxVmemFree(
    uint8_t*    ptr);

#ifdef __cplusplus
}
#endif

#endif // __TEMPLATE_H__

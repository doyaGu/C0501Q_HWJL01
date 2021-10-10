/*
 * Copyright (c) 2013 ITE Technology Corp. All Rights Reserved.
 */
/** @file decompress.h
 *
 * @version 0.1
 */

#include "itx.h" 
#if ITX_BOOT_TYPE == ITX_HOST_BOOT

#ifndef DECOMPRESS_H
#define DECOMPRESS_H


#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

int do_decompress(unsigned char *InBuf, unsigned char *outBuf);


#ifdef __cplusplus
}
#endif
#endif


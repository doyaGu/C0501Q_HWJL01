/*
 * Copyright (c) 2007 ITE Corp. All Rights Reserved.
 */
/** @file
 *  Use as SSP error code header file.
 *
 * @author Irene Lin
 */

#ifndef SSP_ERROR_H
#define SSP_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================

#define ERROR_SSP_BASE                  0xEE900000

#define ERROR_SSP_CREATE_DMA_FAIL       (ERROR_SSP_BASE + 0x0001)
#define ERROR_SSP_ALLOC_DMA_BUF_FAIL    (ERROR_SSP_BASE + 0x0002)
#define ERROR_SSP_FIFO_LENGTH_UNSUPPORT (ERROR_SSP_BASE + 0x0003)
#define ERROR_SSP_OVER_FIFO_COUNT       (ERROR_SSP_BASE + 0x0004)
#define ERROR_SSP_TRANSMIT_TIMEOUT      (ERROR_SSP_BASE + 0x0005)
#define ERROR_SSP_FIFO_READY_TIMEOUT    (ERROR_SSP_BASE + 0x0006)

#ifdef __cplusplus
}
#endif

#endif
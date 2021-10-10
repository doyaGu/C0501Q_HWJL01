/*
 * Copyright (c) 2012 ITE Technology Corp. All Rights Reserved.
 */
/** @file encoder_error.h
 *
 * @author
 */

#ifndef _ENCODER_ERROR_H_
#define _ENCODER_ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================

#include "encoder/encoder_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

typedef MMP_UINT32 VIDEO_ERROR_CODE;

#define VIDEO_ERROR_SUCCESS                             (MMP_RESULT_SUCCESS)
#define ERROR_VIDEO_BASE                                (MMP_MODULE_VIDEO << MMP_ERROR_OFFSET)

#define ERROR_ENCODER_SEMAPHORE_CREATE                  (ERROR_VIDEO_BASE + 0x0001)
#define ERROR_ENCODER_INIT_CONFLICT                     (ERROR_VIDEO_BASE + 0x0002)
#define ERROR_ENCODER_INSTANCE_CREATED                  (ERROR_VIDEO_BASE + 0x0003)
#define ERROR_ENCODER_REQ_BUF_NUM                       (ERROR_VIDEO_BASE + 0x0004)
#define ERROR_ENCODER_TIME_OUT                          (ERROR_VIDEO_BASE + 0x0005)
#define ERROR_ENCODER_MEMORY_ACCESS_VIOLATION           (ERROR_VIDEO_BASE + 0x0006)
#define ERROR_ENCODER_SEQ_INIT_FAILURE                  (ERROR_VIDEO_BASE + 0x0007)
#define ERROR_ENCODER_SET_FRAME_FAILURE                 (ERROR_VIDEO_BASE + 0x0008)
#define ERROR_ENCODER_PIC_RUN_FAILURE                   (ERROR_VIDEO_BASE + 0x0009)
#define ERROR_ENCODER_NO_INSTANCE                       (ERROR_VIDEO_BASE + 0x000A)
#define ERROR_ENCODER_MEM_REQUEST                       (ERROR_VIDEO_BASE + 0x000B)
#define ERROR_ENCODER_FRAME_WIDTH_HEIGHT                (ERROR_VIDEO_BASE + 0x000C)

#ifdef __cplusplus
}
#endif

#endif //_ENCODER_ERROR_H_

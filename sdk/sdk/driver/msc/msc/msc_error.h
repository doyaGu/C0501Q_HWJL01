/*
 * Copyright (c) 2007 ITE Corp. All Rights Reserved.
 */
/** @file
 *  Use as MSC error code header file.
 *
 * @author Irene Lin
 */

#ifndef MSC_ERROR_H
#define MSC_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================

#define ERROR_MSC_BASE                    0xC0000000               //(MMP_MODULE_MSC << MMP_ERROR_OFFSET)

#define ERROR_MSC_NO_DEVICE               (ERROR_MSC_BASE + 0x0001)
#define ERROR_MSC_INVALID_LUN             (ERROR_MSC_BASE + 0x0002)
#define ERROR_MSC_SW_RESET_FAIL           (ERROR_MSC_BASE + 0x0003)
#define ERROR_MSC_NOT_SUPPORT_DEVICE_TYPE (ERROR_MSC_BASE + 0x0004)
#define ERROR_MSC_WRITE_PROTECTED         (ERROR_MSC_BASE + 0x0005)
#define ERROR_MSC_CBI_SOFT_RESET_FAIL     (ERROR_MSC_BASE + 0x0006)
#define ERROR_MSC_DEVICE_NOT_EXIST        (ERROR_MSC_BASE + 0x0007)
#define ERROR_MSC_CREATE_SEMAPHORE_FAIL   (ERROR_MSC_BASE + 0x0008)
#define ERROR_MSC_INVALID_SECTOR          (ERROR_MSC_BASE + 0x0009)

#define NEEDS_RETRY                       0x2001

#ifdef __cplusplus
}
#endif

#endif
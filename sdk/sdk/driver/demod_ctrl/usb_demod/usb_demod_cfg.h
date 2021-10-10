/*
 * Copyright (c) 2012 ITE Corp. All Rights Reserved.
 */
/** @file
 *  Use as compile option header file.
 *
 * @author Barry Wu
 */
#ifndef	USBDEMOD_CONFIG_H
#define	USBDEMOD_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ite/itp.h"
// #include "ite/ite_usb.h"

// #include "../mmp_types.h"

#include "demod_ctrl_defs.h"

// error code
#define ERROR_USB_TUNER_BASE                                  0x0 //(MMP_MODULE_MSC << MMP_ERROR_OFFSET)

#define ERROR_USB_TUNER_NO_DEVICE                             (ERROR_USB_TUNER_BASE + 0x0001)
#define ERROR_USB_TUNER_INVALID_LUN                           (ERROR_USB_TUNER_BASE + 0x0002)
#define ERROR_USB_TUNER_SW_RESET_FAIL                         (ERROR_USB_TUNER_BASE + 0x0003)
#define ERROR_USB_TUNER_NOT_SUPPORT_DEVICE_TYPE               (ERROR_USB_TUNER_BASE + 0x0004)
#define ERROR_USB_TUNER_WRITE_PROTECTED                       (ERROR_USB_TUNER_BASE + 0x0005)
#define ERROR_USB_TUNER_CBI_SOFT_RESET_FAIL                   (ERROR_USB_TUNER_BASE + 0x0006)
#define ERROR_USB_TUNER_DEVICE_NOT_EXIST                      (ERROR_USB_TUNER_BASE + 0x0007)
#define ERROR_USB_TUNER_CREATE_SEMAPHORE_FAIL                 (ERROR_USB_TUNER_BASE + 0x0008)

//=============================================================================
//                              Compile Option
//=============================================================================


//=============================================================================
//                              Constant Definition
//=============================================================================
#define UT_BULK_TIMEOUT             10000
#define UT_ADSC_TIMEOUT             2000

#define USB_DEMOD_TRACE_ENABLE      0
//=============================================================================
//                              LOG definition
//=============================================================================


#ifdef __cplusplus
}
#endif

#endif

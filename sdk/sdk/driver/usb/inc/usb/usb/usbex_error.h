/*
 * Copyright (c) 2008 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * Used as USB HCD error code.
 *
 * @author Irene Lin
 */

#ifndef USBEX_ERROR_H
#define USBEX_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "errorno.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
/**
 *  USB HCD error code
 */
#define ERROR_USBEX_BASE                     0xC000

#define ERROR_USB_ALLOC_HCD_FAIL             (ERROR_USBEX_BASE + 0x0081)
#define ERROR_USB_ALLOC_QH_FAIL              (ERROR_USBEX_BASE + 0x0082)
#define ERROR_USB_ALLOC_QTD_FAIL             (ERROR_USBEX_BASE + 0x0083)
#define ERROR_USB_ALLOC_DEVICE_FAIL          (ERROR_USBEX_BASE + 0x0084)
#define ERROR_USB_ALLOC_SYS_MEM_FAIL         (ERROR_USBEX_BASE + 0x0085)
#define ERROR_USB_NOT_FIND_DEVCE_ID          (ERROR_USBEX_BASE + 0x0086)
#define ERROR_USB_TOO_MANY_INTERFACES        (ERROR_USBEX_BASE + 0x0087)
#define ERROR_USB_INVALID_DESCRIPTOR_LEN     (ERROR_USBEX_BASE + 0x0088)
#define ERROR_USB_TOO_MANY_ALTERNATE_SETTING (ERROR_USBEX_BASE + 0x0089)
#define ERROR_USB_TOO_MANY_ENDPOINT          (ERROR_USBEX_BASE + 0x008A)
#define ERROR_USB_BAD_PARAMS                 (ERROR_USBEX_BASE + 0x008B)
#define ERROR_USB_NO_USB_DRIVER_SUPPORT      (ERROR_USBEX_BASE + 0x008C)
#define ERROR_USB_ALLOC_PERIODIC_TABLE_FAIL  (ERROR_USBEX_BASE + 0x008D)
#define ERROR_USB_ALLOC_PT_SHADOW_FAIL       (ERROR_USBEX_BASE + 0x008E)
#define ERROR_USB_ALLOC_ITD_FAIL             (ERROR_USBEX_BASE + 0x008F)
#define ERROR_USB_ALLOC_SITD_FAIL            (ERROR_USBEX_BASE + 0x0090)
#define ERROR_USB_SERIOUS_ERROR              (ERROR_USBEX_BASE + 0x00BF)
#define ERROR_USB_SET_INVALID_INTERFACE      (ERROR_USBEX_BASE + 0x00C0)
#define ERROR_USB_ALLOC_DYMMY_QH_FAIL        (ERROR_USBEX_BASE + 0x00C1)

#ifdef __cplusplus
}
#endif

#endif // #ifndef USBEX_ERROR_H
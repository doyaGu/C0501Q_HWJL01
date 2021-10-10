/*
 * Copyright (c) 2012 ITE Technology Corp. All Rights Reserved.
 */
/** @file usb.h
 * Driver for USB Tuner compliant devices.
 * Main Header File
 *
 * @author Barry Wu
 */
#ifndef USB_TUNER_H
#define USB_TUNER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usb_demod_cfg.h"
//=============================================================================
//                              Constant Definition
//=============================================================================
#define USB_TUNER_STRING_LEN 32

//=============================================================================
//                              Structure Definition
//=============================================================================
struct usbtuner_data;

typedef int (*trans_cmnd)(struct usbtuner_data*);
typedef int (*trans_reset)(struct usbtuner_data*);

/** every device has one */
struct usbtuner_data 
{
    /* the device we're working with */
    struct usb_device*	pusb_dev;	 /* this usb_device */

    /* information about the device -- always good */
    uint8_t			vendor[32];
    uint8_t			product[32];
    uint8_t			serial[32];
    uint8_t			*transport_name;
    uint8_t			*protocol_name;
    uint8_t			subclass;
    uint8_t			protocol;
    uint8_t			max_lun;
    uint8_t         use_mode_sense6;

    /* information about the device  */
    uint8_t			ifnum;		 /* interface number   */
    uint8_t			ep_in;		 /* bulk in endpoint   */
    uint8_t			ep_out;		 /* bulk out endpoint  */
    struct usb_endpoint_descriptor *ep_int;	 /* interrupt endpoint */ 

    /* function pointers for this device */
    trans_cmnd		transport;	 /* transport function	   */
    trans_reset		transport_reset; /* transport device reset */

    void*           semaphore;
    uint8_t       in_rw_access;
    uint8_t       reserved[3];
};

// struct usb_demod_info
// {
//     uint8_t       type;
//     uint8_t       insert_flag;
// };

//=============================================================================
//                              Function Declaration
//=============================================================================

#ifdef __cplusplus
}
#endif


#endif

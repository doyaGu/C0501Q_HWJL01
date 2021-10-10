/*
 * Copyright (c) 2008 ITE Technology Corp. All Rights Reserved.
 */
/** @file usb.h
 * Driver for USB Mass Storage compliant devices.
 * Main Header File
 *
 * @author Irene Lin
 */
#ifndef MSC_USB_H
#define MSC_USB_H

#include "msc/scsi.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define USB_STOR_STRING_LEN 32

//=============================================================================
//                              Structure Definition
//=============================================================================
struct us_data;

typedef int (*trans_cmnd)(Scsi_Cmnd *, struct us_data *);
typedef int (*trans_reset)(struct us_data *);

/** every device has one */
struct us_data
{
    /* the device we're working with */
    struct usb_device *pusb_dev;     /* this usb_device */

    /* information about the device -- always good */
    MMP_UINT8 vendor[USB_STOR_STRING_LEN];
    MMP_UINT8 product[USB_STOR_STRING_LEN];
    MMP_UINT8 serial[USB_STOR_STRING_LEN];
    MMP_UINT8 *transport_name;
    MMP_UINT8 *protocol_name;
    MMP_UINT8 subclass;
    MMP_UINT8 protocol;
    MMP_UINT8 max_lun;
    MMP_UINT8 use_mode_sense6;

    /* information about the device  */
    MMP_UINT8                ifnum;   /* interface number   */
    MMP_UINT8                ep_in;   /* bulk in endpoint   */
    MMP_UINT8                ep_out;  /* bulk out endpoint  */
    struct usb_host_endpoint *ep_int; /* interrupt endpoint */

    /* function pointers for this device */
    trans_cmnd  transport;       /* transport function	   */
    trans_reset transport_reset; /* transport device reset */

    /* SCSI interfaces */
    Scsi_Cmnd   srb;             /* current srb		*/
    Scsi_Device device[MMP_MSC_MAX_LUN_NUM];
    void        *semaphore;
    MMP_UINT8   in_rw_access;
    MMP_UINT8   reserved[3];
};

#endif
/*
 * Copyright (c) 2008 ITE Technology Corp. All Rights Reserved.
 */
/** @file transport.h
 *
 * @author Irene Lin
 */
#ifndef TRANSPORT_H
#define TRANSPORT_H

/* Protocols */

#define US_PR_CBI  0x00         /* Control/Bulk/Interrupt */
#define US_PR_CB   0x01         /* Control/Bulk w/o interrupt */
#define US_PR_BULK 0x50         /* Bulk only */

/*
 * Bulk only data structures
 */

/* command block wrapper */
struct bulk_cb_wrap {
    MMP_UINT32 Signature;          /* contains 'USBC' */
    MMP_UINT32 Tag;                /* unique per command id */
    MMP_UINT32 DataTransferLength; /* size of data */
    MMP_UINT8  Flags;              /* direction in bit 0 */
    MMP_UINT8  Lun;                /* LUN normally 0 */
    MMP_UINT8  Length;             /* of of the CDB */
    MMP_UINT8  CDB[16];            /* max command */
};

#define US_BULK_CB_WRAP_LEN 31
#define US_BULK_CB_SIGN     0x43425355  /*spells out USBC */
#define US_BULK_FLAG_IN     1
#define US_BULK_FLAG_OUT    0

/* command status wrapper */
struct bulk_cs_wrap {
    MMP_UINT32 Signature;       /* should = 'USBS' */
    MMP_UINT32 Tag;             /* same as original command */
    MMP_UINT32 Residue;         /* amount not transferred */
    MMP_UINT8  Status;          /* see below */
    MMP_UINT8  Filler[18];
};

#define US_BULK_CS_WRAP_LEN        13
#define US_BULK_CS_SIGN            0x53425355 /* spells out 'USBS' */
#define US_BULK_STAT_OK            0
#define US_BULK_STAT_FAIL          1
#define US_BULK_STAT_PHASE         2

/* bulk-only class specific requests */
#define US_BULK_RESET_REQUEST      0xFF
#define US_BULK_GET_MAX_LUN        0xFE

/*
 * usb_stor_transfer() return codes
 */
#define US_BULK_TRANSFER_GOOD      0   /* good transfer                 */
#define US_BULK_TRANSFER_SHORT     1   /* transfered less than expected */
#define US_BULK_TRANSFER_FAILED    2   /* transfer died in the middle   */
#define US_BULK_TRANSFER_ABORTED   3   /* transfer canceled             */
#define US_BULK_TRANSFER_STALLED   4   /* endpoint stalled              */

/*
 * Transport return codes
 */

#define USB_STOR_TRANSPORT_GOOD    0   /* Transport good, command good	   */
#define USB_STOR_TRANSPORT_FAILED  1   /* Transport good, command failed   */
#define USB_STOR_TRANSPORT_ERROR   2   /* Transport bad (i.e. device dead) */
#define USB_STOR_TRANSPORT_ABORTED 3   /* Transport aborted                */

// +wlHsu
#define USB_STOR_HW_HANG_ERROR     119
// -wlHsu

/*
 * CBI accept device specific command
 */

#define US_CBI_ADSC                0

void usb_stor_invoke_transport(Scsi_Cmnd *srb, struct us_data *us);
MMP_INT usb_stor_Bulk_max_lun(struct us_data *us);
MMP_INT usb_stor_Bulk_reset(struct us_data *us);
MMP_INT usb_stor_Bulk_transport(Scsi_Cmnd *srb, struct us_data *us);
MMP_INT usb_stor_CB_reset(struct us_data *us);
void usb_stor_CBI_irq(struct urb *urb);
MMP_INT usb_stor_CBI_transport(Scsi_Cmnd *srb, struct us_data *us);
MMP_INT usb_stor_CB_transport(Scsi_Cmnd *srb, struct us_data *us);

#endif
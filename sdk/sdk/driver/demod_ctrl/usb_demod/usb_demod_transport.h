/*
 * Copyright (c) 2012 ITE Technology Corp. All Rights Reserved.
 */
/** @file transport.h
 *
 * @author Barry Wu
 */
#ifndef USBTUNER_TRANSPORT_H
#define USBTUNER_TRANSPORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usb_demod_cfg.h"
/* Protocols */

#define UT_PR_CBI	0x00		/* Control/Bulk/Interrupt */
#define UT_PR_CB	0x01		/* Control/Bulk w/o interrupt */
#define UT_PR_BULK	0x50		/* Bulk only */

/*
 * Bulk only data structures
 */

#define UT_BULK_CB_WRAP_LEN	31
#define UT_BULK_CB_SIGN		0x43425355	/*spells out USBC */
#define UT_BULK_FLAG_IN		1
#define UT_BULK_FLAG_OUT	0

#define UT_BULK_CS_WRAP_LEN	13
#define UT_BULK_CS_SIGN		0x53425355	/* spells out 'USBS' */
#define UT_BULK_STAT_OK		0
#define UT_BULK_STAT_FAIL	1
#define UT_BULK_STAT_PHASE	2

/* bulk-only class specific requests */
#define UT_BULK_RESET_REQUEST	0xFF
#define UT_BULK_GET_MAX_LUN	    0xFE

/*
 * usb_stor_transfer() return codes
 */
#define UT_BULK_TRANSFER_GOOD		0  /* good transfer                 */
#define UT_BULK_TRANSFER_SHORT		1  /* transfered less than expected */
#define UT_BULK_TRANSFER_FAILED		2  /* transfer died in the middle   */
#define UT_BULK_TRANSFER_ABORTED	3  /* transfer canceled             */

/*
 * Transport return codes
 */

#define USB_TUNER_TRANSPORT_GOOD	0   /* Transport good, command good	   */
#define USB_TUNER_TRANSPORT_FAILED  1   /* Transport good, command failed   */
#define USB_TUNER_TRANSPORT_ERROR   2   /* Transport bad (i.e. device dead) */
#define USB_TUNER_TRANSPORT_ABORTED 3   /* Transport aborted                */


/*
 * CBI accept device specific command
 */
typedef void (*USBCALLBACK)(void* pCallbackData);

typedef struct USB_CALLBACK_CONTEXT_TAG
{
    USBCALLBACK pfSelfCallback;
    void* pPrivateData;
} USB_CALLBACK_CONTEXT;

#define UT_CBI_ADSC		0

int 
usb_tuner_CB_reset(
    struct usbtuner_data *ut);
    
uint32_t 
usb_tuner_CBI_sendcmd(
    struct usbtuner_data* ut, 
    uint8_t* buffer, 
    uint16_t size);

uint32_t 
usb_tuner_CBI_receivecmd(
    struct usbtuner_data* ut, 
    uint8_t* rcvbyteBuf, 
    uint16_t size);
    
int 
usb_tuner_CBI_transport(
    struct usbtuner_data *ut);
    
uint32_t 
usb_tuner_CBI_receiveData(
    struct usbtuner_data* ut, 
    uint8_t* buffer, 
    uint32_t size);
    
uint32_t 
usb_tuner_CBI_AsyncReceiveData(
    struct usbtuner_data* ut,
    uint8_t demod_addr,
    uint8_t* buffer, 
    uint32_t size, 
    USB_CALLBACK_CONTEXT* pCallbackContext);



#ifdef __cplusplus
}
#endif

#endif


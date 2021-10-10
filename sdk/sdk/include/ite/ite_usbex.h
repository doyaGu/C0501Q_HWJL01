/** @file
 * ITE USB Driver API header file.
 *
 * @author Irene Lin
 * @version 1.0
 * @date 2011-2012
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */

#ifndef ITE_USB_EX_H
#define ITE_USB_EX_H


#define USBEX_API extern

//=============================================================================
//                              Include Files
//=============================================================================
#include "ite/ith.h"


#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
//=========================
/**
 * For HOST Driver
 */
//=========================
enum
{
    USB0 = 0,
    USB1
};

/**
 * Irene: 2010_1216 
 * example:
 * speed = mmpUsbExGetSpeed(USB0);
 *
 * For connected device speed.
 */
enum 
{
    USBEX_SPEED_UNKNOWN = 0,			/* not connected */
    USBEX_SPEED_LOW, USBEX_SPEED_FULL,	/* usb 1.1 */
    USBEX_SPEED_HIGH				    /* usb 2.0 */
};

/**
 * example: 
 * state = mmpUsbExCheckDeviceState();
 */
enum
{
    USB_DEVICE_TYPE_MSC         = 0x0001,
    USB_DEVICE_TYPE_UAS         = 0x0002,
    USB_DEVICE_TYPE_WIFI        = 0x0004,
    USB_DEVICE_TYPE_KBD         = 0x0008,
    USB_DEVICE_TYPE_MOUSE       = 0x0010,
    USB_DEVICE_TYPE_DEMOD       = 0x0012,
    USB_DEVICE_TYPE_UVC         = 0x0014,	
    USB_DEVICE_TYPE_NOT_MATCH   = 0
};
enum
{
    USB_DEVICE_STATE_NOCHAGNE,
    USB_DEVICE_STATE_CONNECT,
    USB_DEVICE_STATE_DISCONNECT
};
#define USB_DEVICE_CONNECT(x)       ((x)==USB_DEVICE_STATE_CONNECT)
#define USB_DEVICE_DISCONNECT(x)    ((x)==USB_DEVICE_STATE_DISCONNECT)
#define USB_DEVICE_WIFI(x)          ((x)==USB_DEVICE_TYPE_WIFI)
#define USB_DEVICE_UAS(x)           ((x)==USB_DEVICE_TYPE_UAS)
#define USB_DEVICE_MSC(x)           ((x)==USB_DEVICE_TYPE_MSC)
#define USB_DEVICE_KBD(x)           ((x)==USB_DEVICE_TYPE_KBD)
#define USB_DEVICE_MOUSE(x)         ((x)==USB_DEVICE_TYPE_MOUSE)
#define USB_DEVICE_DEMOD(x)         ((x)==USB_DEVICE_TYPE_DEMOD)
#define USB_DEVICE_UVC(x)           ((x)==USB_DEVICE_TYPE_UVC)




//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct
{
    int type;
    void*   ctxt;
} USB_DEVICE_INFO;

//=============================================================================
//                              Enumeration Type Definition 
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

/** @defgroup ite usb driver API
 *  The supported API for usb driver.
 *  @{
 */

//=========================
/**
 * For HOST Driver
 */
//=========================
USBEX_API void* USBEX_ThreadFunc(void* data);

USBEX_API int mmpUsbExInitialize(int);

//=============================================================================   
/**
 * Get newly USB device state.
 *
 * @param  usb      choose USB0 or USB1
 * @param  state    usb device is first connect or disconnect or no change.
 * @param  device_info      get this device information
 * @return MMP_RESULT_SUCCESS if succeed.
 *
 * @see USB_DEVICE_INFO
 */
//=============================================================================   
USBEX_API uint32_t 
mmpUsbExCheckDeviceState(
    int usb, 
    int* state, 
    USB_DEVICE_INFO* device_info
);

USBEX_API int iteUsbExIsPortConnect(int usb);

USBEX_API bool
mmpUsbExUsb0IsOtg(void);

/** 
 * Irene: 2010_1216 
 * example:
 * speed = mmpUsbExGetSpeed(USB0);
 *
 * For connected device speed.
 */
USBEX_API int
mmpUsbExGetSpeed(int usb);


#if defined(CFG_USB_DEVICE)
//=============================================================================
/**
 * Return OTG Device mode device is connect or not
 *
 * @return true if device is connect, return false if device is not connect.
 */
//=============================================================================
USBEX_API bool iteOtgIsDeviceMode(void);

USBEX_API bool
iteOtgisUSBChargeState(
    void);    
#endif // #if defined(CFG_USB_DEVICE)


//@}

enum 
{
    USB_PORT_TEST_J_STATE  = 0x1,	/* Test J_STATE */
    USB_PORT_TEST_K_STATE  = 0x2,	/* Test K_STATE */
    USB_PORT_TEST_SE0_NAK  = 0x3,	/* Test SE0_NAK */
    USB_PORT_TEST_PACKET   = 0x4,	/* Test Packet */
    USB_PORT_TEST_FORCE_EN = 0x5 	/* Test FORCE_ENABLE */
};

USBEX_API int 
mmpUsbExPortControl(
    int     usb,
    uint32_t  ctrl);

#if defined(USB_LOGO_TEST)

#define HUB_ERROR       -9999

#if 0
/** @see mmpUsbExPortControl() param ctrl */
enum 
{
    USB_PORT_TEST_J_STATE  = 0x1,	/* Test J_STATE */
    USB_PORT_TEST_K_STATE  = 0x2,	/* Test K_STATE */
    USB_PORT_TEST_SE0_NAK  = 0x3,	/* Test SE0_NAK */
    USB_PORT_TEST_PACKET   = 0x4,	/* Test Packet */
    USB_PORT_TEST_FORCE_EN = 0x5 	/* Test FORCE_ENABLE */
};
#endif

/** @see mmpUsbExDeviceControl() */
enum 
{
    USB_DEV_CTRL_SINGLE_STEP_GET_DEV      = 0x1,
    USB_DEV_CTRL_SINGLE_STEP_SET_FEATURE  = 0x2
};

#if 0
/** For USB logo test "Host Port Control". */
USBEX_API int 
mmpUsbExPortControl(
    int     usb,
    uint32_t  ctrl);
#endif

USBEX_API int 
mmpUsbExDeviceControl(
    void* usb_dev,
    uint32_t  ctrl,
    uint32_t  step,
    uint8_t*  data);

USBEX_API bool
mmpUsbExIsDeviceConnect(int usb);

USBEX_API int
mmpUsbExSuspend(int usb);

USBEX_API int
mmpUsbExResume(int usb);


#endif // #if defined(USB_LOGO_TEST)


#ifdef __cplusplus
}
#endif

#endif 



/*
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * EHCI hc_driver implementation.
 *
 * @author Irene Lin
 */
//=============================================================================
//                              Include Files
//=============================================================================
#include "usb/config.h"
#include "usb/usb/host.h"


//=============================================================================
//                              Global Data Definition
//=============================================================================

#if (CFG_CHIP_FAMILY == 9070)
ITHUsbInterface g_usb_intf = ITH_USB_AMBA;
#else
ITHUsbInterface g_usb_intf = ITH_USB_WRAP;
#endif

static struct usb_hcd* otg_hcd;


#if defined(CFG_USB_DEVICE)
#include "gadget/ite_udc.c"
#endif

#include "usb_interrupt.c"
sem_t   isr_event;

//=============================================================================
//                              Public Function Definition
//=============================================================================
void* USBEX_ThreadFunc(void* data)
{
    struct ehci_hcd* ehci0 = NULL;
    struct ehci_hcd* ehci1 = NULL;

    if(hcd0)	ehci0 = hcd_to_ehci(hcd0);
    if(hcd1)    ehci1 = hcd_to_ehci(hcd1);

    sem_init(&isr_event, 0, 0);
    while(1)
    {
        #if defined(POLLING_REMOVE)
        {
            int res = 0;
            res = itpSemWaitTimeout(&isr_event, 30);
            if(res) /** timeout */
            {
                if(hcd0) {
                    if(HC_IS_RUNNING (hcd0->state) && !_ehci_dev_exist(ehci0))
                        to_ehci_work(ehci0);
                }
                if(hcd1) {
                    if(HC_IS_RUNNING (hcd1->state) && !_ehci_dev_exist(ehci1))
                        to_ehci_work(ehci1);
                }
            }
        }
        #else
        sem_wait(&isr_event);
        #endif

        ithEnterCritical();

        if(hcd0 && ehci0->tasklet)
            ehci_work(ehci0);

        if(hcd1 && ehci1->tasklet)
            ehci_work(ehci1);

        ithExitCritical();
    }
}


#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9910))
static int usb_checkPhyClock(int usb_enable)
{
    uint32_t  usb0_en = usb_enable & (1 << 0);
    uint32_t  usb1_en = usb_enable & (1 << 1);
    uint16_t value = 0;
    uint32_t cnt = 1000;
    uint32_t step;

    if (usb0_en)
    {
        step = 1;
        while (cnt)
        {
            value = ithReadRegH(0x908);
            if (step == 1)
            {
                if (value & 0x4000)
                    step = 2;
            }
            else if (step == 2)
            {
                if (!(value & 0x4000))
                    step = 3;
            }
            else if (step == 3)
            {
                if (value & 0x4000)
                    break;
            }
            cnt--;
            usleep(50);
        }
        if (cnt == 0)  { printf(" USB0 controller's input clock not work!!! \n\n");  return -1; }
    }
    if (usb1_en)
    {
        cnt = 1000;
        step = 1;
        while (cnt)
        {
            value = ithReadRegH(0x910);
            if (step == 1)
            {
                if (value & 0x4000)
                    step = 2;
            }
            else if (step == 2)
            {
                if (!(value & 0x4000))
                    step = 3;
            }
            else if (step == 3)
            {
                if (value & 0x4000)
                    break;
            }
            cnt--;
            usleep(50);
        }
        if (cnt == 0)  { printf(" USB1 controller's input clock not work!!! \n\n");  return -1; }
    }

    return 0;
}
#else
static int usb_checkPhyClock(int usb_enable)
{
    uint32_t  usb0_en = usb_enable & (1 << 0);
    uint32_t  usb1_en = usb_enable & (1 << 1);
	uint32_t cnt;

	if (usb0_en)
	{
        cnt = 1000;
        while (cnt)
        {
        #if (CFG_CHIP_FAMILY == 9850)
            if (!(ithReadRegH(0x908) & 0x1000))
        #else
            if (!(ithReadRegA(ITH_USB0_BASE + 0x2C) & 0x1))
        #endif
        	{
				cnt--;
				usleep(50);
        	}
			else
				break;
        }
        if (cnt == 0)  { printf(" USB0 controller's input clock not work!!! \n\n");  return -1; }
	}
	if (usb1_en)
	{
        cnt = 1000;
        while (cnt)
        {
        #if (CFG_CHIP_FAMILY == 9850)
            if (!(ithReadRegH(0x910) & 0x1000))
        #else
            if (!(ithReadRegA(ITH_USB1_BASE + 0x2C) & 0x1))
        #endif
        	{
				cnt--;
				usleep(50);
        	}
			else
				break;
        }
        if (cnt == 0)  { printf(" USB1 controller's input clock not work!!! \n\n");  return -1; }
	}

	return 0;
}
#endif


int mmpUsbExInitialize(int usb_enable)
{
    int result = 0;
    uint16_t value = 0;
    uint32_t usb0 = 0x0;
    uint32_t usb1 = 0x1;
    uint16_t packageType = 0;
    uint32_t  usb0_en = usb_enable & (1 << 0);
    uint32_t  usb1_en = usb_enable & (1 << 1);

    if(!usb_enable)
    {
        result = -1;
        goto end;
    }

    if(usb0_en)
        ithUsbPhyPowerOn(ITH_USB0);
    if(usb1_en)
        ithUsbPhyPowerOn(ITH_USB1);
    usleep(10*1000);

    if (result = usb_checkPhyClock(usb_enable))
        goto end;

    ithUsbReset();
    ithUsbInterfaceSel(g_usb_intf);


    #if defined(RUN_FPGA)
    mUsbTstHalfSpeedEn();  
    //ithWriteRegMaskA((ITH_USB0_BASE|0x100), 0x2, 0x2);
    #endif

    #if defined(CFG_USB_DEVICE_USB0)
    usb0 |= USB_HAS_DEVICE_MODE; /** usb0 has device function */
    #elif defined(CFG_USB_DEVICE_USB1)
    usb1 |= USB_HAS_DEVICE_MODE; /** usb1 has device function */
    #endif

    /** usb host controller basic init */
    if(usb0_en)
    {
        result = ehci_hcd_init(usb0);
        if(result)
            goto end;
    }
    if(usb1_en)
    {
        result = ehci_hcd_init(usb1);
        if(result)
            goto end;
    }
        
#if defined(CFG_USB_DEVICE)
    if(usb0 & USB_HAS_DEVICE_MODE)
        otg_hcd = hcd0;
    if(usb1 & USB_HAS_DEVICE_MODE)
        otg_hcd = hcd1;

    result = iteUdcInitialize();
    if(result)
        goto end;
#endif // CFG_USB_DEVICE

#if defined(IT9063)
    ithWriteRegMaskA((USB0_BASE+0x80), 0x1<<12, 0x1<<12); /* this is force full speed workaround */
#endif    
    
    usbIntrEnable();

end:
    if(result)
        LOG_ERROR " mmpUsbExInitialize() return error code 0x%08X \n", result LOG_END
    return result;
}

uint32_t mmpUsbExCheckDeviceState(int usb, int* state, USB_DEVICE_INFO* device_info)
{
    int result = 0;
    uint32_t temp = 0;
    struct ehci_hcd* ehci = NULL;
    struct usb_device* dev = NULL;
    struct usb_hcd* hcd = NULL;
    bool exit = (usb & 0x10) ? true : false; /** for suspend use */
    usb &= ~0x10;

    (*state) = 0;

    if(usb == USB0)
        hcd = hcd0;
    else if(usb == USB1)
        hcd = hcd1;

    if(!hcd)
        goto end;

    ehci = hcd_to_ehci(hcd);
    temp = ithReadRegA(ehci->regs.port_status[0]);

    //if(temp & PORT_CONNECT) /** device present */
    if((temp & PORT_CONNECT) && (exit==false)) /** device present */
    {
        if(!hcd->connect)
        {
            result = ehci_start(hcd, &dev);
            if(result)
                goto end;

                (*state) = USB_DEVICE_STATE_CONNECT;
            memcpy((void*)device_info, &dev->device_info, sizeof(USB_DEVICE_INFO));
        }
    }
    else
    {
        if(hcd->connect)
        {
            (*state) = USB_DEVICE_STATE_DISCONNECT;
            device_info->ctxt = NULL;
            device_info->type = hcd->driver->stop(hcd);
        }
    }

end:
    if(result)
        LOG_ERROR " mmpUsbExCheckDeviceState() return error code 0x%08X \n", ((result<0)? (-result) : result) LOG_END
    return result;
}

int iteUsbExIsPortConnect(int usb)
{
    int connect = 0;
    struct usb_hcd* hcd = NULL;
    struct ehci_hcd* ehci = NULL;

    if(usb == USB0)
        hcd = hcd0;
    else if(usb == USB1)
        hcd = hcd1;

    if(!hcd)
        goto end;

    ehci = hcd_to_ehci(hcd);
    connect = (ithReadRegA(ehci->regs.port_status[0]) & PORT_CONNECT) ? 1 : 0;

end:
    return connect;
}

bool
mmpUsbExUsb0IsOtg(void)
{
    return (hcd0->index & USB_HAS_DEVICE_MODE) ? true : false;
}

/** Irene: get connected usb device speed routine. */
int
mmpUsbExGetSpeed(int usb)
{
    struct ehci_hcd* ehci = NULL;
    struct usb_hcd* hcd = NULL;
    int speed;
    uint32_t value;

    if(usb == USB0)
        hcd = hcd0;
    else if(usb == USB1)
        hcd = hcd1;

    if(!hcd)
        goto end;

    if(hcd->connect)
    {
        ehci = hcd_to_ehci(hcd);
        value = ithReadRegA(ehci->otg_regs.ctrl_status);
        speed = HOST_SPEED(value);

        if(speed == FULL_SPEED)
            return USBEX_SPEED_FULL;
        else if(speed == LOW_SPEED)
            return USBEX_SPEED_LOW;
        else if(speed == HIGH_SPEED)
            return USBEX_SPEED_HIGH;
    }
end:
    return USBEX_SPEED_UNKNOWN;
}

//=============================================================================
/**
 * Return OTG Device mode device is connect or not
 *
 * @return true if device is connect, return false if device is not connect.
 * @see mmpOtgDeviceModeOpen()
 */
//=============================================================================
#if defined(CFG_USB_DEVICE)
bool
iteOtgIsDeviceMode(void)
{
#if defined(SUSPEND_ENABLE)
    struct ite_udc* dev = g_udc;

    /* workaround: 0x80 read in iteOtgIsDeviceMode() will be 0x0 when enable "go suspend" bit */ 
    if(dev->ep0state == EP0_SUSPEND)
        usleep(350*1000);  /* try and error, including USBCV */
#endif

    if(mOtg_Ctrl_ID_Rd() == OTG_ID_B_TYPE)
        return true;
    else
        return false;
}

#endif // #if defined(CFG_USB_DEVICE)

int 
mmpUsbExPortControl(
    int     usb,
    uint32_t  ctrl)
{
    int result=0;
    struct ehci_hcd* ehci = NULL;
    struct usb_hcd* hcd = NULL;

    if(usb == USB0)
        hcd = hcd0;
    else if(usb == USB1)
        hcd = hcd1;

    if(!hcd)
        goto end;

    ehci = hcd_to_ehci(hcd);

    switch(ctrl)
    {
    case USB_PORT_TEST_PACKET:
        ithWriteRegA((hcd->iobase+0x0118), 0x1);
        break;
    case USB_PORT_TEST_J_STATE:
    case USB_PORT_TEST_K_STATE:
    case USB_PORT_TEST_SE0_NAK:
        ithWriteRegMaskA(ehci->regs.port_status[0], (ctrl<<PORT_TEST_SHT), PORT_TEST_MSK);
        break;
    case USB_PORT_TEST_FORCE_EN:
        ithWriteRegMaskA(ehci->regs.port_status[0], (ctrl<<PORT_TEST_SHT), PORT_TEST_MSK);
        usleep(100*1000);
        ithWriteRegMaskA(ehci->regs.port_status[0], 0x0, PORT_TEST_MSK);
        break;
    }

end:
    return result;
}

#if defined(USB_LOGO_TEST)

#if 0
extern int usb_internal_control_msg_step(
    struct usb_device* usb_dev, 
    uint32_t  pipe, 
    struct usb_ctrlrequest* cmd,  
    void*       data, 
    int     len, 
    int     timeout,
    int     step);

int 
mmpUsbExDeviceControl(
    void*       dev,
    uint32_t  ctrl,
    uint32_t  step,
    uint8_t*  data)
{
    int result=0;
    struct usb_device* usb_dev = (struct usb_device*)dev;
    struct usb_ctrlrequest dr = {0};

    switch(ctrl)
    {
    case USB_DEV_CTRL_SINGLE_STEP_GET_DEV:
        {
            if((step<0) || (step>2))
                return -1;

            if(step==0)
            {
                dr.bRequesttype = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
                dr.bRequest = USB_REQ_GET_DESCRIPTOR;
                dr.wValue = cpu_to_le16(USB_DT_DEVICE<< 8);
                dr.wIndex = 0;
                dr.wLength = cpu_to_le16(0x12);
            }
            result = usb_internal_control_msg_step(usb_dev,
                                                   usb_rcvctrlpipe(usb_dev, 0),
                                                   &dr,
                                                   data,
                                                   0x12,
                                                   500,
                                                   step);
            if(result > 0)
                result = 0;
        }
        break;
    case USB_DEV_CTRL_SINGLE_STEP_SET_FEATURE:
        {
            if((step<0) || (step>1))
                return -1;

            if(step==0)
            {
                dr.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
                dr.bRequest = USB_REQ_SET_FEATURE;
                dr.wValue = cpu_to_le16(0x2);    /** Test mode (statndard feature selectors) */
                dr.wIndex = cpu_to_le16(0x3<<8); /** Test_SE0_NAK (Test Mode Selectors) */
                dr.wLength = 0;
            }
            result = usb_internal_control_msg_step(usb_dev,
                                                   usb_sndctrlpipe(usb_dev, 0),
                                                   &dr,
                                                   NULL,
                                                   0,
                                                   500,
                                                   step);
            if(result > 0)
                result = 0;
        }
        break;
    }

end:
    if(result < 0)
        LOG_ERROR " mmpUsbExDeviceControl() ctrl %d, step %d, result = %d \n", ctrl, step, result LOG_END
    return result;
}
#endif // #if 0

bool
mmpUsbExIsDeviceConnect(int usb)
{
    struct ehci_hcd* ehci = NULL;
    struct usb_hcd* hcd = NULL;
    uint32_t status = 0;

    if(usb == USB0)
        hcd = hcd0;
    else if(usb == USB1)
        hcd = hcd1;

    ehci = hcd_to_ehci(hcd);
    status = ithReadRegA(ehci->regs.port_status[0]);
    if(status & PORT_CONNECT)
        return true;
    else
        return false;
}

int
mmpUsbExSuspend(int usb)
{
    struct usb_hcd* hcd = NULL;

    if(usb == USB0)
        hcd = hcd0;
    else if(usb == USB1)
        hcd = hcd1;

    return hcd->driver->suspend(hcd, 0);
}

int
mmpUsbExResume(int usb)
{
    struct usb_hcd* hcd = NULL;

    if(usb == USB0)
        hcd = hcd0;
    else if(usb == USB1)
        hcd = hcd1;

    return hcd->driver->resume(hcd);
}


#endif



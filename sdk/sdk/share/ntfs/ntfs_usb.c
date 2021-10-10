#include <sys/ioctl.h>
#include "gctypes.h"
#include "disc_io.h"
#include "ite/itp.h"
#include "ite/ite_usbex.h"
#include "ite/ite_msc.h"
#if defined(CFG_UAS_ENABLE)
#include "ite/ite_uas.h"
#endif

#define DEVICE_TYPE_WII_USB (('W'<<24)|('U'<<16)|('S'<<8)|'B')

static void* ntfsUsbCtxts[2][8];
static int ntfsUsbType[2];

static bool usb_Startup(int usb, int lun)
{
    ITPUsbInfo usbInfo;
    int retval;

    printf("usb_Startup(%d,%d)\n", usb, lun);

    usbInfo.host = true;
    usbInfo.usbIndex = usb;
    ioctl(ITP_DEVICE_USB, ITP_IOCTL_GET_INFO, (void*)&usbInfo);
    ntfsUsbType[usb] = usbInfo.type;
    
#if defined(CFG_UAS_ENABLE)
    if(USB_DEVICE_UAS(ntfsUsbType[usb]))
        retval = iteUasInitialize(usbInfo.ctxt, lun);
    else
#endif
    retval = iteMscInitialize(usbInfo.ctxt, lun);
    if (retval)
    {
        printf("iteMscInitialize(0x%X,%d) fail: 0x%X\n", usbInfo.ctxt, lun, retval);
        return false;
    }

    ntfsUsbCtxts[usb][lun] = usbInfo.ctxt;
    
    printf("usb_Startup(%d,%d) finish\n", usb, lun);
    
    return true;
}

static bool usb_ReadSectors(int usb, int lun, u32 sector, u32 numSectors, void *buffer)
{
    int retval;
        
#if defined(CFG_UAS_ENABLE)
    if(USB_DEVICE_UAS(ntfsUsbType[usb]))
        retval = iteUasReadMultipleSector(ntfsUsbCtxts[usb][lun], lun, sector, numSectors, (void*)buffer);
    else
#endif
    retval = iteMscReadMultipleSector(ntfsUsbCtxts[usb][lun], lun, sector, numSectors, (void*)buffer);
    if(retval)
    {
        printf("iteMscReadMultipleSector(0x%X,%d,%d,%d,0x%X) fail: 0x%X\n", ntfsUsbCtxts[usb][lun], lun, sector, numSectors, buffer, retval);
        return false;
    }
   
    return true;
}

static bool usb_WriteSectors(int usb, int lun, u32 sector, u32 numSectors, const void *buffer)
{
    int retval;
        
#if defined(CFG_UAS_ENABLE)
    if(USB_DEVICE_UAS(ntfsUsbType[usb]))
        retval = iteUasWriteMultipleSector(ntfsUsbCtxts[usb][lun], lun, sector, numSectors, (void*)buffer);
    else
#endif
    retval = iteMscWriteMultipleSector(ntfsUsbCtxts[usb][lun], lun, sector, numSectors, (void*)buffer);
    if(retval)
    {
        printf("iteMscWriteMultipleSector(0x%X,%d,%d,%d,0x%X) fail: 0x%X\n", ntfsUsbCtxts[usb][lun], lun, sector, numSectors, buffer, retval);
        return false;
    }
   
   return true;
}

static bool usb_ClearStatus(void)
{
    return true;
}

static bool usb_Shutdown(int usb, int lun)
{
#if defined(CFG_UAS_ENABLE)
    if(USB_DEVICE_UAS(ntfsUsbType[usb]))
        iteUasTerminate(ntfsUsbCtxts[usb][lun], lun);
    else
#endif
    iteMscTerminate(ntfsUsbCtxts[usb][lun], lun);

    return true;
}

static bool msc00_Startup(void)
{
    return usb_Startup(0, 0);
}

static bool msc00_IsInserted(void)
{
   bool retval = (bool) ioctl(ITP_DEVICE_CARD, ITP_IOCTL_IS_AVAIL, (void*)ITP_CARD_MSC00);
   printf("msc00_IsInserted(): %d\n", retval);
   return retval;
}

static bool msc00_ReadSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_ReadSectors(0, 0, sector, numSectors, buffer);
}

static bool msc00_WriteSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_WriteSectors(0, 0, sector, numSectors, buffer);
}

static bool msc00_Shutdown(void)
{
    return usb_Shutdown(0, 0);
}

const DISC_INTERFACE __io_msc00 = {
   DEVICE_TYPE_WII_USB,
   FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_USB,
   (FN_MEDIUM_STARTUP)&msc00_Startup,
   (FN_MEDIUM_ISINSERTED)&msc00_IsInserted,
   (FN_MEDIUM_READSECTORS)&msc00_ReadSectors,
   (FN_MEDIUM_WRITESECTORS)&msc00_WriteSectors,
   (FN_MEDIUM_CLEARSTATUS)&usb_ClearStatus,
   (FN_MEDIUM_SHUTDOWN)&msc00_Shutdown
};

static bool msc01_Startup(void)
{
    return usb_Startup(0, 1);
}

static bool msc01_IsInserted(void)
{
   return (bool) ioctl(ITP_DEVICE_CARD, ITP_IOCTL_IS_AVAIL, (void*)ITP_CARD_MSC01);
}

static bool msc01_ReadSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_ReadSectors(0, 1, sector, numSectors, buffer);
}

static bool msc01_WriteSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_WriteSectors(0, 1, sector, numSectors, buffer);
}

static bool msc01_Shutdown(void)
{
    return usb_Shutdown(0, 1);
}

const DISC_INTERFACE __io_msc01 = {
   DEVICE_TYPE_WII_USB,
   FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_USB,
   (FN_MEDIUM_STARTUP)&msc01_Startup,
   (FN_MEDIUM_ISINSERTED)&msc01_IsInserted,
   (FN_MEDIUM_READSECTORS)&msc01_ReadSectors,
   (FN_MEDIUM_WRITESECTORS)&msc01_WriteSectors,
   (FN_MEDIUM_CLEARSTATUS)&usb_ClearStatus,
   (FN_MEDIUM_SHUTDOWN)&msc01_Shutdown
};

static bool msc02_Startup(void)
{
    return usb_Startup(0, 2);
}

static bool msc02_IsInserted(void)
{
   return (bool) ioctl(ITP_DEVICE_CARD, ITP_IOCTL_IS_AVAIL, (void*)ITP_CARD_MSC02);
}

static bool msc02_ReadSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_ReadSectors(0, 2, sector, numSectors, buffer);
}

static bool msc02_WriteSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_WriteSectors(0, 2, sector, numSectors, buffer);
}

static bool msc02_Shutdown(void)
{
    return usb_Shutdown(0, 2);
}

const DISC_INTERFACE __io_msc02 = {
   DEVICE_TYPE_WII_USB,
   FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_USB,
   (FN_MEDIUM_STARTUP)&msc02_Startup,
   (FN_MEDIUM_ISINSERTED)&msc02_IsInserted,
   (FN_MEDIUM_READSECTORS)&msc02_ReadSectors,
   (FN_MEDIUM_WRITESECTORS)&msc02_WriteSectors,
   (FN_MEDIUM_CLEARSTATUS)&usb_ClearStatus,
   (FN_MEDIUM_SHUTDOWN)&msc02_Shutdown
};

static bool msc03_Startup(void)
{
    return usb_Startup(0, 3);
}

static bool msc03_IsInserted(void)
{
   return (bool) ioctl(ITP_DEVICE_CARD, ITP_IOCTL_IS_AVAIL, (void*)ITP_CARD_MSC03);
}

static bool msc03_ReadSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_ReadSectors(0, 3, sector, numSectors, buffer);
}

static bool msc03_WriteSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_WriteSectors(0, 3, sector, numSectors, buffer);
}

static bool msc03_Shutdown(void)
{
    return usb_Shutdown(0, 3);
}

const DISC_INTERFACE __io_msc03 = {
   DEVICE_TYPE_WII_USB,
   FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_USB,
   (FN_MEDIUM_STARTUP)&msc03_Startup,
   (FN_MEDIUM_ISINSERTED)&msc03_IsInserted,
   (FN_MEDIUM_READSECTORS)&msc03_ReadSectors,
   (FN_MEDIUM_WRITESECTORS)&msc03_WriteSectors,
   (FN_MEDIUM_CLEARSTATUS)&usb_ClearStatus,
   (FN_MEDIUM_SHUTDOWN)&msc03_Shutdown
};

static bool msc04_Startup(void)
{
    return usb_Startup(0, 4);
}

static bool msc04_IsInserted(void)
{
   return (bool) ioctl(ITP_DEVICE_CARD, ITP_IOCTL_IS_AVAIL, (void*)ITP_CARD_MSC04);
}

static bool msc04_ReadSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_ReadSectors(0, 4, sector, numSectors, buffer);
}

static bool msc04_WriteSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_WriteSectors(0, 4, sector, numSectors, buffer);
}

static bool msc04_Shutdown(void)
{
    return usb_Shutdown(0, 4);
}

const DISC_INTERFACE __io_msc04 = {
   DEVICE_TYPE_WII_USB,
   FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_USB,
   (FN_MEDIUM_STARTUP)&msc04_Startup,
   (FN_MEDIUM_ISINSERTED)&msc04_IsInserted,
   (FN_MEDIUM_READSECTORS)&msc04_ReadSectors,
   (FN_MEDIUM_WRITESECTORS)&msc04_WriteSectors,
   (FN_MEDIUM_CLEARSTATUS)&usb_ClearStatus,
   (FN_MEDIUM_SHUTDOWN)&msc04_Shutdown
};

static bool msc05_Startup(void)
{
    return usb_Startup(0, 5);
}

static bool msc05_IsInserted(void)
{
   return (bool) ioctl(ITP_DEVICE_CARD, ITP_IOCTL_IS_AVAIL, (void*)ITP_CARD_MSC05);
}

static bool msc05_ReadSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_ReadSectors(0, 5, sector, numSectors, buffer);
}

static bool msc05_WriteSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_WriteSectors(0, 5, sector, numSectors, buffer);
}

static bool msc05_Shutdown(void)
{
    return usb_Shutdown(0, 5);
}

const DISC_INTERFACE __io_msc05 = {
   DEVICE_TYPE_WII_USB,
   FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_USB,
   (FN_MEDIUM_STARTUP)&msc05_Startup,
   (FN_MEDIUM_ISINSERTED)&msc05_IsInserted,
   (FN_MEDIUM_READSECTORS)&msc05_ReadSectors,
   (FN_MEDIUM_WRITESECTORS)&msc05_WriteSectors,
   (FN_MEDIUM_CLEARSTATUS)&usb_ClearStatus,
   (FN_MEDIUM_SHUTDOWN)&msc05_Shutdown
};

static bool msc06_Startup(void)
{
    return usb_Startup(0, 6);
}

static bool msc06_IsInserted(void)
{
   return (bool) ioctl(ITP_DEVICE_CARD, ITP_IOCTL_IS_AVAIL, (void*)ITP_CARD_MSC06);
}

static bool msc06_ReadSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_ReadSectors(0, 6, sector, numSectors, buffer);
}

static bool msc06_WriteSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_WriteSectors(0, 6, sector, numSectors, buffer);
}

static bool msc06_Shutdown(void)
{
    return usb_Shutdown(0, 6);
}

const DISC_INTERFACE __io_msc06 = {
   DEVICE_TYPE_WII_USB,
   FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_USB,
   (FN_MEDIUM_STARTUP)&msc06_Startup,
   (FN_MEDIUM_ISINSERTED)&msc06_IsInserted,
   (FN_MEDIUM_READSECTORS)&msc06_ReadSectors,
   (FN_MEDIUM_WRITESECTORS)&msc06_WriteSectors,
   (FN_MEDIUM_CLEARSTATUS)&usb_ClearStatus,
   (FN_MEDIUM_SHUTDOWN)&msc06_Shutdown
};

static bool msc07_Startup(void)
{
    return usb_Startup(0, 7);
}

static bool msc07_IsInserted(void)
{
   return (bool) ioctl(ITP_DEVICE_CARD, ITP_IOCTL_IS_AVAIL, (void*)ITP_CARD_MSC07);
}

static bool msc07_ReadSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_ReadSectors(0, 7, sector, numSectors, buffer);
}

static bool msc07_WriteSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_WriteSectors(0, 7, sector, numSectors, buffer);
}

static bool msc07_Shutdown(void)
{
    return usb_Shutdown(0, 7);
}

const DISC_INTERFACE __io_msc07 = {
   DEVICE_TYPE_WII_USB,
   FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_USB,
   (FN_MEDIUM_STARTUP)&msc07_Startup,
   (FN_MEDIUM_ISINSERTED)&msc07_IsInserted,
   (FN_MEDIUM_READSECTORS)&msc07_ReadSectors,
   (FN_MEDIUM_WRITESECTORS)&msc07_WriteSectors,
   (FN_MEDIUM_CLEARSTATUS)&usb_ClearStatus,
   (FN_MEDIUM_SHUTDOWN)&msc07_Shutdown
};

static bool msc10_Startup(void)
{
    return usb_Startup(1, 0);
}

static bool msc10_IsInserted(void)
{
   return (bool) ioctl(ITP_DEVICE_CARD, ITP_IOCTL_IS_AVAIL, (void*)ITP_CARD_MSC10);
}

static bool msc10_ReadSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_ReadSectors(1, 0, sector, numSectors, buffer);
}

static bool msc10_WriteSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_WriteSectors(1, 0, sector, numSectors, buffer);
}

static bool msc10_Shutdown(void)
{
    return usb_Shutdown(1, 0);
}

const DISC_INTERFACE __io_msc10 = {
   DEVICE_TYPE_WII_USB,
   FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_USB,
   (FN_MEDIUM_STARTUP)&msc10_Startup,
   (FN_MEDIUM_ISINSERTED)&msc10_IsInserted,
   (FN_MEDIUM_READSECTORS)&msc10_ReadSectors,
   (FN_MEDIUM_WRITESECTORS)&msc10_WriteSectors,
   (FN_MEDIUM_CLEARSTATUS)&usb_ClearStatus,
   (FN_MEDIUM_SHUTDOWN)&msc10_Shutdown
};

static bool msc11_Startup(void)
{
    return usb_Startup(1, 1);
}

static bool msc11_IsInserted(void)
{
   return (bool) ioctl(ITP_DEVICE_CARD, ITP_IOCTL_IS_AVAIL, (void*)ITP_CARD_MSC11);
}

static bool msc11_ReadSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_ReadSectors(1, 1, sector, numSectors, buffer);
}

static bool msc11_WriteSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_WriteSectors(1, 1, sector, numSectors, buffer);
}

static bool msc11_Shutdown(void)
{
    return usb_Shutdown(1, 1);
}

const DISC_INTERFACE __io_msc11 = {
   DEVICE_TYPE_WII_USB,
   FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_USB,
   (FN_MEDIUM_STARTUP)&msc11_Startup,
   (FN_MEDIUM_ISINSERTED)&msc11_IsInserted,
   (FN_MEDIUM_READSECTORS)&msc11_ReadSectors,
   (FN_MEDIUM_WRITESECTORS)&msc11_WriteSectors,
   (FN_MEDIUM_CLEARSTATUS)&usb_ClearStatus,
   (FN_MEDIUM_SHUTDOWN)&msc11_Shutdown
};

static bool msc12_Startup(void)
{
    return usb_Startup(1, 2);
}

static bool msc12_IsInserted(void)
{
   return (bool) ioctl(ITP_DEVICE_CARD, ITP_IOCTL_IS_AVAIL, (void*)ITP_CARD_MSC12);
}

static bool msc12_ReadSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_ReadSectors(1, 2, sector, numSectors, buffer);
}

static bool msc12_WriteSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_WriteSectors(1, 2, sector, numSectors, buffer);
}

static bool msc12_Shutdown(void)
{
    return usb_Shutdown(1, 2);
}

const DISC_INTERFACE __io_msc12 = {
   DEVICE_TYPE_WII_USB,
   FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_USB,
   (FN_MEDIUM_STARTUP)&msc12_Startup,
   (FN_MEDIUM_ISINSERTED)&msc12_IsInserted,
   (FN_MEDIUM_READSECTORS)&msc12_ReadSectors,
   (FN_MEDIUM_WRITESECTORS)&msc12_WriteSectors,
   (FN_MEDIUM_CLEARSTATUS)&usb_ClearStatus,
   (FN_MEDIUM_SHUTDOWN)&msc12_Shutdown
};

static bool msc13_Startup(void)
{
    return usb_Startup(1, 3);
}

static bool msc13_IsInserted(void)
{
   return (bool) ioctl(ITP_DEVICE_CARD, ITP_IOCTL_IS_AVAIL, (void*)ITP_CARD_MSC13);
}

static bool msc13_ReadSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_ReadSectors(1, 3, sector, numSectors, buffer);
}

static bool msc13_WriteSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_WriteSectors(1, 3, sector, numSectors, buffer);
}

static bool msc13_Shutdown(void)
{
    return usb_Shutdown(1, 3);
}

const DISC_INTERFACE __io_msc13 = {
   DEVICE_TYPE_WII_USB,
   FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_USB,
   (FN_MEDIUM_STARTUP)&msc13_Startup,
   (FN_MEDIUM_ISINSERTED)&msc13_IsInserted,
   (FN_MEDIUM_READSECTORS)&msc13_ReadSectors,
   (FN_MEDIUM_WRITESECTORS)&msc13_WriteSectors,
   (FN_MEDIUM_CLEARSTATUS)&usb_ClearStatus,
   (FN_MEDIUM_SHUTDOWN)&msc13_Shutdown
};

static bool msc14_Startup(void)
{
    return usb_Startup(1, 4);
}

static bool msc14_IsInserted(void)
{
   return (bool) ioctl(ITP_DEVICE_CARD, ITP_IOCTL_IS_AVAIL, (void*)ITP_CARD_MSC14);
}

static bool msc14_ReadSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_ReadSectors(1, 4, sector, numSectors, buffer);
}

static bool msc14_WriteSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_WriteSectors(1, 4, sector, numSectors, buffer);
}

static bool msc14_Shutdown(void)
{
    return usb_Shutdown(1, 4);
}

const DISC_INTERFACE __io_msc14 = {
   DEVICE_TYPE_WII_USB,
   FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_USB,
   (FN_MEDIUM_STARTUP)&msc14_Startup,
   (FN_MEDIUM_ISINSERTED)&msc14_IsInserted,
   (FN_MEDIUM_READSECTORS)&msc14_ReadSectors,
   (FN_MEDIUM_WRITESECTORS)&msc14_WriteSectors,
   (FN_MEDIUM_CLEARSTATUS)&usb_ClearStatus,
   (FN_MEDIUM_SHUTDOWN)&msc14_Shutdown
};

static bool msc15_Startup(void)
{
    return usb_Startup(1, 5);
}

static bool msc15_IsInserted(void)
{
   return (bool) ioctl(ITP_DEVICE_CARD, ITP_IOCTL_IS_AVAIL, (void*)ITP_CARD_MSC15);
}

static bool msc15_ReadSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_ReadSectors(1, 5, sector, numSectors, buffer);
}

static bool msc15_WriteSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_WriteSectors(1, 5, sector, numSectors, buffer);
}

static bool msc15_Shutdown(void)
{
    return usb_Shutdown(1, 5);
}

const DISC_INTERFACE __io_msc15 = {
   DEVICE_TYPE_WII_USB,
   FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_USB,
   (FN_MEDIUM_STARTUP)&msc15_Startup,
   (FN_MEDIUM_ISINSERTED)&msc15_IsInserted,
   (FN_MEDIUM_READSECTORS)&msc15_ReadSectors,
   (FN_MEDIUM_WRITESECTORS)&msc15_WriteSectors,
   (FN_MEDIUM_CLEARSTATUS)&usb_ClearStatus,
   (FN_MEDIUM_SHUTDOWN)&msc15_Shutdown
};

static bool msc16_Startup(void)
{
    return usb_Startup(1, 6);
}

static bool msc16_IsInserted(void)
{
   return (bool) ioctl(ITP_DEVICE_CARD, ITP_IOCTL_IS_AVAIL, (void*)ITP_CARD_MSC16);
}

static bool msc16_ReadSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_ReadSectors(1, 6, sector, numSectors, buffer);
}

static bool msc16_WriteSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_WriteSectors(1, 6, sector, numSectors, buffer);
}

static bool msc16_Shutdown(void)
{
    return usb_Shutdown(1, 6);
}

const DISC_INTERFACE __io_msc16 = {
   DEVICE_TYPE_WII_USB,
   FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_USB,
   (FN_MEDIUM_STARTUP)&msc16_Startup,
   (FN_MEDIUM_ISINSERTED)&msc16_IsInserted,
   (FN_MEDIUM_READSECTORS)&msc16_ReadSectors,
   (FN_MEDIUM_WRITESECTORS)&msc16_WriteSectors,
   (FN_MEDIUM_CLEARSTATUS)&usb_ClearStatus,
   (FN_MEDIUM_SHUTDOWN)&msc16_Shutdown
};

static bool msc17_Startup(void)
{
    return usb_Startup(1, 7);
}

static bool msc17_IsInserted(void)
{
   return (bool) ioctl(ITP_DEVICE_CARD, ITP_IOCTL_IS_AVAIL, (void*)ITP_CARD_MSC17);
}

static bool msc17_ReadSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_ReadSectors(1, 7, sector, numSectors, buffer);
}

static bool msc17_WriteSectors(u32 sector, u32 numSectors, void *buffer)
{
    return usb_WriteSectors(1, 7, sector, numSectors, buffer);
}

static bool msc17_Shutdown(void)
{
    return usb_Shutdown(1, 7);
}

const DISC_INTERFACE __io_msc17 = {
   DEVICE_TYPE_WII_USB,
   FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_USB,
   (FN_MEDIUM_STARTUP)&msc17_Startup,
   (FN_MEDIUM_ISINSERTED)&msc17_IsInserted,
   (FN_MEDIUM_READSECTORS)&msc17_ReadSectors,
   (FN_MEDIUM_WRITESECTORS)&msc17_WriteSectors,
   (FN_MEDIUM_CLEARSTATUS)&usb_ClearStatus,
   (FN_MEDIUM_SHUTDOWN)&msc17_Shutdown
};


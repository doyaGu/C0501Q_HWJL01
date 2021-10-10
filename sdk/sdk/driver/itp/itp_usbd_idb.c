#include <errno.h>
#include <string.h>
#include "ite/itp.h"
#include "ite/itc.h"
#include "ite/ug.h"
#include "ite/ite_idb.h"

static struct idb_config cfg;
static ITCArrayStream arrayStream;

static int UsbdIdbUpgradeFirmware(const char* firmwareData, int firmwareSize)
{
	int ret;
	ITCStream* upgradeFile;
	
	ret = itcArrayStreamOpen(&arrayStream,  firmwareData, firmwareSize);
	if (!ret)
	{
		upgradeFile = &arrayStream.stream;
		if (upgradeFile)
        {
            if (ugCheckCrc(upgradeFile, NULL))
                printf("Upgrade failed.\n");
            else
                ret = ugUpgradePackage(upgradeFile);

            #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
				printf("Flushing NOR cache...\n");
				ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
            #endif

            if (ret)
                printf("Upgrade failed.\n");
            else
                printf("Upgrade finished.\n");
				     
        }
	}
	else
		printf("Open stream fail\n");

	return ret;	
}

static int UsbdIdbGetUpgradePercentage(void)
{
	return ugGetProrgessPercentage();
}


static struct idb_operations idb_ops = {
	UsbdIdbUpgradeFirmware,
	UsbdIdbGetUpgradePercentage,
};

static void UsbdIdbInit(void)
{
	memset((void*)&cfg, 0x0, sizeof(struct idb_config));
	cfg.ops = &idb_ops;
}

static int UsbdIdbIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:		
        UsbdIdbInit();
        break;
        
    case ITP_IOCTL_ENABLE:		
		iteIdbInitialize(&cfg);
        break;

    case ITP_IOCTL_DISABLE:
        break;

    default:
        errno = (ITP_DEVICE_USBDIDB << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}


const ITPDevice itpDeviceUsbdIdb =
{
    ":usbd idb",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    itpWriteDefault,
    itpLseekDefault,
    UsbdIdbIoctl,
    NULL
};

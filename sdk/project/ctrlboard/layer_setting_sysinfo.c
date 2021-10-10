#include <sys/ioctl.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lwip/ip.h"
#include "ite/itp.h"
#include "scene.h"
#include "ctrlboard.h"

static ITUText* settingSysInfoIPAddrText;
static ITUText* settingSysInfoMacText;
static ITUText* settingSysInfoHWVersionText;
static ITUText* settingSysInfoSWVersionText;

bool SettingSysInfoOnEnter(ITUWidget* widget, char* param)
{
#ifdef CFG_NET_WIFI
    ITPWifiInfo netInfo;
#elif defined(CFG_NET_ETHERNET)
    ITPEthernetInfo netInfo;
#endif
    char buf[32];
    char* ip;

    if (!settingSysInfoIPAddrText)
    {
        settingSysInfoIPAddrText = ituSceneFindWidget(&theScene, "settingSysInfoIPAddrText");
        assert(settingSysInfoIPAddrText);

        settingSysInfoMacText = ituSceneFindWidget(&theScene, "settingSysInfoMacText");
        assert(settingSysInfoMacText);

        settingSysInfoHWVersionText = ituSceneFindWidget(&theScene, "settingSysInfoHWVersionText");
        assert(settingSysInfoHWVersionText);

        settingSysInfoSWVersionText = ituSceneFindWidget(&theScene, "settingSysInfoSWVersionText");
        assert(settingSysInfoSWVersionText);
    }

#ifdef CFG_NET_WIFI
    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_GET_INFO, &netInfo);
    ip = ipaddr_ntoa((const ip_addr_t*)&netInfo.address);
#elif defined(CFG_NET_ETHERNET)
    netInfo.index = 0;
    ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_GET_INFO, &netInfo);
    ip = ipaddr_ntoa((const ip_addr_t*)&netInfo.address);
#else
    ip = theConfig.ipaddr;
#endif // CFG_NET_WIFI

    ituTextSetString(settingSysInfoIPAddrText, ip);

    buf[0] = '\0';
#ifdef CFG_NET_ENABLE
    sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", 
        (uint8_t)netInfo.hardwareAddress[0], 
        (uint8_t)netInfo.hardwareAddress[1], 
        (uint8_t)netInfo.hardwareAddress[2], 
        (uint8_t)netInfo.hardwareAddress[3], 
        (uint8_t)netInfo.hardwareAddress[4], 
        (uint8_t)netInfo.hardwareAddress[5]);
#endif // CFG_NET_ENABLE
    ituTextSetString(settingSysInfoMacText, buf);

    ituTextSetString(settingSysInfoHWVersionText, CFG_HW_VERSION);
    ituTextSetString(settingSysInfoSWVersionText, CFG_VERSION_MAJOR_STR "." CFG_VERSION_MINOR_STR "." CFG_VERSION_PATCH_STR "." CFG_VERSION_CUSTOM_STR "." CFG_VERSION_TWEAK_STR);

    return true;
}

void SettingSysInfoReset(void)
{
    settingSysInfoIPAddrText = NULL;
}

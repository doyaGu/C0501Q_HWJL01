#include <sys/ioctl.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include "curl/curl.h"
#include "bootloader.h"
#include "config.h"

#ifdef CFG_NET_WIFI
    #include "ite/ite_wifi.h"
#endif

static ITCArrayStream arrayStream;

#if defined (CFG_ENABLE_UART_CLI)
extern char tftppara[128];
#endif

struct FtpBuf
{
    uint8_t* buf;
    uint32_t pos;
};
 
static size_t FtpWrite(void *buffer, size_t size, size_t nmemb, void *stream)
{
    struct FtpBuf* out = (struct FtpBuf*)stream;
    size_t s;

    //LOG_DBG "FtpWrite(0x%X,%d,%d,0x%X)\n", buffer, size, nmemb, stream LOG_END

    assert(out->buf);
    s = size * nmemb;
    memcpy(&out->buf[out->pos], buffer, s);
    out->pos += s;
    return s;
}

static InitNetwork(void)
{
#ifdef CFG_NET_ETHERNET
    ITPEthernetSetting setting;

    // init ethernet device
    itpRegisterDevice(ITP_DEVICE_ETHERNET, &itpDeviceEthernet);
    ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_INIT, NULL);
    
#elif defined(CFG_NET_WIFI)

    struct net_device_config wificfg;

    // init usb
#if defined(CFG_USB0_ENABLE) || defined(CFG_USB1_ENABLE)
    itpRegisterDevice(ITP_DEVICE_USB, &itpDeviceUsb);
    ioctl(ITP_DEVICE_USB, ITP_IOCTL_INIT, NULL);
#endif

    // init wifi device
    itpRegisterDevice(ITP_DEVICE_WIFI, &itpDeviceWifi);
    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_INIT, NULL);

#endif // CFG_NET_ETHERNET

    // init socket device
#ifdef CFG_NET_ENABLE
    itpRegisterDevice(ITP_DEVICE_SOCKET, &itpDeviceSocket);
    ioctl(ITP_DEVICE_SOCKET, ITP_IOCTL_INIT, NULL);
#endif

#ifdef CFG_DBG_NETCONSOLE
    // init network console device
    itpRegisterDevice(ITP_DEVICE_NETCONSOLE, &itpDeviceNetConsole);
    ioctl(ITP_DEVICE_NETCONSOLE, ITP_IOCTL_INIT, NULL);
    ioctl(ITP_DEVICE_NETCONSOLE, ITP_IOCTL_ENABLE, NULL);
    itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceNetConsole);
#endif

#ifdef CFG_NET_ETHERNET

    LOG_INFO "Wait ethernet cable to plugin" LOG_END
    while (!ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_CONNECTED, NULL))
    {
        sleep(1);
        putchar('.');
        fflush(stdout);
    }

#elif defined(CFG_NET_WIFI)

    memset(&wificfg, 0, sizeof (struct net_device_config));
    
    LOG_INFO "Wait WiFi dongle to plugin" LOG_END
    while (!ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_DEVICE_READY, NULL))
    {
        sleep(1);
        putchar('.');
        fflush(stdout);
    }
    putchar('\n');

    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_ENABLE, NULL);
    usleep(1000000);

    wificfg.operationMode               = WLAN_MODE_STA;
    wificfg.securitySuit.securityMode   = WLAN_SEC_NOSEC;    
    strncpy(wificfg.ssidName, CFG_UPGRADE_RECOVERY_SSID, 31);

    // try to connect to WiFi AP
    LOG_INFO "Connect to %s\n", wificfg.ssidName LOG_END
    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_LINK_AP, &wificfg);
    
    LOG_INFO "Wait WiFi AP to connect" LOG_END
    while (!ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_CONNECTED, NULL))
    {
        sleep(1);
        putchar('.');
        fflush(stdout);
    }
    
    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_RESET, &setting);
    
#endif // CFG_NET_ETHERNET
}

ITCStream* OpenRecoveryPackage(void)
{
    CURL *curl;
    CURLcode res;
    struct FtpBuf ftpBuf;

    InitNetwork();

    ftpBuf.buf = malloc(CFG_RAM_SIZE / 2);
    ftpBuf.pos = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
 
    curl = curl_easy_init();
    if (!curl)
    {
        LOG_ERR "curl_easy_init() fail.\n" LOG_END
        goto error;
    }
#if defined (CFG_UPGRADE_RECOVERY_TFTP)
    curl_easy_setopt(curl, CURLOPT_URL, "tftp://" CFG_UPGRADE_RECOVERY_SERVER_ADDR "/" CFG_UPGRADE_FILENAME);
#elif defined (CFG_ENABLE_UART_CLI)
	printf("\ntftppara=%s\n", tftppara);
	curl_easy_setopt(curl, CURLOPT_URL, tftppara);	
#else
    curl_easy_setopt(curl, CURLOPT_URL, "ftp://" CFG_UPGRADE_RECOVERY_SERVER_ADDR "/" CFG_UPGRADE_FILENAME);
#endif
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, FtpWrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpBuf);
 
#ifndef NDEBUG
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

    res = curl_easy_perform(curl);
 
    /* always cleanup */ 
    curl_easy_cleanup(curl);

    if (CURLE_OK != res)
    {
        LOG_ERR "curl fail: %d\n", res LOG_END
        goto error;
    }

    curl_global_cleanup();

    itcArrayStreamOpen(&arrayStream, ftpBuf.buf, ftpBuf.pos);

    return &arrayStream.stream;

error:
    curl_global_cleanup();
    return NULL;
}

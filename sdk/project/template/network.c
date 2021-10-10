#include <sys/ioctl.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
#include "lwip/ip.h"
#include "lwip/dns.h"
#include "ite/itp.h"
#include "iniparser/iniparser.h"
#include "project.h"
#include "wifiMgr.h"

#define DHCP_TIMEOUT_MSEC (60 * 1000) //60sec

static bool networkIsReady;
static int networkSocket;
static bool networkToReset;

#ifdef CFG_NET_WIFI
static WIFI_MGR_SETTING gWifiSetting;
#endif

static  struct timeval tvStart = {0, 0}, tvEnd = {0, 0};
// wifi init
static int gInit =0;

bool NetworkIsReady(void)
{
    return networkIsReady;
}

static void ResetEthernet(void)
{
    ITPEthernetSetting setting;
    ITPEthernetInfo info;
    unsigned long mscnt = 0;
    char buf[16], *saveptr;

    memset(&setting, 0, sizeof (ITPEthernetSetting));

    setting.index = 0;

    // dhcp
    setting.dhcp = theConfig.dhcp;

    // autoip
    setting.autoip = 0;

    // ipaddr
    strcpy(buf, theConfig.ipaddr);
    setting.ipaddr[0] = atoi(strtok_r(buf, ".", &saveptr));
    setting.ipaddr[1] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.ipaddr[2] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.ipaddr[3] = atoi(strtok_r(NULL, " ", &saveptr));

    // netmask
    strcpy(buf, theConfig.netmask);
    setting.netmask[0] = atoi(strtok_r(buf, ".", &saveptr));
    setting.netmask[1] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.netmask[2] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.netmask[3] = atoi(strtok_r(NULL, " ", &saveptr));

    // gateway
    strcpy(buf, theConfig.gw);
    setting.gw[0] = atoi(strtok_r(buf, ".", &saveptr));
    setting.gw[1] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.gw[2] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.gw[3] = atoi(strtok_r(NULL, " ", &saveptr));

    ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_RESET, &setting);

    printf("Wait ethernet cable to plugin");
    while (!ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_CONNECTED, NULL))
    {
        sleep(1);
        putchar('.');
        fflush(stdout);
    }

    printf("\nWait DHCP settings");
    while (!ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_AVAIL, NULL))
    {
        usleep(100000);
        mscnt += 100;

        putchar('.');
        fflush(stdout);

        if (mscnt >= DHCP_TIMEOUT_MSEC)
        {
            printf("\nDHCP timeout, use default settings\n");
            setting.dhcp = setting.autoip = 0;
            ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_RESET, &setting);
            break;
        }
    }
    puts("");

    if (ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_AVAIL, NULL))
    {
        char* ip;

        info.index = 0;
        ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_GET_INFO, &info);
        ip = ipaddr_ntoa((const ip_addr_t*)&info.address);

        printf("IP address: %s\n", ip);

        networkIsReady = true;
    }
}


#ifdef CFG_NET_WIFI
static int wifiCallbackFucntion(int nState)
{
    switch (nState)
    {
        case WIFIMGR_STATE_CALLBACK_CONNECTION_FINISH:
            printf("[Indoor]WifiCallback connection finish \n");
            WebServerInit();
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_DISCONNECT_30S:
            printf("[Indoor]WifiCallback connection disconnect 30s \n");
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_RECONNECTION:
            printf("[Indoor]WifiCallback connection reconnection \n");
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_TEMP_DISCONNECT:
            printf("[Indoor]WifiCallback connection temp disconnect \n");
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_CONNECTING_FAIL:
            printf("[Indoor]WifiCallback connecting fail, please check ssid,password,secmode \n");
        break;

        default:
            printf("[Indoor]WifiCallback unknown %d state  \n",nState);
        break;

    }


}


static ResetWifi()
{
//    ITPEthernetSetting setting;
    char buf[16], *saveptr;

    memset(&gWifiSetting.setting, 0, sizeof (ITPEthernetSetting));

    gWifiSetting.setting.index = 0;

    // dhcp
    gWifiSetting.setting.dhcp = theConfig.dhcp;

    // autoip
    gWifiSetting.setting.autoip = 0;

    // ipaddr
    strcpy(buf, theConfig.ipaddr);
    gWifiSetting.setting.ipaddr[0] = atoi(strtok_r(buf, ".", &saveptr));
    gWifiSetting.setting.ipaddr[1] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.ipaddr[2] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.ipaddr[3] = atoi(strtok_r(NULL, " ", &saveptr));

    // netmask
    strcpy(buf, theConfig.netmask);
    gWifiSetting.setting.netmask[0] = atoi(strtok_r(buf, ".", &saveptr));
    gWifiSetting.setting.netmask[1] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.netmask[2] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.netmask[3] = atoi(strtok_r(NULL, " ", &saveptr));

    // gateway
    strcpy(buf, theConfig.gw);
    gWifiSetting.setting.gw[0] = atoi(strtok_r(buf, ".", &saveptr));
    gWifiSetting.setting.gw[1] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.gw[2] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.gw[3] = atoi(strtok_r(NULL, " ", &saveptr));



}


#endif
static void* NetworkTask(void* arg)
{
#ifdef CFG_NET_WIFI

    int nTemp;
#else
    ResetEthernet();
#endif

    for (;;)
    {


#ifdef CFG_NET_WIFI
        gettimeofday(&tvEnd, NULL);

        nTemp = itpTimevalDiff(&tvStart, &tvEnd);
        if (nTemp>5000 && gInit == 0){
            printf("init wifi \n");
            //printf("[NetworkTask] wifi status = %d\n", theConfig.wifi_status);

			WifiMgr_clientMode_switch(theConfig.wifi_status);
            nTemp = wifiMgr_init(WIFIMGR_MODE_CLIENT, 0,gWifiSetting);
            gInit = 1;

        } else if (gInit == 1){
            networkIsReady = wifiMgr_is_wifi_available(&nTemp);
            networkIsReady = (bool)nTemp;
        }

#else
        networkIsReady = ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_CONNECTED, NULL);
#endif

        if (networkToReset)
        {
        #ifdef CFG_NET_WIFI

        #else
            ResetEthernet();
        #endif
            networkToReset = false;
        }
        sleep(1);
    }
    return NULL;
}

void NetworkInit(void)
{
    pthread_t task;

    networkIsReady = false;
    networkToReset = false;
    networkSocket = -1;
#ifdef CFG_NET_WIFI

    snprintf(gWifiSetting.ssid , WIFI_SSID_MAXLEN, theConfig.ssid);
    snprintf(gWifiSetting.password, WIFI_PASSWORD_MAXLEN, theConfig.password);
    snprintf(gWifiSetting.secumode, WIFI_SECUMODE_MAXLEN, theConfig.secumode);
    gWifiSetting.wifiCallback = wifiCallbackFucntion;
    ResetWifi();
    // init client mode
  //ret = wifiMgr_init(WIFIMGR_MODE_CLIENT, 0,gWifiSetting);

//    ret = wifiMgr_init(WIFIMGR_MODE_SOFTAP, 0,gWifiSetting);
    gettimeofday(&tvStart, NULL);


#endif

    pthread_create(&task, NULL, NetworkTask, NULL);
}

void NetworkReset(void)
{
    networkToReset  = true;
}

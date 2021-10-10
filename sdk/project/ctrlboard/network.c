#include <sys/ioctl.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
#include "lwip/ip.h"
#include "lwip/dns.h"
#include "ite/itp.h"
#include "iniparser/iniparser.h"
#include "ctrlboard.h"
#include "wifiMgr.h"
#ifdef CFG_NET_FTP_SERVER
#include <unistd.h>
#include "ftpd.h"
#endif

#define DHCP_TIMEOUT_MSEC (60 * 1000) //60sec

static struct timeval tvStart = {0, 0}, tvEnd = {0, 0};
static bool networkIsReady, networkToReset;
static int networkSocket;

#ifdef CFG_NET_WIFI
static WIFI_MGR_SETTING gWifiSetting;
static bool wifi_dongle_hotplug, need_reinit_wifimgr;
static int gInit =0; // wifi init
#endif


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
static ResetWifi()
{
//    ITPEthernetSetting setting;
    char buf[16], *saveptr;

    memset(&gWifiSetting.setting, 0, sizeof (ITPEthernetSetting));

    gWifiSetting.setting.index = 0;

    // dhcp
    gWifiSetting.setting.dhcp = 1;

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

static int wifiCallbackFucntion(int nState)
{
    switch (nState)
    {
        case WIFIMGR_STATE_CALLBACK_CONNECTION_FINISH:
            printf("[Indoor]WifiCallback connection finish \n");
            WebServerInit();

#ifdef CFG_NET_FTP_SERVER
		    ftpd_setlogin(theConfig.user_id, theConfig.user_password);
		    ftpd_init();
#endif
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

void NetworkWifiModeSwitch(void)
{
	int ret;

	ret = WifiMgr_Switch_ClientSoftAP_Mode(gWifiSetting);
}
#endif

static void* NetworkTask(void* arg)
{
#ifdef CFG_NET_WIFI
    int nTemp;
#else
    ResetEthernet();
#endif

#if defined(CFG_NET_FTP_SERVER) && !defined(CFG_NET_WIFI)
    ftpd_setlogin(theConfig.user_id, theConfig.user_password);
    ftpd_init();
#endif

    for (;;)
    {
#ifdef CFG_NET_WIFI
        gettimeofday(&tvEnd, NULL);

        nTemp = itpTimevalDiff(&tvStart, &tvEnd);
        if (nTemp>5000 && gInit == 0){
            printf("[%s] Init wifimgr \n", __FUNCTION__);

            while(!ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_DEVICE_READY, NULL)){
                sleep(1);
            }

			WifiMgr_clientMode_switch(theConfig.wifi_status);

            if (theConfig.wifi_mode == WIFI_CLIENT){
				ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_ENABLE, NULL); //determine wifi client mode
                nTemp = wifiMgr_init(WIFIMGR_MODE_CLIENT, 0, gWifiSetting);
			}else if (theConfig.wifi_mode == WIFI_SOFTAP){
				ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFIAP_ENABLE, NULL); //determine wifi softAP mode
				nTemp = wifiMgr_init(WIFIMGR_MODE_SOFTAP, 0, gWifiSetting);
			}

            gInit = 1;
        } else if (gInit == 1){
            networkIsReady = wifiMgr_is_wifi_available(&nTemp);
            networkIsReady = (bool)nTemp;

    	    /* Run Wifi dongle hot plug-in process, in order to re-init wifimgr. */
			//Step 1: check dongle is plug-in or not
            wifi_dongle_hotplug = ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_DEVICE_READY, NULL);

			//Step 2: If not plug-in then reinit flag set true
			if (!wifi_dongle_hotplug)
				need_reinit_wifimgr = true;

			//Step 3: If dongle hot plug-in and reinit flag is true, do wifimgr re-init
			if (wifi_dongle_hotplug && need_reinit_wifimgr){
				wifiMgr_init(WIFIMGR_MODE_CLIENT, 0, gWifiSetting);
				need_reinit_wifimgr = false;
			}
        }
#else
        networkIsReady = ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_CONNECTED, NULL);
#endif

        if (networkToReset)
        {
#ifndef CFG_NET_WIFI
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

    gettimeofday(&tvStart, NULL);
#endif

    pthread_create(&task, NULL, NetworkTask, NULL);
}

bool NetworkIsReady(void)
{
    return networkIsReady;
}

void NetworkReset(void)
{
    networkToReset  = true;
}

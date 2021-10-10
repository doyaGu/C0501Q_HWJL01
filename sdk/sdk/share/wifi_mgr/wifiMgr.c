
#include <pthread.h>
#include "ite/itp.h"
#include "lwip/netif.h"
#include "wifiMgr.h"
#include "../dhcps/dhcps.h"

extern void dhcps_init(void);
extern void dhcps_deinit(void);

#ifdef CFG_NET_WIFI
#include "ite/ite_wifi.h"

#define WIFI_STACK_SIZE                  80000   //(255 * 1024)
#define WIFI_INIFILENAME_MAXLEN          32
#define WIFI_CONNECT_COUNT               (60 * 10)
#define WIFI_SET_TIMEMAX                 (2 * 60 * 1000)      // mini second
#define WIFI_SWITCH_MODE_WAIT_TIME       (5 * 1000 * 1000)

#define WIFI_INIKEY_SOFTAP_SSID          "softap:ssid"
#define WIFI_INIKEY_SOFTAP_PASSWORD      "softap:password"
#define WIFI_INIKEY_SOFTAP_SECUMODE      "softap:secumode"
#define WIFI_INIKEY_SSID                 "wifi:ssid"
#define WIFI_INIKEY_PASSWORD             "wifi:password"
#define WIFI_INIKEY_SECUMODE             "wifi:secumode"
#ifdef CFG_AUDIOLINK_NETWORK_SWITCH_ENABLE
#define WIFI_INIKEY_NETWORK_SWITCH_NETWORK_MODE    "switchNetwork:networkMode"
#endif

#define WIFI_SECUVAL_NOT_AVAILABLE       "NA"
#define WIFI_SECUVAL_NOSEC               "0"
#define WIFI_SECUVAL_WEP                 "1"
#define WIFI_SECUVAL_WPAPSK              "2"
#define WIFI_SECUVAL_WPA2PSK             "3"
#define WIFI_SECUVAL_WPAPSK_MIX          "6" //for wpa tool
#define WIFI_SECUVAL_WPS                 "7" //for wps

#define WIFI_TRIEDSECU_NOSEC             0x1
#define WIFI_TRIEDSECU_WEP               0x2
#define WIFI_TRIEDSECU_WPAPSK            0x4
#define WIFI_TRIEDSECU_WPA2PSK           0x8
#define WIFI_TRIEDSECU_WPAPSK_MIX        0xF //for wpa tool
#define WIFIMGR_CHECK_WIFI_MSEC          1000
#define WIFIMGR_RECONNTECT_MSEC          (60 * 1000)
#define WIFIMGR_TEMPDISCONN_MSEC         (30 * 1000)
#define WIFI_IS_TURNED_CLOSE             0
#define WIFI_IS_TURNED_OPEN              1

static sem_t                             semConnectStart, semConnectStop;
static pthread_mutex_t                   mutexALWiFi, mutexIni, mutexMode;

static sem_t                             semWPSStart, semWPSStop;
static pthread_mutex_t                   mutexALWiFiWPS;

static struct netif                      al_wifi_netif;
static struct net_device_info            apInfo = {0}, gScanApInfo = {0};
static int                               mpMode = 0;
static int                               al_wifi_init = 0;
static int                               gwifiTerminate = 0;
static int                               need_set = false;
static int                               wps_cancel_flag = false;
static int                               soft_ap_init_ready = false;
static int                               connect_cancel_flag = false;
static int                               is_first_connect = true;     // first connect when the system start up
static int                               is_wifi_available = false;
static int                               is_temp_disconnect = false;  // is temporary disconnect
static int                               no_ssid = false;
static int                               no_ini_file = false;
static int                               wifi_client_status;
static char                              macSrting[32];
static char                              gSsid[WIFI_SSID_MAXLEN];
static char                              gPassword[WIFI_PASSWORD_MAXLEN];
static char                              gSecumode[WIFI_SECUMODE_MAXLEN];
static WIFIMGR_CONNSTATE_E               gWifi_connstate = WIFIMGR_CONNSTATE_STOP;
static WIFIMGR_CONNSTATE_E               wifi_conn_state = WIFIMGR_CONNSTATE_STOP;
static WIFIMGR_ECODE_E                   wifi_conn_ecode = WIFIMGR_ECODE_SET_DISCONNECT;
static WIFIMGR_MODE_E                    wifi_mode = 0;
static WIFI_MGR_SETTING                  gWifiMgrSetting = {0};
static WIFI_MGR_SCANAP_LIST              gWifiMgrApList[64];
//static int                                  hLed0 = 0, hLed1 = 0;

typedef enum tagWIFI_TRYAP_PHASE_E
{
    WIFI_TRYAP_PHASE_NONE = 0,
    WIFI_TRYAP_PHASE_SAME_SSID,
    WIFI_TRYAP_PHASE_EMPTY_SSID,
    WIFI_TRYAP_PHASE_FINISH,
    WIFI_TRYAP_PHASE_MAX,
} WIFI_TRYAP_PHASE_E;


static int
WifiMgr_WpaInit(void)
{
    int nRet = WIFIMGR_ECODE_OK;
	struct net_device_config netCfg = {0};

#ifndef CFG_NET_WIFI_WPA
    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_LINK_AP, &netCfg);
#else
    // start wpa state machine
    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_ENABLE, NULL);
    usleep(1000*1000);

    iteStartWPACtrl();
    do
    {
        usleep(1000);
    } while (!iteWPACtrlIsReady());
#endif

//    ping_init();

    return nRet;
}


static int
WifiMgr_WpaTerminate(void)
{
    int nRet = WIFIMGR_ECODE_OK;
    iteWPACtrlDisconnectNetwork();
    usleep(1000*1000);

    iteStopWPACtrl();
    do
    {
        usleep(1000*100);
    }while(iteStopWPADone());

    usleep(1000*100);

    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_DISABLE, NULL);
    usleep(1000*1000);

    return nRet;
}


static int
WifiMgr_HostAPInit(void)
{
    int nRet = WIFIMGR_ECODE_OK;
    ITPWifiInfo wifiInfo;

    //ioctl(hLed1, ITP_IOCTL_FLICKER, (void*)500);
    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFIAP_ENABLE, NULL);
    usleep(1000*1000);

    if (mpMode)
    {
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_GET_INFO, &wifiInfo);
        snprintf(macSrting, sizeof(macSrting), "_%02X:%02X", wifiInfo.hardwareAddress[4], wifiInfo.hardwareAddress[5]);
        iteHostapdSetSSIDWithMac(macSrting);
    }

    dhcps_init();
    usleep(1000*10);

    iteStartHostapdCtrl();

    do
    {
        usleep(1000);
    } while (!iteHOSTAPDCtrlIsReady());

    usleep(1000);
    //ioctl(hLed1, ITP_IOCTL_ON, NULL);

    return nRet;
}


static int
WifiMgr_HostAPTerminate(void)
{
    int nRet = WIFIMGR_ECODE_OK;

    iteStopHostapdCtrl();
    do
    {
        usleep(1000*200);
    }while(!iteStopHostapdDone());
    usleep(1000*200);

    dhcps_deinit();
    usleep(1000*10);

    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFIAP_DISABLE, NULL);
    usleep(1000*1000);

    return nRet;
}

#if 0
static int
WifiMgr_DtmfDecCallback(const DTMF_DEC_RESULT *decResult)
{
    int nRet = WIFIMGR_ECODE_OK;


    return nRet;
}
#endif

static int
WifiMgr_ScanAP(void)
{
    int nRet = WIFIMGR_ECODE_OK;
    int nWifiState = 0;
    int i = 0, j = 0, k = 0;
    struct net_device_info tmpApInfo = {0};
    int found1 = 0;

    memset(&apInfo, 0, sizeof(struct net_device_info));

    if (connect_cancel_flag) {
        goto end;
    }

    for (i = 0; i < 5; i++)
    {
        printf("[WIFIMGR]%s() SCAN Round <%ld> ==========================\r\n", __FUNCTION__, i);
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_SCAN, NULL);
        while (1)
        {
            nWifiState = ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_STATE, NULL);
            printf("[WIFIMGR]%s() nWifiState=0x%X\r\n", __FUNCTION__, nWifiState);
            if ((nWifiState & WLAN_SITE_MONITOR) == 0) {
                // scan finish
                printf("[WIFIMGR]%s() Scan AP Finish!\r\n", __FUNCTION__);
                break;
            }

            if (connect_cancel_flag) {
                goto end;
            }

            usleep(1000 * 1000);
        }

        memset(&tmpApInfo, 0, sizeof(struct net_device_info));
        read(ITP_DEVICE_WIFI, &tmpApInfo, (size_t)NULL);
        printf("[WIFIMGR]%s() L#%ld: tmpApInfo.apCnt = %ld\r\n", __FUNCTION__, __LINE__, tmpApInfo.apCnt);
        for (j = 0; j < tmpApInfo.apCnt; j++)
        {
            found1 = 0;
            for (k = 0; k < apInfo.apCnt; k++)
            {
                if (!memcmp(apInfo.apList[k].apMacAddr, tmpApInfo.apList[j].apMacAddr, 6)){
                    found1 = 1;
                    break;
                }
            }

            if (!found1 && apInfo.apCnt < 64) {
                memcpy(&apInfo.apList[apInfo.apCnt], &tmpApInfo.apList[j], sizeof(apInfo.apList[0]));
                apInfo.apCnt++;
            }

            if (connect_cancel_flag) {
                goto end;
            }
        }

        if (connect_cancel_flag) {
            goto end;
        }
    }

    if (connect_cancel_flag) {
        goto end;
    }

    printf("[WIFIMGR]%s() L#%ld: apInfo.apCnt = %ld\r\n", __FUNCTION__, __LINE__, apInfo.apCnt);
    for (i = 0; i < apInfo.apCnt; i++)
    {
        printf("[WIFIMGR] ssid = %32s, securityOn = %ld, securityMode = %ld, <%02x:%02x:%02x:%02x:%02x:%02x>\r\n", apInfo.apList[i].ssidName, apInfo.apList[i].securityOn, apInfo.apList[i].securityMode,
            apInfo.apList[i].apMacAddr[0], apInfo.apList[i].apMacAddr[1], apInfo.apList[i].apMacAddr[2], apInfo.apList[i].apMacAddr[3], apInfo.apList[i].apMacAddr[4], apInfo.apList[i].apMacAddr[5]);
    }

end:

    if (connect_cancel_flag) {
        printf("[WIFIMGR WPS]%s() L#%ld: End. wps_cancel_flag is set.\r\n", __FUNCTION__, __LINE__);
    }

    return nRet;
}

static int
WifiMgr_Set(void)
{
    int nRet = WIFIMGR_ECODE_OK;

    return nRet;
}


static int
WifiMgr_Connect(void)
{
    int nRet = WIFIMGR_ECODE_OK;

    char* ssid;
    char* password;
    char* secumode;

    struct net_device_config netCfg = {0};
    unsigned long connect_cnt = 0;
    int is_connected = 0, dhcp_available = 0;
    ITPWifiInfo wifiInfo;
    int nSecurity = -1;

    int phase = 0, lastIdx = 0, triedSecu = 0, is_ssid_match = 0;
    ITPEthernetSetting setting;

    if (connect_cancel_flag) {
        goto end;
    }

    ssid = gSsid;
    password = gPassword;
    secumode = gSecumode;

    if (mpMode) {
        printf("[WIFIMGR] Is mpMode, connect to default SSID.\r\n");
        // SSID
        snprintf(ssid, WIFI_SSID_MAXLEN, "%s", CFG_NET_WIFI_MP_SSID);
        // Password
        snprintf(password, WIFI_PASSWORD_MAXLEN, "%s", CFG_NET_WIFI_MP_PASSWORD);
#ifdef DTMF_DEC_HAS_SECUMODE
        // Security mode
        snprintf(secumode, WIFI_SECUMODE_MAXLEN, "%s", CFG_NET_WIFI_MP_SECURITY);
#endif
    }
    else
    {
#if 0
        // read the wifi setting from ini
#ifdef CFG_USE_SD_INI
        snprintf(ini_filename, WIFI_INIFILENAME_MAXLEN, "%s:/%s", "C", CFG_NET_URENDER_CONFIG_INI_FILE);
#else
        snprintf(ini_filename, WIFI_INIFILENAME_MAXLEN, "%s:/%s", CFG_PUBLIC_DRIVE, CFG_NET_URENDER_CONFIG_INI_FILE);
#endif
        pthread_mutex_lock(&mutexIni);
        ini = iniparser_load(ini_filename);
    	if (ini == NULL)
        {
            printf("[WIFIMGR]%s() L#%ld: Error! Cannot load ini file: %s!!\r\n", __FUNCTION__, __LINE__, ini_filename);
            ioctl(hLed1, ITP_IOCTL_OFF, NULL);
    		nRet = WIFIMGR_ECODE_NO_INI_FILE;
            printf("[WIFIMGR]%s() L#%ld: Do ugResetFactory() and reboot!\r\n", __FUNCTION__, __LINE__);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*)ITP_DISK_NOR);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORCE_MOUNT, (void*)ITP_DISK_NOR);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORMAT, (void*)1);
            ugResetFactory();
            i2s_CODEC_standby();
            exit(0);
            while (1);
            goto end;
    	}

        // SSID
        snprintf(ssid, WIFI_SSID_MAXLEN, iniparser_getstring(ini, WIFI_INIKEY_SSID, ""));
        // Password
        snprintf(password, WIFI_PASSWORD_MAXLEN, iniparser_getstring(ini, WIFI_INIKEY_PASSWORD, ""));
#ifdef DTMF_DEC_HAS_SECUMODE
        // Security mode
        snprintf(secumode, WIFI_SECUMODE_MAXLEN, iniparser_getstring(ini, WIFI_INIKEY_SECUMODE, "NA"));
#endif

        // dhcp
        setting.dhcp = iniparser_getboolean(ini, "tcpip:dhcp", 1);

        // autoip
        setting.autoip = iniparser_getboolean(ini, "tcpip:autoip", 0);

        // ipaddr
        setting.ipaddr[0] = atoi(strtok(iniparser_getstring(ini, "tcpip:ipaddr", "192.168.1.1"), "."));
        setting.ipaddr[1] = atoi(strtok(NULL, "."));
        setting.ipaddr[2] = atoi(strtok(NULL, "."));
        setting.ipaddr[3] = atoi(strtok(NULL, " "));

        // netmask
        setting.netmask[0] = atoi(strtok(iniparser_getstring(ini, "tcpip:netmask", "255.255.255.0"), "."));
        setting.netmask[1] = atoi(strtok(NULL, "."));
        setting.netmask[2] = atoi(strtok(NULL, "."));
        setting.netmask[3] = atoi(strtok(NULL, " "));

        // gateway
        setting.gw[0] = atoi(strtok(iniparser_getstring(ini, "tcpip:gw", "192.168.1.254"), "."));
        setting.gw[1] = atoi(strtok(NULL, "."));
        setting.gw[2] = atoi(strtok(NULL, "."));
        setting.gw[3] = atoi(strtok(NULL, " "));

        iniparser_freedict(ini);
        pthread_mutex_unlock(&mutexIni);
#endif
    }


    // dhcp
    setting.dhcp = gWifiMgrSetting.setting.dhcp;

    // autoip
    setting.autoip = gWifiMgrSetting.setting.autoip;

    // ipaddr
    setting.ipaddr[0] = gWifiMgrSetting.setting.ipaddr[0];
    setting.ipaddr[1] = gWifiMgrSetting.setting.ipaddr[1];
    setting.ipaddr[2] = gWifiMgrSetting.setting.ipaddr[2];
    setting.ipaddr[3] = gWifiMgrSetting.setting.ipaddr[3];

    // netmask
    setting.netmask[0] = gWifiMgrSetting.setting.netmask[0];
    setting.netmask[1] = gWifiMgrSetting.setting.netmask[1];
    setting.netmask[2] = gWifiMgrSetting.setting.netmask[2];
    setting.netmask[3] = gWifiMgrSetting.setting.netmask[3];

    // gateway
    setting.gw[0] = gWifiMgrSetting.setting.gw[0];
    setting.gw[1] = gWifiMgrSetting.setting.gw[1];
    setting.gw[2] = gWifiMgrSetting.setting.gw[2];
    setting.gw[3] = gWifiMgrSetting.setting.gw[3];

    printf("[WIFIMGR] ssid     = %s\r\n", ssid);
    printf("[WIFIMGR] password = %s\r\n", password);
    printf("[WIFIMGR] secumode = %s\r\n", secumode);

    if (connect_cancel_flag) {
        goto end;
    }

    if (strlen(ssid) == 0)
    {
        printf("[WIFIMGR]%s() L#%ld: Error! Wifi setting has no SSID\r\n", __FUNCTION__, __LINE__);
        //ioctl(hLed1, ITP_IOCTL_OFF, NULL);
        nRet = WIFIMGR_ECODE_NO_SSID;
        goto end;
    }

#if defined (CFG_NET_ETHERNET_WIFI)
    printf("[WIFIMGR] check wifi netif %d \n",ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_NETIF_STATUS, NULL));
    // Check if the wifi netif is exist
    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_NETIF_STATUS, NULL) == 0) {
        printf("[WIFIMGR]%s() L#%ld: wifi need to add netif !\r\n", __FUNCTION__, __LINE__);
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_ADD_NETIF, NULL);
    }
#endif

    memset(&netCfg, 0, sizeof(struct net_device_config));

    if (!strcmp(secumode, WIFI_SECUVAL_NOT_AVAILABLE))
    {
        int i = 0, found1 = 0;

        wifi_conn_state = WIFIMGR_CONNSTATE_SCANNING;
        WifiMgr_ScanAP();
        wifi_conn_state = WIFIMGR_CONNSTATE_CONNECTING;

        phase = WIFI_TRYAP_PHASE_SAME_SSID;
        lastIdx = 0;
        triedSecu = 0;

        if (connect_cancel_flag) {
            goto end;
        }

retry:
        nSecurity = -1;
        found1 = 0;
        for (i = lastIdx; i < apInfo.apCnt; i++)
        {
            is_ssid_match = 0;

            if (phase == WIFI_TRYAP_PHASE_SAME_SSID) {
                // search for the same SSID
                if (!strcmp(ssid, apInfo.apList[i].ssidName)) {
                    is_ssid_match = 1;
                }
            } else if (phase == WIFI_TRYAP_PHASE_EMPTY_SSID) {
                // search for the empty SSID
                if (strlen(apInfo.apList[i].ssidName) == 0) {
                    is_ssid_match = 1;
                }
            }

            if (is_ssid_match)
            {
                nSecurity = (apInfo.apList[i].securityOn) ? apInfo.apList[i].securityMode : WLAN_SEC_NOSEC;
                switch (nSecurity)
                {
                case WLAN_SEC_NOSEC:
                    if ((triedSecu & WIFI_TRIEDSECU_NOSEC) == 0)
                    {
                        triedSecu |= WIFI_TRIEDSECU_NOSEC;
                        found1 = 1;
                    }
                    break;
                case WLAN_SEC_WEP:
                    if ((triedSecu & WIFI_TRIEDSECU_WEP) == 0)
                    {
                        triedSecu |= WIFI_TRIEDSECU_WEP;
                        found1 = 1;
                    }
                    break;
                case WLAN_SEC_WPAPSK:
                    if ((triedSecu & WIFI_TRIEDSECU_WPAPSK) == 0)
                    {
                        triedSecu |= WIFI_TRIEDSECU_WPAPSK;
                        found1 = 1;
                    }
                    break;
                case WLAN_SEC_WPA2PSK:
                    if ((triedSecu & WIFI_TRIEDSECU_WPA2PSK) == 0)
                    {
                        triedSecu |= WIFI_TRIEDSECU_WPA2PSK;
                        found1 = 1;
                    }
                    break;
		case WLAN_SEC_WPAPSK_MIX:
                    if ((triedSecu & WIFI_TRIEDSECU_WPAPSK_MIX) == 0)
                    {
                        triedSecu |= WIFI_TRIEDSECU_WPAPSK_MIX;
                        found1 = 1;
                    }
                    break;
                }
            }

            if (found1)
            {
                printf("[WIFIMGR]%s() Found 1 AP matches! ssid = %s, securityOn = %ld, securityMode = %ld, <%02x:%02x:%02x:%02x:%02x:%02x>\r\n",
                    __FUNCTION__, apInfo.apList[i].ssidName, apInfo.apList[i].securityOn, apInfo.apList[i].securityMode,
                    apInfo.apList[i].apMacAddr[0], apInfo.apList[i].apMacAddr[1], apInfo.apList[i].apMacAddr[2], apInfo.apList[i].apMacAddr[3], apInfo.apList[i].apMacAddr[4], apInfo.apList[i].apMacAddr[5]);
                lastIdx = i + 1;
                break;
            }

            if (connect_cancel_flag)
            {
                goto end;
            }
        }

        if ((triedSecu == WIFI_TRIEDSECU_NOSEC | WIFI_TRIEDSECU_WEP | WIFI_TRIEDSECU_WPAPSK | WIFI_TRIEDSECU_WPA2PSK | WIFI_TRIEDSECU_WPAPSK_MIX) ||
            ((phase == WIFI_TRYAP_PHASE_EMPTY_SSID) && (i == apInfo.apCnt)))
        {
            phase = WIFI_TRYAP_PHASE_FINISH;
        }

        if ((phase == WIFI_TRYAP_PHASE_SAME_SSID) && (i == apInfo.apCnt))
        {
            phase = WIFI_TRYAP_PHASE_EMPTY_SSID;
            lastIdx = 0;
        }

        if (connect_cancel_flag)
        {
            goto end;
        }

        if (found1)
        {
            if (!apInfo.apList[i].securityOn || apInfo.apList[i].securityMode == WLAN_SEC_NOSEC)
            {
                netCfg.operationMode = WLAN_MODE_STA;
                snprintf(netCfg.ssidName, WIFI_SSID_MAXLEN, ssid);
                netCfg.securitySuit.securityMode = WLAN_SEC_NOSEC;
            }
            else if (apInfo.apList[i].securityMode == WLAN_SEC_WEP)
            {
                netCfg.operationMode = WLAN_MODE_STA;
                snprintf(netCfg.ssidName, WIFI_SSID_MAXLEN, ssid);
                netCfg.securitySuit.securityMode = WLAN_SEC_WEP;
                netCfg.securitySuit.authMode = WLAN_AUTH_OPENSYSTEM;
                snprintf(netCfg.securitySuit.wepKeys[0], WIFI_PASSWORD_MAXLEN, password);
                snprintf(netCfg.securitySuit.wepKeys[1], WIFI_PASSWORD_MAXLEN, password);
                snprintf(netCfg.securitySuit.wepKeys[2], WIFI_PASSWORD_MAXLEN, password);
                snprintf(netCfg.securitySuit.wepKeys[3], WIFI_PASSWORD_MAXLEN, password);
                netCfg.securitySuit.defaultKeyId = 0; /*From 0 to 3*/
            }
            else if (apInfo.apList[i].securityMode == WLAN_SEC_WPAPSK)
            {
                netCfg.operationMode = WLAN_MODE_STA;
                snprintf(netCfg.ssidName, WIFI_SSID_MAXLEN, ssid);
                netCfg.securitySuit.securityMode = WLAN_SEC_WPAPSK;
                snprintf(netCfg.securitySuit.preShareKey, WIFI_PASSWORD_MAXLEN, password);
            }
            else if (apInfo.apList[i].securityMode == WLAN_SEC_WPA2PSK)
            {
                netCfg.operationMode = WLAN_MODE_STA;
                snprintf(netCfg.ssidName, WIFI_SSID_MAXLEN, ssid);
                netCfg.securitySuit.securityMode = WLAN_SEC_WPA2PSK;
                snprintf(netCfg.securitySuit.preShareKey, WIFI_PASSWORD_MAXLEN, password);
            }
            else if (apInfo.apList[i].securityMode == WLAN_SEC_WPAPSK_MIX)
            {
                netCfg.operationMode = WLAN_MODE_STA;
                snprintf(netCfg.ssidName, WIFI_SSID_MAXLEN, ssid);
                netCfg.securitySuit.securityMode = WLAN_SEC_WPAPSK_MIX;
                snprintf(netCfg.securitySuit.preShareKey, WIFI_PASSWORD_MAXLEN, password);
            }
        }
        else
        {
            if (phase == WIFI_TRYAP_PHASE_FINISH) {
                printf("[WIFIMGR]%s() L#%ld: Cannot find the same SSID on air. Unknown Security! Cannot connect to WiFi AP!\r\n", __FUNCTION__, __LINE__);
                //ioctl(hLed1, ITP_IOCTL_OFF, NULL);
                nRet = WIFIMGR_ECODE_UNKNOWN_SECURITY;
                goto end;
            } else {
                printf("[WIFIMGR]%s() L#%ld: Cannot find the same SSID on air. Goto retry.\r\n", __FUNCTION__, __LINE__);
                goto retry;
            }
        }
    }
    else if (!strcmp(secumode, WIFI_SECUVAL_NOSEC))
    {
        netCfg.operationMode = WLAN_MODE_STA;
        snprintf(netCfg.ssidName, WIFI_SSID_MAXLEN, ssid);
        netCfg.securitySuit.securityMode = WLAN_SEC_NOSEC;
    }
    else if (!strcmp(secumode, WIFI_SECUVAL_WEP))
    {
        netCfg.operationMode = WLAN_MODE_STA;
        snprintf(netCfg.ssidName, WIFI_SSID_MAXLEN, ssid);
        netCfg.securitySuit.securityMode = WLAN_SEC_WEP;
        netCfg.securitySuit.authMode = WLAN_AUTH_OPENSYSTEM;
        snprintf(netCfg.securitySuit.wepKeys[0], WIFI_PASSWORD_MAXLEN, password);
        snprintf(netCfg.securitySuit.wepKeys[1], WIFI_PASSWORD_MAXLEN, password);
        snprintf(netCfg.securitySuit.wepKeys[2], WIFI_PASSWORD_MAXLEN, password);
        snprintf(netCfg.securitySuit.wepKeys[3], WIFI_PASSWORD_MAXLEN, password);
        netCfg.securitySuit.defaultKeyId = 0; /*From 0 to 3*/
    }
    else if (!strcmp(secumode, WIFI_SECUVAL_WPAPSK))
    {
        netCfg.operationMode = WLAN_MODE_STA;
        snprintf(netCfg.ssidName, WIFI_SSID_MAXLEN, ssid);
        netCfg.securitySuit.securityMode = WLAN_SEC_WPAPSK;
        snprintf(netCfg.securitySuit.preShareKey, WIFI_PASSWORD_MAXLEN, password);
    }
    else if (!strcmp(secumode, WIFI_SECUVAL_WPA2PSK))
    {
        netCfg.operationMode = WLAN_MODE_STA;
        snprintf(netCfg.ssidName, WIFI_SSID_MAXLEN, ssid);
        netCfg.securitySuit.securityMode = WLAN_SEC_WPA2PSK;
        snprintf(netCfg.securitySuit.preShareKey, WIFI_PASSWORD_MAXLEN, password);
    }
    else if (!strcmp(secumode, WIFI_SECUVAL_WPAPSK_MIX))
    {
        netCfg.operationMode = WLAN_MODE_STA;
        snprintf(netCfg.ssidName, WIFI_SSID_MAXLEN, ssid);
        netCfg.securitySuit.securityMode = WLAN_SEC_WPAPSK_MIX;
        snprintf(netCfg.securitySuit.preShareKey, WIFI_PASSWORD_MAXLEN, password);
    }

    if (connect_cancel_flag) {
        goto end;
    }

    // try to connect to WiFi AP
	if (wifi_client_status == WIFI_IS_TURNED_OPEN){
        iteWPACtrlConnectNetwork(&netCfg);
	}

    // Wait for connecting...
    printf("[WIFIMGR] Wait for connecting");
    connect_cnt = WIFI_CONNECT_COUNT;
    while (connect_cnt)
    {
        if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_CONNECTED, NULL) && iteWPAConnectState())
        {
            printf("\r\n[WIFIMGR] WiFi AP is connected!\r\n");
            is_connected = 1;
            break;
        }
        putchar('.');
        fflush(stdout);
        connect_cnt--;
        if (connect_cnt == 0)
        {
            printf("\r\n[WIFIMGR]%s() L#%ld: Timeout! Cannot connect to %s!\r\n", __FUNCTION__, __LINE__, ssid);
#ifdef CFG_AUDIOLINK_STATUS_TRANSPORT_ENABLE
            WifiMgrConnectFail();
#endif
            break;
        }

        if (connect_cancel_flag)
        {
            goto end;
        }

        usleep(100000);
    }

    if (connect_cancel_flag)
    {
        goto end;
    }

    if (!is_connected)
    {
        if (!strcmp(secumode, WIFI_SECUVAL_NOT_AVAILABLE) &&
            (phase != WIFI_TRYAP_PHASE_FINISH))
        {
            printf("[WIFIMGR]%s() L#%ld: Error! Cannot connect to WiFi AP! Goto retry.\r\n", __FUNCTION__, __LINE__);
            goto retry;
        }
        else
        {
            printf("[WIFIMGR]%s() L#%ld: Error! Cannot connect to WiFi AP!\r\n", __FUNCTION__, __LINE__);
            //ioctl(hLed1, ITP_IOCTL_OFF, NULL);
            nRet = WIFIMGR_ECODE_CONNECT_ERROR;
            gWifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_CLIENT_MODE_CONNECTING_FAIL);
            goto end;
        }
    }

    if (setting.dhcp) {
        // Wait for DHCP setting...
        printf("[WIFIMGR] Wait for DHCP setting");

    	ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_START_DHCP, NULL);

        connect_cnt = WIFI_CONNECT_COUNT;
        while (connect_cnt)
        {
            if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_AVAIL, NULL))
            {
                printf("\r\n[WIFIMGR] DHCP setting OK\r\n");
                dhcp_available = 1;
                break;
            }
            putchar('.');
            fflush(stdout);
            connect_cnt--;
            if (connect_cnt == 0)
            {
                if (!strcmp(secumode, WIFI_SECUVAL_NOT_AVAILABLE) &&
                    (phase != WIFI_TRYAP_PHASE_FINISH))
                {
                    printf("\r\n[WIFIMGR]%s() L#%ld: DHCP timeout! Goto retry.\r\n", __FUNCTION__, __LINE__);
                    goto retry;
                }
                else
                {
                    printf("\r\n[WIFIMGR]%s() L#%ld: DHCP timeout! connect fail!\r\n", __FUNCTION__, __LINE__);
                    //ioctl(hLed1, ITP_IOCTL_OFF, NULL);
                    nRet = WIFIMGR_ECODE_DHCP_ERROR;
                    goto end;
                }
            }

            if (connect_cancel_flag)
            {
                goto end;
            }

            usleep(100000);
        }

        if (connect_cancel_flag)
        {
            goto end;
        }
    }
    else
    {
        printf("[WIFIMGR] Manual setting IP\n");
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_RESET, &setting);
        dhcp_available = 1;
    }

    if (dhcp_available)
    {
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_GET_INFO, &wifiInfo);
        printf("[WIFIMGR] wifiInfo.active          = %ld\r\n", wifiInfo.active);
        printf("[WIFIMGR] wifiInfo.address         = 0x%X\r\n", wifiInfo.address);
        printf("[WIFIMGR] wifiInfo.address         = %ld.%ld.%ld.%ld\r\n",
                (wifiInfo.address & 0xFF), (wifiInfo.address & 0xFF00) >> 8, (wifiInfo.address & 0xFF0000) >> 16, (wifiInfo.address & 0xFF000000) >> 24);
        printf("[WIFIMGR] wifiInfo.displayName     = %s\r\n", wifiInfo.displayName);
        printf("[WIFIMGR] wifiInfo.hardwareAddress = %02X:%02X:%02X:%02X:%02X:%02X\r\n", wifiInfo.hardwareAddress[0], wifiInfo.hardwareAddress[1], wifiInfo.hardwareAddress[2], wifiInfo.hardwareAddress[3], wifiInfo.hardwareAddress[4], wifiInfo.hardwareAddress[5]);
        printf("[WIFIMGR] wifiInfo.name            = %s\r\n", wifiInfo.name);
        //ioctl(hLed1, ITP_IOCTL_ON, NULL);

        // write back to ini
        if (!strcmp(secumode, WIFI_SECUVAL_NOT_AVAILABLE) &&
            nSecurity != -1)
        {
#if 0
            pthread_mutex_lock(&mutexIni);
            ini = iniparser_load(ini_filename);
            if (ini == NULL)
            {
                printf("[WIFIMGR]%s() L#%ld: Error! Cannot load ini file: %s!!\r\n", __FUNCTION__, __LINE__, ini_filename);
                pthread_mutex_unlock(&mutexIni);
                goto end;
            }
#endif

#if 0 //def DTMF_DEC_HAS_SECUMODE
            switch (nSecurity)
            {
            case WLAN_SEC_NOSEC:
                iniparser_set(ini, (char *)WIFI_INIKEY_SECUMODE, WIFI_SECUVAL_NOSEC);
                break;
            case WLAN_SEC_WEP:
                iniparser_set(ini, (char *)WIFI_INIKEY_SECUMODE, WIFI_SECUVAL_WEP);
                break;
            case WLAN_SEC_WPAPSK:
                iniparser_set(ini, (char *)WIFI_INIKEY_SECUMODE, WIFI_SECUVAL_WPAPSK);
                break;
            case WLAN_SEC_WPA2PSK:
                iniparser_set(ini, (char *)WIFI_INIKEY_SECUMODE, WIFI_SECUVAL_WPA2PSK);
                break;
			case WLAN_SEC_WPAPSK_MIX:
                iniparser_set(ini, (char *)WIFI_INIKEY_SECUMODE, WIFI_SECUVAL_WPAPSK_MIX);
                break;
            default:
                printf("[WIFIMGR]%s() L#%ld: Error! Invalid nSecurity = %ld\r\n", __FUNCTION__, __LINE__, nSecurity);
                iniparser_freedict(ini);
                pthread_mutex_unlock(&mutexIni);
                goto end;
                break;
            }
#endif

#if 0
            pFile = fopen(ini_filename, "w");
            if (pFile == NULL)
            {
                printf("[WIFIMGR]%s() L#%ld: Error! Cannot open ini file: %s to write!!\r\n", __FUNCTION__, __LINE__, ini_filename);
                iniparser_freedict(ini);
                pthread_mutex_unlock(&mutexIni);
                goto end;
            }

            printf("[WIFIMGR]%s() L#%ld: Write data to ini %s\r\n", __FUNCTION__, __LINE__, ini_filename);
            iniparser_dump_ini(ini, pFile);
            fclose(pFile);
#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
            ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_DISABLE, NULL);
            ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
            ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_ENABLE, NULL);
#endif
            iniparser_freedict(ini);
            pthread_mutex_unlock(&mutexIni);
#endif

        }

        usleep(1000*1000*5); //workaround random miss frames issue for cisco router

        if (gWifiMgrSetting.wifiCallback)
            gWifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_CONNECTION_FINISH);
    }

end:

    if (connect_cancel_flag)
    {
        printf("[WIFIMGR]%s() L#%ld: End. connect_cancel_flag is set.\r\n", __FUNCTION__, __LINE__);
    }

    return nRet;
}


static int
WifiMgr_WPS(void)
{
    int nRet = WIFIMGR_ECODE_OK;
    struct net_device_config netCfg = {0};
    unsigned long connect_cnt = 0;
    int is_connected = 0, dhcp_available = 0;
    ITPWifiInfo wifiInfo;
    ITPEthernetSetting setting;

    struct net_device_config wpsNetCfg = {0};
    int len = 0;
    char ssid[WIFI_SSID_MAXLEN];
    char password[WIFI_PASSWORD_MAXLEN];

    if (wps_cancel_flag) {
        goto end;
    }

    netCfg.operationMode = WLAN_MODE_STA;
    memset(netCfg.ssidName, 0, sizeof(netCfg.ssidName));
    netCfg.securitySuit.securityMode = WLAN_SEC_WPS;

    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_CONNECTED, NULL)) {

        iteWPACtrlDisconnectNetwork();
        usleep(1000*100);
        // dhcp
        setting.dhcp = 0;

        // autoip
        setting.autoip = 0;

        // ipaddr
        setting.ipaddr[0] =0;
        setting.ipaddr[1] = 0;
        setting.ipaddr[2] = 0;
        setting.ipaddr[3] = 0;

        // netmask
        setting.netmask[0] = 0;
        setting.netmask[1] = 0;
        setting.netmask[2] = 0;
        setting.netmask[3] = 0;

        // gateway
        setting.gw[0] = 0;
        setting.gw[1] = 0;
        setting.gw[2] = 0;
        setting.gw[3] = 0;

        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_RESET, &setting);

    }

    iteWPACtrlConnectNetwork(&netCfg);

    if (wps_cancel_flag)
    {
        goto end;
    }

    // Wait for connecting...
    printf("[WIFIMGR WPS] Wait for connecting");
    connect_cnt = WIFI_CONNECT_COUNT;
    while (connect_cnt)
    {
        if (iteWPACtrlWpsIsComplete()) {
            printf("\r\n[WIFIMGR WPS] WiFi AP is connected!\r\n");
            connect_cancel_flag = true;
            is_connected = 1;
            break;
        }
        putchar('.');
        fflush(stdout);
        connect_cnt--;
        if (connect_cnt == 0) {
            printf("\r\n[WIFIMGR WPS]%s() L#%ld: Timeout! Cannot connect to WIFI AP!\r\n", __FUNCTION__, __LINE__);
            break;
        }

        if (wps_cancel_flag) {
            goto end;
        }

        usleep(100000);
    }

    if (!is_connected) {
        printf("[WIFIMGR WPS]%s() L#%ld: Error! Cannot connect to WiFi AP!\r\n", __FUNCTION__, __LINE__);
        nRet = WIFIMGR_ECODE_CONNECT_ERROR;
        goto end;
    }

    if (wps_cancel_flag) {
        goto end;
    }

    // Wait for DHCP setting...
    printf("[WIFIMGR WPS] Wait for DHCP setting");
    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_START_DHCP, NULL);
    connect_cnt = WIFI_CONNECT_COUNT;
    while (connect_cnt)
    {
        if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_AVAIL, NULL)) {
            printf("\r\n[WIFIMGR WPS] DHCP setting OK\r\n");
            dhcp_available = 1;
            break;
        }
        putchar('.');
        fflush(stdout);
        connect_cnt--;
        if (connect_cnt == 0) {
            printf("\r\n[WIFIMGR WPS]%s() L#%ld: DHCP timeout! connect fail!\r\n", __FUNCTION__, __LINE__);
            nRet = WIFIMGR_ECODE_DHCP_ERROR;
            goto end;
        }

        if (wps_cancel_flag) {
            goto end;
        }
        usleep(100000);
    }

    if (dhcp_available)
    {
#if 0
        char ini_filename[WIFI_INIFILENAME_MAXLEN];
        dictionary* ini = NULL;
        FILE *pFile = NULL;
        char secu_str[5];

        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_GET_INFO, &wifiInfo);
        printf("[WIFIMGR WPS] wifiInfo.active          = %ld\r\n", wifiInfo.active);
        printf("[WIFIMGR WPS] wifiInfo.address         = 0x%X\r\n", wifiInfo.address);
        printf("[WIFIMGR WPS] wifiInfo.address         = %ld.%ld.%ld.%ld\r\n",
                (wifiInfo.address & 0xFF), (wifiInfo.address & 0xFF00) >> 8, (wifiInfo.address & 0xFF0000) >> 16, (wifiInfo.address & 0xFF000000) >> 24);
        printf("[WIFIMGR WPS] wifiInfo.displayName     = %s\r\n", wifiInfo.displayName);
        printf("[WIFIMGR WPS] wifiInfo.hardwareAddress = %2X:%2X:%2X:%2X:%2X:%2X\r\n", wifiInfo.hardwareAddress[0], wifiInfo.hardwareAddress[1], wifiInfo.hardwareAddress[2], wifiInfo.hardwareAddress[3], wifiInfo.hardwareAddress[4], wifiInfo.hardwareAddress[5]);
        printf("[WIFIMGR WPS] wifiInfo.name            = %s\r\n", wifiInfo.name);
        // Get WPS info
        iteWPACtrlGetNetwork(&wpsNetCfg);
        // trim the " char
        memset(ssid, 0, WIFI_SSID_MAXLEN);
        len = strlen(wpsNetCfg.ssidName);
        memcpy(ssid, wpsNetCfg.ssidName + 1, len - 2);
        memset(password, 0, WIFI_PASSWORD_MAXLEN);
        len = strlen(wpsNetCfg.securitySuit.preShareKey);
        memcpy(password, wpsNetCfg.securitySuit.preShareKey + 1, len - 2);

        printf("[WIFIMGR WPS] WPS Info:\r\n");
        printf("[WIFIMGR WPS] WPS SSID     = %s\r\n", ssid);
        printf("[WIFIMGR WPS] WPS Password = %s\r\n", password);
        printf("[WIFIMGR WPS] WPS Security = %ld\r\n", wpsNetCfg.securitySuit.securityMode);
        // Write back WPS info to ini
    #ifdef CFG_USE_SD_INI
        snprintf(ini_filename, WIFI_INIFILENAME_MAXLEN, "%s:/%s", "C", CFG_NET_URENDER_CONFIG_INI_FILE);
    #else
        snprintf(ini_filename, WIFI_INIFILENAME_MAXLEN, "%s:/%s", CFG_PUBLIC_DRIVE, CFG_NET_URENDER_CONFIG_INI_FILE);
    #endif
        pthread_mutex_lock(&mutexIni);
        ini = iniparser_load(ini_filename);
        if (ini == NULL)
        {
            printf("[WIFIMGR]%s() L#%ld: Error! Cannot load ini file: %s!!\r\n", __FUNCTION__, __LINE__, ini_filename);
            printf("[WIFIMGR]%s() L#%ld: Do ugResetFactory() and reboot!\r\n", __FUNCTION__, __LINE__);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*)ITP_DISK_NOR);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORCE_MOUNT, (void*)ITP_DISK_NOR);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORMAT, (void*)1);
            ugResetFactory();
            i2s_CODEC_standby();
            exit(0);
            while (1);
            goto end;
        }

        iniparser_set(ini, (char *)WIFI_INIKEY_SSID, ssid);
        iniparser_set(ini, (char *)WIFI_INIKEY_PASSWORD, password);

        pFile = fopen(ini_filename, "w");
        if (pFile == NULL) {
            printf("[WIFIMGR]%s() L#%ld: Error! Cannot open ini file: %s to write!!\r\n", __FUNCTION__, __LINE__, ini_filename);
            iniparser_freedict(ini);
            goto end;
        }

        printf("[WIFIMGR]%s() L#%ld: Write data to ini %s\r\n", __FUNCTION__, __LINE__, ini_filename);
        iniparser_dump_ini(ini, pFile);
        fclose(pFile);
    #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
        ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_DISABLE, NULL);
        ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
        ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_ENABLE, NULL);
    #endif
        iniparser_freedict(ini);
        pthread_mutex_unlock(&mutexIni);

        //ioctl(hLed1, ITP_IOCTL_ON, NULL);
#else

        // Get WPS info
        iteWPACtrlGetNetwork(&wpsNetCfg);
        // trim the " char
        memset(ssid, 0, WIFI_SSID_MAXLEN);
        len = strlen(wpsNetCfg.ssidName);
        memcpy(ssid, wpsNetCfg.ssidName + 1, len - 2);
        memset(password, 0, WIFI_PASSWORD_MAXLEN);
        len = strlen(wpsNetCfg.securitySuit.preShareKey);
        memcpy(password, wpsNetCfg.securitySuit.preShareKey + 1, len - 2);

        printf("[WIFIMGR WPS] WPS Info:\r\n");
        printf("[WIFIMGR WPS] WPS SSID     = %s\r\n", ssid);
        printf("[WIFIMGR WPS] WPS Password = %s\r\n", password);
        printf("[WIFIMGR WPS] WPS Security = %ld\r\n", wpsNetCfg.securitySuit.securityMode);
#endif
    }

    end:

    if (wps_cancel_flag)
    {
        printf("[WIFIMGR WPS]%s() L#%ld: End. wps_cancel_flag is set.\r\n", __FUNCTION__, __LINE__);
    }

    return nRet;
}

static int WiFiMgrProcessThreadFunc(void)
{
    int nRet = 0;
    int bIsAvail = 0, nWiFiConnState = 0, nWiFiConnEcode = 0;
    int nPlayState = 0;

    int nCheckCnt = WIFIMGR_CHECK_WIFI_MSEC;
    static struct timeval tv1 = {0, 0}, tv2 = {0, 0};
    static struct timeval tv3_temp = {0, 0}, tv4_temp = {0, 0};
    long temp_disconn_time = 0;
    int wifi_mode = 0, is_softap_ready = 0;

    is_first_connect = true;
    is_temp_disconnect = false;
    no_ini_file = false;
    no_ssid = false;

    usleep(20000);

    while (1)
    {
        nCheckCnt--;
        if (gwifiTerminate) {
            printf("[Wifi mgr]terminate WiFiMgrProcessThreadFunc \n");
            break;
        }

        usleep(1000);
        if (nCheckCnt == 0) {
            wifi_mode = wifiMgr_get_wifi_mode();

            if (wifi_mode == WIFIMGR_MODE_SOFTAP){
				#ifdef CFG_NET_WIFI_HOSTAPD
                // Soft AP mode
                if (!soft_ap_init_ready) {
                    is_softap_ready = iteHOSTAPDCtrlIsReady();
                    printf("[Main]%s() L#%ld: %ld, is_softap_ready=%ld\r\n", __FUNCTION__, __LINE__, is_softap_ready);
                    if (is_softap_ready) {
                        soft_ap_init_ready = true;
                        gWifi_connstate = WIFIMGR_CONNSTATE_STOP;
                        if (gWifiMgrSetting.wifiCallback)
                            gWifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_CONNECTION_FINISH);
                    }
                }
				#endif
            }
            else
            {
                // Client mode
                if (is_first_connect) {
                    // first time connect when the system start up
                    nRet = wifiMgr_connect();
                    if (nRet == WIFIMGR_ECODE_OK) {
                        gWifi_connstate = WIFIMGR_CONNSTATE_CONNECTING;
                    }
                    is_first_connect = false;

                    goto end;
                }

                if (gWifi_connstate == WIFIMGR_CONNSTATE_SETTING ||
                    gWifi_connstate == WIFIMGR_CONNSTATE_CONNECTING)
                {
                    nRet = wifiMgr_get_connect_state(&nWiFiConnState, &nWiFiConnEcode);
                    if (nWiFiConnState == WIFIMGR_CONNSTATE_STOP) {
                        gWifi_connstate = WIFIMGR_CONNSTATE_STOP;
                        // the connecting was finish
                        if (nWiFiConnEcode == WIFIMGR_ECODE_OK) {
                            nRet = wifiMgr_is_wifi_available(&bIsAvail);
                            if (!bIsAvail) {
                                // fail, restart the timer
                                gettimeofday(&tv1, NULL);
                            }
                        } else {
                            printf("[WIFIMGR]%s() L#%ld: Error! nWiFiConnEcode = 0%ld\r\n", __FUNCTION__, __LINE__, nWiFiConnEcode);

                            // connection has error
                            if (nWiFiConnEcode == WIFIMGR_ECODE_NO_INI_FILE) {
                                no_ini_file = true;
                            }
                            if (nWiFiConnEcode == WIFIMGR_ECODE_NO_SSID) {
                                no_ssid = true;
                            } else {
                                // fail, restart the timer
                                gettimeofday(&tv1, NULL);
                            }
                        }
                    }
                    goto end;
                }

                nRet = wifiMgr_is_wifi_available(&bIsAvail);
                nRet = wifiMgr_get_connect_state(&nWiFiConnState, &nWiFiConnEcode);
                if (bIsAvail)
                {

                    if (is_temp_disconnect) {
                        is_temp_disconnect = false;     // reset
                        printf("[WIFIMGR]%s() L#%ld: WiFi auto re-connected!\r\n", __FUNCTION__, __LINE__);
                        if (gWifiMgrSetting.wifiCallback)
                            gWifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_CLIENT_MODE_RECONNECTION);
                    }

                    if (!is_wifi_available) {
                        // prev is not available, curr is available
                        is_wifi_available = true;
                        no_ini_file = false;
                        no_ssid = false;
                        printf("[WIFIMGR]%s() L#%ld: WiFi auto re-connected!\r\n", __FUNCTION__, __LINE__);
                    }
                } else {
                    if (is_wifi_available){
                        if (!is_temp_disconnect  && nWiFiConnEcode == WIFIMGR_ECODE_OK)
                        {
                            // first time detect
                            is_temp_disconnect = true;
                            gettimeofday(&tv3_temp, NULL);
                            printf("[WIFIMGR]%s() L#%ld: WiFi temporary disconnected!%d %d\r\n", __FUNCTION__, __LINE__,nWiFiConnState,nWiFiConnEcode);
                            if (gWifiMgrSetting.wifiCallback)
                                gWifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_CLIENT_MODE_TEMP_DISCONNECT);
                        } else if (nWiFiConnEcode == WIFIMGR_ECODE_OK){
                            gettimeofday(&tv4_temp, NULL);
                            temp_disconn_time = itpTimevalDiff(&tv3_temp, &tv4_temp);
                            printf("[WIFIMGR]%s() L#%ld: temp disconnect time = %ld sec. %d %d\r\n", __FUNCTION__, __LINE__, temp_disconn_time / 1000 , nWiFiConnState,nWiFiConnEcode);
                            if (temp_disconn_time >= WIFIMGR_TEMPDISCONN_MSEC) {
                                printf("[WIFIMGR]%s() L#%ld: WiFi temporary disconnected over %ld sec. Services should be shut down.\r\n", __FUNCTION__, __LINE__, temp_disconn_time / 1000);
                                is_temp_disconnect = false;     // reset

                                if (gWifiMgrSetting.wifiCallback)
                                    gWifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_CLIENT_MODE_DISCONNECT_30S);

                                // prev is available, curr is not available
                                is_wifi_available = false;
                            }
                        }
                    }
                    else
                    {
                        // prev is not available, curr is not available
                        if (no_ini_file || no_ssid) {
                            // has no data to connect, skip
                            goto end;
                        }

                        nRet = wifiMgr_get_connect_state(&nWiFiConnState, &nWiFiConnEcode);
                        switch (nWiFiConnState)
                        {
                        case WIFIMGR_CONNSTATE_STOP:
                            gettimeofday(&tv2, NULL);
                            if (itpTimevalDiff(&tv1, &tv2) >= WIFIMGR_RECONNTECT_MSEC) {
                                nRet = wifiMgr_connect();
                                if (nRet == WIFIMGR_ECODE_OK) {
                                    gWifi_connstate = WIFIMGR_CONNSTATE_CONNECTING;
                                }
                            }
                            break;
                        case WIFIMGR_CONNSTATE_SETTING:
                            break;
                        case WIFIMGR_CONNSTATE_CONNECTING:
                            break;
                        }
                    }
                }
            }

    end:
            nCheckCnt = WIFIMGR_CHECK_WIFI_MSEC;
        }
    }
    return nRet;
}

static void*
WifiMgrClientModeThreadFunc(void* arg)
{
    int nRet = WIFIMGR_ECODE_OK;

    while (1)
    {
        sem_wait(&semConnectStart);

        if (gwifiTerminate) {
            printf("[Wifi mgr]terminate WifiMgrClientModeThreadFunc \n");
            break;
        }

        //ioctl(hLed1, ITP_IOCTL_FLICKER, (void*)500);
        if (need_set){
            wifi_conn_state = WIFIMGR_CONNSTATE_SETTING;
            printf("[WIFIMGR] START to Set!\r\n");
            wifi_conn_ecode = nRet = WifiMgr_Set();

            need_set = false;
            printf("[WIFIMGR] Set finish!\r\n");
        }
        usleep(1000);

        if (nRet == WIFIMGR_ECODE_OK) {
            wifi_conn_state = WIFIMGR_CONNSTATE_CONNECTING;
#ifdef CFG_AUDIOLINK_STATUS_TRANSPORT_ENABLE
            WifiMgrConnecting();
#endif
            printf("[WIFIMGR] START to Connect!\r\n");
            iteWPACtrlWpsCancel();

			/* Wait Wifi turn on at UI */
			while (!wifi_client_status){
				strcpy(gSsid    , "");
				strcpy(gPassword, "");
				strcpy(gSecumode, "");

				usleep(1000*100);
			}

            wps_cancel_flag = true;
            wifi_conn_ecode = WifiMgr_Connect();
            wps_cancel_flag = false;
            printf("[WIFIMGR] Connect finish!\r\n");

        }
        wifi_conn_state = WIFIMGR_CONNSTATE_STOP;
        usleep(1000);
    }

    return;
}

static void*
WifiMgrWPSThreadFunc(void* arg)
{
    int nRet = WIFIMGR_ECODE_OK;

    while (1)
    {
        sem_wait(&semWPSStart);
        if (gwifiTerminate) {
            printf("[Wifi mgr]terminate WifiMgrWPSThreadFunc \n");
            break;
        }

        printf("[WIFIMGR] START to Connect WPS!\r\n");
        wifi_conn_ecode = WifiMgr_WPS();
        printf("[WIFIMGR] Connect WPS finish!\r\n");


        usleep(1000);
    }

    return;
}

int
wifiMgr_is_wifi_available(int *is_available)
{
    int nRet = WIFIMGR_ECODE_OK;
    int is_connected = 0, is_avail = 0;

    if (!al_wifi_init) {
        *is_available = 0;
        return WIFIMGR_ECODE_NOT_INIT;
    }

    is_connected = ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_CONNECTED, NULL);
    is_avail = ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_AVAIL, NULL);
    *is_available = is_connected && is_avail && iteWPAConnectState();

    return nRet;
}

int
wifiMgr_get_connect_state(int *conn_state, int *e_code)
{
    int nRet = WIFIMGR_ECODE_OK;

    if (!al_wifi_init) {
        *conn_state = 0;
        *e_code = 0;
        return WIFIMGR_ECODE_NOT_INIT;
    }

    pthread_mutex_lock(&mutexALWiFi);
    *conn_state = wifi_conn_state;
    *e_code = wifi_conn_ecode;
    pthread_mutex_unlock(&mutexALWiFi);

    return nRet;
}

int wifiMgr_get_APMode_Ready()
{
    return soft_ap_init_ready;
}

static int WifiMgr_clientMode_setting(char* ssid,char* password,char* secumode)
{

    if (ssid){
        // SSID
        snprintf(gSsid, WIFI_SSID_MAXLEN, ssid);
    }
    if (password){
        // Password
        snprintf(gPassword, WIFI_PASSWORD_MAXLEN, password);
    }
    if (secumode){
        // Security mode
        snprintf(gSecumode, WIFI_SECUMODE_MAXLEN, secumode);
    }

    return 0;
}

int
wifiMgr_connect(void)
{
    int nRet = WIFIMGR_ECODE_OK;

    if (!al_wifi_init) {
        return WIFIMGR_ECODE_NOT_INIT;
    }

    pthread_mutex_lock(&mutexALWiFi);
    if (wifi_conn_state == WIFIMGR_CONNSTATE_STOP) {
        need_set = false;
        sem_post(&semConnectStart);
    }
    pthread_mutex_unlock(&mutexALWiFi);

    return nRet;
}


int
wifiMgr_set_wps_connect(void)
{
    int nRet = WIFIMGR_ECODE_OK;

    if (!al_wifi_init) {
        return WIFIMGR_ECODE_NOT_INIT;
    }

    pthread_mutex_lock(&mutexALWiFi);
    if (wifi_conn_state == WIFIMGR_CONNSTATE_STOP) {
        need_set = true;
        sem_post(&semWPSStart);
//        sem_post(&semConnectStart);
    }
    pthread_mutex_unlock(&mutexALWiFi);

    return nRet;
}


int
wifiMgr_get_wifi_mode(void)
{
    return wifi_mode;
}

int wifiMgr_get_softap_device_number(void)
{
    int stacount = 0;
    if (wifi_mode ==WIFIMGR_MODE_SOFTAP) {
        stacount = ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_STANUM, NULL);
        printf("sta number = %d\n",stacount);
    }
	return stacount;
}

// get mac addresss
int wifiMgr_get_Mac_address(char cMac[6])
{
    ITPWifiInfo wifiInfo;

    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_GET_INFO, &wifiInfo);
    cMac[0] = wifiInfo.hardwareAddress[0];
    cMac[1] = wifiInfo.hardwareAddress[1];
    cMac[2] = wifiInfo.hardwareAddress[2];
    cMac[3] = wifiInfo.hardwareAddress[3];
    cMac[4] = wifiInfo.hardwareAddress[4];
    cMac[5] = wifiInfo.hardwareAddress[5];

    return 0;
}

int wifiMgr_clientMode_disconnect()
{

    ITPEthernetSetting setting;

    int nRet = WIFIMGR_ECODE_OK;

    if (!al_wifi_init) {
        return WIFIMGR_ECODE_NOT_INIT;
    }

	printf("wifiMgr_clientMode_disconnect \n");
    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_CONNECTED, NULL)) {

        iteWPACtrlDisconnectNetwork();
        usleep(1000*100);
        // dhcp
        setting.dhcp = 0;

        // autoip
        setting.autoip = 0;

        // ipaddr
        setting.ipaddr[0] = 0;
        setting.ipaddr[1] = 0;
        setting.ipaddr[2] = 0;
        setting.ipaddr[3] = 0;

        // netmask
        setting.netmask[0] = 0;
        setting.netmask[1] = 0;
        setting.netmask[2] = 0;
        setting.netmask[3] = 0;

        // gateway
        setting.gw[0] = 0;
        setting.gw[1] = 0;
        setting.gw[2] = 0;
        setting.gw[3] = 0;

        //ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_RESET, &setting);
    }
    printf("wifiMgr_clientMode_disconnect end \n");
    wifi_conn_state = WIFIMGR_CONNSTATE_STOP;
    wifi_conn_ecode = WIFIMGR_ECODE_SET_DISCONNECT;
    usleep(1000*100);

    return WIFIMGR_ECODE_OK;
}

int wifiMgr_clientMode_connect_ap(char* ssid,char* password,char* secumode)
{
//    WifiMgr_clientMode_setting("Apple Network","12345678","6");
    int nRet = WIFIMGR_ECODE_OK;

    if (!al_wifi_init) {
        return WIFIMGR_ECODE_NOT_INIT;
    }

    WifiMgr_clientMode_setting(ssid,password,secumode);
    if (wifi_conn_state == WIFIMGR_CONNSTATE_STOP) {
        need_set = false;
        sem_post(&semConnectStart);
    }

    return WIFIMGR_ECODE_OK;
}

static void Swap(int x, int y)
{
  //    int temp = array[x];
    //array[x] = array[y];
    //array[y] = temp;
    WIFI_MGR_SCANAP_LIST temp;
// printf("%d %d 0x%x 0x%x \n",gWifiMgrApList[x].rfQualityRSSI,gWifiMgrApList[y].rfQualityRSSI, &gWifiMgrApList[x],&gWifiMgrApList[y]);

    memcpy(&temp,&gWifiMgrApList[x],sizeof(WIFI_MGR_SCANAP_LIST));
    memcpy(&gWifiMgrApList[x],&gWifiMgrApList[y],sizeof(WIFI_MGR_SCANAP_LIST));
    memcpy(&gWifiMgrApList[y],&temp,sizeof(WIFI_MGR_SCANAP_LIST));
}


static void InsertSortWifi(int size)
{
    int i,j;
    for(i = 0; i < size; i++){
        for(j = i; j > 0; j--){
            if(gWifiMgrApList[j].rfQualityRSSI > gWifiMgrApList[j - 1].rfQualityRSSI){
                Swap(j, j-1);
            }
        }
    }
}

static int
scanWifiAp(struct net_device_info *apInfo)
{
    int nRet = 0;
    int nWifiState = 0;
    int i = 0;

    memset(apInfo, 0, sizeof(struct net_device_info));

    printf("[Wifi mgr]%s() Start to SCAN AP ==========================\r\n", __FUNCTION__);
    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_SCAN, NULL);
    while (1)
    {
        nWifiState = ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_STATE, NULL);
        //printf("[Presentation]%s() nWifiState=0x%X\r\n", __FUNCTION__, nWifiState);
        if ((nWifiState & WLAN_SITE_MONITOR) == 0)
        {
            // scan finish
            printf("[Wifi mgr]%s() Scan AP Finish!\r\n", __FUNCTION__);
            break;
        }
        usleep(100 * 1000);
    }

    read(ITP_DEVICE_WIFI, apInfo, (size_t)NULL);
    printf("[Wifi mgr]%s() ScanApInfo.apCnt = %ld\r\n", __FUNCTION__, apInfo->apCnt);

	#if 0
    for (i = 0; i < apInfo->apCnt; i++)
    {
        printf("[Main] ssid = %32s, securityOn = %ld, securityMode = %ld, avgQuant = %d, avgRSSI = %d , <%02x:%02x:%02x:%02x:%02x:%02x>\r\n", apInfo->apList[i].ssidName, apInfo->apList[i].securityOn, apInfo->apList[i].securityMode,apInfo->apList[i].rfQualityQuant, apInfo->apList[i].rfQualityRSSI,
        apInfo->apList[i].apMacAddr[0], apInfo->apList[i].apMacAddr[1], apInfo->apList[i].apMacAddr[2], apInfo->apList[i].apMacAddr[3], apInfo->apList[i].apMacAddr[4], apInfo->apList[i].apMacAddr[5]);
    }
	#endif

    for (i = 0; i < apInfo->apCnt; i++)
    {
        gWifiMgrApList[i].channelId = apInfo->apList[i].channelId;
        gWifiMgrApList[i].operationMode = apInfo->apList[i].operationMode ;
        gWifiMgrApList[i].rfQualityQuant = apInfo->apList[i].rfQualityQuant;
        gWifiMgrApList[i].rfQualityRSSI = apInfo->apList[i].rfQualityRSSI;
        gWifiMgrApList[i].securityMode = apInfo->apList[i].securityMode;
        memcpy(gWifiMgrApList[i].apMacAddr,apInfo->apList[i].apMacAddr,8);
        memcpy(gWifiMgrApList[i].ssidName,apInfo->apList[i].ssidName,32);
    }

    InsertSortWifi(apInfo->apCnt);

	#if 0
    for (i = 0; i < apInfo->apCnt; i++)
    {
        printf("[Wifi mgr] ssid = %32s, securityOn = %ld, securityMode = %ld, avgQuant = %d, avgRSSI = %d , <%02x:%02x:%02x:%02x:%02x:%02x>\r\n", gWifiMgrApList[i].ssidName, gWifiMgrApList[i].securityOn, gWifiMgrApList[i].securityMode,gWifiMgrApList[i].rfQualityQuant, gWifiMgrApList[i].rfQualityRSSI,
        gWifiMgrApList[i].apMacAddr[0], gWifiMgrApList[i].apMacAddr[1], gWifiMgrApList[i].apMacAddr[2], gWifiMgrApList[i].apMacAddr[3], gWifiMgrApList[i].apMacAddr[4], gWifiMgrApList[i].apMacAddr[5]);
    }
	#endif

    printf("[Wifi mgr]%s() End to SCAN AP ============================\r\n", __FUNCTION__);
    return apInfo->apCnt;
}

//
int wifiMgr_get_scan_ap_info(WIFI_MGR_SCANAP_LIST* pList)
{
    int nApCount;

    pthread_mutex_lock(&mutexMode);

	if (wifi_client_status == WIFI_IS_TURNED_OPEN){
            nApCount = scanWifiAp(&gScanApInfo);
	}

	if (wifi_client_status == WIFI_IS_TURNED_OPEN){
            memcpy(pList,gWifiMgrApList,sizeof(WIFI_MGR_SCANAP_LIST)*64);
	}else if (wifi_client_status == WIFI_IS_TURNED_CLOSE){
            //memcpy(pList,NULL,sizeof(WIFI_MGR_SCANAP_LIST)*64);
	}


    pthread_mutex_unlock(&mutexMode);

    return nApCount;

}


int
wifiMgr_init(WIFIMGR_MODE_E init_mode, int mp_mode,WIFI_MGR_SETTING wifiSetting)
{
    int nRet = WIFIMGR_ECODE_OK;
    pthread_t task, task1,task2;
    pthread_attr_t attr, attr1,attr2;

    gwifiTerminate = 0;

    wifi_conn_state = WIFIMGR_CONNSTATE_STOP;
    wifi_conn_ecode = WIFIMGR_ECODE_SET_DISCONNECT;
    need_set = false;
    mpMode = mp_mode;

//    gWifiMgrSetting.ssid = gSsid;
//    gWifiMgrSetting.password = gPassword;
//    gWifiMgrSetting.secumode = gSecumode;

    gWifiMgrSetting.wifiCallback = wifiSetting.wifiCallback;

    if (wifi_mode ==WIFIMGR_MODE_CLIENT){
        WifiMgr_clientMode_setting(wifiSetting.ssid,wifiSetting.password,wifiSetting.secumode);

    }

    // default select dhcp
    gWifiMgrSetting.setting.dhcp = 1;
    if (wifiSetting.setting.ipaddr[0]>0){
        memcpy(&gWifiMgrSetting.setting,&wifiSetting.setting,sizeof(ITPEthernetSetting));
    }

    /*hLed0 = open(":led:0", O_RDONLY);
    if (!hLed0)
    {
        // has no led!
        printf("[WIFIMGR]%s() L#%ld: Error! hLed0 init fail!\r\n", __FUNCTION__, __LINE__);
        nRet = WIFIMGR_ECODE_NO_LED;
        goto end;
    }

    ioctl(hLed0, ITP_IOCTL_OFF, NULL);

    hLed1 = open(":led:1", O_RDONLY);
    if (!hLed1)
    {
        // has no led!
        printf("[WIFIMGR]%s() L#%ld: Error! hLed1 init fail!\r\n", __FUNCTION__, __LINE__);
        nRet = WIFIMGR_ECODE_NO_LED;
        goto end;
    }
    ioctl(hLed1, ITP_IOCTL_OFF, NULL);*/

    // Check if the wifi dongle is exist
    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_DEVICE_READY, NULL) == 0) {
        // has no wifi device!
        printf("[WIFIMGR]%s() L#%ld: Error! Has no WiFi device!!\r\n", __FUNCTION__, __LINE__);
        nRet = WIFIMGR_ECODE_NO_WIFI_DONGLE;
        goto end;
    }

    // init semaphore
    nRet = sem_init(&semConnectStart, 0, 0);
    if (nRet == -1) {
        printf("[WIFIMGR] ERROR, semConnectStart sem_init() fail!\r\n");
        nRet = WIFIMGR_ECODE_SEM_INIT;
        goto err_end;
    }

    nRet = sem_init(&semConnectStop, 0, 0);
    if (nRet == -1) {
        printf("[WIFIMGR] ERROR, semConnectStop sem_init() fail!\r\n");
        nRet = WIFIMGR_ECODE_SEM_INIT;
        goto err_end;
    }

    nRet = sem_init(&semWPSStart, 0, 0);
    if (nRet == -1) {
        printf("[WIFIMGR] ERROR, semWPSStart sem_init() fail!\r\n");
        nRet = WIFIMGR_ECODE_SEM_INIT;
        goto err_end;
    }

    nRet = sem_init(&semWPSStop, 0, 0);
    if (nRet == -1) {
        printf("[WIFIMGR] ERROR, semWPSStop sem_init() fail!\r\n");
        nRet = WIFIMGR_ECODE_SEM_INIT;
        goto err_end;
    }

    // init mutex
    nRet = pthread_mutex_init(&mutexALWiFi, NULL);
    if (nRet != 0) {
        printf("[WIFIMGR] ERROR, mutexALWiFi pthread_mutex_init() fail! nRet = %ld\r\n", nRet);
        nRet = WIFIMGR_ECODE_MUTEX_INIT;
        goto err_end;
    }

    nRet = pthread_mutex_init(&mutexALWiFiWPS, NULL);
    if (nRet != 0) {
        printf("[WIFIMGR] ERROR, mutexALWiFiWPS pthread_mutex_init() fail! nRet = %ld\r\n", nRet);
        nRet = WIFIMGR_ECODE_MUTEX_INIT;
        goto err_end;
    }

    nRet = pthread_mutex_init(&mutexIni, NULL);
    if (nRet != 0) {
        printf("[WIFIMGR] ERROR, mutexIni pthread_mutex_init() fail! nRet = %ld\r\n", nRet);
        nRet = WIFIMGR_ECODE_MUTEX_INIT;
        goto err_end;
    }

    nRet = pthread_mutex_init(&mutexMode, NULL);
    if (nRet != 0) {
        printf("[WIFIMGR] ERROR, mutexMode pthread_mutex_init() fail! nRet = %ld\r\n", nRet);
        nRet = WIFIMGR_ECODE_MUTEX_INIT;
        goto err_end;
    }

    // create thread
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr, WIFI_STACK_SIZE);
    pthread_create(&task, &attr, WifiMgrClientModeThreadFunc, NULL);

    pthread_attr_init(&attr1);
    pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr1, WIFI_STACK_SIZE);
    pthread_create(&task1, &attr1, WifiMgrWPSThreadFunc, NULL);

    pthread_attr_init(&attr2);
    pthread_attr_setdetachstate(&attr2, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr2, WIFI_STACK_SIZE);
    pthread_create(&task2, &attr2, WiFiMgrProcessThreadFunc, NULL);

    wifi_mode = init_mode;
    printf("[WIFIMGR]%s() L#%ld: init_mode = %ld, wifi_mode = %ld\r\n", __FUNCTION__, __LINE__, init_mode, wifi_mode);

    if (wifi_mode == WIFIMGR_MODE_SOFTAP) {
#ifdef CFG_NET_WIFI_HOSTAPD
        WifiMgr_HostAPInit();
#endif
    } else {
        WifiMgr_WpaInit();
    }

    connect_cancel_flag = false;
    wps_cancel_flag = false;

    al_wifi_init = 1;
    gwifiTerminate = 0;
end:
    return nRet;

err_end:
    pthread_mutex_destroy(&mutexMode);
    pthread_mutex_destroy(&mutexIni);
    pthread_mutex_destroy(&mutexALWiFiWPS);
    sem_destroy(&semWPSStop);
    sem_destroy(&semWPSStart);
    pthread_mutex_destroy(&mutexALWiFi);
    sem_destroy(&semConnectStop);
    sem_destroy(&semConnectStart);

    return nRet;
}



int
wifiMgr_terminate(void)
{
    int nRet = WIFIMGR_ECODE_OK;

    if (!al_wifi_init) {
        return WIFIMGR_ECODE_NOT_INIT;
    }

    gwifiTerminate = 1;

    sem_post(&semWPSStart);
    sem_post(&semConnectStart);

    if (wifi_mode == WIFIMGR_MODE_SOFTAP) {
#ifdef CFG_NET_WIFI_HOSTAPD
        WifiMgr_HostAPTerminate();
#endif
    } else {
        wps_cancel_flag = true;
        WifiMgr_WpaTerminate();
    }

    pthread_mutex_destroy(&mutexMode);
    pthread_mutex_destroy(&mutexIni);
    pthread_mutex_destroy(&mutexALWiFiWPS);
    sem_destroy(&semWPSStop);
    sem_destroy(&semWPSStart);
    pthread_mutex_destroy(&mutexALWiFi);
    sem_destroy(&semConnectStop);
    sem_destroy(&semConnectStart);
/*
    if (hLed1)
    {
        close(hLed1);
        hLed1 = 0;
    }

    if (hLed0)
    {
        close(hLed0);
        hLed0 = 0;
    }
*/
    al_wifi_init = 0;

    return nRet;
}

void
WifiMgr_clientMode_switch(int status)
{
	wifi_client_status = status;
}

static int gCountT = 0;
static WIFI_MGR_SETTING gWifiSetting;
static  struct timeval gSwitchStart, gSwitchEnd;

static  struct timeval gSwitchCount;
int calculateTime()
{
    if (itpTimevalDiff(&gSwitchCount,&gSwitchEnd)> 1000){
        printf(" %d , %d  \n",itpTimevalDiff(&gSwitchCount,&gSwitchEnd)+(gCountT*1000),itpTimevalDiff(&gSwitchStart,&gSwitchEnd));
        gCountT++;
        gettimeofday(&gSwitchCount, NULL);
    }
}

static int switchThread()
{
    int nTemp;

    printf("WifiMgr_Switch_ClientSoftAP_Mode \n");

    if (!al_wifi_init) {
        return WIFIMGR_ECODE_NOT_INIT;
    }

    wifi_mode = wifiMgr_get_wifi_mode();

    wifiMgr_terminate();

    gCountT = 0;
   gettimeofday(&gSwitchCount, NULL);

    if (wifi_mode == WIFIMGR_MODE_SOFTAP){
        // init client mode
        printf("WifiMgr_Switch_ClientSoftAP_Mode init client  \n");
        nTemp = wifiMgr_init(WIFIMGR_MODE_CLIENT, 0, gWifiSetting);


    } else {
        // init softap mode
        printf("WifiMgr_Switch_ClientSoftAP_Mode init softap  \n");
        do
        {
            usleep(1000*200);
            gettimeofday(&gSwitchEnd, NULL);

            calculateTime();
        }while(!iteWPADeinitDone() && (itpTimevalDiff(&gSwitchStart,&gSwitchEnd)<15000));
        printf("WifiMgr_Switch_ClientSoftAP_Mode deinit done  \n");

        nTemp = wifiMgr_init(WIFIMGR_MODE_SOFTAP, 0, gWifiSetting);
    }

    if (gWifiMgrSetting.wifiCallback)
        gWifiMgrSetting.wifiCallback(WIFIMGR_STATE_CALLBACK_SWITCH_CLIENT_SOFTAP_FINISH);

    return 0;

}


int WifiMgr_Switch_ClientSoftAP_Mode(WIFI_MGR_SETTING wifiSetting)
{

    pthread_t task;
    pthread_attr_t attr;
#ifdef CFG_NET_WIFI_HOSTAPD

#else

    return 0;
#endif
    gettimeofday(&gSwitchStart, NULL);

    memcpy(&gWifiSetting,&wifiSetting,sizeof(WIFI_MGR_SETTING));

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr, WIFI_STACK_SIZE);
    pthread_create(&task, &attr, switchThread, NULL);

}

#elif defined(_WIN32)

int wifiMgr_get_scan_ap_info(WIFI_MGR_SCANAP_LIST* pList)
{
    int nApCount = 0;

    return nApCount;
}

int
wifiMgr_get_connect_state(int *conn_state, int *e_code)
{
    int nRet = WIFIMGR_ECODE_OK;

    *conn_state = 0;
    *e_code = 0;


    return nRet;
}

int wifiMgr_clientMode_connect_ap(char* ssid,char* password,char* secumode)
{
    int nRet = WIFIMGR_ECODE_OK;

    return WIFIMGR_ECODE_OK;
}

int wifiMgr_clientMode_disconnect()
{
    return WIFIMGR_ECODE_OK;
}

#endif


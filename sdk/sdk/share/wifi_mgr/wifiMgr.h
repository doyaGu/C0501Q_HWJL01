

#if 1//def CFG_NET_WIFI

#include "ite/itp.h"

#define WIFI_SSID_MAXLEN                 64
#define WIFI_PASSWORD_MAXLEN             256
#define WIFI_SECUMODE_MAXLEN             3

typedef enum tagWIFIMGR_ECODE_E
{
    WIFIMGR_ECODE_OK = 0,
    WIFIMGR_ECODE_FAIL,
    WIFIMGR_ECODE_NOT_INIT,
    WIFIMGR_ECODE_ALLOC_MEM,
    WIFIMGR_ECODE_SEM_INIT,
    WIFIMGR_ECODE_MUTEX_INIT,
    WIFIMGR_ECODE_NO_LED,
    WIFIMGR_ECODE_NO_WIFI_DONGLE,
    WIFIMGR_ECODE_NO_INI_FILE,
    WIFIMGR_ECODE_NO_SSID,
    WIFIMGR_ECODE_CONNECT_ERROR,
    WIFIMGR_ECODE_DHCP_ERROR,
    WIFIMGR_ECODE_OPEN_FILE,
    WIFIMGR_ECODE_DTMF_DEC_TIMEOUT,
    WIFIMGR_ECODE_UNKNOWN_SECURITY,
    WIFIMGR_ECODE_SET_DISCONNECT,
    WIFIMGR_ECODE_MAX,
} WIFIMGR_ECODE_E;


typedef enum tagWIFIMGR_CONNSTATE_E
{
    WIFIMGR_CONNSTATE_STOP = 0,
    WIFIMGR_CONNSTATE_SETTING,
    WIFIMGR_CONNSTATE_SCANNING,
    WIFIMGR_CONNSTATE_CONNECTING,
    WIFIMGR_CONNSTATE_MAX,
} WIFIMGR_CONNSTATE_E;

typedef enum tagWIFIMGR_STATE_CALLBACK_E
{
    WIFIMGR_STATE_CALLBACK_CONNECTION_FINISH = 0,
    WIFIMGR_STATE_CALLBACK_CLIENT_MODE_DISCONNECT_30S,
    WIFIMGR_STATE_CALLBACK_CLIENT_MODE_RECONNECTION,
    WIFIMGR_STATE_CALLBACK_CLIENT_MODE_TEMP_DISCONNECT,
    WIFIMGR_STATE_CALLBACK_CLIENT_MODE_CONNECTING_FAIL,
    WIFIMGR_STATE_CALLBACK_SWITCH_CLIENT_SOFTAP_FINISH,
    WIFIMGR_STATE_CALLBACK_MAX,
} WIFIMGR_STATE_CALLBACK_E;


typedef enum tagWIFIMGR_MODE_E
{
    WIFIMGR_MODE_CLIENT = 0,
    WIFIMGR_MODE_SOFTAP,
    WIFIMGR_MODE_MAX,
} WIFIMGR_MODE_E;

typedef enum tagWIFIMGR_SWTICH_STATUS_E
{
    WIFIMGR_SWITCH_OFF = 0,
    WIFIMGR_SWITCH_ON,
} WIFIMGR_SWTICH_STATUS_E;


typedef struct WIFI_MGR_SETTING_TAG
{
    int  (*wifiCallback)(int nCondition);
    char ssid[WIFI_SSID_MAXLEN];
    char password[WIFI_PASSWORD_MAXLEN];
    char secumode[WIFI_SECUMODE_MAXLEN];
    ITPEthernetSetting setting;    
}WIFI_MGR_SETTING;

typedef struct WIFI_MGR_SCANAP_LIST_TAG
{
    unsigned char name[16];
    unsigned char apMacAddr[6+2];
    int channelId;
    unsigned char ssidName[32];
    int operationMode;
    int securityOn;
    unsigned char rfQualityQuant; //Percent : 0~100
    signed char  rfQualityRSSI; //RSSI
    unsigned char reserved[2];
    int   bitrate;
    int   securityMode;  /*Sec. Mode*/
} WIFI_MGR_SCANAP_LIST; 



int
wifiMgr_is_wifi_available(int *is_available);

int
wifiMgr_get_connect_state(int *conn_state, int *e_code);


// 0 : not ready , 1:ready
int wifiMgr_get_APMode_Ready();

int
wifiMgr_connect(void);

int
wifiMgr_set_wps_connect(void);

// 0: client mode, 1:softAP mode 
int wifiMgr_get_wifi_mode(void);

// int wifi mode
int wifiMgr_init(WIFIMGR_MODE_E init_mode, int mp_mode,WIFI_MGR_SETTING wifiSetting);

// cMac : mac address, 6 bytes
int wifiMgr_get_Mac_address(char cMac[6]);

// get all of  WIFI_MGR_SCANAP_LIST, return ap list's conut
int wifiMgr_get_scan_ap_info(WIFI_MGR_SCANAP_LIST* pList);

// terminate wifi mode
int wifiMgr_terminate(void);

// get number of connecting device to ap
int wifiMgr_get_softap_device_number(void);

int wifiMgr_clientMode_disconnect();

int wifiMgr_clientMode_connect_ap(char* ssid,char* password,char* secumode);

// check wifi status; 0: WIFI is turned close, 1: WIFI is turned open
void WifiMgr_clientMode_switch(int status);

// switch Client to soft ap  or  soft ap to Client 
int WifiMgr_Switch_ClientSoftAP_Mode(WIFI_MGR_SETTING wifiSetting);


#endif


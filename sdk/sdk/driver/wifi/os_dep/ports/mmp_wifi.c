#include "rt_config.h"

//Debug Code....
struct net_device *smNetDevice;

int mmpWifiTest(MMP_INT sec_mode)
{
    struct net_device *netdev;
    struct net_device_config testConfig = {0};
    struct net_device_info testInfo = {0};  

    /*Init. WiFi*/
    netdev = smNetDevice;

    if(sec_mode == -1)
        goto close;

    /*Open WiFi*/
    smNetOpen(netdev, NULL, NULL);
        
    /*Config WiFi*/
    switch(sec_mode)
    {
    case WLAN_SEC_NOSEC:
        {
            testConfig.operationMode = WLAN_MODE_STA;
            sprintf(testConfig.ssidName, "ireneNoSec");
            testConfig.securitySuit.securityMode = WLAN_SEC_NOSEC;
        }
        break;
    case WLAN_SEC_WEP:
        #if 1
        {
            testConfig.operationMode = WLAN_MODE_STA;
            sprintf(testConfig.ssidName, "ireneWep64Auto");
            testConfig.securitySuit.securityMode = WLAN_SEC_WEP;
            testConfig.securitySuit.authMode = WLAN_AUTH_OPENSYSTEM;
            sprintf(testConfig.securitySuit.wepKeys[0], "11111");
            sprintf(testConfig.securitySuit.wepKeys[1], "22222");
            sprintf(testConfig.securitySuit.wepKeys[2], "33333");
            sprintf(testConfig.securitySuit.wepKeys[3], "44444");    
            testConfig.securitySuit.defaultKeyId = 0; /*From 0 to 3*/
        }
        #else
        {/*WEP128-SHARED*/
            testConfig.operationMode = WLAN_MODE_STA;
            sprintf(testConfig.ssidName, "ireneWep128Shared");
            testConfig.securitySuit.securityMode = WLAN_SEC_WEP128;
            testConfig.securitySuit.authMode = WLAN_AUTH_SHAREDKEY;
            sprintf(&testConfig.securitySuit.wepKeys[0][0], "4444444444444");
            sprintf(&testConfig.securitySuit.wepKeys[1][0], "3333333333333");
            sprintf(&testConfig.securitySuit.wepKeys[2][0], "2222222222222");
            sprintf(&testConfig.securitySuit.wepKeys[3][0], "1111111111111");    
            testConfig.securitySuit.defaultKeyId = 0; /*From 0 to 3*/
        }
        #endif
        break;
    case WLAN_SEC_WPAPSK:
        {/*WPAPSK*/
            testConfig.operationMode = WLAN_MODE_STA;
            sprintf(testConfig.ssidName, "ireneWpaPsk");
            testConfig.securitySuit.securityMode = WLAN_SEC_WPAPSK;
            sprintf(testConfig.securitySuit.preShareKey, "12345678");
        }
        break;
    case WLAN_SEC_WPA2PSK:
        {/*WPA2PSK*/
            testConfig.operationMode = WLAN_MODE_STA;
            sprintf(testConfig.ssidName, "ireneWpa2Psk");
            testConfig.securitySuit.securityMode = WLAN_SEC_WPA2PSK;
            sprintf(testConfig.securitySuit.preShareKey, "87654321");
        }
        break;
    }

    if (smNetCtrl(netdev, &testConfig) != 0)
        printk("@@@@OMG CTRL\n\n");

    return 0;

close:
    printf(" start close ..... \n");
    smNetClose(netdev);

    return 0;
}


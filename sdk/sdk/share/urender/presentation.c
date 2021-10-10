/*
 * presentation.c : GeeXboX uShare UPnP Presentation Page.
 * Originally developped for the GeeXboX project.
 * Copyright (C) 2005-2007 Benjamin Zores <ben@geexbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <sys/ioctl.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>

#if HAVE_LANGINFO_CODESET
# include <langinfo.h>
#endif

#include "urender.h"
#include "config.h"
//#include "metadata.h"
//#include "content.h"
#include "buffer.h"
#include "presentation.h"
#include "gettext.h"
#include "util_iconv.h"
#include "string.h"
#include "libxml/xpath.h"
#include "iniparser/iniparser.h"
#include "ite/itp.h"
#include "ite/ite_wifi.h"
#include "ite/ug.h"



#define CGI_ACTION            "action="
#define CGI_ACTION_NETWORK    "network"
#define CGI_ACTION_SOFTAP     "softap"
#define CGI_ACTION_WIFI       "wifi"
#define CGI_ACTION_REBOOT     "reboot"
#define CGI_ACTION_RESET      "Reset"
#define CGI_ACTION_UPGRADE    "upgrade"
#define CGI_ACTION_TRANSPORTER "transporter"
#define CGI_ACTION_MACADDR    "macaddr"
#define CGI_ACTION_AUTOSHUTDOWN       "autoshutdown"
#define CGI_ACTION_SHAIRPORT  "shairport"

#ifdef CFG_AUDIOLINK_NETWORK_SWITCH_ENABLE
// Client/AP mode swtich
#define CGI_ACTION_NETWORK_SWITCH	"switchNetwork"
#endif

#define PRESENTATION_INIFILENAME_MAXLEN             32
#define PRESENTATION_HTMLFILENAME_MAXLEN            32

#define PRESENTATION_SSID_MAXLEN                    64
#define PRESENTATION_PASSWORD_MAXLEN                256

#define PRESENTATION_INIKEY_SOFTAP                  "softap:softap"
#define PRESENTATION_INIKEY_SSID                    "softap:ssid"
#define PRESENTATION_INIKEY_PASSWORD                "softap:password"
#define PRESENTATION_INIKEY_SECUMODE                "softap:secumode"
#define PRESENTATION_INIKEY_WIFI_SSID               "wifi:ssid"
#define PRESENTATION_INIKEY_WIFI_PASSWORD           "wifi:password"
#define PRESENTATION_INIKEY_WIFI_SECUMODE           "wifi:secumode"
#define PRESENTATION_INIKEY_URENDER_NAME            "urender:friendlyName"
#define PRESENTATION_INIKEY_SHAIRPORT_NAME          "shairport:friendlyName"
#define PRESENTATION_INIKEY_TRANSPORTER_UART_BAUDRATE "transporter:uartBaudrate"
#define PRESENTATION_INIKEY_TRANSPORTER_UDP_PORT    "transporter:udpPort"

#define PRESENTATION_INIKEY_AUTOSHUTDOWN  "autoshutdown:auto_shutdown"
#define PRESENTATION_INIKEY_AUTOSHUTDOWN_TIME    "autoshutdown:auto_shutdown_time"


#ifdef CFG_AUDIOLINK_NETWORK_SWITCH_ENABLE
// Client/AP mode swtich
#define PRESENTATION_INIKEY_NETWORK_SWITCH_NETWORK_MODE    "switchNetwork:networkMode"
#endif

extern size_t upnp_content_length;
extern char* urender_upgrade_content;
char giniName[PRESENTATION_INIFILENAME_MAXLEN];
static char gwebName[PRESENTATION_HTMLFILENAME_MAXLEN];

int gPresentionState =0;

//extern void audiolink_set_airplay_password_ini(char* pPassword,int nLength);

// Look up str2 on str1, if found, replace str2 with str3
static int
replace_str(char *str1, const char *str2, const char *str3)
{
    int ret = 0;
    char *ptr = NULL, *ptr_remain = NULL;

    if (!str1 || !str2 || !str3 || (strlen(str3) > strlen(str2)))
    {
        return -1;
    }

    ptr = str1;
    while (ptr)
    {
        ptr = strstr(ptr, str2);
        if (ptr)
        {
            ptr_remain = ptr + strlen(str2);
            memcpy(ptr, str3, strlen(str3));
            ptr += strlen(str3);
            memmove(ptr, ptr_remain, strlen(ptr_remain));
            ptr[strlen(ptr_remain)] = 0;
        }
    }

    return ret;
}



static int
presentation_scanap(struct net_device_info *apInfo)
{
    int nRet = 0;
    int nWifiState = 0;
    int i = 0;

    memset(apInfo, 0, sizeof(struct net_device_info));

    printf("[Presentation]%s() Start to SCAN AP ==========================\r\n", __FUNCTION__);
    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_SCAN, NULL);
    while (1)
    {
        nWifiState = ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_STATE, NULL);
        //printf("[Presentation]%s() nWifiState=0x%X\r\n", __FUNCTION__, nWifiState);
        if ((nWifiState & WLAN_SITE_MONITOR) == 0)
        {
            // scan finish
            printf("[Presentation]%s() Scan AP Finish!\r\n", __FUNCTION__);
            break;
        }
        usleep(100 * 1000);
    }

    read(ITP_DEVICE_WIFI, apInfo, (size_t)NULL);
    printf("[Presentation]%s() ScanApInfo.apCnt = %ld\r\n", __FUNCTION__, apInfo->apCnt);
    for (i = 0; i < apInfo->apCnt; i++)
    {
        printf("[Presentation] ssid = %32s, securityOn = %ld, securityMode = %ld, <%02x:%02x:%02x:%02x:%02x:%02x>\r\n", apInfo->apList[i].ssidName, apInfo->apList[i].securityOn, apInfo->apList[i].securityMode,
            apInfo->apList[i].apMacAddr[0], apInfo->apList[i].apMacAddr[1], apInfo->apList[i].apMacAddr[2], apInfo->apList[i].apMacAddr[3], apInfo->apList[i].apMacAddr[4], apInfo->apList[i].apMacAddr[5]);
    }

    return nRet;
}


static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int setUrenderIni(char* ptIni,char* ptWeb)
{
    strncpy(giniName,ptIni,PRESENTATION_INIFILENAME_MAXLEN);
    strncpy(gwebName,ptWeb,PRESENTATION_HTMLFILENAME_MAXLEN);

    return 0;
}



int
process_cgi(struct urender_t *ut, char *cgiargs)
{
    char *action = NULL;
    int  refresh = 0;

    printf("[Presentation] process_cgi() cgiargs=%s\r\n", cgiargs);

    if (!ut || !cgiargs)
        return -1;

    if (strncmp(cgiargs, CGI_ACTION, strlen(CGI_ACTION)))
        return -1;

    action = cgiargs + strlen(CGI_ACTION);

    if (!strncmp(action, CGI_ACTION_NETWORK, strlen(CGI_ACTION_NETWORK)))
    {
        char      buf[256];
        char      * ptr;
        char      * dhcp     = NULL;
        char      * ipaddr0  = NULL;
        char      * ipaddr1  = NULL;
        char      * ipaddr2  = NULL;
        char      * ipaddr3  = NULL;
        char      * netmask0 = NULL;
        char      * netmask1 = NULL;
        char      * netmask2 = NULL;
        char      * netmask3 = NULL;
        char      * gw0      = NULL;
        char      * gw1      = NULL;
        char      * gw2      = NULL;
        char      * gw3      = NULL;
        FILE      * f;
        dictionary* ini = NULL;
        char ini_filename[PRESENTATION_INIFILENAME_MAXLEN];

#ifdef CFG_USE_SD_INI
        snprintf(ini_filename, PRESENTATION_INIFILENAME_MAXLEN, "%s:/%s", "C", giniName);
#else
        snprintf(ini_filename, PRESENTATION_INIFILENAME_MAXLEN, "%s:/%s", CFG_PUBLIC_DRIVE, giniName);
#endif
        ini = iniparser_load(ini_filename);
        if (!ini)
        {
            printf("[Presentation] Cannot load ini file: %s\n", ini_filename);
            printf("[Presentation]%s() L#%ld: Do ugResetFactory() and reboot!\r\n", __FUNCTION__, __LINE__);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*)ITP_DISK_NOR);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORCE_MOUNT, (void*)ITP_DISK_NOR);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORMAT, (void*)1);
            ugResetFactory();
            i2s_CODEC_standby();
            exit(0);
            while (1);
        }

        strcpy(buf, action + strlen(CGI_ACTION_NETWORK) + 1);

        for (ptr = strtok(buf, "&"); ptr; ptr = strtok(NULL, "&"))
        {
            if (strncmp(ptr, "dhcp=", 5) == 0)
            {
                dhcp = ptr + 5;
            }
            else if (strncmp(ptr, "ipaddr0=", 8) == 0)
            {
                ipaddr0 = ptr + 8;
            }
            else if (strncmp(ptr, "ipaddr1=", 8) == 0)
            {
                ipaddr1 = ptr + 8;
            }
            else if (strncmp(ptr, "ipaddr2=", 8) == 0)
            {
                ipaddr2 = ptr + 8;
            }
            else if (strncmp(ptr, "ipaddr3=", 8) == 0)
            {
                ipaddr3 = ptr + 8;
            }
            else if (strncmp(ptr, "netmask0=", 9) == 0)
            {
                netmask0 = ptr + 9;
            }
            else if (strncmp(ptr, "netmask1=", 9) == 0)
            {
                netmask1 = ptr + 9;
            }
            else if (strncmp(ptr, "netmask2=", 9) == 0)
            {
                netmask2 = ptr + 9;
            }
            else if (strncmp(ptr, "netmask3=", 9) == 0)
            {
                netmask3 = ptr + 9;
            }
            else if (strncmp(ptr, "gw0=", 4) == 0)
            {
                gw0 = ptr + 4;
            }
            else if (strncmp(ptr, "gw1=", 4) == 0)
            {
                gw1 = ptr + 4;
            }
            else if (strncmp(ptr, "gw2=", 4) == 0)
            {
                gw2 = ptr + 4;
            }
            else if (strncmp(ptr, "gw3=", 4) == 0)
            {
                gw3 = ptr + 4;
            }
        }
        iniparser_set(ini, "tcpip:dhcp", dhcp ? "y" : "n");

        if (ipaddr0 && ipaddr1 && ipaddr2 && ipaddr3)
        {
            sprintf(buf, "%s.%s.%s.%s", ipaddr0, ipaddr1, ipaddr2, ipaddr3);
            iniparser_set(ini, "tcpip:ipaddr", buf);
        }
        if (netmask0 && netmask1 && netmask2 && netmask3)
        {
            sprintf(buf, "%s.%s.%s.%s", netmask0, netmask1, netmask2, netmask3);
            iniparser_set(ini, "tcpip:netmask", buf);
        }
        if (gw0 && gw1 && gw2 && gw3)
        {
            sprintf(buf, "%s.%s.%s.%s", gw0, gw1, gw2, gw3);
            iniparser_set(ini, "tcpip:gw", buf);
        }
        f = fopen(ini_filename, "w");
        if (f)
        {
            printf("[Presentation]%s() L#%ld: Write data to ini %s\r\n", __FUNCTION__, __LINE__, ini_filename);
            iniparser_dump_ini(ini, f);
            fclose(f);
    #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
            ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_DISABLE, NULL);
            ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
            ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_ENABLE, NULL);
    #endif
        }
        else
        {
            printf("[Presentation] Cannot open ini file %s to write: %ld\n", ini_filename, errno);
        }
        iniparser_freedict(ini);
    }
    else if (!strncmp(action, CGI_ACTION_SOFTAP, strlen(CGI_ACTION_SOFTAP)))
    {
        char      buf[256];
        char      * ptr;
        char      * softap   = NULL;
        char      * ssid     = NULL;
        char      * password = NULL;
        char      * hidden     = NULL;        
        FILE      * f;
        dictionary* ini = NULL;
        char ini_filename[PRESENTATION_INIFILENAME_MAXLEN];
        size_t    dummy;

        pthread_mutex_lock(&mutex);

        if (ut->upgrading)
        {
            pthread_mutex_unlock(&mutex);
            return -1;
        }

        strcpy(buf, action + strlen(CGI_ACTION_SOFTAP) + 1);

        for (ptr = strtok(buf, "&"); ptr; ptr = strtok(NULL, "&"))
        {
            if (strncmp(ptr, "softap=", 7) == 0)
            {
                softap = ptr + 7;
            }
            else if (strncmp(ptr, "ssid=", 5) == 0)
            {
                ssid = ptr + 5;
                dummy = strlen(ssid);
                remove_escaped_chars(ssid, &dummy);
            }
            else if (strncmp(ptr, "password=", 9) == 0)
            {
                password = ptr + 9;
                dummy = strlen(password);
                remove_escaped_chars(password, &dummy);
            }
            else if (strncmp(ptr, "hidden=", 7) == 0)
            {
                hidden = ptr + 7;
                dummy = strlen(hidden);
                remove_escaped_chars(hidden, &dummy);
            }
            
        }

        printf("[Presentation] Set SoftAP SSID=%s, Password=%s hidden=%s \r\n", ssid, password,hidden);

        if (!ssid || strlen(ssid) == 0)
        {
            printf("[Presentation] Error! The length of SSID is 0!\r\n");
            goto err_softap;
        }

        if (strncmp(hidden, "1", 1) == 0){
            printf("[Presentation] set SoatAP hidden \n");
            hostapd_set_hiddenssid(1);
        } else {
            hostapd_set_hiddenssid(0);
        }
        if (password && strlen(password) > 0)
        {
            hostapd_config_write(ssid, password);
        }
        else
        {
            hostapd_config_write(ssid, NULL);
        }

#ifdef CFG_USE_SD_INI
        snprintf(ini_filename, PRESENTATION_INIFILENAME_MAXLEN, "%s:/%s", "C", giniName);
#else
        snprintf(ini_filename, PRESENTATION_INIFILENAME_MAXLEN, "%s:/%s", CFG_PUBLIC_DRIVE, giniName);
#endif
        ini = iniparser_load(ini_filename);
        if (ini)
        {
            if (ssid)
            {
                iniparser_set(ini, PRESENTATION_INIKEY_URENDER_NAME, ssid);
                iniparser_set(ini, PRESENTATION_INIKEY_SHAIRPORT_NAME, ssid);
            }

            f = fopen(ini_filename, "w");
            if (f)
            {
                printf("[Presentation]%s() L#%ld: Write data to ini %s\r\n", __FUNCTION__, __LINE__, ini_filename);
                iniparser_dump_ini(ini, f);
                fclose(f);
#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
                ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_DISABLE, NULL);
                ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
                ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_ENABLE, NULL);
#endif
            }
            else
            {
                printf("[Presentation] Cannot open ini file %s to write: %ld\n", ini_filename, errno);
            }
            iniparser_freedict(ini);
        }
        else
        {
            printf("[Presentation] Cannot load ini file: %s\n", ini_filename);
            printf("[Presentation]%s() L#%ld: Do ugResetFactory() and reboot!\r\n", __FUNCTION__, __LINE__);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*)ITP_DISK_NOR);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORCE_MOUNT, (void*)ITP_DISK_NOR);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORMAT, (void*)1);
            ugResetFactory();
            i2s_CODEC_standby();
            exit(0);
            while (1);
        }

        printf("[Presentation] Set SoftAP data and Reboot!\r\n");
        ut->reboot = true;

        pthread_mutex_unlock(&mutex);
        return 0;

err_softap:
        pthread_mutex_unlock(&mutex);
        return -1;
    }
    else if (!strncmp(action, CGI_ACTION_WIFI, strlen(CGI_ACTION_WIFI)))
    {
        char      buf[256];
        char      * ptr;
        char      * ssid     = NULL;
        char      * password = NULL;
        char      * security = NULL;
        FILE      * f;
        dictionary* ini = NULL;
        char ini_filename[PRESENTATION_INIFILENAME_MAXLEN];
        size_t    dummy;

        pthread_mutex_lock(&mutex);

        if (ut->upgrading)
        {
            pthread_mutex_unlock(&mutex);
            return -1;
        }

        strcpy(buf, action + strlen(CGI_ACTION_WIFI) + 1);

        for (ptr = strtok(buf, "&"); ptr; ptr = strtok(NULL, "&"))
        {
            if (strncmp(ptr, "ssid=", 5) == 0)
            {
                ssid = ptr + 5;
                dummy = strlen(ssid);
                remove_escaped_chars(ssid, &dummy);
            }
            else if (strncmp(ptr, "password=", 9) == 0)
            {
                password = ptr + 9;
                dummy = strlen(password);
                remove_escaped_chars(password, &dummy);
            }
            else if (strncmp(ptr, "security=", 9) == 0)
            {
                security = ptr + 9;
            }
        }
        printf("[Presentation] Set WiFi SSID=%s, Password=%s, security=%s\r\n", ssid, password, security);

#ifdef CFG_USE_SD_INI
        snprintf(ini_filename, PRESENTATION_INIFILENAME_MAXLEN, "%s:/%s", "C", giniName);
#else
        snprintf(ini_filename, PRESENTATION_INIFILENAME_MAXLEN, "%s:/%s", CFG_PUBLIC_DRIVE, giniName);
#endif
        ini = iniparser_load(ini_filename);
        if (ini)
        {
            if (ssid)
            {
                iniparser_set(ini, PRESENTATION_INIKEY_WIFI_SSID, ssid);
            }
            if (password)
            {
                iniparser_set(ini, PRESENTATION_INIKEY_WIFI_PASSWORD, password);
            }
            if (security)
            {
                if (strcmp(security, "nosecu") == 0)
                {
                    iniparser_set(ini, PRESENTATION_INIKEY_WIFI_SECUMODE, "0");
                }
                else if (strcmp(security, "wep") == 0)
                {
                    iniparser_set(ini, PRESENTATION_INIKEY_WIFI_SECUMODE, "1");
                }
                else
                {
                    iniparser_set(ini, PRESENTATION_INIKEY_WIFI_SECUMODE, "6");
                }
            }

            f = fopen(ini_filename, "w");
            if (f)
            {
                printf("[Presentation]%s() L#%ld: Write data to ini %s\r\n", __FUNCTION__, __LINE__, ini_filename);
                iniparser_dump_ini(ini, f);
                fclose(f);
#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
                ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_DISABLE, NULL);
                ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
                ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_ENABLE, NULL);
#endif
            }
            else
            {
                printf("[Presentation] Cannot open ini file %s to write: %ld\n", ini_filename, errno);
            }
            iniparser_freedict(ini);
        }
        else
        {
            printf("[Presentation] Cannot load ini file: %s\n", ini_filename);
            printf("[Presentation]%s() L#%ld: Do ugResetFactory() and reboot!\r\n", __FUNCTION__, __LINE__);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*)ITP_DISK_NOR);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORCE_MOUNT, (void*)ITP_DISK_NOR);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORMAT, (void*)1);
            ugResetFactory();
            i2s_CODEC_standby();
            exit(0);
            while (1);
        }
        pthread_mutex_unlock(&mutex);
    }
#ifdef CFG_UPGRADE_ONLINE
    else if (!strncmp(action, CGI_ACTION_UPGRADE, strlen(CGI_ACTION_UPGRADE)))
    {
        int ret = 0;

        pthread_mutex_lock(&mutex);

        printf("upnp_content_length=%d, ushare_upgrade_content=0x%X\n", upnp_content_length, urender_upgrade_content);

        // upgrade firmware
        if (upnp_content_length > 0 && urender_upgrade_content)
        {
            ugArrayStream arrayStream;

            ugArrayStreamOpen(&arrayStream, urender_upgrade_content, upnp_content_length);

        #ifdef CFG_UPGRADE_CHECK_CRC
            if (ugCheckCrc(&arrayStream.stream))
            {
                puts("Upgrade failed.");
                pthread_mutex_unlock(&mutex);
                ret = -1;
            }
            else
        #endif // CFG_UPGRADE_CHECK_CRC
            {
            #ifdef CFG_WATCHDOG_ENABLE
                ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_DISABLE, NULL);
            #endif
                ret = ugUpgradePackage(&arrayStream.stream);

            #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
                puts("Flushing NOR cache...");
                ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_DISABLE, NULL);
                ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
                ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_ENABLE, NULL);
            #endif

            #ifdef CFG_WATCHDOG_ENABLE
                ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_ENABLE, NULL);
            #endif
            }

            if (ret)
                puts("Upgrade failed.");
            else
                puts("Upgrade finished.");

            free (urender_upgrade_content);
            urender_upgrade_content = NULL;
            upnp_content_length = 0;
        }
        pthread_mutex_unlock(&mutex);
        return ret;
    }
#endif // CFG_UPGRADE_ONLINE
    else if (!strncmp(action, CGI_ACTION_REBOOT, strlen(CGI_ACTION_REBOOT)))
    {
        printf("[Presentation] Reboot!\r\n");
        pthread_mutex_lock(&mutex);
        ut->reboot = true;
        pthread_mutex_unlock(&mutex);
        return 0;
    }
    else if (!strncmp(action, CGI_ACTION_RESET, strlen(CGI_ACTION_RESET)))
    {
        pthread_mutex_lock(&mutex);
        if (ut->upgrading)
        {
            pthread_mutex_unlock(&mutex);
            return -1;
        }

        printf("[Presentation] Reset to factory settings!\r\n");
        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*)ITP_DISK_NOR);
        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORCE_MOUNT, (void*)ITP_DISK_NOR);
        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORMAT, (void*)1);
        ugResetFactory();
#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
        ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_DISABLE, NULL);
        ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
        ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_ENABLE, NULL);
#endif
        pthread_mutex_unlock(&mutex);
        return 0;
    }
#ifdef CFG_NET_ETHERNET_MAC_ADDR_STORAGE
    else if (!strncmp(action, CGI_ACTION_MACADDR, strlen(CGI_ACTION_MACADDR)))
    {
        char* buf = NULL;
        char* ptr;
        char* macaddr0 = NULL;
        char* macaddr1 = NULL;
        char* macaddr2 = NULL;
        char* macaddr3 = NULL;
        char* macaddr4 = NULL;
        char* macaddr5 = NULL;
        int fd = -1;
        uint32_t pos, blocksize = 0;

    #if defined(CFG_NET_ETHERNET_MAC_ADDR_NAND)
        fd = open(":nand", O_WRONLY, 0);
    #elif defined(CFG_NET_ETHERNET_MAC_ADDR_NOR)
        fd = open(":nor", O_WRONLY, 0);
    #elif defined(CFG_NET_ETHERNET_MAC_ADDR_SD0)
        fd = open(":sd1", O_WRONLY, 0);
    #elif defined(CFG_NET_ETHERNET_MAC_ADDR_SD1)
        fd = open(":sd1", O_WRONLY, 0);
    #endif
        if (fd == -1)
        {
            printf("open device error: %d\n", fd);
            goto error;
        }
        if (ioctl(fd, ITP_IOCTL_GET_BLOCK_SIZE, &blocksize))
        {
            printf("get block size error\n");
            goto error;
        }
        printf("blocksize=%d\n", blocksize);

        pos = CFG_NET_ETHERNET_MAC_ADDR_POS / blocksize;
        if (lseek(fd, pos, SEEK_SET) != pos)
        {
            printf("seek to mac addr position %d(%d) error\n", CFG_NET_ETHERNET_MAC_ADDR_POS, pos);
            goto error;
        }

        assert(blocksize >= 8);
        buf = (uint8_t*) malloc(blocksize);
        if (!buf)
            goto error;

        strcpy(buf, action + strlen(CGI_ACTION_MACADDR) + 1);

        for (ptr = strtok(buf, "&"); ptr; ptr = strtok(NULL, "&"))
        {
            if (strncmp(ptr, "macaddr0=", 9) == 0)
            {
                macaddr0 = ptr + 9;
            }
            else if (strncmp(ptr, "macaddr1=", 9) == 0)
            {
                macaddr1 = ptr + 9;
            }
            else if (strncmp(ptr, "macaddr2=", 9) == 0)
            {
                macaddr2 = ptr + 9;
            }
            else if (strncmp(ptr, "macaddr3=", 9) == 0)
            {
                macaddr3 = ptr + 9;
            }
            else if (strncmp(ptr, "macaddr4=", 9) == 0)
            {
                macaddr4 = ptr + 9;
            }
            else if (strncmp(ptr, "macaddr5=", 9) == 0)
            {
                macaddr5 = ptr + 9;
            }
        }
        if (macaddr0 && macaddr1 && macaddr2 && macaddr3 && macaddr4 && macaddr5)
        {
            int i;
            uint16_t checksum;

            buf[0] = strtoul(macaddr0, NULL, 16);
            buf[1] = strtoul(macaddr1, NULL, 16);
            buf[2] = strtoul(macaddr2, NULL, 16);
            buf[3] = strtoul(macaddr3, NULL, 16);
            buf[4] = strtoul(macaddr4, NULL, 16);
            buf[5] = strtoul(macaddr5, NULL, 16);

            printf("upgrade mac address to %02X-%02X-%02X-%02X-%02X-%02X\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

            checksum = 0;
            for (i = 0; i < 6; i++)
                checksum += buf[i];

            buf[6] = (uint8_t) (checksum >> 8);
            buf[7] = (uint8_t) checksum;

            if (write(fd, buf, 1) != 1)
            {
                printf("write mac addr fail\n");
                goto error;
            }

        }
    error:
        if (fd != -1)
            close(fd);

        if (buf)
            free(buf);

        return 0;
    }
#endif // CFG_NET_ETHERNET_MAC_ADDR_STORAGE
    else if (!strncmp(action, CGI_ACTION_TRANSPORTER, strlen(CGI_ACTION_TRANSPORTER)))
    {
        char      buf[256];
        char      * ptr;
        char      * uartBaudrate    = NULL;
        char      * udpPort         = NULL;
        FILE      * f;
        dictionary* ini = NULL;
        char ini_filename[PRESENTATION_INIFILENAME_MAXLEN];

        pthread_mutex_lock(&mutex);

        if (ut->upgrading)
        {
            pthread_mutex_unlock(&mutex);
            return -1;
        }

        strcpy(buf, action + strlen(CGI_ACTION_TRANSPORTER) + 1);

        for (ptr = strtok(buf, "&"); ptr; ptr = strtok(NULL, "&"))
        {
            if (strncmp(ptr, "uartBaudrate=", 13) == 0)
            {
                uartBaudrate = ptr + 13;
            }
            else if (strncmp(ptr, "udpPort=", 8) == 0)
            {
                udpPort = ptr + 8;
            }
        }
        printf("[Presentation] Set Transporter uartBaudrate=%s, udpPort=%s\r\n", uartBaudrate, udpPort);

#ifdef CFG_USE_SD_INI
        snprintf(ini_filename, PRESENTATION_INIFILENAME_MAXLEN, "%s:/%s", "C", giniName);
#else
        snprintf(ini_filename, PRESENTATION_INIFILENAME_MAXLEN, "%s:/%s", CFG_PUBLIC_DRIVE, giniName);
#endif
        ini = iniparser_load(ini_filename);
        if (ini)
        {
            if (uartBaudrate)
            {
                iniparser_set(ini, PRESENTATION_INIKEY_TRANSPORTER_UART_BAUDRATE, uartBaudrate);
            }
            if (udpPort)
            {
                iniparser_set(ini, PRESENTATION_INIKEY_TRANSPORTER_UDP_PORT, udpPort);
            }

            f = fopen(ini_filename, "w");
            if (f)
            {
                printf("[Presentation]%s() L#%ld: Write data to ini %s\r\n", __FUNCTION__, __LINE__, ini_filename);
                iniparser_dump_ini(ini, f);
                fclose(f);
#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
                ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_DISABLE, NULL);
                ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
                ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_ENABLE, NULL);
#endif
            }
            else
            {
                printf("[Presentation] Cannot open ini file %s to write: %ld\n", ini_filename, errno);
            }
            iniparser_freedict(ini);
        }
        else
        {
            printf("[Presentation] Cannot load ini file: %s\n", ini_filename);
            printf("[Presentation]%s() L#%ld: Do ugResetFactory() and reboot!\r\n", __FUNCTION__, __LINE__);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*)ITP_DISK_NOR);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORCE_MOUNT, (void*)ITP_DISK_NOR);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORMAT, (void*)1);
            ugResetFactory();
            i2s_CODEC_standby();
            exit(0);
            while (1);
        }
        pthread_mutex_unlock(&mutex);
    }
    else if (!strncmp(action, CGI_ACTION_AUTOSHUTDOWN, strlen(CGI_ACTION_AUTOSHUTDOWN)))
    {
        char      buf[256];
        char      * ptr;
        char      * autoShutdown    = NULL;
        char      * autoShutdownTime  = NULL;
        FILE      * f;
        dictionary* ini = NULL;
        char ini_filename[PRESENTATION_INIFILENAME_MAXLEN];

        pthread_mutex_lock(&mutex);

        if (ut->upgrading)
        {
            pthread_mutex_unlock(&mutex);
            return -1;
        }

        strcpy(buf, action + strlen(CGI_ACTION_AUTOSHUTDOWN) + 1);

        for (ptr = strtok(buf, "&"); ptr; ptr = strtok(NULL, "&"))
        {
            if (strncmp(ptr, "auto_shutdown=", 14) == 0)
            {
                autoShutdown = ptr + 14;
            }
            else if (strncmp(ptr, "auto_shutdown_time=", 19) == 0)
            {
                autoShutdownTime = ptr + 19;
            }
        }
        printf("[Presentation] Set  auto_shutdown=%s, auto_shutdown_time=%s\r\n", autoShutdown, autoShutdownTime);

#ifdef CFG_USE_SD_INI
        snprintf(ini_filename, PRESENTATION_INIFILENAME_MAXLEN, "%s:/%s", "C", giniName);
#else
        snprintf(ini_filename, PRESENTATION_INIFILENAME_MAXLEN, "%s:/%s", CFG_PUBLIC_DRIVE, giniName);
#endif
        ini = iniparser_load(ini_filename);
        if (ini)
        {
            if (autoShutdown)
            {
                iniparser_set(ini, PRESENTATION_INIKEY_AUTOSHUTDOWN, autoShutdown);
            }
            if (autoShutdownTime)
            {
                iniparser_set(ini, PRESENTATION_INIKEY_AUTOSHUTDOWN_TIME, autoShutdownTime);
            }

            f = fopen(ini_filename, "w");
            if (f)
            {
                printf("[Presentation]%s() L#%ld: Write data to ini %s\r\n", __FUNCTION__, __LINE__, ini_filename);
                iniparser_dump_ini(ini, f);
                fclose(f);
#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
                ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_DISABLE, NULL);
                ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
                ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_ENABLE, NULL);
#endif
            }
            else
            {
                printf("[Presentation] Cannot open ini file %s to write: %ld\n", ini_filename, errno);
            }
            iniparser_freedict(ini);
        }
        else
        {
            printf("[Presentation] Cannot load ini file: %s\n", ini_filename);
            printf("[Presentation]%s() L#%ld: Do ugResetFactory() and reboot!\r\n", __FUNCTION__, __LINE__);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*)ITP_DISK_NOR);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORCE_MOUNT, (void*)ITP_DISK_NOR);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORMAT, (void*)1);
            ugResetFactory();
            i2s_CODEC_standby();
            exit(0);
            while (1);
        }
        pthread_mutex_unlock(&mutex);
    }

    // Client/AP mode swtich
#ifdef CFG_AUDIOLINK_NETWORK_SWITCH_ENABLE
    else if (!strncmp(action, CGI_ACTION_NETWORK_SWITCH, strlen(CGI_ACTION_NETWORK_SWITCH)))
    {
        char      buf[256];
        char      * ptr;
        char      * networkMode = NULL;
        FILE      * f;
        dictionary* ini = NULL;
        char ini_filename[PRESENTATION_INIFILENAME_MAXLEN];

        pthread_mutex_lock(&mutex);

        if (ut->upgrading) {
            pthread_mutex_unlock(&mutex);
            return -1;
        }

        strcpy(buf, action + strlen(CGI_ACTION_NETWORK_SWITCH) + 1);

        for (ptr = strtok(buf, "&"); ptr; ptr = strtok(NULL, "&")) {
            if (strncmp(ptr, "networkMode=", 12) == 0) {
                networkMode = ptr + 12;
            }
        }
        printf("[Presentation] Set switchNetwork Client/AP mode networkMode=%s\r\n", networkMode);

    #ifdef CFG_USE_SD_INI
        snprintf(ini_filename, PRESENTATION_INIFILENAME_MAXLEN, "%s:/%s", "C", giniName);
    #else
        snprintf(ini_filename, PRESENTATION_INIFILENAME_MAXLEN, "%s:/%s", CFG_PUBLIC_DRIVE, giniName);
    #endif
        ini = iniparser_load(ini_filename);
        if (ini) {
            if (networkMode) {
                iniparser_set(ini, PRESENTATION_INIKEY_NETWORK_SWITCH_NETWORK_MODE, networkMode);
            }

            f = fopen(ini_filename, "w");
            if (f) {
                printf("[Presentation]%s() L#%ld: Write data to ini %s\r\n", __FUNCTION__, __LINE__, ini_filename);
                iniparser_dump_ini(ini, f);
                fclose(f);
    #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
                ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_DISABLE, NULL);
                ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
                ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_ENABLE, NULL);
    #endif
            } else {
                printf("[Presentation] Cannot open ini file %s to write: %ld\n", ini_filename, errno);
            }
            iniparser_freedict(ini);
        }
        else
        {
            printf("[Presentation] Cannot load ini file: %s\n", ini_filename);
            printf("[Presentation]%s() L#%ld: Do ugResetFactory() and reboot!\r\n", __FUNCTION__, __LINE__);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*)ITP_DISK_NOR);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORCE_MOUNT, (void*)ITP_DISK_NOR);
            ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORMAT, (void*)1);
            ugResetFactory();
            i2s_CODEC_standby();
            exit(0);
            while (1);
        }
        pthread_mutex_unlock(&mutex);
        printf("[Presentation] Set Network Client/AP  mode data and Reboot!\r\n");
        ut->reboot = true;
    }
#endif
    else if (!strncmp(action, CGI_ACTION_SHAIRPORT, strlen(CGI_ACTION_SHAIRPORT)))
    {
        char      buf[256];
        char      * ptr;
        char      * softap   = NULL;
        char      * ssid     = NULL;
        char      * password = NULL;
        char      * hidden     = NULL;        
        FILE      * f;
        dictionary* ini = NULL;
        char ini_filename[PRESENTATION_INIFILENAME_MAXLEN];
        size_t    dummy;

        pthread_mutex_lock(&mutex);

        if (ut->upgrading)
        {
            pthread_mutex_unlock(&mutex);
            return -1;
        }

        strcpy(buf, action + strlen(CGI_ACTION_SHAIRPORT) + 1);

        for (ptr = strtok(buf, "&"); ptr; ptr = strtok(NULL, "&"))
        {
            if (strncmp(ptr, "shairport=", 10) == 0)
            {
                softap = ptr + 10;
            }
            else if (strncmp(ptr, "password=", 9) == 0)
            {
                password = ptr + 9;
                dummy = strlen(password);
                remove_escaped_chars(password, &dummy);
            }
        }
#if 0
        printf("[Presentation] Set Shairport Password=%s \r\n", password);

        if (password && strlen(password) > 0)
        {
            audiolink_set_airplay_password_ini(password,strlen(password));
        }

        printf("[Presentation] Set Shairport password!\r\n");
#endif
        pthread_mutex_unlock(&mutex);
        return 0;

err_airplay:
        pthread_mutex_unlock(&mutex);
        return -1;
    }

    if (ut->presentation)
    {
        buffer_free(ut->presentation);
    }

    ut->presentation = buffer_new();

    buffer_append(ut->presentation, "<html>");
    buffer_append(ut->presentation, "<head>");
    buffer_appendf(ut->presentation, "<title>%s</title>",
                   _(CFG_SYSTEM_NAME " Information Page"));
    buffer_append(ut->presentation,
                  "<meta http-equiv=\"pragma\" content=\"no-cache\"/>");
    buffer_append(ut->presentation,
                  "<meta http-equiv=\"cache-control\" content=\"no-cache\"/>");
    buffer_append(ut->presentation,
                  "<meta http-equiv=\"expires\" content=\"0\"/>");
    buffer_append(ut->presentation,
                  "<meta http-equiv=\"refresh\" content=\"0; URL=/web/info.html\"/>");
    buffer_append(ut->presentation, "</head>");
    buffer_append(ut->presentation, "</html>");

    return 0;
}


int
build_presentation_page(struct urender_t *ut, char *page_name)
{
    char            *mycodeset = NULL;
    xmlDocPtr       doc;
    xmlXPathContext *xpathCtx;
    xmlXPathObject  * xpathObj;
    xmlChar         *xmlbuff;
    int             buffersize;
    dictionary      * ini = NULL;
    char ini_filename[PRESENTATION_INIFILENAME_MAXLEN];
    char html_filename[PRESENTATION_HTMLFILENAME_MAXLEN];
    char ssid[PRESENTATION_SSID_MAXLEN];
    char password[PRESENTATION_PASSWORD_MAXLEN];

    if (!ut)
        return -1;

	pthread_mutex_lock(&mutex);

    if (ut->upgrading)
    {
        return -1;
    }

    if (ut->presentation)
        buffer_free(ut->presentation);

    ut->presentation = buffer_new();
    // check auto shutdown
    gPresentionState = 1;
#if HAVE_LANGINFO_CODESET
    mycodeset = nl_langinfo(CODESET);
#endif
    if (!mycodeset)
        mycodeset = UTF8;

    if (page_name && !strcmp (page_name, URENDER_MOBILE_PAGE))
    {
#ifdef CFG_USE_SD_INI
        snprintf(html_filename, PRESENTATION_HTMLFILENAME_MAXLEN, "C:/%s", "mobile.html");
#else
        snprintf(html_filename, PRESENTATION_HTMLFILENAME_MAXLEN, "%s:/%s", CFG_PRIVATE_DRIVE, "mobile.html");
#endif
    }
    else
    {
#ifdef CFG_USE_SD_INI
        snprintf(html_filename, PRESENTATION_HTMLFILENAME_MAXLEN, "C:/%s", gwebName);
#else
        snprintf(html_filename, PRESENTATION_HTMLFILENAME_MAXLEN, "%s:/%s", CFG_PRIVATE_DRIVE, gwebName);
#endif
    }

    doc      = xmlParseFile(html_filename);
    xpathCtx = xmlXPathNewContext(doc);

    // mac address
    xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='cfg_macaddr']", xpathCtx);
    if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
    {
        xmlNode* node;
        xmlNode* text;
        char buf[32];

#ifdef CFG_NET_ETHERNET
        ITPEthernetInfo info;
        ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_GET_INFO, &info);
        
#elif defined(CFG_NET_WIFI)
        ITPWifiInfo info;
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_GET_INFO, &info);
#endif

        sprintf(buf, "%02X-%02X-%02X-%02X-%02X-%02X", info.hardwareAddress[0], info.hardwareAddress[1], info.hardwareAddress[2], info.hardwareAddress[3], info.hardwareAddress[4], info.hardwareAddress[5]);

        node = xpathObj->nodesetval->nodeTab[0];
        text = xmlNewText(BAD_CAST buf);
        xmlAddChild(node, text);
    }
    xmlXPathFreeObject(xpathObj);

    // version
    xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='cfg_version']", xpathCtx);
    if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
    {
        xmlNode* node = xpathObj->nodesetval->nodeTab[0];
        xmlNode* text = xmlNewText(BAD_CAST CFG_VERSION_MAJOR_STR "." CFG_VERSION_MINOR_STR "." CFG_VERSION_PATCH_STR "." CFG_VERSION_CUSTOM_STR "." CFG_VERSION_TWEAK_STR);
        xmlAddChild(node, text);
    }
    xmlXPathFreeObject(xpathObj);

    // soft ap
    memset(ssid, 0, PRESENTATION_SSID_MAXLEN);
    memset(password, 0, PRESENTATION_PASSWORD_MAXLEN);
    iteGetHostapdSetting(ssid, password);

    // ssid
    xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='cfg_softap_curr_ssid']", xpathCtx);
    if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
    {
        xmlNode* node = xpathObj->nodesetval->nodeTab[0];
        xmlNode* text = xmlNewText(BAD_CAST (strlen(ssid) ? ssid : " "));
        xmlAddChild(node, text);
    }
    xmlXPathFreeObject(xpathObj);

    // password
    xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='cfg_softap_curr_passwd']", xpathCtx);
    if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
    {
        xmlNode* node = xpathObj->nodesetval->nodeTab[0];
        xmlNode* text = xmlNewText(BAD_CAST (strlen(password) ? password : " "));
        xmlAddChild(node, text);
    }
    xmlXPathFreeObject(xpathObj);

    // security
    xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='cfg_softap_curr_secu']", xpathCtx);
    if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
    {
        xmlNode* node = xpathObj->nodesetval->nodeTab[0];
        xmlNode* text = xmlNewText(BAD_CAST (strlen(password) ? "Yes" : "No"));
        xmlAddChild(node, text);
    }
    xmlXPathFreeObject(xpathObj);

    // wifi
#ifdef CFG_USE_SD_INI
    snprintf(ini_filename, PRESENTATION_INIFILENAME_MAXLEN, "%s:/%s", "C", giniName);
#else
    snprintf(ini_filename, PRESENTATION_INIFILENAME_MAXLEN, "%s:/%s", CFG_PUBLIC_DRIVE, giniName);
#endif
    ini = iniparser_load(ini_filename);
    if (ini)
    {
        // ssid
        xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='cfg_wifi_curr_ssid']", xpathCtx);
        if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
        {
            xmlNode* node = xpathObj->nodesetval->nodeTab[0];
            xmlNode* text = xmlNewText(BAD_CAST iniparser_getstring(ini, PRESENTATION_INIKEY_WIFI_SSID, " "));
            xmlAddChild(node, text);
        }
        xmlXPathFreeObject(xpathObj);

        // ssid list
        xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='cfg_wifi_ssid_list']", xpathCtx);
        if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
        {
            struct net_device_info ScanApInfo = {0};

            presentation_scanap(&ScanApInfo);
            if (ScanApInfo.apCnt > 0)
            {
                xmlNodePtr node;
            	unsigned int apIndex = 0;
                char* securityModeString = NULL;
                char itemString[128];
                char *p = NULL;
                int len = 0;

                // append a blank item first
                memset(itemString, 0, sizeof(itemString));
    			node = xmlNewChild(
    				xpathObj->nodesetval->nodeTab[0],
    				NULL,
    				BAD_CAST "option",
    				itemString);
                xmlNewProp(node, BAD_CAST "value", BAD_CAST "0");

                for (apIndex = 0; apIndex < ScanApInfo.apCnt; apIndex++)
                {
                    if (strlen(ScanApInfo.apList[apIndex].ssidName) == 0)
                    {
                        // skip empty ssid
                        continue;
                    }

                    snprintf(itemString, sizeof(itemString), "(%3ld%%) %s", ScanApInfo.apList[apIndex].rfQualityRSSI, ScanApInfo.apList[apIndex].ssidName);
                    // handle & for xml, replace "&" with "&amp;"
                    len = strlen(itemString);
                    p = strchr(itemString, '&');
                    while (p)
                    {
                        memmove(p + 5, p + 1, len - (p - itemString));
                        memcpy(p, "&amp;", 5);
                        p = strchr(p + 1, '&');
                    }
        			node = xmlNewChild(
        				xpathObj->nodesetval->nodeTab[0],
        				NULL,
        				BAD_CAST "option",
        				itemString);

        			switch(ScanApInfo.apList[apIndex].securityMode)
        			{
        			case 0:
                        securityModeString = "0";
                        break;
        			case 1:
                        securityModeString = "1";
                        break;
        			case 6:
                        securityModeString = "2";
                        break;
        			default:
                        securityModeString = "0";
                        break;
        			}
        			xmlNewProp(node, BAD_CAST "value", BAD_CAST securityModeString);
        			printf("[Presentation] Add node: %s, %s, rfQualityRSSI=%ld%%\n", ScanApInfo.apList[apIndex].ssidName, securityModeString, ScanApInfo.apList[apIndex].rfQualityRSSI);
                }
            }
            else
            {
                xmlAddChild(xpathObj->nodesetval->nodeTab[0], xmlNewText(NULL));
            }
        }
        xmlXPathFreeObject(xpathObj);

        // password
        xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='cfg_wifi_curr_passwd']", xpathCtx);
        if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
        {
            xmlNode* node = xpathObj->nodesetval->nodeTab[0];
            xmlNode* text = xmlNewText(BAD_CAST iniparser_getstring(ini, PRESENTATION_INIKEY_WIFI_PASSWORD, " "));
            xmlAddChild(node, text);
        }
        xmlXPathFreeObject(xpathObj);

        // security
        xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='cfg_wifi_curr_secu']", xpathCtx);
        if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
        {
            xmlNode* node = xpathObj->nodesetval->nodeTab[0];
            xmlNode* text;
            int secu_val = 0;

            secu_val = iniparser_getint(ini, PRESENTATION_INIKEY_WIFI_SECUMODE, 0);
            if (secu_val == 0)
            {
                // No security
                text = xmlNewText(BAD_CAST "No Secutiry");
            }
            else if (secu_val == 1)
            {
                // WEP
                text = xmlNewText(BAD_CAST "WEP");
            }
            else if (secu_val == 2 || secu_val == 6)
            {
                // WEP
                text = xmlNewText(BAD_CAST "WPA-PSK/WPA2-PSK");
            }
            else
            {
                // WPA-PSK/WPA2-PSK
                text = xmlNewText(BAD_CAST "Unknown");
            }
            xmlAddChild(node, text);
        }
        xmlXPathFreeObject(xpathObj);

        // transporter
        // uart baudrate
        xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='cfg_transporter_uartBaudrate']", xpathCtx);
        if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
        {
            char buf[16];
            xmlNode* node = xpathObj->nodesetval->nodeTab[0];
            int val = iniparser_getint(ini, PRESENTATION_INIKEY_TRANSPORTER_UART_BAUDRATE, 9600);
            xmlNode* text;

            sprintf(buf, "%d", val);

            text = xmlNewText(BAD_CAST buf);
            xmlAddChild(node, text);
        }
        xmlXPathFreeObject(xpathObj);

        // udp port
        xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='cfg_transporter_udpPort']", xpathCtx);
        if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
        {
            char buf[16];
            xmlNode* node = xpathObj->nodesetval->nodeTab[0];
            int val = iniparser_getint(ini, PRESENTATION_INIKEY_TRANSPORTER_UDP_PORT, 8888);
            xmlNode* text;

            sprintf(buf, "%d", val);

            text = xmlNewText(BAD_CAST buf);
            xmlAddChild(node, text);
        }
        xmlXPathFreeObject(xpathObj);

        iniparser_freedict(ini);
    }
    else
    {
        printf("[Presentation] Cannot load ini file: %s\n", ini_filename);
        printf("[Presentation]%s() L#%ld: Do ugResetFactory() and reboot!\r\n", __FUNCTION__, __LINE__);
        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*)ITP_DISK_NOR);
        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORCE_MOUNT, (void*)ITP_DISK_NOR);
        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORMAT, (void*)1);
        ugResetFactory();
        i2s_CODEC_standby();
        exit(0);
        while (1);
    }

    xmlDocDumpFormatMemory(doc, &xmlbuff, &buffersize, 1);
    buffer_append(ut->presentation, (char *) xmlbuff);

    xmlXPathFreeContext(xpathCtx);
    xmlFree(xmlbuff);
    xmlFreeDoc(doc);

	pthread_mutex_unlock(&mutex);

    return 0;
}



int
build_devmode_page(struct urender_t *ut)
{
    char            *mycodeset = NULL;
    xmlDocPtr       doc;
    xmlChar         *xmlbuff;
    int             buffersize;

    if (!ut)
        return -1;

    if (ut->upgrading)
    {
        return -1;
    }
    
    pthread_mutex_lock(&mutex);

    if (ut->presentation)
        buffer_free(ut->presentation);
    ut->presentation = buffer_new();

#if HAVE_LANGINFO_CODESET
    mycodeset = nl_langinfo(CODESET);
#endif
    if (!mycodeset)
        mycodeset = UTF8;

    doc      = xmlParseFile(CFG_PRIVATE_DRIVE ":/devmode.html");
    //printf("doc=0x%X\n", doc);

    xmlDocDumpFormatMemory(doc, &xmlbuff, &buffersize, 1);
    //printf("xmlbuff=%s\n", xmlbuff);
    buffer_append(ut->presentation, (char *) xmlbuff);

    xmlFree(xmlbuff);
    xmlFreeDoc(doc);

    pthread_mutex_unlock(&mutex);

    return 0;
}



int
build_aplist_xml(struct urender_t *ut)
{
    struct net_device_info ScanApInfo = {0};
    #define TMPSTR_LEN 128
    char tmpstr[TMPSTR_LEN];
    unsigned int apIndex = 0;
    int secuMode = 0, rfQRSSI = 0;
    char *p = NULL;
    int len = 0;

    presentation_scanap(&ScanApInfo);

    if (ut->presentation)
    {
        buffer_free(ut->presentation);
    }

    ut->presentation = buffer_new();

    buffer_append(ut->presentation, "<?xml version=\"1.0\"?>");
    snprintf(tmpstr, TMPSTR_LEN, "<aplist number=\"%ld\">", ScanApInfo.apCnt);
    buffer_append(ut->presentation, tmpstr);

    for (apIndex = 0; apIndex < ScanApInfo.apCnt; apIndex++)
    {
        if (strlen(ScanApInfo.apList[apIndex].ssidName) == 0)
        {
            // skip empty ssid
            continue;
        }

        secuMode = ScanApInfo.apList[apIndex].securityMode;
        rfQRSSI = ScanApInfo.apList[apIndex].rfQualityRSSI;
        snprintf(tmpstr, TMPSTR_LEN, "<ap id=\"%s\" security=\"%d\">%d</ap>",
                 ScanApInfo.apList[apIndex].ssidName,
                 ((secuMode == 1) ? 1 : ((secuMode == 6) ? 2 : 0)),
                 ((rfQRSSI >= 67) ? 3 : ((rfQRSSI >= 34) ? 2 : 1)));

        // handle & for xml, replace "&" with "&amp;"
        len = strlen(tmpstr);
        p = strchr(tmpstr, '&');
        while (p)
        {
            memmove(p + 5, p + 1, len - (p - tmpstr));
            memcpy(p, "&amp;", 5);
            p = strchr(p + 1, '&');
        }

        buffer_append(ut->presentation, tmpstr);
    }

    buffer_append(ut->presentation, "</aplist>");

    return 0;
}



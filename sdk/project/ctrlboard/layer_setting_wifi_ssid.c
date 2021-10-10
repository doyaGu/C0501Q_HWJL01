#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "ctrlboard.h"
#include "wifiMgr.h"

static ITUScrollListBox *settingWiFiSsidNameScrollListBox;
static ITUScrollListBox *settingWiFiSsidStatusScrollListBox;
static ITUScrollListBox *settingWiFiSsidSignalScrollListBox;
static ITULayer         *settingWiFiPasswordLayer;
static ITULayer         *settingWiFiNetworkLayer;

/*
static const char* settingWiFiSsidNameArray[] =
{
    "A SSID",
    "B SSID",
    "C SSID",
    "D SSID",
    "E SSID",
    "F SSID",
    "G SSID",
};
*/
static WIFI_MGR_SCANAP_LIST pList[64];
static int                  gnApCount = 0;

#define MAX_SSID_COUNT 15 // (sizeof(settingWiFiSsidNameArray) / sizeof(settingWiFiSsidNameArray[0]))

static int getMaxSsidCount()
{
    if (gnApCount <= 0)
    {
        return MAX_SSID_COUNT;
    }
    else
    {
        return gnApCount;
    }
}

bool SettingWiFiSsidNameScrollListBoxOnLoad(ITUWidget *widget, char *param)
{
    ITUListBox       *listbox  = (ITUListBox *) widget;
    ITUScrollListBox *slistbox = (ITUScrollListBox *) listbox;
    ITCTree          *node;
    int              i, j, count;

    assert(listbox);

    count              = ituScrollListBoxGetItemCount(slistbox);
    node               = ituScrollListBoxGetLastPageItem(slistbox);

    listbox->pageCount = getMaxSsidCount() ? (getMaxSsidCount() + count - 1) / count : 1;

    printf("SettingWiFiSsidNameScrollListBoxOnLoad,      SSID count per page = %d,   Need %d pages to show all counts \n", count, listbox->pageCount);

    if (listbox->pageIndex == 0)
    {
        // initialize
        listbox->pageIndex  = 1;
        listbox->focusIndex = -1;
    }

    if (listbox->pageIndex <= 1)
    {
        for (i = 0; i < count; i++)
        {
            ITUScrollText *scrolltext = (ITUScrollText *) node;
            ituScrollTextSetString(scrolltext, "");

            node = node->sibling;
        }
    }

    i = 0;
    j = count * (listbox->pageIndex - 2);
    if (j < 0)
        j = 0;

    for (; j < getMaxSsidCount(); j++)
    {
        ITUScrollText *scrolltext = (ITUScrollText *) node;
        char          buf[32];
        const char    *entry;//= settingWiFiSsidNameArray[j];

        if (theConfig.wifi_status == WIFIMGR_SWITCH_ON){
        	sprintf(buf, "%s", pList[j].ssidName);
		}
		else if (theConfig.wifi_status == WIFIMGR_SWITCH_OFF){
			/*CANCEL PANEL DISPALY*/
			memcpy(pList[j].ssidName,NULL,32);
			pList[j].rfQualityRSSI = NULL;

            sprintf(buf, "%s", pList[j].ssidName);
			sprintf(buf, "%s", pList[j].rfQualityRSSI);
		}

        //printf("ssid %s \n",buf);

        ituScrollTextSetString(scrolltext, (char *)buf);
        ituWidgetSetCustomData(scrolltext, j);

        i++;

        node = node->sibling;

        if (node == NULL)
            break;
    }

    for (; node; node = node->sibling)
    {
        ITUScrollText *scrolltext = (ITUScrollText *) node;
        ituScrollTextSetString(scrolltext, "");
    }

    if (listbox->pageIndex == listbox->pageCount)
    {
        listbox->itemCount = i % count;
        if (listbox->itemCount == 0)
            listbox->itemCount = count;
    }
    else
        listbox->itemCount = count;

    return true;
}

bool SettingWiFiSsidStatusScrollListBoxOnLoad(ITUWidget *widget, char *param)
{
    ITUListBox       *listbox  = (ITUListBox *) widget;
    ITUScrollListBox *slistbox = (ITUScrollListBox *) listbox;
    ITCTree          *node;
    int              i, j, count;
    int              nRet;
    int              nConnect = 0;
    int              bIsAvail = 0;

    assert(listbox);

    count              = ituScrollListBoxGetItemCount(slistbox);
    node               = ituScrollListBoxGetLastPageItem(slistbox);

    listbox->pageCount = getMaxSsidCount() ? (getMaxSsidCount() + count - 1) / count : 1;

    if (listbox->pageIndex == 0)
    {
        // initialize
        listbox->pageIndex  = 1;
        listbox->focusIndex = -1;
    }

    if (listbox->pageIndex <= 1)
    {
        for (i = 0; i < count; i++)
        {
            ITUScrollText *scrolltext = (ITUScrollText *) node;
            ituScrollTextSetString(scrolltext, "");

            node = node->sibling;
        }
    }

    i = 0;
    j = count * (listbox->pageIndex - 2);
    if (j < 0)
        j = 0;

#ifdef CFG_NET_WIFI
    nRet = wifiMgr_is_wifi_available(&bIsAvail);
#endif
    printf("SettingWiFiSsidStatusScrollListBoxOnLoad,    IsAvail = %d,   Max Ssid Count = %d,   switch status = %d  \n", bIsAvail, getMaxSsidCount(), theConfig.wifi_status);

    for (; j < getMaxSsidCount(); j++)
    {
        ITUScrollText *scrolltext = (ITUScrollText *) node;

        if (!strncmp(theConfig.ssid, pList[j].ssidName, strlen(pList[j].ssidName)))
        {
            if (bIsAvail && nConnect == 0)
            {
                ituScrollTextSetString(scrolltext, (char *)StringGetWiFiConnected());
                printf("SsidStatus %s \n", theConfig.ssid);
                nConnect++;
            }
            else
            {
                ituScrollTextSetString(scrolltext, "");
            }
        }
        else
        {
            ituScrollTextSetString(scrolltext, "");
        }

        ituWidgetSetCustomData(scrolltext, j);

        i++;

        node = node->sibling;

        if (node == NULL)
            break;
    }

    for (; node; node = node->sibling)
    {
        ITUScrollText *scrolltext = (ITUScrollText *) node;
        ituScrollTextSetString(scrolltext, "");
    }

    if (listbox->pageIndex == listbox->pageCount)
    {
        listbox->itemCount = i % count;
        if (listbox->itemCount == 0)
            listbox->itemCount = count;
    }
    else
        listbox->itemCount = count;

    return true;
}

bool SettingWiFiSsidSignalScrollListBoxOnLoad(ITUWidget *widget, char *param)
{
    ITUListBox       *listbox  = (ITUListBox *) widget;
    ITUScrollListBox *slistbox = (ITUScrollListBox *) listbox;
    ITCTree          *node;
    int              i, j, count;
    assert(listbox);

    count              = ituScrollListBoxGetItemCount(slistbox);
    node               = ituScrollListBoxGetLastPageItem(slistbox);

    listbox->pageCount = getMaxSsidCount() ? (getMaxSsidCount() + count - 1) / count : 1;

    if (listbox->pageIndex == 0)
    {
        // initialize
        listbox->pageIndex  = 1;
        listbox->focusIndex = -1;
    }

    if (listbox->pageIndex <= 1)
    {
        for (i = 0; i < count; i++)
        {
            ITUScrollText *scrolltext = (ITUScrollText *) node;
            ituScrollTextSetString(scrolltext, "");

            node = node->sibling;
        }
    }

    i = 0;
    j = count * (listbox->pageIndex - 2);
    if (j < 0)
        j = 0;

    for (; j < getMaxSsidCount(); j++)
    {
        ITUScrollText *scrolltext = (ITUScrollText *) node;
        char          buf[8];

        sprintf(buf, "%d%%", pList[j].rfQualityRSSI);
        ituScrollTextSetString(scrolltext, buf);

        ituWidgetSetCustomData(scrolltext, j);

        i++;

        node = node->sibling;

        if (node == NULL)
            break;
    }

    for (; node; node = node->sibling)
    {
        ITUScrollText *scrolltext = (ITUScrollText *) node;
        ituScrollTextSetString(scrolltext, "");
    }

    if (listbox->pageIndex == listbox->pageCount)
    {
        listbox->itemCount = i % count;
        if (listbox->itemCount == 0)
            listbox->itemCount = count;
    }
    else
        listbox->itemCount = count;

    return true;
}

bool SettingWiFiSsidScrollListBoxOnSelect(ITUWidget *widget, char *param)
{
    ITUListBox           *listbox    = (ITUListBox *) widget;
    ITUScrollIconListBox *silistbox  = (ITUScrollIconListBox *) listbox;
    ITUScrollText        *scrolltext = (ITUScrollText *)ituListBoxGetFocusItem(listbox);
    int                  nIndex;

    if (scrolltext)
    {
        int i = (int)ituWidgetGetCustomData(scrolltext);
        nIndex = ((listbox->pageIndex - 1) * 6) + listbox->focusIndex;
        if (!strncmp(theConfig.ssid, pList[nIndex].ssidName, strlen(pList[nIndex].ssidName)))
        {
            printf("SettingWiFiSsidScrollListBoxOnSelect \n");
            //ituLayerGoto(settingWiFiNetworkLayer);
        }
        else
        {
            //printf("SettingWiFiSsidScrollListBoxOnSelect pw %s %d %d \n",scrolltext->text.string,listbox->pageIndex,listbox->focusIndex);
            SettingWiFiPasswordSetData(pList[nIndex].ssidName, pList[nIndex].securityMode);
            ituLayerGoto(settingWiFiPasswordLayer);
        }
    }
    return true;
}

bool SettingWiFiSsidOnEnter(ITUWidget *widget, char *param)
{
    int nRet;
    int nWiFiConnState = 0, nWiFiConnEcode = 0;

#ifdef CFG_NET_WIFI
    nRet = wifiMgr_get_connect_state(&nWiFiConnState, &nWiFiConnEcode);
    if (nWiFiConnState == WIFIMGR_CONNSTATE_CONNECTING)
    {
        printf("SettingWiFiSsidOnEnter connecting ,wait ....................\n");
        usleep(500000);
    }
	if (theConfig.wifi_status == WIFIMGR_SWITCH_ON){
    gnApCount = wifiMgr_get_scan_ap_info(pList);
	}
	else{
    	printf("Wifi is closed, no SSID list...\n");
	}
#endif
    if (!settingWiFiSsidNameScrollListBox)
    {
        settingWiFiSsidNameScrollListBox   = ituSceneFindWidget(&theScene, "settingWiFiSsidNameScrollListBox");
        assert(settingWiFiSsidNameScrollListBox);

        settingWiFiSsidStatusScrollListBox = ituSceneFindWidget(&theScene, "settingWiFiSsidStatusScrollListBox");
        assert(settingWiFiSsidStatusScrollListBox);

        settingWiFiSsidSignalScrollListBox = ituSceneFindWidget(&theScene, "settingWiFiSsidSignalScrollListBox");
        assert(settingWiFiSsidSignalScrollListBox);

        settingWiFiPasswordLayer           = ituSceneFindWidget(&theScene, "settingWiFiPasswordLayer");
        assert(settingWiFiPasswordLayer);

        settingWiFiNetworkLayer            = ituSceneFindWidget(&theScene, "settingWiFiNetworkLayer");
        assert(settingWiFiNetworkLayer);
    }

    return true;
}

bool SettingWiFiSsidOnLeave(ITUWidget *widget, char *param)
{
    return true;
}

void SettingWiFiSsidReset(void)
{
    settingWiFiSsidNameScrollListBox = NULL;
}
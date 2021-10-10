#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "ctrlboard.h"
#include "wifiMgr.h"

static ITUSprite* settingWiFiMainTypeSprite;
static ITUBackground* settingWiFiMainGrayBackground;
static ITURadioBox* settingWiFiMainDisableRadioBox;
static ITURadioBox* settingWiFiMainAPRadioBox;
static ITURadioBox* settingWiFiMainConnectRadioBox;
ITUTextBox* settingWiFiMainNameTextBox;
ITUTextBox* settingWiFiMainPasswordTextBox;

// status
static int settingWiFiMainOnOffOld;
static char settingWiFiMainSsidOld[64];
static char settingWiFiMainPasswordOld[256];

bool SettingWiFiMainRadioBoxOnPress(ITUWidget* widget, char* param)
{
    if (ituRadioBoxIsChecked(settingWiFiMainDisableRadioBox))
    {
        theConfig.wifi_on_off = 0;
		theConfig.wifi_status = WIFIMGR_SWITCH_OFF;

		/*Clean wifi info when wifi is turned off*/
        strcpy(theConfig.ssid    , "");
        strcpy(theConfig.password, "");
        strcpy(theConfig.secumode, "");

        #ifdef CFG_NET_WIFI
        wifiMgr_clientMode_disconnect();
        #endif
        ituSpriteGoto(settingWiFiMainTypeSprite, 0);
    }
    else if (ituRadioBoxIsChecked(settingWiFiMainAPRadioBox))
    {
        theConfig.wifi_on_off = 1;
        ituSpriteGoto(settingWiFiMainTypeSprite, 1);
    }
    else if (ituRadioBoxIsChecked(settingWiFiMainConnectRadioBox))
    {
        theConfig.wifi_on_off = 2;
		theConfig.wifi_status = WIFIMGR_SWITCH_ON;
		#ifdef CFG_NET_WIFI
		WifiMgr_clientMode_switch(theConfig.wifi_status);
		#endif
        ituSpriteGoto(settingWiFiMainTypeSprite, 2);
    }

	ConfigSave();

    return true;
}

bool SettingWiFiMainOnEnter(ITUWidget* widget, char* param)
{
    if (!settingWiFiMainTypeSprite)
    {
        settingWiFiMainTypeSprite = ituSceneFindWidget(&theScene, "settingWiFiMainTypeSprite");
        assert(settingWiFiMainTypeSprite);

        settingWiFiMainGrayBackground = ituSceneFindWidget(&theScene, "settingWiFiMainGrayBackground");
        assert(settingWiFiMainGrayBackground);

        settingWiFiMainDisableRadioBox = ituSceneFindWidget(&theScene, "settingWiFiMainDisableRadioBox");
        assert(settingWiFiMainDisableRadioBox);

        settingWiFiMainAPRadioBox = ituSceneFindWidget(&theScene, "settingWiFiMainAPRadioBox");
        assert(settingWiFiMainAPRadioBox);

        settingWiFiMainConnectRadioBox = ituSceneFindWidget(&theScene, "settingWiFiMainConnectRadioBox");
        assert(settingWiFiMainConnectRadioBox);

        settingWiFiMainNameTextBox = ituSceneFindWidget(&theScene, "settingWiFiMainNameTextBox");
        assert(settingWiFiMainNameTextBox);

        settingWiFiMainPasswordTextBox = ituSceneFindWidget(&theScene, "settingWiFiMainPasswordTextBox");
        assert(settingWiFiMainPasswordTextBox);
    }

    // current settings
    switch (theConfig.wifi_on_off)
    {
    case 0:
        ituRadioBoxSetChecked(settingWiFiMainDisableRadioBox, true);
        ituSpriteGoto(settingWiFiMainTypeSprite, 0);
        break;

    case 1:
        ituRadioBoxSetChecked(settingWiFiMainAPRadioBox, true);
        ituSpriteGoto(settingWiFiMainTypeSprite, 1);
        break;

    case 2:
        ituRadioBoxSetChecked(settingWiFiMainConnectRadioBox, true);
        ituSpriteGoto(settingWiFiMainTypeSprite, 2);
        break;
	}
    settingWiFiMainOnOffOld = theConfig.wifi_on_off;

    ituTextBoxSetString(settingWiFiMainNameTextBox, theConfig.ssid);
    strcpy(settingWiFiMainSsidOld, theConfig.ssid);

    ituTextBoxSetString(settingWiFiMainPasswordTextBox, theConfig.password);
    strcpy(settingWiFiMainPasswordOld, theConfig.password);

    return true;
}

bool SettingWiFiMainOnLeave(ITUWidget* widget, char* param)
{
    if (settingWiFiMainOnOffOld != theConfig.wifi_on_off ||
        strcmp(settingWiFiMainSsidOld, theConfig.ssid) ||
        strcmp(settingWiFiMainPasswordOld, theConfig.password))
    {
        ConfigSave();
    }
    return true;
}

void SettingWiFiMainReset(void)
{
    settingWiFiMainTypeSprite = NULL;
}

#include <sys/ioctl.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ite/itp.h"
#include "scene.h"
#include "ctrlboard.h"

static ITUSprite* settingWiFiNetworkIPAssignSprite;
static ITUText* settingWiFiNetworkIPAddrText;
static ITUText* settingWiFiNetworkNetmaskText;
static ITUText* settingWiFiNetworkDNSText;
static ITUText* settingWiFiNetworkMacText;
static ITUBackground* settingWiFiNetworkGrayBackground;
static ITURadioBox* settingWiFiNetworkStaticIPRadioBox;
static ITURadioBox* settingWiFiNetworkDHCPRadioBox;
static ITUBackground* settingBackground;
static ITUBackground* settingWiFiNetworkBackground;
static ITUBackgroundButton* settingWiFiNetworkIPAssignBackgroundButton;
static ITUBackground* mainTopBackground;
static ITUBackgroundButton* settingWiFiNetworkIPInputBackgroundButton;	
static ITUTextBox* settingWiFiNetworkIPInput1TextBox;
static ITUTextBox* settingWiFiNetworkIPInput2TextBox;
static ITUTextBox* settingWiFiNetworkIPInput3TextBox;
static ITUTextBox* settingWiFiNetworkIPInput4TextBox;

// status
static int settingWiFiNetworkIPAssignOld;
static char settingWiFiNetworkIPAddrOld[16];
static char settingWiFiNetworkNetmaskOld[16];
static char settingWiFiNetworkDnsOld[16];
static ITUText* activeSettingWiFiNetworkText;
static ITUTextBox* activeSettingWiFiNetworkIPInputTextBox;
static ITPEthernetInfo settingWiFiNetworkEthernetInfo;

bool SettingWiFiNetworkIPAssignRadioBoxOnPress(ITUWidget* widget, char* param)
{
    if (ituRadioBoxIsChecked(settingWiFiNetworkStaticIPRadioBox))
    {
        theConfig.dhcp = 0;
        ituSpriteGoto(settingWiFiNetworkIPAssignSprite, 0);
    }
    else if (ituRadioBoxIsChecked(settingWiFiNetworkDHCPRadioBox))
    {
        theConfig.dhcp = 1;
        ituSpriteGoto(settingWiFiNetworkIPAssignSprite, 1);
    }
    return true;
}

bool SettingWiFiNetworkIPAddrButtonOnPress(ITUWidget* widget, char* param)
{
    char buf[16];
    char *p, *saveptr;

    strcpy(buf, theConfig.ipaddr);

    activeSettingWiFiNetworkText = settingWiFiNetworkIPAddrText;
    activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput1TextBox;
    p = strtok_r(buf, ".", &saveptr);
    if (p)
    {
        ituTextBoxSetString(settingWiFiNetworkIPInput1TextBox, p);
        activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput2TextBox;
        p = strtok_r(NULL, ".", &saveptr);
        if (p)
        {
            ituTextBoxSetString(settingWiFiNetworkIPInput2TextBox, p);
            activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput3TextBox;
            p = strtok_r(NULL, ".", &saveptr);
            if (p)
            {
                ituTextBoxSetString(settingWiFiNetworkIPInput3TextBox, p);
                activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput4TextBox;
                p = strtok_r(NULL, ".", &saveptr);
                if (p)
                {
                    ituTextBoxSetString(settingWiFiNetworkIPInput4TextBox, p);
                }
            }
        }
    }
    return true;
}

bool SettingWiFiNetworkNetmaskButtonOnPress(ITUWidget* widget, char* param)
{
    char buf[16];
    char *p, *saveptr;

    strcpy(buf, theConfig.netmask);

    activeSettingWiFiNetworkText = settingWiFiNetworkNetmaskText;
    activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput1TextBox;
    p = strtok_r(buf, ".", &saveptr);
    if (p)
    {
        ituTextBoxSetString(settingWiFiNetworkIPInput1TextBox, p);
        activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput2TextBox;
        p = strtok_r(NULL, ".", &saveptr);
        if (p)
        {
            ituTextBoxSetString(settingWiFiNetworkIPInput2TextBox, p);
            activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput3TextBox;
            p = strtok_r(NULL, ".", &saveptr);
            if (p)
            {
                ituTextBoxSetString(settingWiFiNetworkIPInput3TextBox, p);
                activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput4TextBox;
                p = strtok_r(NULL, ".", &saveptr);
                if (p)
                {
                    ituTextBoxSetString(settingWiFiNetworkIPInput4TextBox, p);
                }
            }
        }
    }
    return true;
}

bool SettingWiFiNetworkDNSButtonOnPress(ITUWidget* widget, char* param)
{
    char buf[16];
    char *p, *saveptr;

    strcpy(buf, theConfig.dns);

    activeSettingWiFiNetworkText = settingWiFiNetworkDNSText;
    activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput1TextBox;
    p = strtok_r(buf, ".", &saveptr);
    if (p)
    {
        ituTextBoxSetString(settingWiFiNetworkIPInput1TextBox, p);
        activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput2TextBox;
        p = strtok_r(NULL, ".", &saveptr);
        if (p)
        {
            ituTextBoxSetString(settingWiFiNetworkIPInput2TextBox, p);
            activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput3TextBox;
            p = strtok_r(NULL, ".", &saveptr);
            if (p)
            {
                ituTextBoxSetString(settingWiFiNetworkIPInput3TextBox, p);
                activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput4TextBox;
                p = strtok_r(NULL, ".", &saveptr);
                if (p)
                {
                    ituTextBoxSetString(settingWiFiNetworkIPInput4TextBox, p);
                }
            }
        }
    }
    return true;
}

bool SettingWiFiNetworkIPInputNumberButtonOnPress(ITUWidget* widget, char* param)
{
    ITUButton* btn = (ITUButton*) widget;
    char* input = param;
    char* str = ituTextGetString(activeSettingWiFiNetworkIPInputTextBox);
    int count = str ? strlen(str) : 0;

    if (!ituWidgetIsVisible(settingWiFiNetworkIPInputBackgroundButton))
        return false;

    if (count >= activeSettingWiFiNetworkIPInputTextBox->maxLen)
    {
        if (activeSettingWiFiNetworkIPInputTextBox == settingWiFiNetworkIPInput1TextBox)
            activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput2TextBox;
        else if (activeSettingWiFiNetworkIPInputTextBox == settingWiFiNetworkIPInput2TextBox)
            activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput3TextBox;
        else if (activeSettingWiFiNetworkIPInputTextBox == settingWiFiNetworkIPInput3TextBox)
            activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput4TextBox;
    }
    else if (str)
    {
        char buf[8];
        int value;

        strcpy(buf, str);
        strcat(buf, input);
        value = atoi(buf);
        if (value > 255)
        {
            if (activeSettingWiFiNetworkIPInputTextBox == settingWiFiNetworkIPInput1TextBox)
                activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput2TextBox;
            else if (activeSettingWiFiNetworkIPInputTextBox == settingWiFiNetworkIPInput2TextBox)
                activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput3TextBox;
            else if (activeSettingWiFiNetworkIPInputTextBox == settingWiFiNetworkIPInput3TextBox)
                activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput4TextBox;
            else if (activeSettingWiFiNetworkIPInputTextBox == settingWiFiNetworkIPInput4TextBox)
                return true;
        }
    }
    ituTextBoxInput(activeSettingWiFiNetworkIPInputTextBox, input);
    return true;
}

bool SettingWiFiNetworkIPInputBackButtonOnPress(ITUWidget* widget, char* param)
{
    ituTextSetString(activeSettingWiFiNetworkIPInputTextBox, NULL);

    if (activeSettingWiFiNetworkIPInputTextBox == settingWiFiNetworkIPInput4TextBox)
    {
        activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput3TextBox;
    }
    else if (activeSettingWiFiNetworkIPInputTextBox == settingWiFiNetworkIPInput3TextBox)
    {
        activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput2TextBox;
    }
    else if (activeSettingWiFiNetworkIPInputTextBox == settingWiFiNetworkIPInput2TextBox)
    {
        activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput1TextBox;
    }
    return true;
}

bool SettingWiFiNetworkIPInputClearButtonOnPress(ITUWidget* widget, char* param)
{
    if (!ituWidgetIsVisible(settingWiFiNetworkIPInputBackgroundButton))
        return false;

    ituTextBoxSetString(settingWiFiNetworkIPInput1TextBox, NULL);
    ituTextBoxSetString(settingWiFiNetworkIPInput2TextBox, NULL);
    ituTextBoxSetString(settingWiFiNetworkIPInput3TextBox, NULL);
    ituTextBoxSetString(settingWiFiNetworkIPInput4TextBox, NULL);
    activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput1TextBox;
    return true;
}

bool SettingWiFiNetworkIPInputConfirmButtonOnPress(ITUWidget* widget, char* param)
{
    if (!ituWidgetIsVisible(settingWiFiNetworkIPInputBackgroundButton))
        return false;

    if (activeSettingWiFiNetworkIPInputTextBox == settingWiFiNetworkIPInput1TextBox)
    {
        activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput2TextBox;
    }
    else if (activeSettingWiFiNetworkIPInputTextBox == settingWiFiNetworkIPInput2TextBox)
    {
        activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput3TextBox;
    }
    else if (activeSettingWiFiNetworkIPInputTextBox == settingWiFiNetworkIPInput3TextBox)
    {
        activeSettingWiFiNetworkIPInputTextBox = settingWiFiNetworkIPInput4TextBox;
    }
    else
    {
        char buf[16];

        strcpy(buf, ituTextGetString(settingWiFiNetworkIPInput1TextBox));
        strcat(buf, ".");
        strcat(buf, ituTextGetString(settingWiFiNetworkIPInput2TextBox));
        strcat(buf, ".");
        strcat(buf, ituTextGetString(settingWiFiNetworkIPInput3TextBox));
        strcat(buf, ".");
        strcat(buf, ituTextGetString(settingWiFiNetworkIPInput4TextBox));

        ituTextSetString(activeSettingWiFiNetworkText, buf);

        if (activeSettingWiFiNetworkText == settingWiFiNetworkIPAddrText)
        {
            strcpy(theConfig.ipaddr, buf);
        }
        else if (activeSettingWiFiNetworkText == settingWiFiNetworkNetmaskText)
        {
            strcpy(theConfig.netmask, buf);
        }
        else if (activeSettingWiFiNetworkText == settingWiFiNetworkDNSText)
        {
            strcpy(theConfig.dns, buf);
        }
        ituWidgetSetVisible(settingWiFiNetworkGrayBackground, false);
        ituWidgetSetVisible(settingWiFiNetworkIPInputBackgroundButton, false);
        ituWidgetEnable(mainTopBackground);
        ituWidgetEnable(settingBackground);
        ituWidgetEnable(settingWiFiNetworkBackground);
        return true;
    }
    return false;
}

bool SettingWiFiNetworkOnEnter(ITUWidget* widget, char* param)
{
    if (!settingWiFiNetworkIPAssignSprite)
    {
        settingWiFiNetworkIPAssignSprite = ituSceneFindWidget(&theScene, "settingWiFiNetworkIPAssignSprite");
        assert(settingWiFiNetworkIPAssignSprite);

        settingWiFiNetworkIPAddrText = ituSceneFindWidget(&theScene, "settingWiFiNetworkIPAddrText");
        assert(settingWiFiNetworkIPAddrText);

        settingWiFiNetworkNetmaskText = ituSceneFindWidget(&theScene, "settingWiFiNetworkNetmaskText");
        assert(settingWiFiNetworkNetmaskText);

        settingWiFiNetworkDNSText = ituSceneFindWidget(&theScene, "settingWiFiNetworkDNSText");
        assert(settingWiFiNetworkDNSText);

        settingWiFiNetworkMacText = ituSceneFindWidget(&theScene, "settingWiFiNetworkMacText");
        assert(settingWiFiNetworkMacText);

        settingWiFiNetworkGrayBackground = ituSceneFindWidget(&theScene, "settingWiFiNetworkGrayBackground");
        assert(settingWiFiNetworkGrayBackground);

        settingWiFiNetworkStaticIPRadioBox = ituSceneFindWidget(&theScene, "settingWiFiNetworkStaticIPRadioBox");
        assert(settingWiFiNetworkStaticIPRadioBox);

        settingWiFiNetworkDHCPRadioBox = ituSceneFindWidget(&theScene, "settingWiFiNetworkDHCPRadioBox");
        assert(settingWiFiNetworkDHCPRadioBox);

        settingBackground = ituSceneFindWidget(&theScene, "settingBackground");
        assert(settingBackground);

        settingWiFiNetworkBackground = ituSceneFindWidget(&theScene, "settingWiFiNetworkBackground");
        assert(settingWiFiNetworkBackground);

        settingWiFiNetworkIPAssignBackgroundButton = ituSceneFindWidget(&theScene, "settingWiFiNetworkIPAssignBackgroundButton");
        assert(settingWiFiNetworkIPAssignBackgroundButton);

        mainTopBackground = ituSceneFindWidget(&theScene, "mainTopBackground");
        assert(mainTopBackground);

        settingWiFiNetworkIPInputBackgroundButton = ituSceneFindWidget(&theScene, "settingWiFiNetworkIPInputBackgroundButton");
        assert(settingWiFiNetworkIPInputBackgroundButton);

        settingWiFiNetworkIPInput1TextBox = ituSceneFindWidget(&theScene, "settingWiFiNetworkIPInput1TextBox");
        assert(settingWiFiNetworkIPInput1TextBox);

        settingWiFiNetworkIPInput2TextBox = ituSceneFindWidget(&theScene, "settingWiFiNetworkIPInput2TextBox");
        assert(settingWiFiNetworkIPInput2TextBox);

        settingWiFiNetworkIPInput3TextBox = ituSceneFindWidget(&theScene, "settingWiFiNetworkIPInput3TextBox");
        assert(settingWiFiNetworkIPInput3TextBox);

        settingWiFiNetworkIPInput4TextBox = ituSceneFindWidget(&theScene, "settingWiFiNetworkIPInput4TextBox");
        assert(settingWiFiNetworkIPInput4TextBox);
    }

    // current settings
    if (theConfig.dhcp)
    {
        ituRadioBoxSetChecked(settingWiFiNetworkDHCPRadioBox, true);
        ituSpriteGoto(settingWiFiNetworkIPAssignSprite, 1);
    }
    else
    {
        ituRadioBoxSetChecked(settingWiFiNetworkStaticIPRadioBox, true);
        ituSpriteGoto(settingWiFiNetworkIPAssignSprite, 0);
    }
    settingWiFiNetworkIPAssignOld = theConfig.dhcp;

    ituTextSetString(settingWiFiNetworkIPAddrText, theConfig.ipaddr);
    strcpy(settingWiFiNetworkIPAddrOld, theConfig.ipaddr);

    ituTextSetString(settingWiFiNetworkNetmaskText, theConfig.netmask);
    strcpy(settingWiFiNetworkNetmaskOld, theConfig.netmask);

    ituTextSetString(settingWiFiNetworkDNSText, theConfig.dns);
    strcpy(settingWiFiNetworkDnsOld, theConfig.dns);

    // TODO: IMPLEMENT
    settingWiFiNetworkEthernetInfo.index = 0;
    ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_GET_INFO, &settingWiFiNetworkEthernetInfo);

    return true;
}

bool SettingWiFiNetworkOnLeave(ITUWidget* widget, char* param)
{
    bool toReboot = false;

    if (settingWiFiNetworkIPAssignOld != theConfig.dhcp ||
        strcmp(settingWiFiNetworkIPAddrOld, theConfig.ipaddr) ||
        strcmp(settingWiFiNetworkNetmaskOld, theConfig.netmask) ||
        strcmp(settingWiFiNetworkDnsOld, theConfig.dns))
    {
        ConfigSave();

        // reset network
        SceneQuit(QUIT_RESET_NETWORK);
    }
    return true;
}

void SettingWiFiNetworkReset(void)
{
    settingWiFiNetworkIPAssignSprite = NULL;
}

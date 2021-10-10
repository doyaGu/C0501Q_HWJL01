#include <sys/time.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "scene.h"
#include "airconditioner.h"

#define CONFIG_SAVE_DELAY   4
#define PEN_WIDTH           5

static ITULayer* settingLayer;
static ITULayer* settingTimeLayer;
static ITULayer* settingFuncLayer;
static ITUCheckBox* settingPowerCheckBox;
static ITUContainer* settingPowerContentContainer;
static ITUAnimation* settingTimeAnimation;
static ITUBackground* settingTimeBackground;
static ITUCheckBox* settingTimeCheckBox;
static ITUContainer* settingTimeContentContainer;
static ITUContainer* settingTimeBottomContainer;
static ITUAnimation* settingFuncAnimation;
static ITUBackground* settingFuncBackground;
static ITUCheckBox* settingFuncCheckBox;	
static ITUContainer* settingFuncContentContainer;	
static ITUContainer* settingFuncBottomContainer;		
static ITUBackground* settingSystemBackground;

static ITUSprite* settingFuncSprite;
static ITUCheckBox* settingFuncCoolVerticalCheckBox;
static ITUCheckBox* settingFuncCoolHorizontalCheckBox;
static ITUCheckBox* settingFuncCoolCleanCheckBox;
static ITUCheckBox* settingFuncCoolCoolingCheckBox;
static ITUCheckBox* settingFuncCoolSavingCheckBox;
static ITUCheckBox* settingFuncDryVerticalCheckBox;
static ITUCheckBox* settingFuncDryHorizontalCheckBox;
static ITUCheckBox* settingFuncDryCleanCheckBox;
static ITUCheckBox* settingFuncHeatVerticalCheckBox;
static ITUCheckBox* settingFuncHeatHorizontalCheckBox;
static ITUCheckBox* settingFuncHeatLowCheckBox;
static ITUCheckBox* settingFuncHeatHighCheckBox;
static ITUCheckBox* settingFuncHeatSavingCheckBox;
static ITUCheckBox* settingFuncFanVerticalCheckBox;
static ITUCheckBox* settingFuncFanHorizontalCheckBox;
static ITUCheckBox* settingFuncAutoVerticalCheckBox;
static ITUCheckBox* settingFuncAutoHorizontalCheckBox;
static ITUCheckBox* settingFuncAutoHeatCheckBox;
static ITUCheckBox* settingFuncAutoSavingCheckBox;
static ITUCheckBox* settingFuncPowerOffCleanCheckBox;

static ITUText* settingPowerHourText;
static ITUText* settingPowerVolumeText;
static ITUText* settingPowerTotalVolumeText;
static ITUBackground* settingPowerGraphBackground;
static ITUIcon* settingPowerGraphIcon;
static ITUText* settingPowerPriceText;	
static ITUText* settingPowerTotalPriceText;
static ITUText* settingPowerResetYearText;
static ITUText* settingPowerResetMonthText;
static ITUText* settingPowerResetDayText;
static ITUWheel* settingPowerPriceDollarWheel;
static ITUWheel* settingPowerPriceCentWheel;

static ITUCheckBox* settingTimePowerOnCheckBox;
static ITUCheckBox* settingTimePowerOffCheckBox;
static ITUCheckBox* settingTimeMonCheckBox;
static ITUCheckBox* settingTimeTueCheckBox;
static ITUCheckBox* settingTimeWedCheckBox;
static ITUCheckBox* settingTimeThuCheckBox;
static ITUCheckBox* settingTimeFriCheckBox;
static ITUCheckBox* settingTimeSatCheckBox;
static ITUCheckBox* settingTimeSunCheckBox;
static ITUWheel* settingTimePowerOnHourWheel;
static ITUWheel* settingTimePowerOnMinuteWheel;
static ITUWheel* settingTimePowerOffHourWheel;
static ITUWheel* settingTimePowerOffMinuteWheel;

static ITUCheckBox* settingSystemSoundCheckBox;
static ITUProgressBar* settingSystemBrightnessProgressBar;
static ITUTrackBar* settingSystemBrightnessTrackBar;
static ITUSprite* settingSystemScreenSaverSprite;
static ITUDigitalClock* settingSystemDateDigitalClock;
static ITUCheckBox* settingSystemLightCheckBox;
static ITUText* settingSystemVersionText;
static ITUBackground* settingSystemScreenSaverBackground;
static ITUBackground* settingSystemScreenSaverDialogBackground;
static ITURadioBox* settingSystemScreenSaver1RadioBox;
static ITURadioBox* settingSystemScreenSaver2RadioBox;
static ITURadioBox* settingSystemScreenSaver3RadioBox;
static ITURadioBox* settingSystemScreenSaver4RadioBox;
static ITURadioBox* settingSystemScreenSaver5RadioBox;
static ITUBackground* settingSystemDateBackground;
static ITUWheel* settingSystemDateYearWheel;
static ITUWheel* settingSystemDateMonthWheel;
static ITUWheel* settingSystemDateDayWheel;
static ITUBackground* settingSystemTimeBackground;
static ITUWheel* settingSystemTimeHourWheel;
static ITUWheel* settingSystemTimeMinuteWheel;
static ITUBackground* settingSystemDateHintBackground;
static ITULayer* settingDialogLayer;
static ITUIcon* barTimeIcon;
static ITUSprite* mainFoldSprite;
static ITUSprite* mainUnFoldSprite;

extern bool mainPowerOn;

static int settingConfigSaveDelay, settingTimeBottomContainerY, settingFuncBottomContainerY, settingYear, settingMonth,settingDay, settingWeek, settingPowerVolumes[7];
static float settingPowerHour, settingPowerVolume, settingPowerVolumeUnit, settingPowerTotalVolume, settingPowerTotalPrice;

bool SettingPowerCheckBoxOnPress(ITUWidget* widget, char* param)
{
    if (ituCheckBoxIsChecked(settingPowerCheckBox))
    {
        ExternalEvent ev;

        if (ituCheckBoxIsChecked(settingTimeCheckBox))
        {
            settingTimeAnimation->child = NULL;
            ituAnimationPlay(settingTimeAnimation, 0);
        }
        else
        {
            settingTimeAnimation->child = NULL;
            ituAnimationPlay(settingTimeAnimation, 0);
            settingFuncAnimation->child = NULL;
            ituAnimationPlay(settingFuncAnimation, 0);
        }
        ituCheckBoxSetChecked(settingTimeCheckBox, false);
        ituCheckBoxSetChecked(settingFuncCheckBox, false);
        ituWidgetSetVisible(settingPowerContentContainer, true);
        AudioPlaySound(SOUND_SETTING_POWER);

        ev.type = EXTERNAL_POWER_ENTER;
        ExternalSend(&ev);
    }
    else
    {
        ituCheckBoxSetChecked(settingPowerCheckBox, true);
    }
    return true;
}

bool SettingTimeCheckBoxOnPress(ITUWidget* widget, char* param)
{
    if (ituCheckBoxIsChecked(settingTimeCheckBox))
    {
        if (ituCheckBoxIsChecked(settingPowerCheckBox))
        {
            ituAnimationReversePlay(settingTimeAnimation, 1);
        }
        else
        {
            settingFuncAnimation->child = NULL;
            ituAnimationPlay(settingFuncAnimation, 0);
        }
        ituCheckBoxSetChecked(settingPowerCheckBox, false);
        ituCheckBoxSetChecked(settingFuncCheckBox, false);
        ituWidgetSetVisible(settingTimeContentContainer, true);
        AudioPlaySound(SOUND_SETTING_TIME);
    }
    else
    {
        ituCheckBoxSetChecked(settingTimeCheckBox, true);
    }
    return true;
}

bool SettingTimeAnimationOnStop(ITUWidget* widget, char* param)
{
    itcTreeRotateFront((ITCTree*)settingTimeAnimation);

    if (settingTimeAnimation->animationFlags & ITU_ANIM_REVERSE)
    {
        ituWidgetSetY(settingTimeBackground, 0);
        ituWidgetSetY(settingTimeBottomContainer, settingTimeBottomContainerY);
        ituWidgetSetVisible(settingPowerContentContainer, false);
    }
    else
    {
        ituWidgetSetY(settingTimeBackground, settingTimeBottomContainerY);
        ituWidgetSetY(settingTimeBottomContainer, 0);
        ituWidgetSetVisible(settingTimeContentContainer, false);
    }
    settingTimeAnimation->child = (ITUWidget*)settingTimeBackground;
    return true;
}

bool SettingFuncCheckBoxOnPress(ITUWidget* widget, char* param)
{
    if (ituCheckBoxIsChecked(settingFuncCheckBox))
    {
        if (ituCheckBoxIsChecked(settingPowerCheckBox))
        {
            ituAnimationReversePlay(settingTimeAnimation, 1);
            ituAnimationReversePlay(settingFuncAnimation, 1);
        }
        else
        {
            ituAnimationReversePlay(settingFuncAnimation, 1);
        }
        ituCheckBoxSetChecked(settingPowerCheckBox, false);
        ituCheckBoxSetChecked(settingTimeCheckBox, false);
        ituWidgetSetVisible(settingFuncContentContainer, true);
        AudioPlaySound(SOUND_SETTING_FUNCTION);
    }
    else
    {
        ituCheckBoxSetChecked(settingFuncCheckBox, true);
    }
    return true;
}

bool SettingFuncAnimationOnStop(ITUWidget* widget, char* param)
{
    itcTreeRotateFront((ITCTree*)settingFuncAnimation);

    if (settingFuncAnimation->animationFlags & ITU_ANIM_REVERSE)
    {
        ituWidgetSetY(settingFuncBackground, 0);
        ituWidgetSetY(settingFuncBottomContainer, settingFuncBottomContainerY);
        ituWidgetSetVisible(settingTimeContentContainer, false);
    }
    else
    {
        ituWidgetSetY(settingFuncBackground, settingFuncBottomContainerY);
        ituWidgetSetY(settingFuncBottomContainer, 0);
        ituWidgetSetVisible(settingFuncContentContainer, false);
    }
    settingFuncAnimation->child = (ITUWidget*)settingFuncBackground;
    return true;
}

static void SettingPowerGraphBackgroundDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    int destx, desty, i, dx, week;
    ITURectangle prevClip;
    ITUBackground* bg = (ITUBackground*) widget;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    ITUColor* color = &widget->color;
    ITCTree* child = widget->tree.child;
    assert(bg);
    assert(dest);

    widget->tree.child = NULL;
    ituBackgroundDraw(widget, dest, x, y, alpha);
    widget->tree.child =  child;

    dx = ituWidgetGetWidth(settingPowerGraphBackground) / 6;
    week = settingWeek;

    destx = rect->x + x;
    desty = rect->y + y + rect->height;

    ituWidgetSetClipping(widget, dest, x, y, &prevClip);
    
    for (i = 0; i < week; i++)
    {
        int dy0 = settingPowerVolumes[i];
        int dy1 = settingPowerVolumes[i + 1];
            
        ituDrawLine(dest, destx, desty - dy0, destx + dx, desty - dy1, color, PEN_WIDTH);

        destx += dx;
    }
    ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);

    ituWidgetDrawImpl(widget, dest, x, y, alpha);
}

bool SettingPowerPriceButtonOnPress(ITUWidget* widget, char* param)
{
    ituWheelGoto(settingPowerPriceDollarWheel, theConfig.power_price_dollar);
    ituWheelGoto(settingPowerPriceCentWheel, theConfig.power_price_cent);
    return true;
}

bool SettingPowerPriceConfirmButtonOnPress(ITUWidget* widget, char* param)
{
    int dollar = settingPowerPriceDollarWheel->focusIndex;
    int cent = settingPowerPriceCentWheel->focusIndex;

    if (dollar != theConfig.power_price_dollar || cent != theConfig.power_price_cent)
    {
        ExternalEvent ev;
        char buf[32];

        theConfig.power_price_dollar = dollar;
        theConfig.power_price_cent = cent;

        ev.type = EXTERNAL_POWER_PRICE;
        ev.arg1 = dollar;
        ev.arg2 = cent;

        ExternalSend(&ev);

        sprintf(buf, "%d.%02d", theConfig.power_price_dollar, theConfig.power_price_cent);
        ituTextSetString(settingPowerPriceText, buf);

        settingConfigSaveDelay = CONFIG_SAVE_DELAY;
    }
    return true;
}

bool SettingPowerResetConfirmButton(ITUWidget* widget, char* param)
{
    char buf[32];
    ExternalEvent ev;

    ev.type = EXTERNAL_POWER_RESET;

    ExternalSend(&ev);

    if (settingYear != theConfig.power_reset_year || settingMonth != theConfig.power_reset_month || settingDay != theConfig.power_reset_day)
    {
        theConfig.power_reset_year = settingYear;
        theConfig.power_reset_month = settingMonth;
        theConfig.power_reset_day = settingDay;
        settingConfigSaveDelay = CONFIG_SAVE_DELAY;

        sprintf(buf, "%04d", theConfig.power_reset_year);
        ituTextSetString(settingPowerResetYearText, buf);

        sprintf(buf, "%02d", theConfig.power_reset_month);
        ituTextSetString(settingPowerResetMonthText, buf);

        sprintf(buf, "%02d", theConfig.power_reset_day);
        ituTextSetString(settingPowerResetDayText, buf);
    }

    settingPowerVolumes[settingWeek] = 0;

    switch (settingWeek)
    {
    case 0:
        if (theConfig.power_vol_sun != 0.0f)
        {
            theConfig.power_vol_sun = 0.0f;
            settingConfigSaveDelay = CONFIG_SAVE_DELAY;
        }
        break;

    case 1:
        if (theConfig.power_vol_mon != 0.0f)
        {
            theConfig.power_vol_mon = 0.0f;
            settingConfigSaveDelay = CONFIG_SAVE_DELAY;
        }
        break;

    case 2:
        if (theConfig.power_vol_tue != 0.0f)
        {
            theConfig.power_vol_tue = 0.0f;
            settingConfigSaveDelay = CONFIG_SAVE_DELAY;
        }
        break;

    case 3:
        if (theConfig.power_vol_wed != 0.0f)
        {
            theConfig.power_vol_wed = 0.0f;
            settingConfigSaveDelay = CONFIG_SAVE_DELAY;
        }
        break;

    case 4:
        if (theConfig.power_vol_thu != 0.0f)
        {
            theConfig.power_vol_thu = 0.0f;
            settingConfigSaveDelay = CONFIG_SAVE_DELAY;
        }
        break;

    case 5:
        if (theConfig.power_vol_fri != 0.0f)
        {
            theConfig.power_vol_fri = 0.0f;
            settingConfigSaveDelay = CONFIG_SAVE_DELAY;
        }
        break;

    case 6:
        if (theConfig.power_vol_sat != 0.0f)
        {
            theConfig.power_vol_sat = 0.0f;
            settingConfigSaveDelay = CONFIG_SAVE_DELAY;
        }
        break;
    }

    if (theConfig.power_vol_before != 0.0f)
    {
        theConfig.power_vol_before = 0.0f;
        settingConfigSaveDelay = CONFIG_SAVE_DELAY;
    }
    return true;
}

bool SettingTimePowerOnCheckBoxOnPress(ITUWidget* widget, char* param)
{
    ITUCheckBox* checkbox = (ITUCheckBox*) widget;

    if (ituCheckBoxIsChecked(checkbox))
    {
        ituCheckBoxSetChecked(checkbox, false);
        ituWheelGoto(settingTimePowerOnHourWheel, theConfig.power_on_hour);
        ituWheelGoto(settingTimePowerOnMinuteWheel, theConfig.power_on_min);
        return true;
    }
    else
    {
        theConfig.power_on_enable = false;

        if (!theConfig.power_off_enable)
            ituWidgetSetVisible(barTimeIcon, false);

        return false;
    }
}

bool SettingTimePowerOffCheckBoxOnPress(ITUWidget* widget, char* param)
{
    ITUCheckBox* checkbox = (ITUCheckBox*) widget;

    if (ituCheckBoxIsChecked(checkbox))
    {
        ituCheckBoxSetChecked(checkbox, false);
        ituWheelGoto(settingTimePowerOffHourWheel, theConfig.power_off_hour);
        ituWheelGoto(settingTimePowerOffMinuteWheel, theConfig.power_off_min);
        return true;
    }
    else
    {
        theConfig.power_off_enable = false;

        if (!theConfig.power_on_enable)
            ituWidgetSetVisible(barTimeIcon, false);

        return false;
    }
}

bool SettingTimePowerOnConfirmButtonOnPress(ITUWidget* widget, char* param)
{
    int hour, minute;
    char buf[128];

    hour = settingTimePowerOnHourWheel->focusIndex;
    minute = settingTimePowerOnMinuteWheel->focusIndex;

    if (!theConfig.power_on_enable || hour != theConfig.power_on_hour || minute != theConfig.power_on_min)
    {
        ExternalEvent ev;

        if (theConfig.power_off_enable && hour == theConfig.power_off_hour && minute == theConfig.power_off_min)
            return true;

        theConfig.power_on_enable  = true;
        theConfig.power_on_hour = hour;
        theConfig.power_on_min = minute;
        settingConfigSaveDelay = CONFIG_SAVE_DELAY;

        sprintf(buf, "%02d:%02d %s", theConfig.power_on_hour, theConfig.power_on_min, StringGetPowerOn());
        ituButtonSetString(settingTimePowerOnCheckBox, buf);
        ituCheckBoxSetChecked(settingTimePowerOnCheckBox, true);
        ituWidgetSetVisible(barTimeIcon, true);

        ev.type = EXTERNAL_TIME_POWER_ON;
        ev.arg1 = hour;
        ev.arg2 = minute;

        ExternalSend(&ev);
    }
    return true;
}

bool SettingTimePowerOffConfirmButtonOnPress(ITUWidget* widget, char* param)
{
    int hour, minute;
    char buf[128];

    hour = settingTimePowerOffHourWheel->focusIndex;
    minute = settingTimePowerOffMinuteWheel->focusIndex;

    if (!theConfig.power_off_enable || hour != theConfig.power_off_hour || minute != theConfig.power_off_min)
    {
        ExternalEvent ev;

        if (theConfig.power_on_enable && hour == theConfig.power_on_hour && minute == theConfig.power_on_min)
            return true;

        theConfig.power_off_enable  = true;
        theConfig.power_off_hour = hour;
        theConfig.power_off_min = minute;
        settingConfigSaveDelay = CONFIG_SAVE_DELAY;

        sprintf(buf, "%02d:%02d %s", theConfig.power_off_hour, theConfig.power_off_min, StringGetPowerOff());
        ituButtonSetString(settingTimePowerOffCheckBox, buf);
        ituCheckBoxSetChecked(settingTimePowerOffCheckBox, true);

        ev.type = EXTERNAL_TIME_POWER_OFF;
        ev.arg1 = hour;
        ev.arg2 = minute;

        ExternalSend(&ev);
    }
    return true;
}

bool SettingTimeWeekCheckBoxOnPress(ITUWidget* widget, char* param)
{
    ITUCheckBox* checkbox = (ITUCheckBox*) widget;
    int value = ituCheckBoxIsChecked(checkbox) ? 1 : 0;

    if (checkbox == settingTimeMonCheckBox)
    {
        if (value != theConfig.power_mon_enable)
        {
            theConfig.power_mon_enable = value;
            settingConfigSaveDelay = CONFIG_SAVE_DELAY;
        }
    }
    else if (checkbox == settingTimeTueCheckBox)
    {
        if (value != theConfig.power_tue_enable)
        {
            theConfig.power_tue_enable = value;
            settingConfigSaveDelay = CONFIG_SAVE_DELAY;
        }
    }
    else if (checkbox == settingTimeWedCheckBox)
    {
        if (value != theConfig.power_wed_enable)
        {
            theConfig.power_wed_enable = value;
            settingConfigSaveDelay = CONFIG_SAVE_DELAY;
        }
    }
    else if (checkbox == settingTimeThuCheckBox)
    {
        if (value != theConfig.power_thu_enable)
        {
            theConfig.power_thu_enable = value;
            settingConfigSaveDelay = CONFIG_SAVE_DELAY;
        }
    }
    else if (checkbox == settingTimeFriCheckBox)
    {
        if (value != theConfig.power_fri_enable)
        {
            theConfig.power_fri_enable = value;
            settingConfigSaveDelay = CONFIG_SAVE_DELAY;
        }
    }
    else if (checkbox == settingTimeSatCheckBox)
    {
        if (value != theConfig.power_sat_enable)
        {
            theConfig.power_sat_enable = value;
            settingConfigSaveDelay = CONFIG_SAVE_DELAY;
        }
    }
    else if (checkbox == settingTimeSunCheckBox)
    {
        if (value != theConfig.power_sun_enable)
        {
            theConfig.power_sun_enable = value;
            settingConfigSaveDelay = CONFIG_SAVE_DELAY;
        }
    }

    if (settingConfigSaveDelay == CONFIG_SAVE_DELAY)
    {
        ExternalEvent ev;
        int arg, flag;

        arg = 0;
        flag = 0x1;

        ev.type = EXTERNAL_TIME_WEEK;

        if (theConfig.power_mon_enable)
            arg |= flag;

        flag <<= 1;

        if (theConfig.power_tue_enable)
            arg |= flag;

        flag <<= 1;

        if (theConfig.power_wed_enable)
            arg |= flag;

        flag <<= 1;

        if (theConfig.power_thu_enable)
            arg |= flag;

        flag <<= 1;

        if (theConfig.power_fri_enable)
            arg |= flag;

        flag <<= 1;

        if (theConfig.power_sat_enable)
            arg |= flag;

        flag <<= 1;

        if (theConfig.power_sun_enable)
            arg |= flag;

        ev.arg1 = arg;

        ExternalSend(&ev);
    }
    return true;
}

bool SettingFuncVerticalCheckBoxOnPress(ITUWidget* widget, char* param)
{
    ITUCheckBox* checkbox = (ITUCheckBox*) widget;
    ExternalEvent ev;
    bool checked = ituCheckBoxIsChecked(checkbox);

    ev.type = EXTERNAL_FUNC_VERTICAL;
    ev.arg1 = checked ? 1 : 0;

    ExternalSend(&ev);

    ituCheckBoxSetChecked(settingFuncCoolVerticalCheckBox, checked);
    ituCheckBoxSetChecked(settingFuncDryVerticalCheckBox, checked);
    ituCheckBoxSetChecked(settingFuncHeatVerticalCheckBox, checked);
    ituCheckBoxSetChecked(settingFuncFanVerticalCheckBox, checked);
    ituCheckBoxSetChecked(settingFuncAutoVerticalCheckBox, checked);

    return true;
}

bool SettingFuncHorizontalCheckBoxOnPress(ITUWidget* widget, char* param)
{
    ITUCheckBox* checkbox = (ITUCheckBox*) widget;
    ExternalEvent ev;
    bool checked = ituCheckBoxIsChecked(checkbox);

    ev.type = EXTERNAL_FUNC_HORIZONTAL;
    ev.arg1 = checked ? 1 : 0;

    ExternalSend(&ev);

    ituCheckBoxSetChecked(settingFuncCoolHorizontalCheckBox, checked);
    ituCheckBoxSetChecked(settingFuncDryHorizontalCheckBox, checked);
    ituCheckBoxSetChecked(settingFuncHeatHorizontalCheckBox, checked);
    ituCheckBoxSetChecked(settingFuncFanHorizontalCheckBox, checked);
    ituCheckBoxSetChecked(settingFuncAutoHorizontalCheckBox, checked);

    return true;
}

bool SettingFuncCleanCheckBoxOnPress(ITUWidget* widget, char* param)
{
    ITUCheckBox* checkbox = (ITUCheckBox*) widget;
    ExternalEvent ev;
    bool checked = ituCheckBoxIsChecked(checkbox);

    ev.type = EXTERNAL_FUNC_CLEAN;
    ev.arg1 = checked ? 1 : 0;

    ExternalSend(&ev);

    ituCheckBoxSetChecked(settingFuncCoolCleanCheckBox, checked);
    ituCheckBoxSetChecked(settingFuncDryCleanCheckBox, checked);

    return true;
}

bool SettingFuncCoolingCheckBoxOnPress(ITUWidget* widget, char* param)
{
    ITUCheckBox* checkbox = (ITUCheckBox*) widget;
    ExternalEvent ev;

    ev.type = EXTERNAL_FUNC_COOLING;
    ev.arg1 = ituCheckBoxIsChecked(checkbox) ? 1 : 0;

    ExternalSend(&ev);

    return true;
}

bool SettingFuncSavingCheckBoxOnPress(ITUWidget* widget, char* param)
{
    ITUCheckBox* checkbox = (ITUCheckBox*) widget;
    ExternalEvent ev;
    bool checked = ituCheckBoxIsChecked(checkbox);

    ev.type = EXTERNAL_FUNC_SAVING;
    ev.arg1 = checked ? 1 : 0;

    ExternalSend(&ev);

    ituCheckBoxSetChecked(settingFuncCoolSavingCheckBox, checked);
    ituCheckBoxSetChecked(settingFuncHeatSavingCheckBox, checked);
    ituCheckBoxSetChecked(settingFuncAutoSavingCheckBox, checked);

    return true;
}

bool SettingFuncHeatHighCheckBoxOnPress(ITUWidget* widget, char* param)
{
    ITUCheckBox* checkbox = (ITUCheckBox*) widget;
    ExternalEvent ev;

    ev.type = EXTERNAL_FUNC_HEAT_HIGH;
    ev.arg1 = ituCheckBoxIsChecked(checkbox) ? 1 : 0;

    ExternalSend(&ev);

    return true;
}

bool SettingFuncHeatLowCheckBoxOnPress(ITUWidget* widget, char* param)
{
    ITUCheckBox* checkbox = (ITUCheckBox*) widget;
    ExternalEvent ev;
    bool checked = ituCheckBoxIsChecked(checkbox);

    ev.type = EXTERNAL_FUNC_HEAT_LOW;
    ev.arg1 = checked ? 1 : 0;

    ExternalSend(&ev);

    ituCheckBoxSetChecked(settingFuncHeatLowCheckBox, checked);
    ituCheckBoxSetChecked(settingFuncAutoHeatCheckBox, checked);

    return true;
}

bool SettingFuncPowerOffCleanCheckBoxOnPress(ITUWidget* widget, char* param)
{
    ITUCheckBox* checkbox = (ITUCheckBox*) widget;
    ExternalEvent ev;

    ev.type = EXTERNAL_FUNC_STANDBY_CLEAN;
    ev.arg1 = ituCheckBoxIsChecked(checkbox) ? 1 : 0;

    ExternalSend(&ev);

    return true;
}

bool SettingFuncSystemButtonOnPress(ITUWidget* widget, char* param)
{
    AudioPlaySound(SOUND_SETTING_SYSTEM);
    return true;
}

bool SettingSystemSoundCheckBoxOnPress(ITUWidget* widget, char* param)
{
    int value = ituCheckBoxIsChecked(settingSystemSoundCheckBox) ? 1 : 0;
    
    if (value != theConfig.sound_enable)
    {
        if (value)
            AudioUnMute();
        else
            AudioMute();

        theConfig.sound_enable = value;
        settingConfigSaveDelay = CONFIG_SAVE_DELAY;
    }
    return true;
}

bool SettingSystemBrightnessTrackBarOnChanged(ITUWidget* widget, char* param)
{
    int value = settingSystemBrightnessTrackBar->value;

    if (value != theConfig.brightness)
    {
        ScreenSetBrightness(value);

        theConfig.brightness = value;
        settingConfigSaveDelay = CONFIG_SAVE_DELAY;
    }
    return true;
}

bool SettingSystemScreenSaverRadioBoxOnPress(ITUWidget* widget, char* param)
{
    int value;

    if (ituRadioBoxIsChecked(settingSystemScreenSaver1RadioBox))
    {
        ituSpriteGoto(settingSystemScreenSaverSprite, SCREENSAVER_15SEC);
        value = 15;
    }
    else if (ituRadioBoxIsChecked(settingSystemScreenSaver2RadioBox))
    {
        ituSpriteGoto(settingSystemScreenSaverSprite, SCREENSAVER_30SEC);
        value = 30;
    }
    else if (ituRadioBoxIsChecked(settingSystemScreenSaver3RadioBox))
    {
        ituSpriteGoto(settingSystemScreenSaverSprite, SCREENSAVER_1MIN);
        value = 1 * 60;
    }
    else if (ituRadioBoxIsChecked(settingSystemScreenSaver4RadioBox))
    {
        ituSpriteGoto(settingSystemScreenSaverSprite, SCREENSAVER_5MIN);
        value = 5 * 60;
    }
    else
    {
        ituSpriteGoto(settingSystemScreenSaverSprite, SCREENSAVER_NONE);
        value = 0;
    }

    if (value != theConfig.screensaver_time)
    {
        theConfig.screensaver_time = value;
        settingConfigSaveDelay = CONFIG_SAVE_DELAY;
    }
    return true;
}

bool SettingSystemDateButtonOnPress(ITUWidget* widget, char* param)
{
    int value;
    
    value = settingSystemDateDigitalClock->year + 1900 - 1970;
    ituWheelGoto(settingSystemDateYearWheel, value);

    value = settingSystemDateDigitalClock->month;
    ituWheelGoto(settingSystemDateMonthWheel, value);

    value = settingSystemDateDigitalClock->day - 1;
    ituWheelGoto(settingSystemDateDayWheel, value);

    return true;
}

bool SettingSystemTimeButtonOnPress(ITUWidget* widget, char* param)
{
    int value;
    
    value = settingSystemDateDigitalClock->hour;
    ituWheelGoto(settingSystemTimeHourWheel, value);

    value = settingSystemDateDigitalClock->minute;
    ituWheelGoto(settingSystemTimeMinuteWheel, value);

    return true;
}

bool SettingSystemDateWheelOnChanged(ITUWidget* widget, char* param)
{
    struct timeval tv;
    struct tm *tm, mytime, mytime2;

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);

    memcpy(&mytime, tm, sizeof (struct tm));

    mytime.tm_mday = settingSystemDateDayWheel->focusIndex + 1;
    mytime.tm_mon = settingSystemDateMonthWheel->focusIndex;
    mytime.tm_year = settingSystemDateYearWheel->focusIndex - 1900 + 1970;

    memcpy(&mytime2, &mytime, sizeof (struct tm));

    tv.tv_sec = mktime(&mytime);
    //printf("y:%d m:%d d:%d s:%d\n", mytime.tm_year, mytime.tm_mon, mytime.tm_mday, tv.tv_sec);

    if (mytime.tm_mon != mytime2.tm_mon || mytime.tm_mday != mytime2.tm_mday)
    {
        ituWheelGoto(settingSystemDateMonthWheel, mytime.tm_mon);
        ituWheelGoto(settingSystemDateDayWheel, mytime.tm_mday - 1);
    }
    return true;
}

bool SettingSystemDateConfirmButtonOnPress(ITUWidget* widget, char* param)
{
    struct timeval tv;
    struct tm *tm, mytime;

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);

    memcpy(&mytime, tm, sizeof (struct tm));

    if (mytime.tm_mday != settingSystemDateDayWheel->focusIndex + 1 ||
        mytime.tm_mon != settingSystemDateMonthWheel->focusIndex ||
        mytime.tm_year != settingSystemDateYearWheel->focusIndex - 1900 + 1970)
    {
        ituWidgetSetVisible(settingSystemDateHintBackground, true);

        mytime.tm_mday = settingSystemDateDayWheel->focusIndex + 1;
        mytime.tm_mon = settingSystemDateMonthWheel->focusIndex;
        mytime.tm_year = settingSystemDateYearWheel->focusIndex - 1900 + 1970;

        tv.tv_sec = mktime(&mytime);
        tv.tv_usec = 0;

    #ifndef _WIN32
        settimeofday(&tv, NULL);
    #endif
    }
    return true;
}

bool SettingSystemTimeConfirmButtonOnPress(ITUWidget* widget, char* param)
{
    struct timeval tv;
    struct tm *tm, mytime;

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);

    memcpy(&mytime, tm, sizeof (struct tm));

    if (mytime.tm_hour != settingSystemTimeHourWheel->focusIndex || mytime.tm_min != settingSystemTimeMinuteWheel->focusIndex)
    {
        mytime.tm_hour = settingSystemTimeHourWheel->focusIndex;
        mytime.tm_min = settingSystemTimeMinuteWheel->focusIndex;
        mytime.tm_sec = 0;

        tv.tv_sec = mktime(&mytime);
        tv.tv_usec = 0;

    #ifndef _WIN32
        settimeofday(&tv, NULL);
    #endif
    }
    return true;
}

bool SettingSystemLightCheckBoxOnPress(ITUWidget* widget, char* param)
{
    ExternalEvent ev;

    theConfig.light_enable = ituCheckBoxIsChecked(settingSystemLightCheckBox) ? 1 : 0;

    ev.type = EXTERNAL_LIGHT;
    ev.arg1 = theConfig.light_enable;
    ExternalSend(&ev);

    settingConfigSaveDelay = CONFIG_SAVE_DELAY;
    return true;
}

void SettingOnSecond(void)
{
    struct timeval tv;
    struct tm* tm;

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);

    if (settingWeek != tm->tm_wday)
    {
        int pointX, pointY;

        if (!settingPowerGraphBackground)
        {
            settingPowerGraphBackground = ituSceneFindWidget(&theScene, "settingPowerGraphBackground");
            assert(settingPowerGraphBackground);

            settingPowerGraphIcon = ituSceneFindWidget(&theScene, "settingPowerGraphIcon");
            assert(settingPowerGraphIcon);
        }

        settingYear = tm->tm_year + 1900;
        settingMonth = tm->tm_mon + 1;
        settingDay = tm->tm_mday;
        settingWeek = tm->tm_wday;

        theConfig.power_vol_before = settingPowerTotalVolume;
        settingConfigSaveDelay = CONFIG_SAVE_DELAY;

        settingPowerVolumes[settingWeek] = 0;
        switch (settingWeek)
        {
        case 0:
            theConfig.power_vol_sun = 0;
            break;

        case 1:
            theConfig.power_vol_mon = 0;
            break;

        case 2:
            theConfig.power_vol_tue = 0;
            break;

        case 3:
            theConfig.power_vol_wed = 0;
            break;

        case 4:
            theConfig.power_vol_thu = 0;
            break;

        case 5:
            theConfig.power_vol_fri = 0;
            break;

        case 6:
            theConfig.power_vol_sat = 0;
            break;
        }

        pointX = ituWidgetGetWidth(settingPowerGraphBackground) / 6;
        pointX *= settingWeek;
        pointX -= ituWidgetGetWidth(settingPowerGraphIcon) / 2;
        pointY = ituWidgetGetHeight(settingPowerGraphBackground);
        pointY -= ituWidgetGetHeight(settingPowerGraphIcon) / 2;
        pointY -= settingPowerVolumes[settingWeek];
        ituWidgetSetPosition(settingPowerGraphIcon, pointX, pointY);
    }

    if (settingConfigSaveDelay > 0)
    {
        if (--settingConfigSaveDelay <= 0)
        {
            settingConfigSaveDelay = 0;

            ConfigSave();
        }
    }
}

static void SettingProcessExternalPowerTime(ExternalEvent* ev)
{
    char buf[32];
    assert(ev);

    if (!settingPowerHourText)
    {
        settingPowerHourText = ituSceneFindWidget(&theScene, "settingPowerHourText");
        assert(settingPowerHourText);
    }
    settingPowerHour = *(float*)&ev->arg1;

    sprintf(buf, "%.1f", settingPowerHour);
    ituTextSetString(settingPowerHourText, buf);
}

static void MainProcessExternalPowerVolume(ExternalEvent* ev)
{
    char buf[32];
    assert(ev);

    if (!settingPowerVolumeText)
    {
        struct timeval tv;
        struct tm* tm;

        settingPowerVolumeText = ituSceneFindWidget(&theScene, "settingPowerVolumeText");
        assert(settingPowerVolumeText);

        gettimeofday(&tv, NULL);
        tm = localtime(&tv.tv_sec);

        settingYear = tm->tm_year + 1900;
        settingMonth = tm->tm_mon + 1;
        settingDay = tm->tm_mday;
        settingWeek = tm->tm_wday;
    }

    settingPowerVolume = *(float*)&ev->arg1;

    sprintf(buf, "%.1f", settingPowerVolume);
    ituTextSetString(settingPowerVolumeText, buf);
}

static void MainProcessExternalPowerTotalVolume(ExternalEvent* ev)
{
    char buf[32];
    float vol;
    int pointY;
    assert(ev);

    if (!settingPowerTotalVolumeText)
    {
        settingPowerTotalVolumeText = ituSceneFindWidget(&theScene, "settingPowerTotalVolumeText");
        assert(settingPowerTotalVolumeText);

        settingPowerGraphBackground = ituSceneFindWidget(&theScene, "settingPowerGraphBackground");
        assert(settingPowerGraphBackground);

        settingPowerGraphIcon = ituSceneFindWidget(&theScene, "settingPowerGraphIcon");
        assert(settingPowerGraphIcon);
    }
    settingPowerTotalVolume = *(float*)&ev->arg1;

    sprintf(buf, "%.1f", settingPowerTotalVolume);
    ituTextSetString(settingPowerTotalVolumeText, buf);

    vol = settingPowerTotalVolume - theConfig.power_vol_before;

    switch (settingWeek)
    {
    case 0:
        if (vol != theConfig.power_vol_sun)
        {
            theConfig.power_vol_sun = vol;
            settingConfigSaveDelay = CONFIG_SAVE_DELAY;
            settingPowerVolumes[0] = (int)roundf(theConfig.power_vol_sun * settingPowerVolumeUnit);
        }
        break;

    case 1:
        if (vol != theConfig.power_vol_mon)
        {
            theConfig.power_vol_mon = vol;
            settingConfigSaveDelay = CONFIG_SAVE_DELAY;
            settingPowerVolumes[1] = (int)roundf(theConfig.power_vol_mon * settingPowerVolumeUnit);
        }
        break;

    case 2:
        if (vol != theConfig.power_vol_tue)
        {
            theConfig.power_vol_tue = vol;
            settingConfigSaveDelay = CONFIG_SAVE_DELAY;
            settingPowerVolumes[2] = (int)roundf(theConfig.power_vol_tue * settingPowerVolumeUnit);
        }
        break;

    case 3:
        if (vol != theConfig.power_vol_wed)
        {
            theConfig.power_vol_wed = vol;
            settingConfigSaveDelay = CONFIG_SAVE_DELAY;
            settingPowerVolumes[3] = (int)roundf(theConfig.power_vol_wed * settingPowerVolumeUnit);
        }
        break;

    case 4:
        if (vol != theConfig.power_vol_thu)
        {
            theConfig.power_vol_thu = vol;
            settingConfigSaveDelay = CONFIG_SAVE_DELAY;
            settingPowerVolumes[4] = (int)roundf(theConfig.power_vol_thu * settingPowerVolumeUnit);
        }
        break;

    case 5:
        if (vol != theConfig.power_vol_fri)
        {
            theConfig.power_vol_fri = vol;
            settingConfigSaveDelay = CONFIG_SAVE_DELAY;
            settingPowerVolumes[5] = (int)roundf(theConfig.power_vol_fri * settingPowerVolumeUnit);
        }
        break;

    case 6:
        if (vol != theConfig.power_vol_sat)
        {
            theConfig.power_vol_sat = vol;
            settingConfigSaveDelay = CONFIG_SAVE_DELAY;
            settingPowerVolumes[6] = (int)roundf(theConfig.power_vol_sat * settingPowerVolumeUnit);
        }
        break;
    }

    pointY = ituWidgetGetHeight(settingPowerGraphBackground);
    pointY -= ituWidgetGetHeight(settingPowerGraphIcon) / 2;
    pointY -= settingPowerVolumes[settingWeek];
    ituWidgetSetY(settingPowerGraphIcon, pointY);
}

static void MainProcessExternalPowerTotalPrice(ExternalEvent* ev)
{
    char buf[32];
    assert(ev);

    if (!settingPowerTotalPriceText)
    {
        settingPowerTotalPriceText = ituSceneFindWidget(&theScene, "settingPowerTotalPriceText");
        assert(settingPowerTotalPriceText);
    }
    settingPowerTotalPrice = *(float*)&ev->arg1;

    sprintf(buf, "%.1f", settingPowerTotalPrice);
    ituTextSetString(settingPowerTotalPriceText, buf);
}

void SettingProcessExternalEvent(ExternalEvent* ev)
{
    assert(ev);
    switch (ev->type)
    {
    case EXTERNAL_POWER_TIME:
        printf("EXTERNAL_POWER_TIME: %d\n", ev->arg1);
        SettingProcessExternalPowerTime(ev);
        break;

    case EXTERNAL_POWER_VOLUME:
        printf("EXTERNAL_POWER_VOLUME: %d\n", ev->arg1);
        MainProcessExternalPowerVolume(ev);
        break;

    case EXTERNAL_POWER_TOTAL_VOLUME:
        printf("EXTERNAL_POWER_TOTAL_VOLUME: %d\n", ev->arg1);
        MainProcessExternalPowerTotalVolume(ev);
        break;

    case EXTERNAL_POWER_TOTAL_PRICE:
        printf("EXTERNAL_POWER_TOTAL_PRICE: %d\n", ev->arg1);
        MainProcessExternalPowerTotalPrice(ev);
        break;
    }
}

void SettingPowerOn(void)
{
    if (!settingTimePowerOnCheckBox)
    {
        settingTimePowerOnCheckBox = ituSceneFindWidget(&theScene, "settingTimePowerOnCheckBox");
        assert(settingTimePowerOnCheckBox);

        settingTimePowerOffCheckBox = ituSceneFindWidget(&theScene, "settingTimePowerOffCheckBox");
        assert(settingTimePowerOffCheckBox);
    }

    if (!theConfig.power_mon_enable && 
        !theConfig.power_tue_enable && 
        !theConfig.power_wed_enable && 
        !theConfig.power_thu_enable && 
        !theConfig.power_fri_enable && 
        !theConfig.power_sat_enable && 
        !theConfig.power_sun_enable)
    {
        ituCheckBoxSetChecked(settingTimePowerOnCheckBox, false);
        ituCheckBoxSetChecked(settingTimePowerOffCheckBox, false);
    }
}

void SettingPowerOff(void)
{
    if (!settingTimePowerOnCheckBox)
    {
        settingTimePowerOnCheckBox = ituSceneFindWidget(&theScene, "settingTimePowerOnCheckBox");
        assert(settingTimePowerOnCheckBox);

        settingTimePowerOffCheckBox = ituSceneFindWidget(&theScene, "settingTimePowerOffCheckBox");
        assert(settingTimePowerOffCheckBox);
    }

    if (!theConfig.power_mon_enable && 
        !theConfig.power_tue_enable && 
        !theConfig.power_wed_enable && 
        !theConfig.power_thu_enable && 
        !theConfig.power_fri_enable && 
        !theConfig.power_sat_enable && 
        !theConfig.power_sun_enable)
    {
        ituCheckBoxSetChecked(settingTimePowerOnCheckBox, false);
        ituCheckBoxSetChecked(settingTimePowerOffCheckBox, false);
    }
    settingPowerHour = settingPowerVolume = 0.0f;
}

bool SettingOnEnter(ITUWidget* widget, char* param)
{
    char buf[128];
    int pointX, pointY;
    ITCTree* node;

    if (!settingLayer)
    {
        struct timeval tv;
        struct tm* tm;

        settingLayer = ituSceneFindWidget(&theScene, "settingLayer");
        assert(settingLayer);

        settingTimeLayer = ituSceneFindWidget(&theScene, "settingTimeLayer");
        assert(settingTimeLayer);

        settingFuncLayer = ituSceneFindWidget(&theScene, "settingFuncLayer");
        assert(settingFuncLayer);

        settingPowerCheckBox = ituSceneFindWidget(&theScene, "settingPowerCheckBox");
        assert(settingPowerCheckBox);

        settingPowerContentContainer = ituSceneFindWidget(&theScene, "settingPowerContentContainer");
        assert(settingPowerContentContainer);

        settingTimeAnimation = ituSceneFindWidget(&theScene, "settingTimeAnimation");
        assert(settingTimeAnimation);

        settingTimeBackground = ituSceneFindWidget(&theScene, "settingTimeBackground");
        assert(settingTimeBackground);

        settingTimeCheckBox = ituSceneFindWidget(&theScene, "settingTimeCheckBox");
        assert(settingTimeCheckBox);

        settingTimeContentContainer = ituSceneFindWidget(&theScene, "settingTimeContentContainer");
        assert(settingTimeContentContainer);

        settingTimeBottomContainer = ituSceneFindWidget(&theScene, "settingTimeBottomContainer");
        assert(settingTimeBottomContainer);

        settingFuncAnimation = ituSceneFindWidget(&theScene, "settingFuncAnimation");
        assert(settingFuncAnimation);

        settingFuncBackground = ituSceneFindWidget(&theScene, "settingFuncBackground");
        assert(settingFuncBackground);

        settingFuncCheckBox = ituSceneFindWidget(&theScene, "settingFuncCheckBox");
        assert(settingFuncCheckBox);

        settingFuncContentContainer = ituSceneFindWidget(&theScene, "settingFuncContentContainer");
        assert(settingFuncContentContainer);

        settingFuncBottomContainer = ituSceneFindWidget(&theScene, "settingFuncBottomContainer");
        assert(settingFuncBottomContainer);

        settingSystemBackground = ituSceneFindWidget(&theScene, "settingSystemBackground");
        assert(settingSystemBackground);

        settingFuncSprite = ituSceneFindWidget(&theScene, "settingFuncSprite");
        assert(settingFuncSprite);

        settingFuncCoolVerticalCheckBox = ituSceneFindWidget(&theScene, "settingFuncCoolVerticalCheckBox");
        assert(settingFuncCoolVerticalCheckBox);

        settingFuncCoolHorizontalCheckBox = ituSceneFindWidget(&theScene, "settingFuncCoolHorizontalCheckBox");
        assert(settingFuncCoolHorizontalCheckBox);

        settingFuncCoolCleanCheckBox = ituSceneFindWidget(&theScene, "settingFuncCoolCleanCheckBox");
        assert(settingFuncCoolCleanCheckBox);

        settingFuncCoolCoolingCheckBox = ituSceneFindWidget(&theScene, "settingFuncCoolCoolingCheckBox");
        assert(settingFuncCoolCoolingCheckBox);

        settingFuncCoolSavingCheckBox = ituSceneFindWidget(&theScene, "settingFuncCoolSavingCheckBox");
        assert(settingFuncCoolSavingCheckBox);

        settingFuncDryVerticalCheckBox = ituSceneFindWidget(&theScene, "settingFuncDryVerticalCheckBox");
        assert(settingFuncDryVerticalCheckBox);

        settingFuncDryHorizontalCheckBox = ituSceneFindWidget(&theScene, "settingFuncDryHorizontalCheckBox");
        assert(settingFuncDryHorizontalCheckBox);

        settingFuncDryCleanCheckBox = ituSceneFindWidget(&theScene, "settingFuncDryCleanCheckBox");
        assert(settingFuncDryCleanCheckBox);

        settingFuncHeatVerticalCheckBox = ituSceneFindWidget(&theScene, "settingFuncHeatVerticalCheckBox");
        assert(settingFuncHeatVerticalCheckBox);

        settingFuncHeatHorizontalCheckBox = ituSceneFindWidget(&theScene, "settingFuncHeatHorizontalCheckBox");
        assert(settingFuncHeatHorizontalCheckBox);

        settingFuncHeatLowCheckBox = ituSceneFindWidget(&theScene, "settingFuncHeatLowCheckBox");
        assert(settingFuncHeatLowCheckBox);

        settingFuncHeatHighCheckBox = ituSceneFindWidget(&theScene, "settingFuncHeatHighCheckBox");
        assert(settingFuncHeatHighCheckBox);

        settingFuncHeatSavingCheckBox = ituSceneFindWidget(&theScene, "settingFuncHeatSavingCheckBox");
        assert(settingFuncHeatSavingCheckBox);

        settingFuncFanVerticalCheckBox = ituSceneFindWidget(&theScene, "settingFuncFanVerticalCheckBox");
        assert(settingFuncFanVerticalCheckBox);

        settingFuncFanHorizontalCheckBox = ituSceneFindWidget(&theScene, "settingFuncFanHorizontalCheckBox");
        assert(settingFuncFanHorizontalCheckBox);

        settingFuncAutoVerticalCheckBox = ituSceneFindWidget(&theScene, "settingFuncAutoVerticalCheckBox");
        assert(settingFuncAutoVerticalCheckBox);

        settingFuncAutoHorizontalCheckBox = ituSceneFindWidget(&theScene, "settingFuncAutoHorizontalCheckBox");
        assert(settingFuncAutoHorizontalCheckBox);

        settingFuncAutoHeatCheckBox = ituSceneFindWidget(&theScene, "settingFuncAutoHeatCheckBox");
        assert(settingFuncAutoHeatCheckBox);

        settingFuncAutoSavingCheckBox = ituSceneFindWidget(&theScene, "settingFuncAutoSavingCheckBox");
        assert(settingFuncAutoSavingCheckBox);

        settingFuncPowerOffCleanCheckBox = ituSceneFindWidget(&theScene, "settingFuncPowerOffCleanCheckBox");
        assert(settingFuncPowerOffCleanCheckBox);

        settingPowerHourText = ituSceneFindWidget(&theScene, "settingPowerHourText");
        assert(settingPowerHourText);

        settingPowerVolumeText = ituSceneFindWidget(&theScene, "settingPowerVolumeText");
        assert(settingPowerVolumeText);

        settingPowerTotalVolumeText = ituSceneFindWidget(&theScene, "settingPowerTotalVolumeText");
        assert(settingPowerTotalVolumeText);

        settingPowerGraphBackground = ituSceneFindWidget(&theScene, "settingPowerGraphBackground");
        assert(settingPowerGraphBackground);
        ituWidgetSetDraw(settingPowerGraphBackground, SettingPowerGraphBackgroundDraw);

        settingPowerGraphIcon = ituSceneFindWidget(&theScene, "settingPowerGraphIcon");
        assert(settingPowerGraphIcon);

        settingPowerPriceText = ituSceneFindWidget(&theScene, "settingPowerPriceText");
        assert(settingPowerPriceText);

        settingPowerTotalPriceText = ituSceneFindWidget(&theScene, "settingPowerTotalPriceText");
        assert(settingPowerTotalPriceText);

        settingPowerResetYearText = ituSceneFindWidget(&theScene, "settingPowerResetYearText");
        assert(settingPowerResetYearText);

        settingPowerResetMonthText = ituSceneFindWidget(&theScene, "settingPowerResetMonthText");
        assert(settingPowerResetMonthText);

        settingPowerResetDayText = ituSceneFindWidget(&theScene, "settingPowerResetDayText");
        assert(settingPowerResetDayText);
        
        settingPowerPriceDollarWheel = ituSceneFindWidget(&theScene, "settingPowerPriceDollarWheel");
        assert(settingPowerPriceDollarWheel);

        settingPowerPriceCentWheel = ituSceneFindWidget(&theScene, "settingPowerPriceCentWheel");
        assert(settingPowerPriceCentWheel);

        settingTimePowerOnCheckBox = ituSceneFindWidget(&theScene, "settingTimePowerOnCheckBox");
        assert(settingTimePowerOnCheckBox);

        settingTimePowerOffCheckBox = ituSceneFindWidget(&theScene, "settingTimePowerOffCheckBox");
        assert(settingTimePowerOffCheckBox);

        settingTimeMonCheckBox = ituSceneFindWidget(&theScene, "settingTimeMonCheckBox");
        assert(settingTimeMonCheckBox);

        settingTimeTueCheckBox = ituSceneFindWidget(&theScene, "settingTimeTueCheckBox");
        assert(settingTimeTueCheckBox);

        settingTimeWedCheckBox = ituSceneFindWidget(&theScene, "settingTimeWedCheckBox");
        assert(settingTimeWedCheckBox);

        settingTimeThuCheckBox = ituSceneFindWidget(&theScene, "settingTimeThuCheckBox");
        assert(settingTimeThuCheckBox);

        settingTimeFriCheckBox = ituSceneFindWidget(&theScene, "settingTimeFriCheckBox");
        assert(settingTimeFriCheckBox);

        settingTimeSatCheckBox = ituSceneFindWidget(&theScene, "settingTimeSatCheckBox");
        assert(settingTimeSatCheckBox);

        settingTimeSunCheckBox = ituSceneFindWidget(&theScene, "settingTimeSunCheckBox");
        assert(settingTimeSunCheckBox);

        settingTimePowerOnHourWheel = ituSceneFindWidget(&theScene, "settingTimePowerOnHourWheel");
        assert(settingTimePowerOnHourWheel);

        settingTimePowerOnMinuteWheel = ituSceneFindWidget(&theScene, "settingTimePowerOnMinuteWheel");
        assert(settingTimePowerOnMinuteWheel);

        settingTimePowerOffHourWheel = ituSceneFindWidget(&theScene, "settingTimePowerOffHourWheel");
        assert(settingTimePowerOffHourWheel);

        settingTimePowerOffMinuteWheel = ituSceneFindWidget(&theScene, "settingTimePowerOffMinuteWheel");
        assert(settingTimePowerOffMinuteWheel);

        settingSystemSoundCheckBox = ituSceneFindWidget(&theScene, "settingSystemSoundCheckBox");
        assert(settingSystemSoundCheckBox);

        settingSystemBrightnessProgressBar = ituSceneFindWidget(&theScene, "settingSystemBrightnessProgressBar");
        assert(settingSystemBrightnessProgressBar);

        settingSystemBrightnessTrackBar = ituSceneFindWidget(&theScene, "settingSystemBrightnessTrackBar");
        assert(settingSystemBrightnessTrackBar);

        settingSystemScreenSaverSprite = ituSceneFindWidget(&theScene, "settingSystemScreenSaverSprite");
        assert(settingSystemScreenSaverSprite);

        settingSystemDateDigitalClock = ituSceneFindWidget(&theScene, "settingSystemDateDigitalClock");
        assert(settingSystemDateDigitalClock);

        settingSystemLightCheckBox = ituSceneFindWidget(&theScene, "settingSystemLightCheckBox");
        assert(settingSystemLightCheckBox);

        settingSystemVersionText = ituSceneFindWidget(&theScene, "settingSystemVersionText");
        assert(settingSystemVersionText);

        settingSystemScreenSaverBackground = ituSceneFindWidget(&theScene, "settingSystemScreenSaverBackground");
        assert(settingSystemScreenSaverBackground);

        settingSystemScreenSaverDialogBackground = ituSceneFindWidget(&theScene, "settingSystemScreenSaverDialogBackground");
        assert(settingSystemScreenSaverDialogBackground);

        settingSystemScreenSaver1RadioBox = ituSceneFindWidget(&theScene, "settingSystemScreenSaver1RadioBox");
        assert(settingSystemScreenSaver1RadioBox);

        settingSystemScreenSaver2RadioBox = ituSceneFindWidget(&theScene, "settingSystemScreenSaver2RadioBox");
        assert(settingSystemScreenSaver2RadioBox);

        settingSystemScreenSaver3RadioBox = ituSceneFindWidget(&theScene, "settingSystemScreenSaver3RadioBox");
        assert(settingSystemScreenSaver3RadioBox);

        settingSystemScreenSaver4RadioBox = ituSceneFindWidget(&theScene, "settingSystemScreenSaver4RadioBox");
        assert(settingSystemScreenSaver4RadioBox);

        settingSystemScreenSaver5RadioBox = ituSceneFindWidget(&theScene, "settingSystemScreenSaver5RadioBox");
        assert(settingSystemScreenSaver5RadioBox);

        settingSystemDateBackground = ituSceneFindWidget(&theScene, "settingSystemDateBackground");
        assert(settingSystemDateBackground);

        settingSystemDateYearWheel = ituSceneFindWidget(&theScene, "settingSystemDateYearWheel");
        assert(settingSystemDateYearWheel);

        settingSystemDateMonthWheel = ituSceneFindWidget(&theScene, "settingSystemDateMonthWheel");
        assert(settingSystemDateMonthWheel);

        settingSystemDateDayWheel = ituSceneFindWidget(&theScene, "settingSystemDateDayWheel");
        assert(settingSystemDateDayWheel);

        settingSystemTimeBackground = ituSceneFindWidget(&theScene, "settingSystemTimeBackground");
        assert(settingSystemTimeBackground);

        settingSystemTimeHourWheel = ituSceneFindWidget(&theScene, "settingSystemTimeHourWheel");
        assert(settingSystemTimeHourWheel);

        settingSystemTimeMinuteWheel = ituSceneFindWidget(&theScene, "settingSystemTimeMinuteWheel");
        assert(settingSystemTimeMinuteWheel);

        settingSystemDateHintBackground = ituSceneFindWidget(&theScene, "settingSystemDateHintBackground");
        assert(settingSystemDateHintBackground);

        settingDialogLayer = ituSceneFindWidget(&theScene, "settingDialogLayer");
        assert(settingDialogLayer);

        barTimeIcon = ituSceneFindWidget(&theScene, "barTimeIcon");
        assert(barTimeIcon);

        mainFoldSprite = ituSceneFindWidget(&theScene, "mainFoldSprite");
        assert(mainFoldSprite);

        mainUnFoldSprite = ituSceneFindWidget(&theScene, "mainUnFoldSprite");
        assert(mainUnFoldSprite);

        settingTimeBottomContainerY = ituWidgetGetY(settingTimeBottomContainer);
        settingFuncBottomContainerY = ituWidgetGetY(settingFuncBottomContainer);

        settingPowerVolumeUnit = ituWidgetGetHeight(settingPowerGraphBackground) / 30.0f;

        gettimeofday(&tv, NULL);
        tm = localtime(&tv.tv_sec);

        settingYear = tm->tm_year + 1900;
        settingMonth = tm->tm_mon + 1;
        settingDay = tm->tm_mday;
        settingWeek = tm->tm_wday;
    }

    if (mainFoldSprite->playing || mainUnFoldSprite->playing)
        return false;

    ituWidgetEnable(settingLayer);
    ituWidgetEnable(settingTimeLayer);
    ituWidgetEnable(settingFuncLayer);

    for (node = ((ITCTree *)settingDialogLayer)->child; node; node = node->sibling)
    {
        ituWidgetHide(node, ITU_EFFECT_NONE, 0);
    }

    ituCheckBoxSetChecked(settingPowerCheckBox, false);
    ituCheckBoxSetChecked(settingTimeCheckBox, false);
    ituCheckBoxSetChecked(settingFuncCheckBox, true);
    ituWidgetSetVisible(settingPowerContentContainer, false);
    ituWidgetSetVisible(settingTimeContentContainer, false);
    ituWidgetSetVisible(settingFuncContentContainer, true);
    ituWidgetEnable(settingSystemBackground);
    ituWidgetHide(settingSystemBackground, ITU_EFFECT_NONE, 0);

    if (ituWidgetGetY(settingTimeBackground) == settingTimeBottomContainerY)
    {
        itcTreeRotateFront((ITCTree*)settingTimeAnimation);
        ituWidgetSetY(settingTimeBackground, 0);
        ituWidgetSetY(settingTimeBottomContainer, settingTimeBottomContainerY);
        settingTimeAnimation->child = NULL;
        ituAnimationReset(settingTimeAnimation);
    }

    if (ituWidgetGetY(settingFuncBackground) == settingFuncBottomContainerY)
    {
        itcTreeRotateFront((ITCTree*)settingFuncAnimation);
        ituWidgetSetY(settingFuncBackground, 0);
        ituWidgetSetY(settingFuncBottomContainer, settingFuncBottomContainerY);
        settingFuncAnimation->child = NULL;
        ituAnimationReset(settingFuncAnimation);
    }

    if (mainPowerOn)
    {
        ituSpriteGoto(settingFuncSprite, theConfig.mode);
    }
    else
    {
        ituSpriteGoto(settingFuncSprite, EXTERNAL_MODE_MAX);
    }

    sprintf(buf, "%.1f", settingPowerHour);
    ituTextSetString(settingPowerHourText, buf);

    sprintf(buf, "%.1f", settingPowerVolume);
    ituTextSetString(settingPowerVolumeText, buf);

    sprintf(buf, "%.1f", settingPowerTotalVolume);
    ituTextSetString(settingPowerTotalVolumeText, buf);

    settingPowerVolumes[0] = (int)roundf(theConfig.power_vol_sun * settingPowerVolumeUnit);
    settingPowerVolumes[1] = (int)roundf(theConfig.power_vol_mon * settingPowerVolumeUnit);
    settingPowerVolumes[2] = (int)roundf(theConfig.power_vol_tue * settingPowerVolumeUnit);
    settingPowerVolumes[3] = (int)roundf(theConfig.power_vol_wed * settingPowerVolumeUnit);
    settingPowerVolumes[4] = (int)roundf(theConfig.power_vol_thu * settingPowerVolumeUnit);
    settingPowerVolumes[5] = (int)roundf(theConfig.power_vol_fri * settingPowerVolumeUnit);
    settingPowerVolumes[6] = (int)roundf(theConfig.power_vol_sat * settingPowerVolumeUnit);

    pointX = ituWidgetGetWidth(settingPowerGraphBackground) / 6;
    pointX *= settingWeek;
    pointX -= ituWidgetGetWidth(settingPowerGraphIcon) / 2;
    pointY = ituWidgetGetHeight(settingPowerGraphBackground);
    pointY -= ituWidgetGetHeight(settingPowerGraphIcon) / 2;
    pointY -= settingPowerVolumes[settingWeek];
    ituWidgetSetPosition(settingPowerGraphIcon, pointX, pointY);

    sprintf(buf, "%d.%02d", theConfig.power_price_dollar, theConfig.power_price_cent);
    ituTextSetString(settingPowerPriceText, buf);

    if (theConfig.power_reset_year > 0)
    {
        sprintf(buf, "%04d", theConfig.power_reset_year);
        ituTextSetString(settingPowerResetYearText, buf);
    }
    else
    {
        ituTextSetString(settingPowerResetYearText, "----");
    }

    if (theConfig.power_reset_month > 0)
    {
        sprintf(buf, "%02d", theConfig.power_reset_month);
        ituTextSetString(settingPowerResetMonthText, buf);
    }
    else
    {
        ituTextSetString(settingPowerResetMonthText, "--");
    }

    if (theConfig.power_reset_day > 0)
    {
        sprintf(buf, "%02d", theConfig.power_reset_day);
        ituTextSetString(settingPowerResetDayText, buf);
    }
    else
    {
        ituTextSetString(settingPowerResetDayText, "--");
    }

    ituCheckBoxSetChecked(settingTimePowerOnCheckBox, theConfig.power_on_enable ? true : false);

    sprintf(buf, "%02d:%02d %s", theConfig.power_on_hour, theConfig.power_on_min, StringGetPowerOn());
    ituButtonSetString(settingTimePowerOnCheckBox, buf);

    ituCheckBoxSetChecked(settingTimePowerOffCheckBox, theConfig.power_off_enable ? true : false);

    sprintf(buf, "%02d:%02d %s", theConfig.power_off_hour, theConfig.power_off_min, StringGetPowerOff());
    ituButtonSetString(settingTimePowerOffCheckBox, buf);

    ituCheckBoxSetChecked(settingTimeMonCheckBox, theConfig.power_mon_enable ? true : false);
    ituCheckBoxSetChecked(settingTimeTueCheckBox, theConfig.power_tue_enable ? true : false);
    ituCheckBoxSetChecked(settingTimeWedCheckBox, theConfig.power_wed_enable ? true : false);
    ituCheckBoxSetChecked(settingTimeThuCheckBox, theConfig.power_thu_enable ? true : false);
    ituCheckBoxSetChecked(settingTimeFriCheckBox, theConfig.power_fri_enable ? true : false);
    ituCheckBoxSetChecked(settingTimeSatCheckBox, theConfig.power_sat_enable ? true : false);
    ituCheckBoxSetChecked(settingTimeSunCheckBox, theConfig.power_sun_enable ? true : false);

    ituCheckBoxSetChecked(settingSystemSoundCheckBox, theConfig.sound_enable ? true : false);

    ituProgressBarSetValue(settingSystemBrightnessProgressBar, theConfig.brightness);
    ituTrackBarSetValue(settingSystemBrightnessTrackBar, theConfig.brightness);

    ituWidgetHide(settingSystemScreenSaverBackground, ITU_EFFECT_NONE, 0);
    ituWidgetHide(settingSystemScreenSaverDialogBackground, ITU_EFFECT_NONE, 0);

    switch (theConfig.screensaver_time)
    {
    case 15:
        ituSpriteGoto(settingSystemScreenSaverSprite, SCREENSAVER_15SEC);
        ituRadioBoxSetChecked(settingSystemScreenSaver1RadioBox, true);
        break;

    case 30:
        ituSpriteGoto(settingSystemScreenSaverSprite, SCREENSAVER_30SEC);
        ituRadioBoxSetChecked(settingSystemScreenSaver2RadioBox, true);
        break;

    case 1 * 60:
        ituSpriteGoto(settingSystemScreenSaverSprite, SCREENSAVER_1MIN);
        ituRadioBoxSetChecked(settingSystemScreenSaver3RadioBox, true);
        break;

    case 5 * 60:
        ituSpriteGoto(settingSystemScreenSaverSprite, SCREENSAVER_5MIN);
        ituRadioBoxSetChecked(settingSystemScreenSaver4RadioBox, true);
        break;

    default:
        ituSpriteGoto(settingSystemScreenSaverSprite, SCREENSAVER_NONE);
        ituRadioBoxSetChecked(settingSystemScreenSaver5RadioBox, true);
        break;
    }

    ituWidgetHide(settingSystemDateBackground, ITU_EFFECT_NONE, 0);
    ituWidgetHide(settingSystemTimeBackground, ITU_EFFECT_NONE, 0);

    ituCheckBoxSetChecked(settingSystemLightCheckBox, theConfig.light_enable ? true : false);

    ituTextSetString(settingSystemVersionText, CFG_VERSION_MAJOR_STR "." CFG_VERSION_MINOR_STR "." CFG_VERSION_PATCH_STR "." CFG_VERSION_CUSTOM_STR "." CFG_VERSION_TWEAK_STR);

    settingConfigSaveDelay = 0;

    return true;
}

void SettingOnLeave(bool before)
{
    if (before)
    {
        ITCTree* node;

        for (node = ((ITCTree *)settingDialogLayer)->child; node; node = node->sibling)
        {
            ituWidgetHide(node, ITU_EFFECT_NONE, 0);
        }
    }
    else
    {
        if (settingConfigSaveDelay > 0)
        {
            settingConfigSaveDelay = 0;

            ConfigSave();
        }
    }
}

void SettingReset(void)
{
    settingLayer = NULL;
}

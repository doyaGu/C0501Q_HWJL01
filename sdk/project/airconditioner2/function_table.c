#include "ite/itu.h"

// Logo
extern bool LogoOnEnter(ITUWidget* widget, char* param);

// Main
extern bool MainOnEnter(ITUWidget* widget, char* param);
extern bool MainOnLeave(ITUWidget* widget, char* param);
extern bool MainOnTimer(ITUWidget* widget, char* param);
extern bool MainPureOffButtonOnPress(ITUWidget* widget, char* param);
extern bool MainPureOnButtonOnPress(ITUWidget* widget, char* param);
extern bool MainTemperatureWheelOnChanged(ITUWidget* widget, char* param);
extern bool MainModePopupRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool MainPowerTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool MainAutoWindButtonOnPress(ITUWidget* widget, char* param);
extern bool MainAnimationOnStop(ITUWidget* widget, char* param);
extern bool MainAnimationOnStop(ITUWidget* widget, char* param);
extern bool MainBackgroundButtonOnLongPress(ITUWidget* widget, char* param);
extern bool MainLockBackgroundButtonOnPress(ITUWidget* widget, char* param);

// Setting
extern bool SettingOnEnter(ITUWidget* widget, char* param);
extern bool SettingPowerCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingPowerPriceButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingPowerPriceConfirmButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingPowerResetConfirmButton(ITUWidget* widget, char* param);
extern bool SettingTimeCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingTimeAnimationOnStop(ITUWidget* widget, char* param);
extern bool SettingTimePowerOnCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingTimePowerOffCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingTimePowerOnConfirmButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingTimePowerOffConfirmButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingTimeWeekCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingFuncCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingFuncAnimationOnStop(ITUWidget* widget, char* param);

extern bool SettingFuncVerticalCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingFuncHorizontalCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingFuncCleanCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingFuncCoolingCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingFuncSavingCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingFuncHeatHighCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingFuncHeatLowCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingFuncPowerOffCleanCheckBoxOnPress(ITUWidget* widget, char* param);

extern bool SettingFuncSystemButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingSystemSoundCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingSystemBrightnessTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool SettingSystemLightCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingSystemScreenSaverRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingSystemDateWheelOnChanged(ITUWidget* widget, char* param);
extern bool SettingSystemDateButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingSystemTimeButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingSystemDateConfirmButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingSystemTimeConfirmButtonOnPress(ITUWidget* widget, char* param);

// VideoPlayer
extern bool VideoPlayerOnEnter(ITUWidget* widget, char* param);
extern bool VideoPlayerOnLeave(ITUWidget* widget, char* param);
extern bool VideoPlayerOnTimer(ITUWidget* widget, char* param);
extern bool VideoPlayerBackgroundButtonOnPress(ITUWidget* widget, char* param);
extern bool VideoPlayerPlayCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool VideoPlayerTrackBarOnChanged(ITUWidget* widget, char* param);

// Engineer
bool EngineerOnEnter(ITUWidget* widget, char* param);
bool EngineerOnLeave(ITUWidget* widget, char* param);
bool EngineerOnTimer(ITUWidget* widget, char* param);
bool EngineerWarnTableListBoxOnLoad(ITUWidget* widget, char* param);

// EngineerPower
bool EngineerPowerOnEnter(ITUWidget* widget, char* param);
bool EngineerPowerOnLeave(ITUWidget* widget, char* param);
bool EngineerPowerOnTimer(ITUWidget* widget, char* param);
bool EngineerPowerMemoryOnButtonOnPress(ITUWidget* widget, char* param);
bool EngineerPowerMemoryOffButtonOnPress(ITUWidget* widget, char* param);

// EngineerMachine
bool EngineerMachineOnEnter(ITUWidget* widget, char* param);
bool EngineerMachineOnLeave(ITUWidget* widget, char* param);
bool EngineerMachineOnTimer(ITUWidget* widget, char* param);

// EngineerTry
bool EngineerTryOnEnter(ITUWidget* widget, char* param);
bool EngineerTryOnLeave(ITUWidget* widget, char* param);
bool EngineerTryOnTimer(ITUWidget* widget, char* param);
bool EngineerTryWarnTableListBoxOnLoad(ITUWidget* widget, char* param);

ITUActionFunction actionFunctions[] =
{
    // Logo
    "LogoOnEnter", LogoOnEnter,

    // Main
    "MainOnEnter", MainOnEnter,
    "MainOnLeave", MainOnLeave,
    "MainOnTimer", MainOnTimer,
    "MainPureOffButtonOnPress", MainPureOffButtonOnPress,
    "MainPureOnButtonOnPress", MainPureOnButtonOnPress,
    "MainTemperatureWheelOnChanged", MainTemperatureWheelOnChanged,
    "MainModePopupRadioBoxOnPress", MainModePopupRadioBoxOnPress,
    "MainPowerTrackBarOnChanged", MainPowerTrackBarOnChanged,
    "MainAutoWindButtonOnPress", MainAutoWindButtonOnPress,
    "MainAnimationOnStop", MainAnimationOnStop,
    "MainBackgroundButtonOnLongPress", MainBackgroundButtonOnLongPress,
    "MainLockBackgroundButtonOnPress", MainLockBackgroundButtonOnPress,

    // Setting
    "SettingOnEnter", SettingOnEnter,
    "SettingPowerCheckBoxOnPress", SettingPowerCheckBoxOnPress,
    "SettingPowerPriceButtonOnPress", SettingPowerPriceButtonOnPress,
    "SettingPowerPriceConfirmButtonOnPress", SettingPowerPriceConfirmButtonOnPress,
    "SettingPowerResetConfirmButton", SettingPowerResetConfirmButton,
    "SettingTimeCheckBoxOnPress", SettingTimeCheckBoxOnPress,
    "SettingTimeAnimationOnStop", SettingTimeAnimationOnStop,
    "SettingTimePowerOnCheckBoxOnPress", SettingTimePowerOnCheckBoxOnPress,
    "SettingTimePowerOffCheckBoxOnPress", SettingTimePowerOffCheckBoxOnPress,
    "SettingTimePowerOnConfirmButtonOnPress", SettingTimePowerOnConfirmButtonOnPress,
    "SettingTimePowerOffConfirmButtonOnPress", SettingTimePowerOffConfirmButtonOnPress,
    "SettingTimeWeekCheckBoxOnPress", SettingTimeWeekCheckBoxOnPress,
    "SettingFuncCheckBoxOnPress", SettingFuncCheckBoxOnPress,
    "SettingFuncAnimationOnStop", SettingFuncAnimationOnStop,
    "SettingFuncVerticalCheckBoxOnPress", SettingFuncVerticalCheckBoxOnPress,
    "SettingFuncHorizontalCheckBoxOnPress", SettingFuncHorizontalCheckBoxOnPress,
    "SettingFuncCleanCheckBoxOnPress", SettingFuncCleanCheckBoxOnPress,
    "SettingFuncCoolingCheckBoxOnPress", SettingFuncCoolingCheckBoxOnPress,
    "SettingFuncSavingCheckBoxOnPress", SettingFuncSavingCheckBoxOnPress,
    "SettingFuncHeatHighCheckBoxOnPress", SettingFuncHeatHighCheckBoxOnPress,
    "SettingFuncHeatLowCheckBoxOnPress", SettingFuncHeatLowCheckBoxOnPress,
    "SettingFuncPowerOffCleanCheckBoxOnPress", SettingFuncPowerOffCleanCheckBoxOnPress,
    "SettingFuncSystemButtonOnPress", SettingFuncSystemButtonOnPress,
    "SettingSystemSoundCheckBoxOnPress", SettingSystemSoundCheckBoxOnPress,
    "SettingSystemBrightnessTrackBarOnChanged", SettingSystemBrightnessTrackBarOnChanged,
    "SettingSystemLightCheckBoxOnPress", SettingSystemLightCheckBoxOnPress,
    "SettingSystemScreenSaverRadioBoxOnPress", SettingSystemScreenSaverRadioBoxOnPress,
    "SettingSystemDateWheelOnChanged", SettingSystemDateWheelOnChanged,
    "SettingSystemDateButtonOnPress", SettingSystemDateButtonOnPress,
    "SettingSystemTimeButtonOnPress", SettingSystemTimeButtonOnPress,
    "SettingSystemDateConfirmButtonOnPress", SettingSystemDateConfirmButtonOnPress,
    "SettingSystemTimeConfirmButtonOnPress", SettingSystemTimeConfirmButtonOnPress,

    // VideoPlayer
    "VideoPlayerOnEnter", VideoPlayerOnEnter,
    "VideoPlayerOnLeave", VideoPlayerOnLeave,
    "VideoPlayerOnTimer", VideoPlayerOnTimer,
    "VideoPlayerBackgroundButtonOnPress", VideoPlayerBackgroundButtonOnPress,
    "VideoPlayerPlayCheckBoxOnPress", VideoPlayerPlayCheckBoxOnPress,
    "VideoPlayerTrackBarOnChanged", VideoPlayerTrackBarOnChanged,

    // Engineer
    "EngineerOnEnter", EngineerOnEnter,
    "EngineerOnLeave", EngineerOnLeave,
    "EngineerOnTimer", EngineerOnTimer,
    "EngineerWarnTableListBoxOnLoad", EngineerWarnTableListBoxOnLoad,

    // EngineerPower
    "EngineerPowerOnEnter", EngineerPowerOnEnter,
    "EngineerPowerOnLeave", EngineerPowerOnLeave,
    "EngineerPowerOnTimer", EngineerPowerOnTimer,
    "EngineerPowerMemoryOnButtonOnPress", EngineerPowerMemoryOnButtonOnPress,
    "EngineerPowerMemoryOffButtonOnPress", EngineerPowerMemoryOffButtonOnPress,

    // EngineerMachine
    "EngineerMachineOnEnter", EngineerMachineOnEnter,
    "EngineerMachineOnLeave", EngineerMachineOnLeave,
    "EngineerMachineOnTimer", EngineerMachineOnTimer,

    // EngineerTry
    "EngineerTryOnEnter", EngineerTryOnEnter,
    "EngineerTryOnLeave", EngineerTryOnLeave,
    "EngineerTryOnTimer", EngineerTryOnTimer,
    "EngineerTryWarnTableListBoxOnLoad", EngineerTryWarnTableListBoxOnLoad,

    NULL, NULL
};

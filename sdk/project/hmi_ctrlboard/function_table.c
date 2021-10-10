#include "ite/itu.h"

extern bool LogoOnEnter(ITUWidget* widget, char* param);
extern bool ScreensaverOnEnter(ITUWidget* widget, char* param);
extern bool MainMenuOnEnter(ITUWidget* widget, char* param);
extern bool MainMenuOnLeave(ITUWidget* widget, char* param);
extern bool MainMenuCoverFlowOnChanged(ITUWidget* widget, char* param);
extern bool MainMenuAirConditionerPopupButtonOnPress(ITUWidget* widget, char* param);
extern bool MainMenuMediaPopupButtonOnPress(ITUWidget* widget, char* param);
extern bool AirConditionerOnEnter(ITUWidget* widget, char* param);
extern bool AirConditionerOnTimer(ITUWidget* widget, char* param);
extern bool AirConditionerPowerRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool AirConditionerAutoRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool AirConditionerCoolRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool AirConditionerDryRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool AirConditionerFanRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool AirConditionerHeatRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool AirConditionerWindSlowRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool AirConditionerWindNormalRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool AirConditionerWindFastRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool AirConditionerWindAutoRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool AirConditionerTemperatureTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool VideoViewOnEnter(ITUWidget* widget, char* param);
extern bool VideoViewOnLeave(ITUWidget* widget, char* param);
extern bool VideoViewSDRemovedOnCustom(ITUWidget* widget, char* param);
extern bool VideoViewUsbRemovedOnCustom(ITUWidget* widget, char* param);
extern bool UsbDeviceModeOnEnter(ITUWidget* widget, char* param);
extern bool UsbDeviceModeOnLeave(ITUWidget* widget, char* param);
extern bool TouchCalibrationCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool TouchCalibrationOnEnter(ITUWidget* widget, char* param);
extern bool TouchCalibrationOnLeave(ITUWidget* widget, char* param);

ITUActionFunction actionFunctions[] =
{
    "LogoOnEnter", LogoOnEnter,
    "ScreensaverOnEnter", ScreensaverOnEnter,
    "MainMenuOnEnter", MainMenuOnEnter,
    "MainMenuOnLeave", MainMenuOnLeave,
    "MainMenuCoverFlowOnChanged", MainMenuCoverFlowOnChanged,
    "MainMenuAirConditionerPopupButtonOnPress", MainMenuAirConditionerPopupButtonOnPress,
    "MainMenuMediaPopupButtonOnPress", MainMenuMediaPopupButtonOnPress,
    "AirConditionerOnEnter", AirConditionerOnEnter,
    "AirConditionerOnTimer", AirConditionerOnTimer,
    "AirConditionerPowerRadioBoxOnPress", AirConditionerPowerRadioBoxOnPress,
    "AirConditionerAutoRadioBoxOnPress", AirConditionerAutoRadioBoxOnPress,
    "AirConditionerCoolRadioBoxOnPress", AirConditionerCoolRadioBoxOnPress,
    "AirConditionerDryRadioBoxOnPress", AirConditionerDryRadioBoxOnPress,
    "AirConditionerFanRadioBoxOnPress", AirConditionerFanRadioBoxOnPress,
    "AirConditionerHeatRadioBoxOnPress", AirConditionerHeatRadioBoxOnPress,
    "AirConditionerWindSlowRadioBoxOnPress", AirConditionerWindSlowRadioBoxOnPress,
    "AirConditionerWindNormalRadioBoxOnPress", AirConditionerWindNormalRadioBoxOnPress,
    "AirConditionerWindFastRadioBoxOnPress", AirConditionerWindFastRadioBoxOnPress,
    "AirConditionerWindAutoRadioBoxOnPress", AirConditionerWindAutoRadioBoxOnPress,
    "AirConditionerTemperatureTrackBarOnChanged", AirConditionerTemperatureTrackBarOnChanged,
    "VideoViewOnEnter", VideoViewOnEnter,
    "VideoViewOnLeave", VideoViewOnLeave,
    "VideoViewSDRemovedOnCustom", VideoViewSDRemovedOnCustom,
    "VideoViewUsbRemovedOnCustom", VideoViewUsbRemovedOnCustom,
    "UsbDeviceModeOnEnter", UsbDeviceModeOnEnter,
    "UsbDeviceModeOnLeave", UsbDeviceModeOnLeave,
    "TouchCalibrationCheckBoxOnPress", TouchCalibrationCheckBoxOnPress,
	"TouchCalibrationOnEnter", TouchCalibrationOnEnter,
	"TouchCalibrationOnLeave", TouchCalibrationOnLeave,
    NULL, NULL
};

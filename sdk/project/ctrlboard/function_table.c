#include "ite/itu.h"

extern bool LogoOnEnter(ITUWidget* widget, char* param);
extern bool ScreensaverOnEnter(ITUWidget* widget, char* param);
extern bool MainMenuOnEnter(ITUWidget* widget, char* param);
extern bool MainMenuOnLeave(ITUWidget* widget, char* param);
extern bool ClockOnTimer(ITUWidget* widget, char* param);
extern bool ClockOnEnter(ITUWidget* widget, char* param);
extern bool ClockConfirmButtonOnPress(ITUWidget* widget, char* param);
extern bool ClockWheelOnChanged(ITUWidget* widget, char* param);
extern bool AirConditionerOnEnter(ITUWidget* widget, char* param);
extern bool AirConditionerOnTimer(ITUWidget* widget, char* param);
extern bool Keyboard2OnEnter(ITUWidget* widget, char* param);
extern bool Keyboard2EnterButtonOnPress(ITUWidget* widget, char* param);
extern bool Keyboard2PageDownButtonOnPress(ITUWidget* widget, char* param);
extern bool Keyboard2PageUpButtonOnPress(ITUWidget* widget, char* param);
extern bool Keyboard2BackButtonOnPress(ITUWidget* widget, char* param);
extern bool Keyboard2EnUpperButtonOnPress(ITUWidget* widget, char* param);
extern bool Keyboard2ChsButtonOnPress(ITUWidget* widget, char* param);
extern bool Keyboard2ChsCharButtonOnPress(ITUWidget* widget, char* param);
extern bool AudioPlayerOnEnter(ITUWidget* widget, char* param);
extern bool AudioPlayerOnLeave(ITUWidget* widget, char* param);
extern bool AudioPlayerSDInsertedOnCustom(ITUWidget* widget, char* param);
extern bool AudioPlayerSDRemovedOnCustom(ITUWidget* widget, char* param);
extern bool AudioPlayerUsbInsertedOnCustom(ITUWidget* widget, char* param);
extern bool AudioPlayerUsbRemovedOnCustom(ITUWidget* widget, char* param);
extern bool AudioPlayerOnTimer(ITUWidget* widget, char* param);
extern bool AudioPlayerStorageTypeCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool AudioPlayerStorageRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool AudioPlayerRepeatButtonOnPress(ITUWidget* widget, char* param);
extern bool AudioPlayerVolTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool AudioPlayerPlayCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool AudioPlayerNextButtonOnPress(ITUWidget* widget, char* param);
extern bool AudioPlayerLastButtonOnPress(ITUWidget* widget, char* param);
extern bool AudioPlayerRandomCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool AudioPlayerScrollMediaFileListBoxOnSelection(ITUWidget* widget, char* param);
extern bool VideoPlayerOnEnter(ITUWidget* widget, char* param);
extern bool VideoPlayerOnLeave(ITUWidget* widget, char* param);
extern bool VideoPlayerSDInsertedOnCustom(ITUWidget* widget, char* param);
extern bool VideoPlayerSDRemovedOnCustom(ITUWidget* widget, char* param);
extern bool VideoPlayerUsbInsertedOnCustom(ITUWidget* widget, char* param);
extern bool VideoPlayerUsbRemovedOnCustom(ITUWidget* widget, char* param);
extern bool VideoPlayerOnTimer(ITUWidget* widget, char* param);
extern bool VideoPlayerStorageRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool VideoPlayerVolTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool VideoPlayerPlayCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool VideoPlayerNextButtonOnPress(ITUWidget* widget, char* param);
extern bool VideoPlayerLastButtonOnPress(ITUWidget* widget, char* param);
extern bool VideoPlayerRandomCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool VideoPlayerRepeatButtonOnPress(ITUWidget* widget, char* param);
extern bool VideoPlayerScrollMediaFileListBoxOnSelection(ITUWidget* widget, char* param);
extern bool VideoPlayerStorageTypeCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool VideoViewOnTimer(ITUWidget* widget, char* param);
extern bool VideoViewOnEnter(ITUWidget* widget, char* param);
extern bool VideoViewOnLeave(ITUWidget* widget, char* param);
extern bool VideoViewSDRemovedOnCustom(ITUWidget* widget, char* param);
extern bool VideoViewUsbRemovedOnCustom(ITUWidget* widget, char* param);
extern bool VideoViewPlayCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool VideoViewRepeatButtonOnPress(ITUWidget* widget, char* param);
extern bool VideoViewVolTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool VideoViewNextButtonOnPress(ITUWidget* widget, char* param);
extern bool VideoViewLastButtonOnPress(ITUWidget* widget, char* param);
extern bool VideoViewRandomCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool VideoViewViewButtonOnPress(ITUWidget* widget, char* param);
extern bool ImagePlayerOnEnter(ITUWidget* widget, char* param);
extern bool ImagePlayerOnLeave(ITUWidget* widget, char* param);
extern bool ImagePlayerSDInsertedOnCustom(ITUWidget* widget, char* param);
extern bool ImagePlayerSDRemovedOnCustom(ITUWidget* widget, char* param);
extern bool ImagePlayerUsbInsertedOnCustom(ITUWidget* widget, char* param);
extern bool ImagePlayerUsbRemovedOnCustom(ITUWidget* widget, char* param);
extern bool ImagePlayerOnTimer(ITUWidget* widget, char* param);
extern bool ImagePlayerStorageRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool ImagePlayerNextButtonOnPress(ITUWidget* widget, char* param);
extern bool ImagePlayerLastButtonOnPress(ITUWidget* widget, char* param);
extern bool ImagePlayerScrollMediaFileListBoxOnSelection(ITUWidget* widget, char* param);
extern bool ImagePlayerStorageTypeCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool ImageViewOnTimer(ITUWidget* widget, char* param);
extern bool ImageViewOnEnter(ITUWidget* widget, char* param);
extern bool ImageViewOnLeave(ITUWidget* widget, char* param);
extern bool ImageViewSDRemovedOnCustom(ITUWidget* widget, char* param);
extern bool ImageViewUsbRemovedOnCustom(ITUWidget* widget, char* param);
extern bool ImageViewViewButtonOnPress(ITUWidget* widget, char* param);
extern bool ImageViewNextButtonOnPress(ITUWidget* widget, char* param);
extern bool ImageViewLastButtonOnPress(ITUWidget* widget, char* param);
extern bool CalendarOnEnter(ITUWidget* widget, char* param);
extern bool CalendarWheelOnChanged(ITUWidget* widget, char* param);
extern bool CalendarConfirmButtonOnPress(ITUWidget* widget, char* param);
extern bool CalendarCoverFlowOnChanged(ITUWidget* widget, char* param);
extern bool CalendarBackButtonOnPress(ITUWidget* widget, char* param);
extern bool ButtonOnEnter(ITUWidget* widget, char* param);
extern bool ButtonOnTimer(ITUWidget* widget, char* param);
extern bool ButtonUpgradeButtonOnPress(ITUWidget* widget, char* param);
extern bool ButtonRemoteButtonOnPress(ITUWidget* widget, char* param);
extern bool ButtonUartSendButtonOnPress(ITUWidget* widget, char* param);
extern bool ButtonI2cWriteButtonOnPress(ITUWidget* widget, char* param);
extern bool ButtonI2cReadButtonOnPress(ITUWidget* widget, char* param);
extern bool ButtonSpiWriteButtonOnPress(ITUWidget* widget, char* param);
extern bool ButtonSpiReadButtonOnPress(ITUWidget* widget, char* param);
extern bool ButtonAirConditionerPopupButtonOnPress(ITUWidget* widget, char* param);
extern bool ButtonTVPopupButtonOnPress(ITUWidget* widget, char* param);
extern bool ButtonLightPopupButtonOnPress(ITUWidget* widget, char* param);
extern bool ButtonPlugPopupButtonOnPress(ITUWidget* widget, char* param);
extern bool MeterOnEnter(ITUWidget* widget, char* param);
extern bool MeterOnTimer(ITUWidget* widget, char* param);
extern bool MeterSpeedTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool MeterValueMeterOnChanged(ITUWidget* widget, char* param);
extern bool CheckAnimOnEnter(ITUWidget* widget, char* param);
extern bool CheckListOnEnter(ITUWidget* widget, char* param);
extern bool CheckAnimSpeedTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool CheckAnimMoveCheckBox(ITUWidget* widget, char* param);
extern bool CheckAnimAlphablendCheckBox(ITUWidget* widget, char* param);
extern bool CheckAnimRotateCheckBox(ITUWidget* widget, char* param);
extern bool CheckAnimScaleCheckBoxOnPress(ITUWidget* widget, char* param);
extern bool CheckListScrollListBoxOnLoad(ITUWidget* widget, char* param);
extern bool CheckListScrollListBoxOnSelection(ITUWidget* widget, char* param);
extern bool CheckListScrollIconListBoxOnLoad(ITUWidget* widget, char* param);
extern bool CheckListScrollIconListBoxOnSelection(ITUWidget* widget, char* param);
extern bool CheckListDeleteButtonOnPress(ITUWidget* widget, char* param);
extern bool CheckListResetButtonOnPress(ITUWidget* widget, char* param);
extern bool CircleCtrlOnEnter(ITUWidget* widget, char* param);
extern bool CircleCtrlOnTimer(ITUWidget* widget, char* param);
extern bool CircleCtrlStartButtonOnPress(ITUWidget* widget, char* param);
extern bool CircleCtrlStopButtonOnPress(ITUWidget* widget, char* param);
extern bool CircleCtrlFastButtonOnPress(ITUWidget* widget, char* param);
extern bool CircleCtrlSlowButtonOnPress(ITUWidget* widget, char* param);
extern bool ListOnEnter(ITUWidget* widget, char* param);
extern bool ListTimeListBoxOnLoad(ITUWidget* widget, char* param);
extern bool ListDateListBoxOnLoad(ITUWidget* widget, char* param);
extern bool ListTypeListBoxOnLoad(ITUWidget* widget, char* param);
extern bool SettingDisplayOnEnter(ITUWidget* widget, char* param);
extern bool SettingDisplayOnLeave(ITUWidget* widget, char* param);
extern bool SettingDisplayScreensaverTypeRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingDisplayScreensaverTimeRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingDisplayMainMenuTypeRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingDisplayBrightnessTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool SettingSoundOnEnter(ITUWidget* widget, char* param);
extern bool SettingSoundOnLeave(ITUWidget* widget, char* param);
extern bool SettingSoundVolumeTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool SettingSoundKeyVolumeTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool SettingLangOnEnter(ITUWidget* widget, char* param);
extern bool SettingLangOnLeave(ITUWidget* widget, char* param);
extern bool SettingLangChtRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingLangEngRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingLangChsRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiMainOnEnter(ITUWidget* widget, char* param);
extern bool SettingWiFiMainOnLeave(ITUWidget* widget, char* param);
extern bool SettingWiFiMainRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidOnEnter(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidOnLeave(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidSignalScrollListBoxOnLoad(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidScrollListBoxOnSelect(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidStatusScrollListBoxOnLoad(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidNameScrollListBoxOnLoad(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordOnEnter(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordEnUpperButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordChsCharButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordEnterButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordPageDownButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordPageUpButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordBackButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiPasswordChsButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiNetworkOnEnter(ITUWidget* widget, char* param);
extern bool SettingWiFiNetworkOnLeave(ITUWidget* widget, char* param);
extern bool SettingWiFiNetworkIPAssignRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiNetworkIPInputBackButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiNetworkIPInputNumberButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiNetworkIPInputConfirmButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiNetworkIPInputClearButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiNetworkIPAddrButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiNetworkGatewayButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiNetworkDNSButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingWiFiNetworkNetmaskButtonOnPress(ITUWidget* widget, char* param);
extern bool SettingSysInfoOnEnter(ITUWidget* widget, char* param);
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
    "ClockOnTimer", ClockOnTimer,
    "ClockOnEnter", ClockOnEnter,
    "ClockConfirmButtonOnPress", ClockConfirmButtonOnPress,
    "ClockWheelOnChanged", ClockWheelOnChanged,
    "AirConditionerOnEnter", AirConditionerOnEnter,
    "AirConditionerOnTimer", AirConditionerOnTimer,
    "Keyboard2OnEnter", Keyboard2OnEnter,
    "Keyboard2EnterButtonOnPress", Keyboard2EnterButtonOnPress,
    "Keyboard2PageDownButtonOnPress", Keyboard2PageDownButtonOnPress,
    "Keyboard2PageUpButtonOnPress", Keyboard2PageUpButtonOnPress,
    "Keyboard2BackButtonOnPress", Keyboard2BackButtonOnPress,
    "Keyboard2EnUpperButtonOnPress", Keyboard2EnUpperButtonOnPress,
    "Keyboard2ChsButtonOnPress", Keyboard2ChsButtonOnPress,
    "Keyboard2ChsCharButtonOnPress", Keyboard2ChsCharButtonOnPress,
    "AudioPlayerOnEnter", AudioPlayerOnEnter,
    "AudioPlayerOnLeave", AudioPlayerOnLeave,
    "AudioPlayerSDInsertedOnCustom", AudioPlayerSDInsertedOnCustom,
    "AudioPlayerSDRemovedOnCustom", AudioPlayerSDRemovedOnCustom,
    "AudioPlayerUsbInsertedOnCustom", AudioPlayerUsbInsertedOnCustom,
    "AudioPlayerUsbRemovedOnCustom", AudioPlayerUsbRemovedOnCustom,
    "AudioPlayerOnTimer", AudioPlayerOnTimer,
    "AudioPlayerStorageTypeCheckBoxOnPress", AudioPlayerStorageTypeCheckBoxOnPress,
    "AudioPlayerStorageRadioBoxOnPress", AudioPlayerStorageRadioBoxOnPress,
    "AudioPlayerRepeatButtonOnPress", AudioPlayerRepeatButtonOnPress,
    "AudioPlayerVolTrackBarOnChanged", AudioPlayerVolTrackBarOnChanged,
    "AudioPlayerPlayCheckBoxOnPress", AudioPlayerPlayCheckBoxOnPress,
    "AudioPlayerNextButtonOnPress", AudioPlayerNextButtonOnPress,
    "AudioPlayerLastButtonOnPress", AudioPlayerLastButtonOnPress,
    "AudioPlayerRandomCheckBoxOnPress", AudioPlayerRandomCheckBoxOnPress,
    "AudioPlayerScrollMediaFileListBoxOnSelection", AudioPlayerScrollMediaFileListBoxOnSelection,
    "VideoPlayerOnEnter", VideoPlayerOnEnter,
    "VideoPlayerOnLeave", VideoPlayerOnLeave,
    "VideoPlayerSDInsertedOnCustom", VideoPlayerSDInsertedOnCustom,
    "VideoPlayerSDRemovedOnCustom", VideoPlayerSDRemovedOnCustom,
    "VideoPlayerUsbInsertedOnCustom", VideoPlayerUsbInsertedOnCustom,
    "VideoPlayerUsbRemovedOnCustom", VideoPlayerUsbRemovedOnCustom,
    "VideoPlayerOnTimer", VideoPlayerOnTimer,
    "VideoPlayerStorageRadioBoxOnPress", VideoPlayerStorageRadioBoxOnPress,
    "VideoPlayerVolTrackBarOnChanged", VideoPlayerVolTrackBarOnChanged,
    "VideoPlayerPlayCheckBoxOnPress", VideoPlayerPlayCheckBoxOnPress,
    "VideoPlayerNextButtonOnPress", VideoPlayerNextButtonOnPress,
    "VideoPlayerLastButtonOnPress", VideoPlayerLastButtonOnPress,
    "VideoPlayerRandomCheckBoxOnPress", VideoPlayerRandomCheckBoxOnPress,
    "VideoPlayerRepeatButtonOnPress", VideoPlayerRepeatButtonOnPress,
    "VideoPlayerScrollMediaFileListBoxOnSelection", VideoPlayerScrollMediaFileListBoxOnSelection,
    "VideoPlayerStorageTypeCheckBoxOnPress", VideoPlayerStorageTypeCheckBoxOnPress,
    "VideoViewOnTimer", VideoViewOnTimer,
    "VideoViewOnEnter", VideoViewOnEnter,
    "VideoViewOnLeave", VideoViewOnLeave,
    "VideoViewSDRemovedOnCustom", VideoViewSDRemovedOnCustom,
    "VideoViewUsbRemovedOnCustom", VideoViewUsbRemovedOnCustom,
    "VideoViewPlayCheckBoxOnPress", VideoViewPlayCheckBoxOnPress,
    "VideoViewRepeatButtonOnPress", VideoViewRepeatButtonOnPress,
    "VideoViewVolTrackBarOnChanged", VideoViewVolTrackBarOnChanged,
    "VideoViewNextButtonOnPress", VideoViewNextButtonOnPress,
    "VideoViewLastButtonOnPress", VideoViewLastButtonOnPress,
    "VideoViewRandomCheckBoxOnPress", VideoViewRandomCheckBoxOnPress,
    "VideoViewViewButtonOnPress", VideoViewViewButtonOnPress,
    "ImagePlayerOnEnter", ImagePlayerOnEnter,
    "ImagePlayerOnLeave", ImagePlayerOnLeave,
    "ImagePlayerSDInsertedOnCustom", ImagePlayerSDInsertedOnCustom,
    "ImagePlayerSDRemovedOnCustom", ImagePlayerSDRemovedOnCustom,
    "ImagePlayerUsbInsertedOnCustom", ImagePlayerUsbInsertedOnCustom,
    "ImagePlayerUsbRemovedOnCustom", ImagePlayerUsbRemovedOnCustom,
    "ImagePlayerOnTimer", ImagePlayerOnTimer,
    "ImagePlayerStorageRadioBoxOnPress", ImagePlayerStorageRadioBoxOnPress,
    "ImagePlayerNextButtonOnPress", ImagePlayerNextButtonOnPress,
    "ImagePlayerLastButtonOnPress", ImagePlayerLastButtonOnPress,
    "ImagePlayerScrollMediaFileListBoxOnSelection", ImagePlayerScrollMediaFileListBoxOnSelection,
    "ImagePlayerStorageTypeCheckBoxOnPress", ImagePlayerStorageTypeCheckBoxOnPress,
    "ImageViewOnTimer", ImageViewOnTimer,
    "ImageViewOnEnter", ImageViewOnEnter,
    "ImageViewOnLeave", ImageViewOnLeave,
    "ImageViewSDRemovedOnCustom", ImageViewSDRemovedOnCustom,
    "ImageViewUsbRemovedOnCustom", ImageViewUsbRemovedOnCustom,
    "ImageViewViewButtonOnPress", ImageViewViewButtonOnPress,
    "ImageViewNextButtonOnPress", ImageViewNextButtonOnPress,
    "ImageViewLastButtonOnPress", ImageViewLastButtonOnPress,
    "CalendarOnEnter", CalendarOnEnter,
    "CalendarWheelOnChanged", CalendarWheelOnChanged,
    "CalendarConfirmButtonOnPress", CalendarConfirmButtonOnPress,
    "CalendarCoverFlowOnChanged", CalendarCoverFlowOnChanged,
    "CalendarBackButtonOnPress", CalendarBackButtonOnPress,
    "ButtonOnEnter", ButtonOnEnter,
    "ButtonOnTimer", ButtonOnTimer,
    "ButtonUpgradeButtonOnPress", ButtonUpgradeButtonOnPress,
    "ButtonRemoteButtonOnPress", ButtonRemoteButtonOnPress,
    "ButtonUartSendButtonOnPress", ButtonUartSendButtonOnPress,
    "ButtonI2cWriteButtonOnPress", ButtonI2cWriteButtonOnPress,
    "ButtonI2cReadButtonOnPress", ButtonI2cReadButtonOnPress,
    "ButtonSpiWriteButtonOnPress", ButtonSpiWriteButtonOnPress,
    "ButtonSpiReadButtonOnPress", ButtonSpiReadButtonOnPress,
    "ButtonAirConditionerPopupButtonOnPress", ButtonAirConditionerPopupButtonOnPress,
    "ButtonTVPopupButtonOnPress", ButtonTVPopupButtonOnPress,
    "ButtonLightPopupButtonOnPress", ButtonLightPopupButtonOnPress,
    "ButtonPlugPopupButtonOnPress", ButtonPlugPopupButtonOnPress,
    "MeterOnEnter", MeterOnEnter,
    "MeterOnTimer", MeterOnTimer,
    "MeterSpeedTrackBarOnChanged", MeterSpeedTrackBarOnChanged,
    "MeterValueMeterOnChanged", MeterValueMeterOnChanged,
    "CheckAnimOnEnter", CheckAnimOnEnter,
    "CheckListOnEnter", CheckListOnEnter,
    "CheckAnimSpeedTrackBarOnChanged", CheckAnimSpeedTrackBarOnChanged,
    "CheckAnimMoveCheckBox", CheckAnimMoveCheckBox,
    "CheckAnimAlphablendCheckBox", CheckAnimAlphablendCheckBox,
    "CheckAnimRotateCheckBox", CheckAnimRotateCheckBox,
    "CheckAnimScaleCheckBoxOnPress", CheckAnimScaleCheckBoxOnPress,
    "CheckListScrollListBoxOnLoad", CheckListScrollListBoxOnLoad,
    "CheckListScrollListBoxOnSelection", CheckListScrollListBoxOnSelection,
    "CheckListScrollIconListBoxOnLoad", CheckListScrollIconListBoxOnLoad,
    "CheckListScrollIconListBoxOnSelection", CheckListScrollIconListBoxOnSelection,
    "CheckListDeleteButtonOnPress", CheckListDeleteButtonOnPress,
    "CheckListResetButtonOnPress", CheckListResetButtonOnPress,
    "CircleCtrlOnEnter", CircleCtrlOnEnter,
    "CircleCtrlOnTimer", CircleCtrlOnTimer,
    "CircleCtrlStartButtonOnPress", CircleCtrlStartButtonOnPress,
    "CircleCtrlStopButtonOnPress", CircleCtrlStopButtonOnPress,
    "CircleCtrlFastButtonOnPress", CircleCtrlFastButtonOnPress,
    "CircleCtrlSlowButtonOnPress", CircleCtrlSlowButtonOnPress,
    "ListOnEnter", ListOnEnter,
    "ListTimeListBoxOnLoad", ListTimeListBoxOnLoad,
    "ListDateListBoxOnLoad", ListDateListBoxOnLoad,
    "ListTypeListBoxOnLoad", ListTypeListBoxOnLoad,
    "SettingDisplayOnEnter", SettingDisplayOnEnter,
    "SettingDisplayOnLeave", SettingDisplayOnLeave,
    "SettingDisplayScreensaverTypeRadioBoxOnPress", SettingDisplayScreensaverTypeRadioBoxOnPress,
    "SettingDisplayScreensaverTimeRadioBoxOnPress", SettingDisplayScreensaverTimeRadioBoxOnPress,
    "SettingDisplayMainMenuTypeRadioBoxOnPress", SettingDisplayMainMenuTypeRadioBoxOnPress,
    "SettingDisplayBrightnessTrackBarOnChanged", SettingDisplayBrightnessTrackBarOnChanged,
    "SettingSoundOnEnter", SettingSoundOnEnter,
    "SettingSoundOnLeave", SettingSoundOnLeave,
    "SettingSoundVolumeTrackBarOnChanged", SettingSoundVolumeTrackBarOnChanged,
    "SettingSoundKeyVolumeTrackBarOnChanged", SettingSoundKeyVolumeTrackBarOnChanged,
    "SettingLangOnEnter", SettingLangOnEnter,
    "SettingLangOnLeave", SettingLangOnLeave,
    "SettingLangChtRadioBoxOnPress", SettingLangChtRadioBoxOnPress,
    "SettingLangEngRadioBoxOnPress", SettingLangEngRadioBoxOnPress,
    "SettingLangChsRadioBoxOnPress", SettingLangChsRadioBoxOnPress,
    "SettingWiFiMainOnEnter", SettingWiFiMainOnEnter,
    "SettingWiFiMainOnLeave", SettingWiFiMainOnLeave,
    "SettingWiFiMainRadioBoxOnPress", SettingWiFiMainRadioBoxOnPress,
    "SettingWiFiSsidOnEnter", SettingWiFiSsidOnEnter,
    "SettingWiFiSsidOnLeave", SettingWiFiSsidOnLeave,
    "SettingWiFiSsidSignalScrollListBoxOnLoad", SettingWiFiSsidSignalScrollListBoxOnLoad,
    "SettingWiFiSsidScrollListBoxOnSelect", SettingWiFiSsidScrollListBoxOnSelect,
    "SettingWiFiSsidStatusScrollListBoxOnLoad", SettingWiFiSsidStatusScrollListBoxOnLoad,
    "SettingWiFiSsidNameScrollListBoxOnLoad", SettingWiFiSsidNameScrollListBoxOnLoad,
    "SettingWiFiPasswordOnEnter", SettingWiFiPasswordOnEnter,
    "SettingWiFiPasswordEnUpperButtonOnPress", SettingWiFiPasswordEnUpperButtonOnPress,
    "SettingWiFiPasswordChsCharButtonOnPress", SettingWiFiPasswordChsCharButtonOnPress,
    "SettingWiFiPasswordEnterButtonOnPress", SettingWiFiPasswordEnterButtonOnPress,
    "SettingWiFiPasswordPageDownButtonOnPress", SettingWiFiPasswordPageDownButtonOnPress,
    "SettingWiFiPasswordPageUpButtonOnPress", SettingWiFiPasswordPageUpButtonOnPress,
    "SettingWiFiPasswordBackButtonOnPress", SettingWiFiPasswordBackButtonOnPress,
    "SettingWiFiPasswordChsButtonOnPress", SettingWiFiPasswordChsButtonOnPress,
    "SettingWiFiNetworkOnEnter", SettingWiFiNetworkOnEnter,
    "SettingWiFiNetworkOnLeave", SettingWiFiNetworkOnLeave,
    "SettingWiFiNetworkIPAssignRadioBoxOnPress", SettingWiFiNetworkIPAssignRadioBoxOnPress,
    "SettingWiFiNetworkIPInputBackButtonOnPress", SettingWiFiNetworkIPInputBackButtonOnPress,
    "SettingWiFiNetworkIPInputNumberButtonOnPress", SettingWiFiNetworkIPInputNumberButtonOnPress,
    "SettingWiFiNetworkIPInputConfirmButtonOnPress", SettingWiFiNetworkIPInputConfirmButtonOnPress,
    "SettingWiFiNetworkIPInputClearButtonOnPress", SettingWiFiNetworkIPInputClearButtonOnPress,
    "SettingWiFiNetworkIPAddrButtonOnPress", SettingWiFiNetworkIPAddrButtonOnPress,
    //"SettingWiFiNetworkGatewayButtonOnPress", SettingWiFiNetworkGatewayButtonOnPress,
    "SettingWiFiNetworkDNSButtonOnPress", SettingWiFiNetworkDNSButtonOnPress,
    "SettingWiFiNetworkNetmaskButtonOnPress", SettingWiFiNetworkNetmaskButtonOnPress,
    "SettingSysInfoOnEnter", SettingSysInfoOnEnter,
    "UsbDeviceModeOnEnter", UsbDeviceModeOnEnter,
    "UsbDeviceModeOnLeave", UsbDeviceModeOnLeave,
    "TouchCalibrationCheckBoxOnPress", TouchCalibrationCheckBoxOnPress,
	"TouchCalibrationOnEnter", TouchCalibrationOnEnter,
	"TouchCalibrationOnLeave", TouchCalibrationOnLeave,
    NULL, NULL
};

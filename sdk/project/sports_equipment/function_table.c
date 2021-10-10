#include "ite/itu.h"

// Standby
extern bool StandbyOnEnter(ITUWidget* widget, char* param);
extern bool StandbyStartPopupButtonOnPress(ITUWidget* widget, char* param);

// Main
extern bool MainOnEnter(ITUWidget* widget, char* param);
extern bool MainOnLeave(ITUWidget* widget, char* param);
extern bool MainOnTimer(ITUWidget* widget, char* param);
extern bool MainTypeBackgroundButtonOnPress(ITUWidget* widget, char* param);
extern bool MainProgressBackgroundButtonOnPress(ITUWidget* widget, char* param);
extern bool MainSlopeWheelOnChanged(ITUWidget* widget, char* param);
extern bool MainSpeedWheelOnChanged(ITUWidget* widget, char* param);
extern bool MainFanBackgroundButtonOnPress(ITUWidget* widget, char* param);
extern bool MainPausePopupButtonOnPress(ITUWidget* widget, char* param);
extern bool MainPlayPopupButtonOnPress(ITUWidget* widget, char* param);
extern bool MainStopPopupButtonOnPress(ITUWidget* widget, char* param);
extern bool MainStatusBackgroundButtonOnPress(ITUWidget* widget, char* param);

// Setting
extern bool SettingOnEnter(ITUWidget* widget, char* param);
extern bool SettingOnLeave(ITUWidget* widget, char* param);
extern bool SettingDateTimeRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingAgeWheelOnChanged(ITUWidget* widget, char* param);
extern bool SettingWeightWheelOnChanged(ITUWidget* widget, char* param);
extern bool SettingPulseWheelOnChanged(ITUWidget* widget, char* param);
extern bool SettingVolumeTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool SettingBrightnessTrackBarOnChanged(ITUWidget* widget, char* param);
extern bool SettingDateWheelOnChanged(ITUWidget* widget, char* param);
extern bool SettingLanguageRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingUnitDistanceRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool SettingUnitWeightRadioBoxOnPress(ITUWidget* widget, char* param);

// Program
extern bool ProgramOnEnter(ITUWidget* widget, char* param);
extern bool ProgramOnLeave(ITUWidget* widget, char* param);
extern bool ProgramTypeRadioBoxOnPress(ITUWidget* widget, char* param);
extern bool ProgramDialogExitButtonOnPress(ITUWidget* widget, char* param);

ITUActionFunction actionFunctions[] =
{
    // Standby
    "StandbyOnEnter", StandbyOnEnter,
    "StandbyStartPopupButtonOnPress", StandbyStartPopupButtonOnPress,

    // Main
    "MainOnEnter", MainOnEnter,
    "MainOnLeave", MainOnLeave,
    "MainOnTimer", MainOnTimer,
    "MainTypeBackgroundButtonOnPress", MainTypeBackgroundButtonOnPress,
    "MainProgressBackgroundButtonOnPress", MainProgressBackgroundButtonOnPress,
    "MainSlopeWheelOnChanged", MainSlopeWheelOnChanged,
    "MainSpeedWheelOnChanged", MainSpeedWheelOnChanged,
    "MainFanBackgroundButtonOnPress", MainFanBackgroundButtonOnPress,
    "MainPausePopupButtonOnPress", MainPausePopupButtonOnPress,
    "MainPlayPopupButtonOnPress", MainPlayPopupButtonOnPress,
    "MainStopPopupButtonOnPress", MainStopPopupButtonOnPress,
    "MainStatusBackgroundButtonOnPress", MainStatusBackgroundButtonOnPress,

    // Setting
    "SettingOnEnter", SettingOnEnter,
    "SettingOnLeave", SettingOnLeave,
    "SettingDateTimeRadioBoxOnPress", SettingDateTimeRadioBoxOnPress,
    "SettingAgeWheelOnChanged", SettingAgeWheelOnChanged,
    "SettingWeightWheelOnChanged", SettingWeightWheelOnChanged,
    "SettingPulseWheelOnChanged", SettingPulseWheelOnChanged,
    "SettingVolumeTrackBarOnChanged", SettingVolumeTrackBarOnChanged,
    "SettingBrightnessTrackBarOnChanged", SettingBrightnessTrackBarOnChanged,
    "SettingDateWheelOnChanged", SettingDateWheelOnChanged,
    "SettingLanguageRadioBoxOnPress", SettingLanguageRadioBoxOnPress,
    "SettingUnitDistanceRadioBoxOnPress", SettingUnitDistanceRadioBoxOnPress,
    "SettingUnitWeightRadioBoxOnPress", SettingUnitWeightRadioBoxOnPress,

    // Program
    "ProgramOnEnter", ProgramOnEnter,
    "ProgramOnLeave", ProgramOnLeave,
    "ProgramTypeRadioBoxOnPress", ProgramTypeRadioBoxOnPress,
    "ProgramDialogExitButtonOnPress", ProgramDialogExitButtonOnPress,

    NULL, NULL
};

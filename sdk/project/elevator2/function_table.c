#include "ite/itu.h"

// Main
extern bool MainOnEnter(ITUWidget* widget, char* param);
extern bool MainOnLeave(ITUWidget* widget, char* param);
extern bool MainOnTimer(ITUWidget* widget, char* param);
extern bool MainOnKeyDown(ITUWidget* widget, char* param);

// Setting
extern bool SettingOnEnter(ITUWidget* widget, char* param);
extern bool SettingOnLeave(ITUWidget* widget, char* param);
extern bool SettingOnKeyDown(ITUWidget* widget, char* param);
extern bool SettingOnTimer(ITUWidget* widget, char* param);

ITUActionFunction actionFunctions[] =
{
    // Main
    "MainOnEnter", MainOnEnter,
    "MainOnLeave", MainOnLeave,
    "MainOnTimer", MainOnTimer,
    "MainOnKeyDown", MainOnKeyDown,

    // Setting
    "SettingOnEnter", SettingOnEnter,
    "SettingOnLeave", SettingOnLeave,
    "SettingOnKeyDown", SettingOnKeyDown,
    "SettingOnTimer", SettingOnTimer,

    NULL, NULL
};

#include "ite/itu.h"

extern bool KeyboardJYOnEnter(ITUWidget* widget, char* param);
extern bool KeyboardJYCharButtonOnPress(ITUWidget* widget, char* param);
extern bool KeyboardJYPageDownButtonOnPress(ITUWidget* widget, char* param);
extern bool KeyboardJYPageUpButtonOnPress(ITUWidget* widget, char* param);
extern bool KeyboardJYNumButtonOnPress(ITUWidget* widget, char* param);
extern bool KeyboardJYBackButtonOnPress(ITUWidget* widget, char* param);
extern bool KeyboardPYOnEnter(ITUWidget* widget, char* param);
extern bool KeyboardPYPageDownButtonOnPress(ITUWidget* widget, char* param);
extern bool KeyboardPYPageUpButtonOnPress(ITUWidget* widget, char* param);
extern bool KeyboardPYNumButtonOnPress(ITUWidget* widget, char* param);
extern bool KeyboardPYCharButtonOnPress(ITUWidget* widget, char* param);
extern bool KeyboardPYBackButtonOnPress(ITUWidget* widget, char* param);

ITUActionFunction actionFunctions[] =
{
    "KeyboardJYOnEnter", KeyboardJYOnEnter,
    "KeyboardJYCharButtonOnPress", KeyboardJYCharButtonOnPress,
    "KeyboardJYPageDownButtonOnPress", KeyboardJYPageDownButtonOnPress,
    "KeyboardJYPageUpButtonOnPress", KeyboardJYPageUpButtonOnPress,
    "KeyboardJYNumButtonOnPress", KeyboardJYNumButtonOnPress,
    "KeyboardJYBackButtonOnPress", KeyboardJYBackButtonOnPress,
    "KeyboardPYOnEnter", KeyboardPYOnEnter,
    "KeyboardPYPageDownButtonOnPress", KeyboardPYPageDownButtonOnPress,
    "KeyboardPYPageUpButtonOnPress", KeyboardPYPageUpButtonOnPress,
    "KeyboardPYNumButtonOnPress", KeyboardPYNumButtonOnPress,
    "KeyboardPYCharButtonOnPress", KeyboardPYCharButtonOnPress,
    "KeyboardPYBackButtonOnPress", KeyboardPYBackButtonOnPress,
    NULL, NULL
};

#include "ite/itu.h"

extern bool DynamicUIOnEnter(ITUWidget* widget, char* param);
extern bool DynamicUIResetButtonOnPress(ITUWidget* widget, char* param);
extern bool DynamicUISaveButtonOnPress(ITUWidget* widget, char* param);
extern bool DynamicUIButtonOnLongPress(ITUWidget* widget, char* param);
extern bool DynamicUIBackgroundButtonOnPress(ITUWidget* widget, char* param);
extern bool DynamicUITrashCanBackgroundButtonOnMouseUp(ITUWidget* widget, char* param);

ITUActionFunction actionFunctions[] =
{
    "DynamicUIOnEnter", DynamicUIOnEnter,
    "DynamicUIResetButtonOnPress", DynamicUIResetButtonOnPress,
    "DynamicUISaveButtonOnPress", DynamicUISaveButtonOnPress,
    "DynamicUIButtonOnLongPress", DynamicUIButtonOnLongPress,
    "DynamicUIBackgroundButtonOnPress", DynamicUIBackgroundButtonOnPress,
    "DynamicUITrashCanBackgroundButtonOnMouseUp", DynamicUITrashCanBackgroundButtonOnMouseUp,

    NULL, NULL
};

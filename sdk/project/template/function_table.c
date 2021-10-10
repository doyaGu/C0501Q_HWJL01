#include "ite/itu.h"

//Main
extern bool MainOnEnter(ITUWidget* widget, char* param);
extern bool MainButtonOnPress(ITUWidget* widget, char* param);

ITUActionFunction actionFunctions[] =
{
	//Main
    "MainOnEnter", MainOnEnter,
    "MainButtonOnPress", MainButtonOnPress,

    NULL, NULL
};

#include <assert.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char keyboardName[] = "ITUKeyboard";

bool ituKeyboardUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result;
    ITUKeyboard* keyboard = (ITUKeyboard*) widget;
    assert(keyboard);

    result = ituIconUpdate(widget, ev, arg1, arg2, arg3);

    return widget->visible ? result : false;
}

void ituKeyboardOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    ITUKeyboard* keyboard = (ITUKeyboard*) widget;
    assert(keyboard);

    switch (action)
    {
    case ITU_ACTION_BIND:
        if (param[0] != '\0')
            keyboard->target = ituSceneFindWidget(ituScene, param);
        else
            keyboard->target = ituScene->focused;
        break;

    case ITU_ACTION_INPUT:
        if (keyboard->target)
            ituWidgetOnAction(keyboard->target, ITU_ACTION_INPUT, param);
        break;

    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

void ituKeyboardInit(ITUKeyboard* keyboard)
{
    assert(keyboard);
    ITU_ASSERT_THREAD();

    memset(keyboard, 0, sizeof (ITUKeyboard));

    ituBackgroundInit(&keyboard->bg);

    ituWidgetSetType(keyboard, ITU_KEYBOARD);
    ituWidgetSetName(keyboard, keyboardName);
    ituWidgetSetUpdate(keyboard, ituKeyboardUpdate);
    ituWidgetSetOnAction(keyboard, ituKeyboardOnAction);
}

void ituKeyboardLoad(ITUKeyboard* keyboard, uint32_t base)
{
    assert(keyboard);

    ituBackgroundLoad(&keyboard->bg, base);
    ituWidgetSetUpdate(keyboard, ituKeyboardUpdate);
    ituWidgetSetOnAction(keyboard, ituKeyboardOnAction);
}

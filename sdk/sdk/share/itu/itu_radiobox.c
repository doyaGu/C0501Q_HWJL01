#include <assert.h>
#include <malloc.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char radioboxName[] = "ITURadioBox";

bool ituRadioBoxClone(ITUWidget* widget, ITUWidget** cloned)
{
    ITURadioBox* radiobox = (ITURadioBox*)widget;
    assert(widget);
    assert(cloned);
    ITU_ASSERT_THREAD();

    if (*cloned == NULL)
    {
        ITUWidget* newWidget = malloc(sizeof(ITURadioBox));
        if (newWidget == NULL)
            return false;

        memcpy(newWidget, widget, sizeof(ITURadioBox));
        newWidget->tree.child = newWidget->tree.parent = newWidget->tree.sibling = NULL;
        *cloned = newWidget;
    }

    return ituCheckBoxClone(widget, cloned);
}

bool ituRadioBoxUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUButton* btn = (ITUButton*) widget;
    ITURadioBox* radiobox = (ITURadioBox*) widget;
    ITUEvent response_key = widget->flags & ITU_RESPONSE_TO_UP_KEY ? ITU_EVENT_MOUSEUP : ITU_EVENT_MOUSEDOWN;
    assert(radiobox);

    if (ev == response_key)
    {
        result |= ituIconUpdate(widget, ev, arg1, arg2, arg3);

        if (ituWidgetIsEnabled(widget) && !result)
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ev == ITU_EVENT_MOUSEDOWN)
            {
                if (ituWidgetIsInside(widget, x, y))
                {
                    ituButtonSetPressed(btn, true);
                    ituFocusWidget(radiobox);
                    ituRadioBoxSetChecked(radiobox, true);
                    result |= ituWidgetOnPress(widget, ev, arg1, x, y);
                    result |= ituExecActions((ITUWidget*)btn, btn->actions, ev, 0);
                    result |= widget->dirty;
                }
            }
            else if (ev == ITU_EVENT_MOUSEUP)
            {
                if (btn->pressed)
                {
                    ituButtonSetPressed(btn, false);

                    if (ituWidgetIsInside(widget, x, y))
                    {
                        ituFocusWidget(radiobox);
                        ituRadioBoxSetChecked(radiobox, true);
                        result |= ituWidgetOnPress(widget, ev, arg1, x, y);
                        result |= ituExecActions((ITUWidget*)btn, btn->actions, ev, 0);
                        result |= widget->dirty;
                    }
                }
            }
        }
        return widget->visible ? result : false;
    }
    
    result |= ituCheckBoxUpdate(widget, ev, arg1, arg2, arg3);

    if (ituWidgetIsActive(widget) && ituWidgetIsEnabled(widget) && !result)
    {
        result |= ituButtonUpdate(widget, ev, arg1, arg2, arg3);

        response_key = widget->flags & ITU_RESPONSE_TO_UP_KEY ? ITU_EVENT_KEYUP : ITU_EVENT_KEYDOWN;

        if (ev == response_key)
        {
            if (arg1 == ituScene->enterKey)
            {
                ituFocusWidget(radiobox);
                ituRadioBoxSetChecked(radiobox, true);
                result |= widget->dirty;
            }
        }
        return widget->visible ? result : false;
    }
    return widget->visible ? result : false;
}

void ituRadioBoxOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    ITURadioBox* radiobox = (ITURadioBox*) widget;
    assert(radiobox);

    switch (action)
    {
    case ITU_ACTION_CHECK:
        ituRadioBoxSetChecked(radiobox, true);
        break;

    case ITU_ACTION_UNCHECK:
        ituRadioBoxSetChecked(radiobox, false);
        break;

    default:
        ituButtonOnAction(widget, action, param);
        break;
    }
}

void ituRadioBoxInit(ITURadioBox* radiobox)
{
    assert(radiobox);
    ITU_ASSERT_THREAD();

    memset(radiobox, 0, sizeof (ITURadioBox));

    ituCheckBoxInit(&radiobox->checkbox);

    ituWidgetSetType(radiobox, ITU_RADIOBOX);
    ituWidgetSetName(radiobox, radioboxName);
    ituWidgetSetClone(radiobox, ituRadioBoxClone);
    ituWidgetSetUpdate(radiobox, ituRadioBoxUpdate);
    ituWidgetSetOnAction(radiobox, ituRadioBoxOnAction);
}

void ituRadioBoxLoad(ITURadioBox* radiobox, uint32_t base)
{
    assert(radiobox);

    ituCheckBoxLoad(&radiobox->checkbox, base);

    ituWidgetSetClone(radiobox, ituRadioBoxClone);
    ituWidgetSetUpdate(radiobox, ituRadioBoxUpdate);
    ituWidgetSetOnAction(radiobox, ituRadioBoxOnAction);
}

void ituRadioBoxSetChecked(ITURadioBox* radiobox, bool checked)
{
    ITCTree* parent = ((ITCTree*)radiobox)->parent;
    assert(radiobox);
    ITU_ASSERT_THREAD();

    if (parent)
    {
        ITCTree* node;

        for (node = parent->child; node; node = node->sibling)
        {
            ITUWidget* child = (ITUWidget*) node;

            if (child == (ITUWidget*)radiobox)
                continue;

            if (child->type == ITU_RADIOBOX || child->type == ITU_POPUPRADIOBOX)
            {
                ITURadioBox* rb = (ITURadioBox*) child;
                if (ituRadioBoxIsChecked(rb))
                    ituCheckBoxSetChecked(&rb->checkbox, false);
            }
        }
    }
    ituCheckBoxSetChecked(&radiobox->checkbox, checked);
}

bool ituRadioBoxIsChecked(ITURadioBox* radiobox)
{
    assert(radiobox);
    ITU_ASSERT_THREAD();
    return radiobox->checkbox.checked;
}

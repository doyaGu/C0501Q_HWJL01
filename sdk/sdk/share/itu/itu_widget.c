#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char widgetName[] = "ITUWidget";

void ituWidgetExitImpl(ITUWidget* widget)
{
    ITCTree *node, *next;
    assert(widget);

    node = widget->tree.child;
    while (node)
    {
        next = node->sibling;
        ituWidgetExit(node);
        node = next;
    }

    if (widget->flags & ITU_DYNAMIC)
        free(widget);
}

bool ituWidgetCloneImpl(ITUWidget* widget, ITUWidget** cloned)
{
    ITCTree *node, *next;
    assert(widget);
    assert(cloned);

    if (*cloned == NULL)
    {
        ITUWidget* newWidget = malloc(sizeof(ITUWidget));
        if (newWidget == NULL)
            return false;

        memcpy(newWidget, widget, sizeof(ITUWidget));
        newWidget->tree.child = newWidget->tree.parent = newWidget->tree.sibling = NULL;
        *cloned = newWidget;
    }
    (*cloned)->flags |= ITU_DYNAMIC;

    node = widget->tree.child;
    while (node)
    {
        bool result;
        ITUWidget* child = NULL;
        next = node->sibling;

        result = ituWidgetClone(node, &child);
        if (!result)
            return false;

        ituWidgetAdd(*cloned, child);
        node = next;
    }
    (*cloned)->dirty = true;
    return true;
}

void ituWidgetAddImpl(ITUWidget* widget, ITUWidget* child)
{
    assert(widget);
    assert(child);

    widget->dirty = true;

    itcTreePushBack(&widget->tree, child);
}

static void WidgetHide(int arg)
{
    ITUWidget* widget = (ITUWidget*)arg;
    assert(widget);

    ituWidgetSetVisible(widget, false);
}

bool ituWidgetUpdateImpl(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITCTree* node;
    int childCount;
    ITUWidget* children[ITU_WIDGET_CHILD_MAX];

    assert(widget);

    if (ev != ITU_EVENT_TIMER && ev != ITU_EVENT_MOUSEMOVE)
    {
        LOG_UPDATE "%s{%d,%d,%d,%d} t:%d v:%d, ac:%d d:%d a:%d\n",
            widget->name,
            widget->rect.x,
            widget->rect.y,
            widget->rect.width,
            widget->rect.height,
            widget->type,
            widget->visible,
            widget->active,
            widget->dirty,
            widget->color.alpha
        LOG_END
    }

    childCount = 0;
    for (node = widget->tree.child; node; node = node->sibling)
        children[childCount++] = (ITUWidget *)node;

    if (ev == ITU_EVENT_KEYDOWN || ev == ITU_EVENT_KEYUP)
    {
        if (ituWidgetIsEnabled(widget))
        {
            while (--childCount >= 0)
            {
                ITUWidget *child = children[childCount];

                if (ituWidgetIsVisible(child))
                {
                    result |= ituWidgetUpdate(child, ev, arg1, arg2, arg3);
                    if (result)
                        break;
                }
            }

            if (ituWidgetIsActive(widget) && !result)
            {
                result = ituWidgetOnPress(widget, ev, arg1, arg2, arg3);
            }
        }
    }
    else if (ev == ITU_EVENT_MOUSEDOWN)
    {
        if (ituWidgetIsEnabled(widget))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            while (--childCount >= 0)
            {
                ITUWidget *child = children[childCount];

                if (ituWidgetIsVisible(child))
                {
                    result |= ituWidgetUpdate(child, ev, arg1, x, y);
                    if (result)
                        break;
                }
            }

            if (!result && ituWidgetIsInside(widget, x, y))
            {
                result = ituWidgetOnPress(widget, ev, arg1, x, y);
            }
        }
    }
    else if (ev == ITU_EVENT_MOUSEUP)
    {
        int x = arg2 - widget->rect.x;
        int y = arg3 - widget->rect.y;

        while (--childCount >= 0)
        {
            ITUWidget *child = children[childCount];
            result |= ituWidgetUpdate(child, ev, arg1, x, y);
        }
    }
    else if (ev == ITU_EVENT_MOUSEMOVE)
    {
        if (ituWidgetIsEnabled(widget))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            while (--childCount >= 0)
            {
                ITUWidget *child = children[childCount];
                if (ituWidgetIsVisible(child))
                {
                    result |= ituWidgetUpdate(child, ev, arg1, x, y);
                }
            }
        }
    }
    else if (ev == ITU_EVENT_MOUSEDOUBLECLICK 
          || ev == ITU_EVENT_MOUSELONGPRESS 
          || ev == ITU_EVENT_TOUCHSLIDELEFT 
          || ev == ITU_EVENT_TOUCHSLIDEUP 
          || ev == ITU_EVENT_TOUCHSLIDERIGHT 
          || ev == ITU_EVENT_TOUCHSLIDEDOWN 
          || ev == ITU_EVENT_TOUCHPINCH)
    {
        if (ituWidgetIsEnabled(widget))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            while (--childCount >= 0)
            {
                ITUWidget *child = children[childCount];

                if (ituWidgetIsVisible(child))
                {
                    result |= ituWidgetUpdate(child, ev, arg1, x, y);
                    if (result)
                        break;
                }
            }
        }
    }
    else
    {
        if (ev == ITU_EVENT_TIMER && ituWidgetIsEffecting(widget))
        {
            if (ituEffectIsPlaying(widget->effect))
            {
                ituEffectUpdate(widget->effect, widget);
            }
            else
            {
                if (widget->state == ITU_STATE_SHOWING)
                {
                    if (widget->hideDelay != -1)
                        ituSceneExecuteCommand(ituScene, widget->hideDelay, WidgetHide, (int)widget);
                }
                else if (widget->state == ITU_STATE_HIDING)
                {
                    widget->visible = false;
                    ituWidgetUpdate(widget, ITU_EVENT_RELEASE, 0, 0, 0);
                }
                ituEffectStop(widget->effect, widget);
                ituEffectExit(widget->effect);
                free(widget->effect);
                widget->effect = NULL;
                widget->state = ITU_STATE_NORMAL;
                ituWidgetSetEffect(widget, ITU_STATE_NORMAL, 0);
            }
            widget->dirty = true;
        }

        for (node = widget->tree.child; node; node = node->sibling)
        {
            ITUWidget* child = (ITUWidget*)node;

            if (child->visible || (ev == ITU_EVENT_LOAD && child->visible) || ev == ITU_EVENT_RELEASE || ev == ITU_EVENT_LANGUAGE || ev == ITU_EVENT_LOAD_IMAGE || ev == ITU_EVENT_LAYOUT || ev == ITU_EVENT_TIMER)
            {
                widget->dirty |= ituWidgetUpdate(child, ev, arg1, arg2, arg3);
                //if (widget->dirty)
                //    printf("widget=%s\n", child->name);
            }
        }
        result = widget->dirty;
    }
    return widget->visible ? result : false;
}

void ituWidgetDrawImpl(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    ITCTree* node;
    ITURectangle prevClip;

    assert(widget);
    assert(dest);

    LOG_DRAW "%s{%d,%d,%d,%d} t:%d v:%d, ac:%d d:%d a:%d\n",
        widget->name,
        widget->rect.x,
        widget->rect.y,
        widget->rect.width,
        widget->rect.height,
        widget->type,
        widget->visible,
        widget->active,
        widget->dirty,
        widget->color.alpha
    LOG_END

    if (!widget->visible)
        return;

    ituWidgetSetClipping(widget, dest, x, y, &prevClip);

    x += widget->rect.x;
    y += widget->rect.y;
    alpha = alpha * widget->alpha / 255;

    for (node = widget->tree.child; node; node = node->sibling)
    {
        ITUWidget* child = (ITUWidget*)node;
        if (child->visible && ituWidgetIsOverlapClipping(child, dest, x, y))
            ituWidgetDraw(node, dest, x, y, alpha);

        child->dirty = false;
    }
    ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
}

static void WidgetShow(int arg)
{
    ITUWidget* widget = (ITUWidget*)arg;
    assert(widget);

    ituWidgetSetVisible(widget, true);
}

void ituWidgetOnActionImpl(ITUWidget* widget, ITUActionType action, char* param)
{
    assert(widget);

    switch (action)
    {
    case ITU_ACTION_SHOW:
        if (param[0] != '\0')
        {
            ITUEffectType effect, oldEffect = widget->effects[ITU_STATE_SHOWING];
            char buf[ITU_ACTION_PARAM_SIZE], *ptr, *saveptr;
            int oldStep = widget->effects[ITU_STATE_NORMAL];

            strcpy(buf, param);
            effect = atoi(strtok_r(buf, " ", &saveptr));
            ptr = strtok_r(NULL, " ", &saveptr);
            if (ptr)
                ituWidgetSetEffect(widget, ITU_STATE_NORMAL, atoi(ptr));    // use widget->effects[ITU_STATE_NORMAL] to store step parameter

            ituWidgetSetEffect(widget, ITU_STATE_SHOWING, effect);

            ituWidgetSetVisible(widget, true);

            widget->effects[ITU_STATE_SHOWING] = oldEffect;
            widget->effects[ITU_STATE_NORMAL] = oldStep;
        }
        else
        {
            if (widget->showDelay == 0)
            {
                ituWidgetSetVisible(widget, true);
            }
            else
            {
                ituSceneExecuteCommand(ituScene, widget->showDelay, WidgetShow, (int)widget);
            }
        }
        break;

    case ITU_ACTION_HIDE:
        if (param[0] != '\0')
        {
            ITUEffectType effect, oldEffect = widget->effects[ITU_STATE_HIDING];
            char buf[ITU_ACTION_PARAM_SIZE], *ptr, *saveptr;
            int oldStep = widget->effects[ITU_STATE_NORMAL];

            strcpy(buf, param);
            effect = atoi(strtok_r(buf, " ", &saveptr));
            ptr = strtok_r(NULL, " ", &saveptr);
            if (ptr)
                ituWidgetSetEffect(widget, ITU_STATE_NORMAL, atoi(ptr));    // use widget->effects[ITU_STATE_NORMAL] to store step parameter

            ituWidgetSetEffect(widget, ITU_STATE_HIDING, effect);

            ituWidgetSetVisible(widget, false);

            widget->effects[ITU_STATE_HIDING] = oldEffect;
            widget->effects[ITU_STATE_NORMAL] = oldStep;
        }
        else
        {
            ituWidgetSetVisible(widget, false);
        }
        break;

    case ITU_ACTION_FOCUS:
        ituFocusWidget(widget);
        break;

    case ITU_ACTION_ENABLE:
        ituWidgetEnable(widget);
        break;

    case ITU_ACTION_DISABLE:
        ituWidgetDisable(widget);
        break;

    default:
		printf("================================\n");
		printf("[widget %s assert]\n", widget->name);
		printf("================================\n");
        assert(0);
        break;
    }
}

bool ituWidgetOnPressImpl(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    return false;
}

void ituWidgetInit(ITUWidget* widget)
{
    assert(widget);

    memset(widget, 0, sizeof (ITUWidget));
    ITU_ASSERT_THREAD();

    widget->type        = ITU_WIDGET;
    strcpy(widget->name, widgetName);

    widget->Exit        = ituWidgetExitImpl;
    widget->Clone       = ituWidgetCloneImpl;
    widget->Update      = ituWidgetUpdateImpl;
    widget->Draw        = ituWidgetDrawImpl;
    widget->OnAction    = ituWidgetOnActionImpl;
    widget->OnPress     = ituWidgetOnPressImpl;

    widget->visible     = true;
    widget->dirty       = true;
    widget->color.alpha = 255;

    ituWidgetEnable(widget);
}

void ituWidgetLoad(ITUWidget* widget, uint32_t base)
{
    assert(widget);

    LOG_LOAD "%s{%d,%d,%d,%d} t:%d v:%d, ac:%d d:%d a:%d\n",
        widget->name,
        widget->rect.x,
        widget->rect.y,
        widget->rect.width,
        widget->rect.height,
        widget->type,
        widget->visible,
        widget->active,
        widget->dirty,
        widget->color.alpha
    LOG_END

    if (widget->tree.parent)
        widget->tree.parent = (ITCTree*)((uint32_t)widget->tree.parent + base);

    if (widget->tree.sibling)
        widget->tree.sibling = (ITCTree*)((uint32_t)widget->tree.sibling + base);

    if (widget->tree.child)
        widget->tree.child = (ITCTree*)((uint32_t)widget->tree.child + base);

    widget->effect      = NULL;
    widget->Exit        = ituWidgetExitImpl;
    widget->Clone       = ituWidgetCloneImpl;
    widget->Update      = ituWidgetUpdateImpl;
    widget->Draw        = ituWidgetDrawImpl;
    widget->OnAction    = ituWidgetOnActionImpl;
    widget->OnPress     = ituWidgetOnPressImpl;
}

void ituWidgetSetNameImpl(ITUWidget* widget, const char* name)
{
    assert(widget);
    assert(name);
    ITU_ASSERT_THREAD();

    strncpy(widget->name, name, ITU_WIDGET_NAME_SIZE - 1);
    widget->name[ITU_WIDGET_NAME_SIZE - 1] = '\0';
}

void ituWidgetSetVisibleImpl(ITUWidget* widget, bool visible)
{
    assert(widget);
    ITU_ASSERT_THREAD();

    if (!visible && (widget->flags & ITU_ALWAYS_VISIBLE))
    {
        return;
    }
    else if (widget->effect)
    {
        if (widget->state == ITU_STATE_HIDING)
        {
            widget->visible = false;
            ituWidgetUpdate(widget, ITU_EVENT_RELEASE, 0, 0, 0);
        }
        ituEffectStop(widget->effect, widget);
        ituEffectExit(widget->effect);
        free(widget->effect);
        widget->effect = NULL;
        widget->state = ITU_STATE_NORMAL;
        ituWidgetSetEffect(widget, ITU_STATE_NORMAL, 0);
    }
    else if (widget->visible == visible)
    {
        return;
    }

    if (visible)
    {
        switch (widget->effects[ITU_STATE_SHOWING])
        {
        case ITU_EFFECT_FADE:
            widget->effect = malloc(sizeof (ITUFadeEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];
                
                ituFadeEffectInit((ITUFadeEffect*)widget->effect, true);
                
                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_SHOWING;
            }
            break;

        case ITU_EFFECT_SCROLL_LEFT:
            widget->effect = malloc(sizeof (ITUScrollEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituScrollEffectInit((ITUScrollEffect*)widget->effect, ITU_SCROLL_IN_LEFT);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_SHOWING;
            }
            break;

        case ITU_EFFECT_SCROLL_UP:
            widget->effect = malloc(sizeof (ITUScrollEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituScrollEffectInit((ITUScrollEffect*)widget->effect, ITU_SCROLL_IN_UP);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_SHOWING;
            }
            break;

        case ITU_EFFECT_SCROLL_RIGHT:
            widget->effect = malloc(sizeof (ITUScrollEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituScrollEffectInit((ITUScrollEffect*)widget->effect, ITU_SCROLL_IN_RIGHT);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_SHOWING;
            }
            break;

        case ITU_EFFECT_SCROLL_DOWN:
            widget->effect = malloc(sizeof (ITUScrollEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituScrollEffectInit((ITUScrollEffect*)widget->effect, ITU_SCROLL_IN_DOWN);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_SHOWING;
            }
            break;

        case ITU_EFFECT_SCROLL_LEFT_FADE:
            widget->effect = malloc(sizeof (ITUScrollFadeEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituScrollFadeEffectInit((ITUScrollFadeEffect*)widget->effect, ITU_SCROLL_IN_LEFT, true);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_SHOWING;
            }
            break;

        case ITU_EFFECT_SCROLL_UP_FADE:
            widget->effect = malloc(sizeof (ITUScrollFadeEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituScrollFadeEffectInit((ITUScrollFadeEffect*)widget->effect, ITU_SCROLL_IN_UP, true);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_SHOWING;
            }
            break;

        case ITU_EFFECT_SCROLL_RIGHT_FADE:
            widget->effect = malloc(sizeof (ITUScrollFadeEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituScrollFadeEffectInit((ITUScrollFadeEffect*)widget->effect, ITU_SCROLL_IN_RIGHT, true);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_SHOWING;
            }
            break;

        case ITU_EFFECT_SCROLL_DOWN_FADE:
            widget->effect = malloc(sizeof (ITUScrollFadeEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituScrollFadeEffectInit((ITUScrollFadeEffect*)widget->effect, ITU_SCROLL_IN_DOWN, true);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_SHOWING;
            }
            break;

        case ITU_EFFECT_SCALE:
            widget->effect = malloc(sizeof (ITUScaleEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituScaleEffectInit((ITUScaleEffect*)widget->effect, true);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_SHOWING;
            }
            break;

        case ITU_EFFECT_SCALE_FADE:
            widget->effect = malloc(sizeof (ITUScaleFadeEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituScaleFadeEffectInit((ITUScaleFadeEffect*)widget->effect, true);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_SHOWING;
            }
            break;

        case ITU_EFFECT_WIPE_LEFT:
            widget->effect = malloc(sizeof (ITUWipeEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituWipeEffectInit((ITUWipeEffect*)widget->effect, ITU_WIPE_IN_LEFT);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_SHOWING;
            }
            break;

        case ITU_EFFECT_WIPE_UP:
            widget->effect = malloc(sizeof (ITUWipeEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituWipeEffectInit((ITUWipeEffect*)widget->effect, ITU_WIPE_IN_UP);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_SHOWING;
            }
            break;

        case ITU_EFFECT_WIPE_RIGHT:
            widget->effect = malloc(sizeof (ITUWipeEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituWipeEffectInit((ITUWipeEffect*)widget->effect, ITU_WIPE_IN_RIGHT);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_SHOWING;
            }
            break;

        case ITU_EFFECT_WIPE_DOWN:
            widget->effect = malloc(sizeof (ITUWipeEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituWipeEffectInit((ITUWipeEffect*)widget->effect, ITU_WIPE_IN_DOWN);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_SHOWING;
            }
            break;

        default:
            break;
        }
        widget->visible = true;
        ituWidgetUpdate(widget, ITU_EVENT_LAYOUT, 0, 0, 0);
        ituWidgetUpdate(widget, ITU_EVENT_LOAD, 0, 0, 0);
        ituWidgetUpdate(widget, ITU_EVENT_LOAD_EXTERNAL, 0, 0, 0);

        if (widget->state != ITU_STATE_SHOWING && widget->hideDelay != -1)
            ituSceneExecuteCommand(ituScene, widget->hideDelay, WidgetHide, (int)widget);
    }
    else
    {
        switch (widget->effects[ITU_STATE_HIDING])
        {
        case ITU_EFFECT_FADE:
            widget->effect = malloc(sizeof (ITUFadeEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituFadeEffectInit((ITUFadeEffect*)widget->effect, false);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_HIDING;
            }
            break;

        case ITU_EFFECT_SCROLL_LEFT:
            widget->effect = malloc(sizeof (ITUScrollEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituScrollEffectInit((ITUScrollEffect*)widget->effect, ITU_SCROLL_OUT_LEFT);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_HIDING;
            }
            break;

        case ITU_EFFECT_SCROLL_UP:
            widget->effect = malloc(sizeof (ITUScrollEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituScrollEffectInit((ITUScrollEffect*)widget->effect, ITU_SCROLL_OUT_UP);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_HIDING;
            }
            break;

        case ITU_EFFECT_SCROLL_RIGHT:
            widget->effect = malloc(sizeof (ITUScrollEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituScrollEffectInit((ITUScrollEffect*)widget->effect, ITU_SCROLL_OUT_RIGHT);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_HIDING;
            }
            break;

        case ITU_EFFECT_SCROLL_DOWN:
            widget->effect = malloc(sizeof (ITUScrollEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituScrollEffectInit((ITUScrollEffect*)widget->effect, ITU_SCROLL_OUT_DOWN);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_HIDING;
            }
            break;

        case ITU_EFFECT_SCROLL_LEFT_FADE:
            widget->effect = malloc(sizeof (ITUScrollFadeEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituScrollFadeEffectInit((ITUScrollFadeEffect*)widget->effect, ITU_SCROLL_OUT_LEFT, false);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_HIDING;
            }
            break;

        case ITU_EFFECT_SCROLL_UP_FADE:
            widget->effect = malloc(sizeof (ITUScrollFadeEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituScrollFadeEffectInit((ITUScrollFadeEffect*)widget->effect, ITU_SCROLL_OUT_UP, false);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_HIDING;
            }
            break;

        case ITU_EFFECT_SCROLL_RIGHT_FADE:
            widget->effect = malloc(sizeof (ITUScrollFadeEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituScrollFadeEffectInit((ITUScrollFadeEffect*)widget->effect, ITU_SCROLL_OUT_RIGHT, false);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_HIDING;
            }
            break;

        case ITU_EFFECT_SCROLL_DOWN_FADE:
            widget->effect = malloc(sizeof (ITUScrollFadeEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituScrollFadeEffectInit((ITUScrollFadeEffect*)widget->effect, ITU_SCROLL_OUT_DOWN, false);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_HIDING;
            }
            break;

        case ITU_EFFECT_SCALE:
            widget->effect = malloc(sizeof (ITUScaleEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituScaleEffectInit((ITUScaleEffect*)widget->effect, false);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_HIDING;
            }
            break;

        case ITU_EFFECT_SCALE_FADE:
            widget->effect = malloc(sizeof (ITUScaleFadeEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituScaleFadeEffectInit((ITUScaleFadeEffect*)widget->effect, false);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_HIDING;
            }
            break;

        case ITU_EFFECT_WIPE_LEFT:
            widget->effect = malloc(sizeof (ITUWipeEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituWipeEffectInit((ITUWipeEffect*)widget->effect, ITU_WIPE_OUT_LEFT);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_HIDING;
            }
            break;

        case ITU_EFFECT_WIPE_UP:
            widget->effect = malloc(sizeof (ITUWipeEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituWipeEffectInit((ITUWipeEffect*)widget->effect, ITU_WIPE_OUT_UP);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_HIDING;
            }
            break;

        case ITU_EFFECT_WIPE_RIGHT:
            widget->effect = malloc(sizeof (ITUWipeEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituWipeEffectInit((ITUWipeEffect*)widget->effect, ITU_WIPE_OUT_RIGHT);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_HIDING;
            }
            break;

        case ITU_EFFECT_WIPE_DOWN:
            widget->effect = malloc(sizeof (ITUWipeEffect));
            if (widget->effect)
            {
                int step = widget->effects[ITU_STATE_NORMAL];

                ituWipeEffectInit((ITUWipeEffect*)widget->effect, ITU_WIPE_OUT_DOWN);

                if (step > 0)
                    ituEffectSetTotalStep(widget->effect, step);
                else
                    ituEffectSetTotalStep(widget->effect, widget->effectSteps);

                ituEffectStart(widget->effect, widget);
                ituEffectUpdate(widget->effect, widget);
                widget->state = ITU_STATE_HIDING;
            }
            break;

        default:
            widget->visible = false;
            ituWidgetUpdate(widget, ITU_EVENT_RELEASE, 0, 0, 0);
            if (widget->tree.parent)
                ituWidgetSetDirty(widget->tree.parent, true);
            break;
        }
    }
    widget->dirty = true;
}

void ituWidgetSetActiveImpl(ITUWidget* widget, bool active)
{
    assert(widget);
    ITU_ASSERT_THREAD();

    widget->active  = active;
    widget->dirty   = true;
}

void ituWidgetSetAlphaImpl(ITUWidget* widget, uint8_t alpha)
{
    assert(widget);
    ITU_ASSERT_THREAD();

    widget->alpha   = alpha;
    widget->dirty   = true;
}


void ituWidgetSetColorImpl(ITUWidget* widget, uint8_t alpha, uint8_t red, uint8_t green, uint8_t blue)
{
    assert(widget);
    ITU_ASSERT_THREAD();

    widget->color.alpha = alpha;
    widget->color.red   = red;
    widget->color.green = green;
    widget->color.blue  = blue;
    widget->dirty       = true;
}

void ituWidgetSetXImpl(ITUWidget* widget, int x)
{
    assert(widget);
    ITU_ASSERT_THREAD();

    widget->rect.x  = x;
    widget->dirty   = true;
}

void ituWidgetSetYImpl(ITUWidget* widget, int y)
{
    assert(widget);
    ITU_ASSERT_THREAD();

    widget->rect.y  = y;
    widget->dirty   = true;
}

void ituWidgetSetPositionImpl(ITUWidget* widget, int x, int y)
{
    assert(widget);
    ITU_ASSERT_THREAD();

    widget->rect.x  = x;
    widget->rect.y  = y;
    widget->dirty   = true;
}

void ituWidgetSetWidthImpl(ITUWidget* widget, int width)
{
    assert(widget);
    ITU_ASSERT_THREAD();

    widget->rect.width  = width;
    widget->dirty       = true;

    ituWidgetUpdate(widget, ITU_EVENT_LAYOUT, 0, 0, 0);
}

void ituWidgetSetHeightImpl(ITUWidget* widget, int height)
{
    assert(widget);
    ITU_ASSERT_THREAD();

    widget->rect.height = height;
    widget->dirty       = true;

    ituWidgetUpdate(widget, ITU_EVENT_LAYOUT, 0, 0, 0);
}

void ituWidgetSetDimensionImpl(ITUWidget* widget, int width, int height)
{
    assert(widget);
    ITU_ASSERT_THREAD();

    widget->rect.width  = width;
    widget->rect.height = height;
    widget->dirty       = true;

    ituWidgetUpdate(widget, ITU_EVENT_LAYOUT, 0, 0, 0);
}

void ituWidgetSetBoundImpl(ITUWidget* widget, int x, int y, int width, int height)
{
    assert(widget);
    ITU_ASSERT_THREAD();

    widget->bound.x         = x;
    widget->bound.y         = y;
    widget->bound.width     = width;
    widget->bound.height    = height;
    widget->dirty           = true;
}

bool ituWidgetIsInsideImpl(ITUWidget* widget, int x, int y)
{
    int bw, bh;

    ITU_ASSERT_THREAD();

    if (!widget->visible)
        return false;

    bw = widget->bound.width ? widget->bound.width : widget->rect.width;
    bh = widget->bound.height ? widget->bound.height : widget->rect.height;

    if (x >= 0 && y >= 0 && x <= bw && y <= bh)
    {
        return true;
    }
    return false;
}

void ituWidgetSetClipping(ITUWidget* widget, ITUSurface* dest, int x, int y, ITURectangle* prevClip)
{
    int cx, cy, cw, ch;

    assert(widget);
    assert(dest);
    assert(prevClip);
    ITU_ASSERT_THREAD();

    memcpy(prevClip, &dest->clipping, sizeof(ITURectangle));

    if (dest->flags & ITU_CLIPPING)
    {   
        int left, top, right, bottom, r1, r2;

        if (widget->bound.width > 0 || widget->bound.height > 0)
        {
            x += widget->bound.x;
            y += widget->bound.y;

            left = x > prevClip->x ? x : prevClip->x;

            r1 = x + widget->bound.width;
            r2 = prevClip->x + prevClip->width;
            right = r1 < r2 ? r1 : r2;

            r1 = y + widget->bound.height;
            r2 = prevClip->y + prevClip->height;
            bottom = r1 < r2 ? r1 : r2;

            top = y > prevClip->y ? y : prevClip->y;
        }
        else
        {
            x += widget->rect.x;
            y += widget->rect.y;

            left = x > prevClip->x ? x : prevClip->x;

            r1 = x + widget->rect.width;
            r2 = prevClip->x + prevClip->width;
            right = r1 < r2 ? r1 : r2;

            r1 = y + widget->rect.height;
            r2 = prevClip->y + prevClip->height;
            bottom = r1 < r2 ? r1 : r2;

            top = y > prevClip->y ? y : prevClip->y;
        }

        cx = left;
        cy = top;
        cw = right - left;
        ch = bottom - top;

        if (cw < 0)
            cw = 0;

        if (ch < 0)
            ch = 0;

        ituSurfaceSetClipping(dest, cx, cy, cw, ch);
    }
    else
    {
        if (widget->bound.width > 0 || widget->bound.height > 0)
        {
            x += widget->bound.x;
            y += widget->bound.y;
            ituSurfaceSetClipping(dest, x, y, widget->bound.width, widget->bound.height);
        }
        else
        {
            x += widget->rect.x;
            y += widget->rect.y;
            ituSurfaceSetClipping(dest, x, y, widget->rect.width, widget->rect.height);
        }
    }
}

static bool ValueInRange(int value, int min, int max)
{
    return (value >= min) && (value <= max);
}

static bool RectOverlap(ITURectangle* A, ITURectangle* B)
{
    bool xOverlap = ValueInRange(A->x, B->x, B->x + B->width) ||
                    ValueInRange(B->x, A->x, A->x + A->width);

    bool yOverlap = ValueInRange(A->y, B->y, B->y + B->height) ||
                    ValueInRange(B->y, A->y, A->y + A->height);

    return xOverlap && yOverlap;
}

bool ituWidgetIsOverlapClipping(ITUWidget* widget, ITUSurface* dest, int x, int y)
{
    ITURectangle A, B;

    if (widget->bound.width > 0)
    {
        A.x = x + widget->bound.x;
        A.y = y + widget->bound.y;
        A.width = widget->bound.width;
        A.height = widget->bound.height;
    }
    else
    {
        A.x = x + widget->rect.x;
        A.y = y + widget->rect.y;
        A.width = widget->rect.width;
        A.height = widget->rect.height;
    }

    if (dest->flags & ITU_CLIPPING)
    {   
        B.x = dest->clipping.x;
        B.y = dest->clipping.y;
        B.width = dest->clipping.width;
        B.height = dest->clipping.height;
    }
    else
    {
        B.x = 0;
        B.y = 0;
        B.width = dest->width;
        B.height = dest->height;
    }

    return RectOverlap(&A, &B);
}

void ituWidgetGetGlobalPositionImpl(ITUWidget* widget, int* x, int* y)
{
    int cx = 0;
    int cy = 0;

    assert(widget);
    ITU_ASSERT_THREAD();

    while (widget)
    {
        cx += widget->rect.x;
        cy += widget->rect.y;

        widget = (ITUWidget*) widget->tree.parent;
    }

    if (x)
        *x = cx;

    if (y)
        *y = cy;
}

void ituWidgetShowImpl(ITUWidget* widget, ITUEffectType effect, int step)
{
    ITUEffectType oldEffect = widget->effects[ITU_STATE_SHOWING];
    int oldStep = widget->effects[ITU_STATE_NORMAL];
    assert(widget);
    ITU_ASSERT_THREAD();

    ituWidgetSetEffect(widget, ITU_STATE_NORMAL, step);    // use widget->effects[ITU_STATE_NORMAL] to store step parameter
    ituWidgetSetEffect(widget, ITU_STATE_SHOWING, effect);
    ituWidgetSetVisible(widget, true);
    widget->effects[ITU_STATE_SHOWING] = oldEffect;
    widget->effects[ITU_STATE_NORMAL] = oldStep;
}

void ituWidgetHideImpl(ITUWidget* widget, ITUEffectType effect, int step)
{
    ITUEffectType oldEffect = widget->effects[ITU_STATE_HIDING];
    int oldStep = widget->effects[ITU_STATE_NORMAL];
    assert(widget);
    ITU_ASSERT_THREAD();

    ituWidgetSetEffect(widget, ITU_STATE_NORMAL, step);    // use widget->effects[ITU_STATE_NORMAL] to store step parameter
    ituWidgetSetEffect(widget, ITU_STATE_HIDING, effect);
    ituWidgetSetVisible(widget, false);
    widget->effects[ITU_STATE_HIDING] = oldEffect;
    widget->effects[ITU_STATE_NORMAL] = oldStep;

}
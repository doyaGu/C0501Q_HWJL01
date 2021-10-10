#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char tbarName[] = "ITUTrackBar";

bool ituTrackBarUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result;
    ITUTrackBar* tbar = (ITUTrackBar*) widget;
    assert(tbar);

    if (ev == ITU_EVENT_MOUSEMOVE)
    {
        if (ituWidgetIsEnabled(widget) && tbar->tracker && ituButtonIsPressed(tbar->tracker))
        {
            if (tbar->layout == ITU_LAYOUT_HORIZONTAL)
            {
                int x = arg2 - widget->rect.x;

                if (tbar->max > tbar->min && tbar->step > 0)
                {
                    int i, value;
                    float sw = (float)(ituWidgetGetWidth(tbar) - tbar->gap * 2) / (tbar->max - tbar->min);

                    if (x < tbar->gap)
                    {
                        value = tbar->min;
                    }
                    else if (x > (int)(sw * (tbar->max - tbar->min)) + tbar->gap)
                    {
                        value = tbar->max;
                    }
                    else
                    {
                        value = tbar->min;
                        for (i = tbar->min; i <= tbar->max; i += tbar->step)
                        {
                            int v0 = (int)(sw * (i - tbar->min)) + tbar->gap;
                            int v1 = (int)(sw * (i + tbar->step - tbar->min)) + tbar->gap;

                            if (v0 <= x && x <= v1)
                            {
                                int diffLeft = x - v0;
                                int diffRight = v1 - x;
                                if (diffLeft < diffRight)
                                    value = i;
                                else
                                    value = i + tbar->step;

                                //printf("x=%d v0=%d v1=%d l=%d r=%d val=%d\n", x, v0, v1, diffLeft, diffRight, value);
                                break;
                            }
                        }
                    }

                    if (tbar->value != value)
                    {
                        ituTrackBarSetValue(tbar, value);
                        ituTrackBarOnValueChanged(tbar, value, false);
                        ituExecActions((ITUWidget*)tbar, tbar->actions, ITU_EVENT_CHANGED, value);
                    }
                }
            }
            else //if (tbar->layout == ITU_LAYOUT_VERTICAL)
            {
                int y = arg3 - widget->rect.y;

                if (tbar->max > tbar->min && tbar->step > 0)
                {
                    int i, value;
                    float sh = (float)(ituWidgetGetHeight(tbar) - tbar->gap * 2) / (tbar->max - tbar->min);

                    if (y > ituWidgetGetHeight(tbar) + tbar->gap)
                    {
                        value = tbar->min;
                    }
                    else if (y < tbar->gap)
                    {
                        value = tbar->max;
                    }
                    else
                    {
                        value = tbar->min;
                        for (i = tbar->min; i <= tbar->max; i += tbar->step)
                        {
                            int v0 = ituWidgetGetHeight(tbar) - (int)(sh * (i - tbar->min)) + tbar->gap;
                            int v1 = ituWidgetGetHeight(tbar) - (int)(sh * (i + tbar->step - tbar->min)) + tbar->gap;

                            if (v0 >= y && y >= v1)
                            {
                                int diffBottom = v0 - y;
                                int diffTop = y - v1;
                                if (diffBottom < diffTop)
                                    value = i;
                                else
                                    value = i + tbar->step;

                                //printf("y=%d v0=%d v1=%d b=%d t=%d val=%d\n", y, v0, v1, diffBottom, diffTop, value);
                                break;
                            }
                        }
                    }

                    if (tbar->value != value)
                    {
                        ituTrackBarSetValue(tbar, value);
                        ituTrackBarOnValueChanged(tbar, value, false);
                        ituExecActions((ITUWidget*)tbar, tbar->actions, ITU_EVENT_CHANGED, value);
                    }
                }
            }
        }
    }

    if (ev == ITU_EVENT_MOUSEDOWN)
    {
        if (ituWidgetIsEnabled(widget))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (tbar->tracker && ituButtonIsPressed(tbar->tracker))
            {
                ituExecActions((ITUWidget*)tbar, tbar->actions, ITU_EVENT_CHANGED, tbar->value);
                ituTrackBarOnValueChanged(tbar, tbar->value, true);
            }
			else if (!(widget->flags & ITU_PROGRESS))
            {
                if (ituWidgetIsInside(widget, x, y))
                {
                    if (tbar->layout == ITU_LAYOUT_HORIZONTAL)
                    {
                        if (tbar->max > tbar->min && tbar->step > 0)
                        {
                            int i, value;
                            float sw = (float)(ituWidgetGetWidth(tbar) - tbar->gap * 2) / (tbar->max - tbar->min);

                            if (x < tbar->gap)
                            {
                                value = tbar->min;
                            }
                            else if (x > (int)(sw * (tbar->max - tbar->min)) + tbar->gap)
                            {
                                value = tbar->max;
                            }
                            else
                            {
                                value = tbar->min;
                                for (i = tbar->min; i <= tbar->max; i += tbar->step)
                                {
                                    int v0 = (int)(sw * (i - tbar->min)) + tbar->gap;
                                    int v1 = (int)(sw * (i + tbar->step - tbar->min)) + tbar->gap;

                                    if (v0 <= x && x <= v1)
                                    {
                                        int diffLeft = x - v0;
                                        int diffRight = v1 - x;
                                        if (diffLeft < diffRight)
                                            value = i;
                                        else
                                            value = i + tbar->step;

                                        //printf("x=%d v0=%d v1=%d l=%d r=%d val=%d\n", x, v0, v1, diffLeft, diffRight, value);
                                        break;
                                    }
                                }
                            }

                            if (tbar->value != value)
                            {
                                ituTrackBarSetValue(tbar, value);
                                ituTrackBarOnValueChanged(tbar, value, true);
                                ituExecActions((ITUWidget*)tbar, tbar->actions, ITU_EVENT_CHANGED, value);
                            }
                        }
                    }
                    else // if (bar->layout == ITU_LAYOUT_VERTICAL)
                    {
                        if (tbar->max > tbar->min && tbar->step > 0)
                        {
                            int i, value;
                            float sh = (float)(ituWidgetGetHeight(tbar) - tbar->gap * 2) / (tbar->max - tbar->min);

                            if (y > ituWidgetGetHeight(tbar) + tbar->gap)
                            {
                                value = tbar->min;
                            }
                            else if (y < tbar->gap)
                            {
                                value = tbar->max;
                            }
                            else
                            {
                                value = tbar->min;
                                for (i = tbar->min; i <= tbar->max; i += tbar->step)
                                {
                                    int v0 = ituWidgetGetHeight(tbar) - (int)(sh * (i - tbar->min)) + tbar->gap;
                                    int v1 = ituWidgetGetHeight(tbar) - (int)(sh * (i + tbar->step - tbar->min)) + tbar->gap;

                                    if (v0 >= y && y >= v1)
                                    {
                                        int diffBottom = v0 - y;
                                        int diffTop = y - v1;
                                        if (diffBottom < diffTop)
                                            value = i;
                                        else
                                            value = i + tbar->step;

                                        //printf("y=%d v0=%d v1=%d b=%d t=%d val=%d\n", y, v0, v1, diffBottom, diffTop, value);
                                        break;
                                    }
                                }
                            }

                            if (tbar->value != value)
                            {
                                ituTrackBarSetValue(tbar, value);
                                ituTrackBarOnValueChanged(tbar, value, true);
                                ituExecActions((ITUWidget*)tbar, tbar->actions, ITU_EVENT_CHANGED, value);
                            }
                        }
                    }
                    if (tbar->tracker)
                    {
                        ituButtonSetPressed(tbar->tracker, true);
                        ((ITUWidget*)tbar->tracker)->flags |= ITU_DRAGGING;
                        ituScene->dragged = (ITUWidget*)tbar->tracker;
                    }
                    result = widget->dirty = true;
                }
            }
			if (tbar->tip && !(widget->flags & ITU_PROGRESS) && ituWidgetIsInside(widget, x, y))
                ituWidgetSetVisible(tbar->tip, true);
        }
    }
    else if (ev == ITU_EVENT_MOUSEUP)
    {
        if (ituWidgetIsEnabled(widget))
        {
            if (tbar->tip)
            {
                ituWidgetSetVisible(tbar->tip, false);
            }

            if (tbar->tracker && ituScene->dragged == (ITUWidget*)tbar->tracker)
            {
                ((ITUWidget*)tbar->tracker)->flags &= ~ITU_DRAGGING;
                ituScene->dragged = NULL;

                ituTrackBarOnValueChanged(tbar, tbar->value, true);
                ituExecActions((ITUWidget*)tbar, tbar->actions, ITU_EVENT_CHANGED, tbar->value);
            }
        }
    }
    else if (ev == ITU_EVENT_RELEASE)
    {
        if (ituWidgetIsEnabled(widget))
        {
            if (tbar->tracker && tbar->tip)
            {
                ituButtonSetPressed(tbar->tracker, false);
                ituWidgetSetVisible(tbar->tip, false);
            }
        }
    }
    else if (ev == ITU_EVENT_MOUSELONGPRESS)
    {
        if (ituWidgetIsEnabled(widget))
        {
            if (tbar->tracker && ituButtonIsPressed(tbar->tracker))
            {
                int x = arg2 - widget->rect.x;
                int y = arg3 - widget->rect.y;

                if (ituWidgetIsInside(widget, x, y))
                {
                    result = widget->dirty = true;
                    return widget->visible ? result : false;
                }
            }
        }
    }

    result = ituIconUpdate(widget, ev, arg1, arg2, arg3);

    if (ev == ITU_EVENT_LAYOUT)
    {
        if (!tbar->tracker && (tbar->trackerName[0] != '\0'))
        {
            tbar->tracker = (ITUButton*) ituSceneFindWidget(ituScene, tbar->trackerName);
        }

        if (tbar->tracker)
        {
            if (tbar->layout == ITU_LAYOUT_HORIZONTAL)
            {
                int w;
                float sw;

                w = ituWidgetGetWidth(tbar->tracker) / 2;
                if (tbar->max > tbar->min && tbar->step > 0)
                    sw = (float)(ituWidgetGetWidth(tbar) - tbar->gap * 2) / (tbar->max - tbar->min);
                else
                    sw = 0.0f;

                ituWidgetSetX(tbar->tracker, (int)(sw * (tbar->value - tbar->min)) - w + tbar->gap);
            }
            else //if (tbar->layout == ITU_LAYOUT_VERTICAL)
            {
                int h;
                float sh;

                h = ituWidgetGetHeight(tbar->tracker) / 2;
                if (tbar->max > tbar->min && tbar->step > 0)
                    sh = (float)(ituWidgetGetHeight(tbar) - tbar->gap * 2) / (tbar->max - tbar->min);
                else
                    sh = 0.0f;

                ituWidgetSetY(tbar->tracker, ituWidgetGetHeight(tbar) - (int)(sh * (tbar->value - tbar->min)) - h + tbar->gap);
            }

            if (!tbar->valueText && (tbar->valueName[0] != '\0'))
            {
                tbar->valueText = (ITUText*) ituSceneFindWidget(ituScene, tbar->valueName);
            }

            if (tbar->valueText)
            {
                char buf[8];
                sprintf(buf, "%i", tbar->value);
                ituTextSetString(tbar->valueText, buf);
            }

            if (!tbar->tip && (tbar->tipName[0] != '\0'))
            {
                tbar->tip = (ITUWidget*) ituSceneFindWidget(ituScene, tbar->tipName);
            }

            if (tbar->tip)
            {
                if (tbar->layout == ITU_LAYOUT_HORIZONTAL)
                {
                    int w;
                    float sw;

                    w = ituWidgetGetWidth(tbar->tip) / 2;
                    if (tbar->max > tbar->min && tbar->step > 0)
                        sw = (float)(ituWidgetGetWidth(tbar) - tbar->gap * 2) / (tbar->max - tbar->min);
                    else
                        sw = 0.0f;

                    ituWidgetSetX(tbar->tip, (int)(sw * (tbar->value - tbar->min)) - w + tbar->gap);
                }
                else //if (tbar->layout == ITU_LAYOUT_VERTICAL)
                {
                    int h;
                    float sh;

                    h = ituWidgetGetHeight(tbar->tip) / 2;
                    if (tbar->max > tbar->min && tbar->step > 0)
                        sh = (float)(ituWidgetGetHeight(tbar) - tbar->gap * 2) / (tbar->max - tbar->min);
                    else
                        sh = 0.0f;

                    ituWidgetSetY(tbar->tip, ituWidgetGetHeight(tbar) - (int)(sh * (tbar->value - tbar->min)) - h + tbar->gap);
                }
                if (tbar->tracker && ituButtonIsPressed(tbar->tracker))
                    ituWidgetSetVisible(tbar->tip, true);
                else
                    ituWidgetSetVisible(tbar->tip, false);
            }
        }
        result = widget->dirty = true;
    }
    else if (ituWidgetIsEnabled(widget) && tbar->tracker && ituWidgetIsActive(tbar->tracker))
    {
        switch (ev)
        {
        case ITU_EVENT_KEYDOWN:
            if (tbar->layout == ITU_LAYOUT_HORIZONTAL)
            {
                int w, sw;

                w = ituWidgetGetWidth(tbar->tracker);
                if (tbar->max > tbar->min && tbar->step > 0)
                    sw = (ituWidgetGetWidth(tbar) - tbar->gap * 2 - ituWidgetGetWidth(tbar->tracker)) / (tbar->max - tbar->min);
                else
                    sw = 0;

                if (arg1 == ituScene->leftKey || arg1 == ituScene->rightKey)
                {
                    int value = tbar->value;

                    if (arg1 == ituScene->leftKey)
                    {
                        if (value > tbar->min)
                            value -= tbar->step;
                    }
                    else if (arg1 == ituScene->rightKey)
                    {
                        if (value < tbar->max)
                            value += tbar->step;
                    }

                    if (value != tbar->value)
                    {
                        tbar->value = value;

                        ituWidgetSetX(tbar->tracker, sw * value + tbar->gap);

                        if (!tbar->valueText && (tbar->valueName[0] != '\0'))
                        {
                            tbar->valueText = (ITUText*) ituSceneFindWidget(ituScene, tbar->valueName);
                        }

                        if (tbar->valueText)
                        {
                            char buf[8];
                            sprintf(buf, "%i", value);
                            ituTextSetString(tbar->valueText, buf);
                        }
                        ituExecActions((ITUWidget*)tbar, tbar->actions, ITU_EVENT_CHANGED, value);
                        ituTrackBarOnValueChanged(tbar, value, true);
                    }
                }
            }
            else //if (tbar->layout == ITU_LAYOUT_VERTICAL)
            {
                int h, sh;

                h = ituWidgetGetHeight(tbar->tracker);
                if (tbar->max > tbar->min && tbar->step > 0)
                    sh = (ituWidgetGetHeight(tbar) - tbar->gap * 2 - ituWidgetGetHeight(tbar->tracker)) / (tbar->max - tbar->min);
                else
                    sh = 0;

                if (arg1 == ituScene->upKey || arg1 == ituScene->downKey)
                {
                    int value = tbar->value;

                    if (arg1 == ituScene->upKey)
                    {
                        if (value < tbar->max)
                            value += tbar->step;
                    }
                    else if (arg1 == ituScene->downKey)
                    {
                        if (value > tbar->min)
                            value -= tbar->step;
                    }

                    if (value != tbar->value)
                    {
                        tbar->value = value;

                        ituWidgetSetY(tbar->tracker, ituWidgetGetHeight(tbar) - h - sh * value + tbar->gap);

                        if (!tbar->valueText && (tbar->valueName[0] != '\0'))
                        {
                            tbar->valueText = (ITUText*) ituSceneFindWidget(ituScene, tbar->valueName);
                        }

                        if (tbar->valueText)
                        {
                            char buf[8];
                            sprintf(buf, "%i", value);
                            ituTextSetString(tbar->valueText, buf);
                        }
                        ituExecActions((ITUWidget*)tbar, tbar->actions, ITU_EVENT_CHANGED, value);
                        ituTrackBarOnValueChanged(tbar, value, true);
                    }
                }
            }
            break;
        }
        result |= widget->dirty;
    }

    return result;
}

void ituTrackBarDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    int destx, desty;
    uint8_t desta;
    ITURectangle prevClip;
    ITUBackground* bg = (ITUBackground*) widget;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    ITCTree* node;
    assert(widget);
    assert(dest);

    destx = rect->x + x;
    desty = rect->y + y;
    desta = alpha * widget->color.alpha / 255;
    desta = desta * widget->alpha / 255;

    ituWidgetSetClipping(widget, dest, x, y, &prevClip);

    if (!bg->icon.surf || 
        (bg->icon.surf->width < widget->rect.width || bg->icon.surf->height < widget->rect.height) ||
        (bg->icon.surf->format == ITU_ARGB1555 || bg->icon.surf->format == ITU_ARGB4444 || bg->icon.surf->format == ITU_ARGB8888))
    {
        if (desta == 255)
        {
            if (bg->graidentMode == ITU_GF_NONE)
                ituColorFill(dest, destx, desty, rect->width, rect->height, &widget->color);
            else
                ituGradientFill(dest, destx, desty, rect->width, rect->height, &widget->color, &bg->graidentColor, bg->graidentMode);
        }
        else if (desta > 0)
        {
            ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
            if (surf)
            {
                if (bg->graidentMode == ITU_GF_NONE)
                    ituColorFill(surf, 0, 0, rect->width, rect->height, &widget->color);
                else
                    ituGradientFill(surf, 0, 0, rect->width, rect->height, &widget->color, &bg->graidentColor, bg->graidentMode);

                ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);                
                ituDestroySurface(surf);
            }
        }
    }
    
    if (bg->icon.surf)
    {
        desta = alpha * widget->alpha / 255;
        if (desta > 0)
        {
            if (widget->flags & ITU_STRETCH)
            {
#if (CFG_CHIP_FAMILY == 9070)
                if (desta == 255)
                {
                    if (widget->angle == 0)
                    {
                        ituStretchBlt(dest, destx, desty, rect->width, rect->height, bg->icon.surf, 0, 0, bg->icon.surf->width, bg->icon.surf->height);
                    }
                    else
                    {
                        float scaleX = (float)rect->width / bg->icon.surf->width;
                        float scaleY = (float)rect->height / bg->icon.surf->height;

                        ituRotate(dest, destx + rect->width / 2, desty + rect->height / 2, bg->icon.surf, bg->icon.surf->width / 2, bg->icon.surf->height / 2, (float)widget->angle, scaleX, scaleY);
                    }
                }
                else
                {
                    ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
                    if (surf)
                    {
                        ituBitBlt(surf, 0, 0, rect->width, rect->height, dest, destx, desty);

                        if (widget->angle == 0)
                        {
                            ituStretchBlt(surf, 0, 0, rect->width, rect->height, bg->icon.surf, 0, 0, bg->icon.surf->width, bg->icon.surf->height);
                        }
                        else
                        {
                            float scaleX = (float)rect->width / bg->icon.surf->width;
                            float scaleY = (float)rect->height / bg->icon.surf->height;

                            ituRotate(surf, rect->width / 2, rect->height / 2, bg->icon.surf, bg->icon.surf->width / 2, bg->icon.surf->height / 2, (float)widget->angle, scaleX, scaleY);
                        }
                        ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);
                        ituDestroySurface(surf);
                    }
                }
#else
                float scaleX = (float)rect->width / bg->icon.surf->width;
                float scaleY = (float)rect->height / bg->icon.surf->height;
               
                ituTransform(
                    dest, destx, desty, rect->width, rect->height,
                    bg->icon.surf, 0, 0, bg->icon.surf->width, bg->icon.surf->height,
                    bg->icon.surf->width / 2, bg->icon.surf->height / 2,
                    scaleX,
                    scaleY,
                    (float)widget->angle,
                    0,
                    true,
                    true,
                    desta);
#endif
            }
            else
            {
                if (desta == 255)
                {
                    ituBitBlt(dest, destx, desty, bg->icon.surf->width, bg->icon.surf->height, bg->icon.surf, 0, 0);
                }
                else
                {
                    ituAlphaBlend(dest, destx, desty, bg->icon.surf->width, bg->icon.surf->height, bg->icon.surf, 0, 0, desta);
                }
            }
        }
    }
    ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);

    alpha = alpha * widget->alpha / 255;

    for (node = widget->tree.child; node; node = node->sibling)
    {
        ITUWidget* child = (ITUWidget*)node;
        if (child->visible && ituWidgetIsOverlapClipping(child, dest, destx, desty))
            ituWidgetDraw(node, dest, destx, desty, alpha);

        child->dirty = false;
    }
}

static void TrackBarOnValueChanged(ITUTrackBar* tbar, int value, bool confirm)
{
    // DO NOTHING
}

void ituTrackBarOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    ITUTrackBar* tbar = (ITUTrackBar*) widget;
    int value;
    assert(widget);

    switch (action)
    {
    case ITU_ACTION_GOTO:
        value = atoi(param);
        ituTrackBarSetValue(tbar, value);
        ituExecActions(widget, tbar->actions, ITU_EVENT_CHANGED, value);
        ituTrackBarOnValueChanged(tbar, value, true);
        break;

    case ITU_ACTION_DODELAY0:
        ituExecActions(widget, tbar->actions, ITU_EVENT_DELAY0, atoi(param));
        break;

    case ITU_ACTION_DODELAY1:
        ituExecActions(widget, tbar->actions, ITU_EVENT_DELAY1, atoi(param));
        break;

    case ITU_ACTION_DODELAY2:
        ituExecActions(widget, tbar->actions, ITU_EVENT_DELAY2, atoi(param));
        break;

    case ITU_ACTION_DODELAY3:
        ituExecActions(widget, tbar->actions, ITU_EVENT_DELAY3, atoi(param));
        break;

    case ITU_ACTION_DODELAY4:
        ituExecActions(widget, tbar->actions, ITU_EVENT_DELAY4, atoi(param));
        break;

    case ITU_ACTION_DODELAY5:
        ituExecActions(widget, tbar->actions, ITU_EVENT_DELAY5, atoi(param));
        break;

    case ITU_ACTION_DODELAY6:
        ituExecActions(widget, tbar->actions, ITU_EVENT_DELAY6, atoi(param));
        break;

    case ITU_ACTION_DODELAY7:
        ituExecActions(widget, tbar->actions, ITU_EVENT_DELAY7, atoi(param));
        break;

    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

void ituTrackBarInit(ITUTrackBar* tbar, ITULayout layout)
{
    assert(tbar);
    ITU_ASSERT_THREAD();

    memset(tbar, 0, sizeof (ITUTrackBar));

    ituBackgroundInit(&tbar->bg);
    
    ituWidgetSetType(tbar, ITU_TRACKBAR);
    ituWidgetSetName(tbar, tbarName);
    ituWidgetSetUpdate(tbar, ituTrackBarUpdate);
    ituWidgetSetDraw(tbar, ituTrackBarDraw);
    ituWidgetSetOnAction(tbar, ituTrackBarOnAction);
    ituTrackBarSetValueChanged(tbar, TrackBarOnValueChanged);

    tbar->layout = layout;
}

void ituTrackBarLoad(ITUTrackBar* tbar, uint32_t base)
{
    assert(tbar);

    ituBackgroundLoad(&tbar->bg, base);

    if (tbar->tracker)
        tbar->tracker = (ITUButton*)((uint32_t)tbar->tracker + base);

    if (tbar->valueText)
        tbar->valueText = (ITUText*)((uint32_t)tbar->valueText + base);

    if (tbar->tip)
        tbar->tip = (ITUWidget*)((uint32_t)tbar->tip + base);

    ituWidgetSetUpdate(tbar, ituTrackBarUpdate);
    ituWidgetSetDraw(tbar, ituTrackBarDraw);
    ituWidgetSetOnAction(tbar, ituTrackBarOnAction);
    ituTrackBarSetValueChanged(tbar, TrackBarOnValueChanged);
}

void ituTrackBarSetValue(ITUTrackBar* bar, int value)
{
    assert(bar);
    ITU_ASSERT_THREAD();

    if (value < bar->min || value > bar->max)
    {
        LOG_WARN "incorrect value: %d\n", value LOG_END
        return;
    }
    bar->value = value;

    ituWidgetUpdate(bar, ITU_EVENT_LAYOUT, 0, 0, 0);
    ituWidgetSetDirty(bar, true);
}

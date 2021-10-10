#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char scrollBarName[] = "ITUScrollBar";

bool ituScrollBarUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUScrollBar* bar = (ITUScrollBar*) widget;
    assert(bar);

    if (ev == ITU_EVENT_MOUSEDOWN)
    {
        if (ituWidgetIsEnabled(widget))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                if (bar->layout == ITU_LAYOUT_HORIZONTAL)
                {
                    int x = arg2 - widget->rect.x;
                    int pos = bar->length * x / widget->rect.width + 1;

                    if (pos > bar->length)
                        pos = bar->length;

                    if (pos != bar->pos)
                    {
                        ituScrollBarSetPosition(bar, pos);
                        ituExecActions((ITUWidget*)bar, bar->actions, ITU_EVENT_CHANGED, pos);
                        ituScrollBarOnPositionChanged(bar, pos, true);
                    }
                }
                else // if (bar->layout == ITU_LAYOUT_VERTICAL)
                {
                    int y = arg3 - widget->rect.y;
                    int pos = bar->length * y / widget->rect.height + 1;

                    if (pos > bar->length)
                        pos = bar->length;

                    if (pos != bar->pos)
                    {
                        ituScrollBarSetPosition(bar, pos);
                        ituExecActions((ITUWidget*)bar, bar->actions, ITU_EVENT_CHANGED, pos);
                        ituScrollBarOnPositionChanged(bar, pos, true);
                    }
                }
                if (widget->flags & ITU_DRAGGABLE)
                    widget->flags |= ITU_DRAGGING;

                result = widget->dirty = true;
            }
        }
    }
    else if (ev == ITU_EVENT_MOUSEUP)
    {
        if (widget->flags & ITU_DRAGGABLE)
            widget->flags &= ~ITU_DRAGGING;
    }
    else if (ev == ITU_EVENT_MOUSEMOVE)
    {
        if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGING))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                if (bar->layout == ITU_LAYOUT_HORIZONTAL)
                {
                    int x = arg2 - widget->rect.x;
                    int pos = bar->length * x / widget->rect.width + 1;

                    if (pos > bar->length)
                        pos = bar->length;

                    if (pos != bar->pos)
                    {
                        ituScrollBarSetPosition(bar, pos);
                        ituExecActions((ITUWidget*)bar, bar->actions, ITU_EVENT_CHANGED, pos);
                        ituScrollBarOnPositionChanged(bar, pos, true);
                        result = widget->dirty = true;
                    }
                }
                else // if (bar->layout == ITU_LAYOUT_VERTICAL)
                {
                    int y = arg3 - widget->rect.y;
                    int pos = bar->length * y / widget->rect.height + 1;

                    if (pos > bar->length)
                        pos = bar->length;

                    if (pos != bar->pos)
                    {
                        ituScrollBarSetPosition(bar, pos);
                        ituExecActions((ITUWidget*)bar, bar->actions, ITU_EVENT_CHANGED, pos);
                        ituScrollBarOnPositionChanged(bar, pos, true);
                        result = widget->dirty = true;
                    }
                }
            }
        }
    }
    result |= ituIconUpdate(widget, ev, arg1, arg2, arg3);

    return widget->visible ? result : false;
}

void ituScrollBarDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    int destx, desty;
    uint8_t desta;
    ITURectangle prevClip;
    ITUScrollBar* bar = (ITUScrollBar*) widget;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    assert(bar);
    assert(dest);

    destx = rect->x + x;
    desty = rect->y + y;
    desta = alpha * widget->color.alpha / 255;
    desta = desta * widget->alpha / 255;

    ituWidgetSetClipping(widget, dest, x, y, &prevClip);

    if (desta == 255)
    {
        ituColorFill(dest, destx, desty, rect->width, rect->height, &widget->color);
            
        if (bar->pos > 0 && bar->length > 0)
        {
            if (bar->layout == ITU_LAYOUT_HORIZONTAL)
            {
                int width = widget->rect.width / bar->length + 1;
                int x = (bar->pos - 1) * width;

                ituColorFill(dest, destx + x, desty, width, rect->height, &bar->fgColor);
            }
            else //if (bar->layout == ITU_LAYOUT_VERTICAL)
            {
                int height = widget->rect.height / bar->length + 1;
                int y = (bar->pos - 1) * height;            

                ituColorFill(dest, destx, desty + y, rect->width, height, &bar->fgColor);            
            }
        }
    }
    else if (desta > 0)
    {
        ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
        if (surf)
        {
            ituColorFill(surf, 0, 0, rect->width, rect->height, &widget->color);
            
            if (bar->pos > 0 && bar->length > 0)
            {
                if (bar->layout == ITU_LAYOUT_HORIZONTAL)
                {
                    int width = widget->rect.width / bar->length;
                    int x = (bar->pos - 1) * width;
                                
                    ituColorFill(surf, x, 0, width, rect->height, &bar->fgColor);
                }
                else //if (bar->layout == ITU_LAYOUT_VERTICAL)
                {
                    int height = widget->rect.height / bar->length;
                    int y = (bar->pos - 1) * height;            
                    
                    ituColorFill(surf, 0, y, rect->width, rect->height, &bar->fgColor);
                }
            }
            ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);                
            ituDestroySurface(surf);
        }
    }
    ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
    ituWidgetDrawImpl(widget, dest, x, y, alpha);
}

static void ScrollBarOnPositionChanged(ITUScrollBar* bar, int pos, bool confirm)
{
    // DO NOTHING
}

void ituScrollBarOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    ITUScrollBar* bar = (ITUScrollBar*) widget;
    assert(bar);

    switch (action)
    {
    case ITU_ACTION_RELOAD:
        if (param && param[0] != '\0')
        {
            char buf[ITU_ACTION_PARAM_SIZE], *saveptr;

            strcpy(buf, param);
            bar->pos = atoi(strtok_r(buf, " ", &saveptr));
            bar->length = atoi(strtok_r(NULL, " ", &saveptr));
            
            ituWidgetSetDirty(bar, true);
        }
        break;

    case ITU_ACTION_DODELAY0:
        ituExecActions(widget, bar->actions, ITU_EVENT_DELAY0, atoi(param));
        break;

    case ITU_ACTION_DODELAY1:
        ituExecActions(widget, bar->actions, ITU_EVENT_DELAY1, atoi(param));
        break;

    case ITU_ACTION_DODELAY2:
        ituExecActions(widget, bar->actions, ITU_EVENT_DELAY2, atoi(param));
        break;

    case ITU_ACTION_DODELAY3:
        ituExecActions(widget, bar->actions, ITU_EVENT_DELAY3, atoi(param));
        break;

    case ITU_ACTION_DODELAY4:
        ituExecActions(widget, bar->actions, ITU_EVENT_DELAY4, atoi(param));
        break;

    case ITU_ACTION_DODELAY5:
        ituExecActions(widget, bar->actions, ITU_EVENT_DELAY5, atoi(param));
        break;

    case ITU_ACTION_DODELAY6:
        ituExecActions(widget, bar->actions, ITU_EVENT_DELAY6, atoi(param));
        break;

    case ITU_ACTION_DODELAY7:
        ituExecActions(widget, bar->actions, ITU_EVENT_DELAY7, atoi(param));
        break;

    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

void ituScrollBarInit(ITUScrollBar* bar)
{
    assert(bar);
    ITU_ASSERT_THREAD();

    memset(bar, 0, sizeof (ITUScrollBar));

    ituBackgroundInit(&bar->bg);
    ituWidgetSetType(bar, ITU_SCROLLBAR);
    ituWidgetSetName(bar, scrollBarName);
    ituWidgetSetUpdate(bar, ituScrollBarUpdate);
    ituWidgetSetDraw(bar, ituScrollBarDraw);
    ituWidgetSetOnAction(bar, ituScrollBarOnAction);
    ituScrollBarSetPositionChanged(bar, ScrollBarOnPositionChanged);
}

void ituScrollBarLoad(ITUScrollBar* bar, uint32_t base)
{
    assert(bar);

    ituBackgroundLoad(&bar->bg, base);
    ituWidgetSetUpdate(bar, ituScrollBarUpdate);
    ituWidgetSetDraw(bar, ituScrollBarDraw);
    ituWidgetSetOnAction(bar, ituScrollBarOnAction);
    ituScrollBarSetPositionChanged(bar, ScrollBarOnPositionChanged);
}

void ituScrollBarSetLength(ITUScrollBar* bar, int length)
{
    assert(bar);
    ITU_ASSERT_THREAD();

    if (length < 0)
    {
        LOG_WARN "incorrect length: %d\n", length LOG_END
        return;
    }
    bar->length = length;

    ituWidgetSetDirty(bar, true);
}

void ituScrollBarSetPosition(ITUScrollBar* bar, int pos)
{
    assert(bar);
    ITU_ASSERT_THREAD();

    if (pos < 0 || pos > bar->length)
    {
        LOG_WARN "incorrect pos: %d\n", pos LOG_END
        return;
    }
    bar->pos = pos;

    ituWidgetSetDirty(bar, true);
}

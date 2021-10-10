#include <assert.h>
#include <malloc.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char bwName[] = "ITUBorderWindow";

bool ituBorderWindowClone(ITUWidget* widget, ITUWidget** cloned)
{
    ITUBorderWindow* bwin = (ITUBorderWindow*)widget;
    assert(widget);
    assert(cloned);
    ITU_ASSERT_THREAD();

    if (*cloned == NULL)
    {
        ITUWidget* newWidget = malloc(sizeof(ITUBorderWindow));
        if (newWidget == NULL)
            return false;

        memcpy(newWidget, widget, sizeof(ITUBorderWindow));
        newWidget->tree.child = newWidget->tree.parent = newWidget->tree.sibling = NULL;
        *cloned = newWidget;
    }

    return ituWidgetCloneImpl(widget, cloned);
}

bool ituBorderWindowUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result;
    ITUBorderWindow* bwin = (ITUBorderWindow*) widget;
    assert(bwin);

    result = ituWidgetUpdateImpl(widget, ev, arg1, arg2, arg3);

    if (ev == ITU_EVENT_LAYOUT)
    {
        ITURectangle* rect;
        int width, height;

        width = widget->rect.width;
        height = widget->rect.height;

        // calc all position of widgets
        if (bwin->widgets[ITU_LAYOUT_UP])
        {
            rect    = &bwin->widgets[ITU_LAYOUT_UP]->rect;
            rect->x = width / 2 - rect->width / 2;

            if (rect->x < bwin->borderSize)
                rect->x = bwin->borderSize;

            rect->y = bwin->borderSize;
        }

        if (bwin->widgets[ITU_LAYOUT_LEFT])
        {
            rect    = &bwin->widgets[ITU_LAYOUT_LEFT]->rect;
            rect->x = bwin->borderSize;
            rect->y = height / 2 - rect->height / 2;

            if (rect->y < bwin->borderSize)
                rect->y = bwin->borderSize;
        }

        if (bwin->widgets[ITU_LAYOUT_CENTER])
        {
            rect    = &bwin->widgets[ITU_LAYOUT_CENTER]->rect;
            rect->x = width / 2 - rect->width / 2;
            rect->y = height / 2 - rect->height / 2;

            if (rect->x < bwin->borderSize)
                rect->x = bwin->borderSize;

            if (rect->y < bwin->borderSize)
                rect->y = bwin->borderSize;
        }

        if (bwin->widgets[ITU_LAYOUT_RIGHT])
        {
            rect    = &bwin->widgets[ITU_LAYOUT_RIGHT]->rect;
            rect->x = width - rect->width - bwin->borderSize;
            rect->y = height / 2 - rect->height / 2;

            if (rect->y < bwin->borderSize)
                rect->y = bwin->borderSize;
        }

        if (bwin->widgets[ITU_LAYOUT_DOWN])
        {
            rect    = &bwin->widgets[ITU_LAYOUT_DOWN]->rect;
            rect->x = width / 2 - rect->width / 2;

            if (rect->x < bwin->borderSize)
                rect->x = bwin->borderSize;

            rect->y = height - rect->height - bwin->borderSize;
        }

        result = widget->dirty = true;
    }
    return widget->visible ? result : false;
}

void ituBorderWindowDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    ITUBorderWindow* bwin = (ITUBorderWindow*) widget;
    assert(bwin);
    assert(dest);

    if (bwin->borderSize > 0)
    {
        int destx, desty;
        uint8_t desta;
        ITURectangle* rect = (ITURectangle*) &widget->rect;

        destx = rect->x + x;
        desty = rect->y + y;
        desta = alpha * widget->color.alpha / 255;
        desta = desta * widget->alpha / 255;

        if (desta == 255)
        {
            ituColorFill(dest, destx, desty, rect->width, bwin->borderSize, &widget->color);
            ituColorFill(dest, destx, desty + rect->height - bwin->borderSize, rect->width, bwin->borderSize, &widget->color);
            ituColorFill(dest, destx, desty + bwin->borderSize, bwin->borderSize, rect->height - bwin->borderSize * 2, &widget->color);
            ituColorFill(dest, destx + rect->width - bwin->borderSize, desty + bwin->borderSize, bwin->borderSize, rect->height - bwin->borderSize * 2, &widget->color);
        }
        else if (desta > 0)
        {
            ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, ITU_ARGB4444, NULL, 0);
            if (surf)
            {
                ITUColor black = { 0, 0, 0, 0 };
                ituColorFill(surf, 0, 0, rect->width, rect->height, &black);
                ituColorFill(surf, 0, 0, rect->width, bwin->borderSize, &widget->color);
                ituColorFill(surf, 0, rect->height - bwin->borderSize, rect->width, bwin->borderSize, &widget->color);
                ituColorFill(surf, 0, bwin->borderSize, bwin->borderSize, rect->height - bwin->borderSize * 2, &widget->color);
                ituColorFill(surf, rect->width - bwin->borderSize, bwin->borderSize, bwin->borderSize, rect->height - bwin->borderSize * 2, &widget->color);
                ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);                
                ituDestroySurface(surf);
            }
        }
    }
    ituWidgetDrawImpl(widget, dest, x, y, alpha);
}

void ituBorderWindowInit(ITUBorderWindow* bwin)
{
    assert(bwin);
    ITU_ASSERT_THREAD();

    memset(bwin, 0, sizeof (ITUBorderWindow));

    ituWidgetInit(&bwin->widget);

    ituWidgetSetType(bwin, ITU_BORDERWINDOW);
    ituWidgetSetName(bwin, bwName);
    ituWidgetSetClone(bwin, ituBorderWindowClone);
    ituWidgetSetUpdate(bwin, ituBorderWindowUpdate);
    ituWidgetSetDraw(bwin, ituBorderWindowDraw);
}

void ituBorderWindowLoad(ITUBorderWindow* bwin, uint32_t base)
{
    assert(bwin);

    ituWidgetLoad((ITUWidget*)bwin, base);
    ituWidgetSetClone(bwin, ituBorderWindowClone);
    ituWidgetSetUpdate(bwin, ituBorderWindowUpdate);
    ituWidgetSetDraw(bwin, ituBorderWindowDraw);
}

void ituBorderWindowAdd(ITUBorderWindow* bwin, ITUWidget* child, ITULayout layout)
{
    assert(bwin);
    assert(child);
    ITU_ASSERT_THREAD();

    ituWidgetAdd(&bwin->widget, child);
    bwin->widgets[layout] = child;
}

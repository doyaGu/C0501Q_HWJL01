#include <assert.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char fwName[] = "ITUFlowWindow";

bool ituFlowWindowUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result;
    ITUFlowWindow* fwin = (ITUFlowWindow*) widget;
    assert(fwin);

    result = ituWidgetUpdateImpl(widget, ev, arg1, arg2, arg3);

	if ((widget->type == ITU_WHEEL) && (ev == ITU_EVENT_LAYOUT))
	{
		ITCTree* node;
		ITUWidget* child;
		ITURectangle* rect;
		ITUWheel* wheel = (ITUWheel*)widget;
		int size;

		if (wheel->cycle_tor > 0)
		{
			ITUWidget* fc = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->focus_c);
			int fix_focus_font_size = ((wheel->focusFontHeight - wheel->fontHeight) / ITU_WHEEL_FOCUS_FONT_FIX_POS);

			size = fwin->borderSize;

			for (node = widget->tree.child; node; node = node->sibling)
			{
				child = (ITUWidget*)node;
				rect = &child->rect;
				rect->x = fwin->borderSize;
				//rect->y = size - wheel->layout_ci;

				if (child == fc)
					rect->y = size - wheel->layout_ci - fix_focus_font_size;
				else
					rect->y = size - wheel->layout_ci;

				size = rect->y + rect->height;

				if (child == fc)
					size += fix_focus_font_size;
			}

			result = widget->dirty = true;
			return widget->visible ? result : false;
		}
		else
		{
			size = fwin->borderSize + wheel->layout_ci;

			for (node = widget->tree.child; node; node = node->sibling)
			{
				child = (ITUWidget*)node;
				rect = &child->rect;

				rect->x = fwin->borderSize;
				rect->y = size;
				size = rect->y + rect->height;
			}

			result = widget->dirty = true;
			return widget->visible ? result : false;
		}
	}

	if (ev == ITU_EVENT_LAYOUT)
    {
        ITCTree* node;
        ITUWidget* child;
        ITURectangle* rect;
        int size;

        // calc all position of widgets
        if (fwin->layout == ITU_LAYOUT_UP)
        {
            size = fwin->borderSize;

            for (node = widget->tree.child; node; node = node->sibling)
            {
                child = (ITUWidget*) node;
                rect = &child->rect;

                rect->x = fwin->borderSize;
                rect->y = size;
                size = rect->y + rect->height;
            }
        }
        else if (fwin->layout == ITU_LAYOUT_LEFT)
        {
            size = fwin->borderSize;

            for (node = widget->tree.child; node; node = node->sibling)
            {
                child = (ITUWidget*) node;
                rect = &child->rect;

                rect->y = fwin->borderSize;
                rect->x = size;
                size = rect->x + rect->width;
            }
        }
        else if (fwin->layout == ITU_LAYOUT_DOWN)
        {
            size = fwin->borderSize;

            // calc total height
            for (node = widget->tree.child; node; node = node->sibling)
            {
                child = (ITUWidget*) node;
                rect = &child->rect;

                size += rect->height;
            }

            size = widget->rect.height - size;

            for (node = widget->tree.child; node; node = node->sibling)
            {
                child = (ITUWidget*) node;
                rect = &child->rect;

                rect->x = fwin->borderSize;
                rect->y = size;
                size = rect->y + rect->height;
            }
        }
        else if (fwin->layout == ITU_LAYOUT_RIGHT)
        {
            size = fwin->borderSize;

            // calc total width
            for (node = widget->tree.child; node; node = node->sibling)
            {
                child = (ITUWidget*) node;
                rect = &child->rect;

                size += rect->width;
            }

            size = widget->rect.width - size;

            for (node = widget->tree.child; node; node = node->sibling)
            {
                child = (ITUWidget*) node;
                rect = &child->rect;

                rect->y = fwin->borderSize;
                rect->x = size;
                size = rect->x + rect->width;
            }
        }
        result = widget->dirty = true;
    }
    return widget->visible ? result : false;
}

void ituFlowWindowDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    ITUFlowWindow* fwin = (ITUFlowWindow*) widget;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    assert(fwin);
    assert(dest);

    if (fwin->borderSize > 0)
    {
        int destx, desty;
        uint8_t desta;

        destx = rect->x + x;
        desty = rect->y + y;
        desta = alpha * widget->color.alpha / 255;
        desta = desta * widget->alpha / 255;
       
        if (desta == 255)
        {
            ituColorFill(dest, destx, desty, rect->width, fwin->borderSize, &widget->color);
            ituColorFill(dest, destx, desty + rect->height - fwin->borderSize, rect->width, fwin->borderSize, &widget->color);
            ituColorFill(dest, destx, desty + fwin->borderSize, fwin->borderSize, rect->height - fwin->borderSize * 2, &widget->color);
            ituColorFill(dest, destx + rect->width - fwin->borderSize, desty + fwin->borderSize, fwin->borderSize, rect->height - fwin->borderSize * 2, &widget->color);
        }
        else if (desta > 0)
        {
            ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
            if (surf)
            {
                ITUColor black = { 255, 0, 0, 0 };
                ituColorFill(surf, 0, 0, rect->width, rect->height, &black);
                ituColorFill(surf, 0, 0, rect->width, fwin->borderSize, &widget->color);
                ituColorFill(surf, 0, rect->height - fwin->borderSize, rect->width, fwin->borderSize, &widget->color);
                ituColorFill(surf, 0, fwin->borderSize, fwin->borderSize, rect->height - fwin->borderSize * 2, &widget->color);
                ituColorFill(surf, rect->width - fwin->borderSize, fwin->borderSize, fwin->borderSize, rect->height - fwin->borderSize * 2, &widget->color);
                ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);                
                ituDestroySurface(surf);
            }
        }
    }
    ituWidgetDrawImpl(widget, dest, x, y, alpha);
}

void ituFlowWindowInit(ITUFlowWindow* fwin, ITULayout layout)
{
    assert(fwin);
    ITU_ASSERT_THREAD();

    memset(fwin, 0, sizeof (ITUFlowWindow));

    ituWidgetInit(&fwin->widget);

    ituWidgetSetType(fwin, ITU_FLOWWINDOW);
    ituWidgetSetName(fwin, fwName);
    ituWidgetSetUpdate(fwin, ituFlowWindowUpdate);
    ituWidgetSetDraw(fwin, ituFlowWindowDraw);

    fwin->layout = layout;
}

void ituFlowWindowLoad(ITUFlowWindow* fwin, uint32_t base)
{
    assert(fwin);

    ituWidgetLoad((ITUWidget*)fwin, base);
    ituWidgetSetUpdate(fwin, ituFlowWindowUpdate);
    ituWidgetSetDraw(fwin, ituFlowWindowDraw);
}

void ituFlowWindowAdd(ITUFlowWindow* fwin, ITUWidget* child)
{
    assert(fwin);
    assert(child);
    ITU_ASSERT_THREAD();

    ituWidgetAdd(&fwin->widget, child);
}

#include <assert.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char btnName[] = "ITUButton";

//Special flag used to active the press alpha(image) delay mode
#define PRESS_ALPHA_DELAY_ARG 255

//The delay frame when active the press alpha(image) delay mode
#define PRESS_ALPHA_DELAY_FRAME 3

void ituButtonExit(ITUWidget* widget)
{
    ITUButton* btn = (ITUButton*) widget;
    assert(widget);
    ITU_ASSERT_THREAD();

    if (btn->pressSurf)
    {
        ituSurfaceRelease(btn->pressSurf);
        btn->pressSurf = NULL;
    }

    if (btn->focusSurf)
    {
        ituSurfaceRelease(btn->focusSurf);
        btn->focusSurf = NULL;
    }
    ituIconExit(widget);
}

bool ituButtonClone(ITUWidget* widget, ITUWidget** cloned)
{
    ITUButton* btn = (ITUButton*)widget;
    ITUButton* newBtn;
    ITUSurface* surf;
    assert(widget);
    assert(cloned);
    ITU_ASSERT_THREAD();

    if (*cloned == NULL)
    {
        ITUWidget* newWidget = malloc(sizeof(ITUButton));
        if (newWidget == NULL)
            return false;

        memcpy(newWidget, widget, sizeof(ITUButton));
        newWidget->tree.child = newWidget->tree.parent = newWidget->tree.sibling = NULL;
        *cloned = newWidget;
    }

    newBtn = (ITUButton*)*cloned;

    // change internal tree structure of new button
    newBtn->bwin.widgets[ITU_LAYOUT_CENTER] = &newBtn->text.widget;
    newBtn->bwin.widget.tree.child = &newBtn->text.widget.tree;
    newBtn->bg.icon.widget.tree.child = &newBtn->bwin.widget.tree;

    surf = newBtn->bg.icon.staticSurf;

    if (surf)
    {
        if (surf->flags & ITU_COMPRESSED)
            newBtn->bg.icon.surf = ituSurfaceDecompress(surf);
        else
            newBtn->bg.icon.surf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }

    surf = newBtn->staticFocusSurf;

    if (surf)
    {
        if (surf->flags & ITU_COMPRESSED)
            newBtn->focusSurf = ituSurfaceDecompress(surf);
        else
            newBtn->focusSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }

    surf = newBtn->staticPressSurf;

    if (surf)
    {
        if (surf->flags & ITU_COMPRESSED)
            newBtn->pressSurf = ituSurfaceDecompress(surf);
        else
            newBtn->pressSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }

    return ituBackgroundClone(widget, cloned);
}

static void ButtonLoadExternalData(ITUButton* btn, ITULayer* layer)
{
    ITUWidget* widget = (ITUWidget*)btn;
    ITUSurface* surf;

    assert(widget);

    if (!(widget->flags & ITU_EXTERNAL))
        return;

    if (!layer)
        layer = ituGetLayer(widget);

    if (btn->staticFocusSurf && !btn->focusSurf)
    {
        surf = ituLayerLoadExternalSurface(layer, (uint32_t)btn->staticFocusSurf);

        if (surf->flags & ITU_COMPRESSED)
            btn->focusSurf = ituSurfaceDecompress(surf);
        else
            btn->focusSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }

    if (btn->staticPressSurf && !btn->pressSurf)
    {
        surf = ituLayerLoadExternalSurface(layer, (uint32_t)btn->staticPressSurf);

        if (surf->flags & ITU_COMPRESSED)
            btn->pressSurf = ituSurfaceDecompress(surf);
        else
            btn->pressSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }
}

bool ituButtonUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUButton* btn = (ITUButton*) widget;
    assert(btn);

    if (ev == ITU_EVENT_LAYOUT)
    {
        ituTextResize(&btn->text);
        btn->bwin.widget.rect.width = widget->rect.width;
        btn->bwin.widget.rect.height = widget->rect.height;
    }

    result |= ituIconUpdate(widget, ev, arg1, arg2, arg3);

    if (ev == ITU_EVENT_MOUSEDOWN)
    {
        if (ituWidgetIsEnabled(widget) && !result)
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
				if (arg1 == PRESS_ALPHA_DELAY_ARG)
				{
					int count = (int)ituWidgetGetCustomData(btn);

					if (count == 0)
						ituWidgetSetCustomData(btn, PRESS_ALPHA_DELAY_FRAME);

					widget->dirty = true;
					btn->pressed = true;
				}
				else
				{
					ituButtonSetPressed(btn, true);
				}

                result |= ituExecActions((ITUWidget*)btn, btn->actions, ev, 0);

                if (widget->type == ITU_BUTTON)
                    result |= ituWidgetOnPress(widget, ev, arg1, x, y);

                result |= widget->dirty;
                return result;
            }
        }
    }
    else if (ev == ITU_EVENT_MOUSEUP)
    {
        if (ituWidgetIsEnabled(widget))
        {
			ituWidgetSetCustomData(btn, 0);

            if (btn->pressed)
            {
                ituButtonSetPressed(btn, false);
                result |= ituExecActions((ITUWidget*)btn, btn->actions, ev, 0);

                if (widget->type == ITU_BUTTON)
                    ituFocusWidget(btn);

                result |= widget->dirty;
                return result;
            }
        }
    }
    else if (ev == ITU_EVENT_MOUSEDOUBLECLICK || ev == ITU_EVENT_MOUSELONGPRESS)
    {
        if (ituWidgetIsEnabled(widget) && !result)
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

			ituWidgetSetCustomData(btn, 0);

            if (!widget->rect.width || !widget->rect.height || ituWidgetIsInside(widget, x, y))
            {
                result |= ituExecActions(widget, btn->actions, ev, arg1);
            }
            if (btn->pressed && (ev != ITU_EVENT_MOUSELONGPRESS))
            {
                ituButtonSetPressed(btn, false);
                result |= widget->dirty;
            }
        }
    }
	else if (ev == ITU_EVENT_MOUSEMOVE)
	{
		if (ituWidgetIsEnabled(widget))
		{
			int count = (int)ituWidgetGetCustomData(btn);

			if ((count < 255) && (arg1 == PRESS_ALPHA_DELAY_ARG))
			{
				ituButtonSetPressed(btn, false);
				ituWidgetSetCustomData(btn, 0);
			}
		}
	}
    else if (ev >= ITU_EVENT_CUSTOM || ev == ITU_EVENT_TIMER)
    {
        if (ituWidgetIsEnabled(widget))
        {
			if (ev == ITU_EVENT_TIMER)
			{
				int count = (int)ituWidgetGetCustomData(btn);

				if ((count > 0) && (count < 255))
				{
					if (count == 1)
					{
						ituWidgetSetColor(widget, btn->pressColor.alpha, btn->pressColor.red, btn->pressColor.green, btn->pressColor.blue);
						//btn->pressed = true;
						widget->dirty = true;
					}

					count--;
					ituWidgetSetCustomData(btn, count);
				}
			}
            result |= ituExecActions((ITUWidget*)btn, btn->actions, ev, arg1);
        }
    }
    else if (ev == ITU_EVENT_LOAD)
    {
        ituButtonLoadStaticData(btn);
        result = true;
    }
    else if (ev == ITU_EVENT_LOAD_EXTERNAL)
    {
        ButtonLoadExternalData(btn, (ITULayer*)arg1);
        result = true;
    }
    else if (ev == ITU_EVENT_RELEASE)
    {
        ituButtonReleaseSurface(btn);
        result = true;
    }
    else if (ituWidgetIsActive(widget) && ituWidgetIsEnabled(widget) && !result)
    {
        switch (ev)
        {
        case ITU_EVENT_KEYDOWN:
            if (arg1 == ituScene->enterKey)
            {
                ituButtonSetPressed(btn, true);
                ituFocusWidget(btn);
                result |= ituExecActions((ITUWidget*)btn, btn->actions, ev, arg1);
            }
            break;

        case ITU_EVENT_KEYUP:
            if (arg1 == ituScene->enterKey)
            {
                ituButtonSetPressed(btn, false);
                result |= ituExecActions((ITUWidget*)btn, btn->actions, ev, arg1);
            }
            break;
        }
        result |= widget->dirty;
    }
    else if (ev == ITU_EVENT_LAYOUT)
    {
        ituButtonSetPressed(btn, btn->pressed);
        result = widget->dirty = true;
    }
    return result;
}

void ituButtonDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    int destx, desty;
    uint8_t desta;
    ITURectangle prevClip;
    ITUButton* btn = (ITUButton*) widget;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    assert(btn);
    assert(dest);

    if ((btn->pressed && btn->pressSurf) || (widget->active && btn->focusSurf))
    {
        ITUSurface* srcSurf;

        destx = rect->x + x;
        desty = rect->y + y;
        desta = alpha * widget->color.alpha / 255;
        desta = desta * widget->alpha / 255;

        if (widget->angle == 0)
            ituWidgetSetClipping(widget, dest, x, y, &prevClip);

        if (btn->pressed && btn->pressSurf)
        {
			int count = (int)ituWidgetGetCustomData(btn);

			if ((count > 0) && (count < 255))
				return;
			
			srcSurf = btn->pressSurf;
        }
        else // widget->active == true
        {
            srcSurf = btn->focusSurf;
        }

        if (!srcSurf || 
            (srcSurf->width < widget->rect.width || srcSurf->height < widget->rect.height) ||
            (srcSurf->format == ITU_ARGB1555 || srcSurf->format == ITU_ARGB4444 || srcSurf->format == ITU_ARGB8888))
        {
            if (desta > 0)
            {
                if (desta == 255)
                {
                    ituColorFill(dest, destx, desty, rect->width, rect->height, &widget->color);
                }
                else
                {
                    ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
                    if (surf)
                    {
                        ituColorFill(surf, 0, 0, rect->width, rect->height, &widget->color);
                        ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);                
                        ituDestroySurface(surf);
                    }
                }
            }
        }

        if (srcSurf)
        {
            desta = alpha * widget->alpha / 255;
            if (desta > 0)
            {
                if (desta == 255)
                {
                    if (widget->flags & ITU_STRETCH)
                        ituStretchBlt(dest, destx, desty, rect->width, rect->height, srcSurf, 0, 0, srcSurf->width, srcSurf->height);
                    else
                        ituBitBlt(dest, destx, desty, srcSurf->width, srcSurf->height, srcSurf, 0, 0);
                }
                else
                {
                    if (widget->flags & ITU_STRETCH)
                    {
                        ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
                        if (surf)
                        {
                            ituBitBlt(surf, 0, 0, rect->width, rect->height, dest, destx, desty);
                            ituStretchBlt(surf, 0, 0, rect->width, rect->height, srcSurf, 0, 0, srcSurf->width, srcSurf->height);
                            ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);
                            ituDestroySurface(surf);
                        }
                    }
                    else
                    {
                        ituAlphaBlend(dest, destx, desty, srcSurf->width, srcSurf->height, srcSurf, 0, 0, desta);
                    }
                }
            }
        }
        if (widget->angle == 0)
            ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);

        ituWidgetDrawImpl(widget, dest, x, y, alpha);
    }
    else
    {
        ituBackgroundDraw(widget, dest, x, y, alpha);
    }
}

void ituButtonOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    ITUButton* button = (ITUButton*) widget;
    assert(button);

    switch (action)
    {
    case ITU_ACTION_DODELAY0:
        ituExecActions(widget, button->actions, ITU_EVENT_DELAY0, atoi(param));
        break;

    case ITU_ACTION_DODELAY1:
        ituExecActions(widget, button->actions, ITU_EVENT_DELAY1, atoi(param));
        break;

    case ITU_ACTION_DODELAY2:
        ituExecActions(widget, button->actions, ITU_EVENT_DELAY2, atoi(param));
        break;

    case ITU_ACTION_DODELAY3:
        ituExecActions(widget, button->actions, ITU_EVENT_DELAY3, atoi(param));
        break;

    case ITU_ACTION_DODELAY4:
        ituExecActions(widget, button->actions, ITU_EVENT_DELAY4, atoi(param));
        break;

    case ITU_ACTION_DODELAY5:
        ituExecActions(widget, button->actions, ITU_EVENT_DELAY5, atoi(param));
        break;

    case ITU_ACTION_DODELAY6:
        ituExecActions(widget, button->actions, ITU_EVENT_DELAY6, atoi(param));
        break;

    case ITU_ACTION_DODELAY7:
        ituExecActions(widget, button->actions, ITU_EVENT_DELAY7, atoi(param));
        break;

    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

void ituButtonInit(ITUButton* btn)
{
    assert(btn);
    ITU_ASSERT_THREAD();

    memset(btn, 0, sizeof (ITUButton));

    ituBackgroundInit(&btn->bg);
    ituBorderWindowInit(&btn->bwin);
    ituTextInit(&btn->text);

    ituWidgetSetType(btn, ITU_BUTTON);
    ituWidgetSetName(btn, btnName);
    ituWidgetSetExit(btn, ituButtonExit);
    ituWidgetSetClone(btn, ituButtonClone);
    ituWidgetSetUpdate(btn, ituButtonUpdate);
    ituWidgetSetDraw(btn, ituButtonDraw);
    ituWidgetSetOnAction(btn, ituButtonOnAction);

    ituBorderWindowAdd(&btn->bwin, &btn->text.widget, ITU_LAYOUT_CENTER);
}

void ituButtonLoad(ITUButton* btn, uint32_t base)
{
    ITUWidget* widget = (ITUWidget*)btn;
    assert(btn);

    ituBackgroundLoad(&btn->bg, base);
    ituWidgetSetExit(btn, ituButtonExit);
    ituWidgetSetClone(btn, ituButtonClone);
    ituWidgetSetUpdate(btn, ituButtonUpdate);
    ituWidgetSetDraw(btn, ituButtonDraw);
    ituWidgetSetOnAction(btn, ituButtonOnAction);

    if (!(widget->flags & ITU_EXTERNAL))
    {
        if (btn->staticFocusSurf)
        {
            ITUSurface* surf = (ITUSurface*)(base + (uint32_t)btn->staticFocusSurf);
            if (surf->flags & ITU_COMPRESSED)
                btn->focusSurf = NULL;
            else
                btn->focusSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);

            btn->staticFocusSurf = surf;
        }

        if (btn->staticPressSurf)
        {
            ITUSurface* surf = (ITUSurface*)(base + (uint32_t)btn->staticPressSurf);
            if (surf->flags & ITU_COMPRESSED)
                btn->pressSurf = NULL;
            else
                btn->pressSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);

            btn->staticPressSurf = surf;
        }
    }
    btn->bwin.widgets[ITU_LAYOUT_CENTER] = &btn->text.widget;
}

void ituButtonSetPressed(ITUButton* btn, bool pressed)
{
    ITUWidget* widget = (ITUWidget*) btn;
    assert(btn);
    ITU_ASSERT_THREAD();

    if (pressed && btn->pressColor.alpha > 0)
    {
        ituWidgetSetColor(widget, btn->pressColor.alpha, btn->pressColor.red, btn->pressColor.green, btn->pressColor.blue);
    }
    else
    {
        if (widget->active && btn->focusColor.alpha > 0)
        {
            ituWidgetSetColor(widget, btn->focusColor.alpha, btn->focusColor.red, btn->focusColor.green, btn->focusColor.blue);
        }
        else if (btn->bgColor.alpha > 0)
        {
            ituWidgetSetColor(widget, btn->bgColor.alpha, btn->bgColor.red, btn->bgColor.green, btn->bgColor.blue);
        }
    }
    btn->pressed = pressed;
    widget->dirty = true;
}

void ituButtonSetStringImpl(ITUButton* btn, char* string)
{
    assert(btn);
    ITU_ASSERT_THREAD();

    ituTextSetString(&btn->text, string);
    ituTextResize(&btn->text);
    ituWidgetUpdate(btn, ITU_EVENT_LAYOUT, 0, 0, 0);
}

void ituButtonLoadStaticData(ITUButton* btn)
{
    ITUWidget* widget = (ITUWidget*)btn;
    ITUSurface* surf;

    if (widget->flags & ITU_EXTERNAL)
        return;

    if (btn->staticFocusSurf && !btn->focusSurf)
    {
        surf = btn->staticFocusSurf;

        if (surf->flags & ITU_COMPRESSED)
            btn->focusSurf = ituSurfaceDecompress(surf);
        else
            btn->focusSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }

    if (btn->staticPressSurf && !btn->pressSurf)
    {
        surf = btn->staticPressSurf;

        if (surf->flags & ITU_COMPRESSED)
            btn->pressSurf = ituSurfaceDecompress(surf);
        else
            btn->pressSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }
}

void ituButtonReleaseSurface(ITUButton* btn)
{
    ITU_ASSERT_THREAD();

    if (btn->pressSurf)
    {
        ituSurfaceRelease(btn->pressSurf);
        btn->pressSurf = NULL;
    }

    if (btn->focusSurf)
    {
        ituSurfaceRelease(btn->focusSurf);
        btn->focusSurf = NULL;
    }
}

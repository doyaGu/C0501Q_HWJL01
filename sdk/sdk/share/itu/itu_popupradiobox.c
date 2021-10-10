#include <assert.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char popupRadioBoxName[] = "ITUPopupRadioBox";

bool ituPopupRadioBoxUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUButton* prb = (ITUButton*) widget;
    ITUPopupRadioBox* popupRadioBox = (ITUPopupRadioBox*) widget;
    ITUEvent response_key = widget->flags & ITU_RESPONSE_TO_UP_KEY ? ITU_EVENT_MOUSEUP : ITU_EVENT_MOUSEDOWN;
    assert(popupRadioBox);

    result |= ituRadioBoxUpdate(widget, ev, arg1, arg2, arg3);

    if (ev == ITU_EVENT_MOUSEDOWN || ev == ITU_EVENT_MOUSEUP)
    {
        result |= ituIconUpdate(widget, ev, arg1, arg2, arg3);

        if (ituWidgetIsEnabled(widget) && popupRadioBox->frame == 0)
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                if (ev == response_key && prb->pressed)
                {
                    memcpy(&popupRadioBox->orgRect, &widget->rect, sizeof (ITURectangle));
                    popupRadioBox->orgAlpha = widget->alpha;

                    widget->rect.x = popupRadioBox->orgRect.x - popupRadioBox->orgRect.width * popupRadioBox->incPercent / (popupRadioBox->totalframe * 100);
                    widget->rect.y = popupRadioBox->orgRect.y - popupRadioBox->orgRect.height * popupRadioBox->incPercent / (popupRadioBox->totalframe * 100);
                    widget->rect.width = popupRadioBox->orgRect.width + popupRadioBox->orgRect.width * popupRadioBox->incPercent / (popupRadioBox->totalframe * 100 / 2);
                    widget->rect.height = popupRadioBox->orgRect.height + popupRadioBox->orgRect.height * popupRadioBox->incPercent / (popupRadioBox->totalframe * 100 / 2);

                    widget->alpha = popupRadioBox->orgAlpha * popupRadioBox->alphaPercent / 100;

                    ituWidgetUpdate(widget, ITU_EVENT_LAYOUT, 0, 0, 0);

                    popupRadioBox->frame = 1;
                }
                result |= widget->dirty;
            }
        }
    }
    else if (ev == ITU_EVENT_TIMER)
    {
        if (popupRadioBox->frame > 0)
        {
            if (popupRadioBox->frame >= popupRadioBox->totalframe - 1)
            {
                memcpy(&widget->rect, &popupRadioBox->orgRect, sizeof (ITURectangle));
                widget->alpha = popupRadioBox->orgAlpha;

                popupRadioBox->frame = 0;
            }
            else
            {
                int ratio, step;

                popupRadioBox->frame++;
                
                ratio = popupRadioBox->totalframe * 100 / 2;
                step = (popupRadioBox->frame > popupRadioBox->totalframe / 2) ? (popupRadioBox->totalframe - popupRadioBox->frame) : popupRadioBox->frame;

                widget->rect.x = popupRadioBox->orgRect.x - popupRadioBox->orgRect.width * step * popupRadioBox->incPercent / (popupRadioBox->totalframe * 100);
                widget->rect.y = popupRadioBox->orgRect.y - popupRadioBox->orgRect.height * step * popupRadioBox->incPercent / (popupRadioBox->totalframe * 100);
                widget->rect.width = popupRadioBox->orgRect.width + popupRadioBox->orgRect.width * step * popupRadioBox->incPercent / (popupRadioBox->totalframe * 100 / 2);
                widget->rect.height = popupRadioBox->orgRect.height + popupRadioBox->orgRect.height * step * popupRadioBox->incPercent / (popupRadioBox->totalframe * 100 / 2);

                widget->alpha = popupRadioBox->orgAlpha * popupRadioBox->alphaPercent / 100;
            }
            ituWidgetUpdate(widget, ITU_EVENT_LAYOUT, 0, 0, 0);
            result = widget->dirty = true;
        }
    }
    return widget->visible ? result : false;
}

void ituPopupRadioBoxDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    int destx, desty;
    uint8_t desta;
    ITURectangle prevClip;
    ITUCheckBox* checkbox = (ITUCheckBox*) widget;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    assert(checkbox);
    assert(dest);

    if ((checkbox->checked && checkbox->checkedSurf) || (widget->active && checkbox->checked && checkbox->focusCheckedSurf))
    {
        ITUSurface* srcSurf = checkbox->checkedSurf;

        destx = rect->x + x;
        desty = rect->y + y;
        desta = alpha * widget->color.alpha / 255;
        desta = desta * widget->alpha / 255;

        ituWidgetSetClipping(widget, dest, x, y, &prevClip);

        srcSurf = checkbox->checkedSurf;

        if (widget->active && checkbox->focusCheckedSurf)
            srcSurf = checkbox->focusCheckedSurf;

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
        ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
        ituWidgetDrawImpl(widget, dest, x, y, alpha);
    }
    else
    {
        ituButtonDraw(widget, dest, x, y, alpha);
    }
}

void ituPopupRadioBoxInit(ITUPopupRadioBox* popupRadioBox)
{
    assert(popupRadioBox);
    ITU_ASSERT_THREAD();

    memset(popupRadioBox, 0, sizeof (ITUPopupRadioBox));

    ituRadioBoxInit(&popupRadioBox->rb);

    ituWidgetSetType(popupRadioBox, ITU_POPUPRADIOBOX);
    ituWidgetSetName(popupRadioBox, popupRadioBoxName);
    ituWidgetSetUpdate(popupRadioBox, ituPopupRadioBoxUpdate);
    ituWidgetSetDraw(popupRadioBox, ituPopupRadioBoxDraw);
}

void ituPopupRadioBoxLoad(ITUPopupRadioBox* popupRadioBox, uint32_t base)
{
    assert(popupRadioBox);

    ituRadioBoxLoad(&popupRadioBox->rb, base);
    ituWidgetSetUpdate(popupRadioBox, ituPopupRadioBoxUpdate);
    ituWidgetSetDraw(popupRadioBox, ituPopupRadioBoxDraw);
}

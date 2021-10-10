#include <assert.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char cpName[] = "ITUColorPicker";

bool ituColorPickerUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUColorPicker* cp = (ITUColorPicker*) widget;
    assert(cp);

    result |= ituIconUpdate(widget, ev, arg1, arg2, arg3);

    if (ev == ITU_EVENT_MOUSEDOWN)
    {
        if (ituWidgetIsEnabled(widget) && !result)
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                cp->touchX = x;
                cp->touchY = y;
                widget->flags |= ITU_DRAGGING;
                cp->colorPickerFlags |= ITU_COLORPICKER_PICKING;
                result = true;
            }
        }
    }
    else if (ev == ITU_EVENT_MOUSEUP)
    {
        if (ituWidgetIsEnabled(widget))
        {
            widget->flags &= ~ITU_DRAGGING;
        }
    }
    else if (ev == ITU_EVENT_MOUSEMOVE)
    {
        if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGING))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                cp->touchX = x;
                cp->touchY = y;
                cp->colorPickerFlags |= ITU_COLORPICKER_PICKING;
            }
            result = true;
        }
    }
    else if (ev == ITU_EVENT_TIMER)
    {
        if (ituWidgetIsEnabled(widget) && (cp->colorPickerFlags & ITU_COLORPICKER_PICKED))
        {
            int value = (int)ITH_ARGB8888(cp->pickedColor.alpha, cp->pickedColor.red, cp->pickedColor.green, cp->pickedColor.blue);

            ituExecActions(widget, cp->actions, ITU_EVENT_SELECT, value);
            cp->colorPickerFlags &= ~ITU_COLORPICKER_PICKED;
        }
    }
    return result;
}

void ituColorPickerDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    ITUColorPicker* cp = (ITUColorPicker*) widget;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    assert(cp);
    assert(dest);

    ituBackgroundDraw(widget, dest, x, y, alpha);

    if (cp->colorPickerFlags & ITU_COLORPICKER_PICKING)
    {
        int destx, desty;

        destx = rect->x + x;
        desty = rect->y + y;

        if (dest->format == ITU_RGB565)
        {
            uint16_t* ptr = (uint16_t*)ituLockSurface(dest, destx + cp->touchX, desty + cp->touchY, 1, 1);
            if (ptr)
            {
                //printf("*ptr=0x%04X\n", *ptr);
                cp->pickedColor.alpha = 255;
                cp->pickedColor.red = ((*ptr & 0xf800) >> 11) << 3;
                cp->pickedColor.green = ((*ptr & 0x07e0) >> 5) << 2;
                cp->pickedColor.blue = (*ptr & 0x001f) << 3;
                ituUnlockSurface(dest);
            }
        }
        else if (dest->format == ITU_ARGB8888)
        {
            uint32_t* ptr = (uint32_t*)ituLockSurface(dest, destx + cp->touchX, desty + cp->touchY, 1, 1);
            if (ptr)
            {
                cp->pickedColor.alpha = (*ptr & 0xFF000000) >> 24;
                cp->pickedColor.red = (*ptr & 0x00FF0000) >> 16;
                cp->pickedColor.green = (*ptr & 0x0000FF00) >> 8;
                cp->pickedColor.blue = *ptr & 0x000000FF;
                ituUnlockSurface(dest);
            }
        }
        else
        {
            LOG_WARN "Unsupport pixel format: %d\n", dest->format LOG_END
        }
        cp->colorPickerFlags &= ~ITU_COLORPICKER_PICKING;
        cp->colorPickerFlags |= ITU_COLORPICKER_PICKED;
    }
}

void ituColorPickerInit(ITUColorPicker* cp)
{
    assert(cp);
    ITU_ASSERT_THREAD();

    memset(cp, 0, sizeof (ITUColorPicker));

    ituBackgroundInit(&cp->bg);
    
    ituWidgetSetType(cp, ITU_COLORPICKER);
    ituWidgetSetName(cp, cpName);
    ituWidgetSetUpdate(cp, ituColorPickerUpdate);
    ituWidgetSetDraw(cp, ituColorPickerDraw);
}

void ituColorPickerLoad(ITUColorPicker* cp, uint32_t base)
{
    assert(cp);

    ituBackgroundLoad(&cp->bg, base);
    ituWidgetSetUpdate(cp, ituColorPickerUpdate);
    ituWidgetSetDraw(cp, ituColorPickerDraw);
}

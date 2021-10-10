#include <assert.h>
#include <malloc.h>
#include <math.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char rbName[] = "ITURippleBackground";
#define NN 1024

void ituRippleBackgroundExit(ITUWidget* widget)
{
    ITURippleBackground* rb = (ITURippleBackground*) widget;
    assert(widget);
    ITU_ASSERT_THREAD();

    if (rb->rippleMap)
    {
        free(rb->rippleMap);
        rb->rippleMap = NULL;
    }
    ituIconExit(widget);
}

bool ituRippleBackgroundUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUIcon* icon = (ITUIcon*) widget;
    ITURippleBackground* rb = (ITURippleBackground*) widget;
    assert(rb);

    if (ev == ITU_EVENT_MOUSEDOWN)
    {
        if (ituWidgetIsEnabled(widget))
        {
            int dx = arg2 - widget->rect.x;
            int dy = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, dx, dy))
            {
                if (icon->surf)
                {
                    int width, height, radius;
                    ripple_t* rippleMap = NULL;

                    width = icon->surf->width;
                    height = icon->surf->height;
                    radius = rb->radius;

                    if (rb->rippleMap)
                    {
                        rippleMap = rb->rippleMap;
                    }
                    else
                    {
                        int size = width * (height + 2) * 2;
                        rippleMap = rb->rippleMap = calloc(sizeof(ripple_t), size);

                        rb->oldIndex = width;
                        rb->newIndex = width * (height + 3);
                    }

                    if (rippleMap)
                    {
                        int j, k, oldIndex;

                        oldIndex = rb->oldIndex;

                        for (j = dy - radius; j < dy + radius; j++)
                        {
                            for (k = dx - radius; k < dx + radius; k++)
                            {
                                if (j >= 0 && j < height && k >= 0 && k < width)
                                {
                                    rippleMap[oldIndex + (j * width) + k] += (NN/2);
                                }
                            }
                        }
                        rb->rippleBackgroundFlags |= (ITU_RIPPLEBACKGROUND_DROPPING | ITU_RIPPLEBACKGROUND_DRAGGING);
                    }
                }
            }
            result = true;
        }
    }
    else if (ev == ITU_EVENT_MOUSEUP)
    {
        if (ituWidgetIsEnabled(widget))
        {
            rb->rippleBackgroundFlags &= ~ITU_RIPPLEBACKGROUND_DRAGGING;
        }
    }
    else if (ev == ITU_EVENT_MOUSEMOVE)
    {
        if (ituWidgetIsEnabled(widget) && (rb->rippleBackgroundFlags & ITU_RIPPLEBACKGROUND_DRAGGABLE) && (rb->rippleBackgroundFlags & ITU_RIPPLEBACKGROUND_DRAGGING))
        {
            int dx = arg2 - widget->rect.x;
            int dy = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, dx, dy))
            {
                int width, height, radius, j, k, oldIndex;
                ripple_t* rippleMap;

                assert(icon->surf);
                assert(rb->rippleMap);

                width = icon->surf->width;
                height = icon->surf->height;
                radius = rb->radius;
                rippleMap = rb->rippleMap;
                oldIndex = rb->oldIndex;

                for (j = dy - radius; j < dy + radius; j++)
                {
                    for (k = dx - radius; k < dx + radius; k++)
                    {
                        if (j >= 0 && j < height && k >= 0 && k < width)
                        {
                            rippleMap[oldIndex + (j * width) + k] += (NN/2);
                        }
                    }
                }
            }
            result = true;
        }
    }
    else if (ev == ITU_EVENT_TIMER)
    {
        if (ituWidgetIsEnabled(widget) && (rb->rippleBackgroundFlags & ITU_RIPPLEBACKGROUND_DROPPING))
        {
            result = true;
        }
    }
    result |= ituIconUpdate(widget, ev, arg1, arg2, arg3);
    return result;
}

void ituRippleBackgroundDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    ITURippleBackground* rb = (ITURippleBackground*) widget;
    assert(rb);
    assert(dest);

    if (rb->rippleBackgroundFlags & ITU_RIPPLEBACKGROUND_DROPPING)
    {
        ITUIcon* icon = (ITUIcon*) widget;
        ITURectangle* rect = (ITURectangle*) &widget->rect;
        ITURectangle prevClip;
        int width, height, destx, desty, hwidth, hheight;
        uint8_t desta;
        ripple_t* rippleMap = rb->rippleMap;
        ITUSurface* surf = NULL;
        bool hasRipple = false;

        assert(icon->surf);

        width = icon->surf->width;
        height = icon->surf->height;
        destx = rect->x + x;
        desty = rect->y + y;
        desta = alpha * widget->color.alpha / 255;
        desta = desta * widget->alpha / 255;
        hwidth = width >> 1;
        hheight = height >> 1;

        ituWidgetSetClipping(widget, dest, x, y, &prevClip);

        if (icon->surf->format == ITU_RGB565)
        {
            uint16_t* srcPtr = (uint16_t*)ituLockSurface(icon->surf, 0, 0, width, height);
            if (srcPtr)
            {
                surf = ituCreateSurface(width, height, 0, icon->surf->format, NULL, 0);
                if (surf)
                {
                    uint16_t* destPtr = (uint16_t*)ituLockSurface(surf, 0, 0, width, height);
                    if (destPtr)
                    {
                        int y, x, a, b, i, mapIndex, newIndex, range;

                        // toggle maps each frame
                        i = rb->oldIndex;
                        rb->oldIndex = rb->newIndex;
                        rb->newIndex = i;

                        mapIndex = rb->oldIndex;
                        newIndex = rb->newIndex;
                        i = 0;
                        range = rb->range;

                        for (y = 0; y < height; y++)
                        {
                            for (x = 0; x < width; x++)
                            {
                                ripple_t data = (ripple_t)((rippleMap[mapIndex - width] + rippleMap[mapIndex + width] + rippleMap[mapIndex - 1] + rippleMap[mapIndex + 1]) >> 1);
                                data -= rippleMap[newIndex + i];
                                data -= data >> range;
                                rippleMap[newIndex + i] = data;

                                if (data)
                                {
                                    // where data=0 then still, where data>0 then wave
                                    data = (ripple_t)(NN - data);

                                    // offsets
                                    a = ((x - hwidth) * data / NN) + hwidth;
                                    b = ((y - hheight) * data / NN) + hheight;

                                    // bounds check
                                    if (a >= width)
                                        a = width - 1;

                                    if (a < 0)
                                        a = 0;

                                    if (b >= height)
                                        b = height - 1;

                                    if (b < 0)
                                        b = 0;

                                    destPtr[i] = srcPtr[a + (b * width)];

                                    hasRipple = true;
                                }
                                else
                                {
                                    destPtr[i] = srcPtr[i];
                                }
                                mapIndex++;
                                i++;
                            }
                        }
                        ituUnlockSurface(surf);
                    }
                }
                ituUnlockSurface(icon->surf);
            }
        }
        else if (icon->surf->format == ITU_ARGB8888)
        {
            uint32_t* srcPtr = (uint32_t*)ituLockSurface(icon->surf, 0, 0, width, height);
            if (srcPtr)
            {
                surf = ituCreateSurface(width, height, 0, icon->surf->format, NULL, 0);
                if (surf)
                {
                    uint32_t* destPtr = (uint32_t*)ituLockSurface(surf, 0, 0, width, height);
                    if (destPtr)
                    {
                        int y, x, a, b, i, mapIndex, newIndex;

                        // toggle maps each frame
                        i = rb->oldIndex;
                        rb->oldIndex = rb->newIndex;
                        rb->newIndex = i;

                        i = 0;
                        mapIndex = rb->oldIndex;
                        newIndex = rb->newIndex;
                        for (y = 0; y < height; y++)
                        {
                            for (x = 0; x < width; x++)
                            {
                                ripple_t data = (ripple_t)((rippleMap[mapIndex - width] + rippleMap[mapIndex + width] + rippleMap[mapIndex - 1] + rippleMap[mapIndex + 1]) >> 1);
                                data -= rippleMap[newIndex + i];
                                data -= data >> 5;
                                rippleMap[newIndex + i] = data;

                                if (data)
                                {
                                    // where data=0 then still, where data>0 then wave
                                    data = (ripple_t)(NN - data);

                                    // offsets
                                    a = ((x - hwidth) * data / NN) + hwidth;
                                    b = ((y - hheight) * data / NN) + hheight;

                                    // bounds check
                                    if (a >= width)
                                        a = width - 1;

                                    if (a < 0)
                                        a = 0;

                                    if (b >= height)
                                        b = height - 1;

                                    if (b < 0)
                                        b = 0;

                                    destPtr[i] = srcPtr[a + (b * width)];

                                    hasRipple = true;
                                }
                                else
                                {
                                    destPtr[i] = srcPtr[i];
                                }
                                mapIndex++;
                                i++;
                            }
                        }
                        ituUnlockSurface(surf);
                    }
                }
                ituUnlockSurface(icon->surf);
            }
        }
        else
        {
            LOG_WARN "Unsupport pixel format: %d\n", dest->format LOG_END
        }
        ituUnlockSurface(dest);

        if (surf)
        {
            if (desta > 0)
            {
                if (desta == 255)
                {
                    ituBitBlt(dest, destx, desty, rect->width, rect->height, surf, 0, 0);
                }
                else
                {
                    ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);
                }
            }
            ituDestroySurface(surf);
        }

        ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);

        if (!hasRipple)
            rb->rippleBackgroundFlags &= ~ITU_RIPPLEBACKGROUND_DROPPING;
    }
    else
    {
        ituBackgroundDraw(widget, dest, x, y, alpha);
    }
}

void ituRippleBackgroundInit(ITURippleBackground* rb)
{
    assert(rb);
    ITU_ASSERT_THREAD();

    memset(rb, 0, sizeof (ITURippleBackground));

    ituBackgroundInit(&rb->bg);

    ituWidgetSetType(rb, ITU_RIPPLEBACKGROUND);
    ituWidgetSetName(rb, rbName);
    ituWidgetSetExit(rb, ituRippleBackgroundExit);
    ituWidgetSetUpdate(rb, ituRippleBackgroundUpdate);
    ituWidgetSetDraw(rb, ituRippleBackgroundDraw);
}

void ituRippleBackgroundLoad(ITURippleBackground* rb, uint32_t base)
{
    assert(rb);

    ituBackgroundLoad(&rb->bg, base);
    ituWidgetSetExit(rb, ituRippleBackgroundExit);
    ituWidgetSetUpdate(rb, ituRippleBackgroundUpdate);
    ituWidgetSetDraw(rb, ituRippleBackgroundDraw);
}

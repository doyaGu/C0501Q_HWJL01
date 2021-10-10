#include <assert.h>
#include <malloc.h>
#include <math.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char blurName[] = "ITUBlur";

void ituBlurExit(ITUWidget* widget)
{
    ITUBlur* blur = (ITUBlur*) widget;
    assert(blur);
    ITU_ASSERT_THREAD();

    if (blur->maskSurf)
    {
        ituSurfaceRelease(blur->maskSurf);
        blur->maskSurf = NULL;
    }

    ituWidgetExitImpl(&blur->widget);
}

bool ituBlurClone(ITUWidget* widget, ITUWidget** cloned)
{
    ITUBlur* blur = (ITUBlur*)widget;
    ITUBlur* newBlur;
    assert(widget);
    assert(cloned);
    ITU_ASSERT_THREAD();

    if (*cloned == NULL)
    {
        ITUWidget* newWidget = malloc(sizeof(ITUBlur));
        if (newWidget == NULL)
            return false;

        memcpy(newWidget, widget, sizeof(ITUBlur));
        newWidget->tree.child = newWidget->tree.parent = newWidget->tree.sibling = NULL;
        *cloned = newWidget;
    }

    newBlur = (ITUBlur*)*cloned;
    newBlur->maskSurf = NULL;

    return ituWidgetCloneImpl(widget, cloned);
}

bool ituBlurUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUBlur* blur = (ITUBlur*) widget;
    assert(blur);

    if (ev == ITU_EVENT_LAYOUT)
    {
        if (blur->maskSurf)
        {
            ituSurfaceRelease(blur->maskSurf);
            blur->maskSurf = NULL;
        }
    }
    result |= ituWidgetUpdateImpl(widget, ev, arg1, arg2, arg3);
    
    return widget->visible ? result : false;
}

void ituBlurDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    int destx, desty;
    ITUBlur* blur = (ITUBlur*) widget;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    assert(blur);
    assert(dest);

    destx = rect->x + x;
    desty = rect->y + y;

    blur->factor = 50;
    blur->iter = 4;

    if (!blur->maskSurf && blur->factor > 0)
    {
        ITUSurface* surf;

        surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
        if (surf)
        {
            int w = blur->factor * rect->width / 100;
            int h = blur->factor * rect->height / 100;
            ITUSurface* surf2 = ituCreateSurface(w, h, 0, dest->format, NULL, 0);

            if (surf2)
            {
                int i;

                ituBitBlt(surf, 0, 0, rect->width, rect->height, dest, destx, desty);

                for (i = 0; i < blur->iter; i++)
                {
                    ituStretchBlt(surf2, 0, 0, w, h, surf, 0, 0, rect->width, rect->height);
                    ituStretchBlt(surf, 0, 0, rect->width, rect->height, surf2, 0, 0, w, h);
                }
                ituDestroySurface(surf2);

                blur->maskSurf = surf;
            }
        }
    }

    if (blur->maskSurf)
    {
        ituBitBlt(dest, destx, desty, rect->width, rect->height, blur->maskSurf, 0, 0);
        if (widget->flags & ITU_PROGRESS)
        {
            ituSurfaceRelease(blur->maskSurf);
            blur->maskSurf = NULL;
        }
    }
    ituWidgetDrawImpl(widget, dest, x, y, alpha);
}

void ituBlurInit(ITUBlur* blur)
{
    assert(blur);
    ITU_ASSERT_THREAD();

    memset(blur, 0, sizeof (ITUBlur));

    ituWidgetInit(&blur->widget);

    ituWidgetSetType(blur, ITU_LAYER);
    ituWidgetSetName(blur, blurName);
    ituWidgetSetExit(blur, ituBlurExit);
    ituWidgetSetClone(blur, ituBlurClone);
    ituWidgetSetUpdate(blur, ituBlurUpdate);
    ituWidgetSetDraw(blur, ituBlurDraw);
}

void ituBlurLoad(ITUBlur* blur, uint32_t base)
{
    assert(blur);

    ituWidgetLoad((ITUWidget*)blur, base);
    ituWidgetSetExit(blur, ituBlurExit);
    ituWidgetSetClone(blur, ituBlurClone);
    ituWidgetSetUpdate(blur, ituBlurUpdate);
    ituWidgetSetDraw(blur, ituBlurDraw);
}

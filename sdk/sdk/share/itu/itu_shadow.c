#include <assert.h>
#include <malloc.h>
#include <math.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char shadowName[] = "ITUShadow";

void ituShadowExit(ITUWidget* widget)
{
    ITUShadow* shadow = (ITUShadow*) widget;
    assert(shadow);
    ITU_ASSERT_THREAD();

    if (shadow->maskSurf)
    {
        ituSurfaceRelease(shadow->maskSurf);
        shadow->maskSurf = NULL;
    }

    ituWidgetExitImpl(&shadow->widget);
}

bool ituShadowClone(ITUWidget* widget, ITUWidget** cloned)
{
    ITUShadow* shadow = (ITUShadow*)widget;
    ITUShadow* newShadow;
    ITURectangle* rect;
    ITUSurface* surf;
    assert(widget);
    assert(cloned);
    ITU_ASSERT_THREAD();

    if (*cloned == NULL)
    {
        ITUWidget* newWidget = malloc(sizeof(ITUShadow));
        if (newWidget == NULL)
            return false;

        memcpy(newWidget, widget, sizeof(ITUShadow));
        newWidget->tree.child = newWidget->tree.parent = newWidget->tree.sibling = NULL;
        *cloned = newWidget;
    }

    newShadow = (ITUShadow*)*cloned;
    newShadow->maskSurf = NULL;

    rect = (ITURectangle*) &widget->rect;
    surf = ituCreateSurface(rect->width, rect->height, 0, ITU_A8, NULL, 0);
    if (surf)
    {
        ITUColor startColor, endColor;

        ituSetColor(&startColor, widget->color.alpha, widget->color.alpha, widget->color.alpha, widget->color.alpha);
        ituSetColor(&endColor, 0, 0, 0, 0);
        ituGradientFill(surf, 0, 0, rect->width, rect->height, &startColor, &endColor, ITU_GF_VERTICAL);
        newShadow->maskSurf = surf;
    }
    return ituWidgetCloneImpl(widget, cloned);
}

bool ituShadowUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUShadow* shadow = (ITUShadow*) widget;
    assert(shadow);

    if (ev == ITU_EVENT_LAYOUT)
    {
        if (!shadow->target && (shadow->targetName[0] != '\0'))
        {
            shadow->target = (ITUWidget*) ituSceneFindWidget(ituScene, shadow->targetName);
        }

        if (!shadow->maskSurf)
        {
            ITURectangle* rect = (ITURectangle*) &widget->rect;
            ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, ITU_A8, NULL, 0);
            if (surf)
            {
                ITUColor startColor, endColor;

                ituSetColor(&startColor, widget->color.alpha, widget->color.alpha, widget->color.alpha, widget->color.alpha);
                ituSetColor(&endColor, 0, 0, 0, 0);
                ituGradientFill(surf, 0, 0, rect->width, rect->height, &startColor, &endColor, ITU_GF_VERTICAL);
                shadow->maskSurf = surf;
            }
        }
    }
    result |= ituWidgetUpdateImpl(widget, ev, arg1, arg2, arg3);
    
    return widget->visible ? result : false;
}

void ituShadowDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    int destx, desty;
    ITUShadow* shadow = (ITUShadow*) widget;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    assert(shadow);
    assert(dest);

    destx = rect->x + x;
    desty = rect->y + y;

    if (shadow->target)
    {
        ITUSurface* surf;

        surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
        if (surf)
        {
            ITUWidget* target;
            ITURectangle* targetRect;
            int starty;

            if (shadow->target->type == ITU_WHEEL)
            {
                ITUWheel* wheel = (ITUWheel*)shadow->target;

                if (wheel->inc > 0)
                {
                    target = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->itemCount / 2 + wheel->focusIndex - 1);
                    desty = desty - wheel->inc + wheel->inc * wheel->frame / wheel->totalframe;
                }
                else if (wheel->inc < 0)
                {
                    target = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->itemCount / 2 + wheel->focusIndex + 1);
                    desty = desty - wheel->inc + wheel->inc * wheel->frame / wheel->totalframe;
                }
                else
                {
                    target = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->itemCount / 2 + wheel->focusIndex);
                }
            }
            else
            {
                target = shadow->target;
            }
            targetRect = &target->rect;

            if (shadow->maskSurf)
            {
                ITUSurface* surf2 = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);

                if (surf2)
                {
                    ituBitBlt(surf, 0, 0, rect->width, rect->height, dest, destx, desty);
                    ituReflected(surf2, 0, 0, surf, 0, 0, rect->width, rect->height, NULL);

                    if (rect->height < targetRect->height)
                        starty = targetRect->height - rect->height;
                    else
                        starty = 0;

                    ituSurfaceSetClipping(surf2, 0, 0, targetRect->width, targetRect->height);
                    ituWidgetDraw(target, surf2, -targetRect->x, -targetRect->y - starty, 255);

                    ituReflected(surf, 0, 0, surf2, 0, 0, rect->width, rect->height, shadow->maskSurf);
                    ituBitBlt(dest, destx, desty, rect->width, rect->height, surf, 0, 0);

                    ituDestroySurface(surf2);
                }
            }
            ituDestroySurface(surf);
        }
    }
    ituWidgetDrawImpl(widget, dest, x, y, alpha);
}

void ituShadowInit(ITUShadow* shadow)
{
    assert(shadow);
    ITU_ASSERT_THREAD();

    memset(shadow, 0, sizeof (ITUShadow));

    ituWidgetInit(&shadow->widget);

    ituWidgetSetType(shadow, ITU_LAYER);
    ituWidgetSetName(shadow, shadowName);
    ituWidgetSetExit(shadow, ituShadowExit);
    ituWidgetSetClone(shadow, ituShadowClone);
    ituWidgetSetUpdate(shadow, ituShadowUpdate);
    ituWidgetSetDraw(shadow, ituShadowDraw);
}

void ituShadowLoad(ITUShadow* shadow, uint32_t base)
{
    assert(shadow);

    ituWidgetLoad((ITUWidget*)shadow, base);
    ituWidgetSetExit(shadow, ituShadowExit);
    ituWidgetSetClone(shadow, ituShadowClone);
    ituWidgetSetUpdate(shadow, ituShadowUpdate);
    ituWidgetSetDraw(shadow, ituShadowDraw);

    if (shadow->target)
        shadow->target = (ITUWidget*)((uint32_t)shadow->target + base);
}

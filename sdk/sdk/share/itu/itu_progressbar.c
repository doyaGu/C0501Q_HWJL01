#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char progressBarName[] = "ITUProgressBar";

void ituProgressBarExit(ITUWidget* widget)
{
    ITUProgressBar* bar = (ITUProgressBar*) widget;
    assert(widget);
    ITU_ASSERT_THREAD();

    if (bar->barSurf)
    {
        ituSurfaceRelease(bar->barSurf);
        bar->barSurf = NULL;
    }
    ituIconExit(widget);
}

static void ProgressBarLoadExternalData(ITUProgressBar* bar, ITULayer* layer)
{
    ITUWidget* widget = (ITUWidget*)bar;
    ITUSurface* surf;

    assert(widget);

    if (!(widget->flags & ITU_EXTERNAL))
        return;

    if (!layer)
        layer = ituGetLayer(widget);

    if (bar->staticBarSurf && !bar->barSurf)
    {
        surf = ituLayerLoadExternalSurface(layer, (uint32_t)bar->staticBarSurf);

        if (surf->flags & ITU_COMPRESSED)
            bar->barSurf = ituSurfaceDecompress(surf);
        else
            bar->barSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }
}

bool ituProgressBarUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUProgressBar* bar = (ITUProgressBar*) widget;
    assert(bar);

    if (ev == ITU_EVENT_LAYOUT)
    {
        if (!bar->valueText && (bar->valueName[0] != '\0'))
        {
            bar->valueText = (ITUText*) ituSceneFindWidget(ituScene, bar->valueName);
        }

        if (bar->valueText)
        {
            char buf[8];
            sprintf(buf, "%i", bar->value);
            ituTextSetString(bar->valueText, buf);
        }
    }
    else if (ev == ITU_EVENT_LOAD)
    {
        ituProgressBarLoadStaticData(bar);
        result = true;
    }
    else if (ev == ITU_EVENT_LOAD_EXTERNAL)
    {
        ProgressBarLoadExternalData(bar, (ITULayer*)arg1);
        result = true;
    }
    else if (ev == ITU_EVENT_RELEASE)
    {
        ituProgressBarReleaseSurface(bar);
        result = true;
    }
    result |= ituIconUpdate(widget, ev, arg1, arg2, arg3);

    return widget->visible ? result : false;
}

void ituProgressBarDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    int destx, desty;
    uint8_t desta;
    ITURectangle prevClip;
    ITUIcon* icon = (ITUIcon*) widget;
    ITUBackground* bg = (ITUBackground*) widget;
    ITUProgressBar* bar = (ITUProgressBar*) widget;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    assert(bar);
    assert(dest);

    destx = rect->x + x;
    desty = rect->y + y;
    desta = alpha * widget->alpha / 255;

    ituWidgetSetClipping(widget, dest, x, y, &prevClip);

    if (desta == 255)
    {
        if (bar->layout == ITU_LAYOUT_HORIZONTAL)
        {
            int width = rect->width * (bar->value - bar->min) / (bar->max - bar->min);

            if (icon->surf)
            {
                ituBitBlt(dest, destx + width, desty, rect->width - width, rect->height, icon->surf, width, 0);
            }
            else
            {
                if (widget->color.alpha == 255)
                {
                    ituColorFill(dest, destx + width, desty, rect->width - width, rect->height, &widget->color);
                }
                else if (widget->color.alpha > 0)
                {
                    ITUSurface* surf = ituCreateSurface(rect->width - width, rect->height, 0, dest->format, NULL, 0);
                    if (surf)
                    {
                        ituColorFill(surf, 0, 0, rect->width - width, rect->height, &widget->color);
                        ituAlphaBlend(dest, destx + width, desty, rect->width - width, rect->height, surf, 0, 0, widget->color.alpha);                
                        ituDestroySurface(surf);
                    }
                }
            }

            if (width > 0)
            {
				ituSurfaceSetClipping(dest, destx, desty, width, rect->height);

                if (bar->barSurf)
                {
                    ituBitBlt(dest, destx, desty, width, rect->height, bar->barSurf, 0, 0);
                }
                else
                {
                    if (bg->graidentMode == ITU_GF_NONE)
                    {
                        ituColorFill(dest, destx, desty, width, rect->height, &bar->fgColor);
                    }
                    else
                    {
						if (desty + rect->height < dest->height)
						{
							//ituGradientFill(dest, destx, desty, width, rect->height, &bar->fgColor, &bg->graidentColor, bg->graidentMode);
							ituGradientFill(dest, destx, desty, rect->width, rect->height, &bar->fgColor, &bg->graidentColor, bg->graidentMode);
						}
                    }
                }
            }
        }
        else //if (bar->layout == ITU_LAYOUT_VERTICAL)
        {
            int height = rect->height * (bar->value - bar->min) / (bar->max - bar->min);

            if (icon->surf)
            {
                ituBitBlt(dest, destx, desty, rect->width, rect->height - height, icon->surf, 0, 0);
            }
            else
            {
                if (widget->color.alpha == 255)
                {
                    ituColorFill(dest, destx, desty, rect->width, rect->height - height, &widget->color);
                }
                else if (widget->color.alpha > 0)
                {
                    ITUSurface* surf = ituCreateSurface(rect->width, rect->height - height, 0, dest->format, NULL, 0);
                    if (surf)
                    {
                        ituColorFill(surf, 0, 0, rect->width, rect->height - height, &widget->color);
                        ituAlphaBlend(dest, destx, desty, rect->width, rect->height - height, surf, 0, 0, widget->color.alpha);                
                        ituDestroySurface(surf);
                    }
                }
            }

            if (height > 0)
            {
				ituSurfaceSetClipping(dest, destx, desty + rect->height - height, rect->width, height);

                if (bar->barSurf)
                {
                    ituBitBlt(dest, destx, desty + rect->height - height, rect->width, height, bar->barSurf, 0, rect->height - height);
                }
                else
                {
                    if (bg->graidentMode == ITU_GF_NONE)
                        ituColorFill(dest, destx, desty + rect->height - height, rect->width, height, &bar->fgColor);
					else
					{
						//ituGradientFill(dest, destx, desty + rect->height - height, rect->width, height, &bar->fgColor, &bg->graidentColor, bg->graidentMode);
						ituGradientFill(dest, destx, desty, rect->width, rect->height, &bar->fgColor, &bg->graidentColor, bg->graidentMode);
					}
                }
            }
        }
    }
    else if (desta > 0)
    {

        if (bar->layout == ITU_LAYOUT_HORIZONTAL)
        {
            int width = rect->width * (bar->value - bar->min) / (bar->max - bar->min);
            if (width > 0)
            {
                ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
                if (surf)
                {
                    ituBitBlt(surf, 0, 0, rect->width, rect->height, dest, destx, desty);

                    if (icon->surf)
                    {
                        ituBitBlt(surf, width, 0, rect->width - width, rect->height, icon->surf, width, 0);
                    }
                    else
                    {
                        if (widget->color.alpha == 255)
						    ituColorFill(surf, width, 0, rect->width - width, rect->height, &widget->color);
                    }

					ituSurfaceSetClipping(surf, 0, 0, width, rect->height);

                    if (bar->barSurf)
                    {
                        ituBitBlt(surf, 0, 0, width, rect->height, bar->barSurf, 0, 0);
                    }
                    else
                    {
                        if (bg->graidentMode == ITU_GF_NONE)
                        {
                            ituColorFill(surf, 0, 0, width, rect->height, &bar->fgColor);
                        }
                        else
                        {
							if (rect->height < dest->height)
							{
								ituGradientFill(surf, 0, 0, rect->width, rect->height, &bar->fgColor, &bg->graidentColor, bg->graidentMode);
							}
                        }
                    }
                    ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);                
                    ituDestroySurface(surf);
                }
            }
        }
        else //if (bar->layout == ITU_LAYOUT_VERTICAL)
        {
            int height = rect->height * (bar->value - bar->min) / (bar->max - bar->min);
            if (height > 0)
            {
                ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
                if (surf)
                {
                    ituBitBlt(surf, 0, 0, rect->width, rect->height, dest, destx, desty);

                    if (icon->surf)
                    {
                        ituBitBlt(surf, 0, 0, rect->width, rect->height - height, icon->surf, 0, 0);
                    }
					else
					{
                        if (widget->color.alpha == 255)
                            ituColorFill(surf, 0, 0, rect->width, rect->height, &widget->color);
                    }
					ituSurfaceSetClipping(surf, 0, rect->height - height, rect->width, height);

					if (bar->barSurf)
					{
						ituBitBlt(surf, 0, rect->height - height, rect->width, height, bar->barSurf, 0, rect->height - height);
					}
					else
					{
						if (bg->graidentMode == ITU_GF_NONE)
                        {
							ituColorFill(surf, 0, rect->height - height, rect->width, height, &bar->fgColor);
                        }
						else
						{
							ituGradientFill(surf, 0, 0, rect->width, rect->height, &bar->fgColor, &bg->graidentColor, bg->graidentMode);
						}
					}
                }
                ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);                
                ituDestroySurface(surf);
            }
        }

    }
    ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
}

void ituProgressBarOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    assert(widget);

    switch (action)
    {
    case ITU_ACTION_GOTO:
        if (param[0] != '\0')
            ituProgressBarSetValue((ITUProgressBar*)widget, atoi(param));
        break;

    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

void ituProgressBarInit(ITUProgressBar* bar, ITULayout layout)
{
    assert(bar);
    ITU_ASSERT_THREAD();

    memset(bar, 0, sizeof (ITUProgressBar));

    ituBackgroundInit(&bar->bg);

    ituWidgetSetType(bar, ITU_PROGRESSBAR);
    ituWidgetSetName(bar, progressBarName);
    ituWidgetSetExit(bar, ituProgressBarExit);
    ituWidgetSetUpdate(bar, ituProgressBarUpdate);
    ituWidgetSetDraw(bar, ituProgressBarDraw);
    ituWidgetSetOnAction(bar, ituProgressBarOnAction);

    bar->layout = layout;
}

void ituProgressBarLoad(ITUProgressBar* bar, uint32_t base)
{
    ITUWidget* widget = (ITUWidget*)bar;
    assert(bar);

    ituBackgroundLoad(&bar->bg, base);

    if (bar->valueText)
        bar->valueText = (ITUText*)((uint32_t)bar->valueText + base);

    ituWidgetSetExit(bar, ituProgressBarExit);
    ituWidgetSetUpdate(bar, ituProgressBarUpdate);
    ituWidgetSetDraw(bar, ituProgressBarDraw);
    ituWidgetSetOnAction(bar, ituProgressBarOnAction);

    if (!(widget->flags & ITU_EXTERNAL))
    {
        if (bar->staticBarSurf)
        {
            ITUSurface* surf = (ITUSurface*)(base + (uint32_t)bar->staticBarSurf);
            if (surf->flags & ITU_COMPRESSED)
                bar->barSurf = NULL;
            else
                bar->barSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);

            bar->staticBarSurf = surf;
        }
    }
}

void ituProgressBarSetValue(ITUProgressBar* bar, int value)
{
    assert(bar);
    ITU_ASSERT_THREAD();

    //printf("value=%d\n", value);

    if (value < bar->min || value > bar->max)
    {
        LOG_WARN "incorrect value: %d\n", value LOG_END
        return;
    }
    bar->value = value;

    ituWidgetUpdate(bar, ITU_EVENT_LAYOUT, 0, 0, 0);
    ituWidgetSetDirty(bar, true);
}

void ituProgressBarLoadStaticData(ITUProgressBar* bar)
{
    ITUWidget* widget = (ITUWidget*)bar;
    ITUSurface* surf;

    if (widget->flags & ITU_EXTERNAL)
        return;

    if (bar->staticBarSurf && !bar->barSurf)
    {
        surf = bar->staticBarSurf;

        if (surf->flags & ITU_COMPRESSED)
            bar->barSurf = ituSurfaceDecompress(surf);
        else
            bar->barSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }
}

void ituProgressBarReleaseSurface(ITUProgressBar* bar)
{
    ITU_ASSERT_THREAD();

    if (bar->barSurf)
    {
        ituSurfaceRelease(bar->barSurf);
        bar->barSurf = NULL;
    }
}

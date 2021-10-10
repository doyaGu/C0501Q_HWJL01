#include <assert.h>
#include <malloc.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char checkboxName[] = "ITUCheckBox";

void ituCheckBoxExit(ITUWidget* widget)
{
    ITUCheckBox* checkbox = (ITUCheckBox*) widget;
    assert(widget);
    ITU_ASSERT_THREAD();

    if (checkbox->focusCheckedSurf)
    {
        ituSurfaceRelease(checkbox->focusCheckedSurf);
        checkbox->focusCheckedSurf = NULL;
    }

    if (checkbox->checkedSurf)
    {
        ituSurfaceRelease(checkbox->checkedSurf);
        checkbox->checkedSurf = NULL;
    }
    ituButtonExit(widget);
}

bool ituCheckBoxClone(ITUWidget* widget, ITUWidget** cloned)
{
    ITUCheckBox* checkbox = (ITUCheckBox*)widget;
    ITUCheckBox* newCheckBox;
    ITUSurface* surf;
    assert(widget);
    assert(cloned);
    ITU_ASSERT_THREAD();

    if (*cloned == NULL)
    {
        ITUWidget* newWidget = malloc(sizeof(ITUCheckBox));
        if (newWidget == NULL)
            return false;

        memcpy(newWidget, widget, sizeof(ITUCheckBox));
        newWidget->tree.child = newWidget->tree.parent = newWidget->tree.sibling = NULL;
        *cloned = newWidget;
    }

    newCheckBox = (ITUCheckBox*)*cloned;
    surf = checkbox->staticCheckedSurf;

    if (surf)
    {
        if (surf->flags & ITU_COMPRESSED)
            newCheckBox->checkedSurf = ituSurfaceDecompress(surf);
        else
            newCheckBox->checkedSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }

    surf = checkbox->staticFocusCheckedSurf;

    if (surf)
    {
        if (surf->flags & ITU_COMPRESSED)
            newCheckBox->focusCheckedSurf = ituSurfaceDecompress(surf);
        else
            newCheckBox->focusCheckedSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }

    return ituButtonClone(widget, cloned);
}

static void CheckBoxLoadExternalData(ITUCheckBox* checkbox, ITULayer* layer)
{
    ITUWidget* widget = (ITUWidget*)checkbox;
    ITUSurface* surf;

    assert(widget);

    if (!(widget->flags & ITU_EXTERNAL))
        return;

    if (!layer)
        layer = ituGetLayer(widget);

    if (checkbox->staticCheckedSurf && !checkbox->checkedSurf)
    {
        surf = ituLayerLoadExternalSurface(layer, (uint32_t)checkbox->staticCheckedSurf);

        if (surf->flags & ITU_COMPRESSED)
            checkbox->checkedSurf = ituSurfaceDecompress(surf);
        else
            checkbox->checkedSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }

    if (checkbox->staticFocusCheckedSurf && !checkbox->focusCheckedSurf)
    {
        surf = ituLayerLoadExternalSurface(layer, (uint32_t)checkbox->staticFocusCheckedSurf);

        if (surf->flags & ITU_COMPRESSED)
            checkbox->focusCheckedSurf = ituSurfaceDecompress(surf);
        else
            checkbox->focusCheckedSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }
}

bool ituCheckBoxUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUButton* btn = (ITUButton*) widget;
    ITUCheckBox* checkbox = (ITUCheckBox*) widget;
    ITUEvent response_key = widget->flags & ITU_RESPONSE_TO_UP_KEY ? ITU_EVENT_MOUSEUP : ITU_EVENT_MOUSEDOWN;
    assert(checkbox);

    if (ev == response_key)
    {
        result |= ituIconUpdate(widget, ev, arg1, arg2, arg3);

        if (ituWidgetIsEnabled(widget) && !result)
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ev == ITU_EVENT_MOUSEDOWN)
            {
                if (ituWidgetIsInside(widget, x, y))
                {
                    ituButtonSetPressed(btn, true);
                    ituFocusWidget(checkbox);
                    ituCheckBoxSetChecked(checkbox, !checkbox->checked);
                    result |= ituWidgetOnPress(widget, ev, arg1, x, y);
                    result |= ituExecActions((ITUWidget*)btn, btn->actions, ev, 0);
                    result |= widget->dirty;
                }
            }
            else if (ev == ITU_EVENT_MOUSEUP)
            {
                if (btn->pressed)
                {
                    ituButtonSetPressed(btn, false);

                    if (ituWidgetIsInside(widget, x, y))
                    {
                        ituFocusWidget(btn);
                        ituCheckBoxSetChecked(checkbox, !checkbox->checked);
                        ituWidgetOnPress(widget, ev, arg1, x, y);
                        result |= ituExecActions((ITUWidget*)btn, btn->actions, ev, 0);
                        result |= widget->dirty;
                    }
                }
            }
        }
    }
    else
    {
        result |= ituButtonUpdate(widget, ev, arg1, arg2, arg3);
    }

    if (ev == ITU_EVENT_LAYOUT)
    {
        ituCheckBoxSetChecked(checkbox, checkbox->checked);
        result = widget->dirty = true;
    }
    else if (ev == ITU_EVENT_LOAD)
    {
        ituCheckBoxLoadStaticData(checkbox);
        result = true;
    }
    else if (ev == ITU_EVENT_LOAD_EXTERNAL)
    {
        CheckBoxLoadExternalData(checkbox, (ITULayer*)arg1);
        result = true;
    }
    else if (ev == ITU_EVENT_RELEASE)
    {
        ituCheckBoxReleaseSurface(checkbox);
        result = true;
    }
    else if (ituWidgetIsActive(widget) && ituWidgetIsEnabled(widget) && !result)
    {
        response_key = widget->flags & ITU_RESPONSE_TO_UP_KEY ? ITU_EVENT_KEYUP : ITU_EVENT_KEYDOWN;

        if (ev == response_key)
        {
            if (arg1 == ituScene->enterKey)
            {
                ituFocusWidget(checkbox);
                ituCheckBoxSetChecked(checkbox, !checkbox->checked);
                result |= widget->dirty;
            }
        }
    }
    return widget->visible ? result : false;
}

void ituCheckBoxDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
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
                    if (widget->angle == 0)
                    {
                        ituBitBlt(dest, destx, desty, srcSurf->width, srcSurf->height, srcSurf, 0, 0);
                    }
                    else
                    {
#if (CFG_CHIP_FAMILY == 9070)
                        ituRotate(dest, destx + rect->width / 2, desty + rect->height / 2, srcSurf, srcSurf->width / 2, srcSurf->height / 2, (float)widget->angle, 1.0f, 1.0f);
#else
                        ituRotate(dest, destx, desty, srcSurf, srcSurf->width / 2, srcSurf->height / 2, (float)widget->angle, 1.0f, 1.0f);
#endif
                    }
                }
                else
                {
                    ituAlphaBlend(dest, destx, desty, srcSurf->width, srcSurf->height, srcSurf, 0, 0, desta);
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

void ituCheckBoxOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    ITUCheckBox* checkbox = (ITUCheckBox*) widget;
    assert(checkbox);

    switch (action)
    {
    case ITU_ACTION_CHECK:
        ituCheckBoxSetChecked(checkbox, true);
        break;

    case ITU_ACTION_UNCHECK:
        ituCheckBoxSetChecked(checkbox, false);
        break;

    default:
        ituButtonOnAction(widget, action, param);
        break;
    }
}

void ituCheckBoxInit(ITUCheckBox* checkbox)
{
    assert(checkbox);
    ITU_ASSERT_THREAD();

    memset(checkbox, 0, sizeof (ITUCheckBox));

    ituButtonInit(&checkbox->btn);

    ituWidgetSetType(checkbox, ITU_CHECKBOX);
    ituWidgetSetName(checkbox, checkboxName);
    ituWidgetSetExit(checkbox, ituCheckBoxExit);
    ituWidgetSetClone(checkbox, ituCheckBoxClone);
    ituWidgetSetUpdate(checkbox, ituCheckBoxUpdate);
    ituWidgetSetDraw(checkbox, ituCheckBoxDraw);
    ituWidgetSetOnAction(checkbox, ituCheckBoxOnAction);
}

void ituCheckBoxLoad(ITUCheckBox* checkbox, uint32_t base)
{
    ITUWidget* widget = (ITUWidget*)checkbox;
    assert(checkbox);

    ituButtonLoad(&checkbox->btn, base);
    ituWidgetSetExit(checkbox, ituCheckBoxExit);
    ituWidgetSetClone(checkbox, ituCheckBoxClone);
    ituWidgetSetUpdate(checkbox, ituCheckBoxUpdate);
    ituWidgetSetDraw(checkbox, ituCheckBoxDraw);
    ituWidgetSetOnAction(checkbox, ituCheckBoxOnAction);

    if (!(widget->flags & ITU_EXTERNAL))
    {
        if (checkbox->staticCheckedSurf)
        {
            ITUSurface* surf = (ITUSurface*)(base + (uint32_t)checkbox->staticCheckedSurf);
            if (surf->flags & ITU_COMPRESSED)
                checkbox->checkedSurf = NULL;
            else
                checkbox->checkedSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);

            checkbox->staticCheckedSurf = surf;
        }

        if (checkbox->staticFocusCheckedSurf)
        {
            ITUSurface* surf = (ITUSurface*)(base + (uint32_t)checkbox->staticFocusCheckedSurf);
            if (surf->flags & ITU_COMPRESSED)
                checkbox->focusCheckedSurf = NULL;
            else
                checkbox->focusCheckedSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);

            checkbox->staticFocusCheckedSurf = surf;
        }
    }
}

void ituCheckBoxSetChecked(ITUCheckBox* checkbox, bool checked)
{
    ITUWidget* widget = (ITUWidget*) checkbox;
    ITUText* text = &checkbox->btn.text;
    assert(checkbox);
    ITU_ASSERT_THREAD();

    if (checked && checkbox->checkedFontColor.alpha > 0)
        ituWidgetSetColor(text, checkbox->checkedFontColor.alpha, checkbox->checkedFontColor.red, checkbox->checkedFontColor.green, checkbox->checkedFontColor.blue);
    else
        ituWidgetSetColor(text, checkbox->orgFontColor.alpha, checkbox->orgFontColor.red, checkbox->orgFontColor.green, checkbox->orgFontColor.blue);

    if (checked && checkbox->checkedColor.alpha > 0)
        ituWidgetSetColor(widget, checkbox->checkedColor.alpha, checkbox->checkedColor.red, checkbox->checkedColor.green, checkbox->checkedColor.blue);
    else
        ituWidgetSetColor(widget, checkbox->btn.bgColor.alpha, checkbox->btn.bgColor.red, checkbox->btn.bgColor.green, checkbox->btn.bgColor.blue);

    if (widget->active && checkbox->btn.focusColor.alpha > 0)
        ituWidgetSetColor(widget, checkbox->btn.focusColor.alpha, checkbox->btn.focusColor.red, checkbox->btn.focusColor.green, checkbox->btn.focusColor.blue);

    checkbox->checked = checked;
    widget->dirty = true;
}

bool ituCheckBoxIsChecked(ITUCheckBox* checkbox)
{
    assert(checkbox);
    ITU_ASSERT_THREAD();
    return checkbox->checked;
}

void ituCheckBoxLoadStaticData(ITUCheckBox* checkbox)
{
    ITUWidget* widget = (ITUWidget*)checkbox;
    ITUSurface* surf;

    if (widget->flags & ITU_EXTERNAL)
        return;

    if (checkbox->staticCheckedSurf && !checkbox->checkedSurf)
    {
        surf = checkbox->staticCheckedSurf;

        if (surf->flags & ITU_COMPRESSED)
            checkbox->checkedSurf = ituSurfaceDecompress(surf);
        else
            checkbox->checkedSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }

    if (checkbox->staticFocusCheckedSurf && !checkbox->focusCheckedSurf)
    {
        surf = checkbox->staticFocusCheckedSurf;

        if (surf->flags & ITU_COMPRESSED)
            checkbox->focusCheckedSurf = ituSurfaceDecompress(surf);
        else
            checkbox->focusCheckedSurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }
}

void ituCheckBoxReleaseSurface(ITUCheckBox* checkbox)
{
    ITU_ASSERT_THREAD();

    if (checkbox->focusCheckedSurf)
    {
        ituSurfaceRelease(checkbox->focusCheckedSurf);
        checkbox->focusCheckedSurf = NULL;
    }

    if (checkbox->checkedSurf)
    {
        ituSurfaceRelease(checkbox->checkedSurf);
        checkbox->checkedSurf = NULL;
    }
}

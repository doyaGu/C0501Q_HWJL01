#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char ilistboxName[] = "ITUIconListBoxListBox";

void ituIconListBoxExit(ITUWidget* widget)
{
    ITUIconListBox* ilistbox = (ITUIconListBox*) widget;
    int i;
    assert(widget);
    ITU_ASSERT_THREAD();

    for (i = 0; i < ITU_ICON_LISTBOX_TYPE_COUNT; i++)
    {
        if (ilistbox->surfArray[i])
        {
            ituSurfaceRelease(ilistbox->surfArray[i]);
            ilistbox->surfArray[i] = NULL;
        }
        else
            break;
    }
    for (i = 0; i < ITU_ICON_LISTBOX_TYPE_COUNT; i++)
    {
        if (ilistbox->focusSurfArray[i])
        {
            ituSurfaceRelease(ilistbox->focusSurfArray[i]);
            ilistbox->focusSurfArray[i] = NULL;
        }
        else
            break;
    }
    ituWidgetExitImpl(widget);
}

static void IconListBoxLoadExternalData(ITUIconListBox* ilistbox, ITULayer* layer)
{
    ITUWidget* widget = (ITUWidget*)ilistbox;
    int i;

    assert(widget);

    if (!(widget->flags & ITU_EXTERNAL))
        return;

    if (!layer)
        layer = ituGetLayer(widget);

    for (i = 0; i < ITU_ICON_LISTBOX_TYPE_COUNT; i++)
    {
        ITUSurface* surf;

        if (!ilistbox->staticSurfArray[i] || ilistbox->surfArray[i])
            break;

        surf = ituLayerLoadExternalSurface(layer, (uint32_t)ilistbox->staticSurfArray[i]);

        if (surf->flags & ITU_COMPRESSED)
            ilistbox->surfArray[i] = ituSurfaceDecompress(surf);
        else
            ilistbox->surfArray[i] = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }

    for (i = 0; i < ITU_ICON_LISTBOX_TYPE_COUNT; i++)
    {
        ITUSurface* surf;

        if (!ilistbox->focusStaticSurfArray[i] || ilistbox->focusSurfArray[i])
            break;

        surf = ituLayerLoadExternalSurface(layer, (uint32_t)ilistbox->focusStaticSurfArray[i]);

        if (surf->flags & ITU_COMPRESSED)
            ilistbox->focusSurfArray[i] = ituSurfaceDecompress(surf);
        else
            ilistbox->focusSurfArray[i] = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }
}

bool ituIconListBoxUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUIconListBox* ilistbox = (ITUIconListBox*) widget;
    assert(ilistbox);

    if (ev == ITU_EVENT_LOAD)
    {
        ituIconListBoxLoadStaticData(ilistbox);
        result = true;
    }
    else if (ev == ITU_EVENT_LOAD_EXTERNAL)
    {
        IconListBoxLoadExternalData(ilistbox, (ITULayer*)arg1);
        result = true;
    }
    else if (ev == ITU_EVENT_RELEASE)
    {
        ituIconListBoxReleaseSurface(ilistbox);
        result = true;
    }
    result |= ituListBoxUpdate(widget, ev, arg1, arg2, arg3);
    return result;
}

void ituIconListBoxDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    int destx, desty, count, i;
    uint8_t desta;
    ITURectangle prevClip;
    ITCTree* node;
    ITUIconListBox* ilistbox = (ITUIconListBox*) widget;
    ITUFlowWindow* fwin = (ITUFlowWindow*) widget;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    assert(widget);
    assert(dest);

    destx = rect->x + x;
    desty = rect->y + y;
    desta = alpha * widget->color.alpha / 255;
    desta = desta * widget->alpha / 255;
   
    ituWidgetSetClipping(widget, dest, x, y, &prevClip);

    if (desta == 255)
    {
        ituColorFill(dest, destx, desty, rect->width, rect->height, &widget->color);
    }
    else if (desta > 0)
    {
        ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
        if (surf)
        {
            ituColorFill(surf, 0, 0, rect->width, rect->height, &widget->color);
            ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);                
            ituDestroySurface(surf);
        }
    }

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

    if (widget->type == ITU_SCROLLICONLISTBOX)
        count = itcTreeGetChildCount(ilistbox) / 3;
    else
        count = 0;

    i = 0;
    alpha = alpha * widget->alpha / 255;
    for (node = widget->tree.child; node; node = node->sibling)
    {
        ITUWidget* child = (ITUWidget*)node;
        ITUText* text = (ITUText*) child;
        ITURectangle* childRect = (ITURectangle*) &child->rect;
        int childx, childy;
        ITURectangle childPrevClip;
        uint8_t childa, childbga;

        childx = childRect->x + destx;
        childy = childRect->y + desty;
        childa = alpha * child->color.alpha / 255;
        childa = childa * child->alpha / 255;
        childbga = alpha * text->bgColor.alpha / 255;
        childbga = childbga * child->alpha / 255;

        ituWidgetSetClipping(child, dest, destx, desty, &childPrevClip);

        if (childbga == 255)
        {
            ituColorFill(dest, childx, childy, childRect->width, childRect->height, &text->bgColor);
        }
        else if (childbga > 0)
        {
            ITUSurface* surf = ituCreateSurface(childRect->width, childRect->height, 0, dest->format, NULL, 0);
            if (surf)
            {
                ituColorFill(surf, 0, 0, childRect->width, childRect->height, &text->bgColor);
                ituAlphaBlend(dest, childx, childy, childRect->width, childRect->height, surf, 0, 0, childbga);                
                ituDestroySurface(surf);
            }
        }

        if (childa > 0)
        {
            ITUSurface* surf = NULL;
            char* str = ituTextGetString(text);
            if (str && str[0] != '\0')
            {
                int index = atoi(str);
                
                if (index < ITU_ICON_LISTBOX_TYPE_COUNT)
                {
                    if (i == count + ilistbox->listbox.focusIndex && ilistbox->focusSurfArray[index])
                        surf = ilistbox->focusSurfArray[index];
                    else
                        surf = ilistbox->surfArray[index];
                }

                if (surf)
                {
                    int xx = childx + childRect->width / 2 - surf->width / 2;
                    int yy = childy + childRect->height / 2 - surf->height / 2;
                    if (childa == 255)
                    {
                        ituBitBlt(dest, xx, yy, childRect->width, childRect->height, surf, 0, 0);
                    }
                    else
                    {
                        ituAlphaBlend(dest, xx, yy, childRect->width, childRect->height, surf, 0, 0, childa);                
                    }
                }
            }
        }
        i++;
        child->dirty = false;
        ituSurfaceSetClipping(dest, childPrevClip.x, childPrevClip.y, childPrevClip.width, childPrevClip.height);
    }
    widget->dirty = false;
    ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
}

void ituIconListBoxInit(ITUIconListBox* ilistbox, int width)
{
    assert(ilistbox);
    ITU_ASSERT_THREAD();

    memset(ilistbox, 0, sizeof (ITUIconListBox));

    ituListBoxInit(&ilistbox->listbox, width);

    ituWidgetSetType(ilistbox, ITU_ICONLISTBOX);
    ituWidgetSetName(ilistbox, ilistboxName);
    ituWidgetSetExit(ilistbox, ituIconListBoxExit);
    ituWidgetSetUpdate(ilistbox, ituIconListBoxUpdate);
    ituWidgetSetDraw(ilistbox, ituIconListBoxDraw);
}

void ituIconListBoxLoad(ITUIconListBox* ilistbox, uint32_t base)
{
    ITUWidget* widget = (ITUWidget*)ilistbox;
    assert(ilistbox);

    ituListBoxLoad(&ilistbox->listbox, base);

    ituWidgetSetExit(ilistbox, ituIconListBoxExit);
    ituWidgetSetUpdate(ilistbox, ituIconListBoxUpdate);
    ituWidgetSetDraw(ilistbox, ituIconListBoxDraw);

    if (!(widget->flags & ITU_EXTERNAL))
    {
        int i;

        for (i = 0; i < ITU_ICON_LISTBOX_TYPE_COUNT; i++)
        {
            if (ilistbox->staticSurfArray[i])
			{
                ITUSurface* surf = (ITUSurface*)(base + (uint32_t)ilistbox->staticSurfArray[i]);
                if (surf->flags & ITU_COMPRESSED)
                    ilistbox->surfArray[i] = NULL;
                else
                    ilistbox->surfArray[i] = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);

                ilistbox->staticSurfArray[i] = surf;
            }
        }

        for (i = 0; i < ITU_ICON_LISTBOX_TYPE_COUNT; i++)
        {
            if (ilistbox->focusStaticSurfArray[i])
            {
                ITUSurface* surf = (ITUSurface*)(base + (uint32_t)ilistbox->focusStaticSurfArray[i]);
                if (surf->flags & ITU_COMPRESSED)
                    ilistbox->focusSurfArray[i] = NULL;
                else
                    ilistbox->focusSurfArray[i] = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);

                ilistbox->focusStaticSurfArray[i] = surf;
            }
        }
    }
}

void ituIconListBoxLoadStaticData(ITUIconListBox* ilistbox)
{
    ITUWidget* widget = (ITUWidget*)ilistbox;
    int i;

    if (widget->flags & ITU_EXTERNAL)
        return;

    for (i = 0; i < ITU_ICON_LISTBOX_TYPE_COUNT; i++)
    {
        ITUSurface* surf;

        if (!ilistbox->staticSurfArray[i] || ilistbox->surfArray[i])
            break;

        surf = ilistbox->staticSurfArray[i];

        if (surf->flags & ITU_COMPRESSED)
            ilistbox->surfArray[i] = ituSurfaceDecompress(surf);
        else
            ilistbox->surfArray[i] = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }

    for (i = 0; i < ITU_ICON_LISTBOX_TYPE_COUNT; i++)
    {
        ITUSurface* surf;

        if (!ilistbox->focusStaticSurfArray[i] || ilistbox->focusSurfArray[i])
            break;

        surf = ilistbox->focusStaticSurfArray[i];

        if (surf->flags & ITU_COMPRESSED)
            ilistbox->focusSurfArray[i] = ituSurfaceDecompress(surf);
        else
            ilistbox->focusSurfArray[i] = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
    }
}

void ituIconListBoxReleaseSurface(ITUIconListBox* ilistbox)
{
    int i;
    ITU_ASSERT_THREAD();

    for (i = 0; i < ITU_ICON_LISTBOX_TYPE_COUNT; i++)
    {
        if (ilistbox->surfArray[i])
        {
            ituSurfaceRelease(ilistbox->surfArray[i]);
            ilistbox->surfArray[i] = NULL;
        }
        else 
            break;
    }

    for (i = 0; i < ITU_ICON_LISTBOX_TYPE_COUNT; i++)
    {
        if (ilistbox->focusSurfArray[i])
        {
            ituSurfaceRelease(ilistbox->focusSurfArray[i]);
            ilistbox->focusSurfArray[i] = NULL;
        }
        else 
            break;
    }
}

#include <assert.h>
#include <malloc.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

typedef enum
{
    READY,
    STOP,
    START_STOP,
    SCROLL_LEFT,
    END_STOP,
    SCROLL_RIGHT
} ScrollTextState;

static const char scrollTextName[] = "ITUScrollText";

bool ituScrollTextClone(ITUWidget* widget, ITUWidget** cloned)
{
    ITUScrollText* stext = (ITUScrollText*)widget;
    assert(widget);
    assert(cloned);
    ITU_ASSERT_THREAD();

    if (*cloned == NULL)
    {
        ITUWidget* newWidget = malloc(sizeof(ITUScrollText));
        if (newWidget == NULL)
            return false;

        memcpy(newWidget, widget, sizeof(ITUScrollText));
        newWidget->tree.child = newWidget->tree.parent = newWidget->tree.sibling = NULL;
        *cloned = newWidget;
    }

    return ituTextClone(widget, cloned);
}

bool ituScrollTextUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    ITUScrollText* text = (ITUScrollText*) widget;
    bool result;
    assert(text);

    result = ituTextUpdate(&text->text.widget, ev, arg1, arg2, arg3);

    if (ev == ITU_EVENT_TIMER)
    {
        if (text->state > STOP)
        {
            if (--text->delayCount <= 0)
            {
                switch (text->state)
                {
                case START_STOP:
                    text->state = SCROLL_LEFT;
                    text->delayCount = text->scrollDelay;
                    break;

                case SCROLL_LEFT:
                    text->offsetX--;
                    if ((int)text->offsetWidth + text->offsetX <= 0)
                    {
                        text->state = END_STOP;
                        text->delayCount = text->stopDelay;
                    }
                    else
                    {
                        text->delayCount = text->scrollDelay;
                    }
                    break;

                case END_STOP:
                    text->state = SCROLL_RIGHT;
                    text->delayCount = text->scrollDelay;
                    break;

                case SCROLL_RIGHT:
                    text->offsetX++;
                    if (text->offsetX >= 0)
                    {
                        text->state = START_STOP;
                        text->delayCount = text->stopDelay;
                    }
                    else
                    {
                        text->delayCount = text->scrollDelay;
                    }
                    break;
                }
                result = widget->dirty = true;
            }
        }
    }
    else if (ev == ITU_EVENT_LAYOUT && text->state != STOP)
    {
        char* s = ituTextGetString(text);
        if (s)
        {
            ituFtSetCurrentFont(text->text.fontIndex);

            if (text->text.fontHeight > 0)
                ituFtSetFontSize(text->text.fontWidth, text->text.fontHeight);

            if (text->text.textFlags & ITU_TEXT_BOLD)
            {
                ituFtSetFontStyle(ITU_FT_STYLE_BOLD);
                ituFtSetFontStyleValue(ITU_FT_STYLE_BOLD, text->text.boldSize);
            }
            else
                ituFtSetFontStyle(ITU_FT_STYLE_DEFAULT);

            ituFtGetTextDimension(s, &text->width, NULL);
        }
        if (ituWidgetGetWidth(text) >= text->width)
        {
            text->state         = READY;
            text->delayCount    = 0;
        }
        else
        {
            text->state         = START_STOP;
            text->delayCount    = text->stopDelay;
            text->offsetWidth   = text->width - ituWidgetGetWidth(text);
        }
        text->offsetX = 0;
        result = widget->dirty = true;
    }

    return widget->visible ? result : false;
}

void ituScrollTextDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    ITUScrollText* stext = (ITUScrollText*) widget;
    ITUText* text = (ITUText*) widget;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    int destx, desty;
    uint8_t desta, destbga;
    ITURectangle prevClip;
    char* string;
    assert(text);
    assert(dest);

    if (text->string)
        string = text->string;
    else if (text->stringSet)
        string = text->stringSet->strings[text->lang];
    else
    {
        widget->dirty = false;
        return;
    }

    destx = rect->x + x;
    desty = rect->y + y;
    desta = alpha * widget->color.alpha / 255;
    desta = desta * widget->alpha / 255;
    destbga = alpha * text->bgColor.alpha / 255;
    destbga = destbga * widget->alpha / 255;

    ituWidgetSetClipping(widget, dest, x, y, &prevClip);

    if (destbga == 255)
    {
        ituColorFill(dest, destx, desty, rect->width, rect->height, &text->bgColor);
    }
    else if (destbga > 0)
    {
        ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
        if (surf)
        {
            ituColorFill(surf, 0, 0, rect->width, rect->height, &text->bgColor);
            ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, destbga);                
            ituDestroySurface(surf);
        }
    }

    ituFtSetCurrentFont(text->fontIndex);

    if (desta == 255)
    {
        int w, h;

        ituSetColor(&dest->fgColor, desta, widget->color.red, widget->color.green, widget->color.blue);

        if (text->fontHeight > 0)
            ituFtSetFontSize(text->fontWidth, text->fontHeight);

        if (text->textFlags & ITU_TEXT_BOLD)
        {
            ituFtSetFontStyle(ITU_FT_STYLE_BOLD);
            ituFtSetFontStyleValue(ITU_FT_STYLE_BOLD, text->boldSize);
        }
        else
            ituFtSetFontStyle(ITU_FT_STYLE_DEFAULT);

        if (text->layout == ITU_LAYOUT_TOP_CENTER)
        {
            ituFtGetTextDimension(string, &w, &h);
            destx += rect->width / 2 - w / 2;
        }
        else if (text->layout == ITU_LAYOUT_TOP_RIGHT)
        {
            ituFtGetTextDimension(string, &w, &h);
            destx += rect->width - w;
        }
        else if (text->layout == ITU_LAYOUT_MIDDLE_LEFT)
        {
            ituFtGetTextDimension(string, &w, &h);
            desty += rect->height / 2 - h / 2;
        }
        else if (text->layout == ITU_LAYOUT_MIDDLE_CENTER)
        {
            ituFtGetTextDimension(string, &w, &h);
            destx += rect->width / 2 - w / 2;
            desty += rect->height / 2 - h / 2;
        }
        else if (text->layout == ITU_LAYOUT_MIDDLE_RIGHT)
        {
            ituFtGetTextDimension(string, &w, &h);
            destx += rect->width - w;
            desty += rect->height / 2 - h / 2;
        }
        else if (text->layout == ITU_LAYOUT_BOTTOM_LEFT)
        {
            ituFtGetTextDimension(string, &w, &h);
            desty += rect->height - h;
        }
        else if (text->layout == ITU_LAYOUT_BOTTOM_CENTER)
        {
            ituFtGetTextDimension(string, &w, &h);
            destx += rect->width / 2 - w / 2;
            desty += rect->height - h;
        }
        else if (text->layout == ITU_LAYOUT_BOTTOM_RIGHT)
        {
            ituFtGetTextDimension(string, &w, &h);
            destx += rect->width - w;
            desty += rect->height - h;
        }
        ituFtDrawText(dest, destx + stext->offsetX, desty, string);
    }
    else if (desta > 0)
    {
        ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
        if (surf)
        {
            int w, h, dx = 0, dy = 0;

            ituBitBlt(surf, 0, 0, rect->width, rect->height, dest, destx, desty);
            ituSetColor(&surf->fgColor, desta, widget->color.red, widget->color.green, widget->color.blue);
            if (text->fontHeight > 0)
                ituFtSetFontSize(text->fontWidth, text->fontHeight);

            if (text->textFlags & ITU_TEXT_BOLD)
            {
                ituFtSetFontStyle(ITU_FT_STYLE_BOLD);
                ituFtSetFontStyleValue(ITU_FT_STYLE_BOLD, text->boldSize);
            }
            else
                ituFtSetFontStyle(ITU_FT_STYLE_DEFAULT);

            if (text->layout == ITU_LAYOUT_TOP_CENTER)
            {
                ituFtGetTextDimension(string, &w, &h);
                dx += rect->width / 2 - w / 2;
            }
            else if (text->layout == ITU_LAYOUT_TOP_RIGHT)
            {
                ituFtGetTextDimension(string, &w, &h);
                dx += rect->width - w;
            }
            else if (text->layout == ITU_LAYOUT_MIDDLE_LEFT)
            {
                ituFtGetTextDimension(string, &w, &h);
                dy += rect->height / 2 - h / 2;
            }
            else if (text->layout == ITU_LAYOUT_MIDDLE_CENTER)
            {
                ituFtGetTextDimension(string, &w, &h);
                dx += rect->width / 2 - w / 2;
                dy += rect->height / 2 - h / 2;
            }
            else if (text->layout == ITU_LAYOUT_MIDDLE_RIGHT)
            {
                ituFtGetTextDimension(string, &w, &h);
                dx += rect->width - w;
                dy += rect->height / 2 - h / 2;
            }
            else if (text->layout == ITU_LAYOUT_BOTTOM_LEFT)
            {
                ituFtGetTextDimension(string, &w, &h);
                dy += rect->height - h;
            }
            else if (text->layout == ITU_LAYOUT_BOTTOM_CENTER)
            {
                ituFtGetTextDimension(string, &w, &h);
                dx += rect->width / 2 - w / 2;
                dy += rect->height - h;
            }
            else if (text->layout == ITU_LAYOUT_BOTTOM_RIGHT)
            {
                ituFtGetTextDimension(string, &w, &h);
                dx += rect->width - w;
                dy += rect->height - h;
            }
            ituFtDrawText(surf, dx + stext->offsetX, dy, string);
            ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);                
            ituDestroySurface(surf);
        }
    }
    ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
    ituWidgetDrawImpl(widget, dest, x, y, alpha);
}

void ituScrollTextInit(ITUScrollText* text, int width)
{
    assert(text);
    ITU_ASSERT_THREAD();

    memset(text, 0, sizeof (ITUScrollText));

    ituTextInit(&text->text);

    ituWidgetSetType(text, ITU_SCROLLTEXT);
    ituWidgetSetName(text, scrollTextName);
    ituWidgetSetClone(text, ituScrollTextClone);
    ituWidgetSetUpdate(text, ituScrollTextUpdate);
    ituWidgetSetDraw(text, ituScrollTextDraw);

    text->width = width;
    ituScrollTextSetDelay(text, ITU_SCROLL_DELAY, ITU_STOP_DELAY);
}

void ituScrollTextLoad(ITUScrollText* text, uint32_t base)
{
    assert(text);

    ituTextLoad((ITUText*)text, base);
    ituWidgetSetClone(text, ituScrollTextClone);
    ituWidgetSetUpdate(text, ituScrollTextUpdate);
    ituWidgetSetDraw(text, ituScrollTextDraw);
}

void ituScrollTextSetDelay(ITUScrollText* text, int scrollDelay, int stopDelay)
{
    assert(text);
    ITU_ASSERT_THREAD();

    text->scrollDelay       = scrollDelay;
    text->stopDelay         = stopDelay;
    text->text.widget.dirty = true;
}

void ituScrollTextStart(ITUScrollText* text)
{
    assert(text);
    ITU_ASSERT_THREAD();

    text->state = READY;
    ituWidgetUpdate(text, ITU_EVENT_LAYOUT, 0, 0, 0);
}

void ituScrollTextStop(ITUScrollText* text)
{
    assert(text);
    ITU_ASSERT_THREAD();

    text->state         = STOP;
    text->delayCount    = 0;
    text->offsetX       = 0;
    ituWidgetSetDirty(text, true);
}

void ituScrollTextSetString(ITUScrollText* text, char* string)
{
    assert(text);
    ITU_ASSERT_THREAD();

    ituTextSetString(&text->text, string);
    if (string)
        ituFtGetTextDimension(string, &text->width, NULL);
}

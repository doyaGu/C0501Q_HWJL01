#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char textName[] = "ITUText";

void ituTextExit(ITUWidget* widget)
{
    ITUText* text = (ITUText*) widget;
    assert(widget);
    ITU_ASSERT_THREAD();

    if (text->string)
    {
        free(text->string);
        text->string = NULL;
    }
    ituWidgetExitImpl(widget);
}

bool ituTextClone(ITUWidget* widget, ITUWidget** cloned)
{
    ITUText* text = (ITUText*)widget;
    assert(widget);
    assert(cloned);
    ITU_ASSERT_THREAD();

    if (*cloned == NULL)
    {
        ITUWidget* newWidget = malloc(sizeof(ITUText));
        if (newWidget == NULL)
            return false;

        memcpy(newWidget, widget, sizeof(ITUText));
        newWidget->tree.child = newWidget->tree.parent = newWidget->tree.sibling = NULL;
        *cloned = newWidget;
    }

    if (text->string)
    {
        ITUText* newText = (ITUText*)*cloned;
        newText->string =strdup(text->string);
    }
    return ituWidgetCloneImpl(widget, cloned);
}

bool ituTextUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result;
    ITUText* text = (ITUText*) widget;
    assert(text);

    result = ituWidgetUpdateImpl(widget, ev, arg1, arg2, arg3);

    if (ev == ITU_EVENT_LANGUAGE)
    {
        ituTextSetLanguage(text, arg1);
        result = widget->dirty = true;
    }

    return widget->visible ? result : false;
}

void ituTextDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    ITUText* text = (ITUText*) widget;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    int destx, desty;
    uint8_t desta, destbga;
    ITURectangle prevClip;
    char* string;
    assert(text);
    assert(dest);

    if (widget->rect.width == 0 || widget->rect.height == 0)
    {
        widget->dirty = false;
        return;
    }

	/*
	if (text->stringSet->count > 0)
	{
		string = text->stringSet->strings[text->lang];
		if (string[0] == '2')
		{
			printf("[TEXT LANG] lang text: %s,string count: %d, lang %d\n", string, text->stringSet->count, text->lang);
			printf("1:%s 2:%s 3:%d\n", text->stringSet->strings[0], text->stringSet->strings[1], text->stringSet->totalSize);
		}
		
	}
	*/
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

    if ((widget->flags & ITU_CLIP_DISABLED) == 0)
        ituWidgetSetClipping(widget, dest, x, y, &prevClip);
    
    if (destbga == 255)
    {
        ituColorFill(dest, destx, desty, rect->width, rect->height, &text->bgColor);
    }
    else if (destbga > 0)
    {
#if (CFG_CHIP_FAMILY == 9070)
        ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
        if (surf)
        {
            ituColorFill(surf, 0, 0, rect->width, rect->height, &text->bgColor);
            ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, destbga);                
            ituDestroySurface(surf);
        }
#else
        ituColorFillBlend(dest, destx, desty, rect->width, rect->height, &text->bgColor, true, true, destbga);
#endif
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
        ituFtDrawText(dest, destx, desty, string);
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
            ituFtDrawText(surf, dx, dy, string);
            ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);                
            ituDestroySurface(surf);
        }
    }
    if ((widget->flags & ITU_CLIP_DISABLED) == 0)
        ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);

    ituWidgetDrawImpl(widget, dest, x, y, alpha);
}

void ituTextOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    ITUText* text = (ITUText*) widget;
    assert(widget);

    switch (action)
    {
    case ITU_ACTION_INPUT:
        ituTextSetString(text, param);
        break;

    case ITU_ACTION_LANGUAGE:
        ituTextUpdate(widget, ITU_EVENT_LANGUAGE, atoi(param), 0, 0);
        break;

    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

void ituTextInit(ITUText* text)
{
    assert(text);
    ITU_ASSERT_THREAD();

    memset(text, 0, sizeof (ITUText));

    ituWidgetInit(&text->widget);

    ituWidgetSetType(text, ITU_TEXT);
    ituWidgetSetName(text, textName);
    ituWidgetSetExit(text, ituTextExit);
    ituWidgetSetClone(text, ituTextClone);
    ituWidgetSetUpdate(text, ituTextUpdate);
    ituWidgetSetDraw(text, ituTextDraw);
    ituWidgetSetOnAction(text, ituTextOnAction);

    ituSetColor(&text->widget.color, 255, 255, 255, 255);
}

void ituTextLoad(ITUText* text, uint32_t base)
{
    assert(text);

    ituWidgetLoad((ITUWidget*)text, base);
    
    ituWidgetSetExit(text, ituTextExit);
    ituWidgetSetClone(text, ituTextClone);
    ituWidgetSetUpdate(text, ituTextUpdate);
    ituWidgetSetDraw(text, ituTextDraw);
    ituWidgetSetOnAction(text, ituTextOnAction);

    if (text->stringSet)
        text->stringSet = (ITUStringSet*)(base + (uint32_t)text->stringSet);
}

void ituTextSetFontWidth(ITUText* text, int height)
{
    assert(text);
    ITU_ASSERT_THREAD();
    text->fontWidth = height;
    text->widget.dirty = true;
}

void ituTextSetFontHeight(ITUText* text, int height)
{
    assert(text);
    ITU_ASSERT_THREAD();
    text->fontHeight = height;
    text->widget.dirty = true;
}

void ituTextSetFontSize(ITUText* text, int size)
{
    assert(text);
    ITU_ASSERT_THREAD();
    text->fontWidth = size;
    text->fontHeight = size;
    text->widget.dirty = true;
}

void ituTextSetLanguage(ITUText* text, int lang)
{
    assert(text);
    ITU_ASSERT_THREAD();

    if (!text->stringSet)
        return;

    if (lang >= text->stringSet->count)
        lang = 0;

    text->lang = lang;

    if (!text->string)
        text->widget.dirty = true;
}

void ituTextSetStringImpl(ITUText* text, char* string)
{
    assert(text);
    ITU_ASSERT_THREAD();

    if (text->string)
        free(text->string);

	if (string)
    {
        text->string = strdup(string);
    }
    else
    {
        text->string = NULL;

        if (text->stringSet)
            string = text->stringSet->strings[text->lang];
    }
    if (string)
    {
        if (text->fontHeight > 0)
        {
            ituFtSetCurrentFont(text->fontIndex);
            ituFtSetFontSize(text->fontWidth, text->fontHeight);
        }

        if (text->textFlags & ITU_TEXT_BOLD)
        {
            ituFtSetFontStyle(ITU_FT_STYLE_BOLD);
            ituFtSetFontStyleValue(ITU_FT_STYLE_BOLD, text->boldSize);
        }
        else
            ituFtSetFontStyle(ITU_FT_STYLE_DEFAULT);
    }
    text->widget.dirty = true;
}

char* ituTextGetStringImpl(ITUText* text)
{
    assert(text);
    ITU_ASSERT_THREAD();

    if (text->string)
        return text->string;
    else if (text->stringSet)
        return text->stringSet->strings[text->lang];
    else
        return NULL;
}

void ituTextSetBackColor(ITUText* text, uint8_t alpha, uint8_t red, uint8_t green, uint8_t blue)
{
    assert(text);
    ITU_ASSERT_THREAD();

    text->bgColor.alpha = alpha;
    text->bgColor.red   = red;
    text->bgColor.green = green;
    text->bgColor.blue  = blue;
    text->widget.dirty  = true;
}

void ituTextResize(ITUText* text)
{
    char* string;
    ITURectangle* rect = (ITURectangle*) &text->widget.rect;
    assert(text);
    ITU_ASSERT_THREAD();

    if (text->string)
        string = text->string;
    else if (text->stringSet)
        string = text->stringSet->strings[text->lang];
    else
        return;

    if (text->fontHeight > 0)
    {
        ituFtSetCurrentFont(text->fontIndex);
        ituFtSetFontSize(text->fontWidth, text->fontHeight);
    }

    if (text->textFlags & ITU_TEXT_BOLD)
    {
        ituFtSetFontStyle(ITU_FT_STYLE_BOLD);
        ituFtSetFontStyleValue(ITU_FT_STYLE_BOLD, text->boldSize);
    }
    else
        ituFtSetFontStyle(ITU_FT_STYLE_DEFAULT);

    if (rect->height == 0)
    {
        ituFtGetTextDimension(string, &rect->width, &rect->height);
        if (rect->height)
            rect->height += rect->height / 4;
    }
    else
    {
        ituFtGetTextDimension(string, &rect->width, NULL);
    }
    text->widget.dirty  = true;
}

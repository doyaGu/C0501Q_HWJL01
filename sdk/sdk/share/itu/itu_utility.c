#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

unsigned int ituFormat2Bpp(ITUPixelFormat format)
{
    switch (format)
    {
    case ITU_RGB565:
    case ITU_ARGB1555:
    case ITU_ARGB4444:
    case ITU_RGB565A8:
        return 16;
        break;

    case ITU_ARGB8888:
        return 32;
        break;

    case ITU_A8:
        return 8;
        break;

    case ITU_MONO:
        return 1;
        break;

    default:
        return 0;
    }
}

void ituFocusWidgetImpl(ITUWidget* widget)
{
    assert(ituScene);
    ITU_ASSERT_THREAD();

    if (widget)
    {
        if (ituScene->focused && ituScene->focused != widget)
        {
            ituWidgetSetActive(ituScene->focused, false);
            ituWidgetUpdate(ituScene->focused, ITU_EVENT_LAYOUT, ITU_ACTION_FOCUS, 0, 0);
            ituScene->focused = NULL;
        }
        ituWidgetSetActive(widget, true);
        ituWidgetUpdate(widget, ITU_EVENT_LAYOUT, ITU_ACTION_FOCUS, 0, 0);
        ituScene->focused = widget;
    }
    else
    {
        if (ituScene->focused)
        {
            ituWidgetSetActive(ituScene->focused, false);
            ituWidgetUpdate(ituScene->focused, ITU_EVENT_LAYOUT, ITU_ACTION_FOCUS, 0, 0);
            ituScene->focused = NULL;
        }
    }
}

void ituDirtyWidgetImpl(ITUWidget* widget, bool dirty)
{
    ITCTree* node;
    ITU_ASSERT_THREAD();

    for (node = widget->tree.child; node; node = node->sibling)
    {
        ITUWidget* child = (ITUWidget*)node;
        ituDirtyWidgetImpl(child, dirty);
    }
    ituWidgetSetDirty(widget, dirty);
}

void ituUnPressWidgetImpl(ITUWidget* widget)
{
    ITCTree* node;
    ITU_ASSERT_THREAD();

    for (node = widget->tree.child; node; node = node->sibling)
    {
        ITUWidget* child = (ITUWidget*)node;
        ituUnPressWidgetImpl(child);
    }
    if (widget->type == ITU_BUTTON || widget->type == ITU_CHECKBOX || widget->type == ITU_RADIOBOX || widget->type == ITU_POPUPBUTTON)
    {
        ituWidgetSetActive(widget, false);

        if (ituButtonIsPressed(widget))
            ituButtonSetPressed((ITUButton*)widget, false);

        if (widget->type == ITU_CHECKBOX || widget->type == ITU_RADIOBOX)
        {
            ituCheckBoxSetChecked((ITUCheckBox*)widget, ituCheckBoxIsChecked((ITUCheckBox*)widget));
        }
    }
    else if (widget->type == ITU_SPRITEBUTTON)
    {
        ITUSpriteButton* sb = (ITUSpriteButton*) widget;
        sb->pressed = false;
    }
}

void ituScreenshot(ITUSurface* surf, char* filepath)
{
    FILE* fp;

    ITU_ASSERT_THREAD();

    fp = fopen(filepath, "wb");
    if (fp)
    {
        int size = surf->width * surf->height * 3;
        uint8_t *dest = malloc(size);
        if (dest)
        {
            int h;
            uint8_t* src = ituLockSurface(surf, 0, 0, surf->width, surf->height);
            assert(src);

            sprintf(dest, "P6\n%d\n%d\n255\n", surf->width, surf->height);
            fwrite(dest, 1, strlen(dest), fp);

            for (h = 0; h < surf->height; h++) 
            {
                int i, j;
                uint8_t* ptr = src + surf->width * 2 * h;

                // color trasform from RGB565 to RGB888
                for (i = (surf->width-1)*2, j = (surf->width-1)*3; i >= 0 && j >= 0; i -= 2, j -= 3)
                {
                    dest[surf->width * h * 3 + j+0] = ((ptr[i+1]     ) & 0xf8) + ((ptr[i+1] >> 5) & 0x07);
                    dest[surf->width * h * 3 + j+1] = ((ptr[i+0] >> 3) & 0x1c) + ((ptr[i+1] << 5) & 0xe0) + ((ptr[i+1] >> 1) & 0x3);
                    dest[surf->width * h * 3 + j+2] = ((ptr[i+0] << 3) & 0xf8) + ((ptr[i+0] >> 2) & 0x07);
                }
            }
            fwrite(dest, 1, size, fp);
            ituUnlockSurface(surf);
        }
        else
        {
            LOG_ERR "out of memory: %d.\n", size LOG_END
        }
        fclose(fp);
    }
    else
    {
        LOG_ERR "open %s fail.\n", filepath LOG_END
    }
}

ITULayer* ituGetLayer(ITUWidget* widget)
{
    ITUWidget* parent = (ITUWidget*)widget->tree.parent;

    ITU_ASSERT_THREAD();

    while (parent)
    {
        if (parent->type == ITU_LAYER)
            return (ITULayer*)parent;

        parent = (ITUWidget*)parent->tree.parent;
    }
    return NULL;
}

void ituPreloadFontCache(ITUWidget* widget, ITUSurface* surf)
{
#ifdef CFG_ITU_FT_CACHE_SIZE
    ITCTree* node;

    ITU_ASSERT_THREAD();

    for (node = widget->tree.child; node; node = node->sibling)
    {
        ITUWidget* child = (ITUWidget*)node;
        ituPreloadFontCache(child, surf);
    }

    switch (widget->type)
    {
    case ITU_TEXT:
    case ITU_SCROLLTEXT:
        {
            ITUText* text = (ITUText*)widget;
            char* string = NULL;

            if (text->string)
                string = text->string;
            else if (text->stringSet)
                string = text->stringSet->strings[text->lang];

            if (string && string[0] != '\0')
            {
                if (text->fontHeight > 0)
                    ituFtSetFontSize(text->fontWidth, text->fontHeight);

                ituFtDrawText(surf, 0, 0, string);
            }
        }
        break;

    case ITU_TEXTBOX:
        {
            ITUTextBox* textbox = (ITUTextBox*)widget;
            ITUText* text = &textbox->text;
            char* string = NULL;

            if (text->string)
                string = text->string;
            else if (text->stringSet)
                string = text->stringSet->strings[text->lang];

            if (string && string[0] != '\0')
            {
                if (text->fontHeight > 0)
                    ituFtSetFontSize(text->fontWidth, text->fontHeight);

                ituFtDrawText(surf, 0, 0, string);
            }
        }
        break;

    case ITU_BUTTON:
    case ITU_CHECKBOX:
    case ITU_RADIOBOX:
    case ITU_POPUPBUTTON:
        {
            ITUButton* button = (ITUButton*)widget;
            ITUText* text = &button->text;
            char* string = NULL;

            if (text->string)
                string = text->string;
            else if (text->stringSet)
                string = text->stringSet->strings[text->lang];

            if (string && string[0] != '\0')
            {
                if (text->fontHeight > 0)
                    ituFtSetFontSize(text->fontWidth, text->fontHeight);

                ituFtDrawText(surf, 0, 0, string);
            }
        }
        break;
    }
#endif // CFG_ITU_FT_CACHE_SIZE
}

void ituDrawGlyphEmpty(ITUSurface* surf, int x, int y, ITUGlyphFormat format, const uint8_t* bitmap, int w, int h)
{
    // DO NOTHING
}

ITUWidget* ituGetVarTarget(int index)
{
    ITUVariable* var;
    ITUWidget* target = NULL;
    assert(ituScene);
    ITU_ASSERT_THREAD();

    var = &ituScene->variables[index];

    if (var->cachedTarget)
    {
        target = var->cachedTarget;
    }
    else if (var->target[0] != '\0')
    {
        ITUWidget* widget = ituSceneFindWidget(ituScene, var->target);
        if (widget)
        {
            target = widget;
            var->cachedTarget = (void*)target;
        }
    }
    return target;
}

void ituSetVarTarget(int index, ITUWidget* target)
{
    ITUVariable* var;

    assert(index >= 0);
    assert(index < ITU_VARIABLE_SIZE);
    if (index < 0 || index >= ITU_VARIABLE_SIZE)
    {
        LOG_ERR "incorrect index: %d\n", index LOG_END
        return;
    }
    var = &ituScene->variables[index];

    if (target)
        strcpy(var->target, target->name);
    else
        var->target[0] = '\0';

    var->cachedTarget = target;
}

void ituSetVarParam(int index, char* param)
{
    ITUVariable* var;

    assert(index >= 0);
    assert(index < ITU_VARIABLE_SIZE);
    if (index < 0 || index >= ITU_VARIABLE_SIZE)
    {
        LOG_ERR "incorrect index: %d\n", index LOG_END
        return;
    }
    var = &ituScene->variables[index];

    if (param)
        strcpy(var->param, param);
    else
        var->param[0] = '\0';
}

char* ituGetVarParam(int index)
{
    ITUVariable* var;
    assert(ituScene);
    ITU_ASSERT_THREAD();

    var = &ituScene->variables[index];
    return var->param;
}

void ituAssertThread(const char *file)
{
    assert(ituScene);
    if (!pthread_equal(ituScene->threadID, pthread_self()))
    {
        LOG_ERR "itu thread assertion fail: %s\n", file LOG_END
        sleep(1);
        abort();
    }
}

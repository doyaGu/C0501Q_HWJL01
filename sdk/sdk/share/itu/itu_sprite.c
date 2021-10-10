#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"

static const char spriteName[] = "ITUSprite";

static void SpriteOnStop(ITUSprite* sprite)
{
    // DO NOTHING
}

bool ituSpriteClone(ITUWidget* widget, ITUWidget** cloned)
{
    ITUSprite* newSprite;
    assert(widget);
    assert(cloned);
    ITU_ASSERT_THREAD();

    if (*cloned == NULL)
    {
        ITUWidget* newWidget = malloc(sizeof(ITUSprite));
        if (newWidget == NULL)
            return false;

        memcpy(newWidget, widget, sizeof(ITUSprite));
        newWidget->tree.child = newWidget->tree.parent = newWidget->tree.sibling = NULL;
        *cloned = newWidget;
    }

    newSprite = (ITUSprite*)*cloned;
    newSprite->child = (ITUWidget*) itcTreeGetChildAt(newSprite, newSprite->frame);

    return ituWidgetCloneImpl(widget, cloned);
}

bool ituSpriteUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    ITUSprite* sprite = (ITUSprite*) widget;
    bool result = false;
    assert(sprite);

    if (ev == ITU_EVENT_TIMER)
    {
        if (sprite->playing)
        {
            if (--sprite->delayCount <= 0)
            {
                if (sprite->child && !(widget->flags & ITU_PROGRESS))
                {
                    ituWidgetUpdate(sprite->child, ITU_EVENT_RELEASE, 0, 0, 0);
                }
                if (sprite->frame >= sprite->childCount - 1)
                {
                    if (sprite->repeat)
                    {
                        sprite->frame = 0;
                        sprite->child = (ITUWidget*) itcTreeGetChildAt(sprite, sprite->frame);
                    }
                    else
                    {
                        sprite->playing = false;
                        ituSpriteOnStop(sprite);
                        ituExecActions((ITUWidget*)sprite, sprite->actions, ITU_EVENT_STOPPED, 0);
                    }
                }
                else
                {
                    sprite->frame++;
                    sprite->child = (ITUWidget*) itcTreeGetChildAt(sprite, sprite->frame);
                }
                if (sprite->child && !(widget->flags & ITU_PROGRESS))
                {
                    ituWidgetUpdate(sprite->child, ITU_EVENT_LOAD, 0, 0, 0);
					ituWidgetUpdate(sprite->child, ITU_EVENT_LOAD_EXTERNAL, 0, 0, 0);
                }
                sprite->delayCount = sprite->delay;
                result = widget->dirty = true;
            }
        }
    }
    else if (ev == ITU_EVENT_LAYOUT)
    {
		ITUWidget* child = NULL;

		if (sprite->childCount == 0)
			sprite->childCount = itcTreeGetChildCount(sprite);

		if (sprite->frame >= 0)
			child = (ITUWidget*)itcTreeGetChildAt(sprite, sprite->frame);

		if (child && sprite->child != child)
		{
			if (sprite->child && !(widget->flags & ITU_PROGRESS))
			{
				ituWidgetUpdate(sprite->child, ITU_EVENT_RELEASE, 0, 0, 0);
			}
			sprite->child = child;

			if (sprite->child && !(widget->flags & ITU_PROGRESS))
			{
				result |= ituWidgetUpdate(sprite->child, ITU_EVENT_LOAD, 0, 0, 0);
			}
		}
        result = widget->dirty = true;
    }
    else if (ev != ITU_EVENT_RELEASE && ev != ITU_EVENT_LANGUAGE && ev != ITU_EVENT_LOAD_IMAGE)
    {
        if (sprite->child && !(widget->flags & ITU_PROGRESS))
        {
            if (ev == ITU_EVENT_MOUSEDOWN || ev == ITU_EVENT_MOUSEUP || ev == ITU_EVENT_MOUSEDOUBLECLICK || ev == ITU_EVENT_MOUSEMOVE || ev == ITU_EVENT_MOUSELONGPRESS ||
                ev == ITU_EVENT_TOUCHSLIDELEFT || ev == ITU_EVENT_TOUCHSLIDEUP || ev == ITU_EVENT_TOUCHSLIDERIGHT || ev == ITU_EVENT_TOUCHSLIDEDOWN || ev == ITU_EVENT_TOUCHPINCH)
            {
                arg2 -= widget->rect.x;
                arg3 -= widget->rect.y;
            }
            result |= ituWidgetUpdate(sprite->child, ev, arg1, arg2, arg3);
            return widget->visible ? result : false;
        }
    }
    result |= ituWidgetUpdateImpl(widget, ev, arg1, arg2, arg3);
    return widget->visible ? result : false;
}

void ituSpriteDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    ITUSprite* sprite = (ITUSprite*) widget;

    if (sprite->child)
    {
        ITURectangle prevClip;

        ituWidgetSetClipping(widget, dest, x, y, &prevClip);

        x += widget->rect.x;
        y += widget->rect.y;
        alpha = alpha * widget->alpha / 255;

        if (widget->flags & ITU_PROGRESS)
        {
            ITCTree* child;

            for (child = widget->tree.child; child; child = child->sibling)
            {
                ituWidgetDraw(child, dest, x, y, alpha);
                if (child == &sprite->child->tree)
                    break;
            }
        }
        else
        {
            ituWidgetDraw(sprite->child, dest, x, y, alpha);
        }
        ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
    }
    ituDirtyWidget(sprite, false);
}

void ituSpriteOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    ITUSprite* sprite = (ITUSprite*) widget;
    assert(widget);

    switch (action)
    {
    case ITU_ACTION_PLAY:
        if (param[0] != '\0')
            ituSpritePlay(sprite, atoi(param));
        break;

    case ITU_ACTION_STOP:
        ituSpriteStop(sprite);
        ituSpriteOnStop(sprite);
        ituExecActions(widget, sprite->actions, ITU_EVENT_STOPPED, 0);
        break;

    case ITU_ACTION_GOTO:
        if (param[0] != '\0')
            ituSpriteGoto(sprite, atoi(param));
        break;

    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

void ituSpriteInit(ITUSprite* sprite)
{
    assert(sprite);
    ITU_ASSERT_THREAD();

    memset(sprite, 0, sizeof (ITUSprite));

    ituWidgetInit(&sprite->widget);

    ituWidgetSetType(sprite, ITU_SPRITE);
    ituWidgetSetName(sprite, spriteName);
    ituWidgetSetClone(sprite, ituSpriteClone);
    ituWidgetSetUpdate(sprite, ituSpriteUpdate);
    ituWidgetSetDraw(sprite, ituSpriteDraw);
    ituWidgetSetOnAction(sprite, ituSpriteOnAction);
    ituSpriteSetOnStop(sprite, SpriteOnStop);

    ituSpriteSetDelay(sprite, 10);
    sprite->childCount = itcTreeGetChildCount(sprite);
    sprite->child = (ITUWidget*) itcTreeGetChildAt(sprite, sprite->frame);
}

void ituSpriteLoad(ITUSprite* sprite, uint32_t base)
{
    assert(sprite);

    ituWidgetLoad((ITUWidget*)sprite, base);

    ituWidgetSetClone(sprite, ituSpriteClone);
    ituWidgetSetUpdate(sprite, ituSpriteUpdate);
    ituWidgetSetDraw(sprite, ituSpriteDraw);
    ituWidgetSetOnAction(sprite, ituSpriteOnAction);
    ituSpriteSetOnStop(sprite, SpriteOnStop);

    if (sprite->playing)
        sprite->delayCount = sprite->delay;
}

void ituSpriteSetDelay(ITUSprite* sprite, int delay)
{
    assert(sprite);
    ITU_ASSERT_THREAD();

    sprite->delay           = delay;
    sprite->widget.dirty    = true;
}

void ituSpritePlay(ITUSprite* sprite, int frame)
{
    assert(sprite);
    ITU_ASSERT_THREAD();

    if (frame >= 0)
        ituSpriteGoto(sprite, frame);

    sprite->delayCount      = sprite->delay;
    sprite->playing         = true;
    sprite->widget.dirty    = true;
}

void ituSpriteStop(ITUSprite* sprite)
{
    assert(sprite);
    ITU_ASSERT_THREAD();

    sprite->playing         = false;
    sprite->widget.dirty    = true;
}

void ituSpriteGoto(ITUSprite* sprite, int frame)
{
    int count;
    assert(sprite);
    ITU_ASSERT_THREAD();

    if (sprite->frame == frame)
        return;
    
    if (frame == -1)
    {
        if (sprite->child)
        {
            ituWidgetUpdate(sprite->child, ITU_EVENT_RELEASE, 0, 0, 0);
        }
        sprite->child = NULL;
    }
    count = itcTreeGetChildCount(sprite);
    if (frame >= 0 && frame < count)
    {
        sprite->frame = frame;
        ituWidgetUpdate(sprite, ITU_EVENT_LAYOUT, 0, 0, 0);
    }
}

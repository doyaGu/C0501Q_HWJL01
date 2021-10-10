#include <assert.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"

static const char spritebuttonName[] = "ITUSpriteButton";

static void SpriteButtonOnStop(ITUSprite* sprite)
{
    ITUSpriteButton* sbtn = (ITUSpriteButton*) sprite;
    assert(sbtn);

    ituExecActions((ITUWidget*)sbtn, sbtn->sprite.actions, ITU_EVENT_PRESS, 0);
    sbtn->pressed = false;
    ituSpriteGoto(sprite, 0);
}

bool ituSpriteButtonUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUSpriteButton* sbtn = (ITUSpriteButton*) widget;
    assert(sbtn);

    result |= ituSpriteUpdate(widget, ev, arg1, arg2, arg3);

    if (ev == ITU_EVENT_MOUSEDOWN)
    {
        if (ituWidgetIsEnabled(widget) && !result)
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                sbtn->pressed = true;
                result |= ituExecActions((ITUWidget*)sbtn, sbtn->sprite.actions, ev, 0);
            }
        }
    }
    else if (ev == ITU_EVENT_MOUSEUP)
    {
        if (ituWidgetIsEnabled(widget) && sbtn->pressed)
        {
            ituSpritePlay(&sbtn->sprite, 0);
        }
    }
    else if (ev == ITU_EVENT_TOUCHSLIDELEFT || ev == ITU_EVENT_TOUCHSLIDEUP || ev == ITU_EVENT_TOUCHSLIDERIGHT || ev == ITU_EVENT_TOUCHSLIDEDOWN || ev >= ITU_EVENT_CUSTOM || ev == ITU_EVENT_TIMER)
    {
        if (ituWidgetIsEnabled(widget) && !result)
            result |= ituExecActions((ITUWidget*)sbtn, sbtn->sprite.actions, ev, arg1);
    }
    else if (ituWidgetIsActive(widget) && ituWidgetIsEnabled(widget) && !result)
    {
        switch (ev)
        {
        case ITU_EVENT_KEYDOWN:
            if (arg1 == ituScene->enterKey)
            {
                sbtn->pressed = true;
                result |= ituExecActions((ITUWidget*)sbtn, sbtn->sprite.actions, ev, arg1);
            }
            break;

        case ITU_EVENT_KEYUP:
            if (arg1 == ituScene->enterKey)
            {
                ituSpritePlay(&sbtn->sprite, 0);
            }
            break;
        }
        result |= widget->dirty;
    }
    return widget->visible ? result : false;
}

void ituSpriteButtonInit(ITUSpriteButton* sbtn)
{
    assert(sbtn);
    ITU_ASSERT_THREAD();

    memset(sbtn, 0, sizeof (ITUSpriteButton));

    ituSpriteInit(&sbtn->sprite);

    ituWidgetSetType(sbtn, ITU_SPRITEBUTTON);
    ituWidgetSetName(sbtn, spritebuttonName);
    ituWidgetSetUpdate(sbtn, ituSpriteButtonUpdate);
    ituSpriteSetOnStop(sbtn, SpriteButtonOnStop);
}

void ituSpriteButtonLoad(ITUSpriteButton* sbtn, uint32_t base)
{
    assert(sbtn);

    ituSpriteLoad(&sbtn->sprite, base);
    ituWidgetSetUpdate(sbtn, ituSpriteButtonUpdate);
    ituSpriteSetOnStop(sbtn, SpriteButtonOnStop);
}

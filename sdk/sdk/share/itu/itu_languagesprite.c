#include <assert.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"

static const char languageSpriteName[] = "ITULanguageSprite";

bool ituLanguageSpriteUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUSprite* sprite = (ITUSprite*) widget;
    ITULanguageSprite* ls = (ITULanguageSprite*) widget;
    assert(ls);

    if (ev == ITU_EVENT_LANGUAGE)
    {
        int count = itcTreeGetChildCount(sprite);

        if (arg1 < 0 || arg1 >= count)
            arg1 = 0;

        ituSpriteGoto(sprite, arg1);
        result = widget->dirty = true;
    }
    result |= ituSpriteUpdate(widget, ev, arg1, arg2, arg3);

    return widget->visible ? result : false;
}

void ituLanguageSpriteOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    ITUSprite* sprite = (ITUSprite*) widget;
    assert(widget);

    ituWidgetOnAction(sprite->child, action, param);
}

void ituLanguageSpriteInit(ITULanguageSprite* ls)
{
    assert(ls);
    ITU_ASSERT_THREAD();

    memset(ls, 0, sizeof (ITULanguageSprite));

    ituSpriteInit(&ls->sprite);

    ituWidgetSetType(ls, ITU_LANGUAGESPRITE);
    ituWidgetSetName(ls, languageSpriteName);
    ituWidgetSetUpdate(ls, ituLanguageSpriteUpdate);
    ituWidgetSetOnAction(ls, ituLanguageSpriteOnAction);
}

void ituLanguageSpriteLoad(ITULanguageSprite* ls, uint32_t base)
{
    assert(ls);

    ituSpriteLoad(&ls->sprite, base);
    ituWidgetSetUpdate(ls, ituLanguageSpriteUpdate);
    ituWidgetSetOnAction(ls, ituLanguageSpriteOnAction);
}

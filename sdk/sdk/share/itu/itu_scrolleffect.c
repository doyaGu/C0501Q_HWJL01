#include <assert.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"

void ituScrollEffectUpdate(struct ITUEffectTag* effect, struct ITUWidgetTag* widget)
{
    ITUScrollEffect* se = (ITUScrollEffect*) effect;
    assert(effect);
    assert(widget);

    switch (se->type)
    {
    case ITU_SCROLL_IN_LEFT:
        widget->rect.x = se->orgX + widget->rect.width - widget->rect.width * effect->currStep / effect->totalStep;
        break;

    case ITU_SCROLL_IN_UP:
        widget->rect.y = se->orgY + widget->rect.height - widget->rect.height * effect->currStep / effect->totalStep;
        break;

    case ITU_SCROLL_IN_RIGHT:
        widget->rect.x = se->orgX - (widget->rect.width - widget->rect.width * effect->currStep / effect->totalStep);
        break;

    case ITU_SCROLL_IN_DOWN:
        widget->rect.y = se->orgY - (widget->rect.height - widget->rect.height * effect->currStep / effect->totalStep);
        break;

    case ITU_SCROLL_OUT_LEFT:
        widget->rect.x = se->orgX - widget->rect.width * effect->currStep / effect->totalStep;
        break;

    case ITU_SCROLL_OUT_UP:
        widget->rect.y = se->orgY - widget->rect.height * effect->currStep / effect->totalStep;
        break;

    case ITU_SCROLL_OUT_RIGHT:
        widget->rect.x = se->orgX + widget->rect.width * effect->currStep / effect->totalStep;
        break;

    case ITU_SCROLL_OUT_DOWN:
        widget->rect.y = se->orgY + widget->rect.height * effect->currStep / effect->totalStep;
        break;
    }
    ituEffectUpdateImpl(effect, widget);
}

void ituScrollEffectStart(struct ITUEffectTag* effect, struct ITUWidgetTag* widget)
{
    ITUScrollEffect* se = (ITUScrollEffect*) effect;
    assert(effect);
    assert(widget);
    ITU_ASSERT_THREAD();

    ituEffectStartImpl(effect, widget);

    se->orgX = widget->rect.x;
    se->orgY = widget->rect.y;
    ituWidgetSetBound(widget, widget->rect.x, widget->rect.y, widget->rect.width, widget->rect.height);
}

void ituScrollEffectStop(struct ITUEffectTag* effect, struct ITUWidgetTag* widget)
{
    ITUScrollEffect* se = (ITUScrollEffect*) effect;
    assert(effect);
    assert(widget);
    ITU_ASSERT_THREAD();

    ituEffectStopImpl(effect, widget);

    widget->rect.x = se->orgX;
    widget->rect.y = se->orgY;
    ituWidgetSetBound(widget, 0, 0, 0, 0);
}

void ituScrollEffectInit(ITUScrollEffect* se, ITUScrollType type)
{
    assert(se);
    ITU_ASSERT_THREAD();

    memset(se, 0, sizeof (ITUScrollEffect));

    ituEffectInit(&se->effect);
    
    ituEffectSetStart(se, ituScrollEffectStart);
    ituEffectSetStop(se, ituScrollEffectStop);
    ituEffectSetUpdate(se, ituScrollEffectUpdate);

    se->type = type;
}

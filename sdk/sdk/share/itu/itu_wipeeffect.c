#include <assert.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"

void ituWipeEffectUpdate(struct ITUEffectTag* effect, struct ITUWidgetTag* widget)
{
    ITUWipeEffect* we = (ITUWipeEffect*) effect;
    int value;
    assert(effect);
    assert(widget);

    switch (we->type)
    {
    case ITU_WIPE_IN_LEFT:
        value = widget->rect.width - widget->rect.width * effect->currStep / effect->totalStep;
        ituWidgetSetBound(widget, widget->rect.x + value, widget->rect.y, widget->rect.width - value, widget->rect.height);
        break;

    case ITU_WIPE_IN_UP:
        value = widget->rect.height - widget->rect.height * effect->currStep / effect->totalStep;
        ituWidgetSetBound(widget, widget->rect.x, widget->rect.y + value, widget->rect.width, widget->rect.height - value);
        break;

    case ITU_WIPE_IN_RIGHT:
        value = widget->rect.width - widget->rect.width * effect->currStep / effect->totalStep;
        ituWidgetSetBound(widget, widget->rect.x, widget->rect.y, widget->rect.width - value, widget->rect.height);
        break;

    case ITU_WIPE_IN_DOWN:
        value = widget->rect.height - widget->rect.height * effect->currStep / effect->totalStep;
        ituWidgetSetBound(widget, widget->rect.x, widget->rect.y, widget->rect.width, widget->rect.height - value);
        break;

    case ITU_WIPE_OUT_LEFT:
        value = widget->rect.width * effect->currStep / effect->totalStep;
        ituWidgetSetBound(widget, widget->rect.x, widget->rect.y, widget->rect.width - value, widget->rect.height);
        break;

    case ITU_WIPE_OUT_UP:
        value = widget->rect.height * effect->currStep / effect->totalStep;
        ituWidgetSetBound(widget, widget->rect.x, widget->rect.y, widget->rect.width, widget->rect.height - value);
        break;

    case ITU_WIPE_OUT_RIGHT:
        value = widget->rect.width * effect->currStep / effect->totalStep;
        ituWidgetSetBound(widget, widget->rect.x + value, widget->rect.y, widget->rect.width - value, widget->rect.height);
        break;

    case ITU_WIPE_OUT_DOWN:
        value = widget->rect.height * effect->currStep / effect->totalStep;
        ituWidgetSetBound(widget, widget->rect.x, widget->rect.y + value, widget->rect.width, widget->rect.height - value);
        break;
    }
    ituEffectUpdateImpl(effect, widget);
}

void ituWipeEffectStart(struct ITUEffectTag* effect, struct ITUWidgetTag* widget)
{
    ITUWipeEffect* we = (ITUWipeEffect*) effect;
    assert(effect);
    assert(widget);
    ITU_ASSERT_THREAD();

    ituEffectStartImpl(effect, widget);
    ituWidgetSetBound(widget, widget->rect.x, widget->rect.y, widget->rect.width, widget->rect.height);
}

void ituWipeEffectStop(struct ITUEffectTag* effect, struct ITUWidgetTag* widget)
{
    ITUWipeEffect* we = (ITUWipeEffect*) effect;
    assert(effect);
    assert(widget);
    ITU_ASSERT_THREAD();

    ituEffectStopImpl(effect, widget);
    ituWidgetSetBound(widget, 0, 0, 0, 0);
}

void ituWipeEffectInit(ITUWipeEffect* we, ITUWipeType type)
{
    assert(we);
    ITU_ASSERT_THREAD();

    memset(we, 0, sizeof (ITUWipeEffect));

    ituEffectInit(&we->effect);
    
    ituEffectSetStart(we, ituWipeEffectStart);
    ituEffectSetStop(we, ituWipeEffectStop);
    ituEffectSetUpdate(we, ituWipeEffectUpdate);

    we->type = type;
}

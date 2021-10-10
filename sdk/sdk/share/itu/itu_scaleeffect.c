#include <assert.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

void ituScaleEffectUpdate(struct ITUEffectTag* effect, struct ITUWidgetTag* widget)
{
    ITUScaleEffect* se = (ITUScaleEffect*) effect;
    int centerX, centerY;
    assert(effect);
    assert(widget);

    centerX = se->orgX + se->orgWidth / 2;
    centerY = se->orgY + se->orgHeight / 2;

    if (se->enlarge)
    {
        widget->rect.x = centerX - (se->orgWidth / 2) * effect->currStep / effect->totalStep;
        widget->rect.y = centerY - (se->orgHeight / 2) * effect->currStep / effect->totalStep;
        widget->rect.width = se->orgWidth * effect->currStep / effect->totalStep;
        widget->rect.height = se->orgHeight * effect->currStep / effect->totalStep;
    }
    else
    {
        widget->rect.x = centerX - (se->orgWidth / 2) * (effect->totalStep - effect->currStep) / effect->totalStep;
        widget->rect.y = centerY - (se->orgHeight / 2) * (effect->totalStep - effect->currStep) / effect->totalStep;
        widget->rect.width = se->orgWidth * (effect->totalStep - effect->currStep) / effect->totalStep;
        widget->rect.height = se->orgHeight * (effect->totalStep - effect->currStep) / effect->totalStep;
    }
    ituWidgetSetBound(widget, widget->rect.x, widget->rect.y, widget->rect.width, widget->rect.height);
    ituEffectUpdateImpl(effect, widget);
}

void ituScaleEffectStart(struct ITUEffectTag* effect, struct ITUWidgetTag* widget)
{
    ITUScaleEffect* se = (ITUScaleEffect*) effect;
    assert(effect);
    assert(widget);
    ITU_ASSERT_THREAD();

    ituEffectStartImpl(effect, widget);

    se->orgX = widget->rect.x;
    se->orgY = widget->rect.y;
    se->orgWidth = widget->rect.width;
    se->orgHeight = widget->rect.height;
    ituWidgetSetBound(widget, widget->rect.x, widget->rect.y, widget->rect.width, widget->rect.height);
}

void ituScaleEffectStop(struct ITUEffectTag* effect, struct ITUWidgetTag* widget)
{
    ITUScaleEffect* se = (ITUScaleEffect*) effect;
    assert(effect);
    assert(widget);
    ITU_ASSERT_THREAD();

    ituEffectStopImpl(effect, widget);

    widget->rect.x = se->orgX;
    widget->rect.y = se->orgY;
    widget->rect.width = se->orgWidth;
    widget->rect.height = se->orgHeight;    
    ituWidgetSetBound(widget, 0, 0, 0, 0);
}

void ituScaleEffectInit(ITUScaleEffect* se, bool enlarge)
{
    assert(se);
    ITU_ASSERT_THREAD();

    memset(se, 0, sizeof (ITUScaleEffect));

    ituEffectInit(&se->effect);
    
    ituEffectSetStart(se, ituScaleEffectStart);
    ituEffectSetStop(se, ituScaleEffectStop);
    ituEffectSetUpdate(se, ituScaleEffectUpdate);

    se->enlarge = enlarge;
}

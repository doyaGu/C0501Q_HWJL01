#include <assert.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"

static void ScrollFadeEffectUpdate(struct ITUEffectTag* effect, struct ITUWidgetTag* widget)
{
    ITUScrollFadeEffect* sfe = (ITUScrollFadeEffect*) effect;
    assert(effect);
    assert(widget);

    if (sfe->fadeIn)
        widget->alpha = sfe->orgAlpha * effect->currStep / effect->totalStep;
    else
        widget->alpha = sfe->orgAlpha * (effect->totalStep - effect->currStep) / effect->totalStep;

    ituScrollEffectUpdate(effect, widget);
}

static void ScrollFadeEffectStart(struct ITUEffectTag* effect, struct ITUWidgetTag* widget)
{
    ITUScrollFadeEffect* sfe = (ITUScrollFadeEffect*) effect;
    assert(effect);
    assert(widget);
    ITU_ASSERT_THREAD();

    ituScrollEffectStart(effect, widget);

    sfe->orgAlpha = widget->alpha;
}

static void ScrollFadeEffectStop(struct ITUEffectTag* effect, struct ITUWidgetTag* widget)
{
    ITUScrollFadeEffect* sfe = (ITUScrollFadeEffect*) effect;
    assert(effect);
    assert(widget);
    ITU_ASSERT_THREAD();

    ituScrollEffectStop(effect, widget);

    widget->alpha = sfe->orgAlpha;
}

void ituScrollFadeEffectInit(ITUScrollFadeEffect* sfe, ITUScrollType type, bool fadeIn)
{
    assert(sfe);
    ITU_ASSERT_THREAD();

    memset(sfe, 0, sizeof (ITUScrollFadeEffect));

    ituScrollEffectInit(&sfe->effect, type);
    
    ituEffectSetStart(sfe, ScrollFadeEffectStart);
    ituEffectSetStop(sfe, ScrollFadeEffectStop);
    ituEffectSetUpdate(sfe, ScrollFadeEffectUpdate);

    sfe->fadeIn = fadeIn;
}

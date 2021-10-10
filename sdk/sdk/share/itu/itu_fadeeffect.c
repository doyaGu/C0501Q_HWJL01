#include <assert.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static void FadeEffectUpdate(struct ITUEffectTag* effect, struct ITUWidgetTag* widget)
{
    ITUFadeEffect* fe = (ITUFadeEffect*) effect;
    assert(effect);
    assert(widget);

    if (fe->fadeIn)
        widget->alpha = fe->orgAlpha * effect->currStep / effect->totalStep;
    else
        widget->alpha = fe->orgAlpha * (effect->totalStep - effect->currStep) / effect->totalStep;

    ituEffectUpdateImpl(effect, widget);
}

static void FadeEffectStart(struct ITUEffectTag* effect, struct ITUWidgetTag* widget)
{
    ITUFadeEffect* fe = (ITUFadeEffect*) effect;
    assert(effect);
    assert(widget);
    ITU_ASSERT_THREAD();

    ituEffectStartImpl(effect, widget);

    fe->orgAlpha = widget->alpha;
}

static void FadeEffectStop(struct ITUEffectTag* effect, struct ITUWidgetTag* widget)
{
    ITUFadeEffect* fe = (ITUFadeEffect*) effect;
    assert(effect);
    assert(widget);
    ITU_ASSERT_THREAD();

    ituEffectStopImpl(effect, widget);

    widget->alpha = fe->orgAlpha;
}

void ituFadeEffectInit(ITUFadeEffect* fe, bool fadeIn)
{
    assert(fe);
    ITU_ASSERT_THREAD();

    memset(fe, 0, sizeof (ITUFadeEffect));

    ituEffectInit(&fe->effect);
    
    ituEffectSetStart(fe, FadeEffectStart);
    ituEffectSetStop(fe, FadeEffectStop);
    ituEffectSetUpdate(fe, FadeEffectUpdate);

    fe->fadeIn = fadeIn;
}

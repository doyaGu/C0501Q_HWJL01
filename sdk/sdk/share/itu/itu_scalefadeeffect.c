#include <assert.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static void ScaleFadeEffectUpdate(struct ITUEffectTag* effect, struct ITUWidgetTag* widget)
{
    ITUScaleFadeEffect* sfe = (ITUScaleFadeEffect*) effect;
    assert(effect);
    assert(widget);

    if (sfe->fadeIn)
        widget->alpha = sfe->orgAlpha * effect->currStep / effect->totalStep;
    else
        widget->alpha = sfe->orgAlpha * (effect->totalStep - effect->currStep) / effect->totalStep;

    ituScaleEffectUpdate(effect, widget);
}

static void ScaleFadeEffectStart(struct ITUEffectTag* effect, struct ITUWidgetTag* widget)
{
    ITUScaleFadeEffect* sfe = (ITUScaleFadeEffect*) effect;
    assert(effect);
    assert(widget);
    ITU_ASSERT_THREAD();

    ituScaleEffectStart(effect, widget);

    sfe->orgAlpha = widget->alpha;
}

static void ScaleFadeEffectStop(struct ITUEffectTag* effect, struct ITUWidgetTag* widget)
{
    ITUScaleFadeEffect* sfe = (ITUScaleFadeEffect*) effect;
    assert(effect);
    assert(widget);
    ITU_ASSERT_THREAD();

    ituScaleEffectStop(effect, widget);

    widget->alpha = sfe->orgAlpha;
}

void ituScaleFadeEffectInit(ITUScaleFadeEffect* sfe, bool fadeIn)
{
    assert(sfe);
    ITU_ASSERT_THREAD();

    memset(sfe, 0, sizeof (ITUScaleFadeEffect));

    ituScaleEffectInit(&sfe->effect, fadeIn);
    
    ituEffectSetStart(sfe, ScaleFadeEffectStart);
    ituEffectSetStop(sfe, ScaleFadeEffectStop);
    ituEffectSetUpdate(sfe, ScaleFadeEffectUpdate);

    sfe->fadeIn = fadeIn;
}

#include <assert.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static void EffectExit(struct ITUEffectTag* effect)
{
    // DO NOTHING
}

void ituEffectUpdateImpl(struct ITUEffectTag* effect, struct ITUWidgetTag* widget)
{
    assert(effect);
    assert(widget);

    if (effect->currStep == effect->totalStep)
        effect->playing = false;
    else
        effect->currStep++;
}

void ituEffectStartImpl(struct ITUEffectTag* effect, struct ITUWidgetTag* widget)
{
    assert(effect);
    assert(widget);

    effect->currStep    = 0;
    effect->playing     = true;
}

void ituEffectStopImpl(struct ITUEffectTag* effect, struct ITUWidgetTag* widget)
{
    assert(effect);
    assert(widget);

    effect->currStep    = effect->totalStep;
    effect->playing     = false;
}

void ituEffectInit(ITUEffect* effect)
{
    assert(effect);

    memset(effect, 0, sizeof (ITUEffect));

    effect->totalStep = ITU_EFFECT_STEP_COUNT;

    effect->Exit    = EffectExit;
    effect->Start   = ituEffectStartImpl;    
    effect->Stop    = ituEffectStopImpl;
    effect->Update  = ituEffectUpdateImpl;
}

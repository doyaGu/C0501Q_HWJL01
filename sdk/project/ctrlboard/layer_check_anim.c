#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"

static ITUAnimation* checkAnimOKAnimation;
static ITUTrackBar* checkAnimSpeedTrackBar;
static ITUProgressBar* checkAnimSpeedProgressBar;

bool CheckAnimSpeedTrackBarOnChanged(ITUWidget* widget, char* param)
{
    int value = atoi(param);

    if (value == 0)
    {
        checkAnimOKAnimation->playing = false;
    }
    else
    {
        checkAnimOKAnimation->playing = true;
        checkAnimOKAnimation->delay = 100 / value;
    }
    return true;
}

bool CheckAnimScaleCheckBoxOnPress(ITUWidget* widget, char* param)
{
    ITUCheckBox* checkbox = (ITUCheckBox*)widget;

    if (ituCheckBoxIsChecked(checkbox))
        checkAnimOKAnimation->animationFlags |= ITU_ANIM_SCALE;
    else
        checkAnimOKAnimation->animationFlags &= ~ITU_ANIM_SCALE;

    if (checkAnimOKAnimation->animationFlags & (ITU_ANIM_SCALE | ITU_ANIM_ROTATE | ITU_ANIM_COLOR | ITU_ANIM_MOVE) && checkAnimSpeedTrackBar->value > 0)
        checkAnimOKAnimation->playing = true;
    else
        checkAnimOKAnimation->playing = false;

    return true;
}

bool CheckAnimRotateCheckBox(ITUWidget* widget, char* param)
{
    ITUCheckBox* checkbox = (ITUCheckBox*)widget;

    if (ituCheckBoxIsChecked(checkbox))
        checkAnimOKAnimation->animationFlags |= ITU_ANIM_ROTATE;
    else
        checkAnimOKAnimation->animationFlags &= ~ITU_ANIM_ROTATE;

    if (checkAnimOKAnimation->animationFlags & (ITU_ANIM_SCALE | ITU_ANIM_ROTATE | ITU_ANIM_COLOR | ITU_ANIM_MOVE) && checkAnimSpeedTrackBar->value > 0)
        checkAnimOKAnimation->playing = true;
    else
        checkAnimOKAnimation->playing = false;

    return true;
}

bool CheckAnimAlphablendCheckBox(ITUWidget* widget, char* param)
{
    ITUCheckBox* checkbox = (ITUCheckBox*)widget;

    if (ituCheckBoxIsChecked(checkbox))
        checkAnimOKAnimation->animationFlags |= ITU_ANIM_COLOR;
    else
        checkAnimOKAnimation->animationFlags &= ~ITU_ANIM_COLOR;

    if (checkAnimOKAnimation->animationFlags & (ITU_ANIM_SCALE | ITU_ANIM_ROTATE | ITU_ANIM_COLOR | ITU_ANIM_MOVE) && checkAnimSpeedTrackBar->value > 0)
        checkAnimOKAnimation->playing = true;
    else
        checkAnimOKAnimation->playing = false;

    return true;
}

bool CheckAnimMoveCheckBox(ITUWidget* widget, char* param)
{
    ITUCheckBox* checkbox = (ITUCheckBox*)widget;

    if (ituCheckBoxIsChecked(checkbox))
        checkAnimOKAnimation->animationFlags |= ITU_ANIM_MOVE;
    else
        checkAnimOKAnimation->animationFlags &= ~ITU_ANIM_MOVE;

    if (checkAnimOKAnimation->animationFlags & (ITU_ANIM_SCALE | ITU_ANIM_ROTATE | ITU_ANIM_COLOR | ITU_ANIM_MOVE) && checkAnimSpeedTrackBar->value > 0)
        checkAnimOKAnimation->playing = true;
    else
        checkAnimOKAnimation->playing = false;

    return true;
}

bool CheckAnimOnEnter(ITUWidget* widget, char* param)
{
    if (!checkAnimOKAnimation)
    {
        checkAnimOKAnimation = ituSceneFindWidget(&theScene, "checkAnimOKAnimation");
        assert(checkAnimOKAnimation);

        checkAnimSpeedTrackBar = ituSceneFindWidget(&theScene, "checkAnimSpeedTrackBar");
        assert(checkAnimSpeedTrackBar);

        checkAnimSpeedProgressBar = ituSceneFindWidget(&theScene, "checkAnimSpeedProgressBar");
        assert(checkAnimSpeedProgressBar);
    }
    checkAnimOKAnimation->animationFlags &= ~(ITU_ANIM_SCALE | ITU_ANIM_ROTATE | ITU_ANIM_COLOR | ITU_ANIM_MOVE);
    ituAnimationReset(checkAnimOKAnimation);
    ituTrackBarSetValue(checkAnimSpeedTrackBar, 0);
    ituProgressBarSetValue(checkAnimSpeedProgressBar, 0);

    return true;
}

void CheckAnimReset(void)
{
    checkAnimOKAnimation = NULL;
}

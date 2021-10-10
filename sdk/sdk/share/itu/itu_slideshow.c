#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include "ite/itu.h"
#include "itu_cfg.h"

static const char slideshowName[] = "ITUSlideshow";

static void SlideshowOnStop(ITUSlideshow* slideshow)
{
    // DO NOTHING
}

bool ituSlideshowClone(ITUWidget* widget, ITUWidget** cloned)
{
    ITUSlideshow* newSlideshow;
    assert(widget);
    assert(cloned);
    ITU_ASSERT_THREAD();

    if (*cloned == NULL)
    {
        ITUWidget* newWidget = malloc(sizeof(ITUSlideshow));
        if (newWidget == NULL)
            return false;

        memcpy(newWidget, widget, sizeof(ITUSlideshow));
        newWidget->tree.child = newWidget->tree.parent = newWidget->tree.sibling = NULL;
        *cloned = newWidget;
    }

    newSlideshow = (ITUSlideshow*)*cloned;
    newSlideshow->child = (ITUWidget*) itcTreeGetChildAt(newSlideshow, newSlideshow->frame);

    return ituWidgetCloneImpl(widget, cloned);
}

bool ituSlideshowUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    ITUSlideshow* slideshow = (ITUSlideshow*) widget;
    bool result = false;
    assert(slideshow);

    if (ev == ITU_EVENT_TIMER)
    {
        if (slideshow->playing)
        {
            if (--slideshow->delayCount <= 0)
            {
                ITUEffectType effect;

                if (slideshow->effect == -1)
                {
                    time_t t;
                    int tryCount = ITU_EFFECT_MAX_COUNT * 2;

                    srand((unsigned) time(&t));

                    do
                    {
                        if (--tryCount <= 0)
                            break;

                        effect = rand() % ITU_EFFECT_MAX_COUNT;
                    } while (effect <= 0);

                    if (tryCount <= 0)
                        effect = ITU_EFFECT_FADE;
                }
                else
                {
                    effect = slideshow->effect;
                }

                if (slideshow->child)
                {
                    ituWidgetHide(slideshow->child, effect, slideshow->effectSteps);
                }
                if (slideshow->frame >= slideshow->childCount - 1)
                {
                    if (slideshow->repeat)
                    {
                        slideshow->frame = 0;
                        slideshow->child = (ITUWidget*) itcTreeGetChildAt(slideshow, slideshow->frame);
                        ituWidgetShow(slideshow->child, effect, slideshow->effectSteps);
                    }
                    else
                    {
                        slideshow->playing = false;
                        ituSlideshowOnStop(slideshow);
                        ituExecActions((ITUWidget*)slideshow, slideshow->actions, ITU_EVENT_STOPPED, 0);
                    }
                }
                else
                {
                    slideshow->frame++;
                    slideshow->child = (ITUWidget*) itcTreeGetChildAt(slideshow, slideshow->frame);
                    ituWidgetShow(slideshow->child, effect, slideshow->effectSteps);
                }
                slideshow->delayCount = slideshow->delay;
                result = widget->dirty = true;
            }
        }
    }
    else if (ev == ITU_EVENT_LAYOUT)
    {
        ITCTree* node;
        slideshow->childCount = itcTreeGetChildCount(slideshow);

        if (slideshow->frame >= 0)
        {
            slideshow->child = (ITUWidget*) itcTreeGetChildAt(slideshow, slideshow->frame);
        }
        else
        {
            slideshow->child = NULL;
        }

        for (node = widget->tree.child; node; node = node->sibling)
        {
            ITUWidget* child = (ITUWidget*)node;
            ituWidgetSetVisible(child, child == slideshow->child ? true : false);
        }

        result = widget->dirty = true;
    }
    result |= ituWidgetUpdateImpl(widget, ev, arg1, arg2, arg3);
    return widget->visible ? result : false;
}

void ituSlideshowOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    ITUSlideshow* slideshow = (ITUSlideshow*) widget;
    assert(widget);

    switch (action)
    {
    case ITU_ACTION_PLAY:
        if (param[0] != '\0')
            ituSlideshowPlay(slideshow, atoi(param));
        break;

    case ITU_ACTION_STOP:
        ituSlideshowStop(slideshow);
        ituSlideshowOnStop(slideshow);
        ituExecActions(widget, slideshow->actions, ITU_EVENT_STOPPED, 0);
        break;

    case ITU_ACTION_GOTO:
        if (param[0] != '\0')
            ituSlideshowGoto(slideshow, atoi(param));
        break;

    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

void ituSlideshowInit(ITUSlideshow* slideshow)
{
    assert(slideshow);
    ITU_ASSERT_THREAD();

    memset(slideshow, 0, sizeof (ITUSlideshow));

    ituWidgetInit(&slideshow->widget);

    ituWidgetSetType(slideshow, ITU_SLIDESHOW);
    ituWidgetSetName(slideshow, slideshowName);
    ituWidgetSetClone(slideshow, ituSlideshowClone);
    ituWidgetSetUpdate(slideshow, ituSlideshowUpdate);
    ituWidgetSetOnAction(slideshow, ituSlideshowOnAction);
    ituSlideshowSetOnStop(slideshow, SlideshowOnStop);

    ituSlideshowSetDelay(slideshow, 10);
    slideshow->childCount = itcTreeGetChildCount(slideshow);
    slideshow->child = (ITUWidget*) itcTreeGetChildAt(slideshow, slideshow->frame);
}

void ituSlideshowLoad(ITUSlideshow* slideshow, uint32_t base)
{
    assert(slideshow);

    ituWidgetLoad((ITUWidget*)slideshow, base);

    ituWidgetSetClone(slideshow, ituSlideshowClone);
    ituWidgetSetUpdate(slideshow, ituSlideshowUpdate);
    ituWidgetSetOnAction(slideshow, ituSlideshowOnAction);
    ituSlideshowSetOnStop(slideshow, SlideshowOnStop);

    if (slideshow->playing)
        slideshow->delayCount = slideshow->delay;
}

void ituSlideshowSetDelay(ITUSlideshow* slideshow, int delay)
{
    assert(slideshow);
    ITU_ASSERT_THREAD();

    slideshow->delay           = delay;
    slideshow->widget.dirty    = true;
}

void ituSlideshowPlay(ITUSlideshow* slideshow, int frame)
{
    assert(slideshow);
    ITU_ASSERT_THREAD();

    if (frame >= 0)
        ituSlideshowGoto(slideshow, frame);

    slideshow->delayCount      = slideshow->delay;
    slideshow->playing         = true;
    slideshow->widget.dirty    = true;
}

void ituSlideshowStop(ITUSlideshow* slideshow)
{
    assert(slideshow);
    ITU_ASSERT_THREAD();
    slideshow->playing         = false;
    slideshow->widget.dirty    = true;
}

void ituSlideshowGoto(ITUSlideshow* slideshow, int frame)
{
    int count;
    assert(slideshow);
    ITU_ASSERT_THREAD();

    if (slideshow->frame == frame)
        return;
    
    if (frame == -1)
    {
        slideshow->child = NULL;
    }
    count = itcTreeGetChildCount(slideshow);
    if (frame >= 0 && frame < count)
    {
        slideshow->frame = frame;
        ituWidgetUpdate(slideshow, ITU_EVENT_LAYOUT, 0, 0, 0);
    }
}

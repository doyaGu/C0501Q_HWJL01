#include <assert.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char layerName[] = "ITULayer";

void ituLayerExit(ITUWidget* widget)
{
    ITULayer* layer = (ITULayer*) widget;
    assert(widget);
    ITU_ASSERT_THREAD();

    ituWidgetExitImpl(widget);

    if (layer->buffer)
    {
        free(layer->buffer);
        layer->buffer = NULL;
    }
}

bool ituLayerClone(ITUWidget* widget, ITUWidget** cloned)
{
    assert(widget);
    assert(cloned);
    ITU_ASSERT_THREAD();

    if (*cloned == NULL)
    {
        ITUWidget* newWidget = malloc(sizeof(ITULayer));
        if (newWidget == NULL)
            return false;

        memcpy(newWidget, widget, sizeof(ITULayer));
        newWidget->tree.child = newWidget->tree.parent = newWidget->tree.sibling = NULL;
        *cloned = newWidget;
    }

    return ituWidgetCloneImpl(widget, cloned);
}

bool ituLayerUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITULayer* layer = (ITULayer*) widget;
    assert(layer);

	if (ev == ITU_EVENT_LAYOUT)
	{
		int layer_init_customdata = 0;
		ITCTree* node;
		assert(ituScene);

		for (node = &ituScene->root->tree; node; node = node->sibling)
		{
			ITULayer* ll = (ITULayer*)node;

			if (ll)
				layer_init_customdata += (int)ituWidgetGetCustomData(ll);
		}

		if (layer_init_customdata == 0)
			ituWidgetSetCustomData(layer, 1);
	}

    if (ev == ITU_EVENT_MOUSEUP)
    {
        if (ituWidgetIsEnabled(widget))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                ituExecActions(widget, layer->actions, ev, 0);
                result |= widget->dirty;
            }
        }
    }
    else if (ev == ITU_EVENT_MOUSEDOUBLECLICK 
          || ev == ITU_EVENT_MOUSELONGPRESS 
          || ev == ITU_EVENT_TOUCHSLIDELEFT 
          || ev == ITU_EVENT_TOUCHSLIDEUP 
          || ev == ITU_EVENT_TOUCHSLIDERIGHT 
          || ev == ITU_EVENT_TOUCHSLIDEDOWN)
    {
        if (ituWidgetIsEnabled(widget))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                ituExecActions(widget, layer->actions, ev, arg1);
                result |= widget->dirty;
            }
        }
    }
    else if (ev >= ITU_EVENT_CUSTOM || ev == ITU_EVENT_TIMER)
    {
        if (ituWidgetIsEnabled(widget))
        {
            ituExecActions(widget, layer->actions, ev, arg1);
            result |= widget->dirty;
        }
    }
    else if (ev == ITU_EVENT_LOAD_EXTERNAL)
    {
        arg1 = (int)layer;
    }
    else if (ituWidgetIsEnabled(widget))
    {
        switch (ev)
        {
        case ITU_EVENT_KEYDOWN:
            ituExecActions(widget, layer->actions, ev, arg1);
            break;

        case ITU_EVENT_KEYUP:
            ituExecActions(widget, layer->actions, ev, arg1);
            break;
        }
        result |= widget->dirty;
    }
    result |= ituWidgetUpdateImpl(widget, ev, arg1, arg2, arg3);
    return widget->visible ? result : false;
}

void ituLayerOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    ITULayer* layer = (ITULayer*) widget;
    assert(layer);

    switch (action)
    {
    case ITU_ACTION_GOTO:
        ituLayerGoto(layer);
        break;

    case ITU_ACTION_DODELAY0:
        ituExecActions(widget, layer->actions, ITU_EVENT_DELAY0, atoi(param));
        break;

    case ITU_ACTION_DODELAY1:
        ituExecActions(widget, layer->actions, ITU_EVENT_DELAY1, atoi(param));
        break;

    case ITU_ACTION_DODELAY2:
        ituExecActions(widget, layer->actions, ITU_EVENT_DELAY2, atoi(param));
        break;

    case ITU_ACTION_DODELAY3:
        ituExecActions(widget, layer->actions, ITU_EVENT_DELAY3, atoi(param));
        break;

    case ITU_ACTION_DODELAY4:
        ituExecActions(widget, layer->actions, ITU_EVENT_DELAY4, atoi(param));
        break;

    case ITU_ACTION_DODELAY5:
        ituExecActions(widget, layer->actions, ITU_EVENT_DELAY5, atoi(param));
        break;

    case ITU_ACTION_DODELAY6:
        ituExecActions(widget, layer->actions, ITU_EVENT_DELAY6, atoi(param));
        break;

    case ITU_ACTION_DODELAY7:
        ituExecActions(widget, layer->actions, ITU_EVENT_DELAY7, atoi(param));
        break;

	case ITU_ACTION_LOAD_EXTERNAL:
		ituWidgetUpdate(widget, ITU_EVENT_LOAD_EXTERNAL, 0, 0, 0);
		break;

	case ITU_ACTION_RELEASE_EXTERNAL:
		free(layer->buffer);
		layer->buffer = NULL;
		break;

    case ITU_ACTION_LOAD_FONT:
        ituLayerLoadFont(layer);
        break;

    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

void ituLayerInit(ITULayer* layer)
{
    assert(layer);
    ITU_ASSERT_THREAD();

    memset(layer, 0, sizeof (ITULayer));

    ituWidgetInit(&layer->widget);

    ituWidgetSetType(layer, ITU_LAYER);
    ituWidgetSetName(layer, layerName);
    ituWidgetSetExit(layer, ituLayerExit);
    ituWidgetSetClone(layer, ituLayerClone);
    ituWidgetSetUpdate(layer, ituLayerUpdate);
    ituWidgetSetOnAction(layer, ituLayerOnAction);
}

void ituLayerLoad(ITULayer* layer, uint32_t base)
{
    assert(layer);

	//to reset custom data
	ituWidgetSetCustomData(layer, 0);

    ituWidgetLoad((ITUWidget*)layer, base);
    ituWidgetSetExit(layer, ituLayerExit);
    ituWidgetSetClone(layer, ituLayerClone);
    ituWidgetSetUpdate(layer, ituLayerUpdate);
    ituWidgetSetOnAction(layer, ituLayerOnAction);
}

static void LayerGoto(int arg)
{
    int* args = (int*) arg;
    ITULayer* layer = (ITULayer*) args[0];
    ITULayer* leaveLayer = (ITULayer*) args[1];
    ITCTree* node;
    assert(ituScene);

    for (node = &ituScene->root->tree; node; node = node->sibling)
    {
        if (ituWidgetIsVisible(node))
        {
            ITULayer* l = (ITULayer*)node;
            ituWidgetSetVisible(l, false);
        }
    }

    ituWidgetSetVisible(layer, true);
	ituWidgetSetCustomData(leaveLayer, -100);
	ituWidgetSetCustomData(layer, 1);
    ituExecActions((ITUWidget*)layer, layer->actions, ITU_EVENT_ENTER, leaveLayer ? (int)leaveLayer->widget.name : 0);
    ituExecActions((ITUWidget*)layer, layer->actions, ITU_EVENT_DELAY, 0);
    ituWidgetUpdate(layer, ITU_EVENT_LAYOUT, 0, 0, 0);
}

void ituLayerGoto(ITULayer* layer)
{
    static int args[2];
    int delay = 0;
    ITULayer* leaveLayer = NULL;
    ITULayer* visibleLayer = NULL;
    ITCTree* node;
    assert(ituScene);
    ITU_ASSERT_THREAD();

    for (node = &ituScene->root->tree; node; node = node->sibling)
    {
        ITUWidget* widget = (ITUWidget*) node;

        if (ituWidgetIsVisible(widget))
        {
            ITULayer* l = (ITULayer*)widget;
            if (ituExecActions((ITUWidget*)l, l->actions, ITU_EVENT_LEAVE, (int)layer->widget.name))
            {
                int i;

                for (i = 0; i < ITU_ACTIONS_SIZE; i++)
                {
                    ITUAction* action = &l->actions[i];
                    if (action->action == ITU_ACTION_NONE)
                        break;

                    if (action->ev == ITU_EVENT_LEAVE && action->action == ITU_ACTION_HIDE)
                    {
                        if (action->param[0] != '\0')
                        {
                            char buf[ITU_ACTION_PARAM_SIZE], *ptr, *saveptr;
                            ITUEffectType effect;

                            strcpy(buf, action->param);
                            effect = atoi(strtok_r(buf, " ", &saveptr));
                            ptr = strtok_r(NULL, " ", &saveptr);
                            if (ptr)
                            {
                                delay = ITH_MAX(delay, atoi(ptr));
                            }
                            else
                            {
                                ITUWidget* target;

                                if (action->cachedTarget)
                                {
                                    target = action->cachedTarget;
                                }
                                else
                                {
                                    target = ituSceneFindWidget(ituScene, action->target);
                                    if (!target)
                                        continue;
                                    
                                    action->cachedTarget = (void*)target;
                                }

                                if (target->effects[ITU_STATE_HIDING] != ITU_EFFECT_NONE)
                                    delay = ITH_MAX(delay, target->effectSteps);
                            }
                        }
                        else
                        {
                            ITUWidget* target;

                            if (action->cachedTarget)
                            {
                                target = action->cachedTarget;
                            }
                            else
                            {
                                target = ituSceneFindWidget(ituScene, action->target);
                                if (!target)
                                    continue;
                                
                                action->cachedTarget = (void*)target;
                            }

                            if (target->effects[ITU_STATE_HIDING] != ITU_EFFECT_NONE)
                                delay = ITH_MAX(delay, target->effectSteps);
                        }
                    }
                }
                leaveLayer = l;
            }
            visibleLayer = l;
        }
    }
    if (!leaveLayer)
        leaveLayer = visibleLayer;

    args[0] = (int) layer;
    args[1] = (int) leaveLayer;

    if (delay > 0)
        ituSceneExecuteCommand(ituScene, delay, LayerGoto, (int)args);
    else
        LayerGoto((int)args);
}

void ituLayerEnableOthers(ITULayer* layer)
{
    ITCTree* node;
    ITULayer* leaveLayer = NULL;
    assert(ituScene);
    ITU_ASSERT_THREAD();

    for (node = &ituScene->root->tree; node; node = node->sibling)
    {
        if (node != (ITCTree*)layer)
            ituWidgetEnable(node);
    }
}

void ituLayerDisableOthers(ITULayer* layer)
{
    ITCTree* node;
    ITULayer* leaveLayer = NULL;
    assert(ituScene);
    ITU_ASSERT_THREAD();

    for (node = &ituScene->root->tree; node; node = node->sibling)
    {
        if (node != (ITCTree*)layer)
            ituWidgetDisable(node);
    }
}

void ituLayerLoadExternal(ITULayer* layer)
{
    ITU_ASSERT_THREAD();
    ituWidgetUpdate(layer, ITU_EVENT_LOAD_EXTERNAL, 0, 0, 0);
}

void ituLayerReleaseExternal(ITULayer* layer)
{
    ITU_ASSERT_THREAD();
    free(layer->buffer);
    layer->buffer = NULL;
}

void ituLayerLoadFont(ITULayer* layer)
{
#ifdef CFG_ITU_FT_CACHE_SIZE
    void(*DrawGlyphPrev)(ITUSurface* surf, int x, int y, ITUGlyphFormat format, const uint8_t* bitmap, int w, int h) = ituDrawGlyph;
    ITU_ASSERT_THREAD();

    ituDrawGlyph = ituDrawGlyphEmpty;

    ituPreloadFontCache(&layer->widget, ituGetDisplaySurface());

    ituDrawGlyph = DrawGlyphPrev;

#endif // CFG_ITU_FT_CACHE_SIZE
}

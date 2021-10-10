#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

bool ituExecActions(struct ITUWidgetTag* widget, ITUAction* actions, ITUEvent ev, int arg)
{
    int i;
    bool result = false;
    assert(actions);

    for (i = 0; i < ITU_ACTIONS_SIZE; i++)
    {
        ITUAction* action = &actions[i];
        if (action->action == ITU_ACTION_NONE)
            break;

        if (action->ev == ITU_EVENT_PRESS)
        {
            switch (ev)
            {
            case ITU_EVENT_KEYDOWN:
            case ITU_EVENT_MOUSEDOWN:
            case ITU_EVENT_PRESS:
                break;

            default:
                continue;
            }
        }
        else if (action->ev != ev)
            continue;

        if (action->ev >= ITU_EVENT_DELAY0 && action->ev <= ITU_EVENT_DELAY7)
        {
            int j;
            ITUActionExecution* delayExec = NULL;

            for (j = 0; i < ITU_ACTION_QUEUE_SIZE; j++)
            {
                delayExec = &ituScene->delayQueue[j];
                
                if (delayExec->delay == 0)
                    break;

                if (delayExec->widget == widget && delayExec->action == action)
                    break;   // duplicate action
            }

            if (delayExec)
            {
                if (delayExec->delay > 0)
                    continue; // duplicate action

                delayExec->widget = widget;
                delayExec->action = action;
                delayExec->delay  = arg;
            }
            else
            {
                LOG_ERR "Out of delay queue: %d\n", ITU_ACTION_QUEUE_SIZE LOG_END
                break;
            }
        }
        else if (action->ev == ITU_EVENT_DELAY)
        {
            int j;
            ITUActionExecution* delayExec = NULL;

            for (j = 0; i < ITU_ACTION_QUEUE_SIZE; j++)
            {
                delayExec = &ituScene->delayQueue[j];
                
                if (delayExec->delay == 0)
                    break;

                if (delayExec->widget == widget && delayExec->action == action)
                    break;   // duplicate action
            }

            if (delayExec)
            {
                if (delayExec->delay > 0)
                    continue; // duplicate action

                delayExec->widget = widget;
                delayExec->action = action;
                delayExec->delay  = atoi(action->param);
            }
            else
            {
                LOG_ERR "Out of delay queue: %d\n", ITU_ACTION_QUEUE_SIZE LOG_END
                break;
            }
        }
        else
        {
            if (action->ev == ITU_EVENT_SELECT)
            {
                if (action->action == ITU_ACTION_CHECK || widget->type == ITU_COLORPICKER)
                    sprintf(action->param, "%d", arg);
            }
            else if (action->ev == ITU_EVENT_CHANGED)
            {
                sprintf(action->param, "%d", arg);
            }
            else if (action->ev == ITU_EVENT_ENTER || action->ev == ITU_EVENT_LEAVE)
            {
                if (arg && action->action == ITU_ACTION_FUNCTION)
                    strcpy(action->param, (char*)arg);
            }
            else if (action->ev >= ITU_EVENT_CUSTOM)
            {
                if (arg)
                    strcpy(action->param, (char*)arg);
            }
            else if (action->ev == ITU_EVENT_LOAD)
            {
                if (action->action == ITU_ACTION_RELOAD)
                {
                    if (arg)
                        strcpy(action->param, (char*)arg);
                    else
                        action->param[0] = '\0';
                }
            }
            else if (action->ev == ITU_EVENT_SYNC)
            {
                if (arg)
                    strcpy(action->param, (char*)arg);
            }
            else if (action->ev == ITU_EVENT_MOUSELONGPRESS)
            {
                if ((widget->type == ITU_SCROLLLISTBOX || widget->type == ITU_SCROLLICONLISTBOX) &&
                    (action->action == ITU_ACTION_CHECK || action->action == ITU_ACTION_FUNCTION))
                {
                    sprintf(action->param, "%d", arg);
                }
            }
            else if (action->ev == ITU_EVENT_KEYDOWN || action->ev == ITU_EVENT_KEYUP)
            {
                sprintf(action->param, "%d", arg);
            }

            if (ituScene->actionQueueLen < ITU_ACTION_QUEUE_SIZE)
            {
                ITUActionExecution* actionExec;

                if (ituScene->actionQueueLen > 0)
                {
                    int j;

                    for (j = 0; j < ituScene->actionQueueLen; j++)
                    {
                        ITUActionExecution* actionExec = &ituScene->actionQueue[j];
                        if (actionExec->widget == widget && actionExec->action == action)
                            break;   // duplicate action
                    }
                    if (j < ituScene->actionQueueLen)
                        continue;   // duplicate action
                }
                actionExec = &ituScene->actionQueue[ituScene->actionQueueLen];
                actionExec->widget = widget;
                actionExec->action = action;
                ituScene->actionQueueLen++;
                
                if (ev != ITU_EVENT_TIMER && ev != ITU_EVENT_SYNC)
                    widget->dirty = true;

                result = true;
            }
            else
            {
                LOG_ERR "Out of action queue: %d\n", ituScene->actionQueueLen LOG_END
                break;
            }
        }
    }
    return result;
}

bool ituFlushActionQueue(ITUActionExecution actionQueue[ITU_ACTION_QUEUE_SIZE], int* queueLen)
{
    int i;
    ITUWidget* skipWidget = NULL;
    bool result = false;

    for (i = 0; i < *queueLen; i++)
    {
        ITUActionExecution* actionExec = &actionQueue[i];
        ITUAction* action = actionExec->action;

        if (skipWidget && skipWidget == actionExec->widget)
            continue;

        if (action->action == ITU_ACTION_LANGUAGE && action->target[0] == '\0')
        {
            ituSceneUpdate(ituScene, ITU_EVENT_LANGUAGE, atoi(action->param), 0, 0);
            continue;
        }

        if (action->action == ITU_ACTION_FUNCTION)
        {
            bool ret;
            ITUActionFunc func;

            if (action->cachedTarget)
            {
                func = (ITUActionFunc)action->cachedTarget;
            }
            else
            {
                func = ituSceneFindFunction(ituScene, action->target);
                if (!func)
                    continue;

                action->cachedTarget = (void*)func;
            }
            ret = func(actionExec->widget, action->param);
            if (ret)
            {
                ituWidgetSetDirty(actionExec->widget, true);
                result |= ret;
            }
            else if (action->ev != ITU_EVENT_TIMER)
            {
                skipWidget = actionExec->widget;
            }

            if (action->ev == ITU_EVENT_LOAD)
                ituWidgetUpdate(actionExec->widget, ITU_EVENT_LAYOUT, 0, 0, 0);
        }
        else if (action->action >= ITU_ACTION_SET0 && action->action <= ITU_ACTION_SET7)
        {
            int index = action->action - ITU_ACTION_SET0;
            ITUVariable* var = &ituScene->variables[index];

            if (!action->cachedTarget)
            {
                ITUWidget* target = ituSceneFindWidget(ituScene, action->target);
                if (!target)
                    continue;

                action->cachedTarget = (void*)target;
            }

            strcpy(var->target, action->target);
            strcpy(var->param, action->param);
            var->cachedTarget = action->cachedTarget;
        }
        else
        {
            ITUWidget* target;
            char* param;

            if (action->target[0] == '$')
            {
                int index = atoi(&action->target[1]);
                ITUVariable* var = &ituScene->variables[index];

                if (var->cachedTarget)
                {
                    target = var->cachedTarget;
                }
                else if (var->target[0] != '\0')
                {
                    ITUWidget* widget = ituSceneFindWidget(ituScene, var->target);
                    if (!widget)
                        continue;

                    target = widget;
                    var->cachedTarget = (void*)target;
                }
                else
                    continue;
            }
            else
            {
                if (action->cachedTarget)
                {
                    target = action->cachedTarget;
                }
                else if (action->target[0] != '\0')
                {
                    ITUWidget* widget = ituSceneFindWidget(ituScene, action->target);
                    if (!widget)
                        continue;

                    target = widget;
                    action->cachedTarget = (void*)target;
                }
                else
                    continue;
            }

            if (action->param[0] == '$' && isdigit(action->param[1]))
            {
                int index = atoi(&action->param[1]);
                ITUVariable* var = &ituScene->variables[index];

                param = var->param;
            }
            else
            {
                param = action->param;
            }
            ituWidgetOnAction(target, action->action, param);
        }
    }
    *queueLen = 0;
    return result;
}


void ituExecDelayQueue(ITUActionExecution delayQueue[ITU_ACTION_QUEUE_SIZE])
{
    int i;

    for (i = 0; i < ITU_ACTION_QUEUE_SIZE; i++)
    {
        ITUActionExecution* delayExec = &delayQueue[i];

        if (delayExec->delay <= 0)
            continue;

        if (--delayExec->delay == 0 && ituWidgetIsVisible(delayExec->widget))
        {
            ITUAction* action = delayExec->action;

            if (action->action == ITU_ACTION_LANGUAGE && action->target[0] == '\0')
            {
                ituSceneUpdate(ituScene, ITU_EVENT_LANGUAGE, atoi(action->param), 0, 0);
                continue;
            }

            if (action->action == ITU_ACTION_FUNCTION)
            {
                ITUActionFunc func;

                if (action->cachedTarget)
                {
                    func = (ITUActionFunc)action->cachedTarget;
                }
                else
                {
                    func = ituSceneFindFunction(ituScene, action->target);
                    if (!func)
                        continue;

                    action->cachedTarget = (void*)func;
                }
                func(delayExec->widget, action->param);
            }
            else
            {
                ITUWidget* target;
                char* param;

                if (action->target[0] == '$')
                {
                    int index = atoi(&action->target[1]);
                    ITUVariable* var = &ituScene->variables[index];

                    if (var->cachedTarget)
                    {
                        target = var->cachedTarget;
                    }
                    else if (var->target[0] != '\0')
                    {
                        ITUWidget* widget = ituSceneFindWidget(ituScene, var->target);
                        if (!widget)
                            continue;

                        target = widget;
                        var->cachedTarget = (void*)target;
                    }
                    else
                        continue;
                }
                else
                {
                    if (action->cachedTarget)
                    {
                        target = action->cachedTarget;
                    }
                    else if (action->target[0] != '\0')
                    {
                        ITUWidget* widget = ituSceneFindWidget(ituScene, action->target);
                        if (!widget)
                            continue;

                        target = widget;
                        action->cachedTarget = (void*)target;
                    }
                    else
                        continue;
                }

                if (action->param[0] == '$' && isdigit(action->param[1]))
                {
                    int index = atoi(&action->param[1]);
                    ITUVariable* var = &ituScene->variables[index];

                    param = var->param;
                }
                else
                {
                    param = action->param;
                }
                ituWidgetOnAction(target, action->action, param);
            }
        }
    }
}

bool ituActionHasEvent(struct ITUWidgetTag* widget, ITUAction* actions, ITUEvent ev)
{
    int i;
    assert(actions);

    for (i = 0; i < ITU_ACTIONS_SIZE; i++)
    {
        ITUAction* action = &actions[i];

        if (action->ev == ev)
        {
            return true;
        }
    }
    return false;
}

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL/SDL.h"
#include "dynamic_ui.h"

static ITUContainer* dynamicUIContainer;
static ITUText* dynamicUIMessageText;

bool DynamicUIButtonOnLongPress(ITUWidget* widget, char* param)
{
    ITCTree* node;
    int i = 0;

    dynamicUIContainer->widget.flags |= (ITU_DRAGGABLE | ITU_DRAGGING);
    ituContainerUpdate(&dynamicUIContainer->widget, ITU_EVENT_MOUSEDOWN, 1, -dynamicUIContainer->widget.rect.x + dynamicUIContainer->touchX, -dynamicUIContainer->widget.rect.y + dynamicUIContainer->touchY);
    ituTextSetString(dynamicUIMessageText, "Editting");

    for (node = dynamicUIContainer->widget.tree.child; node; node = node->sibling)
    {
        ITUAnimation* child = (ITUAnimation*) node;
        ituAnimationPlay(child, i++ % 3);
    }
    return true;
}

bool DynamicUIBackgroundButtonOnPress(ITUWidget* widget, char* param)
{
    ITCTree* node;

    dynamicUIContainer->widget.flags &= ~ITU_DRAGGABLE;
    ituTextSetString(dynamicUIMessageText, NULL);

    for (node = dynamicUIContainer->widget.tree.child; node; node = node->sibling)
    {
        ITUAnimation* child = (ITUAnimation*) node;
        ituAnimationReset(child);
    }
    return true;
}

bool DynamicUITrashCanBackgroundButtonOnMouseUp(ITUWidget* widget, char* param)
{
    if (ituScene->dragged)
    {
        ITCTree* node;

        ituWidgetSetVisible(ituScene->dragged, false);
        ituWidgetUpdate(dynamicUIContainer, ITU_EVENT_LAYOUT, 0, 0, 0);
        dynamicUIContainer->widget.flags &= ~ITU_DRAGGABLE;
        ituTextSetString(dynamicUIMessageText, NULL);

        for (node = dynamicUIContainer->widget.tree.child; node; node = node->sibling)
        {
            ITUAnimation* child = (ITUAnimation*) node;
            ituAnimationReset(child);
        }
    }
    return true;
}

bool DynamicUIResetButtonOnPress(ITUWidget* widget, char* param)
{
    ITCTree* node;
    int i = 0;

    for (node = dynamicUIContainer->widget.tree.child; node; node = node->sibling)
    {
        ITUWidget* child = (ITUWidget*) node;

        ituWidgetSetVisible(child, true);
        child->tabIndex = i;
        i++;
    }
    ituWidgetUpdate(dynamicUIContainer, ITU_EVENT_LAYOUT, 0, 0, 0);

    return true;
}

bool DynamicUISaveButtonOnPress(ITUWidget* widget, char* param)
{
    ITCTree* node;
    int i = 0;

    for (node = dynamicUIContainer->widget.tree.child; node; node = node->sibling)
    {
        ITUWidget* child = (ITUWidget*) node;

        theConfig.btn_visible[i] = ituWidgetIsVisible(child);
        theConfig.btn_tabindex[i] = child->tabIndex;
        i++;
    }

    ConfigSave();
    return true;
}

bool DynamicUIOnEnter(ITUWidget* widget, char* param)
{
    if (!dynamicUIContainer)
    {
        ITCTree* node;
        int i = 0;

        dynamicUIContainer = ituSceneFindWidget(&theScene, "dynamicUIContainer");
        assert(dynamicUIContainer);

        for (node = dynamicUIContainer->widget.tree.child; node; node = node->sibling)
        {
            ITUWidget* child = (ITUWidget*) node;

            ituWidgetSetVisible(child, theConfig.btn_visible[i]);
            child->tabIndex = theConfig.btn_tabindex[i];
            i++;
        }
        ituWidgetUpdate(dynamicUIContainer, ITU_EVENT_LAYOUT, 0, 0, 0);

        dynamicUIMessageText = ituSceneFindWidget(&theScene, "dynamicUIMessageText");
        assert(dynamicUIMessageText);
    }
    return true;
}
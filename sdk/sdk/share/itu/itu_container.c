#include <assert.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char containerName[] = "ITUContainer";

bool ituContainerClone(ITUWidget* widget, ITUWidget** cloned)
{
    assert(widget);
    assert(cloned);
    ITU_ASSERT_THREAD();

    if (*cloned == NULL)
    {
        ITUWidget* newWidget = malloc(sizeof(ITUContainer));
        if (newWidget == NULL)
            return false;

        memcpy(newWidget, widget, sizeof(ITUContainer));
        newWidget->tree.child = newWidget->tree.parent = newWidget->tree.sibling = NULL;
        *cloned = newWidget;
    }

    return ituWidgetCloneImpl(widget, cloned);
}

static int ContainerCompare(const void *arg1, const void *arg2)
{
    ITUWidget* a = *(ITUWidget**)arg1;
    ITUWidget* b = *(ITUWidget**)arg2;
    return  a->tabIndex - b->tabIndex;
}

static ITUWidget* ContainerGetNearestChild(ITUContainer* container, ITUWidget* currChild)
{
    ITCTree* node;
    ITUWidget* nearestChild = NULL;
    int minDist, distX, distY;

    distX = ituScene->lastMouseX - container->touchX;
    distY = ituScene->lastMouseY - container->touchY;
    minDist = distX * distX + distY * distY;

    for (node = container->widget.tree.child; node; node = node->sibling)
    {
        int dist;
        ITUWidget* child = (ITUWidget*) node;

        if (!child->visible || child == currChild)
            continue;

        distX = child->rect.x + child->rect.width / 2 - (currChild->rect.x + currChild->rect.width / 2);
        distY = child->rect.y  + child->rect.height / 2 - (currChild->rect.y + currChild->rect.height / 2);

        dist = distX * distX + distY * distY;
        if (dist < minDist)
        {
            minDist = dist;
            nearestChild = child;
        }
    }
    return nearestChild;
}

bool ituContainerUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result;
    ITUContainer* container = (ITUContainer*) widget;
    assert(container);

    result = ituWidgetUpdateImpl(widget, ev, arg1, arg2, arg3);

    if (ev == ITU_EVENT_LAYOUT)
    {
        if (container->containerFlags & ITU_CONTAINER_GRID)
        {
            ITCTree* node;
            ITUWidget* child;
            ITUWidget* children[ITU_WIDGET_CHILD_MAX];
            ITURectangle* rect;
            int width, height, maxChildWidth, maxChildHeight, count, i;

            width = height = maxChildWidth = maxChildHeight = 0;

            count = 0;
            for (node = widget->tree.child; node; node = node->sibling)
            {
                child = (ITUWidget*) node;

                if (!child->visible)
                    continue;

                children[count++] = child;

                rect = &child->rect;

                if (maxChildWidth < rect->width)
                    maxChildWidth = rect->width;

                if (maxChildHeight < rect->height)
                    maxChildHeight = rect->height;
            }

            qsort(children, count, sizeof(ITUWidget*), ContainerCompare);

            for (i = 0; i < count; i++)
            {
                child = children[i];

                rect = &child->rect;

                if (width + maxChildWidth > widget->rect.width)
                {
                    width = 0;
                    height += maxChildHeight + container->gap;
                }
                rect->x = width + maxChildWidth / 2 - rect->width / 2;
                rect->y = height + maxChildHeight / 2 - rect->height / 2;
                width += maxChildWidth + container->gap;
            }
            result = widget->dirty = true;
        }
    }
    else if (ev == ITU_EVENT_MOUSEMOVE)
    {
        if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGING) && ituScene->dragged)
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            //if (ituWidgetIsInside(widget, x, y))
            {
                ITUWidget* child = ituScene->dragged;
                ITURectangle* rect = &child->rect;

                ituWidgetSetPosition(child, x - container->offsetX, y - container->offsetY);
            }
        }
    }
    else if (ev == ITU_EVENT_MOUSEDOWN)
    {
        if (ituWidgetIsEnabled(widget))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                ITCTree* node;
                ITUWidget* child;
                ITURectangle* rect;

                for (node = widget->tree.child; node; node = node->sibling)
                {
                    int xx, yy;

                    child = (ITUWidget*) node;

                    if (!child->visible)
                        continue;

                    rect = &child->rect;

                    xx = x - rect->x;
                    yy = y - rect->y;

                    if (ituWidgetIsInside(child, xx, yy))
                    {
                        container->touchX = x;
                        container->touchY = y;
                        container->offsetX = xx;
                        container->offsetY = yy;

                        if (widget->flags & ITU_DRAGGABLE)
                        {
                            if (widget->flags & ITU_HAS_LONG_PRESS)
                            {
                                container->touchCount = 1;
                            }
                            else
                            {
                                widget->flags |= ITU_DRAGGING;
                                ituScene->dragged = child;
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
    else if (ev == ITU_EVENT_MOUSEUP)
    {
        if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGABLE) && (widget->flags & ITU_DRAGGING))
        {
            ITUWidget* nearestChild = ContainerGetNearestChild(container, ituScene->dragged);
            if (nearestChild)
            {
                ITCTree* node;
                ITUWidget* children[ITU_WIDGET_CHILD_MAX];
                int count, i, nearestChildIndex, currChildIndex, tabIndex;

                count = 0;
                for (node = widget->tree.child; node; node = node->sibling)
                {
                    ITUWidget* child = (ITUWidget*) node;

                    if (!child->visible)
                        continue;

                    children[count++] = child;
                }
                qsort(children, count, sizeof(ITUWidget*), ContainerCompare);

                nearestChildIndex = currChildIndex = -1;
                for (i = 0; i < count; i++)
                {
                    if (children[i] == nearestChild)
                    {
                        nearestChildIndex = i;
                    }
                    else if (children[i] == ituScene->dragged)
                    {
                        currChildIndex = i;
                    }
                    if (nearestChildIndex != -1 && currChildIndex != -1)
                        break;
                }

                tabIndex = ituScene->dragged->tabIndex;
                ituScene->dragged->tabIndex = nearestChild->tabIndex;

                if (currChildIndex > nearestChildIndex)
                {
                    for (i = currChildIndex - 1; i >= nearestChildIndex; i--)
                    {
                        int tmp = children[i]->tabIndex;
                        children[i]->tabIndex = tabIndex;
                        tabIndex = tmp;
                    }
                }
                else
                {
                    for (i = currChildIndex + 1; i <= nearestChildIndex; i++)
                    {
                        int tmp = children[i]->tabIndex;
                        children[i]->tabIndex = tabIndex;
                        tabIndex = tmp;
                    }
                }
            }
            ituWidgetUpdate(container, ITU_EVENT_LAYOUT, 0, 0, 0);
        }
        widget->flags &= ~ITU_DRAGGING;
        container->touchCount = 0;
    }
    else if (ev == ITU_EVENT_TIMER)
    {
        if (container->touchCount > 0)
        {
            int x, y, dist, distX, distY;

            assert(widget->flags & ITU_HAS_LONG_PRESS);

            ituWidgetGetGlobalPosition(widget, &x, &y);

            distX = ituScene->lastMouseX - (x + container->touchX);
            distY = ituScene->lastMouseY - (y + container->touchY);

            dist = distX * distX + distY * distY;

            if (dist >= ITU_DRAG_DISTANCE * ITU_DRAG_DISTANCE)
            {
                widget->flags |= ITU_DRAGGING;
                ituScene->dragged = widget;
                container->touchCount = 0;
            }
        }
    }
    return widget->visible ? result : false;
}

void ituContainerDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    int destx, desty;
    uint8_t desta;
    ITUContainer* container = (ITUContainer*) widget;
    ITURectangle* rect = &widget->rect;
    ITCTree* node;
    ITURectangle prevClip;
    assert(container);
    assert(dest);

    if (!widget->visible)
        return;

    destx = rect->x + x;
    desty = rect->y + y;
    desta = alpha * widget->alpha / 255;

    if (widget->angle == 0)
    {
        for (node = widget->tree.child; node; node = node->sibling)
        {
            ITUWidget* child = (ITUWidget*)node;

            if (child != ituScene->dragged)
                ituWidgetSetClipping(widget, dest, x, y, &prevClip);

            if (child->visible && ituWidgetIsOverlapClipping(child, dest, destx, desty))
                ituWidgetDraw(node, dest, destx, desty, desta);

            if (child != ituScene->dragged)
                ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);

            child->dirty = false;
        }
    }
    else
    {
        ITUSurface* surf;

        surf = ituCreateSurface(rect->width, rect->height, 0, ITU_ARGB8888, NULL, 0);
        if (surf)
        {
            ITUColor color = { 0, 0, 0, 0 };

            ituColorFill(surf, 0, 0, rect->width, rect->height, &color);

            for (node = widget->tree.child; node; node = node->sibling)
            {
                ITUWidget* child = (ITUWidget*)node;

                if (child->visible && ituWidgetIsOverlapClipping(child, dest, destx, desty))
                    ituWidgetDraw(node, surf, 0, 0, desta);

                child->dirty = false;
            }

        #if (CFG_CHIP_FAMILY == 9070)
            ituRotate(dest, destx + rect->width / 2, desty + rect->height / 2, surf, surf->width / 2, surf->height / 2, (float)widget->angle, 1.0f, 1.0f);
        #else
            ituRotate(dest, destx, desty, surf, surf->width / 2, surf->height / 2, (float)widget->angle, 1.0f, 1.0f);
        #endif
            ituDestroySurface(surf);
        }
    }
}

void ituContainerOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    ITUContainer* container = (ITUContainer*) widget;
    assert(container);

    switch (action)
    {
    case ITU_ACTION_RELOAD:
        ituContainerUpdate(widget, ITU_EVENT_LAYOUT, 0, 0, 0);
        break;

    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

void ituContainerInit(ITUContainer* container)
{
    assert(container);
    ITU_ASSERT_THREAD();

    memset(container, 0, sizeof (ITUContainer));

    ituWidgetInit(&container->widget);

    ituWidgetSetType(container, ITU_CONTAINER);
    ituWidgetSetName(container, containerName);
    ituWidgetSetClone(container, ituContainerClone);
    ituWidgetSetUpdate(container, ituContainerUpdate);
    ituWidgetSetDraw(container, ituContainerDraw);
    ituWidgetSetOnAction(container, ituContainerOnAction);
}

void ituContainerLoad(ITUContainer* container, uint32_t base)
{
    assert(container);

    ituWidgetLoad(&container->widget, base);
    ituWidgetSetClone(container, ituContainerClone);
    ituWidgetSetUpdate(container, ituContainerUpdate);
    ituWidgetSetDraw(container, ituContainerDraw);
    ituWidgetSetOnAction(container, ituContainerOnAction);
}

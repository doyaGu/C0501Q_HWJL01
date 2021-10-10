#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"

static const char scrolllistboxName[] = "ITUScrollListBox";

bool ituScrollListBoxUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUListBox* listbox = (ITUListBox*) widget;
    ITUScrollListBox* scrolllistbox = (ITUScrollListBox*) widget;
    assert(scrolllistbox);

    if (ev == ITU_EVENT_LAYOUT)
    {
        int i, count = itcTreeGetChildCount(scrolllistbox);

        for (i = 0; i < count; ++i)
        {
            ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(scrolllistbox, i);
            int fy = 0 - child->rect.height * count / 3;
            fy += i * child->rect.height;
            ituWidgetSetY(child, fy);
        }

        if (!listbox->pageIndexText && (listbox->pageIndexName[0] != '\0'))
        {
            listbox->pageIndexText = (ITUText*) ituSceneFindWidget(ituScene, listbox->pageIndexName);
        }

        if (listbox->pageIndexText)
        {
            char buf[8]; //Bless
            printf("[pageIndex is %d, pageCount is %d]\n",listbox->pageIndex,listbox->pageCount);
            sprintf(buf, "%i", listbox->pageIndex);
            ituTextSetString(listbox->pageIndexText, buf);
        }

        if (!listbox->pageCountText && listbox->pageCountName[0] != '\0')
        {
            listbox->pageCountText = (ITUText*) ituSceneFindWidget(ituScene, listbox->pageCountName);
        }

        if (listbox->pageCountText)
        {
            char buf[8];
            sprintf(buf, "%i", listbox->pageCount);
            ituTextSetString(listbox->pageCountText, buf);
        }
        scrolllistbox->preindex = -1;
        result = true;
    }
    else if (ev == ITU_EVENT_MOUSEMOVE)
    {
        if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGING))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;
            int offset = y - scrolllistbox->touchY;
            int absoffset = offset > 0 ? offset : -offset;

            //printf("0: offset=%d y=%d\n", offset, y);

            if ((absoffset <= widget->rect.height / 2) ||
                (listbox->pageIndex > 1 && offset > 0) || 
                (listbox->pageIndex < listbox->pageCount && offset < 0))
            {
                int i, count = itcTreeGetChildCount(widget);

                for (i = 0; i < count; ++i)
                {
                    ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(widget, i);
                    int fy = 0 - child->rect.height * count / 3;
                    fy += i * child->rect.height;
                    fy += offset;
                    ituWidgetSetY(child, fy);
                }
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
                ITCTree* node = ituScrollListBoxGetCurrPageItem(scrolllistbox);
                int count, i = 0;

                scrolllistbox->preindex = -1;
                count = listbox->itemCount;

                for (; node; node = node->sibling)
                {
                    ITUWidget* item = (ITUWidget*) node;
                    int x1, y1;

                    if (i >= count)
                        break;

                    x1 = x - item->rect.x;
                    y1 = y - item->rect.y;

                    if (ituWidgetIsInside(item, x1, y1))
                    {
                        scrolllistbox->preindex = count + i;
                        break;
                    }
                    i++;
                }

                if (widget->flags & ITU_DRAGGABLE)
                {
                    scrolllistbox->touchY = y;

                    if (widget->flags & ITU_HAS_LONG_PRESS)
                    {
                        scrolllistbox->touchCount = 1;
                    }
                    else
                    {
                        widget->flags |= ITU_DRAGGING;
                        ituScene->dragged = widget;
                        ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_SYNC, (int)widget->name);
                    }
                }
                result = true;
            }
        }
    }
    else if (ev == ITU_EVENT_MOUSEUP || ev == ITU_EVENT_MOUSEDOUBLECLICK)
    {
        if (ituWidgetIsEnabled(widget))
        {
            int dist;
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if ((widget->flags & ITU_DRAGGABLE) && (widget->flags & ITU_DRAGGING))
            {
                int offset = y - scrolllistbox->touchY;
                int absoffset = offset > 0 ? offset : -offset;

                if ((absoffset > widget->rect.height / 2) &&
                    ((listbox->pageIndex > 1 && offset > 0) ||
                    (listbox->pageIndex < listbox->pageCount && offset < 0)))
                {
                    scrolllistbox->frame = absoffset / (widget->rect.height / scrolllistbox->totalframe) + 1;

                    if (offset == 0)
                    {
                        scrolllistbox->inc = 0;
                    }
                    else if (offset > 0)
                    {
                        scrolllistbox->inc = widget->rect.height / scrolllistbox->totalframe;
                        //printf("1: frame=%d offset=%d inc=%d pageIndex=%d\n", scrolllistbox->frame, offset, scrolllistbox->inc, listbox->pageIndex);
                    }
                    else
                    {
                        scrolllistbox->inc = -widget->rect.height / scrolllistbox->totalframe;
                        //printf("2: frame=%d offset=%d inc=%d pageIndex=%d\n", scrolllistbox->frame, offset, scrolllistbox->inc, listbox->pageIndex);
                    }
                    widget->flags |= ITU_PROGRESS;
                }
                else
                {
                    scrolllistbox->frame = scrolllistbox->totalframe - absoffset / (widget->rect.height / scrolllistbox->totalframe);

                    if (offset == 0)
                    {
                        scrolllistbox->inc = 0;
                    }
                    else if (offset > 0)
                    {
                        scrolllistbox->inc = -widget->rect.height / scrolllistbox->totalframe;
                        //printf("3: frame=%d offset=%d inc=%d pageIndex=%d\n", scrolllistbox->frame, offset, scrolllistbox->inc, listbox->pageIndex);
                    }
                    else
                    {
                        scrolllistbox->inc = widget->rect.height / scrolllistbox->totalframe;
                        //printf("4: frame=%d offset=%d inc=%d pageIndex=%d\n", scrolllistbox->frame, offset, scrolllistbox->inc, listbox->pageIndex);
                    }
                }
                widget->flags |= ITU_UNDRAGGING;
                ituScene->dragged = NULL;
                widget->flags &= ~ITU_DRAGGING;
                ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_SYNC, (int)widget->name);
                result = true;
            }
            
            dist = y - scrolllistbox->touchY;
            if (dist < 0)
                dist = -dist;

            if (ituWidgetIsInside(widget, x, y) && ((dist < ITU_DRAG_DISTANCE) || !(widget->flags & ITU_DRAGGABLE)))
            {
                ITCTree* node = ituScrollListBoxGetCurrPageItem(scrolllistbox);
                int count, i = 0;

                count = listbox->itemCount;

                for (; node; node = node->sibling)
                {
                    ITUWidget* item = (ITUWidget*) node;
                    int x1, y1;

                    if (i >= count)
                        break;

                    x1 = x - item->rect.x;
                    y1 = y - item->rect.y;

                    if (ituWidgetIsInside(item, x1, y1) && scrolllistbox->preindex == count + i)
                    {
                        if (ev == ITU_EVENT_MOUSEUP)
                        {
                            if (ituScene->focused != widget)
                            {
                                ituFocusWidget(widget);
                            }
                            if (!(widget->flags & ITU_LONG_PRESSING))
                            {
                                ituListBoxSelect(listbox, i);
                                ituExecActions(widget, listbox->actions, ITU_EVENT_SELECT, i);
                            }
                        }
                        else
                        {
                            ituListBoxOnSelection(listbox, (ITUScrollText*)item, true);
                        }
                        break;
                    }
                    i++;
                }
                scrolllistbox->preindex = -1;
                result = true;
            }
            scrolllistbox->touchCount = 0;
            widget->flags &= ~ITU_LONG_PRESSING;
        }
    }
    else if (ev == ITU_EVENT_MOUSELONGPRESS)
    {
        if (ituWidgetIsEnabled(widget))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                ITCTree* node = ituScrollListBoxGetCurrPageItem(scrolllistbox);
                int count, i = 0;

                count = listbox->itemCount;

                for (; node; node = node->sibling)
                {
                    ITUWidget* item = (ITUWidget*) node;
                    int x1, y1;

                    if (i >= count)
                        break;

                    x1 = x - item->rect.x;
                    y1 = y - item->rect.y;

                    if (ituWidgetIsInside(item, x1, y1) && scrolllistbox->preindex == count + i)
                    {
                        ituListBoxCheck(listbox, i);
                        ituExecActions(widget, listbox->actions, ev, i);
                        result |= widget->dirty;
                        break;
                    }
                    i++;
                }
                widget->flags |= ITU_LONG_PRESSING;
            }
        }
        scrolllistbox->touchCount = 0;
    }
    else if (ev == ITU_EVENT_TOUCHSLIDEUP || ev == ITU_EVENT_TOUCHSLIDEDOWN)
    {
        scrolllistbox->touchCount = 0;

        if (ituWidgetIsEnabled(widget))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                if (ev == ITU_EVENT_TOUCHSLIDEUP)
                {
                    int i, count = itcTreeGetChildCount(scrolllistbox);

                    if (widget->flags & ITU_DRAGGING)
                    {
                        widget->flags &= ~ITU_DRAGGING;
                        ituScene->dragged = NULL;
                        scrolllistbox->inc = 0;
                    }

                    if (listbox->pageIndex < listbox->pageCount)
                    {
                        for (i = 0; i < count; ++i)
                        {
                            ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(scrolllistbox, i);
                            int fy = 0 - child->rect.height * count / 3;
                            fy += i * child->rect.height;

                            if (scrolllistbox->inc == 0)
                                scrolllistbox->inc = 0 - widget->rect.height;

                            fy += scrolllistbox->inc;
                            ituWidgetSetY(child, fy);
                        }
                    }
                    else
                    {
                        for (i = 0; i < count; ++i)
                        {
                            ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(scrolllistbox, i);
                            int fy = 0 - child->rect.height * count / 3;
                            fy += i * child->rect.height;

                            if (scrolllistbox->inc == 0)
                                scrolllistbox->inc = 0 - widget->rect.height / 5;

                            fy += scrolllistbox->inc;
                            ituWidgetSetY(child, fy);
                        }
                        widget->flags |= ITU_BOUNCING;
                    }
                    scrolllistbox->frame = 1;
                    ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_TOUCHSLIDEUP, 0);
                }
                else // if (ev == ITU_EVENT_TOUCHSLIDEDOWN)
                {
                    int i, count = itcTreeGetChildCount(scrolllistbox);

                    if (widget->flags & ITU_DRAGGING)
                    {
                        widget->flags &= ~ITU_DRAGGING;
                        ituScene->dragged = NULL;
                        scrolllistbox->inc = 0;
                    }

                    if (listbox->pageIndex > 1)
                    {
                        for (i = 0; i < count; ++i)
                        {
                            ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(scrolllistbox, i);
                            int fy = 0 - child->rect.height * count / 3;
                            fy += i * child->rect.height;

                            if (scrolllistbox->inc == 0)
                                scrolllistbox->inc = widget->rect.height;

                            fy += scrolllistbox->inc;
                            ituWidgetSetY(child, fy);
                        }
                    }
                    else
                    {
                        for (i = 0; i < count; ++i)
                        {
                            ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(scrolllistbox, i);
                            int fy = 0 - child->rect.height * count / 3;
                            fy += i * child->rect.height;

                            if (scrolllistbox->inc == 0)
                                scrolllistbox->inc = widget->rect.height / 5;

                            fy += scrolllistbox->inc;
                            ituWidgetSetY(child, fy);
                        }
                        widget->flags |= ITU_BOUNCING;
                    }
                    scrolllistbox->frame = 1;
                    ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_TOUCHSLIDEDOWN, 0);
                }
                ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_SYNC, (int)widget->name);
                result = true;
            }
        }
    }
    else if (ev == ITU_EVENT_TIMER)
    {
        if (scrolllistbox->touchCount > 0)
        {
            int y, dist;

            assert(widget->flags & ITU_HAS_LONG_PRESS);

            ituWidgetGetGlobalPosition(widget, NULL, &y);

            dist = ituScene->lastMouseY - (y + scrolllistbox->touchY);

            if (dist < 0)
                dist = -dist;

            if (dist >= ITU_DRAG_DISTANCE)
            {
                widget->flags |= ITU_DRAGGING;
                ituScene->dragged = widget;
                scrolllistbox->touchCount = 0;
                ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_SYNC, (int)widget->name);
            }
        }

        if (widget->flags & ITU_UNDRAGGING)
        {
            int i, count = itcTreeGetChildCount(scrolllistbox);

            for (i = 0; i < count; ++i)
            {
                ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(scrolllistbox, i);
                int fy = 0 - child->rect.height * count / 3;
                
                if ((widget->flags & ITU_PROGRESS) == 0)
                {
                    if (scrolllistbox->inc > 0)
                        fy -= widget->rect.height;
                    else if (scrolllistbox->inc < 0)
                        fy += widget->rect.height;
                }

                fy += i * child->rect.height;
                fy += scrolllistbox->inc * scrolllistbox->frame;

                ituWidgetSetY(child, fy);
            }
            scrolllistbox->frame++;

            if (scrolllistbox->frame > scrolllistbox->totalframe)
            {
                for (i = 0; i < count; ++i)
                {
                    ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(scrolllistbox, i);
                    int fy = 0 - child->rect.height * count / 3;
                    fy += i * child->rect.height;

                    ituWidgetSetY(child, fy);
                }

                if (widget->flags & ITU_PROGRESS)
                {
                    char buf[32];

                    if (scrolllistbox->inc > 0)
                    {
                        listbox->pageIndex--;
                    }
                    else if (scrolllistbox->inc < 0)
                    {
                        listbox->pageIndex++;
                    }
                    sprintf(buf, "%d %d", listbox->pageIndex, listbox->pageCount);
                    ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_LOAD, (int)buf);
                    ituListBoxSelect(listbox, -1);
                }

                scrolllistbox->frame = 0;
                scrolllistbox->inc = 0;
                widget->flags &= ~ITU_UNDRAGGING;
                widget->flags &= ~ITU_PROGRESS;
            }
            result = true;
        }
        else if (widget->flags & ITU_BOUNCING)
        {
            float step;
            int i, count = itcTreeGetChildCount(scrolllistbox);

            if (scrolllistbox->frame <= scrolllistbox->totalframe / 2)
                step = (float)scrolllistbox->frame;
            else
                step = (float)scrolllistbox->totalframe - scrolllistbox->frame;

            step /= scrolllistbox->totalframe / 2;
            step = (float)M_PI / 2 - step * (float)M_PI / 2;
            step = 1.0f - sinf(step);

            //printf("step=%f\n", step);

            for (i = 0; i < count; ++i)
            {
                ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(scrolllistbox, i);
                int fy = 0 - child->rect.height * count / 3;
                fy += i * child->rect.height;
                fy += (int)(scrolllistbox->inc * step);

                ituWidgetSetY(child, fy);
            }

            scrolllistbox->frame++;

            if (scrolllistbox->frame > scrolllistbox->totalframe)
            {
                scrolllistbox->frame = 0;
                scrolllistbox->inc = 0;
                widget->flags &= ~ITU_BOUNCING;
            }
            result = true;
        }
        else if (scrolllistbox->inc)
        {
            int i, count = itcTreeGetChildCount(scrolllistbox);
            float step = (float)scrolllistbox->frame / scrolllistbox->totalframe;
            step = step * (float)M_PI / 2;
            step = sinf(step);

            //printf("step=%f\n", step);

            for (i = 0; i < count; ++i)
            {
                ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(scrolllistbox, i);
                int fy = 0 - child->rect.height * count / 3;
                fy += i * child->rect.height;
                fy += (int)(scrolllistbox->inc * step);

                ituWidgetSetY(child, fy);
            }
            scrolllistbox->frame++;

            if (scrolllistbox->frame > scrolllistbox->totalframe)
            {
                char buf[32];

                if (scrolllistbox->inc > 0)
                {
                    listbox->pageIndex--;
                }
                else //if (scrolllistbox->inc < 0)
                {
                    listbox->pageIndex++;
                }

                scrolllistbox->frame = 0;
                scrolllistbox->inc = 0;

                sprintf(buf, "%d %d", listbox->pageIndex, listbox->pageCount);
                ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_LOAD, (int)buf);
                ituListBoxSelect(listbox, -1);
            }
            result = true;
        }
    }
    
    if (!result)
    {
        result = ituListBoxUpdate(widget, ev, arg1, arg2, arg3);
    }
    result |= widget->dirty;
    return widget->visible ? result : false;
}

static void ScrollListBoxPrevPage(ITUScrollListBox* scrolllistbox)
{
    ITUListBox* listbox = (ITUListBox*) scrolllistbox;

    if (listbox->pageIndex < listbox->pageCount)
    {
        int i, count = itcTreeGetChildCount(scrolllistbox);

        for (i = 0; i < count; ++i)
        {
            ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(scrolllistbox, i);
            int fy = 0 - child->rect.height * count / 3;
            fy += i * child->rect.height;

            if (scrolllistbox->inc == 0)
                scrolllistbox->inc = 0 - (child->rect.height * count / 3) / scrolllistbox->totalframe;

            fy += scrolllistbox->inc;
            ituWidgetSetY(child, fy);
        }
        scrolllistbox->frame = 1;
    }
}

static void ScrollListBoxNextPage(ITUScrollListBox* scrolllistbox)
{
    ITUListBox* listbox = (ITUListBox*) scrolllistbox;

    if (listbox->pageIndex > 1)
    {
        int i, count = itcTreeGetChildCount(scrolllistbox);

        for (i = 0; i < count; ++i)
        {
            ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(scrolllistbox, i);
            int fy = 0 - child->rect.height * count / 3;
            fy += i * child->rect.height;

            if (scrolllistbox->inc == 0)
                scrolllistbox->inc = (child->rect.height * count / 3) / scrolllistbox->totalframe;

            fy += scrolllistbox->inc;
            ituWidgetSetY(child, fy);
        }
        scrolllistbox->frame = 1;
    }
}

static void ScrollListBoxBind(ITUScrollListBox* scrolllistbox, char* param)
{
    ITUWidget* widget = (ITUWidget*) scrolllistbox;
    ITUWidget* target = ituSceneFindWidget(ituScene, param);
    if (target)
    {
        if (target->flags & ITU_PROGRESS)
            widget->flags |= ITU_PROGRESS;
        else
            widget->flags &= ~ITU_PROGRESS;

        if (target->flags & ITU_DRAGGING)
            widget->flags |= ITU_DRAGGING;
        else
            widget->flags &= ~ITU_DRAGGING;

        if (target->flags & ITU_UNDRAGGING)
            widget->flags |= ITU_UNDRAGGING;
        else
            widget->flags &= ~ITU_UNDRAGGING;

        if (target->flags & ITU_BOUNCING)
            widget->flags |= ITU_BOUNCING;
        else
            widget->flags &= ~ITU_BOUNCING;

        if (target->type == ITU_SCROLLLISTBOX)
        {
            ITUScrollListBox* targetslistbox = (ITUScrollListBox*) target;
            scrolllistbox->inc = targetslistbox->inc;
            scrolllistbox->frame = targetslistbox->frame;
            scrolllistbox->touchY = targetslistbox->touchY;
        }
        else if (target->type == ITU_SCROLLMEDIAFILELISTBOX)
        {
            ITUScrollMediaFileListBox* targetsmflistbox = (ITUScrollMediaFileListBox*) target;
            scrolllistbox->inc = targetsmflistbox->inc;
            scrolllistbox->frame = targetsmflistbox->frame;
            scrolllistbox->touchY = targetsmflistbox->touchY;
        }
        else if (target->type == ITU_SCROLLICONLISTBOX)
        {
            ITUScrollIconListBox* targetsilistbox = (ITUScrollIconListBox*) target;
            scrolllistbox->inc = targetsilistbox->inc;
            scrolllistbox->frame = targetsilistbox->frame;
            scrolllistbox->touchY = targetsilistbox->touchY;
        }
    }
}

void ituScrollListBoxOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    assert(widget);

    switch (action)
    {
    case ITU_ACTION_PREV:
        ScrollListBoxPrevPage((ITUScrollListBox*)widget);
        break;

    case ITU_ACTION_NEXT:
        ScrollListBoxNextPage((ITUScrollListBox*)widget);
        break;

    case ITU_ACTION_BIND:
        ScrollListBoxBind((ITUScrollListBox*)widget, param);
        break;

    default:
        ituListBoxOnAction(widget, action, param);
        break;
    }
}

void ituScrollListBoxInit(ITUScrollListBox* scrolllistbox, int width)
{
    assert(scrolllistbox);
    ITU_ASSERT_THREAD();

    memset(scrolllistbox, 0, sizeof (ITUScrollListBox));

    ituListBoxInit(&scrolllistbox->listbox, width);

    ituWidgetSetType(scrolllistbox, ITU_SCROLLLISTBOX);
    ituWidgetSetName(scrolllistbox, scrolllistboxName);
    ituWidgetSetUpdate(scrolllistbox, ituScrollListBoxUpdate);
    ituWidgetSetOnAction(scrolllistbox, ituScrollListBoxOnAction);
}

void ituScrollListBoxLoad(ITUScrollListBox* scrolllistbox, uint32_t base)
{
    assert(scrolllistbox);

    ituListBoxLoad(&scrolllistbox->listbox, base);

    ituWidgetSetUpdate(scrolllistbox, ituScrollListBoxUpdate);
    ituWidgetSetOnAction(scrolllistbox, ituScrollListBoxOnAction);
}

int ituScrollListBoxGetItemCount(ITUScrollListBox* scrolllistbox)
{
    ITU_ASSERT_THREAD();
    return itcTreeGetChildCount(scrolllistbox) / 3;
}

ITCTree* ituScrollListBoxGetLastPageItem(ITUScrollListBox* scrolllistbox)
{
    ITU_ASSERT_THREAD();
    return itcTreeGetChildAt(scrolllistbox, 0);
}

ITCTree* ituScrollListBoxGetCurrPageItem(ITUScrollListBox* scrolllistbox)
{
    ITU_ASSERT_THREAD();
    return itcTreeGetChildAt(scrolllistbox, itcTreeGetChildCount(scrolllistbox) / 3);
}

ITCTree* ituScrollListBoxGetNextPageItem(ITUScrollListBox* scrolllistbox)
{
    ITU_ASSERT_THREAD();
    return itcTreeGetChildAt(scrolllistbox, itcTreeGetChildCount(scrolllistbox) * 2 / 3);
}

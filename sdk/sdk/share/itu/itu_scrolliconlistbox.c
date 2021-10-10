#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"

static const char scrolliconlistboxName[] = "ITUScrollIconListBox";

bool ituScrollIconListBoxUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUListBox* listbox = (ITUListBox*) widget;
    ITUScrollIconListBox* scrolliconlistbox = (ITUScrollIconListBox*) widget;
    assert(scrolliconlistbox);

    if (ev == ITU_EVENT_LAYOUT)
    {
        int i, count = itcTreeGetChildCount(scrolliconlistbox);

        for (i = 0; i < count; ++i)
        {
            ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(scrolliconlistbox, i);
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
            char buf[8];
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
        scrolliconlistbox->preindex = -1;
        result = true;
    }
    else if (ev == ITU_EVENT_MOUSEMOVE)
    {
        if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGING))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;
            int offset = y - scrolliconlistbox->touchY;
            int absoffset = offset > 0 ? offset : -offset;

            //printf("0: offset=%d\n", offset);

            if ((absoffset <= widget->rect.height / 2) ||
                ((listbox->pageIndex > 1 && offset > 0) || 
                (listbox->pageIndex < listbox->pageCount && offset < 0)))
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
                ITCTree* node = ituScrollIconListBoxGetCurrPageItem(scrolliconlistbox);
                int count, i = 0;

                scrolliconlistbox->preindex = -1;
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
                        scrolliconlistbox->preindex = count + i;
                        break;
                    }
                    i++;
                }
                if (widget->flags & ITU_DRAGGABLE)
                {
                    scrolliconlistbox->touchY = y;

                    if (widget->flags & ITU_HAS_LONG_PRESS)
                    {
                        scrolliconlistbox->touchCount = 1;
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
                int offset = y - scrolliconlistbox->touchY;
                int absoffset = offset > 0 ? offset : -offset;

                if ((absoffset > widget->rect.height / 2) &&
                    ((listbox->pageIndex > 1 && offset > 0) ||
                    (listbox->pageIndex < listbox->pageCount && offset < 0)))
                {
                    scrolliconlistbox->frame = absoffset / (widget->rect.height / scrolliconlistbox->totalframe) + 1;

                    if (offset == 0)
                    {
                        scrolliconlistbox->inc = 0;
                    }
                    else if (offset > 0)
                    {
                        scrolliconlistbox->inc = widget->rect.height / scrolliconlistbox->totalframe;
                        //printf("1: frame=%d offset=%d inc=%d pageIndex=%d\n", scrolliconlistbox->frame, offset, scrolliconlistbox->inc, listbox->pageIndex);
                    }
                    else
                    {
                        scrolliconlistbox->inc = -widget->rect.height / scrolliconlistbox->totalframe;
                        //printf("2: frame=%d offset=%d inc=%d pageIndex=%d\n", scrolliconlistbox->frame, offset, scrolliconlistbox->inc, listbox->pageIndex);
                    }
                    widget->flags |= ITU_PROGRESS;
                }
                else
                {
                    scrolliconlistbox->frame = scrolliconlistbox->totalframe - absoffset / (widget->rect.height / scrolliconlistbox->totalframe);

                    if (offset == 0)
                    {
                        scrolliconlistbox->inc = 0;
                    }
                    else if (offset > 0)
                    {
                        scrolliconlistbox->inc = -widget->rect.height / scrolliconlistbox->totalframe;
                        //printf("3: frame=%d offset=%d inc=%d pageIndex=%d\n", scrolliconlistbox->frame, offset, scrolliconlistbox->inc, listbox->pageIndex);
                    }
                    else
                    {
                        scrolliconlistbox->inc = widget->rect.height / scrolliconlistbox->totalframe;
                        //printf("4: frame=%d offset=%d inc=%d pageIndex=%d\n", scrolliconlistbox->frame, offset, scrolliconlistbox->inc, listbox->pageIndex);
                    }
                }
                widget->flags |= ITU_UNDRAGGING;
                ituScene->dragged = NULL;
                widget->flags &= ~ITU_DRAGGING;
                ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_SYNC, (int)widget->name);
                result = true;
            }

            dist = y - scrolliconlistbox->touchY;
            if (dist < 0)
                dist = -dist;

            if (ituWidgetIsInside(widget, x, y) && ((dist < ITU_DRAG_DISTANCE) || !(widget->flags & ITU_DRAGGABLE)))
            {
                ITCTree* node = ituScrollIconListBoxGetCurrPageItem(scrolliconlistbox);
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

                    if (ituWidgetIsInside(item, x1, y1) && scrolliconlistbox->preindex == count + i)
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
                scrolliconlistbox->preindex = -1;
                result = true;
            }
            scrolliconlistbox->touchCount = 0;
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
                ITCTree* node = ituScrollIconListBoxGetCurrPageItem(scrolliconlistbox);
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

                    if (ituWidgetIsInside(item, x1, y1) && scrolliconlistbox->preindex == count + i)
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
        scrolliconlistbox->touchCount = 0;
    }
    else if (ev == ITU_EVENT_TOUCHSLIDEUP || ev == ITU_EVENT_TOUCHSLIDEDOWN)
    {
        scrolliconlistbox->touchCount = 0;

        if (ituWidgetIsEnabled(widget) && !scrolliconlistbox->inc)
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                if (ev == ITU_EVENT_TOUCHSLIDEUP)
                {
                    int i, count = itcTreeGetChildCount(scrolliconlistbox);

                    if (widget->flags & ITU_DRAGGING)
                    {
                        widget->flags &= ~ITU_DRAGGING;
                        ituScene->dragged = NULL;
                        scrolliconlistbox->inc = 0;
                    }

                    if (listbox->pageIndex < listbox->pageCount)
                    {
                        for (i = 0; i < count; ++i)
                        {
                            ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(scrolliconlistbox, i);
                            int fy = 0 - child->rect.height * count / 3;
                            fy += i * child->rect.height;

                            if (scrolliconlistbox->inc == 0)
                                scrolliconlistbox->inc = 0 - widget->rect.height;

                            fy += scrolliconlistbox->inc;
                            ituWidgetSetY(child, fy);
                        }
                    }
                    else
                    {
                        for (i = 0; i < count; ++i)
                        {
                            ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(scrolliconlistbox, i);
                            int fy = 0 - child->rect.height * count / 3;
                            fy += i * child->rect.height;

                            if (scrolliconlistbox->inc == 0)
                                scrolliconlistbox->inc = 0 - widget->rect.height / 5;

                            fy += scrolliconlistbox->inc;
                            ituWidgetSetY(child, fy);
                        }
                        widget->flags |= ITU_BOUNCING;
                    }
                    scrolliconlistbox->frame = 1;
                    ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_TOUCHSLIDEUP, 0);

                }
                else // if (ev == ITU_EVENT_TOUCHSLIDEDOWN)
                {
                    int i, count = itcTreeGetChildCount(scrolliconlistbox);

                    if (widget->flags & ITU_DRAGGING)
                    {
                        widget->flags &= ~ITU_DRAGGING;
                        ituScene->dragged = NULL;
                        scrolliconlistbox->inc = 0;
                    }

                    if (listbox->pageIndex > 1)
                    {
                        for (i = 0; i < count; ++i)
                        {
                            ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(scrolliconlistbox, i);
                            int fy = 0 - child->rect.height * count / 3;
                            fy += i * child->rect.height;

                            if (scrolliconlistbox->inc == 0)
                                scrolliconlistbox->inc = widget->rect.height;

                            fy += scrolliconlistbox->inc;
                            ituWidgetSetY(child, fy);
                        }
                    }
                    else
                    {
                        for (i = 0; i < count; ++i)
                        {
                            ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(scrolliconlistbox, i);
                            int fy = 0 - child->rect.height * count / 3;
                            fy += i * child->rect.height;

                            if (scrolliconlistbox->inc == 0)
                                scrolliconlistbox->inc = widget->rect.height / 5;

                            fy += scrolliconlistbox->inc;
                            ituWidgetSetY(child, fy);
                        }
                        widget->flags |= ITU_BOUNCING;
                    }
                    scrolliconlistbox->frame = 1;
                    ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_TOUCHSLIDEDOWN, 0);
                }
                ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_SYNC, (int)widget->name);
                result = true;
            }
        }
    }
    else if (ev == ITU_EVENT_TIMER)
    {
        if (scrolliconlistbox->touchCount > 0)
        {
            int y, dist;

            assert(widget->flags & ITU_HAS_LONG_PRESS);

            ituWidgetGetGlobalPosition(widget, NULL, &y);
            dist = ituScene->lastMouseY - (y + scrolliconlistbox->touchY);

            if (dist < 0)
                dist = -dist;

            if (dist >= ITU_DRAG_DISTANCE)
            {
                widget->flags |= ITU_DRAGGING;
                ituScene->dragged = widget;
                scrolliconlistbox->touchCount = 0;
                ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_SYNC, (int)widget->name);
            }
        }

        if (widget->flags & ITU_UNDRAGGING)
        {
            int i, count = itcTreeGetChildCount(scrolliconlistbox);

            for (i = 0; i < count; ++i)
            {
                ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(scrolliconlistbox, i);
                int fy = 0 - child->rect.height * count / 3;
                
                if ((widget->flags & ITU_PROGRESS) == 0)
                {
                    if (scrolliconlistbox->inc > 0)
                        fy -= widget->rect.height;
                    else if (scrolliconlistbox->inc < 0)
                        fy += widget->rect.height;
                }

                fy += i * child->rect.height;
                fy += scrolliconlistbox->inc * scrolliconlistbox->frame;

                ituWidgetSetY(child, fy);
            }
            scrolliconlistbox->frame++;

            if (scrolliconlistbox->frame > scrolliconlistbox->totalframe)
            {
                for (i = 0; i < count; ++i)
                {
                    ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(scrolliconlistbox, i);
                    int fy = 0 - child->rect.height * count / 3;

                    fy += i * child->rect.height;

                    ituWidgetSetY(child, fy);
                }

                if (widget->flags & ITU_PROGRESS)
                {
                    char buf[32];

                    if (scrolliconlistbox->inc > 0)
                    {
                        listbox->pageIndex--;
                    }
                    else if (scrolliconlistbox->inc < 0)
                    {
                        listbox->pageIndex++;
                    }
                    sprintf(buf, "%d %d", listbox->pageIndex, listbox->pageCount);
                    ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_LOAD, (int)buf);
                    ituListBoxSelect(listbox, -1);
                }

                scrolliconlistbox->frame = 0;
                scrolliconlistbox->inc = 0;
                widget->flags &= ~ITU_UNDRAGGING;
                widget->flags &= ~ITU_PROGRESS;
            }
            result = true;
        }
        else if (widget->flags & ITU_BOUNCING)
        {
            float step;
            int i, count = itcTreeGetChildCount(scrolliconlistbox);

            if (scrolliconlistbox->frame <= scrolliconlistbox->totalframe / 2)
                step = (float)scrolliconlistbox->frame;
            else
                step = (float)scrolliconlistbox->totalframe - scrolliconlistbox->frame;

            step /= scrolliconlistbox->totalframe / 2;
            step = (float)M_PI / 2 - step * (float)M_PI / 2;
            step = 1.0f - sinf(step);

            //printf("step=%f\n", step);

            for (i = 0; i < count; ++i)
            {
                ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(scrolliconlistbox, i);
                int fy = 0 - child->rect.height * count / 3;
                fy += i * child->rect.height;
                fy += (int)(scrolliconlistbox->inc * step);

                ituWidgetSetY(child, fy);
            }

            scrolliconlistbox->frame++;

            if (scrolliconlistbox->frame > scrolliconlistbox->totalframe)
            {
                scrolliconlistbox->frame = 0;
                scrolliconlistbox->inc = 0;
                widget->flags &= ~ITU_BOUNCING;
            }
            result = true;
        }
        else if (scrolliconlistbox->inc)
        {
            int i, count = itcTreeGetChildCount(scrolliconlistbox);
            float step = (float)scrolliconlistbox->frame / scrolliconlistbox->totalframe;
            step = step * (float)M_PI / 2;
            step = sinf(step);

            //printf("step=%f\n", step);

            for (i = 0; i < count; ++i)
            {
                ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(scrolliconlistbox, i);
                int fy = 0 - child->rect.height * count / 3;
                fy += i * child->rect.height;
                fy += (int)(scrolliconlistbox->inc * step);

                ituWidgetSetY(child, fy);
            }
            scrolliconlistbox->frame++;

            if (scrolliconlistbox->frame > scrolliconlistbox->totalframe)
            {
                char buf[32];

                if (scrolliconlistbox->inc > 0)
                {
                    listbox->pageIndex--;
                }
                else // if (scrolliconlistbox->inc < 0)
                {
                    listbox->pageIndex++;
                }
                scrolliconlistbox->frame = 0;
                scrolliconlistbox->inc = 0;

                sprintf(buf, "%d %d", listbox->pageIndex, listbox->pageCount);
                ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_LOAD, (int)buf);
                ituListBoxSelect(listbox, -1);
            }
            result = true;
        }
    }
    
    if (!result)
    {
        result = ituIconListBoxUpdate(widget, ev, arg1, arg2, arg3);
    }
    result |= widget->dirty;
    return widget->visible ? result : false;
}

static void ScrollIconListBoxPrevPage(ITUScrollIconListBox* scrolliconlistbox)
{
    ITUListBox* listbox = (ITUListBox*) scrolliconlistbox;

    if (listbox->pageIndex < listbox->pageCount)
    {
        int i, count = itcTreeGetChildCount(scrolliconlistbox);

        for (i = 0; i < count; ++i)
        {
            ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(scrolliconlistbox, i);
            int fy = 0 - child->rect.height * count / 3;
            fy += i * child->rect.height;

            if (scrolliconlistbox->inc == 0)
                scrolliconlistbox->inc = 0 - (child->rect.height * count / 3) / scrolliconlistbox->totalframe;

            fy += scrolliconlistbox->inc;
            ituWidgetSetY(child, fy);
        }
        scrolliconlistbox->frame = 1;
    }
}

static void ScrollIconListBoxNextPage(ITUScrollIconListBox* scrolliconlistbox)
{
    ITUListBox* listbox = (ITUListBox*) scrolliconlistbox;

    if (listbox->pageIndex > 1)
    {
        int i, count = itcTreeGetChildCount(scrolliconlistbox);

        for (i = 0; i < count; ++i)
        {
            ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(scrolliconlistbox, i);
            int fy = 0 - child->rect.height * count / 3;
            fy += i * child->rect.height;

            if (scrolliconlistbox->inc == 0)
                scrolliconlistbox->inc = (child->rect.height * count / 3) / scrolliconlistbox->totalframe;

            fy += scrolliconlistbox->inc;
            ituWidgetSetY(child, fy);
        }
        scrolliconlistbox->frame = 1;
    }
}

static void ScrollIconListBoxBind(ITUScrollIconListBox* scrolliconlistbox, char* param)
{
    ITUWidget* widget = (ITUWidget*) scrolliconlistbox;
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
            scrolliconlistbox->inc = targetslistbox->inc;
            scrolliconlistbox->frame = targetslistbox->frame;
            scrolliconlistbox->touchY = targetslistbox->touchY;
        }
        else if (target->type == ITU_SCROLLMEDIAFILELISTBOX)
        {
            ITUScrollMediaFileListBox* targetsmflistbox = (ITUScrollMediaFileListBox*) target;
            scrolliconlistbox->inc = targetsmflistbox->inc;
            scrolliconlistbox->frame = targetsmflistbox->frame;
            scrolliconlistbox->touchY = targetsmflistbox->touchY;
        }
        else if (target->type == ITU_SCROLLICONLISTBOX)
        {
            ITUScrollIconListBox* targetsilistbox = (ITUScrollIconListBox*) target;
            scrolliconlistbox->inc = targetsilistbox->inc;
            scrolliconlistbox->frame = targetsilistbox->frame;
            scrolliconlistbox->touchY = targetsilistbox->touchY;
        }
    }
}

void ituScrollIconListBoxOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    assert(widget);

    switch (action)
    {
    case ITU_ACTION_PREV:
        ScrollIconListBoxPrevPage((ITUScrollIconListBox*)widget);
        break;

    case ITU_ACTION_NEXT:
        ScrollIconListBoxNextPage((ITUScrollIconListBox*)widget);
        break;

    case ITU_ACTION_BIND:
        ScrollIconListBoxBind((ITUScrollIconListBox*)widget, param);
        break;

    default:
        ituListBoxOnAction(widget, action, param);
        break;
    }
}

void ituScrollIconListBoxInit(ITUScrollIconListBox* scrolliconlistbox, int width)
{
    assert(scrolliconlistbox);
    ITU_ASSERT_THREAD();

    memset(scrolliconlistbox, 0, sizeof (ITUScrollIconListBox));

    ituIconListBoxInit(&scrolliconlistbox->listbox, width);

    ituWidgetSetType(scrolliconlistbox, ITU_SCROLLICONLISTBOX);
    ituWidgetSetName(scrolliconlistbox, scrolliconlistboxName);
    ituWidgetSetUpdate(scrolliconlistbox, ituScrollIconListBoxUpdate);
    ituWidgetSetOnAction(scrolliconlistbox, ituScrollIconListBoxOnAction);
}

void ituScrollIconListBoxLoad(ITUScrollIconListBox* scrolliconlistbox, uint32_t base)
{
    assert(scrolliconlistbox);

    ituIconListBoxLoad(&scrolliconlistbox->listbox, base);

    ituWidgetSetUpdate(scrolliconlistbox, ituScrollIconListBoxUpdate);
    ituWidgetSetOnAction(scrolliconlistbox, ituScrollIconListBoxOnAction);
}

int ituScrollIconListBoxGetItemCount(ITUScrollIconListBox* scrolliconlistbox)
{
    ITU_ASSERT_THREAD();
    return itcTreeGetChildCount(scrolliconlistbox) / 3;
}

void ituScrollIconListBoxSetSelectable(ITUScrollIconListBox* scrolliconlistbox, bool selectable)
{
	assert(scrolliconlistbox);
    ITU_ASSERT_THREAD();

	if (selectable)
	{
		int count = itcTreeGetChildCount(scrolliconlistbox);
		scrolliconlistbox->listbox.listbox.itemCount = count / 3;

		if ((count / 3) > 0)
			printf("[ScrollIconListBox]Set selectable successfaul! (%d)\n", (count / 3));
		else
			printf("[ScrollIconListBox]Set selectable fail!\n");
	}
	else
	{
		scrolliconlistbox->listbox.listbox.itemCount = 0;
		printf("[ScrollIconListBox]Set non-selectable!\n");
	}
}

ITCTree* ituScrollIconListBoxGetLastPageItem(ITUScrollIconListBox* scrolliconlistbox)
{
    ITU_ASSERT_THREAD();
    return itcTreeGetChildAt(scrolliconlistbox, 0);
}

ITCTree* ituScrollIconListBoxGetCurrPageItem(ITUScrollIconListBox* scrolliconlistbox)
{
    ITU_ASSERT_THREAD();
    return itcTreeGetChildAt(scrolliconlistbox, itcTreeGetChildCount(scrolliconlistbox) / 3);
}

ITCTree* ituScrollIconListBoxGetNextPageItem(ITUScrollIconListBox* scrolliconlistbox)
{
    ITU_ASSERT_THREAD();
    return itcTreeGetChildAt(scrolliconlistbox, itcTreeGetChildCount(scrolliconlistbox) * 2 / 3);
}

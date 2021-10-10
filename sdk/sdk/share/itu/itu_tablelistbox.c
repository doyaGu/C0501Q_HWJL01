#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"

#define UNDRAGGING_DECAY 20//10
#define BOUNDARY_TOR 3//30

static const char tablelistboxName[] = "ITUTableListBox";
static bool bottom_touch = false;

bool ituTableListBoxUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUListBox* listbox = (ITUListBox*) widget;
    ITUTableListBox* tablelistbox = (ITUTableListBox*) widget;
    assert(tablelistbox);

    if (ev == ITU_EVENT_LAYOUT && (arg1 != ITU_ACTION_FOCUS))
    {
        int i, count = itcTreeGetChildCount(tablelistbox);

        for (i = 0; i < count; ++i)
        {
            ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(tablelistbox, i);
            int fy = i * child->rect.height;
            fy += tablelistbox->touchOffset;
            ituWidgetSetY(child, fy);
        }

        tablelistbox->preindex = -1;
        result = true;
    }
    else if (ev == ITU_EVENT_MOUSEMOVE)
    {
        if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGING))
        {
            int y = arg3 - widget->rect.y;
            int offset = y + tablelistbox->touchOffset - tablelistbox->touchY;
            int i, count = itcTreeGetChildCount(widget);
            int topY, bottomY;
            ITUWidget* child = (ITUWidget*)((ITCTree*)tablelistbox)->child;
            if (child)
            {
				if (count > tablelistbox->listbox.itemCount)
					count = tablelistbox->listbox.itemCount;
                topY = child->rect.y;
                bottomY = topY + child->rect.height * count;
            }
            else
            {
                topY = bottomY = 0;
            }

            //printf("0: touchOffset=%d offset=%d y=%d\n", tablelistbox->touchOffset, offset, y);

            //if (topY <= widget->rect.height / 2 &&
            //    bottomY >= widget->rect.height / 2)
			if ((topY <= BOUNDARY_TOR) && (bottomY >= (widget->rect.height/2 - BOUNDARY_TOR)))
            {
                for (i = 0; i < count; ++i)
                {
                    ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(widget, i);
                    int fy = i * child->rect.height;
                    fy += offset;
                    ituWidgetSetY(child, fy);
                }
            }
        }
    }
    else if (ev == ITU_EVENT_MOUSEDOWN)
    {
        if (ituWidgetIsEnabled(widget) && !(widget->flags & ITU_UNDRAGGING))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                if (tablelistbox->inc && !(widget->flags & ITU_BOUNCING))
                {
                    ITUWidget* child = (ITUWidget*)((ITCTree*)tablelistbox)->child;
                    if (child)
                        tablelistbox->touchOffset = child->rect.y;

                    tablelistbox->frame = 0;
                    tablelistbox->inc = 0;

                    ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_SYNC, (int)widget->name);
                }
                else
                {
                    ITCTree* node = ((ITCTree*)tablelistbox)->child;
                    int count, i = 0;

                    tablelistbox->preindex = -1;
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
                            tablelistbox->preindex = count + i;
                            break;
                        }
                        i++;
                    }

                    if (widget->flags & ITU_DRAGGABLE)
                    {
                        tablelistbox->touchY = y;

                        if (widget->flags & ITU_HAS_LONG_PRESS)
                        {
                            tablelistbox->touchCount = 1;
                        }
                        else
                        {
                            widget->flags |= ITU_DRAGGING;
                            ituScene->dragged = widget;
                            ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_SYNC, (int)widget->name);
                        }
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
                int topY, bottomY;
                ITUWidget* child = (ITUWidget*)((ITCTree*)tablelistbox)->child;
                if (child)
                {
                    int count = itcTreeGetChildCount(widget);
					if (count > tablelistbox->listbox.itemCount)
						count = tablelistbox->listbox.itemCount;

                    tablelistbox->touchOffset = child->rect.y;
                    topY = child->rect.y;
                    bottomY = topY + child->rect.height * count;
                }
                else
                {
                    tablelistbox->touchOffset = topY = bottomY = 0;
                }

                if (topY > 0)
                {
                    tablelistbox->frame = (tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY)) - topY * (tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY)) / (widget->rect.height / 2);
                    dist = tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY);
                    tablelistbox->inc = (dist != 0) ? -(widget->rect.height / 2) / dist : -(widget->rect.height / 2);											
                    //printf("1: frame=%d topY=%d inc=%d\n", tablelistbox->frame, topY, tablelistbox->inc);
                    widget->flags |= ITU_UNDRAGGING;
                }
                else if (bottomY < widget->rect.height)
                {
					bottom_touch = true;
                    tablelistbox->frame = bottomY * (tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY)) / (widget->rect.height / 2);
                    dist = tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY);
                    tablelistbox->inc = (dist != 0) ? (widget->rect.height / 2) / dist : (widget->rect.height / 2);											
                    //printf("2: frame=%d bottomY=%d inc=%d\n", tablelistbox->frame, bottomY, tablelistbox->inc);
                    widget->flags |= ITU_UNDRAGGING;
                }
				else
					widget->flags |= ITU_UNDRAGGING;
                ituScene->dragged = NULL;
                widget->flags &= ~ITU_DRAGGING;
                ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_SYNC, (int)widget->name);
                result = true;
            }
            
            dist = y - tablelistbox->touchY;
            if (dist < 0)
                dist = -dist;

            if (ituWidgetIsInside(widget, x, y) && ((dist < ITU_DRAG_DISTANCE) || !(widget->flags & ITU_DRAGGABLE)))
            {
                ITCTree* node = ((ITCTree*)tablelistbox)->child;
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

                    if (ituWidgetIsInside(item, x1, y1) && tablelistbox->preindex == count + i)
                    {
                        if (ev == ITU_EVENT_MOUSEUP)
                        {
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
                tablelistbox->preindex = -1;
                result = true;
            }
            tablelistbox->touchCount = 0;
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
                ITCTree* node = ((ITCTree*)tablelistbox)->child;
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

                    if (ituWidgetIsInside(item, x1, y1) && tablelistbox->preindex == count + i)
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
        tablelistbox->touchCount = 0;
    }
    else if (ev == ITU_EVENT_TOUCHSLIDEUP || ev == ITU_EVENT_TOUCHSLIDEDOWN)
    {
        tablelistbox->touchCount = 0;

        if (ituWidgetIsEnabled(widget))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                if (ev == ITU_EVENT_TOUCHSLIDEUP)
                {
                    int count = itcTreeGetChildCount(tablelistbox);

                    if (widget->flags & ITU_DRAGGING)
                    {
                        widget->flags &= ~ITU_DRAGGING;
                        ituScene->dragged = NULL;
                        tablelistbox->inc = 0;
                    }

					if (tablelistbox->inc == 0)
					{
						tablelistbox->inc = 0 - widget->rect.height * tablelistbox->slidePage;
					}

					tablelistbox->frame = 1;
                    ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_TOUCHSLIDEUP, 0);
                }
                else // if (ev == ITU_EVENT_TOUCHSLIDEDOWN)
                {
                    int i, count = itcTreeGetChildCount(tablelistbox);

                    if (widget->flags & ITU_DRAGGING)
                    {
                        widget->flags &= ~ITU_DRAGGING;
                        ituScene->dragged = NULL;
                        tablelistbox->inc = 0;
                    }

                    for (i = 0; i < count; ++i)
                    {
                        ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(tablelistbox, i);
                        int fy = 0 - child->rect.height * count;
                        fy += i * child->rect.height;

                        if (tablelistbox->inc == 0)
                            tablelistbox->inc = widget->rect.height * tablelistbox->slidePage;

                        fy += tablelistbox->inc;
                        ituWidgetSetY(child, fy);
                    }

                    tablelistbox->frame = 1;
                    ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_TOUCHSLIDEDOWN, 0);
                }
                ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_SYNC, (int)widget->name);
                result = true;
            }
        }
    }
    else if (ev == ITU_EVENT_TIMER)
    {
        if (tablelistbox->touchCount > 0)
        {
            int y, dist;

            assert(widget->flags & ITU_HAS_LONG_PRESS);

            ituWidgetGetGlobalPosition(widget, NULL, &y);

            dist = ituScene->lastMouseY - (y + tablelistbox->touchY);

            if (dist < 0)
                dist = -dist;

            if (dist >= ITU_DRAG_DISTANCE)
            {
                widget->flags |= ITU_DRAGGING;
                ituScene->dragged = widget;
                tablelistbox->touchCount = 0;
                ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_SYNC, (int)widget->name);
            }
        }

        if (widget->flags & ITU_UNDRAGGING)
        {
            int i, count = itcTreeGetChildCount(tablelistbox);
			if (count > tablelistbox->listbox.itemCount)
				count = tablelistbox->listbox.itemCount;

            if (tablelistbox->inc > 0)
            {
				int fy = 0;
				int maxc = 0;
				
                for (i = 0; i < count; ++i)
                {
                    ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(tablelistbox, i);
					maxc = widget->rect.height / child->rect.height;

					if (!(widget->flags & ITU_DRAGGABLE))
					{
						fy = 0 - (child->rect.height * (count - maxc));

						fy += i * child->rect.height;
					}
					else
					{
						if (bottom_touch)
						{
							fy = 0 - (child->rect.height * (count - maxc));
							fy += i * child->rect.height;
							fy += tablelistbox->inc *(tablelistbox->frame + 1) / tablelistbox->totalframe;
						}
						else
						{
							fy = 0 - child->rect.height * count + widget->rect.height / 2;
							fy += i * child->rect.height;
							fy += tablelistbox->inc * tablelistbox->frame;
						}
					}

					if (count <= maxc)
					{
						fy = i * child->rect.height;
					}
                    ituWidgetSetY(child, fy);//here
					//printf("[fy1] %d\n", fy);
					//printf("[TIMER]child %d, fy, %d, recth %d, inc %d\n", i, fy, child->rect.height, tablelistbox->inc);
                }

				if (bottom_touch)
					bottom_touch = false;
            }
            else
            {
				int maxc = 0;

                for (i = 0; i < count; ++i)
                {
                    ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(tablelistbox, i);
					
					int fy = i * child->rect.height;// +widget->rect.height / 2;
					maxc = widget->rect.height / child->rect.height;

                    fy += tablelistbox->inc * tablelistbox->frame;

					if (count <= maxc)
					{
						fy = i * child->rect.height;
					}
                    ituWidgetSetY(child, fy);
					//printf("[fy2] %d\n", fy);
                }
            }
            tablelistbox->frame++;

            if (tablelistbox->frame > (tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY)))
            {
                ITUWidget* child = (ITUWidget*)((ITCTree*)tablelistbox)->child;

                if (child)
                    tablelistbox->touchOffset = child->rect.y;
                else
                    tablelistbox->touchOffset = 0;

                tablelistbox->frame = 0;
                tablelistbox->inc = 0;
                widget->flags &= ~ITU_UNDRAGGING;
            }
            result = true;
        }
        else if (tablelistbox->inc)
        {
            int i, count = itcTreeGetChildCount(tablelistbox);
            int topY, bottomY;
            ITUWidget* child = (ITUWidget*)((ITCTree*)tablelistbox)->child;
			if (count > tablelistbox->listbox.itemCount)
				count = tablelistbox->listbox.itemCount;

            if (child)
            {
                topY = child->rect.y;
                bottomY = topY + child->rect.height * count;
            }
            else
            {
                topY = bottomY = 0;
            }

            if (topY > widget->rect.height / 2)
            {
                tablelistbox->frame = (tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY)) - topY * (tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY)) / (widget->rect.height / 2);
                i = tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY);
                tablelistbox->inc = (i != 0) ? -(widget->rect.height / 2) / i : -(widget->rect.height / 2);
                //printf("3: frame=%d topY=%d inc=%d\n", tablelistbox->frame, topY, tablelistbox->inc);
                widget->flags |= ITU_UNDRAGGING;
            }
            else if (bottomY < widget->rect.height / 2)
            {
                tablelistbox->frame = bottomY * (tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY)) / (widget->rect.height / 2);
                i = tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY);
                tablelistbox->inc = (i != 0) ? (widget->rect.height / 2) / i : (widget->rect.height / 2);
                //printf("4: frame=%d bottomY=%d inc=%d\n", tablelistbox->frame, bottomY, tablelistbox->inc);
                widget->flags |= ITU_UNDRAGGING;
            }
            else
            {
                float lamda = 5.0f * (float)tablelistbox->frame / tablelistbox->totalframe;
                float step = tablelistbox->inc - tablelistbox->inc * expf(-lamda);

                //printf("step=%f %f %f\n", step, tablelistbox->inc * expf(-lamda), expf(-lamda));

                for (i = 0; i < count; ++i)
                {
                    ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(tablelistbox, i);
					int fy = tablelistbox->touchOffset;
                    fy += i * child->rect.height;
                    fy += (int)step;

                    ituWidgetSetY(child, fy);
					//printf("[fy3] %d\n", fy);
                }
                tablelistbox->frame++;

                if (tablelistbox->frame > tablelistbox->totalframe)
                {
                    if (topY > 0)
                    {
                        tablelistbox->frame = (tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY)) - topY * (tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY)) / (widget->rect.height / 2);
                        i = tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY);
                        tablelistbox->inc = (i != 0) ? -(widget->rect.height / 2) / i : -(widget->rect.height / 2);						
                        //printf("5: frame=%d topY=%d inc=%d\n", tablelistbox->frame, topY, tablelistbox->inc);
                        widget->flags |= ITU_UNDRAGGING;
                    }
                    else if (bottomY < widget->rect.height)
                    {
                        tablelistbox->frame = bottomY * (tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY)) / (widget->rect.height / 2);
                        i = tablelistbox->totalframe / (tablelistbox->slidePage * UNDRAGGING_DECAY);
                        tablelistbox->inc = (i != 0) ? (widget->rect.height / 2) / i : (widget->rect.height / 2);						
                        //printf("6: frame=%d bottomY=%d inc=%d\n", tablelistbox->frame, bottomY, tablelistbox->inc);
                        widget->flags |= ITU_UNDRAGGING;
                    }
                    else
                    {
                        if (child)
                            tablelistbox->touchOffset = child->rect.y;

                        //printf("7: frame=%d bottomY=%d inc=%d\n", tablelistbox->frame, bottomY, tablelistbox->inc);

                        tablelistbox->frame = 0;
                        tablelistbox->inc = 0;
                    }
                }
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

static void TableListBoxBind(ITUTableListBox* tablelistbox, char* param)
{
    ITUWidget* widget = (ITUWidget*) tablelistbox;
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

        if (target->type == ITU_TABLELISTBOX)
        {
            ITUTableListBox* targetslistbox = (ITUTableListBox*) target;
            tablelistbox->touchOffset = targetslistbox->touchOffset;
            tablelistbox->inc = targetslistbox->inc;
            tablelistbox->frame = targetslistbox->frame;
            tablelistbox->touchY = targetslistbox->touchY;
        }
		else if (target->type == ITU_TABLEICONLISTBOX)
		{
			ITUTableIconListBox* targettableiconlistbox = (ITUTableIconListBox*)target;
            tablelistbox->touchOffset = targettableiconlistbox->touchOffset;
			tablelistbox->inc = targettableiconlistbox->inc;
			tablelistbox->frame = targettableiconlistbox->frame;
			tablelistbox->touchY = targettableiconlistbox->touchY;
		}
    }
}

void ituTableListBoxOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    assert(widget);

    switch (action)
    {
    case ITU_ACTION_BIND:
        TableListBoxBind((ITUTableListBox*)widget, param);
        break;

    default:
        ituListBoxOnAction(widget, action, param);
        break;
    }
}

void ituTableListBoxOnLoadPage(ITUListBox* listbox, int pageIndex)
{
    ITUTableListBox* tablelistbox = (ITUTableListBox*) listbox;
    int i, count = itcTreeGetChildCount(tablelistbox);

    for (i = 0; i < count; ++i)
    {
        ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(tablelistbox, i);
        int fy = i * child->rect.height;
        ituWidgetSetY(child, fy);
    }

    tablelistbox->preindex = -1;
    tablelistbox->touchOffset = 0;
}

void ituTableListBoxInit(ITUTableListBox* tablelistbox, int width)
{
    assert(tablelistbox);
    ITU_ASSERT_THREAD();

    memset(tablelistbox, 0, sizeof (ITUTableListBox));

    ituListBoxInit(&tablelistbox->listbox, width);

    ituWidgetSetType(tablelistbox, ITU_TABLELISTBOX);
    ituWidgetSetName(tablelistbox, tablelistboxName);
    ituWidgetSetUpdate(tablelistbox, ituTableListBoxUpdate);
    ituWidgetSetOnAction(tablelistbox, ituTableListBoxOnAction);
    ituListBoxSetOnLoadPage(tablelistbox, ituTableListBoxOnLoadPage);
}

void ituTableListBoxLoad(ITUTableListBox* tablelistbox, uint32_t base)
{
    assert(tablelistbox);

    ituListBoxLoad(&tablelistbox->listbox, base);

    ituWidgetSetUpdate(tablelistbox, ituTableListBoxUpdate);
    ituWidgetSetOnAction(tablelistbox, ituTableListBoxOnAction);
    ituListBoxSetOnLoadPage(tablelistbox, ituTableListBoxOnLoadPage);
}

int ituTableListBoxGetItemCount(ITUTableListBox* tablelistbox)
{
    ITU_ASSERT_THREAD();
    return itcTreeGetChildCount(tablelistbox);
}

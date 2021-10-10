#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"

#define UNDRAGGING_DECAY 10
#define BOUNDARY_TOR 10

static const char tableiconlistboxName[] = "ITUTableIconListBox";
static bool bottom_touch = false;

void ituTableIconListBoxExit(ITUWidget* widget)
{
	ITUTableIconListBox* tableiconlistbox = (ITUTableIconListBox*)widget;
	int i;
	assert(widget);
    ITU_ASSERT_THREAD();

	for (i = 0; i < ITU_ICON_LISTBOX_TYPE_COUNT; i++)
	{
		if (tableiconlistbox->surfArray[i])
		{
			ituSurfaceRelease(tableiconlistbox->surfArray[i]);
			tableiconlistbox->surfArray[i] = NULL;
		}
		else
			break;
	}
	for (i = 0; i < ITU_ICON_LISTBOX_TYPE_COUNT; i++)
	{
		if (tableiconlistbox->focusSurfArray[i])
		{
			ituSurfaceRelease(tableiconlistbox->focusSurfArray[i]);
			tableiconlistbox->focusSurfArray[i] = NULL;
		}
		else
			break;
	}
	ituWidgetExitImpl(widget);
}

static void TableIconListBoxLoadExternalData(ITUTableIconListBox* tableiconlistbox, ITULayer* layer)
{
	ITUWidget* widget = (ITUWidget*)tableiconlistbox;
	int i;

	assert(widget);

	if (!(widget->flags & ITU_EXTERNAL))
		return;

	if (!layer)
		layer = ituGetLayer(widget);

	for (i = 0; i < ITU_ICON_LISTBOX_TYPE_COUNT; i++)
	{
		ITUSurface* surf;

		if (!tableiconlistbox->staticSurfArray[i] || tableiconlistbox->surfArray[i])
			break;

		surf = ituLayerLoadExternalSurface(layer, (uint32_t)tableiconlistbox->staticSurfArray[i]);

		if (surf->flags & ITU_COMPRESSED)
			tableiconlistbox->surfArray[i] = ituSurfaceDecompress(surf);
		else
			tableiconlistbox->surfArray[i] = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
	}

	for (i = 0; i < ITU_ICON_LISTBOX_TYPE_COUNT; i++)
	{
		ITUSurface* surf;

		if (!tableiconlistbox->focusStaticSurfArray[i] || tableiconlistbox->focusSurfArray[i])
			break;

		surf = ituLayerLoadExternalSurface(layer, (uint32_t)tableiconlistbox->focusStaticSurfArray[i]);

		if (surf->flags & ITU_COMPRESSED)
			tableiconlistbox->focusSurfArray[i] = ituSurfaceDecompress(surf);
		else
			tableiconlistbox->focusSurfArray[i] = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
	}
}

void ituTableIconListBoxLoadStaticData(ITUTableIconListBox* tableiconlistbox)
{
	ITUWidget* widget = (ITUWidget*)tableiconlistbox;
	int i;

	assert(widget);

	if (widget->flags & ITU_EXTERNAL)
		return;

	for (i = 0; i < ITU_ICON_LISTBOX_TYPE_COUNT; i++)
	{
		ITUSurface* surf;

		if (!tableiconlistbox->staticSurfArray[i] || tableiconlistbox->surfArray[i])
			break;

		surf = tableiconlistbox->staticSurfArray[i];

		if (surf->flags & ITU_COMPRESSED)
			tableiconlistbox->surfArray[i] = ituSurfaceDecompress(surf);
		else
			tableiconlistbox->surfArray[i] = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
	}

	for (i = 0; i < ITU_ICON_LISTBOX_TYPE_COUNT; i++)
	{
		ITUSurface* surf;

		if (!tableiconlistbox->focusStaticSurfArray[i] || tableiconlistbox->focusSurfArray[i])
			break;

		surf = tableiconlistbox->focusStaticSurfArray[i];

		if (surf->flags & ITU_COMPRESSED)
			tableiconlistbox->focusSurfArray[i] = ituSurfaceDecompress(surf);
		else
			tableiconlistbox->focusSurfArray[i] = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
	}
}

void ituTableIconListBoxReleaseSurface(ITUTableIconListBox* tableiconlistbox)
{
	int i;

    ITU_ASSERT_THREAD();

	for (i = 0; i < ITU_ICON_LISTBOX_TYPE_COUNT; i++)
	{
		if (tableiconlistbox->surfArray[i])
		{
			ituSurfaceRelease(tableiconlistbox->surfArray[i]);
			tableiconlistbox->surfArray[i] = NULL;
		}
		else
			break;
	}

	for (i = 0; i < ITU_ICON_LISTBOX_TYPE_COUNT; i++)
	{
		if (tableiconlistbox->focusSurfArray[i])
		{
			ituSurfaceRelease(tableiconlistbox->focusSurfArray[i]);
			tableiconlistbox->focusSurfArray[i] = NULL;
		}
		else
			break;
	}
}

bool ituTableIconListBoxUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
	bool result = false;
	ITUListBox* listbox = (ITUListBox*)widget;
	ITUTableIconListBox* tableiconlistbox = (ITUTableIconListBox*)widget;

	assert(tableiconlistbox);

	if (ev == ITU_EVENT_LOAD)
	{
		ituTableIconListBoxLoadStaticData(tableiconlistbox);
		result = true;
	}
	else if (ev == ITU_EVENT_LOAD_EXTERNAL)
	{
		TableIconListBoxLoadExternalData(tableiconlistbox, (ITULayer*)arg1);
		result = true;
	}
	else if (ev == ITU_EVENT_RELEASE)
	{
		ituTableIconListBoxReleaseSurface(tableiconlistbox);
		result = true;
	}
	
	else if (ev == ITU_EVENT_LAYOUT && (arg1 != ITU_ACTION_FOCUS))
	{
		int i, count = itcTreeGetChildCount(tableiconlistbox);

		for (i = 0; i < count; ++i)
		{
			ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(tableiconlistbox, i);
			int fy = i * child->rect.height;
            fy += tableiconlistbox->touchOffset;
			ituWidgetSetY(child, fy);
		}

		tableiconlistbox->preindex = -1;

		result = true;
	}
	else if (ev == ITU_EVENT_MOUSEMOVE)
	{
		if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGING))
		{
			int y = arg3 - widget->rect.y;
			int offset = y + tableiconlistbox->touchOffset - tableiconlistbox->touchY;
			int i, count = itcTreeGetChildCount(widget);
			int topY, bottomY;
			ITUWidget* child = (ITUWidget*)((ITCTree*)tableiconlistbox)->child;
			if (child)
			{
				topY = child->rect.y;
				bottomY = topY + child->rect.height * count;
			}
			else
			{
				topY = bottomY = 0;
			}

			//printf("0: touchOffset=%d offset=%d y=%d\n", tableiconlistbox->touchOffset, offset, y);

			//if (topY <= widget->rect.height / 2 &&
			//	bottomY >= widget->rect.height / 2)
			if (topY <= BOUNDARY_TOR && bottomY >= (widget->rect.height - BOUNDARY_TOR))
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
				if (tableiconlistbox->inc && !(widget->flags & ITU_BOUNCING))
				{
					ITUWidget* child = (ITUWidget*)((ITCTree*)tableiconlistbox)->child;
					if (child)
						tableiconlistbox->touchOffset = child->rect.y;

					tableiconlistbox->frame = 0;
					tableiconlistbox->inc = 0;

                    ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_SYNC, (int)widget->name);
 				}
				else
				{
					ITCTree* node = ((ITCTree*)tableiconlistbox)->child;
					int count, i = 0;

					tableiconlistbox->preindex = -1;
					count = listbox->itemCount;

					for (; node; node = node->sibling)
					{
						ITUWidget* item = (ITUWidget*)node;
						int x1, y1;

						if (i >= count)
							break;

						x1 = x - item->rect.x;
						y1 = y - item->rect.y;

						if (ituWidgetIsInside(item, x1, y1))
						{
							tableiconlistbox->preindex = count + i;
							break;
						}
						i++;
					}

					if (widget->flags & ITU_DRAGGABLE)
					{
						tableiconlistbox->touchY = y;

						if (widget->flags & ITU_HAS_LONG_PRESS)
						{
							tableiconlistbox->touchCount = 1;
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
				ITUWidget* child = (ITUWidget*)((ITCTree*)tableiconlistbox)->child;
				if (child)
				{
					int count = itcTreeGetChildCount(widget);

					tableiconlistbox->touchOffset = child->rect.y;
					topY = child->rect.y;
					bottomY = topY + child->rect.height * count;
				}
				else
				{
					tableiconlistbox->touchOffset = topY = bottomY = 0;
				}

				if (topY > 0)
				{
					int check = (tableiconlistbox->totalframe / (tableiconlistbox->slidePage * UNDRAGGING_DECAY)) - topY * (tableiconlistbox->totalframe / (tableiconlistbox->slidePage * UNDRAGGING_DECAY)) / (widget->rect.height / 2);
					tableiconlistbox->frame = (check >= 0)?(check):(0);
                    dist = tableiconlistbox->totalframe / (tableiconlistbox->slidePage * UNDRAGGING_DECAY);
                    tableiconlistbox->inc = (dist != 0) ? -(widget->rect.height / 2) / dist : -(widget->rect.height / 2);											
					//printf("1: frame=%d topY=%d inc=%d\n", tableiconlistbox->frame, topY, tableiconlistbox->inc);
					widget->flags |= ITU_UNDRAGGING;
				}
				else if (bottomY < widget->rect.height)
				{
					bottom_touch = true;
					tableiconlistbox->frame = bottomY * (tableiconlistbox->totalframe / (tableiconlistbox->slidePage * UNDRAGGING_DECAY)) / (widget->rect.height / 2);
                    dist = tableiconlistbox->totalframe / (tableiconlistbox->slidePage * UNDRAGGING_DECAY);
                    tableiconlistbox->inc = (dist != 0) ? (widget->rect.height / 2) / dist : (widget->rect.height / 2);											
					//printf("2: frame=%d bottomY=%d inc=%d\n", tableiconlistbox->frame, bottomY, tableiconlistbox->inc);
					widget->flags |= ITU_UNDRAGGING;
				}
				ituScene->dragged = NULL;
				widget->flags &= ~ITU_DRAGGING;
				ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_SYNC, (int)widget->name);
				result = true;
			}

			dist = y - tableiconlistbox->touchY;
			if (dist < 0)
				dist = -dist;

			if (ituWidgetIsInside(widget, x, y) && ((dist < ITU_DRAG_DISTANCE) || !(widget->flags & ITU_DRAGGABLE)))
			{
				ITCTree* node = ((ITCTree*)tableiconlistbox)->child;
				int count, i = 0;

				count = listbox->itemCount;

				for (; node; node = node->sibling)
				{
					ITUWidget* item = (ITUWidget*)node;
					int x1, y1;

					if (i >= count)
						break;

					x1 = x - item->rect.x;
					y1 = y - item->rect.y;

					if (ituWidgetIsInside(item, x1, y1) && tableiconlistbox->preindex == count + i)
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
				tableiconlistbox->preindex = -1;
				result = true;
			}
			tableiconlistbox->touchCount = 0;
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
				ITCTree* node = ((ITCTree*)tableiconlistbox)->child;
				int count, i = 0;

				count = listbox->itemCount;

				for (; node; node = node->sibling)
				{
					ITUWidget* item = (ITUWidget*)node;
					int x1, y1;

					if (i >= count)
						break;

					x1 = x - item->rect.x;
					y1 = y - item->rect.y;

					if (ituWidgetIsInside(item, x1, y1) && tableiconlistbox->preindex == count + i)
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
		tableiconlistbox->touchCount = 0;
	}
	else if (ev == ITU_EVENT_TOUCHSLIDEUP || ev == ITU_EVENT_TOUCHSLIDEDOWN)
	{
		tableiconlistbox->touchCount = 0;

		if (ituWidgetIsEnabled(widget))
		{
			int x = arg2 - widget->rect.x;
			int y = arg3 - widget->rect.y;

			if (ituWidgetIsInside(widget, x, y))
			{
				if (ev == ITU_EVENT_TOUCHSLIDEUP)
				{
					int count = itcTreeGetChildCount(tableiconlistbox);

					if (widget->flags & ITU_DRAGGING)
					{
						widget->flags &= ~ITU_DRAGGING;
						ituScene->dragged = NULL;
						tableiconlistbox->inc = 0;
					}

					/*
					for (i = 0; i < count; ++i)
					{
						ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(tableiconlistbox, i);
						int fy = 0 - child->rect.height * count;
						fy += i * child->rect.height;

						if (tableiconlistbox->inc == 0)
							tableiconlistbox->inc = widget->rect.height * tableiconlistbox->slidePage;

						fy -= tableiconlistbox->inc;
						ituWidgetSetY(child, fy);
					}
					*/
					//to sync with tablelistbox slide up
					if (tableiconlistbox->inc == 0)
						tableiconlistbox->inc = 0 - widget->rect.height * tableiconlistbox->slidePage;

					tableiconlistbox->frame = 1;
					ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_TOUCHSLIDEUP, 0);
				}
				else // if (ev == ITU_EVENT_TOUCHSLIDEDOWN)
				{
					int i, count = itcTreeGetChildCount(tableiconlistbox);

					if (widget->flags & ITU_DRAGGING)
					{
						widget->flags &= ~ITU_DRAGGING;
						ituScene->dragged = NULL;
						tableiconlistbox->inc = 0;
					}

					for (i = 0; i < count; ++i)
					{
						ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(tableiconlistbox, i);
						int fy = 0 - child->rect.height * count;

						fy += i * child->rect.height;

						if (tableiconlistbox->inc == 0)
							tableiconlistbox->inc = widget->rect.height * tableiconlistbox->slidePage;

						fy += tableiconlistbox->inc;
						ituWidgetSetY(child, fy);
					}

					tableiconlistbox->frame = 1;
					ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_TOUCHSLIDEDOWN, 0);
				}
				ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_SYNC, (int)widget->name);
				result = true;
			}
		}
	}
	else if (ev == ITU_EVENT_TIMER)
	{
		if (tableiconlistbox->touchCount > 0)
		{
			int y, dist;

			assert(widget->flags & ITU_HAS_LONG_PRESS);

			ituWidgetGetGlobalPosition(widget, NULL, &y);

			dist = ituScene->lastMouseY - (y + tableiconlistbox->touchY);

			if (dist < 0)
				dist = -dist;

			if (dist >= ITU_DRAG_DISTANCE)
			{
				widget->flags |= ITU_DRAGGING;
				ituScene->dragged = widget;
				tableiconlistbox->touchCount = 0;
				ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_SYNC, (int)widget->name);
			}
		}

		if (widget->flags & ITU_UNDRAGGING)
		{
			int i, count = itcTreeGetChildCount(tableiconlistbox);

			if (tableiconlistbox->inc > 0)
			{
				for (i = 0; i < count; ++i)
				{
					ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(tableiconlistbox, i);
					int fy = 0 - child->rect.height * count + widget->rect.height / 2;

					fy += i * child->rect.height;

					if (bottom_touch)
						fy += tableiconlistbox->inc * (tableiconlistbox->frame + 1) / 2;
					else
						fy += tableiconlistbox->inc * tableiconlistbox->frame;

					ituWidgetSetY(child, fy);
				}
				if (bottom_touch)
					bottom_touch = false;
			}
			else
			{
				for (i = 0; i < count; ++i)
				{
					ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(tableiconlistbox, i);
					int fy = i * child->rect.height + widget->rect.height / 2;

					fy += tableiconlistbox->inc * tableiconlistbox->frame;

					ituWidgetSetY(child, fy);
				}
			}
			tableiconlistbox->frame++;

			if (tableiconlistbox->frame > (tableiconlistbox->totalframe / (tableiconlistbox->slidePage * UNDRAGGING_DECAY)))
			{
				ITUWidget* child = (ITUWidget*)((ITCTree*)tableiconlistbox)->child;

				if (child)
					tableiconlistbox->touchOffset = child->rect.y;
				else
					tableiconlistbox->touchOffset = 0;

				tableiconlistbox->frame = 0;
				tableiconlistbox->inc = 0;
				widget->flags &= ~ITU_UNDRAGGING;
			}
			result = true;
		}
		else if (tableiconlistbox->inc)
		{
			int i, count = itcTreeGetChildCount(tableiconlistbox);
			int topY, bottomY;
			ITUWidget* child = (ITUWidget*)((ITCTree*)tableiconlistbox)->child;

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
				tableiconlistbox->frame = (tableiconlistbox->totalframe / (tableiconlistbox->slidePage * UNDRAGGING_DECAY)) - topY * (tableiconlistbox->totalframe / (tableiconlistbox->slidePage * UNDRAGGING_DECAY)) / (widget->rect.height / 2);
                i = tableiconlistbox->totalframe / (tableiconlistbox->slidePage * UNDRAGGING_DECAY);
                tableiconlistbox->inc = (i != 0) ? -(widget->rect.height / 2) / i : -(widget->rect.height / 2);
				//printf("3: frame=%d topY=%d inc=%d\n", tableiconlistbox->frame, topY, tableiconlistbox->inc);
				widget->flags |= ITU_UNDRAGGING;
			}
			else if (bottomY < widget->rect.height / 2)
			{
				tableiconlistbox->frame = bottomY * (tableiconlistbox->totalframe / (tableiconlistbox->slidePage * UNDRAGGING_DECAY)) / (widget->rect.height / 2);
                i = tableiconlistbox->totalframe / (tableiconlistbox->slidePage * UNDRAGGING_DECAY);
                tableiconlistbox->inc = (i != 0) ? (widget->rect.height / 2) / i : (widget->rect.height / 2);
				//printf("4: frame=%d bottomY=%d inc=%d\n", tableiconlistbox->frame, bottomY, tableiconlistbox->inc);
				widget->flags |= ITU_UNDRAGGING;
			}
			else
			{
				float lamda = 5.0f * (float)tableiconlistbox->frame / tableiconlistbox->totalframe;
				float step = tableiconlistbox->inc - tableiconlistbox->inc * expf(-lamda);

				//printf("step=%f %f %f\n", step, tableiconlistbox->inc * expf(-lamda), expf(-lamda));

				for (i = 0; i < count; ++i)
				{
					ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(tableiconlistbox, i);
					int fy = tableiconlistbox->touchOffset;
					fy += i * child->rect.height;
					fy += (int)step;

					ituWidgetSetY(child, fy);
				}
				tableiconlistbox->frame++;

				if (tableiconlistbox->frame > tableiconlistbox->totalframe)
				{
					if (topY > 0)
					{
						tableiconlistbox->frame = (tableiconlistbox->totalframe / (tableiconlistbox->slidePage * UNDRAGGING_DECAY)) - topY * (tableiconlistbox->totalframe / (tableiconlistbox->slidePage * UNDRAGGING_DECAY)) / (widget->rect.height / 2);
                        i = tableiconlistbox->totalframe / (tableiconlistbox->slidePage * UNDRAGGING_DECAY);
                        tableiconlistbox->inc = (i != 0) ? -(widget->rect.height / 2) / i : -(widget->rect.height / 2);						
						//printf("5: frame=%d topY=%d inc=%d\n", tableiconlistbox->frame, topY, tableiconlistbox->inc);
						widget->flags |= ITU_UNDRAGGING;
					}
					else if (bottomY < widget->rect.height)
					{
						tableiconlistbox->frame = bottomY * (tableiconlistbox->totalframe / (tableiconlistbox->slidePage * UNDRAGGING_DECAY)) / (widget->rect.height / 2);
                        i = tableiconlistbox->totalframe / (tableiconlistbox->slidePage * UNDRAGGING_DECAY);
                        tableiconlistbox->inc = (i != 0) ? (widget->rect.height / 2) / i : (widget->rect.height / 2);						
						//printf("6: frame=%d bottomY=%d inc=%d\n", tableiconlistbox->frame, bottomY, tableiconlistbox->inc);
						widget->flags |= ITU_UNDRAGGING;
					}
					else
					{
						if (child)
							tableiconlistbox->touchOffset = child->rect.y;

						//printf("7: frame=%d bottomY=%d inc=%d\n", tableiconlistbox->frame, bottomY, tableiconlistbox->inc);

						tableiconlistbox->frame = 0;
						tableiconlistbox->inc = 0;
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

void ituTableIconListBoxDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
	int destx, desty, count, i;
	uint8_t desta;
	ITURectangle prevClip;
	ITCTree* node;
	ITUTableIconListBox* tableiconlistbox = (ITUTableIconListBox*)widget;
	ITUFlowWindow* fwin = (ITUFlowWindow*)widget;
	ITURectangle* rect = (ITURectangle*)&widget->rect;
	assert(widget);
	assert(dest);

	destx = rect->x + x;
	desty = rect->y + y;
	desta = alpha * widget->color.alpha / 255;
	desta = desta * widget->alpha / 255;

	ituWidgetSetClipping(widget, dest, x, y, &prevClip);

	//drawing whole background
	if (desta == 255)
	{
		ituColorFill(dest, destx, desty, rect->width, rect->height, &widget->color);
	}
	else if (desta > 0)
	{
		ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
		if (surf)
		{
			ituColorFill(surf, 0, 0, rect->width, rect->height, &widget->color);
			ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);
			ituDestroySurface(surf);
		}
	}

	if (fwin->borderSize > 0)
	{
		int destx, desty;
		uint8_t desta;

		destx = rect->x + x;
		desty = rect->y + y;
		desta = alpha * widget->color.alpha / 255;
		desta = desta * widget->alpha / 255;

		if (desta == 255)
		{
			ituColorFill(dest, destx, desty, rect->width, fwin->borderSize, &widget->color);
			ituColorFill(dest, destx, desty + rect->height - fwin->borderSize, rect->width, fwin->borderSize, &widget->color);
			ituColorFill(dest, destx, desty + fwin->borderSize, fwin->borderSize, rect->height - fwin->borderSize * 2, &widget->color);
			ituColorFill(dest, destx + rect->width - fwin->borderSize, desty + fwin->borderSize, fwin->borderSize, rect->height - fwin->borderSize * 2, &widget->color);
		}
		else if (desta > 0)
		{
			ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
			if (surf)
			{
				ITUColor black = { 255, 0, 0, 0 };
				ituColorFill(surf, 0, 0, rect->width, rect->height, &black);
				ituColorFill(surf, 0, 0, rect->width, fwin->borderSize, &widget->color);
				ituColorFill(surf, 0, rect->height - fwin->borderSize, rect->width, fwin->borderSize, &widget->color);
				ituColorFill(surf, 0, fwin->borderSize, fwin->borderSize, rect->height - fwin->borderSize * 2, &widget->color);
				ituColorFill(surf, rect->width - fwin->borderSize, fwin->borderSize, fwin->borderSize, rect->height - fwin->borderSize * 2, &widget->color);
				ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);
				ituDestroySurface(surf);
			}
		}
	}

	if (widget->type == ITU_SCROLLICONLISTBOX)
		count = itcTreeGetChildCount(tableiconlistbox) / 3;
	else
		count = 0;

	i = 0;
	alpha = alpha * widget->alpha / 255;
	for (node = widget->tree.child; node; node = node->sibling)
	{
		ITUWidget* child = (ITUWidget*)node;
		ITUText* text = (ITUText*)child;
		ITURectangle* childRect = (ITURectangle*)&child->rect;
		int childx, childy;
		ITURectangle childPrevClip;
		uint8_t childa, childbga;

		childx = childRect->x + destx;
		childy = childRect->y + desty;
		childa = alpha * child->color.alpha / 255;
		childa = childa * child->alpha / 255;
		childbga = alpha * text->bgColor.alpha / 255;
		childbga = childbga * child->alpha / 255;

		ituWidgetSetClipping(child, dest, destx, desty, &childPrevClip);
		//draw item background
		if (childbga == 255)
		{
			ituColorFill(dest, childx, childy, childRect->width, childRect->height, &text->bgColor);
		}
		else if (childbga > 0)
		{
			ITUSurface* surf = ituCreateSurface(childRect->width, childRect->height, 0, dest->format, NULL, 0);
			if (surf)
			{
				ituColorFill(surf, 0, 0, childRect->width, childRect->height, &text->bgColor);
				ituAlphaBlend(dest, childx, childy, childRect->width, childRect->height, surf, 0, 0, childbga);
				ituDestroySurface(surf);
			}
		}

		if (childa > 0)
		{
			ITUSurface* surf = NULL;
			char* str = ituTextGetString(text);
			if (str && str[0] != '\0')
			{
				int index = atoi(str);

				if (index < ITU_ICON_LISTBOX_TYPE_COUNT)
				{
					if (i == count + tableiconlistbox->listbox.focusIndex && tableiconlistbox->focusSurfArray[index])
						surf = tableiconlistbox->focusSurfArray[index];
					else
						surf = tableiconlistbox->surfArray[index];
				}

				if (surf)
				{
					int xx = childx + childRect->width / 2 - surf->width / 2;
					int yy = childy + childRect->height / 2 - surf->height / 2;

					if (childa == 255)
					{
						ituBitBlt(dest, xx, yy, childRect->width, childRect->height, surf, 0, 0);
					}
					else
					{
						ituAlphaBlend(dest, xx, yy, childRect->width, childRect->height, surf, 0, 0, childa);
					}
				}
			}
		}
		i++;
		child->dirty = false;
		ituSurfaceSetClipping(dest, childPrevClip.x, childPrevClip.y, childPrevClip.width, childPrevClip.height);
	}
	widget->dirty = false;
	ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
}

static void TableIconListBoxBind(ITUTableIconListBox* tableiconlistbox, char* param)
{
	ITUWidget* widget = (ITUWidget*)tableiconlistbox;
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
			ITUTableListBox* targettablelistbox = (ITUTableListBox*)target;
            tableiconlistbox->touchOffset = targettablelistbox->touchOffset;
 			tableiconlistbox->inc = targettablelistbox->inc;
			tableiconlistbox->frame = targettablelistbox->frame;
			tableiconlistbox->touchY = targettablelistbox->touchY;
		}
		else if (target->type == ITU_TABLEICONLISTBOX)
		{
			ITUTableIconListBox* targettableiconlistbox = (ITUTableIconListBox*)target;
            tableiconlistbox->touchOffset = targettableiconlistbox->touchOffset;
			tableiconlistbox->inc = targettableiconlistbox->inc;
			tableiconlistbox->frame = targettableiconlistbox->frame;
			tableiconlistbox->touchY = targettableiconlistbox->touchY;
		}
	}
}

void ituTableIconListBoxOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
	assert(widget);

	switch (action)
	{
	case ITU_ACTION_BIND:
		TableIconListBoxBind((ITUTableIconListBox*)widget, param);
		break;

	default:
		ituListBoxOnAction(widget, action, param);
		break;
	}
}

void ituTableIconListBoxOnLoadPage(ITUListBox* listbox, int pageIndex)
{
    ITUTableIconListBox* tableiconlistbox = (ITUTableIconListBox*) listbox;
    int i, count = itcTreeGetChildCount(tableiconlistbox);

    for (i = 0; i < count; ++i)
    {
        ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(tableiconlistbox, i);
        int fy = i * child->rect.height;
        ituWidgetSetY(child, fy);
    }

    tableiconlistbox->preindex = -1;
    tableiconlistbox->touchOffset = 0;
}

void ituTableIconListBoxInit(ITUTableIconListBox* tableiconlistbox, int width)
{
	assert(tableiconlistbox);
    ITU_ASSERT_THREAD();

	memset(tableiconlistbox, 0, sizeof (ITUTableIconListBox));

	ituListBoxInit(&tableiconlistbox->listbox, width);

	ituWidgetSetType(tableiconlistbox, ITU_TABLEICONLISTBOX);
	ituWidgetSetName(tableiconlistbox, tableiconlistboxName);
	ituWidgetSetExit(tableiconlistbox, ituTableIconListBoxExit);
	ituWidgetSetUpdate(tableiconlistbox, ituTableIconListBoxUpdate);
	ituWidgetSetDraw(tableiconlistbox, ituTableIconListBoxDraw);
	ituWidgetSetOnAction(tableiconlistbox, ituTableIconListBoxOnAction);
    ituListBoxSetOnLoadPage(tableiconlistbox, ituTableIconListBoxOnLoadPage);
}

void ituTableIconListBoxLoad(ITUTableIconListBox* tableiconlistbox, uint32_t base)
{
	ITUWidget* widget = (ITUWidget*)tableiconlistbox;
	assert(tableiconlistbox);

	ituListBoxLoad(&tableiconlistbox->listbox, base);

	ituWidgetSetExit(tableiconlistbox, ituTableIconListBoxExit);
	ituWidgetSetUpdate(tableiconlistbox, ituTableIconListBoxUpdate);
	ituWidgetSetDraw(tableiconlistbox, ituTableIconListBoxDraw);
	ituWidgetSetOnAction(tableiconlistbox, ituTableIconListBoxOnAction);
    ituListBoxSetOnLoadPage(tableiconlistbox, ituTableIconListBoxOnLoadPage);

	if (!(widget->flags & ITU_EXTERNAL))
	{
		int i;
		
		for (i = 0; i < ITU_ICON_LISTBOX_TYPE_COUNT; i++)
		{
			if (tableiconlistbox->staticSurfArray[i])
			{
				ITUSurface* surf = (ITUSurface*)(base + (uint32_t)tableiconlistbox->staticSurfArray[i]);
				if (surf->flags & ITU_COMPRESSED)
					tableiconlistbox->surfArray[i] = NULL;
				else
					tableiconlistbox->surfArray[i] = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);

				tableiconlistbox->staticSurfArray[i] = surf;
			}
		}

		for (i = 0; i < ITU_ICON_LISTBOX_TYPE_COUNT; i++)
		{
			if (tableiconlistbox->focusStaticSurfArray[i])
			{
				ITUSurface* surf = (ITUSurface*)(base + (uint32_t)tableiconlistbox->focusStaticSurfArray[i]);
				if (surf->flags & ITU_COMPRESSED)
					tableiconlistbox->focusSurfArray[i] = NULL;
				else
					tableiconlistbox->focusSurfArray[i] = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);

				tableiconlistbox->focusStaticSurfArray[i] = surf;
			}
		}
	}

}

int ituTableIconListBoxGetItemCount(ITUTableIconListBox* tableiconlistbox)
{
    ITU_ASSERT_THREAD();
	return itcTreeGetChildCount(tableiconlistbox);
}

void ituTableIconListBoxSetIconIndex(ITUTableIconListBox* tableiconlistbox, int itemIndex, int iconIndex)
{
	ITUListBox* listbox = (ITUListBox *)tableiconlistbox;
    ITUScrollText* item;
    ITU_ASSERT_THREAD();

	if (listbox->itemCount == 0 || itemIndex >= listbox->itemCount || iconIndex >= ITU_ICON_LISTBOX_TYPE_COUNT)
		return;

    item = (ITUScrollText*)itcTreeGetChildAt(tableiconlistbox, itemIndex);
    if (item)
    {
        char buf[32];

        sprintf(buf, "%d", iconIndex);
        ituTextSetString(item, buf);

        ituWidgetSetDirty(tableiconlistbox, true);
    }
}

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/time.h>
#include "ite/itp.h"
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

#define COVERFLOW_FAST_SLIDE_TIMECHECK 140
#define ITU_COVERFLOW_ANYSTOP       0x10
#define ITU_COVERFLOW_ANYBOUNCE       0x20
#define ITU_COVERFLOW_ANYBOUNCE1       0x40
#define ITU_COVERFLOW_ANYBOUNCE2       0x80
#define ITU_BOUNCE_1 0x1000
#define ITU_BOUNCE_2 0x2000

#define COVERFLOW_FACTOR 10
#define COVERFLOW_PROCESS_STAGE1 0.2f
#define COVERFLOW_PROCESS_STAGE2 0.4f
#define COVERFLOW_OVERLAP_MAX_PERCENTAGE 80

static int slide_diff = 0;
static const char coverFlowName[] = "ITUCoverFlow";
static int mousedown_position;

int CoverFlowGetVisibleChildCount(ITUCoverFlow* coverflow)
{
    int i = 0;
    ITCTree *child, *tree = (ITCTree*)coverflow;
    assert(tree);

    for (child = tree->child; child; child = child->sibling)
    {
        ITUWidget* widget = (ITUWidget*)child;
        if (widget->visible)
            i++;
    }
    return i;
}

ITUWidget* CoverFlowGetVisibleChild(ITUCoverFlow* coverflow, int index)
{
    int i = 0;
    ITCTree *child, *tree = (ITCTree*)coverflow;
    assert(tree);

    for (child = tree->child; child; child = child->sibling)
    {
        ITUWidget* widget = (ITUWidget*)child;

		if (widget->visible)
		{
			if (i++ == index)
				return widget;
		}
    }
    return NULL;
}

int CoverFlowCheckBoundaryTouch(ITUWidget* widget)
{
	ITUCoverFlow* coverFlow = (ITUCoverFlow*)widget;
	ITUWidget* childbase = CoverFlowGetVisibleChild(coverFlow, 0);
	int count = CoverFlowGetVisibleChildCount(coverFlow);
	int base_size;
	int max_neighbor_item;
	int max_width_item;
	int result = 0;

	if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
	{
		base_size = childbase->rect.height - (((coverFlow->overlapsize > 0)) ? (coverFlow->overlapsize) : (0));
	}
	else
	{
		base_size = childbase->rect.width - (((coverFlow->overlapsize > 0)) ? (coverFlow->overlapsize) : (0));
	}

	max_neighbor_item = ((widget->rect.width / base_size) - 1) / 2;
	max_width_item = widget->rect.width / base_size;

	if (max_neighbor_item == 0)
		max_neighbor_item++;

	if (coverFlow->focusIndex >= max_neighbor_item)
	{
		if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_neighbor_item))
			result = ITU_BOUNCE_2;
		else
		{
			if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_width_item))
				result = ITU_BOUNCE_2;
		}
	}
	else
		result = ITU_BOUNCE_1;

	return result;
}

void CoverFlowLayout(ITUWidget* widget)
{
	ITUCoverFlow* coverFlow = (ITUCoverFlow*)widget;

	if (coverFlow != NULL)
	{
		int i, count = CoverFlowGetVisibleChildCount(coverFlow);
		int base_size;
		bool open_debug = false;
		ITUWidget* childbase = CoverFlowGetVisibleChild(coverFlow, 0);

		if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
		{
			base_size = childbase->rect.height - (((coverFlow->overlapsize > 0)) ? (coverFlow->overlapsize) : (0));
		}
		else
		{
			base_size = childbase->rect.width - (((coverFlow->overlapsize > 0)) ? (coverFlow->overlapsize) : (0));
		}

		if (count > 0)
		{
			//if (coverFlow->focusIndex >= count)
			//	coverFlow->focusIndex = count - 1;

			if (coverFlow->focusIndex > (count - 1))
				coverFlow->focusIndex = count - 1;
			else if (coverFlow->focusIndex < 0)
				coverFlow->focusIndex = 0;

			if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
			{
				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
				{
					int index, count2;

					count2 = count / 2 + 1;
					index = coverFlow->focusIndex;

					for (i = 0; i < count2; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
						int fy = widget->rect.height / 2 - child->rect.height / 2;
						fy += i * child->rect.height;
						ituWidgetSetY(child, fy);

						if (index >= count - 1)
							index = 0;
						else
							index++;
					}

					count2 = count - count2;
					for (i = 0; i < count2; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
						int fy = widget->rect.height / 2 - child->rect.height / 2;
						fy -= count2 * child->rect.height;
						fy += i * child->rect.height;
						ituWidgetSetY(child, fy);

						if (index >= count - 1)
							index = 0;
						else
							index++;
					}
				}
				else //[LAYOUT][Vertical][non-cycle]
				{
					for (i = 0; i < count; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
						int fy = widget->rect.height / 2 - base_size / 2;

						if (coverFlow->boundaryAlign)
						{
							int max_neighbor_item = ((widget->rect.height / base_size) - 1) / 2;
							int max_height_item = (widget->rect.height / base_size);
							fy = 0;

							if (max_neighbor_item == 0)
								max_neighbor_item++;

							if (coverFlow->focusIndex > 0) //>= max_neighbor_item) //Bless new debug
							{
								//if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_neighbor_item))
								if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_height_item))
									fy = widget->rect.height - (count * base_size);
								else
									fy -= base_size * coverFlow->focusIndex;
							}
							else
								fy = 0;
						}
						else
						{
							fy -= base_size * coverFlow->focusIndex;
						}

						if (coverFlow->overlapsize > 0)
						{
							fy += i * base_size;
							ituWidgetSetY(child, fy - coverFlow->overlapsize);
						}
						else
						{
							fy += i * child->rect.height;
							ituWidgetSetY(child, fy);
						}
					}
				}
			}
			else
			{
				//[LAYOUT][Horizontal][cycle]
				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
				{
					int index, count2;

					count2 = count / 2 + 1;
					index = coverFlow->focusIndex;

					for (i = 0; i < count2; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
						int fx = widget->rect.width / 2 - child->rect.width / 2;
						fx += i * child->rect.width;
						ituWidgetSetX(child, fx);

						if (index >= count - 1)
							index = 0;
						else
							index++;
					}

					count2 = count - count2;
					for (i = 0; i < count2; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
						int fx = widget->rect.width / 2 - child->rect.width / 2;
						fx -= count2 * child->rect.width;
						fx += i * child->rect.width;
						ituWidgetSetX(child, fx);

						if (index >= count - 1)
							index = 0;
						else
							index++;
					}
				}
				else //[LAYOUT][Horizontal][non-cycle]
				{
					for (i = 0; i < count; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
						int fx = widget->rect.width / 2 - base_size / 2;

						if (coverFlow->boundaryAlign)
						{
							int max_neighbor_item = ((widget->rect.width / base_size) - 1) / 2;
							int max_width_item = (widget->rect.width / base_size);

							fx = 0;

							if (max_neighbor_item == 0)
								max_neighbor_item++;

							if (coverFlow->focusIndex > 0)
							{
								if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_width_item))
									fx = widget->rect.width - (count * base_size);
								else
									fx -= base_size * coverFlow->focusIndex;
							}
							else
								fx = 0;
						}
						else
						{
							fx -= base_size * coverFlow->focusIndex;
						}

						if (coverFlow->overlapsize > 0)
						{
							fx += i * base_size;
							ituWidgetSetX(child, fx - coverFlow->overlapsize);
						}
						else
						{
							fx += i * child->rect.width;
							ituWidgetSetX(child, fx);
						}
					}
				}
			}

			if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ENABLE_ALL) == 0)
			{
				for (i = 0; i < count; ++i)
				{
					ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);

					if (i == coverFlow->focusIndex)
						ituWidgetEnable(child);
					else
						ituWidgetDisable(child);
				}
			}
			widget->flags &= ~ITU_DRAGGING;
			coverFlow->touchCount = 0;

			//fix for stop anywhere not display after load
			if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
			{
				ITUWidget* widget = (ITUWidget*)coverFlow;
				int count = CoverFlowGetVisibleChildCount(coverFlow);
				int i = 0;
				int fd = 0;
				int move_step = 0;
				ITUWidget* child_1 = CoverFlowGetVisibleChild(coverFlow, 0);
				ITUWidget* child_2 = CoverFlowGetVisibleChild(coverFlow, count - 1);

				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
				{
					if ((child_1->rect.y > 0) || ((child_2->rect.y + child_2->rect.height) < widget->rect.height))
					{
						if (child_1->rect.y > 0)
							move_step = 0 - child_1->rect.y;
						else
							move_step = widget->rect.height - (child_2->rect.y + child_2->rect.height);

						for (i = 0; i < count; i++)
						{
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
							fd = child->rect.y;
							fd += move_step;
							ituWidgetSetY(child, fd);
						}

						coverFlow->frame = 0;
					}
				}
				else
				{
					if ((child_1->rect.x > 0) || ((child_2->rect.x + child_2->rect.width) < widget->rect.width))
					{
						if (child_1->rect.x > 0)
							move_step = 0 - child_1->rect.x;
						else
							move_step = widget->rect.width - (child_2->rect.x + child_2->rect.width);

						for (i = 0; i < count; i++)
						{
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
							fd = child->rect.x;
							fd += move_step;
							ituWidgetSetX(child, fd);
						}

						coverFlow->frame = 0;
					}
				}
			}

			if (open_debug)
			{
				int x;
				ITUWidget* cc;

				for (x = 0; x < count; x++)
				{
					cc = CoverFlowGetVisibleChild(coverFlow, x);

					if (x != 1)
						continue;

					if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
						printf("[CoverFlowLayout] [%d] [%d]\n", x, cc->rect.y);
					else
						printf("[CoverFlowLayout] [%d] [%d]\n", x, cc->rect.x);
				}
			}
		}
	}
}

bool ituCoverFlowUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
	int widget_size, base_size;
    ITUCoverFlow* coverFlow = (ITUCoverFlow*) widget;
	assert(coverFlow);

	if (coverFlow)
	{
		ITUWidget* childbase = CoverFlowGetVisibleChild(coverFlow, 0);
		ITUWidget* cc = NULL;
		int count = CoverFlowGetVisibleChildCount(coverFlow);
		int i, min = 0;
		int min_w = childbase->rect.width;
		int min_h = childbase->rect.height;

		for (i = 1; i < count; i++)
		{
			cc = CoverFlowGetVisibleChild(coverFlow, i);

			if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
			{
				if (cc->rect.height < min_h)
				{
					min_h = cc->rect.height;
					childbase = cc;
				}
			}
			else
			{
				if (cc->rect.width < min_w)
				{
					min_w = cc->rect.width;
					childbase = cc;
				}
			}
		}

		if (!childbase)
			return result;

		if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
		{
			base_size = childbase->rect.height - (((coverFlow->overlapsize > 0)) ? (coverFlow->overlapsize) : (0));
			widget_size = widget->rect.height;
		}
		else
		{
			base_size = childbase->rect.width - (((coverFlow->overlapsize > 0)) ? (coverFlow->overlapsize) : (0));
			widget_size = widget->rect.width;
		}
	}

    if ((widget->flags & ITU_TOUCHABLE) && ituWidgetIsEnabled(widget) && (ev == ITU_EVENT_MOUSEDOWN || ev == ITU_EVENT_MOUSEUP))
    {
        int x = arg2 - widget->rect.x;
		int y = arg3 - widget->rect.y;

        if (ituWidgetIsInside(widget, x, y))
            result |= ituFlowWindowUpdate(widget, ev, arg1, arg2, arg3);
    }
    else
    {
        result |= ituFlowWindowUpdate(widget, ev, arg1, arg2, arg3);
    }

    if (widget->flags & ITU_TOUCHABLE) 
    {
		bool fast_slide = false;

		if (ev == ITU_EVENT_TOUCHSLIDELEFT || ev == ITU_EVENT_TOUCHSLIDERIGHT || ev == ITU_EVENT_TOUCHSLIDEUP || ev == ITU_EVENT_TOUCHSLIDEDOWN)
		{
			// to fix when slide at Non-Cycle mode boundary area
			if (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)) 
			{
				int count = CoverFlowGetVisibleChildCount(coverFlow);
				ITUWidget* child_first = CoverFlowGetVisibleChild(coverFlow, 0);
				ITUWidget* child_last  = CoverFlowGetVisibleChild(coverFlow, count - 1);
				int pos1, pos2;
				bool bForceMouseUp = false;

				if (coverFlow->boundaryAlign)
				{
					pos1 = 0;
					pos2 = widget_size;
				}
				else
				{
					pos1 = (widget_size - base_size) / 2;
					pos2 = (widget_size + base_size) / 2;
				}

				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
				{
					if ((child_first->rect.y >= pos1) || ((child_last->rect.y + child_last->rect.height) <= pos2))
						bForceMouseUp = true;
				}
				else
				{
					if ((child_first->rect.x >= pos1) || ((child_last->rect.x + child_last->rect.width) <= pos2))
						bForceMouseUp = true;
				}

				if (bForceMouseUp)
				{
					ituUnPressWidget(widget);
					ituWidgetUpdate(coverFlow, ITU_EVENT_MOUSEUP, arg1, arg2, arg3);
					return true;
				}
			}

			if ((itpGetTickCount() - coverFlow->clock) < COVERFLOW_FAST_SLIDE_TIMECHECK)
				fast_slide = true;

			slide_diff = arg1;

			if (ituWidgetIsEnabled(widget) && !result)
			{
				int x = arg2 - widget->rect.x;
				int y = arg3 - widget->rect.y;

				if (!widget->rect.width || !widget->rect.height || ituWidgetIsInside(widget, x, y))
				{
					result |= ituExecActions(widget, coverFlow->actions, ev, arg1);
				}
			}

			///////try to fix no slide and no mouseup at the same time
			if (coverFlow->slideMaxCount == 0)
			{
				//coverFlow->coverFlowFlags |= ITU_COVERFLOW_SLIDING;
				ituWidgetUpdate(coverFlow, ITU_EVENT_MOUSEUP, arg1, arg2, arg3);
			}
		}

		if ((ev == ITU_EVENT_TOUCHSLIDELEFT || ev == ITU_EVENT_TOUCHSLIDERIGHT) 
			&& ((coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL) == 0)
			&& (coverFlow->slideMaxCount > 0))
		{
			coverFlow->touchCount = 0;

			if (ituWidgetIsEnabled(widget))
			{
				int x = arg2 - widget->rect.x;
				int y = arg3 - widget->rect.y;

				if (ituWidgetIsInside(widget, x, y))
				{
					int count = CoverFlowGetVisibleChildCount(coverFlow);
					if (count > 0)
					{
						bool boundary_touch = false;
						bool boundary_touch_left = false;
						bool boundary_touch_right = false;
						////try to fix the mouse up shadow(last frame) diff when sliding start(frame 0)
						int offset, absoffset, interval;
						offset = x - coverFlow->touchPos;
						interval = offset / base_size;
						offset -= (interval * base_size);
						absoffset = offset > 0 ? offset : -offset;

						if (absoffset > base_size / 2)
							coverFlow->frame = (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
						else if (absoffset)
							coverFlow->frame = coverFlow->totalframe - (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
						else
							coverFlow->frame = 0;
						
						//if (widget->flags & ITU_DRAGGABLE)
						//	coverFlow->frame = coverFlow->totalframe - ((abs(x - coverFlow->touchPos) * coverFlow->totalframe)/base_size) + 1;

						if ((!(widget->flags & ITU_DRAGGABLE)) || fast_slide)
						{//debug here
							if (!(widget->flags & ITU_DRAGGABLE))
								coverFlow->frame = 0;
							LOG_DBG "[CoverFlow][Fast Slide!!]\n\n" LOG_END
						}
						else
							LOG_DBG "[CoverFlow][Normal Slide!!]\n\n" LOG_END

                        ituUnPressWidget(widget);

						//check boundary touch for H non-cycle
						if (coverFlow->boundaryAlign)
						{
							int max_neighbor_item = ((widget_size / base_size) - 1) / 2;

							coverFlow->slideCount = 0;

							if (max_neighbor_item == 0)
								max_neighbor_item++;

							if (coverFlow->focusIndex >= max_neighbor_item)
							{
								if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_neighbor_item))
								{
									boundary_touch = true;
									boundary_touch_right = true;
									coverFlow->coverFlowFlags |= ITU_BOUNCE_2;
								}
								else
								{
									ITUWidget* cf = CoverFlowGetVisibleChild(coverFlow, count - 1);
									if ((cf->rect.x + cf->rect.width) <= widget_size)
									{
										boundary_touch = true;
										boundary_touch_right = true;
										coverFlow->coverFlowFlags |= ITU_BOUNCE_2;
									}
								}
							}
							else
							{
								boundary_touch = true;
								boundary_touch_left = true;
								coverFlow->coverFlowFlags |= ITU_BOUNCE_1;
							}
						}

						if (ev == ITU_EVENT_TOUCHSLIDELEFT)
						{//debugging
							if ((coverFlow->slideMaxCount > 0) && (widget->flags & ITU_DRAGGABLE))//(coverFlow->boundaryAlign)
							{
								coverFlow->coverFlowFlags |= ITU_COVERFLOW_SLIDING;
								coverFlow->touchCount = 0;
							}

							if (widget->flags & ITU_DRAGGING)
							{
								widget->flags &= ~ITU_DRAGGING;
								ituScene->dragged = NULL;
								coverFlow->inc = 0;
							}

							if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE) ||
								(coverFlow->focusIndex < (count - 1)) || boundary_touch)
							{
								if (count > 0)
								{
									if (widget->flags & ITU_DRAGGING)
									{
										widget->flags &= ~ITU_DRAGGING;
										ituScene->dragged = NULL;
										coverFlow->inc = 0;
									}

									

									if (coverFlow->inc == 0)
										coverFlow->inc = 0 - base_size;

									if (boundary_touch)
									{
										ITUWidget* cf = CoverFlowGetVisibleChild(coverFlow, count - 1);
										if ((cf->rect.x + cf->rect.width) <= widget_size)
										{
											coverFlow->inc = -1;
											coverFlow->frame = coverFlow->totalframe - 1;

											//if ((boundary_touch) && (coverFlow->focusIndex > 0))
											//	coverFlow->focusIndex = CoverFlowGetVisibleChildCount(coverFlow) - widget_size / base_size;

											if (boundary_touch_right && !(widget->flags & ITU_DRAGGING) && coverFlow->bounceRatio > 0)
											{
												coverFlow->inc = 0 - (base_size / coverFlow->bounceRatio);

												coverFlow->focusIndex++;
												
												widget->flags |= ITU_BOUNCING;
												coverFlow->frame = 0;
											}
										}
									}
								}
							}
							else if (coverFlow->focusIndex >= count - 1)
							{//maybe useless now
								//try to fix the ScaleCoverFlow side effect for non-cycle mode
								if ((!(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)) && (coverFlow->boundaryAlign == 0))
								{
									coverFlow->inc = 0;
								}
								else if ((count) > 0 && !(widget->flags & ITU_DRAGGING) && coverFlow->bounceRatio > 0)
								{
									if (coverFlow->inc == 0)
										coverFlow->inc = 0 - (base_size / coverFlow->bounceRatio);

									widget->flags |= ITU_BOUNCING;
								}
							}
						}
						else // if (ev == ITU_EVENT_TOUCHSLIDERIGHT)
						{//debugging
							if ((coverFlow->slideMaxCount > 0) && (widget->flags & ITU_DRAGGABLE))//(coverFlow->boundaryAlign)
							{
								coverFlow->coverFlowFlags |= ITU_COVERFLOW_SLIDING;
								coverFlow->touchCount = 0;
							}

							if (widget->flags & ITU_DRAGGING)
							{
								widget->flags &= ~ITU_DRAGGING;
								ituScene->dragged = NULL;
								coverFlow->inc = 0;
							}

							if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE) ||
								(coverFlow->focusIndex > 0) || boundary_touch)
							{
								if (count > 0)
								{
									if (widget->flags & ITU_DRAGGING)
									{
										widget->flags &= ~ITU_DRAGGING;
										ituScene->dragged = NULL;
										coverFlow->inc = 0;
									}

									if (boundary_touch)
										coverFlow->focusIndex -= ((coverFlow->focusIndex > 1) ? (2) : (0));

									

									if (coverFlow->inc == 0)
										coverFlow->inc = base_size;

									if (boundary_touch)
									{
										ITUWidget* cf = CoverFlowGetVisibleChild(coverFlow, count - 1);
										if ((cf->rect.x + cf->rect.width) <= widget_size)
										{
											coverFlow->focusIndex = CoverFlowGetVisibleChildCount(coverFlow) - widget_size / base_size;
										}

										cf = CoverFlowGetVisibleChild(coverFlow, 0);
										if ((cf->rect.x) >= 0)
										{
											coverFlow->inc = 1;
											coverFlow->frame = coverFlow->totalframe - 1;
										}

										if (boundary_touch_left && !(widget->flags & ITU_DRAGGING) && coverFlow->bounceRatio > 0)
										{
											coverFlow->inc = (base_size / coverFlow->bounceRatio);
											coverFlow->focusIndex++;
											widget->flags |= ITU_BOUNCING;
											coverFlow->frame = 0;
										}
									}
								}
							}
							else if (coverFlow->focusIndex <= 0)
							{//maybe useless now
								//try to fix the ScaleCoverFlow side effect for non-cycle mode
								if ((!(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)) && (coverFlow->boundaryAlign == 0))
								{
									if (widget->flags & ITU_DRAGGING)
										widget->flags &= ~ITU_DRAGGING;
									coverFlow->inc = 0;
								}
								else if (count > 0 && !(widget->flags & ITU_DRAGGING) && coverFlow->bounceRatio > 0)
								{
									if (coverFlow->inc == 0)
										coverFlow->inc = (base_size / coverFlow->bounceRatio);

									widget->flags |= ITU_BOUNCING;
									//coverFlow->frame = 1;
								}
							}
						}
						result = true;
					}
				}
			}

			if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
				coverFlow->frame = 0;
		}
        else if ((ev == ITU_EVENT_TOUCHSLIDEUP || ev == ITU_EVENT_TOUCHSLIDEDOWN) 
			&& (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
			&& (coverFlow->slideMaxCount > 0))
        {
            coverFlow->touchCount = 0;

			if (ituWidgetIsEnabled(widget))// && (widget->flags & ITU_DRAGGABLE))
            {
                int x = arg2 - widget->rect.x;
                int y = arg3 - widget->rect.y;

                if (ituWidgetIsInside(widget, x, y))
                {
					int count = CoverFlowGetVisibleChildCount(coverFlow);
					if (count > 0)
					{
						bool boundary_touch = false;
						bool boundary_touch_top = false;
						bool boundary_touch_bottom = false;
						////try to fix the mouse up shadow(last frame) diff when sliding start(frame 0)
						int offset, absoffset, interval;
						offset = y - coverFlow->touchPos;
						interval = offset / base_size;
						offset -= (interval * base_size);
						absoffset = offset > 0 ? offset : -offset;

						if (absoffset > base_size / 2)
							coverFlow->frame = (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
						else if (absoffset)
							coverFlow->frame = coverFlow->totalframe - (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
						else
							coverFlow->frame = 0;

						if ((!(widget->flags & ITU_DRAGGABLE)) || fast_slide)
						{
							if (!(widget->flags & ITU_DRAGGABLE))
								coverFlow->frame = 0;
							LOG_DBG "[CoverFlow][Fast Slide!!]\n\n" LOG_END
						}
						else
							LOG_DBG "[CoverFlow][Normal Slide!!]\n\n" LOG_END

                        ituUnPressWidget(widget);

						if (coverFlow->boundaryAlign)
						{
							int max_neighbor_item = ((widget->rect.height / base_size) - 1) / 2;

							coverFlow->slideCount = 0;

							if (max_neighbor_item == 0)
								max_neighbor_item++;

							if (coverFlow->focusIndex >= max_neighbor_item)
							{
								if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_neighbor_item))
								{
									boundary_touch = true;
									boundary_touch_bottom = true;
									coverFlow->coverFlowFlags |= ITU_BOUNCE_2;
								}
								else
								{
									ITUWidget* cf = CoverFlowGetVisibleChild(coverFlow, count - 1);
									if ((cf->rect.y + cf->rect.height) <= widget_size)
									{
										boundary_touch = true;
										boundary_touch_bottom = true;
										coverFlow->coverFlowFlags |= ITU_BOUNCE_2;
									}
								}
							}
							else
							{
								boundary_touch = true;
								boundary_touch_top = true;
								coverFlow->coverFlowFlags |= ITU_BOUNCE_1;
							}
						}

						if (ev == ITU_EVENT_TOUCHSLIDEUP)
						{
							if ((coverFlow->slideMaxCount > 0) && (widget->flags & ITU_DRAGGABLE))//(coverFlow->boundaryAlign)
								coverFlow->coverFlowFlags |= ITU_COVERFLOW_SLIDING;

							if (widget->flags & ITU_DRAGGING)
							{
								widget->flags &= ~ITU_DRAGGING;
								ituScene->dragged = NULL;
								coverFlow->inc = 0;
							}

							if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE) ||
								(coverFlow->focusIndex < count - 1) || boundary_touch)
							{
								if (count > 0)
								{
									if (widget->flags & ITU_DRAGGING)
									{
										widget->flags &= ~ITU_DRAGGING;
										ituScene->dragged = NULL;
										coverFlow->inc = 0;
									}

									//if (boundary_touch)
									//	coverFlow->focusIndex += ((coverFlow->focusIndex < (count - 2)) ? (1) : (0));

									

									if (coverFlow->inc == 0)
										coverFlow->inc = 0 - base_size;

									if (boundary_touch)
									{
										ITUWidget* cf = CoverFlowGetVisibleChild(coverFlow, count - 1);
										if ((cf->rect.y + cf->rect.height) <= widget_size)
										{
											coverFlow->inc = -1;
											coverFlow->frame = coverFlow->totalframe - 1;


											if ((boundary_touch) && (coverFlow->focusIndex > 0))
												coverFlow->focusIndex = CoverFlowGetVisibleChildCount(coverFlow) - widget_size / base_size;

											if (boundary_touch_bottom && !(widget->flags & ITU_DRAGGING) && coverFlow->bounceRatio > 0)
											{
												coverFlow->inc = 0 - (base_size / coverFlow->bounceRatio);

												coverFlow->focusIndex++;

												widget->flags |= ITU_BOUNCING;
												coverFlow->frame = 0;
											}
										}
									}
								}
							}
							else if (coverFlow->focusIndex >= count - 1)
							{//maybe useless now
								//try to fix the ScaleCoverFlow side effect for non-cycle mode
								if ((!(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)) && (coverFlow->boundaryAlign == 0))
								{
									coverFlow->inc = 0;
								}
								else if (count > 0 && !(widget->flags & ITU_DRAGGING) && coverFlow->bounceRatio > 0)
								{
									if (coverFlow->inc == 0)
										coverFlow->inc = 0 - (base_size / coverFlow->bounceRatio);

									widget->flags |= ITU_BOUNCING;
									//coverFlow->frame = 1;
								}
							}
						}
						else // if (ev == ITU_EVENT_TOUCHSLIDEDOWN)
						{
							if ((coverFlow->slideMaxCount > 0) && (widget->flags & ITU_DRAGGABLE))//(coverFlow->boundaryAlign)
								coverFlow->coverFlowFlags |= ITU_COVERFLOW_SLIDING;

							if (widget->flags & ITU_DRAGGING)
							{
								widget->flags &= ~ITU_DRAGGING;
								ituScene->dragged = NULL;
								coverFlow->inc = 0;
							}

							if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE) ||
								(coverFlow->focusIndex > 0) || boundary_touch)
							{
								if (count > 0)
								{
									if (widget->flags & ITU_DRAGGING)
									{
										widget->flags &= ~ITU_DRAGGING;
										ituScene->dragged = NULL;
										coverFlow->inc = 0;
									}

									//if (boundary_touch)
									//	coverFlow->focusIndex -= ((coverFlow->focusIndex > 1) ? (1) : (0));
									if (boundary_touch)
										coverFlow->focusIndex -= ((coverFlow->focusIndex > 1) ? (2) : (0));

									

									if (coverFlow->inc == 0)
										coverFlow->inc = base_size;

									if (boundary_touch)
									{
										ITUWidget* cf = CoverFlowGetVisibleChild(coverFlow, count - 1);
										//if ((cf->rect.y + cf->rect.height) <= widget_size)
										//{
										//	coverFlow->focusIndex = CoverFlowGetVisibleChildCount(coverFlow) - widget_size / base_size;
										//}

										cf = CoverFlowGetVisibleChild(coverFlow, 0);
										if ((cf->rect.y) >= 0)
										{
											coverFlow->inc = +1;
											coverFlow->frame = coverFlow->totalframe - 1;
										}

										if (boundary_touch_top && !(widget->flags & ITU_DRAGGING) && coverFlow->bounceRatio > 0)
										{
											coverFlow->inc = (base_size / coverFlow->bounceRatio);
											coverFlow->focusIndex += 1;
											widget->flags |= ITU_BOUNCING;
											coverFlow->frame = 0;
										}
									}
									//coverFlow->frame = 1;
								}
							}
							else if (coverFlow->focusIndex <= 0)
							{//maybe useless now
								//try to fix the ScaleCoverFlow side effect for non-cycle mode
								if ((!(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)) && (coverFlow->boundaryAlign == 0))
								{
									coverFlow->inc = 0;
								}
								else if (count > 0 && !(widget->flags & ITU_DRAGGING) && coverFlow->bounceRatio > 0)
								{
									if (coverFlow->inc == 0)
										coverFlow->inc = (base_size / coverFlow->bounceRatio);

									widget->flags |= ITU_BOUNCING;
									//coverFlow->frame = 1;
								}
							}
						}
						result = true;
					}
                }
            }

			if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
				coverFlow->frame = 0;
        }
        else if (ev == ITU_EVENT_MOUSEMOVE)
        {
			if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGING))
            {
                int x = arg2 - widget->rect.x;
                int y = arg3 - widget->rect.y;

                if (ituWidgetIsInside(widget, x, y))
                {
                    int i, dist, offset, count = CoverFlowGetVisibleChildCount(coverFlow);
                    
					if (count > 0)
					{
						if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
						{
							dist = y - coverFlow->touchPos;
						}
						else
						{
							dist = x - coverFlow->touchPos;
						}

						if (dist < 0)
							dist = -dist;

						if (dist >= ITU_DRAG_DISTANCE)
						{
                            ituUnPressWidget(widget);
							ituWidgetUpdate(widget, ITU_EVENT_DRAGGING, 0, 0, 0);
						}

						if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
						{
							offset = y - coverFlow->touchPos;
							//printf("0: offset=%d\n", offset);
							if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
							{
								if (offset >= base_size)
								{
									if (coverFlow->focusIndex > 0)
										coverFlow->focusIndex--;
									else
										coverFlow->focusIndex = count - 1;

									offset -= base_size;
									coverFlow->touchPos += base_size;
								}

								if (offset <= (base_size * -1))
								{
									if (coverFlow->focusIndex < (count - 1))
										coverFlow->focusIndex++;
									else
										coverFlow->focusIndex = 0;

									offset += base_size;
									coverFlow->touchPos -= base_size;
								}
							}

							if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
							{
								int index, count2;

								count2 = count / 2 + 1;
								index = coverFlow->focusIndex;

								for (i = 0; i < count2; ++i)
								{
									ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
									int fy = widget->rect.height / 2 - child->rect.height / 2;
									fy += i * child->rect.height;
									fy += offset;

									ituWidgetSetY(child, fy);

									if (index >= count - 1)
										index = 0;
									else
										index++;
								}

								count2 = count - count2;
								for (i = 0; i < count2; ++i)
								{
									ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
									int fy = widget->rect.height / 2 - child->rect.height / 2;
									fy -= count2 * child->rect.height;
									fy += i * child->rect.height;
									fy += offset;

									ituWidgetSetY(child, fy);

									if (index >= count - 1)
										index = 0;
									else
										index++;
								}
							}
							else
							{
								//limit the move under non-cycle/Vertical boundaryAlign mode
								int fy = 0;
								bool b_touch = false;

								if (coverFlow->boundaryAlign)
								{
									ITUWidget* child_1 = CoverFlowGetVisibleChild(coverFlow, 0);
									ITUWidget* child_2 = CoverFlowGetVisibleChild(coverFlow, count - 1);
									int child_height = child_1->rect.height;

									if ((child_1->rect.y + coverFlow->overlapsize) > 0)
									{
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE1;
										b_touch = true;
									}
									else if ((child_2->rect.y + coverFlow->overlapsize + base_size) < widget->rect.height)
									{
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE2;
										b_touch = true;
									}
								}

								if (coverFlow->focusIndex <= 0)
								{
									if (coverFlow->bounceRatio > 0)
									{
										if (offset >= base_size / coverFlow->bounceRatio)
											offset = base_size / coverFlow->bounceRatio;
									}
									else
									{
										offset = 0;
									}
								}
								else if (coverFlow->focusIndex >= count - 1)
								{
									if (coverFlow->bounceRatio > 0)
									{
										if (offset <= -base_size / coverFlow->bounceRatio)
											offset = -base_size / coverFlow->bounceRatio;
									}
									else
									{
										offset = 0;
									}
								}

								for (i = 0; i < count; ++i)
								{//[MOVE][Vertical][non-cycle][layout]
									ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
									//int fy = widget->rect.height / 2 - child->rect.height / 2;


									if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
									{
										if (i == 0)
										{
											ITUWidget* ccc = CoverFlowGetVisibleChild(coverFlow, 0);
											fy = ccc->rect.y + offset;

											if (!b_touch)
												coverFlow->touchPos = y;
										}
									}
									else
									{
										fy = widget->rect.height / 2 - base_size / 2;
									}

									if (coverFlow->boundaryAlign && b_touch)
									{
										if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
										{
											widget->flags |= ITU_UNDRAGGING;
											ituScene->dragged = NULL;

											if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE1) && (offset > 0))
											{
												break;
											}
											else if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE2) && (offset < 0))
											{
												break;
											}
										}
										else
											break;
									}

									if (!((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP)))
									{
										if (coverFlow->boundaryAlign)
										{
											int max_neighbor_item = ((widget->rect.height / base_size) - 1) / 2;
											int max_height_item = widget->rect.height / base_size;
											fy = 0;

											if (max_neighbor_item == 0)
												max_neighbor_item++;

											if (coverFlow->focusIndex > 0)//>= max_neighbor_item) //Bless debug now
											{
												//if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_neighbor_item))
												if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_height_item))
													fy = widget->rect.height - (count * base_size);
												else
													fy -= base_size * coverFlow->focusIndex;
											}
											else
												fy = 0;
										}
										else
										{
											fy -= base_size * coverFlow->focusIndex;
										}
									}

									if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
									{
										//fy += offset;
										ituWidgetSetY(child, fy + (i * child->rect.height));
									}
									else
									{
										if (coverFlow->overlapsize > 0)
										{
											fy += i * base_size;
											fy += offset;
											ituWidgetSetY(child, fy - coverFlow->overlapsize);
										}
										else
										{
											fy += i * child->rect.height;
											fy += offset;
											ituWidgetSetY(child, fy);
										}
									}

									if (i == 0)
									{
										if (coverFlow->overlapsize > 0)
											coverFlow->movelog = fy - coverFlow->overlapsize;
										else
											coverFlow->movelog = fy;
									}
								}
							}
						}
						else
						{
							offset = x - coverFlow->touchPos;
							//printf("0: offset=%d\n", offset);
							if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
							{
								if (offset >= base_size)
								{
									if (coverFlow->focusIndex > 0)
										coverFlow->focusIndex--;
									else
										coverFlow->focusIndex = count - 1;

									offset -= base_size;
									coverFlow->touchPos += base_size;
								}

								if (offset <= (base_size * -1))
								{
									if (coverFlow->focusIndex < (count - 1))
										coverFlow->focusIndex++;
									else
										coverFlow->focusIndex = 0;

									offset += base_size;
									coverFlow->touchPos -= base_size;
								}
							}

							if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
							{
								int index, count2;
								//workaround for wrong left-side display with hide child
								count2 = count / 2 + 1 - ((offset>0) ? (1) : (0));
								//count2 = count / 2 + 1;
								index = coverFlow->focusIndex;

								for (i = 0; i < count2; ++i)
								{
									ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
									int fx = widget->rect.width / 2 - child->rect.width / 2;
									fx += i * child->rect.width;
									fx += offset;

									ituWidgetSetX(child, fx);

									if (index >= count - 1)
										index = 0;
									else
										index++;
								}

								count2 = count - count2;
								for (i = 0; i < count2; ++i)
								{
									ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
									int fx = widget->rect.width / 2 - child->rect.width / 2;
									fx -= count2 * child->rect.width;
									fx += i * child->rect.width;
									fx += offset;

									ituWidgetSetX(child, fx);

									if (index >= count - 1)
										index = 0;
									else
										index++;
								}
							}
							else
							{
								//limit the move under non-cycle/Horizontal boundaryAlign mode
								int fx = 0;
								bool b_touch = false;

								if (coverFlow->boundaryAlign)
								{
									ITUWidget* child_1 = CoverFlowGetVisibleChild(coverFlow, 0);
									ITUWidget* child_2 = CoverFlowGetVisibleChild(coverFlow, count - 1);
									//int child_width = child_1->rect.width;

									if ((child_1->rect.x + coverFlow->overlapsize) > 0)
									{
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE1;
										b_touch = true;
									}
									else if ((child_2->rect.x + coverFlow->overlapsize + base_size) < widget->rect.width)
									{
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE2;
										b_touch = true;
									}
								}

								if (coverFlow->focusIndex <= 0)
								{
									ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, 0);
									if (coverFlow->bounceRatio > 0)
									{
										if (offset >= base_size / coverFlow->bounceRatio)
											offset = base_size / coverFlow->bounceRatio;
									}
									else
									{
										offset = 0;
									}
								}
								else if (coverFlow->focusIndex >= count - 1)
								{
									ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, 0);
									if (coverFlow->bounceRatio > 0)
									{
										if (offset <= -base_size / coverFlow->bounceRatio)
											offset = -base_size / coverFlow->bounceRatio;
									}
									else
									{
										offset = 0;
									}
								}



								for (i = 0; i < count; ++i)
								{
									ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
									//int fx = widget->rect.width / 2 - child->rect.width / 2;

									if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
									{
										if (i == 0)
										{
											ITUWidget* ccc = CoverFlowGetVisibleChild(coverFlow, 0);
											fx = ccc->rect.x + offset;

											if (!b_touch)
												coverFlow->touchPos = x;
										}
									}
									else
									{
										fx = widget->rect.width / 2 - base_size / 2;
									}

									if (coverFlow->boundaryAlign && b_touch)
									{
										if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
										{
											widget->flags |= ITU_UNDRAGGING;
											ituScene->dragged = NULL;

											if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE1) && (offset > 0))
											{
												break;
											}
											else if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE2) && (offset < 0))
											{
												break;
											}
										}
										else
											break;
									}

									if (!((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP)))
									{
										if (coverFlow->boundaryAlign)
										{//[MOVE][Horizontal][non-cycle][layout]
											int max_neighbor_item = ((widget->rect.width / base_size) - 1) / 2;
											int max_width_item = widget->rect.width / base_size;
											fx = 0;

											if (max_neighbor_item == 0)
												max_neighbor_item++;



											if (coverFlow->focusIndex > 0) //>= max_neighbor_item) //Bless debug now
											{
												//if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_neighbor_item))
												if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_width_item))
													fx = widget->rect.width - (count * base_size);
												else
													fx -= base_size * coverFlow->focusIndex;
											}
											else
												fx = 0;
										}
										else
										{
											fx -= base_size * coverFlow->focusIndex;
										}
									}

									if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
									{
										ituWidgetSetX(child, fx + (i * child->rect.width));
									}
									else
									{
										//for powei
										if (coverFlow->overlapsize > 0)
										{
											fx += i * base_size;
											fx += offset;
											ituWidgetSetX(child, fx - coverFlow->overlapsize);
										}
										else
										{
											fx += i * child->rect.width;
											fx += offset;
											ituWidgetSetX(child, fx);
										}
									}
									if (i == 0)
									{
										if (coverFlow->overlapsize > 0)
											coverFlow->movelog = fx - coverFlow->overlapsize;
										else
											coverFlow->movelog = fx;
									}
								}
							}
						}
						result = true;
					}
                }
				else
				{
					ituWidgetUpdate(coverFlow, ITU_EVENT_MOUSEUP, arg1, arg2, arg3);
					return true;
				}
            }
        }
        else if (ev == ITU_EVENT_MOUSEDOWN)
        {
			int x = arg2 - widget->rect.x;
			int y = arg3 - widget->rect.y;

			coverFlow->clock = itpGetTickCount();

			if (ituWidgetIsInside(widget, x, y))
			{
				if (ituScene->dragged != NULL)
				{
					printf("other widget dragging, name %s\n", widget->name);
					return true;
				}
				//else
				//	printf("name %s\n", widget->name);
			}

			if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
				coverFlow->frame = coverFlow->totalframe;

			if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGABLE))
			{
				ITUWidget* c1 = CoverFlowGetVisibleChild(coverFlow, 0);

				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
				{
					mousedown_position = c1->rect.y;
				}
				else
				{
					mousedown_position = c1->rect.x;
				}
			}

            if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGABLE) && coverFlow->bounceRatio > 0)
            {
                if (ituWidgetIsInside(widget, x, y))
                {
                    if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
                    {
						//if ((!(coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP)) || (coverFlow->frame == 0))
							coverFlow->touchPos = y;
                    }
                    else
                    {
						//if ((!(coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP)) || (coverFlow->frame == 0))
							coverFlow->touchPos = x;
                    }

                    if (widget->flags & ITU_HAS_LONG_PRESS)
                    {
                        coverFlow->touchCount = 1;
                    }
                    else
                    {
                        widget->flags |= ITU_DRAGGING;
                        ituScene->dragged = widget;
                    }
                    //result = true;
                }
            }
        }
        else if (ev == ITU_EVENT_MOUSEUP)
        {
			if ((!(coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)) && coverFlow->boundaryAlign && (coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP))
			{ //fix the error position when drag too much outside or too fast then mouse up
				int count = CoverFlowGetVisibleChildCount(coverFlow);
				int i = 0;
				int fd = 0;
				int move_step = 0;
				ITUWidget* child_1 = CoverFlowGetVisibleChild(coverFlow, 0);
				ITUWidget* child_2 = CoverFlowGetVisibleChild(coverFlow, count - 1);

				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
				{
					if ((child_1->rect.y > 0) || ((child_2->rect.y + child_2->rect.height) < widget->rect.height))
					{
						if (child_1->rect.y > 0)
							move_step = 0 - child_1->rect.y;
						else
							move_step = widget->rect.height - (child_2->rect.y + child_2->rect.height);

						for (i = 0; i < count; i++)
						{
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
							fd = child->rect.y;
							fd += move_step;
							ituWidgetSetY(child, fd);
						}

						coverFlow->frame = 0;
					}
				}
				else
				{
					if ((child_1->rect.x > 0) || ((child_2->rect.x + child_2->rect.width) < widget->rect.width))
					{
						if (child_1->rect.x > 0)
							move_step = 0 - child_1->rect.x;
						else
							move_step = widget->rect.width - (child_2->rect.x + child_2->rect.width);

						for (i = 0; i < count; i++)
						{
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
							fd = child->rect.x;
							fd += move_step;
							ituWidgetSetX(child, fd);
						}

						coverFlow->frame = 0;
					}
				}
			}

			if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGABLE) && ((widget->flags & ITU_DRAGGING)) 
				&& (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP)))
			{
                int count = CoverFlowGetVisibleChildCount(coverFlow);
                int x = arg2 - widget->rect.x;
                int y = arg3 - widget->rect.y;

                result = false; //Bless debug

                if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
                {
                    if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
                    {
                        if (!result && count > 0)
                        {
                            ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, 0);

                            if (coverFlow->inc == 0)
                            {
                                int offset, absoffset, interval;
                                
                                offset = y - coverFlow->touchPos;
                                interval = offset / child->rect.height;
                                offset -= (interval * child->rect.height);
                                absoffset = offset > 0 ? offset : -offset;

                                if (absoffset > child->rect.height / 2)
                                {
									coverFlow->frame = (int)((float)coverFlow->totalframe * (float)absoffset / (float)child->rect.height);
                                    coverFlow->focusIndex -= interval;

                                    if (offset >= 0)
                                    {
                                        //coverFlow->inc = child->rect.height;
										coverFlow->focusIndex--;

                                        if (coverFlow->focusIndex < 0)
                                            coverFlow->focusIndex += count;

										ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
										LOG_DBG "1: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                    }
                                    else
                                    {
                                        //coverFlow->inc = -child->rect.height;
										coverFlow->focusIndex++;

                                        if (coverFlow->focusIndex >= count)
                                            coverFlow->focusIndex -= count;

										ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
										LOG_DBG "2: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                    }

									//try fix bug bbbbb
									coverFlow->inc = offset;
									/*while (coverFlow->inc > (base_size / 2))
									{
										coverFlow->inc -= base_size;
									}
									while (coverFlow->inc < (-1 * base_size / 2))
									{
										coverFlow->inc += base_size;
									}*/
                                }
                                else if (absoffset)
                                {
									coverFlow->frame = coverFlow->totalframe - (int)((float)coverFlow->totalframe * (float)absoffset / (float)child->rect.height);

                                    if (offset >= 0)
                                    {
                                        //coverFlow->inc = -child->rect.height;
										coverFlow->focusIndex -= interval;// +1;

                                        if (coverFlow->focusIndex < 0)
                                            coverFlow->focusIndex += count;

										LOG_DBG "3: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                    }
                                    else
                                    {
                                        //coverFlow->inc = child->rect.height;
										coverFlow->focusIndex -= interval;// -1;

                                        if (coverFlow->focusIndex >= count)
                                            coverFlow->focusIndex -= count;

										LOG_DBG "4: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                    }

									coverFlow->inc = offset;
                                }
                            }
                            widget->flags |= ITU_UNDRAGGING;
                            ituScene->dragged = NULL;
                        }
                    }
                    else
                    {
						if (!result && count > 0)
                        {//[MOUSEUP][Vertical][non-cycle][layout]
                            //ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, 0);
							bool boundary_touch = false;
							//[MOUSEUP][Vertical][non-cycle][layout]
							if (coverFlow->boundaryAlign)
							{
								int max_neighbor_item = ((widget->rect.height / base_size) - 1) / 2;
								int max_height_item = widget->rect.height / base_size;

								if (max_neighbor_item == 0)
									max_neighbor_item++;

								if (coverFlow->focusIndex >= max_neighbor_item)
								{
									if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_neighbor_item))
										boundary_touch = true;
									else
									{
										if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_height_item))
											boundary_touch = true;
									}
								}
								else
									boundary_touch = true;
							}

                            if (coverFlow->inc == 0)
                            {
                                int offset, absoffset, interval;
                                
                                //offset = y - coverFlow->touchPos;//PPAP
								offset = CoverFlowGetDraggingDist(coverFlow);
								interval = offset / base_size;
								offset -= (interval * base_size);
                                //absoffset = offset > 0 ? offset : -offset;

								//////////check bounce
								if (coverFlow->focusIndex <= 0)
								{
									if (coverFlow->bounceRatio > 0)
									{
										if (offset >= base_size / coverFlow->bounceRatio)
											offset = base_size / coverFlow->bounceRatio;
									}
									else
									{
										offset = 0;
									}
								}
								else if (coverFlow->focusIndex >= count - 1)
								{
									if (coverFlow->bounceRatio > 0)
									{
										if (offset <= -base_size / coverFlow->bounceRatio)
											offset = -base_size / coverFlow->bounceRatio;
									}
									else
									{
										offset = 0;
									}
								}
								absoffset = offset > 0 ? offset : -offset;

								if (absoffset > base_size / 2)
                                {//small shift alignment
									if (offset >= 0)
									{
										coverFlow->frame = (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);

										if (coverFlow->focusIndex > interval)
										{
											coverFlow->focusIndex -= interval + 1;
											if (boundary_touch)
											{
												while ((CoverFlowCheckBoundaryTouch(widget) == ITU_BOUNCE_2) && (coverFlow->focusIndex > 0))
												{
													coverFlow->focusIndex--;
												}
											}
											//if (boundary_touch)
											//	coverFlow->focusIndex = CoverFlowGetVisibleChildCount(coverFlow) - widget->rect.height / base_size;
											//coverFlow->inc = base_size;
											//coverFlow->focusIndex -= interval + 1;
											ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
											LOG_DBG "5: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
											//if (boundary_touch)
											//	coverFlow->focusIndex -= (interval != 0) ? (((offset >= 0) ? (1) : (-1))) : (((absoffset > child->rect.height / 2) ? (((offset >= 0) ? (1) : (-1))) : (0)));
										}
										else
										{
											//coverFlow->inc = -base_size;
											coverFlow->focusIndex = -1;
											LOG_DBG "6: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
										}
									}
									else
									{
										coverFlow->frame = (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);

										if (coverFlow->focusIndex < count + interval - 1)
										{
											//coverFlow->inc = -base_size;
											coverFlow->focusIndex -= interval - 1;
											ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
											LOG_DBG "7: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
											//if (boundary_touch)
											//	coverFlow->focusIndex -= (interval != 0) ? (((offset >= 0) ? (1) : (-1))) : (((absoffset > child->rect.height / 2) ? (((offset >= 0) ? (1) : (-1))) : (0)));
										}
										else
										{
											//coverFlow->inc = base_size;
											coverFlow->focusIndex = count;
											LOG_DBG "8: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
										}
									}
									//try fix bug bbbbb
									coverFlow->inc = offset;
									/*while (coverFlow->inc > (base_size / 2))
									{
										coverFlow->inc -= base_size;
									}
									while (coverFlow->inc < (-1 * base_size / 2))
									{
										coverFlow->inc += base_size;
									}*/
                                }
								else if (absoffset)
								{
									//try debug 0911
									//coverFlow->frame = coverFlow->totalframe - (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
									coverFlow->frame = (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);

									if (offset >= 0)
									{//big shift alignment
										//if ((boundary_touch) && (coverFlow->focusIndex > 0))
										//	coverFlow->focusIndex = CoverFlowGetVisibleChildCount(coverFlow) - widget->rect.height / base_size;

										//coverFlow->inc = -base_size;
										int lastf = coverFlow->focusIndex;
										coverFlow->focusIndex -= interval;

										//if (boundary_touch)
										//	coverFlow->focusIndex -= (interval != 0) ? (((offset >= 0) ? (1) : (-1))) : (((absoffset > base_size / 2) ? (((offset >= 0) ? (1) : (-1))) : (0)));

										//small shift alignment
										if (boundary_touch)
										{
											while ((CoverFlowCheckBoundaryTouch(widget) == ITU_BOUNCE_2) && (coverFlow->focusIndex > 0))
											{
												coverFlow->focusIndex--;
											}
										}

										//if (coverFlow->focusIndex < -1)
										//	coverFlow->focusIndex = -1;
										if (coverFlow->focusIndex < 0)
											coverFlow->focusIndex = 0;

										if (lastf != coverFlow->focusIndex)
											ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
										LOG_DBG "9: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
									}
									else
									{
										//coverFlow->inc = base_size;
										int lastf = coverFlow->focusIndex;
										coverFlow->focusIndex -= interval;

										if (boundary_touch)
											coverFlow->focusIndex -= (interval != 0) ? (((offset >= 0) ? (1) : (-1))) : (((absoffset > base_size / 2) ? (((offset >= 0) ? (1) : (-1))) : (0)));

										if (coverFlow->focusIndex >= count + 1)
											coverFlow->focusIndex = count;

										if (lastf != coverFlow->focusIndex)
											ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
										LOG_DBG "10: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
									}
									coverFlow->inc = offset;
								}
                            }
                        }
                        widget->flags |= ITU_UNDRAGGING;
                        ituScene->dragged = NULL;
                    }
                }
                else
                {
                    if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
                    {
                        if (count > 0)//bless
                        {
                            ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, 0);

                            if (coverFlow->inc == 0)
                            {
                                int offset, absoffset, interval;
                                
                                offset = x - coverFlow->touchPos;
                                interval = offset / child->rect.width;
                                offset -= (interval * child->rect.width);
                                absoffset = offset > 0 ? offset : -offset;

                                if (absoffset > child->rect.width / 2)
                                {
									coverFlow->frame = (int)((float)coverFlow->totalframe * (float)absoffset / (float)child->rect.width);
                                    //coverFlow->frame = absoffset / ((child->rect.width / coverFlow->totalframe) + 1);

                                    coverFlow->focusIndex -= interval;

                                    if (offset >= 0)
                                    {
                                        //coverFlow->inc = child->rect.width;
										coverFlow->focusIndex--;

                                        if (coverFlow->focusIndex < 0)
                                            coverFlow->focusIndex += count;

										ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
										LOG_DBG "1: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                    }
                                    else
                                    {
                                        //coverFlow->inc = -child->rect.width;
										coverFlow->focusIndex++;

                                        if (coverFlow->focusIndex >= count)
                                            coverFlow->focusIndex -= count;

										ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
										LOG_DBG "2: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                    }

									//try fix bug
									coverFlow->inc = offset;
									/*while (coverFlow->inc > (base_size / 2))
									{
										coverFlow->inc -= base_size;
									}
									while (coverFlow->inc < (-1 * base_size / 2))
									{
										coverFlow->inc += base_size;
									}*/
                                }
                                else if (absoffset)
                                {
									coverFlow->frame = coverFlow->totalframe - (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
                                    //coverFlow->frame = coverFlow->totalframe - absoffset / ((child->rect.width / coverFlow->totalframe) + 1);

                                    if (offset >= 0)
                                    {
                                        //coverFlow->inc = -child->rect.width;
										coverFlow->focusIndex -= interval;// +1;

                                        if (coverFlow->focusIndex < 0)
                                            coverFlow->focusIndex += count;

										LOG_DBG "3: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                    }
                                    else
                                    {
                                        //coverFlow->inc = child->rect.width;
										coverFlow->focusIndex -= interval;// -1;

                                        if (coverFlow->focusIndex >= count)
                                            coverFlow->focusIndex -= count;

										LOG_DBG "4: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                    }

									coverFlow->inc = offset;
                                }
                                widget->flags |= ITU_UNDRAGGING;
                                ituScene->dragged = NULL;
                            }
                        }
                    }
                    else
                    {
                        if (count > 0)
                        {
                            ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, 0);
							bool boundary_touch = false;
							//[MOUSEUP][Horizontal][non-cycle][layout]
							if (coverFlow->boundaryAlign)
							{
								int max_neighbor_item = ((widget->rect.width / base_size) - 1) / 2;
								int max_width_item = widget->rect.width / base_size;

								if (max_neighbor_item == 0)
									max_neighbor_item++;

								if (coverFlow->focusIndex >= max_neighbor_item)
								{
									if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_neighbor_item))
										boundary_touch = true;
									else
									{
										if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_width_item))
											boundary_touch = true;
									}
								}
								else
									boundary_touch = true;
							}

                            if (coverFlow->inc == 0)
                            {
                                int offset, absoffset, interval;
                                
                                //offset = x - coverFlow->touchPos;//PPAP
								offset = CoverFlowGetDraggingDist(coverFlow);
								interval = offset / base_size;
								offset -= (interval * base_size);
                                //absoffset = offset > 0 ? offset : -offset;

								///////////// check bounce
								if (coverFlow->focusIndex <= 0)
								{
									ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, 0);
									if (coverFlow->bounceRatio > 0)
									{
										if (offset >= base_size / coverFlow->bounceRatio)
											offset = base_size / coverFlow->bounceRatio;
									}
									else
									{
										offset = 0;
									}
								}
								else if (coverFlow->focusIndex >= count - 1)
								{
									ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, 0);
									if (coverFlow->bounceRatio > 0)
									{
										if (offset <= -base_size / coverFlow->bounceRatio)
											offset = -base_size / coverFlow->bounceRatio;
									}
									else
									{
										offset = 0;
									}
								}
								absoffset = offset > 0 ? offset : -offset;

								if (absoffset > (base_size / 2))
                                {//f1
									//coverFlow->frame = coverFlow->totalframe - absoffset / (base_size / coverFlow->totalframe);

                                    if (offset >= 0)
                                    {
										//coverFlow->frame = absoffset / (base_size / coverFlow->totalframe) + 1;
										//coverFlow->frame = ((absoffset * coverFlow->totalframe) / base_size) + 1;
										coverFlow->frame = (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);

                                        if (coverFlow->focusIndex > interval)
                                        {
											coverFlow->focusIndex -= interval + 1;
											//small shift alignment
											if (boundary_touch)
											{
												while ((CoverFlowCheckBoundaryTouch(widget) == ITU_BOUNCE_2) && (coverFlow->focusIndex > 0))
												{
													coverFlow->focusIndex--;
												}
											}

											//if (boundary_touch)
											//	coverFlow->focusIndex = 0; //CoverFlowGetVisibleChildCount(coverFlow) - widget->rect.width / base_size;
											
											//coverFlow->inc = base_size;
											ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
											LOG_DBG "5: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
											//if (boundary_touch)
											//	coverFlow->focusIndex -= (interval != 0) ? (((offset >= 0) ? (1) : (-1))) : (((absoffset > child->rect.width / 2) ? (((offset >= 0) ? (1) : (-1))) : (0)));
                                        }
                                        else
                                        {
											//coverFlow->inc = -base_size;
                                            coverFlow->focusIndex = -1;
											LOG_DBG "6: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                        }
                                    }
                                    else
                                    {
										//coverFlow->frame = absoffset / (base_size / coverFlow->totalframe) + 1;
										//coverFlow->frame = ((absoffset * coverFlow->totalframe) / base_size) + 1;
										coverFlow->frame = (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);

                                        if (coverFlow->focusIndex < count + interval - 1)
                                        {
											//coverFlow->inc = -base_size;
                                            coverFlow->focusIndex -= interval - 1;
											ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
											LOG_DBG "7: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
											//if (boundary_touch)
											//	coverFlow->focusIndex -= (interval != 0) ? (((offset >= 0) ? (1) : (-1))) : (((absoffset > child->rect.width / 2) ? (((offset >= 0) ? (1) : (-1))) : (0)));
                                        }
                                        else
                                        {
											//coverFlow->inc = base_size;
                                            coverFlow->focusIndex = count;
											LOG_DBG "8: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                        }
                                    }
									//try fix bug
									coverFlow->inc = offset;
									/*while (coverFlow->inc > (base_size / 2))
									{
										coverFlow->inc -= base_size;
									}
									while (coverFlow->inc < (-1 * base_size / 2))
									{
										coverFlow->inc += base_size;
									}*/
                                }
                                else if (absoffset)
                                {
									//try debug 0911
									//coverFlow->frame = coverFlow->totalframe - (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);
									coverFlow->frame = (int)((float)coverFlow->totalframe * (float)absoffset / (float)base_size);

                                    if (offset >= 0)
                                    {
										//big shift alignment
										//if ((boundary_touch) && (coverFlow->focusIndex > 0))
										//	coverFlow->focusIndex = CoverFlowGetVisibleChildCount(coverFlow) - widget->rect.width / base_size;
										
										//coverFlow->inc = -base_size;
										int lastf = coverFlow->focusIndex;
                                        coverFlow->focusIndex -= interval;

										//if (boundary_touch)
										//	coverFlow->focusIndex -= (interval != 0) ? (((offset >= 0) ? (1) : (-1))) : (((absoffset > child->rect.width / 2) ? (((offset >= 0) ? (1) : (-1))) : (0)));
										
										//small shift alignment
										if (boundary_touch)
										{
											while ((CoverFlowCheckBoundaryTouch(widget) == ITU_BOUNCE_2) && (coverFlow->focusIndex > 0))
											{
												coverFlow->focusIndex--;
											}
										}

                                        //if (coverFlow->focusIndex < -1)
                                        //    coverFlow->focusIndex = -1;
										if (coverFlow->focusIndex < 0)
											coverFlow->focusIndex = 0;

										if (lastf != coverFlow->focusIndex)
											ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
										LOG_DBG "9: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                    }
                                    else
                                    {
										//coverFlow->inc = base_size;
										int lastf = coverFlow->focusIndex;
                                        coverFlow->focusIndex -= interval;

										if (boundary_touch)
											coverFlow->focusIndex -= (interval != 0) ? (((offset >= 0) ? (1) : (-1))) : (((absoffset > base_size / 2) ? (((offset >= 0) ? (1) : (-1))) : (0)));

                                        if (coverFlow->focusIndex >= count + 1)
                                            coverFlow->focusIndex = count;

										if (lastf != coverFlow->focusIndex)
											ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
										LOG_DBG "10: frame=%d offset=%d inc=%d interval=%d focusIndex=%d\n", coverFlow->frame, offset, coverFlow->inc, interval, coverFlow->focusIndex LOG_END
                                    }
									coverFlow->inc = offset;
                                }

                                widget->flags |= ITU_UNDRAGGING;
                                ituScene->dragged = NULL;
                            }
                        }
                    }
                }
                result = true;
            }
            widget->flags &= ~ITU_DRAGGING;
            coverFlow->touchCount = 0;
        }
    }

    if (ev == ITU_EVENT_TIMER)
    {
        if (coverFlow->touchCount > 0)
        {
            int x, y, dist;

            assert(widget->flags & ITU_HAS_LONG_PRESS);

            ituWidgetGetGlobalPosition(widget, &x, &y);

            if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
            {
                dist = ituScene->lastMouseY - (y + coverFlow->touchPos);
            }
            else
            {
                dist = ituScene->lastMouseX - (x + coverFlow->touchPos);
            }

            if (dist < 0)
                dist = -dist;

            if (dist >= ITU_DRAG_DISTANCE)
            {
                widget->flags |= ITU_DRAGGING;
                ituScene->dragged = widget;
                coverFlow->touchCount = 0;
            }
        }

		if (coverFlow->inc)
        {
            int i, count = CoverFlowGetVisibleChildCount(coverFlow);
			int split_shift = 0;

			//try to fix dual bounce
			if ((widget->flags & ITU_BOUNCING) && ((coverFlow->coverFlowFlags & ITU_BOUNCE_1) && (coverFlow->coverFlowFlags & ITU_BOUNCE_2)))
			{
				ITUWidget* child_1 = CoverFlowGetVisibleChild(coverFlow, 0);

				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
				{
					if (child_1->rect.y < (0 - base_size))
						coverFlow->coverFlowFlags &= ~ITU_BOUNCE_1;
					else
						coverFlow->coverFlowFlags &= ~ITU_BOUNCE_2;
				}
				else
				{
					if (child_1->rect.x < (0 - base_size))
						coverFlow->coverFlowFlags &= ~ITU_BOUNCE_1;
					else
						coverFlow->coverFlowFlags &= ~ITU_BOUNCE_2;
				}

				//if ((coverFlow->coverFlowFlags & ITU_BOUNCE_1) && (coverFlow->coverFlowFlags & ITU_BOUNCE_2))
				//	coverFlow->coverFlowFlags &= ~ITU_BOUNCE_1;

			}

			if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
			{
				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
				{
					if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE1) || (coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE2))
					{
						coverFlow->frame = coverFlow->totalframe;
					}
					else
					{
						int fx = 0;
						int fy = 0;
						int move_step = 0;
						ITUWidget* child_1 = CoverFlowGetVisibleChild(coverFlow, 0);
						ITUWidget* child_2 = CoverFlowGetVisibleChild(coverFlow, count - 1);
						float step = (float)coverFlow->frame / (float)coverFlow->totalframe;
						//step = step * (float)M_PI / 2;
						//step = sinf(step);
						//move_step = (int)(coverFlow->inc * step);
						move_step = (int)(((float)coverFlow->inc / (float)COVERFLOW_FACTOR) * (float)slide_diff / 40.0);

						if (step <= COVERFLOW_PROCESS_STAGE1)
							move_step /= 3;
						else if (step <= COVERFLOW_PROCESS_STAGE2)
							move_step /= 6;
						else
							move_step /= 12;

						if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
						{
							if ((coverFlow->bounceRatio > 0) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE))
							{
								int tor = child_1->rect.height / coverFlow->bounceRatio;

								if ((child_1->rect.y + move_step) > tor)
								{
									move_step = tor - child_1->rect.y;
									coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE;
								}
								else if ((child_2->rect.y + child_2->rect.height + move_step) < (widget->rect.height - tor))
								{
									move_step = widget->rect.height - tor - (child_2->rect.y + child_2->rect.height);
									coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE;
								}
							}
							else
							{
								if (coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE)
								{
									int tor_step = ((child_1->rect.height / coverFlow->bounceRatio) / 10) + 1;
									move_step = (coverFlow->inc > 0) ? (-1 * tor_step) : (1 * tor_step);

									if (((child_1->rect.y + move_step) <= 0) && (child_1->rect.y > 0))
									{
										move_step = 0 - child_1->rect.y;
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE1;
									}
									else if (((child_2->rect.y + child_2->rect.height + move_step) >= widget->rect.height) && (child_2->rect.y < widget->rect.height))
									{
										move_step = widget->rect.height - (child_2->rect.y + child_2->rect.height);
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE2;
									}
								}
								else
								{
									if ((child_1->rect.y + move_step) > 0)
									{
										move_step = 0 - child_1->rect.y;
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE1;
									}
									else if ((child_2->rect.y + child_2->rect.height + move_step) < widget->rect.height)
									{
										move_step = widget->rect.height - (child_2->rect.y + child_2->rect.height);
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE2;
									}
								}
							}

							for (i = 0; i < count; i++)
							{
								ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
								fy = child->rect.y;

								if (coverFlow->frame > 0)
									fy += move_step;

								ituWidgetSetY(child, fy);
								//printf("[fy] %d [step] %.3f\n", fy, step);
							}

							//printf("[Frame %d]move_step %d\n", coverFlow->frame, move_step);
						}
						else
						{ //For Horizontal
							if ((coverFlow->bounceRatio > 0) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE))
							{
								int tor = child_1->rect.width / coverFlow->bounceRatio;

								if ((child_1->rect.x + move_step) > tor)
								{
									move_step = tor - child_1->rect.x;
									coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE;
								}
								else if ((child_2->rect.x + child_2->rect.width + move_step) < (widget->rect.width - tor))
								{
									move_step = widget->rect.width - tor - (child_2->rect.x + child_2->rect.width);
									coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE;
								}
							}
							else
							{
								if (coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE)
								{
									int tor_step = ((child_1->rect.width / coverFlow->bounceRatio) / 10) + 1;
									move_step = (coverFlow->inc > 0) ? (-1 * tor_step) : (1 * tor_step);

									if (((child_1->rect.x + move_step) < 0) && (child_1->rect.x > 0))
									{
										move_step = 0 - child_1->rect.x;
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE1;
									}
									else if (((child_2->rect.x + child_2->rect.width + move_step) > widget->rect.width) && (child_2->rect.x < widget->rect.width))
									{
										move_step = widget->rect.width - (child_2->rect.x + child_2->rect.width);
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE2;
									}
								}
								else
								{
									if ((child_1->rect.x + move_step) > 0)
									{
										move_step = 0 - child_1->rect.x;
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE1;
									}
									else if ((child_2->rect.x + child_2->rect.width + move_step) < widget->rect.width)
									{
										move_step = widget->rect.width - (child_2->rect.x + child_2->rect.width);
										coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYBOUNCE2;
									}
								}
							}

							for (i = 0; i < count; i++)
							{
								ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
								fx = child->rect.x;

								if (coverFlow->frame > 0)
									fx += move_step;

								ituWidgetSetX(child, fx);
								//printf("[fx] %d [step] %.3f\n", fx, step);
							}

							//printf("[Frame %d]move_step %d\n", coverFlow->frame, move_step);
						}
					}
				}
				else
					coverFlow->frame = coverFlow->totalframe;
			}
            else if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
            { // <<< ITU_EVENT_TIMER >>>//bless
				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
				{
					int index, count2;
					float step = (float)coverFlow->frame / (float)coverFlow->totalframe;
					int local_inc = coverFlow->inc;
					int local_fi = coverFlow->focusIndex;

					if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
					{
						if (coverFlow->touchPos != 0)
						{
							coverFlow->touchPos = 0;
							coverFlow->frame = 0;
							step = (float)coverFlow->frame / (float)coverFlow->totalframe;
						}
					}
					else
					{
						if (coverFlow->touchPos != 0)
						{
							coverFlow->touchPos = 0;
							coverFlow->frame = 0;
							step = (float)coverFlow->frame / (float)coverFlow->totalframe;
						}
						step = 1.0 - step;
					}

					while (local_inc > (base_size / 2))
					{
						local_inc -= base_size;
						//if (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING))
						//{
						//step = 1.0 - step;
						//local_fi--;
						//}
					}
					while (local_inc < (-1 * base_size / 2))
					{
						local_inc += base_size;
						//if (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING))
						//{
						//step = 1.0 - step;
						//local_fi++;
						//}
					}

					//bbbbb
					// cubic ease out: y = (x - 1)^3 + 1
					//step = step - 1;
					//step = step * step * step + 1;

					count2 = count / 2 + 1;
					index = local_fi;

					if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
					{
						local_inc = coverFlow->inc;
						for (i = 0; i < count2; ++i)
						{
							int ci = ((index >(count - 1)) ? (0) : (index));
							int fix = (int)(local_inc * step);
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, ci);
							int fy = widget->rect.height / 2 - base_size / 2;
							fy += i * base_size;

							if (((fy + fix) < child->rect.y) && (local_inc > 0))
							{
								int ff = 1;
								while ((fy + fix) < child->rect.y)
								{
									step = (float)(coverFlow->frame + ff) / (float)coverFlow->totalframe;
									fix = (int)(local_inc * step);
									ff++;
								}
								coverFlow->frame += (ff - 1);
								break;
							}
							else if (((fy + fix) > child->rect.y) && (local_inc < 0))
							{
								int ff = 1;
								while ((fy + fix) > child->rect.y)
								{
									step = (float)(coverFlow->frame + ff) / (float)coverFlow->totalframe;
									fix = (int)(local_inc * step);
									ff++;
								}
								coverFlow->frame += (ff - 1);
								break;
							}
							fy += fix;
							ituWidgetSetY(child, fy);
							index = ci + 1;
						}

						count2 = count - count2;
						index = local_fi - 1;
						for (i = 0; i < count2; ++i)
						{
							int ci = ((index < 0) ? (count - 1) : (index));
							int fix = (int)(local_inc * step);
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, ci);
							int fy = widget->rect.height / 2 - base_size / 2;
							fy -= base_size;
							fy -= i * base_size;
							fy += fix;
							ituWidgetSetY(child, fy);
							index = ci - 1;
						}
					}
					else
					{
						local_inc = coverFlow->inc;
						step = (float)coverFlow->frame / (float)coverFlow->totalframe;

						for (i = 0; i < count2; ++i)
						{
							int ci = ((index >(count - 1)) ? (0) : (index));
							int fix = (int)(local_inc * step);
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, ci);
							int pway = 1;
							bool layout_check = false;
							int fy = widget->rect.height / 2 - base_size / 2;
							fy += i * base_size;
							//debugging layout_check (something wrong)
							//if (((fy > 0) && (child->rect.y > 0)) || ((fy < 0) && (child->rect.y < 0)))
							//	layout_check = true;

							if (((fy + fix) > child->rect.y) && (local_inc > 0) && layout_check)
							{
								int ff = 1;
								while (((fy + fix) > child->rect.y) && ((coverFlow->frame + ff) < coverFlow->totalframe))
								{
									step = 1.0 - (float)(coverFlow->frame + ff) / (float)coverFlow->totalframe;
									fix = (int)(local_inc * step * pway);
									ff++;
								}
								coverFlow->frame += (ff - 1);
								step = 1.0 - (float)(coverFlow->frame) / (float)coverFlow->totalframe;
							}
							else if (((fy + fix) < child->rect.y) && (local_inc < 0) && layout_check)
							{
								int ff = 1;
								while (((fy + fix) < child->rect.y) && ((coverFlow->frame + ff) < coverFlow->totalframe))
								{
									step = 1.0 - (float)(coverFlow->frame + ff) / (float)coverFlow->totalframe;
									fix = (int)(local_inc * step * pway);
									ff++;
								}
								coverFlow->frame += (ff - 1);
								step = 1.0 - (float)(coverFlow->frame) / (float)coverFlow->totalframe;
							}

							fy += fix;
							ituWidgetSetY(child, fy);
							index = ci + 1;
						}

						count2 = count - count2;
						index = local_fi - 1;
						for (i = 0; i < count2; ++i)
						{
							int ci = ((index < 0) ? (count - 1) : (index));
							int fix = (int)(local_inc * step);
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, ci);
							int fy = widget->rect.height / 2 - base_size / 2;
							fy -= base_size;
							fy -= i * base_size;
							fy += fix;
							ituWidgetSetY(child, fy);
							index = ci - 1;
						}
					}
					result = true;
				}
                else if (widget->flags & ITU_BOUNCING)
                {
					// <<< ITU_EVENT_TIMER >>>
					// <<< Vertical / Non-Cycle >>>
                    float step = (float)coverFlow->frame / (float)coverFlow->totalframe;
					step = step - 1;
					step = step * step * step + 1;

                    //printf("step=%f\n", step);

                    for (i = 0; i < count; ++i)
                    {
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);

						//int fy = widget->rect.height / 2 - base_size / 2;
						//fy -= base_size * coverFlow->focusIndex;
						//fy += i * base_size;

						int fy;
						if (coverFlow->coverFlowFlags & ITU_BOUNCE_1)
						{
							fy = 0;
						}
						else if (coverFlow->coverFlowFlags & ITU_BOUNCE_2)
						{
							fy = widget_size - count * base_size;
						}
						else
						{
							if (coverFlow->focusIndex == 0)
								fy = 0;
							else
								fy = widget_size - (coverFlow->focusIndex + 1) * base_size;
						}

						fy += i * base_size;

						fy += (int)(coverFlow->inc * step - coverFlow->overlapsize);
						ituWidgetSetY(child, fy);
						//if (i == 0)
						//	printf("fy %d\n", fy);
                    }
                }
                else
                {
					// <<< ITU_EVENT_TIMER >>>
					// <<< Vertical / Non-Cycle >>>
					bool wrong_pos_check = true;

					//if (coverFlow->boundaryAlign == 0)
					//	wrong_pos_check = false;

					while (wrong_pos_check)
					{
						int way = (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING) ? (-1) : (1);
						float step = 0.0;
						wrong_pos_check = false;

						if (way > 0)
						{
							step = (float)coverFlow->frame / (float)coverFlow->totalframe;
						}
						else
						{
							step = (float)(coverFlow->totalframe - coverFlow->frame) / (float)coverFlow->totalframe;
						}
						step = step - 1;
						step = step * step * step + 1;

						if (coverFlow->focusIndex < 0)
							coverFlow->focusIndex = 0;
						else if (coverFlow->focusIndex > (count - 1))
							coverFlow->focusIndex = count - 1;

						//step *= widget->rect.height;

						//printf("step=%f\n", step);
						//printf("[inc] %d\n", coverFlow->inc);

						for (i = 0; i < count; ++i)
						{
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
							int fy = widget->rect.height / 2 - base_size / 2;

							if (coverFlow->boundaryAlign)
							{
								int max_neighbor_item = ((widget->rect.width / base_size) - 1) / 2;
								int max_width_item = (widget->rect.width / base_size);

								//fy = 0;
								fy = i * base_size;

								if (max_neighbor_item == 0)
									max_neighbor_item++;

								fy -= (base_size * coverFlow->focusIndex);
								//if (coverFlow->focusIndex == (count - 1))
								//{
								//	fy -= base_size * (coverFlow->focusIndex - 0);
								//}
								//else if (coverFlow->focusIndex >= 0) //>= max_neighbor_item)
								//{
								//	if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_width_item))
								//		fy = widget->rect.height - (count * base_size);
								//	else
								//		fy -= base_size * (coverFlow->focusIndex - 0);
								//}
								//else
								//	fy = 0;
							}
							else
							{
								//fy = base_size - (coverFlow->focusIndex * base_size);
								fy = ((widget_size - base_size) / 2) - ((coverFlow->focusIndex - i) * base_size);
							}




							if (coverFlow->overlapsize > 0)
							{
								int fix;
								if (way > 0)
								{
									fix = (base_size - (int)(base_size * step));
									fix *= ((coverFlow->inc > 0) ? (1) : (-1));
								}
								else
								{
									fix = (int)(base_size * step);
									fix *= ((coverFlow->inc > 0) ? (-1) : (1));
								}

								if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
								{
									if (coverFlow->inc > 0)
										fix += base_size;
									else
										fix -= base_size;
								}

								//fy += i * base_size;

								//fix the slide start position not sync move last position
								if ((i == 0) && (coverFlow->movelog != 0))
								{
									if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
									{
										if (coverFlow->inc > 0)
										{
											if ((fy + fix - coverFlow->overlapsize) < coverFlow->movelog)
												wrong_pos_check = true;
										}
										else
										{
											if ((fy + fix - coverFlow->overlapsize) > coverFlow->movelog)
												wrong_pos_check = true;
										}
									}
									else
									{
										if (coverFlow->inc > 0)
										{
											if ((fy + fix - coverFlow->overlapsize) > coverFlow->movelog)
												wrong_pos_check = true;
										}
										else
										{
											if ((fy + fix - coverFlow->overlapsize) < coverFlow->movelog)
												wrong_pos_check = true;
										}
									}

									if (!wrong_pos_check)
										coverFlow->movelog = 0;

									if (wrong_pos_check)
									{
										coverFlow->frame++;
										i = count;

										if (coverFlow->frame >= (coverFlow->totalframe + 1))
											wrong_pos_check = false;

										continue;
									}
								}

								//////////////////////////////////////////////
								///// Vertical-NonCycle Overlap Split mode
								//////////////////////////////////////////////

								if ((coverFlow->split > 0) && (abs(coverFlow->inc) == base_size) && (coverFlow->frame > coverFlow->totalframe))
								{
									if (i == (count - 1))
									{
										coverFlow->frame = ((coverFlow->totalframe * COVERFLOW_MIN_SFRAME_PERCENT_SPLIT) / 100);

										i = count;
										continue;
									}
								}

								if (coverFlow->split > 0)
								{
									int sd = ((coverFlow->totalframe * 6) / 10);
									float dev = (coverFlow->totalframe - sd);

									float pi = 3.1415926;
									float pos_pi = ((float)(coverFlow->frame - sd) / dev) * pi;


									if ((coverFlow->frame >= sd) && (coverFlow->frame <= coverFlow->totalframe))
									{
										bool check_inside = false;

										if (((fy + fix) >= 0) && ((fy + fix) <= widget_size))
											check_inside = true;
										else if (((fy + fix + base_size) >= 0) && ((fy + fix + base_size) <= widget_size))
											check_inside = true;

										if (check_inside)
										{
											split_shift = (int)((i - coverFlow->focusIndex) * coverFlow->split * sin(pos_pi));

											if ((coverFlow->frame == coverFlow->totalframe) && (split_shift == 0))
											{
												split_shift = (int)(coverFlow->split * 0.3);
											}
										}

										//printf("split_shift %d, frame %d, inc %d\n", split_shift, coverFlow->frame, coverFlow->inc);
									}
									else
										split_shift = 0;
								}
								else
									split_shift = 0;

								if (coverFlow->frame <= coverFlow->totalframe)
								{
									ituWidgetSetY(child, fy + fix - coverFlow->overlapsize + split_shift);
									//printf("[frame %d][C %d][Y %d]\n", coverFlow->frame, i, fy + fix - coverFlow->overlapsize);
								}
							}
							else
							{
								int fix;
								if (way > 0)
								{
									fix = (base_size - (int)(base_size * step));
									fix *= ((coverFlow->inc > 0) ? (1) : (-1));
								}
								else
								{
									fix = (int)(base_size * step);
									fix *= ((coverFlow->inc > 0) ? (-1) : (1));
								}

								if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
								{
									if (coverFlow->inc > 0)
										fix += base_size;
									else
										fix -= base_size;
								}

								//fy += i * child->rect.height;

								//fix the slide start position not sync move last position
								if ((i == 0) && (coverFlow->movelog != 0))
								{
									if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
									{
										if (coverFlow->inc > 0)
										{
											if ((fy + fix) < coverFlow->movelog)
												wrong_pos_check = true;
										}
										else
										{
											if ((fy + fix) > coverFlow->movelog)
												wrong_pos_check = true;
										}
									}
									else
									{
										if (coverFlow->inc > 0)
										{
											if ((fy + fix) > coverFlow->movelog)
												wrong_pos_check = true;
										}
										else
										{
											if ((fy + fix) < coverFlow->movelog)
												wrong_pos_check = true;
										}
									}

									if (!wrong_pos_check)
										coverFlow->movelog = 0;

									if (wrong_pos_check) //bless debug for sc
									{
										coverFlow->frame++;
										i = count;

										if (coverFlow->frame >= (coverFlow->totalframe + 1))
											wrong_pos_check = false;

										continue;
									}
								}

								//////////////////////////////////////////////
								///// Vertical-NonCycle Split mode
								//////////////////////////////////////////////

								if ((coverFlow->split > 0) && (abs(coverFlow->inc) == base_size) && (coverFlow->frame > coverFlow->totalframe))
								{
									if (i == (count - 1))
									{
										coverFlow->frame = ((coverFlow->totalframe * COVERFLOW_MIN_SFRAME_PERCENT_SPLIT) / 100);

										i = count;
										continue;
									}
								}

								if (coverFlow->split > 0)
								{
									int sd = ((coverFlow->totalframe * 6) / 10);
									float dev = (coverFlow->totalframe - sd);

									float pi = 3.1415926;
									float pos_pi = ((float)(coverFlow->frame - sd) / dev) * pi;


									if ((coverFlow->frame >= sd) && (coverFlow->frame <= coverFlow->totalframe))
									{
										bool check_inside = false;

										if (((fy + fix) >= 0) && ((fy + fix) <= widget_size))
											check_inside = true;
										else if (((fy + fix + base_size) >= 0) && ((fy + fix + base_size) <= widget_size))
											check_inside = true;

										if (check_inside)
										{
											split_shift = (int)((i - coverFlow->focusIndex) * coverFlow->split * sin(pos_pi));

											if ((coverFlow->frame == coverFlow->totalframe) && (split_shift == 0))
											{
												split_shift = (int)(coverFlow->split * 0.3);
											}
										}

										//printf("split_shift %d, frame %d, inc %d\n", split_shift, coverFlow->frame, coverFlow->inc);
									}
									else
										split_shift = 0;
								}
								else
									split_shift = 0;

								if (coverFlow->frame <= coverFlow->totalframe)
								{
									ituWidgetSetY(child, fy + fix + split_shift);
									//printf("[frame %d][C %d][Y %d]\n", coverFlow->frame, i, fy + fix);
								}
							}
						}
					}
                }
            }
            else
            {
				// <<< ITU_EVENT_TIMER >>>
				// <<< Horizontal >>>//bless
				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
				{
					int index, count2;
					float step = (float)coverFlow->frame / (float)coverFlow->totalframe;
					int local_inc = coverFlow->inc;
					int local_fi = coverFlow->focusIndex;

					if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
					{
						if (coverFlow->touchPos != 0)
						{
							coverFlow->touchPos = 0;
							coverFlow->frame = 0;
							step = (float)coverFlow->frame / (float)coverFlow->totalframe;
						}
					}
					else
					{
						if (coverFlow->touchPos != 0)
						{
							coverFlow->touchPos = 0;
							coverFlow->frame = 0;
							step = (float)coverFlow->frame / (float)coverFlow->totalframe;
						}
						step = 1.0 - step;
					}

					while (local_inc > (base_size / 2))
					{
						local_inc -= base_size;
						//if (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING))
						//{
							//step = 1.0 - step;
							//local_fi--;
						//}
					}
					while (local_inc < (-1 * base_size / 2))
					{
						local_inc += base_size;
						//if (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING))
						//{
							//step = 1.0 - step;
							//local_fi++;
						//}
					}

					//bbbbb
					// cubic ease out: y = (x - 1)^3 + 1
					//step = step - 1;
					//step = step * step * step + 1;

					count2 = count / 2 + 1;
					index = local_fi;

					if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
					{
						local_inc = coverFlow->inc;
						for (i = 0; i < count2; ++i)
						{
							int ci = ((index >(count - 1)) ? (0) : (index));
							int fix = (int)(local_inc * step);
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, ci);
							int fx = widget->rect.width / 2 - base_size / 2;
							fx += i * base_size;

							if (((fx + fix) < child->rect.x) && (local_inc > 0))
							{
								int ff = 1;
								while ((fx + fix) < child->rect.x)
								{
									step = (float)(coverFlow->frame + ff) / (float)coverFlow->totalframe;
									fix = (int)(local_inc * step);
									ff++;
								}
								coverFlow->frame += (ff - 1);
								break;
							}
							else if (((fx + fix) > child->rect.x) && (local_inc < 0))
							{
								int ff = 1;
								while ((fx + fix) > child->rect.x)
								{
									step = (float)(coverFlow->frame + ff) / (float)coverFlow->totalframe;
									fix = (int)(local_inc * step);
									ff++;
								}
								coverFlow->frame += (ff - 1);
								break;
							}
							fx += fix;
							ituWidgetSetX(child, fx);
							index = ci + 1;
						}

						count2 = count - count2;
						index = local_fi - 1;
						for (i = 0; i < count2; ++i)
						{
							int ci = ((index < 0) ? (count - 1) : (index));
							int fix = (int)(local_inc * step);
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, ci);
							int fx = widget->rect.width / 2 - base_size / 2;
							fx -= base_size;
							fx -= i * base_size;
							fx += fix;
							ituWidgetSetX(child, fx);
							index = ci - 1;
						}
					}
					else
					{
						local_inc = coverFlow->inc;
						step = (float)coverFlow->frame / (float)coverFlow->totalframe;

						for (i = 0; i < count2; ++i)
						{
							int ci = ((index >(count - 1)) ? (0) : (index));
							int fix = (int)(local_inc * step);
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, ci);
							int pway = 1;
							bool layout_check = false;
							int fx = widget->rect.width / 2 - base_size / 2;
							fx += i * base_size;
							//debugging layout_check (something wrong)
							//if (((fx > 0) && (child->rect.x > 0)) || ((fx < 0) && (child->rect.x < 0)))
							//	layout_check = true;

							if (((fx + fix) > child->rect.x) && (local_inc > 0) && layout_check)
							{
								int ff = 1;
								while (((fx + fix) > child->rect.x) && ((coverFlow->frame + ff) < coverFlow->totalframe))
								{
									step = 1.0 - (float)(coverFlow->frame + ff) / (float)coverFlow->totalframe;
									fix = (int)(local_inc * step * pway);
									ff++;
								}
								coverFlow->frame += (ff - 1);
								step = 1.0 - (float)(coverFlow->frame) / (float)coverFlow->totalframe;
							}
							else if (((fx + fix) < child->rect.x) && (local_inc < 0) && layout_check)
							{
								int ff = 1;
								while (((fx + fix) < child->rect.x) && ((coverFlow->frame + ff) < coverFlow->totalframe))
								{
									step = 1.0 - (float)(coverFlow->frame + ff) / (float)coverFlow->totalframe;
									fix = (int)(local_inc * step * pway);
									ff++;
								}
								coverFlow->frame += (ff - 1);
								step = 1.0 - (float)(coverFlow->frame) / (float)coverFlow->totalframe;
							}

							fx += fix;
							ituWidgetSetX(child, fx);
							index = ci + 1;
						}

						count2 = count - count2;
						index = local_fi - 1;
						for (i = 0; i < count2; ++i)
						{
							int ci = ((index < 0) ? (count - 1) : (index));
							int fix = (int)(local_inc * step);
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, ci);
							int fx = widget->rect.width / 2 - base_size / 2;
							fx -= base_size;
							fx -= i * base_size;
							fx += fix;
							ituWidgetSetX(child, fx);
							index = ci - 1;
						}
					}
					result = true;
				}
				else if (widget->flags & ITU_BOUNCING)
                {
					// <<< ITU_EVENT_TIMER >>>
					// <<< Horizontal / Non-Cycle >>>
					float step = (float)coverFlow->frame / (float)coverFlow->totalframe;
					step = step - 1;
					step = step * step * step + 1;

                    //printf("frame=%d step=%f\n", coverFlow->frame, step);
                    for (i = 0; i < count; ++i)
                    {
                        ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
                        //int fx = widget->rect.width / 2 - base_size / 2;
						//fx -= base_size * coverFlow->focusIndex;
						int fx;
						if (coverFlow->coverFlowFlags & ITU_BOUNCE_1)
						{
							fx = 0;
						}
						else if (coverFlow->coverFlowFlags & ITU_BOUNCE_2)
						{
							fx = widget_size - (count * base_size);
						}
						else
						{
							if (coverFlow->focusIndex == 0)
								fx = 0;
							else
								fx = widget_size - (coverFlow->focusIndex + 1) * base_size;
						}


						fx += i * base_size;
                        fx += (int)(coverFlow->inc * step);

						ituWidgetSetX(child, fx - coverFlow->overlapsize);
                    }
                }
                else
                {
					// <<< ITU_EVENT_TIMER >>>
					// <<< Horizontal / Non-Cycle >>>
					bool wrong_pos_check = true;

					//if (coverFlow->boundaryAlign == 0)
					//	wrong_pos_check = false;

					while (wrong_pos_check)
					{
						int way = (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING) ? (-1) : (1);
						float step = 0.0;
						wrong_pos_check = false;

						if (way > 0)
						{
							step = (float)coverFlow->frame / (float)coverFlow->totalframe;
						}
						else
						{
							step = (float)(coverFlow->totalframe - coverFlow->frame) / (float)coverFlow->totalframe;
						}
						step = step - 1;
						step = step * step * step + 1;

						if (coverFlow->focusIndex < 0)
							coverFlow->focusIndex = 0;
						else if (coverFlow->focusIndex > (count - 1))
							coverFlow->focusIndex = count - 1;

						for (i = 0; i < count; ++i)
						{
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
							int fx = widget->rect.width / 2 - base_size / 2;

							if (coverFlow->boundaryAlign) //(1)
							{
								int max_neighbor_item = ((widget->rect.width / base_size) - 1) / 2;
								int max_width_item = (widget->rect.width / base_size);

								//fx = 0;
								fx = i * base_size;

								if (max_neighbor_item == 0)
									max_neighbor_item++;

								fx -= (base_size * coverFlow->focusIndex);
								//if (coverFlow->focusIndex == (count - 1))
								//{
								//	fx -= base_size * (coverFlow->focusIndex - 0);
								//}
								//else if (coverFlow->focusIndex >= 0)     //>= max_neighbor_item) //Bless debug now
								//{
								//	if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_width_item))
								//		fx = widget->rect.width - (count * base_size);
								//	else
								//		fx -= base_size * (coverFlow->focusIndex - 0);
								//}
								//else
								//	fx = 0;
							}
							else
							{
								//fx = base_size - (coverFlow->focusIndex * base_size);
								fx = ((widget_size - base_size) / 2) - ((coverFlow->focusIndex - i) * base_size);
							}




							if (coverFlow->overlapsize > 0)
							{
								int fix;

								if (way > 0)
								{
									fix = (base_size - (int)(base_size * step));
									fix *= ((coverFlow->inc > 0) ? (1) : (-1));
								}
								else
								{
									fix = (int)(base_size * step);
									fix *= ((coverFlow->inc > 0) ? (-1) : (1));
								}

								if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
								{
									if (coverFlow->inc > 0)
										fix += base_size;
									else
										fix -= base_size;
								}

								//fx += i * base_size;

								//fix the slide start position not sync move last position
								if ((i == 0) && (coverFlow->movelog != 0))
								{
									if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
									{
										if (coverFlow->inc > 0)
										{
											if ((fx + fix - coverFlow->overlapsize) < coverFlow->movelog)
												wrong_pos_check = true;
										}
										else
										{
											if ((fx + fix - coverFlow->overlapsize) > coverFlow->movelog)
												wrong_pos_check = true;
										}
									}
									else
									{
										if (coverFlow->inc > 0)
										{
											if ((fx + fix - coverFlow->overlapsize) > coverFlow->movelog)
												wrong_pos_check = true;
										}
										else
										{
											if ((fx + fix - coverFlow->overlapsize) < coverFlow->movelog)
												wrong_pos_check = true;
										}
									}
									//0607
									//if (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING))
									//	wrong_pos_check = false;

									if (!wrong_pos_check)
										coverFlow->movelog = 0;

									if (wrong_pos_check)
									{
										coverFlow->frame++;
										i = count;

										if (coverFlow->frame >= (coverFlow->totalframe + 1))
											wrong_pos_check = false;

										continue;
									}
								}

								//////////////////////////////////////////////
								///// Horizontal-NonCycle Overlap Split mode
								//////////////////////////////////////////////

								if ((coverFlow->split > 0) && (abs(coverFlow->inc) == base_size) && (coverFlow->frame > coverFlow->totalframe))
								{
									if (i == (count - 1))
									{
										coverFlow->frame = ((coverFlow->totalframe * COVERFLOW_MIN_SFRAME_PERCENT_SPLIT) / 100);

										i = count;
										continue;
									}
								}

								if (coverFlow->split > 0)
								{
									int sd = ((coverFlow->totalframe * 6) / 10);
									float dev = (coverFlow->totalframe - sd);

									float pi = 3.1415926;
									float pos_pi = ((float)(coverFlow->frame - sd) / dev) * pi;


									if ((coverFlow->frame >= sd) && (coverFlow->frame <= coverFlow->totalframe))
									{
										bool check_inside = false;

										if (((fx + fix) >= 0) && ((fx + fix) <= widget_size))
											check_inside = true;
										else if (((fx + fix + base_size) >= 0) && ((fx + fix + base_size) <= widget_size))
											check_inside = true;

										if (check_inside)
										{
											split_shift = (int)((i - coverFlow->focusIndex) * coverFlow->split * sin(pos_pi));

											if ((coverFlow->frame == coverFlow->totalframe) && (split_shift == 0))
											{
												split_shift = (int)(coverFlow->split * 0.3);
											}
										}

										//printf("split_shift %d, frame %d, inc %d\n", split_shift, coverFlow->frame, coverFlow->inc);
									}
									else
										split_shift = 0;
								}
								else
									split_shift = 0;

								if (coverFlow->frame <= coverFlow->totalframe)
								{
									ituWidgetSetX(child, fx + fix - coverFlow->overlapsize + split_shift);
									//printf("[frame %d][C %d][X %d]\n", coverFlow->frame, i, fx + fix - coverFlow->overlapsize);
								}
							}
							else
							{
								int fix;

								if (way > 0)
								{
									fix = (base_size - (int)((float)(base_size)* step));
									fix *= ((coverFlow->inc > 0) ? (1) : (-1));
								}
								else
								{
									fix = (int)(base_size * step);
									fix *= ((coverFlow->inc > 0) ? (-1) : (1));
								}

								if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
								{
									if (coverFlow->inc > 0)
										fix += base_size;
									else
										fix -= base_size;
								}

								//fx += i * child->rect.width;

								//fix the slide start position not sync move last position
								if ((i == 0) && (coverFlow->movelog != 0))
								{
									if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
									{
										if (coverFlow->inc > 0)
										{
											if ((fx + fix) < coverFlow->movelog)
												wrong_pos_check = true;
										}
										else
										{
											if ((fx + fix) > coverFlow->movelog)
												wrong_pos_check = true;
										}
									}
									else
									{
										if (coverFlow->inc > 0)
										{
											if ((fx + fix) > coverFlow->movelog)
												wrong_pos_check = true;
										}
										else
										{
											if ((fx + fix) < coverFlow->movelog)
												wrong_pos_check = true;
										}
									}

									if (!wrong_pos_check)
										coverFlow->movelog = 0;

									if (wrong_pos_check) //bless debug for sc
									{
										coverFlow->frame++;
										i = count;

										if (coverFlow->frame >= (coverFlow->totalframe + 1))
											wrong_pos_check = false;

										continue;
									}
								}

								//////////////////////////////////////
								///// Horizontal-NonCycle Split mode
								//////////////////////////////////////

								if ((coverFlow->split > 0) && (abs(coverFlow->inc) == base_size) && (coverFlow->frame > coverFlow->totalframe))
								{
									if (i == (count - 1))
									{
										coverFlow->frame = ((coverFlow->totalframe * COVERFLOW_MIN_SFRAME_PERCENT_SPLIT) / 100);

										i = count;
										continue;
									}
								}

								if (coverFlow->split > 0)
								{
									int sd = ((coverFlow->totalframe * 6) / 10);
									float dev = (coverFlow->totalframe - sd);

									float pi = 3.1415926;
									float pos_pi = ((float)(coverFlow->frame - sd) / dev) * pi;


									if ((coverFlow->frame >= sd) && (coverFlow->frame <= coverFlow->totalframe))
									{
										bool check_inside = false;

										if (((fx + fix) >= 0) && ((fx + fix) <= widget_size))
											check_inside = true;
										else if (((fx + fix + base_size) >= 0) && ((fx + fix + base_size) <= widget_size))
											check_inside = true;

										if (check_inside)
										{
											split_shift = (int)((i - coverFlow->focusIndex) * coverFlow->split * sin(pos_pi));

											if ((coverFlow->frame == coverFlow->totalframe) && (split_shift == 0))
											{
												split_shift = (int)(coverFlow->split * 0.3);
												//printf("fixed!!!!!\n");
											}
										}

										//printf("split_shift %d, frame %d, inc %d\n", split_shift, coverFlow->frame, coverFlow->inc);
									}
									else
										split_shift = 0;
								}
								else
									split_shift = 0;

								if (coverFlow->frame <= coverFlow->totalframe)
								{
									ituWidgetSetX(child, fx + fix + split_shift);
									//printf("frame %d, split %d\n", coverFlow->frame, split_shift);
									//printf("[frame %d][C %d][X %d]\n", coverFlow->frame, i, fx + fix);
								}
							}
						}
					}
                }
            }

            coverFlow->frame++;
			//printf("coverflow frame %d  fd %d\n", coverFlow->frame, coverFlow->focusIndex);

			// <<< ITU_EVENT_TIMER >>>
			// <<< Frame END >>>
            if (coverFlow->frame > coverFlow->totalframe)
            {
				if (widget->flags & ITU_BOUNCING)
				{
					widget->flags &= ~ITU_BOUNCING;
					coverFlow->coverFlowFlags &= ~ITU_BOUNCE_1;
					coverFlow->coverFlowFlags &= ~ITU_BOUNCE_2;
				}

				if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
				{
					bool alignment_done = true;
					ITUWidget* child_1 = CoverFlowGetVisibleChild(coverFlow, 0);
					ITUWidget* child_2 = CoverFlowGetVisibleChild(coverFlow, count - 1);

					if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
					{
						if (child_1->rect.y > 0)
						{
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_ANYBOUNCE1;
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_ANYBOUNCE2;
							alignment_done = false;
							coverFlow->frame--;
						}
						else if ((child_2->rect.y + child_2->rect.height) < (widget->rect.height))
						{
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_ANYBOUNCE1;
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_ANYBOUNCE2;
							alignment_done = false;
							coverFlow->frame--;
						}
					}
					else
					{
						if ((child_1->rect.x) > 0)
						{
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_ANYBOUNCE1;
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_ANYBOUNCE2;
							alignment_done = false;
							coverFlow->frame--;
						}
						else if ((child_2->rect.x + child_2->rect.width) < widget->rect.width)
						{
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_ANYBOUNCE1;
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_ANYBOUNCE2;
							alignment_done = false;
							coverFlow->frame--;
						}
					}

					//to avoid bounce turn back not finish when frame end.
					if (alignment_done)
					{
						if (coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE)
						{
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_ANYBOUNCE;
						}

						if (coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE1)
						{
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_ANYBOUNCE1;
							coverFlow->focusIndex = 0;
						}
						else if (coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYBOUNCE2)
						{
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_ANYBOUNCE2;
							coverFlow->focusIndex = count - 1;
						}

						if (coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING)
						{
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_SLIDING;
							coverFlow->frame = 0;
							coverFlow->inc = 0;
						}
					}
				}
				//here two case should be debug long time
                else if (coverFlow->inc > 0)
                {
					// <<< ITU_EVENT_TIMER >>>
					// <<< Frame END >>>
					// <<< INC > 0 >>>
					if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING) || !(widget->flags & ITU_DRAGGABLE))// || (!coverFlow->boundaryAlign))
					{
						if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
						{
							if (coverFlow->focusIndex <= 0)
								coverFlow->focusIndex = count - 1;
							else
								coverFlow->focusIndex--;

							ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
							//ituCoverFlowOnCoverChanged(coverFlow, widget);
						}
						else
						{
							if (coverFlow->focusIndex > 0)
							{
								coverFlow->focusIndex--;
								ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
								//ituCoverFlowOnCoverChanged(coverFlow, widget);
							}
						}
					}
                }
                else
                {
					// <<< ITU_EVENT_TIMER >>>
					// <<< Frame END >>>
					// <<< INC < 0 >>>
					if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING) || !(widget->flags & ITU_DRAGGABLE))// || (!coverFlow->boundaryAlign))
					{
						if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
						{
							if (coverFlow->focusIndex >= count - 1)
								coverFlow->focusIndex = 0;
							else
								coverFlow->focusIndex++;

							ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
							//ituCoverFlowOnCoverChanged(coverFlow, widget);
						}
						else
						{
							if (coverFlow->focusIndex < (count - 1))
							{
								coverFlow->focusIndex++;
								ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
								//ituCoverFlowOnCoverChanged(coverFlow, widget);
							}
						}
					}
                }

				if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_SLIDING))// && coverFlow->boundaryAlign)
				{
					coverFlow->slideCount++;

					if (((coverFlow->slideCount + 1) >= coverFlow->slideMaxCount) || (coverFlow->focusIndex <= 0) || (coverFlow->focusIndex >= (count - 1)))
					{
						if (coverFlow->frame <= coverFlow->totalframe)
						{
							coverFlow->slideCount--;
						}
						else
						{
							coverFlow->slideCount = 0;
							coverFlow->coverFlowFlags &= ~ITU_COVERFLOW_SLIDING;

							//bless test 0818
							coverFlow->frame = 0;
							//coverFlow->inc = 0; //mark here to make sure queue will be clean
							//try to fix slide frame too short will stuck when sliding (17/09/04)
							ituWidgetUpdate(widget, ITU_EVENT_LAYOUT, 0, 0, 0);
						}
					}
				}
				else if (!(coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP))
				{
					// <<< ITU_EVENT_TIMER >>>
					// <<< QUEUE process >>>
					//long long *customdata = (long long *)ituWidgetGetCustomData(coverFlow);
					int i = 0;
					bool no_queue = true;
					coverFlow->frame = 0;
					//do not change here 20170920
					//ituExecActions(widget, coverFlow->actions, ITU_EVENT_CHANGED, coverFlow->focusIndex);
					ituCoverFlowOnCoverChanged(coverFlow, widget);

					for (i = 0; i < COVERFLOW_MAX_PROCARR_SIZE; i++)
					{
						if (coverFlow->procArr[i] != 0)
						{
							coverFlow->procArr[i] = 0;

							if ((i + 1) < COVERFLOW_MAX_PROCARR_SIZE)
							{
								if (coverFlow->procArr[i + 1] != 0)
									no_queue = false;
							}

							break;
						}
					}

					if (no_queue)
					{
						coverFlow->inc = 0;
						widget->flags &= ~ITU_UNDRAGGING;
						//mark now, debug side effect for wheel update
						ituWidgetUpdate(widget, ITU_EVENT_LAYOUT, 0, 0, 0);
						//CoverFlowLayout(widget);
					}
					else
					{
						bool boundary_touch = false;

						if (coverFlow->boundaryAlign)
						{
							int max_neighbor_item = ((widget_size / base_size) - 1) / 2;

							coverFlow->slideCount = 0;

							if (max_neighbor_item == 0)
								max_neighbor_item++;

							if (coverFlow->focusIndex >= max_neighbor_item)
							{
								if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_neighbor_item))
									boundary_touch = true;
								else
								{
									ITUWidget* cf = CoverFlowGetVisibleChild(coverFlow, count - 1);
									if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
									{
										if ((cf->rect.y + cf->rect.height) <= widget_size)
											boundary_touch = true;
									}
									else
									{
										if ((cf->rect.x + cf->rect.width) <= widget_size)
											boundary_touch = true;
									}
								}
							}
							else
								boundary_touch = true;
						}

						if (!boundary_touch)
						{
							if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
							{
								if (coverFlow->procArr[i + 1] < 0)
									ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDEDOWN, 0, widget->rect.x, widget->rect.y);
								else if (coverFlow->procArr[i + 1] > 0)
									ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDEUP, 0, widget->rect.x, widget->rect.y);
							}
							else
							{
								if (coverFlow->procArr[i + 1] < 0)
									ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDERIGHT, 0, widget->rect.x, widget->rect.y);
								else if (coverFlow->procArr[i + 1] > 0)
									ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDELEFT, 0, widget->rect.x, widget->rect.y);
							}
						}
					}

					ituScene->dragged = NULL;
				}
            }
        }
    }

    if (widget->flags & ITU_TOUCHABLE)
    {
		// <<< Default Event Clear >>>
        if (ev == ITU_EVENT_MOUSEDOWN || ev == ITU_EVENT_MOUSEUP || ev == ITU_EVENT_MOUSEDOUBLECLICK || 
			ev == ITU_EVENT_MOUSELONGPRESS || ev == ITU_EVENT_TOUCHSLIDELEFT || ev == ITU_EVENT_TOUCHSLIDEUP || 
			ev == ITU_EVENT_TOUCHSLIDERIGHT || ev == ITU_EVENT_TOUCHSLIDEDOWN)
        {
            if (ituWidgetIsEnabled(widget))
            {
                int x = arg2 - widget->rect.x;
                int y = arg3 - widget->rect.y;

                if (ituWidgetIsInside(widget, x, y))
                {
                    result |= widget->dirty;
                    return widget->visible ? result : false;
                }
            }
        }
    }

    if (ev == ITU_EVENT_LAYOUT)
    {
        int i, count = CoverFlowGetVisibleChildCount(coverFlow);

		if (count > 0)
		{
			//if (coverFlow->focusIndex >= count)
			//	coverFlow->focusIndex = count - 1;

			if (coverFlow->focusIndex > (count - 1))
				coverFlow->focusIndex = count - 1;
			else if (coverFlow->focusIndex < 0)
				coverFlow->focusIndex = 0;

			if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
			{
				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
				{
					int index, count2;

					count2 = count / 2 + 1;
					index = coverFlow->focusIndex;

					for (i = 0; i < count2; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
						int fy = widget->rect.height / 2 - child->rect.height / 2;
						fy += i * child->rect.height;
						ituWidgetSetY(child, fy);

						if (index >= count - 1)
							index = 0;
						else
							index++;
					}

					count2 = count - count2;
					for (i = 0; i < count2; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
						int fy = widget->rect.height / 2 - child->rect.height / 2;
						fy -= count2 * child->rect.height;
						fy += i * child->rect.height;
						ituWidgetSetY(child, fy);

						if (index >= count - 1)
							index = 0;
						else
							index++;
					}
				}
				else //[LAYOUT][Vertical][non-cycle]
				{
					for (i = 0; i < count; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
						int fy = widget->rect.height / 2 - base_size / 2;

						if (coverFlow->boundaryAlign)
						{
							int max_neighbor_item = ((widget->rect.height / base_size) - 1) / 2;
							int max_height_item = (widget->rect.height / base_size);
							fy = 0;

							if (max_neighbor_item == 0)
								max_neighbor_item++;

							if (coverFlow->focusIndex > 0) //>= max_neighbor_item) //Bless new debug
							{
								//if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_neighbor_item))
								if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_height_item))
									fy = widget->rect.height - (count * base_size);
								else
									fy -= base_size * coverFlow->focusIndex;
							}
							else
								fy = 0;
						}
						else
						{
							fy -= base_size * coverFlow->focusIndex;
						}

						if (coverFlow->overlapsize > 0)
						{
							fy += i * base_size;
							ituWidgetSetY(child, fy - coverFlow->overlapsize);
						}
						else
						{
							fy += i * child->rect.height;
							ituWidgetSetY(child, fy);
						}
					}
				}
			}
			else
			{
				//[LAYOUT][Horizontal][cycle]
				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
				{
					int index, count2;

					count2 = count / 2 + 1;
					index = coverFlow->focusIndex;

					for (i = 0; i < count2; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
						int fx = widget->rect.width / 2 - child->rect.width / 2;
						fx += i * child->rect.width;
						ituWidgetSetX(child, fx);

						if (index >= count - 1)
							index = 0;
						else
							index++;
					}

					count2 = count - count2;
					for (i = 0; i < count2; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, index);
						int fx = widget->rect.width / 2 - child->rect.width / 2;
						fx -= count2 * child->rect.width;
						fx += i * child->rect.width;
						ituWidgetSetX(child, fx);

						if (index >= count - 1)
							index = 0;
						else
							index++;
					}
				}
				else //[LAYOUT][Horizontal][non-cycle]
				{
					for (i = 0; i < count; ++i)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
						int fx = widget->rect.width / 2 - base_size / 2;

						if (coverFlow->boundaryAlign)
						{
							int max_neighbor_item = ((widget->rect.width / base_size) - 1) / 2;
							int max_width_item = (widget->rect.width / base_size);

							fx = 0;

							if (max_neighbor_item == 0)
								max_neighbor_item++;

							if (coverFlow->focusIndex > 0)
							{
								if ((count >= (max_neighbor_item * 2 + 1)) && ((count - coverFlow->focusIndex - 1) < max_width_item))
									fx = widget->rect.width - (count * base_size);
								else
									fx -= base_size * coverFlow->focusIndex;
							}
							else
								fx = 0;
						}
						else
						{
							fx -= base_size * coverFlow->focusIndex;
						}

						if (coverFlow->overlapsize > 0)
						{
							fx += i * base_size;
							ituWidgetSetX(child, fx - coverFlow->overlapsize);
						}
						else
						{
							fx += i * child->rect.width;
							ituWidgetSetX(child, fx);
						}
					}
				}
			}

			if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ENABLE_ALL) == 0)
			{
				for (i = 0; i < count; ++i)
				{
					ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);

					if (i == coverFlow->focusIndex)
						ituWidgetEnable(child);
					else
						ituWidgetDisable(child);
				}
			}
			widget->flags &= ~ITU_DRAGGING;
			coverFlow->touchCount = 0;

			//fix for stop anywhere not display after load
			if ((coverFlow->coverFlowFlags & ITU_COVERFLOW_ANYSTOP) && !(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
			{
				ITUWidget* widget = (ITUWidget*)coverFlow;
				int count = CoverFlowGetVisibleChildCount(coverFlow);
				int i = 0;
				int fd = 0;
				int move_step = 0;
				ITUWidget* child_1 = CoverFlowGetVisibleChild(coverFlow, 0);
				ITUWidget* child_2 = CoverFlowGetVisibleChild(coverFlow, count - 1);

				if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
				{
					if ((child_1->rect.y > 0) || ((child_2->rect.y + child_2->rect.height) < widget->rect.height))
					{
						if (child_1->rect.y > 0)
							move_step = 0 - child_1->rect.y;
						else
							move_step = widget->rect.height - (child_2->rect.y + child_2->rect.height);

						for (i = 0; i < count; i++)
						{
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
							fd = child->rect.y;
							fd += move_step;
							ituWidgetSetY(child, fd);
						}

						coverFlow->frame = 0;
					}
				}
				else
				{
					if ((child_1->rect.x > 0) || ((child_2->rect.x + child_2->rect.width) < widget->rect.width))
					{
						if (child_1->rect.x > 0)
							move_step = 0 - child_1->rect.x;
						else
							move_step = widget->rect.width - (child_2->rect.x + child_2->rect.width);

						for (i = 0; i < count; i++)
						{
							ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, i);
							fd = child->rect.x;
							fd += move_step;
							ituWidgetSetX(child, fd);
						}

						coverFlow->frame = 0;
					}
				}
			}
		}
    }

    result |= widget->dirty;
    return widget->visible ? result : false;
}

void ituCoverFlowDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    int destx, desty;
    uint8_t desta;
    ITURectangle prevClip;
    ITURectangle* rect = (ITURectangle*) &widget->rect;

    assert(widget);
    assert(dest);

    destx = rect->x + x;
    desty = rect->y + y;
    desta = alpha * widget->color.alpha / 255;
    desta = desta * widget->alpha / 255;
   
    ituWidgetSetClipping(widget, dest, x, y, &prevClip);

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
    ituFlowWindowDraw(widget, dest, x, y, alpha);
    ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
    ituDirtyWidget(widget, false);
}

void ituCoverFlowOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    ITUCoverFlow* coverFlow = (ITUCoverFlow*) widget;
    assert(coverFlow);

    switch (action)
    {
    case ITU_ACTION_PREV:
        ituCoverFlowPrev((ITUCoverFlow*)widget);
        break;

    case ITU_ACTION_NEXT:
        ituCoverFlowNext((ITUCoverFlow*)widget);
        break;

    case ITU_ACTION_GOTO:
        if (param[0] != '\0')
            ituCoverFlowGoto((ITUCoverFlow*)widget, atoi(param));
        break;

    case ITU_ACTION_DODELAY0:
        ituExecActions(widget, coverFlow->actions, ITU_EVENT_DELAY0, atoi(param));
        break;

    case ITU_ACTION_DODELAY1:
        ituExecActions(widget, coverFlow->actions, ITU_EVENT_DELAY1, atoi(param));
        break;

    case ITU_ACTION_DODELAY2:
        ituExecActions(widget, coverFlow->actions, ITU_EVENT_DELAY2, atoi(param));
        break;

    case ITU_ACTION_DODELAY3:
        ituExecActions(widget, coverFlow->actions, ITU_EVENT_DELAY3, atoi(param));
        break;

    case ITU_ACTION_DODELAY4:
        ituExecActions(widget, coverFlow->actions, ITU_EVENT_DELAY4, atoi(param));
        break;

    case ITU_ACTION_DODELAY5:
        ituExecActions(widget, coverFlow->actions, ITU_EVENT_DELAY5, atoi(param));
        break;

    case ITU_ACTION_DODELAY6:
        ituExecActions(widget, coverFlow->actions, ITU_EVENT_DELAY6, atoi(param));
        break;

    case ITU_ACTION_DODELAY7:
		if ((!(coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)) && (coverFlow->boundaryAlign))
			coverFlow->coverFlowFlags |= ITU_COVERFLOW_ANYSTOP;
        ituExecActions(widget, coverFlow->actions, ITU_EVENT_DELAY7, atoi(param));
        break;

    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

static void CoverFlowOnCoverChanged(ITUCoverFlow* coverFlow, ITUWidget* widget)
{
    // DO NOTHING
}

void ituCoverFlowInit(ITUCoverFlow* coverFlow, ITULayout layout)
{
    assert(coverFlow);
    ITU_ASSERT_THREAD();

    memset(coverFlow, 0, sizeof (ITUCoverFlow));

    if (layout == ITU_LAYOUT_VERTICAL)
        coverFlow->coverFlowFlags &= ITU_COVERFLOW_VERTICAL;

    ituFlowWindowInit(&coverFlow->fwin, layout);

    ituWidgetSetType(coverFlow, ITU_COVERFLOW);
    ituWidgetSetName(coverFlow, coverFlowName);
    ituWidgetSetUpdate(coverFlow, ituCoverFlowUpdate);
    ituWidgetSetDraw(coverFlow, ituCoverFlowDraw);
    ituWidgetSetOnAction(coverFlow, ituCoverFlowOnAction);
    ituCoverFlowSetCoverChanged(coverFlow, CoverFlowOnCoverChanged);
}

void ituCoverFlowLoad(ITUCoverFlow* coverFlow, uint32_t base)
{
    assert(coverFlow);

    ituFlowWindowLoad(&coverFlow->fwin, base);

    ituWidgetSetUpdate(coverFlow, ituCoverFlowUpdate);
    ituWidgetSetDraw(coverFlow, ituCoverFlowDraw);
    ituWidgetSetOnAction(coverFlow, ituCoverFlowOnAction);
    ituCoverFlowSetCoverChanged(coverFlow, CoverFlowOnCoverChanged);

	if (coverFlow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
		coverFlow->boundaryAlign = false;
	//else
	//	coverFlow->slideMaxCount = 1;

	//to avoid integer div zero
	if (coverFlow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
	{
		ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, 0);

		if (coverFlow->overlapsize > (child->rect.height * COVERFLOW_OVERLAP_MAX_PERCENTAGE / 100))
			coverFlow->overlapsize = child->rect.height * COVERFLOW_OVERLAP_MAX_PERCENTAGE / 100;

		//not support vertical yet
		coverFlow->overlapsize = 0;

		if (coverFlow->totalframe > child->rect.height)
			coverFlow->totalframe = child->rect.height;
	}
	else
	{
		ITUWidget* child = CoverFlowGetVisibleChild(coverFlow, 0);

		if (coverFlow->overlapsize > (child->rect.width * COVERFLOW_OVERLAP_MAX_PERCENTAGE / 100))
			coverFlow->overlapsize = child->rect.width * COVERFLOW_OVERLAP_MAX_PERCENTAGE / 100;

		if (coverFlow->totalframe > child->rect.width)
			coverFlow->totalframe = child->rect.width;
	}
}

void ituCoverFlowGoto(ITUCoverFlow* coverFlow, int index)
{
    assert(coverFlow);
    ITU_ASSERT_THREAD();

    if (coverFlow->focusIndex == index)
        return;
  
    coverFlow->focusIndex = index;
    ituWidgetUpdate(coverFlow, ITU_EVENT_LAYOUT, 0, 0, 0);
}

void ituCoverFlowPrev(ITUCoverFlow* coverflow)
{
    ITUWidget* widget = (ITUWidget*) coverflow;
    unsigned int oldFlags = widget->flags;

	//Bless added for PoWei requirement --> prev/next work queue
	int i = 0;
	bool no_queue = true;
    ITU_ASSERT_THREAD();

	for (i = COVERFLOW_MAX_PROCARR_SIZE - 1; i >= 0; i--)
	{
		//fix bug
		if ((!(coverflow->coverFlowFlags & ITU_COVERFLOW_CYCLE)) && (coverflow->boundaryAlign == 0))
		{
			if (coverflow->focusIndex <= 0)
			{
				no_queue = false;
				break;
			}
		}

		if ((i - 1) >= 0)
		{
			if (coverflow->procArr[i - 1] != 0)
			{
				coverflow->procArr[i] = -1;
				no_queue = false;
				break;
			}
		}
		else
			coverflow->procArr[i] = -1;
	}

    widget->flags |= ITU_TOUCHABLE;

	if (no_queue)
	{
		coverflow->touchPos = 0;

		if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
			ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDEDOWN, 0, widget->rect.x, widget->rect.y);
		else
			ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDERIGHT, 0, widget->rect.x, widget->rect.y);
	}

    if ((oldFlags & ITU_TOUCHABLE) == 0)
        widget->flags &= ~ITU_TOUCHABLE;
}

void ituCoverFlowNext(ITUCoverFlow* coverflow)
{
	ITUWidget* widget = (ITUWidget*)coverflow;
	unsigned int oldFlags = widget->flags;

	//Bless added for PoWei requirement --> prev/next work queue
	int i = 0;
	bool no_queue = true;
    ITU_ASSERT_THREAD();

	for (i = COVERFLOW_MAX_PROCARR_SIZE - 1; i >= 0; i--)
	{
		//fix bug
		if ((!(coverflow->coverFlowFlags & ITU_COVERFLOW_CYCLE)) && (coverflow->boundaryAlign == 0))
		{
			int count = CoverFlowGetVisibleChildCount(coverflow);

			if (coverflow->focusIndex >= (count - 1))
			{
				no_queue = false;
				break;
			}
		}

		if ((i - 1) >= 0)
		{
			if (coverflow->procArr[i - 1] != 0)
			{
				coverflow->procArr[i] = 1;
				no_queue = false;
				break;
			}
		}
		else
			coverflow->procArr[i] = 1;
	}

	widget->flags |= ITU_TOUCHABLE;

	if (no_queue)
	{
		coverflow->touchPos = 0;

		if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
			ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDEUP, 0, widget->rect.x, widget->rect.y);
		else
			ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDELEFT, 0, widget->rect.x, widget->rect.y);
	}

    if ((oldFlags & ITU_TOUCHABLE) == 0)
        widget->flags &= ~ITU_TOUCHABLE;
}


int CoverFlowGetFirstDisplayIndex(ITUCoverFlow* coverflow)
{
	assert(coverflow);
    ITU_ASSERT_THREAD();

	if (coverflow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
		return -1;
	else
	{
		int i, count = CoverFlowGetVisibleChildCount(coverflow);

		for (i = 0; i < count; i++)
		{
			ITUWidget* child = CoverFlowGetVisibleChild(coverflow, i);

			if (child)
			{
				if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
				{
					if ((child->rect.y + child->rect.height) > 0)
						return i;
				}
				else
				{
					if ((child->rect.x + child->rect.width) > 0)
						return i;
				}
			}
			else
				return -1;
		}

		return -1;
	}
}

int CoverFlowGetDraggingDist(ITUCoverFlow* coverflow)
{
	ITUWidget* widget = (ITUWidget*)coverflow;
    ITU_ASSERT_THREAD();

	if (!widget)
	{
		return 0;
	}
	else
	{
		if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGING))
		{
			ITUWidget* child = CoverFlowGetVisibleChild(coverflow, 0);
			int pos;

			if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
			{
				pos = child->rect.y;
			}
			else
			{
				pos = child->rect.x;
			}

			return (pos - mousedown_position);
		}
		else
		{
			return 0;
		}
	}
}
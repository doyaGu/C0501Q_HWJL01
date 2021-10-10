#include <assert.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

#define PAGEFLOW_3DEFFECT_RATIO 4

//DFrame factor (1.3 ~ 1.8)
#define DFRAME_FACTOR 1.4

static const char pageFlowName[] = "ITUPageFlow";

int PageFlowGetVisibleChildCount(ITUPageFlow* pageflow)
{
	int i = 0;
	ITCTree *child, *tree = (ITCTree*)pageflow;
	assert(tree);

	for (child = tree->child; child; child = child->sibling)
	{
		ITUWidget* widget = (ITUWidget*)child;
		if (widget->visible)
			i++;
	}
	return i;
}

ITUWidget* PageFlowGetVisibleChild(ITUPageFlow* pageflow, int index)
{
	int i = 0;
	ITCTree *child, *tree = (ITCTree*)pageflow;
	assert(tree);
    ITU_ASSERT_THREAD();

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

bool ituPageFlowUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUPageFlow* pageFlow = (ITUPageFlow*) widget;
    assert(pageFlow);

	if (pageFlow->effectRatio <= 0)
		pageFlow->effectRatio = PAGEFLOW_3DEFFECT_RATIO;

    if (widget->flags & ITU_TOUCHABLE)
    {
		if ((ev == ITU_EVENT_TOUCHSLIDELEFT) || (ev == ITU_EVENT_TOUCHSLIDERIGHT) || (ev == ITU_EVENT_TOUCHSLIDEUP) || (ev == ITU_EVENT_TOUCHSLIDEDOWN))
        {
			if (ituWidgetIsEnabled(widget))
            {
                int x = arg2 - widget->rect.x;
                int y = arg3 - widget->rect.y;

                if (ituWidgetIsInside(widget, x, y))
                {
                    ituUnPressWidget(widget);

					if (!(pageFlow->pageFlowFlags & ITU_PAGEFLOW_VERTICAL)) // For Horizontal
					{
						if (ev == ITU_EVENT_TOUCHSLIDELEFT)
						{
							int count = itcTreeGetChildCount(pageFlow);

							if (count > 0)
							{
								ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(pageFlow, 0);

								if (pageFlow->inc == 0)
								{
									pageFlow->frame = 0;
									pageFlow->inc = 0 - child->rect.width;
								}
							}
							//pageFlow->frame = 0;
						}
						else // if (ev == ITU_EVENT_TOUCHSLIDERIGHT)
						{
							int count = itcTreeGetChildCount(pageFlow);

							if (count > 0)
							{
								ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(pageFlow, 0);

								if (pageFlow->inc == 0)
								{
									pageFlow->frame = 0;
									pageFlow->inc = child->rect.width;
								}
							}
							//pageFlow->frame = 0;
						}
					}
					else // For Vertical
					{
						if (ev == ITU_EVENT_TOUCHSLIDEUP)
						{
							int count = itcTreeGetChildCount(pageFlow);

							if (count > 0)
							{
								ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(pageFlow, 0);

								if (pageFlow->inc == 0)
								{
									pageFlow->frame = 0;
									pageFlow->inc = child->rect.height;
								}
							}
							//pageFlow->frame = 0;
						}
						else // if (ev == ITU_EVENT_TOUCHSLIDEDOWN)
						{
							int count = itcTreeGetChildCount(pageFlow);

							if (count > 0)
							{
								ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(pageFlow, 0);

								if (pageFlow->inc == 0)
								{
									pageFlow->frame = 0;
									pageFlow->inc = 0 - child->rect.height;
								}
							}
							//pageFlow->frame = 0;
						}
					}
                }
            }
        }
		else if (ev == ITU_EVENT_MOUSEDOWN)
		{
			if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGABLE) && (pageFlow->type == ITU_PAGEFLOW_FOLD))
			{
				int x = arg2 - widget->rect.x;
				int y = arg3 - widget->rect.y;

				if (ituWidgetIsInside(widget, x, y))
				{
					pageFlow->working = 0;

					if (pageFlow->inc == 0)
						pageFlow->frame = 0;

					if (pageFlow->pageFlowFlags & ITU_PAGEFLOW_VERTICAL)
					{
						pageFlow->touchPos = y;
						if (pageFlow->lastpos == 0)
							pageFlow->lastpos = y;
					}
					else
					{
						pageFlow->touchPos = x;
						if (pageFlow->lastpos == 0)
							pageFlow->lastpos = x;
					}

					widget->flags |= ITU_DRAGGING;
					ituScene->dragged = widget;
				}
			}
		}
		else if (ev == ITU_EVENT_MOUSEMOVE)
		{
			int x = arg2 - widget->rect.x;
			int y = arg3 - widget->rect.y;
			int offset, dist;

			if (pageFlow->pageFlowFlags & ITU_PAGEFLOW_VERTICAL)
			{
				//offset = y - pageFlow->touchPos;
				offset = y - pageFlow->lastpos;
			}
			else
			{
				//offset = x - pageFlow->touchPos;
				offset = x - pageFlow->lastpos;
			}

			dist = (offset < 0) ? (-offset) : (offset);

			if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGABLE) 
				&& (pageFlow->type == ITU_PAGEFLOW_FOLD) && (widget->flags & ITU_DRAGGING) && (pageFlow->inc != 0))
			{
				if (ituWidgetIsInside(widget, x, y))
				{
					int count = PageFlowGetVisibleChildCount(pageFlow);

					if (dist >= ITU_DRAG_DISTANCE)
					{
						//ituUnPressWidget(widget);
						//ituWidgetUpdate(widget, ITU_EVENT_DRAGGING, 0, 0, 0);
					}

					if (pageFlow->pageFlowFlags & ITU_PAGEFLOW_VERTICAL)
					{
						pageFlow->working = offset;
						pageFlow->lastpos = y;
					}
					else
					{
						pageFlow->working = offset;
						pageFlow->lastpos = x;
					}
				}
			}
			else if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGABLE)
				&& (pageFlow->type == ITU_PAGEFLOW_FOLD) && (widget->flags & ITU_DRAGGING) && (pageFlow->inc == 0))
			{
				ITUWidget* child;
				int count = itcTreeGetChildCount(pageFlow);

				if (count > 0)
				{
					int size;
					child = (ITUWidget*)itcTreeGetChildAt(pageFlow, 0);

					if (pageFlow->pageFlowFlags & ITU_PAGEFLOW_VERTICAL)
						size = child->rect.height;
					else
						size = child->rect.width;

					if (offset > 0)
						pageFlow->inc = size;
					else
					{
						pageFlow->inc = 0 - size;
						//pageFlow->frame = pageFlow->totalframe - 1;
					}
				}
			}
		}//end if (ev == ITU_EVENT_MOUSEMOVE)
		else if (ev == ITU_EVENT_MOUSEUP)
		{
			if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGABLE) && (pageFlow->type == ITU_PAGEFLOW_FOLD) && (widget->flags & ITU_DRAGGING))
			{
				int x = arg2 - widget->rect.x;
				int y = arg3 - widget->rect.y;
				int offset;
				ITUWidget* child;
				int count = itcTreeGetChildCount(pageFlow);
				int size;
				child = (ITUWidget*)itcTreeGetChildAt(pageFlow, 0);

				if (pageFlow->pageFlowFlags & ITU_PAGEFLOW_VERTICAL)
				{
					offset = y - pageFlow->touchPos;
					size = child->rect.height / 3;
				}
				else
				{
					offset = x - pageFlow->touchPos;
					size = child->rect.width / 3;
				}

				offset = (offset >= 0) ? (offset) : (-offset);

				if ((offset <= size) && (offset != 0))
					pageFlow->working = 255;
				else
					pageFlow->working = 0;

				pageFlow->touchPos = 0;
				pageFlow->lastpos = 0;

				if (pageFlow->pageFlowFlags & ITU_PAGEFLOW_VERTICAL)
				{
					ituWidgetUpdate(widget, ITU_EVENT_LAYOUT, 0, 0, 0);
				}
				else
				{
					ituWidgetUpdate(widget, ITU_EVENT_LAYOUT, 0, 0, 0);
				}

				widget->flags |= ITU_UNDRAGGING;
				ituScene->dragged = NULL;
			}
			widget->flags &= ~ITU_DRAGGING;
		}//end if (ev == ITU_EVENT_MOUSEUP)
    }
    if (ev == ITU_EVENT_TIMER)
    {
        if (pageFlow->inc)
        {
            int count = itcTreeGetChildCount(pageFlow);

			pageFlow->frame += (pageFlow->working != 255) ? (1) : (-1);

			if ((pageFlow->frame < pageFlow->totalframe) && (pageFlow->frame >= 0))
            {
				float step;

				if ((pageFlow->type == ITU_PAGEFLOW_FOLD) && (widget->flags & ITU_DRAGGABLE) && (widget->flags & ITU_DRAGGING))
					pageFlow->frame--;

				if (pageFlow->working && (pageFlow->working != 255))
				{
					int range;
					double dframe;
					double factor;

					if (pageFlow->pageFlowFlags & ITU_PAGEFLOW_VERTICAL)
						range = widget->rect.height;
					else
						range = widget->rect.width;

					factor = (double)(pageFlow->working) / (double)range;

					dframe = factor * pageFlow->totalframe * DFRAME_FACTOR;

					if (pageFlow->inc > 0)
					{
						if (dframe > 0)
						{
							//printf("[inc > 0][%.1f]\n", dframe);
							dframe *= 1.0;
						}
						else
						{
							//printf("[inc > 0][%.1f]\n", dframe);
							dframe *= 1.0;
						}
					}
					else
					{
						if (dframe < 0)
						{
							//printf("[inc < 0][%.1f]\n", dframe);
							dframe *= -1.0;
						}
						else
						{
							//printf("[inc < 0][%.1f]\n", dframe);
							dframe *= -1.0;
						}
					}

					pageFlow->frame += (int)dframe;

					if (pageFlow->frame <= 0)
						pageFlow->frame = 0;
					else if (pageFlow->frame >= (pageFlow->totalframe - 1))
						pageFlow->frame = pageFlow->totalframe - 1;

					pageFlow->working = 0;

					printf("[dframe] %d\n", (int)dframe);
				}

				step = (float)pageFlow->frame / pageFlow->totalframe;

                //printf("step=%f\n", step);

                if (pageFlow->focusIndex < 0 || pageFlow->focusIndex >= count)
                    pageFlow->focusIndex = 0;

                pageFlow->offset = (int)(pageFlow->inc * step);

				if (pageFlow->type == ITU_PAGEFLOW_FOLD)
                {
                    ITUWidget* child;
                    int index;

                    if (pageFlow->inc > 0)
                    {
                        if (pageFlow->focusIndex <= 0)
                            index = count - 1;
                        else
                            index = pageFlow->focusIndex - 1;
                    }
                    else // if (pageFlow->inc < 0)
                    {
                        if (pageFlow->focusIndex >= count - 1)
                            index = 0;
                        else
                            index = pageFlow->focusIndex + 1;
                    }

                    child = (ITUWidget*)itcTreeGetChildAt(pageFlow, index);
                    if (child)
                    {
						if (!(pageFlow->pageFlowFlags & ITU_PAGEFLOW_VERTICAL)) // For Horizontal
						{
							int fx;

							fx = widget->rect.width / 2 - child->rect.width / 2;
							fx -= pageFlow->inc;
							fx += (int)(pageFlow->inc * step);
							ituWidgetSetX(child, fx);
						}
						else // For Vertical
						{
							int fy;

							fy = widget->rect.height / 2 - child->rect.height / 2;
							fy += pageFlow->inc;
							fy -= (int)(pageFlow->inc * step);
							ituWidgetSetY(child, fy);
						}
                    }
                }
            }
            else
            {
				if (pageFlow->frame >= pageFlow->totalframe)
				{
					if (pageFlow->inc > 0)
					{
						if (pageFlow->focusIndex <= 0)
							pageFlow->focusIndex = count - 1;
						else
							pageFlow->focusIndex--;
					}
					else // if (pageFlow->inc < 0)
					{
						if (pageFlow->focusIndex >= count - 1)
							pageFlow->focusIndex = 0;
						else
							pageFlow->focusIndex++;
					}
					pageFlow->frame = 0;
					pageFlow->inc = 0;
					pageFlow->offset = 0;

					ituExecActions(widget, pageFlow->actions, ITU_EVENT_CHANGED, pageFlow->focusIndex);
					ituPageFlowOnPageChanged(pageFlow, widget);
					ituWidgetUpdate(widget, ITU_EVENT_LAYOUT, 0, 0, 0);
				}
				else
				{
					pageFlow->frame = 0;
					pageFlow->inc = 0;
					pageFlow->offset = 0;
					pageFlow->working = 0;

					ituWidgetUpdate(widget, ITU_EVENT_LAYOUT, 0, 0, 0);
				}
            }
            result = true;
        }
    }
    result |= ituWidgetUpdateImpl(widget, ev, arg1, arg2, arg3);

    if (ev == ITU_EVENT_LAYOUT)
    {
        ITCTree* node;
        int i, count = itcTreeGetChildCount(pageFlow);

        if (pageFlow->focusIndex < 0 || pageFlow->focusIndex >= count)
            pageFlow->focusIndex = 0;

        i = 0;
        for (node = widget->tree.child; node; node = node->sibling)
        {
            ITUWidget* child = (ITUWidget*)node;

            if (i++ == pageFlow->focusIndex)
                ituWidgetEnable(child);
            else
                ituWidgetDisable(child);

            if ((pageFlow->type == ITU_PAGEFLOW_FOLD)
				|| (pageFlow->type == ITU_PAGEFLOW_FLIP2))
            {
                int fx = widget->rect.width / 2 - child->rect.width / 2;
                int fy = widget->rect.height / 2 - child->rect.height / 2;

                ituWidgetSetPosition(child, fx, fy);
            }
        }
    }
    return widget->visible ? result : false;
}

void ituPageFlowDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    ITUPageFlow* pageFlow = (ITUPageFlow*) widget;
    ITUWidget* child;
    ITURectangle* rect = &widget->rect;
    int index, destx, desty, cs, maxOffset;
    ITURectangle prevClip;
	bool bCheck;

    ituWidgetSetClipping(widget, dest, x, y, &prevClip);

    destx = rect->x + x;
    desty = rect->y + y;
	cs = (((fabs((double)pageFlow->axisShift) / (rect->width / 2)) > 0.8) ? (0) : (pageFlow->axisShift));
    maxOffset = rect->width * 4 / 5;

    if (pageFlow->inc)
    {
        int count = itcTreeGetChildCount(pageFlow);

        if (pageFlow->type == ITU_PAGEFLOW_FOLD)
        {
            child = (ITUWidget*) itcTreeGetChildAt(widget, pageFlow->focusIndex);
            if (child)
            {
                ITUSurface* surf;
                rect = &child->rect;
                surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
                if (surf)
                {
                    int h = rect->height / pageFlow->effectRatio * pageFlow->frame / pageFlow->totalframe;
					int w = rect->width / (pageFlow->effectRatio * 2) * pageFlow->frame / pageFlow->totalframe;

                    ituBitBlt(surf, 0, 0, rect->width, rect->height, dest, destx + rect->x, desty + rect->y);
                    ituWidgetDraw(child, surf, -rect->x, -rect->y, alpha);

                    if (pageFlow->inc > 0)
                    {
                        if (!(pageFlow->pageFlowFlags & ITU_PAGEFLOW_VERTICAL)) // For Horizontal
                        {
                            if (pageFlow->offset < maxOffset)
                            {
                                //printf("1(%d %d) (%d 0) (%d %d) (%d %d)\n", pageFlow->offset, h, rect->width, rect->width, rect->height, pageFlow->offset, rect->height - h);
                                ituTransformBlt(dest, destx + rect->x, desty + rect->y, surf, 0, 0, rect->width, rect->height, pageFlow->offset, h, rect->width, 0, rect->width, rect->height, pageFlow->offset, rect->height - h, false, pageFlow->type, ITU_TRANSFORM_TURN_RIGHT);
                                //printf("2(%d 0) (%d %d) (%d %d) (%d %d)\n", pageFlow->offset, rect->width, h, rect->width, rect->height - h, pageFlow->offset, rect->height);
                                ituTransformBlt(dest, destx + rect->x, desty + rect->y, surf, 0, 0, rect->width / 2, rect->height, pageFlow->offset, 0, rect->width, h, rect->width, rect->height - h, pageFlow->offset, rect->height, false, pageFlow->type, ITU_TRANSFORM_TURN_RIGHT);
                            }
                        }
                        else // For Vertical
                        {
                            ituTransformBlt(dest, destx + rect->x, desty + rect->y, surf, 0, 0, rect->width, rect->height / 2, 0, 0, rect->width, 0, rect->width - w, rect->height - pageFlow->offset, w, rect->height - pageFlow->offset, false, pageFlow->type, ITU_TRANSFORM_TURN_BOTTOM);
                            ituTransformBlt(dest, destx + rect->x, desty + rect->y, surf, 0, rect->height / 2, rect->width, rect->height, w, 0, rect->width - w, 0, rect->width, rect->height - pageFlow->offset, 0, rect->height - pageFlow->offset, false, pageFlow->type, ITU_TRANSFORM_TURN_BOTTOM);
                        }
                    }
                    else
                    {
                        if (!(pageFlow->pageFlowFlags & ITU_PAGEFLOW_VERTICAL)) // For Horizontal
                        {
                            if (pageFlow->offset > -maxOffset)
                            {
                                //printf("3(0 %d) (%d 0) (%d %d) (0 %d)\n", h, rect->width + pageFlow->offset, rect->width + pageFlow->offset, rect->height, rect->height - h);
                                ituTransformBlt(dest, destx + rect->x, desty + rect->y, surf, 0, 0, rect->width, rect->height, 0, h, rect->width + pageFlow->offset, 0, rect->width + pageFlow->offset, rect->height, 0, rect->height - h, true, pageFlow->type, ITU_TRANSFORM_TURN_LEFT);
                                //printf("4(0 0) (%d %d) (%d %d) (0 %d)\n", rect->width + pageFlow->offset, h, rect->width + pageFlow->offset, rect->height - h, rect->height);
                                ituTransformBlt(dest, destx + rect->x, desty + rect->y, surf, 0, 0, rect->width / 2, rect->height, 0, 0, rect->width + pageFlow->offset, h, rect->width + pageFlow->offset, rect->height - h, 0, rect->height, true, pageFlow->type, ITU_TRANSFORM_TURN_LEFT);
                            }                            
                        }
                        else // For Vertical
                        {
                            ituTransformBlt(dest, destx + rect->x, desty + rect->y, surf, 0, 0, rect->width, rect->height / 2, 0, 0 - pageFlow->offset, rect->width, 0 - pageFlow->offset, rect->width - w, rect->height, w, rect->height, true, pageFlow->type, ITU_TRANSFORM_TURN_TOP);
                            ituTransformBlt(dest, destx + rect->x, desty + rect->y, surf, 0, rect->height / 2, rect->width, rect->height, w, 0 - pageFlow->offset, rect->width - w, 0 - pageFlow->offset, rect->width, rect->height, 0, rect->height, true, pageFlow->type, ITU_TRANSFORM_TURN_TOP);
                        }
                    }
                    ituDestroySurface(surf);
                }
            }

            if (pageFlow->inc > 0)
            {
                if (pageFlow->focusIndex <= 0)
                    index = count - 1;
                else
                    index = pageFlow->focusIndex - 1;
            }
            else // if (pageFlow->inc < 0)
            {
                if (pageFlow->focusIndex >= count - 1)
                    index = 0;
                else
                    index = pageFlow->focusIndex + 1;
            }

            child = (ITUWidget*) itcTreeGetChildAt(widget, index);
            if (child)
                ituWidgetDraw(child, dest, destx, desty, alpha);
        }
        else if (pageFlow->type == ITU_PAGEFLOW_FLIP)
        {
            //printf("pageFlow->frame:%d pageFlow->totalframe:%d pageFlow->offset:%d\n", pageFlow->frame, pageFlow->totalframe, pageFlow->offset);

            if (pageFlow->inc > 0)
            {
                if (pageFlow->focusIndex <= 0)
                    index = count - 1;
                else
                    index = pageFlow->focusIndex - 1;
            }
            else // if (pageFlow->inc < 0)
            {
                if (pageFlow->focusIndex >= count - 1)
                    index = 0;
                else
                    index = pageFlow->focusIndex + 1;
            }

			//if (pageFlow->frame < pageFlow->totalframe / 2)
			if (!(pageFlow->pageFlowFlags & ITU_PAGEFLOW_VERTICAL)) // For Horizontal
			{
				if (pageFlow->inc > 0)
				{
					bCheck = (pageFlow->frame < (double)(rect->width / 2 + cs) / (rect->width)*pageFlow->totalframe);
				}
				else
				{
					bCheck = (pageFlow->frame < (double)(rect->width / 2 - cs) / (rect->width)*pageFlow->totalframe);
				}
			}
			else // For Vertical
			{
				if (pageFlow->inc > 0)
				{
					bCheck = (pageFlow->frame < (double)(rect->height / 2 - cs) / (rect->height)*pageFlow->totalframe);
				}
				else
				{
					bCheck = (pageFlow->frame < (double)(rect->height / 2 + cs) / (rect->height)*pageFlow->totalframe);
				}
			}
			if (bCheck)
			{
				child = (ITUWidget*) itcTreeGetChildAt(widget, pageFlow->focusIndex);
                if (child)
                {
                    ITUSurface* surf;
                    rect = &child->rect;
                    surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
                    if (surf)
                    {
						if (!(pageFlow->pageFlowFlags & ITU_PAGEFLOW_VERTICAL)) // For Horizontal
						{
							ITUSurface* surf2 = ituCreateSurface(rect->width / 2 + ((pageFlow->inc > 0) ? (cs) : (cs*-1)), rect->height, 0, dest->format, NULL, 0);
							if (surf2)
							{
								int h = rect->height / pageFlow->effectRatio * pageFlow->frame / (pageFlow->totalframe / 2);

								ituBitBlt(surf, 0, 0, rect->width, rect->height, dest, destx + rect->x, desty + rect->y);
								ituWidgetDraw(child, surf, -rect->x, -rect->y, alpha);
								ituBitBlt(dest, destx + rect->x, desty + rect->y, rect->width, rect->height, surf, 0, 0);

								child = (ITUWidget*)itcTreeGetChildAt(widget, index);
								if (child)
									ituWidgetDraw(child, dest, destx, desty, alpha);

								if (pageFlow->inc > 0)
								{
									//printf("1 (%d %d) (%d 0) (%d %d) (%d %d)\n", pageFlow->offset, -h, rect->width / 2, rect->width / 2, rect->height, pageFlow->offset, rect->height + h);
									ituBitBlt(dest, destx + rect->x + rect->width / 2 + cs, desty + rect->y, rect->width / 2 - cs, rect->height, surf, rect->width / 2 + cs, 0);

									ituBitBlt(surf2, 0, 0, rect->width / 2 + cs, rect->height, surf, 0, 0);
                                    ituTransformBlt(dest, destx + rect->x, desty + rect->y, surf2, 0, 0, rect->width / 2 + cs, rect->height, pageFlow->offset, -h, rect->width / 2 + cs, 0, rect->width / 2 + cs, rect->height, pageFlow->offset, rect->height + h, false, pageFlow->type, ITU_TRANSFORM_TURN_RIGHT);
								}
								else
								{
									//printf("2 (0 0) (%d %d) (%d %d) (0 %d)\n", rect->width / 2 + pageFlow->offset, -h, rect->width / 2 + pageFlow->offset, rect->height + h, rect->height);
									ituBitBlt(dest, destx + rect->x, desty + rect->y, rect->width / 2 + cs, rect->height, surf, 0, 0);

									ituBitBlt(surf2, 0, 0, rect->width / 2 - cs, rect->height, surf, rect->width / 2 + cs, 0);
                                    ituTransformBlt(dest, destx + rect->x + rect->width / 2 + cs, desty + rect->y, surf2, 0, 0, rect->width / 2 - cs, rect->height, 0, 0, rect->width / 2 + pageFlow->offset - cs, -h, rect->width / 2 + pageFlow->offset - cs, rect->height + h, 0, rect->height, true, pageFlow->type, ITU_TRANSFORM_TURN_LEFT);
								}
								ituDestroySurface(surf2);
							}
						}
						else // For Vertical
						{
							ITUSurface* surf2 = ituCreateSurface(rect->width, rect->height / 2 + ((pageFlow->inc > 0) ? (cs*-1) : (cs)), 0, dest->format, NULL, 0);
							if (surf2)
							{
								int w = rect->width / pageFlow->effectRatio * pageFlow->frame / (pageFlow->totalframe / 2);

								ituBitBlt(surf, 0, 0, rect->width, rect->height, dest, destx + rect->x, desty + rect->y);
								ituWidgetDraw(child, surf, -rect->x, -rect->y, alpha);
								ituBitBlt(dest, destx + rect->x, desty + rect->y, rect->width, rect->height, surf, 0, 0);

								child = (ITUWidget*)itcTreeGetChildAt(widget, index);
								if (child)
									ituWidgetDraw(child, dest, destx, desty, alpha);

								if (pageFlow->inc > 0)
								{
									ituBitBlt(dest, destx + rect->x, desty + rect->y + rect->height / 2 - cs, rect->width, rect->height / 2 + cs, surf, 0, rect->height / 2 - cs);
									
									ituBitBlt(surf2, 0, 0, rect->width, rect->height / 2 - cs, surf, 0, 0);
                                    ituTransformBlt(dest, destx + rect->x, desty + rect->y, surf2, 0, 0, rect->width, rect->height / 2 - cs, -w, pageFlow->offset, rect->width + w, pageFlow->offset, rect->width, rect->height / 2 - cs, 0, rect->height / 2 - cs, false, pageFlow->type, ITU_TRANSFORM_TURN_BOTTOM);
								}
								else
								{
									ituBitBlt(dest, destx + rect->x, desty + rect->y, rect->width, rect->height / 2 - cs, surf, 0, 0);
									
									ituBitBlt(surf2, 0, 0, rect->width, rect->height / 2 + cs, surf, 0, rect->height / 2 - cs);
                                    ituTransformBlt(dest, destx + rect->x, desty + rect->y + rect->height / 2 - cs, surf2, 0, 0, rect->width, rect->height / 2 + cs, 0, 0, rect->width, 0, rect->width + w, rect->height / 2 + pageFlow->offset + cs, -w, rect->height / 2 + pageFlow->offset + cs, true, pageFlow->type, ITU_TRANSFORM_TURN_TOP);
								}
								ituDestroySurface(surf2);
							}
						}
                    }
                    ituDestroySurface(surf);
                }
            }
            else
            {
				child = (ITUWidget*) itcTreeGetChildAt(widget, index);
                if (child)
                {
                    ITUSurface* surf;
                    rect = &child->rect;
                    surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
                    if (surf)
                    {
						if (!(pageFlow->pageFlowFlags & ITU_PAGEFLOW_VERTICAL)) // For Horizontal
						{
							ITUSurface* surf2 = ituCreateSurface(rect->width / 2 + ((pageFlow->inc > 0) ? (cs*-1) : (cs)), rect->height, 0, dest->format, NULL, 0);
							if (surf2)
							{
								int h = rect->height / pageFlow->effectRatio * (pageFlow->totalframe - pageFlow->frame) / (pageFlow->totalframe / 2);

								ituBitBlt(surf, 0, 0, rect->width, rect->height, dest, destx + child->rect.x, desty + child->rect.y);
								ituWidgetDraw(child, surf, -rect->x, -rect->y, alpha);
								ituBitBlt(dest, destx + rect->x, desty + rect->y, rect->width, rect->height, surf, 0, 0);

								child = (ITUWidget*)itcTreeGetChildAt(widget, pageFlow->focusIndex);
								if (child)
									ituWidgetDraw(child, dest, destx, desty, alpha);

								if (pageFlow->inc > 0)
								{
									//printf("3 (0 0) (%d %d) (%d %d) (0 %d)\n", pageFlow->offset - rect->width / 2, -h, pageFlow->offset - rect->width / 2, rect->height + h, rect->height);
									ituBitBlt(dest, destx + rect->x, desty + rect->y, rect->width / 2 + cs, rect->height, surf, 0, 0);

									ituBitBlt(surf2, 0, 0, rect->width / 2 - cs, rect->height, surf, rect->width / 2 + cs, 0);
                                    ituTransformBlt(dest, destx + rect->width / 2 + cs, desty + rect->y, surf2, 0, 0, rect->width / 2 - cs, rect->height, 0, 0, pageFlow->offset - rect->width / 2 - cs, -h, pageFlow->offset - rect->width / 2 - cs, rect->height + h, 0, rect->height, true, pageFlow->type, ITU_TRANSFORM_TURN_LEFT);
								}
								else
								{
									//printf("4 (%d %d) (%d 0) (%d %d) (%d %d)\n", rect->width + pageFlow->offset, -h, rect->width / 2, rect->width / 2, rect->height, rect->width + pageFlow->offset, rect->height + h);
									ituBitBlt(dest, destx + rect->x + rect->width / 2 + cs, desty + rect->y, rect->width / 2 - cs, rect->height, surf, rect->width / 2 + cs, 0);

									ituBitBlt(surf2, 0, 0, rect->width / 2 + cs, rect->height, surf, 0, 0);
                                    ituTransformBlt(dest, destx + rect->x, desty + rect->y, surf2, 0, 0, rect->width / 2 + cs, rect->height, rect->width + pageFlow->offset, -h, rect->width / 2 + cs, 0, rect->width / 2 + cs, rect->height, rect->width + pageFlow->offset, rect->height + h, false, pageFlow->type, ITU_TRANSFORM_TURN_RIGHT);
								}
								ituDestroySurface(surf2);
							}
						}
						else // For Vertical
						{
							ITUSurface* surf2 = ituCreateSurface(rect->width, rect->height / 2 + ((pageFlow->inc > 0) ? (cs) : (cs*-1)), 0, dest->format, NULL, 0);
							if (surf2)
							{
								int w = rect->width / 2 - (rect->width / pageFlow->effectRatio * pageFlow->frame / (pageFlow->totalframe / 2));

								ituBitBlt(surf, 0, 0, rect->width, rect->height, dest, destx + child->rect.x, desty + child->rect.y);
								ituWidgetDraw(child, surf, -rect->x, -rect->y, alpha);
								ituBitBlt(dest, destx + rect->x, desty + rect->y, rect->width, rect->height, surf, 0, 0);

								child = (ITUWidget*)itcTreeGetChildAt(widget, pageFlow->focusIndex);
								if (child)
									ituWidgetDraw(child, dest, destx, desty, alpha);

								if (pageFlow->inc > 0)
								{
									ituBitBlt(dest, destx + rect->x, desty + rect->y, rect->width, rect->height / 2 - cs, surf, 0, 0);
									
									ituBitBlt(surf2, 0, 0, rect->width, rect->height / 2 + cs, surf, 0, rect->height / 2 - cs);
                                    ituTransformBlt(dest, destx + rect->x, desty + rect->y + (rect->height / 2) - cs, surf2, 0, 0, rect->width, rect->height / 2 + cs, 0, 0, rect->width, 0, rect->width + w, pageFlow->offset - (rect->height / 2 - cs), -w, pageFlow->offset - (rect->height / 2 - cs), true, pageFlow->type, ITU_TRANSFORM_TURN_TOP);
								}
								else
								{
									ituBitBlt(dest, destx + rect->x, desty + rect->y + rect->height / 2 - cs, rect->width, rect->height / 2 + cs, surf, 0, rect->height / 2 - cs);
									
									ituBitBlt(surf2, 0, 0, rect->width, rect->height / 2 - cs, surf, 0, 0);
                                    ituTransformBlt(dest, destx + rect->x, desty + rect->y, surf2, 0, 0, rect->width, rect->height / 2 - cs, -w, rect->height + pageFlow->offset, rect->width + w, rect->height + pageFlow->offset, rect->width, rect->height / 2 - cs, 0, rect->height / 2 - cs, false, pageFlow->type, ITU_TRANSFORM_TURN_BOTTOM);
								}
								ituDestroySurface(surf2);
							}
						}
                    }
                    ituDestroySurface(surf);
                }
            }
        }
        else if (pageFlow->type == ITU_PAGEFLOW_FLIP2)
        {
            int frame, totalframe, width, height, offset, axis;

			if (!(pageFlow->pageFlowFlags & ITU_PAGEFLOW_VERTICAL)) // For Horizontal
				axis = rect->width / 2 + pageFlow->axisShift;
			else // For Vertical
				axis = rect->height / 2 + pageFlow->axisShift;

            if (pageFlow->inc > 0)
            {
                if (pageFlow->focusIndex <= 0)
                    index = count - 1;
                else
                    index = pageFlow->focusIndex - 1;
            }
            else // if (pageFlow->inc < 0)
            {
                if (pageFlow->focusIndex >= count - 1)
                    index = 0;
                else
                    index = pageFlow->focusIndex + 1;
            }

            frame = pageFlow->frame;
            totalframe = pageFlow->totalframe * 4 / 5;

            if (frame < totalframe / 2) // first part
            {
                child = (ITUWidget*) itcTreeGetChildAt(widget, pageFlow->focusIndex);
                if (child)
                {
                    ITUSurface* surf;
                    rect = &child->rect;
					height = rect->height / 2;
					width = rect->width / 2;

					if (!(pageFlow->pageFlowFlags & ITU_PAGEFLOW_VERTICAL)) // For Horizontal
					{
						height = rect->height * pageFlow->ratioflip2 / 100;
						surf = ituCreateSurface(rect->width, height, 0, dest->format, NULL, 0);
						if (surf)
						{
							ITUSurface* surf2 = ituCreateSurface(pageFlow->inc > 0 ? axis : rect->width - axis, height, 0, dest->format, NULL, 0);
							if (surf2)
							{
								int h = height / pageFlow->effectRatio * frame / (totalframe / 2);

								ituBitBlt(surf, 0, 0, rect->width, height, dest, destx + rect->x, desty + rect->y);
								ituSurfaceSetClipping(surf, 0, 0, rect->width, height);
								ituWidgetDraw(child, surf, -rect->x, -rect->y, alpha);

								child = (ITUWidget*)itcTreeGetChildAt(widget, index);
								if (child)
								{
									ituSurfaceSetClipping(dest, destx + rect->x, desty + rect->y, rect->width, height);
									ituWidgetDraw(child, dest, destx, desty, alpha);
									ituSurfaceSetClipping(dest, destx, desty, rect->width, rect->height);
								}

								if (pageFlow->inc > 0)
								{
									ituBitBlt(dest, destx + rect->x + axis, desty + rect->y, rect->width - axis, height, surf, axis, 0);

									offset = axis * frame * 2 / totalframe;
									ituBitBlt(surf2, 0, 0, axis, height, surf, 0, 0);
                                    ituTransformBlt(dest, destx + rect->x, desty + rect->y, surf2, 0, 0, axis, height, offset, -h, axis, 0, axis, height, offset, height + h, false, pageFlow->type, ITU_TRANSFORM_TURN_RIGHT);
								}
								else
								{
									ituBitBlt(dest, destx + rect->x, desty + rect->y, axis, height, surf, 0, 0);

									offset = -(rect->width - axis) * frame * 2 / totalframe;
									ituBitBlt(surf2, 0, 0, rect->width - axis, height, surf, axis, 0);
                                    ituTransformBlt(dest, destx + rect->x + axis, desty + rect->y, surf2, 0, 0, rect->width - axis, height, 0, 0, rect->width - axis + offset, -h, rect->width - axis + offset, height + h, 0, height, true, pageFlow->type, ITU_TRANSFORM_TURN_LEFT);
								}
								ituDestroySurface(surf2);
							}
						}
						ituDestroySurface(surf);
					}
					else // For Vertical
					{
						width = rect->width * pageFlow->ratioflip2 / 100;
						surf = ituCreateSurface(width, rect->height, 0, dest->format, NULL, 0);
						if (surf)
						{
							ITUSurface* surf2 = ituCreateSurface(width, pageFlow->inc > 0 ? rect->height - axis : axis, 0, dest->format, NULL, 0);
							if (surf2)
							{
								int w = width / pageFlow->effectRatio * frame / (totalframe / 2);

								ituBitBlt(surf, 0, 0, width, rect->height, dest, destx + rect->x, desty + rect->y);
								ituSurfaceSetClipping(surf, 0, 0, width, rect->height);
								ituWidgetDraw(child, surf, -rect->x, -rect->y, alpha);

								child = (ITUWidget*)itcTreeGetChildAt(widget, index);
								if (child)
								{
									ituSurfaceSetClipping(dest, destx + rect->x, desty + rect->y, width, rect->height);
									ituWidgetDraw(child, dest, destx, desty, alpha);
									ituSurfaceSetClipping(dest, destx, desty, rect->width, rect->height);
								}

								if (pageFlow->inc > 0)
								{
									ituBitBlt(dest, destx + rect->x, desty + rect->y + rect->height - axis, width, axis, surf, 0, rect->height - axis);

									offset = (rect->height - axis) * frame * 2 / totalframe;
									ituBitBlt(surf2, 0, 0, width, rect->height - axis, surf, 0, 0);
                                    ituTransformBlt(dest, destx + rect->x, desty + rect->y, surf2, 0, 0, width, rect->height - axis, -w, offset, width + w, offset, width, rect->height - axis, 0, rect->height - axis, false, pageFlow->type, ITU_TRANSFORM_TURN_BOTTOM);
								}
								else
								{
									ituBitBlt(dest, destx + rect->x, desty + rect->y, width, rect->height - axis, surf, 0, 0);

									offset = axis * frame * 2 / totalframe;
									ituBitBlt(surf2, 0, 0, width, axis, surf, 0, rect->height - axis);
                                    ituTransformBlt(dest, destx + rect->x, desty + rect->y + rect->height - axis, surf2, 0, 0, width, axis, 0, 0, width, 0, width + w, axis - offset, -w, axis - offset, true, pageFlow->type, ITU_TRANSFORM_TURN_TOP);
								}
								ituDestroySurface(surf2);
							}
						}
						ituDestroySurface(surf);
					}
                }
            }
            else if (frame < totalframe) //first part
            {
                child = (ITUWidget*) itcTreeGetChildAt(widget, index);
                if (child)
                {
                    ITUSurface* surf;
                    rect = &child->rect;
					height = rect->height / 2;
					width = rect->width / 2;

					if (!(pageFlow->pageFlowFlags & ITU_PAGEFLOW_VERTICAL)) // For Horizontal
					{
						height = rect->height * pageFlow->ratioflip2 / 100;
						surf = ituCreateSurface(rect->width, height, 0, dest->format, NULL, 0);
						if (surf)
						{
							ITUSurface* surf2 = ituCreateSurface(pageFlow->inc > 0 ? rect->width - axis : axis, height, 0, dest->format, NULL, 0);
							if (surf2)
							{
								int h = height / pageFlow->effectRatio * (totalframe - frame) / (totalframe / 2);

								ituBitBlt(surf, 0, 0, rect->width, height, dest, destx + rect->x, desty + rect->y);
								ituSurfaceSetClipping(surf, 0, 0, rect->width, height);
								ituWidgetDraw(child, surf, -rect->x, -rect->y, alpha);

								child = (ITUWidget*)itcTreeGetChildAt(widget, pageFlow->focusIndex);
								if (child)
								{
									ituSurfaceSetClipping(dest, destx + rect->x, desty + rect->y, rect->width, height);
									ituWidgetDraw(child, dest, destx, desty, alpha);
									ituSurfaceSetClipping(dest, destx, desty, rect->width, rect->height);
								}

								if (pageFlow->inc > 0)
								{
									ituBitBlt(dest, destx + rect->x, desty + rect->y, axis, height, surf, 0, 0);

									offset = (rect->width - axis) * frame * 2 / totalframe;
									ituBitBlt(surf2, 0, 0, rect->width - axis, height, surf, axis, 0);
                                    ituTransformBlt(dest, destx + rect->x + axis, desty + rect->y, surf2, 0, 0, rect->width - axis, height, 0, 0, offset - (rect->width - axis), -h, offset - (rect->width - axis), height + h, 0, height, true, pageFlow->type, ITU_TRANSFORM_TURN_LEFT);
								}
								else
								{
									ituBitBlt(dest, destx + rect->x + axis, desty + rect->y, rect->width - axis, height, surf, axis, 0);

									offset = -axis * (frame - totalframe / 2) / (totalframe / 2);
									ituBitBlt(surf2, 0, 0, axis, height, surf, 0, 0);
                                    ituTransformBlt(dest, destx + rect->x, desty + rect->y, surf2, 0, 0, axis, height, axis + offset, -h, axis, 0, axis, height, axis + offset, height + h, false, pageFlow->type, ITU_TRANSFORM_TURN_RIGHT);
								}
								ituDestroySurface(surf2);
							}
						}
						ituDestroySurface(surf);
					}
					else // For Vertical
					{
						width = rect->width * pageFlow->ratioflip2 / 100;
						surf = ituCreateSurface(width, rect->height, 0, dest->format, NULL, 0);
						if (surf)
						{
							ITUSurface* surf2 = ituCreateSurface(width, pageFlow->inc > 0 ? axis : rect->height - axis, 0, dest->format, NULL, 0);
							if (surf2)
							{
								int w = width / pageFlow->effectRatio * (totalframe - frame) / (totalframe / 2);

								ituBitBlt(surf, 0, 0, width, rect->height, dest, destx + rect->x, desty + rect->y);
								ituSurfaceSetClipping(surf, 0, 0, width, rect->height);
								ituWidgetDraw(child, surf, -rect->x, -rect->y, alpha);

								child = (ITUWidget*)itcTreeGetChildAt(widget, pageFlow->focusIndex);
								if (child)
								{
									ituSurfaceSetClipping(dest, destx + rect->x, desty + rect->y, width, rect->height);
									ituWidgetDraw(child, dest, destx, desty, alpha);
									ituSurfaceSetClipping(dest, destx, desty, rect->width, rect->height);
								}

								if (pageFlow->inc > 0)
								{
									ituBitBlt(dest, destx + rect->x, desty + rect->y, width, rect->height - axis, surf, 0, 0);
									
									offset = axis * (frame - totalframe / 2) / (totalframe / 2);
									ituBitBlt(surf2, 0, 0, width, axis, surf, 0, rect->height - axis);
                                    ituTransformBlt(dest, destx + rect->x, desty + rect->y + rect->height - axis, surf2, 0, 0, width, axis, 0, 0, width, 0, width + w, offset, -w, offset, true, pageFlow->type, ITU_TRANSFORM_TURN_TOP);
								}
								else
								{
									ituBitBlt(dest, destx + rect->x, desty + rect->y + rect->height - axis, width, axis, surf, 0, rect->height - axis);

									offset = (rect->height - axis) * frame * 2 / totalframe;
									ituBitBlt(surf2, 0, 0, width, rect->height - axis, surf, 0, 0);
                                    ituTransformBlt(dest, destx + rect->x, desty + rect->y, surf2, 0, 0, width, rect->height - axis, -w, (rect->height - axis) * 2 - offset, width + w, (rect->height - axis) * 2 - offset, width, rect->height - axis, 0, rect->height - axis, false, pageFlow->type, ITU_TRANSFORM_TURN_BOTTOM);
								}
								ituDestroySurface(surf2);
							}
						}
						ituDestroySurface(surf);
					}
                }
            }
            else //first part
            {
                child = (ITUWidget*) itcTreeGetChildAt(widget, index);
                if (child)
                {
                    rect = &child->rect;
                    height = rect->height / 2;
					width = rect->width / 2;

					if (!(pageFlow->pageFlowFlags & ITU_PAGEFLOW_VERTICAL)) // For Horizontal
					{
						height = rect->height * pageFlow->ratioflip2 / 100;
						ituSurfaceSetClipping(dest, destx + rect->x, desty + rect->y, rect->width, height);
					}
					else // For Vertical
					{
						width = rect->width * pageFlow->ratioflip2 / 100;
						ituSurfaceSetClipping(dest, destx + rect->x, desty + rect->y, width, rect->height);
					}
                    ituWidgetDraw(child, dest, destx, desty, alpha);
                    ituSurfaceSetClipping(dest, destx, desty, rect->width, rect->height);
                }
            }

            frame = pageFlow->frame - (pageFlow->totalframe - totalframe);

            if (frame <= 0) //second part
            {
                child = (ITUWidget*) itcTreeGetChildAt(widget, pageFlow->focusIndex);
                if (child)
                {
					if (!(pageFlow->pageFlowFlags & ITU_PAGEFLOW_VERTICAL)) // For Horizontal
					{
						int ratioef;
						height = rect->height * pageFlow->ratioflip2 / 100;
						ratioef = rect->height - height;
						ituSurfaceSetClipping(dest, destx + rect->x, desty + rect->y + height, rect->width, ratioef);
					}
					else // For Vertical
					{
						int ratioef;
						width = rect->width * pageFlow->ratioflip2 / 100;
						ratioef = rect->width - width;
						ituSurfaceSetClipping(dest, destx + rect->x + width, desty + rect->y, ratioef, rect->height);
					}
                    ituWidgetDraw(child, dest, destx, desty, alpha);
                    ituSurfaceSetClipping(dest, destx, desty, rect->width, rect->height);
                }
            }
            else if (frame < totalframe / 2) //second part
            {
                child = (ITUWidget*) itcTreeGetChildAt(widget, pageFlow->focusIndex);
                if (child)
                {
                    ITUSurface* surf;
                    rect = &child->rect;
                    height = rect->height / 2;
					width = rect->width / 2;

					if (!(pageFlow->pageFlowFlags & ITU_PAGEFLOW_VERTICAL)) // For Horizontal
					{
						int ratioef;
						height = rect->height * pageFlow->ratioflip2 / 100;
						ratioef = rect->height - height;
						surf = ituCreateSurface(rect->width, ratioef, 0, dest->format, NULL, 0);
						if (surf)
						{
							ITUSurface* surf2 = ituCreateSurface(pageFlow->inc > 0 ? axis : rect->width - axis, ratioef, 0, dest->format, NULL, 0);
							if (surf2)
							{
								int h = ratioef / pageFlow->effectRatio * frame / (totalframe / 2);

								ituBitBlt(surf, 0, 0, rect->width, ratioef, dest, destx + rect->x, desty + rect->y + height);
								ituSurfaceSetClipping(surf, 0, 0, rect->width, ratioef);
								ituWidgetDraw(child, surf, -rect->x, -rect->y - height, alpha);

								child = (ITUWidget*)itcTreeGetChildAt(widget, index);
								if (child)
								{
									ituSurfaceSetClipping(dest, destx + rect->x, desty + rect->y + height, rect->width, ratioef);
									ituWidgetDraw(child, dest, destx, desty, alpha);
									ituSurfaceSetClipping(dest, destx, desty + height, rect->width, rect->height);
								}

								if (pageFlow->inc > 0)
								{
									ituBitBlt(dest, destx + rect->x + axis, desty + rect->y + height, rect->width - axis, ratioef, surf, axis, 0);
									
									offset = axis * frame * 2 / totalframe;
									ituBitBlt(surf2, 0, 0, axis, ratioef, surf, 0, 0);
                                    ituTransformBlt(dest, destx + rect->x, desty + rect->y + height, surf2, 0, 0, axis, ratioef, offset, -h, axis, 0, axis, ratioef, offset, ratioef + h, false, pageFlow->type, ITU_TRANSFORM_TURN_RIGHT);
								}
								else
								{
									ituBitBlt(dest, destx + rect->x, desty + rect->y + height, axis, ratioef, surf, 0, 0);
									
									offset = -(rect->width - axis) * frame * 2 / totalframe;
									ituBitBlt(surf2, 0, 0, rect->width - axis, ratioef, surf, axis, 0);
                                    ituTransformBlt(dest, destx + rect->x + axis, desty + rect->y + height, surf2, 0, 0, rect->width - axis, ratioef, 0, 0, rect->width - axis + offset, -h, rect->width - axis + offset, ratioef + h, 0, ratioef, true, pageFlow->type, ITU_TRANSFORM_TURN_LEFT);
								}
								ituDestroySurface(surf2);
							}
						}
						ituDestroySurface(surf);
					}
					else // For Vertical
					{
						int ratioef;
						width = rect->width * pageFlow->ratioflip2 / 100;
						ratioef = rect->width - width;
						surf = ituCreateSurface(ratioef, rect->height, 0, dest->format, NULL, 0);
						if (surf)
						{
							ITUSurface* surf2 = ituCreateSurface(ratioef, pageFlow->inc > 0 ? rect->height - axis : axis, 0, dest->format, NULL, 0);
							if (surf2)
							{
								int w = ratioef / pageFlow->effectRatio * frame / (totalframe / 2);

								ituBitBlt(surf, 0, 0, ratioef, rect->height, dest, destx + rect->x + width, desty + rect->y);
								ituSurfaceSetClipping(surf, 0, 0, ratioef, rect->height);
								ituWidgetDraw(child, surf, -rect->x - width, -rect->y, alpha);

								child = (ITUWidget*)itcTreeGetChildAt(widget, index);
								if (child)
								{
									ituSurfaceSetClipping(dest, destx + rect->x + width, desty + rect->y, ratioef, rect->height);
									ituWidgetDraw(child, dest, destx, desty, alpha);
									ituSurfaceSetClipping(dest, destx + width, desty, rect->width, rect->height);
								}

								if (pageFlow->inc > 0)
								{
									ituBitBlt(dest, destx + rect->x + width, desty + rect->y + rect->height - axis, ratioef, axis, surf, 0, rect->height - axis);

									offset = (rect->height - axis) * frame * 2 / totalframe;
									ituBitBlt(surf2, 0, 0, ratioef, rect->height - axis, surf, 0, 0);
                                    ituTransformBlt(dest, destx + rect->x + width, desty + rect->y, surf2, 0, 0, ratioef, rect->height - axis, -w, offset, ratioef + w, offset, ratioef, rect->height - axis, 0, rect->height - axis, false, pageFlow->type, ITU_TRANSFORM_TURN_BOTTOM);
								}
								else
								{
									ituBitBlt(dest, destx + rect->x + width, desty + rect->y, ratioef, rect->height - axis, surf, 0, 0);

									offset = axis * frame * 2 / totalframe;
									ituBitBlt(surf2, 0, 0, ratioef, axis, surf, 0, rect->height - axis);
                                    ituTransformBlt(dest, destx + rect->x + width, desty + rect->y + rect->height - axis, surf2, 0, 0, ratioef, axis, 0, 0, ratioef, 0, ratioef + w, axis - offset, -w, axis - offset, true, pageFlow->type, ITU_TRANSFORM_TURN_TOP);
								}
								ituDestroySurface(surf2);
							}
						}
						ituDestroySurface(surf);
					}
                }
            }
            else if (frame < totalframe) //second part
            {
                child = (ITUWidget*) itcTreeGetChildAt(widget, index);
                if (child)
                {
                    ITUSurface* surf;
                    rect = &child->rect;
                    height = rect->height / 2;
					width = rect->width / 2;

					if (!(pageFlow->pageFlowFlags & ITU_PAGEFLOW_VERTICAL)) // For Horizontal
					{
						int ratioef;
						height = rect->height * pageFlow->ratioflip2 / 100;
						ratioef = rect->height - height;
						surf = ituCreateSurface(rect->width, ratioef, 0, dest->format, NULL, 0);
						if (surf)
						{
							ITUSurface* surf2 = ituCreateSurface(pageFlow->inc > 0 ? rect->width - axis : axis, ratioef, 0, dest->format, NULL, 0);
							if (surf2)
							{
								int h = ratioef / pageFlow->effectRatio * (totalframe - frame) / (totalframe / 2);

								ituBitBlt(surf, 0, 0, rect->width, ratioef, dest, destx + rect->x, desty + rect->y + height);
								ituSurfaceSetClipping(surf, 0, 0, rect->width, ratioef);
								ituWidgetDraw(child, surf, -rect->x, -rect->y - height, alpha);

								child = (ITUWidget*)itcTreeGetChildAt(widget, pageFlow->focusIndex);
								if (child)
								{
									ituSurfaceSetClipping(dest, destx + rect->x, desty + rect->y + height, rect->width, ratioef);
									ituWidgetDraw(child, dest, destx, desty, alpha);
									ituSurfaceSetClipping(dest, destx, desty + height, rect->width, rect->height);
								}

								if (pageFlow->inc > 0)
								{
									ituBitBlt(dest, destx + rect->x, desty + rect->y + height, axis, ratioef, surf, 0, 0);
									offset = (rect->width - axis) * frame * 2 / totalframe;
									ituBitBlt(surf2, 0, 0, rect->width - axis, ratioef, surf, axis, 0);
                                    ituTransformBlt(dest, destx + rect->x + axis, desty + rect->y + height, surf2, 0, 0, rect->width - axis, ratioef, 0, 0, offset - (rect->width - axis), -h, offset - (rect->width - axis), ratioef + h, 0, ratioef, true, pageFlow->type, ITU_TRANSFORM_TURN_LEFT);
								}
								else
								{
									ituBitBlt(dest, destx + rect->x + axis, desty + rect->y + height, rect->width - axis, ratioef, surf, axis, 0);

									offset = -axis * (frame - totalframe / 2) / (totalframe / 2);
									ituBitBlt(surf2, 0, 0, axis, ratioef, surf, 0, 0);
                                    ituTransformBlt(dest, destx + rect->x, desty + rect->y + height, surf2, 0, 0, axis, ratioef, axis + offset, -h, axis, 0, axis, ratioef, axis + offset, ratioef + h, false, pageFlow->type, ITU_TRANSFORM_TURN_RIGHT);
								}
								ituDestroySurface(surf2);
							}
						}
						ituDestroySurface(surf);
					}
					else // For Vertical
					{
						int ratioef;
						width = rect->width * pageFlow->ratioflip2 / 100;
						ratioef = rect->width - width;
						surf = ituCreateSurface(ratioef, rect->height, 0, dest->format, NULL, 0);
						if (surf)
						{
							ITUSurface* surf2 = ituCreateSurface(ratioef, pageFlow->inc > 0 ? axis : rect->height - axis, 0, dest->format, NULL, 0);
							if (surf2)
							{
								int w = ratioef / pageFlow->effectRatio * (totalframe - frame) / (totalframe / 2);

								ituBitBlt(surf, 0, 0, ratioef, rect->height, dest, destx + rect->x + width, desty + rect->y);
								ituSurfaceSetClipping(surf, 0, 0, ratioef, rect->height);
								ituWidgetDraw(child, surf, -rect->x - width, -rect->y, alpha);

								child = (ITUWidget*)itcTreeGetChildAt(widget, pageFlow->focusIndex);
								if (child)
								{
									ituSurfaceSetClipping(dest, destx + rect->x + width, desty + rect->y, ratioef, rect->height);
									ituWidgetDraw(child, dest, destx, desty, alpha);
									ituSurfaceSetClipping(dest, destx + width, desty, rect->width, rect->height);
								}

								if (pageFlow->inc > 0)
								{
									ituBitBlt(dest, destx + rect->x + width, desty + rect->y, ratioef, rect->height - axis, surf, 0, 0);

									offset = axis * (frame - totalframe / 2) / (totalframe / 2);
									ituBitBlt(surf2, 0, 0, ratioef, axis, surf, 0, rect->height - axis);
                                    ituTransformBlt(dest, destx + rect->x + width, desty + rect->y + rect->height - axis, surf2, 0, 0, ratioef, axis, 0, 0, ratioef, 0, ratioef + w, offset, -w, offset, true, pageFlow->type, ITU_TRANSFORM_TURN_TOP);
								}
								else
								{
									ituBitBlt(dest, destx + rect->x + width, desty + rect->y + rect->height - axis, ratioef, axis, surf, 0, rect->height - axis);

									offset = (rect->height - axis) * frame * 2 / totalframe;
									ituBitBlt(surf2, 0, 0, ratioef, rect->height - axis, surf, 0, 0);
                                    ituTransformBlt(dest, destx + rect->x + width, desty + rect->y, surf2, 0, 0, ratioef, rect->height - axis, -w, (rect->height - axis) * 2 - offset, ratioef + w, (rect->height - axis) * 2 - offset, ratioef, rect->height - axis, 0, rect->height - axis, false, pageFlow->type, ITU_TRANSFORM_TURN_BOTTOM);
								}
								ituDestroySurface(surf2);
							}
						}
						ituDestroySurface(surf);
					}
                }
            }
        }//end if (pageFlow->type == ITU_PAGEFLOW_FLIP2)
    }
    else
    {
        child = (ITUWidget*) itcTreeGetChildAt(widget, pageFlow->focusIndex);
        if (child)
            ituWidgetDraw(child, dest, destx, desty, alpha);
    }

    ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
    ituDirtyWidget(pageFlow, false);
}

static void PageFlowOnPageChanged(ITUPageFlow* pageFlow, ITUWidget* widget)
{
    // DO NOTHING
}

void ituPageFlowOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    ITUPageFlow* pageFlow = (ITUPageFlow*) widget;
    assert(widget);

    switch (action)
    {
    case ITU_ACTION_PREV:
        ituPageFlowPrev((ITUPageFlow*)widget);
        break;

    case ITU_ACTION_NEXT:
        ituPageFlowNext((ITUPageFlow*)widget);
        break;

    case ITU_ACTION_GOTO:
        if (param[0] != '\0')
            ituPageFlowGoto(pageFlow, atoi(param));
        break;

    case ITU_ACTION_DODELAY0:
        ituExecActions(widget, pageFlow->actions, ITU_EVENT_DELAY0, atoi(param));
        break;

    case ITU_ACTION_DODELAY1:
        ituExecActions(widget, pageFlow->actions, ITU_EVENT_DELAY1, atoi(param));
        break;

    case ITU_ACTION_DODELAY2:
        ituExecActions(widget, pageFlow->actions, ITU_EVENT_DELAY2, atoi(param));
        break;

    case ITU_ACTION_DODELAY3:
        ituExecActions(widget, pageFlow->actions, ITU_EVENT_DELAY3, atoi(param));
        break;

    case ITU_ACTION_DODELAY4:
        ituExecActions(widget, pageFlow->actions, ITU_EVENT_DELAY4, atoi(param));
        break;

    case ITU_ACTION_DODELAY5:
        ituExecActions(widget, pageFlow->actions, ITU_EVENT_DELAY5, atoi(param));
        break;

    case ITU_ACTION_DODELAY6:
        ituExecActions(widget, pageFlow->actions, ITU_EVENT_DELAY6, atoi(param));
        break;

    case ITU_ACTION_DODELAY7:
        ituExecActions(widget, pageFlow->actions, ITU_EVENT_DELAY7, atoi(param));
        break;

    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

void ituPageFlowInit(ITUPageFlow* pageFlow)
{
    assert(pageFlow);
    ITU_ASSERT_THREAD();

    memset(pageFlow, 0, sizeof (ITUPageFlow));

    ituWidgetInit(&pageFlow->widget);

    ituWidgetSetType(pageFlow, ITU_PAGEFLOW);
    ituWidgetSetName(pageFlow, pageFlowName);
    ituWidgetSetUpdate(pageFlow, ituPageFlowUpdate);
    ituWidgetSetDraw(pageFlow, ituPageFlowDraw);
    ituWidgetSetOnAction(pageFlow, ituPageFlowOnAction);
    ituPageFlowSetPageChanged(pageFlow, PageFlowOnPageChanged);
}

void ituPageFlowLoad(ITUPageFlow* pageFlow, uint32_t base)
{
    assert(pageFlow);

    ituWidgetLoad(&pageFlow->widget, base);
    ituWidgetSetUpdate(pageFlow, ituPageFlowUpdate);
    ituWidgetSetDraw(pageFlow, ituPageFlowDraw);
    ituWidgetSetOnAction(pageFlow, ituPageFlowOnAction);
    ituPageFlowSetPageChanged(pageFlow, PageFlowOnPageChanged);
}

void ituPageFlowGoto(ITUPageFlow* pageFlow, int index)
{
    assert(pageFlow);
    ITU_ASSERT_THREAD();

    if (pageFlow->focusIndex == index)
        return;
  
    pageFlow->focusIndex = index;
    ituWidgetUpdate(pageFlow, ITU_EVENT_LAYOUT, 0, 0, 0);
}

void ituPageFlowPrev(ITUPageFlow* pageflow)
{
    ITUWidget* widget = (ITUWidget*) pageflow;
    unsigned int oldFlags = widget->flags;

    ITU_ASSERT_THREAD();

    widget->flags |= ITU_TOUCHABLE;

    if (pageflow->pageFlowFlags & ITU_PAGEFLOW_VERTICAL)
        ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDEDOWN, 0, widget->rect.x, widget->rect.y);
    else
        ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDERIGHT, 0, widget->rect.x, widget->rect.y);

    if ((oldFlags & ITU_TOUCHABLE) == 0)
        widget->flags &= ~ITU_TOUCHABLE;
}

void ituPageFlowNext(ITUPageFlow* pageflow)
{
    ITUWidget* widget = (ITUWidget*) pageflow;
    unsigned int oldFlags = widget->flags;

    ITU_ASSERT_THREAD();

    widget->flags |= ITU_TOUCHABLE;

    if (pageflow->pageFlowFlags & ITU_PAGEFLOW_VERTICAL)
        ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDEUP, 0, widget->rect.x, widget->rect.y);
    else
        ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDELEFT, 0, widget->rect.x, widget->rect.y);

    if ((oldFlags & ITU_TOUCHABLE) == 0)
        widget->flags &= ~ITU_TOUCHABLE;
}

int ituPageFlowRatioFlip2(ITUPageFlow* pageFlow, int ratio)
{
	assert(pageFlow);

    ITU_ASSERT_THREAD();

	if (ratio < 0)
		return pageFlow->ratioflip2;
	else if ((ratio < 5) || (ratio > 95))
		return 0;
	else
	{
		pageFlow->ratioflip2 = ratio;
		return 1;
	}
}


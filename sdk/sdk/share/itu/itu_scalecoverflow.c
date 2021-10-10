#include <assert.h>
#include <math.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char scaleCoverFlowName[] = "ITUScaleCoverFlow";

extern int CoverFlowGetVisibleChildCount(ITUCoverFlow* coverflow);
extern ITUWidget* CoverFlowGetVisibleChild(ITUCoverFlow* coverflow, int index);


static void ituWidgetSetPositionFix(ITUCoverFlow* coverflow, ITUWidget* child, int x, int y, int index)
{
	ITUScaleCoverFlow* scalecoverflow = (ITUScaleCoverFlow*)coverflow;
	int pos_shift = scalecoverflow->concentration;
	int count = CoverFlowGetVisibleChildCount(coverflow);

	assert(scalecoverflow);

	if (coverflow->focusIndex < 0)
		coverflow->focusIndex = 0;
	else if (coverflow->focusIndex > (count - 1))
		coverflow->focusIndex = count - 1;

	if (index == coverflow->focusIndex)
	{
		ituWidgetSetPosition(child, x, y);
	}
	else
	{
		ITUWidget* childf = CoverFlowGetVisibleChild(coverflow, coverflow->focusIndex);
		int fix;

		if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
			fix = ((child->rect.y > childf->rect.y) ? (-pos_shift) : (pos_shift));
		else
			fix = ((child->rect.x > childf->rect.x) ? (-pos_shift) : (pos_shift));

		if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
		{
			ituWidgetSetPosition(child, x, y + fix);
		}
		else
		{
			ituWidgetSetPosition(child, x + fix, y);
		}
	}
}

bool ituScaleCoverFlowUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUCoverFlow* coverflow = (ITUCoverFlow*) widget;
    ITUScaleCoverFlow* scalecoverflow = (ITUScaleCoverFlow*) widget;
    assert(scalecoverflow);

    if (ev == ITU_EVENT_TIMER && coverflow->inc)
    {
        result = true;
    }
    else if (((ev == ITU_EVENT_TOUCHSLIDELEFT || ev == ITU_EVENT_TOUCHSLIDERIGHT) && ((coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL) == 0)) ||
             ((ev == ITU_EVENT_TOUCHSLIDEUP || ev == ITU_EVENT_TOUCHSLIDEDOWN) && (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)))
    {
		if (ituWidgetIsEnabled(widget)) // && (widget->flags & ITU_DRAGGABLE))
		{
			int x = arg2 - widget->rect.x;
			int y = arg3 - widget->rect.y;

			if (ituWidgetIsInside(widget, x, y))
            {
                result = true;
            }
        }
    }
    else if (ev == ITU_EVENT_LAYOUT)
    {
        if (scalecoverflow->itemWidth > 0 || scalecoverflow->itemHeight > 0 || scalecoverflow->itemPos > 0)
        {
            int i, count = CoverFlowGetVisibleChildCount(coverflow);

            for (i = 0; i < count; ++i)
            {
                ITUWidget* child = CoverFlowGetVisibleChild(coverflow, i);

				if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
				{
					//ituWidgetSetX(child, scalecoverflow->itemPos);
					ituWidgetSetPositionFix(coverflow, child, scalecoverflow->itemPos, child->rect.y, i);
				}
				else
				{
					//ituWidgetSetY(child, scalecoverflow->itemPos);
					ituWidgetSetPositionFix(coverflow, child, child->rect.x, scalecoverflow->itemPos, i);
				}

                ituWidgetSetDimension(child, scalecoverflow->itemWidth, scalecoverflow->itemHeight);
            }            
        }
    }

    if (result)
    {
        int i, count = CoverFlowGetVisibleChildCount(coverflow);

		if ((coverflow->coverFlowFlags & ITU_COVERFLOW_CYCLE) || (coverflow->inc))
		{
			for (i = 0; i < count; ++i)
			{
				ITUWidget* child = CoverFlowGetVisibleChild(coverflow, i);

				if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
					ituWidgetSetX(child, scalecoverflow->itemPos);
				else
					ituWidgetSetY(child, scalecoverflow->itemPos);

				ituWidgetSetDimension(child, scalecoverflow->itemWidth, scalecoverflow->itemHeight);
			}
		}
    }

    result |= ituCoverFlowUpdate(widget, ev, arg1, arg2, arg3);

    if (ev == ITU_EVENT_TIMER)
    {
		if ((coverflow->inc == 0) && (coverflow->frame == 0) && scalecoverflow->draggable)
		{
			if (!(widget->flags & ITU_DRAGGABLE))
			{
				widget->flags |= ITU_DRAGGABLE;
				//printf("reset not draggable!\n");
			}
		}

        if (coverflow->inc)
        {
            int i, count = CoverFlowGetVisibleChildCount(coverflow);

			if (!(coverflow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
			{
				if ((coverflow->focusIndex == 0) && (coverflow->inc > 0))
				{
					coverflow->inc = 0;
				}
				else if ((coverflow->focusIndex == count) && (coverflow->inc < 0))
				{
					coverflow->inc = 0;
				}
				else
				{//<<< TIMER for non-cycle >>>
					int avail_count, index, factor, width, height, orgWidth, orgHeight, x, y;
					int scalWidth, scalHeight;
					int frame = coverflow->frame -1;

					if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
					{
						avail_count = (widget->rect.height / scalecoverflow->itemHeight);
					}
					else
					{
						avail_count = (widget->rect.width / scalecoverflow->itemWidth);
					}

					index = coverflow->focusIndex - (avail_count / 2) - 1;
					orgWidth = scalecoverflow->itemWidth;
					orgHeight = scalecoverflow->itemHeight;

					for (i = 0; i < (avail_count + 2); i++)
					{
						ITUWidget* child = CoverFlowGetVisibleChild(coverflow, index);

						if (child == NULL)
						{
							index++;
							continue;
						}

						if (index == coverflow->focusIndex)
						{
							float step = (float)frame / coverflow->totalframe;
							int factor_dx, factor_dy, dx, dy;

							if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
							{
								factor_dx = (int)((0.5 - (float)scalecoverflow->factor / 200.0) * orgWidth / 2.0);
								factor_dy = (int)((0.5 - (float)scalecoverflow->factor / 200.0) * orgHeight / 2.0) + orgHeight;
							}
							else
							{
								if (coverflow->inc > 0)
									factor_dx = orgWidth + ((100 - scalecoverflow->factor) * orgWidth / 200);
								else
									factor_dx = (orgWidth * scalecoverflow->factor / 100) + ((100 - scalecoverflow->factor) * orgWidth / 200);

								factor_dy = (100 - scalecoverflow->factor) * orgHeight / 200;
							}

							dx = (int)(factor_dx * step);
							dy = (int)(factor_dy * step);

							scalWidth = (int)((100.0 - ((100 - scalecoverflow->factor) * step)) * (float)orgWidth / 100.0);
							scalHeight = (int)((100.0 - ((100 - scalecoverflow->factor) * step)) * (float)orgHeight / 100.0);

							if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
							{
								x = (widget->rect.width / 2 - orgWidth / 2) + dx;
								y = (widget->rect.height / 2 - orgHeight / 2) + dy * ((coverflow->inc > 0) ? (1) : (-1));
							}
							else
							{
								x = (widget->rect.width / 2 - orgWidth / 2) + (dx * ((coverflow->inc > 0) ? (1) : (-1)));
								y = dy;
							}

							//ituWidgetSetPosition(child, x, y);
							ituWidgetSetPositionFix(coverflow, child, x, y, index);
							ituWidgetSetDimension(child, scalWidth, scalHeight);
						}
						else if (((coverflow->focusIndex - index) == 1) && (coverflow->inc > 0))
						{//the item (focus - 1) that will become focus
							float step = (float)frame / coverflow->totalframe;
							int factor_dx, factor_dy, dx, dy;

							if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
							{
								factor_dx = (((float)(100 - scalecoverflow->factor) * (float)orgWidth / 100.0) / 2);
								factor_dy = (orgHeight * scalecoverflow->factor / 100) + (((float)(100 - scalecoverflow->factor) * (float)orgHeight / 100.0) / 2);
							}
							else
							{
								factor_dx = (orgWidth * scalecoverflow->factor / 100) + (((float)(100 - scalecoverflow->factor) * (float)orgWidth / 100.0) / 2);
								factor_dy = (((float)(100 - scalecoverflow->factor) * (float)orgHeight / 100.0) / 2);
							}

							dx = (int)(factor_dx * step);
							dy = (int)(factor_dy * step);

							scalWidth  = (int)((100.0 - (scalecoverflow->factor * (1.0 - step))) * orgWidth / 100.0);
							scalHeight = (int)((100.0 - (scalecoverflow->factor * (1.0 - step))) * orgHeight / 100.0);

							if (coverflow->inc > 0)
							{
								child->flags &= ~ITU_PROGRESS;
							}
							else
							{
							}
							child->flags |= ITU_CLIP_DISABLED;


							if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
							{
								x = (orgWidth / 2 - (orgWidth * scalecoverflow->factor / 100) / 2) - dx;
								y = (widget->rect.height / 2 - orgHeight / 2) - factor_dy + dy;
							}
							else
							{
								x = (widget->rect.width / 2 - orgWidth / 2) - factor_dx + dx;
								y = (orgHeight / 2 - (orgHeight * scalecoverflow->factor / 100) / 2) - dy;
							}

							//ituWidgetSetPosition(child, x, y);
							ituWidgetSetPositionFix(coverflow, child, x, y, index);
							ituWidgetSetDimension(child, scalWidth, scalHeight);
						}
						else if (((coverflow->focusIndex - index) == -1) && (coverflow->inc < 0))
						{//the item (focus + 1) that will become focus
							float step = (float)frame / coverflow->totalframe;
							int factor_dx, factor_dy, dx, dy;

							if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
							{
								factor_dx = (((float)(100 - scalecoverflow->factor) * (float)orgWidth / 100.0) / 2);
								factor_dy = (orgHeight * (100 - scalecoverflow->factor) / 200) + orgHeight;
							}
							else
							{
								factor_dx = (orgWidth * (100 - scalecoverflow->factor) / 200) + orgWidth;
								factor_dy = (((float)(100 - scalecoverflow->factor) * (float)orgHeight / 100.0) / 2);
							}

							dx = (int)(factor_dx * step);
							dy = (int)(factor_dy * step);

							scalWidth = (int)((100.0 - (scalecoverflow->factor * (1.0 - step))) * orgWidth / 100.0);
							scalHeight = (int)((100.0 - (scalecoverflow->factor * (1.0 - step))) * orgHeight / 100.0);

							if (coverflow->inc > 0)
							{
								child->flags &= ~ITU_PROGRESS;
							}
							else
							{
							}
							child->flags |= ITU_CLIP_DISABLED;


							if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
							{
								x = (orgWidth / 2 - (orgWidth * scalecoverflow->factor / 100) / 2) - dx;
								y = (widget->rect.height / 2 + orgHeight / 2) + (orgHeight * (100 - scalecoverflow->factor) / 200) - dy;
							}
							else
							{
								x = (widget->rect.width / 2 + orgWidth / 2) + (orgWidth * (100 - scalecoverflow->factor) / 200) - dx;
								y = (orgHeight / 2 - (orgHeight * scalecoverflow->factor / 100) / 2) - dy;
							}

							//ituWidgetSetPosition(child, x, y);
							ituWidgetSetPositionFix(coverflow, child, x, y, index);
							ituWidgetSetDimension(child, scalWidth, scalHeight);
						}
						else if (coverflow->inc > 0)
						{// press (<<<)    motion -> -> ->
							int item_dist = coverflow->focusIndex - index;
							float step = (float)frame / coverflow->totalframe;
							int factor_dx, factor_dy, dx, dy;
							int obj_w = scalecoverflow->factor * orgWidth  / 100;
							int obj_h = scalecoverflow->factor * orgHeight / 100;

							if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
							{
								factor_dx = 0;
								factor_dy = orgHeight;
							}
							else
							{
								factor_dx = orgWidth;
								factor_dy = 0;
							}

							dx = (int)(factor_dx * step);
							dy = (int)(factor_dy * step);

							if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
							{
								x = (orgWidth - obj_w) / 2;
								y = (widget->rect.width / 2 - orgWidth / 2) - (item_dist * orgHeight) + ((orgHeight - obj_h) / 2) + dy;
							}
							else
							{
								x = (widget->rect.width / 2 - orgWidth / 2) - (item_dist * orgWidth) + ((orgWidth - obj_w) / 2) + dx;
								y = (orgHeight - obj_h) / 2;
							}

							ituWidgetSetDimension(child, obj_w, obj_h);
							//ituWidgetSetPosition(child, x, y);
							ituWidgetSetPositionFix(coverflow, child, x, y, index);
						}
						else
						{
							int item_dist = coverflow->focusIndex - index;
							float step = (float)frame / coverflow->totalframe;
							int factor_dx, factor_dy, dx, dy;
							int obj_w = (int)((float)scalecoverflow->factor * orgWidth / 100);
							int obj_h = (int)((float)scalecoverflow->factor * orgHeight / 100);

							if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
							{
								factor_dx = 0;
								factor_dy = orgHeight;
							}
							else
							{
								factor_dx = orgWidth;
								factor_dy = 0;
							}

							dx = (int)(factor_dx * step);
							dy = (int)(factor_dy * step);

							if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
							{
								x = (orgWidth - obj_w) / 2;
								y = (widget->rect.height / 2 - orgHeight / 2) - (item_dist * orgHeight) + ((orgHeight - obj_h) / 2) - dy;
							}
							else
							{
								x = (widget->rect.width / 2 - orgWidth / 2) - (item_dist * orgWidth) + ((orgWidth - obj_w) / 2) - dx;
								y = (orgHeight - obj_h) / 2;
							}

							ituWidgetSetDimension(child, obj_w, obj_h);
							//ituWidgetSetPosition(child, x, y);
							ituWidgetSetPositionFix(coverflow, child, x, y, index);

							//if (index == 2)
							//	printf("[child %d] [%d, %d] [%d, %d]\n", index, x, y, obj_w, obj_h);
						}

						//if (index == 0)
						//printf("child [%d] y %d\n", index, child->rect.y);
						index++;
					}
				}
			}
            else if (coverflow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
            {
                int index, count2, orgWidth, orgHeight, factor, width, height, x, y, itemCount2;
                int frame = coverflow->frame - 1;

                count2 = count / 2;
                if (coverflow->inc < 0)
                    count2++;

                index = coverflow->focusIndex;
                itemCount2 = scalecoverflow->itemCount / 2;
                if (coverflow->inc < 0)
                    itemCount2++;

                orgWidth = scalecoverflow->itemWidth;
                orgHeight = scalecoverflow->itemHeight;

                for (i = 0; i < count2; ++i)
                {
                    ITUWidget* child = CoverFlowGetVisibleChild(coverflow, index);

                    if (index == coverflow->focusIndex)
                    {
						float step = (float)frame / coverflow->totalframe;
						step = step - 1;
						step = step * step * step + 1;
						factor = (int)(100 - (100 - scalecoverflow->factor) * step);

                        width = orgWidth * factor / 100;
                        height = orgHeight * factor / 100;
                        x = child->rect.x + (orgWidth - width) / 2;
                        y = child->rect.y + (orgHeight - height) / 2;
                        //ituWidgetSetPosition(child, x, y);
						ituWidgetSetPositionFix(coverflow, child, x, y, index);
                        ituWidgetSetDimension(child, width, height);
                    }
                    else if (i == itemCount2)
                    {
						float step = (float)frame / coverflow->totalframe;
						step = step - 1;
						step = step * step * step + 1;

						if (coverflow->inc > 0)
                        {
                            int current_factor = scalecoverflow->factor;
                            int next_factor = scalecoverflow->factor / 2;
							factor = (int)(current_factor - (current_factor - next_factor) * step);
                        }
                        else
                        {
                            int current_factor = scalecoverflow->factor / 2;
                            int next_factor = scalecoverflow->factor;
							factor = (int)(current_factor + (next_factor - current_factor) * step);
                            child->flags &= ~ITU_PROGRESS;
                        }
                        child->flags |= ITU_CLIP_DISABLED;
                        
                        width = orgWidth * factor / 100;
                        height = orgHeight * factor / 100;

                        if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
                        {
                            x = child->rect.x + (orgWidth - width) / 2;
                            y = widget->rect.height / 2 - orgHeight / 2;
                            
                            if (coverflow->inc > 0)
                                y += orgHeight * i;
                            else
                                y += orgHeight * (i - 1);

                            y += (orgHeight - height) / 2;
                        }
                        else
                        {
                            x = widget->rect.width / 2 - orgWidth / 2;
                            
                            if (coverflow->inc > 0)
                                x += orgWidth * i;
                            else
                                x += orgWidth * (i - 1);

                            x += (orgWidth - width) / 2;
                            y = child->rect.y + (orgHeight - height) / 2;
                        }
                        //ituWidgetSetPosition(child, x, y);
						ituWidgetSetPositionFix(coverflow, child, x, y, index);
                        ituWidgetSetDimension(child, width, height);
                    }
                    else if ((child->flags & ITU_PROGRESS) == 0)
                    {
                        int focusIndex2;

                        if (coverflow->inc > 0)
                        {
                            if (coverflow->focusIndex == 0)
                                focusIndex2 = count - 1;
                            else
                                focusIndex2 = coverflow->focusIndex - 1;

							if (index == focusIndex2)
							{
								float step = (float)frame / coverflow->totalframe;
								step = step - 1;
								step = step * step * step + 1;

								factor = (int)(scalecoverflow->factor + (100 - scalecoverflow->factor) * step);
							}
                            else
                                factor = scalecoverflow->factor;
                        }
                        else
                        {
                            if (coverflow->focusIndex == count - 1)
                                focusIndex2 = 0;
                            else
                                focusIndex2 = coverflow->focusIndex + 1;

							if (index == focusIndex2)
							{
								float step = (float)frame / coverflow->totalframe;
								step = step - 1;
								step = step * step * step + 1;

								factor = (int)(scalecoverflow->factor + (100 - scalecoverflow->factor) * step);
							}
                            else
                                factor = scalecoverflow->factor;
                        }
                        width = orgWidth * factor / 100;
                        height = orgHeight * factor / 100;
                        x = child->rect.x + (orgWidth - width) / 2;
                        y = child->rect.y + (orgHeight - height) / 2;
                        //ituWidgetSetPosition(child, x, y);
						ituWidgetSetPositionFix(coverflow, child, x, y, index);
                        ituWidgetSetDimension(child, width, height);
                    }

                    if (index >= count - 1)
                        index = 0;
                    else
                        index++;
                }

                count2 = count - count2;
                itemCount2 = scalecoverflow->itemCount - itemCount2;
                for (i = 0; i < count2; ++i)
                {
                    ITUWidget* child = CoverFlowGetVisibleChild(coverflow, index);
                    int width, height, x, y;

                    if (i == count2 - itemCount2)
                    {
						float step = (float)frame / coverflow->totalframe;
						step = step - 1;
						step = step * step * step + 1;

                        if (coverflow->inc > 0)
                        {
                            int current_factor = scalecoverflow->factor / 2;
                            int next_factor = scalecoverflow->factor;
							factor = (int)(current_factor + (next_factor - current_factor) * step);
                            child->flags &= ~ITU_PROGRESS;
                        }
                        else
                        {
                            int current_factor = scalecoverflow->factor;
                            int next_factor = scalecoverflow->factor / 2;
							factor = (int)(current_factor - (current_factor - next_factor) * step);
                        }
                        child->flags |= ITU_CLIP_DISABLED;

                        width = orgWidth * factor / 100;
                        height = orgHeight * factor / 100;

                        if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
                        {
                            x = child->rect.x + (orgWidth - width) / 2;
                            y = widget->rect.height / 2 - orgHeight / 2;

                            if (coverflow->inc > 0)
                                y -= orgHeight * (itemCount2 - 1);
                            else
                                y -= orgHeight * itemCount2;

                            y += (orgHeight - height) / 2;
                        }
                        else
                        {
                            x = widget->rect.width / 2 - orgWidth / 2;

                            if (coverflow->inc > 0)
                                x -= orgWidth * (itemCount2 - 1);
                            else
                                x -= orgWidth * itemCount2;

                            x += (orgWidth - width) / 2;
                            y = child->rect.y + (orgHeight - height) / 2;
                        }
                        //ituWidgetSetPosition(child, x, y);
						ituWidgetSetPositionFix(coverflow, child, x, y, index);
                        ituWidgetSetDimension(child, width, height);
                    }
                    else if ((child->flags & ITU_PROGRESS) == 0)
                    {
                        int focusIndex2;

                        if (coverflow->inc > 0)
                        {
                            if (coverflow->focusIndex == 0)
                                focusIndex2 = count - 1;
                            else
                                focusIndex2 = coverflow->focusIndex - 1;

							if (index == focusIndex2)
							{
								float step = (float)frame / coverflow->totalframe;
								step = step - 1;
								step = step * step * step + 1;

								factor = (int)(scalecoverflow->factor + (100 - scalecoverflow->factor) * step);
							}
                            else
                                factor = scalecoverflow->factor;
                        }
                        else
                        {
                            if (coverflow->focusIndex == count - 1)
                                focusIndex2 = 0;
                            else
                                focusIndex2 = coverflow->focusIndex + 1;

							if (index == focusIndex2)
							{
								float step = (float)frame / coverflow->totalframe;
								step = step - 1;
								step = step * step * step + 1;

								factor = (int)(scalecoverflow->factor + (100 - scalecoverflow->factor) * step);
							}
                            else
                                factor = scalecoverflow->factor;
                        }
                        width = orgWidth * factor / 100;
                        height = orgHeight * factor / 100;
                        x = child->rect.x + (orgWidth - width) / 2;
                        y = child->rect.y + (orgHeight - height) / 2;
                        //ituWidgetSetPosition(child, x, y);
						ituWidgetSetPositionFix(coverflow, child, x, y, index);
                        ituWidgetSetDimension(child, width, height);
                    }

                    if (index >= count - 1)
                        index = 0;
                    else
                        index++;
                }
            }
        }
    }
	else if (ev == ITU_EVENT_MOUSEDOWN)
	{
		int x = arg2 - widget->rect.x;
		int y = arg3 - widget->rect.y;

		if (scalecoverflow->draggable && (ituWidgetIsEnabled(widget)) && (ituWidgetIsInside(widget, x, y)))
		{
			widget->flags |= ITU_DRAGGABLE;
		}

		if ((widget->flags & ITU_DRAGGABLE) && (ituWidgetIsEnabled(widget)) && (ituWidgetIsInside(widget, x, y)))
		{
			int i, count = CoverFlowGetVisibleChildCount(coverflow);

			for (i = 0; i < count; i++)
			{
				ITUWidget* child = CoverFlowGetVisibleChild(coverflow, i);
				scalecoverflow->layoutMemoX[i] = child->rect.x;
				scalecoverflow->layoutMemoY[i] = child->rect.y;
				//printf("child %d,  %d, %d\n", i, child->rect.x, child->rect.y);
			}

			scalecoverflow->currentFocusIndex = coverflow->focusIndex;

			widget->flags |= ITU_DRAGGING;
			ituScene->dragged = widget;
		}
	}
	else if (ev == ITU_EVENT_MOUSEUP)
	{
		if (widget->flags & ITU_DRAGGABLE)
		{
			widget->flags |= ITU_UNDRAGGING;
			ituScene->dragged = NULL;

			coverflow->inc = 0;
			coverflow->frame = 0;
			ituWidgetUpdate(scalecoverflow, ITU_EVENT_LAYOUT, 0, 0, 0);
			widget->flags &= ~ITU_DRAGGABLE;

			if (scalecoverflow->currentFocusIndex != coverflow->focusIndex)
			{
				int x = arg2 - widget->rect.x;
				int y = arg3 - widget->rect.y;

				if (ituWidgetIsInside(widget, x, y))
				ituExecActions(widget, coverflow->actions, ITU_EVENT_CHANGED, coverflow->focusIndex);
			}
		}
	}
	else if (ev == ITU_EVENT_MOUSEMOVE)
	{

		if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGING))
		{
			int i, orgWidth, orgHeight, count = CoverFlowGetVisibleChildCount(coverflow);
			int width, height, neib;
			float step;
			int xm = arg2 - widget->rect.x;
			int ym = arg3 - widget->rect.y;
			//printf("move f %d, org %d\n", coverflow->focusIndex, scalecoverflow->currentFocusIndex);
			//fix for drag outside widget
			if (!(ituWidgetIsInside(scalecoverflow, xm, ym)))
			{
				ituWidgetUpdate(scalecoverflow, ITU_EVENT_LAYOUT, 0, 0, 0);
				//ituWidgetUpdate(scalecoverflow, ITU_EVENT_MOUSEDOWN, 0, arg2, arg3);
				return true;
			}

			if (scalecoverflow->currentFocusIndex != coverflow->focusIndex)
			{
				ituExecActions(widget, coverflow->actions, ITU_EVENT_CHANGED, coverflow->focusIndex);
				ituWidgetUpdate(scalecoverflow, ITU_EVENT_LAYOUT, 0, 0, 0);
				ituWidgetUpdate(scalecoverflow, ITU_EVENT_MOUSEDOWN, 0, arg2, arg3);
				
			}

			if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
			{
				scalecoverflow->offsetx = 0;
				scalecoverflow->offsety = ym - coverflow->touchPos;

				if (!(coverflow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
				{
					if (((coverflow->focusIndex == 0) && (scalecoverflow->offsety > 0))
						|| ((coverflow->focusIndex == (count - 1)) && (scalecoverflow->offsety < 0)))
					{
						ituWidgetUpdate(scalecoverflow, ITU_EVENT_LAYOUT, 0, 0, 0);
						return true;
					}


					if (scalecoverflow->offsety >= (scalecoverflow->itemHeight * scalecoverflow->factor / 100))
					{
						if (coverflow->focusIndex > 0)
							coverflow->focusIndex--;

						ituExecActions(widget, coverflow->actions, ITU_EVENT_CHANGED, coverflow->focusIndex);
						ituWidgetUpdate(scalecoverflow, ITU_EVENT_LAYOUT, 0, 0, 0);
						ituWidgetUpdate(scalecoverflow, ITU_EVENT_MOUSEDOWN, 0, arg2, arg3);
					}
					else if (scalecoverflow->offsety <= (-1 * scalecoverflow->itemHeight * scalecoverflow->factor / 100))
					{
						if (coverflow->focusIndex < (count - 1))
							coverflow->focusIndex++;

						ituExecActions(widget, coverflow->actions, ITU_EVENT_CHANGED, coverflow->focusIndex);
						ituWidgetUpdate(scalecoverflow, ITU_EVENT_LAYOUT, 0, 0, 0);
						ituWidgetUpdate(scalecoverflow, ITU_EVENT_MOUSEDOWN, 0, arg2, arg3);
					}
				}

				step = (float)(abs(scalecoverflow->offsety)) / (float)(scalecoverflow->itemHeight);

				neib = ((scalecoverflow->offsety >= 0) ? (1) : (-1));
				neib = coverflow->focusIndex - neib;

				if (neib < 0)
					neib = count - 1;
				else if (neib >= count)
					neib = 0;
			}
			else
			{
				scalecoverflow->offsetx = xm - coverflow->touchPos;
				scalecoverflow->offsety = 0;

				if (!(coverflow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
				{
					if (((coverflow->focusIndex == 0) && (scalecoverflow->offsetx > 0))
						|| ((coverflow->focusIndex == (count - 1)) && (scalecoverflow->offsetx < 0)))
					{
						ituWidgetUpdate(scalecoverflow, ITU_EVENT_LAYOUT, 0, 0, 0);
						return true;
					}


					if (scalecoverflow->offsetx >= (scalecoverflow->itemWidth * scalecoverflow->factor / 100))
					{
						if (coverflow->focusIndex > 0)
							coverflow->focusIndex--;

						ituWidgetUpdate(scalecoverflow, ITU_EVENT_LAYOUT, 0, 0, 0);
						ituWidgetUpdate(scalecoverflow, ITU_EVENT_MOUSEDOWN, 0, arg2, arg3);
					}
					else if (scalecoverflow->offsetx <= (-1 * scalecoverflow->itemWidth * scalecoverflow->factor / 100))
					{
						if (coverflow->focusIndex < (count - 1))
							coverflow->focusIndex++;

						ituWidgetUpdate(scalecoverflow, ITU_EVENT_LAYOUT, 0, 0, 0);
						ituWidgetUpdate(scalecoverflow, ITU_EVENT_MOUSEDOWN, 0, arg2, arg3);
					}
				}

				step = (float)(abs(scalecoverflow->offsetx)) / (float)(scalecoverflow->itemWidth);

				neib = ((scalecoverflow->offsetx >= 0) ? (1) : (-1));
				neib = coverflow->focusIndex - neib;

				if (neib < 0)
					neib = count - 1;
				else if (neib >= count)
					neib = 0;
			}

			//printf("focus is %d neib is %d, tpos %d\n", coverflow->focusIndex, neib, coverflow->touchPos);

			orgWidth = scalecoverflow->itemWidth;
			orgHeight = scalecoverflow->itemHeight;

			if (1)//(coverflow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
			{
				int x, y, factor;
				step = step - 1;
				step = step * step * step + 1;
				factor = (int)(100 - (100 - scalecoverflow->factor) * step);

				for (i = 0; i < count; i++)
				{
					ITUWidget* child = CoverFlowGetVisibleChild(coverflow, i);
					ituWidgetSetPosition(child, scalecoverflow->layoutMemoX[i] + scalecoverflow->offsetx, scalecoverflow->layoutMemoY[i] + scalecoverflow->offsety);
					//ituWidgetSetPosition(coverflow, child, scalecoverflow->layoutMemoX[i] + scalecoverflow->offsetx, scalecoverflow->layoutMemoY[i] + scalecoverflow->offsety, i);

					if (i == coverflow->focusIndex)
					{
						width  = (orgWidth * factor)  / 100;
						height = (orgHeight * factor) / 100;

						x = child->rect.x + ((orgWidth - width) / 2);
						y = child->rect.y + ((orgHeight - height) / 2);

						ituWidgetSetPosition(child, x, y);
						//ituWidgetSetPositionFix(coverflow, child, x, y, i);
						ituWidgetSetDimension(child, width, height);
					}
					else if (i == neib)
					{
						width  = (orgWidth  * (100 - (factor - scalecoverflow->factor))) / 100;
						height = (orgHeight * (100 - (factor - scalecoverflow->factor))) / 100;

						x = child->rect.x - (width - (orgWidth * scalecoverflow->factor / 100)) / 2;
						y = child->rect.y - (height - (orgHeight * scalecoverflow->factor / 100)) / 2;

						ituWidgetSetPosition(child, x, y);
						//ituWidgetSetPositionFix(coverflow, child, x, y, i);
						ituWidgetSetDimension(child, width, height);
					}
					else
					{
						ituWidgetSetPosition(child, child->rect.x, child->rect.y); //Bless
						//ituWidgetSetPositionFix(coverflow, child, child->rect.x, child->rect.y, i);
						ituWidgetSetDimension(child, orgWidth * scalecoverflow->factor / 100, orgHeight * scalecoverflow->factor / 100);
					}
				}
			}

		}
	}
    else if (ev == ITU_EVENT_LAYOUT)
    {
        int i, orgWidth, orgHeight, count = CoverFlowGetVisibleChildCount(coverflow);

        if (scalecoverflow->itemWidth == 0 && scalecoverflow->itemHeight == 0 && scalecoverflow->itemPos == 0)
        {
            ITUWidget* child = CoverFlowGetVisibleChild(coverflow, coverflow->focusIndex);
            scalecoverflow->itemWidth = child->rect.width;
            scalecoverflow->itemHeight = child->rect.height;

            if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
                scalecoverflow->itemPos = child->rect.x;
            else
                scalecoverflow->itemPos = child->rect.y;
        }
        orgWidth = scalecoverflow->itemWidth;
        orgHeight = scalecoverflow->itemHeight;

		if (!(coverflow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
		{
			int index;
			int avail_count;
			int factor, width, height, x, y;

			if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
			{
				avail_count = (widget->rect.height / scalecoverflow->itemHeight);
			}
			else
			{
				avail_count = (widget->rect.width / scalecoverflow->itemWidth);
			}

			index = coverflow->focusIndex - (avail_count / 2) - 1; //use -1 to take care outside child will draw when going inside

			for (i = 0; i < count; i++)
			{
				ITUWidget* child = CoverFlowGetVisibleChild(coverflow, i);
				int valid_1 = ((index < 0) ? (0) : (index));
				int valid_2 = (((index + avail_count) > (count - 1)) ? (count - 1) : (index + avail_count));

				if ((i < valid_1) || (i > valid_2))
				{
					child->flags |= ITU_PROGRESS;
				}

				factor = scalecoverflow->factor;

				if (i == coverflow->focusIndex)
					factor = 100;

				width = orgWidth * factor / 100;
				height = orgHeight * factor / 100;
				x = child->rect.x + (orgWidth - width) / 2;
				y = child->rect.y + (orgHeight - height) / 2;
				//ituWidgetSetPosition(child, x, y);
				ituWidgetSetPositionFix(coverflow, child, x, y, i);
				ituWidgetSetDimension(child, width, height);

				child->flags &= ~ITU_CLIP_DISABLED;

				/*if (i == coverflow->focusIndex)
					printf("[child %d][F] [%d, %d] [%d, %d]\n", i, x, y, width, height);
				else
					printf("[child %d] [%d, %d] [%d, %d]\n", i, x, y, width, height);*/
			}

			if (index < 0)
				index = 0;

			for (i = 0; i < (avail_count + 2); i++) //use +2 to make sure outside child will draw when going inside
			{
				ITUWidget* child = CoverFlowGetVisibleChild(coverflow, index);

				if (child == NULL)
				{
					index++;
					continue;
				}

				child->flags &= ~ITU_PROGRESS;

				index++;
			}
		}
		else //if (coverflow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
        {
            int index, count2, factor, width, height, x, y, itemCount2;
            
            count2 = count / 2 + 1;
            index = coverflow->focusIndex;
            itemCount2 = scalecoverflow->itemCount / 2 + 1;

            for (i = 0; i < count2; ++i)
            {
                ITUWidget* child = CoverFlowGetVisibleChild(coverflow, index);

                if (i < itemCount2)
                {
                    if (index == coverflow->focusIndex)
                        factor = 100;
                    else
                        factor = scalecoverflow->factor;

                    width = orgWidth * factor / 100;
                    height = orgHeight * factor / 100;
                    x = child->rect.x + (orgWidth - width) / 2;
                    y = child->rect.y + (orgHeight - height) / 2;
                    //ituWidgetSetPosition(child, x, y);
					ituWidgetSetPositionFix(coverflow, child, x, y, index);
                    ituWidgetSetDimension(child, width, height);
                    child->flags &= ~ITU_PROGRESS;
                }
                else
                {
                    child->flags |= ITU_PROGRESS;
                }
                child->flags &= ~ITU_CLIP_DISABLED;

                if (index >= count - 1)
                    index = 0;
                else
                    index++;
            }

            count2 = count - count2;
            itemCount2 = scalecoverflow->itemCount - itemCount2;
            for (i = 0; i < count2; ++i)
            {
                ITUWidget* child = CoverFlowGetVisibleChild(coverflow, index);

                if (i >= count2 - itemCount2)
                {
                    factor = scalecoverflow->factor;

                    width = orgWidth * factor / 100;
                    height = orgHeight * factor / 100;
                    x = child->rect.x + (orgWidth - width) / 2;
                    y = child->rect.y + (orgHeight - height) / 2;
                    //ituWidgetSetPosition(child, x, y);
					ituWidgetSetPositionFix(coverflow, child, x, y, index);
                    ituWidgetSetDimension(child, width, height);
                    child->flags &= ~ITU_PROGRESS;
                }
                else
                {
                    child->flags |= ITU_PROGRESS;
                }
                child->flags &= ~ITU_CLIP_DISABLED;

                if (index >= count - 1)
                    index = 0;
                else
                    index++;
            }
        }
    }

    return widget->visible ? result : false;
}

void ituScaleCoverFlowDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    ITUCoverFlow* coverflow = (ITUCoverFlow*)widget;
    ITURectangle prevClip;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    ITCTree* node;
    int count, index, count2, i;
    assert(widget);
    assert(dest);
   
    ituWidgetSetClipping(widget, dest, x, y, &prevClip);

    x += widget->rect.x;
    y += widget->rect.y;
    alpha = alpha * widget->alpha / 255;

	if (!(coverflow->coverFlowFlags & ITU_COVERFLOW_CYCLE))
	{
		int avail_count;
		ITUScaleCoverFlow* scalecoverflow = (ITUScaleCoverFlow*)widget;

		for (node = widget->tree.child; node; node = node->sibling)
		{
			ITUWidget* child = (ITUWidget*)node;
			if (child->visible && !(child->flags & ITU_PROGRESS) && (child->flags & ITU_CLIP_DISABLED) && ituWidgetIsOverlapClipping(child, dest, x, y))
				ituWidgetDraw(node, dest, x, y, alpha);

			child->dirty = false;
		}

		if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
		{
			avail_count = (widget->rect.height / scalecoverflow->itemHeight);
		}
		else
		{
			avail_count = (widget->rect.width / scalecoverflow->itemWidth);
		}

		index = coverflow->focusIndex - (avail_count / 2) - 1;

		for (i = 0; i < (avail_count + 2); i++)
		{
			ITUWidget* child = CoverFlowGetVisibleChild(coverflow, index);

			if (child == NULL)
			{
				index++;
				continue;
			}

			if (child->visible && !(child->flags & ITU_PROGRESS) && !(child->flags & ITU_CLIP_DISABLED) && ituWidgetIsOverlapClipping(child, dest, x, y))
				ituWidgetDraw(child, dest, x, y, alpha);

			child->dirty = false;

			index++;
		}
	}
	else
	{//
		for (node = widget->tree.child; node; node = node->sibling)
		{
			ITUWidget* child = (ITUWidget*)node;
			if (child->visible && !(child->flags & ITU_PROGRESS) && (child->flags & ITU_CLIP_DISABLED) && ituWidgetIsOverlapClipping(child, dest, x, y))
				ituWidgetDraw(node, dest, x, y, alpha);

			child->dirty = false;
		}

		count = CoverFlowGetVisibleChildCount(coverflow);
		count2 = count / 2;
		if ((coverflow->inc < 0 && coverflow->frame < coverflow->totalframe / 2) ||
			(coverflow->inc >= 0 && coverflow->frame >= coverflow->totalframe / 2))
			count2++;

		if (coverflow->focusIndex - count2 >= 0)
			index = coverflow->focusIndex - count2;
		else
			index = count - 1 - (count2 - 1 - coverflow->focusIndex);

		//printf("focus=%d start=%d ", coverflow->focusIndex, index);

		if (index < 0)
			index = 0;

		for (i = 0; i < count2; ++i)
		{
			ITUWidget* child = CoverFlowGetVisibleChild(coverflow, index);

			if (child->visible && !(child->flags & ITU_PROGRESS) && !(child->flags & ITU_CLIP_DISABLED) && ituWidgetIsOverlapClipping(child, dest, x, y))
				ituWidgetDraw(child, dest, x, y, alpha);

			child->dirty = false;

			if (++index >= count)
				index = 0;
		}

		count2 = count - count2;

		if (coverflow->focusIndex + count2 - 1 < count)
			index = coverflow->focusIndex + count2 - 1;
		else
			index = count2 - 1 - (count - coverflow->focusIndex);

		//printf("end=%d\n", index);
		if (index < 0)
			index = 0;

		for (i = 0; i < count2; ++i)
		{
			ITUWidget* child = CoverFlowGetVisibleChild(coverflow, index);

			if (child->visible && !(child->flags & ITU_PROGRESS) && !(child->flags & ITU_CLIP_DISABLED) && ituWidgetIsOverlapClipping(child, dest, x, y))
				ituWidgetDraw(child, dest, x, y, alpha);

			child->dirty = false;

			if (--index < 0)
				index = count - 1;
		}
	}

    ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
    ituDirtyWidget(widget, false);
}

void ituScaleCoverFlowInit(ITUScaleCoverFlow* scaleCoverFlow, ITULayout layout)
{
    assert(scaleCoverFlow);
    ITU_ASSERT_THREAD();

    memset(scaleCoverFlow, 0, sizeof (ITUScaleCoverFlow));

    ituCoverFlowInit(&scaleCoverFlow->coverFlow, layout);

    ituWidgetSetType(scaleCoverFlow, ITU_IMAGECOVERFLOW);
    ituWidgetSetName(scaleCoverFlow, scaleCoverFlowName);
    ituWidgetSetUpdate(scaleCoverFlow, ituScaleCoverFlowUpdate);
    ituWidgetSetDraw(scaleCoverFlow, ituScaleCoverFlowDraw);
	ituWidgetSetOnAction(scaleCoverFlow, ituScaleCoverFlowOnAction);
}

void ituScaleCoverFlowLoad(ITUScaleCoverFlow* scaleCoverFlow, uint32_t base)
{
    assert(scaleCoverFlow);

	if (!(scaleCoverFlow->coverFlow.coverFlowFlags & ITU_COVERFLOW_CYCLE))
	{
		scaleCoverFlow->coverFlow.boundaryAlign = 0;
	}

	//Bless
	if (scaleCoverFlow && scaleCoverFlow->draggable)
	{
		//ITUWidget* widget = (ITUWidget*)(&(scaleCoverFlow->coverFlow));
		//widget->flags |= ITU_DRAGGABLE;
		scaleCoverFlow->orgFocusIndex = scaleCoverFlow->coverFlow.focusIndex;
	}

    ituCoverFlowLoad(&scaleCoverFlow->coverFlow, base);
    ituWidgetSetUpdate(scaleCoverFlow, ituScaleCoverFlowUpdate);
    ituWidgetSetDraw(scaleCoverFlow, ituScaleCoverFlowDraw);
	ituWidgetSetOnAction(scaleCoverFlow, ituScaleCoverFlowOnAction);
}

void ituScaleCoverFlowOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
	ITUScaleCoverFlow* scalecoverFlow = (ITUCoverFlow*)widget;
	assert(scalecoverFlow);

	switch (action)
	{
	case ITU_ACTION_PREV:
		widget->flags &= ~ITU_DRAGGABLE;
		ituCoverFlowPrev((ITUCoverFlow*)widget);
		break;

	case ITU_ACTION_NEXT:
		widget->flags &= ~ITU_DRAGGABLE;
		ituCoverFlowNext((ITUCoverFlow*)widget);
		break;

	default:
		ituWidgetOnActionImpl(widget, action, param);
		break;
	}
}

void ituScaleCoverFlowNext(ITUScaleCoverFlow* scalecoverflow)
{
	ituCoverFlowNext(scalecoverflow);
}

void ituScaleCoverFlowPrev(ITUScaleCoverFlow* scalecoverflow)
{
	ituCoverFlowPrev(scalecoverflow);
}


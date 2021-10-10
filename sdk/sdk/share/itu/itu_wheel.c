#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"


static const char wheelName[] = "ITUWheel";

//SPEED BASE range: 3 ~ 5
#define STEP_SPEED_BASE 5  //4
#define MOTION_FACTOR 20  //60
#define MOTION_THRESHOLD 40
#define PROCESS_STAGE1 0.1f  //0.2f
#define PROCESS_STAGE2 0.2f  //0.4f
//MAX_INIT_POWER range: 2.0 ~ 4.5
#define MAX_INIT_POWER 4.5
#define FOCUSFONT_POS_Y_FIX_DIV ITU_WHEEL_FOCUS_FONT_FIX_POS
#define SLIDE_INIT_FRAME 3
//Make change when (Drag dist) > (item_height / MOUSEUP_CHANGE_FACTOR) (default: 2)
#define MOUSEUP_CHANGE_FACTOR 3

static void use_normal_color(ITUWidget* widget, ITUColor* color)
{
	ITUWheel* wheel = (ITUWheel*)widget;
	ITUColor Cr;

	assert(wheel);

	Cr.alpha = wheel->normalColor.alpha;
	Cr.red   = wheel->normalColor.red;
	Cr.green = wheel->normalColor.green;
	Cr.blue  = wheel->normalColor.blue;
	memcpy(color, &Cr, sizeof (ITUColor));

	return;
}

static bool focus_change(ITUWidget* widget, int newfocus, int line)
{
	ITUWheel* wheel = (ITUWheel*)widget;
	ITUWidget* child;
	ITUColor color;
	ITUText* text;
	int check_1, check_2;

	assert(wheel);

	if (wheel->cycle_tor <= 0)
		return false;

	check_1 = ((wheel->focus_c - 1) < wheel->minci) ? (wheel->maxci) : (wheel->focus_c - 1);
	check_2 = ((wheel->focus_c + 1) > wheel->maxci) ? (wheel->minci) : (wheel->focus_c + 1);

	if ((newfocus != check_1) && (newfocus != check_2))
	{
		//printf("===[wheel][prev][next] [%d %d]===\n", wheel->focus_c, newfocus);
		return false;
	}

	//memcpy(&color, &NColor, sizeof (ITUColor));
	use_normal_color(widget, &color);
	child = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->focus_c);
	text = (ITUText*)child;

	ituWidgetSetColor(child, color.alpha, color.red, color.green, color.blue);
	if (wheel->fontsquare == 1)
		ituTextSetFontSize(text, wheel->fontHeight);
	else
		ituTextSetFontHeight(text, wheel->fontHeight);
	//printf("===[wheel][last][child %d][focusIndex %d][focusStr: %s][line: %d]===\n", wheel->focus_c, wheel->focusIndex, ituTextGetString(text), line);

	wheel->focus_c = newfocus;
	wheel->focusIndex = newfocus - wheel->focus_dev;
	child = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->focus_c);
	text = (ITUText*)child;

	ituWidgetSetColor(child, wheel->focusColor.alpha, wheel->focusColor.red, wheel->focusColor.green, wheel->focusColor.blue);
	if (wheel->fontsquare == 1)
		ituTextSetFontSize(text, wheel->focusFontHeight);
	else
		ituTextSetFontHeight(text, wheel->focusFontHeight);

	//printf("===[wheel][now][child %d][focusIndex %d][focusStr: %s][line: %d]===\n", wheel->focus_c, wheel->focusIndex, ituTextGetString(text), line);

	return true;
}

static void cycle_arrange(ITUWidget* widget, bool shiftway)
{
	int i = 0;
	int fy = 0;
	ITUWheel* wheel = (ITUWheel*)widget;
	ITUWidget* child;
	ITUText* text;

	assert(wheel);

	if (wheel->cycle_arr_count <= 0)
		return;

	if (shiftway)
	{
		child = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->cycle_arr[wheel->cycle_arr_count - 1]);
		ituWidgetSetY(child, wheel->tempy);

		for (i = 0; i < wheel->cycle_arr_count; i++)
		{
			wheel->cycle_arr[i]--;
			
			if (wheel->cycle_arr[i] < wheel->minci)
				wheel->cycle_arr[i] = wheel->maxci;
		}
	}
	else
	{
		child = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->cycle_arr[0]);
		ituWidgetSetY(child, wheel->tempy);

		for (i = 0; i < wheel->cycle_arr_count; i++)
		{
			wheel->cycle_arr[i]++;

			if (wheel->cycle_arr[i] > wheel->maxci)
				wheel->cycle_arr[i] = wheel->minci;
		}
	}

	for (i = 0; i < wheel->cycle_arr_count; i++)
	{
		int focus_fy = 0;
		child = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->cycle_arr[i]);
		text = (ITUText*)child;

		if (text->fontIndex >= ITU_FREETYPE_MAX_FONTS)
			text->fontIndex = 0;

		if (i == 0)
			fy = 0 - child->rect.height * (wheel->itemCount / 2 + wheel->cycle_tor) + (wheel->itemCount / 4 * child->rect.height);

		if (wheel->cycle_arr[i] == wheel->focus_c)
		{
			int focus_fy = fy - ((wheel->focusFontHeight - wheel->fontHeight) / FOCUSFONT_POS_Y_FIX_DIV);
			wheel->layout_ci = (fy - wheel->focus_c * child->rect.height) * (-1);
			ituWidgetSetY(child, focus_fy);
			//printf("===[wheel][cycle][focus at child %d][fy %d][layout_ci %d]===\n", wheel->focus_c, focus_fy, wheel->layout_ci);
		}
		else
		{
			ituWidgetSetY(child, fy);
		}
		//printf("===[wheel][cycle][%d][child %d][str %s][fy %d]===\n", i, wheel->cycle_arr[i], ituTextGetString(text), fy);
		fy += child->rect.height;
	}

	return;
}

static void get_normal_color(ITUWidget* widget)
{
	int i, count;
	ITUWheel* wheel = (ITUWheel*)widget;
	ITUWidget* child;
	ITUText* text;

	assert(wheel);

	count = itcTreeGetChildCount(wheel);

	for (i = 0; i < count; i++)
	{
		char* str = NULL;
		child = (ITUWidget*)itcTreeGetChildAt(wheel, i);
		text = (ITUText*)child;

		if (text->fontIndex >= ITU_FREETYPE_MAX_FONTS)
			text->fontIndex = 0;

		str = ituTextGetString(text);

		if (str && (str[0] != '\0'))
		{
			if ((child->color.alpha == wheel->focusColor.alpha) && (child->color.red == wheel->focusColor.red) && (child->color.green == wheel->focusColor.green) && (child->color.blue == wheel->focusColor.blue))
			{
				continue;
			}
			else
			{
				//memcpy(color, &child->color, sizeof (ITUColor));
				wheel->normalColor.alpha = child->color.alpha;
				wheel->normalColor.red   = child->color.red;
				wheel->normalColor.green = child->color.green;
				wheel->normalColor.blue  = child->color.blue;
				break;
			}
		}
	}

	return;
}

static int get_max_focusindex(ITUWidget* widget)
{
	ITUWheel* wheel = (ITUWheel*)widget;
	int i;
	int realcount = 0;
	int count;
	ITUWidget* child;
	ITUText* text;

	assert(wheel);

	count = itcTreeGetChildCount(wheel);

	for (i = 0; i < count; i++)
	{
		char* str = NULL;

		child = (ITUWidget*)itcTreeGetChildAt(wheel, i);
		text = (ITUText*)child;

		if (text->fontIndex >= ITU_FREETYPE_MAX_FONTS)
			text->fontIndex = 0;

		str = ituTextGetString(text);

		if (str && (str[0] != '\0'))
		{
			realcount++;
		}
	}

	realcount--;

	return realcount;
}

bool ituWheelUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUWheel* wheel = (ITUWheel*) widget;

	ITUWidget* child_check = (ITUWidget*)itcTreeGetChildAt(wheel, 0);
	int good_center;
	int fitc = (widget->rect.height / child_check->rect.height);

	if ((widget->rect.height % child_check->rect.height) > (child_check->rect.height / 2))
		fitc++;

	good_center = (fitc / 2) + (fitc % 2);
	widget->rect.height = (fitc * child_check->rect.height) + (fitc + 1);

    assert(wheel);

	if (ev == ITU_EVENT_LAYOUT)
	{
		ITUText* text;
		int i, fy, height;
		int count = itcTreeGetChildCount(wheel);
		ITUColor color;

		//set custom data to original totalframe
		//ituWidgetSetCustomData(widget, wheel->totalframe);

		if (wheel->scal < 1) //for scal
			wheel->scal = 1;

		wheel->moving_step = 0; //useless now

		wheel->itemCount = count;

		if (wheel->inside > 0) //for inside check
			wheel->inside = 0;

		if ((wheel->slide_step <= 0) || (wheel->slide_step > 10)) //for wheel slide step speed
			wheel->slide_step = 2;

		if (wheel->idle > 0) //for state check
			wheel->idle = 0;

		if (wheel->cycle_tor > 0) //LAYOUT cycle mode
		{//bless
			ITUWidget* child;
			int realcount = 0;
			int si = 0;
			int fitmod = ((fitc % 3) > 0) ? (fitc / 2) : (0);

			//fix for wheel itemcount with wrong rect real size
			if (wheel->itemCount != (fitc + 2 + fitmod))
			{
				wheel->itemCount = (fitc + 2 + fitmod);

				if (wheel->itemCount > count)
					wheel->itemCount = count;
			}

			for (i = 0; i < count; i++)
			{
				char* str = NULL;

				child = (ITUWidget*)itcTreeGetChildAt(wheel, i);
				text = (ITUText*)child;

				if (text->fontIndex >= ITU_FREETYPE_MAX_FONTS)
					text->fontIndex = 0;

				str = ituTextGetString(text);

				if (str && str[0] != '\0')
				{
					realcount++;

					if ((realcount - 1) == wheel->focusIndex)
					{
						wheel->focus_c = i;
						wheel->focus_dev = i - wheel->focusIndex;
					}

					if (wheel->minci == 0)
						wheel->minci = i;
				}
			}

			if ((wheel->minci == 1) && (realcount == count))
				wheel->minci = 0;

			wheel->maxci = wheel->minci + realcount - 1;

			si = wheel->focus_c;
			
			for (i = 0; i < (wheel->itemCount / 2 + wheel->cycle_tor); i++)
			{
				si--;
				if (si < wheel->minci)
					si = wheel->maxci;
			}
			
			wheel->cycle_arr_count = 0;

			for (i = 0; i < (wheel->itemCount + 2 * wheel->cycle_tor); i++)
			{
				//char* str;
				int focus_fy = 0;

				if (i == 0)
				{
					get_normal_color(widget);
					//memcpy(&color, &NColor, sizeof (ITUColor));
					use_normal_color(widget, &color);
					height = child->rect.height;
					fy = 0 - height * (wheel->itemCount / 2 + wheel->cycle_tor) + (wheel->itemCount / 4 * height);
					//fy = 0 - ((wheel->focusIndex + wheel->focus_dev - good_center + 1) * height);
					wheel->tempy = fy - (height * wheel->cycle_tor * 2);
				}

				child = (ITUWidget*)itcTreeGetChildAt(wheel, si);
				text = (ITUText*)child;
				focus_fy = fy - ((wheel->focusFontHeight - wheel->fontHeight) / FOCUSFONT_POS_Y_FIX_DIV);

				if (si == wheel->focus_c)
				{
					ituWidgetSetColor(child, wheel->focusColor.alpha, wheel->focusColor.red, wheel->focusColor.green, wheel->focusColor.blue);

					//child->rect.height = height = wheel->focusFontHeight;
					if (wheel->fontsquare == 1)
						ituTextSetFontSize(text, wheel->focusFontHeight);
					else
						ituTextSetFontHeight(text, wheel->focusFontHeight);
					//fy -= (wheel->focusFontHeight - wheel->fontHeight) / FOCUSFONT_POS_Y_FIX_DIV;
					wheel->layout_ci = (fy - wheel->focus_c * height) * (-1);
					//wheel->layout_ci += ((wheel->focusFontHeight - wheel->fontHeight) / FOCUSFONT_POS_Y_FIX_DIV);
					//printf("===[wheel][layout][focus at child %d][fy %d][layout_ci %d]===\n", wheel->focus_c, fy, wheel->layout_ci);
					ituWidgetSetY(child, focus_fy);
					//printf("===[wheel %s][layout][focus child %d][%s][fy %d]===\n", widget->name, si, ituTextGetString(text), focus_fy);
				}
				else
				{
					ituWidgetSetColor(child, color.alpha, color.red, color.green, color.blue);
					if (wheel->fontsquare == 1)
						ituTextSetFontSize(text, wheel->fontHeight);
					else
						ituTextSetFontHeight(text, wheel->fontHeight);

					ituWidgetSetY(child, fy);
				}

				//ituWidgetSetY(child, fy);
				wheel->cycle_arr[i] = si;
				wheel->cycle_arr_count++;
				//str = ituTextGetString(text);
				printf("===[wheel %s][layout][child %d][%s][fy %d]===\n", widget->name, si, ituTextGetString(text), fy);

				//child->rect.height = height = height0;
				fy += height;

				if ((si + 1) > wheel->maxci)
					si = wheel->minci;
				else
					si++;
			}
			widget->dirty = true;
			result |= widget->dirty;

			//fix for undragging trigger timer will doing trash animation
			wheel->frame = wheel->totalframe;

			widget->flags |= ITU_UNDRAGGING;
		}
		else //LAYOUT no cycle
		{
			//fix wrong count
			for (i = 0; i < count; i++)
			{
				char* str = NULL;
				ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(wheel, i);
				text = (ITUText*)child;

				if (text->fontIndex >= ITU_FREETYPE_MAX_FONTS)
					text->fontIndex = 0;

				str = ituTextGetString(text);

				if (str && str[0] != '\0')
				{
					wheel->focus_dev = i;
					break;
				}
			}

			for (i = 0; i < count; ++i)
			{
				ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(wheel, i);

				if (i == 0)
				{
					get_normal_color(widget);
					//memcpy(&color, &NColor, sizeof (ITUColor));
					use_normal_color(widget, &color);
					height = child->rect.height;
					//fy = 0 - (height * (wheel->focusIndex + wheel->focus_dev - (wheel->itemCount / 2)));
					fy = 0 - ((wheel->focusIndex + wheel->focus_dev - good_center + 1) * height);
					wheel->layout_ci = fy;
					//printf("===[wheel %s][layout][child %d][fy %d]===\n", widget->name, i, fy);
				}

				ituWidgetSetColor(child, color.alpha, color.red, color.green, color.blue);

				text = (ITUText*)child;

				if (text->fontIndex >= ITU_FREETYPE_MAX_FONTS)
					text->fontIndex = 0;

				if (wheel->fontsquare == 1)
					ituTextSetFontSize(text, wheel->fontHeight);
				else
					ituTextSetFontHeight(text, wheel->fontHeight);

				ituWidgetSetY(child, fy);
				//printf("===[wheel][layout][child %d][fy %d]===\n", i, fy);

				//child->rect.height = height = height0;

				if (i == (wheel->focusIndex + wheel->focus_dev))
				{
					ituWidgetSetColor(child, wheel->focusColor.alpha, wheel->focusColor.red, wheel->focusColor.green, wheel->focusColor.blue);

					if (wheel->fontsquare == 1)
						ituTextSetFontSize(text, wheel->focusFontHeight);
					else
						ituTextSetFontHeight(text, wheel->focusFontHeight);
				}
				fy += height;
			}
		}
	}

	//if (wheel->cycle_tor > 0)
	if ((ev == ITU_EVENT_LAYOUT) || (ev == ITU_EVENT_LANGUAGE))
		result |= ituFlowWindowUpdate(widget, ev, arg1, arg2, arg3);

    if (widget->flags & ITU_TOUCHABLE)
    {
        if (ev == ITU_EVENT_TOUCHSLIDEUP || ev == ITU_EVENT_TOUCHSLIDEDOWN)
        {
            wheel->touchCount = 0;

			if (ituWidgetIsEnabled(widget) && !wheel->inc && !result)
            {
                int x = arg2 - widget->rect.x;
                int y = arg3 - widget->rect.y;

				if ((ituWidgetIsInside(widget, x, y)) || (wheel->inside > 0))
                {
					int count = itcTreeGetChildCount(wheel);

					wheel->inside = 0;

					//wheel->moving_step = 1;
					
                    if (widget->flags & ITU_DRAGGING)
                    {
                        widget->flags &= ~ITU_DRAGGING;
                        ituScene->dragged = NULL;
                        wheel->inc = 0;
                    }
					
                    if (ev == ITU_EVENT_TOUCHSLIDEUP)
                    {
						int fmax = get_max_focusindex(widget);

						if ((wheel->focusIndex <= fmax) || (wheel->cycle_tor > 0))
                        {
                            ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(wheel, 0);

							if (wheel->sliding)
							{
								wheel->frame = wheel->totalframe + 1;
								//wheel->inc = 0;
								//wheel->sliding = 0;
								ituWidgetUpdate(wheel, ITU_EVENT_TIMER, 0, 0, 0);
							}

							if (wheel->inc == 0)
							{
								wheel->inc = 0 - child->rect.height;
								wheel->frame = 0;
								wheel->sliding = 1;

								if ((arg1 >= MOTION_THRESHOLD) && (widget->flags & ITU_DRAGGABLE))
								{
									wheel->slide_step = arg1;
									if (arg1 > (MOTION_THRESHOLD * 3 / 2))
									{
										wheel->totalframe = SLIDE_INIT_FRAME;

										wheel->slide_itemcount = (arg1 / MOTION_THRESHOLD) * STEP_SPEED_BASE;
										//printf("[SLIDE 1st count %d]\n", wheel->slide_itemcount);
									}
									else
										wheel->slide_itemcount = 0;
								}
								else
								{
									if (widget->flags & ITU_DRAGGABLE)
									{
										wheel->slide_step = MOTION_THRESHOLD;
										wheel->slide_itemcount = 0;
									}
									else
										wheel->sliding = 0;
								}
							}
                        }
                    }
                    else if (ev == ITU_EVENT_TOUCHSLIDEDOWN)
                    {
						if ((wheel->focusIndex >= 0) || (wheel->cycle_tor > 0))
                        {
                            ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(wheel, 0);

							if (wheel->sliding)
							{
								wheel->frame = wheel->totalframe + 1;
								//wheel->inc = 0;
								//wheel->sliding = 0;
								ituWidgetUpdate(wheel, ITU_EVENT_TIMER, 0, 0, 0);
							}

							if (wheel->inc == 0)
							{
								wheel->inc = child->rect.height;
								wheel->frame = 0;
								wheel->sliding = 1;

								if ((arg1 >= MOTION_THRESHOLD) && (widget->flags & ITU_DRAGGABLE))
								{
									wheel->slide_step = arg1;
									if (arg1 > (MOTION_THRESHOLD * 3 / 2))
									{
										wheel->totalframe = SLIDE_INIT_FRAME;

										wheel->slide_itemcount = (arg1 / MOTION_THRESHOLD) * STEP_SPEED_BASE;
										//printf("[SLIDE 1st count %d]\n", wheel->slide_itemcount);
									}
									else
										wheel->slide_itemcount = 0;
								}
								else
								{
									if (widget->flags & ITU_DRAGGABLE)
									{
										wheel->slide_step = MOTION_THRESHOLD;
										wheel->slide_itemcount = 0;
									}
									else
										wheel->sliding = 0;
								}
							}
                        }
                    }
					widget->flags &= ~ITU_DRAGGING;
					//widget->flags &= ~ITU_UNDRAGGING;
                    result = true;
                }
				else
				{
					widget->flags &= ~ITU_DRAGGING;
					//widget->flags |= ITU_UNDRAGGING;
					//result = true; //BUG
					//ituWidgetUpdate(wheel, ITU_EVENT_LAYOUT, 0, 0, 0);
				}
            }
        }
        else if (ev == ITU_EVENT_MOUSEMOVE) 
        {
            if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGING))
            {
                int x = arg2 - widget->rect.x;
                int y = arg3 - widget->rect.y;

				wheel->idle = 1;

				if (wheel->cycle_tor <= 0)
				{
					if (ituWidgetIsInside(widget, x, y))
					{
						int i, fy, fc, fmax, height, shift, offset, count = itcTreeGetChildCount(wheel);
						ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(wheel, 0);
						ITUColor color;
						ITUText* text;

						//memcpy(&color, &NColor, sizeof (ITUColor));
						use_normal_color(widget, &color);

						height = child->rect.height;
						fc = wheel->focusIndex + wheel->focus_dev;
						fmax = get_max_focusindex(widget);
						offset = y - wheel->touchY;

						if (wheel->scal > 0)
							offset /= wheel->scal;

						if (((offset > 0) && (wheel->focusIndex > 0)) || ((offset < 0) && (wheel->focusIndex < fmax)))
						{
							for (i = 0; i < count; ++i)
							{
								child = (ITUWidget*)itcTreeGetChildAt(wheel, i);
								//fy = 0 - child->rect.height * ((wheel->focusIndex + wheel->focus_dev - (wheel->itemCount / 2)) + (wheel->fix_count * ((offset >= 0) ? (1) : (-1))));
								fy = 0 - ((wheel->focusIndex + wheel->focus_dev - good_center + 1) * height);
								fy += i * child->rect.height;
								fy += offset;
								fy += (wheel->fix_count * child->rect.height) * ((offset > 0) ? (-1) : (1));
								ituWidgetSetY(child, fy);
							}
						}

						if ((offset >= height) || (offset <= (0 - height)))
						{
							if (offset > 0)
							{
								shift = offset / height;

								if ((wheel->fix_count < shift) && (wheel->focusIndex > 0))
								{
									child = (ITUWidget*)itcTreeGetChildAt(wheel, fc);
									text = (ITUText*)child;
									ituWidgetSetColor(child, color.alpha, color.red, color.green, color.blue);
									if (wheel->fontsquare == 1)
										ituTextSetFontSize(text, wheel->fontHeight);
									else
										ituTextSetFontHeight(text, wheel->fontHeight);
									fc--;
									wheel->focusIndex--;
									child = (ITUWidget*)itcTreeGetChildAt(wheel, fc);
									text = (ITUText*)child;
									ituWidgetSetColor(child, wheel->focusColor.alpha, wheel->focusColor.red, wheel->focusColor.green, wheel->focusColor.blue);
									if (wheel->fontsquare == 1)
										ituTextSetFontSize(text, wheel->focusFontHeight);
									else
										ituTextSetFontHeight(text, wheel->focusFontHeight);

									wheel->fix_count++;
									ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
									//printf("===[wheel][A][wheel->fix_count %d][shift %d]===\n", wheel->fix_count, shift);
								}
								else if ((wheel->fix_count > shift) && (wheel->focusIndex < fmax))
								{
									child = (ITUWidget*)itcTreeGetChildAt(wheel, fc);
									text = (ITUText*)child;
									ituWidgetSetColor(child, color.alpha, color.red, color.green, color.blue);
									if (wheel->fontsquare == 1)
										ituTextSetFontSize(text, wheel->fontHeight);
									else
										ituTextSetFontHeight(text, wheel->fontHeight);
									fc++;
									wheel->focusIndex++;
									child = (ITUWidget*)itcTreeGetChildAt(wheel, fc);
									text = (ITUText*)child;
									ituWidgetSetColor(child, wheel->focusColor.alpha, wheel->focusColor.red, wheel->focusColor.green, wheel->focusColor.blue);
									if (wheel->fontsquare == 1)
										ituTextSetFontSize(text, wheel->focusFontHeight);
									else
										ituTextSetFontHeight(text, wheel->focusFontHeight);

									wheel->fix_count--;
									ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
									//printf("===[wheel][B][wheel->fix_count %d][shift %d]===\n", wheel->fix_count, shift);
								}
							}
							else
							{
								shift = (-1) * offset / height;

								if ((wheel->fix_count < shift) && (wheel->focusIndex < fmax))
								{
									child = (ITUWidget*)itcTreeGetChildAt(wheel, fc);
									text = (ITUText*)child;
									ituWidgetSetColor(child, color.alpha, color.red, color.green, color.blue);
									if (wheel->fontsquare == 1)
										ituTextSetFontSize(text, wheel->fontHeight);
									else
										ituTextSetFontHeight(text, wheel->fontHeight);
									fc++;
									wheel->focusIndex++;
									child = (ITUWidget*)itcTreeGetChildAt(wheel, fc);
									text = (ITUText*)child;
									ituWidgetSetColor(child, wheel->focusColor.alpha, wheel->focusColor.red, wheel->focusColor.green, wheel->focusColor.blue);
									if (wheel->fontsquare == 1)
										ituTextSetFontSize(text, wheel->focusFontHeight);
									else
										ituTextSetFontHeight(text, wheel->focusFontHeight);

									wheel->fix_count++;
									ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
									//printf("===[wheel][C][wheel->fix_count %d][shift %d]===\n", wheel->fix_count, shift);
								}
								else if ((wheel->fix_count > shift) && (wheel->focusIndex > 0))
								{
									child = (ITUWidget*)itcTreeGetChildAt(wheel, fc);
									text = (ITUText*)child;
									ituWidgetSetColor(child, color.alpha, color.red, color.green, color.blue);
									if (wheel->fontsquare == 1)
										ituTextSetFontSize(text, wheel->fontHeight);
									else
										ituTextSetFontHeight(text, wheel->fontHeight);
									fc--;
									wheel->focusIndex--;
									child = (ITUWidget*)itcTreeGetChildAt(wheel, fc);
									text = (ITUText*)child;
									ituWidgetSetColor(child, wheel->focusColor.alpha, wheel->focusColor.red, wheel->focusColor.green, wheel->focusColor.blue);
									if (wheel->fontsquare == 1)
										ituTextSetFontSize(text, wheel->focusFontHeight);
									else
										ituTextSetFontHeight(text, wheel->focusFontHeight);

									wheel->fix_count--;
									ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
									//printf("===[wheel][D][wheel->fix_count %d][shift %d]===\n", wheel->fix_count, shift);
								}
							}
						}
						else
						{
							if (offset > 0)
							{
								if ((wheel->fix_count) && (wheel->focusIndex < fmax))
								{
									child = (ITUWidget*)itcTreeGetChildAt(wheel, fc);
									text = (ITUText*)child;
									ituWidgetSetColor(child, color.alpha, color.red, color.green, color.blue);
									if (wheel->fontsquare == 1)
										ituTextSetFontSize(text, wheel->fontHeight);
									else
										ituTextSetFontHeight(text, wheel->fontHeight);
									fc++;
									wheel->focusIndex++;
									child = (ITUWidget*)itcTreeGetChildAt(wheel, fc);
									text = (ITUText*)child;
									ituWidgetSetColor(child, wheel->focusColor.alpha, wheel->focusColor.red, wheel->focusColor.green, wheel->focusColor.blue);
									if (wheel->fontsquare == 1)
										ituTextSetFontSize(text, wheel->focusFontHeight);
									else
										ituTextSetFontHeight(text, wheel->focusFontHeight);

									wheel->fix_count--;
									ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
									//printf("===[wheel][E][wheel->fix_count %d]===\n", wheel->fix_count);
								}
							}
							else
							{
								if ((wheel->fix_count) && (wheel->focusIndex > 0))
								{
									child = (ITUWidget*)itcTreeGetChildAt(wheel, fc);
									text = (ITUText*)child;
									ituWidgetSetColor(child, color.alpha, color.red, color.green, color.blue);
									if (wheel->fontsquare == 1)
										ituTextSetFontSize(text, wheel->fontHeight);
									else
										ituTextSetFontHeight(text, wheel->fontHeight);
									fc--;
									wheel->focusIndex--;
									child = (ITUWidget*)itcTreeGetChildAt(wheel, fc);
									text = (ITUText*)child;
									ituWidgetSetColor(child, wheel->focusColor.alpha, wheel->focusColor.red, wheel->focusColor.green, wheel->focusColor.blue);
									if (wheel->fontsquare == 1)
										ituTextSetFontSize(text, wheel->focusFontHeight);
									else
										ituTextSetFontHeight(text, wheel->focusFontHeight);

									wheel->fix_count--;
									ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
									//printf("===[wheel][F][wheel->fix_count %d]===\n", wheel->fix_count);
								}
							}
						}

						result = true;
					}
				}
				else //for cycle mode
				{
					if (ituWidgetIsInside(widget, x, y))
					{
						int i, fy, fm, offset, shift, count = itcTreeGetChildCount(wheel);
						ITUWidget* child;

						offset = y - wheel->touchY;
						child = (ITUWidget*)itcTreeGetChildAt(wheel, 0);

						if (wheel->scal > 0)
							offset /= wheel->scal;

						for (i = 0; i < wheel->cycle_arr_count; i++)
						{
							child = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->cycle_arr[i]);
							
							if (i == 0)
								fy = 0 - child->rect.height * (wheel->itemCount / 2 + wheel->cycle_tor) + (wheel->itemCount / 4 * child->rect.height);

							ituWidgetSetY(child, fy + offset + ((offset>0)?(-1):(1)) * ((wheel->fix_count) ? (child->rect.height * wheel->fix_count) : (0)));
							fy += child->rect.height;
						}

						if ((offset >= child->rect.height) || (offset <= (0 - child->rect.height)))
						{
							if (offset > 0)
							{
								shift = offset / child->rect.height;

								if (wheel->fix_count < shift)
								{
									for (i = 0; i < wheel->cycle_arr_count; i++)
									{
										wheel->cycle_arr[i]--;

										if (wheel->cycle_arr[i] < wheel->minci)
											wheel->cycle_arr[i] = wheel->maxci;

										child = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->cycle_arr[i]);

										if (i == 0)
											fy = 0 - child->rect.height * (wheel->itemCount / 2 + wheel->cycle_tor) + (wheel->itemCount / 4 * child->rect.height);

										fy += child->rect.height;
									}
									wheel->fix_count++;
									fm = wheel->focus_c - 1;

									if (fm < wheel->minci)
										fm = wheel->maxci;

									focus_change(widget, fm, __LINE__);
									ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
									//printf("===[wheel][A][wheel->fix_count %d][shift %d]===\n", wheel->fix_count, shift);
								}
								else if (wheel->fix_count > shift)
								{
									child = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->cycle_arr[0]);
									ituWidgetSetY(child, wheel->tempy);

									for (i = 0; i < wheel->cycle_arr_count; i++)
									{
										wheel->cycle_arr[i]++;

										if (wheel->cycle_arr[i] > wheel->maxci)
											wheel->cycle_arr[i] = wheel->minci;

										child = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->cycle_arr[i]);

										if (i == 0)
											fy = 0 - child->rect.height * (wheel->itemCount / 2 + wheel->cycle_tor) + (wheel->itemCount / 4 * child->rect.height);

										fy += child->rect.height;
									}
									wheel->fix_count--;
									fm = wheel->focus_c + 1;

									if (fm > wheel->maxci)
										fm = wheel->minci;

									focus_change(widget, fm, __LINE__);
									ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
									//printf("===[wheel][B][wheel->fix_count %d][shift %d]===\n", wheel->fix_count, shift);
								}
							}
							else
							{
								shift = (-1) * offset / child->rect.height;

								if (wheel->fix_count < shift)
								{
									for (i = 0; i < wheel->cycle_arr_count; i++)
									{
										wheel->cycle_arr[i]++;

										if (wheel->cycle_arr[i] > wheel->maxci)
											wheel->cycle_arr[i] = wheel->minci;

										child = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->cycle_arr[i]);

										if (i == 0)
											fy = 0 - child->rect.height * (wheel->itemCount / 2 + wheel->cycle_tor) + (wheel->itemCount / 4 * child->rect.height);

										fy += child->rect.height;
									}
									wheel->fix_count++;
									fm = wheel->focus_c + 1;

									if (fm > wheel->maxci)
										fm = wheel->minci;

									focus_change(widget, fm, __LINE__);
									ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
									ituWidgetUpdate(widget, ITU_EVENT_CHANGED, 0, 0, 0);
									//printf("===[wheel][C][wheel->fix_count %d][shift %d]===\n", wheel->fix_count, shift);
								}
								else if (wheel->fix_count > shift)
								{
									child = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->cycle_arr[wheel->cycle_arr_count - 1]);
									ituWidgetSetY(child, wheel->tempy);

									for (i = 0; i < wheel->cycle_arr_count; i++)
									{
										wheel->cycle_arr[i]--;

										if (wheel->cycle_arr[i] < wheel->minci)
											wheel->cycle_arr[i] = wheel->maxci;

										child = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->cycle_arr[i]);

										if (i == 0)
											fy = 0 - child->rect.height * (wheel->itemCount / 2 + wheel->cycle_tor) + (wheel->itemCount / 4 * child->rect.height);

										fy += child->rect.height;
									}
									wheel->fix_count--;
									fm = wheel->focus_c - 1;

									if (fm < wheel->minci)
										fm = wheel->maxci;

									focus_change(widget, fm, __LINE__);
									ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
									//printf("===[wheel][D][wheel->fix_count %d][shift %d]===\n", wheel->fix_count, shift);
								}
							}
						}
						else
						{
							if (offset > 0)
							{
								if (wheel->fix_count)
								{
									child = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->cycle_arr[0]);
									ituWidgetSetY(child, wheel->tempy);

									for (i = 0; i < wheel->cycle_arr_count; i++)
									{
										wheel->cycle_arr[i]++;

										if (wheel->cycle_arr[i] > wheel->maxci)
											wheel->cycle_arr[i] = wheel->minci;

										child = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->cycle_arr[i]);

										if (i == 0)
											fy = 0 - child->rect.height * (wheel->itemCount / 2 + wheel->cycle_tor) + (wheel->itemCount / 4 * child->rect.height);

										fy += child->rect.height;
									}
									wheel->fix_count--;
									fm = wheel->focus_c + 1;

									if (fm > wheel->maxci)
										fm = wheel->minci;

									focus_change(widget, fm, __LINE__);
									ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
									//printf("===[wheel][E][wheel->fix_count %d]===\n", wheel->fix_count);
								}
							}
							else
							{
								if (wheel->fix_count)
								{
									child = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->cycle_arr[wheel->cycle_arr_count - 1]);
									ituWidgetSetY(child, wheel->tempy);

									for (i = 0; i < wheel->cycle_arr_count; i++)
									{
										wheel->cycle_arr[i]--;

										if (wheel->cycle_arr[i] < wheel->minci)
											wheel->cycle_arr[i] = wheel->maxci;

										child = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->cycle_arr[i]);

										if (i == 0)
											fy = 0 - child->rect.height * (wheel->itemCount / 2 + wheel->cycle_tor) + (wheel->itemCount / 4 * child->rect.height);

										fy += child->rect.height;
									}
									wheel->fix_count--;
									fm = wheel->focus_c - 1;

									if (fm < wheel->minci)
										fm = wheel->maxci;

									focus_change(widget, fm, __LINE__);
									ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
									//printf("===[wheel][F][wheel->fix_count %d]===\n", wheel->fix_count);
								}
							}
						}

						child = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->focusIndex);
						ituWidgetSetY(child, child->rect.y - ((wheel->focusFontHeight - wheel->fontHeight) / FOCUSFONT_POS_Y_FIX_DIV));
						result = true;
					}
				}
            }
        }
        else if (ev == ITU_EVENT_MOUSEDOWN)
        {
			int x = arg2 - widget->rect.x;
			int y = arg3 - widget->rect.y;
			//printf("[x,y,touchy] [%d,%d,%d]\n",x ,y , wheel->touchY);
            if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGABLE) && !result)
            {
                if (ituWidgetIsInside(widget, x, y))
                {
					wheel->inside = 1;
					wheel->fix_count = 0;
                    wheel->touchY = y;
                    widget->flags |= ITU_DRAGGING;
                    ituScene->dragged = widget;
                    result = true;
                }
            }

			if (ituWidgetIsEnabled(widget) && ituWidgetIsInside(widget, x, y))
			{
				get_normal_color(widget);

				//force to reset frame and total frame to default
				wheel->totalframe = (int)ituWidgetGetCustomData(wheel);

				wheel->frame = wheel->totalframe;
				wheel->inc = 0;

				//if (wheel->sliding == 1)
				//{
				//	wheel->idle = 1;
				//}
				//wheel->idle = 1;
			}
			else
			{
				// Force stop wheel sliding
				if (wheel->sliding && wheel->frame)
				{
					wheel->frame = 0;
					wheel->inc = 0;
					wheel->sliding = 0;
					ituWidgetUpdate(wheel, ITU_EVENT_LAYOUT, 0, 0, 0);
					result = true;
					return result;
				}
			}
        }
		else if (ev == ITU_EVENT_MOUSEUP)
		{
			if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGABLE) && (widget->flags & ITU_DRAGGING))
			{
				int count = itcTreeGetChildCount(wheel);
				int fmax = get_max_focusindex(widget);
				int y = arg3 - widget->rect.y;
				bool soft_click = false;
				ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(wheel, 0);
				wheel->sliding = 0;
				wheel->inside = 0;
				if ((count > 0) && (wheel->cycle_tor <= 0))
				{
					if ((wheel->inc == 0) && (wheel->fix_count == 0))
					{
						int offset, absoffset, interval;
						int div_value;
						offset = y - wheel->touchY;
						interval = offset / child->rect.height;
						offset -= (interval * child->rect.height);
						absoffset = offset > 0 ? offset : -offset;

						if (absoffset > child->rect.height / MOUSEUP_CHANGE_FACTOR)
						{
							div_value = child->rect.height / wheel->totalframe;
							div_value = (div_value == 0) ? (1) : (div_value);
							wheel->frame = absoffset / div_value + 1;

							if (offset > 0)
							{
								wheel->inc = child->rect.height;
								wheel->focusIndex -= interval;
								//printf("===[wheel][1][frame %d][offset %d][inc %d][interval %d][focusIndex %d]===\n", wheel->frame, offset, wheel->inc, interval, wheel->focusIndex);
							}
							else if (offset < 0)
							{
								wheel->inc = -child->rect.height;
								wheel->focusIndex -= interval;
								//printf("===[wheel][2][frame %d][offset %d][inc %d][interval %d][focusIndex %d]===\n", wheel->frame, offset, wheel->inc, interval, wheel->focusIndex);
							}
						}
						else
						{
							div_value = child->rect.height / wheel->totalframe;
							div_value = (div_value == 0) ? (1) : (div_value);
							wheel->frame = wheel->totalframe - absoffset / div_value;

							if (offset > 0)
							{
								wheel->inc = -child->rect.height;
								wheel->focusIndex -= interval;// +1;

								wheel->frame = wheel->totalframe;

								if (interval != 0)
									wheel->inc = 0;
								//printf("===[wheel][3][frame %d][offset %d][inc %d][interval %d][focusIndex %d]===\n", wheel->frame, offset, wheel->inc, interval, wheel->focusIndex);
							}
							else if (offset < 0)
							{
								wheel->inc = child->rect.height;
								wheel->focusIndex -= interval;// -1;

								wheel->frame = wheel->totalframe;

								if (interval != 0)
									wheel->inc = 0;
								//printf("===[wheel][4][frame %d][offset %d][inc %d][interval %d][focusIndex %d]===\n", wheel->frame, offset, wheel->inc, interval, wheel->focusIndex);
							}

							soft_click = true;
						}

						if (wheel->inc > 0)
						{
							if (wheel->focusIndex <= 0)
							{
								wheel->focusIndex = 0;
								wheel->frame = wheel->totalframe;
								wheel->inc = 0;
								//printf("===[wheel][5]\n");
							}
							//else
							//{
							//	if (wheel->focusIndex >= fmax)
							//	{
							//		wheel->focusIndex = fmax;
							//		wheel->frame = wheel->totalframe;
							//		wheel->inc = 0;
							//		//printf("===[wheel][6]\n");
							//	}
							//}
						}
						else // if (wheel->inc < 0)
						{
							//if (wheel->focusIndex <= 0)
							//{
							//	wheel->focusIndex = 0;
							//	wheel->frame = wheel->totalframe;
							//	wheel->inc = 0;
							//	//printf("===[wheel][7]\n");
							//}
							//else
							//{
								if (wheel->focusIndex >= fmax)
								{
									wheel->focusIndex = fmax;
									wheel->frame = wheel->totalframe;
									wheel->inc = 0;
									//printf("===[wheel][8]\n");
								}
							//}

							/*
							if (wheel->idle == 0)
							{
							if ((absoffset <= 1) && (ituWidgetIsVisible(widget)))
							{
							//ituExecActions(widget, wheel->actions, ITU_EVENT_CUSTOM, 0);
							wheel->idle = 10;
							}
							}
							else
							{
							wheel->idle = 0;
							}*/
						}
						widget->flags |= ITU_UNDRAGGING;
					}
					else if (wheel->fix_count)
					{
						wheel->frame = wheel->totalframe;
						wheel->inc = 0;
						wheel->fix_count = 0;
						widget->flags |= ITU_UNDRAGGING;
					}
					else
					{
						widget->flags |= ITU_UNDRAGGING;
						wheel->frame = wheel->totalframe;
						widget->flags &= ~ITU_DRAGGING;
					}

					if (soft_click)
					{
						int i = 0;
						int click_shift = 0;
						int goal = wheel->focusIndex;
						int c_top;
						int c_bot;

						c_top = (widget->rect.height - child->rect.height) / 2;
						c_bot = c_top + child->rect.height;

						if (wheel->touchY < (c_top - 2))
						{
							click_shift = ((c_top - wheel->touchY) / child->rect.height) + 1;

							for (i = 0; i < click_shift; i++)
							{
								if (goal > 0)
									goal--;
								else
								{
									if (wheel->cycle_tor)
										goal = get_max_focusindex(widget);
								}
							}

							ituWheelGoto(wheel, goal);
						}
						else if (wheel->touchY > (c_bot + 2))
						{
							click_shift = ((wheel->touchY - c_bot) / child->rect.height) + 1;

							for (i = 0; i < click_shift; i++)
							{
								if (goal < get_max_focusindex(widget))
									goal++;
								else
								{
									if (wheel->cycle_tor)
										goal = 0;
								}
							}

							ituWheelGoto(wheel, goal);
						}

						//printf("[soft click]\n");
					}
					else
					{
						wheel->idle = 0;
						ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
					}
				}
				else if ((count > 0) && wheel->cycle_tor) //for cycle mode
				{
					int i = 0;
					int newfocus = 0;
					int div_value;

					if (wheel->inc == 0)
					{
						int offset, absoffset, interval;

						offset = y - wheel->touchY;
						interval = offset / child->rect.height;
						absoffset = offset > 0 ? offset : -offset;

						if (absoffset > child->rect.height / MOUSEUP_CHANGE_FACTOR)
						{
							div_value = child->rect.height / wheel->totalframe;
							div_value = (div_value == 0) ? (1) : (div_value);
							wheel->frame = absoffset / div_value + 1;

							if (offset >= 0)
							{
								wheel->inc = child->rect.height;
								newfocus = wheel->focus_c;

								for (i = 1; i <= interval; i++)
								{
									newfocus--;

									if (newfocus < wheel->minci)
										newfocus = wheel->maxci;

									if ((interval != 0) && (wheel->fix_count == 0))
									{
										if (focus_change(widget, newfocus, __LINE__))
											cycle_arrange(widget, true);
									}
								}

								if (interval != 0)
								{
									if (wheel->fix_count)
										wheel->inc = 0;
								}
								else
								{
									wheel->inc = 0;
									wheel->frame = wheel->totalframe;
								}
								//printf("===[wheel][1][frame %d][offset %d][inc %d][interval %d][focusIndex %d]===\n", wheel->frame, offset, wheel->inc, interval, wheel->focusIndex);
							}
							else
							{
								wheel->inc = -child->rect.height;
								newfocus = wheel->focus_c;

								for (i = interval; i < 0; i++)
								{
									newfocus++;

									if (newfocus > wheel->maxci)
										newfocus = wheel->minci;

									if ((interval != 0) && (wheel->fix_count == 0))
									{
										if (focus_change(widget, newfocus, __LINE__))
											cycle_arrange(widget, false);
									}
								}

								if (interval != 0)
								{
									if (wheel->fix_count)
										wheel->inc = 0;
								}
								else
								{
									wheel->inc = 0;
									wheel->frame = wheel->totalframe;
								}
								//printf("===[wheel][2][frame %d][offset %d][inc %d][interval %d][focusIndex %d]===\n", wheel->frame, offset, wheel->inc, interval, wheel->focusIndex);
							}
						}
						else
						{
							div_value = child->rect.height / wheel->totalframe;
							div_value = (div_value == 0) ? (1) : (div_value);
							wheel->frame = wheel->totalframe - absoffset / div_value;

							if (offset >= 0)
							{
								wheel->inc = -child->rect.height;
								newfocus = wheel->focus_c;

								for (i = interval; i < 0; i++)
								{
									newfocus++;

									if (newfocus > wheel->maxci)
										newfocus = wheel->minci;

									if ((interval != 0) && (wheel->fix_count == 0))
									{
										if (focus_change(widget, newfocus, __LINE__))
											cycle_arrange(widget, false);
									}
								}

								if (interval != 0)
								{
									if (wheel->fix_count)
										wheel->inc = 0;
								}
								else
								{
									wheel->inc = 0;
									wheel->frame = wheel->totalframe;
								}
								//printf("===[wheel][3][frame %d][offset %d][inc %d][interval %d][focusIndex %d]===\n", wheel->frame, offset, wheel->inc, interval, wheel->focusIndex);
							}
							else
							{
								wheel->inc = child->rect.height;
								newfocus = wheel->focus_c;

								for (i = 1; i <= interval; i++)
								{
									newfocus--;

									if (newfocus < wheel->minci)
										newfocus = wheel->maxci;

									if ((interval != 0) && (wheel->fix_count == 0))
									{
										if (focus_change(widget, newfocus, __LINE__))
											cycle_arrange(widget, true);
									}
								}

								if (interval != 0)
								{
									if (wheel->fix_count)
										wheel->inc = 0;
								}
								else
								{
									wheel->inc = 0;
									wheel->frame = wheel->totalframe;
								}
								//printf("===[wheel][4][frame %d][offset %d][inc %d][interval %d][focusIndex %d]===\n", wheel->frame, offset, wheel->inc, interval, wheel->focusIndex);
							}

							soft_click = true;
						}

						if (wheel->inc > 0)
						{
							if (wheel->focusIndex < 0)
							{
								wheel->focus_c = wheel->maxci;
								wheel->focusIndex = wheel->focus_c - wheel->focus_dev;
								wheel->frame = wheel->totalframe;
								wheel->inc = 0;
							}
							else
							{
								if (wheel->focusIndex >(wheel->maxci - wheel->focus_dev))
								{
									wheel->focus_c = wheel->minci;
									wheel->focusIndex = wheel->focus_c - wheel->focus_dev;
									wheel->frame = wheel->totalframe;
									wheel->inc = 0;
								}
							}
						}
						else // if (wheel->inc <= 0)
						{
							if (wheel->focusIndex < 0)
							{
								wheel->focus_c = wheel->maxci;
								wheel->focusIndex = wheel->focus_c - wheel->focus_dev;
								wheel->frame = wheel->totalframe;
								wheel->inc = 0;
							}
							else
							{
								if (wheel->focusIndex >(wheel->maxci - wheel->focus_dev))
								{
									wheel->focus_c = wheel->minci;
									wheel->focusIndex = wheel->focus_c - wheel->focus_dev;
									wheel->frame = wheel->totalframe;
									wheel->inc = 0;
								}
							}
						}
						widget->flags |= ITU_UNDRAGGING;

						if (soft_click)
						{
							int i = 0;
							int click_shift = 0;
							int goal = wheel->focusIndex;
							int c_top;
							int c_bot;

							c_top = (widget->rect.height - child->rect.height) / 2;
							c_bot = c_top + child->rect.height;

							if (wheel->touchY < (c_top - 2))
							{
								click_shift = ((c_top - wheel->touchY) / child->rect.height) + 1;

								for (i = 0; i < click_shift; i++)
								{
									if (goal > 0)
										goal--;
									else
									{
										if (wheel->cycle_tor)
											goal = get_max_focusindex(widget);
									}
								}

								ituWheelGoto(wheel, goal);
							}
							else if (wheel->touchY > (c_bot + 2))
							{
								click_shift = ((wheel->touchY - c_bot) / child->rect.height) + 1;

								for (i = 0; i < click_shift; i++)
								{
									if (goal < get_max_focusindex(widget))
										goal++;
									else
									{
										if (wheel->cycle_tor)
											goal = 0;
									}
								}

								ituWheelGoto(wheel, goal);
							}

							//printf("[soft click]\n");
						}
						else
						{
							wheel->idle = 0;
							ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
						}
					}
					else
					{
						wheel->frame = wheel->totalframe;
					}
				}
				result = true;
			}
			else
			{
				// Force stop wheel sliding
				if (wheel->sliding && wheel->frame)
				{
					wheel->frame = 0;
					wheel->inc = 0;
					wheel->sliding = 0;
					ituWidgetUpdate(wheel, ITU_EVENT_LAYOUT, 0, 0, 0);
					result = true;
				}
			}

			widget->flags &= ~ITU_DRAGGING;
			wheel->touchCount = 0;
		}
    }
    
    if (ev == ITU_EVENT_TIMER)
    {
		if ((wheel->sliding) || (wheel->frame == wheel->totalframe))
			result = true;

		//fix for some case make slide_step too small and wheel will not stop
		if (wheel->sliding && (wheel->slide_step < MOTION_THRESHOLD))
		{
			wheel->sliding = 0;
			return result;
		}

		if (wheel->cycle_tor <= 0) //non-cycle
		{
			int fmax = get_max_focusindex(widget);

			if ((widget->flags & ITU_UNDRAGGING) && (wheel->sliding == 0))
			{
				int i, count = itcTreeGetChildCount(wheel);
				ITUColor color;
				ITUText* text;

				for (i = 0; i < count; ++i)
				{
				   int fy;
					ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(wheel, i);
					text = (ITUText*)itcTreeGetChildAt(wheel, i);
					//fy = 0 - (child->rect.height * (wheel->focusIndex + wheel->focus_dev - (wheel->itemCount / 2)));
					fy = 0 - ((wheel->focusIndex + wheel->focus_dev - good_center + 1) * child->rect.height);
					fy += i * child->rect.height;
					fy += wheel->inc * wheel->frame / wheel->totalframe;

					if (i == 0)
					{
						//memcpy(&color, &NColor, sizeof (ITUColor));
						use_normal_color(widget, &color);
					}

					ituWidgetSetColor(text, color.alpha, color.red, color.green, color.blue);
					if (wheel->fontsquare == 1)
						ituTextSetFontSize(text, wheel->fontHeight);
					else
						ituTextSetFontHeight(text, wheel->fontHeight);
					ituWidgetSetY(child, fy);
				}

				//if (wheel->sliding == 0)
				//	wheel->frame = wheel->totalframe;

				wheel->frame++;

				if (wheel->frame > wheel->totalframe)
				{
					bool fi_changed = true;

					if ((wheel->inc > 0) && (wheel->focusIndex > 0))
						wheel->focusIndex--;
					else if ((wheel->inc < 0) && (wheel->focusIndex < fmax))
						wheel->focusIndex++;
					else
						fi_changed = false;

					wheel->frame = 0;

					if (wheel->sliding == 0)
					{
						wheel->inc = 0;
						wheel->idle = 0;
					}
					else
					{
						wheel->sliding = 0;
					}

					widget->flags &= ~ITU_UNDRAGGING;

					for (i = 0; i < count; ++i)
					{
						ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(wheel, i);
						//int fy = 0 - (child->rect.height * (wheel->focusIndex + wheel->focus_dev - (wheel->itemCount / 2)));
						int fy = 0 - ((wheel->focusIndex + wheel->focus_dev - good_center + 1) * child->rect.height);
						fy += i * child->rect.height;

						ituWidgetSetY(child, fy);
					}

					text = (ITUText*)itcTreeGetChildAt(wheel, wheel->focusIndex + wheel->focus_dev);
					ituWidgetSetColor(text, wheel->focusColor.alpha, wheel->focusColor.red, wheel->focusColor.green, wheel->focusColor.blue);
					if (wheel->fontsquare == 1)
						ituTextSetFontSize(text, wheel->focusFontHeight);
					else
						ituTextSetFontHeight(text, wheel->focusFontHeight);
					//widget->flags |= ITU_UNDRAGGING;
					//widget->flags &= ~ITU_DRAGGING;

					if (fi_changed)
						ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
					//value = ituTextGetString(text);
					//ituWheelOnValueChanged(wheel, value);
				}
			} //non-cycle mode sliding
			else if ((wheel->inc) && (wheel->sliding) && (wheel->focusIndex != 0) && (wheel->focusIndex != fmax))
			{
				int i, fac, fy, height, height0, count = itcTreeGetChildCount(wheel);
				ITUColor color;
				ITUText* text;

				for (i = 0; i < count; ++i)
				{
					ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(wheel, i);

					if (i == 0)
					{
						height0 = height = child->rect.height;
						//fy = 0 - (child->rect.height * (wheel->focusIndex + wheel->focus_dev - (wheel->itemCount / 2)));
						fy = 0 - ((wheel->focusIndex + wheel->focus_dev - good_center + 1) * child->rect.height);
						use_normal_color(widget, &color);
					}

					if (i == wheel->focusIndex + wheel->focus_dev)
					{
						ituWidgetSetColor(child, wheel->focusColor.alpha, wheel->focusColor.red, wheel->focusColor.green, wheel->focusColor.blue);
					}
					else
					{
						ituWidgetSetColor(child, color.alpha, color.red, color.green, color.blue);
					}
					ituWidgetSetY(child, fy + wheel->inc * wheel->frame / wheel->totalframe);
					fac = fy + (wheel->inc * wheel->frame / wheel->totalframe);
					ituWidgetSetY(child, fac);

					child->rect.height = height = height0;
					fy += height;
				}

				//wheel->moving_step += 1;

				//if (wheel->moving_step > xx)
				//{
				if (widget->flags & ITU_DRAGGABLE)
				{
					if ((wheel->inc > 0) && (wheel->focusIndex == 1))
						wheel->frame = wheel->totalframe;
					else if ((wheel->inc < 0) && (wheel->focusIndex == (fmax - 1)))
						wheel->frame = wheel->totalframe;
				}

				//wheel->moving_step = 1;

				if (wheel->shift_one) // && (widget->flags & ITU_DRAGGABLE))
				{
					wheel->frame = wheel->totalframe;
					wheel->shift_one = 0;
					wheel->sliding = 0;
				}
				wheel->frame++;

				if (wheel->frame > wheel->totalframe)
				{
					if ((wheel->inc > 0) && (wheel->focusIndex > 0))
					{
						if ((!((wheel->totalframe < MOTION_FACTOR) && (wheel->sliding > 0))) || (wheel->focusIndex == 1))
							wheel->idle = 0;
						wheel->focusIndex--;
						ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
					}
					else if ((wheel->inc < 0) && (wheel->focusIndex < fmax))
					{
						if ((!((wheel->totalframe < MOTION_FACTOR) && (wheel->sliding > 0))) || (wheel->focusIndex == (fmax - 1)))
							wheel->idle = 0;
						wheel->focusIndex++;
						ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
					}


					wheel->frame = 0;

					////////////////////////////////
					//Force feedback algorithm
					if ((wheel->totalframe < MOTION_FACTOR) && (wheel->sliding > 0))
					{
						int y1;
						float progress_range;
						double pow_b;
						double fix_mod = ((double)wheel->slide_step / MOTION_THRESHOLD);

						if (fix_mod > MAX_INIT_POWER)
							fix_mod = MAX_INIT_POWER;

						pow_b = 5.0 * MAX_INIT_POWER / fix_mod;

						y1 = (int)(pow(pow_b, (double)wheel->totalframe / (MOTION_FACTOR / (5.0 - fix_mod))));

						if (y1 > MOTION_FACTOR)
							y1 = MOTION_FACTOR;

						printf("[ydiff %d] [pow_b %.3f] [totalframe %d]\n", wheel->slide_step, pow_b, wheel->totalframe);

						progress_range = (float)wheel->totalframe / MOTION_FACTOR;

						if (wheel->slide_itemcount)
						{
							wheel->slide_itemcount--;
						}
						else
						{
							if (progress_range <= PROCESS_STAGE1)
								wheel->totalframe += y1;
							else if (progress_range <= PROCESS_STAGE2)
								wheel->totalframe += y1 * 3;
							else
								wheel->totalframe += y1 * (int)fix_mod;
						}

						if (wheel->totalframe > 40)
							wheel->totalframe = MOTION_FACTOR;
					}
					else
					{
						wheel->inc = 0;
						widget->flags &= ~ITU_UNDRAGGING;

						//wheel->moving_step = 0;

						wheel->totalframe = (int)ituWidgetGetCustomData(wheel);

						if (wheel->sliding == 1)
							wheel->sliding = 0;

						for (i = 0; i < count; ++i)
						{
							ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(wheel, i);

							if (i == 0)
							{
								height0 = height = child->rect.height;
								//fy = 0 - (child->rect.height * (wheel->focusIndex + wheel->focus_dev - (wheel->itemCount / 2)));
								fy = 0 - ((wheel->focusIndex + wheel->focus_dev - good_center + 1) * child->rect.height);
							}

							text = (ITUText*)child;

							ituWidgetSetY(child, fy);

							child->rect.height = height = height0;

							if (i == (wheel->focusIndex + wheel->focus_dev))
							{
								ituWidgetSetColor(text, wheel->focusColor.alpha, wheel->focusColor.red, wheel->focusColor.green, wheel->focusColor.blue);

								if (wheel->fontsquare == 1)
									ituTextSetFontSize(text, wheel->focusFontHeight);
								else
									ituTextSetFontHeight(text, wheel->focusFontHeight);

								//ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
								//value = ituTextGetString(text);
								//ituWheelOnValueChanged(wheel, value);
							}
							else
							{
								ituWidgetSetColor(child, color.alpha, color.red, color.green, color.blue);
								if (wheel->fontsquare == 1)
									ituTextSetFontSize(text, wheel->fontHeight);
								else
									ituTextSetFontHeight(text, wheel->fontHeight);
							}
							fy += height;
						}
						//check here (fix the wrong position when slide stop by itself)
						widget->dirty = true;
						result |= widget->dirty;
						//return widget->visible ? result : false;
					}
				}
			}
			else if (wheel->inc) //non-cycle mode bump boundary
			{
				int i, fy, height, height0, count = itcTreeGetChildCount(wheel);
				ITUColor color;
				ITUText* text;

				for (i = 0; i < count; ++i)
				{
					ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(wheel, i);

					if (i == 0)
					{
						height0 = height = child->rect.height;
						//fy = 0 - (child->rect.height * (wheel->focusIndex + wheel->focus_dev - (wheel->itemCount / 2)));
						fy = 0 - ((wheel->focusIndex + wheel->focus_dev - good_center + 1) * child->rect.height);
						//memcpy(&color, &NColor, sizeof (ITUColor));
						use_normal_color(widget, &color);
					}

					ituWidgetSetColor(child, color.alpha, color.red, color.green, color.blue);
					ituWidgetSetY(child, fy + wheel->inc * wheel->frame / wheel->totalframe);

					child->rect.height = height = height0;

					if ((widget->flags & ITU_DRAGGABLE) == 0)
					{
						ITUText* text = (ITUText*)child;

						if (i == (wheel->focusIndex + wheel->focus_dev))
						{
							int a = color.alpha + (wheel->focusColor.alpha - color.alpha) * (wheel->totalframe - wheel->frame) / wheel->totalframe;
							int r = color.red + (wheel->focusColor.red - color.red) * (wheel->totalframe - wheel->frame) / wheel->totalframe;
							int g = color.green + (wheel->focusColor.green - color.green) * (wheel->totalframe - wheel->frame) / wheel->totalframe;
							int b = color.blue + (wheel->focusColor.blue - color.blue) * (wheel->totalframe - wheel->frame) / wheel->totalframe;
							int h = height + (wheel->focusFontHeight - height) * (wheel->totalframe - wheel->frame) / wheel->totalframe;

							//child->rect.height = height = h;
							ituWidgetSetColor(text, a, r, g, b);
							if (wheel->fontsquare == 1)
								ituTextSetFontSize(text, wheel->fontHeight + (wheel->focusFontHeight - wheel->fontHeight) * (wheel->totalframe - wheel->frame) / wheel->totalframe);
							else
								ituTextSetFontHeight(text, wheel->fontHeight + (wheel->focusFontHeight - wheel->fontHeight) * (wheel->totalframe - wheel->frame) / wheel->totalframe);
						}
						else if (((i == wheel->focusIndex + wheel->focus_dev - 1) && (wheel->inc > 0)) ||
							((i == wheel->focusIndex + wheel->focus_dev + 1) && (wheel->inc < 0)))
						{
							int a = color.alpha + (wheel->focusColor.alpha - color.alpha) * wheel->frame / wheel->totalframe;
							int r = color.red + (wheel->focusColor.red - color.red) * wheel->frame / wheel->totalframe;
							int g = color.green + (wheel->focusColor.green - color.green) * wheel->frame / wheel->totalframe;
							int b = color.blue + (wheel->focusColor.blue - color.blue) * wheel->frame / wheel->totalframe;
							int h = height + (wheel->focusFontHeight - height) * wheel->frame / wheel->totalframe;

							//child->rect.height = height = h;
							ituWidgetSetColor(text, a, r, g, b);
							if (wheel->fontsquare == 1)
								ituTextSetFontSize(text, wheel->fontHeight + (wheel->focusFontHeight - wheel->fontHeight) * wheel->frame / wheel->totalframe);
							else
								ituTextSetFontHeight(text, wheel->fontHeight + (wheel->focusFontHeight - wheel->fontHeight) * wheel->frame / wheel->totalframe);
						}
					}
					fy += height;
				}
				if (widget->flags & ITU_DRAGGABLE)
				{
					if ((wheel->inc > 0) && (wheel->focusIndex == 1))
						wheel->frame = wheel->totalframe;
					else if ((wheel->inc < 0) && (wheel->focusIndex == (fmax - 1)))
						wheel->frame = wheel->totalframe;
				}

				if ((wheel->shift_one) && (widget->flags & ITU_DRAGGABLE))
				{
					wheel->frame = wheel->totalframe;
					wheel->shift_one = 0;
				}

				wheel->frame++;

				if (wheel->frame > wheel->totalframe)
				{
					if ((wheel->inc > 0) && (wheel->focusIndex > 0))
					{
						wheel->idle = 0;
						wheel->focusIndex--;
						ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
					}
					else if ((wheel->inc < 0) && (wheel->focusIndex < fmax))
					{
						wheel->idle = 0;
						wheel->focusIndex++;
						ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
					}

					wheel->frame = 0;
					wheel->inc = 0;
					widget->flags &= ~ITU_UNDRAGGING;

					if (wheel->sliding == 1)
						wheel->sliding = 0;

					for (i = 0; i < count; ++i)
					{
						ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(wheel, i);

						if (i == 0)
						{
							height0 = height = child->rect.height;
							//fy = 0 - (child->rect.height * (wheel->focusIndex + wheel->focus_dev - (wheel->itemCount / 2)));
							fy = 0 - ((wheel->focusIndex + wheel->focus_dev - good_center + 1) * child->rect.height);
						}

						text = (ITUText*)child;

						ituWidgetSetY(child, fy);

						child->rect.height = height = height0;

						if (i == wheel->focusIndex + wheel->focus_dev)
						{
							ituWidgetSetColor(text, wheel->focusColor.alpha, wheel->focusColor.red, wheel->focusColor.green, wheel->focusColor.blue);

							if (wheel->fontsquare == 1)
								ituTextSetFontSize(text, wheel->focusFontHeight);
							else
								ituTextSetFontHeight(text, wheel->focusFontHeight);

							//ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
							//value = ituTextGetString(text);
							//ituWheelOnValueChanged(wheel, value);
						}
						else
						{
							ituWidgetSetColor(child, color.alpha, color.red, color.green, color.blue);
							
							if (wheel->fontsquare == 1)
								ituTextSetFontSize(text, wheel->fontHeight);
							else
								ituTextSetFontHeight(text, wheel->fontHeight);
						}
						fy += height;
					}
				}
				else if (widget->flags & ITU_DRAGGABLE)
				{
					if ((wheel->inc > 0) && (wheel->focusIndex > 0))
					{
						wheel->focusIndex--;
						ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
					}
					else if ((wheel->inc < 0) && (wheel->focusIndex < fmax))
					{
						wheel->focusIndex++;
						ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
					}

					for (i = 0; i < count; ++i)
					{
						ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(wheel, i);

						if (i == 0)
						{
							height0 = height = child->rect.height;
							//fy = 0 - (child->rect.height * (wheel->focusIndex + wheel->focus_dev - (wheel->itemCount / 2)));
							fy = 0 - ((wheel->focusIndex + wheel->focus_dev - good_center + 1) * child->rect.height);
						}

						text = (ITUText*)child;

						ituWidgetSetY(child, fy);

						child->rect.height = height = height0;

						if (i == (wheel->focusIndex + wheel->focus_dev))
						{
							ituWidgetSetColor(text, wheel->focusColor.alpha, wheel->focusColor.red, wheel->focusColor.green, wheel->focusColor.blue);

							if (wheel->fontsquare == 1)
								ituTextSetFontSize(text, wheel->focusFontHeight);
							else
								ituTextSetFontHeight(text, wheel->focusFontHeight);

							//ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
							//value = ituTextGetString(text);
							//ituWheelOnValueChanged(wheel, value);
						}
						else
						{
							ituWidgetSetColor(child, color.alpha, color.red, color.green, color.blue);
							if (wheel->fontsquare == 1)
								ituTextSetFontSize(text, wheel->fontHeight);
							else
								ituTextSetFontHeight(text, wheel->fontHeight);
						}
						fy += height;
					}
				}
			}
			else
			{
				if ((wheel->focusIndex == 0) || (wheel->focusIndex == fmax))
				{
					wheel->frame = 0;
					wheel->inc = 0;
					widget->flags &= ~ITU_UNDRAGGING;
				}
			}
		}
		else //for cycle TIMER
		{
			if ((widget->flags & ITU_UNDRAGGING) && (wheel->sliding == 0))
			{//bless
				int i, newfocus, count = itcTreeGetChildCount(wheel);
				ITUColor color;
				ITUText* text;
				int fy = 0;
				int height = 0;

				for (i = 0; i < wheel->cycle_arr_count; i++)
				{
					ITUWidget* child;
					int focus_fy = 0;

					if (i == 0)
					{
						child = (ITUWidget*)itcTreeGetChildAt(wheel, 0);
						height = child->rect.height;
						//memcpy(&color, &NColor, sizeof (ITUColor));
						use_normal_color(widget, &color);
						fy = 0 - height * (wheel->itemCount / 2 + wheel->cycle_tor) + (wheel->itemCount / 4 * height);
					}

					focus_fy = fy - ((wheel->focusFontHeight - wheel->fontHeight) / FOCUSFONT_POS_Y_FIX_DIV);

					child = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->cycle_arr[i]);
					text = (ITUText*)child;

					if (wheel->cycle_arr[i] == wheel->focus_c)
					{
						/*
						if (wheel->fontsquare == 1)
							ituTextSetFontSize(text, wheel->focusFontHeight);
						else
							ituTextSetFontHeight(text, wheel->focusFontHeight); */

						//if (child->rect.y != focus_fy)
						//	printf("oh!....different %d %d\n", child->rect.y, focus_fy);
						ituWidgetSetY(child, focus_fy);
						//ituWidgetSetY(child, fy);
						//printf("===[wheel %s][timer][focus child %d][str %s][fy %d]===\n", widget->name, wheel->cycle_arr[i], ituTextGetString(text), focus_fy);
					}
					else
					{
						/*
						if (wheel->fontsquare == 1)
							ituTextSetFontSize(text, wheel->fontHeight);
						else
							ituTextSetFontHeight(text, wheel->fontHeight); */

						ituWidgetSetY(child, fy);
						//printf("===[wheel][timer][child %d][str %s][fy %d]===\n", wheel->cycle_arr[i], ituTextGetString(text), fy);
					}

					fy += height;
					fy += (wheel->inc * wheel->frame / wheel->totalframe);
				}

				wheel->frame++;

				if (wheel->frame > wheel->totalframe)
				{
					if ((wheel->inc > 0) && (wheel->fix_count == 0)) // check
					{
						newfocus = wheel->focus_c - 1;
						if (wheel->cycle_tor && (newfocus < wheel->minci))
							newfocus = wheel->maxci;
						
						if (focus_change(widget, newfocus, __LINE__))
						{
							wheel->idle = 0;
							cycle_arrange(widget, true);
							ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
						}
					}
					else if ((wheel->inc < 0) && (wheel->fix_count == 0)) // check
					{
						newfocus = wheel->focus_c + 1;
						if (wheel->cycle_tor && (newfocus > wheel->maxci))
							newfocus = wheel->minci;
						
						if (focus_change(widget, newfocus, __LINE__))
						{
							wheel->idle = 0;
							cycle_arrange(widget, false);
							ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
						}
					}
					else if (wheel->inc > 0) //for slide
					{
						for (i = 0; i < wheel->fix_count; i++)
						{
							newfocus = wheel->focus_c - 1;
							if (newfocus < wheel->minci)
								newfocus = wheel->maxci;
							
							
							if (focus_change(widget, newfocus, __LINE__))
							{
								wheel->idle = 0;
								cycle_arrange(widget, true);
								ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
							}
						}
					}
					else if (wheel->inc < 0) // for slide
					{
						for (i = 0; i < wheel->fix_count; i++)
						{
							newfocus = wheel->focus_c + 1;
							if (newfocus > wheel->maxci)
								newfocus = wheel->minci;
							
							
							if (focus_change(widget, newfocus, __LINE__))
							{
								wheel->idle = 0;
								cycle_arrange(widget, false);
								ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
							}
						}
					}
					wheel->frame = 0;

					if (wheel->sliding == 0)
					{
						wheel->inc = 0;
					}
					else
					{
						wheel->sliding = 0;
					}


					widget->flags &= ~ITU_UNDRAGGING;

					//text = (ITUText*)itcTreeGetChildAt(wheel, wheel->focus_c);
					//value = ituTextGetString(text);

					//ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
					//ituWheelOnValueChanged(wheel, value);
				}
			}
			else if (wheel->inc) //cycle mode sliding
			{
				int i, fy, height, count = itcTreeGetChildCount(wheel);
				ITUColor color;
				ITUText* text;

				for (i = 0; i < wheel->cycle_arr_count; ++i)
				{
					ITUWidget* child;

					if (i == 0)
					{
						child = (ITUWidget*)itcTreeGetChildAt(wheel, 0);
						//memcpy(&color, &NColor, sizeof (ITUColor));
						use_normal_color(widget, &color);
						height = child->rect.height;
						fy = 0 - (height * (wheel->itemCount / 2 + wheel->cycle_tor)) + ((wheel->itemCount / 4) * height);
					}

					child = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->cycle_arr[i]);
					text = (ITUText*)child;

					if (wheel->cycle_arr[i] == wheel->focus_c)
					{
						ituWidgetSetColor(child, wheel->focusColor.alpha, wheel->focusColor.red, wheel->focusColor.green, wheel->focusColor.blue);
						if (wheel->fontsquare == 1)
							ituTextSetFontSize(text, wheel->focusFontHeight);
						else
							ituTextSetFontHeight(text, wheel->focusFontHeight);
						ituWidgetSetY(child, fy + (wheel->inc * wheel->frame / wheel->totalframe) - ((wheel->focusFontHeight - wheel->fontHeight) / FOCUSFONT_POS_Y_FIX_DIV));
					}
					else
					{
						ituWidgetSetColor(child, color.alpha, color.red, color.green, color.blue);
						if (wheel->fontsquare == 1)
							ituTextSetFontSize(text, wheel->fontHeight);
						else
							ituTextSetFontHeight(text, wheel->fontHeight);
						ituWidgetSetY(child, fy + wheel->inc * wheel->frame / wheel->totalframe);
					}
					fy += height;
				}

				if ((wheel->shift_one) && (widget->flags & ITU_DRAGGABLE))
				{
					wheel->frame = wheel->totalframe;
					wheel->shift_one = 0;
				}

				wheel->frame++;

				if (wheel->frame > wheel->totalframe)
				{
					int newfocus;

					if (wheel->inc > 0)
					{
						newfocus = wheel->focus_c;

						for (i = 0; i < (1 + 0); i++) // +wheel->fix_count
						{
							newfocus--;

							if (newfocus < wheel->minci)
								newfocus = wheel->maxci;
						}
						if (focus_change(widget, newfocus, __LINE__))
						{
							if (!((wheel->totalframe < MOTION_FACTOR) && (wheel->sliding > 0)))
								wheel->idle = 0;
							cycle_arrange(widget, true);
							ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
						}
					}
					else if (wheel->inc < 0)
					{
						newfocus = wheel->focus_c;

						for (i = 0; i < (1 + 0); i++) // +wheel->fix_count
						{
							newfocus++;

							if (newfocus > wheel->maxci)
								newfocus = wheel->minci;
						}
						if (focus_change(widget, newfocus, __LINE__))
						{
							if (!((wheel->totalframe < MOTION_FACTOR) && (wheel->sliding > 0)))
								wheel->idle = 0;
							cycle_arrange(widget, false);
							ituExecActions(widget, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
						}
					}

					wheel->frame = 0;

					///////////////////////////////////
					//Force feedback algorithm cycle-mode
					if ((wheel->totalframe < MOTION_FACTOR) && (wheel->sliding > 0))
					{
						int y1;
						float progress_range;
						double pow_b;
						double fix_mod = ((double)wheel->slide_step / MOTION_THRESHOLD);
						
						if (fix_mod > MAX_INIT_POWER)
							fix_mod = MAX_INIT_POWER;

						pow_b = 5.0 * MAX_INIT_POWER / fix_mod;

						y1 = (int)(pow(pow_b, (double)wheel->totalframe / (MOTION_FACTOR / (5.0 - fix_mod))));

						if (y1 > MOTION_FACTOR)
							y1 = MOTION_FACTOR;
						
						printf("[ydiff %d] [pow_b %.3f] [totalframe %d]\n", wheel->slide_step, pow_b, wheel->totalframe);

						progress_range = (float)wheel->totalframe / MOTION_FACTOR;

						if (wheel->slide_itemcount)
						{
							wheel->slide_itemcount--;
						}
						else
						{
							if (progress_range <= PROCESS_STAGE1)
								wheel->totalframe += y1;
							else if (progress_range <= PROCESS_STAGE2)
								wheel->totalframe += y1 * 3;
							else
								wheel->totalframe += y1 * (int)fix_mod;
						}

						if (wheel->totalframe > 40)
							wheel->totalframe = MOTION_FACTOR;
					}
					else
					{
						wheel->inc = 0;
						widget->flags &= ~ITU_UNDRAGGING;
						wheel->totalframe = (int)ituWidgetGetCustomData(wheel);

						if (wheel->sliding == 1)
							wheel->sliding = 0;
					}
				}
			} 
		}
    }
    result |= widget->dirty;
    return widget->visible ? result : false;
}

void ituWheelDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
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
}

static void WheelOnValueChanged(ITUWheel* wheel, char* value)
{
    // DO NOTHING
}

void ituWheelInit(ITUWheel* wheel)
{
    assert(wheel);
    ITU_ASSERT_THREAD();

    memset(wheel, 0, sizeof (ITUWheel));

    ituFlowWindowInit(&wheel->fwin, ITU_LAYOUT_UP);

    ituWidgetSetType(wheel, ITU_WHEEL);
    ituWidgetSetName(wheel, wheelName);
    ituWidgetSetUpdate(wheel, ituWheelUpdate);
    ituWidgetSetDraw(wheel, ituWheelDraw);
    ituWidgetSetOnAction(wheel, ituWheelOnAction);
    ituWheelSetValueChanged(wheel, WheelOnValueChanged);
}

void ituWheelLoad(ITUWheel* wheel, uint32_t base)
{
	ITUWidget* widget = (ITUWidget*)wheel;

    assert(wheel);

    ituFlowWindowLoad(&wheel->fwin, base);

	//set custom data to original totalframe
	ituWidgetSetCustomData(widget, wheel->totalframe);

    ituWidgetSetUpdate(wheel, ituWheelUpdate);
    ituWidgetSetDraw(wheel, ituWheelDraw);
    ituWidgetSetOnAction(wheel, ituWheelOnAction);
    ituWheelSetValueChanged(wheel, WheelOnValueChanged);
}

void ituWheelOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    unsigned int oldFlags;
	ITUWheel* wheel = (ITUWheel*)widget;
    assert(widget);

    switch (action)
    {
    case ITU_ACTION_PREV:
		if ((wheel->cycle_tor <= 0) && (!(widget->flags & ITU_DRAGGABLE)))
		{
			oldFlags = widget->flags;
			widget->flags |= ITU_TOUCHABLE;
			wheel->shift_one = 0;
			ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDEUP, 0, widget->rect.x, widget->rect.y);
			if ((oldFlags & ITU_TOUCHABLE) == 0)
				widget->flags &= ~ITU_TOUCHABLE;
		}
		else
		{
			ituWheelPrev(wheel);
		}
		
        break;

    case ITU_ACTION_NEXT:
		if ((wheel->cycle_tor <= 0) && (!(widget->flags & ITU_DRAGGABLE)))
		{
			oldFlags = widget->flags;
			widget->flags |= ITU_TOUCHABLE;
			wheel->shift_one = 0;
			ituWidgetUpdate(widget, ITU_EVENT_TOUCHSLIDEDOWN, 0, widget->rect.x, widget->rect.y);
			if ((oldFlags & ITU_TOUCHABLE) == 0)
				widget->flags &= ~ITU_TOUCHABLE;
		}
		else
		{
			ituWheelNext(wheel);
		}
		
        break;

    case ITU_ACTION_GOTO:
        if (param[0] != '\0')
            ituWheelGoto((ITUWheel*)widget, atoi(param));
        break;

    case ITU_ACTION_DODELAY0:
        ituExecActions(widget, ((ITUWheel*)widget)->actions, ITU_EVENT_DELAY0, atoi(param));
        break;

    case ITU_ACTION_DODELAY1:
        ituExecActions(widget, ((ITUWheel*)widget)->actions, ITU_EVENT_DELAY1, atoi(param));
        break;

    case ITU_ACTION_DODELAY2:
        ituExecActions(widget, ((ITUWheel*)widget)->actions, ITU_EVENT_DELAY2, atoi(param));
        break;

    case ITU_ACTION_DODELAY3:
        ituExecActions(widget, ((ITUWheel*)widget)->actions, ITU_EVENT_DELAY3, atoi(param));
        break;

    case ITU_ACTION_DODELAY4:
        ituExecActions(widget, ((ITUWheel*)widget)->actions, ITU_EVENT_DELAY4, atoi(param));
        break;

    case ITU_ACTION_DODELAY5:
        ituExecActions(widget, ((ITUWheel*)widget)->actions, ITU_EVENT_DELAY5, atoi(param));
        break;

    case ITU_ACTION_DODELAY6:
        ituExecActions(widget, ((ITUWheel*)widget)->actions, ITU_EVENT_DELAY6, atoi(param));
        break;

    case ITU_ACTION_DODELAY7:
        ituExecActions(widget, ((ITUWheel*)widget)->actions, ITU_EVENT_DELAY7, atoi(param));
        break;

    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

void ituWheelPrev(ITUWheel* wheel)
{
	int fmax;

    assert(wheel);
    ITU_ASSERT_THREAD();

	fmax = get_max_focusindex((ITUWidget*)wheel);

	if (wheel->cycle_tor)
	{
		if (wheel->focusIndex > 0)
			wheel->focusIndex--;
		else
			wheel->focusIndex = fmax;
	}
	else
	{
		if (wheel->focusIndex > 0)
			wheel->focusIndex--;
	}

	ituWidgetUpdate(wheel, ITU_EVENT_LAYOUT, 0, 0, 0);
	ituExecActions((ITUWidget*)wheel, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);

	/*
    if (wheel->focusIndex > 0)
    {
        ITUText* text;
        char* value;
        wheel->focusIndex--;
        ituWidgetUpdate(wheel, ITU_EVENT_LAYOUT, 0, 0, 0);

        text = (ITUText*)itcTreeGetChildAt(wheel, wheel->itemCount / 2 + wheel->focusIndex);
        value = ituTextGetString(text);

        ituExecActions((ITUWidget*)wheel, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
        //ituWheelOnValueChanged(wheel, value);
    }
	*/
}

void ituWheelNext(ITUWheel* wheel)
{
	int fmax;

	assert(wheel);
    ITU_ASSERT_THREAD();

	fmax = get_max_focusindex((ITUWidget*)wheel);

	if (wheel->cycle_tor)
	{
		if (wheel->focusIndex < fmax)
			wheel->focusIndex++;
		else
			wheel->focusIndex = 0;
	}
	else
	{
		if (wheel->focusIndex < fmax)
			wheel->focusIndex++;
	}

	ituWidgetUpdate(wheel, ITU_EVENT_LAYOUT, 0, 0, 0);
	ituExecActions((ITUWidget*)wheel, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);

	/*
    int count = itcTreeGetChildCount(wheel) - wheel->itemCount;

    if (wheel->focusIndex < count)
    {
        ITUText* text;
        char* value;
        wheel->focusIndex++;
        ituWidgetUpdate(wheel, ITU_EVENT_LAYOUT, 0, 0, 0);

        text = (ITUText*)itcTreeGetChildAt(wheel, wheel->itemCount / 2 + wheel->focusIndex);
        value = ituTextGetString(text);

        ituExecActions((ITUWidget*)wheel, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
        //ituWheelOnValueChanged(wheel, value);
    }
	*/
}

ITUWidget* ituWheelGetFocusItem(ITUWheel* wheel)
{
	ITUWidget* item;

    assert(wheel);
    ITU_ASSERT_THREAD();

	if (wheel->focusIndex >= 0)
	{
		item = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->focus_dev + wheel->focusIndex);
		return item;
	}
	return NULL;

	/*
    if (wheel->focusIndex >= 0)
    {
        ITUWidget* item = (ITUWidget*)itcTreeGetChildAt(wheel, wheel->itemCount / 2 + wheel->focusIndex);
        return item;
    }
    return NULL;
	*/
}

void ituWheelGoto(ITUWheel* wheel, int index)
{
	bool debug_print = false;

    ITU_ASSERT_THREAD();

	if ((wheel->cycle_tor == 1) && (wheel->maxci == 0))
		ituWidgetUpdate(wheel, ITU_EVENT_LAYOUT, 0, 0, 0);

	if (debug_print)
		printf("[WHEEL][GOTO %d >> %d]sliding:%d, frame:%d, totalframe:%d, inc:%d\n", wheel->focusIndex, index, wheel->sliding, wheel->frame, wheel->totalframe, wheel->inc);

	if (wheel->sliding)
	{
		wheel->sliding = 0;
		wheel->frame = 0;
		wheel->inc = 0;
		wheel->focusIndex = index;
		wheel->totalframe = (int)ituWidgetGetCustomData(wheel);
		ituWidgetUpdate(wheel, ITU_EVENT_LAYOUT, 0, 0, 0);
		ituWheelGoto(wheel, index);
	}
	else
	{
		if (wheel->cycle_tor)
		{
			int i, shift, newfocus;
			ITUWidget* widget = (ITUWidget*)wheel;

			if ((index + wheel->focus_dev) > wheel->maxci)
				return;

			if ((index == 0) && (wheel->focusIndex != 0))
			{
				bool check = true;
				bool way = true;

				way = ((wheel->maxci - wheel->focusIndex) > wheel->focusIndex) ? (true) : (false);

				while (check)
				{
					if (way)
					{
						newfocus = wheel->focus_c - 1;

						if (debug_print)
							printf("[WHEEL][GOTO %d][A][newfc %d][minci %d]\n", index, newfocus, wheel->minci);

						if (newfocus < wheel->minci)
						{
							check = false;
							newfocus = wheel->maxci;
						}

						//patch for KL
						if (newfocus == index)
							check = false;

						if (focus_change(widget, newfocus, __LINE__))
							cycle_arrange(widget, true);
					}
					else
					{
						newfocus = wheel->focus_c + 1;

						if (debug_print)
							printf("[WHEEL][GOTO %d][B][newfc %d][maxci %d]\n", index, newfocus, wheel->maxci);

						if (newfocus > wheel->maxci)
						{
							check = false;
							newfocus = wheel->minci;
						}

						//patch for KL
						if (newfocus == index)
							check = false;

						if (focus_change(widget, newfocus, __LINE__))
							cycle_arrange(widget, false);
					}
				}
			}
			else if (index < wheel->focusIndex)
			{
				shift = wheel->focusIndex - index;
				for (i = 0; i < shift; i++)
				{
					newfocus = wheel->focus_c - 1;

					if (debug_print)
						printf("[WHEEL][GOTO %d][C][newfc %d][maxci %d][%d/%d]\n", index, newfocus, wheel->minci, i, shift);

					if (newfocus < wheel->minci)
						newfocus = wheel->maxci;
					//cycle_arrange(widget, true);
					if (focus_change(widget, newfocus, __LINE__))
						cycle_arrange(widget, true);
				}
			}
			else if (index > wheel->focusIndex)
			{
				shift = index - wheel->focusIndex;
				for (i = 0; i < shift; i++)
				{
					newfocus = wheel->focus_c + 1;

					if (debug_print)
						printf("[WHEEL][GOTO %d][D][newfc %d][maxci %d][%d/%d]\n", index, newfocus, wheel->maxci, i, shift);

					if (newfocus > wheel->maxci)
						newfocus = wheel->minci;
					//cycle_arrange(widget, false);
					if (focus_change(widget, newfocus, __LINE__))
						cycle_arrange(widget, false);
				}
			}
			else
			{
				if (debug_print)
					printf("[WHEEL][GOTO][E][focus %d][goto %d]\n", wheel->focusIndex, index);

				cycle_arrange(widget, true);
				cycle_arrange(widget, false);
			}

			//widget->flags |= ITU_UNDRAGGING;
			//widget->flags &= ~ITU_DRAGGING;
			wheel->touchCount = 0;
			wheel->inc = 0;
			wheel->frame = wheel->totalframe;
			ituWidgetUpdate(wheel, ITU_EVENT_LAYOUT, 0, 0, 0);
		}
		else
		{
			int count = itcTreeGetChildCount(wheel);

			if (index >= count)
				return;

			wheel->focusIndex = index;
			ituWidgetUpdate(wheel, ITU_EVENT_LAYOUT, 0, 0, 0);
		}
	}

	ituExecActions((ITUWidget*)wheel, wheel->actions, ITU_EVENT_CHANGED, wheel->focusIndex);
}

void ituWheelScal(ITUWheel* wheel, int scal)
{
    ITU_ASSERT_THREAD();

	if (scal >= 1)
		wheel->scal = scal;
}

bool ituWheelCheckIdle(ITUWheel* wheel)
{
    ITU_ASSERT_THREAD();
	return (wheel->idle <= 0) ? (true) : (false);
}


int ituWheelItemCount(ITUWheel* wheel)
{
	int count;
    ITU_ASSERT_THREAD();
    
    count = get_max_focusindex(wheel) + 1;
	return count;
}

bool ituWheelSetItemTree(ITUWheel* wheel, char** stringarr, int itemcount)
{
    ITU_ASSERT_THREAD();

	if ((!wheel) || (itemcount <= 0) || (itemcount > ITU_WHEEL_CYCLE_ARR_LIMIT) || (stringarr[itemcount - 1] == NULL))
	{
		return false;
	}
	else
	{
		int i, tick;
		ITCTree *child, *tree = (ITCTree*)wheel;

		tick = (ituWheelItemCount(wheel) - itemcount);
		wheel->focusIndex = 0;

		while (tick > 0)
		{
			for (child = tree->child; child; child = child->sibling)
			{
				if (!(child->sibling))
				{
					itcTreeRemove(child);
					tick--;
					break;
				}
			}
		}

		while (tick < 0)
		{
			ITUText* text = (ITUText*)itcTreeGetChildAt(wheel, 0);
			ITUText* cloneobj = NULL;
			bool cloned = ituTextClone(text, &cloneobj);

			if (cloned)
			{
				ITCTree *clonechild = (ITCTree*)cloneobj;
				itcTreePushBack(tree, clonechild);
				tick++;
			}
			else
			{
				return false;
			}
		}

		for (i = 0; i < itemcount; i++)
		{
			printf("<<ituWheelSetItemTree>> child %d [%s]\n", i, stringarr[i]);
		}

		for (i = 0; i < itemcount; i++)
		{
			ITUWidget* item = (ITUWidget*)itcTreeGetChildAt(wheel, i);
			ITUText* text = (ITUText*)item;
			ituTextSetString(text, stringarr[i]);
		}

		ituWidgetUpdate(wheel, ITU_EVENT_LAYOUT, 0, 0, 0);
		return true;
	}
}
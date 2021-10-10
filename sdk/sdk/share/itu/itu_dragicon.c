#include <assert.h>
#include <malloc.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char dragiconName[] = "ITUDragIcon";

//The initial background width %
#define INIT_BACKGROUND_W 25

//The initial background height %
#define INIT_BACKGROUND_H 25

bool ituDragIconUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
	ITUDragIcon* dragicon = (ITUDragIcon*)widget;
	assert(dragicon);

    result |= ituIconUpdate(widget, ev, arg1, arg2, arg3);

    if (ev == ITU_EVENT_MOUSEDOWN)
    {
		int x = arg2 - widget->rect.x;
		int y = arg3 - widget->rect.y;

        if (ituWidgetIsEnabled(widget))
        {
            if (ituWidgetIsInside(widget, x, y))
            {
				dragicon->touchX = x;
				dragicon->touchY = y;
				//printf("[DRAGICON] down [%d\t%d]\n", dragicon->touchX, dragicon->touchY);
				result |= ituExecActions((ITUWidget*)dragicon, dragicon->actions, ev, 0);
            }
        }
    }
	else if (ev == ITU_EVENT_MOUSEMOVE)
	{
		if (ituWidgetIsEnabled(widget))
		{
			int x = arg2 - widget->rect.x;
			int y = arg3 - widget->rect.y;

			if (ituWidgetIsInside(widget, x, y) && dragicon->working)
			{
				if (dragicon->surf)
				{
					int max_move_x = (widget->rect.width - dragicon->surf->width) / 2;
					int max_move_y = (widget->rect.height - dragicon->surf->height) / 2;
					int move_x_val = x - dragicon->touchX;
					int move_y_val = y - dragicon->touchY;
					int circle_border_line_width = dragicon->surf->width * 1 / 10;
					bool auto_fix = false;

					if ((dragicon->working > 1) && dragicon->bsurf && (dragicon->bsurf->width > (dragicon->surf->width + circle_border_line_width)) && (dragicon->frame == dragicon->totalframe))
					{
						bool rad_inside = true;
						double point_d = sqrt((double)((move_x_val*move_x_val) + (move_y_val*move_y_val)));

						if (point_d >= ((dragicon->bsurf->width - dragicon->surf->width - circle_border_line_width) * 0.5))
						{
							rad_inside = false;
						}

						if (rad_inside)
						{
							dragicon->moveX = move_x_val;
							dragicon->moveY = move_y_val;
						}
						else if (auto_fix)
						{
							double tc = ((dragicon->bsurf->width - dragicon->surf->width - circle_border_line_width) * 0.5);

							if (abs(move_x_val) >= abs(dragicon->moveX))
							{
								while (sqrt((double)((move_x_val*move_x_val) + (move_y_val*move_y_val))) >= tc)
								{
									move_x_val += (move_x_val >= 0) ? (-1) : (1);

									if (move_y_val < 0)
										move_y_val += 2;
									else
										move_y_val -= 2;

									if (abs(move_y_val) >= max_move_y)
									{
										move_y_val = (move_y_val > 0) ? (max_move_y) : (max_move_y * -1);
										move_x_val = 0;
										break;
									}
								}
								dragicon->moveX = move_x_val;
								dragicon->moveY = move_y_val;
							}
							else if (abs(move_y_val) >= abs(dragicon->moveY))
							{
								while (sqrt((double)((move_x_val*move_x_val) + (move_y_val*move_y_val))) >= tc)
								{
									move_y_val += (move_y_val >= 0) ? (-1) : (1);

									if (move_x_val < 0)
										move_x_val += 2;
									else
										move_x_val -= 2;

									if (abs(move_x_val) >= max_move_x)
									{
										move_x_val = (move_x_val > 0) ? (max_move_x) : (max_move_x * -1);
										move_y_val = 0;
										break;
									}
								}
								dragicon->moveX = move_x_val;
								dragicon->moveY = move_y_val;
							}
						}
					}
					else
					{
						dragicon->moveX = move_x_val;
						dragicon->moveY = move_y_val;
					}

					if (dragicon->moveX < (max_move_x * -1))
						dragicon->moveX = max_move_x * -1;
					else if (dragicon->moveX > max_move_x)
						dragicon->moveX = max_move_x;

					if (dragicon->moveY < (max_move_y * -1))
						dragicon->moveY = max_move_y * -1;
					else if (dragicon->moveY > max_move_y)
						dragicon->moveY = max_move_y;
					//printf("[DRAGICON] move [%d\t%d]\n", dragicon->moveX, dragicon->moveY);
				}
				else
				{
					dragicon->moveX = 0;
					dragicon->moveY = 0;
				}
				result = true;
			}
		}
	}
    else if (ev == ITU_EVENT_MOUSEUP)
    {
        if (ituWidgetIsEnabled(widget))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

			if (ituWidgetIsInside(widget, x, y) || (dragicon->working > 0))
				result |= ituExecActions((ITUWidget*)dragicon, dragicon->actions, ev, 0);
        }

		dragicon->touchX = 0;
		dragicon->touchY = 0;
		dragicon->moveX = 0;
		dragicon->moveY = 0;
		dragicon->working = 0;
		dragicon->frame = 0;
    }
    else if (ev == ITU_EVENT_MOUSELONGPRESS)
    {
		if (ituWidgetIsEnabled(widget))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
				if (arg1 > 1)
					dragicon->working = arg1;
				else
					dragicon->working = 1;

				result |= ituExecActions(widget, dragicon->actions, ev, arg1);
				//result = true;
				//widget->dirty |= result;
            }
        }
    }
    else if (ev >= ITU_EVENT_CUSTOM || ev == ITU_EVENT_TIMER)
    {
        if (ituWidgetIsEnabled(widget))
        {
			//result |= ituExecActions((ITUWidget*)dragicon, dragicon->actions, ev, arg1);
			result = true;
        }
    }
    else if (ituWidgetIsActive(widget) && ituWidgetIsEnabled(widget) && !result)
    {
        switch (ev)
        {
        case ITU_EVENT_KEYDOWN:
            if (arg1 == ituScene->enterKey)
            {
				ituFocusWidget(dragicon);
				result |= ituExecActions((ITUWidget*)dragicon, dragicon->actions, ev, arg1);
            }
            break;

        case ITU_EVENT_KEYUP:
            if (arg1 == ituScene->enterKey)
            {
				result |= ituExecActions((ITUWidget*)dragicon, dragicon->actions, ev, arg1);
            }
            break;
        }
    }

    return result;
}

void ituDragIconOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
	ITUDragIcon* dragicon = (ITUDragIcon*)widget;
	assert(dragicon);

    switch (action)
    {
    case ITU_ACTION_DODELAY0:
		ituExecActions(widget, dragicon->actions, ITU_EVENT_DELAY0, atoi(param));
        break;

    case ITU_ACTION_DODELAY1:
		ituExecActions(widget, dragicon->actions, ITU_EVENT_DELAY1, atoi(param));
        break;

    case ITU_ACTION_DODELAY2:
		ituExecActions(widget, dragicon->actions, ITU_EVENT_DELAY2, atoi(param));
        break;

    case ITU_ACTION_DODELAY3:
		ituExecActions(widget, dragicon->actions, ITU_EVENT_DELAY3, atoi(param));
        break;

    case ITU_ACTION_DODELAY4:
		ituExecActions(widget, dragicon->actions, ITU_EVENT_DELAY4, atoi(param));
        break;

    case ITU_ACTION_DODELAY5:
		ituExecActions(widget, dragicon->actions, ITU_EVENT_DELAY5, atoi(param));
        break;

    case ITU_ACTION_DODELAY6:
		ituExecActions(widget, dragicon->actions, ITU_EVENT_DELAY6, atoi(param));
        break;

    case ITU_ACTION_DODELAY7:
		ituExecActions(widget, dragicon->actions, ITU_EVENT_DELAY7, atoi(param));
        break;

    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

void ituDragIconDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
	int destx, desty;
	int cdestx, cdesty;
	uint8_t desta;
	ITURectangle prevClip;
	ITUDragIcon* icon = (ITUDragIcon*)widget;
	ITURectangle* rect = &widget->rect;
	assert(icon);
	assert(dest);

	if (!icon->surf)
	{
		ituWidgetSetDirty(widget, false);
		return;
	}

	destx = rect->x + x;
	desty = rect->y + y;
	desta = alpha * widget->color.alpha / 255;
	desta = desta * widget->alpha / 255;
	
	if (icon->bsurf)
	{
		cdestx = destx + ((rect->width - icon->surf->width) / 2);
		cdesty = desty + ((rect->height - icon->surf->height) / 2);
	}

	if (widget->angle == 0)
		ituWidgetSetClipping(widget, dest, x, y, &prevClip);

	if (desta > 0)
	{
		if (desta == 255)
		{
			if (widget->flags & ITU_STRETCH)
			{
#if (CFG_CHIP_FAMILY == 9070)
				if (widget->angle == 0)
				{
					if (widget->transformType == ITU_TRANSFORM_NONE)
					{
						ituStretchBlt(dest, destx, desty, rect->width, rect->height, icon->surf, 0, 0, icon->surf->width, icon->surf->height);
					}
					else
					{
						ITUSurface* surf = ituCreateSurface(icon->surf->width, icon->surf->height, 0, dest->format, NULL, 0);
						if (surf)
						{
							int w = (icon->surf->width - icon->surf->width * widget->transformX / 100) / 2;
							int h = (icon->surf->height - icon->surf->height * widget->transformY / 100) / 2;

							ituStretchBlt(surf, 0, 0, icon->surf->width, icon->surf->height, dest, destx, desty, rect->width, rect->height);

							switch (widget->transformType)
							{
							case ITU_TRANSFORM_TURN_LEFT:
                                ituTransformBlt(surf, 0, 0, icon->surf, 0, 0, icon->surf->width, icon->surf->height, w, h, icon->surf->width - w, 0, icon->surf->width - w, icon->surf->height, w, icon->surf->height - h, true, ITU_PAGEFLOW_FOLD2, widget->transformType);
								break;

							case ITU_TRANSFORM_TURN_TOP:
                                ituTransformBlt(surf, 0, 0, icon->surf, 0, 0, icon->surf->width, icon->surf->height, w, h, icon->surf->width - w, h, icon->surf->width, icon->surf->height - h, 0, icon->surf->height - h, true, ITU_PAGEFLOW_FOLD2, widget->transformType);
								break;

							case ITU_TRANSFORM_TURN_RIGHT:
                                ituTransformBlt(surf, 0, 0, icon->surf, 0, 0, icon->surf->width, icon->surf->height, w, 0, icon->surf->width - w, h, icon->surf->width - w, icon->surf->height - h, w, icon->surf->height, false, ITU_PAGEFLOW_FOLD2, widget->transformType);
								break;

							case ITU_TRANSFORM_TURN_BOTTOM:
                                ituTransformBlt(surf, 0, 0, icon->surf, 0, 0, icon->surf->width, icon->surf->height, 0, h, icon->surf->width, h, icon->surf->width - w, icon->surf->height - h, w, icon->surf->height - h, false, ITU_PAGEFLOW_FOLD2, widget->transformType);
								break;
							}
							ituStretchBlt(dest, destx, desty, rect->width, rect->height, surf, 0, 0, surf->width, surf->height);
							ituDestroySurface(surf);
						}
					}
				}
				else
				{
					float scaleX = (float)rect->width / icon->surf->width;
					float scaleY = (float)rect->height / icon->surf->height;

					ituRotate(dest, destx + rect->width / 2, desty + rect->height / 2, icon->surf, icon->surf->width / 2, icon->surf->height / 2, (float)widget->angle, scaleX, scaleY);
				}
#else
				float scaleX = (float)rect->width / icon->surf->width;
				float scaleY = (float)rect->height / icon->surf->height;

				ituTransform(
					dest, destx, desty, rect->width, rect->height,
					icon->surf, 0, 0, icon->surf->width, icon->surf->height,
					icon->surf->width / 2, icon->surf->height / 2,
					scaleX,
					scaleY,
					(float)widget->angle,
					0,
					true,
					true,
					desta);
#endif
			}
			else
			{
				if (widget->angle == 0)
				{
					if (widget->transformType == ITU_TRANSFORM_NONE)
					{//Bless
						if ((icon->totalframe > 0) && (icon->frame < icon->totalframe) && icon->working)
						{
							int dx, dy, dw, dh;
							icon->frame++;
							dw = (rect->width * (100 - icon->pw) / 100) * icon->frame / icon->totalframe + (rect->width * icon->pw / 100);
							dh = (rect->height * (100 - icon->ph) / 100) * icon->frame / icon->totalframe + (rect->height * icon->ph / 100);
							dx = destx + (rect->width  - dw) / 2;
							dy = desty + (rect->height - dh) / 2;

							ituStretchBlt(dest, dx, dy, dw, dh, icon->bsurf, 0, 0, icon->bsurf->width, icon->bsurf->height);
						}
						else
						{
							if ((icon->totalframe == 0) || (icon->frame == icon->totalframe))
								ituBitBlt(dest, destx, desty, rect->width, rect->height, icon->bsurf, 0, 0);
						}

						if (icon->working)
						{
							//int mx = icon->moveX;
							//int my = icon->moveY;



							ituBitBlt(dest, cdestx + icon->moveX, cdesty + icon->moveY, icon->surf->width, icon->surf->height, icon->surf, 0, 0);
						}
						else
						{
							if (icon->totalframe > 0)
							{
								int dx, dy, dw, dh;
								dw = rect->width * icon->pw / 100;
								dh = rect->height * icon->ph / 100;
								dx = destx + (rect->width - dw) / 2;
								dy = desty + (rect->height - dh) / 2;

								ituStretchBlt(dest, dx, dy, dw, dh, icon->bsurf, 0, 0, icon->bsurf->width, icon->bsurf->height);
							}
							else
								ituBitBlt(dest, cdestx, cdesty, icon->surf->width, icon->surf->height, icon->surf, 0, 0);
						}
						//ituBitBlt(dest, destx, desty, icon->surf->width, icon->surf->height, icon->surf, 0, 0);
					}
					else
					{
						int w = (rect->width - rect->width * widget->transformX / 100) / 2;
						int h = (rect->height - rect->height * widget->transformY / 100) / 2;

						switch (widget->transformType)
						{
						case ITU_TRANSFORM_TURN_LEFT:
                            ituTransformBlt(dest, destx, desty, icon->surf, 0, 0, rect->width, rect->height, w, h, rect->width - w, 0, rect->width - w, rect->height, w, rect->height - h, true, ITU_PAGEFLOW_FOLD2, widget->transformType);
							break;

						case ITU_TRANSFORM_TURN_TOP:
                            ituTransformBlt(dest, destx, desty, icon->surf, 0, 0, rect->width, rect->height, w, h, rect->width - w, h, rect->width, rect->height - h, 0, rect->height - h, true, ITU_PAGEFLOW_FOLD2, widget->transformType);
							break;

						case ITU_TRANSFORM_TURN_RIGHT:
                            ituTransformBlt(dest, destx, desty, icon->surf, 0, 0, rect->width, rect->height, w, 0, rect->width - w, h, rect->width - w, rect->height - h, w, rect->height, false, ITU_PAGEFLOW_FOLD2, widget->transformType);
							break;

						case ITU_TRANSFORM_TURN_BOTTOM:
                            ituTransformBlt(dest, destx, desty, icon->surf, 0, 0, rect->width, rect->height, 0, h, rect->width, h, rect->width - w, rect->height - h, w, rect->height - h, false, ITU_PAGEFLOW_FOLD2, widget->transformType);
							break;
						}
					}
				}
				else
				{
#if (CFG_CHIP_FAMILY == 9070)
					ituRotate(dest, destx + rect->width / 2, desty + rect->height / 2, icon->surf, icon->surf->width / 2, icon->surf->height / 2, (float)widget->angle, 1.0f, 1.0f);
#else
					ituRotate(dest, destx, desty, icon->surf, icon->surf->width / 2, icon->surf->height / 2, (float)widget->angle, 1.0f, 1.0f);
#endif
				}
			}
		}
		else
		{
			if (widget->flags & ITU_STRETCH)
			{
#if (CFG_CHIP_FAMILY == 9070)
				ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
				if (surf)
				{
					ituBitBlt(surf, 0, 0, rect->width, rect->height, dest, destx, desty);

					if (widget->angle == 0)
					{
						ituStretchBlt(surf, 0, 0, rect->width, rect->height, icon->surf, 0, 0, icon->surf->width, icon->surf->height);
					}
					else
					{
						float scaleX = (float)rect->width / icon->surf->width;
						float scaleY = (float)rect->height / icon->surf->height;

						ituRotate(surf, rect->width / 2, rect->height / 2, icon->surf, icon->surf->width / 2, icon->surf->height / 2, (float)widget->angle, scaleX, scaleY);
					}
					ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);
					ituDestroySurface(surf);
				}
#else
				float scaleX = (float)rect->width / icon->surf->width;
				float scaleY = (float)rect->height / icon->surf->height;

				ituTransform(
					dest, destx, desty, rect->width, rect->height,
					icon->surf, 0, 0, icon->surf->width, icon->surf->height,
					icon->surf->width / 2, icon->surf->height / 2,
					scaleX,
					scaleY,
					(float)widget->angle,
					0,
					true,
					true,
					desta);
#endif
			}
			else
			{
				ituAlphaBlend(dest, destx, desty, rect->width, rect->height, icon->surf, 0, 0, desta);
			}
		}
	}

	if (widget->angle == 0)
		ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);

	ituWidgetDrawImpl(widget, dest, x, y, alpha);
}

bool ituDragIconPress(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
	return true;
}

void ituDragIconInit(ITUDragIcon* dragicon)
{
    assert(dragicon);
    ITU_ASSERT_THREAD();

	memset(dragicon, 0, sizeof (ITUDragIcon));

	//ituIconInit
    //ituBackgroundInit(&dragicon->bg);

	ituWidgetSetType(dragicon, ITU_DRAGICON);
	ituWidgetSetName(dragicon, dragiconName);
	ituWidgetSetUpdate(dragicon, ituDragIconUpdate);
	ituWidgetSetDraw(dragicon, ituDragIconDraw);
	//ituWidgetSetOnPress(dragicon, ituDragIconPress);
	ituWidgetSetOnAction(dragicon, ituDragIconOnAction);
}

void ituDragIconLoad(ITUDragIcon* dragicon, uint32_t base)
{
	assert(dragicon);

	//ituBackgroundLoad(&dragicon->bg, base);
	ituWidgetLoad((ITUWidget*)dragicon, base);
	//ituIconLoad(&dragicon->staticIconSurf, base);

	if (dragicon->staticIconSurf)
	{
		ITUSurface* surf = (ITUSurface*)(base + (uint32_t)dragicon->staticIconSurf);
		dragicon->surf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
		dragicon->staticIconSurf = surf;
	}


	if (dragicon->staticBackSurf)
	{
		ITUSurface* surf = (ITUSurface*)(base + (uint32_t)dragicon->staticBackSurf);
		dragicon->bsurf = ituCreateSurface(surf->width, surf->height, surf->pitch, surf->format, (const uint8_t*)surf->addr, surf->flags);
		dragicon->staticBackSurf = surf;
	}

	ituWidgetSetUpdate(dragicon, ituDragIconUpdate);
	ituWidgetSetDraw(dragicon, ituDragIconDraw);
	//ituWidgetSetOnPress(dragicon, ituDragIconPress);
	ituWidgetSetOnAction(dragicon, ituDragIconOnAction);
}


void ituDragIconMouseShift(ITUWidget* widget, int* shiftX, int* shiftY)
{
	ITUDragIcon* dragicon = (ITUDragIcon*)widget;

	assert(dragicon);
    ITU_ASSERT_THREAD();

	*shiftX = dragicon->moveX;
	*shiftY = dragicon->moveY;
}


bool ituDragIconIsWorking(ITUWidget* widget)
{
	ITUDragIcon* dragicon = (ITUDragIcon*)widget;

	assert(dragicon);
    ITU_ASSERT_THREAD();

	if (dragicon->working)
		return true;
	else
		return false;
}

void ituDragIconSetAniBSN(ITUWidget* widget, int pw, int ph)
{
	ITUDragIcon* dragicon = (ITUDragIcon*)widget;

	assert(dragicon);
    ITU_ASSERT_THREAD();

	dragicon->pw = pw;
	dragicon->ph = ph;
}

void ituDragIconReset(ITUWidget* widget)
{
	ITUDragIcon* dragicon = (ITUDragIcon*)widget;

	assert(dragicon);
    ITU_ASSERT_THREAD();

	if (dragicon->working)
	{
		dragicon->touchX = 0;
		dragicon->touchY = 0;
		dragicon->moveX = 0;
		dragicon->moveY = 0;
		dragicon->working = 0;
		dragicon->frame = 0;
	}
}

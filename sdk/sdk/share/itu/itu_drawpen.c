#include <assert.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <pthread.h>
#include <semaphore.h>
#include "ite/itu.h"
#include "ite/itp.h"
#include "itu_cfg.h"
#include "itu_private.h"


#define PENCURSOR_HEADSIZE 6
#define MAX_PENCURVE_ARRAY 512 //1024
#define MAX_PEN_PRESSURE 16
#define MIN_PEN_PRESSURE 4
#define SMOOTH_FACTOR 10 //20
// 20 is limit
//#define DRAWPEN_USB_DEVICE_DEBUG
//#define SEND_MAX_CURVE_RAW
#define SEND_MAX_CURVE_RAW_FACTOR 2

#define ID_CUSTOER_DEMOLOGGER 0x99990000
#define ID_CUSTOER_DEMOECHO 0x99990001
#define ID_DRAWPEN_USB_CURVE 0x99990002
#define ID_DRAWPEN_USB_MOUSEDOWN 0x99990003
#define ID_DRAWPEN_USB_MOUSEUP 0x99990004
#define ID_DRAWPEN_USB_BUTTON_OK 0x99990005
#define ID_DRAWPEN_USB_BUTTON_CANCEL 0x99990006
#define ID_DRAWPEN_USB_BUTTON_CLEAR 0x99990007

#define ID_DRAWPEN_USB_TEST 0x99999999


#define FLAGS_OVERRUN 0x0001

static unsigned int blitcount = 0;

struct FIFOB{
	int *buf;
	int putP, getP, size, free, flags;
};

static const char drawpenName[] = "ITUDrawPen";

#ifdef SEND_MAX_CURVE_RAW
static int ARRAYDATA[(MAX_PENCURVE_ARRAY * 2 * SEND_MAX_CURVE_RAW_FACTOR) + 4 + 5] = { 0 };
static int ARRAYX2[MAX_PENCURVE_ARRAY * SEND_MAX_CURVE_RAW_FACTOR] = { 0 };
static int ARRAYY2[MAX_PENCURVE_ARRAY * SEND_MAX_CURVE_RAW_FACTOR] = { 0 };
#else
static int ARRAYDATA[(MAX_PENCURVE_ARRAY * 2) + 4 + 5] = { 0 };
static int ARRAYX[MAX_PENCURVE_ARRAY] = { 0 };
static int ARRAYY[MAX_PENCURVE_ARRAY] = { 0 };
#endif

static int BUFFERX[1024] = { 0 };
static int BUFFERY[1024] = { 0 };
static struct FIFOB fifoX;
static struct FIFOB fifoY;

static ITUColor emptycolor;
static ITUSurface* pressure_surf[MAX_PEN_PRESSURE + 1];
static sem_t sem;
static pthread_t task;
static sem_t sem_pp;
static pthread_t task_pp;

static int fsg_connected = 0;
struct prop {
	unsigned int size;
	unsigned int id;
	unsigned int flags;
	signed int status;
};

#ifdef _WIN32
int idbSend2Host(void* data, int size, int wait)
{
	return 0;
}
int idbSend2HostAsync(void* data, int size, int wait)
{
	return 0;
}
#else
#ifndef CFG_USBD_IDB
int idbSend2Host(void* data, int size, int wait)
{
	return 0;
}
int idbSend2HostAsync(void* data, int size, int wait)
{
	return 0;
}
#endif
#endif




void fifoB_init(struct FIFOB *fifo, int size, int *buf)
{
	fifo->buf = buf;
	fifo->flags = 0;
	fifo->free = size;
	fifo->size = size;
	fifo->putP = 0;
	fifo->getP = 0;

	return;
}

int fifoB_putPut(struct FIFOB *fifo, int data)
{
	if (fifo->free == 0){
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->putP] = data;
	fifo->putP++;

	if (fifo->putP == fifo->size){
		fifo->putP = 0;
	}
	fifo->free--;

	return 0;
}

int fifoB_get(struct FIFOB *fifo)
{
	int data;
	if (fifo->free == fifo->size){
		return -1;
	}
	data = fifo->buf[fifo->getP];
	fifo->getP++;
	if (fifo->getP == fifo->size){
		fifo->getP = 0;
	}
	fifo->free++;

	return data;
}

int fifoB_status(struct FIFOB *fifo)
{
	return fifo->size - fifo->free;
}

int fifoB_free(struct FIFOB *fifo)
{
	return fifo->free;
}

void fifoB_getall(struct FIFOB *fifox, struct FIFOB *fifoy, ITUDrawPen* drawpen)
{
	drawpen->px0 = fifoB_get(fifox);
	drawpen->py0 = fifoB_get(fifoy);
	drawpen->px1 = fifoB_get(fifox);
	drawpen->py1 = fifoB_get(fifoy);
	drawpen->px2 = fifoB_get(fifox);
	drawpen->py2 = fifoB_get(fifoy);
	drawpen->px3 = fifoB_get(fifox);
	drawpen->py3 = fifoB_get(fifoy);
}


static void* USB_SEND_TASK(void* arg)
{
	ITUDrawPen* drawpen = (ITUDrawPen *)arg;

	if (drawpen)
	{
		int *customdata = (int *)ituWidgetGetCustomData(drawpen);

		if (!customdata)
		{
#ifdef SEND_MAX_CURVE_RAW
			memset(ARRAYDATA, 0, sizeof(int)* (MAX_PENCURVE_ARRAY * 2 * SEND_MAX_CURVE_RAW_FACTOR + 4 + 5));
#else
			memset(ARRAYDATA, 0, sizeof(int)* (MAX_PENCURVE_ARRAY * 2 + 4 + 5));
#endif
			ituWidgetSetCustomData(drawpen, &ARRAYDATA);
			customdata = (int *)ituWidgetGetCustomData(drawpen);
		}

		for (;;)
		{
			if (drawpen)
			{
				//sem_t* sem = (sem_t *)drawpen->usb_lock;

				int rc = sem_wait(&sem);

				if (drawpen->usb_working == 0)
				{
					printf("[USB]Job stop!\n");
					break;
				}
				else if (customdata)
				{
					int ret;
					int dsize = customdata[0];
					struct prop* propOut = (struct prop*)customdata;

					if (dsize == 1)
						propOut->id = cpu_to_le32(ID_DRAWPEN_USB_MOUSEDOWN);
					else
						propOut->id = cpu_to_le32(ID_DRAWPEN_USB_CURVE);

					propOut->flags = 0;
					//propOut->size = cpu_to_le32(sizeof(int)* (MAX_PENCURVE_ARRAY * 2 + 4 + 5));
					propOut->size = cpu_to_le32(sizeof(int)* (dsize * 2 + 4 + 5));
					propOut->status = 0;
					ret = idbSend2HostAsync(propOut, le32_to_cpu(propOut->size), 1);

					//printf("[target 0] %d,%d status %d\n", customdata[0 + 9], customdata[1 + 9], ret);
				}
			}
			else
				break;
		}
	}

	return NULL;
}

static void DrawPenOnStop(ITUDrawPen* drawpen)
{
    // DO NOTHING
}

static void pos_surf_draw_circle(ITUSurface *surf, int cx, int cy, int radius, ITUColor* color)
{
	int x, y, xx, yy;
	int savecount = 0;

	for (y = cy - radius; y <= cy + radius; y++)
	{
		for (x = cx - radius; x <= cx + radius; x++)
		{
			xx = x - cx;
			yy = y - cy;

			if ((xx * xx) + (yy * yy) < (radius * radius))
			{
				if ((radius >= 4) && (abs(xx) <= radius / 2) && (abs(yy) <= radius / 2))
				{
					savecount++;
					continue;
				}
				else
				{
					ituColorFill(surf, x, y, 1, 1, color);
				}
			}

			//ituColorFill(surf, x - pressure / 2, y - pressure / 2, pressure, pressure, color);
		}
	}

	
	if (radius >= 4)
	{
		ituColorFill(surf, cx - radius / 2, cy - radius / 2, radius + 1, radius + 1, color);
	}

	//printf("savecount %d\n", savecount);
}


void blitpen(ITUDrawPen* drawpen, int x, int y, int size)
{
	/*if ((drawpen->lastcolor.alpha != drawpen->pencolor.alpha) 
		|| (drawpen->lastcolor.red != drawpen->pencolor.red) 
		|| (drawpen->lastcolor.green != drawpen->pencolor.green) 
		|| (drawpen->lastcolor.blue != drawpen->pencolor.blue)
		|| ((drawpen->pensurf->width != size) || (drawpen->pensurf->height != size))
		)
	{
		if (drawpen->pensurf)
			ituSurfaceRelease(drawpen->pensurf);

		drawpen->pensurf = ituCreateSurface(size, size, 0, ITU_ARGB8888, NULL, 0); //ITU_ARGB8888

		ituColorFill(drawpen->pensurf, 0, 0, size, size, &emptycolor);

		pos_surf_draw_circle(drawpen->pensurf, size / 2, size / 2, size / 2, &drawpen->pencolor);

		drawpen->lastcolor = drawpen->pencolor;
	}*/

	if ((drawpen->pensurf->width != size) || (drawpen->pensurf->height != size))
	{
		drawpen->pensurf = pressure_surf[size];
	}

	//ituAlphaBlend(drawpen->bitsurf, x - (size / 2), y - (size / 2), size, size, drawpen->pensurf, 0, 0, 255);
	ituBitBlt(drawpen->bitsurf, x - (size / 2), y - (size / 2), size, size, drawpen->pensurf, 0, 0);
}


void bzc(int px0, int py0, int px1, int py1, int px2, int py2, int px3, int py3, int* X, int* Y, int size)
{
	int i = 0;
	float t = 0.0;
	float step;

	assert(size);

	step = 1.0 / (float)size;

	for (t = 0.0; t <= 1.0; t += step)
	{
		X[i] = (int)((-t*t*t + 3 * t*t - 3 * t + 1) * px0
			+ (3 * t*t*t - 6 * t*t + 3 * t) * px1
			+ (-3 * t*t*t + 3 * t*t) * px2
			+ (t*t*t) * px3);
		Y[i] = (int)((-t*t*t + 3 * t*t - 3 * t + 1) * py0
			+ (3 * t*t*t - 6 * t*t + 3 * t) * py1
			+ (-3 * t*t*t + 3 * t*t) * py2
			+ (t*t*t) * py3);

		//X++;
		//Y++;
		i++;
	}
}

void check_pressure(ITUDrawPen* drawpen)
{
	if (drawpen->pressure < MIN_PEN_PRESSURE) //minimum pen width
	{
		drawpen->pressure = MIN_PEN_PRESSURE;
	}
	else if (drawpen->pressure > MAX_PEN_PRESSURE)
	{
		drawpen->pressure = MAX_PEN_PRESSURE;
	}
}

void vandermonde_curve(int px0, int py0, int px1, int py1, int px2, int py2, int px3, int py3, int* X, int* Y, int size)
{
	int i = 0;
	float t = 0.0;
	float step;
	float x0 = (float)px0;
	float y0 = (float)py0;
	float x1 = (float)px1;
	float y1 = (float)py1;
	float x2 = (float)px2;
	float y2 = (float)py2;
	float x3 = (float)px3;
	float y3 = (float)py3;

	assert(size);

	step = 1.0 / (float)size;

	//printf("\n");

	for (t = 0.0; t < 1.0; t += step)
	{
		/*
		if (i == 0)
		{
			X[i] = px0;
			Y[i] = py0;
			i++;
			continue;
		}
		else if (i == (size - 1))
		{
			X[i] = px3;
			Y[i] = py3;
			break;
		}*/

		X[i] = (int)((pow(1.0 - t, 3.0) * x0) + (3.0 * t * pow(1.0 - t, 2.0) * x1) + (3.0 * pow(t, 2.0) * (1.0 - t) * x2) + (pow(t, 3.0) * x3));

		Y[i] = (int)((pow(1.0 - t, 3.0) * y0) + (3.0 * t * pow(1.0 - t, 2.0) * y1) + (3.0 * pow(t, 2.0) * (1.0 - t) * y2) + (pow(t, 3.0) * y3));

		/*
		X[i] = (int)(((-4.5 * (t - 0.33333) * (t - 0.66666) * (t - 1.0)) * x0)
			+ ((13.5 * t * (t - 0.66666) * (t - 1.0)) * x1)
			+ ((-13.5 * t * (t - 0.33333) * (t - 1.0)) * x2)
			+ ((4.5 * t * (t - 0.33333) * (t - 0.66666)) * x3));

		Y[i] = (int)(((-4.5 * (t - 0.33333) * (t - 0.66666) * (t - 1.0)) * y0)
			+ ((13.5 * t * (t - 0.66666) * (t - 1.0)) * y1)
			+ ((-13.5 * t * (t - 0.33333) * (t - 1.0)) * y2)
			+ ((4.5 * t * (t - 0.33333) * (t - 0.66666)) * y3));*/

		i++;
	}

	//for (i = 0; i < size; i++)
	//{
	//	printf("T0[%d,%d]T1[%d,%d] [%d,%d]\n", px0, py0, px3, py3, X[i], Y[i]);
	//}
}


bool ituDrawPenUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
	bool result;
	ITUDrawPen* drawpen = (ITUDrawPen*)widget;
    assert(drawpen);

	result = ituWidgetUpdateImpl(widget, ev, arg1, arg2, arg3);

	if (ev == ITU_EVENT_MOUSEDOWN)
	{
		int x = arg2 - widget->rect.x;
		int y = arg3 - widget->rect.y;

		if (ituWidgetIsEnabled(widget))
		{
			if (ituWidgetIsInside(widget, x, y))
			{
				drawpen->pressure = (int)((arg1 * MAX_PEN_PRESSURE) / 2048.0);
				drawpen->pressure_rawdata = arg1;
				check_pressure(drawpen);

				fifoB_init(&fifoX, 1024, BUFFERX);
				fifoB_init(&fifoY, 1024, BUFFERY);

				fifoB_putPut(&fifoX, x);
				fifoB_putPut(&fifoY, y);
				
				drawpen->last_pen_x = 10000;
				drawpen->last_pen_y = 10000;
				drawpen->px0 = x;
				drawpen->py0 = y;
				drawpen->px1 = x;
				drawpen->py1 = y;
				drawpen->px2 = x;
				drawpen->py2 = y;
				drawpen->px3 = x;
				drawpen->py3 = y;
				drawpen->pen_x = x;
				drawpen->pen_y = y;

				drawpen->hasmove = 1;

				if ((drawpen->pen_x > drawpen->pressure) && (drawpen->pen_x < (drawpen->surf->width - drawpen->pressure))
					&& (drawpen->pen_y > drawpen->pressure) && (drawpen->pen_y < (drawpen->surf->height - drawpen->pressure)))
				{
					blitpen(drawpen, drawpen->pen_x, drawpen->pen_y, drawpen->pressure);

					if (drawpen->usb_working == 1)
					{
						int *customdata = (int *)ituWidgetGetCustomData(drawpen);

						if (customdata)
						{
							memset(customdata, 0, sizeof(int)* (MAX_PENCURVE_ARRAY * 2 + 4 + 5));

							customdata[0] = 1;
							customdata[4] = drawpen->pressure_rawdata;
							customdata[5] = drawpen->pencolor.alpha;
							customdata[6] = drawpen->pencolor.red;
							customdata[7] = drawpen->pencolor.green;
							customdata[8] = drawpen->pencolor.blue;
							customdata[9] = drawpen->pen_x;
							customdata[10] = drawpen->pen_y;
#ifdef DRAWPEN_USB_DEVICE_DEBUG
							printf("\n");
							printf("[DrawPen][USB][Mouse Down] %d\n", customdata[0]);
							printf("[DrawPen][USB][Pressure] %d\n", customdata[4]);
							printf("[DrawPen][USB][ColorARGB] %d,%d,%d,%d\n", customdata[5], customdata[6], customdata[7], customdata[8]);
							printf("[DrawPen][USB][x,y] %d,%d\n", customdata[9], customdata[10]);
							printf("\n");
#endif
							drawpen->usb_ready_to_rw = 1;
						}
					}
				}

				drawpen->drawpenFlags |= ITU_DRAWPEN_WORKING;
				result = true;
				result |= ituExecActions((ITUWidget*)drawpen, drawpen->actions, ev, 0);

				//printf("[DrawPen]PenWidth => %d / %d\n", drawpen->pressure, MAX_PEN_PRESSURE);
			}
		}
	}
	else if (ev == ITU_EVENT_MOUSEMOVE)
	{
		int x = arg2 - widget->rect.x;
		int y = arg3 - widget->rect.y;

		if (ituWidgetIsEnabled(widget))
		{
			if (ituWidgetIsInside(widget, x, y))
			{
				//if ((drawpen->drawpenFlags & ITU_DRAWPEN_WORKING) && (drawpen->hasmove <= 1))
				if ((drawpen->drawpenFlags & ITU_DRAWPEN_WORKING))
				{
					int newp = (int)((((float)arg1) * MAX_PEN_PRESSURE) / 2048.0);
					if (newp != drawpen->pressure)
					{
						//drawpen->pressure = (int)((arg1 * MAX_PEN_PRESSURE) / 2048.0);
						drawpen->pressure = newp;
						drawpen->pressure_rawdata = arg1;
						check_pressure(drawpen);
					}
				}

				drawpen->last_pen_x = drawpen->pen_x;
				drawpen->last_pen_y = drawpen->pen_y;
				drawpen->pen_x = x;
				drawpen->pen_y = y;

				//if ((drawpen->hasmove < 4) && (drawpen->drawpenFlags & ITU_DRAWPEN_WORKING))
				if (drawpen->drawpenFlags & ITU_DRAWPEN_WORKING)
				{
					fifoB_putPut(&fifoX, x);
					fifoB_putPut(&fifoY, y);
					drawpen->hasmove++;

					drawpen->px0 = drawpen->px1;
					drawpen->py0 = drawpen->py1;
					drawpen->px1 = drawpen->px2;
					drawpen->py1 = drawpen->py2;
					drawpen->px2 = drawpen->px3;
					drawpen->py2 = drawpen->py3;
					drawpen->px3 = x;
					drawpen->py3 = y;

					if ((drawpen->hasmove % 4) == 0)
					{
						fifoB_putPut(&fifoX, x);
						fifoB_putPut(&fifoY, y);
						drawpen->hasmove++;
					}
					//printf("mmm %d,%d\n", x, y);
				}

				//if (drawpen->drawpenFlags & ITU_DRAWPEN_WORKING)
				//result = true;

				//if (drawpen->hasmove >= 4)
				//{
				//	widget->dirty = 1;
				//}
			}
		}
	}
	else if (ev == ITU_EVENT_MOUSEUP)
	{
		int x = arg2 - widget->rect.x;
		int y = arg3 - widget->rect.y;

		if ((drawpen->usb_working == 1) && (drawpen->drawpenFlags & ITU_DRAWPEN_WORKING))
		{
			int customdata[11] = { 0 };
			int dsize = 1;
			struct prop* propOut = (struct prop*)customdata;

			customdata[0] = 1;
			customdata[4] = 5; //float value?
			customdata[5] = drawpen->pencolor.alpha;
			customdata[6] = drawpen->pencolor.red;
			customdata[7] = drawpen->pencolor.green;
			customdata[8] = drawpen->pencolor.blue;
			customdata[9] = x;
			customdata[10] = y;

			propOut->id = cpu_to_le32(ID_DRAWPEN_USB_MOUSEUP);
			propOut->flags = 0;
			propOut->size = cpu_to_le32(sizeof(int)* (dsize * 2 + 4 + 5));
			propOut->status = 0;
#ifdef DRAWPEN_USB_DEVICE_DEBUG
			printf("\n");
			printf("[DrawPen][USB][Mouse UP] %d\n", dsize);
			printf("[DrawPen][USB][Pressure] %d\n", customdata[4]);
			printf("[DrawPen][USB][ColorARGB] %d,%d,%d,%d\n", customdata[5], customdata[6], customdata[7], customdata[8]);
			printf("[DrawPen][USB][x,y] %d,%d\n", customdata[9], customdata[10]);
			printf("\n");
#endif
			idbSend2HostAsync(propOut, le32_to_cpu(propOut->size), 1);
		}

		drawpen->drawpenFlags &= ~ITU_DRAWPEN_WORKING;
		//drawpen->pressure = MIN_PEN_PRESSURE;
		//drawpen->hasmove = 0;
	}
    else if (ev == ITU_EVENT_TIMER)
    {
		/*
		if ((drawpen->hasmove > 0) && (drawpen->hasmove < 4))
		{
			drawpen->px0 = drawpen->px1;
			drawpen->py0 = drawpen->py1;
			drawpen->px1 = drawpen->px2;
			drawpen->py1 = drawpen->py2;
			drawpen->px2 = drawpen->px3;
			drawpen->py2 = drawpen->py3;
			drawpen->px3 = drawpen->pen_x;
			drawpen->py3 = drawpen->pen_y;

			drawpen->hasmove++;
		}*/

		/*
		if ((drawpen->drawpenFlags & ITU_DRAWPEN_WORKING))
		{
			if ((drawpen->last_pen_x == drawpen->pen_x) && (drawpen->last_pen_y == drawpen->pen_y))
			{
				fifoB_putPut(&fifoX, drawpen->pen_x);
				fifoB_putPut(&fifoY, drawpen->pen_y);
				drawpen->hasmove++;
			}
		}*/
	    
		widget->dirty = 1;
		result = true;
    }
    else if (ev == ITU_EVENT_LAYOUT)
    {
		//printf("[LAYOUT]\n");
		widget->dirty = true;
    }

	return widget->visible ? result : false;
}

void ituDrawPenDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
	int destx, desty;
	uint8_t desta;
	ITUDrawPen* drawpen = (ITUDrawPen*)widget;
	ITURectangle* rect = &widget->rect;
	ITCTree* node;
	//ITURectangle prevClip;
	assert(drawpen);
	assert(dest);

	destx = rect->x + x;
	desty = rect->y + y;
	desta = alpha * widget->alpha / 255;

	for (node = widget->tree.child; node; node = node->sibling)
	{
		ITUWidget* child = (ITUWidget*)node;

		if (child->visible && ituWidgetIsOverlapClipping(child, dest, destx, desty))
			ituWidgetDraw(node, dest, destx, desty, desta);

		child->dirty = false;
	}

	check_pressure(drawpen);
	
	if (drawpen->bitsurf == NULL)
	{
		drawpen->bitsurf = ituCreateSurface(drawpen->widget.rect.width, drawpen->widget.rect.height, 0, ITU_RGB565, NULL, 0); //ITU_ARGB8888
		ituBitBlt(drawpen->bitsurf, 0, 0, drawpen->widget.rect.width, drawpen->widget.rect.height, drawpen->screensurf, destx, desty);
	}

	if ((drawpen->screensurf) && (drawpen->bitsurf))
	{
		int pressure = drawpen->pressure;
		int radious = drawpen->pressure / 2;
		int i;
		int dd;
		double dx, dy;
		double dt;
		double diff;
		double Fa;
#ifdef SEND_MAX_CURVE_RAW
		int dd2 = SEND_MAX_CURVE_RAW_FACTOR;
#else
		int dd2 = 1;
#endif

		//if ((drawpen->drawpenFlags & ITU_DRAWPEN_WORKING) && (drawpen->hasmove >= 4))
		if (drawpen->hasmove >= 4)
		{
			bool point_mode = false;

			fifoB_getall(&fifoX, &fifoY, drawpen);

			if (point_mode)
			{
				diff = sqrt((drawpen->pen_x - drawpen->last_pen_x)*(drawpen->pen_x - drawpen->last_pen_x)
					+ (drawpen->pen_y - drawpen->last_pen_y)*(drawpen->pen_y - drawpen->last_pen_y));
			}
			else
			{
				diff = sqrt((drawpen->px3 - drawpen->px0)*(drawpen->px3 - drawpen->px0)
					+ (drawpen->py3 - drawpen->py0)*(drawpen->py3 - drawpen->py0));
			}

			if (point_mode)
			{
				if ((drawpen->pen_x == drawpen->last_pen_x) || (drawpen->pen_y == drawpen->last_pen_y))
					Fa = 0.0;
				else
					Fa = fabs((double)(drawpen->pen_y - drawpen->last_pen_y) / (double)(drawpen->pen_x - drawpen->last_pen_x));
			}


			//if ((drawpen->pen_x > pressure) && (drawpen->pen_x < (drawpen->surf->width - pressure))
			//	&& (drawpen->pen_y > pressure) && (drawpen->pen_y < (drawpen->surf->height - pressure)))
			if (1)
			{
				dd = (int)(diff / ((double)pressure / (double)SMOOTH_FACTOR));

				if (point_mode) //fit point mode
				{
					if ((diff > pressure) && (drawpen->last_pen_x < 10000) && (drawpen->last_pen_y < 10000))
					{
						if (Fa != 0.0)
						{
							for (i = 0; i < dd; i++)
							{
								dt = (double)(pressure / SMOOTH_FACTOR) * (double)(i + 0.5);

								dx = dt / sqrt(1 + Fa * Fa);
								dy = dx * Fa;

								dx *= (drawpen->pen_x >= drawpen->last_pen_x) ? (1) : (-1);
								dy *= (drawpen->pen_y >= drawpen->last_pen_y) ? (1) : (-1);

								//pos_surf_draw_circle(drawpen->surf, drawpen->last_pen_x + (int)dx, drawpen->last_pen_y + (int)dy, radious, &drawpen->pencolor);
								blitpen(drawpen, drawpen->last_pen_x + (int)dx, drawpen->last_pen_y + (int)dy, pressure);
							}
						}
						else
						{
							for (i = 0; i < dd; i++)
							{
								dt = (double)(pressure / SMOOTH_FACTOR) * (double)(i + 0.5);

								if (drawpen->pen_x == drawpen->last_pen_x)
								{
									dx = 0.0;
									dy = dt * ((drawpen->pen_y >= drawpen->last_pen_y) ? (1) : (-1));
								}
								else
								{
									dy = 0.0;
									dx = dt * ((drawpen->pen_x >= drawpen->last_pen_x) ? (1) : (-1));
								}

								//pos_surf_draw_circle(drawpen->surf, drawpen->last_pen_x + (int)dx, drawpen->last_pen_y + (int)dy, radious, &drawpen->pencolor);
								blitpen(drawpen, drawpen->last_pen_x + (int)dx, drawpen->last_pen_y + (int)dy, pressure);
							}
						}
					}
					else
					{
						//pos_surf_draw_circle(drawpen->surf, drawpen->pen_x, drawpen->pen_y, radious, &drawpen->pencolor);
						blitpen(drawpen, drawpen->pen_x, drawpen->pen_y, pressure);
					}
				}
				else if (drawpen->hasmove >= 4) //fit curve mode
				{
					int *customdata = (int *)ituWidgetGetCustomData(drawpen);

					//fifoB_getall(&fifoX, &fifoY, drawpen);
					drawpen->hasmove -= 4;
					//printf("[CHECK %d] P1(%d,%d) P2(%d,%d)\n", drawpen->hasmove, drawpen->px0, drawpen->py0, drawpen->px3, drawpen->py3);

					if (dd < 4)
					{
						dd = 4;
					}
					else if (dd >= MAX_PENCURVE_ARRAY)
					{
						dd = MAX_PENCURVE_ARRAY - 1;
						printf("[DrawPen] MAX_PENCURVE_ARRAY %d limited!\n", MAX_PENCURVE_ARRAY);
					}

					
#ifdef SEND_MAX_CURVE_RAW
					memset(ARRAYX2, 0, MAX_PENCURVE_ARRAY * SEND_MAX_CURVE_RAW_FACTOR);
					memset(ARRAYY2, 0, MAX_PENCURVE_ARRAY * SEND_MAX_CURVE_RAW_FACTOR);
					vandermonde_curve(drawpen->px0, drawpen->py0, drawpen->px1, drawpen->py1, drawpen->px2, drawpen->py2, drawpen->px3, drawpen->py3, &ARRAYX2[0], &ARRAYY2[0], dd * SEND_MAX_CURVE_RAW_FACTOR);
#else
					memset(ARRAYX, 0, MAX_PENCURVE_ARRAY);
					memset(ARRAYY, 0, MAX_PENCURVE_ARRAY);
					vandermonde_curve(drawpen->px0, drawpen->py0, drawpen->px1, drawpen->py1, drawpen->px2, drawpen->py2, drawpen->px3, drawpen->py3, &ARRAYX[0], &ARRAYY[0], dd);
#endif

					//bzc(drawpen->px0, drawpen->py0, drawpen->px1, drawpen->py1, drawpen->px2, drawpen->py2, drawpen->px3, drawpen->py3, X, Y, dd);
					
					//printf("\n\n=====p1,p2,p3,p4=====\n");
					//printf("[%d,%d]\n", drawpen->px0, drawpen->py0);
					//printf("[%d,%d]\n", drawpen->px1, drawpen->py1);
					//printf("[%d,%d]\n", drawpen->px2, drawpen->py2);
					//printf("[%d,%d]\n", drawpen->px3, drawpen->py3);

					//drawpen->px3 = drawpen->pen_x;
					//drawpen->py3 = drawpen->pen_y;

					for (i = 0; i < (dd * dd2); i++)
					{
						//pos_surf_draw_circle(drawpen->surf, X[i], Y[i], radious, &drawpen->pencolor);
#ifdef SEND_MAX_CURVE_RAW
						if (((i + 1) >= (dd * dd2)) || ((i % SEND_MAX_CURVE_RAW_FACTOR) == 0))
							blitpen(drawpen, ARRAYX2[i], ARRAYY2[i], pressure);
#else
						blitpen(drawpen, ARRAYX[i], ARRAYY[i], pressure);
#endif

						if ((drawpen->usb_working == 1) && customdata)
						{
							if (i == 0)
							{
								memset(customdata, 0, sizeof(int)* (MAX_PENCURVE_ARRAY * 2 * dd2 + 4 + 5));

								customdata[0] = dd * dd2;

								customdata[4] = drawpen->pressure_rawdata;
								customdata[5] = drawpen->pencolor.alpha;
								customdata[6] = drawpen->pencolor.red;
								customdata[7] = drawpen->pencolor.green;
								customdata[8] = drawpen->pencolor.blue;
							}
#ifdef SEND_MAX_CURVE_RAW
							customdata[i * 2 + 9] = ARRAYX2[i];
							customdata[(i * 2) + 9 + 1] = ARRAYY2[i];
#else
							customdata[i * 2 + 9] = ARRAYX[i];
							customdata[(i * 2) + 9 + 1] = ARRAYY[i];
#endif
						}

						if ((i == (dd * dd2 - 1)) && (drawpen->usb_working == 1))
						{
#ifdef DRAWPEN_USB_DEVICE_DEBUG
							printf("\n");
							printf("[DrawPen][USB][ARR size] %d\n", customdata[0]);
							printf("[DrawPen][USB][Pressure] %d\n", customdata[4]);
							printf("[DrawPen][USB][ColorARGB] %d,%d,%d,%d\n", customdata[5], customdata[6], customdata[7], customdata[8]);
							printf("[DrawPen][USB][P1] %d,%d\n", customdata[9], customdata[10]);
							printf("[DrawPen][USB][P2] %d,%d\n", customdata[11], customdata[12]);
							printf("[DrawPen][USB][P3] %d,%d\n", customdata[13], customdata[14]);
							printf("\n");
#endif
							drawpen->usb_ready_to_rw = 1;

							//sem_post(&sem);
						}
					}

					//printf("[dd][%d]\n", dd);
				}
				else
				{
					//pos_surf_draw_circle(drawpen->surf, drawpen->pen_x, drawpen->pen_y, radious, &drawpen->pencolor);
					//blitpen(drawpen, drawpen->pen_x, drawpen->pen_y, pressure);
				}
			}
		}


		//main
		//ituBitBlt(drawpen->surf, 0, 0, drawpen->surf->width, drawpen->surf->height, drawpen->bitsurf, 0, 0);


		ituBitBlt(drawpen->screensurf, destx, desty, drawpen->bitsurf->width, drawpen->bitsurf->height, drawpen->bitsurf, 0, 0);

		if (drawpen->usb_working == 1 && (drawpen->usb_ready_to_rw == 1))
		{
			drawpen->usb_ready_to_rw = 0;
			sem_post(&sem);
		}

		if (!(drawpen->drawpenFlags & ITU_DRAWPEN_WORKING))
		{
			if ((drawpen->hasmove > 0) && (drawpen->hasmove < 4))
			{
				drawpen->hasmove = 0;
			}

			ituAlphaBlend(drawpen->screensurf, drawpen->pen_x + destx - PENCURSOR_HEADSIZE / 2, drawpen->pen_y + desty - PENCURSOR_HEADSIZE / 2, PENCURSOR_HEADSIZE, PENCURSOR_HEADSIZE, drawpen->cursorsurf, 0, 0, 128);
		}
		else if (drawpen->hasmove < 4)
			widget->dirty = 1;
	}

}

void ituDrawPenOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
	ITUDrawPen* drawpen = (ITUDrawPen*)widget;
    assert(drawpen);

    switch (action)
    {
    case ITU_ACTION_PLAY:
        if (widget->flags & ITU_ENABLED)
			ituDrawPenPlay(drawpen);
        break;

	case ITU_ACTION_BIND:
		if (widget->flags & ITU_ENABLED)
		{ //ColorPicker use BIND to change the color of DrawPen
			if (param)
			{
				unsigned int value = atoi(param);

				if (value >= 0)
				{
					drawpen->pencolor.alpha = (value & 0xFF000000) >> 24;
					drawpen->pencolor.red = (value & 0x00FF0000) >> 16;
					drawpen->pencolor.green = (value & 0x0000FF00) >> 8;
					drawpen->pencolor.blue = (value & 0x000000FF);

					printf("[DrawPen] %s Color[A,R,G,B][%d,%d,%d,%d]\n", widget->name, drawpen->pencolor.alpha, drawpen->pencolor.red, drawpen->pencolor.green, drawpen->pencolor.blue);
				}
			}
		}
		break;

	case ITU_ACTION_GOTO:
		if (widget->flags & ITU_ENABLED)
		{
			if (param)
			{
				int pressure = atoi(param);

				if (pressure >= 0)
					drawpen->pressure = pressure;
			}
		}
		break;

    case ITU_ACTION_STOP:
        if (widget->flags & ITU_ENABLED)
        {
			ituDrawPenStop(drawpen);
			ituDrawPenOnStop(drawpen);
            ituExecActions(widget, drawpen->actions, ITU_EVENT_STOPPED, 0);
        }
        break;

	case ITU_ACTION_CLEAR:
		if (widget->flags & ITU_ENABLED)
		{
			ituDrawPenStop(drawpen);
			ituDrawPenOnStop(drawpen);

			if (drawpen->bitsurf)
			{
				ituSurfaceRelease(drawpen->bitsurf);
				drawpen->bitsurf = NULL;
				printf("[DrawPen] %s clear!\n", widget->name);
			}
		}
		break;

    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

void ituDrawPenInit(ITUDrawPen* drawpen)
{
    assert(drawpen);
    ITU_ASSERT_THREAD();

	memset(drawpen, 0, sizeof (ITUDrawPen));

    ituWidgetInit(&drawpen->widget);

	ituWidgetSetType(drawpen, ITU_DRAWPEN);
	ituWidgetSetName(drawpen, drawpenName);
	ituWidgetSetUpdate(drawpen, ituDrawPenUpdate);
	ituWidgetSetDraw(drawpen, ituDrawPenDraw);
	ituWidgetSetOnAction(drawpen, ituDrawPenOnAction);
	ituDrawPenSetOnStop(drawpen, DrawPenOnStop);
}

void ituDrawPenLoad(ITUDrawPen* drawpen, uint32_t base)
{
	assert(drawpen);

	ituWidgetLoad((ITUWidget*)drawpen, base);
	ituWidgetSetUpdate(drawpen, ituDrawPenUpdate);
	ituWidgetSetDraw(drawpen, ituDrawPenDraw);
	ituWidgetSetOnAction(drawpen, ituDrawPenOnAction);
	ituDrawPenSetOnStop(drawpen, DrawPenOnStop);

	emptycolor.alpha = 0;
	emptycolor.red = 0;
	emptycolor.green = 0;
	emptycolor.blue = 0;

	if (drawpen->screensurf == NULL)
	{
		int *customdata = (int *)ituWidgetGetCustomData(drawpen);

		if (!customdata)
		{
			//int *newdata = malloc(sizeof(int)* (MAX_PENCURVE_ARRAY * 2 + 4 + 5));
#ifdef SEND_MAX_CURVE_RAW
			memset(ARRAYDATA, 0, sizeof(int)* (MAX_PENCURVE_ARRAY * 2 * SEND_MAX_CURVE_RAW_FACTOR + 4 + 5));
#else
			memset(ARRAYDATA, 0, sizeof(int)* (MAX_PENCURVE_ARRAY * 2 + 4 + 5));
#endif
			ituWidgetSetCustomData(drawpen, &ARRAYDATA);
		}

		drawpen->screensurf = ituGetDisplaySurface();

		drawpen->last_pen_x = 10000;
		drawpen->last_pen_y = 10000;
		drawpen->px0 = 10000;		drawpen->py0 = 10000;
		drawpen->px1 = 10000;		drawpen->py1 = 10000;
		drawpen->px2 = 10000;		drawpen->py2 = 10000;
		drawpen->px3 = 10000;		drawpen->py3 = 10000;
	}

	if (drawpen->screensurf)
	{
		if (drawpen->surf == NULL)
		{
			drawpen->surf = ituCreateSurface(drawpen->widget.rect.width, drawpen->widget.rect.height, 0, ITU_ARGB8888, NULL, 0);

			ituColorFill(drawpen->surf, 0, 0, drawpen->widget.rect.width, drawpen->widget.rect.height, &emptycolor);
		}

		if (drawpen->pensurf == NULL)
		{
			//drawpen->pensurf = ituCreateSurface(100, 100, 0, ITU_ARGB8888, NULL, 0);
			//ituColorFill(drawpen->pensurf, 0, 0, 100, 100, &emptycolor);

			int i = 0;

			for (i = MIN_PEN_PRESSURE; i <= MAX_PEN_PRESSURE; i++)
			{
				pressure_surf[i] = ituCreateSurface(i, i, 0, ITU_ARGB8888, NULL, 0); //ITU_ARGB8888

				ituColorFill(pressure_surf[i], 0, 0, i, i, &emptycolor);

				pos_surf_draw_circle(pressure_surf[i], i / 2, i / 2, i / 2, &drawpen->pencolor);
			}

			drawpen->pensurf = pressure_surf[MIN_PEN_PRESSURE];

			drawpen->lastcolor = drawpen->pencolor;
		}

		if (drawpen->cursorsurf == NULL)
		{
			drawpen->cursorsurf = ituCreateSurface(PENCURSOR_HEADSIZE, PENCURSOR_HEADSIZE, 0, ITU_ARGB8888, NULL, 0);
			ituColorFill(drawpen->cursorsurf, 0, 0, PENCURSOR_HEADSIZE, PENCURSOR_HEADSIZE, &drawpen->pencolor);
		}
	}
}

void ituDrawPenPlay(ITUDrawPen* drawpen)
{
	assert(drawpen);
    ITU_ASSERT_THREAD();

	if ((drawpen->widget.flags & ITU_ENABLED) == 0)
        return;
}

void ituDrawPenStop(ITUDrawPen* drawpen)
{
	assert(drawpen);
    ITU_ASSERT_THREAD();

	if ((drawpen->widget.flags & ITU_ENABLED) == 0)
        return;

	drawpen->drawpenFlags &= ~ITU_DRAWPEN_WORKING;
	drawpen->pressure = MIN_PEN_PRESSURE;
	drawpen->hasmove = 0;

	if (drawpen->bitsurf)
	{
		ituSurfaceRelease(drawpen->bitsurf);
		drawpen->bitsurf = NULL;
		drawpen->widget.dirty = 1;
	}

    //ituScene->playingAudio = NULL;
}

bool ituDrawPenDumpJpeg(ITUDrawPen* drawpen, int percent, char* filename)
{
	bool result = false;
	assert(drawpen);
    ITU_ASSERT_THREAD();

	if ((filename != NULL) && (drawpen->surf != NULL))
	{
		FILE* outfile = fopen(filename, "wb");

		if (outfile != NULL)
		{
			fclose(outfile);
			result = true;
		}
	}

	return result;
}

void ituDrawPenSetPenColor(ITUDrawPen* drawpen, ITUColor color)
{
	int i = 0;

	assert(drawpen);
    ITU_ASSERT_THREAD();

	drawpen->pencolor.alpha = color.alpha;
	drawpen->pencolor.red = color.red;
	drawpen->pencolor.green = color.green;
	drawpen->pencolor.blue = color.blue;

	for (i = MIN_PEN_PRESSURE; i <= MAX_PEN_PRESSURE; i++)
	{
		if (pressure_surf[i])
			ituSurfaceRelease(pressure_surf[i]);

		pressure_surf[i] = ituCreateSurface(i, i, 0, ITU_ARGB8888, NULL, 0); //ITU_ARGB8888

		ituColorFill(pressure_surf[i], 0, 0, i, i, &emptycolor);

		pos_surf_draw_circle(pressure_surf[i], i / 2, i / 2, i / 2, &drawpen->pencolor);
		drawpen->pensurf = pressure_surf[drawpen->pressure];
	}
}

void ituDrawPenUSBTask(ITUDrawPen* drawpen, bool usbtask)
{
	if (!usbtask && (drawpen->usb_working == 1))
	{
		//sem_t * sem = (sem_t *)drawpen->usb_lock;
		drawpen->usb_working = 0;
		fsg_connected = 0;
		sem_post(&sem);
		pthread_join(task, NULL);
	}
	else if (usbtask && (drawpen->usb_working == 0))
	{
		//pthread_mutex_t usbjob;
		//pthread_t task;
		//sem_t sem;
		
		pthread_attr_t attr;
		
		drawpen->usb_working = 1;
		sem_init(&sem, 0, 0);
		//drawpen->usb_lock = (unsigned int *)(&sem);

		if (ioctl(ITP_DEVICE_USBDFSG, ITP_IOCTL_IS_CONNECTED, NULL))
		{
			printf("\n\nusb fsg enter.... \n");
			fsg_connected = 1;
			//idbSetCallback(idbCallback);
		}
		else
		{
			fsg_connected = 0;
			drawpen->usb_working = 0;
		}

		if ((fsg_connected == 1) || (drawpen->usb_working == 1))
		{
			pthread_attr_init(&attr);
			pthread_attr_setstacksize(&attr, sizeof(int)* MAX_PENCURVE_ARRAY * 2 * 2);
			pthread_create(&task, &attr, USB_SEND_TASK, drawpen);
			//drawpen->usb_thread = (unsigned long *)(&task);

			printf("[USB]Job start!\n");
		}
		else
		{
			printf("[USB]Connect fail!\n");
		}
	}
}

void ituDrawPenUSBSendID(ITUDrawPen* drawpen, unsigned int id)
{
	assert(drawpen);
    ITU_ASSERT_THREAD();

	if (fsg_connected == 1)
	{
		int customdata[4] = { 0 };
		struct prop* propOut = (struct prop*)customdata;
		propOut->id = cpu_to_le32(id);
		propOut->flags = 0;
		propOut->size = cpu_to_le32(sizeof(int) * 4);
		propOut->status = 0;
		idbSend2HostAsync(propOut, le32_to_cpu(propOut->size), 1);
#ifdef DRAWPEN_USB_DEVICE_DEBUG
		printf("[DrawPen][USB][SendID][0x%8X]\n", id);
#endif
	}
}

void ituDrawPenCursorSwitch(ITUDrawPen* drawpen, bool switchon)
{
    ITU_ASSERT_THREAD();

	if ((drawpen) && (drawpen->cursorsurf))
	{
		if (switchon)
			ituColorFill(drawpen->cursorsurf, 0, 0, PENCURSOR_HEADSIZE, PENCURSOR_HEADSIZE, &drawpen->pencolor);
		else
			ituColorFill(drawpen->cursorsurf, 0, 0, PENCURSOR_HEADSIZE, PENCURSOR_HEADSIZE, &emptycolor);
	}
}


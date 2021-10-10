/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL STN LCD functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include "itp_cfg.h"
#include "itp_font.h"
#include <stdlib.h>
#include <string.h>

static uint8_t stnLcdBuffer[CFG_STNLCD_WIDTH * CFG_STNLCD_HEIGHT / 8];

static StnLcdGetInfo(ITPStnLcdInfo* info)
{
    memset(info, 0, sizeof (ITPStnLcdInfo));

    info->width     = CFG_STNLCD_WIDTH;
    info->height    = CFG_STNLCD_HEIGHT;
    info->pitch     = (CFG_STNLCD_WIDTH + 7) / 8;
    info->buf       = stnLcdBuffer;
}

static int StnLcdIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_CLEAR:
        memset(stnLcdBuffer, 0, sizeof (stnLcdBuffer));
        break;

    case ITP_IOCTL_FLUSH:
        itpStnLcdWriteBuffer(stnLcdBuffer);
        break;

    case ITP_IOCTL_GET_INFO:
        StnLcdGetInfo((ITPStnLcdInfo*)ptr);
        break;

    case ITP_IOCTL_INIT:
        itpStnLcdInit();
        break;

    case ITP_IOCTL_ENABLE:
        itpStnLcdEnable();
        break;

    case ITP_IOCTL_DISABLE:
        itpStnLcdDisable();
        break;

    default:
        errno = (ITP_DEVICE_STNLCD << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceStnLcd =
{
    ":stnlcd",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    itpWriteDefault,
    itpLseekDefault,
    StnLcdIoctl,
    NULL
};

void itpStnLcdDrawPixel(int x, int y, unsigned int color)
{
    if ((x < 0) || (y < 0) || (x >= CFG_STNLCD_WIDTH) || (y >= CFG_STNLCD_HEIGHT))
        return;

    // x is which column
    if (color) 
        stnLcdBuffer[x + (y / 8) * CFG_STNLCD_WIDTH] |= 1u << (7 - (y % 8));
    else
        stnLcdBuffer[x + (y / 8) * CFG_STNLCD_WIDTH] &= ~(1u << (7 - (y % 8)));
}

void itpStnLcdDrawBitmap(int x, int y, const uint8_t* bitmap, unsigned int w, unsigned int h)
{
    unsigned int i, j;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            if (bitmap[(i / 8) + j * (w / 8)] & (1u << (i % 8)))
	            itpStnLcdDrawPixel(x + i, y + j, 1);
            else
                itpStnLcdDrawPixel(x + i, y + j, 0);
        }
    }
}

void itpStnLcdDrawChar(int x, int y, char c)
{
    if (c >= ITP_FONT_CHARS)
        return;

    itpStnLcdDrawBitmap(x, y, &itpFontData[c * ITP_FONT_HEIGHT], ITP_FONT_WIDTH, ITP_FONT_HEIGHT);
}

void itpStnLcdDrawText(int x, int y, const char* text)
{
    int i, len = strlen(text);
    for (i = 0; i < len; i++)
    {
        itpStnLcdDrawChar(x, y, text[i]);
        x += ITP_FONT_WIDTH;
    }
}

#define swap(a, b) { uint8_t t = a; a = b; b = t; }

void itpStnLcdDrawLine(int x0, int y0, int x1, int y1, unsigned int color)
{
    int dx, dy, steep, err, ystep;

    steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep)
    {
        swap(x0, y0);
        swap(x1, y1);
    }

    if (x0 > x1)
    {
        swap(x0, x1);
        swap(y0, y1);
    }

    dx = x1 - x0;
    dy = abs(y1 - y0);

    err = dx / 2;

    if (y0 < y1)
    {
        ystep = 1;
    }
    else
    {
        ystep = -1;
    }

    for (; x0 < x1; x0++)
    {
        if (steep)
        {
            itpStnLcdDrawPixel(y0, x0, color);
        }
        else
        {
            itpStnLcdDrawPixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0)
        {
            y0 += ystep;
            err += dx;
        }
    }
}

void itpStnLcdDrawRect(int x, int y, unsigned int w, unsigned int h, unsigned int color)
{
    unsigned int i;

    for (i = x; i < x + w; i++)
    {
        itpStnLcdDrawPixel(i, y, color);
        itpStnLcdDrawPixel(i, y + h - 1, color);
    }
  
    for (i = y; i < y + h; i++)
    {
        itpStnLcdDrawPixel(x, i, color);
        itpStnLcdDrawPixel(x + w - 1, i, color);
    } 
}

void itpStnLcdFillRect(int x, int y, unsigned int w, unsigned int h, unsigned int color)
{
    unsigned int i, j;
  
    for (i = x; i < x + w; i++)
    {
        for (j = y; j < y + h; j++)
        {
            itpStnLcdDrawPixel(i, j, color);
        }
    }
}

/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL LCD OSD console functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include <string.h>
#include "itp_cfg.h"
#include "itp_font.h"

static int lcd_line_length;

static uint32_t osd_addr;
static unsigned char *osd_base;                /* Start of framebuffer memory    */
static short console_col;
static short console_row;

#define CONSOLE_ROWS		(ithLcdGetHeight() / ITP_FONT_HEIGHT)
#define CONSOLE_COLS		(ithLcdGetWidth() / ITP_FONT_WIDTH)
#define CONSOLE_ROW_SIZE	(ITP_FONT_HEIGHT * lcd_line_length)
#define CONSOLE_SIZE		(CONSOLE_ROW_SIZE * CONSOLE_ROWS)
#define CONSOLE_SCROLL_SIZE	(CONSOLE_SIZE - CONSOLE_ROW_SIZE)

static void lcd_drawchars (unsigned short x, unsigned short y, unsigned char *str, int count)
{
	unsigned char *dest;
	unsigned short off, row;

	dest = (unsigned char *)(osd_base + y * lcd_line_length + x * 2 / 8);
	off  = x * 2 % 8;

	for (row=0;  row < ITP_FONT_HEIGHT;  ++row, dest += lcd_line_length)  {
		unsigned char *s = str;
		int i, j;
		unsigned char *d = dest;
		unsigned char rest = *d & -(1 << (8-off));
		unsigned char sym;

		for (i=0; i<count; ++i) {
			unsigned char c, bits;
		
			c = *s++;
			bits = itpFontData[c * ITP_FONT_HEIGHT + row];

            for (j = 0; j < 2; j++)
            {
                sym = 0;
    			for (c=0; c<4; ++c) {
                    sym >>= 2;
                    sym |= (bits & 0x80) ? 0x40 : 0x80;
                    bits <<= 1;
                }
                *d++ = rest | (sym >> off);
                rest = sym << (8-off);
            }
		}
		*d  = rest | (*d & ((1 << (8-off)) - 1));
	}
}

static inline void lcd_putc_xy (unsigned short x, unsigned short y, unsigned char c)
{
	lcd_drawchars (x, y, &c, 1);
}

static void console_scrollup (void)
{
	/* Copy up rows ignoring the first one */
	memcpy (osd_base, osd_base + CONSOLE_ROW_SIZE, CONSOLE_SCROLL_SIZE);

	/* Clear the last one */
	memset (osd_base + CONSOLE_SIZE - CONSOLE_ROW_SIZE, 0xAA, CONSOLE_ROW_SIZE);
}

static inline void console_back (void)
{
	if (--console_col < 0) {
		console_col = CONSOLE_COLS-1 ;
		if (--console_row < 0) {
			console_row = 0;
		}
	}

	lcd_putc_xy (console_col * ITP_FONT_WIDTH,
		     console_row * ITP_FONT_HEIGHT,
		     ' ');
}

static inline void console_newline (void)
{
	++console_row;
	console_col = 0;

	/* Check if we need to scroll the terminal */
	if (console_row >= CONSOLE_ROWS) {
		/* Scroll everything up */
		console_scrollup () ;
		--console_row;
	}
}

static void lcd_putc (const char c)
{
	switch (c) {
	case '\r':	console_col = 0;
			return;

	case '\n':	console_newline();
			return;

	case '\t':	/* Tab (8 chars alignment) */
			console_col +=  8;
			console_col &= ~7;

			if (console_col >= CONSOLE_COLS) {
				console_newline();
			}
			return;

	case '\b':	console_back();
			return;

	default:	lcd_putc_xy (console_col * ITP_FONT_WIDTH,
				     console_row * ITP_FONT_HEIGHT,
				     c);
			if (++console_col >= CONSOLE_COLS) {
				console_newline();
			}
			return;
	}
	/* NOTREACHED */
}

static int OsdConsolePutchar(int c)
{
    lcd_putc((const char)c);
    if (c == '\n')
        ithFlushDCacheRange(osd_base, lcd_line_length * ithLcdGetHeight());
        
    return c;
}

void OsdConsoleClear(void)
{
	/* set osdbuffer to default color */
	memset ((char *)osd_base,
		0xAA,
		lcd_line_length * ithLcdGetHeight());
    
    ithFlushDCacheRange(osd_base, lcd_line_length * ithLcdGetHeight());

    console_col = 0;
    console_row = 0;
}

void OsdConsoleInit(void)
{
    lcd_line_length = (((ithLcdGetWidth() * CFG_LCD_BPP + 31) & ~31) >> 3);
    osd_addr = itpVmemAlloc(lcd_line_length * ithLcdGetHeight());
    osd_base = (void*) ithMapVram(osd_addr, lcd_line_length * ithLcdGetHeight(), ITH_VRAM_WRITE);

    OsdConsoleClear();
    while (ithLcdCursorIsUpdateDone());
    ithLcdCursorSetWidth(ithLcdGetWidth());
    ithLcdCursorSetHeight(ithLcdGetHeight());
    ithLcdCursorSetPitch(lcd_line_length);
    ithLcdCursorSetX(1);
    ithLcdCursorSetY(1);
    ithLcdCursorSetBaseAddr(osd_addr);
    ithLcdCursorSetColor(ITH_LCD_CURSOR_FG_COLOR, 0xFFFF);
    ithLcdCursorSetColor(ITH_LCD_CURSOR_BG_COLOR, 0x0);
    ithLcdCursorEnable();
    ithLcdCursorUpdate();
    
    ithPutcharFunc = OsdConsolePutchar;
}

static int OsdConsoleWrite(int file, char *ptr, int len, void* info)
{
    int i;

    for (i = 0; i < ITP_OSD_TIMEOUT_COUNT; i++)
    {
        if (ithLcdCursorIsUpdateDone() == 0)
            break;
    }
    
    for (i = 0; i < len && ptr[i] != 0; i++)
		lcd_putc(ptr[i]);

    ithFlushDCacheRange(osd_base, lcd_line_length * ithLcdGetHeight());
    return i;
}

static int OsdConsoleIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_CLEAR:
        OsdConsoleClear();
        break;

	case ITP_IOCTL_SET_FGCOLOR:
	    while (ithLcdCursorIsUpdateDone());
        ithLcdCursorSetColor(ITH_LCD_CURSOR_FG_COLOR, (int)ptr);
        ithLcdCursorUpdate();
        break;

    case ITP_IOCTL_SET_BGCOLOR:
        while (ithLcdCursorIsUpdateDone());
        ithLcdCursorSetColor(ITH_LCD_CURSOR_BG_COLOR, (int)ptr);
        ithLcdCursorUpdate();
        break;

	case ITP_IOCTL_INIT:
        OsdConsoleInit();
        break;
        
    default:
        errno = (ITP_DEVICE_OSDCONSOLE << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceOsdConsole =
{
    ":osdconsole",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    OsdConsoleWrite,
    itpLseekDefault,
    OsdConsoleIoctl,
    NULL
};

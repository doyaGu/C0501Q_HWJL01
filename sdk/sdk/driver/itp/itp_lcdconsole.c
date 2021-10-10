/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL LCD console functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include <string.h>
#include "itp_cfg.h"
#include "itp_font.h"

//#define DRAW_VIDEO_BUF

static int lcd_line_length;
static int lcd_color_fg;
static int lcd_color_bg;

static unsigned char *lcd_base_a, *lcd_base_b, *lcd_base_c;                /* Start of framebuffer memory    */
static short console_col;
static short console_row;

#define CONSOLE_ROWS		(ithLcdGetHeight() / ITP_FONT_HEIGHT)
#define CONSOLE_COLS		(ithLcdGetWidth() / ITP_FONT_WIDTH)
#define CONSOLE_ROW_SIZE	(ITP_FONT_HEIGHT * lcd_line_length)
#define CONSOLE_SIZE		(CONSOLE_ROW_SIZE * CONSOLE_ROWS)
#define CONSOLE_SCROLL_SIZE	(CONSOLE_SIZE - CONSOLE_ROW_SIZE)

static void lcd_drawchars (unsigned short x, unsigned short y, unsigned char *str, int count, char *lcd_console_address)
{
	unsigned char *dest;
	unsigned short row;

	dest = (unsigned char *)(lcd_console_address + y * lcd_line_length + x * 16 / 8);

	for (row=0;  row < ITP_FONT_HEIGHT;  ++row, dest += lcd_line_length)  {
		unsigned char *s = str;
		int i;
		unsigned short *d = (unsigned short *)dest;

		for (i=0; i<count; ++i) {
			unsigned char c, bits;

			c = *s++;
			bits = itpFontData[c * ITP_FONT_HEIGHT + row];

			for (c=0; c<8; ++c) {
				*d++ = (bits & 0x80) ?
						lcd_color_fg : lcd_color_bg;
				bits <<= 1;
			}
		}
	}
}

static inline void lcd_putc_xy (unsigned short x, unsigned short y, unsigned char c)
{
	lcd_drawchars (x, y, &c, 1, lcd_base_a);
	lcd_drawchars (x, y, &c, 1, lcd_base_b);
#ifdef DRAW_VIDEO_BUF
	lcd_drawchars (x, y, &c, 1, lcd_base_c);
#endif
}

static void console_scrollup (void)
{
	/* Copy up rows ignoring the first one */
	memcpy (lcd_base_a, lcd_base_a + CONSOLE_ROW_SIZE, CONSOLE_SCROLL_SIZE);
	memcpy (lcd_base_b, lcd_base_b + CONSOLE_ROW_SIZE, CONSOLE_SCROLL_SIZE);

	/* Clear the last one */
	ithMemset16(lcd_base_a + CONSOLE_SIZE - CONSOLE_ROW_SIZE, lcd_color_bg, CONSOLE_ROW_SIZE / sizeof (uint16_t));
	ithMemset16(lcd_base_b + CONSOLE_SIZE - CONSOLE_ROW_SIZE, lcd_color_bg, CONSOLE_ROW_SIZE / sizeof (uint16_t));
	
#ifdef DRAW_VIDEO_BUF
	memcpy (lcd_base_c, lcd_base_c + CONSOLE_ROW_SIZE, CONSOLE_SCROLL_SIZE);
	ithMemset16(lcd_base_c + CONSOLE_SIZE - CONSOLE_ROW_SIZE, lcd_color_bg, CONSOLE_ROW_SIZE / sizeof (uint16_t));
#endif
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

static int LcdConsolePutchar(int c)
{
    lcd_putc((const char)c);
    if (c == '\n')
    {
        ithFlushDCacheRange(lcd_base_a, lcd_line_length * ithLcdGetHeight());
        ithFlushDCacheRange(lcd_base_b, lcd_line_length * ithLcdGetHeight());
    #ifdef DRAW_VIDEO_BUF        
        ithFlushDCacheRange(lcd_base_c, lcd_line_length * ithLcdGetHeight());
    #endif
    }
    return c;
}

void LcdConsoleInit(void)
{
    lcd_line_length = ithLcdGetPitch();
    lcd_base_a = (unsigned char*) ithMapVram(ithLcdGetBaseAddrA(), lcd_line_length * ithLcdGetHeight(), ITH_VRAM_WRITE);
    lcd_base_b = (unsigned char*) ithMapVram(ithLcdGetBaseAddrB(), lcd_line_length * ithLcdGetHeight(), ITH_VRAM_WRITE);
#ifdef DRAW_VIDEO_BUF    
    lcd_base_c = (unsigned char*) ithMapVram(ithLcdGetBaseAddrC(), lcd_line_length * ithLcdGetHeight(), ITH_VRAM_WRITE);
#endif
    lcd_color_fg	= 0xFFFF;
    lcd_color_bg	= 0;
	console_col		= 0;
	console_row		= 1;
	
	ithPutcharFunc = LcdConsolePutchar;
}

void LcdConsoleClear(void)
{
	/* set framebuffer to background color */
	ithMemset16(lcd_base_a,
		lcd_color_bg,
		lcd_line_length * ithLcdGetHeight() / sizeof (uint16_t));
    
    ithFlushDCacheRange(lcd_base_a, lcd_line_length * ithLcdGetHeight());

	ithMemset16(lcd_base_b,
		lcd_color_bg,
		lcd_line_length * ithLcdGetHeight() / sizeof (uint16_t));
    
    ithFlushDCacheRange(lcd_base_b, lcd_line_length * ithLcdGetHeight());

#ifdef DRAW_VIDEO_BUF
	ithMemset16(lcd_base_c,
		lcd_color_bg,
		lcd_line_length * ithLcdGetHeight() / sizeof (uint16_t));
    
    ithFlushDCacheRange(lcd_base_c, lcd_line_length * ithLcdGetHeight());
#endif // DRAW_VIDEO_BUF

    console_col = 0;
    console_row = 0;
}

static int LcdConsoleWrite(int file, char *ptr, int len, void* info)
{
    int i;
    
    for (i = 0; i < len && ptr[i] != 0; i++)
		lcd_putc(ptr[i]);

    ithFlushDCacheRange(lcd_base_a, lcd_line_length * ithLcdGetHeight());
    ithFlushDCacheRange(lcd_base_b, lcd_line_length * ithLcdGetHeight());
#ifdef DRAW_VIDEO_BUF
    ithFlushDCacheRange(lcd_base_c, lcd_line_length * ithLcdGetHeight());
#endif
    return i;
}

static int LcdConsoleIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_CLEAR:
        LcdConsoleClear();
        break;

	case ITP_IOCTL_SET_FGCOLOR:
        lcd_color_fg = (int)ptr;
        break;

    case ITP_IOCTL_SET_BGCOLOR:
        lcd_color_bg = (int)ptr;
        break;

	case ITP_IOCTL_INIT:
        LcdConsoleInit();
        break;
        
    default:
        errno = (ITP_DEVICE_LCDCONSOLE << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceLcdConsole =
{
    ":lcdconsole",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    LcdConsoleWrite,
    itpLseekDefault,
    LcdConsoleIoctl,
    NULL
};

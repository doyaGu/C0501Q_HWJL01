/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2011 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "SDL_config.h"

#include "../SDL_sysvideo.h"
#include "ite/ith.h"

#define CASTOR3_SURFACE   "_SDL_Castor3Surface"

int SDL_Castor3_CreateWindowFramebuffer(_THIS, SDL_Window * window, Uint32 * format, void ** pixels, int *pitch)
{
	const Uint32 lcd2pixel[] = { SDL_PIXELFORMAT_RGB565, SDL_PIXELFORMAT_ARGB1555, SDL_PIXELFORMAT_ARGB4444, SDL_PIXELFORMAT_ARGB8888, SDL_PIXELFORMAT_IYUV };
    SDL_Surface *surface;
    const Uint32 surface_format = lcd2pixel[ithLcdGetFormat()];
    int w, h, p;
    int bpp;
    Uint32 Rmask, Gmask, Bmask, Amask;
	void *buf;

    /* Free the old framebuffer surface */
    surface = (SDL_Surface *) SDL_GetWindowData(window, CASTOR3_SURFACE);
    if (surface) {
		ithUnmapVram(surface->pixels, surface->pitch * surface->h);
        SDL_FreeSurface(surface);
    }

    /* Create a new one */
    SDL_PixelFormatEnumToMasks(surface_format, &bpp, &Rmask, &Gmask, &Bmask, &Amask);
    w = ithLcdGetWidth();
    h = ithLcdGetHeight();
	p = ithLcdGetPitch();
	buf = ithMapVram(ithLcdGetBaseAddrA(), p * h, ITH_VRAM_READ | ITH_VRAM_WRITE);
	surface = SDL_CreateRGBSurfaceFrom(buf, w, h, bpp, p, Rmask, Gmask, Bmask, Amask);
    if (!surface) {
        return -1;
    }

    /* Save the info and return! */
    SDL_SetWindowData(window, CASTOR3_SURFACE, surface);
    *format = surface_format;
    *pixels = surface->pixels;
    *pitch = surface->pitch;
    return 0;
}

int SDL_Castor3_UpdateWindowFramebuffer(_THIS, SDL_Window * window, SDL_Rect * rects, int numrects)
{
    static int frame_number;
    SDL_Surface *surface;

    surface = (SDL_Surface *) SDL_GetWindowData(window, CASTOR3_SURFACE);
    if (!surface) {
        SDL_SetError("Couldn't find openrtos surface for window");
        return -1;
    }

	ithFlushDCacheRange(surface->pixels, surface->pitch * surface->h);

    /* Send the data to the display */
#if 0
	if (SDL_getenv("SDL_VIDEO_CASTOR3_SAVE_FRAMES")) {
        char file[128];
        SDL_snprintf(file, sizeof(file), "SDL_window%d-%8.8d.bmp",
                     SDL_GetWindowID(window), ++frame_number);
        SDL_SaveBMP(surface, file);
    }
#endif

    return 0;
}

void SDL_Castor3_DestroyWindowFramebuffer(_THIS, SDL_Window * window)
{
    SDL_Surface *surface;

    surface = (SDL_Surface *) SDL_SetWindowData(window, CASTOR3_SURFACE, NULL);
    if (surface) {
		ithUnmapVram(surface->pixels, surface->pitch * surface->h);
        SDL_FreeSurface(surface);
    }
}

/* vi: set ts=4 sw=4 expandtab: */

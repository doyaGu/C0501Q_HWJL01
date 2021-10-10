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

/* OpenRTOS SDL video driver implementation; this is just enough to make an
 *  SDL-based application THINK it's got a working video driver, for
 *  applications that call SDL_Init(SDL_INIT_VIDEO) when they don't need it,
 *  and also for use as a collection of stubs when porting SDL to a new
 *  platform for which you haven't yet written a valid video driver.
 *
 * This is also a great way to determine bottlenecks: if you think that SDL
 *  is a performance problem for a given platform, enable this driver, and
 *  then see if your application runs faster without video overhead.
 *
 * Initial work by Ryan C. Gordon (icculus@icculus.org). A good portion
 *  of this was cut-and-pasted from Stephane Peter's work in the AAlib
 *  SDL video driver.  Renamed to "DUMMY" by Sam Lantinga.
 */

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "SDL_castor3video.h"
#include "SDL_castor3events_c.h"
#include "SDL_castor3framebuffer_c.h"
#include "SDL_castor3openvg.h"
#include "SDL_castor3window.h"
#include "SDL_castor3keyboard.h"
#include "SDL_castor3keypad.h"
#include "SDL_castor3mouse.h"
#include "ite/itp.h"
#include <sys/ioctl.h>

#define CASTOR3VID_DRIVER_NAME "castor3"

/* Initialization/Query functions */
static int Castor3_VideoInit(_THIS);
static int Castor3_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode);
static void Castor3_VideoQuit(_THIS);

/* Castor3 driver bootstrap functions */

static int
Castor3_Available(void)
{
    return (1);
}

static void
Castor3_DeleteDevice(SDL_VideoDevice * device)
{
    SDL_free(device->driverdata);
    SDL_free(device);
}

static SDL_VideoDevice *
Castor3_CreateDevice(int devindex)
{
    SDL_VideoDevice *device;
    SDL_VideoData *data;

    /* Initialize all variables that we clean on shutdown */
    device = (SDL_VideoDevice *) SDL_calloc(1, sizeof(SDL_VideoDevice));
    if (device) {
        data = (struct SDL_VideoData *) SDL_calloc(1, sizeof(SDL_VideoData));
    } else {
        data = NULL;
    }
    if (!data) {
        SDL_OutOfMemory();
        if (device) {
            SDL_free(device);
        }
        return NULL;
    }
    device->driverdata = data;

    /* Set the function pointers */
    device->VideoInit = Castor3_VideoInit;
    device->VideoQuit = Castor3_VideoQuit;
    device->SetDisplayMode = Castor3_SetDisplayMode;
    device->PumpEvents = Castor3_PumpEvents;

#undef CreateWindow
    device->CreateWindow = Castor3_CreateWindow;
	device->GetWindowWMInfo = Castor3_GetWindowWMInfo;
	device->DestroyWindow = Castor3_DestroyWindow;

#ifdef CFG_LCD_ENABLE
    device->CreateWindowFramebuffer = SDL_Castor3_CreateWindowFramebuffer;
    device->UpdateWindowFramebuffer = SDL_Castor3_UpdateWindowFramebuffer;
    device->DestroyWindowFramebuffer = SDL_Castor3_DestroyWindowFramebuffer;
#endif

#if SDL_VIDEO_RENDER_OVG && !SDL_RENDER_DISABLED    
    device->GL_CreateContext = Castor3_OVG_CreateContext;
    device->GL_MakeCurrent = Castor3_OVG_MakeCurrent;
    device->GL_SetSwapInterval = Castor3_OVG_SetSwapInterval;
    device->GL_GetSwapInterval = Castor3_OVG_GetSwapInterval;
    device->GL_SwapWindow = Castor3_OVG_SwapWindow;
    device->GL_DeleteContext = Castor3_OVG_DeleteContext;
#endif

    device->_free = Castor3_DeleteDevice;

    return device;
}

VideoBootStrap Castor3_bootstrap = {
    CASTOR3VID_DRIVER_NAME, "SDL castor3 video driver",
    Castor3_Available, Castor3_CreateDevice
};


int
Castor3_VideoInit(_THIS)
{
#ifdef CFG_LCD_ENABLE
	static const Uint32 lcd2pixel[] = { SDL_PIXELFORMAT_RGB565, SDL_PIXELFORMAT_ARGB1555, SDL_PIXELFORMAT_ARGB4444, SDL_PIXELFORMAT_ARGB8888, SDL_PIXELFORMAT_IYUV };
    SDL_DisplayMode mode;

    mode.format = lcd2pixel[ithLcdGetFormat()];
    mode.w = ithLcdGetWidth();
    mode.h = ithLcdGetHeight();
    mode.refresh_rate = 0;
    mode.driverdata = NULL;
    if (SDL_AddBasicVideoDisplay(&mode) < 0) {
        return -1;
    }

    SDL_zero(mode);
    SDL_AddDisplayMode(&_this->displays[0], &mode);

#if !defined(CFG_LCD_INIT_ON_BOOTING) && !defined(CFG_BL_SHOW_LOGO)
    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
#endif

#else
    SDL_AddBasicVideoDisplay(NULL);

#endif // CFG_LCD_ENABLE

#ifdef CFG_KEYPAD_ENABLE
    Castor3_InitKeypad();
#endif

#ifdef CFG_IR_ENABLE
    Castor3_InitIr();
#endif

#ifdef CFG_TOUCH_ENABLE
    Castor3_InitTouch();
#endif

#ifdef CFG_USB_MOUSE
    Castor3_InitMouse();
#endif

#ifdef CFG_USB_KBD
    Castor3_InitKeyboard();
#endif

#if defined(CFG_BACKLIGHT_ENABLE) && !defined(CFG_LCD_INIT_ON_BOOTING) && !defined(CFG_BL_SHOW_LOGO) && !defined(CFG_LCD_MULTIPLE)
    usleep(100000);    
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_RESET, NULL);
#endif

    /* We're done! */
    return 0;
}

static int
Castor3_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode)
{
    return 0;
}

void
Castor3_VideoQuit(_THIS)
{
#ifdef CFG_TOUCH_ENABLE
    Castor3_QuitTouch();
#endif
}

/* vi: set ts=4 sw=4 expandtab: */

/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2012 Sam Lantinga <slouken@libsdl.org>

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

#include "SDL_windowsvideo.h"

#include "SDL_openvg.h"

#if SDL_VIDEO_RENDER_OVG && !SDL_RENDER_DISABLED

SDL_GLContext
WIN_OVG_CreateContext(_THIS, SDL_Window * window)
{
    SDL_VideoData *vdata = (SDL_VideoData *) _this->driverdata;
    SDL_WindowData *wdata = (SDL_WindowData *) window->driverdata;
	EGLContext egl_context;

    egl_context = eglCreateContext(vdata->egl_display,
								   vdata->egl_config,
                                   EGL_NO_CONTEXT, NULL);

    if (egl_context == EGL_NO_CONTEXT) {
        SDL_SetError("Could not create EGL context");
        return NULL;
    }

    if (eglMakeCurrent(vdata->egl_display,
                       wdata->egl_surface,
                       wdata->egl_surface,
                       egl_context) != EGL_TRUE) {
        SDL_SetError("Unable to make EGL context current");
        return NULL;
    }

	return egl_context;
}

int 
WIN_OVG_MakeCurrent(_THIS, SDL_Window * window, SDL_GLContext context)
{
	SDL_VideoData *vdata = (SDL_VideoData *) _this->driverdata;
	SDL_WindowData *wdata = (SDL_WindowData *) window->driverdata;

    if (eglMakeCurrent(vdata->egl_display,
                       wdata->egl_surface,
                       wdata->egl_surface,
                       (EGLContext)context) != EGL_TRUE) {
        SDL_SetError("Unable to make EGL context current");
        return -1;
    }
	return 0;
}

int 
WIN_OVG_SetSwapInterval(_THIS, int interval)
{
	SDL_VideoData *vdata = (SDL_VideoData *) _this->driverdata;

    if (eglSwapInterval(vdata->egl_display,
                        interval) != EGL_TRUE) {
        SDL_SetError("Unable to set EGL swap interval");
        return -1;
    }
	vdata->swapinterval = interval;
	return 0;
}

int 
WIN_OVG_GetSwapInterval(_THIS)
{
	SDL_VideoData *vdata = (SDL_VideoData *) _this->driverdata;
	return vdata->swapinterval;
}

void 
WIN_OVG_SwapWindow(_THIS, SDL_Window * window)
{
	SDL_VideoData *vdata = (SDL_VideoData *) _this->driverdata;
	SDL_WindowData *wdata = (SDL_WindowData *) window->driverdata;

	eglSwapBuffers(vdata->egl_display, wdata->egl_surface);
}

void 
WIN_OVG_DeleteContext(_THIS, SDL_GLContext context)
{
	SDL_VideoData *vdata = (SDL_VideoData *) _this->driverdata;

    if (context != EGL_NO_CONTEXT) {
        eglDestroyContext(vdata->egl_display, (EGLContext)context);
    }
}

#endif /* SDL_VIDEO_RENDER_OVG && !SDL_RENDER_DISABLED */

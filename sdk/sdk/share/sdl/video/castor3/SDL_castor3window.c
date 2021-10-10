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

#include "SDL_castor3video.h"
#include "SDL_castor3window.h"
#include "SDL_syswm.h"

int Castor3_CreateWindow(_THIS, SDL_Window * window)
{
    SDL_WindowData *wdata;

    /* Allocate the window data */
    wdata = (SDL_WindowData *) SDL_malloc(sizeof(*wdata));
    if (!wdata) {
        SDL_OutOfMemory();
        return -1;
    }
    window->driverdata = wdata;

#if SDL_VIDEO_RENDER_OVG
    /* Check if window must support OpenGL rendering */
    if ((window->flags & SDL_WINDOW_OPENGL) == SDL_WINDOW_OPENGL) {
        SDL_VideoData *vdata = (SDL_VideoData *) _this->driverdata;

        /* Create connection to OpenVG */
        if (vdata->egl_display == EGL_NO_DISPLAY) {
			static const EGLint attribs[] =
			{
				EGL_RED_SIZE,			5,
				EGL_GREEN_SIZE,         6,
				EGL_BLUE_SIZE,			5,
				EGL_LUMINANCE_SIZE,		EGL_DONT_CARE,            //EGL_DONT_CARE
				EGL_SURFACE_TYPE,		EGL_WINDOW_BIT,
				EGL_RENDERABLE_TYPE,    EGL_OPENVG_BIT,
				EGL_NONE
			};
			EGLint found_configs;

			vdata->egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
            if (vdata->egl_display == EGL_NO_DISPLAY) {
                SDL_SetError("Could not get EGL display");
                return -1;
            }

            if (eglInitialize(vdata->egl_display, NULL, NULL) != EGL_TRUE) {
                SDL_SetError("Could not initialize EGL");
                return -1;
            }

			if (eglChooseConfig(vdata->egl_display, attribs, &vdata->egl_config, 1, &found_configs) == EGL_FALSE ||
				found_configs == 0) {
				SDL_SetError("Couldn't find matching EGL config");
				return -1;
			}

			if (eglBindAPI(EGL_OPENVG_API) != EGL_TRUE) {
				SDL_SetError("Could not bind OpenVG");
				return -1;
			}
		}

        wdata->egl_surface = eglCreateWindowSurface(vdata->egl_display,
                                                    vdata->egl_config,
                                                    NULL, NULL);

        if (wdata->egl_surface == EGL_NO_SURFACE) {
            SDL_SetError("Could not create EGL window surface");
            return -1;
        }
    }
#endif
    window->flags |= SDL_WINDOW_FULLSCREEN;
    return 0;
}

SDL_bool
Castor3_GetWindowWMInfo(_THIS, SDL_Window * window, SDL_SysWMinfo * info)
{
    if (info->version.major <= SDL_MAJOR_VERSION) {
        info->subsystem = SDL_SYSWM_CASTOR3;
        info->info.castor3.window = NULL;
        return SDL_TRUE;
    } else {
        SDL_SetError("Application not compiled with SDL %d.%d\n",
                     SDL_MAJOR_VERSION, SDL_MINOR_VERSION);
        return SDL_FALSE;
    }
}

void Castor3_DestroyWindow(_THIS, SDL_Window * window)
{
    SDL_free(window->driverdata);
}

/* vi: set ts=4 sw=4 expandtab: */

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

#include "../../events/SDL_events_c.h"

#include "SDL_castor3mouse.h"
#include <unistd.h>
#include "ite/itp.h"

void Castor3_InitMouse(void)
{
    // DO NOTHING
}

void Castor3_PumpMouseEvent(_THIS)
{
    ITPMouseEvent ev;

    if (read(ITP_DEVICE_USBMOUSE, &ev, sizeof (ITPMouseEvent)) == sizeof (ITPMouseEvent))
    {
        if (ev.flags & ITP_MOUSE_LBTN_DOWN)
        {
            SDL_SendMouseButton(NULL, SDL_PRESSED, SDL_BUTTON_LEFT);
        }
        if (ev.flags & ITP_MOUSE_LBTN_UP)
        {
            SDL_SendMouseButton(NULL, SDL_RELEASED, SDL_BUTTON_LEFT);
        }
        if (ev.flags & ITP_MOUSE_RBTN_DOWN)
        {
            SDL_SendMouseButton(NULL, SDL_PRESSED, SDL_BUTTON_RIGHT);
        }
        if (ev.flags & ITP_MOUSE_RBTN_UP)
        {
            SDL_SendMouseButton(NULL, SDL_RELEASED, SDL_BUTTON_RIGHT);
        }
        if (ev.flags & ITP_MOUSE_MBTN_DOWN)
        {
            SDL_SendMouseButton(NULL, SDL_PRESSED, SDL_BUTTON_MIDDLE);
        }
        if (ev.flags & ITP_MOUSE_MBTN_UP)
        {
            SDL_SendMouseButton(NULL, SDL_RELEASED, SDL_BUTTON_MIDDLE);
        }
        if (ev.x || ev.y)
        {
            //printf("x=%d y=%d\n", ev.x, ev.y);
            SDL_SendMouseMotion(_this->windows, 1, ev.x, ev.y);
        }
    }
}

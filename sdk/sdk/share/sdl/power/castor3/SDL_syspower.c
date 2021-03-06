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

#ifndef SDL_POWER_DISABLED
#ifdef SDL_POWER_CASTOR3

#include "ite/itp.h"

#include "SDL_power.h"

SDL_bool
SDL_GetPowerInfo_Castor3(SDL_PowerState * state, int *seconds, int *percent)
{
#ifdef CFG_BATTERY_ENABLE
    ITPPowerStatus status;

    if (read(ITP_DEVICE_POWER, &status, sizeof (ITPPowerStatus)) == 0)
    {
        *state = SDL_POWERSTATE_UNKNOWN;
    } else if (status.batteryState == ITP_BATTERY_UNKNOWN) {    /* unknown state */
        *state = SDL_POWERSTATE_UNKNOWN;
    } else if (status.batteryState == ITP_BATTERY_NO_BATTERY) { /* no battery */
        *state = SDL_POWERSTATE_NO_BATTERY;
    } else if (status.batteryState == ITP_BATTERY_CHARGING) { /* charging */
        *state = SDL_POWERSTATE_CHARGING;
    } else if (status.batteryState == ITP_BATTERY_CHARGED) {
        *state = SDL_POWERSTATE_CHARGED;        /* on AC, not charging. */
    } else {
        *state = SDL_POWERSTATE_ON_BATTERY;     /* not on AC. */
    }

    *percent = status.batteryPercent;
    *seconds = -1;
#endif // CFG_BATTERY_ENABLE
    return SDL_TRUE;            /* always the definitive answer on Castor3. */
}

#endif /* SDL_POWER_CASTOR3 */
#endif /* SDL_POWER_DISABLED */

/* vi: set ts=4 sw=4 expandtab: */

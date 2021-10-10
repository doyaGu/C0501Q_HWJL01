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

#if defined(SDL_TIMER_OPENRTOS)

#include "SDL_timer.h"
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"

static TickType_t start;

void
SDL_StartTicks(void)
{
    /* Set first ticks value */
    start = xTaskGetTickCount();
}

Uint32
SDL_GetTicks(void)
{
    TickType_t tick = xTaskGetTickCount();
    if (tick >= start)
        return ((tick - start) / portTICK_PERIOD_MS);
    else
        return ((0xFFFFFFFF - start + tick) / portTICK_PERIOD_MS);
}

Uint64
SDL_GetPerformanceCounter(void)
{
    return SDL_GetTicks();
}

Uint64
SDL_GetPerformanceFrequency(void)
{
    return configTICK_RATE_HZ;
}

void
SDL_Delay(Uint32 ms)
{
    vTaskDelay(ms * portTICK_PERIOD_MS);
}

#endif /* SDL_TIMER_OPENRTOS */

/* vi: set ts=4 sw=4 expandtab: */

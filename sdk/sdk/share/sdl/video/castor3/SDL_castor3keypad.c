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

#include "SDL_castor3keypad.h"
#include <unistd.h>
#include "ite/itp.h"

void Castor3_InitKeypad(void)
{
    SDL_Keycode keymap[SDL_NUM_SCANCODES];

    /* Add default scancode to key mapping */
    SDL_GetDefaultKeymap(keymap);
    SDL_SetKeymap(0, keymap, SDL_NUM_SCANCODES);
}

static SDL_Scancode Castor3_Keycodes[] = {
    #include "keypad_mapping_table.inc"
};

static SDL_Scancode
TranslateKeycode(int keycode)
{
    SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;

    if (keycode < SDL_arraysize(Castor3_Keycodes)) {
        scancode = Castor3_Keycodes[keycode];
    }
    return scancode;
}

void Castor3_PumpKeypadEvent(void)
{
    ITPKeypadEvent ev;

#if CFG_KEYPAD_PROBE_INTERVAL == 0
    ioctl(ITP_DEVICE_KEYPAD, ITP_IOCTL_PROBE, NULL);
#endif
    if (read(ITP_DEVICE_KEYPAD, &ev, sizeof (ITPKeypadEvent)) == sizeof (ITPKeypadEvent))
    {
        if (ev.flags & ITP_KEYPAD_DOWN)
        {
            SDL_SendKeyboardKey(SDL_PRESSED, TranslateKeycode(ev.code));
        }
        else if (ev.flags & ITP_KEYPAD_UP)
        {
            SDL_SendKeyboardKey(SDL_RELEASED, TranslateKeycode(ev.code));
        }
    }
}

/* vi: set ts=4 sw=4 expandtab: */

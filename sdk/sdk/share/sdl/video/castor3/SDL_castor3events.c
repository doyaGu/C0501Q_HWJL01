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

/* Being a null driver, there's no event stream. We just define stubs for
   most of the API. */

#include "../../events/SDL_events_c.h"

#include "SDL_castor3video.h"
#include "SDL_castor3events_c.h"
#include "SDL_castor3keyboard.h"
#include "SDL_castor3keypad.h"
#include "SDL_castor3ir.h"
#include "SDL_castor3touch.h"
#include "SDL_castor3mouse.h"
#include "../../audio/castor3/main_processor_message_queue.h"
#include "ite/ith.h"

void
Castor3_PumpEvents(_THIS)
{
    int ret;
    
#ifdef CFG_KEYPAD_ENABLE
    Castor3_PumpKeypadEvent();
#endif

#ifdef CFG_IR_ENABLE
    Castor3_PumpIrEvent();
#endif

#ifdef CFG_TOUCH_ENABLE
    Castor3_PumpTouchEvent();
#endif

#ifdef CFG_USB_MOUSE
    Castor3_PumpMouseEvent(_this);
#endif
    
#ifdef CFG_USB_KBD
    Castor3_PumpKeyboardEvent();
#endif

#ifdef CFG_AUDIO_ENABLE
    // check which audio processor cmd to process
    {
        uint16_t nAudioPluginRegister = 0;
        nAudioPluginRegister = getAudioPluginMessageStatus();
        if (((nAudioPluginRegister & 0xc000)>>14) == SMTK_AUDIO_PROCESSOR_ID)
        {
            // do audio api
            //printf("nAudioPluginRegister 0x%x \n",nAudioPluginRegister);
            smtkMainProcessorExecuteAudioPluginCmd(nAudioPluginRegister);
        }
    }
#endif // CFG_AUDIO_ENABLE

#ifdef CFG_WATCHDOG_ENABLE
    ithWatchDogRestart();
#endif
}

/* vi: set ts=4 sw=4 expandtab: */

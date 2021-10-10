#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL/SDL.h"
#include "ite/itu.h"

#define GESTURE_THRESHOLD           40

ITUScene theScene;
extern ITUActionFunction actionFunctions[];


int SDL_main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Event ev;
    int done, delay, lastx, lasty;
    uint32_t tick, dblclk, mouseDownTick;

    // wait mouting USB storage
#ifndef _WIN32
    sleep(3);
#endif

    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        return (1);
    }
    window = SDL_CreateWindow("ITU Function Test",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              800, 600, 0);
    if (!window)
    {
        printf("Couldn't create 800x600 window: %s\n",
                SDL_GetError());
        SDL_Quit();
        exit(2);
    }

    // init itu
    ituLcdInit();

#ifdef CFG_M2D_ENABLE
    ituM2dInit();
#else
    ituSWInit();
#endif

    ituFtInit();
    ituFtLoadFont(0, CFG_PRIVATE_DRIVE ":/font/WenQuanYiMicroHeiMono.ttf", ITU_GLYPH_8BPP);

    // load itu file
    tick = SDL_GetTicks();

    ituSceneInit(&theScene, NULL);
    ituSceneSetFunctionTable(&theScene, actionFunctions);
    ituSceneLoadFile(&theScene, CFG_PRIVATE_DRIVE ":/ChineseKeyboard.itu");

    printf("loading time: %dms\n", SDL_GetTicks() - tick);

    theScene.upKey         = SDLK_UP;
    theScene.downKey       = SDLK_DOWN;

    /* Watch keystrokes */
    done = dblclk = 0;
    while (!done)
    {
        tick = SDL_GetTicks();

        while (SDL_PollEvent(&ev))
        {
            switch (ev.type)
            {
            case SDL_FINGERDOWN:
                printf("SDL_FINGERUP\n");
                {
                    mouseDownTick = SDL_GetTicks();
                    {
                        ituSceneUpdate(&theScene, ITU_EVENT_MOUSEDOWN, 1, ev.tfinger.x, ev.tfinger.y);
                        dblclk = mouseDownTick;
                        lastx = ev.tfinger.x;
                        lasty = ev.tfinger.y;
                    }
                }
                break;

            case SDL_FINGERUP:
                printf("SDL_FINGERDOWN\n");
                if (SDL_GetTicks() - dblclk <= 300)
                {
                    int xdiff = abs(ev.tfinger.x - lastx);
                    int ydiff = abs(ev.tfinger.y - lasty);

                    if (xdiff >= GESTURE_THRESHOLD && xdiff > ydiff)
                    {
                        if (ev.tfinger.x > lastx)
                        {
                            printf("touch: slide to right %d %d\n", ev.tfinger.x, ev.tfinger.y);
                            ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDERIGHT, xdiff, ev.tfinger.x, ev.tfinger.y);
                        }
                        else
                        {
                            printf("touch: slide to left %d %d\n", ev.button.x, ev.button.y);
                            ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDELEFT, xdiff, ev.tfinger.x, ev.tfinger.y);
                        }
                    }
                    else if (ydiff >= GESTURE_THRESHOLD)
                    {
                        if (ev.tfinger.y > lasty)
                        {
                            printf("touch: slide to down %d %d\n", ev.tfinger.x, ev.tfinger.y);
                            ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDEDOWN, ydiff, ev.tfinger.x, ev.tfinger.y);
                        }
                        else
                        {
                            printf("touch: slide to up %d %d\n", ev.tfinger.x, ev.tfinger.y);
                            ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDEUP, ydiff, ev.tfinger.x, ev.tfinger.y);
                        }
                    }
                }
                ituSceneUpdate(&theScene, ITU_EVENT_MOUSEUP, 1, ev.tfinger.x, ev.tfinger.y);
                mouseDownTick = 0;
                break;

            case SDL_QUIT:
                done = 1;
                break;

            }
        }

        if (ituSceneUpdate(&theScene, ITU_EVENT_TIMER, 0, 0, 0))
        {
            ituSceneDraw(&theScene, ituGetDisplaySurface());
            ituFlip(ituGetDisplaySurface());
        }

        delay = 33 - (SDL_GetTicks() - tick);
        if (delay > 0)
        {
            SDL_Delay(delay);
        }
    }

    SDL_Quit();
    return 0;
}


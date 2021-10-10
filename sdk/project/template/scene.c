#include <sys/time.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "SDL/SDL.h"
#include "ite/itu.h"
#include "scene.h"
#include "project.h"

#define FPS_ENABLE
#define GESTURE_THRESHOLD 40
//#define DOUBLE_KEY_ENABLE

extern ITUActionFunction actionFunctions[];

// scene
ITUScene theScene;
static SDL_Window *window;
static ITUSurface* screenSurf;
static int periodPerFrame;

void SceneInit(void)
{
    window = SDL_CreateWindow("Template", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, CFG_LCD_WIDTH, CFG_LCD_HEIGHT, 0);
    if (!window)
    {
        printf("Couldn't create window: %s\n", SDL_GetError());
        return;
    }

    // init itu
    ituLcdInit();

#ifdef CFG_M2D_ENABLE
    ituM2dInit();
    ituFrameFuncInit();
#else
    ituSWInit();
#endif

#ifdef CFG_ENABLE_ROTATE
    ituSceneSetRotation(&theScene, ITU_ROT_90, ithLcdGetWidth(), ithLcdGetHeight());
#endif

#ifdef CFG_PLAY_VIDEO_ON_BOOTING
#ifndef CFG_BOOT_VIDEO_ENABLE_WINDOW_MODE
#ifndef CFG_ENABLE_ROTATE
    PlayVideo(0, 0, ithLcdGetWidth(), ithLcdGetHeight(), CFG_BOOT_VIDEO_BGCOLOR, CFG_BOOT_VIDEO_VOLUME);
#else
    PlayVideo(0, 0, ithLcdGetHeight(), ithLcdGetWidth(), CFG_BOOT_VIDEO_BGCOLOR, CFG_BOOT_VIDEO_VOLUME);
#endif
#else
    PlayVideo(CFG_VIDEO_WINDOW_X_POS, CFG_VIDEO_WINDOW_Y_POS, CFG_VIDEO_WINDOW_WIDTH, CFG_VIDEO_WINDOW_HEIGHT, CFG_BOOT_VIDEO_BGCOLOR, CFG_BOOT_VIDEO_VOLUME);
#endif
    WaitPlayVideoFinish();
#endif

#ifdef CFG_PLAY_MJPEG_ON_BOOTING
#ifndef CFG_BOOT_VIDEO_ENABLE_WINDOW_MODE
#ifndef CFG_ENABLE_ROTATE
	PlayMjpeg(0, 0, ithLcdGetWidth(), ithLcdGetHeight(), CFG_BOOT_VIDEO_BGCOLOR, 0);
#else
	PlayMjpeg(0, 0, ithLcdGetHeight(), ithLcdGetWidth(), CFG_BOOT_VIDEO_BGCOLOR, 0);
#endif
#else
	PlayMjpeg(CFG_VIDEO_WINDOW_X_POS, CFG_VIDEO_WINDOW_Y_POS, CFG_VIDEO_WINDOW_WIDTH, CFG_VIDEO_WINDOW_HEIGHT, CFG_BOOT_VIDEO_BGCOLOR, 0);
#endif
    WaitPlayMjpegFinish();
#endif

    screenSurf = ituGetDisplaySurface();

    ituFtInit();
    ituFtLoadFont(0, CFG_PRIVATE_DRIVE ":/font/" CFG_FONT_FILENAME, ITU_GLYPH_8BPP);

    ituSceneInit(&theScene, NULL);
	ituSceneSetFunctionTable(&theScene, actionFunctions);

    theScene.leftKey   = SDLK_LEFT;
    theScene.upKey     = SDLK_UP;
    theScene.rightKey  = SDLK_RIGHT;
    theScene.downKey   = SDLK_DOWN;
    
    periodPerFrame = MS_PER_FRAME;
}

void SceneExit(void)
{
    if (theScene.root)
    {
        ituSceneExit(&theScene);
    }
    ituFtExit();

#ifdef CFG_M2D_ENABLE
    ituM2dExit();
#ifdef CFG_VIDEO_ENABLE
    ituFrameFuncExit();
#endif // CFG_VIDEO_ENABLE
#else
    ituSWExit();
#endif // CFG_M2D_ENABLE

    SDL_DestroyWindow(window);
}

void SceneLoad(void)
{
    // load itu file
    ituSceneLoadFile(&theScene, CFG_PRIVATE_DRIVE ":/template.itu");
}

int SceneRun(void)
{
    SDL_Event ev, lastev;
    int done, delay, frames, lastx, lasty;
    uint32_t tick, dblclk, lasttick;

    /* Watch keystrokes */
    done = dblclk = frames = lasttick = lastx = lasty = 0;
#ifndef _WIN32
    AudioPlayKeySound(); // init i2s
#endif
    while (!done)
    {
        bool result = false;

        tick = SDL_GetTicks();

        frames++;

#ifdef FPS_ENABLE
        if (tick - lasttick >= 1000)
        {
            printf("fps: %d\n", frames);
            frames = 0;
            lasttick = tick;
        }
#endif

        while (SDL_PollEvent(&ev))
        {
            switch (ev.type)
            {
            case SDL_KEYDOWN:
                result = ituSceneUpdate(&theScene, ITU_EVENT_KEYDOWN, ev.key.keysym.sym, 0, 0);
#ifndef _WIN32
                if (result)
                    AudioPlayKeySound();
#endif
                break;

            case SDL_KEYUP:
                result = ituSceneUpdate(&theScene, ITU_EVENT_KEYUP, ev.key.keysym.sym, 0, 0);
                break;

            case SDL_MOUSEMOTION:
                result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEMOVE, ev.button.button, ev.button.x, ev.button.y);
                break;

            case SDL_MOUSEBUTTONDOWN:
                {
                    uint32_t t = SDL_GetTicks();
                    if (t - dblclk <= 300)
                    {
                        result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEDOUBLECLICK, ev.button.button, ev.button.x, ev.button.y);
                        dblclk = 0;
                    }
                    else
                    {
						printf("mouse: down %d, %d\n", ev.button.x, ev.button.y);
                        result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEDOWN, ev.button.button, ev.button.x, ev.button.y);
                        dblclk = t;
                        lastx = ev.button.x;
                        lasty = ev.button.y;
                    }
#ifndef _WIN32
                    if (result)
                        AudioPlayKeySound();
#endif
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (SDL_GetTicks() - dblclk <= 300)
                {
					int xdiff = abs(ev.button.x - lastx);
					int ydiff = abs(ev.button.y - lasty);

					if (xdiff >= GESTURE_THRESHOLD && xdiff > ydiff)
					{
						if (ev.button.x > lastx)
						{
							printf("mouse: slide to right\n");
							result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDERIGHT, xdiff, ev.button.x, ev.button.y);
						}
						else
						{
							printf("mouse: slide to left\n");
							result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDELEFT, xdiff, ev.button.x, ev.button.y);
						}
					}
					else if (ydiff >= GESTURE_THRESHOLD)
					{
						if (ev.button.y > lasty)
						{
							printf("mouse: slide to down\n");
							result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDEDOWN, ydiff, ev.button.x, ev.button.y);
						}
						else
						{
							printf("mouse: slide to up\n");
							result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDEUP, ydiff, ev.button.x, ev.button.y);
						}
					}
                }

				printf("mouse: up %d, %d\n", ev.button.x, ev.button.y);
				result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEUP, ev.button.button, ev.button.x, ev.button.y);

                break;

            case SDL_FINGERDOWN:
                printf("touch: down %d, %d\n", ev.tfinger.x, ev.tfinger.y);
                {
                    uint32_t t = SDL_GetTicks();
#ifdef DOUBLE_KEY_ENABLE
                    if (t - dblclk <= 300)
                    {
                        result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEDOUBLECLICK, 1, ev.tfinger.x, ev.tfinger.y);
                        dblclk = 0;
                    }
                    else
#endif
                    {
                        result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEDOWN, 1, ev.tfinger.x, ev.tfinger.y);
                        dblclk = t;
                        lastx = ev.tfinger.x;
                        lasty = ev.tfinger.y;
                    }
#ifndef _WIN32
                    if (result)
                        AudioPlayKeySound();
#endif
                }
                break;

            case SDL_FINGERMOTION:
                printf("touch: move %d, %d\n", ev.tfinger.x, ev.tfinger.y);
                result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEMOVE, 1, ev.tfinger.x, ev.tfinger.y);
                break;

            case SDL_FINGERUP:
                printf("touch: up %d, %d\n", ev.tfinger.x, ev.tfinger.y);
                if (SDL_GetTicks() - dblclk <= 300)
                {
					int xdiff = abs(ev.tfinger.x - lastx);
					int ydiff = abs(ev.tfinger.y - lasty);

					if (xdiff >= GESTURE_THRESHOLD && xdiff > ydiff)
					{
						if (ev.tfinger.x > lastx)
						{
							printf("touch: slide to right %d %d\n", ev.tfinger.x, ev.tfinger.y);
							result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDERIGHT, xdiff, ev.tfinger.x, ev.tfinger.y);
						}
						else
						{
							printf("touch: slide to left %d %d\n", ev.tfinger.x, ev.tfinger.y);
							result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDELEFT, xdiff, ev.tfinger.x, ev.tfinger.y);
						}
					}
					else if (ydiff >= GESTURE_THRESHOLD)
					{
						if (ev.tfinger.y > lasty)
						{
							printf("touch: slide to down %d %d\n", ev.tfinger.x, ev.tfinger.y);
							result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDEDOWN, ydiff, ev.tfinger.x, ev.tfinger.y);
						}
						else
						{
							printf("touch: slide to up %d %d\n", ev.tfinger.x, ev.tfinger.y);
							result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDEUP, ydiff, ev.tfinger.x, ev.tfinger.y);
						}
					}
                }
				
				result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEUP, 1, ev.tfinger.x, ev.tfinger.y);

                break;
				/*
            case SDL_SLIDEGESTURE:
                switch (ev.dgesture.gestureId)
                {
                case SDL_TG_LEFT:
                    printf("touch: slide to left\n");
                    result = ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDELEFT, 1, ev.dgesture.x, ev.dgesture.y);
                    break;

                case SDL_TG_UP:
                    printf("touch: slide to up\n");
                    result = ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDEUP, 1, ev.dgesture.x, ev.dgesture.y);
                    break;

                case SDL_TG_RIGHT:
                    printf("touch: slide to right\n");
                    result = ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDERIGHT, 1, ev.dgesture.x, ev.dgesture.y);
                    break;

                case SDL_TG_DOWN:
                    printf("touch: slide to down\n");
                    result = ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDEDOWN, 1, ev.dgesture.x, ev.dgesture.y);
                    break;
                }
                break;
				*/
            case SDL_QUIT:
                done = 1;
                break;
            }
            lastev = ev;
        }
        
        result |= ituSceneUpdate(&theScene, ITU_EVENT_TIMER, 0, 0, 0);
#ifndef _WIN32
        if (result)
#endif
        {
            ituSceneDraw(&theScene, screenSurf);
            ituFlip(screenSurf);
        }

        delay = periodPerFrame - (SDL_GetTicks() - tick);
        //printf("scene loop delay=%d\n", delay);
        if (delay > 0)
        {
            SDL_Delay(delay);
        }
        else
            sched_yield();

        //delay = 33 - (SDL_GetTicks() - tick);
        //if (delay > 0)
        //{
        //    SDL_Delay(delay);
        //}
    }

	//SDL_Quit();
    return 0;
}

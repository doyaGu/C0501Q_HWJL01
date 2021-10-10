#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "SDL/SDL.h"
#include "audio_mgr.h"
#include "ite/itu.h"

#define GESTURE_THRESHOLD           40
#define MOUSEDOWN_LONGPRESS_DELAY   1000

static ITUScene scene;
static ITUSurface* lcdSurf;

static bool IconUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    if (ev == ITU_EVENT_TOUCHPINCH)
    {
        int x = ituWidgetGetX(widget) - arg1 / 2;
        int y = ituWidgetGetY(widget) - arg1 / 2;
        int width = ituWidgetGetWidth(widget) + arg1;
        int height = ituWidgetGetHeight(widget) + arg1;

        if (width > 0 && width <= lcdSurf->width && height > 0 && height <= lcdSurf->height)
        {
            ituWidgetSetPosition(widget, x, y);
            ituWidgetSetDimension(widget, width, height);
        }
    }
    return ituIconUpdate(widget, ev, arg1, arg2, arg3);
}

int SDL_main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Event ev;
    float screenDistance;
    ITURotation lcdRotation;
    int dist, done, delay, frames, lastx, lasty;
    uint32_t tick, dblclk, lasttick, mouseDownTick;
    ITUIcon* icon;

    // wait mouting USB storage
#ifndef _WIN32
    sleep(5);
#endif

    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        return (1);
    }
    window = SDL_CreateWindow("ITU MULTIGESTURE Test",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              800, 480, 0);
    if (!window)
    {
        printf("Couldn't create 800x480 window: %s\n",
                SDL_GetError());
        SDL_Quit();
        exit(2);
    }

    // init itu
    ituLcdInit();
    lcdSurf = ituGetDisplaySurface();
    lcdRotation = ITU_ROT_0;

    screenDistance = sqrtf(lcdSurf->width * lcdSurf->width + lcdSurf->height * lcdSurf->height);

#ifdef CFG_M2D_ENABLE
    ituM2dInit();
#else
    ituSWInit();
#endif

    ituFtInit();
    ituFtLoadFont(0, CFG_PRIVATE_DRIVE ":/font/WenQuanYiMicroHeiMono.ttf", ITU_GLYPH_8BPP);

    // load itu file
    tick = SDL_GetTicks();

    ituSceneInit(&scene, NULL);
    ituSceneLoadFile(&scene, CFG_PRIVATE_DRIVE ":/multigesture.itu");

    printf("loading time: %dms\n", SDL_GetTicks() - tick);

    scene.leftKey   = SDLK_LEFT;
    scene.upKey     = SDLK_UP;
    scene.rightKey  = SDLK_RIGHT;
    scene.downKey   = SDLK_DOWN;

    icon = ituSceneFindWidget(&scene, "Icon3");
    assert(icon);
    ituWidgetSetUpdate(icon, IconUpdate);

    /* Watch keystrokes */
    dist = done = dblclk = frames = lasttick = lastx = lasty = mouseDownTick = 0;
    while (!done)
    {
        bool result = false;

        tick = SDL_GetTicks();
        frames++;
        if (tick - lasttick >= 1000)
        {
            printf("fps: %d\n", frames);
            frames = 0;
            lasttick = tick;
        }

        while (SDL_PollEvent(&ev))
        {
            switch (ev.type)
            {
            case SDL_KEYDOWN:
                result = ituSceneUpdate(&scene, ITU_EVENT_KEYDOWN, ev.key.keysym.sym, 0, 0);
                switch (ev.key.keysym.sym)
                {
                case SDLK_KP_MINUS:
                    if (dist > 0)
                        dist = 0;

                    dist -= 10;
                    result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHPINCH, dist, lcdSurf->width / 2, lcdSurf->height / 2);
                    break;

                case SDLK_KP_PLUS:
                case SDLK_TAB:
                    if (dist < 0)
                        dist = 0;

                    dist += 10;
                    result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHPINCH, dist, lcdSurf->width / 2, lcdSurf->height / 2);
                    break;

                case SDLK_PAGEUP:
                    if (--lcdRotation < ITU_ROT_0)
                        lcdRotation = ITU_ROT_270;

                    ituSceneSetRotation(&scene, lcdRotation, CFG_LCD_WIDTH, CFG_LCD_HEIGHT);
                    result = true;
                    break;

                case SDLK_PAGEDOWN:
                    if (++lcdRotation > ITU_ROT_270)
                        lcdRotation = ITU_ROT_0;

                    ituSceneSetRotation(&scene, lcdRotation, CFG_LCD_WIDTH, CFG_LCD_HEIGHT);
                    result = true;
                    break;
                }
                break;

            case SDL_KEYUP:
                result = ituSceneUpdate(&scene, ITU_EVENT_KEYUP, ev.key.keysym.sym, 0, 0);
                break;

            case SDL_MOUSEMOTION:
                result = ituSceneUpdate(&scene, ITU_EVENT_MOUSEMOVE, ev.button.button, ev.button.x, ev.button.y);
                break;

            case SDL_MOUSEBUTTONDOWN:
                {
                    mouseDownTick = SDL_GetTicks();
                    if (mouseDownTick - dblclk <= 300)
                    {
                        result = ituSceneUpdate(&scene, ITU_EVENT_MOUSEDOUBLECLICK, ev.button.button, ev.button.x, ev.button.y);
                        dblclk = mouseDownTick = 0;
                    }
                    else
                    {
                        result = ituSceneUpdate(&scene, ITU_EVENT_MOUSEDOWN, ev.button.button, ev.button.x, ev.button.y);
                        dblclk = mouseDownTick;
                        lastx = ev.button.x;
                        lasty = ev.button.y;
                    }
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (SDL_GetTicks() - dblclk <= 300)
                {
                    int xdiff = abs(ev.button.x - lastx);
                    int ydiff = abs(ev.button.y - lasty);

                    if (xdiff >= GESTURE_THRESHOLD)
                    {
                        if (ev.button.x > lastx)
                        {
                            printf("mouse: slide to right\n");
                            result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDERIGHT, 1, ev.button.x, ev.button.y);
                        }
                        else
                        {
                            printf("mouse: slide to left\n");
                            result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDELEFT, 1, ev.button.x, ev.button.y);
                        }
                    }
                    else if (ydiff >= GESTURE_THRESHOLD)
                    {
                        if (ev.button.y > lasty)
                        {
                            printf("mouse: slide to down\n");
                            result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDEDOWN, 1, ev.button.x, ev.button.y);
                        }
                        else
                        {
                            printf("mouse: slide to up\n");
                            result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDEUP, 1, ev.button.x, ev.button.y);
                        }
                    }
                }
                result |= ituSceneUpdate(&scene, ITU_EVENT_MOUSEUP, ev.button.button, ev.button.x, ev.button.y);
                mouseDownTick = 0;
                break;

            case SDL_FINGERMOTION:
                printf("touch: move %d, %d\n", ev.tfinger.x, ev.tfinger.y);
                result = ituSceneUpdate(&scene, ITU_EVENT_MOUSEMOVE, 1, ev.tfinger.x, ev.tfinger.y);
                break;

            case SDL_FINGERDOWN:
                printf("touch: down %d, %d\n", ev.tfinger.x, ev.tfinger.y);
                {
                    mouseDownTick = SDL_GetTicks();
                    if (mouseDownTick - dblclk <= 300)
                    {
                        result = ituSceneUpdate(&scene, ITU_EVENT_MOUSEDOUBLECLICK, 1, ev.tfinger.x, ev.tfinger.y);
                        dblclk = mouseDownTick = 0;
                    }
                    else
                    {
                        result = ituSceneUpdate(&scene, ITU_EVENT_MOUSEDOWN, 1, ev.tfinger.x, ev.tfinger.y);
                        dblclk = mouseDownTick;
                        lastx = ev.tfinger.x;
                        lasty = ev.tfinger.y;
                    }
                }
                break;

            case SDL_FINGERUP:
                printf("touch: up %d, %d\n", ev.tfinger.x, ev.tfinger.y);
                if (SDL_GetTicks() - dblclk <= 300)
                {
                    int xdiff = abs(ev.tfinger.x - lastx);
                    int ydiff = abs(ev.tfinger.y - lasty);

                    if (xdiff >= GESTURE_THRESHOLD)
                    {
                        if (ev.tfinger.x > lastx)
                        {
                            printf("touch: slide to right %d %d\n", ev.tfinger.x, ev.tfinger.y);
                            result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDERIGHT, 1, ev.tfinger.x, ev.tfinger.y);
                        }
                        else
                        {
                            printf("touch: slide to left %d %d\n", ev.button.x, ev.button.y);
                            result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDELEFT, 1, ev.tfinger.x, ev.tfinger.y);
                        }
                    }
                    else if (ydiff >= GESTURE_THRESHOLD)
                    {
                        if (ev.tfinger.y > lasty)
                        {
                            printf("touch: slide to down %d %d\n", ev.tfinger.x, ev.tfinger.y);
                            result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDEDOWN, 1, ev.tfinger.x, ev.tfinger.y);
                        }
                        else
                        {
                            printf("touch: slide to up %d %d\n", ev.tfinger.x, ev.tfinger.y);
                            result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDEUP, 1, ev.tfinger.x, ev.tfinger.y);
                        }
                    }
                }
                result |= ituSceneUpdate(&scene, ITU_EVENT_MOUSEUP, 1, ev.tfinger.x, ev.tfinger.y);
                mouseDownTick = 0;
                break;

            case SDL_MULTIGESTURE:
                printf("touch: multi %d, %d, %f\n", ev.mgesture.x, ev.mgesture.y, ev.mgesture.dDist);
                if (ev.mgesture.dDist > 0.002f || ev.mgesture.dDist < -0.002f)
                {
                    int dist = (int)(screenDistance * ev.mgesture.dDist);
                    int x = (int)(lcdSurf->width * ev.mgesture.x);
                    int y = (int)(lcdSurf->height * ev.mgesture.y);
                    result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHPINCH, dist, x, y);
                }
                break;

            case SDL_QUIT:
                done = 1;
                break;
            }
        }

        if (mouseDownTick > 0 && (SDL_GetTicks() - mouseDownTick >= MOUSEDOWN_LONGPRESS_DELAY))
        {
            result |= ituSceneUpdate(&scene, ITU_EVENT_MOUSELONGPRESS, 1, lastx, lasty);
            mouseDownTick = 0;
        }

        result |= ituSceneUpdate(&scene, ITU_EVENT_TIMER, 0, 0, 0);
    #ifndef _WIN32
        if (result)
    #endif
        {
            ituSceneDraw(&scene, lcdSurf);
            ituFlip(lcdSurf);
        }

        delay = 17 - (SDL_GetTicks() - tick);
        if (delay > 0)
        {
            SDL_Delay(delay);
        }
    }

    SDL_Quit();
    return (0);
}

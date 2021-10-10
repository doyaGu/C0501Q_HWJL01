#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "SDL/SDL.h"
#include "ite/itu.h"

#define GESTURE_THRESHOLD           40
#define MOUSEDOWN_LONGPRESS_DELAY   1000

static ITUScene scene;

static ITUMediaFileListBox* mediaFileListBox;
static ITUSlideshow* slideshow;
static ITUIcon* icon1;
static ITUIcon* icon2;

static bool OnTimer(ITUWidget* widget, char* param)
{
    if ((mediaFileListBox->mflistboxFlags & ITU_FILELIST_BUSYING) == 0 &&
        (mediaFileListBox->mflistboxFlags & ITU_FILELIST_CREATED) == 0 &&
        (mediaFileListBox->mflistboxFlags & ITU_FILELIST_DESTROYING) == 0)
    {
        if (slideshow->delayCount == slideshow->delay / 2)
        {
            ITUScrollText* item = ituMediaFileListNext(mediaFileListBox);
            if (item)
            {
                char* filepath = (char*)ituWidgetGetCustomData(item);

                if (!ituWidgetIsVisible(icon1))
                {
                    ituIconLoadJpegFile(icon1, filepath);
                }
                else
                {
                    ituIconLoadJpegFile(icon2, filepath);
                }
                return true;
            }
        }
    }
    return false;
}

static ITUActionFunction actionFunctions[] =
{
    "OnTimer", OnTimer,
    NULL, NULL
};

int SDL_main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Event ev;
    int done, delay, frames, lastx, lasty;
    uint32_t tick, dblclk, lasttick, mouseDownTick;

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
    window = SDL_CreateWindow("ITU fileslidesow Test",
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

    ituSceneInit(&scene, NULL);
    ituSceneSetFunctionTable(&scene, actionFunctions);
    ituSceneLoadFile(&scene, CFG_PRIVATE_DRIVE ":/fileslideshow.itu");

    printf("loading time: %dms\n", SDL_GetTicks() - tick);

    scene.upKey     = SDLK_UP;
    scene.downKey   = SDLK_DOWN;

    mediaFileListBox = (ITUMediaFileListBox*)ituSceneFindWidget(&scene, "MediaFileListBox1");
    mediaFileListBox->repeatMode = ITU_MEDIA_REPEAT_ALL;

    slideshow = (ITUSlideshow*)ituSceneFindWidget(&scene, "Slideshow3");
    icon1 = (ITUIcon*)ituSceneFindWidget(&scene, "Icon4");
    icon2 = (ITUIcon*)ituSceneFindWidget(&scene, "Icon5");

    /* Watch keystrokes */
    done = dblclk = frames = lasttick = lastx = lasty = mouseDownTick = 0;
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
                case SDLK_TAB:
                    ituSceneFocusNext(&scene);
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
            ituSceneDraw(&scene, ituGetDisplaySurface());
            ituFlip(ituGetDisplaySurface());
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

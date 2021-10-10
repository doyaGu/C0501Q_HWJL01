#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL/SDL.h"
#include "ite/itu.h"

static ITUScene scene;

static bool OnPress(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    static char msg[256];

    ITUText* txt = (ITUText*)ituSceneFindWidget(&scene, "Text8");
    if (txt)
    {
        ITUButton* btn = (ITUButton*) widget;

        if ((ev == ITU_EVENT_KEYDOWN && arg1 == SDLK_RETURN))
        {
            strcpy(msg, widget->name);
            strcat(msg, ": KEY DOWN");
            ituTextSetString(txt, msg);
            return true;
        }
        else if (ev == ITU_EVENT_MOUSEDOWN)
        {
            strcpy(msg, widget->name);
            strcat(msg, ": MOUSE DOWN");
            ituTextSetString(txt, msg);
            return true;
        }
        else if (ev == ITU_EVENT_MOUSEUP)
        {
            if (ituButtonIsPressed(btn))
            {
                strcpy(msg, widget->name);
                strcat(msg, ": MOUSE UP");
                ituTextSetString(txt, msg);
                return true;
            }
        }
    }
    return false;
}

int SDL_main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Event ev;
    int done, delay;
    ITUIcon* cursor;
    uint32_t tick, dblclk;
    ITUWidget* widget;

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
    window = SDL_CreateWindow("ITU Cursor Test",
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
    ituFtLoadFont(0, CFG_PRIVATE_DRIVE ":/font/DroidSans.ttf", ITU_GLYPH_8BPP);

    // load itu file
    tick = SDL_GetTicks();

    ituSceneInit(&scene, NULL);
    ituSceneLoadFile(&scene, CFG_PRIVATE_DRIVE ":/cursor.itu");

    printf("loading time: %dms\n", SDL_GetTicks() - tick);

    scene.upKey     = SDLK_UP;
    scene.downKey   = SDLK_DOWN;

    // get cursor icon
    cursor = (ITUIcon*)ituSceneFindWidget(&scene, "cursor");

    // customize button behavior
    widget = ituSceneFindWidget(&scene, "Button2");
    if (widget)
        ituWidgetSetOnPress(widget, OnPress);

    widget = ituSceneFindWidget(&scene, "CheckBox3");
    if (widget)
        ituWidgetSetOnPress(widget, OnPress);

    widget = ituSceneFindWidget(&scene, "CheckBox4");
    if (widget)
        ituWidgetSetOnPress(widget, OnPress);

    widget = ituSceneFindWidget(&scene, "RadioBox5");
    if (widget)
        ituWidgetSetOnPress(widget, OnPress);

    widget = ituSceneFindWidget(&scene, "RadioBox6");
    if (widget)
        ituWidgetSetOnPress(widget, OnPress);

    widget = ituSceneFindWidget(&scene, "RadioBox7");
    if (widget)
        ituWidgetSetOnPress(widget, OnPress);

    /* Watch keystrokes */
    done = dblclk = 0;
    while (!done)
    {
        tick = SDL_GetTicks();

        while (SDL_PollEvent(&ev))
        {
            switch (ev.type)
            {
            case SDL_KEYDOWN:
                ituSceneUpdate(&scene, ITU_EVENT_KEYDOWN, ev.key.keysym.sym, 0, 0);
                switch (ev.key.keysym.sym)
                {
                case SDLK_TAB:
                    ituSceneFocusNext(&scene);
                    break;
                }
                break;

            case SDL_KEYUP:
                ituSceneUpdate(&scene, ITU_EVENT_KEYUP, ev.key.keysym.sym, 0, 0);
                break;

            case SDL_MOUSEMOTION:
                if (cursor)
                {
                    cursor->widget.rect.x = ev.button.x;
                    cursor->widget.rect.y = ev.button.y;
                    cursor->widget.dirty = true;
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                {
                    uint32_t t = SDL_GetTicks();
                    if (t - dblclk <= 300)
                    {
                        ituSceneUpdate(&scene, ITU_EVENT_MOUSEDOUBLECLICK, ev.button.button, ev.button.x, ev.button.y);
                        dblclk = 0;
                    }
                    else
                    {
                        ituSceneUpdate(&scene, ITU_EVENT_MOUSEDOWN, ev.button.button, ev.button.x, ev.button.y);
                        dblclk = t;
                    }
                }
                break;

            case SDL_MOUSEBUTTONUP:
                ituSceneUpdate(&scene, ITU_EVENT_MOUSEUP, ev.button.button, ev.button.x, ev.button.y);
                break;

            case SDL_QUIT:
                done = 1;
                break;
            }
        }

        if (ituSceneUpdate(&scene, ITU_EVENT_TIMER, 0, 0, 0))
        {
            ituSceneDraw(&scene, ituGetDisplaySurface());
            ituFlip(ituGetDisplaySurface());
        }

        delay = 33 - (SDL_GetTicks() - tick);
        if (delay > 0)
        {
            SDL_Delay(delay);
        }
    }

    SDL_Quit();
    return (0);
}

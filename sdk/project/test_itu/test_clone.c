#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL/SDL.h"
#include "ite/itu.h"

static ITUScene scene;

int SDL_main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Event ev;
    int done, delay;
    uint32_t tick, dblclk;
    ITUBackground* bg = NULL;
    ITUIcon* icon = NULL;
    ITUIcon* iconCloned = NULL;
    ITUBackground* bg2 = NULL;
    ITUBackground* bgCloned = NULL;
    ITUText* text2 = NULL;
    ITUText* textCloned = NULL;
    ITUTextBox* textbox2 = NULL;
    ITUTextBox* textboxCloned = NULL;
    ITUButton* btn2 = NULL;
    ITUButton* btnCloned = NULL;
    ITUCheckBox* checkbox2 = NULL;
    ITUCheckBox* checkBoxCloned = NULL;
    ITURadioBox* radiobox2 = NULL;
    ITURadioBox* radioBoxCloned = NULL;
    ITUScrollText* stext2 = NULL;
    ITUScrollText* stextCloned = NULL;
    ITUPopupButton* pbtn2 = NULL;
    ITUPopupButton* pbtnCloned = NULL;
    ITUSprite* sprite2 = NULL;
    ITUSprite* spriteCloned = NULL;
    bool result;

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
    window = SDL_CreateWindow("ITU Clone Test",
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
    ituSceneLoadFile(&scene, CFG_PRIVATE_DRIVE ":/clone.itu");

    printf("loading time: %dms\n", SDL_GetTicks() - tick);

    scene.upKey     = SDLK_UP;
    scene.downKey   = SDLK_DOWN;

    bg = (ITUBackground*)ituSceneFindWidget(&scene, "Background1");

    icon = (ITUIcon*)ituSceneFindWidget(&scene, "Icon2");
    result = ituWidgetClone(icon, &iconCloned);
    if (result)
    {
        ituWidgetSetName(iconCloned, "IconCloned");
        ituWidgetSetX(iconCloned, 0);
        ituWidgetAdd(bg, iconCloned);

        ituSceneUpdate(&scene, ITU_EVENT_LAYOUT, 0, 0, 0);
    }

    bg2 = (ITUBackground*)ituSceneFindWidget(&scene, "Background2");
    result = ituWidgetClone(bg2, &bgCloned);
    if (result)
    {
        ituWidgetSetX(bgCloned, 0);
        ituWidgetAdd(bg, bgCloned);
    }

    text2 = (ITUText*)ituSceneFindWidget(&scene, "Text3");
    ituTextSetString(text2, "Test");
    result = ituWidgetClone(text2, &textCloned);
    if (result)
    {
        ituWidgetSetX(textCloned, 0);
        ituWidgetAdd(bg, textCloned);
    }

    textbox2 = (ITUTextBox*)ituSceneFindWidget(&scene, "TextBox4");
    result = ituWidgetClone(textbox2, &textboxCloned);
    if (result)
    {
        ituWidgetSetX(textboxCloned, 0);
        ituWidgetAdd(bg, textboxCloned);
    }

    btn2 = (ITUButton*)ituSceneFindWidget(&scene, "Button5");
    result = ituWidgetClone(btn2, &btnCloned);
    if (result)
    {
        ituWidgetSetX(btnCloned, 0);
        ituWidgetAdd(bg, btnCloned);
    }

    checkbox2 = (ITUCheckBox*)ituSceneFindWidget(&scene, "CheckBox1");
    result = ituWidgetClone(checkbox2, &checkBoxCloned);
    if (result)
    {
        ituWidgetSetX(checkBoxCloned, 0);
        ituWidgetAdd(bg, checkBoxCloned);
    }

    radiobox2 = (ITURadioBox*)ituSceneFindWidget(&scene, "RadioBox2");
    result = ituWidgetClone(radiobox2, &radioBoxCloned);
    if (result)
    {
        ituWidgetSetX(radioBoxCloned, 0);
        ituWidgetAdd(bg, radioBoxCloned);
    }

    stext2 = (ITUScrollText*)ituSceneFindWidget(&scene, "ScrollText3");
    result = ituWidgetClone(stext2, &stextCloned);
    if (result)
    {
        ituWidgetSetX(stextCloned, 0);
        ituWidgetAdd(bg, stextCloned);
    }

    pbtn2 = (ITUPopupButton*)ituSceneFindWidget(&scene, "PopupButton4");
    result = ituWidgetClone(pbtn2, &pbtnCloned);
    if (result)
    {
        ituWidgetSetX(pbtnCloned, 0);
        ituWidgetAdd(bg, pbtnCloned);
    }

    sprite2 = (ITUSprite*)ituSceneFindWidget(&scene, "Sprite1");
    result = ituWidgetClone(sprite2, &spriteCloned);
    if (result)
    {
        ituWidgetSetX(spriteCloned, 0);
        ituWidgetAdd(bg, spriteCloned);
    }

    ituSceneUpdate(&scene, ITU_EVENT_LAYOUT, 0, 0, 0);

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

        result = ituSceneUpdate(&scene, ITU_EVENT_TIMER, 0, 0, 0);
    #ifndef _WIN32
        if (result)
    #endif
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

    itcTreeRemove(bgCloned);
    ituWidgetExit(bgCloned);

    ituSceneExit(&scene);
    SDL_Quit();
    return (0);
}

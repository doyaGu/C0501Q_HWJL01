#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "SDL/SDL.h"
#include "ite/itu.h"

static ITUScene scene;
static ITUAnimation *Animation1Top;
static ITUAnimation *Animation2Top;
static ITUAnimation *Animation3Top;
static ITUIcon *Icon1TopS;
static ITUIcon *Icon2TopS;
static ITUIcon *Icon3TopS;
static ITUIcon *Icon1TopM;
static ITUIcon *Icon2TopM;
static ITUIcon *Icon3TopM;

static ITUAnimation *Animation1Bottom;
static ITUAnimation *Animation2Bottom;
static ITUAnimation *Animation3Bottom;
static ITUIcon *Icon1BottomS;
static ITUIcon *Icon2BottomS;
static ITUIcon *Icon3BottomS;
static ITUIcon *Icon1BottomM;
static ITUIcon *Icon2BottomM;
static ITUIcon *Icon3BottomM;

static bool AnimationOnStopped(ITUWidget* widget, char* param)
{
	ITUSurface* surf;
	ITUSurface* surf2;
	ITUSurface* surf3;
	ITUSurface* surf4;
	
	if (param != NULL)
	{
		if (atoi(param) == 1)
		{
			
			surf = Icon3TopM->surf;
			Icon3TopM->surf = Icon2TopM->surf;
			Icon2TopM->surf = Icon1TopM->surf;
			Icon1TopM->surf = surf;

			surf2 = Icon3TopS->surf;
			Icon3TopS->surf = Icon2TopS->surf;
			Icon2TopS->surf = Icon1TopS->surf;
			Icon1TopS->surf = surf2;
			

			surf3 = Icon3TopM->staticSurf;
			Icon3TopM->staticSurf = Icon2TopM->staticSurf;
			Icon2TopM->staticSurf = Icon1TopM->staticSurf;
			Icon1TopM->staticSurf = surf3;

			surf4 = Icon3TopS->staticSurf;
			Icon3TopS->staticSurf = Icon2TopS->staticSurf;
			Icon2TopS->staticSurf = Icon1TopS->staticSurf;
			Icon1TopS->staticSurf = surf4;

			printf("A 1\n");
		}
		else if (atoi(param) == 2)
		{
			
			surf = Icon1TopM->surf;
			Icon1TopM->surf = Icon2TopM->surf;
			Icon2TopM->surf = Icon3TopM->surf;
			Icon3TopM->surf = surf;

			surf2 = Icon1TopS->surf;
			Icon1TopS->surf = Icon2TopS->surf;
			Icon2TopS->surf = Icon3TopS->surf;
			Icon3TopS->surf = surf2;
			

			surf3 = Icon1TopM->staticSurf;
			Icon1TopM->staticSurf = Icon2TopM->staticSurf;
			Icon2TopM->staticSurf = Icon3TopM->staticSurf;
			Icon3TopM->staticSurf = surf3;

			surf4 = Icon1TopS->staticSurf;
			Icon1TopS->staticSurf = Icon2TopS->staticSurf;
			Icon2TopS->staticSurf = Icon3TopS->staticSurf;
			Icon3TopS->staticSurf = surf4;

			printf("A 2\n");
		}

		if ((atoi(param) == 1) || (atoi(param) == 2))
		{
			ituAnimationReset(Animation1Top);
			ituAnimationReset(Animation2Top);
			ituAnimationReset(Animation3Top);
		}
	}

	return true;
}

static bool Animation2BottomOnStopped(ITUWidget* widget, char* param)
{
	ITUSurface* surf;
	ITUSurface* surf2;
	ITUSurface* surf3;
	ITUSurface* surf4;

	if (param != NULL)
	{
		if (atoi(param) == 1)
		{
			
			surf = Icon3BottomM->surf;
			Icon3BottomM->surf = Icon2BottomM->surf;
			Icon2BottomM->surf = Icon1BottomM->surf;
			Icon1BottomM->surf = surf;

			surf2 = Icon3BottomS->surf;
			Icon3BottomS->surf = Icon2BottomS->surf;
			Icon2BottomS->surf = Icon1BottomS->surf;
			Icon1BottomS->surf = surf2;
			

			surf3 = Icon3BottomM->staticSurf;
			Icon3BottomM->staticSurf = Icon2BottomM->staticSurf;
			Icon2BottomM->staticSurf = Icon1BottomM->staticSurf;
			Icon1BottomM->staticSurf = surf3;

			surf4 = Icon3BottomS->staticSurf;
			Icon3BottomS->staticSurf = Icon2BottomS->staticSurf;
			Icon2BottomS->staticSurf = Icon1BottomS->staticSurf;
			Icon1BottomS->staticSurf = surf4;

			printf("B 1\n");
		}
		else if (atoi(param) == 2)
		{
			
			surf = Icon1BottomM->surf;
			Icon1BottomM->surf = Icon2BottomM->surf;
			Icon2BottomM->surf = Icon3BottomM->surf;
			Icon3BottomM->surf = surf;

			surf2 = Icon1BottomS->surf;
			Icon1BottomS->surf = Icon2BottomS->surf;
			Icon2BottomS->surf = Icon3BottomS->surf;
			Icon3BottomS->surf = surf2;
			

			surf3 = Icon1BottomM->staticSurf;
			Icon1BottomM->staticSurf = Icon2BottomM->staticSurf;
			Icon2BottomM->staticSurf = Icon3BottomM->staticSurf;
			Icon3BottomM->staticSurf = surf3;

			surf4 = Icon1BottomS->staticSurf;
			Icon1BottomS->staticSurf = Icon2BottomS->staticSurf;
			Icon2BottomS->staticSurf = Icon3BottomS->staticSurf;
			Icon3BottomS->staticSurf = surf4;

			printf("B 2\n");
		}

		if ((atoi(param) == 1) || (atoi(param) == 2))
		{
			ituAnimationReset(Animation1Bottom);
			ituAnimationReset(Animation2Bottom);
			ituAnimationReset(Animation3Bottom);
		}
	}

	return true;
}

static ITUActionFunction actionFunctions[] =
{
    "AnimationOnStopped",           AnimationOnStopped,
    "Animation2BottomOnStopped",    Animation2BottomOnStopped,
    NULL, NULL
};

int SDL_main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Event ev;
    int done, delay;
    uint32_t tick, dblclk;

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
    window = SDL_CreateWindow("ITU AnimPage Test",
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
    ituSceneSetFunctionTable(&scene, actionFunctions);
    ituSceneLoadFile(&scene, CFG_PRIVATE_DRIVE ":/animpage.itu");

    printf("loading time: %dms\n", SDL_GetTicks() - tick);

    scene.upKey         = SDLK_UP;
    scene.downKey       = SDLK_DOWN;

	Animation1Top = (ITUAnimation*)ituSceneFindWidget(&scene, "Animation1Top");
	assert(Animation1Top);

	Animation2Top = (ITUAnimation*)ituSceneFindWidget(&scene, "Animation2Top");
	assert(Animation2Top);

	Animation3Top = (ITUAnimation*)ituSceneFindWidget(&scene, "Animation3Top");
	assert(Animation3Top);

	Icon1TopS = (ITUIcon*)ituSceneFindWidget(&scene, "Icon1TopS");
	assert(Icon1TopS);

	Icon2TopS = (ITUIcon*)ituSceneFindWidget(&scene, "Icon2TopS");
	assert(Icon2TopS);

	Icon3TopS = (ITUIcon*)ituSceneFindWidget(&scene, "Icon3TopS");
	assert(Icon3TopS);

	Icon1TopM = (ITUIcon*)ituSceneFindWidget(&scene, "Icon1TopM");
	assert(Icon1TopM);

	Icon2TopM = (ITUIcon*)ituSceneFindWidget(&scene, "Icon2TopM");
	assert(Icon2TopM);

	Icon3TopM = (ITUIcon*)ituSceneFindWidget(&scene, "Icon3TopM");
	assert(Icon3TopM);

	Animation1Bottom = (ITUAnimation*)ituSceneFindWidget(&scene, "Animation1Bottom");
	assert(Animation1Bottom);

	Animation2Bottom = (ITUAnimation*)ituSceneFindWidget(&scene, "Animation2Bottom");
	assert(Animation2Bottom);

	Animation3Bottom = (ITUAnimation*)ituSceneFindWidget(&scene, "Animation3Bottom");
	assert(Animation3Bottom);

	Icon1BottomS = (ITUIcon*)ituSceneFindWidget(&scene, "Icon1BottomS");
	assert(Icon1BottomS);

	Icon2BottomS = (ITUIcon*)ituSceneFindWidget(&scene, "Icon2BottomS");
	assert(Icon2BottomS);

	Icon3BottomS = (ITUIcon*)ituSceneFindWidget(&scene, "Icon3BottomS");
	assert(Icon3BottomS);

	Icon1BottomM = (ITUIcon*)ituSceneFindWidget(&scene, "Icon1BottomM");
	assert(Icon1BottomM);

	Icon2BottomM = (ITUIcon*)ituSceneFindWidget(&scene, "Icon2BottomM");
	assert(Icon2BottomM);

	Icon3BottomM = (ITUIcon*)ituSceneFindWidget(&scene, "Icon3BottomM");
	assert(Icon3BottomM);


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

#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/itp.h"
#include "ite/itu.h"
#include "SDL/SDL.h"

#include "qrencode.h"
#include "qrenc.h"


//	-------------------------------------------------------
//	DEFines
//	-------------------------------------------------------

#define QRCODE_TEXT					"User:Sampo\nDate:2017.03.08\nhttp://www.ite.com.tw"		// Text to encode into QRCode
#define OUT_FILE					"A:/123.png"								// Output file name

//#pragma pack(pop)
//	-------------------------------------------------------

static ITUScene scene;

static bool OnPress(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
	static char msg[256];
	ITUTextBox* txtbox = (ITUTextBox*)ituSceneFindWidget(&scene, "TextBox");
	ITUIcon* icon = (ITUIcon*)ituSceneFindWidget(&scene, "Icon");
	if (icon)
	{
		ITUButton* btn = (ITUButton*)widget;

		if ((ev == ITU_EVENT_KEYDOWN && arg1 == SDLK_RETURN))
		{
			return true;
		}
		else if (ev == ITU_EVENT_MOUSEDOWN)
		{
			if (!strcmp(widget->name, "BtnClear"))
			{
				strcpy(msg, "QR Clear");
				ituTextSetString(txtbox, msg);
				ituIconReleaseSurface(icon);
			}
			else
			{
				strcpy(msg, QRCODE_TEXT);
				ituTextSetString(txtbox, msg);
				ituIconLoadPngFile(icon, OUT_FILE);
			}
			return true;
		}
		else if (ev == ITU_EVENT_MOUSEUP)
		{
			if (ituButtonIsPressed(btn))
			{
				return true;
			}
		}
	}
	return false;
}

//	-------------------------------------------------------
//	Main
//	-------------------------------------------------------

void* TestFunc(void* arg)
{
	itpInit();

	// wait mouting USB storage
#ifndef _WIN32
	sleep(3);
#endif

	// Encode input string & export 123.png to private drive
	qrencode(QRCODE_TEXT, 21, OUT_FILE);



	// SDL start
	SDL_Window *window;
	SDL_Event ev;
	int done, delay;
	uint32_t tick, dblclk;
	ITUWidget* widget;
	ITUIcon* icon;

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
	window = SDL_CreateWindow("ITU Button Test",
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
	ituSceneLoadFile(&scene, CFG_PRIVATE_DRIVE ":/qrencode.itu");

	printf("loading time: %dms\n", SDL_GetTicks() - tick);

	scene.upKey = SDLK_UP;
	scene.downKey = SDLK_DOWN;

	// customize button behavior
	widget = ituSceneFindWidget(&scene, "BtnShow");
	if (widget)
		ituWidgetSetOnPress(widget, OnPress);

	widget = ituSceneFindWidget(&scene, "BtnClear");
	if (widget)
		ituWidgetSetOnPress(widget, OnPress);
	// customize button behavior

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
				printf("\nOuch, you touch me at: (%d, %d)", ev.tfinger.x, ev.tfinger.y);
				ituSceneUpdate(&scene, ITU_EVENT_MOUSEDOWN, 1, ev.tfinger.x, ev.tfinger.y);
				switch (ev.key.keysym.sym)
				{
				case SDLK_TAB:
					ituSceneFocusNext(&scene);
					break;
				}
				break;

			case SDL_FINGERUP:
				ituSceneUpdate(&scene, ITU_EVENT_KEYUP, ev.key.keysym.sym, 0, 0);
				break;

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
	// SDL end

	return NULL;
}


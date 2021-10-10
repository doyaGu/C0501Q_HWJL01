#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "SDL/SDL.h"
#include "ite/itu.h"

int SDL_main(int argc, char *argv[])
{
    SDL_Window *window;
    ITUSurface* lcdSurf;
    ITUSurface* logoSurf;
    uint32_t tick;

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
    window = SDL_CreateWindow("ITU showlogo Test",
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
    lcdSurf = ituGetDisplaySurface();

#ifdef CFG_M2D_ENABLE
    ituM2dInit();
#else
    ituSWInit();
#endif

    // load logo file
    tick = SDL_GetTicks();

    logoSurf = ituJpegLoadFile(lcdSurf->width, lcdSurf->height, "A:/logo.jpg", 0);
    if (logoSurf)
    {
        ituBitBlt(lcdSurf, 0, 0, lcdSurf->width, lcdSurf->height, logoSurf, 0, 0);
        ituFlip(lcdSurf);
    }
    printf("show logo time: %dms\n", SDL_GetTicks() - tick);

#ifdef _WIN32
    sleep(5);
#endif

#ifdef CFG_M2D_ENABLE
    ituM2dExit();
#else
    ituSWExit();
#endif
    ituLcdExit();
    SDL_Quit();
    return (0);
}

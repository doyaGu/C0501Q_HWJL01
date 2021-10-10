#include <sys/ioctl.h>
#include <sys/time.h>
#include <assert.h>
#include <math.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "SDL/SDL.h"
#include "ite/itp.h"
#include "scene.h"
#include "sports_equipment.h"

#ifdef _WIN32
    #include <crtdbg.h>
#endif

//#define FPS_ENABLE
//#define DOUBLE_KEY_ENABLE

#define GESTURE_THRESHOLD           40
#define MAX_COMMAND_QUEUE_SIZE      8
#define MOUSEDOWN_LONGPRESS_DELAY   1000
#define DEMO_DELAY                  (1 * 1000 / MS_PER_FRAME)

extern ITUActionFunction actionFunctions[];
extern void resetScene(void);

// status
static QuitValue quitValue;
static int currDelay = DEMO_DELAY;

// command
typedef enum
{
    CMD_NONE,
    CMD_LOAD_SCENE,
    CMD_CHANGE_LANG,
} CommandID;

#define MAX_STRARG_LEN 32

typedef struct
{
    CommandID   id;
    int         arg1;
    int         arg2;
    char        strarg1[MAX_STRARG_LEN];
} Command;

static mqd_t commandQueue = -1;

// scene
ITUScene theScene;
static SDL_Window *window;
static ITUSurface* screenSurf;
static int screenWidth;
static int screenHeight;
static bool isReady;

void SceneInit(void)
{
    struct mq_attr attr;

    screenWidth = ithLcdGetWidth();
    screenHeight = ithLcdGetHeight();

    window = SDL_CreateWindow("SportsEquipment", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, 0);
    if (!window)
    {
        printf("Couldn't create window: %s\n", SDL_GetError());
        return;
    }

    // init itu
    ituLcdInit();

#ifdef CFG_M2D_ENABLE
    ituM2dInit();
#else
    ituSWInit();
#endif // CFG_M2D_ENABLE

    ituSceneInit(&theScene, NULL);

    screenSurf = ituGetDisplaySurface();

    ituFtInit();
    ituFtLoadFont(0, CFG_PRIVATE_DRIVE ":/font/" CFG_FONT_FILENAME, ITU_GLYPH_8BPP);

	ituSceneSetFunctionTable(&theScene, actionFunctions);

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_COMMAND_QUEUE_SIZE;
    attr.mq_msgsize = sizeof(Command);

    commandQueue = mq_open("scene", O_CREAT | O_NONBLOCK, 0644, &attr);
    assert(commandQueue != -1);

    isReady = false;
}

void SceneExit(void)
{
    mq_close(commandQueue);
    commandQueue = -1;

    resetScene();

    if (theScene.root)
    {
        ituSceneExit(&theScene);
    }
    ituFtExit();

#ifdef CFG_M2D_ENABLE
    ituM2dExit();
#else
    ituSWExit();
#endif // CFG_M2D_ENABLE

    SDL_DestroyWindow(window);
}

void SceneLoad(void)
{
    Command cmd;

    if (commandQueue == -1)
        return;

    isReady  = false;

    cmd.id = CMD_LOAD_SCENE;

    mq_send(commandQueue, (const char*)&cmd, sizeof (Command), 0);
}

void SceneChangeLanguage(void)
{
    Command cmd;

    if (commandQueue == -1)
        return;

    cmd.id     = CMD_CHANGE_LANG;
    mq_send(commandQueue, (const char*)&cmd, sizeof (Command), 0);
}

void SceneSetReady(bool ready)
{
    isReady = ready;
}

static void LoadScene(void)
{
    uint32_t tick1, tick2;

	resetScene();

    if (theScene.root)
    {
        ituSceneExit(&theScene);
    }

    // load itu file
    tick1 = SDL_GetTicks();

    ituSceneLoadFile(&theScene, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/sports_equipment.itu");
    ituSceneUpdate(&theScene, ITU_EVENT_LOAD_IMAGE, (int)CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/", 0, 0);

    tick2 = SDL_GetTicks();
    printf("itu loading time: %dms\n", tick2 - tick1);

    ituSceneUpdate(&theScene, ITU_EVENT_LANGUAGE, theConfig.lang, 0, 0);
    ituSceneUpdate(&theScene, ITU_EVENT_LAYOUT, 0, 0, 0);

    tick1 = tick2;

    tick2 = SDL_GetTicks();
    printf("itu init time: %dms\n", tick2 - tick1);
}

static void ProcessCommand(void)
{
    Command cmd;

    while (mq_receive(commandQueue, (char*)&cmd, sizeof(Command), 0) > 0)
    {
        switch (cmd.id)
        {
        case CMD_LOAD_SCENE:
            LoadScene();            
            break;

        case CMD_CHANGE_LANG:
            ituSceneUpdate(&theScene, ITU_EVENT_LANGUAGE, theConfig.lang, 0, 0);
            ituSceneUpdate(&theScene, ITU_EVENT_LAYOUT, 0, 0, 0);
            break;
        }
    }
}

static void CheckExternal(void)
{
    ExternalEvent ev;
    int ret = ExternalCheck(&ev);

    if (ret)
    {
        MainProcessExternalEvent(&ev);
    }
}

static void CheckStorage(void)
{
    char path[PATH_MAX];

    StorageAction action = StorageCheck();

    switch (action)
    {
    case STORAGE_SD_INSERTED:
        printf("STORAGE_SD_INSERTED\n");
    case STORAGE_USB_INSERTED:
        printf("STORAGE_USB_INSERTED\n");
        strcpy(path, StorageGetDrivePath(StorageGetCurrType()));
        strcat(path, CFG_RES_PATH);
        if (access(path, R_OK) == 0)
            SceneQuit(QUIT_UPGRADE_RESOURCE);
        else
            MainStorageOnInserted(StorageGetDrivePath(StorageGetCurrType()));
        break;

    case STORAGE_SD_REMOVED:
        printf("STORAGE_SD_REMOVED\n");
    case STORAGE_USB_REMOVED:
        printf("STORAGE_USB_REMOVED\n");
        MainStorageOnRemoved();
        break;
    }
}

static bool CheckQuitValue(void)
{
    if (quitValue)
    {
        return true;
    }
    return false;
}

static void AutoDemo(void)
{
#define DEMO_MIN_PULSE (100)
#define DEMO_MAX_PULSE (150)

    static ExternalEventType currEventType = EXTERNAL_PULSE;
    static int currPulse = DEMO_MIN_PULSE;
    static bool pulseDown = false;

    if (--currDelay <= 0)
    {
        ExternalEvent ev;

        switch (currEventType)
        {
        case EXTERNAL_PULSE:
            ev.type = EXTERNAL_PULSE;
            ev.arg1 = currPulse;
            MainProcessExternalEvent(&ev);

            if (pulseDown)
            {
                currPulse -= 10;
                if (currPulse < DEMO_MIN_PULSE)
                {
                    currPulse = DEMO_MIN_PULSE + 10;
                    pulseDown = false;
                }
            }
            else
            {
                currPulse += 10;
                if (currPulse > DEMO_MAX_PULSE)
                {
                    currPulse = DEMO_MAX_PULSE - 10;
                    pulseDown = true;
                }
            }
            break;
        }

        if (++currEventType > EXTERNAL_PULSE)
            currEventType = EXTERNAL_PULSE;

        currDelay = DEMO_DELAY;
    }
}

int SceneRun(void)
{
    SDL_Event ev;
    int delay, frames, lastx, lasty;
    uint32_t tick, dblclk, lasttick, mouseDownTick;

    dblclk = frames = lasttick = lastx = lasty = mouseDownTick = 0;
    for (;;)
    {
        bool result = false;

        if (CheckQuitValue())
            break;

        ProcessCommand();
        CheckExternal();
        CheckStorage();

        if (theConfig.demo_enable)
            AutoDemo();

        tick = SDL_GetTicks();

    #ifdef FPS_ENABLE
        frames++;
        if (tick - lasttick >= 1000)
        {
            printf("fps: %d\n", frames);
            frames = 0;
            lasttick = tick;
        }
    #endif // FPS_ENABLE

        while (SDL_PollEvent(&ev))
        {
            switch (ev.type)
            {
            case SDL_KEYDOWN:
                puts("SDL_KEYDOWN");
                result = ituSceneUpdate(&theScene, ITU_EVENT_KEYDOWN, ev.key.keysym.sym, 0, 0);
                switch (ev.key.keysym.sym)
                {
                case SDLK_RETURN:
                    printf("SDLK_RETURN\n");
                    break;

                case SDLK_UP:
                    printf("SDLK_UP\n");
                    break;

                case SDLK_DOWN:
                    printf("SDLK_DOWN\n");
                    break;

                case SDLK_INSERT:
                    printf("SDLK_INSERT\n");
                    break;

                case SDLK_SPACE:
                    printf("SDLK_SPACE\n");
                    break;

            #ifdef _WIN32
                case SDLK_e:
                    MainStorageOnInserted("T:/");
                    break;
                    
                case SDLK_f:
                    break;

            #endif // _WIN32
                }
                break;

            case SDL_KEYUP:
                puts("SDL_KEYUP");
                result = ituSceneUpdate(&theScene, ITU_EVENT_KEYUP, ev.key.keysym.sym, 0, 0);
                break;

            case SDL_MOUSEMOTION:
                result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEMOVE, ev.button.button, ev.button.x, ev.button.y);
                break;

            case SDL_MOUSEBUTTONDOWN:
                {
                    mouseDownTick = SDL_GetTicks();
                    if (mouseDownTick - dblclk <= 300)
                    {
                        result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEDOUBLECLICK, ev.button.button, ev.button.x, ev.button.y);
                        dblclk = mouseDownTick = 0;
                    }
                    else
                    {
                        result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEDOWN, ev.button.button, ev.button.x, ev.button.y);
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
                result |= ituSceneUpdate(&theScene, ITU_EVENT_MOUSEUP, ev.button.button, ev.button.x, ev.button.y);
                mouseDownTick = 0;
                break;

            case SDL_FINGERMOTION:
                printf("touch: move %d, %d\n", ev.tfinger.x, ev.tfinger.y);
                result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEMOVE, 1, ev.tfinger.x, ev.tfinger.y);
                break;

            case SDL_FINGERDOWN:
                printf("touch: down %d, %d\n", ev.tfinger.x, ev.tfinger.y);
                {
                    mouseDownTick = SDL_GetTicks();
                    if (mouseDownTick - dblclk <= 300)
                    {
                        result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEDOUBLECLICK, 1, ev.tfinger.x, ev.tfinger.y);
                        dblclk = mouseDownTick = 0;
                    }
                    else
                    {
                        result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEDOWN, 1, ev.tfinger.x, ev.tfinger.y);
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
                result |= ituSceneUpdate(&theScene, ITU_EVENT_MOUSEUP, 1, ev.tfinger.x, ev.tfinger.y);
                mouseDownTick = 0;
                break;
            }
        }

        if (mouseDownTick > 0 && (SDL_GetTicks() - mouseDownTick >= MOUSEDOWN_LONGPRESS_DELAY))
        {
            result |= ituSceneUpdate(&theScene, ITU_EVENT_MOUSELONGPRESS, 1, lastx, lasty);
            mouseDownTick = 0;
        }

        result |= ituSceneUpdate(&theScene, ITU_EVENT_TIMER, 0, 0, 0);
    #ifndef _WIN32
        if (result)
    #endif
        {
            ituSceneDraw(&theScene, screenSurf);
            ituFlip(screenSurf);
        }

        delay = MS_PER_FRAME - (SDL_GetTicks() - tick);
        //printf("scene loop delay=%d\n", delay);
        if (delay > 0)
        {
            SDL_Delay(delay);
        }
        else
            sched_yield();
    }

    return quitValue;
}

void SceneQuit(QuitValue value)
{
    quitValue = value;
}

QuitValue SceneGetQuitValue(void)
{
    return quitValue;
}

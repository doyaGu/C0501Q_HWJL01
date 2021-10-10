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
#include "elevator.h"

#ifdef _WIN32
    #include <crtdbg.h>
#endif

//#define FPS_ENABLE
#define MAX_COMMAND_QUEUE_SIZE  8
#define LONGPRESS_DELAY         3000
#define DEMO_DELAY              (1.2 * 1000 / MS_PER_FRAME)

extern ITUActionFunction actionFunctions[];
extern void resetScene(void);

// status
static QuitValue quitValue;
static ExternalOrientation orientation;
static int currDelay = DEMO_DELAY;

// command
typedef enum
{
    CMD_NONE,
    CMD_LOAD_SCENE,
    CMD_CHANGE_LANG,
    CMD_UPDATE_BY_CONFIG
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

    window = SDL_CreateWindow("Elevator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, 0);
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

void SceneUpdateByConfig(void)
{
    Command cmd;

    if (commandQueue == -1)
        return;

    cmd.id     = CMD_UPDATE_BY_CONFIG;
    mq_send(commandQueue, (const char*)&cmd, sizeof (Command), 0);
}

void SceneSetReady(bool ready)
{
    isReady = ready;
}

bool SceneIsVertical(void)
{
    return orientation == EXTERNAL_VERTICAL;
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

    orientation = ExternalGetOrientation();
    if (orientation == EXTERNAL_VERTICAL)
    {
        ituSceneLoadFile(&theScene, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/elevator_v.itu");
    }
    else
    {
        ituSceneLoadFile(&theScene, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/elevator_h.itu");
    }
    ituSceneUpdate(&theScene, ITU_EVENT_LOAD_IMAGE, (int)CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/", 0, 0);

    tick2 = SDL_GetTicks();
    printf("itu loading time: %dms\n", tick2 - tick1);

    ituSceneUpdate(&theScene, ITU_EVENT_LANGUAGE, theConfig.lang, 0, 0);
    ituSceneUpdate(&theScene, ITU_EVENT_LAYOUT, 0, 0, 0);

    if (orientation == EXTERNAL_VERTICAL)
        ituSceneSetRotation(&theScene, ITU_ROT_90, ithLcdGetWidth(), ithLcdGetHeight());
    else
        ituSceneSetRotation(&theScene, ITU_ROT_0, ithLcdGetWidth(), ithLcdGetHeight());

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

        case CMD_UPDATE_BY_CONFIG:
            MainUpdateByConfig();
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
        ScreenSaverRefresh();
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
        break;

    case STORAGE_SD_REMOVED:
        printf("STORAGE_SD_REMOVED\n");
        break;

    case STORAGE_USB_REMOVED:
        printf("STORAGE_USB_REMOVED\n");
        break;
    }
}

static bool CheckQuitValue(void)
{
    if (quitValue)
    {
        if (ScreenSaverIsScreenSaving() && theConfig.screensaver_type == SCREENSAVER_BLANK)
            ScreenSaverRefresh();

        return true;
    }
    return false;
}

static void AutoDemo(void)
{
#define DEMO_MIN_FLOOR (-2)
#define DEMO_MAX_FLOOR (20)
#define DEMO_MIN_TEMPERATURE (18)
#define DEMO_MAX_TEMPERATURE (38)

    static ExternalEventType currEventType = EXTERNAL_ARROW1;
    static bool dirDown1 = false;
    static bool dirDown2 = false;
    static int currFloor1 = DEMO_MIN_FLOOR;
    static int currFloor2 = DEMO_MIN_FLOOR;
    static int currTemperature = DEMO_MIN_TEMPERATURE;
	static ExternalWarnType currWarning = EXTERNAL_WARN_EARTHQUAKE;

    if (--currDelay <= 0)
    {
        ExternalEvent ev;

        switch (currEventType)
        {
        case EXTERNAL_ARROW1:
        case EXTERNAL_FLOOR1:
            ev.type = EXTERNAL_ARROW1;
            ev.arg1 = dirDown1 ? EXTERNAL_ARROW_DOWN : EXTERNAL_ARROW_UP;
            MainProcessExternalEvent(&ev);

            ev.type = EXTERNAL_FLOOR1;
            if (currFloor1 > 0)
            {
                if (currFloor1 >= 10)
                {
                    ev.arg1 = '0' + currFloor1 / 10;
                    ev.arg2 = '0' + currFloor1 % 10;
                }
                else
                {
                    ev.arg1 = '0' + currFloor1;
                    ev.arg2 = EXTERNAL_FLOOR_NONE;
                }
                ev.arg3 = EXTERNAL_FLOOR_NONE;
            }
            else
            {
                ev.arg1 = 'B';
                ev.arg2 = '1' - currFloor1;
                ev.arg3 = EXTERNAL_FLOOR_NONE;
            }
            MainProcessExternalEvent(&ev);
            break;

        case EXTERNAL_ARROW2:
        case EXTERNAL_FLOOR2:
            ev.type = EXTERNAL_ARROW2;
            ev.arg1 = dirDown2 ? EXTERNAL_ARROW_DOWN : EXTERNAL_ARROW_UP;
            MainProcessExternalEvent(&ev);

            ev.type = EXTERNAL_FLOOR2;
            if (currFloor2 > 0)
            {
                if (currFloor2 >= 10)
                {
                    ev.arg1 = '0' + currFloor2 / 10;
                    ev.arg2 = '0' + currFloor2 % 10;
                }
                else
                {
                    ev.arg1 = '0' + currFloor2;
                    ev.arg2 = EXTERNAL_FLOOR_NONE;
                }
                ev.arg3 = EXTERNAL_FLOOR_NONE;
            }
            else
            {
                ev.arg1 = 'B';
                ev.arg2 = '1' - currFloor2;
                ev.arg3 = EXTERNAL_FLOOR_NONE;
            }
            MainProcessExternalEvent(&ev);
            break;

        case EXTERNAL_TEMPERATURE:
            ev.type = EXTERNAL_TEMPERATURE;
            ev.arg1 = currTemperature;
            MainProcessExternalEvent(&ev);

            if (++currTemperature > DEMO_MAX_TEMPERATURE)
                currTemperature = DEMO_MIN_TEMPERATURE;

            break;

        case EXTERNAL_WARN1:
#if 0
            ev.type = EXTERNAL_WARN1;
            ev.arg1 = currWarning;
            MainProcessExternalEvent(&ev);

            if (++currWarning > EXTERNAL_WARN_EXT2)
                currWarning = EXTERNAL_WARN_NONE;
#endif // 0
            break;

        case EXTERNAL_FLOOR_ARRIVE1:
            if (dirDown1)
            {
                currFloor1--;
            }
            else
            {
                currFloor1++;
            }

            ev.type = EXTERNAL_FLOOR_ARRIVE1;
            if (currFloor1 > 0)
            {
                if (currFloor1 >= 10)
                {
                    ev.arg1 = '0' + currFloor1 / 10;
                    ev.arg2 = '0' + currFloor1 % 10;
                }
                else
                {
                    ev.arg1 = '0' + currFloor1;
                    ev.arg2 = EXTERNAL_FLOOR_NONE;
                }
                ev.arg3 = EXTERNAL_FLOOR_NONE;
            }
            else
            {
                ev.arg1 = 'B';
                ev.arg2 = '1' - currFloor1;
                ev.arg3 = EXTERNAL_FLOOR_NONE;
            }
            MainProcessExternalEvent(&ev);

            ev.type = EXTERNAL_ARROW1;
            ev.arg1 = EXTERNAL_ARROW_NONE;
            MainProcessExternalEvent(&ev);

            ev.type = EXTERNAL_FLOOR1;
            if (currFloor1 > 0)
            {
                if (currFloor1 >= 10)
                {
                    ev.arg1 = '0' + currFloor1 / 10;
                    ev.arg2 = '0' + currFloor1 % 10;
                }
                else
                {
                    ev.arg1 = '0' + currFloor1;
                    ev.arg2 = EXTERNAL_FLOOR_NONE;
                }
                ev.arg3 = EXTERNAL_FLOOR_NONE;
            }
            else
            {
                ev.arg1 = 'B';
                ev.arg2 = '1' - currFloor1;
                ev.arg3 = EXTERNAL_FLOOR_NONE;
            }
            MainProcessExternalEvent(&ev);

            if (dirDown1 && currFloor1 <= DEMO_MIN_FLOOR)
            {
                dirDown1 = false;
            }
            else if (!dirDown1 && currFloor1 >= DEMO_MAX_FLOOR)
            {
                dirDown1 = true;
            }
            break;

        case EXTERNAL_FLOOR_ARRIVE2:
            if (dirDown2)
            {
                currFloor2--;
            }
            else
            {
                currFloor2++;
            }

            ev.type = EXTERNAL_FLOOR_ARRIVE2;
            if (currFloor2 > 0)
            {
                if (currFloor2 >= 10)
                {
                    ev.arg1 = '0' + currFloor2 / 10;
                    ev.arg2 = '0' + currFloor2 % 10;
                }
                else
                {
                    ev.arg1 = '0' + currFloor2;
                    ev.arg2 = EXTERNAL_FLOOR_NONE;
                }
                ev.arg3 = EXTERNAL_FLOOR_NONE;
            }
            else
            {
                ev.arg1 = 'B';
                ev.arg2 = '1' - currFloor2;
                ev.arg3 = EXTERNAL_FLOOR_NONE;
            }
            MainProcessExternalEvent(&ev);

            ev.type = EXTERNAL_ARROW2;
            ev.arg1 = EXTERNAL_ARROW_NONE;
            MainProcessExternalEvent(&ev);

            ev.type = EXTERNAL_FLOOR2;
            if (currFloor2 > 0)
            {
                if (currFloor2 >= 10)
                {
                    ev.arg1 = '0' + currFloor2 / 10;
                    ev.arg2 = '0' + currFloor2 % 10;
                }
                else
                {
                    ev.arg1 = '0' + currFloor2;
                    ev.arg2 = EXTERNAL_FLOOR_NONE;
                }
                ev.arg3 = EXTERNAL_FLOOR_NONE;
            }
            else
            {
                ev.arg1 = 'B';
                ev.arg2 = '1' - currFloor2;
                ev.arg3 = EXTERNAL_FLOOR_NONE;
            }
            MainProcessExternalEvent(&ev);

            if (dirDown2 && currFloor2 <= DEMO_MIN_FLOOR)
            {
                dirDown2 = false;
            }
            else if (!dirDown2 && currFloor2 >= DEMO_MAX_FLOOR)
            {
                dirDown2 = true;
            }
            break;
        }

        if (++currEventType > EXTERNAL_FLOOR_ARRIVE2)
            currEventType = EXTERNAL_ARROW1;

        currDelay = DEMO_DELAY;
    }
}

int SceneRun(void)
{
    SDL_Event ev;
    int delay, frames;
    uint32_t tick, keyDownTick;

    frames = 0;
    keyDownTick = 0;

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
                ScreenSaverRefresh();
                result = ituSceneUpdate(&theScene, ITU_EVENT_KEYDOWN, ev.key.keysym.sym, 0, 0);
                switch (ev.key.keysym.sym)
                {
                case SDLK_RETURN:
                    printf("SDLK_RETURN\n");
                    ituSceneSendEvent(&theScene, EVENT_CUSTOM_GOTO_SETTING, NULL);
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
                    SceneQuit(QUIT_UPGRADE_RESOURCE);
                    break;
                    
                case SDLK_f:
                    break;

            #endif // _WIN32
                }
                break;

            case SDL_KEYUP:
                puts("SDL_KEYUP");
                keyDownTick = 0;
                result = ituSceneUpdate(&theScene, ITU_EVENT_KEYUP, ev.key.keysym.sym, 0, 0);
                break;
            }
        }
        if (!ScreenIsOff())
        {
            if (keyDownTick > 0 && (SDL_GetTicks() - keyDownTick >= LONGPRESS_DELAY))
            {
                printf("long pressed\n");
                ituSceneSendEvent(&theScene, EVENT_CUSTOM_GOTO_SETTING, NULL);
                keyDownTick = 0;
            }
            result |= ituSceneUpdate(&theScene, ITU_EVENT_TIMER, 0, 0, 0);
            //printf("%d\n", result);
        #ifndef _WIN32
            if (result)
        #endif
            {
                ituSceneDraw(&theScene, screenSurf);
                ituFlip(screenSurf);
            }

            if (theConfig.screensaver_type != SCREENSAVER_NONE &&
                ScreenSaverCheck())
            {
                if (theConfig.screensaver_type == SCREENSAVER_BLANK)
                {
                    // have a change to flush action commands
                    ituSceneUpdate(&theScene, ITU_EVENT_TIMER, 0, 0, 0);

                    // draw black screen
                    ituSceneDraw(&theScene, screenSurf);
                    ituFlip(screenSurf);

                    ScreenOff();
                }
            }
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

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
#include "airconditioner.h"

#ifdef _WIN32
    #include <crtdbg.h>
#endif

//#define FPS_ENABLE
//#define DOUBLE_KEY_ENABLE

#define GESTURE_THRESHOLD           40
#define MAX_COMMAND_QUEUE_SIZE      8
#define MOUSEDOWN_LONGPRESS_DELAY   1000

extern ITUActionFunction actionFunctions[];
extern void resetScene(void);

// status
static QuitValue quitValue;

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
    ITURotation rot;

    screenWidth = ithLcdGetWidth();
    screenHeight = ithLcdGetHeight();

    window = SDL_CreateWindow("AirConditioner", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, 0);
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

#ifdef CFG_ENABLE_ROTATE
    ituSceneSetRotation(&theScene, ITU_ROT_90, ithLcdGetWidth(), ithLcdGetHeight());
#endif

#ifdef CFG_VIDEO_ENABLE
    ituFrameFuncInit();
#endif // CFG_VIDEO_ENABLE

#ifdef CFG_PLAY_VIDEO_ON_BOOTING
#ifndef CFG_BOOT_VIDEO_ENABLE_WINDOW_MODE
    rot = itv_get_rotation();
    
    if (rot == ITU_ROT_90 || rot == ITU_ROT_270)
        PlayVideo(0, 0, ithLcdGetHeight(), ithLcdGetWidth(), CFG_BOOT_VIDEO_BGCOLOR, CFG_BOOT_VIDEO_VOLUME);
    else
    	PlayVideo(0, 0, ithLcdGetWidth(), ithLcdGetHeight(), CFG_BOOT_VIDEO_BGCOLOR, CFG_BOOT_VIDEO_VOLUME);	
#else
    PlayVideo(CFG_VIDEO_WINDOW_X_POS, CFG_VIDEO_WINDOW_Y_POS, CFG_VIDEO_WINDOW_WIDTH, CFG_VIDEO_WINDOW_HEIGHT, CFG_BOOT_VIDEO_BGCOLOR, CFG_BOOT_VIDEO_VOLUME);
#endif
#endif

#ifdef CFG_PLAY_MJPEG_ON_BOOTING
#ifndef CFG_BOOT_VIDEO_ENABLE_WINDOW_MODE
    rot = itv_get_rotation();
    
    if (rot == ITU_ROT_90 || rot == ITU_ROT_270)
        PlayMjpeg(0, 0, ithLcdGetHeight(), ithLcdGetWidth(), CFG_BOOT_VIDEO_BGCOLOR, 0);
    else
    	PlayMjpeg(0, 0, ithLcdGetWidth(), ithLcdGetHeight(), CFG_BOOT_VIDEO_BGCOLOR, 0);
#else
	PlayMjpeg(CFG_VIDEO_WINDOW_X_POS, CFG_VIDEO_WINDOW_Y_POS, CFG_VIDEO_WINDOW_WIDTH, CFG_VIDEO_WINDOW_HEIGHT, CFG_BOOT_VIDEO_BGCOLOR, 0);
#endif
#endif

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

    ituSceneLoadFileCore(&theScene, CFG_PRIVATE_DRIVE ":/airconditioner.itu");

    tick2 = SDL_GetTicks();
    printf("itu loading time: %dms\n", tick2 - tick1);

    if (theConfig.lang != LANG_ENG)
        ituSceneUpdate(&theScene, ITU_EVENT_LANGUAGE, theConfig.lang, 0, 0);

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
            ituScenePreDraw(&theScene, screenSurf);
#if defined(CFG_PLAY_VIDEO_ON_BOOTING)
            WaitPlayVideoFinish();
#elif defined(CFG_PLAY_MJPEG_ON_BOOTING)
			WaitPlayMjpegFinish();
#endif
            ituSceneStart(&theScene);
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
    int ret;
    
    if (!isReady)
        return;

    ret = ExternalReceive(&ev);
    if (ret)
    {
        MainProcessExternalEvent(&ev);
        SettingProcessExternalEvent(&ev);
    }
}

static void CheckPower(void)
{
    int ret;

    if (!isReady)
        return;

    ret = PowerCheck();
    if (ret > 0)
    {
        MainPowerOn();
    }
    else if (ret < 0)
    {
        MainPowerOff();
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
        CheckPower();

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
                ScreenSaverRefresh();
                puts("SDL_KEYDOWN");
                result = ituSceneUpdate(&theScene, ITU_EVENT_KEYDOWN, ev.key.keysym.sym, 0, 0);
                switch (ev.key.keysym.sym)
                {
                case SDLK_RETURN:
                    printf("SDLK_RETURN\n");
                    {
                        ExternalEvent ev;

                        ev.type = EXTERNAL_ENG_MODE;
                        ev.arg1 = 1;

                        MainProcessExternalEvent(&ev);
                    }
                    break;

                case SDLK_UP:
                    printf("SDLK_UP\n");
                    {
                        ExternalEvent ev;

                        ev.type = EXTERNAL_ENG_MODE;
                        ev.arg1 = 0;

                        MainProcessExternalEvent(&ev);
                    }
                    break;

                case SDLK_DOWN:
                    printf("SDLK_DOWN\n");
                    {
                        ExternalEvent ev;
                        float value = 13.8f;

                        ev.type = EXTERNAL_TEMPERATURE_INDOOR;
                        ev.arg1 = *(int*)&value;

                        MainProcessExternalEvent(&ev);
                    }
                    break;

                case SDLK_INSERT:
                    printf("SDLK_INSERT\n");
                    {
                        ExternalEvent ev;
                        float value = -10.5f;

                        ev.type = EXTERNAL_TEMPERATURE_OUTDOOR;
                        ev.arg1 = *(int*)&value;

                        MainProcessExternalEvent(&ev);
                    }
                    break;

                case SDLK_SPACE:
                    printf("SDLK_SPACE\n");
                    {
                        ExternalEvent ev;
                        float value;

                        value = 36.0f;
                        ev.type = EXTERNAL_POWER_TIME;
                        ev.arg1 = *(int*)&value;
                        SettingProcessExternalEvent(&ev);

                        value = 9.5f;
                        ev.type = EXTERNAL_POWER_VOLUME;
                        ev.arg1 = *(int*)&value;
                        SettingProcessExternalEvent(&ev);

                        value = 126.5f;
                        ev.type = EXTERNAL_POWER_TOTAL_VOLUME;
                        ev.arg1 = *(int*)&value;
                        SettingProcessExternalEvent(&ev);

                        value = 3029.4f;
                        ev.type = EXTERNAL_POWER_TOTAL_PRICE;
                        ev.arg1 = *(int*)&value;
                        SettingProcessExternalEvent(&ev);
                    }
                    break;

            #ifdef _WIN32
                case SDLK_e:
                    {
                        ExternalEvent ev;

                        ev.type = EXTERNAL_WARN;
                        ev.arg1 = EXTERNAL_WARN_COMM;
                        ev.arg2 = EXTERNAL_WARN_NONE;
                        ev.arg3 = EXTERNAL_WARN_NONE;

                        MainProcessExternalEvent(&ev);
                    }
                    break;
                    
                case SDLK_f:
                    {
                        ExternalEvent ev;

                        ev.type = EXTERNAL_WARN;
                        ev.arg1 = EXTERNAL_WARN_NONE;
                        ev.arg2 = EXTERNAL_WARN_NONE;
                        ev.arg3 = EXTERNAL_WARN_NONE;

                        MainProcessExternalEvent(&ev);
                    }
                    break;

                case SDLK_g:
                    {
                        ExternalEvent ev;

                        ev.type = EXTERNAL_MODE;
                        ev.arg1 = EXTERNAL_MODE_COOL;

                        MainProcessExternalEvent(&ev);
                    }
                    break;

                case SDLK_h:
                    {
                        ExternalEvent ev;

                        ev.type = EXTERNAL_MODE;
                        ev.arg1 = EXTERNAL_MODE_DRY;

                        MainProcessExternalEvent(&ev);
                    }
                    break;

                case SDLK_i:
                    {
                        ExternalEvent ev;

                        ev.type = EXTERNAL_POWER;
                        ev.arg1 = 0;

                        MainProcessExternalEvent(&ev);
                    }
                    break;

                case SDLK_j:
                    {
                        ExternalEvent ev;

                        ev.type = EXTERNAL_POWER;
                        ev.arg1 = 1;

                        MainProcessExternalEvent(&ev);
                    }
                    break;

                case SDLK_k:
                    {
                        ExternalEvent ev;

                        ev.type = EXTERNAL_PM25;
                        ev.arg1 = 0;

                        MainProcessExternalEvent(&ev);
                    }
                    break;

                case SDLK_l:
                    {
                        ExternalEvent ev;

                        ev.type = EXTERNAL_PM25;
                        ev.arg1 = 1;

                        MainProcessExternalEvent(&ev);
                    }
                    break;

                case SDLK_m:
                    {
                        ExternalEvent ev;

                        ev.type = EXTERNAL_WIND;
                        ev.arg1 = 80;

                        MainProcessExternalEvent(&ev);
                    }
                    break;

            #endif // _WIN32
                }
                //if (result && !ScreenIsOff())
                //    AudioPlaySound(SOUND_KEY);
                break;

            case SDL_KEYUP:
                ScreenSaverRefresh();
                puts("SDL_KEYUP");
                result = ituSceneUpdate(&theScene, ITU_EVENT_KEYUP, ev.key.keysym.sym, 0, 0);
                break;

            case SDL_MOUSEMOTION:
                ScreenSaverRefresh();
                result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEMOVE, ev.button.button, ev.button.x, ev.button.y);
                break;

            case SDL_MOUSEBUTTONDOWN:
                {
                    ScreenSaverRefresh();
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
                ScreenSaverRefresh();
                if (SDL_GetTicks() - dblclk <= 300)
                {
                    int xdiff = abs(ev.button.x - lastx);
                    int ydiff = abs(ev.button.y - lasty);

                    if (xdiff >= GESTURE_THRESHOLD)
                    {
                        if (ev.button.x > lastx)
                        {
                            printf("mouse: slide to right\n", xdiff/3);
                            result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDERIGHT, xdiff/3, ev.button.x, ev.button.y);
                        }
                        else
                        {
                            printf("mouse: slide to left\n", xdiff/3);
                            result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDELEFT, xdiff/3, ev.button.x, ev.button.y);
                        }
                    }
                    else if (ydiff >= GESTURE_THRESHOLD)
                    {
                        if (ev.button.y > lasty)
                        {
                            printf("mouse: slide to down\n");
                            result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDEDOWN, ydiff/3, ev.button.x, ev.button.y);
                        }
                        else
                        {
                            printf("mouse: slide to up\n");
                            result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDEUP, ydiff/3, ev.button.x, ev.button.y);
                        }
                    }
                }
                result |= ituSceneUpdate(&theScene, ITU_EVENT_MOUSEUP, ev.button.button, ev.button.x, ev.button.y);
                mouseDownTick = 0;
                break;

            case SDL_FINGERMOTION:
                ScreenSaverRefresh();
                printf("touch: move %d, %d\n", ev.tfinger.x, ev.tfinger.y);
                result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEMOVE, 1, ev.tfinger.x, ev.tfinger.y);
                break;

            case SDL_FINGERDOWN:
                ScreenSaverRefresh();
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
                ScreenSaverRefresh();
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
                            result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDERIGHT, xdiff/3, ev.tfinger.x, ev.tfinger.y);
                        }
                        else
                        {
                            printf("touch: slide to left %d %d\n", ev.button.x, ev.button.y);
                            result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDELEFT, xdiff/3, ev.tfinger.x, ev.tfinger.y);
                        }
                    }
                    else if (ydiff >= GESTURE_THRESHOLD)
                    {
                        if (ev.tfinger.y > lasty)
                        {
                            printf("touch: slide to down %d %d\n", ev.tfinger.x, ev.tfinger.y);
                            result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDEDOWN, ydiff/3, ev.tfinger.x, ev.tfinger.y);
                        }
                        else
                        {
                            printf("touch: slide to up %d %d\n", ev.tfinger.x, ev.tfinger.y);
                            result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDEUP, ydiff/3, ev.tfinger.x, ev.tfinger.y);
                        }
                    }
                }
                result |= ituSceneUpdate(&theScene, ITU_EVENT_MOUSEUP, 1, ev.tfinger.x, ev.tfinger.y);
                mouseDownTick = 0;
                break;
            }
        }

        if (!ScreenIsOff())
        {
            if (result)
                MainRefreshTime();

            if (mouseDownTick > 0 && (SDL_GetTicks() - mouseDownTick >= MOUSEDOWN_LONGPRESS_DELAY))
            {
                result |= ituSceneUpdate(&theScene, ITU_EVENT_MOUSELONGPRESS, 1, lastx, lasty);
                mouseDownTick = 0;
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

            if (theConfig.screensaver_time != 0 && ScreenSaverCheck())
            {
                ituSceneSendEvent(&theScene, EVENT_CUSTOM_SCREENSAVER, NULL);

                // have a change to flush action commands
                ituSceneUpdate(&theScene, ITU_EVENT_TIMER, 0, 0, 0);

                // draw black screen
                ituSceneDraw(&theScene, screenSurf);
                ituFlip(screenSurf);

                ScreenOff();
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

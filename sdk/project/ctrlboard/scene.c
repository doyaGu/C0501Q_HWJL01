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
#include "ctrlboard.h"

#ifdef _WIN32
    #include <crtdbg.h>

#ifndef CFG_VIDEO_ENABLE
    #define DISABLE_SWITCH_VIDEO_STATE
#endif
#endif // _WIN32

#ifndef CFG_POWER_WAKEUP_DOUBLE_CLICK_INTERVAL
	#define DOUBLE_KEY_INTERVAL 200
#endif

//#define FPS_ENABLE
#define DOUBLE_KEY_ENABLE

#define GESTURE_THRESHOLD           40
#define MAX_COMMAND_QUEUE_SIZE      8
#define MOUSEDOWN_LONGPRESS_DELAY   1000

extern ITUActionFunction actionFunctions[];
extern void resetScene(void);

// status
static QuitValue quitValue;
static bool      inVideoState;

// command
typedef enum
{
    CMD_NONE,
    CMD_LOAD_SCENE,
    CMD_CALL_CONNECTED,
    CMD_GOTO_MAINMENU,
    CMD_CHANGE_LANG,
    CMD_PREDRAW
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
static float screenDistance;
static bool isReady;
static int periodPerFrame;

#if defined(CFG_USB_MOUSE) || defined(_WIN32)
static ITUIcon* cursorIcon;
#endif

void SceneInit(void)
{
    struct mq_attr attr;
	ITURotation rot;

#ifdef CFG_LCD_ENABLE
    screenWidth = ithLcdGetWidth();
    screenHeight = ithLcdGetHeight();

    window = SDL_CreateWindow("Display Control Board", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, 0);
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

    //ituSceneInit(&theScene, NULL);
	ituSceneSetFunctionTable(&theScene, actionFunctions);

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_COMMAND_QUEUE_SIZE;
    attr.mq_msgsize = sizeof(Command);

    commandQueue = mq_open("scene", O_CREAT | O_NONBLOCK, 0644, &attr);
    assert(commandQueue != -1);

    screenDistance = sqrtf(screenWidth * screenWidth + screenHeight * screenHeight);

    isReady = false;
    periodPerFrame = MS_PER_FRAME;
#endif
}

void SceneExit(void)
{
#ifdef CFG_LCD_ENABLE
    mq_close(commandQueue);
    commandQueue = -1;

    if (theScene.root)
    {
        ituSceneExit(&theScene);
    }
    ituFtExit();

#ifdef CFG_M2D_ENABLE
    ituM2dExit();
#ifdef CFG_VIDEO_ENABLE
    ituFrameFuncExit();
#endif // CFG_VIDEO_ENABLE
#else
    ituSWExit();
#endif // CFG_M2D_ENABLE

    SDL_DestroyWindow(window);
#endif
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

void SceneGotoMainMenu(void)
{
    Command cmd;

    if (commandQueue == -1)
        return;

    cmd.id     = CMD_GOTO_MAINMENU;
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

void ScenePredraw(int arg)
{
    Command cmd;

    if (commandQueue == -1)
        return;

    cmd.id     = CMD_PREDRAW;
    mq_send(commandQueue, (const char*)&cmd, sizeof (Command), 0);
}

void SceneSetReady(bool ready)
{
    isReady = ready;
}

static void LoadScene(void)
{
#ifdef CFG_LCD_ENABLE
    uint32_t tick1, tick2;

	resetScene();
    if (theScene.root)
    {
        ituSceneExit(&theScene);
    }

    // load itu file
    tick1 = SDL_GetTicks();

#ifdef CFG_LCD_MULTIPLE
    {
        char filepath[PATH_MAX];

        sprintf(filepath, CFG_PRIVATE_DRIVE ":/itu/%ux%u/ctrlboard.itu", ithLcdGetWidth(), ithLcdGetHeight());
        ituSceneLoadFileCore(&theScene, filepath);
    }
#else
    ituSceneLoadFileCore(&theScene, CFG_PRIVATE_DRIVE ":/ctrlboard.itu");
#endif // CFG_LCD_MULTIPLE

    tick2 = SDL_GetTicks();
    printf("itu loading time: %dms\n", tick2 - tick1);

    if (theConfig.lang != LANG_ENG)
        ituSceneUpdate(&theScene, ITU_EVENT_LANGUAGE, theConfig.lang, 0, 0);
    
    //ituSceneSetRotation(&theScene, ITU_ROT_90, ithLcdGetWidth(), ithLcdGetHeight());

    tick1 = tick2;

#if defined(CFG_USB_MOUSE) || defined(_WIN32)
    cursorIcon = ituSceneFindWidget(&theScene, "cursorIcon");
    if (cursorIcon)
    {
        ituWidgetSetVisible(cursorIcon, true);
    }
#endif // defined(CFG_USB_MOUSE) || defined(_WIN32)

    tick2 = SDL_GetTicks();
    printf("itu init time: %dms\n", tick2 - tick1);

    ExternalProcessInit();
#endif
}

void SceneEnterVideoState(int timePerFrm)
{
    if (inVideoState)
    {
        return;
    }

#ifndef DISABLE_SWITCH_VIDEO_STATE
    #ifdef CFG_VIDEO_ENABLE
    ituFrameFuncInit();
    #endif
    screenSurf   = ituGetDisplaySurface();
    inVideoState = true;
    if (timePerFrm != 0)
        periodPerFrame = timePerFrm;
#endif
}

void SceneLeaveVideoState(void)
{
    if (!inVideoState)
    {
        return;
    }

#ifndef DISABLE_SWITCH_VIDEO_STATE
    #ifdef CFG_VIDEO_ENABLE
    ituFrameFuncExit();
    #endif
    #ifdef CFG_LCD_ENABLE
    ituLcdInit();
    #endif
    #ifdef CFG_M2D_ENABLE
    ituM2dInit();
    #else
    ituSWInit();
    #endif

    screenSurf   = ituGetDisplaySurface();
    periodPerFrame = MS_PER_FRAME;
#endif
    inVideoState = false;
}

static void GotoMainMenu(void)
{
    ITULayer* mainMenuLayer = ituSceneFindWidget(&theScene, "mainMenuLayer");
    assert(mainMenuLayer);
    ituLayerGoto(mainMenuLayer);
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
#if defined(CFG_PLAY_VIDEO_ON_BOOTING)
            ituScenePreDraw(&theScene, screenSurf);
            WaitPlayVideoFinish();
#elif defined(CFG_PLAY_MJPEG_ON_BOOTING)
            ituScenePreDraw(&theScene, screenSurf);
			WaitPlayMjpegFinish();
#endif
            ituSceneStart(&theScene);
            break;

        case CMD_GOTO_MAINMENU:
            GotoMainMenu();
            break;

        case CMD_CHANGE_LANG:
            ituSceneUpdate(&theScene, ITU_EVENT_LANGUAGE, theConfig.lang, 0, 0);
            ituSceneUpdate(&theScene, ITU_EVENT_LAYOUT, 0, 0, 0);
            break;

#if !defined(CFG_PLAY_VIDEO_ON_BOOTING) && !defined(CFG_PLAY_MJPEG_ON_BOOTING)
        case CMD_PREDRAW:
            ituScenePreDraw(&theScene, screenSurf);
            break;
#endif            
        }
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

static void CheckStorage(void)
{
    StorageAction action = StorageCheck();

    switch (action)
    {
    case STORAGE_SD_INSERTED:
        ituSceneSendEvent(&theScene, EVENT_CUSTOM_SD_INSERTED, NULL);
        break;

    case STORAGE_SD_REMOVED:
        ituSceneSendEvent(&theScene, EVENT_CUSTOM_SD_REMOVED, NULL);
        break;

    case STORAGE_USB_INSERTED:
        ituSceneSendEvent(&theScene, EVENT_CUSTOM_USB_INSERTED, NULL);
        break;

    case STORAGE_USB_REMOVED:
        ituSceneSendEvent(&theScene, EVENT_CUSTOM_USB_REMOVED, NULL);
        break;

    case STORAGE_USB_DEVICE_INSERTED:
        {
            ITULayer* usbDeviceModeLayer = ituSceneFindWidget(&theScene, "usbDeviceModeLayer");
            assert(usbDeviceModeLayer);

            ituLayerGoto(usbDeviceModeLayer);
        }
        break;

    case STORAGE_USB_DEVICE_REMOVED:
        {
            ITULayer* mainMenuLayer = ituSceneFindWidget(&theScene, "mainMenuLayer");
            assert(mainMenuLayer);

            ituLayerGoto(mainMenuLayer);
        }
        break;
    }
}

static void CheckExternal(void)
{
    ExternalEvent ev;
    int ret = ExternalReceive(&ev);

    if (ret)
    {
        ScreenSaverRefresh();
        ExternalProcessEvent(&ev);
    }
}

#if defined(CFG_USB_MOUSE) || defined(_WIN32)

static void CheckMouse(void)
{
    if (ioctl(ITP_DEVICE_USBMOUSE, ITP_IOCTL_IS_AVAIL, NULL))
    {
        if (!ituWidgetIsVisible(cursorIcon))
            ituWidgetSetVisible(cursorIcon, true);
    }
    else
    {
        if (ituWidgetIsVisible(cursorIcon))
            ituWidgetSetVisible(cursorIcon, false);
    }
}
#endif // defined(CFG_USB_MOUSE) || defined(_WIN32)

int SceneRun(void)
{
    SDL_Event ev;
    int delay, frames, lastx, lasty;
    uint32_t tick, dblclk, lasttick, mouseDownTick;
    static bool first_screenSurf = true , sleepModeDoubleClick = false;
#if defined(CFG_POWER_WAKEUP_IR)
    static bool sleepModeIR = false;
#endif

    /* Watch keystrokes */
    dblclk = frames = lasttick = lastx = lasty = mouseDownTick = 0;

    for (;;)
    {
        bool result = false;

        if (CheckQuitValue())
            break;

#ifdef CFG_LCD_ENABLE
        ProcessCommand();
#endif
        CheckExternal();
        CheckStorage();

    #if defined(CFG_USB_MOUSE) || defined(_WIN32)
        if (cursorIcon)
            CheckMouse();
    #endif // defined(CFG_USB_MOUSE) || defined(_WIN32)

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

#ifdef CFG_LCD_ENABLE
        while (SDL_PollEvent(&ev))
        {
            switch (ev.type)
            {
            case SDL_KEYDOWN:
                ScreenSaverRefresh();
                result = ituSceneUpdate(&theScene, ITU_EVENT_KEYDOWN, ev.key.keysym.sym, 0, 0);
                switch (ev.key.keysym.sym)
                {
                case SDLK_UP:
                    ituSceneSendEvent(&theScene, EVENT_CUSTOM_KEY0, NULL);
                    break;

                case SDLK_DOWN:
                    ituSceneSendEvent(&theScene, EVENT_CUSTOM_KEY1, NULL);
                    break;

                case SDLK_LEFT:
                    ituSceneSendEvent(&theScene, EVENT_CUSTOM_KEY2, NULL);
                    break;

                case SDLK_RIGHT:
                    ituSceneSendEvent(&theScene, EVENT_CUSTOM_KEY3, NULL);
                    break;

                case SDLK_INSERT:
                    break;

            #ifdef _WIN32
                case SDLK_e:
                    result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHPINCH, 20, 30, 40);
                    break;
                    
                case SDLK_f:
                    {
                        ITULayer* usbDeviceModeLayer = ituSceneFindWidget(&theScene, "usbDeviceModeLayer");
                        assert(usbDeviceModeLayer);

                        ituLayerGoto(usbDeviceModeLayer);
                    }
                    break;

                case SDLK_g:
                    {
                        ExternalEvent ev;

                        ev.type = EXTERNAL_SHOW_MSG;
                        strcpy(ev.buf1, "test");

                        ScreenSaverRefresh();
                        ExternalProcessEvent(&ev);
                    }
                    break;

            #endif // _WIN32
                }
                if (result && !ScreenIsOff() && !StorageIsInUsbDeviceMode())
                    AudioPlayKeySound();

                break;

            case SDL_KEYUP:
                result = ituSceneUpdate(&theScene, ITU_EVENT_KEYUP, ev.key.keysym.sym, 0, 0);
                break;

            case SDL_MOUSEMOTION:
                ScreenSaverRefresh();
                #if defined(CFG_USB_MOUSE) || defined(_WIN32)
                if (cursorIcon)
                {
                    ituWidgetSetX(cursorIcon, ev.button.x);
                    ituWidgetSetY(cursorIcon, ev.button.y);
                    ituWidgetSetDirty(cursorIcon, true);
            	    //printf("mouse: move %d, %d\n", ev.button.x, ev.button.y);
                }
                #endif // defined(CFG_USB_MOUSE) || defined(_WIN32)
                result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEMOVE, ev.button.button, ev.button.x, ev.button.y);
                break;

            case SDL_MOUSEBUTTONDOWN:
                ScreenSaverRefresh();
                printf("mouse: down %d, %d\n", ev.button.x, ev.button.y);
                {
                    mouseDownTick = SDL_GetTicks();
                #ifdef DOUBLE_KEY_ENABLE
                    if (mouseDownTick - dblclk <= 200)
                    {
                        result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEDOUBLECLICK, ev.button.button, ev.button.x, ev.button.y);
                        dblclk = 0;
                    }
                    else
                #endif // DOUBLE_KEY_ENABLE
                    {
                        result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEDOWN, ev.button.button, ev.button.x, ev.button.y);
                        dblclk = mouseDownTick;
                        lastx = ev.button.x;
                        lasty = ev.button.y;
                    }
                    if (result && !ScreenIsOff() && !StorageIsInUsbDeviceMode())
                        AudioPlayKeySound();

                #ifdef CFG_SCREENSHOT_ENABLE
                    if (ev.button.x < 50 && ev.button.y > CFG_LCD_HEIGHT - 50)
                        Screenshot(screenSurf);
                #endif // CFG_SCREENSHOT_ENABLE
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (SDL_GetTicks() - dblclk <= 200)
                {
                    int xdiff = abs(ev.button.x - lastx);
                    int ydiff = abs(ev.button.y - lasty);

                    if (xdiff >= GESTURE_THRESHOLD && xdiff > ydiff)
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
                ScreenSaverRefresh();
                printf("touch: move %d, %d\n", ev.tfinger.x, ev.tfinger.y);
                result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEMOVE, 1, ev.tfinger.x, ev.tfinger.y);
                break;

            case SDL_FINGERDOWN:
                ScreenSaverRefresh();
                printf("touch: down %d, %d\n", ev.tfinger.x, ev.tfinger.y);
                {
                    mouseDownTick = SDL_GetTicks();
                #ifdef DOUBLE_KEY_ENABLE
					#ifdef CFG_POWER_WAKEUP_DOUBLE_CLICK_INTERVAL
					if (mouseDownTick - dblclk <= CFG_POWER_WAKEUP_DOUBLE_CLICK_INTERVAL)
					#else
                    if (mouseDownTick - dblclk <= 200)
					#endif
                    {
	                 	printf("double touch!\n");
						if(sleepModeDoubleClick)
						{
							ScreenSetDoubleClick();
							ScreenSaverRefresh();
							sleepModeDoubleClick = false;
						}
                        result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEDOUBLECLICK, 1, ev.tfinger.x, ev.tfinger.y);
                        dblclk = mouseDownTick = 0;
                    }
                    else
                #endif // DOUBLE_KEY_ENABLE
                    {
                        result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEDOWN, 1, ev.tfinger.x, ev.tfinger.y);
                        dblclk = mouseDownTick;
                        lastx = ev.tfinger.x;
                        lasty = ev.tfinger.y;
                    }
                    if (result && !ScreenIsOff() && !StorageIsInUsbDeviceMode())
                        AudioPlayKeySound();

                #ifdef CFG_SCREENSHOT_ENABLE
                    if (ev.tfinger.x < 50 && ev.tfinger.y > CFG_LCD_HEIGHT - 50)
                        Screenshot(screenSurf);
                #endif // CFG_SCREENSHOT_ENABLE
                    //if (ev.tfinger.x < 50 && ev.tfinger.y > CFG_LCD_HEIGHT - 50)
                    //    SceneQuit(QUIT_UPGRADE_WEB);
                }
                break;

            case SDL_FINGERUP:
                printf("touch: up %d, %d\n", ev.tfinger.x, ev.tfinger.y);
                if (SDL_GetTicks() - dblclk <= 300)
                {
                    int xdiff = abs(ev.tfinger.x - lastx);
                    int ydiff = abs(ev.tfinger.y - lasty);

                    if (xdiff >= GESTURE_THRESHOLD && xdiff > ydiff)
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

            case SDL_MULTIGESTURE:
                printf("touch: multi %d, %d\n", ev.mgesture.x, ev.mgesture.y);
                if (ev.mgesture.dDist > 0.0f)
                {
                    int dist = (int)(screenDistance * ev.mgesture.dDist);
                    int x = (int)(screenWidth * ev.mgesture.x);
                    int y = (int)(screenHeight * ev.mgesture.y);
                    result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHPINCH, dist, x, y);
                }
                break;
            }
        }
        if (!ScreenIsOff())
        {
            if (mouseDownTick > 0 && (SDL_GetTicks() - mouseDownTick >= MOUSEDOWN_LONGPRESS_DELAY))
            {
                printf("long press: %d %d\n", lastx, lasty);
                result |= ituSceneUpdate(&theScene, ITU_EVENT_MOUSELONGPRESS, 1, lastx, lasty);
                mouseDownTick = 0;
            }
            result |= ituSceneUpdate(&theScene, ITU_EVENT_TIMER, 0, 0, 0);
            //printf("%d\n", result);
            if (result)
            {
                ituSceneDraw(&theScene, screenSurf);
                ituFlip(screenSurf);
                if (first_screenSurf) {
                    ScreenSetBrightness(theConfig.brightness);
                    first_screenSurf = false;
                }
            }

            if (theConfig.screensaver_type != SCREENSAVER_NONE &&
                ScreenSaverCheck())
            {
                ituSceneSendEvent(&theScene, EVENT_CUSTOM_SCREENSAVER, "0");

                if (theConfig.screensaver_type == SCREENSAVER_BLANK)
                {
                    // have a change to flush action commands
                    ituSceneUpdate(&theScene, ITU_EVENT_TIMER, 0, 0, 0);

                    // draw black screen
                    ituSceneDraw(&theScene, screenSurf);
                    ituFlip(screenSurf);

                    ScreenOff();
					
                    #if defined(CFG_POWER_WAKEUP_IR)
                        sleepModeIR = true;
                    #endif
					#if defined(CFG_POWER_WAKEUP_TOUCH_DOUBLE_CLICK)
						sleepModeDoubleClick = true;			
					#endif
                }
            }
        }

#if defined(CFG_POWER_WAKEUP_IR)
        if (ScreenIsOff() && sleepModeIR)
        {
            printf("Wake up by remote IR!\n");
            ScreenSaverRefresh();
            ituSceneUpdate(&theScene, ITU_EVENT_MOUSEDOWN, 1, 0, 0);
            ituSceneDraw(&theScene, screenSurf);
            ituFlip(screenSurf);
            sleepModeIR = false;
        }
#endif

		if(sleepModeDoubleClick)
		{
			if(theConfig.screensaver_type != SCREENSAVER_NONE &&
                ScreenSaverCheckForDoubleClick())
            { 
                if (theConfig.screensaver_type == SCREENSAVER_BLANK)
                    ScreenOffContinue();
            }
		}
#endif
        delay = periodPerFrame - (SDL_GetTicks() - tick);
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

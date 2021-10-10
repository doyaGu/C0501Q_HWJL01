#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "SDL/SDL.h"
#include "ite/itp.h"
#include "audio_mgr.h"
#include "scene.h"
#include "sports_equipment.h"
#include "castor3player.h"

static ITUBackground* mainBackground;
static ITUSlideshow* mainSlideshow;
static ITUVideo* mainVideo;
static ITUMediaFileListBox* mainVideoMediaFileListBox;
static ITUBackground* mainOverlay1Background;
static ITUBackground* mainOverlay2Background;
static ITUSprite* mainCountDownSprite;
static ITUAnimation* mainCountDown3Animation;
static ITUAnimation* mainCountDown2Animation;
static ITUAnimation* mainCountDown1Animation;
static ITUText* mainDistanceText;
static ITUText* mainTimeText;
static ITUText* mainPaceText;
static ITUText* mainCalorieText;
static ITUText* mainPulseText;
static ITUText* mainGoalTimeText;
static ITUProgressBar* mainProgressBar;
static ITUText* mainProgressText;
static ITUTrackBar* mainTrackBar;
static ITUButton* mainTrackButton;
static ITUSprite* mainSlopeSprite;
static ITUWheel* mainSlopeWheel;
static ITUSprite* mainSpeedSprite;
static ITUWheel* mainSpeedWheel;
static ITUSprite* mainSpeedUnitSprite;
static ITUSprite* mainFanSprite;
static ITULayer* standbyLayer;
static ITUContainer* mainWarnContainer;
static ITUSprite* mainWarnSprite;

#define MAX_SLOPE_INDEX 15
#define SLOPE_INDEX 15
#define MAX_SPEED_INDEX 200
#define SPEED_INDEX 192
#define MAX_FAN_INDEX 3
#define MAX_PAUSE_SEC 20

typedef enum
{
    MAIN_COUNTDOWN,
    MAIN_RUNNING,
    MAIN_PAUSED,
    MAIN_WARNING,
    MAIN_WARNED
} MainState;

typedef enum
{
    SOUNDPLAY_NONE,
    SOUNDPLAY_COUNTDOWN,
    SOUNDPLAY_STATUS_DISTANCE,
    SOUNDPLAY_STATUS_DISTANCE_TEN_NUMBER,
    SOUNDPLAY_STATUS_DISTANCE_TEN,
    SOUNDPLAY_STATUS_DISTANCE_NUMBER,
    SOUNDPLAY_STATUS_DISTANCE_POINT,
    SOUNDPLAY_STATUS_DISTANCE_POINT_NUMBER,
    SOUNDPLAY_STATUS_DISTANCE_KM,
    SOUNDPLAY_STATUS_TIME,
    SOUNDPLAY_STATUS_TIME_HOUR_NUMBER,
    SOUNDPLAY_STATUS_TIME_HOUR,
    SOUNDPLAY_STATUS_TIME_MINUTE_TEN_NUMBER,
    SOUNDPLAY_STATUS_TIME_MINUTE_TEN,
    SOUNDPLAY_STATUS_TIME_MINUTE_NUMBER,
    SOUNDPLAY_STATUS_TIME_MINUTE,
    SOUNDPLAY_STATUS_TIME_SECOND_TEN_NUMBER,
    SOUNDPLAY_STATUS_TIME_SECOND_TEN,
    SOUNDPLAY_STATUS_TIME_SECOND_NUMBER,
    SOUNDPLAY_STATUS_TIME_SECOND,
    SOUNDPLAY_STATUS_CALORIE,
    SOUNDPLAY_STATUS_CALORIE_HUNDRED_NUMBER,
    SOUNDPLAY_STATUS_CALORIE_HUNDRED,
    SOUNDPLAY_STATUS_CALORIE_TEN_NUMBER,
    SOUNDPLAY_STATUS_CALORIE_TEN,
    SOUNDPLAY_STATUS_CALORIE_NUMBER,
    SOUNDPLAY_STATUS_CALORIE_CALORIE,
    SOUNDPLAY_STATUS_PULSE,
    SOUNDPLAY_STATUS_PULSE_HUNDRED_NUMBER,
    SOUNDPLAY_STATUS_PULSE_HUNDRED,
    SOUNDPLAY_STATUS_PULSE_TEN_NUMBER,
    SOUNDPLAY_STATUS_PULSE_TEN,
    SOUNDPLAY_STATUS_PULSE_NUMBER,
    SOUNDPLAY_WARN
} SoundPlayState;

typedef enum
{
	PROGRESS_PERCENTAGE,
    PROGRESS_DISTANCE,
    PROGRESS_TIME,
	PROGRESS_PACE,
    PROGRESS_CALORIE
} ProgressState;

static MainState mainState;
static SoundPlayState soundPlayState;
static ProgressState progressState;
static uint32_t mainLastTick, mainWarnSoundTick;
static int mainCountDown, mainFanIndex, mainPauseSeconds, mainPulse, mainStatusDistance, mainStatusTime, mainStatusCalorie, mainStatusPulse, mainStatusPace;
static time_t mainPauseTime;
time_t mainBeginTime;

static float EvalVideoSpeed(float speed)
{
    if(speed < 2.0)    
        return 0.5;
    else if(speed >= 2.0 && speed < 3.0)
        return 0.6;
    else if(speed >= 3.0 && speed < 4.0)
        return 0.7;
    else if(speed >= 4.0 && speed < 5.0)
        return 0.8;
    else if(speed >= 5.0 && speed < 6.0)
        return 0.9;
    else if(speed >= 6.0 && speed < 7.0)
        return 1.0;
    else if(speed >= 7.0 && speed < 8.0)
        return 1.1;
    else if(speed >= 8.0 && speed < 9.0)
        return 1.2;
    else if(speed >= 9.0 && speed < 10.0)
        return 1.3;
    else if(speed >= 10.0 && speed < 11.0)
        return 1.4;
    else if(speed >= 11.0 && speed < 12.0)
        return 1.5;
    else if(speed >= 12.0 && speed < 13.0)
        return 1.6;
    else if(speed >= 13.0 && speed < 14.0)
        return 1.7;
    else if(speed >= 14.0 && speed < 15.0)
        return 1.8;
    else if(speed >= 15.0 && speed < 16.0)
        return 1.9;
    else if(speed >= 16)
        return 2.0;
}

static int MainAudioPlayCallback(int state)
{
    int value;
    char buf[PATH_MAX];

    switch (state)
    {
    case AUDIOMGR_STATE_CALLBACK_PLAYING_FINISH:
        switch (soundPlayState)
        {
        case SOUNDPLAY_COUNTDOWN:
            if (mainCountDown <= 0)
                soundPlayState = SOUNDPLAY_NONE;
            break;

        case SOUNDPLAY_STATUS_DISTANCE:
            if (mainStatusDistance >= 100000)
            {
                if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/time.wav", MainAudioPlayCallback) < 0)
                {
                    soundPlayState = SOUNDPLAY_NONE;
                    return 0;
                }
                soundPlayState = SOUNDPLAY_STATUS_TIME;
            }
            else if (mainStatusDistance >= 10000)
            {
                value = mainStatusDistance / 10000;
                sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", value);

                if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                {
                    soundPlayState = SOUNDPLAY_NONE;
                    return 0;
                }
                soundPlayState = SOUNDPLAY_STATUS_DISTANCE_TEN_NUMBER;
            }
            else
            {
                value = mainStatusDistance / 10000;
                value = mainStatusDistance / 1000 - value * 10;
                sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", value);

                if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                {
                    soundPlayState = SOUNDPLAY_NONE;
                    return 0;
                }
                soundPlayState = SOUNDPLAY_STATUS_DISTANCE_NUMBER;
            }
            break;

        case SOUNDPLAY_STATUS_DISTANCE_TEN_NUMBER:
            if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/ten.wav", MainAudioPlayCallback) < 0)
            {
                soundPlayState = SOUNDPLAY_NONE;
                return 0;
            }
            soundPlayState = SOUNDPLAY_STATUS_DISTANCE_TEN;
            break;

        case SOUNDPLAY_STATUS_DISTANCE_TEN:
            {
                value = mainStatusDistance / 10000;
                value = mainStatusDistance / 1000 - value * 10;
                sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", value);

                if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                {
                    soundPlayState = SOUNDPLAY_NONE;
                    return 0;
                }
                soundPlayState = SOUNDPLAY_STATUS_DISTANCE_NUMBER;
            }
            break;

        case SOUNDPLAY_STATUS_DISTANCE_NUMBER:
            {
                value = mainStatusDistance % 1000;
                if (value > 0)
                {
                    if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/point.wav", MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_DISTANCE_POINT;
                }
                else
                {
                    if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/km.wav", MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_DISTANCE_KM;
                }
            }
            break;

        case SOUNDPLAY_STATUS_DISTANCE_POINT:
            {
                value = mainStatusDistance % 1000;
                value /= 100;

                sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", value);

                if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                {
                    soundPlayState = SOUNDPLAY_NONE;
                    return 0;
                }
                soundPlayState = SOUNDPLAY_STATUS_DISTANCE_POINT_NUMBER;
            }
            break;

        case SOUNDPLAY_STATUS_DISTANCE_POINT_NUMBER:
            if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/km.wav", MainAudioPlayCallback) < 0)
            {
                soundPlayState = SOUNDPLAY_NONE;
                return 0;
            }
            soundPlayState = SOUNDPLAY_STATUS_DISTANCE_KM;
            break;

        case SOUNDPLAY_STATUS_DISTANCE_KM:
            {
                int h, seconds;
                time_t now;

                time(&now);
                seconds = (int)difftime(now, mainBeginTime);
                h = seconds / (60 * 60);
                if (h > 9)
                {
                    if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/consume.wav", MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_CALORIE;
                }
                else
                {
                    if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/time.wav", MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_TIME;
                }
            }
            break;

        case SOUNDPLAY_STATUS_TIME:
            {
                int h, m, s;

                h = mainStatusTime / (60 * 60);
                m = mainStatusTime / 60 - h * 60;
                s = mainStatusTime % 60;
                if (h > 0)
                {
                    sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", h);

                    if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_TIME_HOUR_NUMBER;
                }
                else if (m > 10)
                {
                    value = m / 10;
                    sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", value);

                    if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_TIME_MINUTE_TEN_NUMBER;
                }
                else if (m > 0)
                {
                    sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", m);

                    if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_TIME_MINUTE_NUMBER;
                }
                else if (s > 10)
                {
                    value = s / 10;
                    sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", value);

                    if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_TIME_SECOND_TEN_NUMBER;
                }
                else
                {
                    sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", s);

                    if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_TIME_SECOND_NUMBER;
                }
            }
            break;

        case SOUNDPLAY_STATUS_TIME_HOUR_NUMBER:
            if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/hour.wav", MainAudioPlayCallback) < 0)
            {
                soundPlayState = SOUNDPLAY_NONE;
                return 0;
            }
            soundPlayState = SOUNDPLAY_STATUS_TIME_HOUR;
            break;

        case SOUNDPLAY_STATUS_TIME_HOUR:
            {
                int h, m, s;

                h = mainStatusTime / (60 * 60);
                m = mainStatusTime / 60 - h * 60;
                s = mainStatusTime % 60;
                if (m > 10)
                {
                    value = m / 10;
                    sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", value);

                    if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_TIME_MINUTE_TEN_NUMBER;
                }
                else if (m > 0)
                {
                    sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", m);

                    if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_TIME_MINUTE_NUMBER;
                }
                else if (s > 10)
                {
                    value = s / 10;
                    sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", value);

                    if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_TIME_SECOND_TEN_NUMBER;
                }
                else
                {
                    sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", s);

                    if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_TIME_SECOND_NUMBER;
                }
            }
            break;

        case SOUNDPLAY_STATUS_TIME_MINUTE_TEN_NUMBER:
            if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/ten.wav", MainAudioPlayCallback) < 0)
            {
                soundPlayState = SOUNDPLAY_NONE;
                return 0;
            }
            soundPlayState = SOUNDPLAY_STATUS_TIME_MINUTE_TEN;
            break;

        case SOUNDPLAY_STATUS_TIME_MINUTE_TEN:
            {
                int h, m, s;

                h = mainStatusTime / (60 * 60);
                m = mainStatusTime / 60 - h * 60;
                s = mainStatusTime % 60;
                if (m > 0)
                {
                    value = m % 10;
                    sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", value);

                    if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_TIME_MINUTE_NUMBER;
                }
                else
                {
                    if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/minute.wav", MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_TIME_MINUTE;
                }
            }
            break;

        case SOUNDPLAY_STATUS_TIME_MINUTE_NUMBER:
            if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/minute.wav", MainAudioPlayCallback) < 0)
            {
                soundPlayState = SOUNDPLAY_NONE;
                return 0;
            }
            soundPlayState = SOUNDPLAY_STATUS_TIME_MINUTE;
            break;

        case SOUNDPLAY_STATUS_TIME_MINUTE:
            {
                int s = mainStatusTime % 60;
                if (s > 10)
                {
                    value = s / 10;
                    sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", value);

                    if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_TIME_SECOND_TEN_NUMBER;
                }
                else if (s > 0)
                {
                    sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", s);

                    if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_TIME_SECOND_NUMBER;
                }
                else
                {
                    if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/consume.wav", MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_CALORIE;
                }
            }
            break;

        case SOUNDPLAY_STATUS_TIME_SECOND_TEN_NUMBER:
            if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/ten.wav", MainAudioPlayCallback) < 0)
            {
                soundPlayState = SOUNDPLAY_NONE;
                return 0;
            }
            soundPlayState = SOUNDPLAY_STATUS_TIME_SECOND_TEN;
            break;

        case SOUNDPLAY_STATUS_TIME_SECOND_TEN:
            {
                int s = mainStatusTime % 10;
                if (s > 0)
                {
                    sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", s);

                    if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_TIME_SECOND_NUMBER;
                }
                else
                {
                    if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/second.wav", MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_TIME_SECOND;
                }
            }
            break;

        case SOUNDPLAY_STATUS_TIME_SECOND_NUMBER:
            if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/second.wav", MainAudioPlayCallback) < 0)
            {
                soundPlayState = SOUNDPLAY_NONE;
                return 0;
            }
            soundPlayState = SOUNDPLAY_STATUS_TIME_SECOND;
            break;

        case SOUNDPLAY_STATUS_TIME_SECOND:
            if (mainStatusCalorie >= 1000)
            {
                if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/pulse.wav", MainAudioPlayCallback) < 0)
                {
                    soundPlayState = SOUNDPLAY_NONE;
                    return 0;
                }
                soundPlayState = SOUNDPLAY_STATUS_PULSE;
            }
            else
            {
                if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/consume.wav", MainAudioPlayCallback) < 0)
                {
                    soundPlayState = SOUNDPLAY_NONE;
                    return 0;
                }
                soundPlayState = SOUNDPLAY_STATUS_CALORIE;
            }
            break;

        case SOUNDPLAY_STATUS_CALORIE:
            if (mainStatusCalorie >= 100)
            {
                value = mainStatusCalorie / 100;
                sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", value);

                if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                {
                    soundPlayState = SOUNDPLAY_NONE;
                    return 0;
                }
                soundPlayState = SOUNDPLAY_STATUS_CALORIE_HUNDRED_NUMBER;
            }
            else if (mainStatusCalorie >= 10)
            {
                value = mainStatusCalorie / 10;
                sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", value);

                if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                {
                    soundPlayState = SOUNDPLAY_NONE;
                    return 0;
                }
                soundPlayState = SOUNDPLAY_STATUS_CALORIE_TEN_NUMBER;
            }
            else
            {
                value = mainStatusCalorie;
                sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", value);

                if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                {
                    soundPlayState = SOUNDPLAY_NONE;
                    return 0;
                }
                soundPlayState = SOUNDPLAY_STATUS_CALORIE_NUMBER;
            }
            break;

        case SOUNDPLAY_STATUS_CALORIE_HUNDRED_NUMBER:
            if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/hundred.wav", MainAudioPlayCallback) < 0)
            {
                soundPlayState = SOUNDPLAY_NONE;
                return 0;
            }
            soundPlayState = SOUNDPLAY_STATUS_CALORIE_HUNDRED;
            break;

        case SOUNDPLAY_STATUS_CALORIE_HUNDRED:
            {
                value = mainStatusCalorie / 100;
                value = mainStatusCalorie - value * 10;

                if (value >= 10)
                {
                    value /= 10;
                    sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", value);

                    if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_CALORIE_TEN_NUMBER;
                }
                else if (value > 0)
                {
                    if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/0.wav", MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_CALORIE_TEN;
                }
                else
                {
                    if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/calorie.wav", MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_CALORIE_CALORIE;
                }
            }
            break;

        case SOUNDPLAY_STATUS_CALORIE_TEN_NUMBER:
            if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/ten.wav", MainAudioPlayCallback) < 0)
            {
                soundPlayState = SOUNDPLAY_NONE;
                return 0;
            }
            soundPlayState = SOUNDPLAY_STATUS_CALORIE_TEN;
            break;

        case SOUNDPLAY_STATUS_CALORIE_TEN:
            {
                value = mainStatusCalorie % 10;

                if (value > 0)
                {
                    sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", value);

                    if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_CALORIE_NUMBER;
                }
                else
                {
                    if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/calorie.wav", MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_CALORIE_CALORIE;
                }
            }
            break;

        case SOUNDPLAY_STATUS_CALORIE_NUMBER:
            if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/calorie.wav", MainAudioPlayCallback) < 0)
            {
                soundPlayState = SOUNDPLAY_NONE;
                return 0;
            }
            soundPlayState = SOUNDPLAY_STATUS_CALORIE_CALORIE;
            break;

        case SOUNDPLAY_STATUS_CALORIE_CALORIE:
            if (mainStatusPulse >= 1000)
            {
                soundPlayState = SOUNDPLAY_NONE;
            }
            else
            {
                if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/pulse.wav", MainAudioPlayCallback) < 0)
                {
                    soundPlayState = SOUNDPLAY_NONE;
                    return 0;
                }
                soundPlayState = SOUNDPLAY_STATUS_PULSE;
            }
            break;

        case SOUNDPLAY_STATUS_PULSE:
            if (mainStatusPulse >= 100)
            {
                value = mainStatusPulse / 100;
                sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", value);

                if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                {
                    soundPlayState = SOUNDPLAY_NONE;
                    return 0;
                }
                soundPlayState = SOUNDPLAY_STATUS_PULSE_HUNDRED_NUMBER;
            }
            else if (mainStatusPulse >= 10)
            {
                value = mainStatusPulse / 10;
                sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", value);

                if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                {
                    soundPlayState = SOUNDPLAY_NONE;
                    return 0;
                }
                soundPlayState = SOUNDPLAY_STATUS_PULSE_TEN_NUMBER;
            }
            else
            {
                value = mainStatusPulse;
                sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", value);

                if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                {
                    soundPlayState = SOUNDPLAY_NONE;
                    return 0;
                }
                soundPlayState = SOUNDPLAY_STATUS_PULSE_NUMBER;
            }
            break;

        case SOUNDPLAY_STATUS_PULSE_HUNDRED_NUMBER:
            if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/hundred.wav", MainAudioPlayCallback) < 0)
            {
                soundPlayState = SOUNDPLAY_NONE;
                return 0;
            }
            soundPlayState = SOUNDPLAY_STATUS_PULSE_HUNDRED;
            break;

            {
                value = mainStatusPulse / 100;
                value = mainStatusPulse - value * 10;

                if (value >= 10)
                {
                    value /= 10;
                    sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", value);

                    if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_PULSE_TEN_NUMBER;
                }
                else if (value > 0)
                {
                    if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/0.wav", MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_PULSE_TEN;
                }
                else
                {
                    soundPlayState = SOUNDPLAY_NONE;
                }
            }
            break;

        case SOUNDPLAY_STATUS_PULSE_TEN_NUMBER:
            if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/ten.wav", MainAudioPlayCallback) < 0)
            {
                soundPlayState = SOUNDPLAY_NONE;
                return 0;
            }
            soundPlayState = SOUNDPLAY_STATUS_PULSE_TEN;
            break;

        case SOUNDPLAY_STATUS_PULSE_TEN:
            {
                value = mainStatusPulse % 10;

                if (value > 0)
                {
                    sprintf(buf, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/%d.wav", value);

                    if (AudioPlay(buf, MainAudioPlayCallback) < 0)
                    {
                        soundPlayState = SOUNDPLAY_NONE;
                        return 0;
                    }
                    soundPlayState = SOUNDPLAY_STATUS_PULSE_NUMBER;
                }
                else
                {
                    soundPlayState = SOUNDPLAY_NONE;
                }
            }
            break;

        case SOUNDPLAY_STATUS_PULSE_NUMBER:
            soundPlayState = SOUNDPLAY_NONE;
            break;

        case SOUNDPLAY_WARN:
            soundPlayState = SOUNDPLAY_NONE;
            break;
        }
        break;
    }
    return 0;
}

static void MainProcessExternalPulse(ExternalEvent* ev)
{
    char buf[32];
    assert(ev);

    if (!mainPulseText)
        return;

    mainPulse = ev->arg1;

    if (mainState == MAIN_RUNNING)
    {
        sprintf(buf, "%d", mainPulse);
        ituTextSetString(mainPulseText, buf);
    }
}

static MainProcessExternalWarn(ExternalEvent* ev)
{
    assert(ev);

    if (ev->arg1 == EXTERNAL_WARN_NONE)
    {
        if (soundPlayState == SOUNDPLAY_WARN)
        {
            ituSlideshowPlay(mainSlideshow, -1);
            ituWidgetSetVisible(mainOverlay2Background, false);
            ituWidgetSetVisible(mainWarnContainer, false);
            ituWidgetEnable(mainOverlay1Background);
            mainState = MAIN_RUNNING;
            soundPlayState = SOUNDPLAY_NONE;
            mainBeginTime += mainPauseSeconds;
            mainPauseSeconds = 0;
            AudioStop();
        }
    }
    else
    {
        switch (ev->arg1)
        {
        case EXTERNAL_WARN_SAFETY:
            ituSlideshowStop(mainSlideshow);
            ituWidgetSetVisible(mainOverlay2Background, true);
            ituWidgetSetVisible(mainWarnContainer, true);
            ituWidgetDisable(mainOverlay1Background);
            ituSpritePlay(mainWarnSprite, 0);
            mainState = MAIN_WARNING;
            if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/safety.mp3", MainAudioPlayCallback) < 0)
                soundPlayState = SOUNDPLAY_NONE;
            else
                soundPlayState = SOUNDPLAY_WARN;

            time(&mainPauseTime);
            mainPauseSeconds = 0;
            mainLastTick = mainWarnSoundTick = itpGetTickCount();
            break;
        }
    }
}

void MainProcessExternalEvent(ExternalEvent* ev)
{
    assert(ev);
    switch (ev->type)
    {
    case EXTERNAL_PULSE:
        printf("EXTERNAL_PULSE: %d\n", ev->arg1);
        MainProcessExternalPulse(ev);
        break;

    case EXTERNAL_WARN:
        printf("EXTERNAL_WARN: %d\n", ev->arg1);
        MainProcessExternalWarn(ev);
        break;
    }
}

static void MainUpdateProgress(void)
{
    int value = 0;
    time_t now;
    char buf[64];

    switch (progressState)
    {
	case PROGRESS_PERCENTAGE:
		{
			switch (theProgram.prog)
			{
			case PROG_AEROBIC:
				value = 100 * mainStatusTime / theProgram.time;
				break;

			case PROG_FAT_BURNING:
				value = 100 * mainStatusCalorie / theProgram.calorie;
				break;

			case PROG_BUFFER:
				value = 100 * mainStatusPace / theProgram.pace;
				break;

			default:
				value = 100 * mainStatusDistance / theProgram.distance;
				break;
			}
            if (value > 100)
                value = 100;

			sprintf(buf, "%d%%", value);
		}
		break;

	case PROGRESS_DISTANCE:
		{
			value = 100 * mainStatusDistance / theProgram.distance;
			if (theConfig.unit_mile)
            {
                if (theProgram.distance > mainStatusDistance)
				    sprintf(buf, "%.1f mile", (theProgram.distance - mainStatusDistance) * 0.621371f / 1000.0f);
                else
                    sprintf(buf, "%.1f mile", 0.0f);
            }
			else
            {
                if (theProgram.distance > mainStatusDistance)
				    sprintf(buf, "%.1f km", (theProgram.distance - mainStatusDistance) / 1000.0f);
                else
                    sprintf(buf, "%.1f km", 0.0f);
            }
		}
        break;

	case PROGRESS_TIME:
		{
			int h, m, s, seconds;
			time(&now);
			seconds = (int)difftime(now, mainBeginTime);
			value = 100 * seconds / theProgram.time;

            if (theProgram.time > seconds)
    			seconds = theProgram.time - seconds;
            else
                seconds = 0;

			h = seconds / (60 * 60);
			m = seconds / 60 - h * 60;
			s = seconds % 60;
			sprintf(buf, "%2d:%02d:%02d", h, m, s);
		}
        break;

	case PROGRESS_PACE:
		value = 100 * mainStatusPace / theProgram.pace;
        if (theProgram.pace > mainStatusPace)
		    sprintf(buf, "%d pace", theProgram.pace - mainStatusPace);
        else
            sprintf(buf, "%d pace", 0);
		break;

    case PROGRESS_CALORIE:
        value = 100 * mainStatusCalorie / theProgram.calorie;
        if (theProgram.calorie > mainStatusCalorie)
		    sprintf(buf, "%d cal", theProgram.calorie - mainStatusCalorie);
        else
            sprintf(buf, "%d cal", 0);
        break;
    }

    if (value > 100)
        value = 100;

    ituProgressBarSetValue(mainProgressBar, value);
    ituTextSetString(mainProgressText, buf);
    ituTrackBarSetValue(mainTrackBar, value);
}

bool MainOnTimer(ITUWidget* widget, char* param)
{
    if (mainState == MAIN_COUNTDOWN)
    {
        if (itpGetTickDuration(mainLastTick) >= 1000)
        {
            mainLastTick = itpGetTickCount();
            if (--mainCountDown <= 0)
            {
                int h, m, s;
                char buf[32];

                ituWidgetSetVisible(mainOverlay2Background, false);
                ituWidgetSetVisible(mainCountDownSprite, false);
                ituSlideshowPlay(mainSlideshow, 0);
                ituWidgetEnable(mainOverlay1Background);

                if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/start.mp3", MainAudioPlayCallback) == 0)
                    soundPlayState = SOUNDPLAY_COUNTDOWN;

                ituWidgetSetVisible(mainOverlay1Background, false);
                ((ITUWidget*)mainOverlay1Background)->alpha = 255;
                ituWidgetShow(mainOverlay1Background, ITU_EFFECT_FADE, 1000 / MS_PER_FRAME + 1000 / MS_PER_FRAME * 30 / 100);
                ((ITUWidget*)mainOverlay1Background)->effect->currStep = 1000 / MS_PER_FRAME * 30 / 100;

                h = theProgram.time / (60 * 60);
                m = theProgram.time / 60 - h * 60;
                s = theProgram.time % 60;
                sprintf(buf, "%2d:%02d:%02d", h, m, s);
                ituTextSetString(mainGoalTimeText, buf);

                time(&mainBeginTime);

                mainState = MAIN_RUNNING;
                // TODO: IMPLEMENT
            }
            else
            {
                char path[PATH_MAX];

                ituSpriteGoto(mainCountDownSprite, 3 - mainCountDown);
                if (mainCountDown == 2)
                    ituAnimationPlay(mainCountDown2Animation, 0);
                else if (mainCountDown == 1)
                    ituAnimationPlay(mainCountDown1Animation, 0);

                sprintf(path, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/countdown%d.mp3", mainCountDown);
                if (AudioPlay(path, MainAudioPlayCallback) == 0)
                    soundPlayState = SOUNDPLAY_COUNTDOWN;
            }
            return true;
        }
    }
    else if (mainState == MAIN_RUNNING)
    {
        if (itpGetTickDuration(mainLastTick) >= 250)
        {
            char buf[32];
            int h, m, s, seconds;
            time_t now;

            mainLastTick = itpGetTickCount();
            time(&now);
            mainStatusTime = (int)difftime(now, mainBeginTime);

            if (theConfig.demo_enable)
            {
                char buf[64];

                mainStatusDistance = 6000.0f * mainStatusTime / (60.0f * 60.0f);

                if (theConfig.unit_mile)
                    sprintf(buf, "%.1f mile", mainStatusDistance * 0.621371f / 1000.0f);
                else
                    sprintf(buf, "%.1f km", mainStatusDistance / 1000.0f);

                ituTextSetString(mainDistanceText, buf);

                mainStatusPace = mainStatusDistance;
                sprintf(buf, "%d", mainStatusPace);
                ituTextSetString(mainPaceText, buf);

                mainStatusCalorie = 180 * mainStatusTime / (60 * 60);
                sprintf(buf, "%d", mainStatusCalorie);
                ituTextSetString(mainCalorieText, buf);
            }

            h = mainStatusTime / (60 * 60);
            m = mainStatusTime / 60 - h * 60;
            s = mainStatusTime % 60;
            sprintf(buf, "%2d:%02d:%02d", h, m, s);
            ituTextSetString(mainTimeText, buf);

            if (theProgram.time > mainStatusTime)
                seconds = theProgram.time - mainStatusTime;
            else
                seconds = 0;

            h = seconds / (60 * 60);
            m = seconds / 60 - h * 60;
            s = seconds % 60;
            sprintf(buf, "%2d:%02d:%02d", h, m, s);
            ituTextSetString(mainGoalTimeText, buf);

            MainUpdateProgress();

            return true;
        }
    }
    else if (mainState == MAIN_PAUSED)
    {
        if (itpGetTickDuration(mainLastTick) >= 250)
        {
            time_t now;

            mainLastTick = itpGetTickCount();

            time(&now);

            mainPauseSeconds = (int)difftime(now, mainPauseTime);
            if (mainPauseSeconds >= MAX_PAUSE_SEC)
            {
                ituLayerGoto(standbyLayer);
            }
            return true;
        }
    }
    else if (mainState == MAIN_WARNING)
    {
        if (itpGetTickDuration(mainLastTick) >= 4000)
        {
            time_t now;

            mainLastTick = itpGetTickCount();

            time(&now);
            mainPauseSeconds = (int)difftime(now, mainPauseTime);

            ituSpriteStop(mainWarnSprite);
            ituSpriteGoto(mainWarnSprite, 0);

            mainState = MAIN_WARNED;
        }
    }
    else if (mainState == MAIN_WARNED)
    {
        if (itpGetTickDuration(mainWarnSoundTick) >= 5000)
        {
            if (theConfig.demo_enable)
            {
                ituSlideshowPlay(mainSlideshow, -1);
                ituWidgetSetVisible(mainOverlay2Background, false);
                ituWidgetSetVisible(mainWarnContainer, false);
                ituWidgetEnable(mainOverlay1Background);
                mainState = MAIN_RUNNING;
                soundPlayState = SOUNDPLAY_NONE;
                mainBeginTime += mainPauseSeconds;
                mainPauseSeconds = 0;
                AudioStop();
            }
            else
            {
                time_t now;

                mainLastTick = mainWarnSoundTick = itpGetTickCount();

                time(&now);
                mainPauseSeconds = (int)difftime(now, mainPauseTime);

                if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/safety.mp3", MainAudioPlayCallback) < 0)
                    soundPlayState = SOUNDPLAY_NONE;
                else
                    soundPlayState = SOUNDPLAY_WARN;
            }
        }
    }

    if (ituWidgetIsVisible(mainVideo) && ituWidgetIsVisible(mainVideoMediaFileListBox) && !mainVideo->playing)
    {
        if ((mainVideoMediaFileListBox->mflistboxFlags & ITU_FILELIST_BUSYING) == 0 &&
            (mainVideoMediaFileListBox->mflistboxFlags & ITU_FILELIST_CREATED) == 0 &&
            (mainVideoMediaFileListBox->mflistboxFlags & ITU_FILELIST_DESTROYING) == 0)
        {
            ITUScrollText* item = ituMediaFileListPlay(mainVideoMediaFileListBox);
            if (item)
            {
                ITUText* text = (ITUText*)ituWheelGetFocusItem(mainSpeedWheel);
                float speed = (float)atof(ituTextGetString(text));
                char* filepath = (char*)ituWidgetGetCustomData(item);
                strcpy(mainVideo->filePath, filepath);

#ifdef CFG_VIDEO_ENABLE
                if (mtal_pb_get_audio_codec_id(mainVideo->filePath) != 0)
#endif
                {
                    //stop background audio
                    if (soundPlayState != SOUNDPLAY_NONE)
                    {
                        // TODO: FIX ME
                        AudioStop();
                        soundPlayState = SOUNDPLAY_NONE;
                    }
                }
                ituVideoPlay(mainVideo, 0);
                ituVideoSpeedUpDown(mainVideo, EvalVideoSpeed(speed));
            }
        }
    }
    return false;
}

bool MainTypeBackgroundButtonOnPress(ITUWidget* widget, char* param)
{
    if (theConfig.demo_enable)
    {
        ExternalEvent ev;

        ev.type = EXTERNAL_WARN;
        ev.arg1 = EXTERNAL_WARN_SAFETY;
        MainProcessExternalEvent(&ev);

        return true;
    }
    return false;
}

bool MainProgressBackgroundButtonOnPress(ITUWidget* widget, char* param)
{
    if (++progressState > PROGRESS_CALORIE)
        progressState = PROGRESS_PERCENTAGE;

    MainUpdateProgress();

    return true;
}

bool MainSlopeWheelOnChanged(ITUWidget* widget, char* param)
{
    ITUText* text = (ITUText*)ituWheelGetFocusItem((ITUWheel*)widget);
    printf("Slope: %s\n", ituTextGetString(text));

	ituSpriteGoto(mainSlopeSprite, 100 * (MAX_SLOPE_INDEX - mainSlopeWheel->focusIndex) / MAX_SLOPE_INDEX);

    return true;
}

bool MainSpeedWheelOnChanged(ITUWidget* widget, char* param)
{
    ITUText* text = (ITUText*)ituWheelGetFocusItem((ITUWheel*)widget);
    float speed = (float)atof(ituTextGetString(text));
    printf("Speed: %s, %f\n", ituTextGetString(text), speed);

    ituVideoSpeedUpDown(mainVideo, EvalVideoSpeed(speed));

	ituSpriteGoto(mainSpeedSprite, 100 * (MAX_SPEED_INDEX - mainSpeedWheel->focusIndex) / MAX_SPEED_INDEX);

    return true;
}

bool MainFanBackgroundButtonOnPress(ITUWidget* widget, char* param)
{
    if (++mainFanIndex > MAX_FAN_INDEX)
        mainFanIndex = 0;

    printf("Fan: %d\n", mainFanIndex);
    ituSpriteGoto(mainFanSprite, mainFanIndex);

    return true;
}

bool MainPausePopupButtonOnPress(ITUWidget* widget, char* param)
{
    ituSlideshowStop(mainSlideshow);
    ituVideoPause(mainVideo);
    time(&mainPauseTime);
    mainState = MAIN_PAUSED;
    mainPauseSeconds = 0;
    return true;
}

bool MainPlayPopupButtonOnPress(ITUWidget* widget, char* param)
{
    ituSlideshowPlay(mainSlideshow, -1);
    ituVideoPlay(mainVideo, 0);
    mainState = MAIN_RUNNING;
    mainBeginTime += mainPauseSeconds;
    mainPauseSeconds = 0;
    return true;
}

bool MainStopPopupButtonOnPress(ITUWidget* widget, char* param)
{
    if (mainVideo && mainVideo->playing)
    {
        ituVideoStop(mainVideo);
    }
    ituWidgetSetVisible(mainSlideshow, true);
    ituWidgetSetVisible(mainVideo, false);
    return true;
}

bool MainStatusBackgroundButtonOnPress(ITUWidget* widget, char* param)
{
    time_t now;

    time(&now);

    // TODO: IMPLEMENT
    mainStatusDistance = 0;
    mainStatusTime = (int)difftime(now, mainBeginTime);
    mainStatusCalorie = 0;
    mainStatusPulse = mainPulse;

    if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/distance.wav", MainAudioPlayCallback) < 0)
    {
        soundPlayState = SOUNDPLAY_NONE;
        return false;
    }
    soundPlayState = SOUNDPLAY_STATUS_DISTANCE;
    return true;
}

static void MainSlideshowOnStop(ITUSlideshow* slideshow)
{
    ITUScrollText* item;
    ituListBoxSelect(&mainVideoMediaFileListBox->listbox, 0);
    item = ituMediaFileListPlay(mainVideoMediaFileListBox);
    if (item)
    {
        ITUText* text = (ITUText*)ituWheelGetFocusItem(mainSpeedWheel);
        float speed = (float)atof(ituTextGetString(text));
        char* filepath = (char*)ituWidgetGetCustomData(item);
        strcpy(mainVideo->filePath, filepath);
#ifdef CFG_VIDEO_ENABLE
        if (mtal_pb_get_audio_codec_id(mainVideo->filePath) != 0)
#endif
        {
            //stop background audio
            if (soundPlayState != SOUNDPLAY_NONE)
            {
                // TODO: FIX ME
                AudioStop();
                soundPlayState = SOUNDPLAY_NONE;
            }
        }
        printf("play video %s\n", mainVideo->filePath);
        ituVideoPlay(mainVideo, 0);
        ituVideoSpeedUpDown(mainVideo, EvalVideoSpeed(speed));
        
        ituWidgetSetVisible(mainSlideshow, false);
        ituWidgetSetVisible(mainVideo, true);
    }
    else
    {
        ituSlideshowPlay(mainSlideshow, 0);
    }
}

static void MainVideoOnStop(ITUVideo* video)
{
    if (ituWidgetIsVisible(video))
    {
        ITUScrollText* item = ituMediaFileListNext(mainVideoMediaFileListBox);
        if (item)
        {
            ITUText* text = (ITUText*)ituWheelGetFocusItem(mainSpeedWheel);
            float speed = (float)atof(ituTextGetString(text));
            char* filepath = (char*)ituWidgetGetCustomData(item);
            strcpy(mainVideo->filePath, filepath);
#ifdef CFG_VIDEO_ENABLE
            if (mtal_pb_get_audio_codec_id(mainVideo->filePath) != 0)
            {
                //stop background audio
                if (soundPlayState != SOUNDPLAY_NONE)
                {
                    // TODO: FIX ME
                    AudioStop();
                    soundPlayState = SOUNDPLAY_NONE;
                }
            }
#endif // CFG_VIDEO_ENABLE
            printf("play video %s\n", mainVideo->filePath);
            ituVideoPlay(mainVideo, 0);
            ituVideoSpeedUpDown(mainVideo, EvalVideoSpeed(speed));
        }
        else
        {
            ituWidgetSetVisible(mainSlideshow, true);
            ituWidgetSetVisible(mainVideo, false);

            ituSlideshowPlay(mainSlideshow, 0);
        }
    }
}

void MainStorageOnInserted(char* path)
{
    if (!mainVideoMediaFileListBox)
    {
        mainVideoMediaFileListBox = ituSceneFindWidget(&theScene, "mainVideoMediaFileListBox");
        assert(mainVideoMediaFileListBox);
    }
    ituMediaFileListSetPath(mainVideoMediaFileListBox, path);
}

void MainStorageOnRemoved(void)
{
    if (mainVideo && mainVideo->playing)
    {
        ituVideoStop(mainVideo);
        ituWidgetSetVisible(mainSlideshow, true);
        ituWidgetSetVisible(mainVideo, false);
    }
}

bool MainOnEnter(ITUWidget* widget, char* param)
{
    if (!mainBackground)
    {
        mainBackground = ituSceneFindWidget(&theScene, "mainBackground");
        assert(mainBackground);

        mainSlideshow = ituSceneFindWidget(&theScene, "mainSlideshow");
        assert(mainSlideshow);
        ituSlideshowSetOnStop(mainSlideshow, MainSlideshowOnStop);

        mainVideo = ituSceneFindWidget(&theScene, "mainVideo");
        assert(mainVideo);
        ituVideoSetOnStop(mainVideo, MainVideoOnStop);

        mainVideoMediaFileListBox = ituSceneFindWidget(&theScene, "mainVideoMediaFileListBox");
        assert(mainVideoMediaFileListBox);

        mainOverlay1Background = ituSceneFindWidget(&theScene, "mainOverlay1Background");
        assert(mainOverlay1Background);

        mainOverlay2Background = ituSceneFindWidget(&theScene, "mainOverlay2Background");
        assert(mainOverlay2Background);

        mainCountDownSprite = ituSceneFindWidget(&theScene, "mainCountDownSprite");
        assert(mainCountDownSprite);

        mainCountDown3Animation = ituSceneFindWidget(&theScene, "mainCountDown3Animation");
        assert(mainCountDown3Animation);

        mainCountDown2Animation = ituSceneFindWidget(&theScene, "mainCountDown2Animation");
        assert(mainCountDown2Animation);

        mainCountDown1Animation = ituSceneFindWidget(&theScene, "mainCountDown1Animation");
        assert(mainCountDown1Animation);

        mainDistanceText = ituSceneFindWidget(&theScene, "mainDistanceText");
        assert(mainDistanceText);

        mainTimeText = ituSceneFindWidget(&theScene, "mainTimeText");
        assert(mainTimeText);

        mainPaceText = ituSceneFindWidget(&theScene, "mainPaceText");
        assert(mainPaceText);

        mainCalorieText = ituSceneFindWidget(&theScene, "mainCalorieText");
        assert(mainCalorieText);

        mainPulseText = ituSceneFindWidget(&theScene, "mainPulseText");
        assert(mainPulseText);

        mainGoalTimeText = ituSceneFindWidget(&theScene, "mainGoalTimeText");
        assert(mainGoalTimeText);

        mainProgressBar = ituSceneFindWidget(&theScene, "mainProgressBar");
        assert(mainProgressBar);

        mainProgressText = ituSceneFindWidget(&theScene, "mainProgressText");
        assert(mainProgressText);

        mainTrackBar = ituSceneFindWidget(&theScene, "mainTrackBar");
        assert(mainTrackBar);
        ituWidgetDisable(mainTrackBar);

        mainTrackButton = ituSceneFindWidget(&theScene, "mainTrackButton");
        assert(mainTrackButton);
        ituWidgetDisable(mainTrackButton);

        mainSlopeSprite = ituSceneFindWidget(&theScene, "mainSlopeSprite");
        assert(mainSlopeSprite);

        mainSlopeWheel = ituSceneFindWidget(&theScene, "mainSlopeWheel");
        assert(mainSlopeWheel);

        mainSpeedSprite = ituSceneFindWidget(&theScene, "mainSpeedSprite");
        assert(mainSpeedSprite);

        mainSpeedWheel = ituSceneFindWidget(&theScene, "mainSpeedWheel");
        assert(mainSpeedWheel);

        mainSpeedUnitSprite = ituSceneFindWidget(&theScene, "mainSpeedUnitSprite");
        assert(mainSpeedUnitSprite);

        mainFanSprite = ituSceneFindWidget(&theScene, "mainFanSprite");
        assert(mainFanSprite);

        standbyLayer = ituSceneFindWidget(&theScene, "standbyLayer");
        assert(standbyLayer);

        mainWarnContainer = ituSceneFindWidget(&theScene, "mainWarnContainer");
        assert(mainWarnContainer);

        mainWarnSprite = ituSceneFindWidget(&theScene, "mainWarnSprite");
        assert(mainWarnSprite);
    }

    if (theProgram.restart)
    {
        char buf[32];
        int h, m, s;

        theProgram.restart = false;

        mainState = MAIN_COUNTDOWN;
        progressState = PROGRESS_PERCENTAGE;
        ((ITUWidget*)mainOverlay1Background)->alpha = 255 * 30 / 100;
        ituWidgetDisable(mainOverlay1Background);
        ituWidgetSetVisible(mainOverlay2Background, true);
        ituWidgetSetVisible(mainCountDownSprite, true);
        ituSpriteGoto(mainCountDownSprite, 0);
        ituAnimationPlay(mainCountDown3Animation, 0);
        mainCountDown = 3;
        mainFanIndex = 0;

        ituSpriteGoto(mainSlopeSprite, 100 * (MAX_SLOPE_INDEX - SLOPE_INDEX) / MAX_SLOPE_INDEX);
        ituWheelGoto(mainSlopeWheel, SLOPE_INDEX);

		ituSpriteGoto(mainSpeedSprite, 100 * (MAX_SPEED_INDEX - SPEED_INDEX) / MAX_SPEED_INDEX);
        ituWheelGoto(mainSpeedWheel, SPEED_INDEX);

        if (theConfig.unit_mile)
            ituTextSetString(mainDistanceText, "0.0 mile");
        else
            ituTextSetString(mainDistanceText, "0.0 km");

        ituTextSetString(mainTimeText, "0:00:00");
        ituTextSetString(mainPaceText, "0");
        ituTextSetString(mainCalorieText, "0");
        ituTextSetString(mainPulseText, "0");

        h = theProgram.time / (60 * 60);
        m = theProgram.time / 60 - h * 60;
        s = theProgram.time % 60;
        sprintf(buf, "%2d:%02d:%02d", h, m, s);

        ituTextSetString(mainGoalTimeText, buf);

        if (AudioPlay(CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/countdown3.mp3", MainAudioPlayCallback) == 0)
            soundPlayState = SOUNDPLAY_COUNTDOWN;

        mainPauseSeconds = 0;
		mainPulse = mainStatusDistance = mainStatusCalorie = mainStatusPulse = mainStatusPace = 0;

        mainLastTick = itpGetTickCount();
    }
    else
    {
        ituWidgetSetVisible(mainOverlay2Background, false);
        ituWidgetSetVisible(mainCountDownSprite, false);
        ituWidgetEnable(mainOverlay1Background);
    }

    if (theConfig.unit_mile)
        ituSpriteGoto(mainSpeedUnitSprite, 1);
    else
        ituSpriteGoto(mainSpeedUnitSprite, 0);

    mainVideo->volume = theConfig.audiolevel;

    return true;
}

bool MainOnLeave(ITUWidget* widget, char* param)
{
    ituWidgetDisable(mainOverlay1Background);
    return true;
}

void MainReset(void)
{
    mainBackground = NULL;
    SceneSetReady(false);
}

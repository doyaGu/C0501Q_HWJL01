#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "ite/audio.h"
#include "audio_mgr.h"
#include "airconditioner.h"

static bool                       audioInited;
extern bool                       audioKeySoundPaused;
static AudioPlayCallback          audioPlayCallback;
static pthread_mutex_t            audioPlayMutex;

extern MMP_INT smtkAudioMgrPlayNetwork(SMTK_AUDIO_PARAM_NETWORK* pNetwork);

static int AudioPlayCallbackFunc(int state);

static void *               AL_Local_player_http_handle = NULL;

#define AUDIOLINK_LOCAL_PLAYER_BUFFER_LEN            (64 * 1024)

const int VolumSetLevel[] = {40,45,50,55,60,65,70,75,80,85,90};

static SoundType audioNextSound;

const char* soundFileNames[SOUND_MAX] =
{
    "key.wav",
    "19.wav",
    "20.wav",
    "10.wav",
    "12.wav",
    "11.wav",
    "13.wav",
    "14.wav",
    "22.wav",
    "23.wav",
    "24.wav",
    "25.wav",
    "26.wav",
    "27.wav",
    "28.wav",
    "29.wav",
    "30.wav",
    "31.wav",
    "32.wav",
    "33.wav",
    "34.wav",
    "35.wav",
    "3.wav",
    "4.wav",
    "39.wav",
    "64.wav",
    "63.wav",
    "153.wav",
    "154.wav",
    "155.wav",
    "152.wav",
    "21.wav",
    "5.wav",
    "36.wav",
};

void AudioInit(void)
{
    smtkAudioMgrInitialize();
    smtkAudioMgrSetCallbackFunction((int*)AudioPlayCallbackFunc);

    audioKeySoundPaused = false;
    audioInited         = false;

    pthread_mutex_init(&audioPlayMutex, NULL);

    smtkAudioMgrSetVolume(theConfig.audiolevel);
    audioNextSound = SOUND_KEY;

    audioInited = true;
}

void AudioExit(void)
{
    if (!audioInited)
    {
        return;
    }

    if (AudioIsPlaying())
    {
        AudioStop();
    }

    smtkAudioMgrTerminate();
    pthread_mutex_destroy(&audioPlayMutex);

    itp_codec_standby();
}

static int AudioPlayCallbackFunc(int state)
{
    if (audioPlayCallback)
    {
        return audioPlayCallback(state);
    }
    return 0;
}

static int AudioPlayCallbackFuncEmpty(int state)
{
    // DO NOTHING
    return 0;
}

int AudioPlay(char* filename, AudioPlayCallback func)
{
    void *                   local_player_http_handle = NULL;
    SMTK_AUDIO_PARAM_NETWORK audiomgr_local;
    int                      nResult                  = 0;
    char*                    ext;

    ext = strrchr(filename, '.');
    if (!ext)
    {
        printf("Invalid file name: %s\n", filename);
        return -1;
    }
    ext++;


    pthread_mutex_lock(&audioPlayMutex);

#ifdef __OPENRTOS__
    smtkAudioMgrQuickStop();
#endif

    // close handler (if any)
    if (AL_Local_player_http_handle)
    {
        fclose(AL_Local_player_http_handle);
        AL_Local_player_http_handle = NULL;
    }

    audiomgr_local.audioMgrCallback = AudioPlayCallbackFunc;

    if (stricmp(ext, "wav") == 0)
    {
        audiomgr_local.nType            = SMTK_AUDIO_WAV;
        audiomgr_local.audioMgrCallback = func;
    }
    else if (stricmp(ext, "mp3") == 0)
    {
        audiomgr_local.nType = SMTK_AUDIO_MP3;
        audioPlayCallback  = func;
    }
    else if (stricmp(ext, "wma") == 0)
    {
        audiomgr_local.nType = SMTK_AUDIO_WMA;
        audioPlayCallback  = func;
    }
    else
    {
        printf("Unsupport file format: %s\n", ext);
        pthread_mutex_unlock(&audioPlayMutex);
        return -1;
    }

    if (filename)
    {
        AL_Local_player_http_handle = fopen(filename, "rb");
    }
    if (!AL_Local_player_http_handle)
    {
        printf("[Main]%s() L#%ld: fopen error \r\n", __FUNCTION__, __LINE__);
        pthread_mutex_unlock(&audioPlayMutex);
        return -1;
    }
    printf("[Main]%s() L#%ld:  %s success \r\n", __FUNCTION__, __LINE__, filename);
    audiomgr_local.pHandle     = AL_Local_player_http_handle;
    audiomgr_local.LocalRead   = fread;
    audiomgr_local.nReadLength = AUDIOLINK_LOCAL_PLAYER_BUFFER_LEN;
    audiomgr_local.bSeek       = 0;
    audiomgr_local.nM4A        = 0;
    audiomgr_local.bLocalPlay  = 1;
    audiomgr_local.pFilename   = filename;

    nResult                    = smtkAudioMgrPlayNetwork(&audiomgr_local);

    pthread_mutex_unlock(&audioPlayMutex);
    return 0;
}

void AudioStop(void)
{
    audioPlayCallback = NULL;
    smtkAudioMgrQuickStop();
}

void AudioPauseKeySound(void)
{
    audioKeySoundPaused = true;
}

void AudioResumeKeySound(void)
{
    audioKeySoundPaused = false;
}
void AudioMute(void)
{
    smtkAudioMgrMuteOn();
}

void AudioUnMute(void)
{
    smtkAudioMgrMuteOff();
}

bool AudioIsMuted(void)
{
    return smtkAudioMgrIsMuteOn() ? true : false;
}

bool AudioIsPlaying(void)
{
    return smtkAudioMgrGetState() == SMTK_AUDIO_PLAY;
}

void AudioSetVolume(int level)
{
    if (level < 0 || level > 100)
    {
        return;
    }

    theConfig.audiolevel = level;
	
    smtkAudioMgrSetVolume(theConfig.audiolevel);
}

int AudioGetVolume(void)
{
    return theConfig.audiolevel;
}

static int AudioSoundPlayCallback(int state)
{
    switch (state)
    {
    case AUDIOMGR_STATE_CALLBACK_PLAYING_FINISH:
        if (audioNextSound != SOUND_KEY)
        {
            char path[PATH_MAX];

            sprintf(path, CFG_PRIVATE_DRIVE ":/sound/%s", soundFileNames[audioNextSound]);
            AudioPlay(path, AudioSoundPlayCallback);

            if (audioNextSound >= SOUND_TEMPERATURE_17 && audioNextSound <= SOUND_TEMPERATURE_20)
                audioNextSound = SOUND_TEMPERATURE_LOW;
            else
                audioNextSound = SOUND_KEY;
        }
        break;
    }
    return 0;
}

void AudioPlaySound(SoundType sound)
{
    assert(sound < SOUND_MAX);

    if (!AudioIsMuted())
    {
        char path[PATH_MAX];

        if (sound == SOUND_KEY && (audioKeySoundPaused || AudioIsPlaying()))
            return;

        if (sound >= SOUND_TEMPERATURE_17 && sound <= SOUND_TEMPERATURE_30)
        {
            if (audioNextSound >= SOUND_TEMPERATURE_17 && audioNextSound <= SOUND_TEMPERATURE_30)
            {
                audioNextSound = sound;
                return;
            }
            audioNextSound = sound;
            sound = SOUND_TEMPERATURE;
        }
        else if (sound == SOUND_POWER_OFF)
        {
            audioNextSound = SOUND_CLOSEDOOR;
        }
        else
        {
            audioNextSound = SOUND_KEY;
        }

        sprintf(path, CFG_PRIVATE_DRIVE ":/sound/%s", soundFileNames[sound]);
        AudioPlay(path, AudioSoundPlayCallback);
    }
    else
    {
        audioNextSound = SOUND_KEY;
    }
    //printf("audioNextSound=%d\n", audioNextSound);
}
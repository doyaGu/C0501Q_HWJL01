#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ite/audio.h"
#include "audio_mgr.h"
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

#define AUDIOMGR_LOCAL_PLAYER_BUFFER_LEN            (64 * 1024)

FILE* fp = NULL;

extern MMP_INT smtkAudioMgrPlayNetwork(SMTK_AUDIO_PARAM_NETWORK* pNetwork);

static const char audioName[] = "ITUAudio";

static void AudioOnStop(ITUAudio* audio)
{
    // DO NOTHING
}

bool ituAudioUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    ITUAudio* audio = (ITUAudio*) widget;
    assert(audio);

    if ((widget->flags & ITU_ENABLED) == 0)
        return false;

    if (ev == ITU_EVENT_TIMER)
    {
        if (audio->audioFlags & ITU_AUDIO_STOPPING)
        {
            if (audio->audioFlags & ITU_AUDIO_REPEAT)
            {
                ituAudioPlay(audio);
            }
            else
            {
                LOG_DBG "audio stopped\n" LOG_END
                ituAudioOnStop(audio);
                ituExecActions(widget, audio->actions, ITU_EVENT_STOPPED, 0);
                audio->audioFlags &= ~ITU_AUDIO_PLAYING;
                ituScene->playingAudio = NULL;
            }
            audio->audioFlags &= ~ITU_AUDIO_STOPPING;
        }
    }
    else if (ev == ITU_EVENT_LAYOUT)
    {
        if ((audio->audioFlags & ITU_AUDIO_PLAYING)
        #ifdef CFG_BUILD_AUDIO_MGR
            && (smtkAudioMgrGetState() != SMTK_AUDIO_PLAY)
        #endif
            )
        {
            ituAudioPlay(audio);
        }
    }
    return false;
}

void ituAudioOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    ITUAudio* audio = (ITUAudio*) widget;
    assert(audio);

    switch (action)
    {
    case ITU_ACTION_PLAY:
        if (widget->flags & ITU_ENABLED)
            ituAudioPlay(audio);
        break;

    case ITU_ACTION_STOP:
        if (widget->flags & ITU_ENABLED)
        {
            ituAudioStop(audio);
            ituAudioOnStop(audio);
            ituExecActions(widget, audio->actions, ITU_EVENT_STOPPED, 0);
        }
        break;

    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

void ituAudioInit(ITUAudio* audio)
{
    assert(audio);
    ITU_ASSERT_THREAD();

    memset(audio, 0, sizeof (ITUAudio));

    ituWidgetInit(&audio->widget);

    ituWidgetSetType(audio, ITU_AUDIO);
    ituWidgetSetName(audio, audioName);
    ituWidgetSetUpdate(audio, ituAudioUpdate);
    ituWidgetSetOnAction(audio, ituAudioOnAction);
    ituAudioSetOnStop(audio, AudioOnStop);
}

void ituAudioLoad(ITUAudio* audio, uint32_t base)
{
    assert(audio);

    ituWidgetLoad(&audio->widget, base);
    ituWidgetSetUpdate(audio, ituAudioUpdate);
    ituWidgetSetOnAction(audio, ituAudioOnAction);
    ituAudioSetOnStop(audio, AudioOnStop);
}

static int AudioPlayCallback(int state)
{
    LOG_DBG "audio play state: %d\n", state LOG_END

    if (ituScene->playingAudio)
    {
        if (state == AUDIOMGR_STATE_CALLBACK_PLAYING_FINISH)
        {
            ituScene->playingAudio->audioFlags |= ITU_AUDIO_STOPPING;
        }
    }
    return 0;
}

void ituAudioPlay(ITUAudio* audio)
{
    assert(audio);
    ITU_ASSERT_THREAD();

    if ((audio->widget.flags & ITU_ENABLED) == 0)
        return;

    LOG_DBG "playing %s\n", audio->filePath LOG_END

    if(fp) fclose(fp);
    
#ifdef CFG_BUILD_AUDIO_MGR
    if (audio->filePath[0] != '\0')
    {
        char* p = strrchr(audio->filePath, '.');
        if (p)
        {
            SMTK_AUDIO_PARAM_NETWORK audiomgr_local;

            if (stricmp(p, ".mp3") == 0)
                audiomgr_local.nType = SMTK_AUDIO_MP3;
            else if (stricmp(p, ".wma") == 0)
                audiomgr_local.nType = SMTK_AUDIO_WMA;
            else if (stricmp(p, ".wav") == 0)
                audiomgr_local.nType = SMTK_AUDIO_WAV;
            else
                audiomgr_local.nType = 0;

            if (audiomgr_local.nType != 0)
            {
                
                #ifdef __OPENRTOS__
                    smtkAudioMgrQuickStop();
                #endif
                
                if(fp) {
                    fclose(fp);
                    fp = NULL;
                }
                
                fp = fopen(audio->filePath, "rb");
                if (fp)
                {
                    int result;

                    smtkAudioMgrSetCallbackFunction((int*)AudioPlayCallback);

                    audiomgr_local.audioMgrCallback = AudioPlayCallback;

                    audiomgr_local.pHandle     = fp;
                    audiomgr_local.LocalRead   = fread;
                    audiomgr_local.nReadLength = AUDIOMGR_LOCAL_PLAYER_BUFFER_LEN;
                    audiomgr_local.bSeek       = 0;
                    audiomgr_local.nM4A        = 0;
                    audiomgr_local.bLocalPlay  = 1;
                    audiomgr_local.pFilename   = audio->filePath;

                    if (audio->audioFlags & ITU_AUDIO_VOLUME)
                        smtkAudioMgrSetVolume(audio->volume);

                    result = smtkAudioMgrPlayNetwork(&audiomgr_local);
                    if (result)
                    {
                        ituScene->playingAudio = audio;
                        audio->audioFlags |= ITU_AUDIO_PLAYING;
                    }
                }

            }

        }
    }
#endif // CFG_BUILD_AUDIO_MGR
}

void ituAudioStop(ITUAudio* audio)
{
    assert(audio);
    ITU_ASSERT_THREAD();

    if ((audio->widget.flags & ITU_ENABLED) == 0)
        return;

#if defined(CFG_BUILD_AUDIO_MGR) && defined(__OPENRTOS__)
    smtkAudioMgrQuickStop();
#endif

    if(fp) {
        fclose(fp);
        fp = NULL;
    }
    audio->audioFlags &= ~ITU_AUDIO_PLAYING;
    ituScene->playingAudio = NULL;
}

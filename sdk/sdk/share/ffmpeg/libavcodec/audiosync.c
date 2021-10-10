#include "audiosync.h"

#define ENABLE_STC_SYNC
static ITE_AUDIO_DECODER gtAudioDecoder = { 0 };
static bool              gbDropAudio;
// Audio sync STC API
void
audioSyncSTC(AVCodecContext *avctx)
{
    // get stcTime
    unsigned int stcTime           = 0;//STC - gtAudioDecoder.nSTCStartTime //mmpSTCGetTime()- TsSync_GetBaseTimeStamp(TS_SYNC_STC_TIME_STAMP) - gtAudioDecoder.nPauseSTCTime;
    unsigned int audioEngTimeStamp = 0;
    unsigned int nAudio            = iteAudioGetDecodeTimeV2(&audioEngTimeStamp);
    unsigned int nTemp;

    int          nSTCAudioGap      = stcTime - nAudio;
    bool         bMPlayerReady; // start audio sync

    // get audio status
    iteAudioGetAttrib(ITE_AUDIO_DECODE_DROP_DATA, &nTemp);

    // using play movie
    //if (bMPlayer==MMP_FALSE)
    //{
    //    return;
    //}

#if 0
    dbg_msg(DBG_MSG_TYPE_ERROR, "audio gap aud: %u,stc: %u, %d %d %d %d\n",
            nAudio,
            stcTime,
            nSTCAudioGap,
            gtAudioDecoder.nSTCDifferent,
            nSTCAudioGap - gtAudioDecoder.nPreSTCAudioGap,
            mmpSTCGetTime());
#endif

#ifdef ENABLE_STC_SYNC

    // stop drop audio
    if (nTemp >= AUDIO_BEHIND_STC_GAP)
    {
        if (nSTCAudioGap > gtAudioDecoder.nSTCDifferent && nSTCAudioGap - gtAudioDecoder.nSTCDifferent < STC_DROP_AUDIO_GAP)
        {
            // stop drop audio
            gtAudioDecoder.nAudioBehindSTC = 0;
            iteAudioSetAttrib(ITE_AUDIO_DECODE_DROP_DATA, (void *)&gtAudioDecoder.nAudioBehindSTC);
            av_log(avctx, AV_LOG_DEBUG, "[Audio Sync] stop drop audio %d %d #line %d\n", nSTCAudioGap, gtAudioDecoder.nSTCDifferent, __LINE__);
            audioSetSTCDifferent(nSTCAudioGap);
            audioSetDropAudio(false);
        }
        else if (nSTCAudioGap < gtAudioDecoder.nSTCDifferent && gtAudioDecoder.nSTCDifferent - nSTCAudioGap < STC_DROP_AUDIO_GAP)
        {
            // stop drop audio
            gtAudioDecoder.nAudioBehindSTC = 0;
            iteAudioSetAttrib(ITE_AUDIO_DECODE_DROP_DATA, (void *)&gtAudioDecoder.nAudioBehindSTC);
            av_log(avctx, AV_LOG_DEBUG, "[Core] stop drop audio %d %d #line %d\n", nSTCAudioGap, gtAudioDecoder.nSTCDifferent, __LINE__);
            audioSetSTCDifferent(nSTCAudioGap);
            audioSetDropAudio(false);
        }
        else if (gtAudioDecoder.nPreSTCAudioGap - nSTCAudioGap > STC_DROP_AUDIO_GAP)
        {
            // stop drop audio
            gtAudioDecoder.nAudioBehindSTC = 0;
            iteAudioSetAttrib(ITE_AUDIO_DECODE_DROP_DATA, (void *)&gtAudioDecoder.nAudioBehindSTC);
            av_log(avctx, AV_LOG_DEBUG, "[Core] stop drop audio %d %d #line %d\n", nSTCAudioGap, gtAudioDecoder.nSTCDifferent, __LINE__);
            audioSetSTCDifferent(nSTCAudioGap);
            audioSetDropAudio(false);
        }
        else if (gtAudioDecoder.nSTCDifferent < 0)
        {
            // stop drop audio
            gtAudioDecoder.nAudioBehindSTC = 0;
            iteAudioSetAttrib(ITE_AUDIO_DECODE_DROP_DATA, (void *)&gtAudioDecoder.nAudioBehindSTC);
            av_log(avctx, AV_LOG_DEBUG, "[Core] stop drop audio %d %d #line %d\n", nSTCAudioGap, gtAudioDecoder.nSTCDifferent, __LINE__);
            audioSetSTCDifferent(nSTCAudioGap);
            audioSetDropAudio(false);
        }
    }
    //start drop audio
    if (nTemp == 0 && gtAudioDecoder.nSTCDifferent && nSTCAudioGap - gtAudioDecoder.nPreSTCAudioGap > 10 && nSTCAudioGap - gtAudioDecoder.nPreSTCAudioGap < AUDIO_BEHIND_STC_GAP)
    {
        av_log(avctx, AV_LOG_DEBUG, "[Core] audio behind stc %d %d\n", nSTCAudioGap - gtAudioDecoder.nPreSTCAudioGap, gtAudioDecoder.nAudioBehindSTC);
        gtAudioDecoder.nAudioBehindSTC += nSTCAudioGap - gtAudioDecoder.nPreSTCAudioGap;
        if (gtAudioDecoder.nAudioBehindSTC >= AUDIO_BEHIND_STC_GAP)
        {
            av_log(avctx, AV_LOG_DEBUG, "[Core] start drop audio #line %d\n", __LINE__);
            // start drop audio
            iteAudioSetAttrib(ITE_AUDIO_DECODE_DROP_DATA, (void *)&gtAudioDecoder.nAudioBehindSTC);
            audioSetDropAudio(true);
        }
    }

    // start STC
    // Castor3 change to check sync modle API
    // if it the API return null,audio can not play decoded data

    #if 0 // psudeo code
    if (gtAudioDecoder.nSTCDifferent == 0 && bMPlayerReady == true)
    {
        if (sync_modle say Yes)
        {
            audioSetSTCDifferent(1);

            gtAudioDecoder.nSTCStartTime = STC_GET_TIME();
            audioSetDropAudio(false);
            bMPlayerReady                = false;
            iteAudioSetAttrib(ITE_AUDIO_MPLAYER_STC_READY, &bMPlayerReady);
            iteAudioGetAttrib(ITE_AUDIO_DECODE_ERROR, &nTemp);
        }
        else
        {
            break;
        }
    }
    #endif
    /*if (gtAudioDecoder.nSTCDifferent == 0 && bMPlayerReady == true)
       {
        audioSetSTCDifferent(stcTime-nAudio);
        audioSetDropAudio(false);
        bMPlayerReady = false;
        iteAudioSetAttrib(ITE_AUDIO_MPLAYER_STC_READY, &bMPlayerReady);
        iteAudioGetAttrib(ITE_AUDIO_DECODE_ERROR, &nTemp);
        // audio adjust before video init
        if (gtAudioDecoder.nSTCDifferent < 0 && nTemp > 0)
        {
            nTemp = 0;
            iteAudioSetAttrib(ITE_AUDIO_DECODE_ERROR, &nTemp);
            if (gtAudioDecoder.nSTCDifferent+100 < 0)
            {
                nTemp = -gtAudioDecoder.nSTCDifferent;
                audioSetSTCDifferent(1);
            }
            else
            {
                nTemp = 100;
                audioSetSTCDifferent(gtAudioDecoder.nSTCDifferent+nTemp);
            }
            av_log(avctx, AV_LOG_DEBUG,"[Core] init audio sleep %d \n",nTemp);
            gInitSTCDiff = nTemp;
            iteAudioSTCSyncPause(true);
            if (nTemp > 500)
                PalSleep(500);
            else
                PalSleep(nTemp);
            iteAudioSTCSyncPause(false);
            gtAudioDecoder.nPreSTCAudioGap = nSTCAudioGap+nTemp;
            return;
        }
       }*/

    // check different of STC and audio engine time,audio engine time +100
    if (gtAudioDecoder.nSTCDifferent && (gtAudioDecoder.nSTCDifferent - nSTCAudioGap > 100))
    {
        av_log(avctx, AV_LOG_DEBUG, "[Core] audio adjust %d\n", gtAudioDecoder.nSTCDifferent - nSTCAudioGap);
        // if audio decoder decode error and adjust audio time, sleep the time gap
        iteAudioGetAttrib(ITE_AUDIO_DECODE_ERROR, &nTemp);
        //if (nTemp)
        {
            nTemp = 0;
            iteAudioSetAttrib(ITE_AUDIO_DECODE_ERROR, &nTemp);
            //printf("[Core]audio decode error,pause %d\n",gtAudioDecoder.nSTCDifferent-nSTCAudioGap);
            iteAudioSTCSyncPause(true);
            if ((gtAudioDecoder.nSTCDifferent - nSTCAudioGap) > 500)
                PalSleep(500);
            else
                PalSleep(gtAudioDecoder.nSTCDifferent - nSTCAudioGap);
            iteAudioSTCSyncPause(false);
            gtAudioDecoder.nPreSTCAudioGap = nSTCAudioGap + gtAudioDecoder.nSTCDifferent - nSTCAudioGap;
        }
        return;
    }

    // audio engine time -100, pre audio gap > audio gap
    if (gtAudioDecoder.nSTCDifferent && nSTCAudioGap - gtAudioDecoder.nPreSTCAudioGap > 100)
    {
        av_log(avctx, AV_LOG_DEBUG, "[Core] start drop audio %d #line %d\n", nSTCAudioGap - gtAudioDecoder.nPreSTCAudioGap, __LINE__);
        gtAudioDecoder.nAudioBehindSTC = nSTCAudioGap - gtAudioDecoder.nPreSTCAudioGap;
        // start drop audio
        iteAudioSetAttrib(ITE_AUDIO_DECODE_DROP_DATA, (void *)&gtAudioDecoder.nAudioBehindSTC);
        audioSetDropAudio(true);
    }
    gtAudioDecoder.nPreSTCAudioGap = nSTCAudioGap;

#endif //ENABLE_STC_SYNC
}

void
audioSetSTCDifferent(
    int nValue)
{
    gtAudioDecoder.nSTCDifferent = nValue;
    if (gtAudioDecoder.nSTCDifferent == 0)
    {
        gtAudioDecoder.nSTCDifferent = 1;
    }
}

void
audioSetDropAudio(
    bool nValue)
{
    gbDropAudio = nValue;
    if (gbDropAudio == true)
    {
        //gtStartDropAudioTime = PalGetClock();
    }
}
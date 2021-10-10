#include <stdio.h>
#include "avcodec.h"
#include "ite/audio.h"

#define STC_DROP_AUDIO_GAP       50
#define AUDIO_BEHIND_STC_GAP     80
#define DROP_AUDIO_DIRECTLY_TIME 500

void audioSyncSTC(AVCodecContext *avctx);
void audioSetSTCDifferent(int nValue);
void audioSetDropAudio(bool nValue);
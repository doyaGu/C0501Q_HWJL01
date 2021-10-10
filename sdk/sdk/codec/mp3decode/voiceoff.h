
#ifndef __VOICEOFF_H__
#define __VOICEOFF_H__

#include "type.h"

void voiceoff_init(int sample_rate);
void voiceoff_filter(SAMPBUF *IOBuf);

#endif // __VOICEOFF_H__

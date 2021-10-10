
// Example of filter settings
// comb filter0     {15ms, 0.35}
// comb filter1     {40ms, 0.25}
// allpass filter   {30ms, 0.45}

#ifndef __REVERB_H__
#define __REVERB_H__

#include "mp3_config.h"
#include "type.h"

#define NCOMB                   2
#define REVERB_PRECISION        28
#define DATA_CONST(A)           ((int)(((A)*(1<<REVERB_PRECISION)+0.5)))
#define MUL(x,coef)             MULSHIFT(x, coef, REVERB_PRECISION)
#define MAX_FILTER_BUFSIZE      (MAX_FRAMESIZE * MAX_FRAMEDELAY)
#define REVERB_BUF_SIZE         ((MAX_FILTER_BUFSIZE*MAX_NCHAN*(NCOMB+4))*sizeof(int))

// number of channel, sample rate, reverb buffer pointer
void reverb_init(int nch, int sr, int *bufptr);
void reverb_filter(SAMPBUF *InBuf, SAMPBUF *OutBuf, param_reverb_struct *p_param);

#endif  // __REVERB_H__


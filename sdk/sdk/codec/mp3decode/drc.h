#ifndef __DRC_H__
#define __DRC_H__

#include "type.h"

void drc_init(int nch, int nsamples);
void drc_filter(SAMPBUF *InBuf, SAMPBUF *OutBuf, param_drc_struct *p_param);
void delay_filter(SAMPBUF *InBuf, SAMPBUF *OutBuf);

#endif // __DRC_H__


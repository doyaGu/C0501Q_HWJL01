/*
 * WMA compatible decoder
 * Copyright (c) 2002 The FFmpeg Project.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __FFT_H__
#define __FFT_H__

#include "types.h"

typedef fixed32 FFTSample;

typedef struct FFTComplex
{
    fixed32 re, im;
}
FFTComplex;

typedef struct FFTContext
{
    int nbits;
    int inverse;
    uint16_t *revtab;
    FFTComplex *exptab;
    FFTComplex *exptab1; /* only used by SSE code */
    int (*fft_calc)(struct FFTContext *s, FFTComplex *z);
}
FFTContext;

int fft_calc_unscaled(FFTContext *s, fixed32 *cos_tab, FFTComplex *z);
int fft_init_global(void);

#endif // __FFT_H__


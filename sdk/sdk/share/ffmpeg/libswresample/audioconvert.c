﻿/*
 * audio conversion
 * Copyright (c) 2006 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * audio conversion
 * @author Michael Niedermayer <michaelni@gmx.at>
 */

#include "libavutil/avstring.h"
#include "libavutil/avassert.h"
#include "libavutil/libm.h"
#include "libavutil/samplefmt.h"
#include "audioconvert.h"


typedef void (conv_func_type)(uint8_t *po, const uint8_t *pi, int is, int os, uint8_t *end);

struct AudioConvert {
    int channels;
    conv_func_type *conv_f;
    const int *ch_map;
    uint8_t silence[8]; ///< silence input sample
};

#define CONV_FUNC_NAME(dst_fmt, src_fmt) conv_ ## src_fmt ## _to_ ## dst_fmt

//FIXME rounding ?
#define CONV_FUNC(ofmt, otype, ifmt, expr)\
static void CONV_FUNC_NAME(ofmt, ifmt)(uint8_t *po, const uint8_t *pi, int is, int os, uint8_t *end)\
{\
    do{\
        *(otype*)po = expr; pi += is; po += os;\
    }while(po < end);\
}

//FIXME put things below under ifdefs so we do not waste space for cases no codec will need
CONV_FUNC(AV_SAMPLE_FMT_U8 , uint8_t, AV_SAMPLE_FMT_U8 ,  *(const uint8_t*)pi)
CONV_FUNC(AV_SAMPLE_FMT_S16, int16_t, AV_SAMPLE_FMT_U8 , (*(const uint8_t*)pi - 0x80)<<8)
CONV_FUNC(AV_SAMPLE_FMT_S32, int32_t, AV_SAMPLE_FMT_U8 , (*(const uint8_t*)pi - 0x80)<<24)
CONV_FUNC(AV_SAMPLE_FMT_FLT, float  , AV_SAMPLE_FMT_U8 , (*(const uint8_t*)pi - 0x80)*(1.0 / (1<<7)))
CONV_FUNC(AV_SAMPLE_FMT_DBL, double , AV_SAMPLE_FMT_U8 , (*(const uint8_t*)pi - 0x80)*(1.0 / (1<<7)))
CONV_FUNC(AV_SAMPLE_FMT_U8 , uint8_t, AV_SAMPLE_FMT_S16, (*(const int16_t*)pi>>8) + 0x80)
CONV_FUNC(AV_SAMPLE_FMT_S16, int16_t, AV_SAMPLE_FMT_S16,  *(const int16_t*)pi)
CONV_FUNC(AV_SAMPLE_FMT_S32, int32_t, AV_SAMPLE_FMT_S16,  *(const int16_t*)pi<<16)
CONV_FUNC(AV_SAMPLE_FMT_FLT, float  , AV_SAMPLE_FMT_S16,  *(const int16_t*)pi*(1.0 / (1<<15)))
CONV_FUNC(AV_SAMPLE_FMT_DBL, double , AV_SAMPLE_FMT_S16,  *(const int16_t*)pi*(1.0 / (1<<15)))
CONV_FUNC(AV_SAMPLE_FMT_U8 , uint8_t, AV_SAMPLE_FMT_S32, (*(const int32_t*)pi>>24) + 0x80)
CONV_FUNC(AV_SAMPLE_FMT_S16, int16_t, AV_SAMPLE_FMT_S32,  *(const int32_t*)pi>>16)
CONV_FUNC(AV_SAMPLE_FMT_S32, int32_t, AV_SAMPLE_FMT_S32,  *(const int32_t*)pi)
CONV_FUNC(AV_SAMPLE_FMT_FLT, float  , AV_SAMPLE_FMT_S32,  *(const int32_t*)pi*(1.0 / (1U<<31)))
CONV_FUNC(AV_SAMPLE_FMT_DBL, double , AV_SAMPLE_FMT_S32,  *(const int32_t*)pi*(1.0 / (1U<<31)))
CONV_FUNC(AV_SAMPLE_FMT_U8 , uint8_t, AV_SAMPLE_FMT_FLT, av_clip_uint8(  lrintf(*(const float*)pi * (1<<7)) + 0x80))
CONV_FUNC(AV_SAMPLE_FMT_S16, int16_t, AV_SAMPLE_FMT_FLT, av_clip_int16(  lrintf(*(const float*)pi * (1<<15))))
CONV_FUNC(AV_SAMPLE_FMT_S32, int32_t, AV_SAMPLE_FMT_FLT, av_clipl_int32(llrintf(*(const float*)pi * (1U<<31))))
CONV_FUNC(AV_SAMPLE_FMT_FLT, float  , AV_SAMPLE_FMT_FLT, *(const float*)pi)
CONV_FUNC(AV_SAMPLE_FMT_DBL, double , AV_SAMPLE_FMT_FLT, *(const float*)pi)
CONV_FUNC(AV_SAMPLE_FMT_U8 , uint8_t, AV_SAMPLE_FMT_DBL, av_clip_uint8(  lrint(*(const double*)pi * (1<<7)) + 0x80))
CONV_FUNC(AV_SAMPLE_FMT_S16, int16_t, AV_SAMPLE_FMT_DBL, av_clip_int16(  lrint(*(const double*)pi * (1<<15))))
CONV_FUNC(AV_SAMPLE_FMT_S32, int32_t, AV_SAMPLE_FMT_DBL, av_clipl_int32(llrint(*(const double*)pi * (1U<<31))))
CONV_FUNC(AV_SAMPLE_FMT_FLT, float  , AV_SAMPLE_FMT_DBL, *(const double*)pi)
CONV_FUNC(AV_SAMPLE_FMT_DBL, double , AV_SAMPLE_FMT_DBL, *(const double*)pi)

#if _MSC_VER
conv_func_type *fmt_pair_to_conv_functions[AV_SAMPLE_FMT_NB*AV_SAMPLE_FMT_NB] = {0};
static int  g_bInited = 0;
#define FMT_PAIR_FUNC(out, in) fmt_pair_to_conv_functions[out + AV_SAMPLE_FMT_NB*in] = CONV_FUNC_NAME(out, in)
static void 
_init_fmt_pair_to_conv_func(void)
{
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_U8 , AV_SAMPLE_FMT_U8 );
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_U8 );
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_S32, AV_SAMPLE_FMT_U8 );
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_U8 );
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_DBL, AV_SAMPLE_FMT_U8 );
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_U8 , AV_SAMPLE_FMT_S16);
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S16);
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_S32, AV_SAMPLE_FMT_S16);
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_S16);
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_DBL, AV_SAMPLE_FMT_S16);
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_U8 , AV_SAMPLE_FMT_S32);
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S32);
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_S32, AV_SAMPLE_FMT_S32);
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_S32);
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_DBL, AV_SAMPLE_FMT_S32);
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_U8 , AV_SAMPLE_FMT_FLT);
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_FLT);
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_S32, AV_SAMPLE_FMT_FLT);
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_FLT);
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_DBL, AV_SAMPLE_FMT_FLT);
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_U8 , AV_SAMPLE_FMT_DBL);
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_DBL);
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_S32, AV_SAMPLE_FMT_DBL);
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_DBL);
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_DBL, AV_SAMPLE_FMT_DBL);
    g_bInited = 1;
}
#else  //#if CONFIG_SWRESAMPLE && _MSC_VER
#define FMT_PAIR_FUNC(out, in) [out + AV_SAMPLE_FMT_NB*in] = CONV_FUNC_NAME(out, in)
conv_func_type *fmt_pair_to_conv_functions[AV_SAMPLE_FMT_NB*AV_SAMPLE_FMT_NB] = {
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_U8 , AV_SAMPLE_FMT_U8 ),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_U8 ),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_S32, AV_SAMPLE_FMT_U8 ),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_U8 ),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_DBL, AV_SAMPLE_FMT_U8 ),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_U8 , AV_SAMPLE_FMT_S16),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S16),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_S32, AV_SAMPLE_FMT_S16),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_S16),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_DBL, AV_SAMPLE_FMT_S16),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_U8 , AV_SAMPLE_FMT_S32),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S32),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_S32, AV_SAMPLE_FMT_S32),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_S32),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_DBL, AV_SAMPLE_FMT_S32),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_U8 , AV_SAMPLE_FMT_FLT),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_FLT),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_S32, AV_SAMPLE_FMT_FLT),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_FLT),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_DBL, AV_SAMPLE_FMT_FLT),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_U8 , AV_SAMPLE_FMT_DBL),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_DBL),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_S32, AV_SAMPLE_FMT_DBL),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_DBL),
    FMT_PAIR_FUNC(AV_SAMPLE_FMT_DBL, AV_SAMPLE_FMT_DBL),
};
#endif

AudioConvert *swri_audio_convert_alloc(enum AVSampleFormat out_fmt,
                                       enum AVSampleFormat in_fmt,
                                       int channels, const int *ch_map,
                                       int flags)
{
    AudioConvert *ctx;
#if _MSC_VER
    conv_func_type *f = NULL;
    if( !g_bInited )
        _init_fmt_pair_to_conv_func();
    f = fmt_pair_to_conv_functions[out_fmt + AV_SAMPLE_FMT_NB*in_fmt];
#else
    conv_func_type *f = fmt_pair_to_conv_functions[out_fmt + AV_SAMPLE_FMT_NB*in_fmt];
#endif

    if (!f)
        return NULL;
    ctx = av_mallocz(sizeof(*ctx));
    if (!ctx)
        return NULL;
    ctx->channels = channels;
    ctx->conv_f   = f;
    ctx->ch_map   = ch_map;
    if (in_fmt == AV_SAMPLE_FMT_U8)
        memset(ctx->silence, 0x80, sizeof(ctx->silence));
    return ctx;
}

void swri_audio_convert_free(AudioConvert **ctx)
{
#if _MSC_VER
    g_bInited = 0;
#endif

    av_freep(ctx);
}

int swri_audio_convert(AudioConvert *ctx, AudioData *out, AudioData *in, int len)
{
    int ch;

    av_assert0(ctx->channels == out->ch_count);

    //FIXME optimize common cases

    for(ch=0; ch<ctx->channels; ch++){
        const int ich= ctx->ch_map ? ctx->ch_map[ch] : ch;
        const int is= ich < 0 ? 0 : (in->planar ? 1 : in->ch_count) * in->bps;
        const int os= (out->planar ? 1 :out->ch_count) *out->bps;
        const uint8_t *pi= ich < 0 ? ctx->silence : in->ch[ich];
        uint8_t       *po= out->ch[ch];
        uint8_t *end= po + os*len;
        if(!po)
            continue;
        ctx->conv_f(po, pi, is, os, end);
    }
    return 0;
}

﻿/*
 * ADX ADPCM codecs
 * Copyright (c) 2001,2003 BERO
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

#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "adx.h"
#include "put_bits.h"

/**
 * @file
 * SEGA CRI adx codecs.
 *
 * Reference documents:
 * http://ku-www.ss.titech.ac.jp/~yatsushi/adx.html
 * adx2wav & wav2adx http://www.geocities.co.jp/Playtown/2004/
 */

/* 18 bytes <-> 32 samples */

static void adx_encode(ADXContext *c, unsigned char *adx, const short *wav,
                       ADXChannelState *prev)
{
    PutBitContext pb;
    int scale;
    int i;
    int s0,s1,s2,d;
    int max=0;
    int min=0;
    int data[32];

    s1 = prev->s1;
    s2 = prev->s2;
    for(i=0;i<32;i++) {
        s0 = wav[i];
        d = ((s0 << COEFF_BITS) - c->coeff[0] * s1 - c->coeff[1] * s2) >> COEFF_BITS;
        data[i]=d;
        if (max<d) max=d;
        if (min>d) min=d;
        s2 = s1;
        s1 = s0;
    }
    prev->s1 = s1;
    prev->s2 = s2;

    /* -8..+7 */

    if (max==0 && min==0) {
        memset(adx,0,18);
        return;
    }

    if (max/7>-min/8) scale = max/7;
    else scale = -min/8;

    if (scale==0) scale=1;

    AV_WB16(adx, scale);

    init_put_bits(&pb, adx + 2, 16);
    for (i = 0; i < 32; i++)
        put_sbits(&pb, 4, av_clip(data[i]/scale, -8, 7));
    flush_put_bits(&pb);
}

static int adx_encode_header(AVCodecContext *avctx,unsigned char *buf,size_t bufsize)
{
#if 0
    struct {
        uint32_t offset; /* 0x80000000 + sample start - 4 */
        unsigned char unknown1[3]; /* 03 12 04 */
        unsigned char channel; /* 1 or 2 */
        uint32_t freq;
        uint32_t size;
        uint32_t unknown2; /* 01 f4 03 00 */
        uint32_t unknown3; /* 00 00 00 00 */
        uint32_t unknown4; /* 00 00 00 00 */

    /* if loop
        unknown3 00 15 00 01
        unknown4 00 00 00 01
        long loop_start_sample;
        long loop_start_byte;
        long loop_end_sample;
        long loop_end_byte;
        long
    */
    } adxhdr; /* big endian */
    /* offset-6 "(c)CRI" */
#endif
    ADXContext *c = avctx->priv_data;

    AV_WB32(buf+0x00,0x80000000|0x20);
    AV_WB32(buf+0x04,0x03120400|avctx->channels);
    AV_WB32(buf+0x08,avctx->sample_rate);
    AV_WB32(buf+0x0c,0); /* FIXME: set after */
    AV_WB16(buf + 0x10, c->cutoff);
    AV_WB32(buf + 0x12, 0x03000000);
    AV_WB32(buf + 0x16, 0x00000000);
    AV_WB32(buf + 0x1a, 0x00000000);
    memcpy (buf + 0x1e, "(c)CRI", 6);
    return 0x20+4;
}

static av_cold int adx_encode_init(AVCodecContext *avctx)
{
    ADXContext *c = avctx->priv_data;

    if (avctx->channels > 2)
        return -1; /* only stereo or mono =) */
    avctx->frame_size = 32;

    avctx->coded_frame= avcodec_alloc_frame();
    avctx->coded_frame->key_frame= 1;

//    avctx->bit_rate = avctx->sample_rate*avctx->channels*18*8/32;

    /* the cutoff can be adjusted, but this seems to work pretty well */
    c->cutoff = 500;
    ff_adx_calculate_coeffs(c->cutoff, avctx->sample_rate, COEFF_BITS, c->coeff);

    av_log(avctx, AV_LOG_DEBUG, "adx encode init\n");

    return 0;
}

static av_cold int adx_encode_close(AVCodecContext *avctx)
{
    av_freep(&avctx->coded_frame);

    return 0;
}

static int adx_encode_frame(AVCodecContext *avctx,
                uint8_t *frame, int buf_size, void *data)
{
    ADXContext *c = avctx->priv_data;
    const short *samples = data;
    unsigned char *dst = frame;
    int rest = avctx->frame_size;

/*
    input data size =
    ffmpeg.c: do_audio_out()
    frame_bytes = enc->frame_size * 2 * enc->channels;
*/

//    printf("sz=%d ",buf_size); fflush(stdout);
    if (!c->header_parsed) {
        int hdrsize = adx_encode_header(avctx,dst,buf_size);
        dst+=hdrsize;
        c->header_parsed = 1;
    }

    if (avctx->channels==1) {
        while(rest>=32) {
            adx_encode(c, dst, samples, c->prev);
            dst+=18;
            samples+=32;
            rest-=32;
        }
    } else {
        while(rest>=32*2) {
            short tmpbuf[32*2];
            int i;

            for(i=0;i<32;i++) {
                tmpbuf[i] = samples[i*2];
                tmpbuf[i+32] = samples[i*2+1];
            }

            adx_encode(c, dst,      tmpbuf,      c->prev);
            adx_encode(c, dst + 18, tmpbuf + 32, c->prev + 1);
            dst+=18*2;
            samples+=32*2;
            rest-=32*2;
        }
    }
    return dst-frame;
}
#if __OPENRTOS__ || _MSC_VER // WIN32
static enum AVSampleFormat sample_fmts[2] = {AV_SAMPLE_FMT_S16,AV_SAMPLE_FMT_NONE};
AVCodec ff_adpcm_adx_encoder = {
    "adpcm_adx",            // const char *name;
    AVMEDIA_TYPE_AUDIO,     // enum AVMediaType type;
    CODEC_ID_ADPCM_ADX,     // enum CodecID id;
    sizeof(ADXContext),     // int priv_data_size;
    adx_encode_init,        // int (*init)(AVCodecContext *);
    adx_encode_frame,       // int (*encode)(AVCodecContext *, uint8_t *buf, int buf_size, void *data);
    adx_encode_close,       // int (*close)(AVCodecContext *);
    NULL,                   // int (*decode)(AVCodecContext *, void *outdata, int *outdata_size, AVPacket *avpkt);
    0,                      // int capabilities;
    NULL,                   // struct AVCodec *next;
    NULL,                   // void (*flush)(AVCodecContext *);
    NULL,                   // const AVRational *supported_framerates;
    NULL,                   // const enum PixelFormat *pix_fmts;
    "SEGA CRI ADX ADPCM",   // const char *long_name;
    NULL,                   // const int *supported_samplerates;
    sample_fmts,            // const enum AVSampleFormat *sample_fmts;
    NULL,                   // const uint64_t *channel_layouts;
    0,                      // uint8_t max_lowres;
    NULL,                   // const AVClass *priv_class;
    NULL,                   // const AVProfile *profiles;
    NULL,                   // int (*init_thread_copy)(AVCodecContext *);
    NULL,                   // int (*update_thread_context)(AVCodecContext *dst, const AVCodecContext *src);
    NULL,                   // const AVCodecDefault *defaults;
    NULL,                   // void (*init_static_data)(struct AVCodec *codec);
};
#else
AVCodec ff_adpcm_adx_encoder = {
    .name           = "adpcm_adx",
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = CODEC_ID_ADPCM_ADX,
    .priv_data_size = sizeof(ADXContext),
    .init           = adx_encode_init,
    .encode         = adx_encode_frame,
    .close          = adx_encode_close,
    .sample_fmts = (const enum AVSampleFormat[]){AV_SAMPLE_FMT_S16,AV_SAMPLE_FMT_NONE},
    .long_name = NULL_IF_CONFIG_SMALL("SEGA CRI ADX ADPCM"),
};
#endif

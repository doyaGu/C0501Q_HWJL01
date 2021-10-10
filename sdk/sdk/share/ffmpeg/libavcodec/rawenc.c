﻿/*
 * Raw Video Encoder
 * Copyright (c) 2001 Fabrice Bellard
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
 * Raw Video Encoder
 */

#include "avcodec.h"
#include "raw.h"
#include "libavutil/pixdesc.h"
#include "libavutil/intreadwrite.h"

static av_cold int raw_init_encoder(AVCodecContext *avctx)
{
    avctx->coded_frame = (AVFrame *)avctx->priv_data;
    avctx->coded_frame->pict_type = AV_PICTURE_TYPE_I;
    avctx->coded_frame->key_frame = 1;
    avctx->bits_per_coded_sample = av_get_bits_per_pixel(&av_pix_fmt_descriptors[avctx->pix_fmt]);
    if(!avctx->codec_tag)
        avctx->codec_tag = avcodec_pix_fmt_to_codec_tag(avctx->pix_fmt);
    return 0;
}

static int raw_encode(AVCodecContext *avctx,
                            unsigned char *frame, int buf_size, void *data)
{
    int ret = avpicture_layout((AVPicture *)data, avctx->pix_fmt, avctx->width,
                                               avctx->height, frame, buf_size);

    if(avctx->codec_tag == AV_RL32("yuv2") && ret > 0 &&
       avctx->pix_fmt   == PIX_FMT_YUYV422) {
        int x;
        for(x = 1; x < avctx->height*avctx->width*2; x += 2)
            frame[x] ^= 0x80;
    }
    return ret;
}

#if __OPENRTOS__ || _MSC_VER 
AVCodec ff_rawvideo_encoder = {
    "rawvideo",             // const char *name;
    AVMEDIA_TYPE_VIDEO,     // enum AVMediaType type;
    CODEC_ID_RAWVIDEO,      // enum CodecID id;
    sizeof(AVFrame),        // int priv_data_size;
    raw_init_encoder,       // int (*init)(AVCodecContext *);
    raw_encode,             // int (*encode)(AVCodecContext *, uint8_t *buf, int buf_size, void *data);
    NULL,                   // int (*close)(AVCodecContext *);
    NULL,                   // int (*decode)(AVCodecContext *, void *outdata, int *outdata_size, AVPacket *avpkt);
    0,                      // int capabilities;
    NULL,                   // struct AVCodec *next;
    NULL,                   // void (*flush)(AVCodecContext *);
    NULL,                   // const AVRational *supported_framerates;
    NULL,                   // const enum PixelFormat *pix_fmts;
    "raw video",            // const char *long_name;
    NULL,                   // const int *supported_samplerates;
    NULL,                   // const enum AVSampleFormat *sample_fmts;
    NULL,                   // const uint64_t *channel_layouts;
    0,                      // uint8_t max_lowres;
    NULL,                   // const AVClass *priv_class;
    NULL,                   // const AVProfile *profiles;
    NULL,                   // int (*init_thread_copy)(AVCodecContext *);
    NULL,                   // int (*update_thread_context)(AVCodecContext *dst, const AVCodecContext *src);
    NULL,                   // const AVCodecDefault *defaults;
    NULL,                   // void (*init_static_data)(struct AVCodec *codec);
};
#else  // #if __OPENRTOS__ || _MSC_VER 
AVCodec ff_rawvideo_encoder = {
    .name           = "rawvideo",
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = CODEC_ID_RAWVIDEO,
    .priv_data_size = sizeof(AVFrame),
    .init           = raw_init_encoder,
    .encode         = raw_encode,
    .long_name = NULL_IF_CONFIG_SMALL("raw video"),
};
#endif

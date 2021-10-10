/*
 * MPEG Audio decoder
 * Copyright (c) 2001, 2002 Fabrice Bellard
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

#include "avcodec.h"
#include "libavutil/internal.h"
#include "config.h"

static av_cold int decode_init(AVCodecContext *avctx)
{
    return 0;
}

static int decode_frame(AVCodecContext *avctx, void *data,
                            int *got_frame_ptr, AVPacket *avpkt)
{
    return 0;
}

#if CONFIG_MP2_DECODER
AVCodec ff_mp2_decoder = {
    "mp2",
    AVMEDIA_TYPE_AUDIO,
    CODEC_ID_MP2,
    0,
    decode_init,
    NULL,
    NULL,
    decode_frame,
    0,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL_IF_CONFIG_SMALL("MP2 (MPEG audio layer 2)"),
};
#endif
#if CONFIG_MP3_DECODER
AVCodec ff_mp3_decoder =
{
    "mp3",
    AVMEDIA_TYPE_AUDIO,
    CODEC_ID_MP3,
    0,
    decode_init,
    NULL,
    NULL,
    decode_frame,
    0,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL_IF_CONFIG_SMALL("MP3 (MPEG audio layer 3)"),
};
#endif

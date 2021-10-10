/*
 * AC-3 Audio Decoder
 * This code was developed as part of Google Summer of Code 2006.
 * E-AC-3 support was added as part of Google Summer of Code 2007.
 *
 * Copyright (c) 2006 Kartikey Mahendra BHATT (bhattkm at gmail dot com)
 * Copyright (c) 2007-2008 Bartlomiej Wolowiec <bartek.wolowiec@gmail.com>
 * Copyright (c) 2007 Justin Ruggles <justin.ruggles@gmail.com>
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

AVCodec ff_ac3_decoder = {
    "ac3",
    AVMEDIA_TYPE_AUDIO,
    CODEC_ID_AC3,
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
    NULL_IF_CONFIG_SMALL("ATSC A/52A (AC-3)"),
};

#if CONFIG_EAC3_DECODER
AVCodec ff_eac3_decoder = {
    "eac3",
    AVMEDIA_TYPE_AUDIO,
    CODEC_ID_EAC3,
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
    NULL_IF_CONFIG_SMALL("ATSC A/52B (AC-3, E-AC-3)"),
};
#endif

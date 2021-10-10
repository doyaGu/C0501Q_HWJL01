/*
 * WMA compatible decoder
 * Copyright (c) 2002 The FFmpeg Project
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
 * WMA compatible decoder.
 * This decoder handles Microsoft Windows Media Audio data, versions 1 & 2.
 * WMA v1 is identified by audio format 0x160 in Microsoft media files
 * (ASF/AVI/WAV). WMA v2 is identified by audio format 0x161.
 *
 * To use this decoder, a calling application must supply the extra data
 * bytes provided with the WMA data. These are the extra, codec-specific
 * bytes at the end of a WAVEFORMATEX data structure. Transmit these bytes
 * to the decoder using the extradata[_size] fields in AVCodecContext. There
 * should be 4 extra bytes for v1 data and 6 extra bytes for v2 data.
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

AVCodec ff_wmav1_decoder = {
    "wmav1",
    AVMEDIA_TYPE_AUDIO,
    CODEC_ID_WMAV1,
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
    NULL_IF_CONFIG_SMALL("WMA(Windows Media Audio 1)"),
};

AVCodec ff_wmav2_decoder = {
    "wmav2",
    AVMEDIA_TYPE_AUDIO,
    CODEC_ID_WMAV2,
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
    NULL_IF_CONFIG_SMALL("WMA(Windows Media Audio 2)"),
};

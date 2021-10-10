﻿/*
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard
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
#include "avformat.h"
#include "avio_internal.h"
#include "libavutil/opt.h"

/**
 * @file
 * Options definition for AVFormatContext.
 */

static const char* format_to_name(void* ptr)
{
    AVFormatContext* fc = (AVFormatContext*) ptr;
    if(fc->iformat) return fc->iformat->name;
    else if(fc->oformat) return fc->oformat->name;
    else return "NULL";
}

static void *format_child_next(void *obj, void *prev)
{
    AVFormatContext *s = obj;
    if (!prev && s->priv_data &&
        ((s->iformat && s->iformat->priv_class) ||
          s->oformat && s->oformat->priv_class))
        return s->priv_data;
#if !FF_API_OLD_AVIO
    if (s->pb && s->pb->av_class && prev != s->pb)
        return s->pb;
#endif
    return NULL;
}

static const AVClass *format_child_class_next(const AVClass *prev)
{
    AVInputFormat  *ifmt = NULL;
    AVOutputFormat *ofmt = NULL;

    if (!prev)
#if !FF_API_OLD_AVIO
        return &ffio_url_class;
#else
    prev = (void *)&ifmt; // Dummy pointer;
#endif

    while ((ifmt = av_iformat_next(ifmt)))
        if (ifmt->priv_class == prev)
            break;

    if (!ifmt)
        while ((ofmt = av_oformat_next(ofmt)))
            if (ofmt->priv_class == prev)
                break;
    if (!ofmt)
        while (ifmt = av_iformat_next(ifmt))
            if (ifmt->priv_class)
                return ifmt->priv_class;

    while (ofmt = av_oformat_next(ofmt))
        if (ofmt->priv_class)
            return ofmt->priv_class;

    return NULL;
}

#define OFFSET(x) offsetof(AVFormatContext,x)
#define DEFAULT 0 //should be NAN but it does not work as it is not a constant in glibc as required by ANSI/ISO C
//these names are too long to be readable
#define E AV_OPT_FLAG_ENCODING_PARAM
#define D AV_OPT_FLAG_DECODING_PARAM

#if defined(WIN32)
static const AVOption options[]={
{"probesize", "set probing size", OFFSET(probesize), AV_OPT_TYPE_INT, { 5000000 }, 32, INT_MAX, D},
#if FF_API_MUXRATE
{"muxrate", "set mux rate", OFFSET(mux_rate), AV_OPT_TYPE_INT, { DEFAULT }, 0, INT_MAX, E},
#endif
{"packetsize", "set packet size", OFFSET(packet_size), AV_OPT_TYPE_INT, { DEFAULT }, 0, INT_MAX, E},
{"fflags", NULL, OFFSET(flags), AV_OPT_TYPE_FLAGS, { DEFAULT }, INT_MIN, INT_MAX, D|E, "fflags"},
{"ignidx", "ignore index", 0, AV_OPT_TYPE_CONST, { AVFMT_FLAG_IGNIDX }, INT_MIN, INT_MAX, D, "fflags"},
{"genpts", "generate pts", 0, AV_OPT_TYPE_CONST, { AVFMT_FLAG_GENPTS }, INT_MIN, INT_MAX, D, "fflags"},
{"nofillin", "do not fill in missing values that can be exactly calculated", 0, AV_OPT_TYPE_CONST, { AVFMT_FLAG_NOFILLIN }, INT_MIN, INT_MAX, D, "fflags"},
{"noparse", "disable AVParsers, this needs nofillin too", 0, AV_OPT_TYPE_CONST, { AVFMT_FLAG_NOPARSE }, INT_MIN, INT_MAX, D, "fflags"},
{"igndts", "ignore dts", 0, AV_OPT_TYPE_CONST, { AVFMT_FLAG_IGNDTS }, INT_MIN, INT_MAX, D, "fflags"},
#if FF_API_FLAG_RTP_HINT
{"rtphint", "add rtp hinting (deprecated, use the -movflags rtphint option instead)", 0, AV_OPT_TYPE_CONST, { AVFMT_FLAG_RTP_HINT }, INT_MIN, INT_MAX, E, "fflags"},
#endif
{"discardcorrupt", "discard corrupted frames", 0, AV_OPT_TYPE_CONST, { AVFMT_FLAG_DISCARD_CORRUPT }, INT_MIN, INT_MAX, D, "fflags"},
{"sortdts", "try to interleave outputted packets by dts", 0, AV_OPT_TYPE_CONST, { AVFMT_FLAG_SORT_DTS }, INT_MIN, INT_MAX, D, "fflags"},
{"keepside", "dont merge side data", 0, AV_OPT_TYPE_CONST, { AVFMT_FLAG_KEEP_SIDE_DATA }, INT_MIN, INT_MAX, D, "fflags"},
{"latm", "enable RTP MP4A-LATM payload", 0, AV_OPT_TYPE_CONST, { AVFMT_FLAG_MP4A_LATM }, INT_MIN, INT_MAX, E, "fflags"},
{"analyzeduration", "how many microseconds are analyzed to estimate duration", OFFSET(max_analyze_duration), AV_OPT_TYPE_INT, { 5*AV_TIME_BASE }, 0, INT_MAX, D},
{"cryptokey", "decryption key", OFFSET(key), AV_OPT_TYPE_BINARY, { 0}, 0, 0, D},
{"indexmem", "max memory used for timestamp index (per stream)", OFFSET(max_index_size), AV_OPT_TYPE_INT, { 1<<20 }, 0, INT_MAX, D},
{"rtbufsize", "max memory used for buffering real-time frames", OFFSET(max_picture_buffer), AV_OPT_TYPE_INT, { 3041280 }, 0, INT_MAX, D}, /* defaults to 1s of 15fps 352x288 YUYV422 video */
{"fdebug", "print specific debug info", OFFSET(debug), AV_OPT_TYPE_FLAGS, { DEFAULT }, 0, INT_MAX, E|D, "fdebug"},
{"ts", NULL, 0, AV_OPT_TYPE_CONST, { FF_FDEBUG_TS }, INT_MIN, INT_MAX, E|D, "fdebug"},
{"max_delay", "maximum muxing or demuxing delay in microseconds", OFFSET(max_delay), AV_OPT_TYPE_INT, { DEFAULT }, 0, INT_MAX, E|D},
{"fer", "set error detection aggressivity", OFFSET(error_recognition), AV_OPT_TYPE_INT, { FF_ER_CAREFUL }, INT_MIN, INT_MAX, D, "fer"},
{"careful", NULL, 0, AV_OPT_TYPE_CONST, { FF_ER_CAREFUL }, INT_MIN, INT_MAX, D, "fer"},
{"explode", "abort decoding on error recognition", 0, AV_OPT_TYPE_CONST, { FF_ER_EXPLODE }, INT_MIN, INT_MAX, D, "fer"},
{"fpsprobesize", "number of frames used to probe fps", OFFSET(fps_probe_size), AV_OPT_TYPE_INT, { -1}, -1, INT_MAX-1, D},
{"audio_preload", "microseconds by which audio packets should be interleaved earlier", OFFSET(audio_preload), AV_OPT_TYPE_INT, { 0}, 0, INT_MAX-1, E},
{"chunk_duration", "microseconds for each chunk", OFFSET(max_chunk_duration), AV_OPT_TYPE_INT, { 0}, 0, INT_MAX-1, E},
{"chunk_size", "size in bytes for each chunk", OFFSET(max_chunk_size), AV_OPT_TYPE_INT, { 0}, 0, INT_MAX-1, E},
{NULL},
};
#else
static const AVOption options[]={
{"probesize", "set probing size", OFFSET(probesize), AV_OPT_TYPE_INT, {.dbl = 5000000 }, 32, INT_MAX, D},
#if FF_API_MUXRATE
{"muxrate", "set mux rate", OFFSET(mux_rate), AV_OPT_TYPE_INT, {.dbl = DEFAULT }, 0, INT_MAX, E},
#endif
{"packetsize", "set packet size", OFFSET(packet_size), AV_OPT_TYPE_INT, {.dbl = DEFAULT }, 0, INT_MAX, E},
{"fflags", NULL, OFFSET(flags), AV_OPT_TYPE_FLAGS, {.dbl = DEFAULT }, INT_MIN, INT_MAX, D|E, "fflags"},
{"ignidx", "ignore index", 0, AV_OPT_TYPE_CONST, {.dbl = AVFMT_FLAG_IGNIDX }, INT_MIN, INT_MAX, D, "fflags"},
{"genpts", "generate pts", 0, AV_OPT_TYPE_CONST, {.dbl = AVFMT_FLAG_GENPTS }, INT_MIN, INT_MAX, D, "fflags"},
{"nofillin", "do not fill in missing values that can be exactly calculated", 0, AV_OPT_TYPE_CONST, {.dbl = AVFMT_FLAG_NOFILLIN }, INT_MIN, INT_MAX, D, "fflags"},
{"noparse", "disable AVParsers, this needs nofillin too", 0, AV_OPT_TYPE_CONST, {.dbl = AVFMT_FLAG_NOPARSE }, INT_MIN, INT_MAX, D, "fflags"},
{"igndts", "ignore dts", 0, AV_OPT_TYPE_CONST, {.dbl = AVFMT_FLAG_IGNDTS }, INT_MIN, INT_MAX, D, "fflags"},
#if FF_API_FLAG_RTP_HINT
{"rtphint", "add rtp hinting (deprecated, use the -movflags rtphint option instead)", 0, AV_OPT_TYPE_CONST, {.dbl = AVFMT_FLAG_RTP_HINT }, INT_MIN, INT_MAX, E, "fflags"},
#endif
{"discardcorrupt", "discard corrupted frames", 0, AV_OPT_TYPE_CONST, {.dbl = AVFMT_FLAG_DISCARD_CORRUPT }, INT_MIN, INT_MAX, D, "fflags"},
{"sortdts", "try to interleave outputted packets by dts", 0, AV_OPT_TYPE_CONST, {.dbl = AVFMT_FLAG_SORT_DTS }, INT_MIN, INT_MAX, D, "fflags"},
{"keepside", "dont merge side data", 0, AV_OPT_TYPE_CONST, {.dbl = AVFMT_FLAG_KEEP_SIDE_DATA }, INT_MIN, INT_MAX, D, "fflags"},
{"latm", "enable RTP MP4A-LATM payload", 0, AV_OPT_TYPE_CONST, {.dbl = AVFMT_FLAG_MP4A_LATM }, INT_MIN, INT_MAX, E, "fflags"},
{"analyzeduration", "how many microseconds are analyzed to estimate duration", OFFSET(max_analyze_duration), AV_OPT_TYPE_INT, {.dbl = 5*AV_TIME_BASE }, 0, INT_MAX, D},
{"cryptokey", "decryption key", OFFSET(key), AV_OPT_TYPE_BINARY, {.dbl = 0}, 0, 0, D},
{"indexmem", "max memory used for timestamp index (per stream)", OFFSET(max_index_size), AV_OPT_TYPE_INT, {.dbl = 1<<20 }, 0, INT_MAX, D},
{"rtbufsize", "max memory used for buffering real-time frames", OFFSET(max_picture_buffer), AV_OPT_TYPE_INT, {.dbl = 3041280 }, 0, INT_MAX, D}, /* defaults to 1s of 15fps 352x288 YUYV422 video */
{"fdebug", "print specific debug info", OFFSET(debug), AV_OPT_TYPE_FLAGS, {.dbl = DEFAULT }, 0, INT_MAX, E|D, "fdebug"},
{"ts", NULL, 0, AV_OPT_TYPE_CONST, {.dbl = FF_FDEBUG_TS }, INT_MIN, INT_MAX, E|D, "fdebug"},
{"max_delay", "maximum muxing or demuxing delay in microseconds", OFFSET(max_delay), AV_OPT_TYPE_INT, {.dbl = DEFAULT }, 0, INT_MAX, E|D},
{"fer", "set error detection aggressivity", OFFSET(error_recognition), AV_OPT_TYPE_INT, {.dbl = FF_ER_CAREFUL }, INT_MIN, INT_MAX, D, "fer"},
{"careful", NULL, 0, AV_OPT_TYPE_CONST, {.dbl = FF_ER_CAREFUL }, INT_MIN, INT_MAX, D, "fer"},
{"explode", "abort decoding on error recognition", 0, AV_OPT_TYPE_CONST, {.dbl = FF_ER_EXPLODE }, INT_MIN, INT_MAX, D, "fer"},
{"fpsprobesize", "number of frames used to probe fps", OFFSET(fps_probe_size), AV_OPT_TYPE_INT, {.dbl = -1}, -1, INT_MAX-1, D},
{"audio_preload", "microseconds by which audio packets should be interleaved earlier", OFFSET(audio_preload), AV_OPT_TYPE_INT, {.dbl = 0}, 0, INT_MAX-1, E},
{"chunk_duration", "microseconds for each chunk", OFFSET(max_chunk_duration), AV_OPT_TYPE_INT, {.dbl = 0}, 0, INT_MAX-1, E},
{"chunk_size", "size in bytes for each chunk", OFFSET(max_chunk_size), AV_OPT_TYPE_INT, {.dbl = 0}, 0, INT_MAX-1, E},
{NULL},
};
#endif

#undef E
#undef D
#undef DEFAULT

#if defined(WIN32)
static const AVClass av_format_context_class = {
    "AVFormatContext",
    format_to_name,
    options,
    LIBAVUTIL_VERSION_INT,
    0,
    0,
    format_child_next,
    format_child_class_next,
};
#else
static const AVClass av_format_context_class = {
    .class_name     = "AVFormatContext",
    .item_name      = format_to_name,
    .option         = options,
    .version        = LIBAVUTIL_VERSION_INT,
    .child_next     = format_child_next,
    .child_class_next = format_child_class_next,
};
#endif

static void avformat_get_context_defaults(AVFormatContext *s)
{
    memset(s, 0, sizeof(AVFormatContext));

    s->av_class = &av_format_context_class;

    av_opt_set_defaults(s);
}

AVFormatContext *avformat_alloc_context(void)
{
    AVFormatContext *ic;
    ic = av_malloc(sizeof(AVFormatContext));
    if (!ic) return ic;
    avformat_get_context_defaults(ic);
    return ic;
}

const AVClass *avformat_get_class(void)
{
    return &av_format_context_class;
}

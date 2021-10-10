﻿/*
 * Misc image conversion routines
 * Copyright (c) 2001, 2002, 2003 Fabrice Bellard
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
 * misc image conversion routines
 */

/* TODO:
 * - write 'ffimg' program to test all the image related stuff
 * - move all api to slice based system
 * - integrate deinterlacing, postprocessing and scaling in the conversion process
 */

#include "avcodec.h"
#include "dsputil.h"
#include "internal.h"
#include "imgconvert.h"
#include "libavutil/colorspace.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
#include "libavutil/internal.h"

#if HAVE_MMX && HAVE_YASM
#include "x86/dsputil_mmx.h"
#endif

#define FF_COLOR_RGB      0 /**< RGB color space */
#define FF_COLOR_GRAY     1 /**< gray color space */
#define FF_COLOR_YUV      2 /**< YUV color space. 16 <= Y <= 235, 16 <= U, V <= 240 */
#define FF_COLOR_YUV_JPEG 3 /**< YUV color space. 0 <= Y <= 255, 0 <= U, V <= 255 */

#if HAVE_MMX && HAVE_YASM
#define deinterlace_line_inplace ff_deinterlace_line_inplace_mmx
#define deinterlace_line         ff_deinterlace_line_mmx
#else
#define deinterlace_line_inplace deinterlace_line_inplace_c
#define deinterlace_line         deinterlace_line_c
#endif

typedef struct PixFmtInfo {
    uint8_t color_type;      /**< color type (see FF_COLOR_xxx constants) */
    uint8_t is_alpha : 1;    /**< true if alpha can be specified */
    uint8_t padded_size;     /**< padded size in bits if different from the non-padded size */
} PixFmtInfo;

/* this table gives more information about formats */
#if defined(WIN32)
static const PixFmtInfo pix_fmt_info[PIX_FMT_NB] = {
    /* YUV formats */
    //[PIX_FMT_YUV420P] = // 0
    {
        FF_COLOR_YUV,
    },
    //[PIX_FMT_YUYV422] = // 1
    {
        FF_COLOR_YUV,
    },
    //[PIX_FMT_RGB24] = // 2
    {
        FF_COLOR_RGB,
    },
    //[PIX_FMT_BGR24] = // 3
    {
        FF_COLOR_RGB,
    },
    //[PIX_FMT_YUV422P] = // 4
    {
        FF_COLOR_YUV,
    },
    //[PIX_FMT_YUV444P] = // 5
    {
        FF_COLOR_YUV,
    },
    //[PIX_FMT_YUV410P] = // 6
    {
        FF_COLOR_YUV,
    },
    //[PIX_FMT_YUV411P] = // 7
    {
        FF_COLOR_YUV,
    },
    //[PIX_FMT_GRAY8] = // 8
    {
        FF_COLOR_GRAY,
    },
    //[PIX_FMT_MONOWHITE] = // 9
    {
        FF_COLOR_GRAY,
    },
    //[PIX_FMT_MONOBLACK] = // 10
    {
        FF_COLOR_GRAY,
    },
    //[PIX_FMT_PAL8] = // 11
    {
        FF_COLOR_RGB,
        1,
    },
    //[PIX_FMT_YUVJ420P] = // 12
    {
        FF_COLOR_YUV_JPEG,
    },
    //[PIX_FMT_YUVJ422P] = // 13
    {
        FF_COLOR_YUV_JPEG,
    },
    //[PIX_FMT_YUVJ444P] = // 14
    {
        FF_COLOR_YUV_JPEG,
    },
    { 0, 0, 0 }, // 15
    { 0, 0, 0 }, // 16
    //[PIX_FMT_UYVY422] = // 17
    {
        FF_COLOR_YUV,
    },
    //[PIX_FMT_UYYVYY411] = // 18
    {
        FF_COLOR_YUV,
    },
    //[PIX_FMT_BGR8] = // 19
    {
        FF_COLOR_RGB,
    },
    //[PIX_FMT_BGR4] = // 20
    {
        FF_COLOR_RGB,
    },
    //[PIX_FMT_BGR4_BYTE] = // 21
    {
        FF_COLOR_RGB,
        0,
        8,
    },
    //[PIX_FMT_RGB8] = // 22
    {
        FF_COLOR_RGB,
    },
    //[PIX_FMT_RGB4] = // 23
    {
        FF_COLOR_RGB,
    },
    //[PIX_FMT_RGB4_BYTE] = // 24
    {
        FF_COLOR_RGB,
        0,
        8,
    },
    //[PIX_FMT_NV12] = // 25
    {
        FF_COLOR_YUV,
    },
    //[PIX_FMT_NV21] = // 26
    {
        FF_COLOR_YUV,
    },
    //[PIX_FMT_ARGB] = // 27
    {
        FF_COLOR_RGB,
        1,
    },
    //[PIX_FMT_RGBA] = // 28
    {
        FF_COLOR_RGB,
        1,
    },
    //[PIX_FMT_ABGR] = // 29
    {
        FF_COLOR_RGB,
        1,
    },
    //[PIX_FMT_BGRA] = // 30
    {
        FF_COLOR_RGB,
        1,
    },
    //[PIX_FMT_GRAY16BE] = // 31
    {
        FF_COLOR_GRAY,
    },
    //[PIX_FMT_GRAY16LE] = // 32
    {
        FF_COLOR_GRAY,
    },
    //[PIX_FMT_YUV440P] = // 33
    {
        FF_COLOR_YUV,
    },
    //[PIX_FMT_YUVJ440P] = // 34
    {
        FF_COLOR_YUV_JPEG,
    },
    //[PIX_FMT_YUVA420P] = // 35
    {
        FF_COLOR_YUV,
    },
    { 0, 0, 0 }, // 36
    { 0, 0, 0 }, // 37
    { 0, 0, 0 }, // 38
    { 0, 0, 0 }, // 39
    { 0, 0, 0 }, // 40
    //[PIX_FMT_RGB48BE] = // 41
    {
        FF_COLOR_RGB,
    },
    //[PIX_FMT_RGB48LE] = // 42
    {
        FF_COLOR_RGB,
    },
    //[PIX_FMT_RGB565BE] = // 43
    {
        FF_COLOR_RGB,
    },
    //[PIX_FMT_RGB565LE] = // 44
    {
        FF_COLOR_RGB,
    },
    //[PIX_FMT_RGB555BE] = // 45
    {
        FF_COLOR_RGB,
        0,
        16,
    },
    //[PIX_FMT_RGB555LE] = // 46
    {
        FF_COLOR_RGB,
        0,
        16,
    },
    //[PIX_FMT_BGR565BE] = // 47
    {
        FF_COLOR_RGB,
        0,
        16,
    },
    //[PIX_FMT_BGR565LE] = // 48
    {
        FF_COLOR_RGB,
        0,
        16,
    },
    //[PIX_FMT_BGR555BE] = // 49
    {
        FF_COLOR_RGB,
        0,
        16,
    },
    //[PIX_FMT_BGR555LE] = // 50
    {
        FF_COLOR_RGB,
        0,
        16,
    },
    { 0, 0, 0 }, // 51
    { 0, 0, 0 }, // 52
    { 0, 0, 0 }, // 53
    //[PIX_FMT_YUV420P16LE] = // 54
    {
        FF_COLOR_YUV,
    },
    //[PIX_FMT_YUV420P16BE] = // 55
    {
        FF_COLOR_YUV,
    },
    //[PIX_FMT_YUV422P16LE] = // 56
    {
        FF_COLOR_YUV,
    },
    //[PIX_FMT_YUV422P16BE] = // 57
    {
        FF_COLOR_YUV,
    },
    //[PIX_FMT_YUV444P16LE] = // 58
    {
        FF_COLOR_YUV,
    },
    //[PIX_FMT_YUV444P16BE] = // 59
    {
        FF_COLOR_YUV,
    },
    { 0, 0, 0 }, // 60
    { 0, 0, 0 }, // 61
    //[PIX_FMT_RGB444LE] = // 62
    {
        FF_COLOR_RGB,
        0,
        16,
    },
    //[PIX_FMT_RGB444BE] = // 63
    {
        FF_COLOR_RGB,
        0,
        16,
    },
    //[PIX_FMT_BGR444LE] = // 64
    {
        FF_COLOR_RGB,
        0,
        16,
    },
    //[PIX_FMT_BGR444BE] = // 65
    {
        FF_COLOR_RGB,
        0,
        16,
    },
    { 0, 0, 0 }, // 66
    //[PIX_FMT_BGR48BE] = // 67
    {
        FF_COLOR_RGB,
    },
    //[PIX_FMT_BGR48LE] = // 68
    {
        FF_COLOR_RGB,
    },
    { 0, 0, 0 }, // 69
    { 0, 0, 0 }, // 70
    { 0, 0, 0 }, // 71
    { 0, 0, 0 }, // 72
    { 0, 0, 0 }, // 73
    { 0, 0, 0 }, // 74
    { 0, 0, 0 }, // 75
    { 0, 0, 0 }, // 76
    { 0, 0, 0 }, // 77
    { 0, 0, 0 }, // 78
    { 0, 0, 0 }, // 79
    { 0, 0, 0 }, // 80
    { 0, 0, 0 }, // 81
#ifdef AV_PIX_FMT_ABI_GIT_MASTER
    //[PIX_FMT_RGBA64BE] = // 82
    {
        FF_COLOR_RGB,
        1,
    },
    //[PIX_FMT_RGBA64LE] = // 83
    {
        FF_COLOR_RGB,
        1,
    },
    //[PIX_FMT_BGRA64BE] = // 84
    {
        FF_COLOR_RGB,
        1,
    },
    //[PIX_FMT_BGRA64LE] = // 85
    {
        FF_COLOR_RGB,
        1,
    },
#else
    { 0, 0, 0 }, // 82
    { 0, 0, 0 }, // 83
    { 0, 0, 0 }, // 84
    { 0, 0, 0 }, // 85
    { 0, 0, 0 }, // 86
    { 0, 0, 0 }, // 87
    { 0, 0, 0 }, // 88
    { 0, 0, 0 }, // 89
    { 0, 0, 0 }, // 90
    { 0, 0, 0 }, // 91
    { 0, 0, 0 }, // 92
    { 0, 0, 0 }, // 93
    { 0, 0, 0 }, // 94
    { 0, 0, 0 }, // 95
    { 0, 0, 0 }, // 96
    { 0, 0, 0 }, // 97
    { 0, 0, 0 }, // 98
    { 0, 0, 0 }, // 99
    { 0, 0, 0 }, // 100
    { 0, 0, 0 }, // 101
    { 0, 0, 0 }, // 102
    { 0, 0, 0 }, // 103
    { 0, 0, 0 }, // 104
    { 0, 0, 0 }, // 105
    { 0, 0, 0 }, // 106
    { 0, 0, 0 }, // 107
    { 0, 0, 0 }, // 108
    { 0, 0, 0 }, // 109
    { 0, 0, 0 }, // 110
    { 0, 0, 0 }, // 111
    { 0, 0, 0 }, // 112
    { 0, 0, 0 }, // 113
    { 0, 0, 0 }, // 114
    { 0, 0, 0 }, // 115
    { 0, 0, 0 }, // 116
    { 0, 0, 0 }, // 117
    { 0, 0, 0 }, // 118
    { 0, 0, 0 }, // 119
    { 0, 0, 0 }, // 120
    { 0, 0, 0 }, // 121
    { 0, 0, 0 }, // 122
    { 0, 0, 0 }, // 123
    { 0, 0, 0 }, // 124
    { 0, 0, 0 }, // 125
    { 0, 0, 0 }, // 126
    { 0, 0, 0 }, // 127
    { 0, 0, 0 }, // 128
    { 0, 0, 0 }, // 129
    { 0, 0, 0 }, // 130
    { 0, 0, 0 }, // 131
    { 0, 0, 0 }, // 132
    { 0, 0, 0 }, // 133
    { 0, 0, 0 }, // 134
    { 0, 0, 0 }, // 135
    { 0, 0, 0 }, // 136
    { 0, 0, 0 }, // 137
    { 0, 0, 0 }, // 138
    { 0, 0, 0 }, // 139
    { 0, 0, 0 }, // 140
    { 0, 0, 0 }, // 141
    { 0, 0, 0 }, // 142
    { 0, 0, 0 }, // 143
    { 0, 0, 0 }, // 144
    { 0, 0, 0 }, // 145
    { 0, 0, 0 }, // 146
    { 0, 0, 0 }, // 147
    { 0, 0, 0 }, // 148
    { 0, 0, 0 }, // 149
    { 0, 0, 0 }, // 150
    { 0, 0, 0 }, // 151
    { 0, 0, 0 }, // 152
    { 0, 0, 0 }, // 153
    { 0, 0, 0 }, // 154
    { 0, 0, 0 }, // 155
    { 0, 0, 0 }, // 156
    { 0, 0, 0 }, // 157
    { 0, 0, 0 }, // 158
    { 0, 0, 0 }, // 159
    { 0, 0, 0 }, // 160
    { 0, 0, 0 }, // 161
    { 0, 0, 0 }, // 162
    { 0, 0, 0 }, // 163
    { 0, 0, 0 }, // 164
    { 0, 0, 0 }, // 165
    { 0, 0, 0 }, // 166
    { 0, 0, 0 }, // 167
    { 0, 0, 0 }, // 168
    { 0, 0, 0 }, // 169
    { 0, 0, 0 }, // 170
    { 0, 0, 0 }, // 171
    { 0, 0, 0 }, // 172
    { 0, 0, 0 }, // 173
    { 0, 0, 0 }, // 174
    { 0, 0, 0 }, // 175
    { 0, 0, 0 }, // 176
    { 0, 0, 0 }, // 177
    { 0, 0, 0 }, // 178
    { 0, 0, 0 }, // 179
    { 0, 0, 0 }, // 180
    { 0, 0, 0 }, // 181
    { 0, 0, 0 }, // 182
    { 0, 0, 0 }, // 183
    { 0, 0, 0 }, // 184
    { 0, 0, 0 }, // 185
    { 0, 0, 0 }, // 186
    { 0, 0, 0 }, // 187
    { 0, 0, 0 }, // 188
    { 0, 0, 0 }, // 189
    { 0, 0, 0 }, // 190
    { 0, 0, 0 }, // 191
    { 0, 0, 0 }, // 192
    { 0, 0, 0 }, // 193
    { 0, 0, 0 }, // 194
    { 0, 0, 0 }, // 195
    { 0, 0, 0 }, // 196
    { 0, 0, 0 }, // 197
    { 0, 0, 0 }, // 198
    { 0, 0, 0 }, // 199
    { 0, 0, 0 }, // 200
    { 0, 0, 0 }, // 201
    { 0, 0, 0 }, // 202
    { 0, 0, 0 }, // 203
    { 0, 0, 0 }, // 204
    { 0, 0, 0 }, // 205
    { 0, 0, 0 }, // 206
    { 0, 0, 0 }, // 207
    { 0, 0, 0 }, // 208
    { 0, 0, 0 }, // 209
    { 0, 0, 0 }, // 210
    { 0, 0, 0 }, // 211
    { 0, 0, 0 }, // 212
    { 0, 0, 0 }, // 213
    { 0, 0, 0 }, // 214
    { 0, 0, 0 }, // 215
    { 0, 0, 0 }, // 216
    { 0, 0, 0 }, // 217
    { 0, 0, 0 }, // 218
    { 0, 0, 0 }, // 219
    { 0, 0, 0 }, // 220
    { 0, 0, 0 }, // 221
    { 0, 0, 0 }, // 222
    { 0, 0, 0 }, // 223
    { 0, 0, 0 }, // 224
    { 0, 0, 0 }, // 225
    { 0, 0, 0 }, // 226
    { 0, 0, 0 }, // 227
    { 0, 0, 0 }, // 228
    { 0, 0, 0 }, // 229
    { 0, 0, 0 }, // 230
    { 0, 0, 0 }, // 231
    { 0, 0, 0 }, // 232
    { 0, 0, 0 }, // 233
    { 0, 0, 0 }, // 234
    { 0, 0, 0 }, // 235
    { 0, 0, 0 }, // 236
    { 0, 0, 0 }, // 237
    { 0, 0, 0 }, // 238
    { 0, 0, 0 }, // 239
    { 0, 0, 0 }, // 240
    { 0, 0, 0 }, // 241
    { 0, 0, 0 }, // 242
    { 0, 0, 0 }, // 243
    { 0, 0, 0 }, // 244
    { 0, 0, 0 }, // 245
    { 0, 0, 0 }, // 246
    { 0, 0, 0 }, // 247
    { 0, 0, 0 }, // 248
    { 0, 0, 0 }, // 249
    { 0, 0, 0 }, // 250
    { 0, 0, 0 }, // 251
    { 0, 0, 0 }, // 252
    { 0, 0, 0 }, // 253
    { 0, 0, 0 }, // 254
    { 0, 0, 0 }, // 255
    { 0, 0, 0 }, // 256
    { 0, 0, 0 }, // 257
    { 0, 0, 0 }, // 258
    { 0, 0, 0 }, // 259
    { 0, 0, 0 }, // 260
    { 0, 0, 0 }, // 261
    { 0, 0, 0 }, // 262
    { 0, 0, 0 }, // 263
    { 0, 0, 0 }, // 264
    { 0, 0, 0 }, // 265
    { 0, 0, 0 }, // 266
    { 0, 0, 0 }, // 267
    { 0, 0, 0 }, // 268
    { 0, 0, 0 }, // 269
    { 0, 0, 0 }, // 270
    { 0, 0, 0 }, // 271
    { 0, 0, 0 }, // 272
    { 0, 0, 0 }, // 273
    { 0, 0, 0 }, // 274
    { 0, 0, 0 }, // 275
    { 0, 0, 0 }, // 276
    { 0, 0, 0 }, // 277
    { 0, 0, 0 }, // 278
    { 0, 0, 0 }, // 279
    { 0, 0, 0 }, // 280
    { 0, 0, 0 }, // 281
    { 0, 0, 0 }, // 282
    { 0, 0, 0 }, // 283
    { 0, 0, 0 }, // 284
    { 0, 0, 0 }, // 285
    { 0, 0, 0 }, // 286
    { 0, 0, 0 }, // 287
    { 0, 0, 0 }, // 288
    { 0, 0, 0 }, // 289
    { 0, 0, 0 }, // 290
    //[PIX_FMT_RGBA64BE] = // 291
    {
        FF_COLOR_RGB,
        1,
    },
    //[PIX_FMT_RGBA64LE] = // 292
    {
        FF_COLOR_RGB,
        1,
    },
    //[PIX_FMT_BGRA64BE] = // 293
    {
        FF_COLOR_RGB,
        1,
    },
    //[PIX_FMT_BGRA64LE] = // 294
    {
        FF_COLOR_RGB,
        1,
    },
#endif
};
#else
static const PixFmtInfo pix_fmt_info[PIX_FMT_NB] = {
    /* YUV formats */
    [PIX_FMT_YUV420P] = {
        .color_type = FF_COLOR_YUV,
    },
    [PIX_FMT_YUV422P] = {
        .color_type = FF_COLOR_YUV,
    },
    [PIX_FMT_YUV444P] = {
        .color_type = FF_COLOR_YUV,
    },
    [PIX_FMT_YUYV422] = {
        .color_type = FF_COLOR_YUV,
    },
    [PIX_FMT_UYVY422] = {
        .color_type = FF_COLOR_YUV,
    },
    [PIX_FMT_YUV410P] = {
        .color_type = FF_COLOR_YUV,
    },
    [PIX_FMT_YUV411P] = {
        .color_type = FF_COLOR_YUV,
    },
    [PIX_FMT_YUV440P] = {
        .color_type = FF_COLOR_YUV,
    },
    [PIX_FMT_YUV420P16LE] = {
        .color_type = FF_COLOR_YUV,
    },
    [PIX_FMT_YUV422P16LE] = {
        .color_type = FF_COLOR_YUV,
    },
    [PIX_FMT_YUV444P16LE] = {
        .color_type = FF_COLOR_YUV,
    },
    [PIX_FMT_YUV420P16BE] = {
        .color_type = FF_COLOR_YUV,
    },
    [PIX_FMT_YUV422P16BE] = {
        .color_type = FF_COLOR_YUV,
    },
    [PIX_FMT_YUV444P16BE] = {
        .color_type = FF_COLOR_YUV,
    },

    /* YUV formats with alpha plane */
    [PIX_FMT_YUVA420P] = {
        .color_type = FF_COLOR_YUV,
    },

    /* JPEG YUV */
    [PIX_FMT_YUVJ420P] = {
        .color_type = FF_COLOR_YUV_JPEG,
    },
    [PIX_FMT_YUVJ422P] = {
        .color_type = FF_COLOR_YUV_JPEG,
    },
    [PIX_FMT_YUVJ444P] = {
        .color_type = FF_COLOR_YUV_JPEG,
    },
    [PIX_FMT_YUVJ440P] = {
        .color_type = FF_COLOR_YUV_JPEG,
    },

    /* RGB formats */
    [PIX_FMT_RGB24] = {
        .color_type = FF_COLOR_RGB,
    },
    [PIX_FMT_BGR24] = {
        .color_type = FF_COLOR_RGB,
    },
    [PIX_FMT_ARGB] = {
        .is_alpha = 1,
        .color_type = FF_COLOR_RGB,
    },
    [PIX_FMT_RGB48BE] = {
        .color_type = FF_COLOR_RGB,
    },
    [PIX_FMT_RGB48LE] = {
        .color_type = FF_COLOR_RGB,
    },
    [PIX_FMT_RGBA64BE] = {
        .is_alpha = 1,
        .color_type = FF_COLOR_RGB,
    },
    [PIX_FMT_RGBA64LE] = {
        .is_alpha = 1,
        .color_type = FF_COLOR_RGB,
    },
    [PIX_FMT_RGB565BE] = {
        .color_type = FF_COLOR_RGB,
    },
    [PIX_FMT_RGB565LE] = {
        .color_type = FF_COLOR_RGB,
    },
    [PIX_FMT_RGB555BE] = {
        .color_type = FF_COLOR_RGB,
        .padded_size = 16,
    },
    [PIX_FMT_RGB555LE] = {
        .color_type = FF_COLOR_RGB,
        .padded_size = 16,
    },
    [PIX_FMT_RGB444BE] = {
        .color_type = FF_COLOR_RGB,
        .padded_size = 16,
    },
    [PIX_FMT_RGB444LE] = {
        .color_type = FF_COLOR_RGB,
        .padded_size = 16,
    },

    /* gray / mono formats */
    [PIX_FMT_GRAY16BE] = {
        .color_type = FF_COLOR_GRAY,
    },
    [PIX_FMT_GRAY16LE] = {
        .color_type = FF_COLOR_GRAY,
    },
    [PIX_FMT_GRAY8] = {
        .color_type = FF_COLOR_GRAY,
    },
    [PIX_FMT_MONOWHITE] = {
        .color_type = FF_COLOR_GRAY,
    },
    [PIX_FMT_MONOBLACK] = {
        .color_type = FF_COLOR_GRAY,
    },

    /* paletted formats */
    [PIX_FMT_PAL8] = {
        .is_alpha = 1,
        .color_type = FF_COLOR_RGB,
    },
    [PIX_FMT_UYYVYY411] = {
        .color_type = FF_COLOR_YUV,
    },
    [PIX_FMT_ABGR] = {
        .is_alpha = 1,
        .color_type = FF_COLOR_RGB,
    },
    [PIX_FMT_BGR48BE] = {
        .color_type = FF_COLOR_RGB,
    },
    [PIX_FMT_BGR48LE] = {
        .color_type = FF_COLOR_RGB,
    },
    [PIX_FMT_BGRA64BE] = {
        .is_alpha = 1,
        .color_type = FF_COLOR_RGB,
    },
    [PIX_FMT_BGRA64LE] = {
        .is_alpha = 1,
        .color_type = FF_COLOR_RGB,
    },
    [PIX_FMT_BGR565BE] = {
        .color_type = FF_COLOR_RGB,
        .padded_size = 16,
    },
    [PIX_FMT_BGR565LE] = {
        .color_type = FF_COLOR_RGB,
        .padded_size = 16,
    },
    [PIX_FMT_BGR555BE] = {
        .color_type = FF_COLOR_RGB,
        .padded_size = 16,
    },
    [PIX_FMT_BGR555LE] = {
        .color_type = FF_COLOR_RGB,
        .padded_size = 16,
    },
    [PIX_FMT_BGR444BE] = {
        .color_type = FF_COLOR_RGB,
        .padded_size = 16,
    },
    [PIX_FMT_BGR444LE] = {
        .color_type = FF_COLOR_RGB,
        .padded_size = 16,
    },
    [PIX_FMT_RGB8] = {
        .color_type = FF_COLOR_RGB,
    },
    [PIX_FMT_RGB4] = {
        .color_type = FF_COLOR_RGB,
    },
    [PIX_FMT_RGB4_BYTE] = {
        .color_type = FF_COLOR_RGB,
        .padded_size = 8,
    },
    [PIX_FMT_BGR8] = {
        .color_type = FF_COLOR_RGB,
    },
    [PIX_FMT_BGR4] = {
        .color_type = FF_COLOR_RGB,
    },
    [PIX_FMT_BGR4_BYTE] = {
        .color_type = FF_COLOR_RGB,
        .padded_size = 8,
    },
    [PIX_FMT_NV12] = {
        .color_type = FF_COLOR_YUV,
    },
    [PIX_FMT_NV21] = {
        .color_type = FF_COLOR_YUV,
    },

    [PIX_FMT_BGRA] = {
        .is_alpha = 1,
        .color_type = FF_COLOR_RGB,
    },
    [PIX_FMT_RGBA] = {
        .is_alpha = 1,
        .color_type = FF_COLOR_RGB,
    },
};
#endif

void avcodec_get_chroma_sub_sample(enum PixelFormat pix_fmt, int *h_shift, int *v_shift)
{
    *h_shift = av_pix_fmt_descriptors[pix_fmt].log2_chroma_w;
    *v_shift = av_pix_fmt_descriptors[pix_fmt].log2_chroma_h;
}

#if FF_API_GET_PIX_FMT_NAME
const char *avcodec_get_pix_fmt_name(enum PixelFormat pix_fmt)
{
    return av_get_pix_fmt_name(pix_fmt);
}
#endif

int ff_is_hwaccel_pix_fmt(enum PixelFormat pix_fmt)
{
    return av_pix_fmt_descriptors[pix_fmt].flags & PIX_FMT_HWACCEL;
}

int avpicture_fill(AVPicture *picture, uint8_t *ptr,
                   enum PixelFormat pix_fmt, int width, int height)
{
    int ret;

    if ((ret = av_image_check_size(width, height, 0, NULL)) < 0)
        return ret;

    if ((ret = av_image_fill_linesizes(picture->linesize, pix_fmt, width)) < 0)
        return ret;

    return av_image_fill_pointers(picture->data, pix_fmt, height, ptr, picture->linesize);
}

int avpicture_layout(const AVPicture* src, enum PixelFormat pix_fmt, int width, int height,
                     unsigned char *dest, int dest_size)
{
    int i, j, nb_planes = 0, linesizes[4];
    const AVPixFmtDescriptor *desc = &av_pix_fmt_descriptors[pix_fmt];
    int size = avpicture_get_size(pix_fmt, width, height);

    if (size > dest_size || size < 0)
        return AVERROR(EINVAL);

    for (i = 0; i < desc->nb_components; i++)
        nb_planes = FFMAX(desc->comp[i].plane, nb_planes);
    nb_planes++;

    av_image_fill_linesizes(linesizes, pix_fmt, width);
    for (i = 0; i < nb_planes; i++) {
        int h, shift = (i == 1 || i == 2) ? desc->log2_chroma_h : 0;
        const unsigned char *s = src->data[i];
        h = (height + (1 << shift) - 1) >> shift;

        for (j = 0; j < h; j++) {
            memcpy(dest, s, linesizes[i]);
            dest += linesizes[i];
            s += src->linesize[i];
        }
    }

    switch (pix_fmt) {
    case PIX_FMT_RGB8:
    case PIX_FMT_BGR8:
    case PIX_FMT_RGB4_BYTE:
    case PIX_FMT_BGR4_BYTE:
    case PIX_FMT_GRAY8:
        // do not include palette for these pseudo-paletted formats
        return size;
    }

    if (desc->flags & PIX_FMT_PAL)
        memcpy((unsigned char *)(((size_t)dest + 3) & ~3), src->data[1], 256 * 4);

    return size;
}

int avpicture_get_size(enum PixelFormat pix_fmt, int width, int height)
{
    AVPicture dummy_pict;
    if(av_image_check_size(width, height, 0, NULL))
        return -1;
    switch (pix_fmt) {
    case PIX_FMT_RGB8:
    case PIX_FMT_BGR8:
    case PIX_FMT_RGB4_BYTE:
    case PIX_FMT_BGR4_BYTE:
    case PIX_FMT_GRAY8:
        // do not include palette for these pseudo-paletted formats
        return width * height;
    }
    return avpicture_fill(&dummy_pict, NULL, pix_fmt, width, height);
}

static int get_pix_fmt_depth(int *min, int *max, enum PixelFormat pix_fmt)
{
    const AVPixFmtDescriptor *desc = &av_pix_fmt_descriptors[pix_fmt];
    int i;

    if (!desc->nb_components) {
        *min = *max = 0;
        return AVERROR(EINVAL);
    }

    *min = INT_MAX, *max = -INT_MAX;
    for (i = 0; i < desc->nb_components; i++) {
        *min = FFMIN(desc->comp[i].depth_minus1+1, *min);
        *max = FFMAX(desc->comp[i].depth_minus1+1, *max);
    }
    return 0;
}

int avcodec_get_pix_fmt_loss(enum PixelFormat dst_pix_fmt, enum PixelFormat src_pix_fmt,
                             int has_alpha)
{
    const PixFmtInfo *pf, *ps;
    const AVPixFmtDescriptor *src_desc;
    const AVPixFmtDescriptor *dst_desc;
    int src_min_depth, src_max_depth, dst_min_depth, dst_max_depth;
    int ret, loss;

    if (dst_pix_fmt >= PIX_FMT_NB || dst_pix_fmt <= PIX_FMT_NONE)
        return ~0;

    src_desc = &av_pix_fmt_descriptors[src_pix_fmt];
    dst_desc = &av_pix_fmt_descriptors[dst_pix_fmt];
    ps = &pix_fmt_info[src_pix_fmt];

    /* compute loss */
    loss = 0;

    if ((ret = get_pix_fmt_depth(&src_min_depth, &src_max_depth, src_pix_fmt)) < 0)
        return ret;
    if ((ret = get_pix_fmt_depth(&dst_min_depth, &dst_max_depth, dst_pix_fmt)) < 0)
        return ret;
    if (dst_min_depth < src_min_depth ||
        dst_max_depth < src_max_depth)
        loss |= FF_LOSS_DEPTH;
    if (dst_desc->log2_chroma_w > src_desc->log2_chroma_w ||
        dst_desc->log2_chroma_h > src_desc->log2_chroma_h)
        loss |= FF_LOSS_RESOLUTION;

    pf = &pix_fmt_info[dst_pix_fmt];
    switch(pf->color_type) {
    case FF_COLOR_RGB:
        if (ps->color_type != FF_COLOR_RGB &&
            ps->color_type != FF_COLOR_GRAY)
            loss |= FF_LOSS_COLORSPACE;
        break;
    case FF_COLOR_GRAY:
        if (ps->color_type != FF_COLOR_GRAY)
            loss |= FF_LOSS_COLORSPACE;
        break;
    case FF_COLOR_YUV:
        if (ps->color_type != FF_COLOR_YUV)
            loss |= FF_LOSS_COLORSPACE;
        break;
    case FF_COLOR_YUV_JPEG:
        if (ps->color_type != FF_COLOR_YUV_JPEG &&
            ps->color_type != FF_COLOR_YUV &&
            ps->color_type != FF_COLOR_GRAY)
            loss |= FF_LOSS_COLORSPACE;
        break;
    default:
        /* fail safe test */
        if (ps->color_type != pf->color_type)
            loss |= FF_LOSS_COLORSPACE;
        break;
    }
    if (pf->color_type == FF_COLOR_GRAY &&
        ps->color_type != FF_COLOR_GRAY)
        loss |= FF_LOSS_CHROMA;
    if (!pf->is_alpha && (ps->is_alpha && has_alpha))
        loss |= FF_LOSS_ALPHA;
    if (dst_pix_fmt == PIX_FMT_PAL8 &&
        (src_pix_fmt != PIX_FMT_PAL8 && ps->color_type != FF_COLOR_GRAY))
        loss |= FF_LOSS_COLORQUANT;
    return loss;
}

static int avg_bits_per_pixel(enum PixelFormat pix_fmt)
{
    const PixFmtInfo *info = &pix_fmt_info[pix_fmt];
    const AVPixFmtDescriptor *desc = &av_pix_fmt_descriptors[pix_fmt];

    return info->padded_size ?
        info->padded_size : av_get_bits_per_pixel(desc);
}

enum PixelFormat avcodec_find_best_pix_fmt(int64_t pix_fmt_mask, enum PixelFormat src_pix_fmt,
                                            int has_alpha, int *loss_ptr)
{
    enum PixelFormat dst_pix_fmt;
    int i;

    if (loss_ptr) /* all losses count (for backward compatibility) */
        *loss_ptr = 0;

    dst_pix_fmt = PIX_FMT_NONE; /* so first iteration doesn't have to be treated special */
    for(i = 0; i< FFMIN(PIX_FMT_NB, 64); i++){
        if (pix_fmt_mask & (1ULL << i))
            dst_pix_fmt = avcodec_find_best_pix_fmt2(dst_pix_fmt, i, src_pix_fmt, has_alpha, loss_ptr);
    }
    return dst_pix_fmt;
}

enum PixelFormat avcodec_find_best_pix_fmt2(enum PixelFormat dst_pix_fmt1, enum PixelFormat dst_pix_fmt2,
                                            enum PixelFormat src_pix_fmt, int has_alpha, int *loss_ptr)
{
    enum PixelFormat dst_pix_fmt;
    int loss1, loss2, loss_order1, loss_order2, i, loss_mask;
    static const int loss_mask_order[] = {
        ~0, /* no loss first */
        ~FF_LOSS_ALPHA,
        ~FF_LOSS_RESOLUTION,
        ~(FF_LOSS_COLORSPACE | FF_LOSS_RESOLUTION),
        ~FF_LOSS_COLORQUANT,
        ~FF_LOSS_DEPTH,
        ~(FF_LOSS_RESOLUTION | FF_LOSS_DEPTH | FF_LOSS_COLORSPACE | FF_LOSS_ALPHA |
          FF_LOSS_COLORQUANT | FF_LOSS_CHROMA),
        0x80000, //non zero entry that combines all loss variants including future additions
        0,
    };

    loss_mask= loss_ptr?~*loss_ptr:~0; /* use loss mask if provided */
    dst_pix_fmt = PIX_FMT_NONE;
    loss1 = avcodec_get_pix_fmt_loss(dst_pix_fmt1, src_pix_fmt, has_alpha) & loss_mask;
    loss2 = avcodec_get_pix_fmt_loss(dst_pix_fmt2, src_pix_fmt, has_alpha) & loss_mask;

    /* try with successive loss */
    for(i = 0;loss_mask_order[i] != 0 && dst_pix_fmt == PIX_FMT_NONE;i++) {
        loss_order1 = loss1 & loss_mask_order[i];
        loss_order2 = loss2 & loss_mask_order[i];

        if (loss_order1 == 0 && loss_order2 == 0){ /* use format with smallest depth */
            dst_pix_fmt = avg_bits_per_pixel(dst_pix_fmt2) < avg_bits_per_pixel(dst_pix_fmt1) ? dst_pix_fmt2 : dst_pix_fmt1;
        } else if (loss_order1 == 0 || loss_order2 == 0) { /* use format with no loss */
            dst_pix_fmt = loss_order2 ? dst_pix_fmt1 : dst_pix_fmt2;
        }
    }

    if (loss_ptr)
        *loss_ptr = avcodec_get_pix_fmt_loss(dst_pix_fmt, src_pix_fmt, has_alpha);
    return dst_pix_fmt;
}

void av_picture_copy(AVPicture *dst, const AVPicture *src,
                     enum PixelFormat pix_fmt, int width, int height)
{
    av_image_copy(dst->data, dst->linesize, src->data,
                  src->linesize, pix_fmt, width, height);
}

/* 2x2 -> 1x1 */
void ff_shrink22(uint8_t *dst, int dst_wrap,
                     const uint8_t *src, int src_wrap,
                     int width, int height)
{
    int w;
    const uint8_t *s1, *s2;
    uint8_t *d;

    for(;height > 0; height--) {
        s1 = src;
        s2 = s1 + src_wrap;
        d = dst;
        for(w = width;w >= 4; w-=4) {
            d[0] = (s1[0] + s1[1] + s2[0] + s2[1] + 2) >> 2;
            d[1] = (s1[2] + s1[3] + s2[2] + s2[3] + 2) >> 2;
            d[2] = (s1[4] + s1[5] + s2[4] + s2[5] + 2) >> 2;
            d[3] = (s1[6] + s1[7] + s2[6] + s2[7] + 2) >> 2;
            s1 += 8;
            s2 += 8;
            d += 4;
        }
        for(;w > 0; w--) {
            d[0] = (s1[0] + s1[1] + s2[0] + s2[1] + 2) >> 2;
            s1 += 2;
            s2 += 2;
            d++;
        }
        src += 2 * src_wrap;
        dst += dst_wrap;
    }
}

/* 4x4 -> 1x1 */
void ff_shrink44(uint8_t *dst, int dst_wrap,
                     const uint8_t *src, int src_wrap,
                     int width, int height)
{
    int w;
    const uint8_t *s1, *s2, *s3, *s4;
    uint8_t *d;

    for(;height > 0; height--) {
        s1 = src;
        s2 = s1 + src_wrap;
        s3 = s2 + src_wrap;
        s4 = s3 + src_wrap;
        d = dst;
        for(w = width;w > 0; w--) {
            d[0] = (s1[0] + s1[1] + s1[2] + s1[3] +
                    s2[0] + s2[1] + s2[2] + s2[3] +
                    s3[0] + s3[1] + s3[2] + s3[3] +
                    s4[0] + s4[1] + s4[2] + s4[3] + 8) >> 4;
            s1 += 4;
            s2 += 4;
            s3 += 4;
            s4 += 4;
            d++;
        }
        src += 4 * src_wrap;
        dst += dst_wrap;
    }
}

/* 8x8 -> 1x1 */
void ff_shrink88(uint8_t *dst, int dst_wrap,
                     const uint8_t *src, int src_wrap,
                     int width, int height)
{
    int w, i;

    for(;height > 0; height--) {
        for(w = width;w > 0; w--) {
            int tmp=0;
            for(i=0; i<8; i++){
                tmp += src[0] + src[1] + src[2] + src[3] + src[4] + src[5] + src[6] + src[7];
                src += src_wrap;
            }
            *(dst++) = (tmp + 32)>>6;
            src += 8 - 8*src_wrap;
        }
        src += 8*src_wrap - 8*width;
        dst += dst_wrap - width;
    }
}


int avpicture_alloc(AVPicture *picture,
                    enum PixelFormat pix_fmt, int width, int height)
{
    int ret;

    if ((ret = av_image_alloc(picture->data, picture->linesize, width, height, pix_fmt, 1)) < 0) {
        memset(picture, 0, sizeof(AVPicture));
        return ret;
    }

    return 0;
}

void avpicture_free(AVPicture *picture)
{
    av_free(picture->data[0]);
}

/* return true if yuv planar */
static inline int is_yuv_planar(enum PixelFormat fmt)
{
    const PixFmtInfo         *info = &pix_fmt_info[fmt];
    const AVPixFmtDescriptor *desc = &av_pix_fmt_descriptors[fmt];
    int i;
    int planes[4] = { 0 };

    if (info->color_type != FF_COLOR_YUV &&
        info->color_type != FF_COLOR_YUV_JPEG)
        return 0;

    /* set the used planes */
    for (i = 0; i < desc->nb_components; i++)
        planes[desc->comp[i].plane] = 1;

    /* if there is an unused plane, the format is not planar */
    for (i = 0; i < desc->nb_components; i++)
        if (!planes[i])
            return 0;
    return 1;
}

int av_picture_crop(AVPicture *dst, const AVPicture *src,
                    enum PixelFormat pix_fmt, int top_band, int left_band)
{
    int y_shift;
    int x_shift;

    if (pix_fmt < 0 || pix_fmt >= PIX_FMT_NB)
        return -1;

    y_shift = av_pix_fmt_descriptors[pix_fmt].log2_chroma_h;
    x_shift = av_pix_fmt_descriptors[pix_fmt].log2_chroma_w;

    if (is_yuv_planar(pix_fmt)) {
    dst->data[0] = src->data[0] + (top_band * src->linesize[0]) + left_band;
    dst->data[1] = src->data[1] + ((top_band >> y_shift) * src->linesize[1]) + (left_band >> x_shift);
    dst->data[2] = src->data[2] + ((top_band >> y_shift) * src->linesize[2]) + (left_band >> x_shift);
    } else{
        if(top_band % (1<<y_shift) || left_band % (1<<x_shift))
            return -1;
        if(left_band) //FIXME add support for this too
            return -1;
        dst->data[0] = src->data[0] + (top_band * src->linesize[0]) + left_band;
    }

    dst->linesize[0] = src->linesize[0];
    dst->linesize[1] = src->linesize[1];
    dst->linesize[2] = src->linesize[2];
    return 0;
}

int av_picture_pad(AVPicture *dst, const AVPicture *src, int height, int width,
                   enum PixelFormat pix_fmt, int padtop, int padbottom, int padleft, int padright,
            int *color)
{
    uint8_t *optr;
    int y_shift;
    int x_shift;
    int yheight;
    int i, y;

    if (pix_fmt < 0 || pix_fmt >= PIX_FMT_NB ||
        !is_yuv_planar(pix_fmt)) return -1;

    for (i = 0; i < 3; i++) {
        x_shift = i ? av_pix_fmt_descriptors[pix_fmt].log2_chroma_w : 0;
        y_shift = i ? av_pix_fmt_descriptors[pix_fmt].log2_chroma_h : 0;

        if (padtop || padleft) {
            memset(dst->data[i], color[i],
                dst->linesize[i] * (padtop >> y_shift) + (padleft >> x_shift));
        }

        if (padleft || padright) {
            optr = dst->data[i] + dst->linesize[i] * (padtop >> y_shift) +
                (dst->linesize[i] - (padright >> x_shift));
            yheight = (height - 1 - (padtop + padbottom)) >> y_shift;
            for (y = 0; y < yheight; y++) {
                memset(optr, color[i], (padleft + padright) >> x_shift);
                optr += dst->linesize[i];
            }
        }

        if (src) { /* first line */
            uint8_t *iptr = src->data[i];
            optr = dst->data[i] + dst->linesize[i] * (padtop >> y_shift) +
                    (padleft >> x_shift);
            memcpy(optr, iptr, (width - padleft - padright) >> x_shift);
            iptr += src->linesize[i];
            optr = dst->data[i] + dst->linesize[i] * (padtop >> y_shift) +
                (dst->linesize[i] - (padright >> x_shift));
            yheight = (height - 1 - (padtop + padbottom)) >> y_shift;
            for (y = 0; y < yheight; y++) {
                memset(optr, color[i], (padleft + padright) >> x_shift);
                memcpy(optr + ((padleft + padright) >> x_shift), iptr,
                       (width - padleft - padright) >> x_shift);
                iptr += src->linesize[i];
                optr += dst->linesize[i];
            }
        }

        if (padbottom || padright) {
            optr = dst->data[i] + dst->linesize[i] *
                ((height - padbottom) >> y_shift) - (padright >> x_shift);
            memset(optr, color[i],dst->linesize[i] *
                (padbottom >> y_shift) + (padright >> x_shift));
        }
    }
    return 0;
}

#if FF_API_GET_ALPHA_INFO
/* NOTE: we scan all the pixels to have an exact information */
static int get_alpha_info_pal8(const AVPicture *src, int width, int height)
{
    const unsigned char *p;
    int src_wrap, ret, x, y;
    unsigned int a;
    uint32_t *palette = (uint32_t *)src->data[1];

    p = src->data[0];
    src_wrap = src->linesize[0] - width;
    ret = 0;
    for(y=0;y<height;y++) {
        for(x=0;x<width;x++) {
            a = palette[p[0]] >> 24;
            if (a == 0x00) {
                ret |= FF_ALPHA_TRANSP;
            } else if (a != 0xff) {
                ret |= FF_ALPHA_SEMI_TRANSP;
            }
            p++;
        }
        p += src_wrap;
    }
    return ret;
}

int img_get_alpha_info(const AVPicture *src,
                       enum PixelFormat pix_fmt, int width, int height)
{
    const PixFmtInfo *pf = &pix_fmt_info[pix_fmt];
    int ret;

    /* no alpha can be represented in format */
    if (!pf->is_alpha)
        return 0;
    switch(pix_fmt) {
    case PIX_FMT_PAL8:
        ret = get_alpha_info_pal8(src, width, height);
        break;
    default:
        /* we do not know, so everything is indicated */
        ret = FF_ALPHA_TRANSP | FF_ALPHA_SEMI_TRANSP;
        break;
    }
    return ret;
}
#endif

#if !(HAVE_MMX && HAVE_YASM)
/* filter parameters: [-1 4 2 4 -1] // 8 */
static void deinterlace_line_c(uint8_t *dst,
                             const uint8_t *lum_m4, const uint8_t *lum_m3,
                             const uint8_t *lum_m2, const uint8_t *lum_m1,
                             const uint8_t *lum,
                             int size)
{
    uint8_t *cm = ff_cropTbl + MAX_NEG_CROP;
    int sum;

    for(;size > 0;size--) {
        sum = -lum_m4[0];
        sum += lum_m3[0] << 2;
        sum += lum_m2[0] << 1;
        sum += lum_m1[0] << 2;
        sum += -lum[0];
        dst[0] = cm[(sum + 4) >> 3];
        lum_m4++;
        lum_m3++;
        lum_m2++;
        lum_m1++;
        lum++;
        dst++;
    }
}

static void deinterlace_line_inplace_c(uint8_t *lum_m4, uint8_t *lum_m3,
                                       uint8_t *lum_m2, uint8_t *lum_m1,
                                       uint8_t *lum, int size)
{
    uint8_t *cm = ff_cropTbl + MAX_NEG_CROP;
    int sum;

    for(;size > 0;size--) {
        sum = -lum_m4[0];
        sum += lum_m3[0] << 2;
        sum += lum_m2[0] << 1;
        lum_m4[0]=lum_m2[0];
        sum += lum_m1[0] << 2;
        sum += -lum[0];
        lum_m2[0] = cm[(sum + 4) >> 3];
        lum_m4++;
        lum_m3++;
        lum_m2++;
        lum_m1++;
        lum++;
    }
}
#endif

/* deinterlacing : 2 temporal taps, 3 spatial taps linear filter. The
   top field is copied as is, but the bottom field is deinterlaced
   against the top field. */
static void deinterlace_bottom_field(uint8_t *dst, int dst_wrap,
                                    const uint8_t *src1, int src_wrap,
                                    int width, int height)
{
    const uint8_t *src_m2, *src_m1, *src_0, *src_p1, *src_p2;
    int y;

    src_m2 = src1;
    src_m1 = src1;
    src_0=&src_m1[src_wrap];
    src_p1=&src_0[src_wrap];
    src_p2=&src_p1[src_wrap];
    for(y=0;y<(height-2);y+=2) {
        memcpy(dst,src_m1,width);
        dst += dst_wrap;
        deinterlace_line(dst,src_m2,src_m1,src_0,src_p1,src_p2,width);
        src_m2 = src_0;
        src_m1 = src_p1;
        src_0 = src_p2;
        src_p1 += 2*src_wrap;
        src_p2 += 2*src_wrap;
        dst += dst_wrap;
    }
    memcpy(dst,src_m1,width);
    dst += dst_wrap;
    /* do last line */
    deinterlace_line(dst,src_m2,src_m1,src_0,src_0,src_0,width);
}

static void deinterlace_bottom_field_inplace(uint8_t *src1, int src_wrap,
                                             int width, int height)
{
    uint8_t *src_m1, *src_0, *src_p1, *src_p2;
    int y;
    uint8_t *buf;
    buf = (uint8_t*)av_malloc(width);

    src_m1 = src1;
    memcpy(buf,src_m1,width);
    src_0=&src_m1[src_wrap];
    src_p1=&src_0[src_wrap];
    src_p2=&src_p1[src_wrap];
    for(y=0;y<(height-2);y+=2) {
        deinterlace_line_inplace(buf,src_m1,src_0,src_p1,src_p2,width);
        src_m1 = src_p1;
        src_0 = src_p2;
        src_p1 += 2*src_wrap;
        src_p2 += 2*src_wrap;
    }
    /* do last line */
    deinterlace_line_inplace(buf,src_m1,src_0,src_0,src_0,width);
    av_free(buf);
}

int avpicture_deinterlace(AVPicture *dst, const AVPicture *src,
                          enum PixelFormat pix_fmt, int width, int height)
{
    int i;

    if (pix_fmt != PIX_FMT_YUV420P &&
        pix_fmt != PIX_FMT_YUVJ420P &&
        pix_fmt != PIX_FMT_YUV422P &&
        pix_fmt != PIX_FMT_YUVJ422P &&
        pix_fmt != PIX_FMT_YUV444P &&
        pix_fmt != PIX_FMT_YUV411P &&
        pix_fmt != PIX_FMT_GRAY8)
        return -1;
    if ((width & 3) != 0 || (height & 3) != 0)
        return -1;

    for(i=0;i<3;i++) {
        if (i == 1) {
            switch(pix_fmt) {
            case PIX_FMT_YUVJ420P:
            case PIX_FMT_YUV420P:
                width >>= 1;
                height >>= 1;
                break;
            case PIX_FMT_YUV422P:
            case PIX_FMT_YUVJ422P:
                width >>= 1;
                break;
            case PIX_FMT_YUV411P:
                width >>= 2;
                break;
            default:
                break;
            }
            if (pix_fmt == PIX_FMT_GRAY8) {
                break;
            }
        }
        if (src == dst) {
            deinterlace_bottom_field_inplace(dst->data[i], dst->linesize[i],
                                 width, height);
        } else {
            deinterlace_bottom_field(dst->data[i],dst->linesize[i],
                                        src->data[i], src->linesize[i],
                                        width, height);
        }
    }
    emms_c();
    return 0;
}


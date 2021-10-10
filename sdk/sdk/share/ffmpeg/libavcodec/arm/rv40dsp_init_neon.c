﻿/*
 * Copyright (c) 2011 Janne Grunau <janne-libav@jannau.net>
 *
 * This file is part of Libav.
 *
 * Libav is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Libav is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Libav; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdint.h>

#include "libavcodec/avcodec.h"
#include "libavcodec/rv34dsp.h"

#define DECL_QPEL3(type, w, pos) \
    void ff_##type##_rv40_qpel##w##_mc##pos##_neon(uint8_t *dst, uint8_t *src,\
                                                   int stride)
#define DECL_QPEL2(w, pos)                      \
    DECL_QPEL3(put, w, pos);                    \
    DECL_QPEL3(avg, w, pos)

#define DECL_QPEL_XY(x, y)                      \
    DECL_QPEL2(16, x ## y);                     \
    DECL_QPEL2(8,  x ## y)

#define DECL_QPEL_Y(y)                          \
    DECL_QPEL_XY(0, y);                         \
    DECL_QPEL_XY(1, y);                         \
    DECL_QPEL_XY(2, y);                         \
    DECL_QPEL_XY(3, y);                         \

DECL_QPEL_Y(0);
DECL_QPEL_Y(1);
DECL_QPEL_Y(2);
DECL_QPEL_Y(3);

void ff_put_rv40_chroma_mc8_neon(uint8_t *, uint8_t *, int, int, int, int);
void ff_put_rv40_chroma_mc4_neon(uint8_t *, uint8_t *, int, int, int, int);

void ff_avg_rv40_chroma_mc8_neon(uint8_t *, uint8_t *, int, int, int, int);
void ff_avg_rv40_chroma_mc4_neon(uint8_t *, uint8_t *, int, int, int, int);

void ff_rv40_weight_func_16_neon(uint8_t *, uint8_t *, uint8_t *, int, int, int);
void ff_rv40_weight_func_8_neon(uint8_t *, uint8_t *, uint8_t *, int, int, int);

void ff_rv40dsp_init_neon(RV34DSPContext *c, DSPContext* dsp)
{
    c->put_pixels_tab[0][ 1] = ff_put_rv40_qpel16_mc10_neon;
    c->put_pixels_tab[0][ 3] = ff_put_rv40_qpel16_mc30_neon;
    c->put_pixels_tab[0][ 4] = ff_put_rv40_qpel16_mc01_neon;
    c->put_pixels_tab[0][ 5] = ff_put_rv40_qpel16_mc11_neon;
    c->put_pixels_tab[0][ 6] = ff_put_rv40_qpel16_mc21_neon;
    c->put_pixels_tab[0][ 7] = ff_put_rv40_qpel16_mc31_neon;
    c->put_pixels_tab[0][ 9] = ff_put_rv40_qpel16_mc12_neon;
    c->put_pixels_tab[0][10] = ff_put_rv40_qpel16_mc22_neon;
    c->put_pixels_tab[0][11] = ff_put_rv40_qpel16_mc32_neon;
    c->put_pixels_tab[0][12] = ff_put_rv40_qpel16_mc03_neon;
    c->put_pixels_tab[0][13] = ff_put_rv40_qpel16_mc13_neon;
    c->put_pixels_tab[0][14] = ff_put_rv40_qpel16_mc23_neon;
    c->put_pixels_tab[0][15] = ff_put_rv40_qpel16_mc33_neon;
    c->avg_pixels_tab[0][ 1] = ff_avg_rv40_qpel16_mc10_neon;
    c->avg_pixels_tab[0][ 3] = ff_avg_rv40_qpel16_mc30_neon;
    c->avg_pixels_tab[0][ 4] = ff_avg_rv40_qpel16_mc01_neon;
    c->avg_pixels_tab[0][ 5] = ff_avg_rv40_qpel16_mc11_neon;
    c->avg_pixels_tab[0][ 6] = ff_avg_rv40_qpel16_mc21_neon;
    c->avg_pixels_tab[0][ 7] = ff_avg_rv40_qpel16_mc31_neon;
    c->avg_pixels_tab[0][ 9] = ff_avg_rv40_qpel16_mc12_neon;
    c->avg_pixels_tab[0][10] = ff_avg_rv40_qpel16_mc22_neon;
    c->avg_pixels_tab[0][11] = ff_avg_rv40_qpel16_mc32_neon;
    c->avg_pixels_tab[0][12] = ff_avg_rv40_qpel16_mc03_neon;
    c->avg_pixels_tab[0][13] = ff_avg_rv40_qpel16_mc13_neon;
    c->avg_pixels_tab[0][14] = ff_avg_rv40_qpel16_mc23_neon;
    c->avg_pixels_tab[0][15] = ff_avg_rv40_qpel16_mc33_neon;
    c->put_pixels_tab[1][ 1] = ff_put_rv40_qpel8_mc10_neon;
    c->put_pixels_tab[1][ 3] = ff_put_rv40_qpel8_mc30_neon;
    c->put_pixels_tab[1][ 4] = ff_put_rv40_qpel8_mc01_neon;
    c->put_pixels_tab[1][ 5] = ff_put_rv40_qpel8_mc11_neon;
    c->put_pixels_tab[1][ 6] = ff_put_rv40_qpel8_mc21_neon;
    c->put_pixels_tab[1][ 7] = ff_put_rv40_qpel8_mc31_neon;
    c->put_pixels_tab[1][ 9] = ff_put_rv40_qpel8_mc12_neon;
    c->put_pixels_tab[1][10] = ff_put_rv40_qpel8_mc22_neon;
    c->put_pixels_tab[1][11] = ff_put_rv40_qpel8_mc32_neon;
    c->put_pixels_tab[1][12] = ff_put_rv40_qpel8_mc03_neon;
    c->put_pixels_tab[1][13] = ff_put_rv40_qpel8_mc13_neon;
    c->put_pixels_tab[1][14] = ff_put_rv40_qpel8_mc23_neon;
    c->put_pixels_tab[1][15] = ff_put_rv40_qpel8_mc33_neon;
    c->avg_pixels_tab[1][ 1] = ff_avg_rv40_qpel8_mc10_neon;
    c->avg_pixels_tab[1][ 3] = ff_avg_rv40_qpel8_mc30_neon;
    c->avg_pixels_tab[1][ 4] = ff_avg_rv40_qpel8_mc01_neon;
    c->avg_pixels_tab[1][ 5] = ff_avg_rv40_qpel8_mc11_neon;
    c->avg_pixels_tab[1][ 6] = ff_avg_rv40_qpel8_mc21_neon;
    c->avg_pixels_tab[1][ 7] = ff_avg_rv40_qpel8_mc31_neon;
    c->avg_pixels_tab[1][ 9] = ff_avg_rv40_qpel8_mc12_neon;
    c->avg_pixels_tab[1][10] = ff_avg_rv40_qpel8_mc22_neon;
    c->avg_pixels_tab[1][11] = ff_avg_rv40_qpel8_mc32_neon;
    c->avg_pixels_tab[1][12] = ff_avg_rv40_qpel8_mc03_neon;
    c->avg_pixels_tab[1][13] = ff_avg_rv40_qpel8_mc13_neon;
    c->avg_pixels_tab[1][14] = ff_avg_rv40_qpel8_mc23_neon;
    c->avg_pixels_tab[1][15] = ff_avg_rv40_qpel8_mc33_neon;

    c->put_chroma_pixels_tab[0] = ff_put_rv40_chroma_mc8_neon;
    c->put_chroma_pixels_tab[1] = ff_put_rv40_chroma_mc4_neon;
    c->avg_chroma_pixels_tab[0] = ff_avg_rv40_chroma_mc8_neon;
    c->avg_chroma_pixels_tab[1] = ff_avg_rv40_chroma_mc4_neon;

    c->rv40_weight_pixels_tab[0] = ff_rv40_weight_func_16_neon;
    c->rv40_weight_pixels_tab[1] = ff_rv40_weight_func_8_neon;
}

/*
 * AC-3 tables
 * copyright (c) 2001 Fabrice Bellard
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
 * tables taken directly from the AC-3 spec.
 */

//#include "audioconvert.h"
#include "ac3tab.h"

/*! Channel exists table */
const int8_t ff_channeltab[8][6] =
{
	{FRONT_LEFT, FRONT_RIGHT, LOW_FREQUENCY, NONE, NONE, NONE},
	{FRONT_CENTER, LOW_FREQUENCY, NONE, NONE, NONE, NONE},
	{FRONT_LEFT, FRONT_RIGHT, LOW_FREQUENCY, NONE, NONE, NONE},
	{FRONT_LEFT, FRONT_CENTER, FRONT_RIGHT, LOW_FREQUENCY, NONE, NONE},
	{FRONT_LEFT, FRONT_RIGHT, BACK_CENTER, LOW_FREQUENCY, NONE, NONE},
	{FRONT_LEFT, FRONT_CENTER, FRONT_RIGHT, BACK_CENTER, LOW_FREQUENCY, NONE},
	{FRONT_LEFT, FRONT_RIGHT, BACK_LEFT, BACK_RIGHT, LOW_FREQUENCY, NONE},
	{FRONT_LEFT, FRONT_CENTER, FRONT_RIGHT, BACK_LEFT, BACK_RIGHT, LOW_FREQUENCY}
};

/**
 * Map audio coding mode (acmod) to number of full-bandwidth channels.
 * from ATSC A/52 Table 5.8 Audio Coding Mode
 */
const uint8_t ff_ac3_channels_tab[8] = {
    2, 1, 2, 3, 3, 4, 4, 5
};

/**
 * Table to remap channels from from AC-3 order to SMPTE order.
 * [channel_mode][lfe][ch]
 */
/*const uint8_t ff_ac3_dec_channel_map[8][2][6] = {
    { { 0, 1,          }, { 0, 1, 2,         } },
    { { 0,             }, { 0, 1,            } },
    { { 0, 1,          }, { 0, 1, 2,         } },
    { { 0, 2, 1,       }, { 0, 2, 1, 3,      } },
    { { 0, 1, 2,       }, { 0, 1, 3, 2,      } },
    { { 0, 2, 1, 3,    }, { 0, 2, 1, 4, 3,   } },
    { { 0, 1, 2, 3,    }, { 0, 1, 4, 2, 3,   } },
    { { 0, 2, 1, 3, 4, }, { 0, 2, 1, 5, 3, 4 } },
};*/


/* AC-3 MDCT window */


const uint8_t ff_ac3_bap_tab[64]= {
    0, 1, 1, 1, 1, 1, 2, 2, 3, 3,
    3, 4, 4, 5, 5, 6, 6, 6, 6, 7,
    7, 7, 7, 8, 8, 8, 8, 9, 9, 9,
    9, 10, 10, 10, 10, 11, 11, 11, 11, 12,
    12, 12, 12, 13, 13, 13, 13, 14, 14, 14,
    14, 14, 14, 14, 14, 15, 15, 15, 15, 15,
    15, 15, 15, 15,
};

const uint8_t ff_ac3_slow_decay_tab[4]={
    0x0f, 0x11, 0x13, 0x15,
};

const uint8_t ff_ac3_fast_decay_tab[4]={
    0x3f, 0x53, 0x67, 0x7b,
};

const uint16_t ff_ac3_slow_gain_tab[4]= {
    0x540, 0x4d8, 0x478, 0x410,
};

const uint16_t ff_ac3_db_per_bit_tab[4]= {
    0x000, 0x700, 0x900, 0xb00,
};

const int16_t ff_ac3_floor_tab[8]= {
    0x2f0, 0x2b0, 0x270, 0x230, 0x1f0, 0x170, 0x0f0, 0xf800,
};

const uint16_t ff_ac3_fast_gain_tab[8]= {
    0x080, 0x100, 0x180, 0x200, 0x280, 0x300, 0x380, 0x400,
};

const uint8_t default_spx_band_struct[17] =
{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 };

/**
 * Table E2.16 Default Coupling Banding Structure
 */
const uint8_t ff_eac3_default_cpl_band_struct[18] =
{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1 };

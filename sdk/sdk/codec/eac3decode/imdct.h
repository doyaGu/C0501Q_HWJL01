/*
 * imdct.h
 * Copyright (C) 2004-2007 Kuoping Hsu <kuoping@smediatech.com>
 * Copyright (C) 2000-2003 Michel Lespinasse <walken@zoy.org>
 * Copyright (C) 1999-2000 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
 *
 * This file is part of a52dec, a free ATSC A-52 stream decoder.
 * See http://liba52.sourceforge.net/ for updates.
 *
 * a52dec is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * a52dec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __IMDCT_H__
#define __IMDCT_H__

#include "common.h"

typedef struct complex_s {
    int32_t real;
    int32_t imag;
} complex_t;

#define BIAS(x) (x)
void ac3_imdct_512(int32_t * data, int32_t * delay, int32_t * output);
void ac3_imdct_256(int32_t * data, int32_t * delay, int32_t * output);
void woad_decode(int32_t * data, int32_t * delay);
void ac3_imdct_init(void);

#endif // __IMDCT_H__


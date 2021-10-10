/*
 * imdct.c
 * Copyright (C) 2000-2003 Michel Lespinasse <walken@zoy.org>
 * Copyright (C) 1999-2000 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
 *
 * The ifft algorithms in this file have been largely inspired by Dan
 * Bernstein's work, djbfft, available at http://cr.yp.to/djbfft.html
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

#define USE_IMDCT_TABLE

#include <math.h>
#include <stdio.h>
#include "common.h"
#include "math.h"
#include "imdct.h"
#include "imdct_tables.h"

const uint8_t ff_log2_tab[256]={
        0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
        5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
};

static uint8_t fftorder[] = {
    0, 128, 64, 192, 32, 160, 224, 96, 16, 144, 80, 208, 240, 112, 48, 176,
    8, 136, 72, 200, 40, 168, 232, 104, 248, 120, 56, 184, 24, 152, 216, 88,
    4, 132, 68, 196, 36, 164, 228, 100, 20, 148, 84, 212, 244, 116, 52, 180,
    252, 124, 60, 188, 28, 156, 220, 92, 12, 140, 76, 204, 236, 108, 44, 172,
    2, 130, 66, 194, 34, 162, 226, 98, 18, 146, 82, 210, 242, 114, 50, 178,
    10, 138, 74, 202, 42, 170, 234, 106, 250, 122, 58, 186, 26, 154, 218, 90,
    254, 126, 62, 190, 30, 158, 222, 94, 14, 142, 78, 206, 238, 110, 46, 174,
    6, 134, 70, 198, 38, 166, 230, 102, 246, 118, 54, 182, 22, 150, 214, 86
};

#  include "imdct_tables.h"

static __inline void ifft2(complex_t * buf)
{
    int32_t r, i;

    r = buf[0].real;
    i = buf[0].imag;
    buf[0].real += buf[1].real;
    buf[0].imag += buf[1].imag;
    buf[1].real = r - buf[1].real;
    buf[1].imag = i - buf[1].imag;
}

static __inline void ifft4(complex_t * buf)
{
    int32_t tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;

    tmp1 = buf[0].real + buf[1].real;
    tmp2 = buf[3].real + buf[2].real;
    tmp3 = buf[0].imag + buf[1].imag;
    tmp4 = buf[2].imag + buf[3].imag;
    tmp5 = buf[0].real - buf[1].real;
    tmp6 = buf[0].imag - buf[1].imag;
    tmp7 = buf[2].imag - buf[3].imag;
    tmp8 = buf[3].real - buf[2].real;

    buf[0].real = tmp1 + tmp2;
    buf[0].imag = tmp3 + tmp4;
    buf[2].real = tmp1 - tmp2;
    buf[2].imag = tmp3 - tmp4;
    buf[1].real = tmp5 + tmp7;
    buf[1].imag = tmp6 + tmp8;
    buf[3].real = tmp5 - tmp7;
    buf[3].imag = tmp6 - tmp8;
}

/* basic radix-2 ifft butterfly */

#define BUTTERFLY_0(t0,t1,W0,W1,d0,d1) do { \
    t0 = MUL_Add(W1, d1, W0, d0);       \
    t1 = MUL_Sub(W0, d1, W1, d0);       \
} while (0)

/* radix-2 ifft butterfly with bias */

#define BUTTERFLY_B(t0,t1,W0,W1,d0,d1) do {  \
    t0 = BIAS (MUL_Add(d1, W1, d0, W0)); \
    t1 = BIAS (MUL_Sub(d1, W0, d0, W1)); \
} while (0)

/* the basic split-radix ifft butterfly */

#define BUTTERFLY(a0,a1,a2,a3,wr,wi) do {       \
    BUTTERFLY_0 (tmp5, tmp6, wr, wi, a2.real, a2.imag); \
    BUTTERFLY_0 (tmp8, tmp7, wr, wi, a3.imag, a3.real); \
    tmp1 = tmp5 + tmp7;                 \
    tmp2 = tmp6 + tmp8;                 \
    tmp3 = tmp6 - tmp8;                 \
    tmp4 = tmp7 - tmp5;                 \
    a2.real = a0.real - tmp1;           \
    a2.imag = a0.imag - tmp2;           \
    a3.real = a1.real - tmp3;           \
    a3.imag = a1.imag - tmp4;           \
    a0.real += tmp1;                    \
    a0.imag += tmp2;                    \
    a1.real += tmp3;                    \
    a1.imag += tmp4;                    \
} while (0)

/* split-radix ifft butterfly, specialized for wr=1 wi=0 */

#define BUTTERFLY_ZERO(a0,a1,a2,a3) do {\
    tmp1 = a2.real + a3.real;           \
    tmp2 = a2.imag + a3.imag;           \
    tmp3 = a2.imag - a3.imag;           \
    tmp4 = a3.real - a2.real;           \
    a2.real = a0.real - tmp1;           \
    a2.imag = a0.imag - tmp2;           \
    a3.real = a1.real - tmp3;           \
    a3.imag = a1.imag - tmp4;           \
    a0.real += tmp1;                    \
    a0.imag += tmp2;                    \
    a1.real += tmp3;                    \
    a1.imag += tmp4;                    \
} while (0)

/* split-radix ifft butterfly, specialized for wr=wi */

#define BUTTERFLY_HALF(a0,a1,a2,a3,w) do {  \
    tmp5 = MUL (a2.real + a2.imag, w);      \
    tmp6 = MUL (a2.imag - a2.real, w);      \
    tmp7 = MUL (a3.real - a3.imag, w);      \
    tmp8 = MUL (a3.imag + a3.real, w);      \
    tmp1 = tmp5 + tmp7;                     \
    tmp2 = tmp6 + tmp8;                     \
    tmp3 = tmp6 - tmp8;                     \
    tmp4 = tmp7 - tmp5;                     \
    a2.real = a0.real - tmp1;               \
    a2.imag = a0.imag - tmp2;               \
    a3.real = a1.real - tmp3;               \
    a3.imag = a1.imag - tmp4;               \
    a0.real += tmp1;                        \
    a0.imag += tmp2;                        \
    a1.real += tmp3;                        \
    a1.imag += tmp4;                        \
} while (0)

static __inline void ifft8(complex_t * buf)
{
    int32_t tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;

    ifft4(buf);
    ifft2(buf + 4);
    ifft2(buf + 6);
    BUTTERFLY_ZERO(buf[0], buf[2], buf[4], buf[6]);
    BUTTERFLY_HALF(buf[1], buf[3], buf[5], buf[7], roots16[1]);
}

static void ifft_pass(complex_t * buf, int32_t * weight, int n)
{
    complex_t *buf1;
    complex_t *buf2;
    complex_t *buf3;
    int32_t tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    int i;

    buf++;
    buf1 = buf + n;
    buf2 = buf + 2 * n;
    buf3 = buf + 3 * n;

    BUTTERFLY_ZERO(buf[-1], buf1[-1], buf2[-1], buf3[-1]);

    i = n - 1;

    do {
        BUTTERFLY(buf[0], buf1[0], buf2[0], buf3[0], weight[0], weight[2 * i - n]);
        buf++;
        buf1++;
        buf2++;
        buf3++;
        weight++;
    } while (--i);
}

static void ifft16(complex_t * buf)
{
    ifft8(buf);
    ifft4(buf + 8);
    ifft4(buf + 12);
    ifft_pass(buf, roots16, 4);
}

static void ifft32(complex_t * buf)
{
    ifft16(buf);
    ifft8(buf + 16);
    ifft8(buf + 24);
    ifft_pass(buf, roots32, 8);
}

static void ifft64_c(complex_t * buf)
{
    ifft32(buf);
    ifft16(buf + 32);
    ifft16(buf + 48);
    ifft_pass(buf, roots64, 16);
}

static void ifft128_c(complex_t * buf)
{
    ifft32(buf);
    ifft16(buf + 32);
    ifft16(buf + 48);
    ifft_pass(buf, roots64, 16);

    ifft32(buf + 64);
    ifft32(buf + 96);
    ifft_pass(buf, roots128, 32);
}

void ac3_imdct_512(int32_t * data, int32_t * delay, int32_t * output)
{
    int i, k;
    int32_t t_r, t_i, a_r, a_i, b_r, b_i, w_1, w_2;
    const int32_t *window = ac3_imdct_window;
    complex_t buf[128];

    for (i = 0; i < 128; i++) {
        k = fftorder[i];
        t_r = pre1[i].real>>1;
        t_i = pre1[i].imag>>1;
        BUTTERFLY_0(buf[i].real, buf[i].imag, t_r, t_i, data[k], data[255 - k]);
   }

    ifft128_c(buf);
	//for(i=0;i<128;i++) {
    //    buf[i].real = SATSHIFT(buf[i].real, 1)>>1;
    //    buf[i].imag = SATSHIFT(buf[i].imag, 1)>>1;
	//}

    /* Post IFFT complex multiply plus IFFT complex conjugate */
    /* Window and convert to real valued signal */
    for (i = 0; i < 64; i++) {
        /* y[n] = z[n] * (xcos1[n] + j * xsin1[n]) ; */
        t_r = post1[i].real>>1;
        t_i = post1[i].imag>>1;
        BUTTERFLY_0(a_r, a_i, t_i, t_r, buf[i].imag, buf[i].real);
        BUTTERFLY_0(b_r, b_i, t_r, t_i, buf[127 - i].imag, buf[127 - i].real);
		a_r = SATSHIFT(a_r, 2);
		a_i = SATSHIFT(a_i, 2);
		b_r = SATSHIFT(b_r, 2);
		b_i = SATSHIFT(b_i, 2);

        w_1 = window[2 * i];
        w_2 = window[255 - 2 * i];
        BUTTERFLY_B(output[255 - 2 * i], output[2 * i], w_2, w_1, a_r, delay[2 * i]);
		output[255 - 2 * i] = SATSHIFT(output[255 - 2 * i], 1);
		output[2 * i] = SATSHIFT(output[2 * i], 1);
        delay[2 * i] = a_i;
        w_1 = window[2 * i + 1];
        w_2 = window[254 - 2 * i];
        BUTTERFLY_B(output[2 * i + 1], output[254 - 2 * i], w_1, w_2, b_r, delay[2 * i + 1]);
		output[254 - 2 * i] = SATSHIFT(output[254 - 2 * i], 1);
		output[2 * i + 1] = SATSHIFT(output[2 * i + 1], 1);
        delay[2 * i + 1] = b_i;
    }
}

void ac3_imdct_256(int32_t * data, int32_t * delay, int32_t * output)
{
    int i, k;
    int32_t t_r, t_i, a_r, a_i, b_r, b_i, c_r, c_i, d_r, d_i, w_1, w_2;
    const int32_t *window = ac3_imdct_window;
    complex_t buf1[64], buf2[64];

    /* Pre IFFT complex multiply plus IFFT cmplx conjugate */
    for (i = 0; i < 64; i++) {
        k = fftorder[i];
        t_r = pre2[i].real>>1;
        t_i = pre2[i].imag>>1;		
        BUTTERFLY_0(buf1[i].real, buf1[i].imag, t_r, t_i, data[k], data[254 - k]);
        BUTTERFLY_0(buf2[i].real, buf2[i].imag, t_r, t_i, data[k+1], data[255 - k]);
    }

    ifft64_c(buf1);
    ifft64_c(buf2);
	/*for(i=0;i<64;i++) {
        buf1[i].real = SATSHIFT(buf1[i].real, 1)>>1;
        buf1[i].imag = SATSHIFT(buf1[i].imag, 1)>>1;
	}
	for(i=0;i<64;i++) {
        buf2[i].real = SATSHIFT(buf2[i].real, 1)>>1;
        buf2[i].imag = SATSHIFT(buf2[i].imag, 1)>>1;
	}*/

    /* Post IFFT complex multiply */
    /* Window and convert to real valued signal */
    for (i = 0; i < 32; i++) {
        /* y1[n] = z1[n] * (xcos2[n] + j * xs in2[n]) ; */
        t_r = post2[i].real>>1;
        t_i = post2[i].imag>>1;
        BUTTERFLY_0(a_r, a_i, t_i, t_r, buf1[i].imag, buf1[i].real);
        BUTTERFLY_0(b_r, b_i, t_r, t_i, buf1[63 - i].imag, buf1[63 - i].real);
        BUTTERFLY_0(c_r, c_i, t_i, t_r, buf2[i].imag, buf2[i].real);
        BUTTERFLY_0(d_r, d_i, t_r, t_i, buf2[63 - i].imag, buf2[63 - i].real);
		a_r = SATSHIFT(a_r, 2);
		a_i = SATSHIFT(a_i, 2);
		b_r = SATSHIFT(b_r, 2);
		b_i = SATSHIFT(b_i, 2);
		c_r = SATSHIFT(c_r, 2);
		c_i = SATSHIFT(c_i, 2);
		d_r = SATSHIFT(d_r, 2);
		d_i = SATSHIFT(d_i, 2);

        w_1 = window[2 * i];
        w_2 = window[255 - 2 * i];
        BUTTERFLY_B(output[255 - 2 * i], output[2 * i], w_2, w_1, a_r, delay[2 * i]);
		output[255 - 2 * i] = SATSHIFT(output[255 - 2 * i], 1);
		output[2 * i] = SATSHIFT(output[2 * i], 1);
        delay[2 * i] = c_i;

        w_1 = window[128 + 2 * i];
        w_2 = window[127 - 2 * i];
        BUTTERFLY_B(output[128 + 2 * i], output[127 - 2 * i], w_1, w_2, a_i, delay[127 - 2 * i]);
		output[128 + 2 * i] = SATSHIFT(output[128 + 2 * i], 1);
		output[127 - 2 * i] = SATSHIFT(output[127 - 2 * i], 1);
        delay[127 - 2 * i] = c_r;

        w_1 = window[2 * i + 1];
        w_2 = window[254 - 2 * i];
        BUTTERFLY_B(output[254 - 2 * i], output[2 * i + 1], w_2, w_1, b_i, delay[2 * i + 1]);
		output[254 - 2 * i] = SATSHIFT(output[254 - 2 * i], 1);
		output[2 * i + 1] = SATSHIFT(output[2 * i + 1], 1);
        delay[2 * i + 1] = d_r;

        w_1 = window[129 + 2 * i];
        w_2 = window[126 - 2 * i];
        BUTTERFLY_B(output[129 + 2 * i], output[126 - 2 * i], w_1, w_2, b_r, delay[126 - 2 * i]);
		output[129 + 2 * i] = SATSHIFT(output[129 + 2 * i], 1);
		output[126 - 2 * i] = SATSHIFT(output[126 - 2 * i], 1);
        delay[126 - 2 * i] = d_i;
    }

}

/*****************************************************************
* woad_decode: Performs window/overlap/add decode
*****************************************************************/
void woad_decode(int32_t * data, int32_t * delay)
{
    int i;
    const int32_t *window = ac3_imdct_window;
    int buf[256];

	/* perform window overlap/add */
	for (i = 0; i < 64; i++)
	{
        BUTTERFLY_B(buf[255 - 2 * i], buf[2 * i], window[255-2*i], window[2*i], data[192+i], delay[2 * i]);
        BUTTERFLY_B(buf[254 - 2 * i], buf[2 * i + 1], window[254-2*i], window[2*i+1], data[63-i], delay[2 * i + 1]);
		buf[255 - 2 * i] = SATSHIFT(buf[255 - 2 * i], 1);
		buf[2 * i] = SATSHIFT(buf[2 * i], 1);
		buf[254 - 2 * i] = SATSHIFT(buf[254 - 2 * i], 1);
		buf[2 * i + 1] = SATSHIFT(buf[2 * i + 1], 1);
	}

	/* update delay buffers */
	for (i = 0; i < 64; i++) {
		delay[2*i]= -data[64 + i];
		delay[2*i+1] = data[191 -i];
	}
	
	/* update data buffers */
	for (i = 0; i < 256; i++)
		data[i] = buf[i];
}

void ac3_imdct_init(void)
{
}

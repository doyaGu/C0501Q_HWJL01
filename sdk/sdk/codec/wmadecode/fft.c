/*
 * WMA compatible decoder
 * Copyright (c) 2002 The FFmpeg Project.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"
#include "fft.h"
#include "wmafixed.h"

//static FFTComplex exptab0[512];

/* butter fly op */
#define BF(pre, pim, qre, qim, pre1, pim1, qre1, qim1)  \
{                               \
    fixed32 ax, ay, bx, by;     \
    bx=pre1;                    \
    by=pim1;                    \
    ax=qre1;                    \
    ay=qim1;                    \
    pre = (bx + ax);            \
    pim = (by + ay);            \
    qre = (bx - ax);            \
    qim = (by - ay);            \
}

int fft_calc_unscaled(FFTContext *s, fixed32 *cos_tab, FFTComplex *z)
{
    int ln = s->nbits;
    int j, np, np2;
    int nblocks, nloops;
    register FFTComplex *p, *q;
    int l;
    fixed32 tmp_re, tmp_im;
    int tabshift = 10-ln;

    np = 1 << ln;

    /* pass 0 */

    p=&z[0];
    j=(np >> 1);
    do {
        BF(p[0].re, p[0].im, p[1].re, p[1].im,
           p[0].re, p[0].im, p[1].re, p[1].im);
        p+=2;
    } while (--j != 0);

    /* pass 1 */

    p=&z[0];
    j=np >> 2;
    if (s->inverse) {
        do {
            BF(p[0].re, p[0].im, p[2].re, p[2].im,
               p[0].re, p[0].im, p[2].re, p[2].im);
            BF(p[1].re, p[1].im, p[3].re, p[3].im,
               p[1].re, p[1].im, -p[3].im, p[3].re);
            p+=4;
        } while (--j != 0);
    } else {
        do {
            BF(p[0].re, p[0].im, p[2].re, p[2].im,
               p[0].re, p[0].im, p[2].re, p[2].im);
            BF(p[1].re, p[1].im, p[3].re, p[3].im,
               p[1].re, p[1].im, p[3].im, -p[3].re);
            p+=4;
        } while (--j != 0);
    }
    /* pass 2 .. ln-1 */

    nblocks = np >> 3;
    nloops = 1 << 2;
    np2 = np >> 1;
    do {
        p = z;
        q = z + nloops;
        for (j = 0; j < nblocks; ++j) {
            BF(p->re, p->im, q->re, q->im,
               p->re, p->im, q->re, q->im);

            p++;
            q++;
            for(l = nblocks; l < np2; l += nblocks) {
                //CMUL(&tmp_re, &tmp_im, exptab0[(l<<tabshift)].re, exptab0[(l<<tabshift)].im, q->re, q->im);
                int tmp = l<<tabshift;
                if( tmp < 256 )     // 0~1/2pi
                    CMUL(&tmp_re, &tmp_im, cos_tab[tmp<<3], cos_tab[2047-(tmp<<3)], q->re, q->im);
                else                // 1/2pi ~ pi
                {
                    tmp -= 256;
                    CMUL(&tmp_re, &tmp_im, -cos_tab[2047-(tmp<<3)], cos_tab[tmp<<3], q->re, q->im);
                }
                BF(p->re, p->im, q->re, q->im,
                   p->re, p->im, tmp_re, tmp_im);
                p++;
                q++;
            }

            p += nloops;
            q += nloops;
        }
        nblocks = nblocks >> 1;
        nloops = nloops << 1;
    } while (nblocks != 0);

    return 0;
}

int fft_init_global(void)
{
/*
    int i, n;
    fixed32 c1, s1, s2;

    n=1<<10;
    s2 = 1 ? 1 : -1;
    for(i=0;i<(n/2);++i) {
        fixed32 ifix = itofix32(i);
        fixed32 nfix = itofix32(n);
        fixed32 res = fixdiv32(ifix,nfix);

        s1 = fsincos(res<<16, &c1);  // build 0~pi table

        exptab0[i].re = c1;
        exptab0[i].im = s1*s2;
    }
*/
    return 0;
}


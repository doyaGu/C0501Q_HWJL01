/*
 * libmad - MPEG audio decoder library
 * Copyright (C) 2000-2004 Underbit Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: layer12.c,v 1.17 2004/02/05 09:02:39 rob Exp $
 */
# include "mp3_config.h"
# include "bit.h"
# include "layer12.h"

typedef   signed int mad_fixed_t;

# undef CHAR_BIT
# define CHAR_BIT  8
# define MAD_F_FRACBITS     28
# define MAD_F(x)       ((mad_fixed_t) (x##L))
# define MAD_F_ONE	MAD_F(0x10000000)
//# define mad_f_mul(x, y)  (((x) >> 12) * ((y) >> 16))
//# define mad_f_mul(x, y)    MULSHIFT(x, y, 28)

/*
 * scalefactor table
 * used in both Layer I and Layer II decoding
 */
static
mad_fixed_t const sf_table[64] = {
# include "sf_table.dat"
};

#if defined(LAYERII_DEBUG)
    extern FILE *dbgLayerII;
#endif

#if 1

/* --- Layer I ------------------------------------------------------------- */

/* linear scaling table */
static
mad_fixed_t const linear_table[14] = {
  MAD_F(0x15555555),  /* 2^2  / (2^2  - 1) == 1.33333333333333 */
  MAD_F(0x12492492),  /* 2^3  / (2^3  - 1) == 1.14285714285714 */
  MAD_F(0x11111111),  /* 2^4  / (2^4  - 1) == 1.06666666666667 */
  MAD_F(0x10842108),  /* 2^5  / (2^5  - 1) == 1.03225806451613 */
  MAD_F(0x10410410),  /* 2^6  / (2^6  - 1) == 1.01587301587302 */
  MAD_F(0x10204081),  /* 2^7  / (2^7  - 1) == 1.00787401574803 */
  MAD_F(0x10101010),  /* 2^8  / (2^8  - 1) == 1.00392156862745 */
  MAD_F(0x10080402),  /* 2^9  / (2^9  - 1) == 1.00195694716243 */
  MAD_F(0x10040100),  /* 2^10 / (2^10 - 1) == 1.00097751710655 */
  MAD_F(0x10020040),  /* 2^11 / (2^11 - 1) == 1.00048851978505 */
  MAD_F(0x10010010),  /* 2^12 / (2^12 - 1) == 1.00024420024420 */
  MAD_F(0x10008004),  /* 2^13 / (2^13 - 1) == 1.00012208521548 */
  MAD_F(0x10004001),  /* 2^14 / (2^14 - 1) == 1.00006103888177 */
  MAD_F(0x10002000)   /* 2^15 / (2^15 - 1) == 1.00003051850948 */
};

/*
 * NAME:    I_sample()
 * DESCRIPTION: decode one requantized Layer I sample from a bitstream
 */
static
mad_fixed_t I_sample(struct mad_bitptr *ptr, unsigned int nb)
{
  mad_fixed_t sample;

  sample = mad_bit_read(ptr, nb);

  /* invert most significant bit, extend sign, then scale to fixed format */

  sample ^= 1 << (nb - 1);
  sample |= -(sample & (1 << (nb - 1)));

  sample <<= MAD_F_FRACBITS - (nb - 1);

  /* requantize the sample */

  /* s'' = (2^nb / (2^nb - 1)) * (s''' + 2^(-nb + 1)) */

  sample += MAD_F_ONE >> (nb - 1);

  return MULSHIFT(sample, linear_table[nb - 2], 28);

  /* s' = factor * s'' */
  /* (to be performed by caller) */
}

/*
 * NAME:    layer->I()
 * DESCRIPTION: decode a single Layer I frame
 */
int mad_layer_I(unsigned char *buf, int *errcode)
{
  unsigned int nch, bound, ch, s, sb, nb;
  unsigned char allocation[2][32], scalefactor[2][32];
    unsigned int usedbytes;

    struct mad_bitptr start, bitptr;
    IMDCTInfo *mi;
    mi = &(mp3DecInfo.IMDCTInfoPS);

    mad_bit_init(&bitptr, buf);
    mad_bit_init(&start, buf);
    nch = mp3DecInfo.nChans;

  bound = 32;
    if (mp3DecInfo.FrameHeaderPS.sMode == Joint) 
	{
  /* check CRC word */
        bound = 4 + mp3DecInfo.FrameHeaderPS.modeExt * 4;
  }

  /* decode bit allocations */

	for (sb = 0; sb < bound; ++sb) 
	{
		for (ch = 0; ch < nch; ++ch) 
		{
			nb = (unsigned char)mad_bit_read(&bitptr, 4);

			if (nb == 15) 
			{
				*errcode = ERR_MP3_INVALID_FRAMEHEADER;
    return -1;
      }
      allocation[ch][sb] = nb ? nb + 1 : 0;
    }
  }

	for (sb = bound; sb < 32; ++sb) 
	{
		nb = (unsigned char)mad_bit_read(&bitptr, 4);
		if (nb == 15) 
		{
			*errcode = ERR_MP3_INVALID_FRAMEHEADER;
      return -1;
    }

    allocation[0][sb] =
    allocation[1][sb] = nb ? nb + 1 : 0;
  }

  /* decode scalefactors */

	for (sb = 0; sb < 32; ++sb) 
	{
		for (ch = 0; ch < nch; ++ch) 
		{
			if (allocation[ch][sb]) 
			{
				 scalefactor[ch][sb] = (unsigned char)mad_bit_read(&bitptr, 6);

# if defined(OPT_STRICT)
    /*
     * Scalefactor index 63 does not appear in Table B.1 of
     * ISO/IEC 11172-3. Nonetheless, other implementations accept it,
     * so we only reject it if OPT_STRICT is defined.
     */
				if (scalefactor[ch][sb] == 63) 
				{
					*errcode = MAD_ERROR_BADSCALEFACTOR;
      return -1;
    }
# endif
      }
    }
  }

  /* decode samples */

	for (s = 0; s < 12; ++s) 
	{
		for (sb = 0; sb < bound; ++sb) 
		{
			for (ch = 0; ch < nch; ++ch) 
			{
    nb = allocation[ch][sb];
				mi->outBuf[0][ch][s][sb] = nb ? MULSHIFT32(I_sample(&bitptr, nb),sf_table[scalefactor[ch][sb]]) : 0;				
      }
    }

		for (sb = bound; sb < 32; ++sb) 
		{
			if ((nb = allocation[0][sb])) 
			{
    mad_fixed_t sample;
				sample = I_sample(&bitptr, nb);
				for (ch = 0; ch < nch; ++ch) 
				{
					mi->outBuf[0][ch][s][sb] = MULSHIFT32(sample, sf_table[scalefactor[ch][sb]]);				
    }
      }
			else 
			{
    for (ch = 0; ch < nch; ++ch)
					mi->outBuf[0][ch][s][sb] = 0;
      }
    }
  }

    *errcode = ERR_MP3_NONE;

    usedbytes = mad_bit_length(&start, &bitptr);
    if (usedbytes & 0x7) {
        usedbytes = (usedbytes >> 3) + 1;
    }
    else {
        usedbytes = (usedbytes >> 3);
    }

    return usedbytes;

}

#endif  // 0

/* --- Layer II ------------------------------------------------------------ */

/* possible quantization per subband table */
static
struct {
  unsigned int sblimit;
  unsigned char const offsets[30];
} const sbquant_table[5] = {
  /* ISO/IEC 11172-3 Table B.2a */
  { 27, { 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 3, 3, 3, 3, 3,   /* 0 */
      3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0 } },
  /* ISO/IEC 11172-3 Table B.2b */
  { 30, { 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 3, 3, 3, 3, 3,   /* 1 */
      3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0 } },
  /* ISO/IEC 11172-3 Table B.2c */
  {  8, { 5, 5, 2, 2, 2, 2, 2, 2 } },               /* 2 */
  /* ISO/IEC 11172-3 Table B.2d */
  { 12, { 5, 5, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 } },       /* 3 */
  /* ISO/IEC 13818-3 Table B.1 */
  { 30, { 4, 4, 4, 4, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1,   /* 4 */
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } }
};

/* bit allocation table */
static
struct {
  unsigned short nbal;
  unsigned short offset;
} const bitalloc_table[8] = {
  { 2, 0 },  /* 0 */
  { 2, 3 },  /* 1 */
  { 3, 3 },  /* 2 */
  { 3, 1 },  /* 3 */
  { 4, 2 },  /* 4 */
  { 4, 3 },  /* 5 */
  { 4, 4 },  /* 6 */
  { 4, 5 }   /* 7 */
};

/* offsets into quantization class table */
static
unsigned char const offset_table[6][15] = {
  { 0, 1, 16                                             },  /* 0 */
  { 0, 1,  2, 3, 4, 5, 16                                },  /* 1 */
  { 0, 1,  2, 3, 4, 5,  6, 7,  8,  9, 10, 11, 12, 13, 14 },  /* 2 */
  { 0, 1,  3, 4, 5, 6,  7, 8,  9, 10, 11, 12, 13, 14, 15 },  /* 3 */
  { 0, 1,  2, 3, 4, 5,  6, 7,  8,  9, 10, 11, 12, 13, 16 },  /* 4 */
  { 0, 2,  4, 5, 6, 7,  8, 9, 10, 11, 12, 13, 14, 15, 16 }   /* 5 */
};

/* quantization class table */
static
struct quantclass {
  unsigned short nlevels;
  unsigned char group;
  unsigned char bits;
  mad_fixed_t C;
  mad_fixed_t D;
} const qc_table[17] = {
# include "qc_table.dat"
};

/*
 * NAME:    II_samples()
 * DESCRIPTION: decode three requantized Layer II samples from a bitstream
 */
static
void II_samples(struct mad_bitptr *ptr,
        struct quantclass const *quantclass,
        mad_fixed_t output[3])
{
  unsigned int nb, s, sample[3];

  if ((nb = quantclass->group)) {
    unsigned int c, nlevels;

    /* degrouping */
    c = mad_bit_read(ptr, quantclass->bits);
    nlevels = quantclass->nlevels;

    for (s = 0; s < 3; ++s) {
      sample[s] = c % nlevels;
      c /= nlevels;
    }
  }
  else {
    nb = quantclass->bits;

    for (s = 0; s < 3; ++s)
      sample[s] = mad_bit_read(ptr, nb);
  }

  for (s = 0; s < 3; ++s) {
    mad_fixed_t requantized;

    /* invert most significant bit, extend sign, then scale to fixed format */

    requantized  = sample[s] ^ (1 << (nb - 1));
    requantized |= -(requantized & (1 << (nb - 1)));

    requantized <<= MAD_F_FRACBITS - (nb - 1);

    /* requantize the sample */

    /* s'' = C * (s''' + D) */

    output[s] = MULSHIFT(requantized + quantclass->D, quantclass->C, 28);

    /* s' = factor * s'' */
    /* (to be performed by caller) */
  }
}

/*
 * NAME:    layer->II()
 * DESCRIPTION: decode a single Layer II frame
 */

int mad_layer_II(unsigned char *buf, int *errcode)
{
    unsigned int index, sblimit, nbal, nch, bound, gr, ch, s, sb;
    unsigned char const *offsets;
    unsigned char allocation[2][32], scfsi[2][32], scalefactor[2][32][3];
    unsigned int n, m, usedbytes;
    mad_fixed_t samples[3];

    struct mad_bitptr start, bitptr;
    IMDCTInfo *mi;
    mi = &(mp3DecInfo.IMDCTInfoPS);

    mad_bit_init(&bitptr, buf);
    mad_bit_init(&start, buf);
    nch = mp3DecInfo.nChans;

    if (mp3DecInfo.FrameHeaderPS.ver == MPEG2)
        index = 4;
    else if (mp3DecInfo.bitrate == 0)
        goto freeformat;
    else {
        unsigned long bitrate_per_channel;

        bitrate_per_channel = mp3DecInfo.bitrate;
        if (nch == 2) {
            bitrate_per_channel /= 2;
            # if defined(OPT_STRICT)
            /*
             * ISO/IEC 11172-3 allows only single channel mode for 32, 48, 56, and
             * 80 kbps bitrates in Layer II, but some encoders ignore this
             * restriction. We enforce it if OPT_STRICT is defined.
             */
            if (bitrate_per_channel <= 28000 || bitrate_per_channel == 40000) {
                *errcode = ERR_MP3_INVALID_FRAMEHEADER;
                goto end;
            }
            # endif
        }
        else {  /* nch == 1 */
            if (bitrate_per_channel > 192000) {
                /*
                 * ISO/IEC 11172-3 does not allow single channel mode for 224, 256,
                 * 320, or 384 kbps bitrates in Layer II.
                 */
                *errcode = ERR_MP3_INVALID_FRAMEHEADER;
                goto end;
            }
        }

        if (bitrate_per_channel <= 48000)
            index = (mp3DecInfo.samprate == 32000) ? 3 : 2;
        else if (bitrate_per_channel <= 80000)
            index = 0;
        else {
freeformat:
            index = (mp3DecInfo.samprate == 48000) ? 0 : 1;
        }
    }
    sblimit = sbquant_table[index].sblimit;
    offsets = sbquant_table[index].offsets;

    bound = 32;
    if (mp3DecInfo.FrameHeaderPS.sMode == Joint) {
//        header->flags |= MAD_FLAG_I_STEREO;
        bound = 4 + mp3DecInfo.FrameHeaderPS.modeExt * 4;
    }
    if (bound > sblimit)
        bound = sblimit;

    /* decode bit allocations */

    for (sb = 0; sb < bound; ++sb) {
        nbal = bitalloc_table[offsets[sb]].nbal;

        for (ch = 0; ch < nch; ++ch)
            allocation[ch][sb] = (unsigned char)mad_bit_read(&bitptr, nbal);
    }

    for (sb = bound; sb < sblimit; ++sb) {
        nbal = bitalloc_table[offsets[sb]].nbal;

        allocation[0][sb] =
        allocation[1][sb] = (unsigned char)mad_bit_read(&bitptr, nbal);
    }

    /* decode scalefactor selection info */

    for (sb = 0; sb < sblimit; ++sb) {
        for (ch = 0; ch < nch; ++ch) {
            if (allocation[ch][sb])
                scfsi[ch][sb] = (unsigned char)mad_bit_read(&bitptr, 2);
        }
    }

    /* decode scalefactors */

    for (sb = 0; sb < sblimit; ++sb) {
        for (ch = 0; ch < nch; ++ch) {
            if (allocation[ch][sb]) {
                scalefactor[ch][sb][0] = (unsigned char)mad_bit_read(&bitptr, 6);

                switch (scfsi[ch][sb]) {
                case 2:
                    scalefactor[ch][sb][2] =
                    scalefactor[ch][sb][1] =
                    scalefactor[ch][sb][0];
                     break;

                case 0:
                    scalefactor[ch][sb][1] = (unsigned char)mad_bit_read(&bitptr, 6);
                    /* fall through */

                case 1:
                case 3:
                    scalefactor[ch][sb][2] = (unsigned char)mad_bit_read(&bitptr, 6);
                }

                if (scfsi[ch][sb] & 1)
                    scalefactor[ch][sb][1] = scalefactor[ch][sb][scfsi[ch][sb] - 1];

                # if defined(OPT_STRICT)
                /*
                 * Scalefactor index 63 does not appear in Table B.1 of
                 * ISO/IEC 11172-3. Nonetheless, other implementations accept it,
                 * so we only reject it if OPT_STRICT is defined.
                 */
                if (scalefactor[ch][sb][0] == 63 ||
                    scalefactor[ch][sb][1] == 63 ||
                    scalefactor[ch][sb][2] == 63) {
                    *errcode = ERR_MP3_INVALID_SCALEFACT;
                    goto end;
                }
                # endif
            }
        }
    }

    /* decode samples */

    for (gr = 0; gr < 12; ++gr) {
        n = (gr < 6) ? 0 : 1;
        m = 3 * gr;
        if (n) {
            m = m - 18;
        }
        for (sb = 0; sb < bound; ++sb) {
            for (ch = 0; ch < nch; ++ch) {
                if ((index = allocation[ch][sb])) {
                    index = offset_table[bitalloc_table[offsets[sb]].offset][index - 1];

                    II_samples(&bitptr, &qc_table[index], samples);

                    for (s = 0; s < 3; ++s) {
//                      frame->sbsample[ch][3 * gr + s][sb] =
                        mi->outBuf[n][ch][m + s][sb] =
                            //mad_f_mul(samples[s], sf_table[scalefactor[ch][sb][gr / 4]]) >> 4;
                            MULSHIFT32(samples[s], sf_table[scalefactor[ch][sb][gr>>2]]);
                    }
                }
                else {
                    for (s = 0; s < 3; ++s) {
//                      frame->sbsample[ch][3 * gr + s][sb] = 0;
                        mi->outBuf[n][ch][m + s][sb] = 0;
                    }
                }
            }
        }

        for (sb = bound; sb < sblimit; ++sb) {
            if ((index = allocation[0][sb])) {
                index = offset_table[bitalloc_table[offsets[sb]].offset][index - 1];

                II_samples(&bitptr, &qc_table[index], samples);

                for (ch = 0; ch < nch; ++ch) {
                    for (s = 0; s < 3; ++s) {
                        mi->outBuf[n][ch][m + s][sb] =
                            MULSHIFT32(samples[s], sf_table[scalefactor[ch][sb][gr>>2]]);
                    }
                }
            }
            else {
                for (ch = 0; ch < nch; ++ch) {
                    for (s = 0; s < 3; ++s) {
//                      frame->sbsample[ch][3 * gr + s][sb] = 0;
                        mi->outBuf[n][ch][m + s][sb] = 0;
                    }
                }
            }
        }

        for (ch = 0; ch < nch; ++ch) {
            for (s = 0; s < 3; ++s) {
                for (sb = sblimit; sb < 32; ++sb) {
//                  frame->sbsample[ch][3 * gr + s][sb] = 0;
                    mi->outBuf[n][ch][m + s][sb] = 0;
                }
            }
        }
    }

    *errcode = ERR_MP3_NONE;

#if defined(LAYERII_DEBUG)
    {
    static int framecnt = 0;
    fprintf(dbgLayerII, "[frame = %4d]\n", framecnt++);
    fprintf(dbgLayerII, "allocation[2][32]=\n");
    for (ch = 0; ch < 2; ++ch) {
        for (sb = 0; sb < 32; ++sb) {
            fprintf(dbgLayerII, "%d, ", allocation[ch][sb]);
        }
        fprintf(dbgLayerII, "\n");
    }
    fprintf(dbgLayerII, "\n");
    fprintf(dbgLayerII, "scfsi[2][32]=\n");
    for (ch = 0; ch < 2; ++ch) {
        for (sb = 0; sb < 32; ++sb) {
            fprintf(dbgLayerII, "%d, ", scfsi[ch][sb]);
        }
        fprintf(dbgLayerII, "\n");
    }
    fprintf(dbgLayerII, "\n");
    fprintf(dbgLayerII, "scalefactor[2][32][3]=\n");
    for (ch = 0; ch < 2; ++ch) {
        for (sb = 0; sb < 32; ++sb) {
            for (s = 0; s < 3; ++s) {
                fprintf(dbgLayerII, "%d, ", scalefactor[ch][sb][s]);
            }
            fprintf(dbgLayerII, "\n");
        }
        fprintf(dbgLayerII, "\n");
    }
    fprintf(dbgLayerII, "\n");
    fprintf(dbgLayerII, "outBuf[2][2][18][32]=\n");
    for (ch = 0; ch < 2; ++ch) {
        for (n = 0; n < 2; ++n) {
            for (s = 0; s < 18; ++s) {
                for (sb = 0; sb < 32; ++sb) {
                    fprintf(dbgLayerII, "%d, ", mi->outBuf[ch][n][s][sb]);
                }
                fprintf(dbgLayerII, "\n");
            }
            fprintf(dbgLayerII, "\n");
        }
        fprintf(dbgLayerII, "\n");
    }
    fprintf(dbgLayerII, "\n");

    }
#endif

end:
    usedbytes = mad_bit_length(&start, &bitptr);
    if (usedbytes & 0x7) {
        usedbytes = (usedbytes >> 3) + 1;
    }
    else {
        usedbytes = (usedbytes >> 3);
    }

    return usedbytes;
}

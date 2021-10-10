/*
 * a52_internal.h
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

#ifndef __A52_INTERNAL_H__
#define __A52_INTERNAL_H__

enum {
    ERR_AC3_OK                  = 0,
    ERR_AC3_INDATA_UNDERFLOW    = -1,
    ERR_AC3_NULL_POINTER        = -2,

    ERR_AC3_INVALID_FRAME       = -3,
    ERR_AC3_INVALID_BSID        = -4,
    ERR_AC3_INVALID_FRAMCODE    = -5,
    ERR_AC3_INVALID_SAMPLERATE  = -6,
    ERR_AC3_INVALID_EXPONENT    = -7,

    ERR_AC3_DOWNMIX_INIT        = -8,
    ERR_AC3_PARSE_DELTBA        = -9,
    ERR_AC3_BLOCK_DECODE        = -10,

    ERR_AC3_UNKNOWN             = -9999
};

typedef struct {
    uint8_t bai;                /* fine SNR offset, fast gain */
    uint8_t deltbae;            /* delta bit allocation exists */
    int8_t deltba[50];          /* per-band delta bit allocation */
} ba_t;

typedef struct {
    uint8_t exp[256];           /* decoded channel exponents */
    int8_t bap[256];            /* derived channel bit allocation */
} expbap_t;

struct a52_state_s {
    int     sample_rate;        /* sample rate */
    int     bit_rate;           /* bit rate */
    uint8_t fscod;              /* sample rate code */
    uint8_t halfrate;           /* halfrate factor */
    uint8_t acmod;              /* coded channels */
    uint8_t lfeon;              /* coded lfe channel */
    level_t clev;               /* centre channel mix level */
    level_t slev;               /* surround channels mix level */

    int output;                 /* type of output */
    level_t level;              /* output level */
    sample_t bias;              /* output bias */

    int dynrnge;                /* apply dynamic range */
    level_t dynrng;             /* dynamic range */
    void *dynrngdata;           /* dynamic range callback funtion and data */
    level_t(*dynrngcall) (level_t range, void *dynrngdata);

    uint8_t chincpl;            /* channel coupled */
    uint8_t phsflginu;          /* phase flags in use (stereo only) */
    uint8_t cplstrtmant;        /* coupling channel start mantissa */
    uint8_t cplendmant;         /* coupling channel end mantissa */
    uint32_t cplbndstrc;        /* coupling band structure */
    level_t cplco[5][18];       /* coupling coordinates */

    /* derived information */
    uint8_t cplstrtbnd;         /* coupling start band (for bit allocation) */
    uint8_t ncplbnd;            /* number of coupling bands */

    uint8_t rematflg;           /* stereo rematrixing */

    uint8_t endmant[5];         /* channel end mantissa */

    uint16_t bai;               /* bit allocation information */

    uint32_t *buffer_start;
    uint16_t lfsr_state;        /* dither state */
    uint32_t bits_left;
    uint32_t current_word;

    uint8_t csnroffst;          /* coarse SNR offset */
    ba_t cplba;                 /* coupling bit allocation parameters */
    ba_t ba[5];                 /* channel bit allocation parameters */
    ba_t lfeba;                 /* lfe bit allocation parameters */

    uint8_t cplfleak;           /* coupling fast leak init */
    uint8_t cplsleak;           /* coupling slow leak init */

    expbap_t cpl_expbap;
    expbap_t fbw_expbap[5];
    expbap_t lfe_expbap;

    sample_t *samples;
    int downmixed;
};

#define LEVEL_PLUS6DB   2.0
#define LEVEL_PLUS3DB   1.4142135623730951
#define LEVEL_3DB       0.7071067811865476
#define LEVEL_45DB      0.5946035575013605
#define LEVEL_6DB       0.5

#define EXP_REUSE (0)
#define EXP_D15   (1)
#define EXP_D25   (2)
#define EXP_D45   (3)

#define DELTA_BIT_REUSE (0)
#define DELTA_BIT_NEW (1)
#define DELTA_BIT_NONE (2)
#define DELTA_BIT_RESERVED (3)

#ifdef LIBA52_FIXED
#  define CONVERT_LEVEL         (1 << 26)
#  define CONVERT_BIAS          0
#else
#  define CONVERT_LEVEL         1
#  define CONVERT_BIAS          384
#endif
void a52_bit_allocate(a52_state_t * state, ba_t * ba, int bndstart,
                      int start, int end, int fastleak, int slowleak, expbap_t * expbap);

int a52_downmix_init(int input, int flags, level_t * level, level_t clev, level_t slev);
int a52_downmix_coeff(level_t * coeff, int acmod, int output, level_t level, level_t clev, level_t slev);
void a52_downmix(sample_t * samples, int acmod, int output, sample_t bias, level_t clev, level_t slev);
void a52_upmix(sample_t * samples, int acmod, int output);

void a52_imdct_init(void);
void a52_imdct_256(sample_t * data, sample_t * delay, sample_t bias);
void a52_imdct_512(sample_t * data, sample_t * delay, sample_t bias);

#define ROUND(x) ((int)((x) + ((x) > 0 ? 0.5 : -0.5)))

#ifndef LIBA52_FIXED

typedef sample_t quantizer_t;
#  define SAMPLE(x) (x)
#  define LEVEL(x) (x)
#  define MUL(a,b) ((a) * (b))
#  define MUL_L(a,b) ((a) * (b))
#  define MUL_C(a,b) ((a) * (b))
#  define DIV(a,b) ((a) / (b))
#  define BIAS(x) ((x) + bias)

#else /* LIBA52_FIXED */

typedef int16_t quantizer_t;
#  define SAMPLE(x) (sample_t)((x) * (1 << 30))
#  define LEVEL(x) (level_t)((x) * (1 << 26))

#  if defined(__OR32__)
static __inline int32_t MUL(int32_t a, int32_t b) {
    int result;
    asm volatile("l.mac %0, %1" : : "%r"(a), "r"(b));
    asm volatile("l.macrc %0, 30" : "=r"(result) );
    return result;
}
static __inline int32_t MUL_L(int32_t a, int32_t b) {
    int result;
    asm volatile("l.mac %0, %1" : : "%r"(a), "r"(b));
    asm volatile("l.macrc %0, 26" : "=r"(result) );
    return result;
}
#  else
#    if 0
#      define MUL(a,b) ((int)(((int64_t)(a) * (b) + (1 << 29)) >> 30))
#      define MUL_L(a,b) ((int)(((int64_t)(a) * (b) + (1 << 25)) >> 26))
#    elif 0 // Default Setting
static __inline int32_t MUL(int32_t a, int32_t b) {
    int32_t _ta=(a), _tb=(b), _tc;
    _tc=(_ta & 0xffff)*(_tb >> 16)+(_ta >> 16)*(_tb & 0xffff);
    return (int32_t)(((_tc >> 14))+(((_ta >> 16)*(_tb >> 16)) << 2 ));
}
static __inline int32_t MUL_L(int32_t a, int32_t b) {
    int32_t _ta=(a), _tb=(b), _tc;
    _tc=(_ta & 0xffff)*(_tb >> 16)+(_ta >> 16)*(_tb & 0xffff);
    return (int32_t)((_tc >> 10)+(((_ta >> 16)*(_tb >> 16)) << 6));
}
#    elif 0
#      define MUL(a,b) (((a) >> 15) * ((b) >> 15))
#      define MUL_L(a,b) (((a) >> 13) * ((b) >> 13))
#    else // Used to compare data with RISC/l.macrc operation.
#      define MUL(a,b) ((int)(((int64_t)(a) * (b)) >> 30))
#      define MUL_L(a,b) ((int)(((int64_t)(a) * (b)) >> 26))
#    endif
#  endif // __OR32__

#  define MUL_C(a,b) MUL_L (a, LEVEL (b))
#  define BIAS(x) (x)

#  if defined(__USE_INT64_LIB__)
#    define DIV(a,b) __div64_32((((uint64_t)LEVEL (a)) * (uint64_t)(1 << 26)), (uint32_t)(b))
#  else
#    define DIV(a,b) ((((int64_t)LEVEL (a)) * (int64_t)(1 << 26)) / (b))
#  endif

#endif /* LIBA52_FIXED */

#endif /* __A52_INTERNAL_H__ */


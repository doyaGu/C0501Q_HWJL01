/*
 * copyright (c) 2004 Michael Niedermayer <michaelni@gmx.at>
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
 * bitstream reader API header.
 */

#ifndef AVCODEC_GET_BITS_H
#define AVCODEC_GET_BITS_H

#include <stdlib.h>
#include <assert.h>
#include "config.h"

//#include "common.h"
//#include "math.h"
#if 0
typedef signed char             int8_t;
typedef short                   int16_t;
typedef int                    int32_t;
typedef unsigned char           uint8_t;
typedef unsigned short          uint16_t;
typedef unsigned int           uint32_t;

#  if defined(WIN32)
typedef __int64                 int64_t;
typedef unsigned __int64        uint64_t;
#  else
typedef long long               int64_t;
typedef unsigned long long      uint64_t;
#  endif
#endif
#if defined(ALT_BITSTREAM_READER_LE) && !defined(ALT_BITSTREAM_READER)
#   define ALT_BITSTREAM_READER
#endif

#if !defined(A32_BITSTREAM_READER) && !defined(ALT_BITSTREAM_READER)
#   if ARCH_ARM && !HAVE_FAST_UNALIGNED
#       define A32_BITSTREAM_READER
#   else
#       define ALT_BITSTREAM_READER
//#define A32_BITSTREAM_READER
#   endif
#endif

/* bit input */
/* buffer, buffer_end and size_in_bits must be present and used by every reader */
typedef struct GetBitContext {
    const uint8_t *buffer, *buffer_end;
#ifdef ALT_BITSTREAM_READER
    int index;
#elif defined A32_BITSTREAM_READER
    uint32_t *buffer_ptr;
    uint32_t cache0;
    uint32_t cache1;
    int bit_count;
#endif
    int size_in_bits;
} GetBitContext;

#ifndef NEG_USR32
#define NEG_USR32(a,s) (((uint32_t)(a))>>(32-(s)))
#endif

#ifndef NEG_SSR32
#   define NEG_SSR32(a,s) ((( int32_t)(a))>>(32-(s)))
#endif

#ifdef ALT_BITSTREAM_READER
#   define MIN_CACHE_BITS 25

#   define OPEN_READER(name, gb)                \
    unsigned int name##_index = (gb)->index;    \
    int name##_cache          = 0

#   define CLOSE_READER(name, gb) (gb)->index = name##_index

# ifdef ALT_BITSTREAM_READER_LE
#   define AV_RL32(x)                           \
    ((((const uint8_t*)(x))[3] << 24) |         \
     (((const uint8_t*)(x))[2] << 16) |         \
     (((const uint8_t*)(x))[1] <<  8) |         \
      ((const uint8_t*)(x))[0])
#   define UPDATE_CACHE(name, gb) \
    name##_cache = AV_RL32(((const uint8_t *)(gb)->buffer)+(name##_index>>3)) >> (name##_index&0x07)

#   define SKIP_CACHE(name, gb, num) name##_cache >>= (num)
# else
#   define AV_RB32(x)                           \
    ((((const uint8_t*)(x))[0] << 24) |         \
     (((const uint8_t*)(x))[1] << 16) |         \
     (((const uint8_t*)(x))[2] <<  8) |         \
      ((const uint8_t*)(x))[3])
#   define UPDATE_CACHE(name, gb) \
    name##_cache = AV_RB32(((const uint8_t *)(gb)->buffer)+(name##_index>>3)) << (name##_index&0x07)

#   define SKIP_CACHE(name, gb, num) name##_cache <<= (num)
# endif

// FIXME name?
#   define SKIP_COUNTER(name, gb, num) name##_index += (num)

#   define SKIP_BITS(name, gb, num) do {        \
        SKIP_CACHE(name, gb, num);              \
        SKIP_COUNTER(name, gb, num);            \
    } while (0)

#   define LAST_SKIP_BITS(name, gb, num) SKIP_COUNTER(name, gb, num)
#   define LAST_SKIP_CACHE(name, gb, num)

# ifdef ALT_BITSTREAM_READER_LE
#   define SHOW_UBITS(name, gb, num) zero_extend(name##_cache, num)

#   define SHOW_SBITS(name, gb, num) sign_extend(name##_cache, num)
# else
#   define SHOW_UBITS(name, gb, num) NEG_USR32(name##_cache, num)

#   define SHOW_SBITS(name, gb, num) NEG_SSR32(name##_cache, num)
# endif

#   define GET_CACHE(name, gb) ((uint32_t)name##_cache)

static __inline int get_bits_count(const GetBitContext *s){
    return s->index;
}

static __inline void skip_bits_long(GetBitContext *s, int n){
    s->index += n;
}

#elif defined A32_BITSTREAM_READER

#   define MIN_CACHE_BITS 32

#   define OPEN_READER(name, gb)                        \
    int name##_bit_count        = (gb)->bit_count;      \
    uint32_t name##_cache0      = (gb)->cache0;         \
    uint32_t name##_cache1      = (gb)->cache1;         \
    uint32_t *name##_buffer_ptr = (gb)->buffer_ptr

#   define CLOSE_READER(name, gb) do {          \
        (gb)->bit_count  = name##_bit_count;    \
        (gb)->cache0     = name##_cache0;       \
        (gb)->cache1     = name##_cache1;       \
        (gb)->buffer_ptr = name##_buffer_ptr;   \
    } while (0)

#   define UPDATE_CACHE(name, gb) do {                                  \
        if(name##_bit_count > 0){                                       \
            const uint32_t next = av_be2ne32(*name##_buffer_ptr);       \
            name##_cache0 |= NEG_USR32(next, name##_bit_count);         \
            name##_cache1 |= next << name##_bit_count;                  \
            name##_buffer_ptr++;                                        \
            name##_bit_count -= 32;                                     \
        }                                                               \
    } while (0)

#if ARCH_X86
#   define SKIP_CACHE(name, gb, num)                            \
    __asm__("shldl %2, %1, %0          \n\t"                    \
            "shll  %2, %1              \n\t"                    \
            : "+r" (name##_cache0), "+r" (name##_cache1)        \
            : "Ic" ((uint8_t)(num)))
#else
#   define SKIP_CACHE(name, gb, num) do {               \
        name##_cache0 <<= (num);                        \
        name##_cache0 |= NEG_USR32(name##_cache1,num);  \
        name##_cache1 <<= (num);                        \
    } while (0)
#endif

#   define SKIP_COUNTER(name, gb, num) name##_bit_count += (num)

#   define SKIP_BITS(name, gb, num) do {        \
        SKIP_CACHE(name, gb, num);              \
        SKIP_COUNTER(name, gb, num);            \
    } while (0)

#   define LAST_SKIP_BITS(name, gb, num)  SKIP_BITS(name, gb, num)
#   define LAST_SKIP_CACHE(name, gb, num) SKIP_CACHE(name, gb, num)

#   define SHOW_UBITS(name, gb, num) NEG_USR32(name##_cache0, num)

#   define SHOW_SBITS(name, gb, num) NEG_SSR32(name##_cache0, num)

#   define GET_CACHE(name, gb) name##_cache0

static __inline int get_bits_count(const GetBitContext *s) {
    return ((uint8_t*)s->buffer_ptr - s->buffer)*8 - 32 + s->bit_count;
}

static __inline void skip_bits_long(GetBitContext *s, int n){
    OPEN_READER(re, s);
    re_bit_count += n;
    re_buffer_ptr += re_bit_count>>5;
    re_bit_count &= 31;
    re_cache0 = av_be2ne32(re_buffer_ptr[-1]) << re_bit_count;
    re_cache1 = 0;
    UPDATE_CACHE(re, s);
    CLOSE_READER(re, s);
}

#endif

static __inline int get_sbits(GetBitContext *s, int n){
    register int tmp;
    OPEN_READER(re, s);
    UPDATE_CACHE(re, s);
    tmp = SHOW_SBITS(re, s, n);
    LAST_SKIP_BITS(re, s, n);
    CLOSE_READER(re, s);
    return tmp;
}

/**
 * Read 1-25 bits.
 */
static __inline unsigned int get_bits(GetBitContext *s, int n){
    register int tmp;
    OPEN_READER(re, s);
    UPDATE_CACHE(re, s);
    tmp = SHOW_UBITS(re, s, n);
    LAST_SKIP_BITS(re, s, n);
    CLOSE_READER(re, s);
    return tmp;
}

/**
 * Shows 1-25 bits.
 */
static __inline unsigned int show_bits(GetBitContext *s, int n){
    register int tmp;
    OPEN_READER(re, s);
    UPDATE_CACHE(re, s);
    tmp = SHOW_UBITS(re, s, n);
    return tmp;
}

static __inline void skip_bits(GetBitContext *s, int n){
 //Note gcc seems to optimize this to s->index+=n for the ALT_READER :))
    OPEN_READER(re, s);
    UPDATE_CACHE(re, s);
    LAST_SKIP_BITS(re, s, n);
    CLOSE_READER(re, s);
}

static __inline unsigned int get_bits1(GetBitContext *s){
#ifdef ALT_BITSTREAM_READER
    unsigned int index = s->index;
    uint8_t result = s->buffer[index>>3];
#ifdef ALT_BITSTREAM_READER_LE
    result >>= index & 7;
    result &= 1;
#else
    result <<= index & 7;
    result >>= 8 - 1;
#endif
        index++;
    s->index = index;

    return result;
#else
    return get_bits(s, 1);
#endif
}

/**
 * reads 0-32 bits.
 */
static __inline unsigned int get_bits_long(GetBitContext *s, int n){
    if (n <= MIN_CACHE_BITS) return get_bits(s, n);
    else {
#ifdef ALT_BITSTREAM_READER_LE
        int ret = get_bits(s, 16);
        return ret | (get_bits(s, n-16) << 16);
#else
        int ret = get_bits(s, 16) << (n-16);
        return ret | get_bits(s, n-16);
#endif
    }
}

static __inline int sign_extend(int val, unsigned bits)
{
    return (val << ((8 * sizeof(int)) - bits)) >> ((8 * sizeof(int)) - bits);
}

/**
 * reads 0-32 bits as a signed integer.
 */
static __inline int get_sbits_long(GetBitContext *s, int n) {
    return sign_extend(get_bits_long(s, n), n);
}



static unsigned char ff_log2_c_tab[256]={
        0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
        5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
};

static int av_log2_c(unsigned int v)
{
    int n = 0;
    if (v & 0xffff0000) {
        v >>= 16;
        n += 16;
    }
    if (v & 0xff00) {
        v >>= 8;
        n += 8;
    }
    n += ff_log2_c_tab[v];

    return n;
}

/**
 * read unsigned golomb rice code (jpegls).
 */
static __inline int get_ur_golomb_jpegls(GetBitContext *gb, int k, int limit, int esc_len){
    unsigned int buf;
    int log;

    OPEN_READER(re, gb);
    UPDATE_CACHE(re, gb);
    buf=GET_CACHE(re, gb);

    log= av_log2_c(buf);

    if(log - k >= 32-MIN_CACHE_BITS+(MIN_CACHE_BITS==32) && 32-log < limit){
        buf >>= log - k;
        buf += (30-log)<<k;
        LAST_SKIP_BITS(re, gb, 32 + k - log);
        CLOSE_READER(re, gb);

        return buf;
    }else{
        int i;
        for(i=0; SHOW_UBITS(re, gb, 1) == 0; i++){
            if (gb->size_in_bits <= re_index)
                return -1;
            LAST_SKIP_BITS(re, gb, 1);
            UPDATE_CACHE(re, gb);
        }
        SKIP_BITS(re, gb, 1);

        if(i < limit - 1){
            if(k){
                buf = SHOW_UBITS(re, gb, k);
                LAST_SKIP_BITS(re, gb, k);
            }else{
                buf=0;
            }

            CLOSE_READER(re, gb);
            return buf + (i<<k);
        }else if(i == limit - 1){
            buf = SHOW_UBITS(re, gb, esc_len);
            LAST_SKIP_BITS(re, gb, esc_len);
            CLOSE_READER(re, gb);

            return buf + 1;
        }else
            return -1;
    }
}
/**
 * read signed golomb rice code (flac).
 */
static __inline int get_sr_golomb_flac(GetBitContext *gb, int k, int limit, int esc_len){
    int v= get_ur_golomb_jpegls(gb, k, limit, esc_len);
    return (v>>1) ^ -(v&1);
}

/**
 * shows 0-32 bits.
 */
static __inline unsigned int show_bits_long(GetBitContext *s, int n){
    if (n <= MIN_CACHE_BITS) return show_bits(s, n);
    else {
        GetBitContext gb = *s;
        return get_bits_long(&gb, n);
    }
}

/**
 * init GetBitContext.
 * @param buffer bitstream buffer, must be FF_INPUT_BUFFER_PADDING_SIZE bytes larger then the actual read bits
 * because some optimized bitstream readers read 32 or 64 bit at once and could read over the end
 * @param bit_size the size of the buffer in bits
 *
 * While GetBitContext stores the buffer size, for performance reasons you are
 * responsible for checking for the buffer end yourself (take advantage of the padding)!
 */
static __inline void init_get_bits(GetBitContext *s,
                   const uint8_t *buffer, int bit_size)
{
    int buffer_size = (bit_size+7)>>3;
    if (buffer_size < 0 || bit_size < 0) {
        buffer_size = bit_size = 0;
        buffer = NULL;
    }

    s->buffer       = buffer;
    s->size_in_bits = bit_size;
    s->buffer_end   = buffer + buffer_size;
#ifdef ALT_BITSTREAM_READER
    s->index        = 0;
#elif defined A32_BITSTREAM_READER
    s->buffer_ptr   = (uint32_t*)((intptr_t)buffer & ~3);
    s->bit_count    = 32 +     8*((intptr_t)buffer &  3);
    skip_bits_long(s, 0);
#endif
}

static __inline void align_get_bits(GetBitContext *s)
{
    int n= (-get_bits_count(s)) & 7;
    if(n) skip_bits(s, n);
}



#endif /* AVCODEC_GET_BITS_H */

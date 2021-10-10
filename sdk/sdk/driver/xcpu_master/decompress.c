#include "itx.h"
#include "pal.h"
#if ITX_BOOT_TYPE == ITX_HOST_BOOT
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* magic file header for compressed files */
static const unsigned char magic[4] = { 'S', 'M', 'A', 'Z' };

#if !defined(__OR32__)
#define DATA_SWAP32(n) \
    ( (((n) >> 24) & 0x000000FF) | \
      (((n) >>  8) & 0x0000FF00) | \
      (((n) <<  8) & 0x00FF0000) | \
      (((n) << 24) & 0xFF000000) )
#else
#define DATA_SWAP32(n) (n)
#endif

#define SAFE
#if defined(SAFE)
# if defined(__DEBUG__)
#  define fail(x,r)   if (x) { printf("%s #%d\n", __FILE__, __LINE__); *dst_len = olen; return r; }
# else
#  define fail(x,r)   if (x) { *dst_len = olen; return r; }
# endif // __DEBUG__
#else
# define fail(x,r)
#endif // SAFE

/* Thinned out version of the UCL 2e decompression sourcecode
 * Original (C) Markus F.X.J Oberhumer under GNU GPL license */

#define GETBYTE(src)  (src[ilen++])

# define GETBIT(bb, src) \
    (((bb = ((bb & 0x7f) ? (bb*2) : ((unsigned)GETBYTE(src)*2+1))) >> 8) & 1)

enum status {
    UCL_E_OK                =  0,
    UCL_E_INPUT_OVERRUN     = -0x1,
    UCL_E_OUTPUT_OVERRUN    = -0x2,
    UCL_E_LOOKBEHIND_OVERRUN= -0x3,
    UCL_E_OVERLAP_OVERRUN   = -0x4,
    UCL_E_INPUT_NOT_CONSUMED= -0x5
};

int decompress( const unsigned char* src, unsigned int  src_len,
                unsigned char* dst, unsigned int* dst_len)
{
    unsigned int bb = 0;
    unsigned int ilen = 0, olen = 0, last_m_off = 1;

#if defined(SAFE)
    const unsigned int oend = *dst_len;
#endif

    for (;;) {
        unsigned int m_off, m_len;

        while (GETBIT(bb,src)) {
            fail(ilen >= src_len, UCL_E_INPUT_OVERRUN);
            fail(olen >= oend, UCL_E_OUTPUT_OVERRUN);
            dst[olen++] = GETBYTE(src);
        }

        m_off = 1;

        for (;;) {
            m_off = m_off*2 + GETBIT(bb,src);
            fail(ilen >= src_len, UCL_E_INPUT_OVERRUN);
            fail(m_off > 0xfffffful + 3, UCL_E_LOOKBEHIND_OVERRUN);
            if (GETBIT(bb,src)) {
                break;
            }
            m_off = (m_off-1)*2 + GETBIT(bb,src);
        }

        if (m_off == 2) {
            m_off = last_m_off;
            m_len = GETBIT(bb,src);
        } else {
            fail(ilen >= src_len, UCL_E_INPUT_OVERRUN);
            m_off = (m_off-3)*256 + GETBYTE(src);
            if (m_off == 0xfffffffful) {
                break;
            }
            m_len = (m_off ^ 0xfffffffful) & 1;
            m_off >>= 1;
            last_m_off = ++m_off;
        }

        if (m_len) {
            m_len = 1 + GETBIT(bb,src);
        } else if (GETBIT(bb,src)) {
            m_len = 3 + GETBIT(bb,src);
        } else {
            m_len++;
            do {
                m_len = m_len*2 + GETBIT(bb,src);
                fail(ilen >= src_len, UCL_E_INPUT_OVERRUN);
                fail(m_len >= oend, UCL_E_OUTPUT_OVERRUN);
            } while (!GETBIT(bb,src));
            m_len += 3;
        }

        m_len += (m_off > 0x500);
        fail(olen + m_len > oend, UCL_E_OUTPUT_OVERRUN);
        fail(m_off > olen, UCL_E_LOOKBEHIND_OVERRUN);

        {
            const unsigned char* m_pos;
            m_pos = dst + olen - m_off;
            dst[olen++] = *m_pos++;
            do {
                dst[olen++] = *m_pos++;
            } while (--m_len > 0);
        }
    }

    *dst_len = olen;

    return (ilen == src_len) ? UCL_E_OK 
                             : (ilen < src_len ? UCL_E_INPUT_NOT_CONSUMED 
                                               : UCL_E_INPUT_OVERRUN);
}

int do_decompress(unsigned char *InBuf, unsigned char *outBuf)
{
    int            r = 0;
    unsigned char *buf = NULL;
    unsigned char *pOutBuf;
    unsigned int   buf_len;
    unsigned int   block_size;
    unsigned int   overhead = 0;
    unsigned int   icon_header;
    //unsigned int   OutSize;

    /*
    * Step 1: check magic header, read flags & block size, init checksum
    */
    //PalMemcpy(&icon_header, InBuf, sizeof(unsigned char) * sizeof(int));
    //InBuf += sizeof(int);

    //icon_header = DATA_SWAP32(icon_header);

    //OutSize = icon_header;

    //pOutBuf = OutBuf = (unsigned char *) calloc(OutSize, sizeof(unsigned char));
    pOutBuf = outBuf;

    PalMemcpy(&block_size, InBuf, sizeof(int) * 1);
    InBuf += sizeof(int);

    block_size = DATA_SWAP32(block_size);

    overhead   = block_size / 8 + 256;

    if (overhead == 0)
    {
        printf("header error - invalid header\n");
        r = 2;
        goto err;
    }

    if (block_size < 1024 || block_size > 8*1024*1024L)
    {
        printf("header error - invalid block size %ld\n",
            (long) block_size);
        r = 3;
        goto err;
    }

    printf("block-size is %ld bytes\n", (long)block_size);

    /*
    * Step 2: allocate buffer for in-place decompression
    */
    buf_len = (block_size + overhead + 3) & (~3l);
    buf = (unsigned char*) PalMalloc(buf_len);
    if (buf == NULL)
    {
        printf("out of memory\n");
        r = 5;
        goto err;
    }

    /*
    * Step 3: process blocks
    */
    for (;;)
    {
        unsigned char* in;
        unsigned char* out;
        unsigned int in_len;
        unsigned int out_len;

        /* read uncompressed size */
        PalMemcpy(&out_len, InBuf, sizeof(int) * 1);
        InBuf += sizeof(int);

        out_len = DATA_SWAP32(out_len);

        /* exit if last block (EOF marker) */
        if (out_len == 0)
            break;

        /* read compressed size */
        PalMemcpy(&in_len, InBuf, sizeof(int) * 1);
        InBuf += sizeof(int);               
        in_len = DATA_SWAP32(in_len);

        /* sanity check of the size values */
        if (out_len > block_size || in_len == 0)
        {
            printf("in: %u, out: %u, bk: %u\n", in_len, out_len, block_size);
            printf("block size error - data corrupted\n");
            r = 6;
            goto err;
        }

        /* place compressed block at the top of the buffer */
        in = buf + buf_len - ((in_len+3)&~3l);
        out = buf;

        /* read compressed block data */
        PalMemcpy(in, InBuf, sizeof(char) * in_len);
        InBuf += (sizeof(char) * in_len);              

        if (in_len < out_len)
        {
            /* decompress - use safe decompressor as data might be corrupted */
            unsigned int new_len = out_len;

            r = decompress(in,in_len,out,&new_len);
            if (r != UCL_E_OK || new_len != out_len)
            {
                printf("r: %d, out: %u, bk: %u\n", r, new_len, out_len);
                printf("compressed data violation: error %d (%ld/%ld/%ld)\n", r, (long) in_len, (long) out_len, (long) new_len);
                //r = 8;
                //goto err;
            }
            /* write decompressed block */
            PalMemcpy(pOutBuf, out, out_len * sizeof(char));
            pOutBuf += (out_len * sizeof(char));
        }
#if 1 //for Jedi FW compress bug, tmp solution 
        else
        {
            /* decompress - use safe decompressor as data might be corrupted */
            unsigned int new_len = out_len;

            r = decompress(in,in_len,out,&new_len);
            if (r != UCL_E_OK || new_len != out_len)
            {
                printf("compressed data violation: error %d (%ld/%ld/%ld)\n", r, (long) in_len, (long) out_len, (long) new_len);
                r = 8;
                goto err;
            }
            /* write decompressed block */
            PalMemcpy(pOutBuf, out, out_len * sizeof(char));
            pOutBuf += (out_len * sizeof(char));
        }
#else        
        else
        {
            /* write original (incompressible) block */
            PalMemcpy(pOutBuf, in, in_len * sizeof(char));
            pOutBuf += (in_len * sizeof(char));

        }
#endif        
    }

err:
    PalFree(buf);
    //PalFree(OutBuf);
    return r;
}
#endif


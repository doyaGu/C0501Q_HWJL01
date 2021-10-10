/*
 * AC-3 Audio Decoder
 * This code was developed as part of Google Summer of Code 2006.
 * E-AC-3 support was added as part of Google Summer of Code 2007.
 *
 * Copyright (c) 2006 Kartikey Mahendra BHATT (bhattkm at gmail dot com)
 * Copyright (c) 2007-2008 Bartlomiej Wolowiec <bartek.wolowiec@gmail.com>
 * Copyright (c) 2007 Justin Ruggles <justin.ruggles@gmail.com>
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

#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <string.h>

#include "debug.h"
#include "ac3dec.h"
#include "math.h"

#if defined(ENABLE_CODECS_PLUGIN)
#  include "plugin.h"
#endif

//#define AC3_CRC_CHECK
extern int nAC3Plus;
#ifdef AC3_CRC_CHECK
    static int nKeep =0;
    static int16_t lastBlock[2][256]={0};
#endif

static __inline void bswap16_buf(uint8_t *buf, int len)
{
    int i;
    uint8_t tmp;
    for(i=0;i<len;i+=2)
    {
        tmp = buf[i];
        buf[i] = buf[i+1];
        buf[i+1] = tmp;
    }
}

/*!
  \brief  Round 32-bit to 16-bit
*/
/*static int16_t DLB_srnd(int32_t x)
{

  if (x <= ((int32_t)0x7fffffff)-0x8000)
    x = x + 0x8000;
  return ( (int16_t) (x >> 16));
}*/
    
static __inline void convert2s16_interleave(int16_t *dst, int32_t *src[7], long len, int channels)
{
    int i,j,c, val;
    if(channels==1) 
    {
        for(i=0; i<len; i++) 
        {
            val = SATSHIFT(src[0][i],1);
            dst[i] = CLIPTOSHORT(val>>15);
        }
    }
    else if(channels==2) 
    {
        for(i=0; i<len; i++)
        {
            val = SATSHIFT(src[0][i],1);
            dst[2*i] = CLIPTOSHORT(val>>15);
            val = SATSHIFT(src[1][i],1);
            dst[2*i+1] = CLIPTOSHORT(val>>15);
        }
    }
    else
    {
        for(c=0; c<channels; c++)
            for(i=0, j=c; i<len; i++, j+=channels) 
            {
                val = SATSHIFT(src[c][i],1);
                dst[j] = CLIPTOSHORT(val>>15);
            }
    }
}

int frame_count = 0;

static void ac3_crc_full_error(AC3DecodeContext * s,void *out_buf,int out_channels)
{
    int16_t *out_samples = (int16_t *)out_buf;
    int blk, ch, bnd, err;
    const uint8_t *channel_map;
    int32_t *output[AC3_MAX_CHANNELS];
    int temp[AC3_BLOCK_SIZE];
    int tempDelay[AC3_MAX_CHANNELS][AC3_BLOCK_SIZE];   
    int i,j;
    ch = 0;
#if 1 
    for(i=0;i<s->channels;i++)
    {
        ch = ff_channeltab[s->channel_mode][i];
        for (j=0;j<AC3_BLOCK_SIZE;j++)
        {
            temp[j]=s->work_buf[ch][j];
            tempDelay[ch][j] = s->delay[ch][j];
        }
        xfmd_imdct(
            s->block_switch[ch],    /* input    */
            temp,                   /* modify   */
            s->fixed_coeffs[ch]);   /* modify   */    
        woad_decode_dol(
            100,                    /* input    */
            s->fixed_coeffs[ch],    /* input    */
            tempDelay[ch],          /* modify   */
            temp,                   /* output   */
            0);

    }
#endif
    //channel_map = ac3_dec_channel_map[s->dev_mode][s->dev_lfeon];

    //for (ch = 0; ch < out_channels; ch++)
        //output[ch] = s->dnmixed_buf[channel_map[ch]];
    out_channels = s->dev_channels + s->dev_lfeon;
    for (i = 0; i < out_channels; i++) {
        if(s->dev_channels==2)
            ch = ff_channeltab[AC3_CHMODE_STEREO][i];
        else if(s->dev_channels==1)
            ch = ff_channeltab[AC3_CHMODE_MONO][i];
        else
            ch = ff_channeltab[s->dev_mode][i];
        output[i] = s->dnmixed_buf[ch];
    }

    for (blk = 0; blk < s->num_blocks; blk++) 
    {
        for(i=0;i<AC3_MAX_CHANNELS-1;i++)
            s->dnmixbuf_used[i] = 0; // clear downmix output buffer flag

        for(i=0;i<s->channels;i++)
        {
            ch = ff_channeltab[s->channel_mode][i];
            //woad_decode(s->work_buf[ch], s->delay[ch]);
#if 1
            woad_decode_dol(
                100,                    /* input    */
                s->fixed_coeffs[ch],    /* input    */
                tempDelay[ch],           /* modify  */
                s->work_buf[ch],        /* output   */
                0);
#endif
            if(ch != LOW_FREQUENCY)
                eac3_tpnp_decode(s, blk, ch);
            ac3_downmix(s, ch);
        }
        convert2s16_interleave(out_samples, output, 256, out_channels);       
        out_samples += 256 * out_channels;
    }
}
/**
 * Decode a single AC-3 frame.
 */
int ac3_decode_frame(AC3DecodeContext * s, void *in_buf, int *byte_left, void *out_buf, int *out_size)
{
    const uint8_t *buf = in_buf;
    int frame_size, buf_size = *byte_left;
    int16_t *out_samples = (int16_t *)out_buf;
    int blk, ch, bnd, err;
    const uint8_t *channel_map;
    int out_channels;
    int32_t *output[AC3_MAX_CHANNELS];
    short crc;
    int i;
    out_channels = ff_ac3_channels_tab[s->dev_mode] + s->dev_lfeon;

    *out_size = 0;   
    if(*byte_left<6) {
        PRINTF("Not enough stream buffer size!");
        return -1;
    }

    frame_size = ac3_syncinfo((uint8_t*)buf);
    if(frame_size<=0 || frame_size>buf_size) {
        PRINTF("Not enough stream buffer size!");
        return -1;
    }
#ifdef AC3_CRC_CHECK
    crc = 0;
    if (nAC3Plus==0)
    {
        crc = crc_chkddfrm((short)frame_size>>1, (short*)buf);
    }
    else
    {
        /* Perform CRC checking on full frame */
        crc_calcfwd(
            (short*)buf + 1,    /* input    */
            (((short)frame_size>>1) - 1),   /* input    */
            &crc);              /* output   */
        if (crc)
            crc = CRC_ERR_FAIL_CRC1;
    }
    if (crc==CRC_ERR_FAIL_CRC1)
    {
        /* Full frame error, decode zero blocks, conceal all blocks */
        /* Use last valid blocks per frame instead of corrupt data */
        ac3_crc_full_error(s,out_buf,out_channels);
    }

#endif

    if (buf[0]==0x77 && buf[1]==0x0B) {
        // seems to be byte-swapped AC-3
        bswap16_buf((uint8_t *)buf, frame_size);
    } 
    /* initialize the GetBitContext with the start of valid AC-3 Frame */
    init_get_bits(&s->gbc, buf, buf_size * 8);

    /* parse the syncinfo */
    err = parse_frame_header(s);

    if (err) {
        switch(err) {
            case AC3_PARSE_ERROR_SYNC:
                PRINTF("frame sync error\n");
                return -1;
            case AC3_PARSE_ERROR_BSID:
                /* mute error bsid frame */
                PRINTF("invalid bitstream id = %d, size = %d\n", s->bitstream_id, s->frame_size);
                *byte_left = buf_size - s->frame_size;
                *out_size = s->num_blocks * 256 * s->dev_channels * sizeof (int16_t);
                memset( out_samples, 0, *out_size);
                return 0;
            case AC3_PARSE_ERROR_SAMPLE_RATE:
                PRINTF("invalid sample rate\n");
                break;
            case AC3_PARSE_ERROR_FRAME_SIZE:
                PRINTF("invalid frame size\n");
                break;
            case AC3_PARSE_ERROR_FRAME_TYPE:
                /* skip dependent frame */
                if(s->frame_type == EAC3_FRAME_TYPE_DEPENDENT ||
                   s->frame_type == EAC3_FRAME_TYPE_RESERVED ||
                   s->substreamid) {
                    PRINTF("unsupported frame type id = %d: skipping frame\n",s->substreamid);
                    *byte_left = buf_size - s->frame_size;
                    *out_size = 0;
                    return 0;
                } else {
                    PRINTF("invalid frame type\n");
                    return -1;
                }
                break;
            default:
                PRINTF("invalid header\n");
                break;
        }
    }
#ifdef AC3_CRC_CHECK
    if (crc==CRC_ERR_FAIL_CRC1)
    {
        *out_size = s->num_blocks * 256 * out_channels * sizeof (int16_t);
        *byte_left = buf_size - s->frame_size;
        return 0;
    }
#endif
    /* set downmixing coefficients if needed */
    set_downmix_coeffs(s);

    frame_count ++;

    // initial spx band struct
    for (i = 0; i < SPX_MAX_BANDS; i++)
         s->spx_band_struct[i] = default_spx_band_struct[i];

    // initial cpl band struct
    for (i = 0; i < AC3_MAX_CPL_BANDS; i++)
        s->cpl_band_struct[i] = ff_eac3_default_cpl_band_struct[i];

    // initial fast_gain
    for (ch = 0; ch <= s->channels; ch++)  {
        s->fast_gain[ch] = ff_ac3_fast_gain_tab[4];
        s->dba_mode[ch] = DBA_NONE;
        s->dba_nsegs[ch] = 0;
    }

    /* Check for ACMOD transition, or BSID transition */
    if (s->last_channel_mode != s->channel_mode) {
        /* Clear TPNP processing status */
        for (ch = 0; ch < AC3_MAX_CHANNELS; ch++)
        {
            s->tpndinfo[ch].tranprocflag = 0;
            s->tpndinfo[ch].opnflag = 0;
        }
    }

#ifdef AC3_CRC_CHECK
    if (crc==CRC_ERR_FAIL_CRC2)
    {
        /* Partial frame error (DD only) */
        /* Decode first 2 blocks, conceal last 4 */
        s->num_blocks = 2;
    }
#endif

    out_channels = s->dev_channels + s->dev_lfeon;
    for (i = 0; i < out_channels; i++) {
        if(s->dev_channels==2)
            ch = ff_channeltab[AC3_CHMODE_STEREO][i];
        else if(s->dev_channels==1)
            ch = ff_channeltab[AC3_CHMODE_MONO][i];
        else
            ch = ff_channeltab[s->dev_mode][i];
        output[i] = s->dnmixed_buf[ch];
    }

    /* decode the audio blocks */
    for (blk = 0; blk < s->num_blocks; blk++) {
        if (!err && decode_audio_block(s, blk)) {
            printf("error decoding the audio block\n");
            err = 1;
        }
        convert2s16_interleave(out_samples, output, 256, out_channels);
        out_samples += 256 * out_channels;
    }

#ifdef AC3_CRC_CHECK
    if (crc==CRC_ERR_FAIL_CRC2)
    {
        s->num_blocks = 4;
        ac3_crc_full_error(s,out_samples,out_channels);
        s->num_blocks = 6;
    }
#endif

    *out_size = s->num_blocks * 256 * out_channels * sizeof (int16_t);
    *byte_left = buf_size - s->frame_size;
    return 0;
}

/**
 * AVCodec initialization
 */
int ac3_decode_init(AC3DecodeContext *s)
{
    int ch,i;

    s->dith_state = 1;

    for (ch = 0; ch < AC3_MAX_CHANNELS; ch++) {
        s->tpndinfo[ch].synthread = (int*)s->tpndsynthbuf[ch];
        s->tpndinfo[ch].synthwrite = (int*)s->tpndsynthbuf[ch];
        s->tpndinfo[ch].opnflag = 0;
        s->tpndinfo[ch].tranloc = 0;
        s->tpndinfo[ch].tranlen = 0;
        s->tpndinfo[ch].cblknum = 0;
        s->tpndinfo[ch].tranprocflag = 0;
        s->tpndinfo[ch].lasttranloc = AC3_MINTRANDIST*AC3_BLOCK_SIZE;
        // wrong setting
        //for(i=0;i<AC3_MINTRANLOC;i++)
            //s->tpndsynthbuf[ch][i] = 0;
    }

    /* set scale value for float to int16 conversion */
    s->downmixed = 1;
    // reset channel config
    s->last_channel_mode = -1;
    s->last_lfe_on = -1;

    return 0;
}

/**
 * Uninitialize the AC-3 decoder.
 */
int ac3_decode_end(AC3DecodeContext *s)
{
    return 0;
}

/* sy.chuang, 2013-0301, ITE Tech. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ite/ith.h"
#include "ite/itp.h"

#include "i2s/i2s_reg.h"
#include "i2s/i2s_digvol_tb.inc"

/* ************************************************************************** */
/* wrapper */
static inline unsigned short reg_read16(unsigned short addr16) { return ithReadRegH(addr16); }
static inline void reg_write16(unsigned short addr16, unsigned short data16) { ithWriteRegH(addr16, data16); }
static inline void i2s_delay_us(unsigned us) { ithDelay(us); }

/* ************************************************************************** */
#define SERR() do { printf("ERROR# %s:%d, %s\n", __FILE__, __LINE__, __func__); while (1); } while (0)
#define S()    do { printf("=> %s:%d, %s\n",     __FILE__, __LINE__, __func__);            } while (0)

typedef signed long long s64;
typedef signed int s32;
typedef signed short s16;
typedef signed char s8;
typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

/* ************************************************************************** */
static unsigned i2s_curr_digvol_idx = I2S_DIGITAL_VOL_ZERO_IDX; /* 0 dB */

/* ************************************************************************** */
/* === common DAC/ADC ops === */
void itp_codec_wake_up(void)
{
}

void itp_codec_standby(void)
{
}

/* DAC */
void itp_codec_playback_init(unsigned output)
{
    u16 data16;

    /* FOR TEST ONLY */
#if 0
    {
        static int init_internal_9070_DAC_done = 0;
        if (!init_internal_9070_DAC_done)
        {
            /* reset internal_9070_DAC */
            data16  = reg_read16(MMP_AUDIO_CLOCK_REG_3E);
            data16 |= (1 << 14);
            reg_write16(MMP_AUDIO_CLOCK_REG_3E, data16);
            i2s_delay_us(1000); /* FIXME: dummy loop */;
            data16  = reg_read16(MMP_AUDIO_CLOCK_REG_3E);
            data16 &= ~(1 << 14);
            reg_write16(MMP_AUDIO_CLOCK_REG_3E, data16);

            /* power on internal_9070_DAC */
            data16  = reg_read16(I2S_REG_AMP_CTL);
            data16 &= ~(1 << 0);
            reg_write16(I2S_REG_AMP_CTL, data16);

            /* un-mute internal_9070_DAC */
            data16  = reg_read16(I2S_REG_AMP_CTL);
            data16 &= ~(1 << 1);
            reg_write16(I2S_REG_AMP_CTL, data16);

            //-
            init_internal_9070_DAC_done = 1;
        }
    }
#endif

    /* digital volume zero-cross control */
    {
        /* enable fading: digital volume control only works at fading mode */
        data16  = reg_read16(I2S_REG_FADE_IN_OUT); /* 0x1666 */
        data16 |= (1 << 12);
        data16 |= (0xF << 8);                      /* fading_step */
        data16 |= (0x01 & 0xFF);                   /* fading_duration */
        reg_write16(I2S_REG_FADE_IN_OUT, data16);

//		/* enable zero-crossing detection */
//		data16 = reg_read16(I2S_REG_AMP_CTL); /* 0x1672 */
//		data16 |= (1 << 3);
//		reg_write16(I2S_REG_AMP_CTL, data16);
//
//		/* zero-crossing timeout */
//		data16 = reg_read16(I2S_REG_DIG_VOL); /* 0x1668 */
//		data16 &= ~(0x7F << 9);
//		data16 |=  (0x0F << 9);
//		reg_write16(I2S_REG_DIG_VOL, data16);
//
//		/* crossing threshold */
//		reg_write16(I2S_REG_THOLD_CROSS_LO, 0x007F); /* 0x1662 */
//		reg_write16(I2S_REG_THOLD_CROSS_HI, 0x0000); /* 0x1664 */
    }

    /* restore previous playback volume */
    {
        if      (i2s_curr_digvol_idx > I2S_DIGITAL_VOL_MAX_IDX) { i2s_curr_digvol_idx = I2S_DIGITAL_VOL_MAX_IDX; }
        else if (i2s_curr_digvol_idx < I2S_DIGITAL_VOL_MIN_IDX) { i2s_curr_digvol_idx = I2S_DIGITAL_VOL_MIN_IDX; }

        if (i2s_curr_digvol_idx == I2S_DIGITAL_VOL_MIN_IDX)
        {
            data16  = reg_read16(I2S_REG_DIG_VOL); /* 0x1668 */
            data16 &= ~(0x1FF);
            data16 |= (I2S_DIGVOL_TABLE[I2S_DIGITAL_VOL_MIN_IDX] & 0x1FF);
            reg_write16(I2S_REG_DIG_VOL, data16);
            i2s_delay_us(1000); /* FIXME: dummy loop */
        }
        else
        {
            unsigned adj = I2S_DIGITAL_VOL_MIN_IDX; /* -infinite dB (Min) */

            do
            {
                adj++;
                i2s_delay_us(1000);                    /* FIXME: dummy loop */
                data16  = reg_read16(I2S_REG_DIG_VOL); /* 0x1668 */
                data16 &= ~(0x1FF);
                data16 |= (I2S_DIGVOL_TABLE[adj] & 0x1FF);
                reg_write16(I2S_REG_DIG_VOL, data16);
            } while (adj != i2s_curr_digvol_idx);
            i2s_delay_us(1000); /* FIXME: dummy loop */
        }
    }
}

void itp_codec_playback_deinit(void)
{
    if      (i2s_curr_digvol_idx > I2S_DIGITAL_VOL_MAX_IDX) { i2s_curr_digvol_idx = I2S_DIGITAL_VOL_MAX_IDX; }
    else if (i2s_curr_digvol_idx < I2S_DIGITAL_VOL_MIN_IDX) { i2s_curr_digvol_idx = I2S_DIGITAL_VOL_MIN_IDX; }

    /* decrease output volume */
    {
        unsigned short data16;
        unsigned       adj = i2s_curr_digvol_idx;
        if (adj != I2S_DIGITAL_VOL_MIN_IDX) /* -infinite dB (Min) */
        {
            do
            {
                adj--;
                i2s_delay_us(1000);                    /* FIXME: dummy loop */
                data16  = reg_read16(I2S_REG_DIG_VOL); /* 0x1668 */
                data16 &= ~(0x1FF);
                data16 |= (I2S_DIGVOL_TABLE[adj] & 0x1FF);
                reg_write16(I2S_REG_DIG_VOL, data16);
            } while (adj != I2S_DIGITAL_VOL_MIN_IDX); /* -infinite dB (Min) */
        }
    }
}

void itp_codec_playback_amp_volume_down(void)
{
    if (i2s_curr_digvol_idx <= I2S_DIGITAL_VOL_MIN_IDX) { return; }

    switch (i2s_curr_digvol_idx)
    {
#ifndef _WIN32
    case 33 ... 38: { itp_codec_playback_set_direct_vol(32); break; } /* - 6dB, 15 bits resolution */
    case 27 ... 32: { itp_codec_playback_set_direct_vol(26); break; } /* -12dB, 14 bits resolution */
    case 21 ... 26: { itp_codec_playback_set_direct_vol(20); break; } /* -18dB, 13 bits resolution */
    case 15 ... 20: { itp_codec_playback_set_direct_vol(14); break; } /* -24dB, 12 bits resolution */
    case  9 ... 14: { itp_codec_playback_set_direct_vol( 8); break; } /* -30dB, 11 bits resolution */
    case  5 ...  8: { itp_codec_playback_set_direct_vol( 4); break; } /* -36dB, 10 bits resolution */
    case  3 ...  4: { itp_codec_playback_set_direct_vol( 2); break; } /* -42dB,  9 bits resolution */
    case         2: { itp_codec_playback_set_direct_vol( 1); break; } /* -48dB,  8 bits resolution */
    case         1: { itp_codec_playback_set_direct_vol( 0); break; } /* -INFdB */
#endif                                                                // !_WIN32
    }
}

void itp_codec_playback_amp_volume_up(void)
{
    if (i2s_curr_digvol_idx >= I2S_DIGITAL_VOL_MAX_IDX) { return; }

    switch (i2s_curr_digvol_idx)
    {
#ifndef _WIN32
    case 32 ... 37: { itp_codec_playback_set_direct_vol(38); break; } /* - 0dB, 16 bits resolution */
    case 26 ... 31: { itp_codec_playback_set_direct_vol(32); break; } /* - 6dB, 15 bits resolution */
    case 20 ... 25: { itp_codec_playback_set_direct_vol(26); break; } /* -12dB, 14 bits resolution */
    case 14 ... 19: { itp_codec_playback_set_direct_vol(20); break; } /* -18dB, 13 bits resolution */
    case  8 ... 13: { itp_codec_playback_set_direct_vol(14); break; } /* -24dB, 12 bits resolution */
    case  4 ...  7: { itp_codec_playback_set_direct_vol( 8); break; } /* -30dB, 11 bits resolution */
    case  2 ...  3: { itp_codec_playback_set_direct_vol( 4); break; } /* -36dB, 10 bits resolution */
    case         1: { itp_codec_playback_set_direct_vol( 2); break; } /* -42dB,  9 bits resolution */
    case         0: { itp_codec_playback_set_direct_vol( 1); break; } /* -48dB,  8 bits resolution */
#endif                                                                // !_WIN32
    }
}

void itp_codec_playback_set_direct_vol(unsigned target_vol)
{
    u16 data16;
    int direction;

    if ((target_vol > I2S_DIGITAL_VOL_MAX_IDX) || (target_vol < I2S_DIGITAL_VOL_MIN_IDX))
    {
        printf("ERROR# invalid target volume step: 0x%08x\n", target_vol);
        return;
    }

    if      (i2s_curr_digvol_idx == target_vol) { return; }
    else if (i2s_curr_digvol_idx <  target_vol) { direction = 1; } /* + */
    else                                        { direction = 0; } /* - */

    while (i2s_curr_digvol_idx != target_vol)
    {
        if (direction == 1) { i2s_curr_digvol_idx++; }
        else                { i2s_curr_digvol_idx--; }

        data16  = reg_read16(I2S_REG_DIG_VOL); /* 0x1668 */
        data16 &= ~(0x1FF);
        data16 |= (I2S_DIGVOL_TABLE[i2s_curr_digvol_idx] & 0x1FF);
        reg_write16(I2S_REG_DIG_VOL, data16);

        i2s_delay_us(1000); /* FIXME: dummy loop */;
    }
}

void itp_codec_playback_set_direct_volperc(unsigned target_volperc)
{
    unsigned volstep;
    if (target_volperc > 100) { target_volperc = 100; }
    volstep = (target_volperc * I2S_DIGITAL_VOL_MAX_IDX) / 100;
    itp_codec_playback_set_direct_vol(volstep);
}

void itp_codec_playback_get_currvol(unsigned *currvol)
{
    *currvol = i2s_curr_digvol_idx;
}

void itp_codec_playback_get_currvolperc(unsigned *currvolperc)
{
    const unsigned curr_volume = i2s_curr_digvol_idx;
    *currvolperc = (curr_volume * 100) / I2S_DIGITAL_VOL_MAX_IDX;
    if (*currvolperc > 100) { *currvolperc = 100; }
}

void itp_codec_playback_get_vol_range(unsigned *max, unsigned *regular_0db, unsigned *min)
{
    *max         = I2S_DIGITAL_VOL_MAX_IDX;  /* 0 dB */
    *regular_0db = I2S_DIGITAL_VOL_ZERO_IDX; /* 0 dB */
    *min         = I2S_DIGITAL_VOL_MIN_IDX;  /* -INFdB dB */
}

void itp_codec_playback_mute(void)
{
    /* digital mute */
    {
        u16 data16 = reg_read16(I2S_REG_DIG_VOL); /* 0x1668 */
        data16 &= ~(0x1FF);
        data16 |= (0x000);
        reg_write16(I2S_REG_DIG_VOL, data16);
    }
    //-
    i2s_delay_us(25000); /* FIXME: dummy loop */
}

void itp_codec_playback_unmute(void)
{
    /* digital un-mute */
    {
        u16 data16 = reg_read16(I2S_REG_DIG_VOL); /* 0x1668 */
        data16 &= ~(0x1FF);
        data16 |= (I2S_DIGVOL_TABLE[i2s_curr_digvol_idx] & 0x1FF);
        reg_write16(I2S_REG_DIG_VOL, data16);
    }
    //-
}

void itp_codec_playback_linein_bypass(unsigned bypass)
{
    /* line-in bypass to line-out directly */
    /* TODO */
    printf("ERROR# This DAC does NOT support bypass function:(%02x)\n", bypass);
}

/* ADC */
void itp_codec_rec_init(unsigned input_source)
{
    /* HARDWARE NOT SUPPORT RECORDING ! */
    (void)input_source;
}

void itp_codec_rec_deinit(void)
{
    /* HARDWARE NOT SUPPORT RECORDING ! */
}

void itp_codec_rec_set_direct_vol(unsigned target_vol)
{
    /* HARDWARE NOT SUPPORT RECORDING ! */
    (void)target_vol;
}

void itp_codec_rec_get_currvol(unsigned *currvol)
{
    /* HARDWARE NOT SUPPORT RECORDING ! */
    *currvol = 0;
}

void itp_codec_rec_get_vol_range(unsigned *max, unsigned *regular_0db, unsigned *min)
{
    /* HARDWARE NOT SUPPORT RECORDING ! */
    *max         = 0;
    *regular_0db = 0;
    *min         = 0;
}

void itp_codec_rec_mute(void)
{
    /* HARDWARE NOT SUPPORT RECORDING ! */
}

void itp_codec_rec_unmute(void)
{
    /* HARDWARE NOT SUPPORT RECORDING ! */
}

void itp_codec_power_on(void)
{
}

void itp_codec_power_off(void)
{
}

void itp_codec_get_i2s_sample_rate(int* samplerate)
{
    (void)samplerate;
}

void itp_codec_set_i2s_sample_rate(int samplerate)
{
    (void)samplerate;
}

int itp_codec_get_DA_running(void){
    return 0;
}
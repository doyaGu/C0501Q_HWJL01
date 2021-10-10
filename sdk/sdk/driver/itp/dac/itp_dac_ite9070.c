/* sy.chuang, 2012-0423, ITE Tech. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ite/ith.h"
#include "ite/itp.h"

#include "i2s/i2s_reg.h"

/* workaround table, due to amp bug */
#include "ite9070/D_table.inc"
#include "ite9070/A_table.inc"

/* ************************************************************************** */
/* wrapper */
static inline unsigned short reg_read16(unsigned short addr16) { return ithReadRegH(addr16); }
static inline void reg_write16(unsigned short addr16, unsigned short data16) { ithWriteRegH(addr16, data16); }
static inline void i2s_delay_us(unsigned us) { ithDelay(us); }

/* ************************************************************************** */
#define SERR() do { printf("ERROR# %s:%d, %s\n", __FILE__, __LINE__, __func__); while(1); } while(0)
#define S()    do { printf("=> %s:%d, %s\n",     __FILE__, __LINE__, __func__);           } while(0)

typedef signed   long long s64;
typedef signed   int       s32;
typedef signed   short     s16;
typedef signed   char      s8;
typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned short     u16;
typedef unsigned char      u8;

/* ************************************************************************** */
#define ITE9070_MAX_DAC_AMP_VOL     0x3B /* +2  dB */
#define ITE9070_ZERO_DB_DAC_AMP_VOL 0x39 /*  0  dB */
#define ITE9070_MIN_DAC_AMP_VOL     0x00 /* -57 dB */
static unsigned ite9070_curr_dac_amp_volume = ITE9070_ZERO_DB_DAC_AMP_VOL; /* 0 dB */

static unsigned gCurrVolStep = ITE9070_ZERO_DB_DAC_AMP_VOL;
static unsigned gDacHasInit = 0;
//static unsigned gIsDacMute = 1;
static unsigned ITE9079_DAC_VOL_DELAY = 1000;

static void _internal_DAC_9070_reset(void);
static void _internal_DAC_9070_power_on(void);
static void _internal_DAC_9070_power_off(void);
static void _internal_DAC_9070_unmute(void);
static void _internal_DAC_9070_mute(void);

/* workaround table, due to amp bug */
static void _vol_setR(unsigned step);

/* ************************************************************************** */
static void _internal_DAC_9070_reset(void)
{
	u16 data16;

	data16 = reg_read16(MMP_AUDIO_CLOCK_REG_3E);
	data16 |= (1 << 14);
	reg_write16(MMP_AUDIO_CLOCK_REG_3E, data16);

	i2s_delay_us(1000); /* FIXME: dummy loop */;

	data16 = reg_read16(MMP_AUDIO_CLOCK_REG_3E);
	data16 &= ~(1 << 14);
	reg_write16(MMP_AUDIO_CLOCK_REG_3E, data16);
}

static void _internal_DAC_9070_power_on(void)
{
	u16 data16;

	data16 = reg_read16(I2S_REG_AMP_CTL);
	data16 &= ~(1 << 0);
	reg_write16(I2S_REG_AMP_CTL, data16);
}

static void _internal_DAC_9070_power_off(void)
{
	u16 data16;

	data16 = reg_read16(I2S_REG_AMP_CTL);
	data16 |= (1 << 0);
	reg_write16(I2S_REG_AMP_CTL, data16);
}

static void _internal_DAC_9070_unmute(void)
{
	u16 data16;

	data16 = reg_read16(I2S_REG_AMP_CTL);
	data16 &= ~(1 << 1);
	reg_write16(I2S_REG_AMP_CTL, data16);
}

static void _internal_DAC_9070_mute(void)
{
	u16 data16;

	data16 = reg_read16(I2S_REG_AMP_CTL);
	data16 |= (1 << 1);
	reg_write16(I2S_REG_AMP_CTL, data16);
}

/* workaround table, due to amp bug */
/* R = D + A */
static void _vol_setR(unsigned step)
{
	u16 data16;
	//printf(" dacVolStep=%d,gCV=%d\n",step, gCurrVolStep);

	/* D: digital volume */
	{
		u16 D;

		D = (u16)DTBL[step];
		if(D == 0) { D = 0x100; }

		data16 = reg_read16(I2S_REG_DIG_VOL); /* 0x1668 */
		data16 &= ~(0x1FF);
		data16 |= (D & 0x1FF);
		reg_write16(I2S_REG_DIG_VOL, data16);
	}

	/* A: analogue volume */
	{
		data16 = reg_read16(I2S_REG_AMP_VOL);
		data16 &= ~(0x003F);
		data16 |= (ATBL[step] & 0x3F);
		reg_write16(I2S_REG_AMP_VOL, data16);
	}
	gCurrVolStep = step;
}

static void _restoreToCurrAmpVol(unsigned tgtStep)
{
	unsigned adj = gCurrVolStep;
	
	while(tgtStep != adj)
	{
		if(adj<tgtStep)	adj++;
		if(adj>tgtStep)	adj--;
		
		_vol_setR(adj);
		i2s_delay_us(ITE9079_DAC_VOL_DELAY); /* FIXME: dummy loop */;
	}	
}
/* ************************************************************************** */
/* === common DAC/ADC ops === */
void itp_codec_wake_up(void)
{
#ifdef CFG_DAC_USE_EXTERNAL_DEPOP_CIRCUIT /* de-pop */
	printf("DAC(ite9070)# %s, depop use GPIO(%d) +\n", __func__, CFG_DAC_EXT_DEPOP_GPIO_PIN);

	#if 0
	{
		volatile u16 data16;
		data16 = reg_read16(I2S_REG_AMP_VOL);
		data16 &= ~(0x003F);
		printf("readback(1), amp dac vol = 0x%04x\n", data16);
		reg_write16(I2S_REG_AMP_VOL, data16);

		data16 = reg_read16(I2S_REG_AMP_VOL);
		printf("readback(2), amp dac vol = 0x%04x\n", data16);

		usleep(50000);

		data16 = reg_read16(I2S_REG_AMP_CTL);
		data16 |= (1 << 1);
		reg_write16(I2S_REG_AMP_CTL, data16);

		data16 = reg_read16(I2S_REG_AMP_CTL);
		printf("readback, amp mute ctl = 0x%04x\n", data16);
	}
	#endif

	/* config GPIO for depop */
	{
		unsigned data32;

		/* setup mode/direction */
		ithGpioSetMode(CFG_DAC_EXT_DEPOP_GPIO_PIN, ITH_GPIO_MODE0); /* 0xDE000000 | 0x90 */

		/* set as output mode */
		data32 = ithReadRegA(ITH_GPIO_BASE | 0x08);
		data32 |= (1 << CFG_DAC_EXT_DEPOP_GPIO_PIN);
		ithWriteRegA(ITH_GPIO_BASE | 0x08, data32);

		/* set to 1 */
		data32 = ithReadRegA(ITH_GPIO_BASE | 0x00);
		data32 |= (1 << CFG_DAC_EXT_DEPOP_GPIO_PIN);
		ithWriteRegA(ITH_GPIO_BASE | 0x00, data32);

		printf("DAC(ite9070)# SET GPIO(%d) TO 1\n", CFG_DAC_EXT_DEPOP_GPIO_PIN);
	}

	/* debug: read back */
	{
		volatile unsigned data32;
		do
		{
			data32 = ithReadRegA(ITH_GPIO_BASE | 0x04);
			__asm__("");
		}
		while(((data32 >> CFG_DAC_EXT_DEPOP_GPIO_PIN) & 0x1) != 1);
		printf("DAC(ite9070)# GPIO read back OK !\n");
	}
	
	printf("DAC(ite9070)# T1: delay %d ms\n", CFG_DAC_EXT_DEPOP_DELAY_MS_T1);
	i2s_delay_us((CFG_DAC_EXT_DEPOP_DELAY_MS_T1) * 1000);
	
	#if 0
	{
		volatile u16 data16;
//		data16 = reg_read16(I2S_REG_AMP_VOL);
//		data16 &= ~(0x003F);
//		printf("readback(1), dac vol = 0x%04x\n", data16);
//		reg_write16(I2S_REG_AMP_VOL, data16);

		data16 = reg_read16(I2S_REG_AMP_VOL);
		printf("readback(2), amp dac vol = 0x%04x\n", data16);

		data16 = reg_read16(I2S_REG_AMP_CTL);
		printf("readback, amp mute ctl = 0x%04x\n", data16);
	}
	#endif
#endif /* de-pop */

	printf("DAC(ite9070)# _internal_DAC_9070_power_on !\n");
	_internal_DAC_9070_power_on();

	/* workaround: to avoid amp bug, adjust Bias/ClassB */
	{
		u16 data16;

		data16 = reg_read16(I2S_REG_AMP_CTL); /* 0x1672 */

		/* adjust Bias (0.8x) */
		data16 &= ~(3 << 6);

		/* adjust ClassB (0x101) */
		data16 &= ~(7 << 8);
		data16 |=  (5 << 8);

		reg_write16(I2S_REG_AMP_CTL, data16);
	}

#ifdef CFG_DAC_USE_EXTERNAL_DEPOP_CIRCUIT /* de-pop */
	/* delay a while to allow VMID to initially charge */
	printf("DAC(ite9070)# T2: delay %d ms\n", CFG_DAC_EXT_DEPOP_DELAY_MS_T2);
	i2s_delay_us((CFG_DAC_EXT_DEPOP_DELAY_MS_T2) * 1000);

	#if 0
	{
		volatile u16 data16;
//		data16 = reg_read16(I2S_REG_AMP_VOL);
//		data16 &= ~(0x003F);
//		printf("readback(1), dac vol = 0x%04x\n", data16);
//		reg_write16(I2S_REG_AMP_VOL, data16);

		data16 = reg_read16(I2S_REG_AMP_VOL);
		printf("readback(2), amp dac vol = 0x%04x\n", data16);

		data16 = reg_read16(I2S_REG_AMP_CTL);
		printf("readback, amp mute ctl = 0x%04x\n", data16);
	}
	#endif

	/* set to 0 */
	{
		unsigned data32;

		data32 = ithReadRegA(ITH_GPIO_BASE | 0x00);
		data32 &= ~(1 << CFG_DAC_EXT_DEPOP_GPIO_PIN);
		ithWriteRegA(ITH_GPIO_BASE | 0x00, data32);

		printf("DAC(ite9070)# SET GPIO(%d) TO 0\n", CFG_DAC_EXT_DEPOP_GPIO_PIN);
	}

	/* debug: read back */
	{
		volatile unsigned data32;
		do
		{
			data32 = ithReadRegA(ITH_GPIO_BASE | 0x04);
			__asm__("");
		}
		while(((data32 >> CFG_DAC_EXT_DEPOP_GPIO_PIN) & 0x1) != 0);
		printf("DAC(ite9070)# GPIO read back OK !\n");
	}

	printf("DAC(ite9070)# %s, depop -\n", __func__);
#endif /* de-pop */
}

void itp_codec_standby(void)
{
#ifdef CFG_DAC_USE_EXTERNAL_DEPOP_CIRCUIT /* de-pop */
	printf("DAC(ite9070)# %s, depop use GPIO(%d) +\n", __func__, CFG_DAC_EXT_DEPOP_GPIO_PIN);

	#if 0
	{
		volatile u16 data16;
		data16 = reg_read16(I2S_REG_AMP_VOL);
		data16 &= ~(0x003F);
		printf("readback(1), amp dac vol = 0x%04x\n", data16);
		reg_write16(I2S_REG_AMP_VOL, data16);

		data16 = reg_read16(I2S_REG_AMP_VOL);
		printf("readback(2), amp dac vol = 0x%04x\n", data16);

		usleep(50000);

		data16 = reg_read16(I2S_REG_AMP_CTL);
		data16 |= (1 << 1);
		reg_write16(I2S_REG_AMP_CTL, data16);

		data16 = reg_read16(I2S_REG_AMP_CTL);
		printf("readback, amp mute ctl = 0x%04x\n", data16);
	}
	#endif

	/* config GPIO for depop */
	{
		unsigned data32;

		/* setup mode/direction */
		ithGpioSetMode(CFG_DAC_EXT_DEPOP_GPIO_PIN, ITH_GPIO_MODE0); /* 0xDE000000 | 0x90 */

		/* set as output mode */
		data32 = ithReadRegA(ITH_GPIO_BASE | 0x08);
		data32 |= (1 << CFG_DAC_EXT_DEPOP_GPIO_PIN);
		ithWriteRegA(ITH_GPIO_BASE | 0x08, data32);

		/* set to 1 */
		data32 = ithReadRegA(ITH_GPIO_BASE | 0x00);
		data32 |= (1 << CFG_DAC_EXT_DEPOP_GPIO_PIN);
		ithWriteRegA(ITH_GPIO_BASE | 0x00, data32);

		printf("DAC(ite9070)# SET GPIO(%d) TO 1\n", CFG_DAC_EXT_DEPOP_GPIO_PIN);
	}

	/* debug: read back */
	{
		volatile unsigned data32;
		do
		{
			data32 = ithReadRegA(ITH_GPIO_BASE | 0x04);
			__asm__("");
		}
		while(((data32 >> CFG_DAC_EXT_DEPOP_GPIO_PIN) & 0x1) != 1);
		printf("DAC(ite9070)# GPIO read back OK !\n");
	}

	printf("DAC(ite9070)# T1: delay %d ms\n", CFG_DAC_EXT_DEPOP_DELAY_MS_T1);
	i2s_delay_us((CFG_DAC_EXT_DEPOP_DELAY_MS_T1) * 1000);

	#if 0
	{
		volatile u16 data16;
//		data16 = reg_read16(I2S_REG_AMP_VOL);
//		data16 &= ~(0x003F);
//		printf("readback(1), dac vol = 0x%04x\n", data16);
//		reg_write16(I2S_REG_AMP_VOL, data16);

		data16 = reg_read16(I2S_REG_AMP_VOL);
		printf("readback(2), amp dac vol = 0x%04x\n", data16);

		data16 = reg_read16(I2S_REG_AMP_CTL);
		printf("readback, amp mute ctl = 0x%04x\n", data16);
	}
	#endif
#endif /* de-pop */

	printf("DAC(ite9070)# _internal_DAC_9070_power_off !\n");
	_internal_DAC_9070_power_off();

#ifdef CFG_DAC_USE_EXTERNAL_DEPOP_CIRCUIT /* de-pop */
	/* delay a while to allow VMID to initially charge */
	printf("DAC(ite9070)# T2: delay %d ms\n", CFG_DAC_EXT_DEPOP_DELAY_MS_T2);
	i2s_delay_us((CFG_DAC_EXT_DEPOP_DELAY_MS_T2) * 1000);

	#if 0
	{
		volatile u16 data16;
//		data16 = reg_read16(I2S_REG_AMP_VOL);
//		data16 &= ~(0x003F);
//		printf("readback(1), dac vol = 0x%04x\n", data16);
//		reg_write16(I2S_REG_AMP_VOL, data16);

		data16 = reg_read16(I2S_REG_AMP_VOL);
		printf("readback(2), amp dac vol = 0x%04x\n", data16);

		data16 = reg_read16(I2S_REG_AMP_CTL);
		printf("readback, amp mute ctl = 0x%04x\n", data16);
	}
	#endif

	/* set to 0 */
	{
		unsigned data32;

		data32 = ithReadRegA(ITH_GPIO_BASE | 0x00);
		data32 &= ~(1 << CFG_DAC_EXT_DEPOP_GPIO_PIN);
		ithWriteRegA(ITH_GPIO_BASE | 0x00, data32);

		printf("DAC(ite9070)# SET GPIO(%d) TO 0\n", CFG_DAC_EXT_DEPOP_GPIO_PIN);
	}

	/* debug: read back */
	{
		volatile unsigned data32;
		do
		{
			data32 = ithReadRegA(ITH_GPIO_BASE | 0x04);
			__asm__("");
		}
		while(((data32 >> CFG_DAC_EXT_DEPOP_GPIO_PIN) & 0x1) != 0);
		printf("DAC(ite9070)# GPIO read back OK !\n");
	}

	printf("DAC(ite9070)# %s, depop -\n", __func__);
#endif /* de-pop */
}

/* DAC */
void itp_codec_playback_init(unsigned output)
{
	u16 data16;

	_internal_DAC_9070_reset();

//	/* AMP Zero-Cross control */
//	{
//		/* enable zero-cross detection */
//		data16 = reg_read16(I2S_REG_AMP_CTL); /* 0x1672 */
//		data16 |= (1 << 3);
//		reg_write16(I2S_REG_AMP_CTL, data16);
//
//		/* zero-cross timeout */
//		data16 = reg_read16(I2S_REG_DIG_VOL); /* 0x1668 */
//		data16 &= ~(0x7F << 9);
//		data16 |=  (0x00 << 9);
//		reg_write16(I2S_REG_DIG_VOL, data16);
//
//		/* threshold */
//		reg_write16(I2S_REG_THOLD_CROSS_LO, 0x7FFF); /* 0x1662 */
//		reg_write16(I2S_REG_THOLD_CROSS_HI, 0x7FFF); /* 0x1664 */
//	}

	/* workaround: Digital -2 dB to avoid amp bug */
	{
		/* enable_fading, fast response */
		{
			const u32 fading_step = 0xF;
			const u32 fading_duration = 0x01;
			data16 = reg_read16(I2S_REG_FADE_IN_OUT);
			data16 |= (1 << 12);
			data16 |= ((fading_step & 0xF) << 8);
			data16 |= (fading_duration & 0xFF);
			reg_write16(I2S_REG_FADE_IN_OUT, data16);
		}
	}

	/* analogue mute */
	_internal_DAC_9070_mute();
	{
		_vol_setR(ITE9070_MIN_DAC_AMP_VOL);
	}
	//-

	_internal_DAC_9070_power_on();
	_internal_DAC_9070_unmute();

	/* restore previous playback volume */
	{
		unsigned adj = ITE9070_MIN_DAC_AMP_VOL;

		if     (ite9070_curr_dac_amp_volume > ITE9070_MAX_DAC_AMP_VOL) { ite9070_curr_dac_amp_volume = ITE9070_MAX_DAC_AMP_VOL; }
		else if(ite9070_curr_dac_amp_volume < ITE9070_MIN_DAC_AMP_VOL) { ite9070_curr_dac_amp_volume = ITE9070_MIN_DAC_AMP_VOL; }

		if(adj != ite9070_curr_dac_amp_volume)
		{
			do
			{
				adj++;
				i2s_delay_us(ITE9079_DAC_VOL_DELAY); /* FIXME: dummy loop */
				_vol_setR(adj);
			} while(adj != ite9070_curr_dac_amp_volume);
			i2s_delay_us(1000); /* FIXME: dummy loop */
		}
	}
	
	gDacHasInit = 1;
}

void itp_codec_playback_deinit(void)
{
	/* analogue mute */
//	_internal_DAC_9070_mute();

	if     (ite9070_curr_dac_amp_volume > ITE9070_MAX_DAC_AMP_VOL) { ite9070_curr_dac_amp_volume = ITE9070_MAX_DAC_AMP_VOL; }
	else if(ite9070_curr_dac_amp_volume < ITE9070_MIN_DAC_AMP_VOL) { ite9070_curr_dac_amp_volume = ITE9070_MIN_DAC_AMP_VOL; }

	/* decrease output volume */
	{
		unsigned adj = ite9070_curr_dac_amp_volume;
		if(adj != ITE9070_MIN_DAC_AMP_VOL) {
			do
			{
				adj--;
				
				i2s_delay_us(ITE9079_DAC_VOL_DELAY); /* FIXME: dummy loop */
				_vol_setR(adj);
			} while(adj != ITE9070_MIN_DAC_AMP_VOL);
		}
	}

	gDacHasInit = 0;

	/* really turn-off ITE9070 internal DAC, DON'T DO THAT ! */
//	_internal_DAC_9070_power_off();
}

void itp_codec_playback_amp_volume_down(void)
{
	if(ite9070_curr_dac_amp_volume <= ITE9070_MIN_DAC_AMP_VOL) return;

    if (ite9070_curr_dac_amp_volume >= 0x3A && ite9070_curr_dac_amp_volume <= 0x3B) /* 0  dB */
    {
        itp_codec_playback_set_direct_vol(0x39);
    }
    else if (ite9070_curr_dac_amp_volume >= 0x37 && ite9070_curr_dac_amp_volume <= 0x39) /* -3 dB */
    {
        itp_codec_playback_set_direct_vol(0x36);
    }
    else if (ite9070_curr_dac_amp_volume >= 0x34 && ite9070_curr_dac_amp_volume <= 0x36) /* -6 dB */
    {
        itp_codec_playback_set_direct_vol(0x33);
    }
    else if (ite9070_curr_dac_amp_volume >= 0x2E && ite9070_curr_dac_amp_volume <= 0x33) /* -12dB */
    {
        itp_codec_playback_set_direct_vol(0x2D);
    }
    else if (ite9070_curr_dac_amp_volume >= 0x22 && ite9070_curr_dac_amp_volume <= 0x2D) /* -24dB */
    {
        itp_codec_playback_set_direct_vol(0x21);
    }
    else if (ite9070_curr_dac_amp_volume >= 0x01 && ite9070_curr_dac_amp_volume <= 0x21) /* -57dB */
    {
        itp_codec_playback_set_direct_vol(0x00);
    }
}

void itp_codec_playback_amp_volume_up(void)
{
	if(ite9070_curr_dac_amp_volume >= ITE9070_MAX_DAC_AMP_VOL) return;

    if (ite9070_curr_dac_amp_volume >= 0x39 && ite9070_curr_dac_amp_volume <= 0x3A) /* +2 dB */
    {
        itp_codec_playback_set_direct_vol(0x3B);
    }
    else if (ite9070_curr_dac_amp_volume >= 0x36 && ite9070_curr_dac_amp_volume <= 0x38) /* 0  dB */
    {
        itp_codec_playback_set_direct_vol(0x39);
    }
    else if (ite9070_curr_dac_amp_volume >= 0x33 && ite9070_curr_dac_amp_volume <= 0x35) /* -3 dB */
    {
        itp_codec_playback_set_direct_vol(0x36);
    }
    else if (ite9070_curr_dac_amp_volume >= 0x2D && ite9070_curr_dac_amp_volume <= 0x32) /* -6 dB */
    {
        itp_codec_playback_set_direct_vol(0x33);
    }
    else if (ite9070_curr_dac_amp_volume >= 0x21 && ite9070_curr_dac_amp_volume <= 0x2C) /* -12dB */
    {
        itp_codec_playback_set_direct_vol(0x2D);
    }
    else if (ite9070_curr_dac_amp_volume >= 0x00 && ite9070_curr_dac_amp_volume <= 0x20) /* -24dB */
    {
        itp_codec_playback_set_direct_vol(0x21);
    }
}

void itp_codec_playback_set_direct_vol(unsigned target_vol)
{
	int direction;

	if(target_vol > ITE9070_MAX_DAC_AMP_VOL) {
		printf("ERROR# invalid target volume step: 0x%08x\n", target_vol);
		return;
	}

	if(gDacHasInit==0)
	{
		ite9070_curr_dac_amp_volume = target_vol;
	}
	else
	{
		if(gCurrVolStep!=ite9070_curr_dac_amp_volume)	_restoreToCurrAmpVol(ite9070_curr_dac_amp_volume);
    	
		if     (ite9070_curr_dac_amp_volume == target_vol) { return; }
		else if(ite9070_curr_dac_amp_volume <  target_vol) { direction = 1; } /* + */
		else                                { direction = 0; } /* - */
    	
		while(ite9070_curr_dac_amp_volume != target_vol)
		{
			if(direction == 1) { ite9070_curr_dac_amp_volume++; }
			else               { ite9070_curr_dac_amp_volume--; }
    	
			_vol_setR(ite9070_curr_dac_amp_volume);
    	
			i2s_delay_us(ITE9079_DAC_VOL_DELAY); /* FIXME: dummy loop */;
		}
	}
}

void itp_codec_playback_set_direct_volperc(unsigned target_volperc)
{
	unsigned char volstep;

	if(target_volperc > 100) { target_volperc = 100; }

	volstep = ((target_volperc * ITE9070_MAX_DAC_AMP_VOL) / 100);
	itp_codec_playback_set_direct_vol(volstep);
}

void itp_codec_playback_get_currvol(unsigned *currvol)
{
	*currvol = ite9070_curr_dac_amp_volume;
}

void itp_codec_playback_get_currvolperc(unsigned *currvolperc)
{
	const unsigned curr_volume = ite9070_curr_dac_amp_volume;
	*currvolperc = (100 * curr_volume) / ITE9070_MAX_DAC_AMP_VOL;
	if(*currvolperc > 100) { *currvolperc = 100; }
}

void itp_codec_playback_get_vol_range(unsigned *max, unsigned *regular_0db, unsigned *min)
{
	*max         = ITE9070_MAX_DAC_AMP_VOL;     /* +6  dB */
	*regular_0db = ITE9070_ZERO_DB_DAC_AMP_VOL; /*  0  dB */
	*min         = ITE9070_MIN_DAC_AMP_VOL;     /* -57 dB */
}

void itp_codec_playback_mute(void)
{
#if 0
	_internal_DAC_9070_mute();
#else
	/* decrease output volume */
	{
		unsigned adj = ite9070_curr_dac_amp_volume;
		if(adj != ITE9070_MIN_DAC_AMP_VOL) {
			do
			{
				adj--;
				i2s_delay_us(ITE9079_DAC_VOL_DELAY); /* FIXME: dummy loop */
				_vol_setR(adj);
			} while(adj != ITE9070_MIN_DAC_AMP_VOL);
		}
	}
	_internal_DAC_9070_mute();
	//gIsDacMute = 1;
#endif
	i2s_delay_us(25000); /* FIXME: dummy loop */
}

void itp_codec_playback_unmute(void)
{
#if 0
	_internal_DAC_9070_unmute();
#else
	_internal_DAC_9070_unmute();
	/* restore previous playback volume */
	{
		unsigned adj = ITE9070_MIN_DAC_AMP_VOL;

		if     (ite9070_curr_dac_amp_volume > ITE9070_MAX_DAC_AMP_VOL) { ite9070_curr_dac_amp_volume = ITE9070_MAX_DAC_AMP_VOL; }
		else if(ite9070_curr_dac_amp_volume < ITE9070_MIN_DAC_AMP_VOL) { ite9070_curr_dac_amp_volume = ITE9070_MIN_DAC_AMP_VOL; }

		if(adj != ite9070_curr_dac_amp_volume)
		{
			do
			{
				adj++;
				i2s_delay_us(ITE9079_DAC_VOL_DELAY); /* FIXME: dummy loop */
				_vol_setR(adj);
			} while(adj != ite9070_curr_dac_amp_volume);
			i2s_delay_us(1000); /* FIXME: dummy loop */
		}
	}
#endif
}

void itp_codec_playback_linein_bypass(unsigned bypass)
{
	/* line-in bypass to line-out directly */
	/* TODO */
	printf("ERROR# ite9070 DAC does NOT support bypass function:(%02x)\n", bypass);
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

void itp_codec_get_i2s_sample_rate(int *samplerate)
{
}

void itp_codec_set_i2s_sample_rate(int samplerate)
{
}


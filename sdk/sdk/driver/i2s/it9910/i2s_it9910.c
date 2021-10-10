/* sy.chuang, 2012-0823, ITE Tech. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "ite/ith.h"
#include "ite/itp.h"

#include "i2s/i2s_it9910.h"
#include "i2s_reg_it9910.h"

//#define USE_WM8960_ADC

/* ************************************************************************** */
/* platform control */

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
u32 I2S_DA32_GET_RP(void);
u32 I2S_DA32_GET_WP(void);
void I2S_DA32_SET_WP(u32 WP32);

u32 I2S_AD32_GET_RP(void);
u32 I2S_AD32_GET_WP(void);
void I2S_AD32_SET_RP(u32 RP32);

static void _i2s_power_on(void);
static void _i2s_power_off(void);
static void _i2s_reset(void);
static void _i2s_aws_sync(void);
static void _i2s_set_clock(u32 demanded_sample_rate);
static void _i2s_enable_fading(u32 fading_step, u32 fading_duration);
static void _i2s_disable_fading(void);
static void _i2s_setup_io_direction_9910_only(STRC_I2S_SPEC *i2s_spec);

static int _i2s_DA_running = 0;
static int _i2s_AD_running = 0;

static pthread_mutex_t I2S_MUTEX = PTHREAD_MUTEX_INITIALIZER;


/* export APIs */
void i2s_volume_up(void);
void i2s_volume_down(void);
void i2s_pause_ADC(int pause);
void i2s_pause_DAC(int pause);
void i2s_deinit_ADC(void);
void i2s_deinit_DAC(void);
void i2s_init_DAC(STRC_I2S_SPEC *i2s_spec);
void i2s_init_ADC(STRC_I2S_SPEC *i2s_spec);
void i2s_mute_DAC(int mute);
int i2s_set_direct_volstep(unsigned volstep);
int i2s_set_direct_volperc(unsigned volperc);
unsigned i2s_get_current_volstep(void);
unsigned i2s_get_current_volperc(void);
void i2s_get_volstep_range(unsigned *max, unsigned *normal, unsigned *min);

/* ************************************************************************** */
u32 I2S_DA32_GET_RP(void)
{
	u16 dataLO;
	u16 dataHI;
	u32 data32;

    dataLO = reg_read16(I2S_REG_OBUF_RP_LO);
    dataHI = reg_read16(I2S_REG_OBUF_RP_HI);

    data32 = (dataHI << 16) | dataLO;
    return data32;
}
u32 I2S_DA32_GET_WP(void)
{
	u16 dataLO;
	u16 dataHI;
	u32 data32;

    dataLO = reg_read16(I2S_REG_OBUF_WP_LO);
    dataHI = reg_read16(I2S_REG_OBUF_WP_HI);

    data32 = (dataHI << 16) | dataLO;
    return data32;
}
void I2S_DA32_SET_WP(u32 WP32)
{
	/* must be low then hi for hw design */
	reg_write16(I2S_REG_OBUF_WP_LO, WP32 & 0xFFFF);
    reg_write16(I2S_REG_OBUF_WP_HI, WP32 >> 16   );
}

u32 I2S_AD32_GET_RP(void)
{
	u16 dataLO;
	u16 dataHI;
	u32 data32;

    dataLO = reg_read16(I2S_REG_IBUF_LO);
    dataHI = reg_read16(I2S_REG_IBUF_HI);

	data32 = (dataHI << 16) | dataLO;
    return data32;
}
u32 I2S_AD32_GET_WP(void)
{
	u16 dataLO;
	u16 dataHI;
	u32 data32;

    dataLO = reg_read16(I2S_REG_IBUF_LEN);
    dataHI = reg_read16(I2S_REG_IBUF_RP);

	data32 = (dataHI << 16) | dataLO;
    return data32;
}
void I2S_AD32_SET_RP(u32 RP32)
{
	reg_write16(I2S_REG_IBUF_LO, RP32 & 0xFFFF);
    reg_write16(I2S_REG_IBUF_HI, RP32 >> 16   );
}

/* ************************************************************************** */
static void _i2s_power_on(void)
{
	u16 reg_addr;
	u16 data16;

	if(_i2s_DA_running || _i2s_AD_running) {
		printf("I2S# power on already, DA:%d, AD:%d\n", _i2s_DA_running, _i2s_AD_running);
		return;
	}

	/* enable audio clock for I2S modules */
	/* NOTE: we SHOULD enable audio clock before toggling 0x003A/3C/3E */
	{
		reg_addr = 0x003E;
		data16 = reg_read16(reg_addr);

		data16 |= (1 << 1); /* enable AMCLK (system clock for wolfson chip) */
		data16 |= (1 << 3); /* enable ZCLK  (IIS DAC clock)                 */
		data16 |= (1 << 5); /* enable M8CLK (memory clock for DAC)          */
		data16 |= (1 << 7); /* enable M9CLK (memory clock for ADC)          */

		reg_write16(reg_addr, data16);
	}

	/* enable PLL2 for audio */
	{
		data16 = reg_read16(0x00C4);
		data16 &= ~(1 << 15); /* PLL2 power down, 0: Normal operation, 1: Power down */
		reg_write16(0x00C4, data16);

		reg_write16(0x00C4, 0x2000);
		do
		{
			data16 = reg_read16(0x00D4);
			i2s_delay_us(10);
		} while(((data16 >> 1) & 0x1) != 1);
	}

	printf("I2S# %s\n", __func__);
}

static void _i2s_power_off(void)
{
	u16 reg_addr;
	u16 data16;

	if(_i2s_DA_running || _i2s_AD_running) {
		printf("I2S# module still running, skip power-off, DA:%d, AD:%d\n", _i2s_DA_running, _i2s_AD_running);
		return;
	}

	/* disable audio clock for I2S modules */
	{
		reg_addr = 0x003E;
		data16 = reg_read16(reg_addr);

		data16 &= ~(1 << 1); /* disable AMCLK (system clock for wolfson chip) */
		data16 &= ~(1 << 3); /* disable ZCLK  (IIS DAC clock)                 */
		data16 &= ~(1 << 5); /* disable M8CLK (memory clock for DAC)          */
		data16 &= ~(1 << 7); /* disable M9CLK (memory clock for ADC)          */

		reg_write16(reg_addr, data16);
	}

	/* disable PLL2 for audio */
#if 0 /* FIXME: NEED MORE CHECK */
	{
		reg_addr = 0x00C4;
		data16 = reg_read16(reg_addr);
		data16 |= (1 << 15); /* PLL2 power down, 0: Normal operation, 1: Power down */
		reg_write16(reg_addr, data16);
	}
#endif

	printf("I2S# %s\n", __func__);
}

static void _i2s_reset(void)
{
	u16 data16;

	if(_i2s_DA_running) {
		printf("I2S# DAC is running, skip reset I2S !\n");
		return;
	}
	if(_i2s_AD_running) {
		printf("I2S# ADC is running, skip reset I2S !\n");
		return;
	}

	i2s_delay_us(1000); /* FIXME: dummy loop */;

	data16 = reg_read16(MMP_AUDIO_CLOCK_REG_3E);
	data16 |= (1 << 12);
	reg_write16(MMP_AUDIO_CLOCK_REG_3E, data16);

	i2s_delay_us(1000); /* FIXME: dummy loop */;

	data16 = reg_read16(MMP_AUDIO_CLOCK_REG_3E);
	data16 &= ~(1 << 12);
	reg_write16(MMP_AUDIO_CLOCK_REG_3E, data16);

	i2s_delay_us(1000); /* FIXME: dummy loop */;

	I2S_DA32_SET_WP(0);
	I2S_AD32_SET_RP(0);
	
	printf("I2S# %s\n", __func__);
}

static void _i2s_aws_sync(void)
{
	u16 data16;

	/* AWS & ZWS sync */
	data16 = reg_read16(I2S_REG_ADC_SAMPLE);
	data16 |= (1 << 15);
	reg_write16(I2S_REG_ADC_SAMPLE, data16);

	i2s_delay_us(1000); /* FIXME: dummy loop */;

	data16 &= ~(1 << 15);
	reg_write16(I2S_REG_ADC_SAMPLE, data16);
}

/***********************************************************************************
** Set CODEC Clock
** Assume the oversample rate is 256.
** Suggest system clock is (int)(CLK*2/256/sample_rate+1) / 2 * 256 * sample_rate

** For example:
** In 44100 sample rate, the system clock is 33MHz, the system clock should be
** (33000000*2/256/44100+1) / 2 * 256 * 44100 = 33868800.

** The PLL numurator is (int)(33868800 * 2 * 512 / 13000000 + 1) / 2 - 1 = 0x535.
***********************************************************************************/
static void _i2s_set_clock(u32 demanded_sample_rate)
{
	u32 target_sample_rate;
	u16 data16;
	u16 amclk_ratio;
	u16 zclk_ratio;
	int i;

	if(_i2s_DA_running) {
		printf("I2S# DAC is running, skip set clock !\n");
		return;
	}
	if(_i2s_AD_running) {
		printf("I2S# ADC is running, skip set clock !\n");
		return;
	}

	printf("I2S# %s, demanded_sample_rate: %u\n", __func__, demanded_sample_rate);

	if     ((demanded_sample_rate > 45600) && (demanded_sample_rate < 50400)) { target_sample_rate = 48000; }
	else if((demanded_sample_rate > 41895) && (demanded_sample_rate < 46305)) { target_sample_rate = 44100; }
	else if((demanded_sample_rate > 30400) && (demanded_sample_rate < 33600)) { target_sample_rate = 32000; }
	else if((demanded_sample_rate > 22800) && (demanded_sample_rate < 25200)) { target_sample_rate = 24000; }
	else if((demanded_sample_rate > 20900) && (demanded_sample_rate < 23100)) { target_sample_rate = 22050; }
	else if((demanded_sample_rate > 15200) && (demanded_sample_rate < 16800)) { target_sample_rate = 16000; }
	else if((demanded_sample_rate > 11400) && (demanded_sample_rate < 12600)) { target_sample_rate = 12000; }
	else if((demanded_sample_rate > 10450) && (demanded_sample_rate < 11550)) { target_sample_rate = 11025; }
	else if((demanded_sample_rate >  7600) && (demanded_sample_rate <  8400)) { target_sample_rate =  8000; }
	else
	{
		printf("ERROR# I2S, invalid demanded_sample_rate !\n");
		target_sample_rate = 48000;
	}

	printf("I2S# %s, target_sample_rate: %u\n", __func__, target_sample_rate);

	if((target_sample_rate == 48000) || (target_sample_rate == 24000) || (target_sample_rate == 12000))
	{
		/* Xtal: 12 MHz, 48000 Hz */
		/* A:4 B:16.5 C:00811 D:01 E:066 PLLo:12287997.159091 diff:-2.840909 osc:811.008000 */
		reg_write16(0x00C0, 0x9001);
		reg_write16(0x00C2, 0x4040); /* AMCLK select PLL3_OUT? */
		reg_write16(0x00C4, 0x2000);
		reg_write16(0x00C6, 0x0189);
		reg_write16(0x00C4, 0x3000);
		i2s_delay_us(100); /* Experts Recommend Value: 100us */
		reg_write16(0x00C4, 0x2000);

		if(target_sample_rate == 48000)
		{
			amclk_ratio = 1 - 1; /* AMCLK = 12.288MHz (256x48KHz)      */
			zclk_ratio  = 4 - 1; /* ZCLK  = 3.07 MHz (zclk ratio=0x1F) */
		}
		else if(target_sample_rate == 24000)
		{
			amclk_ratio = 2 - 1; /* AMCLK = 6.143 MHz */
			zclk_ratio  = 8 - 1; /* ZCLK  = 1.536 MHz */
		}
		else if(target_sample_rate == 12000)
		{
			amclk_ratio =  4 - 1; /* AMCLK = 3.072MHz */
			zclk_ratio  = 16 - 1; /* ZCLK  = 0.768MHz */
		}
		else
		{
			printf("ERROR# I2S, invalid target_sample_rate !\n");
			/* 48000 */
			amclk_ratio = 1 - 1; /* AMCLK = 12.288MHz (256x48KHz)      */
			zclk_ratio  = 4 - 1; /* ZCLK  = 3.07 MHz (zclk ratio=0x1F) */
		}
	}
	else if((target_sample_rate == 44100) || (target_sample_rate == 22050) || (target_sample_rate == 11025))
	{
		/* Xtal: 12 MHz, 44100 Hz */
		/* A:4 B:16.5 C:00408 D:01 E:071 PLLo:11289612.676056 diff:12.676056 osc:801.561600 */
		reg_write16(0x00C0, 0x9001);
		reg_write16(0x00C2, 0x4747); /* AMCLK select PLL3_OUT? */
		reg_write16(0x00C4, 0x2000);
		reg_write16(0x00C6, 0x3198);
		reg_write16(0x00C4, 0x3000);
		i2s_delay_us(100); /* Experts Recommend Value: 100us */
		reg_write16(0x00C4, 0x2000);

		if(target_sample_rate == 44100)
		{
			amclk_ratio = 1 - 1; /* AMCLK = 11.29MHz (256x44.1KHz)     */
			zclk_ratio  = 4 - 1; /* ZCLK  = 2.82MHz (zclk ratio=0x1F) */
		}
		else if(target_sample_rate == 22050)
		{
			amclk_ratio = 2 - 1; /* AMCLK = 5.645MHz */
			zclk_ratio  = 8 - 1; /* ZCLK  = 1.41 MHz */
		}
		else if(target_sample_rate == 11025)
		{
			amclk_ratio =  4 - 1; /* AMCLK = 2.82MHz */
			zclk_ratio  = 16 - 1; /* ZCLK  = 0.70MHz */
		}
		else
		{
			printf("ERROR# I2S, invalid target_sample_rate !\n");
			/* 44100 */
			amclk_ratio = 1 - 1; /* AMCLK = 11.29MHz (256x44.1KHz)     */
			zclk_ratio  = 4 - 1; /* ZCLK  = 2.82MHz (zclk ratio=0x1F) */
		}
	}
	else if((target_sample_rate == 32000) || (target_sample_rate == 16000) || (target_sample_rate == 8000))
	{
		/* Xtal: 12 MHz, 32000 Hz */
		/* A:4 B:16.5 C:00112 D:01 E:097 PLLo:8192010.309278 diff:10.309278 osc:794.624000 */
		reg_write16(0x00C0, 0x9001);
		reg_write16(0x00C2, 0x6161); /* AMCLK select PLL3_OUT? */
		reg_write16(0x00C4, 0x2000);
		reg_write16(0x00C6, 0x3070);
		reg_write16(0x00C4, 0x3000);
		i2s_delay_us(100); /* Experts Recommend Value: 100us */
		reg_write16(0x00C4, 0x2000);

		if(target_sample_rate == 32000)
		{
			amclk_ratio = 1 - 1; /* AMCLK = 8.192MHz (256x32KHz)       */
			zclk_ratio  = 4 - 1; /* ZCLK  = 2.048MHz (zclk ratio=0x1F) */
		}
		else if(target_sample_rate == 16000)
		{
			amclk_ratio = 2 - 1; /* AMCLK = 4.096MHz */
			zclk_ratio  = 8 - 1; /* ZCLK  = 1.024MHz */
		}
		else if(target_sample_rate == 8000)
		{
			amclk_ratio =  4 - 1; /* AMCLK = 2.048MHz */
			zclk_ratio  = 16 - 1; /* ZCLK  = 0.512MHz */
		}
		else
		{
			printf("ERROR# I2S@ invalid target_sample_rate !\n");
			/* 32000 */
			amclk_ratio = 1 - 1; /* AMCLK = 8.192MHz (256x32KHz)       */
			zclk_ratio  = 4 - 1; /* ZCLK  = 2.048MHz (zclk ratio=0x1F) */
		}
	}

	printf("I2S# %s, amclk_ratio: 0x%04x, zclk_ratio: 0x%04x\n", __func__, amclk_ratio, zclk_ratio);

	/* setup 0x003A/0x003C */
	{
		printf("I2S# %s, setup 0x003A/0x003C !\n", __func__);

		/* AMCLK divider settings */
		{
			/* PART 1: make sure the divider counter equals to zero */
			data16 = reg_read16(0x003A);
			data16 &= ~(1 << 15);
			reg_write16(0x003A, data16);

			data16 &= 0x7F80;
			reg_write16(0x003A, data16);

			data16 |= (1 << 15);
			reg_write16(0x003A, data16);

			for(i=0; i<100; i++) { asm(""); }
			i2s_delay_us(100);

			/* PART 2: hardware asynchronous control */
			data16 &= ~(1 << 15);
			reg_write16(0x003A, data16);

			/* bit 0 ~ 6, AMCLK divide ratio */
			data16 &= 0xFF80;
			data16 |= amclk_ratio;
			reg_write16(0x003A, data16);

			data16 |= (1 << 15);
			reg_write16(0x003A, data16);
		}

		/* ZCLK divider settings */
		{
			/* PART 1: make sure the divider counter equals to zero */
			data16 = reg_read16(0x003C);
			data16 &= ~(1 << 15);
			reg_write16(0x003C, data16);

			data16 &= 0x7C00;
			reg_write16(0x003C, data16);

			data16 |= (1 << 15);
			reg_write16(0x003C, data16);

			for(i=0; i<100; i++) { asm(""); }
			i2s_delay_us(100);

			/* PART 2: hardware asynchronous control */
			data16 &= ~(1 << 15);
			reg_write16(0x003C, data16);

			/* bit 0 ~ 9, ZCLK divide ratio */
			data16 &= 0xFC00;
			data16 |= zclk_ratio;
			reg_write16(0x003C, data16);

			data16 |= (1 << 15);
			reg_write16(0x003C, data16);
		}
	}

	/* dummy loop to wait clock divider stablized */
	i2s_delay_us(1000); /* FIXME: dummy loop */;
}

static void _i2s_enable_fading(u32 fading_step, u32 fading_duration)
{
	u16 data16;

	printf("I2S# %s\n", __func__);

	data16 = reg_read16(I2S_REG_FADE_IN_OUT);
	data16 |= (1 << 12);
	data16 |= ((fading_step & 0xF) << 8);
	data16 |= (fading_duration & 0xFF);
	reg_write16(I2S_REG_FADE_IN_OUT, data16);
}

static void _i2s_disable_fading(void)
{
	u16 data16;

	printf("I2S# %s\n", __func__);

	data16 = reg_read16(I2S_REG_FADE_IN_OUT);
	data16 &= ~(1 << 12);
	reg_write16(I2S_REG_FADE_IN_OUT, data16);
}

static void _i2s_setup_io_direction_9910_only(STRC_I2S_SPEC *i2s_spec)
{
	u16 data16;
	if(i2s_spec->use_hdmirx) /* NOTE: i2s played the slave role */
	{
		if(i2s_spec->internal_hdmirx)
		{
		    if (i2s_spec->use_hdmitx)
		        reg_write16(I2S_REG_IO_SET, 0x000C);
		    else
    			reg_write16(I2S_REG_IO_SET, 0x00BC);
    			
    	    reg_write16(I2S_REG_IOMUX_CTRL, 0x0000);

		}
		else /* ext. hdmirx */
		{
		    if (i2s_spec->use_hdmitx)
		        reg_write16(I2S_REG_IO_SET, 0x00BF);
		    else
    			reg_write16(I2S_REG_IO_SET, 0x00BF);
			
			reg_write16(I2S_REG_IO_SET, 0x0000);
		}
	}
	else /* no use hdmirx */
	{
		if(i2s_spec->slave_mode)
		{
			reg_write16(I2S_REG_IO_SET, 0x00BB);
			reg_write16(I2S_REG_IOMUX_CTRL, 0x0200); /* [9:8] pinZD0 select IO's DSD_I */
		}
		else /* i2s master (Wolfson & Cirrus slave mode) */
		{
			reg_write16(I2S_REG_IO_SET, 0x22B8);
#if defined (IT9919_144TQFP)
    #if (defined(REF_BOARD_AVSENDER) || defined(EVB_BOARD))
			reg_write16(I2S_REG_IOMUX_CTRL, 0x0000); /* [9:8] pinZD0 select IO's ZD0 */
    #endif			
#else    	
			reg_write16(I2S_REG_IOMUX_CTRL, 0x0000); /* [9:8] pinZD0 select IO's DSD_I */
#endif    			
		}
	}
	//disable no-use pin
	data16 = reg_read16(I2S_REG_IO_SET);
	data16 &= ~((1 << 5)|(1 << 7));
	reg_write16(I2S_REG_IO_SET, data16);
}

/* ************************************************************************** */
void i2s_volume_up(void)
{
	printf("I2S# %s\n", __func__);

	itp_codec_playback_amp_volume_up();
}

void i2s_volume_down(void)
{
	printf("I2S# %s\n", __func__);

	itp_codec_playback_amp_volume_down();
}

void i2s_pause_ADC(int pause)
{
	u16 data16;

	printf("I2S# %s(%d)\n", __func__, pause);

	if(pause)
	{
		data16 = reg_read16(I2S_REG_INPUT_CTL);
		data16 &= ~(1 << 0);
		reg_write16(I2S_REG_INPUT_CTL, data16);
	}
	else /* resume */
	{
		data16 = reg_read16(I2S_REG_INPUT_CTL);
		data16 |= (1 << 0);
		reg_write16(I2S_REG_INPUT_CTL, data16);		
	}
}

void i2s_pause_DAC(int pause)
{
	u16 data16;

	printf("I2S# %s(%d)\n", __func__, pause);

	if(pause)
	{
		//_i2s_enable_fading(0xF, 0x01); /* fast response */
		reg_write16(I2S_REG_DIG_VOL, 0x0000);

		data16 = reg_read16(I2S_REG_OUTPUT_CTL2);
		data16 &= ~(1 << 0);
		reg_write16(I2S_REG_OUTPUT_CTL2, data16);
	}
	else /* resume */
	{
		//_i2s_enable_fading(0x1, 0xFF); /* slow response */
		reg_write16(I2S_REG_DIG_VOL, 0xFF00);

		data16 = reg_read16(I2S_REG_OUTPUT_CTL2);
		data16 |= (1 << 0);
		reg_write16(I2S_REG_OUTPUT_CTL2, data16);
	}
}

void i2s_deinit_ADC(void)
{
	u16 data16;

	if(0 == _i2s_AD_running) {
		printf("I2S# ADC is 'NOT' running, skip deinit ADC !\n");
		return;
	}

	printf("I2S# %s\n", __func__);
	
	pthread_mutex_lock(&I2S_MUTEX);
	/* disable hardware I2S */
	{
		data16 = reg_read16(I2S_REG_INPUT_CTL);
		data16 &= ~(3 << 0);
		reg_write16(I2S_REG_INPUT_CTL, data16);
	}

//#ifdef USE_WM8960_ADC
    //if (bResetDevice)
    {
	itp_codec_rec_deinit();
	}
//#endif

	_i2s_AD_running = 0; /* put before _i2s_reset() */
	_i2s_reset();

	_i2s_power_off();
	pthread_mutex_unlock(&I2S_MUTEX);
}

void i2s_deinit_DAC(void)
{
	u16 data16;
	u16 out_status_1;
	u16 out_status_2;
	u32 i2s_idle;
	u32 i2s_memcnt;

	pthread_mutex_lock(&I2S_MUTEX);

	if(0 == _i2s_DA_running) {
		printf("I2S# DAC is 'NOT' running, skip deinit DAC !\n");
		pthread_mutex_unlock(&I2S_MUTEX);
		return;
	}

	printf("I2S# %s +\n", __func__);

	itp_codec_playback_deinit();

	/* disable I2S_OUT_FIRE & I2S_OUTPUT_EN */
	data16 = reg_read16(I2S_REG_OUTPUT_CTL2);
	data16 &= ~(0x3);
	reg_write16(I2S_REG_OUTPUT_CTL2, data16);

	do
	{
		i2s_delay_us(1000); /* FIXME: dummy loop */;

		out_status_1 = reg_read16(I2S_REG_OUTPUT_STATUS1);
		out_status_2 = reg_read16(I2S_REG_OUTPUT_STATUS2);

//		printf("I2S# OUTPUT_STATUS 1,2: 0x%04x,0x%04x\n", out_status_1, out_status_2);

		i2s_idle   =  out_status_1 & 0x1;
		i2s_memcnt = (out_status_2 >> 6) & 0x1F;
	} while(!i2s_idle || (i2s_memcnt != 0));

	_i2s_DA_running = 0; /* put before _i2s_reset() */
	_i2s_reset();

	_i2s_power_off();

	printf("I2S# %s -\n", __func__);
	pthread_mutex_unlock(&I2S_MUTEX);
}

void i2s_init_DAC(STRC_I2S_SPEC *i2s_spec)
{
	int param_fail = 0;
	u16 data16;
//	u32 data32;
	u8 resolution_type;

	pthread_mutex_lock(&I2S_MUTEX);

	if(_i2s_DA_running) {
		printf("I2S# DAC is running, skip re-init !\n");
		pthread_mutex_unlock(&I2S_MUTEX);
		return;
	}

	printf("I2S# %s +\n", __func__);

	/* check buffer base/size alignment */
	if((u32)i2s_spec->base_out_i2s_spdif % 8) { param_fail = 1; }
	if(i2s_spec->buffer_size % 8) { param_fail = 4; }

	/* check channels number */
	if((i2s_spec->channels != 1) && (i2s_spec->channels != 2)) { param_fail = 5; }

	if(param_fail) {
		printf("ERROR# I2S, param_fail = %d\n", param_fail);
		pthread_mutex_unlock(&I2S_MUTEX);
		return;
	}

//	data32 = I2S_DA32_GET_WP();
//	if(data32 != 0) {
//		printf("ERROR# I2S, DA32-WP (%u)\n", data32);
//		return;
//	}

	_i2s_power_on();

	_i2s_set_clock(i2s_spec->sample_rate);
	_i2s_reset();	
	itp_codec_set_i2s_sample_rate(i2s_spec->sample_rate);
	_i2s_DA_running = 1;

	/* mastor mode & AWS & ZWS sync */
	switch(i2s_spec->sample_size) {
		case  8: { resolution_type = 3; break; }
		case 16: { resolution_type = 0; break; }
		case 24: { resolution_type = 1; break; }
		case 32:
		default: { resolution_type = 2; break; }
	}

	{
		u16 data16_tmp;
		data16_tmp = reg_read16(I2S_REG_ADC_SAMPLE);
		data16_tmp &= ~(0x1FF << 0); /* [8:0] B0AWSRatio */
		data16_tmp |= (0x1F << 0);   /* sample width (in bits unit) */
		reg_write16(I2S_REG_ADC_SAMPLE, data16_tmp); /* zclk ratio=0x1F */
	}

	data16 = (1             << 13) /* DAC device selection, 0:External DAC; 1:Internal DAC */
	| (((i2s_spec->slave_mode) ? 1 : 0) << 12) /* 0:Master(ZCLK/ZWS output mode); 1:Slave(ZCLK/ZWS input mode) */
	| (resolution_type      << 10) /* DAC resolution bits, 00:16bits; 01:24bits; 10:32bits */
	| (0x1F                 <<  0); /* sample width (in bits unit) */

	reg_write16(I2S_REG_DAC_SAMPLE, data16);

	/* Big Endian Type (for IIS Input and IIS Output use) */
	if(i2s_spec->is_big_endian)
	{
		if((24 == i2s_spec->sample_size) || (32 == i2s_spec->sample_size))
		{
			data16 = reg_read16(I2S_REG_ADDA_PCM);
			data16 |= (1 << 3);
			reg_write16(I2S_REG_ADDA_PCM, data16);
		}
		else
		{
			data16 = reg_read16(I2S_REG_ADDA_PCM);
			data16 &= ~(1 << 3);
			reg_write16(I2S_REG_ADDA_PCM, data16);
		}
	}

	/* I2S I/O direction for 9910 only */
	_i2s_setup_io_direction_9910_only(i2s_spec);

	/* I2S Engine IO Mode for 9910 only */
	/* DA only: REG[0x1644] B'7 = 0 ; AD only: REG[0x1644] B'7 = 1 */
	{
		data16 = reg_read16(I2S_REG_ADDA_PCM);
		data16 &= ~(1 << 7);
		reg_write16(I2S_REG_ADDA_PCM, data16);
	}

	_i2s_aws_sync();

	itp_codec_playback_init(2);

	/* buffer base */
	{
		/* buf 1 (IIS/SPDIF) */
		reg_write16(I2S_REG_OBUF_1_LO, (u32)i2s_spec->base_i2s & 0xFFFF);
		reg_write16(I2S_REG_OBUF_1_HI, (u32)i2s_spec->base_i2s >> 16);

		/* SPDIF data channel select */
		{
			data16 = reg_read16(I2S_REG_SPDIF_VOL);
			data16 &= ~(1 << 14);
			reg_write16(I2S_REG_SPDIF_VOL, data16);
		}
	}

	/* buffer length */
	reg_write16(I2S_REG_OBUF_LEN_LO, (i2s_spec->buffer_size - 1) & 0xFFFF); /* NOTE: minus one for hardware design */
	reg_write16(I2S_REG_OBUF_LEN_HI, (i2s_spec->buffer_size - 1) >> 16);

	/* DA interrupt threshold: (value = remnant data) to starvation */
	reg_write16(I2S_REG_OUTPUT_GAP_LO, 0x0040); /* TODO: need reasonable threshold */
	reg_write16(I2S_REG_OUTPUT_GAP_HI, 0x0002);

	/* output path */
	{
		reg_write16(I2S_REG_OUTPUT_CTL,
		( (0 << 15) /* Probe signal select[1] */
		| (0 << 14) /* Probe signal select[0] */
		| (0 << 13) /* Probe high 32bit and low 32bit swap */
		| (1 << 12) /* Enable Probe signal debug */
		| (1 <<  8) /* Enable DA RdWrGap Interrupt */
		| (1 <<  0) /* Enable IIS/SPDIF Data Output */
		));
	}

#ifdef ENABLE_ITV_AUDIO_BIST_MODE
	/* HDMI audio Functional Test */
	reg_write16(0x1676, 0x0000); /* SynWordL */
	reg_write16(0x1678, 0x0000); /* SynWordR */
	reg_write16(0x167A, 0x0011); /* step_EnHDMI */
#endif

	reg_write16(I2S_REG_OUTPUT_CTL2,
	(  0                                 << 6) /* Output Channel active level, 0:Low for Left; 1:High for Left */
	| (0                                 << 5) /* Output Interface Format, 0:IIS Mode; 1:MSB Left-Justified Mode */
	| ((i2s_spec->channels - 1)          << 4) /* Output Channel Select, 0:Single Channel; 1:Two Channels */
	| ((i2s_spec->is_big_endian ? 1 : 0) << 3) /* 0:Little Endian; 1:Big Endian */
	| (0                                 << 2) /* 0:NOT_LAST_WPTR; 1:LAST_WPTR */
	| (1                                 << 1) /* Fire the IIS for Audio Output */
	);

	i2s_delay_us(1000); /* FIXME: dummy loop */;

	if(!i2s_spec->postpone_audio_output) {
		i2s_pause_DAC(0); /* Enable Audio Output */
	}

	printf("I2S# %s -\n", __func__);
	pthread_mutex_unlock(&I2S_MUTEX);
	
}

void i2s_init_ADC(STRC_I2S_SPEC *i2s_spec)
{
	int i;
	u16 data16;
	u16 data16_tmp;
	u8 resolution_type;
    u16 chipVer = 0;

    chipVer = ithReadRegH(0x4); 
	if(_i2s_AD_running) {
		printf("I2S# ADC is running, skip re-init ADC !\n");
		return;
	}

	printf("I2S# %s +\n", __func__);


	if(((u32)i2s_spec->base_in_i2s % 8) || (i2s_spec->buffer_size % 8)) {
		printf("ERROR# I2S, bufbase/bufsize must be 8-Bytes alignment !\n");
		return;
	}
	if((i2s_spec->channels != 1) && (i2s_spec->channels != 2)) {
		printf("ERROR# I2S, only support single or two channels !\n");
		return;
	}
	pthread_mutex_lock(&I2S_MUTEX);
	_i2s_power_on();

	_i2s_set_clock(i2s_spec->sample_rate);
	_i2s_reset();	
	itp_codec_set_i2s_sample_rate(i2s_spec->sample_rate);
	_i2s_AD_running = 1;

	/* mastor mode & AWS & ZWS sync */
	switch(i2s_spec->sample_size) {
		case  8: { resolution_type = 3; break; }
		case 16: { resolution_type = 0; break; }
		case 24: { resolution_type = 1; break; }
		case 32:
		default: { resolution_type = 2; break; }
	}

	/* config AD */
	{
		unsigned short B14;

		B14 = (i2s_spec->use_hdmirx && i2s_spec->internal_hdmirx && i2s_spec->slave_mode) ? 1 : 0;

		data16 = (B14                       << 14) /* CLK/WS/DATA1~4 from internal HDMI RX */
		| (0                                << 13) /* 0:Ext. ADC; 1:Internal ADC */
		| (((i2s_spec->slave_mode) ? 1 : 0) << 12) /* hardware not use this bit */
		| (resolution_type                  << 10) /* ADC resolution, 00:16bit; 01:24bit; 10:32bit */
		| (0x1F                             <<  0); /* sample width (in bits unit) */

		reg_write16(I2S_REG_ADC_SAMPLE, data16);

		data16_tmp = reg_read16(I2S_REG_DAC_SAMPLE);
		data16_tmp &= ~(1 << 12);
		data16_tmp |= (data16 & 0x1000);
		data16_tmp &= ~(0x1FF << 0); /* [8:0] B0ZWSRatio */
		data16_tmp |= (0x1F);        /* sample width (in bits unit) */		
		reg_write16(I2S_REG_DAC_SAMPLE, data16_tmp); /* set 'master or slave mode' here ! */

		/* Big Endian Type (for IIS Input and IIS Output use) */
		if(i2s_spec->is_big_endian)
		{
			if((24 == i2s_spec->sample_size) || (32 == i2s_spec->sample_size))
			{
				data16 = reg_read16(I2S_REG_ADDA_PCM);
				data16 |= (1 << 3);
				reg_write16(I2S_REG_ADDA_PCM, data16);
			}
			else
			{
				data16 = reg_read16(I2S_REG_ADDA_PCM);
				data16 &= ~(1 << 3);
				reg_write16(I2S_REG_ADDA_PCM, data16);
			}
		}
	}

	/* I2S I/O direction for 9910 only */
	_i2s_setup_io_direction_9910_only(i2s_spec);

	/* I2S Engine IO Mode for 9910 only */
	/* DA only: REG[0x1644] B'7 = 0 ; AD only: REG[0x1644] B'7 = 1 */
	{		
		data16 = reg_read16(I2S_REG_ADDA_PCM);
		data16 |= (1 << 7);
		reg_write16(I2S_REG_ADDA_PCM, data16);
	}

	_i2s_aws_sync();

//#ifdef USE_WM8960_ADC
	if     ( i2s_spec->from_LineIN && !i2s_spec->from_MIC_IN) { itp_codec_rec_init(1); } /* LineIN only */
	else if(!i2s_spec->from_LineIN &&  i2s_spec->from_MIC_IN) { itp_codec_rec_init(2); } /* MICIN only */
	else                                                      { itp_codec_rec_init(0); } /* both LineIN & MICIN */
//#endif

	/* buffer base */
	{
        if (chipVer == 1)
        {
    		reg_write16(I2S_REG_BASE_IN5_LO, (u32)i2s_spec->base_in_i2s & 0xFFFF);
    		reg_write16(I2S_REG_BASE_IN5_HI, (u32)i2s_spec->base_in_i2s >> 16);
	    }
	    else
	    {
    		reg_write16(I2S_REG_BASE_IN1_LO, (u32)i2s_spec->base_in_i2s & 0xFFFF);
    		reg_write16(I2S_REG_BASE_IN1_HI, (u32)i2s_spec->base_in_i2s >> 16);
	    }
	}

	/* buffer length */
	reg_write16(I2S_REG_IN_LEN_LO, (i2s_spec->buffer_size - 1) & 0xFFFF); /* NOTE: minus one for hardware design */
	reg_write16(I2S_REG_IN_LEN_HI, (i2s_spec->buffer_size - 1) >> 16   ); /* NOTE: minus one for hardware design */

	/* AD interrupt threshold: (value = available space) to full */
	reg_write16(I2S_REG_IN_GAP_LO, 0x0040); /* TODO: need reasonable threshold */
	reg_write16(I2S_REG_IN_GAP_HI, 0x0002);

    if (chipVer == 1)
    {
    	reg_write16(I2S_REG_INPUT_CTL2,
    	( (0 << 9) /* 0:AD no loopback; 1:Buf1, 7:All loopback(Buf1~5) */
    	| (1 << 8) /* Enable AD RdWrGap Interrupt */
    	| ((i2s_spec->use_hdmirx & 0x1) << 3) /* Enable HDMI RX Data3 Input */
    	| ((i2s_spec->use_hdmirx & 0x1) << 2) /* Enable HDMI RX Data2 Input */
    	| ((i2s_spec->use_hdmirx & 0x1) << 1) /* Enable HDMI RX Data1 Input */
    	| ((i2s_spec->use_hdmirx & 0x1) << 0) /* Enable HDMI RX Data0 Input */
    	| (((!i2s_spec->use_hdmirx) & 0x1) << 4) /* Enable IIS Data Input */
    	));
    }
    else
    {
    	reg_write16(I2S_REG_INPUT_CTL2,
    	( (0 << 9) /* 0:AD no loopback; 1:Buf1, 7:All loopback(Buf1~5) */
    	| (1 << 8) /* Enable AD RdWrGap Interrupt */
    	| ((i2s_spec->use_hdmirx & 0x1) << 4) /* Enable HDMI RX Data3 Input */
    	| ((i2s_spec->use_hdmirx & 0x1) << 3) /* Enable HDMI RX Data2 Input */
    	| ((i2s_spec->use_hdmirx & 0x1) << 2) /* Enable HDMI RX Data1 Input */
    	| ((i2s_spec->use_hdmirx & 0x1) << 1) /* Enable HDMI RX Data0 Input */
    	| (((!i2s_spec->use_hdmirx) & 0x1) << 0) /* Enable IIS Data Input */
    	));
    }
	reg_write16(I2S_REG_INPUT_CTL,
	(  0                                 << 6) /* Input Channel active level, 0:Low for Left; 1:High for Left */
	| (0                                 << 5) /* Input Interface Format, 0:IIS Mode; 1:MSB Left-Justified Mode */
	| ((i2s_spec->channels - 1)          << 4) /* Input Channel, 0:Single Channel; 1:Two Channels */
	| ((i2s_spec->is_big_endian ? 1 : 0) << 3) /* 0:Little Endian; 1:Big Endian */
	| ((i2s_spec->record_mode ? 1 : 0)   << 2) /* 0:AV Sync Mode (wait capture to start); 1:Only Record Mode */
	| (1                                 << 1) /* Fire the IIS for Audio Input */
	| (1                                 << 0) /* Enable Audio Input */
	);

//	i2s_pause_ADC(0); /* FIXME: TODO: Enable Audio input */

	printf("I2S# %s -\n", __func__);
	pthread_mutex_unlock(&I2S_MUTEX);
}

void i2s_mute_DAC(int mute)
{
	printf("I2S# %s(%d)\n", __func__, mute);

	if(mute)
	{
		_i2s_enable_fading(0xF, 0x01); /* fast response */
		reg_write16(I2S_REG_DIG_VOL, 0x0000);
	}
	else /* resume */
	{
		_i2s_enable_fading(0x1, 0xFF); /* slow response */
		reg_write16(I2S_REG_DIG_VOL, 0xFF00);
	}
}

int i2s_set_direct_volstep(unsigned volstep)
{
	itp_codec_playback_set_direct_vol(volstep);
	return 0;
}

int i2s_set_direct_volperc(unsigned volperc)
{
	itp_codec_playback_set_direct_volperc(volperc);
	return 0;
}

unsigned i2s_get_current_volstep(void)
{
	unsigned currvol = 0;
	itp_codec_playback_get_currvol(&currvol);
	return currvol;
}

unsigned i2s_get_current_volperc(void)
{
	unsigned currvolperc = 0;
	itp_codec_playback_get_currvolperc(&currvolperc);
	return currvolperc;
}

void i2s_get_volstep_range(unsigned *max, unsigned *normal, unsigned *min)
{
	itp_codec_playback_get_vol_range(max, normal, min);
}

int i2s_ADC_set_direct_volstep(unsigned volstep)
{	
	itp_codec_rec_set_direct_vol(volstep);
	return 0;
}

unsigned i2s_ADC_get_current_volstep(void)
{
	unsigned currvol = 0;
	itp_codec_rec_get_currvol(&currvol);
	return currvol;
}

void i2s_ADC_get_volstep_range(unsigned *max, unsigned *normal, unsigned *min)
{
	itp_codec_rec_get_vol_range(max, normal, min);
}

void i2s_mute_ADC(int mute)
{
	printf("I2S# %s(%d)\n", __func__, mute);

	if(mute) { itp_codec_rec_mute(); }
	else     { itp_codec_rec_unmute(); }

}



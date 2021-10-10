/* sy.chuang, 2012-0423, ITE Tech. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "ite/ith.h"
#include "ite/itp.h"

#include "i2s/i2s.h"
#include "i2s_reg.h"

/* ************************************************************************** */
/* platform control */

#define ENABLE_192KHZ_SAMPLE_RATE

//#define ENABLE_ITV_AUDIO_BIST_MODE

#define I2S_MASTER_MODE

/*******************************************************
   It's for workaround the reset I2S engine issue.
   and it will take 1~500 ms as the price.        
********************************************************/
//#define ENABLE_FORCE_RESET_RW_POINTER

//#define I2S_DEBUG_SET_CLOCK
#define I2S_DEBUG_DEINIT_DAC_COST

#define DA_PTR_SIZE_WIDTH   32
#define AD_PTR_SIZE_WIDTH   16 /* 16:9070; 32:9910 */

/* wrapper */
static inline unsigned short reg_read16(unsigned short addr16) { return ithReadRegH(addr16); }
static inline void reg_write16(unsigned short addr16, unsigned short data16) { ithWriteRegH(addr16, data16); }
static inline void i2s_delay_us(unsigned us) { ithDelay(us); }

/* ************************************************************************** */
#define SERR() do { printf("ERROR# %s:%d, %s\n", __FILE__, __LINE__, __func__); while(1); } while(0)
#define S()    do { printf("=> %s:%d, %s\n",     __FILE__, __LINE__, __func__);           } while(0)
#define TV_CAL_DIFF_MS(TIME2, TIME1) ((((TIME2.tv_sec * 1000000UL) + TIME2.tv_usec) - ((TIME1.tv_sec * 1000000UL) + TIME1.tv_usec)) / 1000)

#ifdef _WIN32
    #define asm()
#endif

typedef signed   long long s64;
typedef signed   int       s32;
typedef signed   short     s16;
typedef signed   char      s8;
typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned short     u16;
typedef unsigned char      u8;

/* ************************************************************************** */
#if (DA_PTR_SIZE_WIDTH == 32)
u32 I2S_DA32_GET_RP(void);
u32 I2S_DA32_GET_WP(void);
void I2S_DA32_SET_WP(u32 data32);
void I2S_DA32_SET_LAST_WP(u32 data32);
#endif

#if (AD_PTR_SIZE_WIDTH == 16)
u16 I2S_AD16_GET_RP(void);
u16 I2S_AD16_GET_WP(void);
void I2S_AD16_SET_RP(u16 data16);
#endif

u16 I2S_DA_STATUS1(void);
u16 I2S_AD_STATUS(void);
u32 i2s_is_DAstarvation(void);

static void _i2s_power_on(void);
static void _i2s_power_off(void);
static void _i2s_reset(void);

#ifdef	ENABLE_FORCE_RESET_RW_POINTER
static void _i2s_reset_AD_RWptr(void);
static void _i2s_reset_DA_RWptr(void);
#endif

static void _i2s_aws_sync(void);
static void _i2s_set_clock(u32 demanded_sample_rate);
static void _i2s_enable_fading(u32 fading_step, u32 fading_duration);
static void _i2s_disable_fading(void);
static void _i2s_use_GPIO_MODE2(void); /* GPIO settings for CFG2 */

static int _g_UseGpioMode2 	= 0;
static int _g_UseSpdif 		= 0;
static int _g_UseOutputPin 	= 0;

//static	STRC_I2S_SPEC	g_curr_DA_Spec;
//static	STRC_I2S_SPEC	g_curr_AD_Spec;
static int _i2s_DA_running = 0;
static int _i2s_AD_running = 0;
static int _i2s_DA_mute = 0;
static pthread_mutex_t I2S_MUTEX = PTHREAD_MUTEX_INITIALIZER;

/* export APIs */
void i2s_CODEC_standby(void);
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
void i2s_set_linein_bypass(int bypass);

/* FIXME: SPDIF */
static void _init_spdif(u32 sample_rate);
static void _deinit_spdif(void);
static void _show_i2s_spec(STRC_I2S_SPEC *i2s_spec);
/* ************************************************************************** */
#if (DA_PTR_SIZE_WIDTH == 32)
u32 I2S_DA32_GET_RP(void)
{
	unsigned short dataLO;
	unsigned short dataHI;
	unsigned data32;

	/* must be low then hi for hw design */
	dataLO = reg_read16(I2S_REG_OBUF_RP_LO);
	dataHI = reg_read16(I2S_REG_OBUF_RP_HI);

	data32 = (dataHI << 16 ) | dataLO;
	return data32;
}
u32 I2S_DA32_GET_WP(void)
{
	u16 dataLO;
	u16 dataHI;
	u32 data32;

	/* must be low then hi for hw design */
	dataLO = reg_read16(I2S_REG_OBUF_WP_LO);
	dataHI = reg_read16(I2S_REG_OBUF_WP_HI);

	data32 = (dataHI << 16 ) | dataLO;
	return data32;
}
void I2S_DA32_SET_WP(u32 data32)
{
	/* check if enable the "last write pointer" function. */
	if( reg_read16(I2S_REG_OUTPUT_CTL2)&0x4 )
	{		
		u32 WPtr=I2S_DA32_GET_WP();
		
		if( WPtr&0x7 )	printf("I2S# waring, WP is not 8B alignment.\n");
		else
		{
			u32 cnt=0;
			/* if enable, then wait the RW=WP, */
			while( I2S_DA32_GET_RP() != WPtr )
			{
				if(cnt++>1000)	break;
				usleep(1000);
			}
			if(cnt++>1000)	printf("I2S# waring, wait for RP=WP timeout.\n");
		}		

		/* Disable this function,and then set WP */
		ithWriteRegMaskH(I2S_REG_OUTPUT_CTL2, 0, 1<<2);
	}
	
	/* must be low then hi for hw design */
	reg_write16(I2S_REG_OBUF_WP_LO, data32 & 0xFFFF);
	reg_write16(I2S_REG_OBUF_WP_HI, data32 >> 16);
}
void I2S_DA32_SET_LAST_WP(u32 data32)
{
	printf("set last WP!!\n");
	/* Enable the last pointer function */
	ithWriteRegMaskH(I2S_REG_OUTPUT_CTL2, 1<<2, 1<<2);
	
	/* must be low then hi for hw design */
	reg_write16(I2S_REG_OBUF_WP_LO, data32 & 0xFFFF);
	reg_write16(I2S_REG_OBUF_WP_HI, data32 >> 16);
}

void I2S_DA32_WAIT_RP_EQUAL_WP(void)
{
	/* Enable the last pointer function */
	ithWriteRegMaskH(I2S_REG_OUTPUT_CTL2, 1 << 2, 1 << 2);
	if (reg_read16(I2S_REG_OUTPUT_CTL2) & 0x4)
	{
		u32 WPtr = I2S_DA32_GET_WP();

		if (WPtr & 0x7) WPtr -= WPtr & 0x7;

		{
			u32 cnt = 0;
			/* if enable, then wait the RW=WP, */
			while (I2S_DA32_GET_RP() != WPtr)
			{
				if (cnt++>1000)	break;
				usleep(1000);
			}
			if (cnt++>1000)	printf("I2S# waring, wait for RP=WP timeout.\n");
		}

		/* Disable this function,and then set WP */
		ithWriteRegMaskH(I2S_REG_OUTPUT_CTL2, 0, 1 << 2);
	}

}
#else
#	error "ERROR# only support DA_PTR_SIZE_WIDTH == 32 BITS !"
#endif

#if (AD_PTR_SIZE_WIDTH == 16)
u16 I2S_AD16_GET_RP(void) { return reg_read16(I2S_REG_IBUF_RP); }
u16 I2S_AD16_GET_WP(void) { return reg_read16(I2S_REG_IBUF_WP); }
void I2S_AD16_SET_RP(u16 data16) { reg_write16(I2S_REG_IBUF_RP, data16); }
#elif(AD_PTR_SIZE_WIDTH == 32)
#	error "ERROR# not yet implement AD_PTR_SIZE_WIDTH == 32 BITS !"
#else
#	error "ERROR# only support AD_PTR_SIZE_WIDTH == (16 or 32) BITS !"
#endif

u16 I2S_DA_STATUS1(void) { return reg_read16(I2S_REG_OUTPUT_STATUS1); }
u16 I2S_AD_STATUS(void) { return reg_read16(I2S_REG_INPUT_STATUS); }
u32 i2s_is_DAstarvation(void)
{
	u16 status_out = reg_read16(I2S_REG_OUTPUT_STATUS1);
	return (status_out >> 1) & 0x1;
}

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
		data16 = reg_read16(0x00B4);
		data16 &= ~(1 << 15); /* PLL2 power down, 0: Normal operation, 1: Power down */
		reg_write16(0x00B4, data16);

		reg_write16(0x00B4, 0x2000);
		do
		{
			data16 = reg_read16(0x00D4);
			ithDelay(10); /* unit: micro-sec (us) */
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
		reg_addr = 0x00B4;
		data16 = reg_read16(reg_addr);
		data16 |= (1 << 15); /* PLL2 power down, 0: Normal operation, 1: Power down */
		reg_write16(reg_addr, data16);
	}
#endif

	printf("I2S# %s\n", __func__);
}

/*
 These 2 functions are for workaround the reset I2S engine issue.
 and it will take 1~500 ms as the price.
*/
#ifdef	ENABLE_FORCE_RESET_RW_POINTER
static void _i2s_reset_AD_RWptr(void)
{
	u32	buf_size = 0;
	u32	buf_base = 0;
	u32	cnt = 0;
	u16	AD_r = 0;
	u16	AD_w = 0;
	
	/* buffer length */
	buf_size = (u32)reg_read16(I2S_REG_IBUF_LEN) + 1;	 /* NOTE: add one for hardware design */	
	if(buf_size<512)
	{
		printf("I2S#  WARNING: AD reset: buf_size(%x)<512 !!\n",buf_size);
		return;
	}
	
	/* buffer base */
	buf_base = (((u32)reg_read16(I2S_REG_IBUF_HI)<<16)&0xFFFF0000) | ((u32)reg_read16(I2S_REG_IBUF_LO))&0x0000FFFF;

	if(!buf_base)
	{
		printf("I2S#  WARNING: AD reset: buf_base=0!!\n");
		return;
	}
	
	/* get DA RP & WP */
	AD_r = I2S_AD16_GET_RP();
	AD_w = I2S_AD16_GET_WP();
	
	/* AD_w=0 || AD_r=0, then skip reset flow */
	if(!AD_w && !AD_r)
	{
		printf("I2S#  WARNING: AD_w(%x) or AD_r(%x) is 0!!\n",AD_w,AD_r);
		return;
	}
	
	/* set buf as 0 (No Necessary)*/
	//memset((void*)(buf_base+AD_r), 0, (buf_size-AD_r) );
	
	/* set RP=0 */ 
	if( (buf_size-AD_w) > 64 )	I2S_AD16_SET_RP(buf_size-32);
	
	I2S_AD16_SET_RP(0);
	
	while(1)
	{
		if(I2S_AD16_GET_WP()==0)	break;
		
		if(cnt++ > 500)
		{	
			printf("I2S# ERROR: can NOT reset AD R/W pointer,RP=%x,WP=%x,cnt=%d\n",I2S_AD16_GET_RP(),I2S_AD16_GET_WP(),cnt);
			while(1);
		}
		usleep(10000);
	}
	
	printf("I2S# reset AD R/W pointer success!![%x,%x],takes %d ms]\n",I2S_AD16_GET_RP(),I2S_AD16_GET_WP(),cnt*10);
}

static void _i2s_reset_DA_RWptr(void)
{
	u32	buf_size = 0;
	u32	buf_base = 0;
	u32	cnt = 0;
	u32	DA_r = 0;
	u32	DA_w = 0;
	
	/* buffer length */
	buf_size = (((u32)reg_read16(I2S_REG_OBUF_LEN_HI)<<16)&0xFFFF0000) | ((u32)reg_read16(I2S_REG_OBUF_LEN_LO))&0x0000FFFF;	
	buf_size +=1;
	
	/* buffer base */
	buf_base = (((u32)reg_read16(I2S_REG_OBUF_1_HI)<<16)&0xFFFF0000) | ((u32)reg_read16(I2S_REG_OBUF_1_LO))&0x0000FFFF;
	
	if( (buf_size<512) || !buf_base )
	{
		printf("I2S# WARNING: buf_size(%x) or buf_base(%x) is 0!!\n",buf_size,buf_base);
		return;
	}
	
	printf("I2S# do reset DA ptr, buf=%x,size=%x\n",buf_base,buf_size);
	
	/* get DA RP & WP */
	DA_r = I2S_DA32_GET_RP();
	DA_w = I2S_DA32_GET_WP();
	
	/* DA_w=0 || DA_r=0, then skip reset flow */
	if(!DA_w && !DA_r)
	{
		printf("I2S# WARNING: DA_w(%x) or DA_r(%x) is 0!!\n",DA_w,DA_r);
		return;
	}
	
	printf("I2S# do reset DA ptr, DA_r=%x,DA_w=%x\n",DA_r,DA_w);	
	
	/* set buf as 0*/
	memset((void*)(buf_base+DA_r), 0, (buf_size-DA_r) );	
	
	/* set WP=0 */ 
	if( (buf_size-DA_r) > 128 )	I2S_DA32_SET_WP(buf_size-64);
	//printf("	** DA_r=%x, DA_w=%x**\n",I2S_DA32_GET_RP(),I2S_DA32_GET_WP());
	I2S_DA32_SET_WP(0);
	//printf("	*2 DA_r=%x, DA_w=%x**\n",I2S_DA32_GET_RP(),I2S_DA32_GET_WP());
	
	while(1)
	{
		if(I2S_DA32_GET_RP()==0)	break;
		//printf("	** DA_r=%x **\n",I2S_DA32_GET_RP());
		
		if(cnt++ > 500)
		{	
			printf("I2S ERROR: can NOT reset DA R/W pointer,RP=%x,WP=%x,cnt=%d\n",I2S_DA32_GET_RP(),I2S_DA32_GET_WP(),cnt);
			while(1);
		}
		usleep(1000);
	}
	
	printf("I2S# reset DA R/W pointer success!![%x,%x],takes %d ms]\n",I2S_DA32_GET_RP(),I2S_DA32_GET_WP(),cnt*10);
}
#endif

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

	i2s_delay_us(100); /* FIXME: dummy loop */;

	data16 = reg_read16(MMP_AUDIO_CLOCK_REG_3E);
	data16 |= (1 << 12);
	reg_write16(MMP_AUDIO_CLOCK_REG_3E, data16);

	i2s_delay_us(100); /* FIXME: dummy loop */;

	data16 = reg_read16(MMP_AUDIO_CLOCK_REG_3E);
	data16 &= ~(1 << 12);
	reg_write16(MMP_AUDIO_CLOCK_REG_3E, data16);

	i2s_delay_us(100); /* FIXME: dummy loop */;

//	while(reg_read16(I2S_PCM_RDPTR) != 0); /* FIXME: why ? */
//	while(I2S_AD16_GET_WP() != 0);         /* FIXME: why ? */

	{
		u32 cWP,cRP;
		u32 ori_size;
		u32 bufSize;	
		u32 bufBase;
		u8 *oBuf=NULL;
		
		/* reduce the buffer for faster around */
		cRP = I2S_DA32_GET_RP();
		cWP = I2S_DA32_GET_WP();
		
		if(cRP || cWP)
		{
			bufSize = (cWP + 32 )&(~0x1F);
			
			ori_size = reg_read16(I2S_REG_OBUF_LEN_HI);
			ori_size = (ori_size<<16) | reg_read16(I2S_REG_OBUF_LEN_LO);
			
			reg_write16(I2S_REG_OBUF_LEN_LO, (bufSize-1)&0xFFFF);
			reg_write16(I2S_REG_OBUF_LEN_HI, (bufSize-1)>>16);
			
			//printf("	getObufLen1:[%04x,%04x]cWP=%x,bufsz=%x\n",reg_read16(I2S_REG_OBUF_LEN_HI),reg_read16(I2S_REG_OBUF_LEN_LO),cWP,bufSize);				
			 	
			bufBase = reg_read16(I2S_REG_OBUF_5_HI);
			bufBase = (bufBase<<16) | reg_read16(I2S_REG_OBUF_5_LO);
			oBuf = (u8*)bufBase;
			
			memset((u8*)&oBuf[cWP], 0, bufSize-cWP);
			/* TODO: set other OutBuffer as 0(NOT onlt buffer5) */    
        	
			I2S_DA32_SET_WP(0);
			I2S_AD16_SET_RP(0);
	    	
			while(I2S_DA32_GET_RP() || I2S_AD16_GET_WP());
	    	
			/* restore buffer size*/
			reg_write16(I2S_REG_OBUF_LEN_LO, (ori_size)&0xFFFF);
			reg_write16(I2S_REG_OBUF_LEN_HI, (ori_size)>>16);
			
			//printf("	getObufLen2:[%04x,%04x]\n",reg_read16(I2S_REG_OBUF_LEN_HI),reg_read16(I2S_REG_OBUF_LEN_LO),cWP,ori_size);	
		}
	}

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

	#ifdef	ENABLE_192KHZ_SAMPLE_RATE
	if     ((demanded_sample_rate > 182400) && (demanded_sample_rate < 210600)) { target_sample_rate = 192000; }
	else if((demanded_sample_rate > 91200) && (demanded_sample_rate < 100800)) { target_sample_rate = 96000; }
	else if((demanded_sample_rate > 83790) && (demanded_sample_rate < 92610)) { target_sample_rate = 88200; }
	else if ((demanded_sample_rate > 45600) && (demanded_sample_rate < 50400)) { target_sample_rate = 48000; }
	#else
	if     ((demanded_sample_rate > 45600) && (demanded_sample_rate < 50400)) { target_sample_rate = 48000; }
	#endif
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

#ifdef I2S_DEBUG_SET_CLOCK
	printf("I2S# %s, target_sample_rate: %u\n", __func__, target_sample_rate);
#endif

	#ifdef	ENABLE_192KHZ_SAMPLE_RATE
	if((target_sample_rate == 192000) || (target_sample_rate == 96000) )
	{
		/* Xtal: 30 MHz, 49152000 Hz */
		/* A:3 B:18.0 C:23 D:03 E:011 PLLo:49152166.19 diff:166.19 osc:540,673,828.13  */
		reg_write16(0x00B0, 0x8003);	//D=3
		reg_write16(0x00B2, 0x0B0B); /* AMCLK select PLL2_OUT1 */	//E=11
		reg_write16(0x00B4, 0x2000);
		reg_write16(0x00B6, 0x2017);	//B=16:17:18:16.5=0:1:2:3  C=23
		reg_write16(0x00B4, 0x3000);
		ithDelay(100); /* unit: micro-sec (us) */ /* Experts Recommend Value: 100us */
		reg_write16(0x00B4, 0x2000);

		if(target_sample_rate == 192000)
		{
			amclk_ratio = 1 - 1; /* AMCLK = 49.152MHz (256x192KHz)     */
			zclk_ratio  = 4 - 1; /* ZCLK  = 12.288 MHz (zclk ratio=0x1F) */
		}
		else if(target_sample_rate == 96000)
		{
			amclk_ratio = 2 - 1; /* AMCLK = 24.576MHz (256x96KHz)      */
			zclk_ratio  = 8 - 1; /* ZCLK  = 6.144 MHz (zclk ratio=0x1F) */
		}
		else
		{
			printf("ERROR# I2S, invalid target_sample_rate !\n");
			/* 192000 */
			amclk_ratio = 4 - 1; /* AMCLK = 12.288MHz (256x48KHz)      */
			zclk_ratio  = 16 - 1; /* ZCLK  = 3.07 MHz (zclk ratio=0x1F) */
		}
	}
	else if((target_sample_rate == 48000) || (target_sample_rate == 24000) || (target_sample_rate == 12000))
	#else
	if((target_sample_rate == 48000) || (target_sample_rate == 24000) || (target_sample_rate == 12000))
	#endif
	{
		/* Xtal: 30 MHz, 48000 Hz */
		/* A:3 B:16.0 C:-0166 D:02 E:058 PLLo:12288018.588362 diff:18.588362 osc:712.704000 */
		reg_write16(0x00B0, 0x8002);
		reg_write16(0x00B2, 0x3A3A); /* AMCLK select PLL2_OUT1 */
		reg_write16(0x00B4, 0x2000);
		reg_write16(0x00B6, 0x075A);
		reg_write16(0x00B4, 0x3000);
		ithDelay(100); /* unit: micro-sec (us) */ /* Experts Recommend Value: 100us */
		reg_write16(0x00B4, 0x2000);

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
	#ifdef	ENABLE_192KHZ_SAMPLE_RATE
	else if((target_sample_rate == 88200) || (target_sample_rate == 44100) || (target_sample_rate == 22050) || (target_sample_rate == 11025))
	#else
	else if((target_sample_rate == 44100) || (target_sample_rate == 22050) || (target_sample_rate == 11025))
	#endif
	{
		#ifdef	ENABLE_192KHZ_SAMPLE_RATE
		if(target_sample_rate == 88200)
		{
			/* Xtal: 30 MHz, 88200 Hz */
			/* A:3 B:16.5 C:119 D:02 E:033 PLLo:22,579,234.7301  diff:34.7301 osc:745,114,746.0938  */
			reg_write16(0x00B0, 0x8002);	//A=3:4:5:6=[13:12][00][01][10][11]	D=2:3=[1:0][10][11]
			reg_write16(0x00B2, 0x2121); /* AMCLK select PLL2_OUT1 */
			reg_write16(0x00B4, 0x2000);
			reg_write16(0x00B6, 0x3077);	//B=16:17:18:16.5=[13:12]0:1:2:3  C=119(0x77)
			reg_write16(0x00B4, 0x3000);
			ithDelay(100); /* unit: micro-sec (us) */ /* Experts Recommend Value: 100us */
			reg_write16(0x00B4, 0x2000);
		}
		else
		{
			/* Xtal: 30 MHz, 44100 Hz */
			/* A:3 B:18.0 C:-0192 D:02 E:071 PLLo:11289612.676056 diff:12.676056 osc:801.561600 */
			reg_write16(0x00B0, 0x8002);
			reg_write16(0x00B2, 0x4747); /* AMCLK select PLL2_OUT1 */
			reg_write16(0x00B4, 0x2000);
			reg_write16(0x00B6, 0x2740);
			reg_write16(0x00B4, 0x3000);
			ithDelay(100); /* unit: micro-sec (us) */ /* Experts Recommend Value: 100us */
			reg_write16(0x00B4, 0x2000);
		}
		#else
		/* Xtal: 30 MHz, 44100 Hz */
		/* A:3 B:18.0 C:-0192 D:02 E:071 PLLo:11289612.676056 diff:12.676056 osc:801.561600 */
		reg_write16(0x00B0, 0x8002);
		reg_write16(0x00B2, 0x4747); /* AMCLK select PLL2_OUT1 */
		reg_write16(0x00B4, 0x2000);
		reg_write16(0x00B6, 0x2740);
		reg_write16(0x00B4, 0x3000);
		ithDelay(100); /* unit: micro-sec (us) */ /* Experts Recommend Value: 100us */
		reg_write16(0x00B4, 0x2000);
		#endif

		#ifdef	ENABLE_192KHZ_SAMPLE_RATE
		if(target_sample_rate == 88200)
		{
			amclk_ratio = 1 - 1; /* AMCLK = 11.29MHz (256x44.1KHz)     */
			zclk_ratio  = 4 - 1; /* ZCLK  = 2.82MHz (zclk ratio=0x1F) */
		}
		else if(target_sample_rate == 44100)
		#else
		if(target_sample_rate == 44100)
		#endif
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
		/* Xtal: 30 MHz, 32000 Hz */
		/* A:5 B:16.5 C:00098 D:03 E:101 PLLo:8192005.724010 diff:5.724010 osc:827.392000 */
		reg_write16(0x00B0, 0xA003);
		reg_write16(0x00B2, 0x6565); /* AMCLK select PLL2_OUT1 */
		reg_write16(0x00B4, 0x2000);
		reg_write16(0x00B6, 0x3062);
		reg_write16(0x00B4, 0x3000);
		ithDelay(100); /* unit: micro-sec (us) */ /* Experts Recommend Value: 100us */
		reg_write16(0x00B4, 0x2000);

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

#ifdef I2S_DEBUG_SET_CLOCK
	printf("I2S# %s, amclk_ratio: 0x%04x, zclk_ratio: 0x%04x\n", __func__, amclk_ratio, zclk_ratio);
#endif

	/* setup 0x003A/0x003C */
	{
#ifdef I2S_DEBUG_SET_CLOCK
		printf("I2S# %s, setup 0x003A/0x003C !\n", __func__);
#endif

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
			ithDelay(100); /* unit: micro-sec (us) */

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
			ithDelay(100); /* unit: micro-sec (us) */

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

//	printf("I2S# %s\n", __func__);

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

static void _i2s_use_GPIO_MODE2(void) /* GPIO settings for CFG2 */
{
	printf("I2S# %s\n", __func__);

	ithGpioSetMode( 6, ITH_GPIO_MODE2); /* ZCLK     (CODEC-BCLK)   */
	ithGpioSetMode( 7, ITH_GPIO_MODE2); /* AMCLK    (CODEC-MCLK)   */
	ithGpioSetMode(11, ITH_GPIO_MODE2); /* ZWS      (CODEC-DACLRC) */
	ithGpioSetMode(12, ITH_GPIO_MODE2); /* DATA-IN  (CODEC-ADCDAT) */

	if(_g_UseOutputPin)		ithGpioSetMode(14, ITH_GPIO_MODE2); /* DATA-OUT (CODEC-DACDAT) */

	/* 9078-EVB with WM8778, no AMCLK (CODEC-MCLK) signal found at GPIO-PIN-7 */
	/* so, we enhance the GPIO-7 driving */
	{
		u32 data32;

		data32 = ithReadRegA(0xDE000000 | 0xA0); /* 0x7CA0 */

		data32 &= ~(3 << 14);
		data32 |= (0 << 14); /* 0:OK, 3:OK, but '0' is better */

		ithWriteRegA(0xDE000000 | 0xA0, data32); /* [15:14] GPIO-7 Driving Setting */
	}
}

/* ************************************************************************** */
void i2s_CODEC_wake_up(void)
{
	itp_codec_wake_up();
}

void i2s_CODEC_standby(void)
{
	itp_codec_standby();
}

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

void i2s_enable_fading(int yesno){
    if(yesno){
//		_i2s_enable_fading(0xF, 0x01); /* fast response */
//		_i2s_enable_fading(0x7, 0x7F); /* moderato */      
        _i2s_enable_fading(0x1, 0xFF); /* slow response */
		reg_write16(I2S_REG_DIG_VOL, 0xFF00);
    }else{
        _i2s_disable_fading();
    }    
}

void i2s_pause_DAC(int pause)
{
	u16 data16;

	printf("I2S# %s(%d)\n", __func__, pause);

	if(pause)
	{
		itp_codec_playback_mute();
		itp_codec_power_off();
#if 0
		_i2s_enable_fading(0xF, 0x01); /* fast response */
//		_i2s_enable_fading(0x7, 0x7F); /* moderato */
//		_i2s_enable_fading(0x1, 0xFF); /* slow response */
		reg_write16(I2S_REG_DIG_VOL, 0x0000);
#endif
		data16 = reg_read16(I2S_REG_OUTPUT_CTL2);
		data16 &= ~(1 << 0);
		reg_write16(I2S_REG_OUTPUT_CTL2, data16);
	}
	else /* resume */
	{
#if 0
		_i2s_enable_fading(0x1, 0xFF); /* slow response */
		reg_write16(I2S_REG_DIG_VOL, 0xFF00);
#endif

		data16 = reg_read16(I2S_REG_OUTPUT_CTL2);
		data16 |= (1 << 0);
		reg_write16(I2S_REG_OUTPUT_CTL2, data16);

        itp_codec_power_on();
		if(!_i2s_DA_mute)
		itp_codec_playback_unmute();
		

//		data16 = reg_read16(I2S_REG_OUTPUT_CTL2);
//		data16 |= (1 << 0);
//		reg_write16(I2S_REG_OUTPUT_CTL2, data16);
	}
}

void i2s_deinit_ADC(void)
{
	u16 data16;

	if(0 == _i2s_AD_running) {
		printf("I2S# ADC is 'NOT' running, skip deinit ADC !\n");
		return;
	}

	printf("I2S# %s +\n", __func__);

    pthread_mutex_lock(&I2S_MUTEX);

    #ifdef	ENABLE_FORCE_RESET_RW_POINTER
	if(_i2s_DA_running)	_i2s_reset_AD_RWptr();
	#endif

	/* disable hardware I2S */
	{
		data16 = reg_read16(I2S_REG_INPUT_CTL);
		data16 &= ~(3 << 0);
		reg_write16(I2S_REG_INPUT_CTL, data16);
	}

	itp_codec_rec_deinit();

	_i2s_AD_running = 0; /* put before _i2s_reset() */
	_i2s_reset();

	_i2s_power_off();

	pthread_mutex_unlock(&I2S_MUTEX);
	printf("I2S# %s -\n", __func__);
}

void i2s_deinit_DAC(void)
{
	u16 data16;
	u16 out_status_1;
	u16 out_status_2;
//	u32 i2s_idle;
	u32 pipe_idle;
	u32 i2s_memcnt;
#ifdef I2S_DEBUG_DEINIT_DAC_COST
	static struct timeval tv_pollS;
	static struct timeval tv_pollE;
#endif

	pthread_mutex_lock(&I2S_MUTEX);

	if(0 == _i2s_DA_running) {
		printf("I2S# DAC is 'NOT' running, skip deinit DAC !\n");
		pthread_mutex_unlock(&I2S_MUTEX);
		return;
	}

	printf("I2S# %s +\n", __func__);

	/* FIXME: SPDIF */
	#ifdef I2S_USE_SPDIF
	if(_g_UseSpdif)	_deinit_spdif();
	#endif

	#ifdef	ENABLE_FORCE_RESET_RW_POINTER
	if(_i2s_AD_running)	_i2s_reset_DA_RWptr();
	#endif

	itp_codec_playback_deinit();

	/* disable I2S_OUT_FIRE & I2S_OUTPUT_EN */
	data16 = reg_read16(I2S_REG_OUTPUT_CTL2);
	data16 &= ~(0x3);
	reg_write16(I2S_REG_OUTPUT_CTL2, data16);

	do {
		data16 = reg_read16(I2S_REG_OUTPUT_CTL2);
	} while(data16 & 0x3);

#ifdef I2S_DEBUG_DEINIT_DAC_COST
	gettimeofday(&tv_pollS, NULL); //-*-
#endif

	do
	{
//		i2s_delay_us(1000); /* FIXME: dummy loop */;

		out_status_1 = reg_read16(I2S_REG_OUTPUT_STATUS1);
		out_status_2 = reg_read16(I2S_REG_OUTPUT_STATUS2);

//		printf("I2S# OUTPUT_STATUS 1,2: 0x%04x,0x%04x\n", out_status_1, out_status_2);

//		i2s_idle   =  out_status_1 & 0x1;
		pipe_idle  = (out_status_1 >> 3) & 0x1;
		i2s_memcnt = (out_status_2 >> 4) & 0xF;
	} while(/*!i2s_idle*/ !pipe_idle || (i2s_memcnt != 0));

#ifdef I2S_DEBUG_DEINIT_DAC_COST
	gettimeofday(&tv_pollE, NULL); //-*-
	printf("I2S# DEINIT_DAC_COST: %ld (ms)\n", TV_CAL_DIFF_MS(tv_pollE, tv_pollS));
#endif

	_i2s_DA_running = 0; /* put before _i2s_reset() */
	_i2s_reset();

	_i2s_power_off();

	printf("I2S# %s -\n", __func__);
	pthread_mutex_unlock(&I2S_MUTEX);
}

void i2s_init_DAC(STRC_I2S_SPEC *i2s_spec)
{
	int i;
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

//	printf("I2S# %s +\n", __func__);

	//memcpy( (void*)&g_curr_DA_Spec, (u8*)i2s_spec, sizeof(g_curr_DA_Spec) );

	/* check buffer base/size alignment */
	for(i=0; i<4; i++) { if((u32)i2s_spec->base_hdmi[i] % 8) { param_fail = 1; } }
	
	if(_g_UseSpdif)	
	{
		if((u32)i2s_spec->base_spdif % 8) { param_fail = 2; }
	}
	else
	{
		if( !i2s_spec->base_spdif || ((u32)i2s_spec->base_spdif % 8) ) { param_fail = 2; }
	}

	if((u32)i2s_spec->base_i2s % 8) { param_fail = 3; }
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
//		pthread_mutex_unlock(&I2S_MUTEX);
//		return;
//	}

	_i2s_power_on();

	if(_g_UseGpioMode2)	_i2s_use_GPIO_MODE2();

	_i2s_set_clock(i2s_spec->sample_rate);
	_i2s_reset();	
	itp_codec_set_i2s_sample_rate(i2s_spec->sample_rate);
	/* FIXME: SPDIF */
	#ifdef I2S_USE_SPDIF
	if(_g_UseSpdif)	_init_spdif(i2s_spec->sample_rate);
	#endif

	/* mastor mode & AWS & ZWS sync */
	switch(i2s_spec->sample_size) {
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

	data16 = (1        << 13) /* DAC device selection, 0:External DAC; 1:Internal DAC */
#ifdef I2S_MASTER_MODE
	| (0               << 12) /* 0:Master(ZCLK/ZWS output mode); 1:Slave(ZCLK/ZWS input mode) */
#else
	| (1               << 12) /* 0:Master(ZCLK/ZWS output mode); 1:Slave(ZCLK/ZWS input mode) */
#endif
	| (resolution_type << 10) /* DAC resolution bits, 00:16bits; 01:24bits; 10:32bits */
	| (0x1F            <<  0); /* sample width (in bits unit) */

	reg_write16(I2S_REG_DAC_SAMPLE, data16);
	_i2s_aws_sync();

	if( i2s_spec->enable_Speaker>=2)
	{
		i2s_spec->enable_Speaker = 0;
		i2s_spec->enable_HeadPhone = 1;
	}
	if( !i2s_spec->enable_Speaker)	i2s_spec->enable_HeadPhone = 1; /* force to set output as headphone if speaker is 0 */

	if( i2s_spec->enable_Speaker &&  i2s_spec->enable_HeadPhone) { itp_codec_playback_init(2); } /* both SEPAKER & HEADPHONE */
	else if(i2s_spec->enable_Speaker)							 { itp_codec_playback_init(1); } /* SPEAKER only */
	else														 { itp_codec_playback_init(0); } /* HEADPHONE(line-out) only */

	/* buffer base */
	{
		for(i=0; i<4; i++) {
			if(i2s_spec->base_hdmi[i] == NULL) {
				i2s_spec->base_hdmi[i] = i2s_spec->base_i2s;
			}
		}
		if(i2s_spec->base_spdif == NULL) { i2s_spec->base_spdif = i2s_spec->base_i2s; }

		/* buf 1 (HDMI Data0) */
		reg_write16(I2S_REG_OBUF_1_LO, (u32)i2s_spec->base_hdmi[0] & 0xFFFF);
		reg_write16(I2S_REG_OBUF_1_HI, (u32)i2s_spec->base_hdmi[0] >> 16);

		/* buf 2 (HDMI Data1) */
		reg_write16(I2S_REG_OBUF_2_LO, (u32)i2s_spec->base_hdmi[1] & 0xFFFF);
		reg_write16(I2S_REG_OBUF_2_HI, (u32)i2s_spec->base_hdmi[1] >> 16);

		/* buf 3 (HDMI Data2) */
		reg_write16(I2S_REG_OBUF_3_LO, (u32)i2s_spec->base_hdmi[2] & 0xFFFF);
		reg_write16(I2S_REG_OBUF_3_HI, (u32)i2s_spec->base_hdmi[2] >> 16);

		/* buf 4 (HDMI Data3) */
		reg_write16(I2S_REG_OBUF_4_LO, (u32)i2s_spec->base_hdmi[3] & 0xFFFF);
		reg_write16(I2S_REG_OBUF_4_HI, (u32)i2s_spec->base_hdmi[3] >> 16);

		/* buf 5 (IIS / SPDIF Data) */
		reg_write16(I2S_REG_OBUF_5_LO, (u32)i2s_spec->base_i2s & 0xFFFF);
		reg_write16(I2S_REG_OBUF_5_HI, (u32)i2s_spec->base_i2s >> 16);

		/* buf 6 (SPDIF Data) */
		reg_write16(I2S_REG_OBUF_6_LO, (u32)i2s_spec->base_spdif & 0xFFFF);
		reg_write16(I2S_REG_OBUF_6_HI, (u32)i2s_spec->base_spdif >> 16);

		/* SPDIF data channel select */
		if(_g_UseSpdif)	
		{
			data16 = reg_read16(I2S_REG_SPDIF_VOL);
			data16 &= ~(1 << 14);

			if(i2s_spec->is_dac_spdif_same_buffer) { data16 |= (0 << 14); } /* 0:Output Buffer Base5 */
			else                                   { data16 |= (1 << 14); } /* 1:Output Buffer Base6 */

			reg_write16(I2S_REG_SPDIF_VOL, data16);
		}
	}

	/* buffer length */
	reg_write16(I2S_REG_OBUF_LEN_LO, (i2s_spec->buffer_size - 1) & 0xFFFF); /* NOTE: minus one for hardware design */
	reg_write16(I2S_REG_OBUF_LEN_HI, (i2s_spec->buffer_size - 1) >> 16);

	/* DA starvation interrupt threshold */
	reg_write16(I2S_REG_OUTPUT_GAP_LO, 0xFFFF); /* TODO: need reasonable threshold */
	reg_write16(I2S_REG_OUTPUT_GAP_HI, 0x0000);

	/* output path */
	{
		data16 = (1 << 8); /* DA starvation (DA RdWrGap) Interrupt */

		if(i2s_spec->is_dac_spdif_same_buffer) { data16 |= (1 << 4); } /* buf 5 (IIS / SPDIF Data) */
		else                                   { data16 |= (1 << 5); } /* buf 6 (SPDIF Data) */

		switch(i2s_spec->num_hdmi_audio_buffer)
		{
			case 1: { data16 |=  1; break; } /* buf 1 (HDMI Data0) */
			case 2: { data16 |=  3; break; }
			case 3: { data16 |=  7; break; }
			case 4: { data16 |= 15; break; }
			case 0:
			default: break;
		}

		reg_write16(I2S_REG_OUTPUT_CTL, data16);
	}

#ifdef ENABLE_ITV_AUDIO_BIST_MODE
	/* HDMI audio Functional Test */
	ithWriteRegH(0x1676, 0x0000); /* SynWordL */
	ithWriteRegH(0x1678, 0x0000); /* SynWordR */
	ithWriteRegH(0x167A, 0x0011); /* step_EnHDMI */
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
	else
	    i2s_pause_DAC(1);

	_i2s_DA_running = 1;

	printf("I2S# %s -\n", __func__);
	pthread_mutex_unlock(&I2S_MUTEX);
}

void i2s_init_ADC(STRC_I2S_SPEC *i2s_spec)
{
	u16 data16;
	u16 data16_tmp;
	u8 resolution_type;

	if(_i2s_AD_running) {
		printf("I2S# ADC is running, skip re-init ADC !\n");
		return;
	}

	printf("I2S# %s +\n", __func__);

	//memcpy((void*)&g_curr_AD_Spec, (u8*)i2s_spec, sizeof(g_curr_AD_Spec) );
	
	if(((u32)i2s_spec->base_i2s % 8) || (i2s_spec->buffer_size % 8)) {
		printf("ERROR# I2S, bufbase/bufsize must be 8-Bytes alignment !\n");
		return;
	}
	if(i2s_spec->buffer_size > 0xFFFF) {
		printf("ERROR# I2S, bufsize must be 16-bits width !\n");
		return;
	}
	if((i2s_spec->channels != 1) && (i2s_spec->channels != 2)) {
		printf("ERROR# I2S, only support single or two channels !\n");
		return;
	}

    pthread_mutex_lock(&I2S_MUTEX);

	_i2s_power_on();

	if(_g_UseGpioMode2)	_i2s_use_GPIO_MODE2();

	_i2s_set_clock(i2s_spec->sample_rate);
	_i2s_reset();	
	itp_codec_set_i2s_sample_rate(i2s_spec->sample_rate);
	_i2s_AD_running = 1;

	/* mastor mode & AWS & ZWS sync */
	switch(i2s_spec->sample_size) {
		case 16: { resolution_type = 0; break; }
		case 24: { resolution_type = 1; break; }
		case 32:
		default: { resolution_type = 2; break; }
	}

	/* config AD */
	{
		data16 = (0        << 13) /* ADC device selection, 0:External ADC; 1:Internal ADC */
#ifdef I2S_MASTER_MODE
		| (0               << 12) /* hardware not use this bit */
#else
		| (1               << 12) /* hardware not use this bit */
#endif
		| (resolution_type << 10) /* ADC resolution bits, 00:16bits; 01:24bits; 10:32bits */
		| (0x1F            <<  0); /* sample width (in bits unit) */

		reg_write16(I2S_REG_ADC_SAMPLE, data16);

		data16_tmp = reg_read16(I2S_REG_DAC_SAMPLE);
		data16_tmp &= ~(1 << 12);
		data16_tmp |= (data16 & 0x1000);
		data16_tmp &= ~(0x1FF << 0); /* [8:0] B0ZWSRatio */
		data16_tmp |= (0x1F << 0);   /* sample width (in bits unit) */
		reg_write16(I2S_REG_DAC_SAMPLE, data16_tmp); /* set 'master or slave mode' here ! */
	}
	_i2s_aws_sync();

	if     ( i2s_spec->from_LineIN && !i2s_spec->from_MIC_IN) { itp_codec_rec_init(1); } /* LineIN only */
	else if(!i2s_spec->from_LineIN &&  i2s_spec->from_MIC_IN) { itp_codec_rec_init(2); } /* MICIN only */
	else                                                      { itp_codec_rec_init(0); } /* both LineIN & MICIN */

	/* buffer base */
	{
	reg_write16(I2S_REG_IBUF_LO, (u32)i2s_spec->base_i2s & 0xFFFF);
	reg_write16(I2S_REG_IBUF_HI, (u32)i2s_spec->base_i2s >> 16);
	}

	/* buffer length */
	reg_write16(I2S_REG_IBUF_LEN, (i2s_spec->buffer_size - 1) & 0xFFFF); /* NOTE: minus one for hardware design */

	/* AD starvation interrupt threshold */
	//data16 = (0x8FFF & 0xFFF8) | (1 << 0);
	data16 = 0;
	reg_write16(I2S_REG_INPUT_CTL2, data16);

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
    pthread_mutex_unlock(&I2S_MUTEX);
	printf("I2S# %s -\n", __func__);
}

int  i2s_get_DA_running(void)
{
    return _i2s_DA_running;
}

void i2s_mute_DAC(int mute)
{
	printf("I2S# %s(%d)\n", __func__, mute);

	if(mute)
	{
		itp_codec_playback_mute();
		_i2s_DA_mute = 1;
#if 0
//		_i2s_enable_fading(0xF, 0x01); /* fast response */
		_i2s_enable_fading(0x7, 0x7F); /* moderato */
		reg_write16(I2S_REG_DIG_VOL, 0x0000);
#endif
	}
	else /* resume */
	{
		itp_codec_playback_unmute();
		_i2s_DA_mute = 0;
#if 0
		_i2s_enable_fading(0x1, 0xFF); /* slow response */
		reg_write16(I2S_REG_DIG_VOL, 0xFF00);
#endif
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

void i2s_set_linein_bypass(int bypass)
{
	printf("I2S# %s(%d)\n", __func__, bypass);
	itp_codec_playback_linein_bypass(bypass);
}

//void i2s_get_DAC_spec(STRC_I2S_SPEC *i2s_spec)
//{
//	memcpy( (void*)i2s_spec, (u8*)&g_curr_DA_Spec, sizeof(g_curr_DA_Spec));
//}

//void i2s_get_ADC_spec(STRC_I2S_SPEC *i2s_spec)
//{
//	memcpy( (void*)i2s_spec, (u8*)&g_curr_AD_Spec, sizeof(g_curr_AD_Spec));
//}

void i2s_init_gpio_mode2(void)
{
	_g_UseGpioMode2 = 1;
}

void i2s_init_spdif(void)
{
	_g_UseSpdif = 1;
}

void i2s_init_output_pin(void)
{
	_g_UseOutputPin = 1;
}

static void _init_spdif(u32 sample_rate)
{
	u32 data32;
	typedef enum SPDIF_MODE_TAG
	{
    	SPDIF_LINEAR_PCM = 0,
    	SPDIF_NON_LINEAR_DATA
	} SPDIF_MODE;

	printf("I2S# %s\n", __func__);

//	ithWriteRegA( 0xDE000000 | 0x90, 0x22840000); //set gpio padselect
	ithGpioSetMode(9, ITH_GPIO_MODE1);
//	ithWriteRegA( 0xDE000000 | 0xD0, 0x0006); //set spdif clk from 1:pclk 0:amclk
	ithClearRegBitA(ITH_GPIO_BASE + 0xD0, 7);

	/* for SPDIF-LPCM mode */
	{
		#define SPDIF_STATUS_COPY_FREE      (0x1 << 2)
		#define SPDIF_STATUS_STEREO         (0x3 << 20)
		#define SPDIF_SAMPLE_MAX_WORD_LEN   (0x0 << 0)
		#define SPDIF_SAMPLE_WORD_LEN       (0x1 << 1)

		/* Original Sampling frequency. */
		#define ORIGINAL_192K_INFO          (0x1 << 4)
		#define ORIGINAL_12K_INFO           (0x2 << 4)
		#define ORIGINAL_176_40K_INFO       (0x3 << 4)
		#define ORIGINAL_96K_INFO           (0x5 << 4)
		#define ORIGINAL_8K_INFO            (0x6 << 4)
		#define ORIGINAL_88_20K_INFO        (0x7 << 4)
		#define ORIGINAL_16K_INFO           (0x8 << 4)
		#define ORIGINAL_24K_INFO           (0x9 << 4)
		#define ORIGINAL_11_025K_INFO       (0xA << 4)
		#define ORIGINAL_22_05K_INFO        (0xB << 4)
		#define ORIGINAL_32K_INFO           (0xC << 4)
		#define ORIGINAL_48K_INFO           (0xD << 4)
		#define ORIGINAL_44_10K_INFO        (0xF << 4)
		#define ORIGINAL_NO_INDICATION      (0x1 << 4)

		/* STATUS BIT 0 */
		data32 = SPDIF_LINEAR_PCM | SPDIF_STATUS_COPY_FREE | SPDIF_STATUS_STEREO;
		ithWriteRegA(0xDE900000 | 0x24, data32); /* SPDIF Status Bit Register 0, STUS0 */

		/* STATUS BIT 1 */
		data32 = SPDIF_SAMPLE_MAX_WORD_LEN | SPDIF_SAMPLE_WORD_LEN;
		switch(sample_rate)
		{
			case 192000:
				data32 |= ORIGINAL_192K_INFO;
				break;
			case 96000:
				data32 |= ORIGINAL_96K_INFO;
				break;
			case 88200:
				data32 |= ORIGINAL_88_20K_INFO;
				break;
			case 11025:
				data32 |= ORIGINAL_11_025K_INFO;
				break;
			case 22050:
				data32 |= ORIGINAL_22_05K_INFO;
				break;
			case 44100:
				data32 |= ORIGINAL_44_10K_INFO;
				break;
			case 12000:
				data32 |= ORIGINAL_12K_INFO;
				break;
			case 24000:
				data32 |= ORIGINAL_24K_INFO;
				break;
			case 48000:
				data32 |= ORIGINAL_48K_INFO;
				break;
			case 8000:
				data32 |= ORIGINAL_8K_INFO;
				break;
			case 16000:
				data32 |= ORIGINAL_16K_INFO;
				break;
			case 32000:
				data32 |= ORIGINAL_32K_INFO;
				break;
			default:
				SERR();
				break;
		}
		ithWriteRegA(0xDE900000 | 0x28, data32); /* SPDIF Status Bit Register 1, STUS1 */
	}

	ithWriteRegA( 0xDE900000 | 0x00, 0x0000504c); /* SSP Control Register 0, CR0 */
	ithWriteRegA( 0xDE900000 | 0x04, 0x000f0000); /* SSP Control Register 1, CR1 */
	ithWriteRegA( 0xDE900000 | 0x10, 0x00100020); /* Interrupt Control Register, ICR */
//	ithWriteRegA( 0xDE900000 | 0x24, 0x00000004); /* SPDIF Status Bit Register 0, STUS0 */
//	ithWriteRegA( 0xDE900000 | 0x28, 0x000000d2); /* SPDIF Status Bit Register 1, STUS1 */
	ithWriteRegA( 0xDE900000 | 0x5C, 0x00040001); /* I2S and AC3 Control Register, CR3 */
	ithWriteRegA( 0xDE900000 | 0x08, 0x0000000f); /* SSP Control Register 2, CR2 */
}

static void _deinit_spdif(void)
{
	printf("I2S# %s\n", __func__);
}

static void _show_i2s_spec(STRC_I2S_SPEC *i2s_spec)
{
	printf("	@channels	%x\n",i2s_spec->channels);
	printf("	@sample_rate	%x\n",i2s_spec->sample_rate);
	printf("	@buffer_size	%x\n",i2s_spec->buffer_size);
	printf("	@is_big_endian	%x\n",i2s_spec->is_big_endian);
	printf("	@sample_size	%x\n",i2s_spec->sample_size);
	printf("	@from_LineIN	%x\n",i2s_spec->from_LineIN);
	printf("	@from_MIC_IN	%x\n",i2s_spec->from_MIC_IN);
	printf("	@num_hdmi_buf	%x\n",i2s_spec->num_hdmi_audio_buffer);
	printf("	@base_i2s	%x\n",i2s_spec->base_i2s);
	printf("	@spdif_same_buf	%x\n",i2s_spec->is_dac_spdif_same_buffer);
	printf("	@base_hdmi[0]	%x\n",i2s_spec->base_hdmi[0]);
	printf("	@base_hdmi[1]	%x\n",i2s_spec->base_hdmi[1]);
	printf("	@base_hdmi[2]	%x\n",i2s_spec->base_hdmi[2]);
	printf("	@base_hdmi[3]	%x\n",i2s_spec->base_hdmi[3]);
	printf("	@base_spdif	%x\n",i2s_spec->base_spdif);
	printf("	@aud_out	%x\n",i2s_spec->postpone_audio_output);
	printf("	@record_mode	%x\n",i2s_spec->record_mode);
	printf("	@Speaker	%x\n",i2s_spec->enable_Speaker);
	printf("	@HeadPhone	%x\n",i2s_spec->enable_HeadPhone);
}

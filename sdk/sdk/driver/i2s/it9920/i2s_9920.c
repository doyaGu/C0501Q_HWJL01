#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "ite/ith.h"
#include "ite/itp.h"

#include "i2s/i2s_it9920.h"
#include "i2s_reg_9920.h"



/* ************************************************************************** */
/* platform control */

#define ENABLE_192KHZ_SAMPLE_RATE

//#define ENABLE_ITV_AUDIO_BIST_MODE

#define I2S_SLAVE_MODE      0
#define I2S_INTERNAL_CODEC  0
#define I2S_SWITCHCHANNEL   0
#define I2S_MSB_MODE        0


/*******************************************************
   It's for workaround the reset I2S engine issue.
   and it will take 1~500 ms as the price.        
********************************************************/
//#define ENABLE_FORCE_RESET_RW_POINTER

//#define I2S_DEBUG_SET_CLOCK
//#define I2S_DEBUG_DEINIT_DAC_COST

/* ************************************************************************** */
#define SERR() do { printf("ERROR# %s:%d, %s\n", __FILE__, __LINE__, __func__); while(1); } while(0)
#define S()    do { printf("=> %s:%d, %s\n",     __FILE__, __LINE__, __func__);           } while(0)
#define TV_CAL_DIFF_MS(TIME2, TIME1) ((((TIME2.tv_sec * 1000000UL) + TIME2.tv_usec) - ((TIME1.tv_sec * 1000000UL) + TIME1.tv_usec)) / 1000)

#ifdef _WIN32
    #define asm()
#endif

/* ************************************************************************** */
u32 I2S_DA32_GET_RP(void);
u32 I2S_DA32_GET_WP(void);
void I2S_DA32_SET_WP(u32 data32);
void I2S_DA32_SET_LAST_WP(u32 data32);

u32 I2S_AD32_GET_RP(int);
u32 I2S_AD32_GET_WP(int);
void I2S_AD32_SET_RP(u32 data32, int);

u32 i2s_is_DAstarvation(void);//

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
static void _i2s_use_GPIO_MODE(void); /* GPIO settings for CFG2 */

static int _g_UseOutputPin 	= 0;

static int _i2s_DA_running = 0;
static int _i2s_AD_running = 0;
static int _i2s_DA_mute = 0;
static int _bar_mute_flag = 0;
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
void i2s_set_direct_volperc(unsigned volperc);
unsigned i2s_get_current_volperc(void);
void i2s_get_volstep_range(unsigned *max, unsigned *normal, unsigned *min);
void i2s_set_linein_bypass(int bypass);

/* FIXME: SPDIF */
static void _init_spdif(u32 sample_rate);
static void _deinit_spdif(void);
static void _show_i2s_spec(STRC_I2S_SPEC *i2s_spec);
/* ************************************************************************** */
u32 I2S_DA32_GET_RP(void)
{
	u32 data32;

	data32 = ithReadRegA(I2S_REG_OUT_RDPTR);

	return data32;
}

u32 I2S_DA32_GET_WP(void)
{
	u32 data32;

	data32 = ithReadRegA(I2S_REG_OUT_WRPTR);

	return data32;

}

void I2S_DA32_SET_WP(u32 data32)
{
	/* check if enable the "last write pointer" function. */
	if( ithReadRegA(I2S_REG_OUT_CTRL)&0x4 )
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
		ithWriteRegMaskA(I2S_REG_OUT_CTRL, 0, 1<<2);
	}
	
	/* must be low then hi for hw design */
	ithWriteRegA(I2S_REG_OUT_WRPTR, data32);
}

void I2S_DA32_SET_LAST_WP(u32 data32)
{
	printf("set last WP!!\n");
	/* Enable the last pointer function */
	ithWriteRegMaskA(I2S_REG_OUT_CTRL, 1<<2, 1<<2);
	
	/* must be low then hi for hw design */
	ithWriteRegA(I2S_REG_OUT_WRPTR, data32);
}

u32 I2S_AD32_GET_RP(int index)
{
	u32 data32;

	if(index == 1) {
		data32 = ithReadRegA(I2S_REG_IN1_RDPTR);
	}
	else if(index == 2) {
		data32 = ithReadRegA(I2S_REG_IN2_RDPTR);
	}

	return data32;

}

u32 I2S_AD32_GET_WP(int index)
{
	u32 data32;

	if(index == 1) {
		data32 = ithReadRegA(I2S_REG_IN1_WRPTR);
	}
	else if(index == 2) {
		data32 = ithReadRegA(I2S_REG_IN2_WRPTR);
	}

	return data32;

}

void I2S_AD32_SET_RP(u32 data32, int index)
{
	if(index == 1) {
		ithWriteRegA(I2S_REG_IN1_RDPTR, data32);
	}
	else if(index == 2) {
		ithWriteRegA(I2S_REG_IN2_RDPTR, data32);
	}
}

u32 i2s_is_DAstarvation(void)
{
	if(ithReadRegA(I2S_REG_OUT_STATUS1) & 0x1) {
		return 1;
	}
	else {
		return 0;
	}
}

static void _i2s_power_on(void)
{
    u32 data32;
	/* enable audio clock for I2S modules */
	/* NOTE: we SHOULD enable audio clock before toggling 0x003A/3C/3E */
	/* enable PLL2 for audio */
    data32 = ithReadRegA(MMP_AUDIO_CLOCK_REG_40);
    data32 |= (1 << 17); /* enable AMCLK (system clock for wolfson chip) */
    ithWriteRegA(MMP_AUDIO_CLOCK_REG_40, data32);

    data32 = ithReadRegA(MMP_AUDIO_CLOCK_REG_3C);  //Audio Clock Register 1
    data32 |= (1 << 19); /* disable ZCLK  (IIS DAC clock)                 */
    data32 |= (1 << 21); /* disable M7CLK (memory clock for DAC)          */
    data32 |= (1 << 23); /* disable M8CLK (memory clock for ADC)          */
    ithWriteRegA(MMP_AUDIO_CLOCK_REG_3C, data32);

	
	/* 9920 */
	/* should enable audio clock by 0x003C, and enable PLL3 for audio */
}

static void _i2s_power_off(void)
{
    u32 data32;
    data32 = ithReadRegA(MMP_AUDIO_CLOCK_REG_40);
    data32 &= ~(1 << 17); /* disable AMCLK (system clock for wolfson chip) */
    ithWriteRegA(MMP_AUDIO_CLOCK_REG_40, data32);

    /* disable audio clock for I2S modules */
    data32 = ithReadRegA(MMP_AUDIO_CLOCK_REG_3C);  //Audio Clock Register 1
    data32 &= ~(1<<19); /* disable ZCLK  (IIS DAC clock)                 */
    data32 &= ~(1<<21); /* disable M7CLK (memory clock for DAC)          */
    data32 &= ~(1<<23); /* disable M8CLK (memory clock for ADC)          */
    ithWriteRegA(MMP_AUDIO_CLOCK_REG_3C, data32);
}

/*
 These 2 functions are for workaround the reset I2S engine issue.
 and it will take 1~500 ms as the price.
*/
#ifdef	ENABLE_FORCE_RESET_RW_POINTER
static void _i2s_reset_AD_RWptr(void)
{
}

static void _i2s_reset_DA_RWptr(void)
{
}
#endif

static void _i2s_reset(void)
{
	u32 data32;

	if(_i2s_DA_running) {
		printf("I2S# DAC is running, skip reset I2S !\n");
		return;
	}
	if(_i2s_AD_running) {
		printf("I2S# ADC is running, skip reset I2S !\n");
		return;
	}

#if 0	//FixMe: should 9920 use???
	i2s_delay_us(100); /* FIXME: dummy loop */;
    
	data32 = ithReadRegA(MMP_AUDIO_CLOCK_REG_3C);
	data32 |= (0xF << 28);//reset DA & AD engine
	ithWriteRegA(MMP_AUDIO_CLOCK_REG_3C, data16);

	i2s_delay_us(100); /* FIXME: dummy loop */;

	data32 = ithReadRegA(MMP_AUDIO_CLOCK_REG_3C);
	data32 &= ~(0xF << 28);//normal option DA & AD engine
	ithWriteRegA(MMP_AUDIO_CLOCK_REG_3C, data16);

	i2s_delay_us(100); /* FIXME: dummy loop */;

//	while(reg_read16(I2S_PCM_RDPTR) != 0); /* FIXME: why ? */
//	while(I2S_AD16_GET_WP() != 0);		   /* FIXME: why ? */
#endif

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
			bufSize = (cWP + 32 )&(~0x1F); /*FixMe: Do 9920 use 32 and 0x1F?? */
	
			ori_size = ithReadRegA(I2S_REG_OUT_LEN);
			
			ithWriteRegA(I2S_REG_OUT_LEN, bufSize-1);			
			
			//printf("	getObufLen1:[%04x,%04x]cWP=%x,bufsz=%x\n",reg_read16(I2S_REG_OBUF_LEN_HI),reg_read16(I2S_REG_OBUF_LEN_LO),cWP,bufSize); 			
	
			bufBase = ithReadRegA(I2S_REG_OUT_BASE1);
			oBuf = (u8*)bufBase;
#if 0 //FIXME: reset DAC memory, 9920 maybe open			
			memset((u8*)&oBuf[cWP], 0, bufSize-cWP);
#endif
			/* TODO: set other OutBuffer as 0(NOT only buffer1) */	  
			
			I2S_DA32_SET_WP(0);
			I2S_AD32_SET_RP(0, 1);
#if 0 //FIXME: reset DAC memory, 9920 maybe open			
			while(I2S_DA32_GET_RP() || I2S_AD32_GET_WP(1));
#endif
			
			/* restore buffer size*/
			ithWriteRegA(I2S_REG_OUT_LEN, ori_size);
			
			//printf("	getObufLen2:[%04x,%04x]\n",reg_read16(I2S_REG_OBUF_LEN_HI),reg_read16(I2S_REG_OBUF_LEN_LO),cWP,ori_size);	
		}
	}


	printf("I2S# %s\n", __func__);

}

static void _i2s_aws_sync()
{
    ithWriteRegA(I2S_REG_CODEC_SET,
    ( 1                                  << 31) //ADCWS/DACWS syn 
    |( I2S_INTERNAL_CODEC                <<  2) //select External/Internal DAC
    |( I2S_SLAVE_MODE                    <<  1) //IIS Output Slave/Master Mode
    |( I2S_SLAVE_MODE                    <<  0) //IIS Input Slave/Master Mode
    ); 
	usleep(10);
    
    ithWriteRegMaskA(I2S_REG_CODEC_SET, 0, 1 << 31);
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
	
////9920 tempoarily use
/* When sample rate=48K, 9850 and 9920 have different behavior */
/* 9850: ZCLK ratio=0x11, AMCLK ratio=0x0, use PLL2 */
/* 9920: ZCLK ratio=0x111, AMCLK ratio=0x2 use PLL3 */

	//only for WS=48k 

	ithWriteRegA(MMP_AUDIO_CLOCK_REG_3C, 0xf2aac808); //reset
//	ithWriteRegA(0xD800003c, 0xf2aac803); //why not ZCLK ratio = 0x11, 9850 is 0x11 for 48K
	usleep(10);
	ithWriteRegA(MMP_AUDIO_CLOCK_REG_3C, 0x02aac808); //reset	//ZCLK	PLL3_OUT1 / d (i2s	 3.072MHz,WS=48K)
	usleep(1);

	ithWriteRegA(MMP_AUDIO_CLOCK_REG_40, 0x0000c802);		//AMCLK PLL3_OUT1 / 3 (AMCLK 12.288MHz)



}

static void _i2s_enable_fading(u32 fading_step, u32 fading_duration)
{
	u32 data32;
	
	ithWriteRegMaskA(I2S_REG_OUT_FADE_SET, 1<<0, 1<<0); //Enable OUT_BASE1 fade
	data32 = ithReadRegA(I2S_REG_OUT_FADE_SET);
	data32 |= ((fading_step & 0xFFFF) << 8);
	data32 |= ((fading_duration & 0xFFFF) << 16);
	ithWriteRegA(I2S_REG_OUT_FADE_SET, data32);	
}

static void _i2s_disable_fading(void)
{
	ithWriteRegMaskA(I2S_REG_OUT_FADE_SET, 0, 1<<0);  //Disable OUT_BASE1 fade
}

static void _i2s_use_GPIO_MODE(void) /* GPIO settings for CFG2 */
{
    ithWriteRegA(0xd1000068, 0x44440000);    //GPIO[23:16]
    ithWriteRegA(0xd100006C, 0x00000004);    //GPIO[31:24]

    //HDMIRX_1  (mode4 40~46)
    ithWriteRegA(0xd10000E4, 0x04444444);    //GPIO[47:40]

    //HDMITX_1  (mode4 57~63)
    ithWriteRegA(0xd10000EC, 0x44444440);    //GPIO[63:56]
}

/* ************************************************************************** */

void i2s_volume_up(void)
{
	// amp use. useless function
}

void i2s_volume_down(void)
{
	// amp use. useless function
}

void i2s_pause_ADC(int pause)
{
	u32 data32;

	printf("I2S# %s(%d)\n", __func__, pause);

	if(pause)
	{
		ithWriteRegMaskA(I2S_REG_IN_CTRL, 0, 1<<0);

	}
	else /* resume */
	{
		ithWriteRegMaskA(I2S_REG_IN_CTRL, 1<<0, 1<<0);

	}
}

void i2s_pause_DAC(int pause)
{
	printf("I2S# %s(%d)\n", __func__, pause);

	if(pause)
	{
#if 0   //fading
		_i2s_enable_fading(0xF, 0x01); /* fast response */
//		_i2s_enable_fading(0x7, 0x7F); /* moderato */
//		_i2s_enable_fading(0x1, 0xFF); /* slow response */
		ithWriteRegA(I2S_REG_OUT_VOLUME, 0x7F0000);
#endif

		ithWriteRegMaskA(I2S_REG_OUT_CTRL, 0, 1<<0);
	}
	else /* resume */
	{
#if 0   //fading
		_i2s_enable_fading(0x1, 0xFF); /* slow response */
		ithWriteRegA(I2S_REG_OUT_VOLUME, 0x7F0100);
#endif
		ithWriteRegMaskA(I2S_REG_OUT_CTRL, 1<<0, 1<<0);
	}
}

void i2s_deinit_ADC(void)
{
    u32 data32;
    
	if(0 == _i2s_AD_running) {
		printf("I2S# ADC is 'NOT' running, skip deinit ADC !\n");
		return;
	}

	printf("I2S# %s +\n", __func__);

    pthread_mutex_lock(&I2S_MUTEX);

	/* disable hardware I2S */
	{
		data32 = ithReadRegA(I2S_REG_IN_CTRL);
		data32 &= ~(3 << 0);
		ithWriteRegA(I2S_REG_IN_CTRL, data32);
	}

	_i2s_AD_running = 0; /* put before _i2s_reset() */
	_i2s_reset();

	_i2s_power_off();

	pthread_mutex_unlock(&I2S_MUTEX);
	printf("I2S# %s -\n", __func__);

}

void i2s_deinit_DAC(void)
{
    u32 data32;
    u32 out_status;
//  u32 i2s_idle;
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

#ifdef	ENABLE_FORCE_RESET_RW_POINTER
    if(_i2s_AD_running) _i2s_reset_DA_RWptr();
#endif

    /* disable I2S_OUT_FIRE & I2S_OUTPUT_EN */
    data32 = ithReadRegA(I2S_REG_OUT_CTRL);
    data32 &= ~(0x3);
    ithWriteRegA(I2S_REG_OUT_CTRL, data32);


#ifdef I2S_DEBUG_DEINIT_DAC_COST
    gettimeofday(&tv_pollS, NULL); //-*-
#endif

    do
    {
//      i2s_delay_us(1000); /* FIXME: dummy loop */;

        out_status = ithReadRegA(I2S_REG_OUT_STATUS1);

      //printf("I2S# OUTPUT_STATUS 1: %0x\n", out_status);

//      i2s_idle   =  out_status & 0x1;
        pipe_idle  = (out_status >> 1) & 0x1;
        i2s_memcnt = (out_status >> 20) & 0x7F;
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
	int param_fail = 0;
	u32 data32 = 0;
	u8 resolution_type;
    u8 hdmi_num;

	if(_i2s_DA_running) {
		printf("I2S# DAC is running, skip re-init !\n");
		return;
	}
	if(((u32)i2s_spec->base_i2s % 8) || (i2s_spec->buffer_size % 8)) {
		printf("ERROR# I2S, bufbase/bufsize must be 8-Bytes alignment !\n");
		return;
	}   
	if((i2s_spec->channels != 1) && (i2s_spec->channels != 2)) {
		printf("ERROR# I2S, only support single or two channels !\n");
		return;
	}    

    printf("I2S# %s Start\n", __func__);
    pthread_mutex_lock(&I2S_MUTEX);
//	_i2s_power_on();

	_i2s_use_GPIO_MODE();

	_i2s_set_clock(i2s_spec->sample_rate);

	_i2s_reset();	

	/* mastor mode & AWS & ZWS sync */
	switch(i2s_spec->sample_size) {
		case 16: { resolution_type = 0; break; }
		case 24: { resolution_type = 1; break; }
		case 32: { resolution_type = 2; break; }
        case 8 : { resolution_type = 3; break; }
		default: { resolution_type = 2; break; }
	}

	ithWriteRegA(I2S_REG_DAC_SRATE_SET,
    (resolution_type	 << 28) /* DAC resolution bits, 00:16bits; 01:24bits; 10:32bits; 11:8bits */
	| (1			     << 24) /* WS mute check, 0:Disable 1:Enable */
	| (0xF               << 16) /* WS mute check times */
	| (0x1F 		     <<  0) /* sample width (in bits unit) */
    );

	/* 9920 remove PCM support*/

	_i2s_aws_sync();

	/* buffer base */
    for (int i = 0; i<4; i++) {
        if(i2s_spec->base_hdmi[i] == NULL) {i2s_spec->base_hdmi[i] = i2s_spec->base_i2s;}
    }
	ithWriteRegA(I2S_REG_OUT_BASE1, (u32)i2s_spec->base_i2s    );/*(IIS /SPDIF out) base 1*/
    ithWriteRegA(I2S_REG_OUT_BASE2, (u32)i2s_spec->base_hdmi[0]);/*(HDMI Data0 out) base 2*/
    ithWriteRegA(I2S_REG_OUT_BASE3, (u32)i2s_spec->base_hdmi[1]);/*(HDMI Data1 out) base 3*/
    ithWriteRegA(I2S_REG_OUT_BASE4, (u32)i2s_spec->base_hdmi[2]);/*(HDMI Data2 out) base 4*/
    ithWriteRegA(I2S_REG_OUT_BASE5, (u32)i2s_spec->base_hdmi[3]);/*(HDMI Data3 out) base 5*/

	/* buffer length */
	ithWriteRegA(I2S_REG_OUT_LEN, i2s_spec->buffer_size - 1);

	/* DA starvation interrupt threshold */
	ithWriteRegA(I2S_REG_OUT_RdWrGAP, 0x00000080);   //OutRdWrGap[31:0]

	/* output path */
    hdmi_num = 0;
    switch(i2s_spec->num_hdmi_audio_buffer){        
        case 1: { hdmi_num |=  1; break; } /* buf 1 (HDMI Data0) */
        case 2: { hdmi_num |=  3; break; }
        case 3: { hdmi_num |=  7; break; }
        case 4: { hdmi_num |= 15; break; }
        case 0:
        default: break;
    }      
       
    /*should 9920 set W0OutRqSize[3:0]=0b1000 ?*/
	ithWriteRegA(I2S_REG_OUT_CTRL,
    (  hdmi_num                          << 9) /* HDMI DATA Output*/
    | (1                                 << 8) /* Enable IIS 1/SPDIF Data Output */
    | (I2S_SWITCHCHANNEL                 << 6) /* Output Channel active level, 0:Low for Left; 1:High for Left */
	| (I2S_MSB_MODE                      << 5) /* Output Interface Format, 0:IIS Mode; 1:MSB Left-Justified Mode */
	| ((i2s_spec->channels - 1)          << 4) /* Output Channel Select, 0:Single Channel; 1:Two Channels */
	| ((i2s_spec->is_big_endian ? 1 : 0) << 3) /* 0:Little Endian; 1:Big Endian */
	| (0                                 << 2) /* 0:NOT_LAST_WPTR; 1:LAST_WPTR */
	| (1                                 << 1) /* Fire the IIS for Audio Output */
	);
	

	if(!i2s_spec->postpone_audio_output) {
		i2s_pause_DAC(0); /* Enable Audio Output */
	}
	else
	    i2s_pause_DAC(1);

	_i2s_DA_running = 1;

    
    printf("I2S# %s End\n", __func__);
	pthread_mutex_unlock(&I2S_MUTEX);
    
}

void i2s_init_ADC(STRC_I2S_SPEC *i2s_spec)
{
	u32 data32 = 0;
	u8 resolution_type;
	u8 hdmi_num;
    
	if(_i2s_AD_running) {
		printf("I2S# ADC is running, skip re-init ADC !\n");
		return;
	}   
	if(((u32)i2s_spec->base_i2s % 8) || (i2s_spec->buffer_size % 8)) {
		printf("ERROR# I2S, bufbase/bufsize must be 8-Bytes alignment !\n");
		return;
	}   
	if((i2s_spec->channels != 1) && (i2s_spec->channels != 2)) {
		printf("ERROR# I2S, only support single or two channels !\n");
		return;
	}

    printf("I2S# %s Start\n", __func__);
	pthread_mutex_lock(&I2S_MUTEX);

//	_i2s_power_on();

	_i2s_use_GPIO_MODE();

	_i2s_set_clock(i2s_spec->sample_rate);

	_i2s_reset();

//	if     ( i2s_spec->from_LineIN && !i2s_spec->from_MIC_IN) { itp_codec_rec_init(1); } /* LineIN only */
//	else if(!i2s_spec->from_LineIN &&  i2s_spec->from_MIC_IN) { itp_codec_rec_init(2); } /* MICIN only */
//	else                                                      { itp_codec_rec_init(0); } /* MICIN only */   

	/* mastor mode & AWS & ZWS sync */
	switch(i2s_spec->sample_size) {
		case 16: { resolution_type = 0; break; }
		case 24: { resolution_type = 1; break; }
		case 32: { resolution_type = 2; break; }
        case 8 : { resolution_type = 3; break; }
		default: { resolution_type = 2; break; }
	}

	/* config AD */
	data32 = (resolution_type   << 28) /* ADC resolution bits, 00:16bits; 01:24bits; 10:32bits; 11:8bits */
            |(1			        << 24) /* WS mute check, 0:Disable 1:Enable */
            |(0xF               << 16) /* WS mute check times */
            |(0x1F 		        <<  0); /* sample width (in bits unit) */
	ithWriteRegA(I2S_REG_ADC_SRATE_SET, data32);


	_i2s_aws_sync();

	/* buffer base */
    for (int i = 0; i<4; i++) {
        if(i2s_spec->base_hdmi[i] == NULL) {i2s_spec->base_hdmi[i] = i2s_spec->base_i2s;}
    }    
	ithWriteRegA(I2S_REG_IN1_BASE1, (u32)i2s_spec->base_i2s    ); /*(I2S  Data  in ) IN1 base1*/
    ithWriteRegA(I2S_REG_IN2_BASE1, (u32)i2s_spec->base_hdmi[0]); /*(HDMI Data1 in)  IN2 base1*/
    ithWriteRegA(I2S_REG_IN2_BASE2, (u32)i2s_spec->base_hdmi[1]); /*(HDMI Data2 in)  IN2 base2*/
    ithWriteRegA(I2S_REG_IN2_BASE3, (u32)i2s_spec->base_hdmi[2]); /*(HDMI Data3 in)  IN2 base3*/
    ithWriteRegA(I2S_REG_IN2_BASE4, (u32)i2s_spec->base_hdmi[3]); /*(HDMI Data4 in)  IN2 base4*/


	/* buffer length */
	ithWriteRegA(I2S_REG_IN_LEN, i2s_spec->buffer_size - 1);


	/* DA starvation interrupt threshold */
	ithWriteRegA(I2S_REG_IN_RdWrGAP, 0x00000040);   //OutRdWrGap[31:0]

	
    hdmi_num = 0;
    switch (i2s_spec->num_hdmi_audio_buffer){
        case  1: { hdmi_num |= 1 ; break; } /* buf 1 (HDMI Data0) */
        case  2: { hdmi_num |= 3 ; break; }
        case  3: { hdmi_num |= 7 ; break; }
        case  4: { hdmi_num |= 15; break; }
        default: { hdmi_num |= 0 ; break; }
    }
    
	ithWriteRegA(I2S_REG_IN_CTRL,
    (  hdmi_num                          <<12) /* Eanble HDMI Data Input */
	| (1                                 << 8) /* Eanble Input1 IIS Data Input */
	| (I2S_SWITCHCHANNEL                 << 6) /* Input Channel active level, 0:Low for Left; 1:High for Left */
	| (I2S_MSB_MODE                      << 5) /* Input Interface Format, 0:IIS Mode; 1:MSB Left-Justified Mode */
	| ((i2s_spec->channels - 1)          << 4) /* Input Channel, 0:Single Channel; 1:Two Channels */
	| ((i2s_spec->is_big_endian ? 1 : 0) << 3) /* 0:Little Endian; 1:Big Endian */
	| ((i2s_spec->record_mode ? 1 : 0)   << 2) /* 0:AV Sync Mode (wait capture to start); 1:Only Record Mode */
	| (1                                 << 1) /* Fire the IIS for Audio Input */
	| (1                                 << 0) /* Enable Audio Input */
	);


	_i2s_AD_running = 1;

	pthread_mutex_unlock(&I2S_MUTEX);
	printf("I2S# %s End\n", __func__);

}

int  i2s_get_DA_running(void)
{
    return _i2s_DA_running;
}

void i2s_mute_DAC(int mute)
{
#if I2S_INTERNAL_CODEC

#else
    
#endif

}

void i2s_set_direct_volperc(unsigned volperc)
{   /*volperc: 0~100*/
    itp_codec_playback_set_direct_volperc(volperc);
}

void i2s_set_direct_volstep(unsigned int volstep)
{   /*volstep: 0x00~0xFF,0x100;*/
    itp_codec_playback_set_direct_vol(volstep);
}

unsigned i2s_get_current_volperc(void)
{
    unsigned currvolperc = 0;
    itp_codec_playback_get_currvol(&currvolperc);
	return currvolperc;
}

void i2s_ADC_set_rec_volperc(unsigned micperc)
{   /*volperc: 0~100*/
    itp_codec_rec_set_direct_volperc(micperc);
}

int i2s_ADC_set_direct_volstep(unsigned int  volstep)
{   /*volstep: 0x00~0xFF,0x100;*/
    itp_codec_rec_set_direct_vol(volstep);
	return 0;
}

unsigned i2s_ADC_get_current_volstep(void)
{
    unsigned currmicperc = 0;
    itp_codec_rec_get_currvol(&currmicperc);
	return currmicperc;
}

void i2s_mute_ADC(int mute)
{
#if I2S_INTERNAL_CODEC

#else
    
#endif
}

void i2s_set_linein_bypass(int bypass)
{
    
}

static void _init_spdif(u32 sample_rate)
{
}

static void _deinit_spdif(void)
{
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
	printf("	@base_spdif	%x\n",i2s_spec->base_spdif);
	printf("	@aud_out	%x\n",i2s_spec->postpone_audio_output);
	printf("	@record_mode	%x\n",i2s_spec->record_mode);
	printf("	@Speaker	%x\n",i2s_spec->enable_Speaker);
	printf("	@HeadPhone	%x\n",i2s_spec->enable_HeadPhone);
}

void i2s_mute_volume(int mute)
{

}

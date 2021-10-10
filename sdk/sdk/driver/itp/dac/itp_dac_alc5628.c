#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "ite/ith.h"
#include "ite/itp_codec.h"
#include "iic/mmp_iic.h"

#define ALC5628_I2CADR               (0x30 >> 1)

#if CFG_DAC_port_I2C0
#define IIC_PORT IIC_PORT_0
#else
#define IIC_PORT IIC_PORT_1
#endif  

//#define DEBUG_PRINT printf
#define DEBUG_PRINT(...)

/* ************************************************************************** */
/* wrapper */
static inline void i2s_delay_us(unsigned us) { ithDelay(us); }

/* ************************************************************************** */
#define MAX_OUT1_VOLUME            0x00 /* +12 dB */
#define DEFAULT_DB_OUT1_VOLUME     0x10 /*  0 dB */
#define MIN_OUT1_VOLUME            0xFF /* -35.25 dB */
static unsigned curr_out1_volume = DEFAULT_DB_OUT1_VOLUME; /* 0 dB */

#define MAX_INPUT_PGA_VOLUME       0x7F /* +30   dB */
#define ZERO_DB_INPUT_PGA_VOLUME   0x2F /* +0    dB */
#define MIN_INPUT_PGA_VOLUME       0x00 /* -17.625 dB */

static unsigned curr_input_pga_volume = ZERO_DB_INPUT_PGA_VOLUME; /* 0 dB, refer to R0, R1 */

static int _alc5628_DA_running = 0;
static int _alc5628_AD_running = 0;
static pthread_mutex_t ALC5628_MUTEX = PTHREAD_MUTEX_INITIALIZER;

static int _alc5628_micin = 1;
static int _alc5628_linein = 0;
static int _alc5628_i2sin  = 1;
static int _alc5628_spkout = 1;
static int _alc5628_hpout = 1;

static int _alc5628_cold_start = 1; /* alc5628q should enable power only once */

static int i2s_sample_rate = 48000;

/* ************************************************************************** */
/* gamma correction */
#include "gc9.2.inc"
//#include "gc3.2.inc"
//#include "gc4.2.inc"
//#include "gc5.2.inc"
//#include "gc6.2.inc"

/* ************************************************************************** */
static void I2C_common_write_word(unsigned char RegAddr, unsigned short d)
{
	int success = 0;
	int flag;
	int retry = 0;
    unsigned short tmp = 0;

	if (CFG_CHIP_FAMILY == 9910)
		tmp = d;
	else
    	tmp = (d&0x00FF) << 8 | (d&0xFF00)  >> 8;
    
    ithWriteRegMaskA(ITH_GPIO_BASE | 0xD0, (0 << 28), (0x3 << 28)); /* IIC: internal HDMI IIC */
    for(retry=0; retry<50; retry++)
    {
        if(0 == (flag = mmpIicSendData(IIC_PORT, IIC_MASTER_MODE, ALC5628_I2CADR, RegAddr, &tmp, 2)))
        {
        	success = 1;
#ifdef ALC5628_DEBUG_I2C_COMMON_WRITE
        	printf("ALC5628_I2CADR# IIC WriteOK!\n");
#endif
            break;
        }
    }
    if(success == 0) {
    	printf("ALC5628# IIC Write Fail!\n");
    	//while(1) { usleep(500000); }
    }
    ithWriteRegMaskA(ITH_GPIO_BASE | 0xD0, (0 << 28), (0x3 << 28)); /* IIC: external APB IIC */
}

static unsigned short I2C_common_read_word(unsigned char RegAddr)
{
    
    int success = 0;
    int flag;
    int retry = 0;
    unsigned short d, tmp = 0;
    
    ithWriteRegMaskA(ITH_GPIO_BASE | 0xD0, (0 << 28), (0x3 << 28)); /* IIC: internal HDMI IIC */
    for(retry=0; retry<50; retry++)
    {
        if(0 == (flag = mmpIicReceiveData(IIC_PORT, IIC_MASTER_MODE, ALC5628_I2CADR, &RegAddr, 1, &d, 2)))
        {
            success = 1;
#ifdef ALC5628_DEBUG_I2C_COMMON_WRITE
            printf("ALC5628_I2CADR# IIC Read OK!\n");
#endif
            break;
        }
    }
    if(success == 0) {
        printf("ALC5628# IIC Write Fail!\n");
        while(1) { usleep(500000); }
    }
    ithWriteRegMaskA(ITH_GPIO_BASE | 0xD0, (0 << 28), (0x3 << 28)); /* IIC: external APB IIC */

	if (CFG_CHIP_FAMILY == 9910)
		tmp = d;
	else
    	tmp = (d&0x00FF) << 8 | (d&0xFF00)  >> 8;
    
    return tmp ;
}

static void alc5628_write_reg(unsigned char reg_addr, unsigned short value)
{

#ifdef ALC5628_DEBUG_PROGRAM_REGVALUE
	printf("ALC5628# write reg[0x%02x] = 0x%04x\n", reg_addr, value);
#endif
	I2C_common_write_word(reg_addr, value);
}

static unsigned short alc5628_read_reg(unsigned char reg_addr)
{    
    return I2C_common_read_word(reg_addr);
}

static inline void alc5628_writeRegMask(uint16_t addr, uint16_t data, uint16_t mask)
{
    alc5628_write_reg(addr, (alc5628_read_reg(addr) & ~mask) | (data & mask));
}

static void Dump_Register(void)
{

	unsigned short tempRegisterValue;
	unsigned char addr; 

	for (addr=0; addr <= 0xFF; addr += 1)
	{
		tempRegisterValue = alc5628_read_reg(addr);
		printf("reg[0x%02x] = 0x%04x\n", addr, tempRegisterValue);
		if (addr == 0xFF)
		{
		    tempRegisterValue = alc5628_read_reg(addr);
		    printf("reg[0x%02x] = 0x%04x\n", addr, tempRegisterValue);
		    break;
		}    
	}

}

static void init_alc5628_common(void)
{
    pthread_mutex_lock(&ALC5628_MUTEX);

	DEBUG_PRINT("ALC5628# %s\n", __func__);

	if(_alc5628_DA_running)
	{
		DEBUG_PRINT("ALC5628# DAC is running, skip re-init process !\n");
		pthread_mutex_unlock(&ALC5628_MUTEX);
		return;
	}
	if(_alc5628_AD_running)
	{
		DEBUG_PRINT("ALC5628# ADC is running, skip re-init process !\n");
		pthread_mutex_unlock(&ALC5628_MUTEX);
		return;
	}
	_alc5628_cold_start = 0;

#if 1
	/* start programming ALC5628 */
	{
        unsigned short data16;
        
        alc5628_write_reg(0x00, 0x0003); /* reset ALC5628 */
		
        /* Power Management Ctrl */
		alc5628_write_reg(0x3A, 0xC130);/* Power on I2S Digital Interface Enable Ctrl */
		alc5628_write_reg(0x3C, 0x7FF8);/* Power on Mixer*/
		alc5628_write_reg(0x3E, 0xF6F0);/* power on amplifer*/
		
		alc5628_write_reg(0x02, 0x0000); // SPK OUT VOL 0db
		alc5628_write_reg(0x04, 0x0000); // HP OUT VOL 0db

		alc5628_write_reg(0x16, 0x000A);
		alc5628_write_reg(0x18, 0x0000);
		alc5628_write_reg(0x1C, 0x8B04);
		alc5628_write_reg(0x38, 0x0000);
		alc5628_write_reg(0x40, 0x0200);
		//alc5628_write_reg(0x42, 0xC000);
		//alc5628_write_reg(0x44, 0xBEA0);

        /* DAC Ctrl */       
        //alc5628_write_reg(0x0C, 0x1010);/* DAC Digital Volume ctrl 0 dB */
		//alc5628_write_reg(0x1C, 0x8860);/* OUTPUT Mixer Control */
		//alc5628_write_reg(0x0C, 0x3F3F);	
		//alc5628_write_reg(0x1C, 0x0B04);
        if(_alc5628_i2sin)
        {
            unsigned short adj = curr_out1_volume;
            adj = (adj << 8)|adj;
            alc5628_write_reg(0x0C, adj); //DAC Digital Volume mute
        }
        else
        {
             alc5628_write_reg(0x0C, 0xFFFF);
        }        
        if(_alc5628_linein)        
        {
            unsigned short adj = curr_out1_volume;
            adj>>=1;
            adj = (adj << 8)|adj;            
	        alc5628_write_reg(0x0A, adj);/* LINE_IN 0db*/
        }
        else
        {
            alc5628_write_reg(0x0A, 0xC8C8);/* LINE_IN Disable */
        }

		/* MISC1/MISC2 Control */
		//alc5628_write_reg(0x5C, 0xFFFF);
		alc5628_write_reg(0x5E, 0x83E0);

		/* AVC Control */
		//alc5628_write_reg(0x68, 0x000B);
		
#if 1 /* RealTek's new flow @ 2015-0211, 12:51 */        
        if(_alc5628_spkout)/* Enable SPK_L and SPK_R */
        {
        	DEBUG_PRINT("---- _alc5628_spkout ----\n");
            alc5628_write_reg(0x02, 0x0000);/* unmute speaker, 46.5db */
			alc5628_write_reg(0x04, 0x0000);
        }
        else
        {
            alc5628_write_reg(0x02, 0x8080);/* mute speaker, 46.5db */
			alc5628_write_reg(0x04, 0x8080);
        }
        usleep(50000);/* Delay (50 ms) to allow HP amps to settle */
    }
    //Dump_Register();
#endif
#endif
    pthread_mutex_unlock(&ALC5628_MUTEX);
}

static void deinit_alc5628_common(void)
{
	pthread_mutex_lock(&ALC5628_MUTEX);
	DEBUG_PRINT("ALC5628# %s\n", __func__);

	if(_alc5628_DA_running)
	{
		DEBUG_PRINT("ALC5628# DAC is running, skip deinit !\n");
		pthread_mutex_unlock(&ALC5628_MUTEX);
		return;
	}
	if(_alc5628_AD_running)
	{
		DEBUG_PRINT("ALC5628# ADC is running, skip deinit !\n");
		pthread_mutex_unlock(&ALC5628_MUTEX);
		return;
	}
	
//	i2s_delay_us(25000); /* FIXME: dummy loop */

	/* DON'T turn off power in any case */
	if     (curr_out1_volume < MAX_OUT1_VOLUME) { curr_out1_volume = MAX_OUT1_VOLUME; }
	else if(curr_out1_volume > MIN_OUT1_VOLUME) { curr_out1_volume = MIN_OUT1_VOLUME; }

	/* just decrease output volume */

    if(_alc5628_i2sin)
        alc5628_write_reg(0x0C, 0xFFFF);
    if(_alc5628_linein)
        alc5628_write_reg(0x0A, 0xC8C8);
	pthread_mutex_unlock(&ALC5628_MUTEX);
	return;
}
void itp_codec_wake_up(void)
{
	pthread_mutex_lock(&ALC5628_MUTEX);
	printf("ALC5628# %s\n", __func__);
	init_alc5628_common();
	_alc5628_DA_running = 1;
	init_alc5628_common();
	_alc5628_AD_running = 1;
	pthread_mutex_unlock(&ALC5628_MUTEX);
}
void itp_codec_standby(void)
{
	pthread_mutex_lock(&ALC5628_MUTEX);
	printf("ALC5628# %s\n", __func__);
	itp_codec_playback_deinit();
	itp_codec_rec_deinit();
	alc5628_write_reg(0x00, 0x00);
	pthread_mutex_unlock(&ALC5628_MUTEX);
}

/* DAC */
void itp_codec_playback_init(unsigned output)
{
	DEBUG_PRINT("ALC5628# %s\n", __func__);

	switch(output)
	{
		case 1: /* Speaker only */
		{
			_alc5628_spkout  = 1;
			_alc5628_hpout = 0;
			break;
		}
		case 2: /* both Speaker & HeadPhone */
		{
			_alc5628_spkout  = 1;
			_alc5628_hpout = 1;
			break;
		}
		case 0: /* HeadPhone only */
		default:
		{
			_alc5628_spkout  = 0;
			_alc5628_hpout = 1;
			break;
		}
	}
	
	init_alc5628_common();
	_alc5628_DA_running = 1;
}

void itp_codec_playback_deinit(void)
{
	DEBUG_PRINT("ALC5628# %s\n", __func__);

	_alc5628_DA_running = 0; /* put before deinit_alc5628_common() */
	deinit_alc5628_common();
}

void itp_codec_playback_amp_volume_down(void)
{
	if(curr_out1_volume <= MIN_OUT1_VOLUME) return;

	switch(curr_out1_volume)
	{
		//case 0xA8 ... 0xAF: { itp_codec_playback_set_direct_vol(0xA7); break; } /* -3 dB */
		//case 0xA0 ... 0xA7: { itp_codec_playback_set_direct_vol(0x9F); break; } /*  -6 dB */
		//case 0x98 ... 0x9F: { itp_codec_playback_set_direct_vol(0x97); break; } /* -9 dB */
		case 0x30 ... 0x3F: { itp_codec_playback_set_direct_vol(0x2F); break; } /* -12 dB */
		case 0x20 ... 0x2F: { itp_codec_playback_set_direct_vol(0x1F); break; } /* -24dB */
		case 0x10 ... 0x1F: { itp_codec_playback_set_direct_vol(0x0F); break; } /* -30dB */
		case 0x00 ... 0x0F: { itp_codec_playback_set_direct_vol(0x00); break; } /* -39dB */
	}
}

void itp_codec_playback_amp_volume_up(void)
{
	if(curr_out1_volume >= MAX_OUT1_VOLUME) return;

	switch(curr_out1_volume)
	{
		//case 0xA7 ... 0xAE: { itp_codec_playback_set_direct_vol(0xAF); break; } /* +0 dB */
		//case 0x9F ... 0xA6: { itp_codec_playback_set_direct_vol(0xA7); break; } /* -3 dB */
		//case 0x97 ... 0x9E: { itp_codec_playback_set_direct_vol(0x9F); break; } /*  -6 dB */
		case 0x2F ... 0x3E: { itp_codec_playback_set_direct_vol(0x3F); break; } /* -9 dB */
		case 0x1F ... 0x2E: { itp_codec_playback_set_direct_vol(0x2F); break; } /* -12 dB */
		case 0x0F ... 0x1E: { itp_codec_playback_set_direct_vol(0x1F); break; } /* -24dB */
		case 0x00 ... 0x0E: { itp_codec_playback_set_direct_vol(0x0F); break; } /* -30dB */
	}
}

void itp_codec_playback_set_direct_vol(unsigned target_vol)
{
	int direction;
	unsigned short data16;
	unsigned short adj, tmp = 0x00;

	pthread_mutex_lock(&ALC5628_MUTEX);

	
	if(target_vol < MAX_OUT1_VOLUME)
	{
		 printf("ERROR# invalid target volume step: 0x%08x\n", target_vol);
		 pthread_mutex_unlock(&ALC5628_MUTEX);
		 return;
	}

	if     (target_vol == curr_out1_volume) { pthread_mutex_unlock(&ALC5628_MUTEX); return; }
	else if(target_vol >  curr_out1_volume) { direction = 0; } /* - */
	else                                    { direction = 1; } /* + */
	
	while(curr_out1_volume != target_vol)
	{
        curr_out1_volume = target_vol;
		//if(direction == 1) { curr_out1_volume--; }
		//else               { curr_out1_volume++; }

		adj = curr_out1_volume;		

		if(_alc5628_DA_running)
		{
		    tmp = (adj<<8) | adj;
            
            if(_alc5628_i2sin)
                alc5628_write_reg(0x0C, tmp);
            
            if(_alc5628_linein){
                adj>>=1;
                tmp = (adj<<8) | adj;
                alc5628_write_reg(0x0A, tmp);
            }
		}

		i2s_delay_us(1000); /* FIXME: dummy loop */;
	}
	pthread_mutex_unlock(&ALC5628_MUTEX);
}

void itp_codec_playback_set_direct_volperc(unsigned target_volperc)
{
	unsigned char volstep;

	if(alc5628_perc_to_reg_tableSize != 100) {
		printf("ERROR# invalid alc5628_perc_to_reg_tableSize !\n");
		return;
	}
	if(target_volperc >= 100) { target_volperc = 99; }

	volstep = alc5628_perc_to_reg_table[target_volperc];
	itp_codec_playback_set_direct_vol(volstep);
}

void itp_codec_playback_get_currvol(unsigned *currvol)
{
	*currvol = curr_out1_volume;
}

void itp_codec_playback_get_currvolperc(unsigned *currvolperc)
{
	unsigned i;

	for(i=0; i<99; i++)
	{
		if((alc5628_perc_to_reg_table[i] >= curr_out1_volume)
		&& (curr_out1_volume > alc5628_perc_to_reg_table[i+1])) {
			*currvolperc = i;
			return;
		}
	}
	*currvolperc = 99;
}

void itp_codec_playback_get_vol_range(unsigned *max, unsigned *regular_0db, unsigned *min)
{
	*max         = MAX_OUT1_VOLUME;
	*regular_0db = DEFAULT_DB_OUT1_VOLUME;
	*min         = MIN_OUT1_VOLUME;
}

void itp_codec_playback_mute(void)
{	
	pthread_mutex_lock(&ALC5628_MUTEX);

    DEBUG_PRINT("%s (%d)\n", __FUNCTION__, __LINE__);
	    		
    /* just decrease output volume */
    if(_alc5628_i2sin)
        alc5628_write_reg(0x0C, 0xFFFF);/*Stereo DAC digital mixer control ,note:if comment, farend have no voice when communicating press mute */
    if(_alc5628_linein)
        alc5628_write_reg(0x0A, 0xC8C8);/* LINE_IN Disable */
    //i2s_delay_us(1000); /* FIXME: dummy loop */

	pthread_mutex_unlock(&ALC5628_MUTEX);
}

void itp_codec_playback_unmute(void)
{	
	pthread_mutex_lock(&ALC5628_MUTEX);
	
    unsigned short data16;
    unsigned short adj, tmp = 0x00;

    DEBUG_PRINT("%s (%d)\n", __FUNCTION__, __LINE__);

	if     (curr_out1_volume < MAX_OUT1_VOLUME) { curr_out1_volume = MAX_OUT1_VOLUME; }
	else if(curr_out1_volume > MIN_OUT1_VOLUME) { curr_out1_volume = MIN_OUT1_VOLUME; }

    adj = curr_out1_volume;

	i2s_delay_us(1000); /* FIXME: dummy loop */
	
    if(_alc5628_i2sin){
        tmp = (adj << 8) | adj;
        alc5628_write_reg(0x0C, tmp);
    }
    
    if(_alc5628_linein){
        adj>>=1;
        adj = (adj << 8)|adj;            
        alc5628_write_reg(0x0A, adj);/* LINE_IN 0db*/    
    }
    
	i2s_delay_us(1000); /* FIXME: dummy loop */

	pthread_mutex_unlock(&ALC5628_MUTEX);	
}

/* line-in bypass to line-out directly */
void itp_codec_playback_linein_bypass(unsigned bypass)
{
	uint16_t	reg16;
	uint16_t	BpVol=7;	
	
//	pthread_mutex_lock(&ALC5628_MUTEX);


//	pthread_mutex_unlock(&ALC5628_MUTEX);
}


/* ADC */
void itp_codec_rec_init(unsigned input_source)
{
	DEBUG_PRINT("ALC5628# %s\n", __func__);
	
	switch(input_source)
	{
		case 1: /* LineIN only */
		{
			_alc5628_micin  = 0;
			_alc5628_linein = 1;
			break;
		}
		case 2: /* MICIN only */
		{
			_alc5628_micin  = 1;
			_alc5628_linein = 0;
			break;
		}
		case 0: /* both LineIN & MICIN */
		default:
		{
			_alc5628_micin  = 1;
			_alc5628_linein = 1;
			break;
		}
	}

	init_alc5628_common();
	_alc5628_AD_running = 1;
}

void itp_codec_rec_deinit(void)
{
	DEBUG_PRINT("ALC5628# %s\n", __func__);

	_alc5628_AD_running = 0; /* put before deinit_alc5628_common() */
	deinit_alc5628_common();
}

void itp_codec_rec_set_direct_vol(unsigned target_vol)
{
	return;
}

void itp_codec_rec_get_currvol(unsigned *currvol)
{
	*currvol = curr_input_pga_volume;
}

void itp_codec_rec_get_vol_range(unsigned *max, unsigned *regular_0db, unsigned *min)
{
	*max         = MAX_INPUT_PGA_VOLUME;
	*regular_0db = ZERO_DB_INPUT_PGA_VOLUME;
	*min         = MIN_INPUT_PGA_VOLUME;
}

void itp_codec_rec_mute(void)
{
	return;
}

void itp_codec_rec_unmute(void)
{
	return;
}

void itp_codec_power_on(void)
{
#ifdef CFG_AUDIOAMP_ENABLE
        // for entrance AMP.
        {
            unsigned short data16;
            data16 = (1 << 1) | (0x7 << 4);
            //alc5628_write_reg( 48, data16);            
            return;
        }
#endif
    return;
}

void itp_codec_power_off(void)
{
#ifdef CFG_AUDIOAMP_ENABLE
            // for entrance AMP.
            {
                unsigned short data16;
                data16 = (1 << 1) | (0x6 << 4);
                //alc5628_write_reg(48, data16);                
                return;
            }
#endif
    return;
}

void itp_codec_get_i2s_sample_rate(int* samplerate)
{
	pthread_mutex_lock(&ALC5628_MUTEX);
	*samplerate = i2s_sample_rate;
	pthread_mutex_unlock(&ALC5628_MUTEX);
}

void itp_codec_set_i2s_sample_rate(int samplerate)
{
	pthread_mutex_lock(&ALC5628_MUTEX);
	i2s_sample_rate = samplerate;	
	pthread_mutex_unlock(&ALC5628_MUTEX);
}

void itp_codec_set_linein_enable(int yesno)
{
    _alc5628_linein = yesno;
    if(_alc5628_linein)        
    {
        unsigned short adj = curr_out1_volume;
        adj>>=1;
        adj = (adj << 8)|adj;            
	    alc5628_write_reg(0x0A, 0x0808);/* LINE_IN 0db*/
    }
    else
    {
        alc5628_write_reg(0x0A, 0xC8C8);/* LINE_IN Disable */
    }    
}

void itp_codec_set_i2sin_enable(int yesno)
{
    _alc5628_i2sin = yesno;
    if(_alc5628_i2sin)
    {
        unsigned short adj = curr_out1_volume;
        adj = (adj << 8)|adj;
        alc5628_write_reg(0x0C, adj); //DAC Digital Volume mute
        alc5628_writeRegMask(0x3A,(1<<15),(1<<15));
    }
    else
    {
        alc5628_write_reg(0x0C, 0xFFFF);
        alc5628_writeRegMask(0x3A,(0<<15),(0<<15));
    }            
    
}

int itp_codec_get_DA_running(void){
    return _alc5628_DA_running;
}

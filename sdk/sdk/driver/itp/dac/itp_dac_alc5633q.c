#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "ite/ith.h"
#include "ite/itp_codec.h"
#include "iic/mmp_iic.h"

#define ALC5633_I2CADR               (0x38 >> 1)

#if CFG_DAC_port_I2C0
#define IIC_PORT IIC_PORT_0
#else
#define IIC_PORT IIC_PORT_1
#endif   

/* ************************************************************************** */
/* wrapper */
static inline void i2s_delay_us(unsigned us) { ithDelay(us); }

/* ************************************************************************** */
#define MAX_OUT1_VOLUME            0x00 /* +0 dB */
#define DEFAULT_DB_OUT1_VOLUME     0x30 /*  -11.25 dB */
#define MIN_OUT1_VOLUME            0x69 /* -39.375 dB */
static unsigned curr_out1_volume = DEFAULT_DB_OUT1_VOLUME; /* -11.25 dB */

#define MAX_INPUT_PGA_VOLUME       0x00 /* +12   dB */
#define ZERO_DB_INPUT_PGA_VOLUME   0x08 /* +0    dB */
#define MIN_INPUT_PGA_VOLUME       0x1F /* -34.5 dB */

static unsigned curr_input_pga_volume = ZERO_DB_INPUT_PGA_VOLUME; /* 0 dB, refer to R0, R1 */

static int _alc5633_DA_running = 0;
static int _alc5633_AD_running = 0;
static pthread_mutex_t ALC5633_MUTEX = PTHREAD_MUTEX_INITIALIZER;

static int _alc5633_micin = 1;
static int _alc5633_linein = 1;
static int _alc5633_spkout = 1;
static int _alc5633_hpout = 1;

static int _alc5633_cold_start = 1; /* alc5633q should enable power only once */

/* ************************************************************************** */
/* gamma correction */
#include "gc7.2.inc"
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

    tmp = (d&0x00FF) << 8 | (d&0xFF00)  >> 8;
    
    ithWriteRegMaskA(ITH_GPIO_BASE | 0xD0, (0 << 28), (0x3 << 28)); /* IIC: internal HDMI IIC */
    for(retry=0; retry<50; retry++)
    {
        if(0 == (flag = mmpIicSendData(IIC_PORT, IIC_MASTER_MODE, ALC5633_I2CADR, RegAddr, &tmp, 2)))
        {
        	success = 1;
#ifdef ALC5633_DEBUG_I2C_COMMON_WRITE
        	printf("ALC5633_I2CADR# IIC WriteOK!\n");
#endif
            break;
        }
    }
    if(success == 0) {
    	printf("ALC5633# IIC Write Fail!\n");
    	while(1) { usleep(500000); }
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
        if(0 == (flag = mmpIicReceiveData(IIC_PORT, IIC_MASTER_MODE, ALC5633_I2CADR, &RegAddr, 1, &d, 2)))
        {
            success = 1;
#ifdef ALC5633_DEBUG_I2C_COMMON_WRITE
            printf("ALC5633_I2CADR# IIC Read OK!\n");
#endif
            break;
        }
    }
    if(success == 0) {
        printf("ALC5633# IIC Write Fail!\n");
        while(1) { usleep(500000); }
    }
    ithWriteRegMaskA(ITH_GPIO_BASE | 0xD0, (0 << 28), (0x3 << 28)); /* IIC: external APB IIC */
    tmp = (d&0x00FF) << 8 | (d&0xFF00)  >> 8;    
    return tmp ;
}

static void alc5633_write_reg(unsigned char reg_addr, unsigned short value)
{

#ifdef ALC5633_DEBUG_PROGRAM_REGVALUE
	printf("ALC5633# write reg[0x%02x] = 0x%04x\n", reg_addr, value);
#endif
	I2C_common_write_word(reg_addr, value);
}

static unsigned short alc5633_read_reg(unsigned char reg_addr)
{    
    return I2C_common_read_word(reg_addr);
}

static inline void alc5633_writeRegMask(uint16_t addr, uint16_t data, uint16_t mask)
{
    alc5633_write_reg(addr, (alc5633_read_reg(addr) & ~mask) | (data & mask));
}

static void Dump_Register(void)
{

	unsigned short tempRegisterValue;
	unsigned char addr; 

	for (addr=0; addr <= 0x7e; addr += 1)
	{
		tempRegisterValue = alc5633_read_reg(addr);
		printf("reg[0x%02x] = 0x%04x\n", addr, tempRegisterValue);
	}

}

static void init_alc5633_common(void)
{
    pthread_mutex_lock(&ALC5633_MUTEX);

	printf("ALC5633# %s\n", __func__);

	if(_alc5633_DA_running)
	{
		printf("ALC5633# DAC is running, skip re-init process !\n");
		pthread_mutex_unlock(&ALC5633_MUTEX);
		return;
	}
	if(_alc5633_AD_running)
	{
		printf("ALC5633# ADC is running, skip re-init process !\n");
		pthread_mutex_unlock(&ALC5633_MUTEX);
		return;
	}
	_alc5633_cold_start = 0;

	/* start programming alc5633 */
	{
        unsigned short data16;
     
        alc5633_write_reg(0x00, 0x0001); /* reset ACL5633 */
        alc5633_write_reg(0x3C, 0xA000); /* Enable Vref voltage, enable all analog Circuit bias */
        //alc5633_write_reg(0x56, 0xB000); /* HP De-pop ctrl */
        alc5633_write_reg(0x38, 0x0000); /* Stereo ADC/DAC Clock ctrl */

        //alc5633_write_reg(0x03, 0x0140); /* SP/HP Output Mixer ctrl */
       
        /* AD */
        alc5633_write_reg(0x4C, 0x0000); /* select as GPIO Pin */
        alc5633_write_reg(0x4D, 0x1800); /* select GPIO pin as Output */
        //alc5633_write_reg(0x18, 0x3E3E); /* HP Mixer Ctrl */
     
        /* Power Management Ctrl */
        alc5633_write_reg(0x3A, 0x9FC0); /* Power on I2S Digital Interface Enable Ctrl */

        /* DAC Ctrl */
        alc5633_write_reg(0x0C, 0x0030);/* DAC ctrl , 28.125db*/
        //alc5633_write_reg(0x66, 0x6000);/* ALC Enable */
        alc5633_write_reg(0x0E, 0x3030);/* DAC Digital Volume ctrl 0 db 0x3030*/
 
        if(_alc5633_micin)        
        {            
            alc5633_write_reg(0x10, 0x8808); /* MIC1 as Differential input , Mic1 Vol 12 db */        
            alc5633_write_reg(0x22, 0x2220);/* MIC1 boost control 20db*/
            alc5633_write_reg(0x14, 0x7D7D);/* MIC1 boost Gain to REC Left/Right Mixer */
            //micCurrVol = (unsigned int)((float)((float)7) / 0.75) + 23;
            alc5633_write_reg(0x16, 0x1E1E);/* ADC digital Volume -37.5db*/

            alc5633_write_reg(0x3B, 0x0C39); /* MIC1/MIC2 Boost Power Ctrl */

            /* restore previous rec volume */
            //data16 = (1 << 8) | (micCurrVol & 0x3F);            
            //alc5633_write_reg( 0, data16); /* left input PGA disable analogue mute, 0dB */
            //alc5633_write_reg( 1, data16); /* right input PGA disable analogue mute, 0dB */
        }
        else
        {
            alc5633_write_reg(0x14, 0x7F7F);/* MIC 1 & MIC2 boost Gain to REC L/R Mixer Mute*/
        }
        
        if(_alc5633_linein)        
        {
            alc5633_writeRegMask(0x14, (0<<10)|(0<<11)|(0<<2)|(0<<3), (1<<10)|(1<<11)|(1<<2)|(1<<3));       
        }
        else
        {
            alc5633_writeRegMask(0x14, (1<<10)|(1<<11)|(1<<2)|(1<<3), (1<<10)|(1<<11)|(1<<2)|(1<<3));
        }

        alc5633_write_reg(0x12, 0x0013);/* ADC pre-boost 8 db*/
        
        /* DA */
        alc5633_write_reg(0x34, 0x8000); /* audio interface: data word length: 16 bits, I2S format */     
        

#if 1 /* RealTek's new flow @ 2015-0211, 12:51 */        
        if(_alc5633_spkout)/* Enable SPK_L and SPK_R */       
            alc5633_write_reg(0x1C, 0x08FC);/* Enable DAC L/R to Speaker Mixer */
        else    
            alc5633_write_reg(0x1C, 0x00FF);/* Disable DAC L/R to Speaker Mixer*/

        usleep(50000);/* Delay (50 ms) to allow HP amps to settle */

        if(_alc5633_spkout)
        {
#ifndef CFG_AUDIOAMP_ENABLE
#if (CFG_CHIP_FAMILY == 9910)
            alc5633_write_reg(0x1E, 0x8000); /* Select speaker Amp Class D mode*/
#else
            alc5633_write_reg(0x1E, 0x8000); /* Select speaker Amp Class D mode*/
#endif
            alc5633_write_reg(0x02, 0x4063);/* Select Spkon Source, LN */
            alc5633_writeRegMask(0x3E, (1<<15)| (1<<14), (1<<15)| (1<<14));/* Left Speaker Volume Power on */
            alc5633_writeRegMask(0x3A, (1<<13), (1<<13));/* Power on class D */

#endif
        }
        else
        {
            alc5633_writeRegMask(0x02, (1<<15), (1<<15));
            alc5633_writeRegMask(0x3E, (0<<15)| (0<<14), (1<<15)| (1<<14));/* Left Speaker Volume Power Off */
            alc5633_writeRegMask(0x3A, (0<<13), (1<<13));/* Power Off class D */
        }
    } 
#endif
    pthread_mutex_unlock(&ALC5633_MUTEX);
}

static void deinit_alc5633_common(void)
{
	pthread_mutex_lock(&ALC5633_MUTEX);
	printf("ALC5633# %s\n", __func__);

	if(_alc5633_DA_running)
	{
		printf("ALC5633# DAC is running, skip deinit !\n");
		pthread_mutex_unlock(&ALC5633_MUTEX);
		return;
	}
	if(_alc5633_AD_running)
	{
		printf("ALC5633# ADC is running, skip deinit !\n");
		pthread_mutex_unlock(&ALC5633_MUTEX);
		return;
	}
	
	/* digital mute flow */
	{
		alc5633_writeRegMask(0x1C, (1<<1)|(1<<0), (1<<1)|(1<<0));
	}
//	i2s_delay_us(25000); /* FIXME: dummy loop */

	/* DON'T turn off power in any case */
	if     (curr_out1_volume < MAX_OUT1_VOLUME) { curr_out1_volume = MAX_OUT1_VOLUME; }
	else if(curr_out1_volume > MIN_OUT1_VOLUME) { curr_out1_volume = MIN_OUT1_VOLUME; }

	/* just decrease output volume */
	{
		unsigned short data16;
		unsigned short tmp, adj = curr_out1_volume;
		if(adj != MIN_OUT1_VOLUME) {
			do
			{
				adj++;
				tmp = (adj << 8)|adj;
                alc5633_write_reg(0x0E, tmp);                				
				i2s_delay_us(1000); /* FIXME: dummy loop */
			} while(adj != MIN_OUT1_VOLUME);
		}
	}

	pthread_mutex_unlock(&ALC5633_MUTEX);
	return;
}

void itp_codec_standby(void)
{
	pthread_mutex_lock(&ALC5633_MUTEX);
	printf("ALC5633# %s\n", __func__);
	
	_alc5633_cold_start = 1;
	pthread_mutex_unlock(&ALC5633_MUTEX);
}

/* DAC */
void itp_codec_playback_init(unsigned output)
{
	printf("ALC5633# %s\n", __func__);

	switch(output)
	{
		case 1: /* Speaker only */
		{
			_alc5633_spkout  = 1;
			_alc5633_hpout = 0;
			break;
		}
		case 2: /* both Speaker & HeadPhone */
		{
			_alc5633_spkout  = 1;
			_alc5633_hpout = 1;
			break;
		}
		case 0: /* HeadPhone only */
		default:
		{
			_alc5633_spkout  = 0;
			_alc5633_hpout = 1;
			break;
		}
	}
	
	init_alc5633_common();
	_alc5633_DA_running = 1;
}

void itp_codec_playback_deinit(void)
{
	printf("ALC5633# %s\n", __func__);

	_alc5633_DA_running = 0; /* put before deinit_alc5633_common() */
	deinit_alc5633_common();
}

void itp_codec_playback_amp_volume_down(void)
{
	if(curr_out1_volume >= MIN_OUT1_VOLUME) return;

	switch(curr_out1_volume)
	{
		case 0x00 ... 0x07: { itp_codec_playback_set_direct_vol(0x08); break; } /* -3 dB */
		case 0x08 ... 0x0F: { itp_codec_playback_set_direct_vol(0x10); break; } /*  -6 dB */
		case 0x10 ... 0x17: { itp_codec_playback_set_direct_vol(0x18); break; } /* -9 dB */
		case 0x18 ... 0x1F: { itp_codec_playback_set_direct_vol(0x20); break; } /* -12 dB */
		case 0x20 ... 0x3F: { itp_codec_playback_set_direct_vol(0x40); break; } /* -24dB */
		case 0x40 ... 0x4F: { itp_codec_playback_set_direct_vol(0x50); break; } /* -30dB */
		case 0x50 ... 0x67: { itp_codec_playback_set_direct_vol(0x68); break; } /* -39dB */
	}
}

void itp_codec_playback_amp_volume_up(void)
{
	if(curr_out1_volume <= MAX_OUT1_VOLUME) return;

	switch(curr_out1_volume)
	{
		case 0x01 ... 0x08: { itp_codec_playback_set_direct_vol(0x00); break; } /* +0 dB */
		case 0x09 ... 0x10: { itp_codec_playback_set_direct_vol(0x08); break; } /* -3 dB */
		case 0x11 ... 0x18: { itp_codec_playback_set_direct_vol(0x10); break; } /*  -6 dB */
		case 0x19 ... 0x20: { itp_codec_playback_set_direct_vol(0x18); break; } /* -9 dB */
		case 0x21 ... 0x40: { itp_codec_playback_set_direct_vol(0x20); break; } /* -12 dB */
		case 0x41 ... 0x50: { itp_codec_playback_set_direct_vol(0x40); break; } /* -24dB */
		case 0x51 ... 0x69: { itp_codec_playback_set_direct_vol(0x50); break; } /* -30dB */
	}
}

void itp_codec_playback_set_direct_vol(unsigned target_vol)
{
	int direction;
	unsigned short data16;
	unsigned short adj, tmp = 0x00;

	pthread_mutex_lock(&ALC5633_MUTEX);

	
	if(target_vol < MAX_OUT1_VOLUME)
	{
		 printf("ERROR# invalid target volume step: 0x%08x\n", target_vol);
		 pthread_mutex_unlock(&ALC5633_MUTEX);
		 return;
	}

	if     (target_vol == curr_out1_volume) { pthread_mutex_unlock(&ALC5633_MUTEX); return; }
	else if(target_vol <  curr_out1_volume) { direction = 1; } /* - */
	else                                    { direction = 0; } /* + */
	
	while(curr_out1_volume != target_vol)
	{
		if(direction == 1) { curr_out1_volume--; }
		else               { curr_out1_volume++; }

		adj = curr_out1_volume;		

		if(_alc5633_DA_running)
		{
		    tmp = (adj<<8) | adj;
            alc5633_write_reg(0x0E, tmp);
		}

		i2s_delay_us(1000); /* FIXME: dummy loop */;
	}
	pthread_mutex_unlock(&ALC5633_MUTEX);
}

void itp_codec_playback_set_direct_volperc(unsigned target_volperc)
{
	unsigned char volstep;

	if(alc5633_perc_to_reg_tableSize != 100) {
		printf("ERROR# invalid a6c5633_perc_to_reg_tableSize !\n");
		return;
	}
	if(target_volperc >= 100) { target_volperc = 99; }

	volstep = alc5633_perc_to_reg_table[target_volperc];
	itp_codec_playback_set_direct_vol(volstep);
}

void itp_codec_playback_get_currvol(unsigned *currvol)
{
	*currvol = curr_out1_volume;
}

void itp_codec_playback_get_currvolperc(unsigned *currvolperc)
{
	unsigned i;

	for(i=99; i>0; i--)
	{
		if((alc5633_perc_to_reg_table[i] >= curr_out1_volume)
		&& (curr_out1_volume > alc5633_perc_to_reg_table[i+1])) {
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
	pthread_mutex_lock(&ALC5633_MUTEX);

    printf("%s (%d)\n", __FUNCTION__, __LINE__);
    		

	//i2s_delay_us(25000); /* FIXME: dummy loop */
    	
    /* DON'T turn off power in any case */
    //if     (curr_out1_volume > MAX_OUT1_VOLUME) { curr_out1_volume = MAX_OUT1_VOLUME; }
    //else if(curr_out1_volume < MIN_OUT1_VOLUME) { curr_out1_volume = MIN_OUT1_VOLUME; }

    /* just decrease output volume */
    {       
        if(_alc5633_hpout)
        {
            alc5633_writeRegMask(0x04, (1<<7)|(1<<15), (1<<7)|(1<<15)); /* Mute HPOL/HPOR */
            alc5633_writeRegMask(0x18, (1<<0)|(1<<8), (1<<0)|(1<<8)); /*  Mute DAC HPMIXL/HPMIXR */
        }
        
        if(_alc5633_spkout)
        {
            alc5633_writeRegMask(0x02, (1<<15), (1<<15)); /* Mute SPO_P/SPO_N */
            alc5633_writeRegMask(0x1C, (1<<0)|(1<<1), (1<<1)|(1<<1)); /* Mute DAC SPKMIX_R/SPK_MIX_L */
        }
        i2s_delay_us(1000); /* FIXME: dummy loop */

    }
	pthread_mutex_unlock(&ALC5633_MUTEX);
}

void itp_codec_playback_unmute(void)
{

	pthread_mutex_lock(&ALC5633_MUTEX);
	
    unsigned short data16;
    unsigned short adj, tmp = 0x00;

    printf("%s (%d)\n", __FUNCTION__, __LINE__);

	if     (curr_out1_volume < MAX_OUT1_VOLUME) { curr_out1_volume = MAX_OUT1_VOLUME; }
	else if(curr_out1_volume > MIN_OUT1_VOLUME) { curr_out1_volume = MIN_OUT1_VOLUME; }

    adj = curr_out1_volume;

	i2s_delay_us(1000); /* FIXME: dummy loop */
	    
    tmp = (adj << 8) | adj;
    alc5633_write_reg(0x0E, tmp);
    	
	if(_alc5633_hpout)
	{
        alc5633_writeRegMask(0x04, (0<<7)|(0<<15), (1<<7)|(1<<15));
        alc5633_writeRegMask(0x18, (0<<0)|(0<<8), (1<<0)|(1<<8));
	}
	
	if(_alc5633_spkout)
	{				
		alc5633_writeRegMask(0x02, (0<<15), (1<<15));
		alc5633_writeRegMask(0x1C, (0<<0)|(0<<1), (1<<0)|(1<<1));
	}
	
	i2s_delay_us(1000); /* FIXME: dummy loop */

	pthread_mutex_unlock(&ALC5633_MUTEX);	
}

/* line-in bypass to line-out directly */
void itp_codec_playback_linein_bypass(unsigned bypass)
{
	uint16_t	reg16;
	uint16_t	BpVol=7;	
	
//	pthread_mutex_lock(&ALC5633_MUTEX);


//	pthread_mutex_unlock(&ALC5633_MUTEX);
}


/* ADC */
void itp_codec_rec_init(unsigned input_source)
{
	printf("ALC5633# %s\n", __func__);

	switch(input_source)
	{
		case 1: /* LineIN only */
		{
			_alc5633_micin  = 0;
			_alc5633_linein = 1;
			break;
		}
		case 2: /* MICIN only */
		{
			_alc5633_micin  = 1;
			_alc5633_linein = 0;
			break;
		}
		case 0: /* both LineIN & MICIN */
		default:
		{
			_alc5633_micin  = 1;
			_alc5633_linein = 1;
			break;
		}
	}

	init_alc5633_common();
	_alc5633_AD_running = 1;
}

void itp_codec_rec_deinit(void)
{
	printf("ALC5633# %s\n", __func__);

	_alc5633_AD_running = 0; /* put before deinit_alc5633_common() */
	deinit_alc5633_common();
}

void itp_codec_rec_set_direct_vol(unsigned target_vol)
{
	int direction;
	unsigned short data16;

	pthread_mutex_lock(&ALC5633_MUTEX);

    target_vol = MIN_INPUT_PGA_VOLUME - MAX_INPUT_PGA_VOLUME - target_vol;

	if(target_vol < MAX_INPUT_PGA_VOLUME)
	{
		 printf("ERROR# invalid target volume step: 0x%08x\n", target_vol);
		 pthread_mutex_unlock(&ALC5633_MUTEX);
		 return;
	}

	if     (target_vol == curr_input_pga_volume) { pthread_mutex_unlock(&ALC5633_MUTEX); return; }
	else if(target_vol <  curr_input_pga_volume) { direction = 1; } /* - */
	else                                         { direction = 0; } /* + */
	
	while(curr_input_pga_volume != target_vol)
	{
		if(direction == 1) { curr_input_pga_volume--; }
		else               { curr_input_pga_volume++; }

		
		if(_alc5633_AD_running)
		{		   
		   alc5633_writeRegMask(0x10, curr_input_pga_volume << 8, 0x1F << 8);
		}

		i2s_delay_us(1000); /* FIXME: dummy loop */;
	}
	pthread_mutex_unlock(&ALC5633_MUTEX);
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
	unsigned short data16;
	unsigned inpvol = curr_input_pga_volume;

	pthread_mutex_lock(&ALC5633_MUTEX);

	while(MIN_INPUT_PGA_VOLUME != inpvol)
	{
		inpvol++;

		if(_alc5633_AD_running)
		{
			alc5633_writeRegMask(0x10, inpvol << 8, 0x1F << 8);
		}

		i2s_delay_us(1000); /* FIXME: dummy loop */;
	}

	if(_alc5633_AD_running)
	{
        alc5633_writeRegMask(0x14, (1<<8)|(1<<9), (1<<8)|(1<<9));
	}
	
	pthread_mutex_unlock(&ALC5633_MUTEX);
}

void itp_codec_rec_unmute(void)
{
	unsigned short data16;
	unsigned inpvol = MIN_INPUT_PGA_VOLUME;
	
	pthread_mutex_lock(&ALC5633_MUTEX);
	
	while(inpvol != curr_input_pga_volume)
	{
		inpvol--;

		if(_alc5633_AD_running)
		{
            alc5633_writeRegMask(0x10, inpvol << 8, 0x1F << 8);
		}

		i2s_delay_us(1000); /* FIXME: dummy loop */;
	}
    alc5633_writeRegMask(0x14, (0<<8)|(0<<9), (1<<8)|(1<<9));
	pthread_mutex_unlock(&ALC5633_MUTEX);
}

void itp_codec_power_on(void)
{
#ifdef CFG_AUDIOAMP_ENABLE
        // for entrance AMP.
        {
            unsigned short data16;
            data16 = (1 << 1) | (0x7 << 4);
            //alc5633_write_reg( 48, data16);            
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
                //alc5633_write_reg(48, data16);                
                return;
            }
#endif
    return;
}

void itp_codec_get_i2s_sample_rate(int *samplerate)
{
}

void itp_codec_set_i2s_sample_rate(int samplerate)
{
}


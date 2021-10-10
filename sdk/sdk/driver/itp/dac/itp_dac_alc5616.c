#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "ite/ith.h"
#include "ite/itp_codec.h"
#include "iic/mmp_iic.h"

#define ALC5616_I2CADR               (0x36 >> 1)

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
#define MAX_OUT1_VOLUME            0xAF /* +0 dB */
#define DEFAULT_DB_OUT1_VOLUME     0x59 /* -32.25 dB */
#define MIN_OUT1_VOLUME            0x00 /* -65.625 dB */
static unsigned curr_out1_volume = DEFAULT_DB_OUT1_VOLUME; /* -11.25 dB */

#define MAX_INPUT_PGA_VOLUME       0x7F /* +30   dB */
#define ZERO_DB_INPUT_PGA_VOLUME   0x2F /* +0    dB */
#define MIN_INPUT_PGA_VOLUME       0x00 /* -17.625 dB */

static unsigned curr_input_pga_volume = ZERO_DB_INPUT_PGA_VOLUME; /* 0 dB, refer to R0, R1 */

static int _alc5616_DA_running = 0;
static int _alc5616_AD_running = 0;
static pthread_mutex_t ALC5616_MUTEX = PTHREAD_MUTEX_INITIALIZER;

static int _alc5616_micin   = 1;
static int _alc5616_linein  = 0;
static int _alc5616_lineout = 0;
static int _alc5616_spkout  = 1;
static int _alc5616_hpout   = 1;
static int _alc5616_channel = 2; //note set 2 can get louder 

static int _alc5616_cold_start = 1; /* alc5616q should enable power only once */
static int _alc5616_mute = 0;

static int i2s_sample_rate = 48000;

/* ************************************************************************** */
/* gamma correction */
#include "gc10.2.inc"
//#include "gc8.2.inc"

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
        if(0 == (flag = mmpIicSendData(IIC_PORT, IIC_MASTER_MODE, ALC5616_I2CADR, RegAddr, &tmp, 2)))
        {
        	success = 1;
#ifdef ALC5616_DEBUG_I2C_COMMON_WRITE
        	printf("ALC5616_I2CADR# IIC WriteOK!\n");
#endif
            break;
        }
    }
    if(success == 0) {
    	printf("ALC5616# IIC Write Fail!\n");
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
        if(0 == (flag = mmpIicReceiveData(IIC_PORT, IIC_MASTER_MODE, ALC5616_I2CADR, &RegAddr, 1, &d, 2)))
        {
            success = 1;
#ifdef ALC5616_DEBUG_I2C_COMMON_WRITE
            printf("ALC5616_I2CADR# IIC Read OK!\n");
#endif
            break;
        }
    }
    if(success == 0) {
        printf("ALC5616# IIC Write Fail!\n");
        while(1) { usleep(500000); }
    }
    ithWriteRegMaskA(ITH_GPIO_BASE | 0xD0, (0 << 28), (0x3 << 28)); /* IIC: external APB IIC */

	if (CFG_CHIP_FAMILY == 9910)
		tmp = d;
	else
    	tmp = (d&0x00FF) << 8 | (d&0xFF00)  >> 8;
    
    return tmp ;
}

static void alc5616_write_reg(unsigned char reg_addr, unsigned short value)
{

#ifdef ALC5616_DEBUG_PROGRAM_REGVALUE
	printf("ALC5616# write reg[0x%02x] = 0x%04x\n", reg_addr, value);
#endif
	I2C_common_write_word(reg_addr, value);
}

static unsigned short alc5616_read_reg(unsigned char reg_addr)
{    
    return I2C_common_read_word(reg_addr);
}

static inline void alc5616_writeRegMask(uint16_t addr, uint16_t data, uint16_t mask)
{
    alc5616_write_reg(addr, (alc5616_read_reg(addr) & ~mask) | (data & mask));
}

static void Dump_Register(void)
{

	unsigned short tempRegisterValue;
	unsigned char addr; 

	for (addr=0; addr <= 0xFF; addr += 1)
	{
		tempRegisterValue = alc5616_read_reg(addr);
		printf("reg[0x%02x] = 0x%04x\n", addr, tempRegisterValue);
		if (addr == 0xFF)
		{
		    tempRegisterValue = alc5616_read_reg(addr);
		    printf("reg[0x%02x] = 0x%04x\n", addr, tempRegisterValue);
		    break;
		}    
	}

}

static void init_alc5616_common(void)
{
    pthread_mutex_lock(&ALC5616_MUTEX);

	DEBUG_PRINT("ALC5616# %s\n", __func__);

	if(_alc5616_DA_running)
	{
		DEBUG_PRINT("ALC5616# DAC is running, skip re-init process !\n");
		pthread_mutex_unlock(&ALC5616_MUTEX);
		return;
	}
	if(_alc5616_AD_running)
	{
		DEBUG_PRINT("ALC5616# ADC is running, skip re-init process !\n");
		pthread_mutex_unlock(&ALC5616_MUTEX);
		return;
	}
	_alc5616_cold_start = 0;

#if 1
	/* start programming ALC5616 */
	{
        unsigned short data16;
        
        alc5616_write_reg(0x00, 0x0001); /* reset ACL5616 */
        /* Power Management Ctrl */
        if(_alc5616_channel == 1)
            alc5616_write_reg(0x61, 0x9006); /* Power on I2S Digital Interface Enable Ctrl */
        else//channel : 2
            alc5616_write_reg(0x61, 0x9806);
        alc5616_write_reg(0x62, 0x8800); /* AD/DA Digital Filter Power */
        alc5616_write_reg(0x63, 0xB8F0); /* Enable Vref voltage, enable all analog Circuit bias */

        alc5616_write_reg(0x3B, 0x0000); /* RECMIXL ctr1 0db*/
        alc5616_write_reg(0x3D, 0x0000); /* RECMIXR ctr1 0db*/
        
        alc5616_write_reg(0x73, 0x0100); /* Stereo ADC/DAC Clock ctrl */
        alc5616_write_reg(0x03, 0x2727); /* SP/HP Output Mixer ctrl & Gain -46.5dB:0x2727 0dB:0x0808 +12dB:0x0000*/
        alc5616_write_reg(0x05, 0x8000); /* Enable Differential Line Output */
       
        /* AD */
        alc5616_write_reg(0xC0, 0x0100); /* select as GPIO Pin */
        alc5616_write_reg(0xC1, 0x0006); /* select GPIO pin as Output and pull High */
        
        /* DAC Ctrl */    
        {
            unsigned short adj = curr_out1_volume;
            adj = (adj << 8)|adj;
            alc5616_write_reg(0x19, adj);/* DAC Digital Volume ctrl -11.25 dB */
        }
        alc5616_write_reg(0x65, 0xCC00); /* OUTMIXR/L RECMIXR/L power on */
        alc5616_write_reg(0x8E, 0x001D); /* HP Amp Control */
 
        if(_alc5616_micin)        
        {   
            /*dual mic as input*/
            //alc5616_write_reg(0x0D, 0x2200);/*MIC1/MIC2 as single end input , Mic1/2 Boost 24db*/
            //alc5616_write_reg(0x3E, 0x0FFB);/* BST2 to RECMIXR */
            //alc5616_write_reg(0x3C, 0x0FFD);/* BST1 to RECMIXL */           
            //alc5616_write_reg(0x64, 0xC800); /* MIC BST1 BST2 Boost Power Ctrl */    
        	/* MIC2 as input */
            //alc5616_write_reg(0x0D, 0x02C0); /* MIC2 as Differential input , Mic2 Boost 24 dB */          
            //alc5616_write_reg(0x3E, 0x1FFB);/* BST2 to RECMIXR */
            //alc5616_write_reg(0x3C, 0x1FFB);/* BST2 to RECMIXL */
            //alc5616_write_reg(0x64, 0x4800); /* MIC BST2 Boost Power Ctrl */
            /* MIC1 as input */			
            alc5616_write_reg(0x0D, 0x2040);/*MIC1/MIC2 as Differential input , Mic1/2 Boost 24db*/
            alc5616_write_reg(0x3E, 0x1FFD);/* BST1 to RECMIXR */
            alc5616_write_reg(0x3C, 0x1FFD);/* BST1 to RECMIXL */           
            alc5616_write_reg(0x64, 0x8800); /* MIC BST1 Boost Power Ctrl */

            alc5616_write_reg(0x1C, 0x722F);/* ADC digital Volume 25.125dB */
            alc5616_writeRegMask(0x1C, curr_input_pga_volume << 8, 0x7F << 8);
            alc5616_write_reg(0x27, 0x3800); 
        }
        else
        {
            alc5616_write_reg(0x3E, 0x1FFF);/* BST2 to RECMIXR */
            alc5616_write_reg(0x3C, 0x1FFF);/* BST2 to RECMIXL */
        }
        
        if(_alc5616_linein){
            alc5616_writeRegMask(0x3C, (0<<5),(1<<5));//input L line-in to ADC
            alc5616_writeRegMask(0x3E, (0<<5),(1<<5));//input R line-in to ADC
        }
        
        //if(_alc5616_linein)        
        //{
        //    alc5616_writeRegMask(0x14, (0<<10)|(0<<11)|(0<<2)|(0<<3), (1<<10)|(1<<11)|(1<<2)|(1<<3));       
        //}
        //else
        //{
        //    alc5616_writeRegMask(0x14, (1<<10)|(1<<11)|(1<<2)|(1<<3), (1<<10)|(1<<11)|(1<<2)|(1<<3));
        //}

        //alc5616_write_reg(0x12, 0x0013);/* ADC pre-boost 8 db*/
        
        /* DA */
        alc5616_write_reg(0x70, 0x8000); /* audio interface: data word length: 16 bits, I2S format */     
        alc5616_write_reg(0x2A, 0x0202); /* DAC Digital Mixer Control */
        alc5616_write_reg(0x02, 0x2727); /* unmute HPOL/HPOVOLL & Gain -46.5dB:0x2727 0dB:0x0808 +12dB:0x0000*/
        alc5616_write_reg(0x45, 0x0000); /* unmute HPOLMIX*/
        alc5616_write_reg(0x29, 0x8080); /* stero ADC to DAC digital Mixer ctrl*/


        alc5616_write_reg(0xFA, 0x0011); /*General ctrl*/
        alc5616_write_reg(0x8F, 0x3100); /*Hp Amp ctrl2*/
        alc5616_write_reg(0x6A, 0x003D); /*private rigister index*/
        alc5616_write_reg(0x6C, 0x3E00); /*private register data*/
        alc5616_write_reg(0x51, 0x0100); /*OUTMIXL Gain :-6db*/
        alc5616_write_reg(0x4E, 0x0100); /*OUTMIXR Gain :-6db*/

#if 1 /* RealTek's new flow @ 2015-0211, 12:51 */        
        if(_alc5616_spkout)/* Enable SPK_L and SPK_R */
        {        	
            alc5616_write_reg(0x4F, 0x0278);/* Enable DAC R to Speaker Mixer */
            alc5616_write_reg(0x52, 0x0278);/* Enable DAC L to Speaker Mixer */
        }
        else
        {
            alc5616_write_reg(0x4F, 0x0279);/* Disable DAC L/R to Speaker Mixer*/
            alc5616_write_reg(0x52, 0x0279);/* Disable DAC L/R to Speaker Mixer*/
        }
        
        if(_alc5616_lineout){
            alc5616_writeRegMask(0x4F, (0<<4),(1<<4));//L line-out to outmixer
            alc5616_writeRegMask(0x52, (0<<4),(1<<4));//R line-out to outmixer        
        }       
        
        usleep(10000);/* Delay (10 ms) to allow HP amps to settle */

        if(_alc5616_spkout)
        {
            alc5616_write_reg(0x53, 0x0000);/* Select OUTVOLL/R to LOUTMIX */
            alc5616_write_reg(0x66, 0x3C00);/* OUTVOLL/R power on */            
        }
        else
        {
            alc5616_write_reg(0x53, 0xF000);/* Mute OUTVOLL/R to LOUTMIX */
            alc5616_write_reg(0x66, 0x0000);/* OUTVOLL/R power down */
            
        }
    }    
#endif
#endif
    pthread_mutex_unlock(&ALC5616_MUTEX);
}

static void deinit_alc5616_common(void)
{
	pthread_mutex_lock(&ALC5616_MUTEX);
	DEBUG_PRINT("ALC5616# %s\n", __func__);

	if(_alc5616_DA_running)
	{
		DEBUG_PRINT("ALC5616# DAC is running, skip deinit !\n");
		pthread_mutex_unlock(&ALC5616_MUTEX);
		return;
	}
	if(_alc5616_AD_running)
	{
		DEBUG_PRINT("ALC5616# ADC is running, skip deinit !\n");
		pthread_mutex_unlock(&ALC5616_MUTEX);
		return;
	}
	
//	i2s_delay_us(25000); /* FIXME: dummy loop */

	/* DON'T turn off power in any case */
	if     (curr_out1_volume > MAX_OUT1_VOLUME) { curr_out1_volume = MAX_OUT1_VOLUME; }
	else if(curr_out1_volume < MIN_OUT1_VOLUME) { curr_out1_volume = MIN_OUT1_VOLUME; }

	/* just decrease output volume */
	{
		unsigned short data16;
		unsigned short tmp, adj = curr_out1_volume;
		if(adj != MIN_OUT1_VOLUME) {
			do
			{
				adj--;
				tmp = (adj << 8)|adj;
                alc5616_write_reg(0x19, tmp);                				
				i2s_delay_us(1000); /* FIXME: dummy loop */
			} while(adj != MIN_OUT1_VOLUME);
		}
	}

	pthread_mutex_unlock(&ALC5616_MUTEX);
	return;
}
void itp_codec_wake_up(void)
{

	pthread_mutex_lock(&ALC5616_MUTEX);
	printf("ALC5616# %s\n", __func__);
	
	init_alc5616_common();
	_alc5616_DA_running = 1;
#if (CFG_DOORBELL_INDOOR)||(CFG_DOORBELL_ADMIN)||(CFG_DOORBELL_LOBBY)
	init_alc5616_common();
	_alc5616_AD_running = 1;
#endif
    if(_alc5616_mute)
        itp_codec_playback_mute();
	pthread_mutex_unlock(&ALC5616_MUTEX);
}
void itp_codec_standby(void)
{
	pthread_mutex_lock(&ALC5616_MUTEX);
	printf("ALC5616# %s\n", __func__);
#if (CFG_DOORBELL_INDOOR)||(CFG_DOORBELL_ADMIN)||(CFG_DOORBELL_LOBBY)
	itp_codec_rec_deinit();
#endif
	itp_codec_playback_deinit();	
	alc5616_write_reg(0x0, 0x0);	
	pthread_mutex_unlock(&ALC5616_MUTEX);
}

/* DAC */
void itp_codec_playback_init(unsigned output)
{
	DEBUG_PRINT("ALC5616# %s\n", __func__);

	switch(output)
	{
		case 1: /* Speaker only */
		{
			_alc5616_spkout  = 1;
			_alc5616_hpout = 0;
			break;
		}
		case 2: /* both Speaker & HeadPhone */
		{
			_alc5616_spkout  = 1;
			_alc5616_hpout = 1;
			break;
		}
		case 0: /* HeadPhone only */
		default:
		{
			_alc5616_spkout  = 0;
			_alc5616_hpout = 1;
			break;
		}
	}
	
	init_alc5616_common();
	_alc5616_DA_running = 1;
}

void itp_codec_playback_deinit(void)
{
	DEBUG_PRINT("ALC5616# %s\n", __func__);

	_alc5616_DA_running = 0; /* put before deinit_alc5616_common() */
    if(_alc5616_spkout){
        alc5616_write_reg(0x53, 0xF000);/* Mute OUTVOLL/R to LOUTMIX */
        alc5616_write_reg(0x66, 0x0000);/* OUTVOLL/R power down */
        alc5616_write_reg(0x4F, 0x0279);/* Disable DAC L/R to Speaker Mixer*/
        alc5616_write_reg(0x52, 0x0279);/* Disable DAC L/R to Speaker Mixer*/ 
    }
	deinit_alc5616_common();
}

void itp_codec_playback_amp_volume_down(void)
{
	if(curr_out1_volume <= MIN_OUT1_VOLUME) return;

	switch(curr_out1_volume)
	{
		case 0xA8 ... 0xAF: { itp_codec_playback_set_direct_vol(0xA7); break; } /* -3 dB */
		case 0xA0 ... 0xA7: { itp_codec_playback_set_direct_vol(0x9F); break; } /*  -6 dB */
		case 0x98 ... 0x9F: { itp_codec_playback_set_direct_vol(0x97); break; } /* -9 dB */
		case 0x90 ... 0x97: { itp_codec_playback_set_direct_vol(0x8F); break; } /* -12 dB */
		case 0x70 ... 0x8F: { itp_codec_playback_set_direct_vol(0x6F); break; } /* -24dB */
		case 0x60 ... 0x6F: { itp_codec_playback_set_direct_vol(0x5F); break; } /* -30dB */
		case 0x50 ... 0x5F: { itp_codec_playback_set_direct_vol(0x68); break; } /* -39dB */
	}
}

void itp_codec_playback_amp_volume_up(void)
{
	if(curr_out1_volume >= MAX_OUT1_VOLUME) return;

	switch(curr_out1_volume)
	{
		case 0xA7 ... 0xAE: { itp_codec_playback_set_direct_vol(0xAF); break; } /* +0 dB */
		case 0x9F ... 0xA6: { itp_codec_playback_set_direct_vol(0xA7); break; } /* -3 dB */
		case 0x97 ... 0x9E: { itp_codec_playback_set_direct_vol(0x9F); break; } /*  -6 dB */
		case 0x8F ... 0x96: { itp_codec_playback_set_direct_vol(0x97); break; } /* -9 dB */
		case 0x6F ... 0x8E: { itp_codec_playback_set_direct_vol(0x8F); break; } /* -12 dB */
		case 0x5F ... 0x6E: { itp_codec_playback_set_direct_vol(0x6F); break; } /* -24dB */
		case 0x57 ... 0x5E: { itp_codec_playback_set_direct_vol(0x5F); break; } /* -30dB */
	}
}

void itp_codec_playback_set_direct_vol(unsigned target_vol)
{
	//int direction;
	//unsigned short data16;
	unsigned short adj, tmp = 0x00;

	pthread_mutex_lock(&ALC5616_MUTEX);

	
	if(target_vol > MAX_OUT1_VOLUME)
	{
		 printf("ERROR# invalid target volume step: 0x%08x\n", target_vol);
		 pthread_mutex_unlock(&ALC5616_MUTEX);
		 return;
	}

	if     (target_vol == curr_out1_volume) { pthread_mutex_unlock(&ALC5616_MUTEX); return; }
	//else if(target_vol >  curr_out1_volume) { direction = 1; } /* + */
	//else                                    { direction = 0; } /* - */
	
	while(curr_out1_volume != target_vol)
	{
        curr_out1_volume = target_vol;
		//if(direction == 1) { curr_out1_volume++; }
		//else               { curr_out1_volume--; }

		adj = curr_out1_volume;		

		if(_alc5616_DA_running)
		{
		    tmp = (adj<<8) | adj;
            alc5616_write_reg(0x19, tmp);
		}

		//i2s_delay_us(1000); /* FIXME: dummy loop */;
	}
	pthread_mutex_unlock(&ALC5616_MUTEX);
}

void itp_codec_playback_set_direct_volperc(unsigned target_volperc)
{
	unsigned char volstep;

	if(alc5616_perc_to_reg_tableSize != 100) {
		printf("ERROR# invalid alc5616_perc_to_reg_tableSize !\n");
		return;
	}
	if(target_volperc >= 100) { target_volperc = 99; }

	volstep = alc5616_perc_to_reg_table[target_volperc];
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
		if((alc5616_perc_to_reg_table[i] >= curr_out1_volume)
		&& (curr_out1_volume > alc5616_perc_to_reg_table[i+1])) {
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
	
	pthread_mutex_lock(&ALC5616_MUTEX);

    DEBUG_PRINT("%s (%d)\n", __FUNCTION__, __LINE__);
    		

	//i2s_delay_us(25000); /* FIXME: dummy loop */
    	
    /* DON'T turn off power in any case */
    //if     (curr_out1_volume > MAX_OUT1_VOLUME) { curr_out1_volume = MAX_OUT1_VOLUME; }
    //else if(curr_out1_volume < MIN_OUT1_VOLUME) { curr_out1_volume = MIN_OUT1_VOLUME; }

    /* just decrease output volume */
	alc5616_write_reg(0x2A, 0x6363);/*Stereo DAC digital mixer control ,note:if comment, farend have no voice when communicating press mute */
	alc5616_writeRegMask(0x65,(0<<14)|(0<<15),(1<<14)|(1<<15)); /*power down OUTMIXL/R note:prevent switch pop sound*/       
        // if(_alc5616_hpout)
        // {
            // alc5616_writeRegMask(0x02, (1<<7)|(1<<15), (1<<7)|(1<<15)); /* Mute HPOL/HPOR */
		// }       
        // if(_alc5616_spkout)
        // {
            // alc5616_writeRegMask(0x03, (1<<7)|(1<<15), (1<<7)|(1<<15)); /* Mute SPO_P/SPO_N */	
		// }
    // i2s_delay_us(1000); /* FIXME: dummy loop */
    _alc5616_mute = 1;
	pthread_mutex_unlock(&ALC5616_MUTEX);
}

void itp_codec_playback_unmute(void)
{

    if(curr_out1_volume == MIN_OUT1_VOLUME) 
    {// vol = min (mute volume) 
        itp_codec_playback_mute();
        return;
    }
	pthread_mutex_lock(&ALC5616_MUTEX);
	
    //unsigned short data16;
    //unsigned short adj, tmp = 0x00;

    DEBUG_PRINT("%s (%d)\n", __FUNCTION__, __LINE__);

	//if     (curr_out1_volume > MAX_OUT1_VOLUME) { curr_out1_volume = MAX_OUT1_VOLUME; }
	//else if(curr_out1_volume < MIN_OUT1_VOLUME) { curr_out1_volume = MIN_OUT1_VOLUME; }

    //adj = curr_out1_volume;

	//i2s_delay_us(1000); /* FIXME: dummy loop */
	    
    //tmp = (adj << 8) | adj;
    //alc5616_write_reg(0x19, tmp);

	alc5616_write_reg(0x2A, 0x0202);/*Stereo DAC digital mixer control ,note:if comment, farend have no voice when communicating press mute */
	alc5616_writeRegMask(0x65,(1<<14)|(1<<15),(1<<14)|(1<<15));/*power on OUTMIXL/R note:prevent switch pop sound*/
 	// if(_alc5616_hpout) 
	// {
        // alc5616_writeRegMask(0x02, (0<<7)|(0<<15), (1<<7)|(1<<15));        
	// }
	// if(_alc5616_spkout)
	// {				
		// alc5616_writeRegMask(0x03, (0<<7)|(0<<15), (1<<7)|(1<<15));
	// } 
	
	// i2s_delay_us(1000); /* FIXME: dummy loop */
    _alc5616_mute = 0;
	pthread_mutex_unlock(&ALC5616_MUTEX);	
}

/* line-in bypass to line-out directly */
void itp_codec_playback_linein_bypass(unsigned bypass)
{
	uint16_t	reg16;
	uint16_t	BpVol=7;	
	
//	pthread_mutex_lock(&ALC5616_MUTEX);


//	pthread_mutex_unlock(&ALC5616_MUTEX);
}


/* ADC */
void itp_codec_rec_init(unsigned input_source)
{
	DEBUG_PRINT("ALC5616# %s\n", __func__);
	
	switch(input_source)
	{
		case 1: /* LineIN only */
		{
			_alc5616_micin  = 0;
			_alc5616_linein = 1;
			break;
		}
		case 2: /* MICIN only */
		{
			_alc5616_micin  = 1;
			_alc5616_linein = 0;
			break;
		}
		case 0: /* both LineIN & MICIN */
		default:
		{
			_alc5616_micin  = 1;
			_alc5616_linein = 1;
			break;
		}
	}

	init_alc5616_common();
	_alc5616_AD_running = 1;
}

void itp_codec_rec_deinit(void)
{
	DEBUG_PRINT("ALC5616# %s\n", __func__);

	_alc5616_AD_running = 0; /* put before deinit_alc5616_common() */
	deinit_alc5616_common();
}

void itp_codec_rec_set_direct_vol(unsigned target_vol)
{
	//int direction;
	//unsigned short data16;

	pthread_mutex_lock(&ALC5616_MUTEX);

	if(target_vol > MAX_INPUT_PGA_VOLUME)
	{
		 printf("ERROR# invalid target volume step: 0x%08x\n", target_vol);
		 pthread_mutex_unlock(&ALC5616_MUTEX);
		 return;
	}

	if     (target_vol == curr_input_pga_volume) { pthread_mutex_unlock(&ALC5616_MUTEX); return; }
	//else if(target_vol >  curr_input_pga_volume) { direction = 1; } /* + */
	//else                                         { direction = 0; } /* - */
	
	while(curr_input_pga_volume != target_vol)
	{
        curr_input_pga_volume = target_vol;
		//if(direction == 1) { curr_input_pga_volume++; }
		//else               { curr_input_pga_volume--; }

		
		if(_alc5616_AD_running)
		{		   
		   alc5616_writeRegMask(0x1C, curr_input_pga_volume << 8, 0x7F << 8);
           alc5616_writeRegMask(0x1C, curr_input_pga_volume << 0, 0x7F << 0);           
		}

		//i2s_delay_us(1000); /* FIXME: dummy loop */;
	}
	pthread_mutex_unlock(&ALC5616_MUTEX);
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

	pthread_mutex_lock(&ALC5616_MUTEX);

	while(MIN_INPUT_PGA_VOLUME != inpvol)
	{
		inpvol--;

		if(_alc5616_AD_running)
		{
			alc5616_writeRegMask(0x1C, inpvol << 8, 0x7F << 8);
		}

		i2s_delay_us(1000); /* FIXME: dummy loop */;
	}

	if(_alc5616_AD_running)
	{
	    alc5616_writeRegMask(0x3C, (1<<2), (1<<2));
        alc5616_writeRegMask(0x3E, (1<<2), (1<<2));     
	}
	
	pthread_mutex_unlock(&ALC5616_MUTEX);
}

void itp_codec_rec_unmute(void)
{
	unsigned short data16;
	unsigned inpvol = MIN_INPUT_PGA_VOLUME;
	
	pthread_mutex_lock(&ALC5616_MUTEX);
	
	while(inpvol != curr_input_pga_volume)
	{
		inpvol++;

		if(_alc5616_AD_running)
		{
            alc5616_writeRegMask(0x1C, inpvol << 8, 0x7F << 8);
		}

		i2s_delay_us(1000); /* FIXME: dummy loop */;
	}
    alc5616_writeRegMask(0x3C, (0<<2), (1<<2));
    alc5616_writeRegMask(0x3E, (0<<2), (1<<2));
	pthread_mutex_unlock(&ALC5616_MUTEX);
}

void itp_codec_power_on(void)
{
#ifdef CFG_AUDIOAMP_ENABLE
        // for entrance AMP.
        {
            unsigned short data16;
            data16 = (1 << 1) | (0x7 << 4);
            //alc5616_write_reg( 48, data16);            
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
                //alc5616_write_reg(48, data16);                
                return;
            }
#endif
    return;
}

void itp_codec_get_i2s_sample_rate(int* samplerate)
{
	pthread_mutex_lock(&ALC5616_MUTEX);
	*samplerate = i2s_sample_rate;
	pthread_mutex_unlock(&ALC5616_MUTEX);
}

void itp_codec_set_i2s_sample_rate(int samplerate)
{
	pthread_mutex_lock(&ALC5616_MUTEX);
	i2s_sample_rate = samplerate;	
	pthread_mutex_unlock(&ALC5616_MUTEX);
}

int itp_codec_get_DA_running(void){
    return _alc5616_DA_running;
}

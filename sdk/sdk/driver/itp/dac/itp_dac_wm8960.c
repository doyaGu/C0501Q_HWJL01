/* sy.chuang, 2012-0622, ITE Tech. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "ite/ith.h"
#include "ite/itp_codec.h"
#include "iic/mmp_iic.h"

#define WM8960_I2CADR               (0x34 >> 1)

#if CFG_DAC_port_I2C0
#define IIC_PORT IIC_PORT_0
#else
#define IIC_PORT IIC_PORT_1
#endif  

//#define DEBUG_PRINT printf
#define DEBUG_PRINT(...)

/* FIXME: need more verification */
//#ifdef CFG_CHIP_REV_A0
	#define WM8960_BEFORE_ACCESS_IIC    0
	#define WM8960_AFTER_ACCESS_IIC     0
//#else /* if (defined CFG_CHIP_REV_A1) */
//	#define WM8960_BEFORE_ACCESS_IIC    3
//	#define WM8960_AFTER_ACCESS_IIC     3
//#endif

//#define WM8960_DEBUG_I2C_COMMON_WRITE
//#define WM8960_DEBUG_PROGRAM_REGVALUE

/* ************************************************************************** */
/* wrapper */
static inline void i2s_delay_us(unsigned us) { ithDelay(us); }

/* ************************************************************************** */
#define MAX_OUT1_VOLUME            0x7F /* +6 dB */
#define ZERO_DB_OUT1_VOLUME        0x79 /*  0 dB */
#define MIN_OUT1_VOLUME            0x30 /* -73dB */
static unsigned curr_out1_volume = ZERO_DB_OUT1_VOLUME; /* 0 dB, refer to R2, R3 */

#define MAX_INPUT_PGA_VOLUME       0x3F /* +30   dB */
#define ZERO_DB_INPUT_PGA_VOLUME   0x17 /* +0    dB */
#define MIN_INPUT_PGA_VOLUME       0x00 /* -17.25dB */
static unsigned curr_input_pga_volume = ZERO_DB_INPUT_PGA_VOLUME; /* 0 dB, refer to R0, R1 */

static int _wm8960_DA_running = 0;
static int _wm8960_AD_running = 0;
static pthread_mutex_t WM8960_MUTEX = PTHREAD_MUTEX_INITIALIZER;

static int _wm8960_micin = 1;
static int _wm8960_linein = 0;
static int _wm8960_spkout = 1;
static int _wm8960_hpout = 1;

static int _wm8960_cold_start = 1; /* WM8960 should enable power only once */

static int i2s_sample_rate = 48000;

/* ************************************************************************** */
/* gamma correction */
#include "gc2.2.inc"
//#include "gc3.2.inc"
//#include "gc4.2.inc"
//#include "gc5.2.inc"
//#include "gc6.2.inc"

/* ************************************************************************** */
static void _init_iic_for_wm8960(void)
{
#if 0 /* Only use IIC send/receive API, IIC reset/init API shouldn't appear here ! */
	/* Host Select on GPIO[3:0], 100: Select As GPIO */
	{
		unsigned uret;
		uret = ithReadRegA(ITH_GPIO_BASE + 0xD0);
		uret &= ~(1 << 4);
		uret &= ~(1 << 5);
		uret &= ~(1 << 6);
		uret |=  (1 << 6);
		ithWriteRegA(ITH_GPIO_BASE + 0xD0, uret);
	}
	
	/* reset IIC controller */
	{
		unsigned uret;
		uret = ithReadRegA(0xDE100000 + 0x00);
		uret |= (1 << 0);
		ithWriteRegA(0xDE100000 + 0x00, uret);
	}
	
	/* init IIC */
	{
		unsigned uret;

		uret = mmpIicInitialize(0, 1);
		if(uret != 0) {
			printf("WM8960# init IIC Fail!\n");
    		while(1) { usleep(500000); }
    	}

		uret = ithReadRegA(ITH_GPIO_BASE + 0xD0);
		uret |= (1 << 30); /* HDMI IIC Slave ID, 0:0x98, 1:0x9A */
		ithWriteRegA(ITH_GPIO_BASE + 0xD0, uret);

		mmpIicSetClockRate(100* 1024); /* let IIC work at 100 kHz */
	}
#endif
}

static void I2C_common_write_byte(unsigned char RegAddr, unsigned char d)
{
	int success = 0;
	int flag;
	int retry = 0;

    ithWriteRegMaskA(ITH_GPIO_BASE | 0xD0, (WM8960_BEFORE_ACCESS_IIC << 28), (0x3 << 28)); /* IIC: internal HDMI IIC */
    for(retry=0; retry<50; retry++)
    {
        if(0 == (flag = mmpIicSendData(IIC_PORT, IIC_MASTER_MODE, WM8960_I2CADR, RegAddr, &d, 1)))
        {
        	success = 1;
#ifdef WM8960_DEBUG_I2C_COMMON_WRITE
        	printf("WM8960# IIC WriteOK!\n");
#endif
            break;
        }
    }
    if(success == 0) {
    	printf("WM8960# IIC Write Fail!\n");
    	while(1) { usleep(500000); }
    }
    ithWriteRegMaskA(ITH_GPIO_BASE | 0xD0, (WM8960_AFTER_ACCESS_IIC << 28), (0x3 << 28)); /* IIC: external APB IIC */
}

static void wm8960_write_reg(unsigned char reg_addr, unsigned short value)
{
	unsigned char A;
	unsigned char B;

#ifdef WM8960_DEBUG_PROGRAM_REGVALUE
	printf("WM8960# reg[0x%02x] = 0x%04x\n", reg_addr, value);
#endif

	A = ((reg_addr & 0x7F) << 1) | ((value >> 8) & 0x1);
	B = (value & 0xFF);

	I2C_common_write_byte(A, B);
}

void mic_input_control(void)
{
    unsigned short data16;

	//mic setting
	data16 = ((0x1 & 0x1) << 8) | //differential inputs
	         ((0x0 & 0x1) << 7) |
	         ((0x1 & 0x1) << 6) |
	         ((0x1 & 0x3) << 4) |
	         ((0x1 & 0x1) << 3);

	wm8960_write_reg(32, data16); /* enable VMID to PGA, and to boost mixer */
	wm8960_write_reg(33, data16); /* enable VMID to PGA, and to boost mixer */
}


static void init_wm8960_common(void)
{
	pthread_mutex_lock(&WM8960_MUTEX);

	DEBUG_PRINT("WM8960# %s\n", __func__);

	if(_wm8960_DA_running)
	{
		DEBUG_PRINT("WM8960# DAC is running, skip re-init process !\n");
		pthread_mutex_unlock(&WM8960_MUTEX);
		return;
	}
	if(_wm8960_AD_running)
	{
		DEBUG_PRINT("WM8960# ADC is running, skip re-init process !\n");
		pthread_mutex_unlock(&WM8960_MUTEX);
		return;
	}

	//_init_iic_for_wm8960();
	
	/* WM8960 should enable power only once */
	if(0) /* restore previous playback volume */
	{
		unsigned short data16;
		unsigned adj = 0x00;

		if     (curr_out1_volume > MAX_OUT1_VOLUME) { curr_out1_volume = MAX_OUT1_VOLUME; }
		else if(curr_out1_volume < MIN_OUT1_VOLUME) { curr_out1_volume = MIN_OUT1_VOLUME; }

		do
		{
			i2s_delay_us(1000); /* FIXME: dummy loop */
			adj++;
			data16 = (1 << 8) | (adj & 0x7F);
			if(_wm8960_hpout)
			{
			    wm8960_write_reg( 2, data16); /* adjust LOUT1 Vol */
			    wm8960_write_reg( 3, data16); /* adjust ROUT1 Vol */
			}
			
			if(_wm8960_spkout)
			{				
				wm8960_write_reg(40, data16); /* adjust SPK_L Vol */
				wm8960_write_reg(41, data16); /* adjust SPK_R Vol */
			}
		} while(adj != curr_out1_volume);

		i2s_delay_us(1000); /* FIXME: dummy loop */
		
		/* digital unmute flow */
		{
			data16 = (1 << 3) | (0 << 2);
			wm8960_write_reg( 6, data16);

			data16 = (0 << 3); /* DACMU */
			wm8960_write_reg( 5, data16);
		}
		pthread_mutex_unlock(&WM8960_MUTEX);
		return;
	}
	_wm8960_cold_start = 0;

	/* programming WM8960 */
	{
		unsigned short data16;
		int sample_rate;

		itp_codec_get_i2s_sample_rate(&sample_rate);		
		
		wm8960_write_reg(15, 0x000); /* reset WM8960 */
		
		if (_wm8960_spkout)
		{					  
		  if (sample_rate ==  8000)
				  wm8960_write_reg( 8, 0x080); //8K
		  else					
				  wm8960_write_reg( 8, 0x180); //others					
		}
		/* AD */
		wm8960_write_reg( 9, 0x040); /* audio interface: ADCLRC/GPIO1 select GPIO1 */
		
		if(_wm8960_micin)
		{
            
			wm8960_write_reg(47, 0x03C); /* enable left/right input PGA, left/right output mixer */
#ifdef CFG_FM2018_ENABLE 
			wm8960_write_reg(32, 0x198); /* enable input 1 & 3 to PGA, and to boost mixer */
			wm8960_write_reg(33, 0x198); /* enable input 1 & 3 to PGA, and to boost mixer */
#else
			wm8960_write_reg(32, 0x158); /* enable input 1 & 2 to PGA, and to boost mixer */
			wm8960_write_reg(33, 0x158); /* enable input 1 & 2 to PGA, and to boost mixer */
#endif

			/* restore previous rec volume */
            
            data16 = (1 << 8) | (curr_input_pga_volume & 0x3F);
			wm8960_write_reg( 0, data16); /* left input PGA disable analogue mute, 0dB */
			wm8960_write_reg( 1, data16); /* right input PGA disable analogue mute, 0dB */
		}
		else
		{
			wm8960_write_reg(47, 0x00C); /* enable left/right output mixer */
			wm8960_write_reg(32, 0x000); /* disable input PGA to input boost mixer */
			wm8960_write_reg(33, 0x000); /* disable input PGA to input boost mixer */
		}
		
		if(_wm8960_linein)
		{
			wm8960_write_reg(43, 0x070); /* LIN3 Boost = 6dB */
			wm8960_write_reg(44, 0x070); /* RIN3 Boost = 6dB */
		}
		else
		{
			wm8960_write_reg(43, 0x000); /* LIN 2&3 Boost = Mute */
			wm8960_write_reg(44, 0x000); /* RIN 2&3 Boost = Mute */
		}

		wm8960_write_reg(21, 0x1C3); /* Left ADC Vol = 0dB =(0x1C3)*/
		wm8960_write_reg(22, 0x1C3); /* Right ADC Vol = 0dB (0x1C3)*/
		
	    //ADC Data Output Select
		wm8960_write_reg(23, 0x1C4); /* [3:2]left data = left ADC; right data = left ADC */

		/* DA */
		wm8960_write_reg( 7, 0x00E); /* audio interface: data word length: 32 bits, I2S format */

#if 1 /* Wolfson's new flow @ 2013-0109, 12:51 */
		/* anti-pop */
		wm8960_write_reg(29, 0x040); /* Enable DISOP */
		usleep(400000);              /* Delay (400 ms) to remove any residual charge on HP output */
		wm8960_write_reg(28, 0x094); /* Enable POBCTRL, SOFT_ST and BUFDCOPEN */
		
		data16 = 0x060;
		if(_wm8960_spkout)	data16 = 0x78;	/* Enable SPK_L and SPK_R */
		wm8960_write_reg(26, data16);/* Enable LOUT1 and ROUT1 */
		
		usleep(50000);               /* Delay (50 ms) to allow HP amps to settle */
		wm8960_write_reg(25, 0x080); /* Enable VMID SEL = 2x50K Ohm Divider */
		usleep(100000);              /* Delay (100 ms) to allow VMID to initially charge */
		if(_wm8960_micin) {
			wm8960_write_reg(25, 0x0FE); /* VMID=50K, Enable VREF, AINL, AINR, ADCL, ADCR and MICB */
		}
		else {
			wm8960_write_reg(25, 0x0CC); /* Enable VREF, ADCL and ADCR. VMID SEL = 2x50K Ohm Divider remain */
		}
		wm8960_write_reg(28, 0x010); /* Disable POBCTRL and SOFT_ST. BUFDCOPEN remain enabled  (Delay from R25 = 080 to Disable POBCTRL and SOFT_ST >100mS) */
		//-
		data16 = 0x1E0;
		if(_wm8960_spkout)	data16 = 0x1F8;	/* Enable LOUT1 and ROUT1 */
		wm8960_write_reg(26, data16);

		//wm8960_write_reg(47, 0x00C); /* Enable left and right output mixers */
		wm8960_write_reg(34, 0x150); /* Enable Left DAC to left mixer */
		wm8960_write_reg(37, 0x150); /* Enable Right DAC to right mixer */
		wm8960_write_reg( 2, 0x100); /* LOUT1VOL (HP) = Analogue Mute */
		wm8960_write_reg( 3, 0x100); /* ROUT1VOL (HP) = Analogue Mute, Enable OUT1VU, load volume settings to both left and right channels */
		wm8960_write_reg(40, 0x100); /* SPK_LN & SPK_LP (SPK) = Analogue Mute */
		wm8960_write_reg(41, 0x100); /* SPK_RN & SPK_RP = Analogue Mute, Enable OUT1VU, load volume settings to both left and right channels */		
		
#if (CFG_CHIP_FAMILY == 9910)
        data16 = (1 << 8) | (0x7F & 0x7F);
#else
        data16 = (1 << 8) | (curr_out1_volume & 0x7F);
#endif  
		wm8960_write_reg( 2, data16); /* LOUT1 Vol = 0dB */
		wm8960_write_reg( 3, data16); /* ROUT1 Vol = 0dB */
		wm8960_write_reg(40, data16); /* SPK_LN & SPK_LP (SPK) = 0db*/
		wm8960_write_reg(41, data16); /* SPK_RN & SPK_RP = 0db*/

		if(_wm8960_spkout)
		{			
		    wm8960_write_reg(51, 0x0A4); /*0x0AD(+5.1dB)  /*0x0A4(+4.5dB) */ /*or 0x09B, DC Speaker Boost(gain +3.6dB)*/
			wm8960_write_reg(49, 0x077); /* Enable Class D Speaker Outputs , 0x77: Left speaker only, 0xF7: Left&Right speaker enable*/		
		}
		else
		{
			wm8960_write_reg(49, 0x037); /* Disable Class D Speaker Outputs */
			wm8960_write_reg(51, 0x080); /* or 0x080, DC Speaker Boost(gain +0.0dB)*/
		}
		
		wm8960_write_reg( 5, 0x000); /* DAC Digital Soft Mute = Unmute (Delay from R25 = 080 to unmute >250mS) */

#if 0
		/* restore previous playback volume */
		{
			unsigned adj = 0x00;

			if     (curr_out1_volume > MAX_OUT1_VOLUME) { curr_out1_volume = MAX_OUT1_VOLUME; }
			else if(curr_out1_volume < MIN_OUT1_VOLUME) { curr_out1_volume = MIN_OUT1_VOLUME; }

			do
			{
				i2s_delay_us(1000); /* FIXME: dummy loop */
				adj++;
				data16 = (1 << 8) | (adj & 0x7F);
				if(_wm8960_hpout)
				{
				wm8960_write_reg( 2, data16); /* adjust LOUT1 Vol */
				wm8960_write_reg( 3, data16); /* adjust ROUT1 Vol */
				}
				
				if(_wm8960_spkout)
				{
					wm8960_write_reg(40, data16); /* adjust SPK_L Vol */
					wm8960_write_reg(41, data16); /* adjust SPK_R Vol */
				}
			} while(adj != curr_out1_volume);
			i2s_delay_us(1000); /* FIXME: dummy loop */
		}
#endif		
#else
		/* anti-pop */
		wm8960_write_reg(28, 0x094); /* Enable POBCTRL, SOFT_ST and BUFDCOPEN */
		wm8960_write_reg(26, 0x060); /* Enable LOUT1 and ROUT1 */
		usleep(50000);
		wm8960_write_reg(25, 0x080); /* VMID=50K, Enable VREF, AINL, AINR, ADCL and ADCR */
		usleep(100000);
		if(_wm8960_micin) {
			wm8960_write_reg(25, 0x0FE); /* VMID=50K, Enable VREF, AINL, AINR, ADCL, ADCR and MICB */
		}
		else {
			wm8960_write_reg(25, 0x0CC); /* VMID=50K, Enable VREF, ADCL and ADCR */
		}
		wm8960_write_reg(28, 0x010); /* Disable POBCTRL and SOFT_ST. BUFDCOPEN remain enabled */
		//-
		
		wm8960_write_reg(26, 0x1E0); /* Enable DACL, DACR. LOUT1 and ROUT1 remain enabled */
		wm8960_write_reg(34, 0x170); /* Left DAC to left output mixer enabled (LD2LO) */
		wm8960_write_reg(37, 0x170); /* Right DAC to right output mixer enabled (RD2RO) */

		/* restore previous playback volume */
		data16 = (1 << 8) | (curr_out1_volume & 0x7F);
		wm8960_write_reg( 2, data16); /* LOUT1 Vol = 0dB */
		wm8960_write_reg( 3, data16); /* ROUT1 Vol = 0dB */

		wm8960_write_reg( 5, 0x000); /* Unmute DAC digital soft mute */
#endif
	}

//	/* L_RINPUT3_to_L_ROUT1_Bypass.txt: OK */
//	{
//		wm8960_write_reg(15, 0x000); //Reset WM8960    
//		wm8960_write_reg(25, 0x0F0); //VMID=50K, Enable VREF, AINL and AINR 
//		wm8960_write_reg(26, 0x060); //Enable LOUT1 and ROUT1 
//		wm8960_write_reg(47, 0x00C); //Enable left and right channel input PGA
//		wm8960_write_reg(34, 0x080); //Enable LINPUT3 to left output mixer (LI2LO), LINPUT3 to Left mixer vol = 0dB 
//		wm8960_write_reg(37, 0x080); //Enable RINPUT3 to right output mixer (RI2RO), RINPUT3 to Right mixer vol = 0dB 
//		wm8960_write_reg(2, 0x179); //LOUT1 Vol = 0dB, volume update enabled 
//		wm8960_write_reg(3, 0x179); //ROUT1 Vol = 0dB, volume update enabled  
//	}
	pthread_mutex_unlock(&WM8960_MUTEX);
}

static void deinit_wm8960_common(void)
{
	pthread_mutex_lock(&WM8960_MUTEX);
	DEBUG_PRINT("WM8960# %s\n", __func__);

	if(_wm8960_DA_running)
	{
		DEBUG_PRINT("WM8960# DAC is running, skip deinit !\n");
		pthread_mutex_unlock(&WM8960_MUTEX);
		return;
	}
	if(_wm8960_AD_running)
	{
		DEBUG_PRINT("WM8960# ADC is running, skip deinit !\n");
		pthread_mutex_unlock(&WM8960_MUTEX);
		return;
	}

	_init_iic_for_wm8960();

	/* digital mute flow */
	{
		unsigned short data16;

		data16 = (1 << 3) | (0 << 2);
		wm8960_write_reg( 6, data16);

		data16 = (1 << 3);
		wm8960_write_reg( 5, data16);
	}
//	i2s_delay_us(25000); /* FIXME: dummy loop */

	/* DON'T turn off power in any case */
	if     (curr_out1_volume > MAX_OUT1_VOLUME) { curr_out1_volume = MAX_OUT1_VOLUME; }
	else if(curr_out1_volume < MIN_OUT1_VOLUME) { curr_out1_volume = MIN_OUT1_VOLUME; }

	/* just decrease output volume */
	{
		unsigned short data16;
		unsigned adj = curr_out1_volume;
		if(adj != 0x00) {
			do
			{
				adj--;
				data16 = (1 << 8) | (adj & 0x7F);
				
				if(_wm8960_hpout)
				{
				wm8960_write_reg( 2, data16); /* adjust LOUT1 Vol */
				wm8960_write_reg( 3, data16); /* adjust ROUT1 Vol */
				}
				
				if(_wm8960_spkout)
				{
					wm8960_write_reg(40, data16); /* adjust SPK_L Vol */
					wm8960_write_reg(41, data16); /* adjust SPK_R Vol */
				}
				i2s_delay_us(1000); /* FIXME: dummy loop */
			} while(adj != 0x00);
		}
	}
	/* Prevent Pop Noise*/
	if(_wm8960_spkout)
	{
		wm8960_write_reg(49, 0x037); /* Disable Class D Speaker Outputs */
		wm8960_write_reg(51, 0x080); /* or 0x080, DC Speaker Boost(gain +0.0dB)*/
	}
	pthread_mutex_unlock(&WM8960_MUTEX);
	return;

	/* really turn-off WM8960 */
//#if 1 /* Wolfson's new flow @ 2013-0109, 12:51 */
//	wm8960_write_reg( 5, 0x008); /* DAC Digital Soft Mute = Mute */
//	wm8960_write_reg(28, 0x094); /* Enable POBCTRL and BUFDCOPEN and SOFT_ST */
//	wm8960_write_reg(25, 0x000); /* Disable VMID and VREF, (allow output to discharge with S-Curve) */
//	usleep(600000);              /* Delay 600 ms to discharge HP outputs */
//	wm8960_write_reg(26, 0x000); /* Disable DACL, DACR, LOUT and ROUT1 */
//	wm8960_write_reg(15, 0x000); /* Reset Device (default registers) */
//#else /* original version */
//	wm8960_write_reg( 5, 0x008); //DAC Digital Soft Mute = Mute
//	wm8960_write_reg(28, 0x090); //Enable POBCTRL and BUFDCOPEN
//	wm8960_write_reg(25, 0x000); //Disable VMID and VREF
//	wm8960_write_reg(28, 0x190); //Enable VMIDTOG to discharge VMID capacitor
//	usleep(800000);
//	wm8960_write_reg(26, 0x000); //Disable DACL, DACR, LOUT1, ROUT1
//	wm8960_write_reg(15, 0x000); //Reset Device (default registers)
//#endif
}

/* ************************************************************************** */
/* === common DAC/ADC ops === */
void itp_codec_wake_up(void)
{
}

void itp_codec_standby(void)
{
	pthread_mutex_lock(&WM8960_MUTEX);
	printf("WM8960# %s\n", __func__);

	_init_iic_for_wm8960();
	
	/* disable CLASS D output */
	if(_wm8960_spkout)
    {
	    wm8960_write_reg(51, 0x080); /* or 0x080, DC Speaker Boost(gain +0.0dB)*/
	    wm8960_write_reg(49, 0x037); /* Disable Class D Speaker Outputs */
        usleep(100000);
    }

	/* really turn-off WM8960 */
#if 1 /* Wolfson's new flow @ 2013-0109, 12:51 */
	wm8960_write_reg( 5, 0x008); /* DAC Digital Soft Mute = Mute */
	wm8960_write_reg(28, 0x094); /* Enable POBCTRL and BUFDCOPEN and SOFT_ST */
	wm8960_write_reg(25, 0x000); /* Disable VMID and VREF, (allow output to discharge with S-Curve) */
	usleep(600000);              /* Delay 600 ms to discharge HP outputs */
	wm8960_write_reg(26, 0x000); /* Disable DACL, DACR, LOUT and ROUT1 */
	wm8960_write_reg(15, 0x000); /* Reset Device (default registers) */
#else /* original version */
	wm8960_write_reg( 5, 0x008); //DAC Digital Soft Mute = Mute
	wm8960_write_reg(28, 0x090); //Enable POBCTRL and BUFDCOPEN
	wm8960_write_reg(25, 0x000); //Disable VMID and VREF
	wm8960_write_reg(28, 0x190); //Enable VMIDTOG to discharge VMID capacitor
	usleep(800000);
	wm8960_write_reg(26, 0x000); //Disable DACL, DACR, LOUT1, ROUT1
	wm8960_write_reg(15, 0x000); //Reset Device (default registers)
#endif

	_wm8960_cold_start = 1;
	pthread_mutex_unlock(&WM8960_MUTEX);
}

/* DAC */
void itp_codec_playback_init(unsigned output)
{
	DEBUG_PRINT("WM8960# %s\n", __func__);

	switch(output)
	{
		case 1: /* Speaker only */
		{
			_wm8960_spkout  = 1;
			_wm8960_hpout = 0;
			break;
		}
		case 2: /* both Speaker & HeadPhone */
		{
			_wm8960_spkout  = 1;
			_wm8960_hpout = 1;
			break;
		}
		case 0: /* HeadPhone only */
		default:
		{
			_wm8960_spkout  = 0;
			_wm8960_hpout = 1;
			break;
		}
	}
	
	init_wm8960_common();
	_wm8960_DA_running = 1;
}

void itp_codec_playback_deinit(void)
{
	DEBUG_PRINT("WM8960# %s\n", __func__);

	_wm8960_DA_running = 0; /* put before deinit_wm8960_common() */
	deinit_wm8960_common();
}

void itp_codec_playback_amp_volume_down(void)
{
	if(curr_out1_volume <= MIN_OUT1_VOLUME) return;

	switch(curr_out1_volume)
	{
		case 0x7D ... 0x7F: { itp_codec_playback_set_direct_vol(0x7C); break; } /* +3 dB */
		case 0x7A ... 0x7C: { itp_codec_playback_set_direct_vol(0x79); break; } /*  0 dB */
		case 0x77 ... 0x79: { itp_codec_playback_set_direct_vol(0x76); break; } /* -3 dB */
		case 0x74 ... 0x76: { itp_codec_playback_set_direct_vol(0x73); break; } /* -6 dB */
		case 0x70 ... 0x73: { itp_codec_playback_set_direct_vol(0x6F); break; } /* -10dB */
		case 0x5C ... 0x6F: { itp_codec_playback_set_direct_vol(0x5B); break; } /* -30dB */
		case 0x31 ... 0x5B: { itp_codec_playback_set_direct_vol(0x30); break; } /* -73dB */
	}
}

void itp_codec_playback_amp_volume_up(void)
{
	if(curr_out1_volume >= MAX_OUT1_VOLUME) return;

	switch(curr_out1_volume)
	{
		case 0x7C ... 0x7E: { itp_codec_playback_set_direct_vol(0x7F); break; } /* +6 dB */
		case 0x79 ... 0x7B: { itp_codec_playback_set_direct_vol(0x7C); break; } /* +3 dB */
		case 0x76 ... 0x78: { itp_codec_playback_set_direct_vol(0x79); break; } /*  0 dB */
		case 0x73 ... 0x75: { itp_codec_playback_set_direct_vol(0x76); break; } /* -3 dB */
		case 0x6F ... 0x72: { itp_codec_playback_set_direct_vol(0x73); break; } /* -6 dB */
		case 0x5B ... 0x6E: { itp_codec_playback_set_direct_vol(0x6F); break; } /* -10dB */
		case 0x30 ... 0x5A: { itp_codec_playback_set_direct_vol(0x5B); break; } /* -30dB */
	}
}

void itp_codec_playback_set_direct_vol(unsigned target_vol)
{
	int direction;
	unsigned short data16;

	pthread_mutex_lock(&WM8960_MUTEX);

	if(target_vol > MAX_OUT1_VOLUME)
	{
		 printf("ERROR# invalid target volume step: 0x%08x\n", target_vol);
		 pthread_mutex_unlock(&WM8960_MUTEX);
		 return;
	}

	if     (target_vol == curr_out1_volume) { pthread_mutex_unlock(&WM8960_MUTEX); return; }
	//else if(target_vol >  curr_out1_volume) { direction = 1; } /* + */
	//else                                    { direction = 0; } /* - */

	_init_iic_for_wm8960();

	while(curr_out1_volume != target_vol)
	{
        curr_out1_volume = target_vol;
		//if(direction == 1) { curr_out1_volume++; }
		//else               { curr_out1_volume--; }

		if(_wm8960_DA_running)
		{
			data16 = (1 << 8)
			| (3 << 6)
			| (0 << 1)
			| (1 << 0);
			wm8960_write_reg(23, data16);

			data16 = (1 << 7) | curr_out1_volume;
			
			if(_wm8960_hpout)
			{
			wm8960_write_reg(2, data16);
				wm8960_write_reg(3, data16);
			}
			
			if(_wm8960_spkout)
			{
				wm8960_write_reg(40, data16);
				wm8960_write_reg(41, data16);
			}

			data16 = (1 << 8) | (1 << 7) | curr_out1_volume;
			
			if(_wm8960_hpout)
			{
				wm8960_write_reg(2, data16);
			wm8960_write_reg(3, data16);
		}

			if(_wm8960_spkout)
			{
				wm8960_write_reg(40, data16);
				wm8960_write_reg(41, data16);
			}
		}

		//i2s_delay_us(1000); /* FIXME: dummy loop */;
	}
	pthread_mutex_unlock(&WM8960_MUTEX);
}

void itp_codec_playback_set_direct_volperc(unsigned target_volperc)
{
	unsigned char volstep;

	if(wm8960_perc_to_reg_tableSize != 100) {
		printf("ERROR# invalid wm8960_perc_to_reg_tableSize !\n");
		return;
	}
	if(target_volperc >= 100) { target_volperc = 99; }
	
	volstep = wm8960_perc_to_reg_table[target_volperc];
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
		if((wm8960_perc_to_reg_table[i] <= curr_out1_volume)
		&& (curr_out1_volume < wm8960_perc_to_reg_table[i+1])) {
			*currvolperc = i;
			return;
		}
	}
	*currvolperc = 99;
}

void itp_codec_playback_get_vol_range(unsigned *max, unsigned *regular_0db, unsigned *min)
{
	*max         = MAX_OUT1_VOLUME;
	*regular_0db = ZERO_DB_OUT1_VOLUME;
	*min         = MIN_OUT1_VOLUME;
}

void itp_codec_playback_mute(void)
{
	pthread_mutex_lock(&WM8960_MUTEX);
#if 0
//	if     (curr_out1_volume > MAX_OUT1_VOLUME) { curr_out1_volume = MAX_OUT1_VOLUME; }
//	else if(curr_out1_volume < MIN_OUT1_VOLUME) { curr_out1_volume = MIN_OUT1_VOLUME; }
//
//	_init_iic_for_wm8960();
//
//	/* from current to 0x00 */
//	{
//		unsigned short data16;
//		unsigned adj = curr_out1_volume;
//		if(adj != 0x00) {
//			do
//			{
//				adj--;
//				data16 = (1 << 8) | (adj & 0x7F);
//				wm8960_write_reg( 2, data16); /* adjust LOUT1 Vol */
//				wm8960_write_reg( 3, data16); /* adjust ROUT1 Vol */
//				i2s_delay_us(1000); /* FIXME: dummy loop */
//			} while(adj != 0x00);
//		}
//	}
//	i2s_delay_us(1000); /* FIXME: dummy loop */
#else
	_init_iic_for_wm8960();
		
	{
		unsigned short data16;

		data16 = (1 << 3) | (0 << 2);
		wm8960_write_reg( 6, data16);

		data16 = (1 << 3);
		wm8960_write_reg( 5, data16);
	}
	i2s_delay_us(25000); /* FIXME: dummy loop */
    	
    /* DON'T turn off power in any case */
        //if     (curr_out1_volume > MAX_OUT1_VOLUME) { curr_out1_volume = MAX_OUT1_VOLUME; }
        //else if(curr_out1_volume < MIN_OUT1_VOLUME) { curr_out1_volume = MIN_OUT1_VOLUME; }
    
        /* just decrease output volume */
        {
            unsigned short data16;
            unsigned adj = curr_out1_volume;

            data16 = (1 << 8) | (0x00 & 0x7F);
            
            if(_wm8960_hpout)
            {
                wm8960_write_reg( 2, data16); /* adjust LOUT1 Vol */
                wm8960_write_reg( 3, data16); /* adjust ROUT1 Vol */
            }
            
            if(_wm8960_spkout)
            {
                wm8960_write_reg(40, data16); /* adjust SPK_L Vol */
                wm8960_write_reg(41, data16); /* adjust SPK_R Vol */
            }
            i2s_delay_us(1000); /* FIXME: dummy loop */

        }
#endif
	pthread_mutex_unlock(&WM8960_MUTEX);
}

void itp_codec_playback_unmute(void)
{
	pthread_mutex_lock(&WM8960_MUTEX);
#if 0
//	if     (curr_out1_volume > MAX_OUT1_VOLUME) { curr_out1_volume = MAX_OUT1_VOLUME; }
//	else if(curr_out1_volume < MIN_OUT1_VOLUME) { curr_out1_volume = MIN_OUT1_VOLUME; }
//
//	_init_iic_for_wm8960();
//
//	/* from 0x00 to current */
//	{
//		unsigned short data16;
//		unsigned adj = 0x00;
//		do
//		{
//			i2s_delay_us(1000); /* FIXME: dummy loop */
//			adj++;
//			data16 = (1 << 8) | (adj & 0x7F);
//			wm8960_write_reg( 2, data16); /* adjust LOUT1 Vol */
//			wm8960_write_reg( 3, data16); /* adjust ROUT1 Vol */
//		} while(adj != curr_out1_volume);
//	}
//	i2s_delay_us(1000); /* FIXME: dummy loop */
#else
	//_init_iic_for_wm8960();
	
    unsigned short data16;
    unsigned adj = 0x00;

	if     (curr_out1_volume > MAX_OUT1_VOLUME) { curr_out1_volume = MAX_OUT1_VOLUME; }
	else if(curr_out1_volume < MIN_OUT1_VOLUME) { curr_out1_volume = MIN_OUT1_VOLUME; }

    adj = curr_out1_volume;

	i2s_delay_us(1000); /* FIXME: dummy loop */
	
	data16 = (1 << 8) | (adj & 0x7F);
	if(_wm8960_hpout)
	{
	    wm8960_write_reg( 2, data16); /* adjust LOUT1 Vol */
	    wm8960_write_reg( 3, data16); /* adjust ROUT1 Vol */
	}
	
	if(_wm8960_spkout)
	{				
		wm8960_write_reg(40, data16); /* adjust SPK_L Vol */
		wm8960_write_reg(41, data16); /* adjust SPK_R Vol */
	}
	
	i2s_delay_us(1000); /* FIXME: dummy loop */

	{
		unsigned short data16;

		data16 = (1 << 3) | (0 << 2);
		wm8960_write_reg( 6, data16);

		data16 = (0 << 3); /* DACMU */
		wm8960_write_reg( 5, data16);
	}

#endif
	pthread_mutex_unlock(&WM8960_MUTEX);
}

/* line-in bypass to line-out directly */
void itp_codec_playback_linein_bypass(unsigned bypass)
{
	uint16_t	reg16;
	uint16_t	BpVol=7;	
	
	pthread_mutex_lock(&WM8960_MUTEX);
	
	if(bypass)
	{	
#ifdef	ENABLE_FULL_BYPASS
		wm8960_write_reg(34, 0x170);	/* set LINPUT3 as -21dB */
		wm8960_write_reg(37, 0x170);	/* set RINPUT3 as -21dB */
		i2s_delay_us(1000); 			/* FIXME: dummy loop */
		
		wm8960_write_reg(34, 0x0F0);	/* set LINPUT3 as bypass mode and cut the DAC path */
		wm8960_write_reg(37, 0x0F0);	/* set RINPUT3 as bypass mode and cut the DAC path */   
		i2s_delay_us(1000); 			/* FIXME: dummy loop */
		
		/* volume up from -21dB to 0dB */
		BpVol = 7;
		while(1)
		{
			reg16 = 0x080 | (BpVol<<4);
			wm8960_write_reg(34, reg16);	/* set LINPUT3 as bypass mode */
			wm8960_write_reg(37, reg16);	/* set RINPUT3 as bypass mode */
			
			if(!BpVol--)	break;
				
			i2s_delay_us(10000); /* FIXME: dummy loop */
		}			
#else
		wm8960_write_reg(34, 0x080);	/* set LINPUT3 as bypass mode and cut the DAC path */
		wm8960_write_reg(37, 0x080);	/* set RINPUT3 as bypass mode and cut the DAC path */
		i2s_delay_us(1000); 			/* FIXME: dummy loop */
#endif
	}
	else
	{	
#ifdef	ENABLE_FULL_BYPASS
		/* bypass path volume down from 0dB to -21dB */
		BpVol = 0;
		while(BpVol<=7)
		{
			reg16 = 0x080 | (BpVol++<<4);
			wm8960_write_reg(34, reg16);	/* set LINPUT3 volume */
			wm8960_write_reg(37, reg16);	/* set RINPUT3 volume */
			i2s_delay_us(10000); /* FIXME: dummy loop */
		}
		
		/* mute hpout & spkout */
		if(_wm8960_hpout)
		{
			wm8960_write_reg( 2, 0x180); /* adjust LOUT1 Vol */
			wm8960_write_reg( 3, 0x180); /* adjust ROUT1 Vol */
		}
		
		if(_wm8960_spkout)
		{
			wm8960_write_reg(40, 0x180); /* adjust LOUT1 Vol */
			wm8960_write_reg(41, 0x180); /* adjust ROUT1 Vol */
		}
		i2s_delay_us(1000); /* FIXME: dummy loop */		
#endif
		/* cut the bypass path and connect dac to line-out*/
		wm8960_write_reg(34, 0x170);	/* cut LINPUT3 bypass path and connect DAC path */
		wm8960_write_reg(37, 0x170);	/* cut RINPUT3 bypass path and connect DAC path */
		i2s_delay_us(1000); /* FIXME: dummy loop */	
		
#ifdef	ENABLE_FULL_BYPASS
		/* fading in headphone/speaker volume */
		BpVol = 0;
		do
		{
			i2s_delay_us(1000); /* FIXME: dummy loop */
			BpVol++;
			reg16 = (3 << 7) | (BpVol & 0x7F);
			if(_wm8960_hpout)
			{
				wm8960_write_reg( 2, reg16); /* adjust LOUT1 Vol */
				wm8960_write_reg( 3, reg16); /* adjust ROUT1 Vol */
			}
			
			if(_wm8960_spkout)
			{				
				wm8960_write_reg(40, reg16); /* adjust SPK_L Vol */
				wm8960_write_reg(41, reg16); /* adjust SPK_R Vol */
			}
		} while(BpVol != curr_out1_volume);	
#endif
	}
	pthread_mutex_unlock(&WM8960_MUTEX);
}

/* ADC */
void itp_codec_rec_init(unsigned input_source)
{
	DEBUG_PRINT("WM8960# %s\n", __func__);

	switch(input_source)
	{
		case 1: /* LineIN only */
		{
			_wm8960_micin  = 0;
			_wm8960_linein = 1;
			break;
		}
		case 2: /* MICIN only */
		{
			_wm8960_micin  = 1;
			_wm8960_linein = 0;
			break;
		}
		case 0: /* both LineIN & MICIN */
		default:
		{
			_wm8960_micin  = 1;
			_wm8960_linein = 1;
			break;
		}
	}

	init_wm8960_common();
	_wm8960_AD_running = 1;
}

void itp_codec_rec_deinit(void)
{
	DEBUG_PRINT("WM8960# %s\n", __func__);

	_wm8960_AD_running = 0; /* put before deinit_wm8960_common() */
	deinit_wm8960_common();
}

void itp_codec_rec_set_direct_vol(unsigned target_vol)
{
	int direction;
	unsigned short data16;

	pthread_mutex_lock(&WM8960_MUTEX);

	if(target_vol > MAX_INPUT_PGA_VOLUME)
	{
		 printf("ERROR# invalid target volume step: 0x%08x\n", target_vol);
		 pthread_mutex_unlock(&WM8960_MUTEX);
		 return;
	}

	if     (target_vol == curr_input_pga_volume) { pthread_mutex_unlock(&WM8960_MUTEX); return; }
	else if(target_vol >  curr_input_pga_volume) { direction = 1; } /* + */
	else                                         { direction = 0; } /* - */

	_init_iic_for_wm8960();

	while(curr_input_pga_volume != target_vol)
	{
		if(direction == 1) { curr_input_pga_volume++; }
		else               { curr_input_pga_volume--; }

		if(_wm8960_AD_running)
		{
			data16 = (1 << 8) | curr_input_pga_volume;
			wm8960_write_reg(0, data16);
			wm8960_write_reg(1, data16);
		}

		i2s_delay_us(1000); /* FIXME: dummy loop */;
	}
	pthread_mutex_unlock(&WM8960_MUTEX);
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

	pthread_mutex_lock(&WM8960_MUTEX);

	_init_iic_for_wm8960();

	while(MIN_INPUT_PGA_VOLUME != inpvol)
	{
		inpvol--;

		if(_wm8960_AD_running)
		{
			data16 = (1 << 8) | inpvol;
			wm8960_write_reg(0, data16);
			wm8960_write_reg(1, data16);
		}

		i2s_delay_us(1000); /* FIXME: dummy loop */;
	}

	if(_wm8960_AD_running)
	{
		data16 = (1 << 8) | (1 << 7);
		wm8960_write_reg(0, data16);
		wm8960_write_reg(1, data16);
	}
	
	pthread_mutex_unlock(&WM8960_MUTEX);
}

void itp_codec_rec_unmute(void)
{
	unsigned short data16;
	unsigned inpvol = MIN_INPUT_PGA_VOLUME;
	
	pthread_mutex_lock(&WM8960_MUTEX);
	
	_init_iic_for_wm8960();

	while(inpvol != curr_input_pga_volume)
	{
		inpvol++;

		if(_wm8960_AD_running)
		{
			data16 = (1 << 8) | inpvol;
			wm8960_write_reg(0, data16);
			wm8960_write_reg(1, data16);
		}

		i2s_delay_us(1000); /* FIXME: dummy loop */;
	}
	
	pthread_mutex_unlock(&WM8960_MUTEX);
}

void itp_codec_power_on(void)
{
#ifdef CFG_AUDIOAMP_ENABLE
        // for entrance AMP.
        {
            unsigned short data16;
            data16 = (1 << 1) | (0x7 << 4);
            wm8960_write_reg( 48, data16);            
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
                wm8960_write_reg(48, data16);                
                return;
            }
#endif
    return;
}

void itp_codec_get_i2s_sample_rate(int* samplerate)
{
	pthread_mutex_lock(&WM8960_MUTEX);
	*samplerate = i2s_sample_rate;
	pthread_mutex_unlock(&WM8960_MUTEX);
}

void itp_codec_set_i2s_sample_rate(int samplerate)
{
	pthread_mutex_lock(&WM8960_MUTEX);
	i2s_sample_rate = samplerate;	
	pthread_mutex_unlock(&WM8960_MUTEX);
}

int itp_codec_get_DA_running(void){
    return _wm8960_DA_running;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ite/ith.h"
#include "ite/itp.h"

#include "i2s_reg_9920.h"
#include "i2s_it9920.h"

/* workaround table, due to amp bug */
#include "ite9920/D_table.inc"
#include "ite9920/A_table.inc"

typedef signed   long long s64;
typedef signed   int       s32;
typedef signed   short     s16;
typedef signed   char      s8;
typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned short     u16;
typedef unsigned char      u8;

/* ************************************************************************** */
#define MAX_OUT_VOLUME           0x100
#define MIN_OUT_VOLUME           0x00
static int current_volstep = MAX_OUT_VOLUME;

#define MAX_IN_VOLUME            0x100
#define MIN_IN_VOLUME            0x00
static int current_micstep = MAX_IN_VOLUME;


static int _ite9920_micin  = 0;
static int _ite9920_linein = 0;
static int _ite9920_spkout = 0;
static int _ite9920_hpout  = 0;
static int _ite9920_DA_running = 0;
static int _ite9920_AD_running = 0;

static void _internal_DAC_9920_reset(void);
static void _internal_DAC_9920_power_on(void);
static void _internal_DAC_9920_power_off(void);
static void _internal_DAC_9920_unmute(void);
static void _internal_DAC_9920_mute(void);

/* workaround table, due to amp bug */
static void _vol_setR(unsigned step);

/* ************************************************************************** */
static void _internal_DAC_9920_reset(void)
{

}

static void _internal_DAC_9920_power_on(void)
{

}

static void _internal_DAC_9920_power_off(void)
{

}

static void _internal_DAC_9920_unmute(void)
{

}

static void _internal_DAC_9920_mute(void)
{

}

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
    printf("ite9920# %s\n", __func__);
    switch(output)
	{
		case 1: /* Speaker only */
		{
			_ite9920_spkout  = 1;
			_ite9920_hpout = 0;
			break;
		}
		case 2: /* both Speaker & HeadPhone */
		{
			_ite9920_spkout  = 1;
			_ite9920_hpout = 1;
			break;
		}
		case 0: /* HeadPhone only */
		default:
		{
			_ite9920_spkout  = 0;
			_ite9920_hpout = 1;
			break;
		}
	}
	
    _internal_DAC_9920_reset();
	_ite9920_DA_running = 1;

}

void itp_codec_playback_deinit(void)
{

}

void itp_codec_playback_amp_volume_down(void)
{

}

void itp_codec_playback_amp_volume_up(void)
{

}

void itp_codec_playback_set_direct_vol(unsigned int volstep)
{// set volstep to register;
    u32 data32;

	if(volstep > MAX_OUT_VOLUME) {
		printf("ERROR# invalid target volume step: 0x%08x\n", volstep);
		return;
	}
	if(volstep == current_volstep) {
		return;
	}
    current_volstep = volstep;
    ithWriteRegMaskA(I2S_REG_OUT_VOLUME,volstep,0x1FF);

}

void itp_codec_playback_set_direct_volperc(unsigned target_volperc)
{   // precent to step :ex  82% -> 0x90 
	unsigned int volstep;

	if(it9920DA_perc_to_reg_tableSize != 100) {
		printf("ERROR# invalid it9920DA_perc_to_reg_tableSize !\n");
		return;
	}
	if(target_volperc >= 100) { target_volperc = 99; }

	volstep = it9920DA_perc_to_reg_table[target_volperc];
	itp_codec_playback_set_direct_vol(volstep);    

}

void itp_codec_playback_get_currvol(unsigned *currvolperc)
{
	unsigned i;
	for(i=0; i<99; i++)
	{
		if((it9920DA_perc_to_reg_table[i] >= current_volstep)
		&& (current_volstep > it9920DA_perc_to_reg_table[i+1])) {
			*currvolperc = i;
			return;
		}
	}
	*currvolperc = 99;
}

void itp_codec_playback_get_currvolperc(unsigned *currvolperc)
{

}

void itp_codec_playback_get_vol_range(unsigned *max, unsigned *regular_0db, unsigned *min)
{

}

void itp_codec_playback_mute(void)
{
}

void itp_codec_playback_unmute(void)
{
}

void itp_codec_playback_linein_bypass(unsigned bypass)
{
}

/* ADC */
void itp_codec_rec_init(unsigned input_source)
{
	printf("ite9920# %s\n", __func__);
	
	switch(input_source)
	{
		case 1: /* LineIN only */
		{
			_ite9920_micin  = 0;
			_ite9920_linein = 1;
			break;
		}
		case 2: /* MICIN only */
		{
			_ite9920_micin  = 1;
			_ite9920_linein = 0;
			break;
		}
		case 0: /* both LineIN & MICIN */
		default:
		{
			_ite9920_micin  = 1;
			_ite9920_linein = 0;
			break;
		}
	}

	_internal_DAC_9920_reset();
	_ite9920_AD_running = 1;    
    
}

void itp_codec_rec_deinit(void)
{
	/* HARDWARE NOT SUPPORT RECORDING ! */
}

void itp_codec_rec_set_direct_vol(unsigned int micstep)
{// set to register;
    u32 data32;

	if(micstep > MAX_IN_VOLUME) {
		printf("ERROR# invalid target volume step: 0x%08x\n", micstep);
		return;
	}

	if(micstep == current_micstep) {
		return;
	}
    
    ithWriteRegMaskA(I2S_REG_IN_VOLUME, micstep, 0x1FF);
}

void itp_codec_rec_set_direct_volperc(unsigned target_micperc)
{   // precent to step :ex  82% -> 0x90 
	unsigned int micstep;

	if(it9920AD_perc_to_reg_tableSize != 100) {
		printf("ERROR# invalid it9920AD_perc_to_reg_tableSize !\n");
		return;
	}
	if(target_micperc >= 100) { target_micperc = 99; }

	micstep = it9920DA_perc_to_reg_table[target_micperc];
    itp_codec_rec_set_direct_vol(micstep);

}

void itp_codec_rec_get_currvol(unsigned *currvol)
{
	unsigned i;
	for(i=0; i<99; i++)
	{
		if((it9920DA_perc_to_reg_table[i] >= current_volstep)
		&& (current_volstep > it9920DA_perc_to_reg_table[i+1])) {
            *currvol = i;
			return;
		}
	}
    *currvol = 99;
}

void itp_codec_rec_get_vol_range(unsigned *max, unsigned *regular_0db, unsigned *min)
{
	*max         = MAX_IN_VOLUME;
	*regular_0db = current_volstep; //current step;
	*min         = MIN_IN_VOLUME;
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


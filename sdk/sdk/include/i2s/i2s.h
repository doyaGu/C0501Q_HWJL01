/* sy.chuang, 2012-0423, ITE Tech. */

#ifndef I2S_H
#define I2S_H

#ifdef __cplusplus
extern "C" {
#endif

/* ************************************************************************** */
typedef struct
{
	/* input/ouput common */
	unsigned channels; /* ex: 1(mono) or 2(stereo) */
	unsigned sample_rate; /* ex: 44100/48000 Hz */
	unsigned buffer_size;
	unsigned is_big_endian;
	unsigned char *base_i2s;
	unsigned sample_size; /* ex: 16/24/32 bits */

	/* for input use */
	unsigned from_LineIN;
	unsigned from_MIC_IN;

	/* for output use */
	unsigned num_hdmi_audio_buffer; /* 0: no hdmi audio output (could save bandwidth) */
	unsigned is_dac_spdif_same_buffer;
#if (CFG_CHIP_FAMILY == 9850)
#else
	unsigned char *base_hdmi[4];
#endif
	unsigned char *base_spdif;
	unsigned postpone_audio_output; /* manually enable audio output */

	/* for input use */
	unsigned record_mode; /* 0: hardware start via capture hardware, 1: hardware start via software set */
	
	/* for output use */
	unsigned enable_Speaker;	//speaker-out
	unsigned enable_HeadPhone;	//line-out. If "enable_Speaker"=0, "enable_HeadPhone" will be set as true in i2s driver defaut setting.
} STRC_I2S_SPEC;

/* ************************************************************************** */
/* DA */
unsigned I2S_DA32_GET_RP(void);
unsigned I2S_DA32_GET_WP(void);
void I2S_DA32_SET_WP(unsigned data32);
void I2S_DA32_SET_LAST_WP(unsigned data32);
void I2S_DA32_WAIT_RP_EQUAL_WP(void);
unsigned short I2S_DA_STATUS1(void);
#if (CFG_CHIP_FAMILY == 9850)
unsigned int i2s_is_DAstarvation(void);
#else
unsigned i2s_is_DAstarvation(void);
#endif


int  i2s_get_DA_running(void);
/* AD */
#if (CFG_CHIP_FAMILY == 9850)
unsigned int I2S_AD32_GET_RP(void);
unsigned int I2S_AD32_GET_WP(void);
void I2S_AD32_SET_RP(unsigned int data16);
#else
unsigned short I2S_AD16_GET_RP(void);
unsigned short I2S_AD16_GET_WP(void);
void I2S_AD16_SET_RP(unsigned short data16);
#endif
unsigned short I2S_AD_STATUS(void);

/********************** export APIs **********************/
void i2s_CODEC_wake_up(void);
void i2s_CODEC_standby(void);

/* for deliver Kconfig option */
void i2s_init_gpio_mode2(void);
void i2s_init_spdif(void);
void i2s_init_output_pin(void);

/* DA */
void i2s_volume_up(void);
void i2s_volume_down(void);
void i2s_pause_DAC(int pause);
void i2s_deinit_DAC(void);
void i2s_init_DAC(STRC_I2S_SPEC *i2s_spec);
void i2s_mute_DAC(int mute);
int i2s_set_direct_volstep(unsigned volstep);
int i2s_set_direct_volperc(unsigned volperc);
unsigned i2s_get_current_volstep(void);
unsigned i2s_get_current_volperc(void);
void i2s_get_volstep_range(unsigned *max, unsigned *normal, unsigned *min);
void i2s_set_linein_bypass(int bypass);
#if (CFG_CHIP_FAMILY == 9850)
void i2s_mute_volume(int mute);
void i2s_enable_fading(int yesno);
#endif

/* AD */
void i2s_pause_ADC(int pause);
void i2s_deinit_ADC(void);
void i2s_init_ADC(STRC_I2S_SPEC *i2s_spec);
int i2s_ADC_set_direct_volstep(unsigned volstep);
unsigned i2s_ADC_get_current_volstep(void);
void i2s_ADC_get_volstep_range(unsigned *max, unsigned *normal, unsigned *min);
void i2s_mute_ADC(int mute);

/* ************************************************************************** */
#ifdef __cplusplus
}
#endif

#endif //I2S_H


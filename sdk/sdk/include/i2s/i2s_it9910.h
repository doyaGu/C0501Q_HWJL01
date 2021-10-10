/* sy.chuang, 2012-0823, ITE Tech. */

/* for 9910 */

#ifndef MMP_I2S_H
#define MMP_I2S_H

#ifdef __cplusplus
extern "C" {
#endif

/* ************************************************************************** */
typedef struct
{
	/* input/ouput common */
	unsigned use_hdmirx;       /* ex: 0:(no use hdmirx), 1:(use hdmirx) */
	unsigned internal_hdmirx;  /* ex: 0:(ext. hdmirx, or 'NoUSE hdmirx'), 1:(internal hdmirx) */
    unsigned use_hdmitx;       /* ex: 0:(no use hdmitx), 1:(use hdmitx) */
	unsigned slave_mode;       /* ex: 0:(i2s master), 1:(i2s slave) */
	unsigned channels;         /* ex: 1:(mono), 2:(stereo) */
	unsigned sample_rate;      /* ex: 44100/48000 Hz */
	unsigned buffer_size;
	unsigned is_big_endian;
	unsigned char *base_i2s;
	unsigned sample_size;      /* ex: 8/16/24/32 bits */

	/* for input use */
	unsigned char *base_in_hdmi[4];
	unsigned char *base_in_i2s;
	unsigned from_LineIN;
	unsigned from_MIC_IN;

	/* for output use */
	unsigned char *base_out_i2s_spdif;
	unsigned num_hdmi_audio_buffer; /* 0: no hdmi audio output (could save bandwidth) */
	unsigned is_dac_spdif_same_buffer;
	unsigned char *base_hdmi[4];
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
void I2S_DA32_SET_WP(unsigned WP32);

/* AD */
unsigned I2S_AD32_GET_RP(void);
unsigned I2S_AD32_GET_WP(void);
void I2S_AD32_SET_RP(unsigned RP32);

/********************** export APIs **********************/
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

#endif //MMP_I2S_H


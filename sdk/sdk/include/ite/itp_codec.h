/* sy.chuang, 2012-0702, ITE Tech. */

#ifndef ITP_CODEC_H
#define ITP_CODEC_H

#ifdef __cplusplus
extern "C" {
#endif

/* === common DAC/ADC ops === */
void itp_codec_wake_up(void);
void itp_codec_standby(void);

/* DAC */
void itp_codec_playback_init(unsigned output);
void itp_codec_playback_deinit(void);

void itp_codec_playback_amp_volume_down(void);
void itp_codec_playback_amp_volume_up(void);

void itp_codec_playback_set_direct_vol(unsigned target_vol);
void itp_codec_playback_set_direct_volperc(unsigned target_volperc);
void itp_codec_playback_get_currvol(unsigned *currvol);
void itp_codec_playback_get_currvolperc(unsigned *currvolperc);
void itp_codec_playback_get_vol_range(unsigned *max, unsigned *regular_0db, unsigned *min);

void itp_codec_playback_mute(void);
void itp_codec_playback_unmute(void);

void itp_codec_playback_linein_bypass(unsigned bypass);

/* ADC */
void itp_codec_rec_init(unsigned input_source);
void itp_codec_rec_deinit(void);

void itp_codec_rec_set_direct_vol(unsigned target_vol);
void itp_codec_rec_get_currvol(unsigned *currvol);
void itp_codec_rec_get_vol_range(unsigned *max, unsigned *regular_0db, unsigned *min);

void itp_codec_rec_mute(void);
void itp_codec_rec_unmute(void);

/* Sample Rate */
void itp_codec_get_i2s_sample_rate(int* samplerate);
void itp_codec_set_i2s_sample_rate(int samplerate);

#ifdef __cplusplus
}
#endif

#endif //ITP_CODEC_H

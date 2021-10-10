#include "mediastreamer2/msfilter.h"

extern MSFilterDesc ms_alaw_dec_desc;
extern MSFilterDesc ms_alaw_enc_desc;
extern MSFilterDesc ms_ulaw_dec_desc;
extern MSFilterDesc ms_ulaw_enc_desc;
extern MSFilterDesc ms_file_player_desc;
extern MSFilterDesc ms_rtp_send_desc;
extern MSFilterDesc ms_rtp_recv_desc;
#ifdef CFG_BUILD_LEAF
extern MSFilterDesc ms_udp_send_desc;
extern MSFilterDesc ms_udp_recv_desc;
#endif
extern MSFilterDesc ms_dtmf_gen_desc;
extern MSFilterDesc ms_file_rec_desc;
extern MSFilterDesc ms_speex_dec_desc;
extern MSFilterDesc ms_speex_enc_desc;
extern MSFilterDesc ms_speex_ec_desc;
extern MSFilterDesc ms_conf_desc;
extern MSFilterDesc ms_h263_dec_desc;
extern MSFilterDesc ms_h263_old_dec_desc;
extern MSFilterDesc ms_mpeg4_dec_desc;
extern MSFilterDesc ms_h264_dec_desc;
extern MSFilterDesc ms_snow_dec_desc;
extern MSFilterDesc ms_mjpeg_dec_desc;
extern MSFilterDesc ms_size_conv_desc;
extern MSFilterDesc ms_pix_conv_desc;
extern MSFilterDesc ms_resample_desc;
extern MSFilterDesc ms_volume_desc;
extern MSFilterDesc ms_static_image_desc;
extern MSFilterDesc ms_mire_desc;
extern MSFilterDesc ms_equalizer_desc;
extern MSFilterDesc ms_dd_display_desc;
extern MSFilterDesc ms_audio_mixer_desc;
extern MSFilterDesc ms_ext_display_desc;
extern MSFilterDesc ms_jpeg_writer_desc;
extern MSFilterDesc ms_tone_detector_desc;
extern MSFilterDesc ms_l16_enc_desc;
extern MSFilterDesc ms_l16_dec_desc;
extern MSFilterDesc ms_g722_enc_desc;
extern MSFilterDesc ms_g722_dec_desc;
extern MSFilterDesc ms_channel_adapter_desc;
extern MSFilterDesc oss_read_desc;
extern MSFilterDesc oss_write_desc;
extern MSFilterDesc ms_v4l2_desc;
extern MSFilterDesc ms_jpeg_dec_desc;
extern MSFilterDesc ms_mkv_player_desc;
extern MSFilterDesc ms_mkv_recorder_desc;
#if defined(CFG_SENSOR_ENABLE) && !defined(CFG_FFMPEG_H264_SW)
extern MSFilterDesc ms_h264_enc_desc; /* Fake h264 encoder */
#endif
extern MSFilterDesc ms_x11video_desc;
extern MSFilterDesc ms_castor3_display_desc;
#if 0//defined(TS_CAM_ENABLE) && !defined(CFG_FFMPEG_H264_SW)
    extern MSFilterDesc ms_es_dec_desc;
#endif
extern MSFilterDesc ms_sbc_aec_desc;
extern MSFilterDesc ms_itc_ipcam_desc;
extern MSFilterDesc ms_mix_voice_desc;
extern MSFilterDesc ms_adpcm_dec_desc;
extern MSFilterDesc ms_adpcm_enc_desc;
extern MSFilterDesc ms_offset8bit_dec_desc;

MSFilterDesc * ms_voip_filter_descs[]={
&ms_alaw_dec_desc,
&ms_alaw_enc_desc,
&ms_ulaw_dec_desc,
&ms_ulaw_enc_desc,
&ms_file_player_desc,
&ms_rtp_send_desc,
&ms_rtp_recv_desc,
#ifdef CFG_BUILD_LEAF
&ms_udp_send_desc,
&ms_udp_recv_desc,
#endif
&ms_dtmf_gen_desc,
&ms_file_rec_desc,
&ms_speex_dec_desc,
&ms_speex_enc_desc,
&ms_speex_ec_desc,
&ms_conf_desc,
&ms_h263_old_dec_desc,
&ms_h263_dec_desc,
&ms_mpeg4_dec_desc,
&ms_h264_dec_desc,
&ms_snow_dec_desc,
&ms_mjpeg_dec_desc,
&ms_size_conv_desc,
&ms_pix_conv_desc,
&ms_resample_desc,
&ms_volume_desc,
&ms_static_image_desc,
&ms_equalizer_desc,
&ms_audio_mixer_desc,
//&ms_ext_display_desc,
&ms_tone_detector_desc,
&ms_jpeg_writer_desc,
&ms_l16_enc_desc,
&ms_l16_dec_desc,
&ms_g722_enc_desc,
&ms_g722_dec_desc,
&ms_channel_adapter_desc,
//&oss_read_desc,
//&oss_write_desc,
//&ms_v4l2_desc,
&ms_mkv_player_desc,
&ms_mkv_recorder_desc,
//#if defined(CFG_FFMPEG_H264_SW)
//&ms_h264_enc_desc,
//#endif
&ms_jpeg_dec_desc,
//&ms_x11video_desc,
//&ms_mire_desc,
#ifdef CFG_LCD_ENABLE
    &ms_castor3_display_desc,
#endif
#if 0//defined(TS_CAM_ENABLE) && !defined(CFG_FFMPEG_H264_SW)
    &ms_es_dec_desc,
#endif
&ms_sbc_aec_desc,
&ms_itc_ipcam_desc,
&ms_mix_voice_desc,
&ms_adpcm_dec_desc,
&ms_adpcm_enc_desc,
&ms_offset8bit_dec_desc,
NULL
};


/* sy.chuang, 2012-0423, ITE Tech. */

#ifndef I2S_REG_H
#define I2S_REG_H

#ifdef __cplusplus
extern "C" {
#endif

/* ************************************************************************** */
#define I2S_REG_BASE            0x1600

#define I2S_REG_OBUF_1_LO       (I2S_REG_BASE | 0x00)
#define I2S_REG_OBUF_1_HI       (I2S_REG_BASE | 0x02)

#define I2S_REG_OBUF_2_LO       (I2S_REG_BASE | 0x04)
#define I2S_REG_OBUF_2_HI       (I2S_REG_BASE | 0x06)

#define I2S_REG_OBUF_3_LO       (I2S_REG_BASE | 0x08)
#define I2S_REG_OBUF_3_HI       (I2S_REG_BASE | 0x0A)

#define I2S_REG_OBUF_4_LO       (I2S_REG_BASE | 0x0C)
#define I2S_REG_OBUF_4_HI       (I2S_REG_BASE | 0x0E)

#define I2S_REG_OBUF_5_LO       (I2S_REG_BASE | 0x10)
#define I2S_REG_OBUF_5_HI       (I2S_REG_BASE | 0x12)

#define I2S_REG_OBUF_6_LO       (I2S_REG_BASE | 0x20)
#define I2S_REG_OBUF_6_HI       (I2S_REG_BASE | 0x22)

#define I2S_REG_OBUF_LEN_LO     (I2S_REG_BASE | 0x14)
#define I2S_REG_OBUF_LEN_HI     (I2S_REG_BASE | 0x16)

#define I2S_REG_OBUF_WP_LO      (I2S_REG_BASE | 0x18)
#define I2S_REG_OBUF_WP_HI      (I2S_REG_BASE | 0x1A)

#define I2S_REG_OBUF_RP_LO      (I2S_REG_BASE | 0x1C)
#define I2S_REG_OBUF_RP_HI      (I2S_REG_BASE | 0x1E)

#define I2S_REG_ADC_SAMPLE      (I2S_REG_BASE | 0x40)
#define I2S_REG_DAC_SAMPLE      (I2S_REG_BASE | 0x42)

#define I2S_REG_ADDA_PCM        (I2S_REG_BASE | 0x44)

#define I2S_REG_IBUF_LO         (I2S_REG_BASE | 0x46)
#define I2S_REG_IBUF_HI         (I2S_REG_BASE | 0x48)

#define I2S_REG_IBUF_LEN        (I2S_REG_BASE | 0x4A)
#define I2S_REG_IBUF_RP         (I2S_REG_BASE | 0x4C)
#define I2S_REG_IBUF_WP         (I2S_REG_BASE | 0x5A)

#define I2S_REG_INPUT_CTL       (I2S_REG_BASE | 0x4E)
#define I2S_REG_INPUT_CTL2      (I2S_REG_BASE | 0x50)

#define I2S_REG_OUTPUT_GAP_LO	(I2S_REG_BASE | 0x52)
#define I2S_REG_OUTPUT_GAP_HI	(I2S_REG_BASE | 0x54)

#define I2S_REG_OUTPUT_CTL      (I2S_REG_BASE | 0x56)
#define I2S_REG_OUTPUT_CTL2     (I2S_REG_BASE | 0x58)

#define I2S_REG_INPUT_STATUS    (I2S_REG_BASE | 0x5C)
#define I2S_REG_OUTPUT_STATUS1  (I2S_REG_BASE | 0x5E)
#define I2S_REG_OUTPUT_STATUS2  (I2S_REG_BASE | 0x60)

#define I2S_REG_THOLD_CROSS_LO  (I2S_REG_BASE | 0x62)
#define I2S_REG_THOLD_CROSS_HI  (I2S_REG_BASE | 0x64)

#define I2S_REG_FADE_IN_OUT     (I2S_REG_BASE | 0x66)

#define I2S_REG_DIG_VOL         (I2S_REG_BASE | 0x68)
#define I2S_REG_SPDIF_VOL       (I2S_REG_BASE | 0x6C)

#define I2S_REG_BITS_CTL        (I2S_REG_BASE | 0x6E)
#define I2S_REG_DAC_CTL         (I2S_REG_BASE | 0x70)

#define I2S_REG_AMP_CTL         (I2S_REG_BASE | 0x72)
#define I2S_REG_AMP_VOL         (I2S_REG_BASE | 0x74)

#define I2S_REG_HDMI_SYNC_LEFT  (I2S_REG_BASE | 0x76)
#define I2S_REG_HDMI_SYNC_RIGHT (I2S_REG_BASE | 0x78)

#define I2S_REG_HDMI_CTL        (I2S_REG_BASE | 0x7A)

/* ************************************************************************** */
/* PCM read ptr at REG risc user defined */
//#define I2S_PCM_RDPTR           0x16AC

#define MMP_AUDIO_CLOCK_REG_3A  0x003A
#define MMP_AUDIO_CLOCK_REG_3C  0x003C
#define MMP_AUDIO_CLOCK_REG_3E  0x003E

/* ************************************************************************** */
#ifdef __cplusplus
}
#endif

#endif //I2S_REG_H


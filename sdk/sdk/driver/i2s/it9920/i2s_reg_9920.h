/* sy.chuang, 2012-0423, ITE Tech. */

#ifndef I2S_REG_H
#define I2S_REG_H

#ifdef __cplusplus
extern "C" {
#endif

/* ************************************************************************** */
#define I2S_REG_BASE                0xD0100000

#define I2S_REG_CODEC_SET           (I2S_REG_BASE | 0x00)

#define I2S_REG_ADC_SRATE_SET       (I2S_REG_BASE | 0x04)
#define I2S_REG_DAC_SRATE_SET       (I2S_REG_BASE | 0x08)

#define I2S_REG_DATA_LOOPBACK       (I2S_REG_BASE | 0x0C)

#define I2S_REG_IN1_BASE1           (I2S_REG_BASE | 0x10)
#define I2S_REG_IN2_BASE1           (I2S_REG_BASE | 0x20)
#define I2S_REG_IN2_BASE2           (I2S_REG_BASE | 0x24)
#define I2S_REG_IN2_BASE3           (I2S_REG_BASE | 0x28)
#define I2S_REG_IN2_BASE4           (I2S_REG_BASE | 0x2C)
#define I2S_REG_IN_LEN              (I2S_REG_BASE | 0x30)
#define I2S_REG_IN_RdWrGAP          (I2S_REG_BASE | 0x34)
#define I2S_REG_IN_VOLUME           (I2S_REG_BASE | 0x38)
#define I2S_REG_IN_CTRL		        (I2S_REG_BASE | 0x40)


#define I2S_REG_IN1_STATUS          (I2S_REG_BASE | 0x44)
#define I2S_REG_IN2_STATUS          (I2S_REG_BASE | 0x48)
#define I2S_REG_IN1_RDPTR           (I2S_REG_BASE | 0x50)
#define I2S_REG_IN1_WRPTR           (I2S_REG_BASE | 0x54)
#define I2S_REG_IN2_RDPTR           (I2S_REG_BASE | 0x58)
#define I2S_REG_IN2_WRPTR           (I2S_REG_BASE | 0x5C)
#define I2S_REG_CODEC_PCM_SET       (I2S_REG_BASE | 0x60)


#define I2S_REG_OUT_BASE1           (I2S_REG_BASE | 0x70)
#define I2S_REG_OUT_BASE2           (I2S_REG_BASE | 0x74)
#define I2S_REG_OUT_BASE3           (I2S_REG_BASE | 0x78)
#define I2S_REG_OUT_BASE4           (I2S_REG_BASE | 0x7C)
#define I2S_REG_OUT_BASE5           (I2S_REG_BASE | 0x80)
#define I2S_REG_OUT_LEN             (I2S_REG_BASE | 0x90)
#define I2S_REG_OUT_RdWrGAP         (I2S_REG_BASE | 0x94)
#define I2S_REG_OUT_BIST            (I2S_REG_BASE | 0x98)


//
#define I2S_REG_IN_CTRL             (I2S_REG_BASE | 0x40) 
#define I2S_REG_OUT_CTRL            (I2S_REG_BASE | 0xA0)
//

#define I2S_REG_OUT_CROSSTH         (I2S_REG_BASE | 0xA4)
#define I2S_REG_OUT_FADE_SET        (I2S_REG_BASE | 0xA8)
#define I2S_REG_OUT_VOLUME          (I2S_REG_BASE | 0xAC)
#define I2S_REG_SPDIF_VOLUME        (I2S_REG_BASE | 0xB0)
#define I2S_REG_OUT_STATUS1         (I2S_REG_BASE | 0xB8)
#define I2S_REG_OUT_STATUS2         (I2S_REG_BASE | 0xBC)
#define I2S_REG_OUT_WRPTR           (I2S_REG_BASE | 0xC0)
#define I2S_REG_OUT_RDPTR           (I2S_REG_BASE | 0xC4)



// IP
#define IP_REG_SYSTEM_CONTROL       (I2S_REG_BASE | 0xD0)
#define IP_REG_AUDIO_CONTROL_1      (I2S_REG_BASE | 0xD4)
#define IP_REG_ALC_CONTROL_1        (I2S_REG_BASE | 0xD8)
#define IP_REG_ALC_CONTROL_2        (I2S_REG_BASE | 0xDC)
#define IP_REG_DAC_HV_SPV_CONTROL   (I2S_REG_BASE | 0xE0)
#define IP_REG_ADC_CONTROL          (I2S_REG_BASE | 0xE4)
#define IP_REG_DAC_DV_CONTROL       (I2S_REG_BASE | 0xE8)
#define IP_REG_AUDIO_CONTROL_2      (I2S_REG_BASE | 0xEC)
#define IP_REG_SOFTMUTE_CONTROL_1   (I2S_REG_BASE | 0xF0)
#define IP_REG_SOFTMUTE_CONTROL_2   (I2S_REG_BASE | 0xF4)
//

/* ************************************************************************** */

#define MMP_AUDIO_CLOCK_REG_3C  0xD800003C
#define MMP_AUDIO_CLOCK_REG_40  0xD8000040

/* ************************************************************************** */
typedef signed   long long s64;
typedef signed   int       s32;
typedef signed   short     s16;
typedef signed   char      s8;
typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned short     u16;
typedef unsigned char      u8;

#ifdef __cplusplus
}
#endif

#endif //I2S_REG_H


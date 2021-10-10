///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <hdmitx_drv.h>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2011/07/19
//   @fileversion: ITE_HDMITX_SAMPLE_2.03
//******************************************/
#include "hdmitx/debug.h"
#include "hdmitx/typedef.h"

#ifndef _HDMITX_DRV_H_
#define _HDMITX_DRV_H_

//#define EXTERN_HDCPROM
/////////////////////////////////////////
// DDC Address
/////////////////////////////////////////
#define DDC_HDCP_ADDRESS 0x74
#define DDC_EDID_ADDRESS 0xA0
#define DDC_FIFO_MAXREQ 0x20

// I2C address

#define _80MHz 80000000
#define HDMI_TX_I2C_SLAVE_ADDR 0x98

///////////////////////////////////////////////////////////////////////
// Register offset
///////////////////////////////////////////////////////////////////////


#define REG_TX_VENDOR_ID0   0x00
#define REG_TX_VENDOR_ID1   0x01
#define REG_TX_DEVICE_ID0   0x02
#define REG_TX_DEVICE_ID1   0x03

    #define O_DEVID 0
    #define M_DEVID 0xF
    #define O_REVID 4
    #define M_REVID 0xF

#define REG_TX_SW_RST       0x04
    #define B_ENTEST    (1<<7)
    #define B_REF_RST_HDMITX (1<<5)
    #define B_AREF_RST (1<<4)
    #define B_VID_RST_HDMITX (1<<3)
    #define B_AUD_RST_HDMITX (1<<2)
    #define B_TX_HDMI_RST (1<<1)
    #define B_HDCP_RST_HDMITX (1<<0)

#define REG_TX_INT_CTRL 0x05
    #define B_INTPOL_ACTL 0
    #define B_INTPOL_ACTH (1<<7)
    #define B_INT_PUSHPULL 0
    #define B_INT_OPENDRAIN (1<<6)

#define REG_TX_INT_STAT1    0x06
    #define B_INT_AUD_OVERFLOW  (1<<7)
    #define B_INT_ROMACQ_NOACK  (1<<6)
    #define B_INT_RDDC_NOACK    (1<<5)
    #define B_INT_DDCFIFO_ERR   (1<<4)
    #define B_INT_ROMACQ_BUS_HANG   (1<<3)
    #define B_INT_DDC_BUS_HANG  (1<<2)
    #define B_INT_RX_SENSE  (1<<1)
    #define B_INT_HPD_PLUG  (1<<0)

#define REG_TX_INT_STAT2    0x07
    #define B_INT_HDCP_SYNC_DET_FAIL  (1<<7)
    #define B_INT_VID_UNSTABLE  (1<<6)
    #define B_INT_PKTACP    (1<<5)
    #define B_INT_PKTNULL  (1<<4)
    #define B_INT_PKTGENERAL   (1<<3)
    #define B_INT_KSVLIST_CHK   (1<<2)
    #define B_INT_AUTH_DONE (1<<1)
    #define B_INT_AUTH_FAIL (1<<0)

#define REG_TX_INT_STAT3    0x08
    #define B_INT_AUD_CTS   (1<<6)
    #define B_INT_VSYNC     (1<<5)
    #define B_INT_VIDSTABLE (1<<4)
    #define B_INT_PKTMPG    (1<<3)
    #define B_INT_PKTSPD    (1<<2)
    #define B_INT_PKTAUD    (1<<1)
    #define B_INT_PKTAVI    (1<<0)

#define REG_TX_INT_MASK1    0x09
    #define B_AUDIO_OVFLW_MASK (1<<7)
    #define B_DDC_NOACK_MASK (1<<5)
    #define B_DDC_FIFO_ERR_MASK (1<<4)
    #define B_DDC_BUS_HANG_MASK (1<<2)
    #define B_RXSEN_MASK (1<<1)
    #define B_HPD_MASK (1<<0)

#define REG_TX_INT_MASK2    0x0A
    #define B_PKT_AVI_MASK (1<<7)
    #define B_PKT_VID_UNSTABLE_MASK (1<<6)
    #define B_PKT_ACP_MASK (1<<5)
    #define B_PKT_NULL_MASK (1<<4)
    #define B_PKT_GEN_MASK (1<<3)
    #define B_KSVLISTCHK_MASK (1<<2)
    #define B_T_AUTH_DONE_MASK (1<<1)
    #define B_AUTH_FAIL_MASK (1<<0)

#define REG_TX_INT_MASK3    0x0B
    #define B_HDCP_SYNC_DET_FAIL_MASK (1<<6)
    #define B_AUDCTS_MASK (1<<5)
    #define B_VSYNC_MASK (1<<4)
    #define B_VIDSTABLE_MASK (1<<3)
    #define B_PKT_MPG_MASK (1<<2)
    #define B_PKT_SPD_MASK (1<<1)
    #define B_PKT_AUD_MASK (1<<0)


#define REG_TX_INT_CLR0      0x0C
    #define B_CLR_PKTACP    (1<<7)
    #define B_CLR_PKTNULL   (1<<6)
    #define B_CLR_PKTGENERAL    (1<<5)
    #define B_CLR_KSVLISTCHK    (1<<4)
    #define B_CLR_AUTH_DONE  (1<<3)
    #define B_CLR_AUTH_FAIL  (1<<2)
    #define B_CLR_RXSENSE   (1<<1)
    #define B_CLR_HPD       (1<<0)

#define REG_TX_INT_CLR1       0x0D
    #define B_CLR_VSYNC (1<<7)
    #define B_CLR_VIDSTABLE (1<<6)
    #define B_CLR_PKTMPG    (1<<5)
    #define B_CLR_PKTSPD    (1<<4)
    #define B_CLR_PKTAUD    (1<<3)
    #define B_CLR_PKTAVI    (1<<2)
    #define B_CLR_HDCP_SYNC_DET_FAIL  (1<<1)
    #define B_CLR_VID_UNSTABLE        (1<<0)

#define REG_TX_SYS_STATUS     0x0E
    // readonly
    #define B_INT_ACTIVE    (1<<7)
    #define B_HPDETECT      (1<<6)
    #define B_RXSENDETECT   (1<<5)
    #define B_TXVIDSTABLE   (1<<4)
    // read/write
    #define O_CTSINTSTEP    2
    #define M_CTSINTSTEP    (3<<2)
    #define B_CLR_AUD_CTS     (1<<1)
    #define B_INTACTDONE    (1<<0)

#define REG_TX_BANK_CTRL        0x0F
    #define B_BANK0 0
    #define B_BANK1 1

// DDC

#define REG_TX_DDC_MASTER_CTRL   0x10
    #define B_MASTERROM (1<<1)
    #define B_MASTERDDC (0<<1)
    #define B_MASTERHOST    (1<<0)
    #define B_MASTERHDCP    (0<<0)

#define REG_TX_DDC_HEADER  0x11
#define REG_TX_DDC_REQOFF  0x12
#define REG_TX_DDC_REQCOUNT    0x13
#define REG_TX_DDC_EDIDSEG 0x14
#define REG_TX_DDC_CMD 0x15
    #define CMD_DDC_SEQ_BURSTREAD 0
    #define CMD_LINK_CHKREAD  2
    #define CMD_EDID_READ   3
    #define CMD_FIFO_CLR    9
    #define CMD_GEN_SCLCLK  0xA
    #define CMD_DDC_ABORT   0xF

#define REG_TX_DDC_STATUS  0x16
    #define B_DDC_DONE  (1<<7)
    #define B_DDC_ACT   (1<<6)
    #define B_DDC_NOACK (1<<5)
    #define B_DDC_WAITBUS   (1<<4)
    #define B_DDC_ARBILOSE  (1<<3)
    #define B_DDC_ERROR     (B_DDC_NOACK|B_DDC_WAITBUS|B_DDC_ARBILOSE)
    #define B_DDC_FIFOFULL  (1<<2)
    #define B_DDC_FIFOEMPTY (1<<1)

#define REG_TX_DDC_READFIFO    0x17
#define REG_TX_ROM_STARTADDR   0x18
#define REG_TX_HDCP_HEADER 0x19
#define REG_TX_ROM_HEADER  0x1A
#define REG_TX_BUSHOLD_T   0x1B
#define REG_TX_ROM_STAT    0x1C
    #define B_ROM_DONE  (1<<7)
    #define B_ROM_ACTIVE    (1<<6)
    #define B_ROM_NOACK (1<<5)
    #define B_ROM_WAITBUS   (1<<4)
    #define B_ROM_ARBILOSE  (1<<3)
    #define B_ROM_BUSHANG   (1<<2)

// HDCP
#define REG_TX_AN_GENERATE 0x1F
    #define B_START_CIPHER_GEN  1
    #define B_STOP_CIPHER_GEN   0

#define REG_TX_HDCP_DESIRE 0x20
    #define B_ENABLE_HDPC11 (1<<1)
    #define B_CPDESIRE  (1<<0)

#define REG_TX_AUTHFIRE    0x21
#define REG_TX_LISTCTRL    0x22
    #define B_LISTFAIL  (1<<1)
    #define B_LISTDONE  (1<<0)

#define REG_TX_AKSV    0x23
#define REG_TX_AKSV0   0x23
#define REG_TX_AKSV1   0x24
#define REG_TX_AKSV2   0x25
#define REG_TX_AKSV3   0x26
#define REG_TX_AKSV4   0x27

#define REG_TX_AN  0x28
#define REG_TX_AN_GEN  0x30
#define REG_TX_ARI     0x38
#define REG_TX_ARI0    0x38
#define REG_TX_ARI1    0x39
#define REG_TX_APJ     0x3A

#define REG_TX_BKSV    0x3B
#define REG_TX_BRI     0x40
#define REG_TX_BRI0    0x40
#define REG_TX_BRI1    0x41
#define REG_TX_BPJ     0x42
#define REG_TX_BCAP    0x43
    #define B_CAP_HDMI_REPEATER (1<<6)
    #define B_CAP_KSV_FIFO_RDY  (1<<5)
    #define B_CAP_HDMI_FAST_MODE    (1<<4)
    #define B_CAP_HDCP_1p1  (1<<1)
    #define B_CAP_FAST_REAUTH   (1<<0)
#define REG_TX_BSTAT   0x44
#define REG_TX_BSTAT0   0x44
#define REG_TX_BSTAT1   0x45
    #define B_CAP_HDMI_MODE (1<<12)
    #define B_CAP_DVI_MODE (0<<12)
    #define B_MAX_CASCADE_EXCEEDED  (1<<11)
    #define M_REPEATER_DEPTH    (0x7<<8)
    #define O_REPEATER_DEPTH    8
    #define B_DOWNSTREAM_OVER   (1<<7)
    #define M_DOWNSTREAM_COUNT  0x7F

#define REG_TX_AUTH_STAT 0x46
#define B_T_AUTH_DONE (1<<7)
#define REG_TX_CLK_CTRL0 0x58
    #define O_OSCLK_SEL 5
    #define M_OSCLK_SEL 3
    #define B_AUTO_OVER_SAMPLING_CLOCK (1<<4)
    #define O_EXT_MCLK_SEL  2
    #define M_EXT_MCLK_SEL  (3<<O_EXT_MCLK_SEL)
    #define B_EXT_128FS (0<<O_EXT_MCLK_SEL)
    #define B_EXT_256FS (1<<O_EXT_MCLK_SEL)
    #define B_EXT_512FS (2<<O_EXT_MCLK_SEL)
    #define B_EXT_1024FS (3<<O_EXT_MCLK_SEL)

#define REG_TX_SHA_SEL       0x50
#define REG_TX_SHA_RD_BYTE1  0x51
#define REG_TX_SHA_RD_BYTE2  0x52
#define REG_TX_SHA_RD_BYTE3  0x53
#define REG_TX_SHA_RD_BYTE4  0x54
#define REG_TX_AKSV_RD_BYTE5 0x55


#define REG_TX_CLK_CTRL1 0x59
    #define B_EN_TXCLK_COUNT    (1<<5)
    #define B_VDO_LATCH_EDGE    (1<<3)
    #define M_AUD_DIV           3
    #define B_AUD_NODIV         0
    #define B_AUD_DIV2          1
    #define B_AUD_DIV4          3
    #define B_AUD_NODEF         2
#define REG_TX_CLK_STATUS1 0x5E
#define REG_TX_CLK_STATUS2 0x5F
    #define B_IP_LOCK (1<<7)
    #define B_XP_LOCK (1<<6)
    #define B_OSF_LOCK (1<<5)


#define REG_TX_AFE_DRV_CTRL 0x61

    #define B_AFE_DRV_PWD    (1<<5)
    #define B_AFE_DRV_RST    (1<<4)
    #define B_AFE_DRV_PDRXDET    (1<<2)
    #define B_AFE_DRV_TERMON    (1<<1)
    #define B_AFE_DRV_ENCAL    (1<<0)

#define REG_TX_AFE_XP_CTRL 0x62
    #define B_AFE_XP_GAINBIT    (1<<7)
    #define B_AFE_XP_PWDPLL    (1<<6)
    #define B_AFE_XP_ENI    (1<<5)
    #define B_AFE_XP_ER0    (1<<4)
    #define B_AFE_XP_RESETB    (1<<3)
    #define B_AFE_XP_PWDI    (1<<2)
    #define B_AFE_XP_DEI    (1<<1)
    #define B_AFE_XP_DER    (1<<0)

#define REG_TX_AFE_ISW_CTRL  0x63
    #define B_AFE_RTERM_SEL  (1<<7)
    #define B_AFE_IP_BYPASS  (1<<6)
    #define M_AFE_DRV_ISW    (7<<3)
    #define O_AFE_DRV_ISW    3
    #define B_AFE_DRV_ISWK   7

#define REG_TX_AFE_IP_CTRL 0x64

    #define B_AFE_IP_GAINBIT    (1<<7)
    #define B_AFE_IP_PWDPLL    (1<<6)
    #define M_AFE_IP_CKSEL    (3<<4)
    #define O_AFE_IP_CKSEL    4
    #define B_AFE_IP_ER0    (1<<3)
    #define B_AFE_IP_RESETB    (1<<2)
    #define B_AFE_IP_ENC    (1<<1)
    #define B_AFE_IP_EC1    (1<<0)

#define REG_TX_AFE_RING    0x65
    #define B_AFE_CAL_UPDATE    (1<<7)
    #define B_AFE_CAL_MANUAL    (1<<6)
    #define M_AFE_CAL_CLK_MODE    (3<<4)
    #define O_AFE_CAL_CLK_MODE    4
    #define O_AFE_DRV_VSW   2
    #define M_AFE_DRV_VSW   (3<<2)
    #define B_AFE_RING_SLOW    (1<<1)
    #define B_AFE_RING_FAST    (1<<0)
#define REG_TX_AFE_TEST    0x66
    #define B_AFE_AFE_ENTEST    (1<<6)
    #define B_AFE_AFE_ENBIST    (1<<5)
    #define M_AFE_CAL_RTERM_MANUAL    0x1F
#define REG_TX_AFE_LFSR     0x67
    #define B_AFE_AFELFSR_VAL   (1<<7)
    #define B_AFE_DIS_AFELFSR   (1<<6)
    #define M_AFE_RTERM_VAOUE    0xF

//
//#define REG_TX_AFE_DRV_CTRL    0x61
//    #define M_AFE_DRV_SR (3<<2)
//    #define O_AFE_DRV_SR 2
//  #define B_AFE_DRV_RST (1<<4)
//  #define B_AFE_DRV_PWD (1<<5)
//  #define B_AFE_DRV_ENBIST (1<<6)
//
//#define REG_TX_AFE_XP_CTRL1 0x62
//  #define B_AFE_XP_GAINBIT (1<<7)
//  #define B_AFE_XP_PWDPLL (1<<6)
//  #define B_AFE_XP_ENI (1<<5)
//  #define B_AFE_XP_ER0 (1<<4)
//  #define B_AFE_XP_RESETB (1<<3)
//  #define B_AFE_XP_PWDI (1<<2)
//  #define B_AFE_XP_DEI (1<<1)
//  #define B_AFE_XP_BYPASS (1<<0)
//
//#define REG_TX_AFE_XP_CTRL2 0x63
//    #define B_XP_ENCLKX5    (1<<3)
//    #define M_XP_CLKSEL 3
//    #define B_XP_CLKSEL_HALF_PCLKHV 0
//    #define B_XP_CLKSEL_1_PCLKHV 1
//    #define B_XP_CLKSEL_2_PCLKHV 2
//    #define B_XP_CLKSEL_4_PCLKHV 3
//
//
//#define REG_TX_AFE_IP_CTRL 0x64
//  #define B_AFE_IP_GAINBIT (1<<6)
//  #define B_AFE_IP_PWDPLL (1<<5)
//  #define B_AFE_IP_SEDB (1<<4)
//  #define B_AFE_IP_ER0 (1<<3)
//  #define B_AFE_IP_RESETB (1<<2)
//  #define B_AFE_IP_PDIV1 (1<<1)
//  #define B_AFE_IP_ENCB (1<<0)
//
//#define REG_TX_AFE_RING    0x65
//  #define B_AFE_RING_FAST (1<<0)
//  #define B_AFE_RING_SLOW (1<<1)
//    #define M_AFE_DRV_VSW (3<<2)
//    #define B_AFE_VSW_NOENH 0
//    #define B_AFE_VSW_12ENH (1<<2)
//    #define B_AFE_VSW_24ENH (2<<2)
//    #define B_AFE_VSW_35ENH (3<<2)

// Input Data Format Register
#define REG_TX_INPUT_MODE  0x70
    #define O_INCLKDLY  0
    #define M_INCLKDLY  3
    #define B_INDDR     (1<<2)
    #define B_SYNCEMB   (1<<3)
    #define B_2X656CLK  (1<<4)
    #define B_PCLKDIV2  (1<<5)
    #define M_INCOLMOD  (3<<6)
    #define B_IN_RGB    0
    #define B_IN_YUV422 (1<<6)
    #define B_IN_YUV444 (2<<6)

#define REG_TX_TXFIFO_RST  0x71
    #define B_ENAVMUTERST   1
    #define B_TXFFRST   (1<<1)

#define REG_TX_CSC_CTRL    0x72
    #define B_HDMITX_CSC_BYPASS    0
    #define B_HDMITX_CSC_RGB2YUV   2
    #define B_HDMITX_CSC_YUV2RGB   3
    #define M_CSC_SEL       3
    #define B_TX_EN_DITHER      (1<<7)
    #define B_TX_EN_UDFILTER    (1<<6)
    #define B_TX_DNFREE_GO      (1<<5)

#define REG_TX_CSC_YOFF 0x73
#define REG_TX_CSC_COFF 0x74
#define REG_TX_CSC_RGBOFF 0x75

#define REG_TX_CSC_MTX11_L 0x76
#define REG_TX_CSC_MTX11_H 0x77
#define REG_TX_CSC_MTX12_L 0x78
#define REG_TX_CSC_MTX12_H 0x79
#define REG_TX_CSC_MTX13_L 0x7A
#define REG_TX_CSC_MTX13_H 0x7B
#define REG_TX_CSC_MTX21_L 0x7C
#define REG_TX_CSC_MTX21_H 0x7D
#define REG_TX_CSC_MTX22_L 0x7E
#define REG_TX_CSC_MTX22_H 0x7F
#define REG_TX_CSC_MTX23_L 0x80
#define REG_TX_CSC_MTX23_H 0x81
#define REG_TX_CSC_MTX31_L 0x82
#define REG_TX_CSC_MTX31_H 0x83
#define REG_TX_CSC_MTX32_L 0x84
#define REG_TX_CSC_MTX32_H 0x85
#define REG_TX_CSC_MTX33_L 0x86
#define REG_TX_CSC_MTX33_H 0x87

#define REG_TX_CSC_GAIN1V_L 0x88
#define REG_TX_CSC_GAIN1V_H 0x89
#define REG_TX_CSC_GAIN2V_L 0x8A
#define REG_TX_CSC_GAIN2V_H 0x8B
#define REG_TX_CSC_GAIN3V_L 0x8C
#define REG_TX_CSC_GAIN3V_H 0x8D

#define REG_TX_HVPol 0x90
#define REG_TX_HfPixel 0x91
#define REG_TX_HSSL 0x95
#define REG_TX_HSEL 0x96
#define REG_TX_HSH 0x97
#define REG_TX_VSS1 0xA0
#define REG_TX_VSE1 0xA1
#define REG_TX_VSS2 0xA2
#define REG_TX_VSE2 0xA3

// HDMI General Control Registers

#define REG_TX_HDMI_MODE   0xC0
    #define B_TX_HDMI_MODE 1
    #define B_TX_DVI_MODE  0
#define REG_TX_AV_MUTE 0xC1
#define REG_TX_GCP     0xC1
    #define B_CLR_AVMUTE    0
    #define B_SET_AVMUTE    1
    #define B_TX_SETAVMUTE        (1<<0)
    #define B_BLUE_SCR_MUTE   (1<<1)
    #define B_NODEF_PHASE    (1<<2)
    #define B_PHASE_RESYNC   (1<<3)

    #define O_COLOR_DEPTH     4
    #define M_COLOR_DEPTH     7
    #define B_COLOR_DEPTH_MASK (M_COLOR_DEPTH<<O_COLOR_DEPTH)
    #define B_CD_NODEF  0
    #define B_CD_24     (4<<4)
    #define B_CD_30     (5<<4)
    #define B_CD_36     (6<<4)
    #define B_CD_48     (7<<4)

#define REG_TX_OESS_CYCLE  0xC3
#define REG_TX_ENCRYPTION  0xC4
    #define B_DISABLE_ENCRYPTION    1
    #define B_ENABLE_ENCRYPTION 0
#define REG_TX_PKT_SINGLE_CTRL 0xC5
    #define B_SINGLE_PKT    1
    #define B_BURST_PKT
    #define B_SW_CTS    (1<<1)
#define REG_TX_PKT_GENERAL_CTRL    0xC6

#define REG_TX_NULL_CTRL 0xC9
#define REG_TX_ACP_CTRL 0xCA
#define REG_TX_ISRC1_CTRL 0xCB
#define REG_TX_ISRC2_CTRL 0xCC
#define REG_TX_AVI_INFOFRM_CTRL 0xCD
#define REG_TX_AUD_INFOFRM_CTRL 0xCE
#define REG_TX_SPD_INFOFRM_CTRL 0xCF
#define REG_TX_MPG_INFOFRM_CTRL 0xD0
    #define B_ENABLE_PKT    1
    #define B_REPEAT_PKT    (1<<1)


// Audio Channel Control
#define REG_TX_AUDIO_CTRL0 0xE0
    #define M_AUD_SWL (3<<6)
    #define M_AUD_16BIT (0<<6)
    #define M_AUD_18BIT (1<<6)
    #define M_AUD_20BIT (2<<6)
    #define M_AUD_24BIT (3<<6)

    #define B_SPDIFTC (1<<5)

    #define B_AUD_SPDIF (1<<4)
    #define B_AUD_I2S (0<<4)
    #define B_AUD_EN_I2S3   (1<<3)
    #define B_AUD_EN_I2S2   (1<<2)
    #define B_AUD_EN_I2S1   (1<<1)
    #define B_AUD_EN_I2S0   (1<<0)
    #define B_AUD_EN_SPDIF  1

#define REG_TX_AUDIO_CTRL1 0xE1
    #define B_AUD_FULLPKT (1<<6)

    #define B_AUDFMT_STD_I2S (0<<0)
    #define B_AUDFMT_32BIT_I2S (1<<0)
    #define B_AUDFMT_LEFT_JUSTIFY (0<<1)
    #define B_AUDFMT_RIGHT_JUSTIFY (1<<1)
    #define B_AUDFMT_DELAY_1T_TO_WS (0<<2)
    #define B_AUDFMT_NO_DELAY_TO_WS (1<<2)
    #define B_AUDFMT_WS0_LEFT   (0<<3)
    #define B_AUDFMT_WS0_RIGHT   (1<<3)
    #define B_AUDFMT_MSB_SHIFT_FIRST (0<<4)
    #define B_AUDFMT_LSB_SHIFT_FIRST (1<<4)
    #define B_AUDFMT_RISE_EDGE_SAMPLE_WS (0<<5)
    #define B_AUDFMT_FALL_EDGE_SAMPLE_WS (0<<5)

#define REG_TX_AUDIO_FIFOMAP 0xE2
    #define O_FIFO3SEL 6
    #define O_FIFO2SEL 4
    #define O_FIFO1SEL 2
    #define O_FIFO0SEL 0
    #define B_SELSRC3  3
    #define B_SELSRC2  2
    #define B_SELSRC1  1
    #define B_SELSRC0  0

#define REG_TX_AUDIO_CTRL3 0xE3
    #define B_AUD_MULCH (1<<7)
    #define B_EN_ZERO_CTS (1<<6)
    #define B_CHSTSEL (1<<4)
    #define B_S3RLCHG (1<<3)
    #define B_S2RLCHG (1<<2)
    #define B_S1RLCHG (1<<1)
    #define B_S0RLCHG (1<<0)

#define REG_TX_AUD_SRCVALID_FLAT 0xE4
    #define B_AUD_SPXFLAT_SRC3 (1<<7)
    #define B_AUD_SPXFLAT_SRC2 (1<<6)
    #define B_AUD_SPXFLAT_SRC1 (1<<5)
    #define B_AUD_SPXFLAT_SRC0 (1<<4)
    #define B_AUD_ERR2FLAT (1<<3)
    #define B_AUD_S3VALID (1<<2)
    #define B_AUD_S2VALID (1<<1)
    #define B_AUD_S1VALID (1<<0)

#define REG_TX_AUD_HDAUDIO 0xE5
#define B_HBR   (1<<3)
#define B_DSD   (1<<1)

//////////////////////////////////////////
// Bank 1
//////////////////////////////////////////

#define REGPktAudCTS0 0x30  // 7:0
#define REGPktAudCTS1 0x31  // 15:8
#define REGPktAudCTS2 0x32  // 19:16
#define REGPktAudN0 0x33    // 7:0
#define REGPktAudN1 0x34    // 15:8
#define REGPktAudN2 0x35    // 19:16
#define REGPktAudCTSCnt0 0x35   // 3:0
#define REGPktAudCTSCnt1 0x36   // 11:4
#define REGPktAudCTSCnt2 0x37   // 19:12

//////////////////////////////////////////
// COMMON PACKET for NULL,ISRC1,ISRC2,SPD
//////////////////////////////////////////

#define REG_TX_PKT_HB00 0x38
#define REG_TX_PKT_HB01 0x39
#define REG_TX_PKT_HB02 0x3A

#define REG_TX_PKT_PB00 0x3B
#define REG_TX_PKT_PB01 0x3C
#define REG_TX_PKT_PB02 0x3D
#define REG_TX_PKT_PB03 0x3E
#define REG_TX_PKT_PB04 0x3F
#define REG_TX_PKT_PB05 0x40
#define REG_TX_PKT_PB06 0x41
#define REG_TX_PKT_PB07 0x42
#define REG_TX_PKT_PB08 0x43
#define REG_TX_PKT_PB09 0x44
#define REG_TX_PKT_PB10 0x45
#define REG_TX_PKT_PB11 0x46
#define REG_TX_PKT_PB12 0x47
#define REG_TX_PKT_PB13 0x48
#define REG_TX_PKT_PB14 0x49
#define REG_TX_PKT_PB15 0x4A
#define REG_TX_PKT_PB16 0x4B
#define REG_TX_PKT_PB17 0x4C
#define REG_TX_PKT_PB18 0x4D
#define REG_TX_PKT_PB19 0x4E
#define REG_TX_PKT_PB20 0x4F
#define REG_TX_PKT_PB21 0x50
#define REG_TX_PKT_PB22 0x51
#define REG_TX_PKT_PB23 0x52
#define REG_TX_PKT_PB24 0x53
#define REG_TX_PKT_PB25 0x54
#define REG_TX_PKT_PB26 0x55
#define REG_TX_PKT_PB27 0x56

#define REG_TX_AVIINFO_DB1 0x58
#define REG_TX_AVIINFO_DB2 0x59
#define REG_TX_AVIINFO_DB3 0x5A
#define REG_TX_AVIINFO_DB4 0x5B
#define REG_TX_AVIINFO_DB5 0x5C
#define REG_TX_AVIINFO_DB6 0x5E
#define REG_TX_AVIINFO_DB7 0x5F
#define REG_TX_AVIINFO_DB8 0x60
#define REG_TX_AVIINFO_DB9 0x61
#define REG_TX_AVIINFO_DB10 0x62
#define REG_TX_AVIINFO_DB11 0x63
#define REG_TX_AVIINFO_DB12 0x64
#define REG_TX_AVIINFO_DB13 0x65
#define REG_TX_AVIINFO_SUM 0x5D

#define REG_TX_PKT_AUDINFO_CC 0x68 // [2:0]
#define REG_TX_PKT_AUDINFO_SF 0x69 // [4:2]
#define REG_TX_PKT_AUDINFO_CA 0x6B // [7:0]

#define REG_TX_PKT_AUDINFO_DM_LSV 0x6C // [7][6:3]
#define REG_TX_PKT_AUDINFO_SUM 0x6D // [7:0]

// Source Product Description Info Frame
#define REG_TX_PKT_SPDINFO_SUM 0x70
#define REG_TX_PKT_SPDINFO_PB1 0x71
#define REG_TX_PKT_SPDINFO_PB2 0x72
#define REG_TX_PKT_SPDINFO_PB3 0x73
#define REG_TX_PKT_SPDINFO_PB4 0x74
#define REG_TX_PKT_SPDINFO_PB5 0x75
#define REG_TX_PKT_SPDINFO_PB6 0x76
#define REG_TX_PKT_SPDINFO_PB7 0x77
#define REG_TX_PKT_SPDINFO_PB8 0x78
#define REG_TX_PKT_SPDINFO_PB9 0x79
#define REG_TX_PKT_SPDINFO_PB10 0x7A
#define REG_TX_PKT_SPDINFO_PB11 0x7B
#define REG_TX_PKT_SPDINFO_PB12 0x7C
#define REG_TX_PKT_SPDINFO_PB13 0x7D
#define REG_TX_PKT_SPDINFO_PB14 0x7E
#define REG_TX_PKT_SPDINFO_PB15 0x7F
#define REG_TX_PKT_SPDINFO_PB16 0x80
#define REG_TX_PKT_SPDINFO_PB17 0x81
#define REG_TX_PKT_SPDINFO_PB18 0x82
#define REG_TX_PKT_SPDINFO_PB19 0x83
#define REG_TX_PKT_SPDINFO_PB20 0x84
#define REG_TX_PKT_SPDINFO_PB21 0x85
#define REG_TX_PKT_SPDINFO_PB22 0x86
#define REG_TX_PKT_SPDINFO_PB23 0x87
#define REG_TX_PKT_SPDINFO_PB24 0x88
#define REG_TX_PKT_SPDINFO_PB25 0x89

#define REG_TX_PKT_MPGINFO_FMT 0x8A
#define B_MPG_FR 1
#define B_MPG_MF_I  (1<<1)
#define B_MPG_MF_B  (2<<1)
#define B_MPG_MF_P  (3<<1)
#define B_MPG_MF_MASK (3<<1)
#define REG_TX_PKG_MPGINFO_DB0 0x8B
#define REG_TX_PKG_MPGINFO_DB1 0x8C
#define REG_TX_PKG_MPGINFO_DB2 0x8D
#define REG_TX_PKG_MPGINFO_DB3 0x8E
#define REG_TX_PKG_MPGINFO_SUM 0x8F

#define REG_TX_AUDCHST_MODE    0x91 // 191 REG_TX_AUD_CHSTD[2:0] 6:4
                                 //     REG_TX_AUD_CHSTC 3
                                 //     REG_TX_AUD_NLPCM 2
                                 //     REG_TX_AUD_MONO 0
#define REG_TX_AUDCHST_CAT     0x92 // 192 REG_TX_AUD_CHSTCAT 7:0
#define REG_TX_AUDCHST_SRCNUM  0x93 // 193 REG_TX_AUD_CHSTSRC 3:0
#define REG_TX_AUD0CHST_CHTNUM 0x94 // 194 REG_TX_AUD0_CHSTCHR 7:4
                                 //     REG_TX_AUD0_CHSTCHL 3:0
#define REG_TX_AUD1CHST_CHTNUM 0x95 // 195 REG_TX_AUD1_CHSTCHR 7:4
                                 //     REG_TX_AUD1_CHSTCHL 3:0
#define REG_TX_AUD2CHST_CHTNUM 0x96 // 196 REG_TX_AUD2_CHSTCHR 7:4
                                 //     REG_TX_AUD2_CHSTCHL 3:0
#define REG_TX_AUD3CHST_CHTNUM 0x97 // 197 REG_TX_AUD3_CHSTCHR 7:4
                                 //     REG_TX_AUD3_CHSTCHL 3:0
#define REG_TX_AUDCHST_CA_FS   0x98 // 198 REG_TX_AUD_CHSTCA 5:4
                                 //     REG_TX_AUD_CHSTFS 3:0
#define REG_TX_AUDCHST_OFS_WL  0x99 // 199 REG_TX_AUD_CHSTOFS 7:4
                                 //     REG_TX_AUD_CHSTWL 3:0

/////////////////////////////////////////////////////////////////////
// Macro
/////////////////////////////////////////////////////////////////////

//#define Switch_HDMITX_Bank(x)   HDMITX_WriteI2C_Byte(0x0f,(x)&1)

#define HDMI_OrREG_TX_Byte_6613(reg,ormask) HDMITX_WriteI2C_Byte_6613(reg,(HDMITX_ReadI2C_Byte_6613(reg) | (ormask)))
#define HDMI_OrREG_TX_Byte_66121(reg,ormask) HDMITX_WriteI2C_Byte_66121(reg,(HDMITX_ReadI2C_Byte_66121(reg) | (ormask)))
#define HDMI_AndREG_TX_Byte_6613(reg,andmask) HDMITX_WriteI2C_Byte_6613(reg,(HDMITX_ReadI2C_Byte_6613(reg) & (andmask)))
#define HDMI_AndREG_TX_Byte_66121(reg,andmask) HDMITX_WriteI2C_Byte_66121(reg,(HDMITX_ReadI2C_Byte_66121(reg) & (andmask)))
#define HDMI_SetREG_TX_Byte_6613(reg,andmask,ormask) HDMITX_WriteI2C_Byte_6613(reg,((HDMITX_ReadI2C_Byte_6613(reg) & (andmask))|(ormask)))
#define HDMI_SetREG_TX_Byte_66121(reg,andmask,ormask) HDMITX_WriteI2C_Byte_66121(reg,((HDMITX_ReadI2C_Byte_66121(reg) & (andmask))|(ormask)))

/////////////////////////////////////////////////////////////////////
// data structure
/////////////////////////////////////////////////////////////////////
typedef struct _INSTANCE_STRUCT {

    BYTE I2C_DEV ;
    BYTE I2C_ADDR ;

    /////////////////////////////////////////////////
    // Interrupt Type
    /////////////////////////////////////////////////
    BYTE bIntType ; // = 0 ;
    /////////////////////////////////////////////////
    // Video Property
    /////////////////////////////////////////////////
    BYTE bInputVideoSignalType ; // for Sync Embedded,CCIR656,InputDDR
    /////////////////////////////////////////////////
    // Audio Property
    /////////////////////////////////////////////////
    BYTE bOutputAudioMode ; // = 0 ;
    BYTE bAudioChannelSwap ; // = 0 ;
    BYTE bAudioChannelEnable ;
    BYTE bAudFs ;
    unsigned long TMDSClock ;
    BYTE bAuthenticated:1 ;
    BYTE bHDMIMode: 1;
    BYTE bIntPOL:1 ; // 0 = Low Active
    BYTE bHPD:1 ;
    // 2009/11/11 added by jj_tseng@ite.com.tw
    BYTE bSPDIF_OUT;
    BYTE TxEMEMStatus:1 ;
    //~jau-chih.tseng@ite.com.tw 2009/11/11
} INSTANCE ;

typedef struct HDMITX_DETIMING_TAG
{
    unsigned int HDES;
    unsigned int HDEE;  
    unsigned int VDES;
    unsigned int VDEE;  
} HDMITX_DETIMING;
//#ifdef HDMITX_IT66121
/////////////////////////////////////////////////////////////////////
// data structure
/////////////////////////////////////////////////////////////////////
typedef struct _HDMITXDEV_STRUCT {

	BYTE I2C_DEV ;
	BYTE I2C_ADDR ;

	/////////////////////////////////////////////////
	// Interrupt Type
	/////////////////////////////////////////////////
	BYTE bIntType ; // = 0 ;
	/////////////////////////////////////////////////
	// Video Property
	/////////////////////////////////////////////////
	BYTE bInputVideoSignalType ; // for Sync Embedded,CCIR656,InputDDR
	/////////////////////////////////////////////////
	// Audio Property
	/////////////////////////////////////////////////
	BYTE bOutputAudioMode ; // = 0 ;
	BYTE bAudioChannelSwap ; // = 0 ;
    BYTE bAudioChannelEnable ;
    BYTE bAudFs ;
    unsigned long TMDSClock ;
    unsigned long RCLK ;
	BYTE bAuthenticated:1 ;
	BYTE bHDMIMode: 1;
	BYTE bIntPOL:1 ; // 0 = Low Active
	BYTE bHPD:1 ;
	// 2009/11/11 added by jj_tseng@ite.com.tw
    BYTE bSPDIF_OUT;
    BYTE TxEMEMStatus:1 ;
    //~jau-chih.tseng@ite.com.tw 2009/11/11
} HDMITXDEV ;

typedef struct structRegSetEntry {
    BYTE offset ;
    BYTE invAndMask ;
    BYTE OrMask ;
} RegSetEntry;
//#endif

extern HDMITX_DETIMING timingDE_6613;
extern HDMITX_DETIMING timingDE_66121;

//~jj_tseng@chipadvanced.com

//////////////////////////////////////////////////////////////////////
// External Interface
//////////////////////////////////////////////////////////////////////
void InitHDMITX_6613();
void InitHDMITX_66121();
void HDMITX_InitInstance_6613(INSTANCE *pInstance);
void HDMITX_InitInstance_66121(INSTANCE *pInstance);
BOOL EnableVideoOutput_6613(VIDEOPCLKLEVEL level,BYTE inputColorMode,BYTE outputColorMode,BYTE bHDMI);
BOOL EnableVideoOutput_66121(VIDEOPCLKLEVEL level,BYTE inputColorMode,BYTE outputColorMode,BYTE bHDMI);
BOOL SetupVideoInputSignal_6613(BYTE inputSignalType);
BOOL SetupVideoInputSignal_66121(BYTE inputSignalType);
BOOL getHDMITX_AuthenticationDone();//2015.3.24, my.wei add

//#ifdef SUPPORT_DEGEN
BOOL ProgramDEGenModeByID_6613(MODE_ID id,BYTE bInputSignalType, TXInput_Device inputDevice);
BOOL ProgramDEGenModeByID_66121(MODE_ID id,BYTE bInputSignalType, TXInput_Device inputDevice);
//#endif // SUPPORT_DEGEN
#ifdef SUPPORT_SYNCEMBEDDED
BOOL ProgramSyncEmbeddedByVIC_6613(BYTE VIC,BYTE bInputType);
BOOL ProgramSyncEmbeddedByVIC_66121(BYTE VIC,BYTE bInputType);
BOOL ProgramSyncEmbeddedByTiming_6613(VIDEO_Timing *pVTiming,BYTE bInputSignalType);
BOOL ProgramSyncEmbeddedByTiming_66121(VIDEO_Timing *pVTiming,BYTE bInputSignalType);
#endif
BOOL EnableAudioOutput(unsigned long VideoPixelClock,BYTE bAudioSampleFreq,BYTE ChannelNumber,BYTE bAudSWL,BYTE bSPDIF);
void DisableHDMITX_6613();
void DisableHDMITX_66121();
void DisableVideoOutput_6613();
void DisableVideoOutput_66121();
void DisableAudioOutput_6613();
void DisableAudioOutput_66121();
BOOL GetEDIDData_6613(int EDIDBlockID,BYTE *pEDIDData);
BOOL GetEDIDData_66121(int EDIDBlockID,BYTE *pEDIDData);
SYS_STATUS ReadEDID_6613(BYTE *pData,BYTE bSegment,BYTE offset,SHORT Count);
SYS_STATUS ReadEDID_66121(BYTE *pData,BYTE bSegment,BYTE offset,SHORT Count);
BYTE CheckHDMITX_6613(BYTE *pHPD,BYTE *pHPDChange);
BYTE CheckHDMITX_66121(BYTE *pHPD,BYTE *pHPDChange);
BOOL EnableHDCP_6613(BYTE bEnable);
BOOL EnableHDCP_66121(BYTE bEnable);
BOOL EnableAVIInfoFrame_6613(BYTE bEnable,BYTE *pAVIInfoFrame);
BOOL EnableAVIInfoFrame_66121(BYTE bEnable,BYTE *pAVIInfoFrame);
BOOL EnableAudioInfoFrame_6613(BYTE bEnable,BYTE *pAudioInfoFrame);
BOOL EnableAudioInfoFrame_66121(BYTE bEnable,BYTE *pAudioInfoFrame);
// BOOL EnableVideoOutputIndirect(BYTE xCnt,BYTE inputColorMode,BYTE outputColorMode,BYTE bHDMI);
void SetAVMute_6613(BYTE bEnable);
void SetAVMute_66121(BYTE bEnable);
void SetOutputColorDepthPhase_6613(BYTE ColorDepth,BYTE bPhase);
void SetOutputColorDepthPhase_66121(BYTE ColorDepth,BYTE bPhase);
void Get6613Reg(BYTE *pReg);
void Get66121Reg(BYTE *pReg);

// 2008/08/18 added by jj_tseng@chipadvanced.com
/////////////////////////////////////////////////////////////////////////////////////
// HDMITX part
/////////////////////////////////////////////////////////////////////////////////////
void setHDMITX_ChStat_6613(BYTE ucIEC60958ChStat[]);
void setHDMITX_ChStat_66121(BYTE ucIEC60958ChStat[]);
void setHDMITX_UpdateChStatFs_6613(ULONG Fs);
void setHDMITX_UpdateChStatFs_66121(ULONG Fs);
void setHDMITX_LPCMAudio_6613(BYTE AudioSrcNum, BYTE AudSWL, BOOL bSPDIF);
void setHDMITX_LPCMAudio_66121(BYTE AudioSrcNum, BYTE AudSWL, BOOL bSPDIF);
void setHDMITX_NLPCMAudio_6613();
void setHDMITX_NLPCMAudio_66121();
void setHDMITX_HBRAudio_6613(BOOL bSPDIF);
void setHDMITX_HBRAudio_66121(BOOL bSPDIF);
void setHDMITX_DSDAudio_6613();
void setHDMITX_DSDAudio_66121();
void EnableHDMIAudio_6613(BYTE AudioType, BOOL bSPDIF,  ULONG SampleFreq,  BYTE ChNum, BYTE *pIEC60958ChStat, ULONG TMDSClock);
void EnableHDMIAudio_66121(BYTE AudioType, BOOL bSPDIF,  ULONG SampleFreq,  BYTE ChNum, BYTE *pIEC60958ChStat, ULONG TMDSClock);
BOOL EnableVendorSpecificInfoFrame_6613(BYTE bEnable, BYTE *pInfoFrame);
BOOL EnableVendorSpecificInfoFrame_66121(BYTE bEnable, BYTE *pInfoFrame);
void DumpCatHDMITXReg_6613();
void DumpCatHDMITXReg_66121();

////////////////////////////////////////////////////////////////////
// Required Interfance
////////////////////////////////////////////////////////////////////
BYTE I2C_Read_Byte(BYTE Addr,BYTE RegAddr);
SYS_STATUS I2C_Write_Byte(BYTE Addr,BYTE RegAddr,BYTE Data);
SYS_STATUS I2C_Read_ByteN(BYTE Addr,BYTE RegAddr,BYTE *pData,int N);
SYS_STATUS I2C_Write_ByteN(BYTE Addr,BYTE RegAddr,BYTE *pData,int N);

BYTE HDMITX_ReadI2C_Byte_6613(BYTE RegAddr);
BYTE HDMITX_ReadI2C_Byte_66121(BYTE RegAddr);
SYS_STATUS HDMITX_WriteI2C_Byte_6613(BYTE RegAddr,BYTE val);
SYS_STATUS HDMITX_WriteI2C_Byte_66121(BYTE RegAddr,BYTE val);
SYS_STATUS HDMITX_ReadI2C_ByteN_6613(BYTE RegAddr,BYTE *pData,int N);
SYS_STATUS HDMITX_ReadI2C_ByteN_66121(BYTE RegAddr,BYTE *pData,int N);
SYS_STATUS HDMITX_WriteI2C_ByteN_6613(BYTE RegAddr,BYTE *pData,int N);
SYS_STATUS HDMITX_WriteI2C_ByteN_66121(BYTE RegAddr,BYTE *pData,int N);
BOOL HDMITX_IsChipEmpty_6613(void);
BOOL HDMITX_IsChipEmpty_66121(void);

#define HDMITX_OrReg_Byte_6613(reg,ormask) HDMITX_WriteI2C_Byte_6613(reg,(HDMITX_ReadI2C_Byte_6613(reg) | (ormask)))
#define HDMITX_OrReg_Byte_66121(reg,ormask) HDMITX_WriteI2C_Byte_66121(reg,(HDMITX_ReadI2C_Byte_66121(reg) | (ormask)))
#define HDMITX_AndReg_Byte_6613(reg,andmask) HDMITX_WriteI2C_Byte_6613(reg,(HDMITX_ReadI2C_Byte_6613(reg) & (andmask)))
#define HDMITX_AndReg_Byte_66121(reg,andmask) HDMITX_WriteI2C_Byte_66121(reg,(HDMITX_ReadI2C_Byte_66121(reg) & (andmask)))
#define HDMITX_SetReg_Byte_6613(reg,andmask,ormask) HDMITX_WriteI2C_Byte_6613(reg,((HDMITX_ReadI2C_Byte_6613(reg) & (andmask))|(ormask)))
#define HDMITX_SetReg_Byte_66121(reg,andmask,ormask) HDMITX_WriteI2C_Byte_66121(reg,((HDMITX_ReadI2C_Byte_66121(reg) & (andmask))|(ormask)))


#endif // _HDMITX_DRV_H_

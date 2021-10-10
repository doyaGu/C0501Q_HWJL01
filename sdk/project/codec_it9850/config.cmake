#
# Automatically generated file; DO NOT EDIT.
# Project Configuration
# Fri Oct 03 14:50:46 2014
#
#
set(CFG_DEF_CPU_SM32 y)
set(CFG_AUDIO_ENABLE y)
set(CFG_AUDIO_DAC_MODULE "itp_dac_ite9070.c")
set(CFG_AUDIO_CODEC_AACDEC y)
set(CFG_AUDIO_CODEC_AMR y)
set(CFG_AUDIO_CODEC_EAC3DEC y)
set(CFG_AUDIO_CODEC_MP3DEC y)
set(CFG_AUDIO_CODEC_WAV y)
set(CFG_AUDIO_CODEC_WMADEC y)
set(CFG_AUDIO_CODEC_FLACDEC y)
set(CFG_RISC_TS_DEMUX_PLUGIN y)
set(CFG_AUDIO_CODEC_SBC y)

#
# (null) (unvisible)
#

#
# (null) (unvisible)
#
set(CFG_DEF_CFG_DEVELOP y)

#
# Develop Environment
#
set(CFG_DEV_DEVELOP y)
# CFG_DEV_RELEASE is not set

#
# OpenRTOS
#
#set(CFG_GCC_LTO y)
set(CFG_OPENRTOS_HEAP_SIZE "0")
set(CFG_MMAP_SIZE "0")

#
# System
#
# CFG_CPU_FA626 is not set
set(CFG_CPU_SM32 y)
set(CFG_CPU_BIG_ENDIAN y)
# CFG_CHIP_REV_A0 is not set
set(CFG_CHIP_REV_A1 y)
# CFG_CHIP_REV_B0 is not set
# CFG_CHIP_PKG_IT9072 is not set
# CFG_CHIP_PKG_IT9076 is not set
# CFG_CHIP_PKG_IT9078 is not set
# CFG_CHIP_PKG_IT9079 is not set
# CFG_CHIP_PKG_IT9079H is not set
set(CFG_CHIP_FAMILY 9850)
# CFG_CHIP_PKG_IT9910 is not set
set(CFG_RAM_SIZE "0x4000000")
set(CFG_RAM_INIT_SCRIPT "IT9079A1_Initial_DDR2_Mem_tiling_pitch2048_320MHZ.scr")
# CFG_WATCHDOG_ENABLE is not set
set(CFG_MEMDBG_ENABLE y)
set(CFG_ROM_COMPRESS y)
# CFG_DCPS_ENABLE is not set
# CFG_DPU_ENABLE is not set
# CFG_SPREAD_SPECTRUM_PLL1_ENABLE is not set
# CFG_SPREAD_SPECTRUM_PLL2_ENABLE is not set
# CFG_SPREAD_SPECTRUM_PLL3_ENABLE is not set
# CFG_CPU_WB is not set
# CFG_CPU_WRITE_BUFFER is not set
# CFG_XCPU_MSGQ is not set

#
# Screen
#
# CFG_LCD_ENABLE is not set
# CFG_BACKLIGHT_ENABLE is not set

#
# Internal Setting
#
# CFG_TVOUT_ENABLE is not set
# CFG_STNLCD_ENABLE is not set

#
# Graphics
#
# CFG_CMDQ_ENABLE is not set
# CFG_JPEG_HW_ENABLE is not set
# CFG_UI_ENC_ENABLE is not set
set(CFG_FONT_FILENAME "WenQuanYiMicroHeiMono.ttf")

#
# Audio
#
# CFG_AUDIO_CODEC_AC3DEC is not set
# CFG_AUDIO_CODEC_AC3SPDIF is not set
# CFG_AUDIO_CODEC_MP3DEC_FLASH is not set
# CFG_AUDIO_CODEC_G711_ALAW is not set
# CFG_AUDIO_CODEC_G711_ULAW is not set

#
# Video
#
# CFG_VIDEO_ENABLE is not set
# CFG_TS_MODULE_ENABLE is not set

#
# (null) (unvisible)
#

#
# (null) (unvisible)
#

#
# (null) (unvisible)
#
# CFG_TSCAM_ENABLE is not set

#
# Storage
#
# CFG_NAND_ENABLE is not set
# CFG_NOR_ENABLE is not set
# CFG_SD0_ENABLE is not set
# CFG_SD1_ENABLE is not set
# CFG_XD_ENABLE is not set
# CFG_RAMDISK_ENABLE is not set

#
# File System
#
set(CFG_PRIVATE_DRIVE "A")
set(CFG_PUBLIC_DRIVE "B")
set(CFG_TEMP_DRIVE "C")

#
# Peripheral
#

#
# Internal Peripheral
#
# CFG_RTC_ENABLE is not set
set(CFG_RTC_DEFAULT_TIMESTAMP "1325376000")
# CFG_I2C_ENABLE is not set
# CFG_I2S_ENABLE is not set
# CFG_SPI_ENABLE is not set
# CFG_WIEGAND_ENABLE is not set
# CFG_PCF8575_ENABLE is not set
# CFG_UART0_ENABLE is not set
# CFG_UART1_ENABLE is not set
# CFG_USB0_ENABLE is not set
# CFG_USB1_ENABLE is not set
# CFG_IRDA_ENABLE is not set
# CFG_IR_ENABLE is not set
# CFG_KEYPAD_ENABLE is not set
# CFG_BATTERY_ENABLE is not set
# CFG_GSENSOR_ENABLE is not set
# CFG_HEADSET_ENABLE is not set
# CFG_SPEAKER_ENABLE is not set
# CFG_AMPLIFIER_ENABLE is not set
# CFG_LED_ENABLE is not set
# CFG_SWITCH_ENABLE is not set
# CFG_TUNER_ENABLE is not set
# CFG_FM2018_ENABLE is not set
# CFG_AUDIOAMP_ENABLE is not set
# CFG_SENSOR_ENABLE is not set

#
# Power
#
# CFG_POWER_ON is not set
# CFG_POWER_STANDBY is not set
# CFG_POWER_SLEEP is not set
# CFG_POWER_DOZE is not set
# CFG_POWER_TICKLESS_IDLE is not set

#
# GPIO
#
# CFG_GPIO_EXTENDER_ENABLE is not set
set(CFG_I2S_USE_GPIO_MODE_2 y)
# CFG_DAC_USE_EXTERNAL_DEPOP_CIRCUIT is not set

#
# Network
#
# CFG_NET_ENABLE is not set

#
# Task
#

#
# Debug
#
# CFG_DBG_NONE is not set
set(CFG_DBG_PRINTBUF y)
# CFG_DBG_SWUART is not set
set(CFG_DBG_INIT_SCRIPT "IT9079A1_Initial_DDR2_Mem_tiling_pitch2048_320MHZ.txt")
set(CFG_DBG_ICE_SCRIPT "IT9070A1_Initial_DDR2_Mem_tiling_pitch2048_320MHZ.csf")
set(CFG_DBG_PRINTBUF_SIZE "0x10000")
# CFG_DBG_MEMLEAK is not set
# CFG_DBG_RMALLOC is not set
# CFG_DBG_STATS is not set
# CFG_DBG_TRACE_ANALYZER is not set

#
# Upgrade
#
# CFG_UPGRADE_BOOTLOADER is not set
# CFG_UPGRADE_IMAGE is not set
# CFG_UPGRADE_DATA is not set
set(CFG_UPGRADE_ENC_KEY "0")
set(CFG_UPGRADE_FILENAME "ITEPKG03.PKG")

#
# SDK
#
# CFG_BUILD_DEBUG is not set
# CFG_BUILD_DEBUGREL is not set
set(CFG_BUILD_RELEASE y)
# CFG_BUILD_MINSIZEREL is not set
set(CFG_VERSION_MAJOR "0")
set(CFG_VERSION_MINOR "1")
set(CFG_VERSION_PATCH "0")
set(CFG_VERSION_CUSTOM "0")
set(CFG_SYSTEM_NAME "ITE Castor3")
set(CFG_MANUFACTURER "www.ite.com.tw")
# CFG_GENERATE_DOC is not set

#
# Drivers
#

#
# ith
#
set(CFG_ITH_ERR y)
set(CFG_ITH_WARN y)
set(CFG_ITH_INFO y)
# CFG_ITH_DBG is not set
# CFG_ITH_FPGA is not set

#
# itp
#
set(CFG_ITP_ERR y)
set(CFG_ITP_WARN y)
set(CFG_ITP_INFO y)
# CFG_ITP_DBG is not set

#
# Gadget
#

#
# Libraries
#

#
# (null) (unvisible)
#

#
# dhcps (unvisible)
#

#
# (null) (unvisible)
#

#
# ffmpeg (unvisible)
#

#
# (null) (unvisible)
#

#
# itc (unvisible)
#

#
# (null) (unvisible)
#

#
# itu (unvisible)
#

#
# (null) (unvisible)
#

#
# upgrade (unvisible)
#

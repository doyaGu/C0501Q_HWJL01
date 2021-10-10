/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Initialize functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <sys/ioctl.h>
#include <errno.h>
#include <locale.h>
#include "fat/fat.h"
#include "itp_cfg.h"
#include "i2s/i2s.h"

#if defined(CFG_LCD_ENABLE) && !defined(CFG_LCD_MULTIPLE)
    #if defined(CFG_VIDEO_ENABLE) || defined(CFG_LCD_TRIPLE_BUFFER)
        static const uint32_t __lcd_base_a = CFG_RAM_SIZE - CFG_LCD_WIDTH * CFG_LCD_HEIGHT * CFG_LCD_BPP * 3 - CFG_CMDQ_SIZE;
        static const uint32_t __lcd_base_b = CFG_RAM_SIZE - CFG_LCD_WIDTH * CFG_LCD_HEIGHT * CFG_LCD_BPP * 2 - CFG_CMDQ_SIZE;
        static const uint32_t __lcd_base_c = CFG_RAM_SIZE - CFG_LCD_WIDTH * CFG_LCD_HEIGHT * CFG_LCD_BPP * 1 - CFG_CMDQ_SIZE;
    #else
        static const uint32_t __lcd_base_a = CFG_RAM_SIZE - CFG_LCD_WIDTH * CFG_LCD_HEIGHT * CFG_LCD_BPP * 2 - CFG_CMDQ_SIZE;
        static const uint32_t __lcd_base_b = CFG_RAM_SIZE - CFG_LCD_WIDTH * CFG_LCD_HEIGHT * CFG_LCD_BPP * 1 - CFG_CMDQ_SIZE;
    #endif // CFG_VIDEO_ENABLE
#endif // defined(CFG_LCD_ENABLE) && !defined(CFG_LCD_MULTIPLE)

void itpInit(void)
{
    // init lcd
#ifdef CFG_LCD_ENABLE
    itpRegisterDevice(ITP_DEVICE_SCREEN, &itpDeviceScreen);
    
#ifndef CFG_LCD_MULTIPLE

#ifndef CFG_BL_SHOW_LOGO
    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_RESET, NULL);

    ithLcdSetBaseAddrA((uint32_t) __lcd_base_a);
    ithLcdSetBaseAddrB((uint32_t) __lcd_base_b);

#if defined(CFG_VIDEO_ENABLE) || defined(CFG_LCD_TRIPLE_BUFFER)
    ithLcdSetBaseAddrC((uint32_t) __lcd_base_c);
#endif
#endif // !CFG_BL_SHOW_LOGO

#endif // !CFG_LCD_MULTIPLE

#endif // CFG_LCD_ENABLE

#ifdef CFG_BACKLIGHT_ENABLE
    itpRegisterDevice(ITP_DEVICE_BACKLIGHT, &itpDeviceBacklight);
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_INIT, NULL);
#endif // CFG_BACKLIGHT_ENABLE

#if defined(CFG_DBG_LCDCONSOLE) && !defined(CFG_LCD_MULTIPLE)
    // init lcd console device
    itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceLcdConsole);
    itpRegisterDevice(ITP_DEVICE_LCDCONSOLE, &itpDeviceLcdConsole);
    
    ioctl(ITP_DEVICE_LCDCONSOLE, ITP_IOCTL_INIT, NULL);
    ioctl(ITP_DEVICE_LCDCONSOLE, ITP_IOCTL_CLEAR, NULL);
#elif defined(CFG_DBG_OSDCONSOLE) && !defined(CFG_LCD_MULTIPLE)
    // init osd console device
    itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceOsdConsole);
    itpRegisterDevice(ITP_DEVICE_OSDCONSOLE, &itpDeviceOsdConsole);
    
    ioctl(ITP_DEVICE_OSDCONSOLE, ITP_IOCTL_INIT, NULL);
#endif // defined(CFG_DBG_LCDCONSOLE)

#ifdef CFG_DBG_STATS
    itpStatsInit();
#endif

    // init GPIO expander device
#ifdef CFG_GPIO_EXPANDER_ENABLE
    itpRegisterDevice(ITP_DEVICE_GPIO_EXPANDER, &itpDeviceGpioExpander);
    ioctl(ITP_DEVICE_GPIO_EXPANDER, ITP_IOCTL_INIT, NULL);
#endif

    // init i2c device
#ifdef CFG_I2C0_ENABLE
    itpRegisterDevice(ITP_DEVICE_I2C0, &itpDeviceI2c0);
    ioctl(ITP_DEVICE_I2C0, ITP_IOCTL_INIT, NULL);
#endif

    // init keypad device
#ifdef CFG_KEYPAD_ENABLE
    itpRegisterDevice(ITP_DEVICE_KEYPAD, &itpDeviceKeypad);
    ioctl(ITP_DEVICE_KEYPAD, ITP_IOCTL_INIT, NULL);
#endif

    // init power device
#if defined(CFG_BATTERY_ENABLE) || defined(CFG_POWER_ON) || defined(CFG_POWER_STANDBY) || defined(CFG_POWER_SLEEP) || defined(CFG_POWER_SUSPEND) || defined(CFG_POWER_OFF)
    itpRegisterDevice(ITP_DEVICE_POWER, &itpDevicePower);
    ioctl(ITP_DEVICE_POWER, ITP_IOCTL_INIT, NULL);
#endif

    // init uart0 device
#if defined(CFG_UART0_ENABLE) && !defined(CFG_DBG_UART0)
    itpRegisterDevice(ITP_DEVICE_UART0, &itpDeviceUart0);
    ioctl(ITP_DEVICE_UART0, ITP_IOCTL_RESET, (void*)CFG_UART0_BAUDRATE);
#endif

    // init uart1 device
#if defined(CFG_UART1_ENABLE) && !defined(CFG_DBG_UART1)
    itpRegisterDevice(ITP_DEVICE_UART1, &itpDeviceUart1);
    ioctl(ITP_DEVICE_UART1, ITP_IOCTL_RESET, (void*)CFG_UART1_BAUDRATE);
#endif

    // init spi device
#ifdef CFG_SPI_ENABLE
    itpRegisterDevice(ITP_DEVICE_SPI, &itpDeviceSpi0);
    ioctl(ITP_DEVICE_SPI, ITP_IOCTL_INIT, NULL);
#endif

    // init i2s device
#ifdef CFG_I2S_ENABLE
	#ifdef CFG_I2S_USE_GPIO_MODE_2
    i2s_init_gpio_mode2();
    #endif    
    #ifdef CFG_I2S_SPDIF_ENABLE
     i2s_init_spdif();
    #endif    
    #ifdef CFG_I2S_OUTPUT_PIN_ENABLE
     i2s_init_output_pin();
    #endif
#endif

    // init nand device
#ifdef CFG_NAND_ENABLE
    itpRegisterDevice(ITP_DEVICE_NAND, &itpDeviceNand);
    ioctl(ITP_DEVICE_NAND, ITP_IOCTL_INIT, NULL);
#endif

    // init xd device
#ifdef CFG_XD_ENABLE
    itpRegisterDevice(ITP_DEVICE_XD, &itpDeviceXd);
    ioctl(ITP_DEVICE_XD, ITP_IOCTL_INIT, NULL);
#endif

    // init nor device
#ifdef CFG_NOR_ENABLE
    itpRegisterDevice(ITP_DEVICE_NOR, &itpDeviceNor);
    ioctl(ITP_DEVICE_NOR, ITP_IOCTL_INIT, NULL);
#endif

    // init sd0 device
#ifdef CFG_SD0_STATIC
    itpRegisterDevice(ITP_DEVICE_SD0, &itpDeviceSd0);
    ioctl(ITP_DEVICE_SD0, ITP_IOCTL_INIT, NULL);
#endif

    // init sd1 device
#ifdef CFG_SD1_STATIC
    itpRegisterDevice(ITP_DEVICE_SD1, &itpDeviceSd1);
    ioctl(ITP_DEVICE_SD1, ITP_IOCTL_INIT, NULL);
#endif

    // init usb device mode device
#ifdef CFG_USB_DEVICE
    itpRegisterDevice(ITP_DEVICE_USB_DEVICE, &itpDeviceUsbDevice);
    ioctl(ITP_DEVICE_USB_DEVICE, ITP_IOCTL_INIT, NULL);
#endif

    // init ethernet device
#ifdef CFG_NET_ETHERNET
    itpRegisterDevice(ITP_DEVICE_ETHERNET, &itpDeviceEthernet);
    ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_INIT, NULL);
#endif

    // init wifi device
#ifdef CFG_NET_WIFI
    itpRegisterDevice(ITP_DEVICE_WIFI, &itpDeviceWifi);
    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_INIT, NULL);
#endif

    // init socket device
#ifdef CFG_NET_ENABLE
    itpRegisterDevice(ITP_DEVICE_SOCKET, &itpDeviceSocket);
    ioctl(ITP_DEVICE_SOCKET, ITP_IOCTL_INIT, NULL);
#endif

    // init g-sensor device
#ifdef CFG_GSENSOR_ENABLE
    itpRegisterDevice(ITP_DEVICE_GSENSOR, &itpDeviceGSensor);
    ioctl(ITP_DEVICE_GSENSOR, ITP_IOCTL_INIT, NULL);
#endif

    // init headset device
#ifdef CFG_HEADSET_ENABLE
    itpRegisterDevice(ITP_DEVICE_HEADSET, &itpDeviceHeadset);
    ioctl(ITP_DEVICE_HEADSET, ITP_IOCTL_INIT, NULL);
#endif

    // init amplifier device
#ifdef CFG_AMPLIFIER_ENABLE
    itpRegisterDevice(ITP_DEVICE_AMPLIFIER, &itpDeviceAmplifier);
    ioctl(ITP_DEVICE_AMPLIFIER, ITP_IOCTL_INIT, NULL);
    ioctl(ITP_DEVICE_AMPLIFIER, ITP_IOCTL_ENABLE, NULL);
#endif

    // init led device
#ifdef CFG_LED_ENABLE
    itpRegisterDevice(ITP_DEVICE_LED, &itpDeviceLed);
    ioctl(ITP_DEVICE_LED, ITP_IOCTL_INIT, NULL);
#endif

    // init switch device
#ifdef CFG_SWITCH_ENABLE
    itpRegisterDevice(ITP_DEVICE_SWITCH, &itpDeviceSwitch);
    ioctl(ITP_DEVICE_SWITCH, ITP_IOCTL_INIT, NULL);
#endif

    // init STN LCD device
#if defined(CFG_STNLCD_ENABLE)
    itpRegisterDevice(ITP_DEVICE_STNLCD, &itpDeviceStnLcd);
    ioctl(ITP_DEVICE_STNLCD, ITP_IOCTL_INIT, NULL);
#endif

    // init decompress
#ifdef CFG_DCPS_ENABLE
    itpRegisterDevice(ITP_DEVICE_DECOMPRESS, &itpDeviceDecompress);
#endif

    // init DPU
#ifdef CFG_DPU_ENABLE
    itpRegisterDevice(ITP_DEVICE_DPU, &itpDeviceDpu);
    ioctl(ITP_DEVICE_DPU, ITP_IOCTL_INIT, NULL);
#endif

    // init codec
#ifdef CFG_AUDIO_ENABLE
    itpRegisterDevice(ITP_DEVICE_CODEC, &itpDeviceCodec);
#endif

    // init fat
#ifdef CFG_FS_FAT
    itpRegisterDevice(ITP_DEVICE_FAT, &itpFSDeviceFat.dev);
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_INIT, NULL);
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_ENABLE, NULL);
#endif

    // init ntfs
#ifdef CFG_FS_NTFS
    itpRegisterDevice(ITP_DEVICE_NTFS, &itpFSDeviceNtfs.dev);
    ioctl(ITP_DEVICE_NTFS, ITP_IOCTL_INIT, NULL);
    ioctl(ITP_DEVICE_NTFS, ITP_IOCTL_ENABLE, NULL);
#endif

    // init drive table
#if defined(CFG_FS_FAT) || defined(CFG_FS_NTFS)
    itpRegisterDevice(ITP_DEVICE_FILE, (ITPDevice*)&itpFSDeviceFile);
    itpRegisterDevice(ITP_DEVICE_DRIVE, &itpDeviceDrive);
    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_INIT, NULL);
    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_MOUNT, NULL);
#endif

    itpRegisterDevice(ITP_DEVICE_LOGDISK, &itpDeviceLogDisk);

#if defined(CFG_VIDEO_ENABLE)
    // init Stc [2012/03/07]
    itpRegisterDevice(ITP_DEVICE_STC, &itpDeviceStc);
    ioctl(ITP_DEVICE_STC, ITP_IOCTL_INIT, NULL);
#endif

#if defined(CFG_DBG_NETCONSOLE)
    // init netconsole device
    itpRegisterDevice(ITP_DEVICE_NETCONSOLE, &itpDeviceNetConsole);
    ioctl(ITP_DEVICE_NETCONSOLE, ITP_IOCTL_INIT, NULL);
    ioctl(ITP_DEVICE_NETCONSOLE, ITP_IOCTL_ENABLE, NULL);
    itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceNetConsole);
#endif

#ifdef CFG_SPREAD_SPECTRUM_PLL1_ENABLE
    ithSetSpreadSpectrum(ITH_PLL1, CFG_SPREAD_SPECTRUM_PLL1_MODE, CFG_SPREAD_SPECTRUM_PLL1_WIDTH, CFG_SPREAD_SPECTRUM_PLL1_FREQ);
    ithEnableSpreadSpectrum(ITH_PLL1);
#endif

#ifdef CFG_SPREAD_SPECTRUM_PLL2_ENABLE
    ithSetSpreadSpectrum(ITH_PLL2, CFG_SPREAD_SPECTRUM_PLL2_MODE, CFG_SPREAD_SPECTRUM_PLL2_WIDTH, CFG_SPREAD_SPECTRUM_PLL2_FREQ);
    ithEnableSpreadSpectrum(ITH_PLL2);
#endif

#ifdef CFG_SPREAD_SPECTRUM_PLL3_ENABLE
    ithSetSpreadSpectrum(ITH_PLL3, CFG_SPREAD_SPECTRUM_PLL3_MODE, CFG_SPREAD_SPECTRUM_PLL3_WIDTH, CFG_SPREAD_SPECTRUM_PLL3_FREQ);
    ithEnableSpreadSpectrum(ITH_PLL3);
#endif

    setlocale(LC_CTYPE, "");
}

void itpExit(void)
{
#ifdef CFG_FS_FAT
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_DISABLE, NULL);
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_EXIT, NULL);
#endif

    // init ethernet device
#ifdef CFG_NET_ETHERNET
    ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_EXIT, NULL);
#endif
}
#include <locale.h>
#include <pthread.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"
#include "ite/itp.h"

#define MAIN_STACK_SIZE 100000

extern uint32_t __lcd_base_a;
extern uint32_t __lcd_base_b;
extern uint32_t __lcd_base_c;

extern void* BootloaderMain(void* arg);

static void* MainTask(void* arg)
{
    // init watch dog
#ifdef CFG_WATCHDOG_ENABLE
    itpRegisterDevice(ITP_DEVICE_WATCHDOG, &itpDeviceWatchDog);
    ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_INIT, NULL);
    ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_ENABLE, NULL);
#endif // CFG_WATCHDOG_ENABLE

    // init lcd
#ifdef CFG_LCD_ENABLE
    itpRegisterDevice(ITP_DEVICE_SCREEN, &itpDeviceScreen);

#ifdef CFG_BL_SHOW_LOGO
    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_RESET, NULL);

    ithLcdSetBaseAddrA((uint32_t) &__lcd_base_a);
    ithLcdSetBaseAddrB((uint32_t) &__lcd_base_b);
#if defined(CFG_VIDEO_ENABLE) || defined(CFG_LCD_TRIPLE_BUFFER)
    ithLcdSetBaseAddrC((uint32_t) &__lcd_base_c);
#endif
#endif

#endif // CFG_LCD_ENABLE

#ifdef CFG_BACKLIGHT_ENABLE
    itpRegisterDevice(ITP_DEVICE_BACKLIGHT, &itpDeviceBacklight);
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_INIT, NULL);
#endif // CFG_BACKLIGHT_ENABLE

    // init i2c0 device
#ifdef CFG_I2C0_ENABLE
	IIC_MODE iic_port0_mode = MASTER_MODE;
    itpRegisterDevice(ITP_DEVICE_I2C0, &itpDeviceI2c0);
    ioctl(ITP_DEVICE_I2C0, ITP_IOCTL_INIT, (void*)iic_port0_mode);
#endif

    // init i2c1 device
#ifdef CFG_I2C1_ENABLE
	IIC_MODE iic_port1_mode = MASTER_MODE;
    itpRegisterDevice(ITP_DEVICE_I2C1, &itpDeviceI2c1);
    ioctl(ITP_DEVICE_I2C1, ITP_IOCTL_INIT, (void*)iic_port1_mode);
#endif

#ifdef CFG_RTC_ENABLE
    itpRegisterDevice(ITP_DEVICE_RTC, &itpDeviceRtc);
    ioctl(ITP_DEVICE_RTC, ITP_IOCTL_INIT, NULL);
#endif // CFG_RTC_ENABLE

    // init GPIO expander device
#ifdef CFG_GPIO_EXPANDER_ENABLE
    //itpRegisterDevice(ITP_DEVICE_GPIO_EXPANDER, &itpDeviceGpioExpander);
    //ioctl(ITP_DEVICE_GPIO_EXPANDER, ITP_IOCTL_INIT, NULL);
    itpIOExpanderInit();
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

#ifdef CFG_SPI_ENABLE
    itpRegisterDevice(ITP_DEVICE_SPI, &itpDeviceSpi0);
    ioctl(ITP_DEVICE_SPI, ITP_IOCTL_INIT, NULL);
#endif

#ifdef CFG_SPI1_ENABLE
    itpRegisterDevice(ITP_DEVICE_SPI1, &itpDeviceSpi1);
    ioctl(ITP_DEVICE_SPI1, ITP_IOCTL_INIT, NULL);
#endif

    // init nand device
#ifdef CFG_NAND_ENABLE
    itpRegisterDevice(ITP_DEVICE_NAND, &itpDeviceNand);
    ioctl(ITP_DEVICE_NAND, ITP_IOCTL_INIT, NULL);
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

    // init mac
#ifdef CFG_NET_MAC_INIT_ON_BOOTLOADER
    extern int MacInit(bool initPhy);
    MacInit(true);
#endif

    // init led device
#ifdef CFG_LED_ENABLE
    itpRegisterDevice(ITP_DEVICE_LED, &itpDeviceLed);
    ioctl(ITP_DEVICE_LED, ITP_IOCTL_INIT, NULL);
#endif

    // init decompress
#ifdef CFG_DCPS_ENABLE
    itpRegisterDevice(ITP_DEVICE_DECOMPRESS, &itpDeviceDecompress);
#endif

    // enable gpio interrupt
    ithIntrEnableIrq(ITH_INTR_GPIO);

    // set locale
    setlocale(LC_CTYPE, "C-UTF-8");

    // print debug information
    ithClockStats();

    // invoke main function
    BootloaderMain(arg);
}

int main(void)
{
    pthread_t task;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, MAIN_STACK_SIZE);
    pthread_create(&task, &attr, MainTask, NULL);

    /* Now all the tasks have been started - start the scheduler. */
    vTaskStartScheduler();

    /* Should never reach here! */
    return 0;
}

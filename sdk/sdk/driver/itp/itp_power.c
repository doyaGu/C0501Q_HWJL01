/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Power functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <sys/ioctl.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#ifdef __OPENRTOS__
    #include "openrtos/FreeRTOS.h"
    #include "openrtos/task.h"
#endif // __OPENRTOS__

#include "itp_cfg.h"

#define TP_GPIO_PIN	    CFG_GPIO_TOUCH_INT

static ITPPowerStatus powerLastStatus;
static ITPPowerState powerState;
static unsigned int powerCpuRatio, powerBusRatio;

#ifdef CFG_POWER_WAKEUP_KEYPAD

static const unsigned int powerKpGpioTable[] = { CFG_GPIO_KEYPAD };
#define KEYPAD_GPIO_COUNT   (ITH_COUNT_OF(powerKpGpioTable))

#endif // CFG_POWER_WAKEUP_KEYPAD

static int PowerRead(int file, char *ptr, int len, void* info)
{
#ifdef CFG_BATTERY_ENABLE
    ITPBatteryState batState = itpBatteryGetState();
    int percent = itpBatteryGetPercent();
    if (len >= sizeof (ITPPowerStatus) && (powerState != powerLastStatus.powerState || batState != powerLastStatus.batteryState || percent != powerLastStatus.batteryPercent))
    {
        ITPPowerStatus* status = (ITPPowerStatus*)ptr;

        status->powerState = powerLastStatus.powerState = powerState;
        status->batteryState = powerLastStatus.batteryState = batState;
        status->batteryPercent = powerLastStatus.batteryPercent = percent;
        return sizeof (ITPPowerStatus);
    }
#else
    if (len >= sizeof (ITPPowerStatus) && powerState != powerLastStatus.powerState)
    {
        ITPPowerStatus* status = (ITPPowerStatus*)ptr;

        status->powerState = powerLastStatus.powerState = powerState;
        return sizeof (ITPPowerStatus);
    }
#endif // CFG_BATTERY_ENABLE
    return 0;
}

static void PowerOff(void)
{
#ifdef CFG_POWER_OFF
#ifdef CFG_RTC_ENABLE
    ithRtcCtrlEnable(ITH_RTC_SW_POWEROFF);
#endif
    ithGpioClear(CFG_GPIO_POWER_ON);
#endif // CFG_POWER_OFF
}

#ifdef CFG_POWER_OFF

static void PowerOffIntrHandler(unsigned int pin, void* arg)
{
    if (ithGpioGet(CFG_GPIO_POWER_OFF_DETECT) == 0)
        powerState = ITP_POWER_OFF;
}

#endif // CFG_POWER_OFF

static void PowerInit(void)
{
    powerLastStatus.powerState      = powerState = ITP_POWER_NORMAL;
    powerLastStatus.batteryState    = ITP_BATTERY_UNKNOWN;
    powerLastStatus.batteryPercent  = 100;

#ifdef CFG_POWER_ON
    ithGpioSetOut(CFG_GPIO_POWER_ON);
    ithGpioSet(CFG_GPIO_POWER_ON);
    ithGpioEnable(CFG_GPIO_POWER_ON);
#endif

#ifdef CFG_BATTERY_ENABLE
    itpBatteryInit();
#endif

#ifdef CFG_POWER_OFF
    ithGpioRegisterIntrHandler(CFG_GPIO_POWER_OFF_DETECT, PowerOffIntrHandler, NULL);
    ithGpioCtrlEnable(CFG_GPIO_POWER_OFF_DETECT, ITH_GPIO_INTR_BOTHEDGE);
    ithGpioEnableIntr(CFG_GPIO_POWER_OFF_DETECT);
    ithGpioSetIn(CFG_GPIO_POWER_OFF_DETECT);
    ithGpioEnable(CFG_GPIO_POWER_OFF_DETECT);
#endif
}

static void PowerResumeForDoubleClick(void)
{
	 ithEnterCritical();
#ifdef CFG_NET_ETHERNET
	extern int iteMacResume(void);
		ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_OFF, (void*)ITP_PHY_WOL); /* disable PHY wol */
		iteMacResume();
#endif
	
#ifdef CFG_WATCHDOG_ENABLE
		ithWatchDogEnable();
#endif
	
#if defined(CFG_VIDEO_ENABLE) || defined(CFG_JPEG_ENABLE)
		//ithIspEnableClock();
#endif
	
#ifdef CFG_CMDQ_ENABLE
		ithCmdQEnableClock();
#endif
	
#ifdef CFG_USB1_ENABLE
		ithUsbResume(ITH_USB1);
#endif
	
#ifdef CFG_USB0_ENABLE
		ithUsbResume(ITH_USB0);
#endif
	
#ifdef CFG_OPENVG_ENABLE
		ithOvgEnableClock();
#endif
	
#ifdef CFG_JPEG_ENABLE
		//ithJpegEnableClock();
#endif
	
#ifdef CFG_VIDEO_ENABLE
		//ithVideoEnableClock();
#endif
	
#ifdef CFG_AUDIO_ENABLE
		itp_codec_wake_up();
#endif
		
#ifdef CFG_DCPS_ENABLE
		ithDcpsResume();
#endif
	
#ifdef CFG_DPU_ENABLE
		ithDpuResume();
#endif 
	
#ifdef CFG_XD_ENABLE
		ithXdEnableClock();
#endif
	
#ifdef CFG_NAND_ENABLE
		ithNandEnableClock();
#endif
	
#ifdef CFG_LCD_ENABLE
		ithLcdEnable();
#endif
	
#ifdef CFG_TVOUT_ENABLE
		ithTveEnableClock();
#endif
	
#ifdef CFG_BACKLIGHT_ENABLE
		usleep(50000); // to avoid white flicker
		ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_ON, NULL);
#endif
	
#ifdef CFG_AMPLIFIER_ENABLE
		ioctl(ITP_DEVICE_AMPLIFIER, ITP_IOCTL_ENABLE, NULL);
#endif
	
		ithExitCritical();
	
#ifdef CFG_DBG_NETCONSOLE
		ioctl(ITP_DEVICE_NETCONSOLE, ITP_IOCTL_ENABLE, NULL);
#endif
	
		powerState = ITP_POWER_NORMAL;
		printf("PowerSleep end\n");

	ithExitCritical();
}


static void PowerResume(void)
{	
#ifdef CFG_POWER_STANDBY
    if (powerState != ITP_POWER_STANDBY)

        return;

#ifdef CFG_AUDIO_ENABLE	
	itp_codec_wake_up();
#endif	
	
    ithEnterCritical();
    
    ithDcpsEnableClock();
    
    ithSetCpuClockRatio(powerCpuRatio);
    ithSetBusClockRatio(powerBusRatio);

#ifdef CFG_I2C0_ENABLE
	IIC_MODE iic_port0_mode = MASTER_MODE;
    ioctl(ITP_DEVICE_I2C0, ITP_IOCTL_RESET, (void*)iic_port0_mode);
#endif

	ithTimerSetMatch(portTIMER, configCPU_CLOCK_HZ / configTICK_RATE_HZ);

#ifdef CFG_LCD_ENABLE
    ithLcdEnable();
#endif

#ifdef CFG_TVOUT_ENABLE
    ithTveEnableClock();
#endif

    ithExitCritical();

#ifdef CFG_BACKLIGHT_ENABLE
    usleep(50000); // to avoid white flicker
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_ON, NULL);
#endif

#ifdef CFG_AMPLIFIER_ENABLE
    ioctl(ITP_DEVICE_AMPLIFIER, ITP_IOCTL_ENABLE, NULL);
#endif

#if defined(CFG_UART0_ENABLE) || defined(CFG_DBG_UART0)
    ioctl(ITP_DEVICE_UART0, ITP_IOCTL_RESET, (void*)CFG_UART0_BAUDRATE);
#endif

#if defined(CFG_UART1_ENABLE) || defined(CFG_DBG_UART1)
    ioctl(ITP_DEVICE_UART1, ITP_IOCTL_RESET, (void*)CFG_UART1_BAUDRATE);
#endif

#if defined(CFG_UART2_ENABLE) || defined(CFG_DBG_UART2)
    ioctl(ITP_DEVICE_UART2, ITP_IOCTL_RESET, (void*)CFG_UART2_BAUDRATE);
#endif

#if defined(CFG_UART3_ENABLE) || defined(CFG_DBG_UART3)
    ioctl(ITP_DEVICE_UART3, ITP_IOCTL_RESET, (void*)CFG_UART3_BAUDRATE);
#endif

#if defined(CFG_WIEGAND0_ENABLE)
	ioctl(ITP_DEVICE_WIEGAND0, ITP_IOCTL_RESUME, NULL);
#endif

#if defined(CFG_WIEGAND1_ENABLE)
	ioctl(ITP_DEVICE_WIEGAND1, ITP_IOCTL_RESUME, NULL);
#endif


#endif // CFG_POWER_STANDBY

    powerState = ITP_POWER_NORMAL;
}

static void PowerStandby(void)
{
#ifdef CFG_POWER_STANDBY
    if (powerState != ITP_POWER_NORMAL)
        return;
	
    powerState = ITP_POWER_STANDBY;

#ifdef CFG_AUDIO_ENABLE
	itp_codec_standby();
#endif

#ifdef CFG_AMPLIFIER_ENABLE
    ioctl(ITP_DEVICE_AMPLIFIER, ITP_IOCTL_DISABLE, NULL);
#endif

#ifdef CFG_CMDQ_ENABLE
#if (CFG_CHIP_FAMILY == 9920) 
    ithCmdQWaitEmpty(ITH_CMDQ0_OFFSET);
    ithCmdQWaitEmpty(ITH_CMDQ1_OFFSET);
#else
    ithCmdQWaitEmpty();
#endif
#endif

#ifdef CFG_BACKLIGHT_ENABLE
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_OFF, NULL);	
#endif

    ithEnterCritical();

#ifdef CFG_TVOUT_ENABLE
    ithTveDisableClock();
#endif

#ifdef CFG_LCD_ENABLE
    ithLcdDisable();
#endif

#ifdef CFG_WATCHDOG_ENABLE
    ithWatchDogRestart();
#endif

    powerCpuRatio = ithGetCpuClockRatio();
    powerBusRatio = ithGetBusClockRatio();
    LOG_DBG "orgional cpu/bus clock ratio=%d/%d, change to %d/%d\n",
        powerCpuRatio, powerBusRatio, CFG_POWER_STANDBY_CPU_FREQ_RATIO, CFG_POWER_STANDBY_BUS_FREQ_RATIO LOG_END

	ithSetCpuClockRatioMultiple( powerCpuRatio, CFG_POWER_STANDBY_CPU_FREQ_RATIO);
	ithSetBusClockRatioMultiple( powerBusRatio, CFG_POWER_STANDBY_BUS_FREQ_RATIO);

    ithDcpsDisableClock();   
    
	ithTimerSetMatch(portTIMER, configCPU_CLOCK_HZ / configTICK_RATE_HZ);
	
    ithExitCritical();

#ifdef CFG_WATCHDOG_ENABLE
    ithWatchDogRestart();
#endif

#if defined(CFG_UART0_ENABLE) || defined(CFG_DBG_UART0)
    ioctl(ITP_DEVICE_UART0, ITP_IOCTL_RESET, (void*)CFG_UART0_BAUDRATE);
#endif

#if defined(CFG_UART1_ENABLE) || defined(CFG_DBG_UART1)	
    ioctl(ITP_DEVICE_UART1, ITP_IOCTL_RESET, (void*)CFG_UART1_BAUDRATE);
#endif

#if defined(CFG_UART2_ENABLE) || defined(CFG_DBG_UART2)
    ioctl(ITP_DEVICE_UART2, ITP_IOCTL_RESET, (void*)CFG_UART2_BAUDRATE);
#endif

#if defined(CFG_UART3_ENABLE) || defined(CFG_DBG_UART3)
    ioctl(ITP_DEVICE_UART3, ITP_IOCTL_RESET, (void*)CFG_UART3_BAUDRATE);
#endif

#if defined(CFG_WIEGAND0_ENABLE)
	ioctl(ITP_DEVICE_WIEGAND0, ITP_IOCTL_SUSPEND, NULL);
#endif

#if defined(CFG_WIEGAND1_ENABLE)
	ioctl(ITP_DEVICE_WIEGAND1, ITP_IOCTL_SUSPEND, NULL);
#endif

#ifdef CFG_I2C0_ENABLE
    IIC_MODE iic_port0_mode = MASTER_MODE;
    ioctl(ITP_DEVICE_I2C0, ITP_IOCTL_RESET, (void*)iic_port0_mode);
#endif

#endif // CFG_POWER_STANDBY
}

#ifdef CFG_POWER_SLEEP
#ifdef _WIN32

unsigned int __power_sleep_func_start, __power_sleep_func_end;

#else // _WIN32

extern unsigned int __power_sleep_func_start, __power_sleep_func_end;

static void __attribute__ ((section (".power_sleep_func"), no_instrument_function))
SleepFunc(register uint32_t* inParam, register uint32_t* outParam, register uint32_t* var)
{
#define KEYPAD_TABLE    ((inParam) + 0)
#define NCLK_SRC        (*((var) + 0))
#define WCLK_SRC        (*((var) + 1))
#define FCLK_SRC        (*((var) + 2))
#define BCLK_SRC        (*((var) + 3))
#define RET             (*((var) + 4))
#define PIN             (*((var) + 5))
#undef VAL
#define VAL             (*((var) + 6))
#define J               (*((var) + 7))
#define K               (*((var) + 8))
#define NCLK_RATIO      (*((var) + 9))
#define WCLK_RATIO      (*((var) + 10))
#define FCLK_RATIO      (*((var) + 11))
#define BCLK_RATIO      (*((var) + 12))

    // set clock sources to 12M
    NCLK_SRC = (ITH_READ_REG_H(ITH_NCLK) & ITH_NCLK_SRC_SEL_MASK) >> ITH_NCLK_SRC_SEL_BIT;
    WCLK_SRC = (ITH_READ_REG_H(ITH_WCLK) & ITH_WCLK_SRC_SEL_MASK) >> ITH_WCLK_SRC_SEL_BIT;
    FCLK_SRC = (ITH_READ_REG_H(ITH_FCLK) & ITH_FCLK_SRC_SEL_MASK) >> ITH_FCLK_SRC_SEL_BIT;
    BCLK_SRC = (ITH_READ_REG_H(ITH_BCLK) & ITH_BCLK_SRC_SEL_MASK) >> ITH_BCLK_SRC_SEL_BIT;

    ITH_WRITE_REG_MASK_H(ITH_NCLK,
        (0x1 << ITH_NCLK_UPD_BIT) | (ITH_CLK_CKSYS << ITH_NCLK_SRC_SEL_BIT),
        ITH_NCLK_UPD_MASK | ITH_NCLK_SRC_SEL_MASK);

    ITH_WRITE_REG_MASK_H(ITH_WCLK,
        (0x1 << ITH_WCLK_UPD_BIT) | (ITH_CLK_CKSYS << ITH_WCLK_SRC_SEL_BIT),
        ITH_WCLK_UPD_MASK | ITH_WCLK_SRC_SEL_MASK);

    ITH_WRITE_REG_MASK_H(ITH_FCLK,
        (0x1 << ITH_FCLK_UPD_BIT) | (ITH_CLK_CKSYS << ITH_FCLK_SRC_SEL_BIT),
        ITH_FCLK_UPD_MASK | ITH_FCLK_SRC_SEL_MASK);

    ITH_WRITE_REG_MASK_H(ITH_BCLK,
        (0x1 << ITH_BCLK_UPD_BIT) | (ITH_CLK_CKSYS << ITH_BCLK_SRC_SEL_BIT),
        ITH_BCLK_UPD_MASK | ITH_BCLK_SRC_SEL_MASK);

    // power down plls
//    ITH_WRITE_REG_MASK_H(ITH_PLL1_SET3_REG,
//        (0x1 << ITH_PLL1_PD_BIT) | (0x1 << ITH_PLL1_UPDATE_BIT),
//        ITH_PLL1_PD_MASK | ITH_PLL1_UPDATE_MASK);

    ITH_WRITE_REG_MASK_H(ITH_PLL2_SET3_REG,
        (0x1 << ITH_PLL2_PD_BIT) | (0x1 << ITH_PLL2_UPDATE_BIT),
        ITH_PLL2_PD_MASK | ITH_PLL2_UPDATE_MASK);

    ITH_WRITE_REG_MASK_H(ITH_PLL3_SET3_REG,
        (0x1 << ITH_PLL3_PD_BIT) | (0x1 << ITH_PLL3_UPDATE_BIT),
        ITH_PLL3_PD_MASK | ITH_PLL3_UPDATE_MASK);

    // stop memory request from other modules
    ITH_SET_REG_BIT_H(ITH_MEM_CTRL6_REG, ITH_B0_RQ_SEV_STOP_BIT);
    do
    {
        VAL = ITH_READ_REG_H(ITH_MEM_CTRL6_REG);
    } while ((VAL & (0x1 << ITH_B0_RQ_EMPTY_BIT)) == 0);

    // enable self refresh mode
    ITH_SET_REG_BIT_H(ITH_MEM_CTRL5_REG, ITH_SELF_REFRESH_BIT);
    do
    {
        VAL = ITH_READ_REG_H(ITH_MEM_CTRL6_REG);
    } while (VAL & (0x1 << ITH_B0_REF_PERIOD_CHG_BIT));

    // set cpu clock ratio to minimize
    NCLK_RATIO = (ITH_READ_REG_H(ITH_NCLK) & ITH_NCLK_RATIO_MASK) >> ITH_NCLK_RATIO_BIT;
    WCLK_RATIO = (ITH_READ_REG_H(ITH_WCLK) & ITH_WCLK_RATIO_MASK) >> ITH_WCLK_RATIO_BIT;
    FCLK_RATIO = (ITH_READ_REG_H(ITH_FCLK) & ITH_FCLK_RATIO_MASK) >> ITH_FCLK_RATIO_BIT;
    BCLK_RATIO = (ITH_READ_REG_H(ITH_BCLK) & ITH_BCLK_RATIO_MASK) >> ITH_BCLK_RATIO_BIT;

    ITH_WRITE_REG_MASK_H(ITH_NCLK,
        (0x1 << ITH_NCLK_UPD_BIT) | (1023 << ITH_NCLK_RATIO_BIT),
        ITH_NCLK_UPD_MASK | ITH_NCLK_RATIO_MASK);

    ITH_WRITE_REG_MASK_H(ITH_WCLK,
        (0x1 << ITH_WCLK_UPD_BIT) | (1023 << ITH_WCLK_RATIO_BIT),
        ITH_WCLK_UPD_MASK | ITH_WCLK_RATIO_MASK);

    ITH_WRITE_REG_MASK_H(ITH_FCLK,
        (0x1 << ITH_FCLK_UPD_BIT) | (1023 << ITH_FCLK_RATIO_BIT),
        ITH_FCLK_UPD_MASK | ITH_FCLK_RATIO_MASK);

    ITH_WRITE_REG_MASK_H(ITH_BCLK,
        (0x1 << ITH_BCLK_UPD_BIT) | (1023 << ITH_BCLK_RATIO_BIT),
        ITH_BCLK_UPD_MASK | ITH_BCLK_RATIO_MASK);

    // disable MCLK
    ITH_CLEAR_REG_BIT_H(ITH_MEM_CLK1_REG, ITH_EN_DIV_MCLK_BIT);

#if defined(CFG_POWER_WAKEUP_DELAY)

    for (VAL = 0; VAL < CFG_POWER_WAKEUP_DELAYLOOP; VAL++)
        __asm__(""); // avoid to be optimized

#elif defined(CFG_POWER_WAKEUP_TOUCH_ONE_CLICK) || defined(CFG_POWER_WAKEUP_TOUCH_DOUBLE_CLICK)

	unsigned int TP_GPIO_MASK;
	
	if(TP_GPIO_PIN <= 31)
		TP_GPIO_MASK = (1<<TP_GPIO_PIN);
	else if (TP_GPIO_PIN >= 32 && TP_GPIO_PIN <=63)
		TP_GPIO_MASK = (1<<(TP_GPIO_PIN-32));
	else if(TP_GPIO_PIN >=64 &&  TP_GPIO_PIN <=95)
		TP_GPIO_MASK = (1<<(TP_GPIO_PIN-64));
	else if(TP_GPIO_PIN >=96 &&  TP_GPIO_PIN <=127)
		TP_GPIO_MASK = (1<<(TP_GPIO_PIN-96));

    for (;;)
    {
        unsigned int regValue;
		
        regValue = ithGpioGet(TP_GPIO_PIN);
        #if defined(CFG_GPIO_TOUCH_INT_ACTIVE_HIGH)
        if (regValue & TP_GPIO_MASK)
        #else
        if (!(regValue & TP_GPIO_MASK))
        #endif
            break;
    }

#elif defined(CFG_POWER_WAKEUP_KEYPAD)

    // detect keypad input to exit suspend mode
    {
        register i;
        for (i = 0; i < KEYPAD_GPIO_COUNT; i++)
            ITH_GPIO_SET_IN(KEYPAD_TABLE[i]);
    }

    for (;;)
    {
        // probe first
        RET = -1;
        {
            register uint32_t i;
            for (i = 0; i < KEYPAD_GPIO_COUNT; i++)
            {
                PIN = KEYPAD_TABLE[i];
                VAL = ITH_GPIO_GET(PIN);

                if (PIN >= 32)
                    PIN -= 32;

                if ((VAL & (0x1 << PIN)) == 0)
                {
                    RET = i;
                    break;
                }
            }
        }
        if (RET == CFG_POWER_WAKEUP_KEYNUM)
            break;
        else if (RET != -1)
            continue;

        // probe next
        {
            register uint32_t i;
            for (i = 0; i < KEYPAD_GPIO_COUNT; i++)
            {
                PIN = KEYPAD_TABLE[i];

                ITH_GPIO_SET_OUT(PIN);
                ITH_GPIO_CLEAR(PIN);

                K = 0;
                for (J = 0; J < KEYPAD_GPIO_COUNT; J++)
                {
                    if (J == i)
                        continue;

                    PIN = KEYPAD_TABLE[J];
                    VAL = ITH_GPIO_GET(PIN);

                    if (PIN >= 32)
                        PIN -= 32;

                    if ((VAL & (0x1 << PIN)) == 0)
                    {
                        RET = KEYPAD_GPIO_COUNT + (KEYPAD_GPIO_COUNT - 1) * i + K;
                        break;
                    }
                    K++;
                }

                // reset to input PIN
                ITH_GPIO_SET_IN(KEYPAD_TABLE[i]);

                if (RET != -1)
                    break;
            }
        }

        if (RET == CFG_POWER_WAKEUP_KEYNUM)
            break;
    }
#elif defined(CFG_POWER_WAKEUP_IR)
    do
    {
        VAL = ITH_READ_REG_A(ITH_IR_BASE + ITH_IR_CAP_STATUS_REG);
    } while ((VAL & (0x1 << ITH_IR_DATAREADY_BIT)) == 0);

#elif defined(CFG_POWER_WAKEUP_WOL)

    #if CFG_GPIO_ETHERNET_WAKE_ON_LAN < 32
        do
        {
            VAL = ITH_READ_REG_A(ITH_GPIO_BASE + ITH_GPIO1_DATAIN_REG);
        } while ((VAL & (0x1 << CFG_GPIO_ETHERNET_WAKE_ON_LAN)));

    #else
        do
        {
            VAL = ITH_READ_REG_A(ITH_GPIO_BASE + ITH_GPIO2_DATAIN_REG);
        } while ((VAL & (0x1 << (CFG_GPIO_ETHERNET_WAKE_ON_LAN - 32))));

    #endif // CFG_GPIO_ETHERNET_WAKE_ON_LAN < 32

#endif // defined(CFG_POWER_WAKEUP_DELAY)

    // restore cpu clock ratio
    ITH_WRITE_REG_MASK_H(ITH_BCLK,
        (0x1 << ITH_BCLK_UPD_BIT) | (BCLK_RATIO << ITH_BCLK_RATIO_BIT),
        ITH_BCLK_UPD_MASK | ITH_BCLK_RATIO_MASK);

    ITH_WRITE_REG_MASK_H(ITH_FCLK,
        (0x1 << ITH_FCLK_UPD_BIT) | (FCLK_RATIO << ITH_FCLK_RATIO_BIT),
        ITH_FCLK_UPD_MASK | ITH_FCLK_RATIO_MASK);

    ITH_WRITE_REG_MASK_H(ITH_WCLK,
        (0x1 << ITH_WCLK_UPD_BIT) | (WCLK_RATIO << ITH_WCLK_RATIO_BIT),
        ITH_WCLK_UPD_MASK | ITH_WCLK_RATIO_MASK);

    ITH_WRITE_REG_MASK_H(ITH_NCLK,
        (0x1 << ITH_NCLK_UPD_BIT) | (NCLK_RATIO << ITH_NCLK_RATIO_BIT),
        ITH_NCLK_UPD_MASK | ITH_NCLK_RATIO_MASK);

    // enable MCLK
    ITH_SET_REG_BIT_H(ITH_MEM_CLK1_REG, ITH_EN_DIV_MCLK_BIT);

    // disable self refresh mode
    ITH_CLEAR_REG_BIT_H(ITH_MEM_CTRL5_REG, ITH_SELF_REFRESH_BIT);

    // delay loop
    for (VAL = 0; VAL < 200; VAL++)
        __asm__(""); // avoid to be optimized

    ITH_CLEAR_REG_BIT_H(ITH_MEM_CTRL6_REG, ITH_B0_RQ_SEV_STOP_BIT);

    // power up plls
    ITH_WRITE_REG_MASK_H(ITH_PLL3_SET3_REG,
        (0x0 << ITH_PLL3_PD_BIT) | (0x1 << ITH_PLL3_UPDATE_BIT),
        ITH_PLL3_PD_MASK | ITH_PLL3_UPDATE_MASK);

    ITH_WRITE_REG_MASK_H(ITH_PLL2_SET3_REG,
        (0x0 << ITH_PLL2_PD_BIT) | (0x1 << ITH_PLL2_UPDATE_BIT),
        ITH_PLL2_PD_MASK | ITH_PLL2_UPDATE_MASK);

//    ITH_WRITE_REG_MASK_H(ITH_PLL1_SET3_REG,
//        (0x0 << ITH_PLL1_PD_BIT) | (0x1 << ITH_PLL1_UPDATE_BIT),
//        ITH_PLL1_PD_MASK | ITH_PLL1_UPDATE_MASK);

    // restore clock sources
    ITH_WRITE_REG_MASK_H(ITH_BCLK,
        (0x1 << ITH_BCLK_UPD_BIT) | (BCLK_SRC << ITH_BCLK_SRC_SEL_BIT),
        ITH_BCLK_UPD_MASK | ITH_BCLK_SRC_SEL_MASK);

    ITH_WRITE_REG_MASK_H(ITH_FCLK,
        (0x1 << ITH_FCLK_UPD_BIT) | (FCLK_SRC << ITH_FCLK_SRC_SEL_BIT),
        ITH_FCLK_UPD_MASK | ITH_FCLK_SRC_SEL_MASK);

    ITH_WRITE_REG_MASK_H(ITH_WCLK,
        (0x1 << ITH_WCLK_UPD_BIT) | (WCLK_SRC << ITH_WCLK_SRC_SEL_BIT),
        ITH_WCLK_UPD_MASK | ITH_WCLK_SRC_SEL_MASK);

    ITH_WRITE_REG_MASK_H(ITH_NCLK,
        (0x1 << ITH_NCLK_UPD_BIT) | (NCLK_SRC << ITH_NCLK_SRC_SEL_BIT),
        ITH_NCLK_UPD_MASK | ITH_NCLK_SRC_SEL_MASK);
}

#endif // _WIN32
#endif // CFG_POWER_SLEEP

static void PowerSleepForDoubleClick(void)
{
	printf("PowerSleepContinue\n");

#ifdef CFG_POWER_SLEEP
		const unsigned int sleepFuncSize = (unsigned int)&__power_sleep_func_end - (unsigned int)&__power_sleep_func_start;
		uint32_t *inParams, *outParams, *var, *ptr;
		int i;

	powerState = ITP_POWER_SLEEP;
    ithEnterCritical();

	ithClockSleep();

    // copy SleepFunc() to audio's SRAM
#ifndef _WIN32
    memcpy((void*)ITH_SRAM_AUDIO_BASE, SleepFunc, sleepFuncSize);

    // setting parameters
    ptr = (uint32_t*)(ITH_SRAM_AUDIO_BASE + sleepFuncSize);

    // input parameters
    inParams = ptr;

#ifdef CFG_POWER_WAKEUP_KEYPAD
    // keypad's gpio
    for (i = 0; i < KEYPAD_GPIO_COUNT; i++)
    {
        *ptr++ = powerKpGpioTable[i];
    }
#endif // CFG_POWER_WAKEUP_KEYPAD

    // output parameters
    outParams = ptr;

    // local variables
    var = ptr;

    ithFlushDCache();
    ithInvalidateICache();

    // call SleepFunc() in SRAM
    ((void (*)(uint32_t*, uint32_t*, uint32_t*))ITH_SRAM_AUDIO_BASE)(inParams, outParams, var);
#endif // !_WIN32

    // wakeup procedure
    ithClockWakeup();

	ithExitCritical(); 
#endif // CFG_POWER_SLEEP
}

static void PowerSleep(void)
{
#ifdef CFG_POWER_SLEEP
    const unsigned int sleepFuncSize = (unsigned int)&__power_sleep_func_end - (unsigned int)&__power_sleep_func_start;
    uint32_t *inParams, *outParams, *var, *ptr;
    int i;

    // must small than SRAM size and 32-bit align
    LOG_DBG "power sleep function size: %d\n", sleepFuncSize LOG_END
    assert(sleepFuncSize < ITH_SRAM_AUDIO_SIZE);
    assert(ITH_IS_ALIGNED(sleepFuncSize, 4));

    powerState = ITP_POWER_SLEEP;
    printf("PowerSleep\n");

#ifdef CFG_DBG_NETCONSOLE
    ioctl(ITP_DEVICE_NETCONSOLE, ITP_IOCTL_DISABLE, NULL);
#endif

#ifdef CFG_AUDIO_ENABLE
    itp_codec_standby();   
#endif

    ithEnterCritical();

#ifdef CFG_AMPLIFIER_ENABLE
    ioctl(ITP_DEVICE_AMPLIFIER, ITP_IOCTL_DISABLE, NULL);
#endif

#ifdef CFG_BACKLIGHT_ENABLE
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_OFF, NULL);
#endif

#ifdef CFG_TVOUT_ENABLE
    ithTveDisableClock();
#endif

#ifdef CFG_LCD_ENABLE
    ithLcdDisable();
#endif

#ifdef CFG_NAND_ENABLE
    ithNandDisableClock();
#endif

#ifdef CFG_XD_ENABLE
    ithXdDisableClock();
#endif

#ifdef CFG_DPU_ENABLE
    ithDpuSuspend();
#endif

#ifdef CFG_DCPS_ENABLE
    ithDcpsSuspend();
#endif

#ifdef CFG_VIDEO_ENABLE
    ithVideoDisableClock();
#endif

#ifdef CFG_JPEG_ENABLE
    ithJpegDisableClock();
#endif

#ifdef CFG_OPENVG_ENABLE
    ithOvgDisableClock();
#endif

#ifdef CFG_CMDQ_ENABLE
#if (CFG_CHIP_FAMILY == 9920) 
    ithCmdQWaitEmpty(ITH_CMDQ0_OFFSET);
    ithCmdQWaitEmpty(ITH_CMDQ1_OFFSET);
#else
    ithCmdQWaitEmpty();
#endif
    ithCmdQDisableClock();
#endif

#ifdef CFG_USB0_ENABLE
    ithUsbSuspend(ITH_USB0);
#endif

#ifdef CFG_USB1_ENABLE
    ithUsbSuspend(ITH_USB1);
#endif

#if defined(CFG_VIDEO_ENABLE) || defined(CFG_JPEG_ENABLE)
    ithIspDisableClock();
#endif

#ifdef CFG_WATCHDOG_ENABLE
    ithWatchDogDisable();
#endif

#ifdef CFG_NET_ETHERNET
extern int iteMacSuspend(void);
    iteMacSuspend();
	ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_ON, (void*)ITP_PHY_WOL); /* enable PHY wol */
#endif

#ifdef CFG_POWER_WAKEUP_WOL
    // enable wake on lan gpio
    ithGpioSetIn(CFG_GPIO_ETHERNET_WAKE_ON_LAN);
    ithGpioEnable(CFG_GPIO_ETHERNET_WAKE_ON_LAN);
#endif

    ithClockSleep();

    // copy SleepFunc() to audio's SRAM
#ifndef _WIN32
    memcpy((void*)ITH_SRAM_AUDIO_BASE, SleepFunc, sleepFuncSize);

    // setting parameters
    ptr = (uint32_t*)(ITH_SRAM_AUDIO_BASE + sleepFuncSize);

    // input parameters
    inParams = ptr;

#ifdef CFG_POWER_WAKEUP_KEYPAD
    // keypad's gpio
    for (i = 0; i < KEYPAD_GPIO_COUNT; i++)
    {
        *ptr++ = powerKpGpioTable[i];
    }
#endif // CFG_POWER_WAKEUP_KEYPAD

    // output parameters
    outParams = ptr;

    // local variables
    var = ptr;

    ithFlushDCache();
    ithInvalidateICache();

    // call SleepFunc() in SRAM
    ((void (*)(uint32_t*, uint32_t*, uint32_t*))ITH_SRAM_AUDIO_BASE)(inParams, outParams, var);
#endif // !_WIN32

    // wakeup procedure
    ithClockWakeup();

#if	defined(CFG_POWER_WAKEUP_TOUCH_DOUBLE_CLICK) /* ifdoubleClick wakeup enable, leave form here. */
	ithExitCritical();
	powerState = ITP_POWER_STANDBY;
	return;
#endif

#ifdef CFG_NET_ETHERNET
extern int iteMacResume(void);
	ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_OFF, (void*)ITP_PHY_WOL); /* disable PHY wol */
    iteMacResume();
#endif

#ifdef CFG_WATCHDOG_ENABLE
    ithWatchDogEnable();
#endif

#if defined(CFG_VIDEO_ENABLE) || defined(CFG_JPEG_ENABLE)
    ithIspEnableClock();
#endif

#ifdef CFG_CMDQ_ENABLE
    ithCmdQEnableClock();
#endif

#ifdef CFG_USB1_ENABLE
    ithUsbResume(ITH_USB1);
#endif

#ifdef CFG_USB0_ENABLE
    ithUsbResume(ITH_USB0);
#endif

#ifdef CFG_OPENVG_ENABLE
    ithOvgEnableClock();
#endif

#ifdef CFG_JPEG_ENABLE
    ithJpegEnableClock();
#endif

#ifdef CFG_VIDEO_ENABLE
    ithVideoEnableClock();
#endif

#ifdef CFG_AUDIO_ENABLE 
    itp_codec_wake_up();
#endif

#ifdef CFG_DCPS_ENABLE
    ithDcpsResume();
#endif

#ifdef CFG_DPU_ENABLE
    ithDpuResume();
#endif

#ifdef CFG_XD_ENABLE
    ithXdEnableClock();
#endif

#ifdef CFG_NAND_ENABLE
    ithNandEnableClock();
#endif

#ifdef CFG_LCD_ENABLE
    ithLcdEnable();
#endif

#ifdef CFG_TVOUT_ENABLE
    ithTveEnableClock();
#endif

#ifdef CFG_BACKLIGHT_ENABLE
    usleep(50000); // to avoid white flicker
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_ON, NULL);
#endif

#ifdef CFG_AMPLIFIER_ENABLE
    ioctl(ITP_DEVICE_AMPLIFIER, ITP_IOCTL_ENABLE, NULL);
#endif

    ithExitCritical();

#ifdef CFG_DBG_NETCONSOLE
    ioctl(ITP_DEVICE_NETCONSOLE, ITP_IOCTL_ENABLE, NULL);
#endif

    powerState = ITP_POWER_NORMAL;
    printf("PowerSleep end\n");

#endif // CFG_POWER_SLEEP
}

#ifdef CFG_POWER_HIBERNATION
#ifndef _WIN32

extern uint32_t IRQAddr;
extern void ithIntrDoIrq(void);

void EmptyIrq( void ) __attribute__((naked));
void EmptyIrq(void)
{
    portSAVE_CONTEXT();

    // TEST ONLY
    //ithPrintf("EmptyIrq()\n");
    //ithWriteRegA(ITH_INTR_BASE + ITH_INTR_IRQ1_CLR_REG, 0xFFFFFFFF);
    //ithWriteRegA(ITH_INTR_BASE + ITH_INTR_IRQ2_CLR_REG, 0xFFFFFFFF);
    //ithWriteRegA(ITH_INTR_BASE + ITH_INTR_FIQ1_CLR_REG, 0xFFFFFFFF);
    //ithWriteRegA(ITH_INTR_BASE + ITH_INTR_FIQ2_CLR_REG, 0xFFFFFFFF);

    // clear wake on lan gpio interrupt
    ithIntrClearIrq(ITH_INTR_GPIO);
    ithGpioClearIntr(CFG_GPIO_ETHERNET_WAKE_ON_LAN);

    portRESTORE_CONTEXT();
}

#endif // !_WIN32
#endif // CFG_POWER_HIBERNATION

static void PowerHibernation(void)
{
#ifdef CFG_POWER_HIBERNATION
    powerState = ITP_POWER_HIBERNATION;

    ithEnterCritical();

#ifdef CFG_AMPLIFIER_ENABLE
    ioctl(ITP_DEVICE_AMPLIFIER, ITP_IOCTL_DISABLE, NULL);
#endif

#ifdef CFG_BACKLIGHT_ENABLE
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_OFF, NULL);
#endif

#ifdef CFG_TVOUT_ENABLE
    ithTveDisableClock();
#endif

#ifdef CFG_LCD_ENABLE
    ithLcdDisable();
#endif

#ifdef CFG_AUDIO_ENABLE
    // TODO: IMPLEMENT
#endif

#ifdef CFG_VIDEO_ENABLE
    ithVideoDisableClock();
#endif

#ifdef CFG_JPEG_ENABLE
    ithJpegDisableClock();
#endif

#ifdef CFG_OPENVG_ENABLE
    ithOvgDisableClock();
#endif

#ifdef CFG_CMDQ_ENABLE
#if (CFG_CHIP_FAMILY == 9920) 
    ithCmdQWaitEmpty(ITH_CMDQ0_OFFSET);
    ithCmdQWaitEmpty(ITH_CMDQ1_OFFSET);
#else
    ithCmdQWaitEmpty();
#endif
    ithCmdQDisableClock();
#endif

#ifdef CFG_USB0_ENABLE
    ithUsbSuspend(ITH_USB0);
#endif

#ifdef CFG_USB1_ENABLE
    ithUsbSuspend(ITH_USB1);
#endif

#if defined(CFG_VIDEO_ENABLE) || defined(CFG_JPEG_ENABLE)
    ithIspDisableClock();
#endif

#ifdef CFG_WATCHDOG_ENABLE
    ithWatchDogDisable();
#endif

    //ithClockSleep();

#ifndef _WIN32
    // backup interrupt settings
    ithIntrSuspend();

    // disable all interrupts
    ithWriteRegA(ITH_INTR_BASE + ITH_INTR_IRQ1_EN_REG, 0);
    ithWriteRegA(ITH_INTR_BASE + ITH_INTR_IRQ2_EN_REG, 0);
    ithWriteRegA(ITH_INTR_BASE + ITH_INTR_FIQ1_EN_REG, 0);
    ithWriteRegA(ITH_INTR_BASE + ITH_INTR_FIQ2_EN_REG, 0);

    IRQAddr = (uint32_t) EmptyIrq;
    ithFlushDCacheRange(&IRQAddr, 4);

    // enable wake on lan gpio interrupt
    ithGpioCtrlEnable(CFG_GPIO_ETHERNET_WAKE_ON_LAN, ITH_GPIO_INTR_LEVELTRIGGER);
    ithGpioEnableIntr(CFG_GPIO_ETHERNET_WAKE_ON_LAN);
    ithGpioSetIn(CFG_GPIO_ETHERNET_WAKE_ON_LAN);
    ithGpioEnable(CFG_GPIO_ETHERNET_WAKE_ON_LAN);
    ithIntrEnableIrq(ITH_INTR_GPIO);

    // POWER OFF CPU
    __asm__ __volatile__ (
		"mcr p15,0,r3,c7,c0,4\n"	/* execute idle mode */
		:
		:
		: "r3"	/* clobber list */
    );

    IRQAddr = (uint32_t) ithIntrDoIrq;

    // restore interrupt settings
    ithIntrResume();

#endif // !_WIN32

    // wakeup procedure
    //ithClockWakeup();

#ifdef CFG_WATCHDOG_ENABLE
    ithWatchDogEnable();
#endif

#if defined(CFG_VIDEO_ENABLE) || defined(CFG_JPEG_ENABLE)
    ithIspEnableClock();
#endif

#ifdef CFG_CMDQ_ENABLE
    ithCmdQEnableClock();
#endif

#ifdef CFG_USB1_ENABLE
    ithUsbResume(ITH_USB1);
#endif

#ifdef CFG_USB0_ENABLE
    ithUsbResume(ITH_USB0);
#endif

#ifdef CFG_OPENVG_ENABLE
    ithOvgEnableClock();
#endif

#ifdef CFG_JPEG_ENABLE
    ithJpegEnableClock();
#endif

#ifdef CFG_VIDEO_ENABLE
    ithVideoEnableClock();
#endif

#ifdef CFG_AUDIO_ENABLE
    // TODO: IMPLEMENT
#endif

#ifdef CFG_LCD_ENABLE
    ithLcdEnable();
#endif

#ifdef CFG_TVOUT_ENABLE
    ithTveEnableClock();
#endif

#ifdef CFG_BACKLIGHT_ENABLE
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_ON, NULL);
#endif

#ifdef CFG_AMPLIFIER_ENABLE
    ioctl(ITP_DEVICE_AMPLIFIER, ITP_IOCTL_ENABLE, NULL);
#endif

    ithExitCritical();

    powerState = ITP_POWER_NORMAL;

#endif // CFG_POWER_HIBERNATION
}

#ifdef CFG_POWER_SUSPEND
#ifdef _WIN32

unsigned int __power_suspend_func_start, __power_suspend_func_end;

#else // _WIN32

extern uint32_t SWIAddr;
extern void itpSuspendSave(void);
extern void vPortYieldProcessor(void);

extern unsigned int __power_suspend_func_start, __power_suspend_func_end;

static void __attribute__ ((section (".power_suspend_func"), no_instrument_function))
SuspendFunc(void)
{
    // to protect MMU after self refresh
    register uint32_t val = ITH_READ_REG_A(ITH_RTC_BASE + ITH_RTC_CR_REG);

    // stop memory request from other modules
    ITH_SET_REG_BIT_H(ITH_MEM_CTRL6_REG, ITH_B0_RQ_SEV_STOP_BIT);
    do
    {
        val = ITH_READ_REG_H(ITH_MEM_CTRL6_REG);
    } while ((val & (0x1 << ITH_B0_RQ_EMPTY_BIT)) == 0);

    // enable self refresh mode
    ITH_SET_REG_BIT_H(ITH_MEM_CTRL5_REG, ITH_SELF_REFRESH_BIT);
    do
    {
        val = ITH_READ_REG_H(ITH_MEM_CTRL6_REG);
    } while (val & (0x1 << ITH_B0_REF_PERIOD_CHG_BIT));

    // power off
    ITH_SET_REG_BIT_A(ITH_RTC_BASE + ITH_RTC_CR_REG, ITH_RTC_SW_POWEROFF);

    // after power off, it should not be here
    while (1);
}

#endif // _WIN32
#endif // CFG_POWER_SUSPEND

static void PowerSuspend(void)
{
#ifdef CFG_POWER_SUSPEND
    // ready to suspend
    const unsigned int suspendFuncSize = (unsigned int)&__power_suspend_func_end - (unsigned int)&__power_suspend_func_start;

    powerState = ITP_POWER_SUSPEND;

    ithEnterCritical();

    // TODO: IMPLEMENT
#ifdef CFG_BACKLIGHT_ENABLE
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_OFF, NULL);
#endif

#if defined(CFG_LCD_ENABLE) || defined(CFG_TVOUT_ENABLE)
    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_SUSPEND, NULL);
#endif

#ifdef CFG_NAND_ENABLE
    ithNandSuspend();
#endif

#ifdef CFG_XD_ENABLE
    ithXdSuspend();
#endif

#ifdef CFG_DPU_ENABLE
    ithDpuSuspend();
#endif

#ifdef CFG_DCPS_ENABLE
    ithDcpsSuspend();
#endif

#ifdef CFG_WATCHDOG_ENABLE
    ithWatchDogRestart();
    ithWatchDogSuspend();
#endif

#if defined(CFG_VIDEO_ENABLE) || defined(CFG_JPEG_ENABLE)
    ithIspDisableClock();
#endif

    ithGpioSuspend();
    ithTimerSuspend();
    ithIntrSuspend();
    ithClockSuspend();
    ithHostSuspend();

#ifndef _WIN32
    // copy SuspendFunc() to audio's SRAM
    memcpy((void*)ITH_SRAM_AUDIO_BASE, SuspendFunc, suspendFuncSize);

    SWIAddr = (uint32_t) itpSuspendSave;
    ithFlushDCacheRange(&SWIAddr, 4);
    ithInvalidateICache();
    asm volatile ( "SWI 0" );
    SWIAddr = (uint32_t) vPortYieldProcessor;
#endif // !_WIN32

    if (ithRtcGetState() == 0x0)
    {
        // must small than SRAM size and 32-bit align
        LOG_DBG "power suspend function size: %d\n", suspendFuncSize LOG_END
        assert(suspendFuncSize < ITH_SRAM_AUDIO_SIZE);
        assert(ITH_IS_ALIGNED(suspendFuncSize, 4));

    #ifndef _WIN32
        ithRtcSetState(0x1);

        ithFlushDCache();
        ithFlushMemBuffer();

        // call SuspendFunc() in SRAM
        ((void (*)(void))ITH_SRAM_AUDIO_BASE)();

    #endif // !_WIN32
    }
    else
    {
        // resume from suspend
        ithRtcCtrlDisable(ITH_RTC_SW_POWEROFF);
        ithRtcSetState(0x0);
    }

    ithHostResume();
    ithClockResume();
    ithIntrResume();
    ithTimerResume();
    ithGpioResume();

#ifdef CFG_DBG_UART0
    ioctl(ITP_DEVICE_UART0, ITP_IOCTL_RESET, (void*)CFG_UART0_BAUDRATE);
#endif

#ifdef CFG_DBG_UART1
    ioctl(ITP_DEVICE_UART1, ITP_IOCTL_RESET, (void*)CFG_UART1_BAUDRATE);
#endif

#if defined(CFG_VIDEO_ENABLE) || defined(CFG_JPEG_ENABLE)
    ithIspEnableClock();
#endif

#ifdef CFG_WATCHDOG_ENABLE
    ithWatchDogResume();
    ithWatchDogRestart();
#endif

#ifdef CFG_DCPS_ENABLE
    ithDcpsResume();
#endif

#ifdef CFG_DPU_ENABLE
    ithDpuResume();
#endif

#ifdef CFG_XD_ENABLE
    ithXdResume();
#endif

#ifdef CFG_NAND_ENABLE
    ithNandResume();
#endif

#ifdef CFG_CMDQ_ENABLE
#if (CFG_CHIP_FAMILY == 9920)
    ithCmdQReset(ITH_CMDQ0_OFFSET);
    ithCmdQReset(ITH_CMDQ1_OFFSET);
#else
    ithCmdQReset();
#endif
#endif

#if defined(CFG_LCD_ENABLE) || defined(CFG_TVOUT_ENABLE)
    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_RESUME, NULL);
#endif

#ifdef CFG_BACKLIGHT_ENABLE
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_ON, NULL);
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

    // resume procedure
    ithExitCritical();

    powerState = ITP_POWER_NORMAL;

#endif // CFG_POWER_SUSPEND
}

static int PowerIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
        PowerInit();
        break;

    case ITP_IOCTL_RESUME:
	#if !defined(CFG_POWER_WAKEUP_TOUCH_DOUBLE_CLICK)
        PowerResume();
	#else
		PowerResumeForDoubleClick();
	#endif
        break;

    case ITP_IOCTL_STANDBY:
		PowerStandby();
        break;

    case ITP_IOCTL_SLEEP:
        PowerSleep();
        break;
		
	case ITP_IOCTL_SLEEP_CONTINUE:
		PowerSleepForDoubleClick();
		break;

    case ITP_IOCTL_HIBERNATION:
        PowerHibernation();
        break;

    case ITP_IOCTL_SUSPEND:
        PowerSuspend();
        break;

    case ITP_IOCTL_OFF:
        PowerOff();
        break;

    default:
        errno = (ITP_DEVICE_POWER << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDevicePower =
{
    ":power",
    itpOpenDefault,
    itpCloseDefault,
    PowerRead,
    itpWriteDefault,
    itpLseekDefault,
    PowerIoctl,
    NULL
};

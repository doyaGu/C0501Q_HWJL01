#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "ite/ite_usbex.h"
#include "bootloader.h"
#include "config.h"
#include "ite/itp.h"
#include "ite/itu.h"
#include "itu_private.h"

static bool blLcdOn = false;
static bool blLcdConsoleOn = false;
static bool stop_led = false;

#if defined (CFG_ENABLE_UART_CLI)
static bool boot = false;
static bool bootstate = false;
char tftppara[128] = "tftp://192.168.1.20/doorbell_indoor2.bin";

#if defined (CFG_UART0_ENABLE) && defined(CFG_DBG_UART0)
#define UART_PORT ":uart0"
#elif defined(CFG_UART1_ENABLE) && defined(CFG_DBG_UART1)
#define UART_PORT ":uart1"
#elif defined(CFG_UART2_ENABLE) && defined(CFG_DBG_UART2)
#define UART_PORT ":uart2"
#elif defined(CFG_UART3_ENABLE) && defined(CFG_DBG_UART3)
#define UART_PORT ":uart3"
#endif

#endif //end of #if defined (CFG_ENABLE_UART_CLI)

#ifdef CFG_MSC_ENABLE
static bool usbInited = false;
#endif

#ifdef CFG_UPGRADE_USB_DEVICE

static bool DetectUsbDeviceMode(void)
{
    bool ret = false;
    LOG_INFO "Detect USB device mode...\r\n" LOG_END
    
    // init usb
#if defined(CFG_USB0_ENABLE) || defined(CFG_USB1_ENABLE)
    itpRegisterDevice(ITP_DEVICE_USB, &itpDeviceUsb);
    if (ioctl(ITP_DEVICE_USB, ITP_IOCTL_INIT, NULL) != -1)
        usbInited = true;    
#endif

    // init usb device mode as mass storage device
#ifdef CFG_USBD_FILE_STORAGE
    if (usbInited)
    {
        int timeout = CFG_UPGRADE_USB_DETECT_TIMEOUT;
        
        itpRegisterDevice(ITP_DEVICE_USBDFSG, &itpDeviceUsbdFsg);
        ioctl(ITP_DEVICE_USBDFSG, ITP_IOCTL_INIT, NULL);
        
        while (!ret)
        {
            if (ioctl(ITP_DEVICE_USBDFSG, ITP_IOCTL_IS_CONNECTED, NULL))
            {
                ret = true;
                break;
            }

            timeout -= 10;
            if (timeout <= 0)
            {
                LOG_INFO "USB device mode not connected.\n" LOG_END
                break;
            }
            usleep(10000);
        }
    }
#endif // CFG_USBD_FILE_STORAGE
    return ret;
}
#endif // CFG_UPGRADE_USB_DEVICE

static void InitFileSystem(void)
{
    // init card device
#if  !defined(_WIN32) && (defined(CFG_SD0_ENABLE) || defined(CFG_SD1_ENABLE) || defined(CFG_CF_ENABLE) || defined(CFG_MS_ENABLE) || defined(CFG_XD_ENABLE) || defined(CFG_MSC_ENABLE) || defined(CFG_RAMDISK_ENABLE))
    itpRegisterDevice(ITP_DEVICE_CARD, &itpDeviceCard);
    ioctl(ITP_DEVICE_CARD, ITP_IOCTL_INIT, NULL);
#endif

    // init usb
#ifdef CFG_MSC_ENABLE
    if (!usbInited)
    {
        itpRegisterDevice(ITP_DEVICE_USB, &itpDeviceUsb);
        if (ioctl(ITP_DEVICE_USB, ITP_IOCTL_INIT, NULL) != -1)
            usbInited = true;
    }
#endif

    // init fat
#ifdef CFG_FS_FAT
    itpRegisterDevice(ITP_DEVICE_FAT, &itpFSDeviceFat.dev);
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_INIT, NULL);
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_ENABLE, NULL);
#endif

    // init drive table
#if defined(CFG_FS_FAT) || defined(CFG_FS_NTFS)
    itpRegisterDevice(ITP_DEVICE_DRIVE, &itpDeviceDrive);
    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_INIT, NULL);

#ifdef CFG_MSC_ENABLE
    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_ENABLE, NULL);
    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_INIT_TASK, NULL);
#endif // CFG_MSC_ENABLE
#endif // defined(CFG_FS_FAT) || defined(CFG_FS_NTFS)

    // mount disks on booting
    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_MOUNT, NULL);

#ifdef CFG_MSC_ENABLE
    // wait msc is inserted
    if (usbInited)
    {
        ITPDriveStatus* driveStatusTable;
        ITPDriveStatus* driveStatus = NULL;
        int i, timeout = CFG_UPGRADE_USB_DETECT_TIMEOUT;
        bool found = false;
        
        while (!found)
        {
            for (i = 0; i < 1; i++)
            {
                if (ioctl(ITP_DEVICE_USB, ITP_IOCTL_IS_CONNECTED, (void*)(USB0 + i)))
                {
                    found = true;
                    break;
                }
            }

            if (found)
            {
                break;
            }
            else
            {
                timeout -= 10;
                if (timeout <= 0)
                {
                    LOG_INFO "USB device not found.\n" LOG_END
                    return;
                }
                usleep(10000);
            }
        }

        found = false;
        timeout = CFG_UPGRADE_USB_TIMEOUT;

        ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);
        
        while (!found)
        {
            for (i = 0; i < ITP_MAX_DRIVE; i++)
            {
                driveStatus = &driveStatusTable[i];
                if (driveStatus->disk >= ITP_DISK_MSC00 && driveStatus->disk <= ITP_DISK_MSC17 && driveStatus->avail)
                {
                    LOG_DBG "drive[%d]:usb disk=%d\n", i, driveStatus->disk LOG_END
                    found = true;
                }
            }
            if (!found)
            {
                timeout -= 100;
                if (timeout <= 0)
                {
                    LOG_INFO "USB disk not found.\n" LOG_END
                    break;
                }
                usleep(100000);
            }
        }
    }
#endif // CFG_MSC_ENABLE
}

#if !defined(CFG_LCD_ENABLE) && defined(CFG_CHIP_PKG_IT9910)
static void* UgLedTask(void* arg)
{
    int gpio_pin = 20;
    ithGpioSetOut(21);
    ithGpioSetMode(21,ITH_GPIO_MODE0);
    ithGpioSetOut(20);
    ithGpioSetMode(20,ITH_GPIO_MODE0);
	stop_led = false;
	
    for(;;)
    {
    	if(stop_led == true)
    	{
    		ithGpioSet(20); 
			ithGpioSet(21);
			while(1)
				usleep(500000);
    	}
        ithGpioClear(gpio_pin);
        if(gpio_pin==21)
            gpio_pin = 20;
        else
            gpio_pin = 21;
        ithGpioSet(gpio_pin); 
        usleep(500000);
    }
}
#endif

#if defined(CFG_CHIP_PKG_IT9910)
static void DetectKey(void)
{
    int ret;
    int phase = 0;
    int time_counter = 0;
    int key_counter = 0;
    bool key_pressed;
    bool key_released;
    ITPKeypadEvent ev;

    while (1)
    {
        key_pressed = key_released = false;
        ioctl(ITP_DEVICE_KEYPAD, ITP_IOCTL_PROBE, NULL);
        if (read(ITP_DEVICE_KEYPAD, &ev, sizeof (ITPKeypadEvent)) == sizeof (ITPKeypadEvent))
        {
            if (ev.code == 0)
            {
                if (ev.flags & ITP_KEYPAD_DOWN)
                    key_pressed = true;
                else if (ev.flags & ITP_KEYPAD_UP)
                    key_released = true;
            }
        }

        if (phase == 0)
        {
            if (key_pressed)
            {
                printf("key detected\n");
                phase = 1;
            }
            else
                break;
        }
        else if (phase == 1)
        {
            if (key_released)
                break;
            if (time_counter > 100)
            {
                phase = 2;
                ithGpioSetOut(21);
                ithGpioSetMode(21, ITH_GPIO_MODE0);
                ithGpioSetOut(20);
                ithGpioSetMode(20, ITH_GPIO_MODE0);
            }
        }
        else if (phase == 2)
        {
            if (key_pressed)
            {
                ithGpioSet(20);
                key_counter++;
            }
            if (key_released)
                ithGpioClear(20);

            if (time_counter > 200)
            {
                ithGpioSet(21);
                ithGpioClear(20);
                phase = 3;
            }

            // blink per 6*50000 us
            if ((time_counter/6)%2)
                ithGpioSet(21);
            else
                ithGpioClear(21);
        }
        else if (phase == 3)
        {
            printf("key_counter: %d\n", key_counter);
            if (key_counter == 1)
            {
                // do reset
                InitFileSystem();
                ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_MOUNT, NULL);
                ret = ugResetFactory();
				#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
				ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
				#endif
                exit(ret);
            }
            if (key_counter == 2)
            {
                // dump addressbook.xml
                InitFileSystem();
                ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_MOUNT, NULL);
                CopyUclFile();
            }
            ithGpioClear(21);
            break;
        }

        usleep(50000);
        time_counter++;
    }
}
#endif

#if defined (CFG_ENABLE_UART_CLI)
static int parseCommand(char* str, int strlength)
{

	int ret;
	
	if (strncmp(str, "boot", 4) == 0)
	{
		printf("going to boot procedure\n");
		boot = true;
	}

	if (strncmp(str, "update", 6) == 0)
	{
		ITCStream* upgradeFile;
		uint8_t* imagebuf;
		uint32_t imagesize;

		upgradeFile = OpenRecoveryPackage();	
		if (upgradeFile)
		{			
			BootBin(upgradeFile);
			return 1;
		}
	}

	if (strncmp(str, "upgrade", 7) == 0)
	{
		ITCStream* upgradeFile;
		
		upgradeFile = OpenRecoveryPackage();
        if (upgradeFile)
        {
            if (ugCheckCrc(upgradeFile, NULL))
                LOG_ERR "Upgrade failed.\n" LOG_END
            else
                ret = ugUpgradePackage(upgradeFile);

            #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
                LOG_INFO "Flushing NOR cache...\n" LOG_END
                ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
            #endif

            if (ret)
                LOG_INFO "Upgrade failed.\n" LOG_END
            else
                LOG_INFO "Upgrade finished.\n" LOG_END

            exit(ret);
        }
        else
        {
        #ifdef CFG_UPGRADE_RECOVERY_LED
            ioctl(fd, ITP_IOCTL_OFF, NULL);
        #endif
        }
		while (1);
	}

	if (strncmp(str, "setenv", 6) == 0)
	{
		char para[128] = {0};
		int i = 0;
		
		strncpy(para, str+7, strlength -7);		
		memset(tftppara, 0 , 128);
		strcpy(tftppara, para);
		printf("\ntftppara=%s\n", tftppara);
	}

	
	return 0;
}

static void CommandReciver(void)
{
	char rcvbuf[128], cmdbuf[128];
	static int wp = 0;
	int fd, len, i, strlength;	

#if defined (CFG_UART0_ENABLE) && defined(CFG_DBG_UART0)
	itpRegisterDevice(ITP_DEVICE_UART0, &itpDeviceUart0);
	ioctl(ITP_DEVICE_UART0, ITP_IOCTL_RESET, (void*)CFG_UART0_BAUDRATE);
#elif defined (CFG_UART1_ENABLE) && defined(CFG_DBG_UART1)
	itpRegisterDevice(ITP_DEVICE_UART1, &itpDeviceUart1);
	ioctl(ITP_DEVICE_UART1, ITP_IOCTL_RESET, (void*)CFG_UART1_BAUDRATE);
#elif defined (CFG_UART2_ENABLE) && defined(CFG_DBG_UART2)
	itpRegisterDevice(ITP_DEVICE_UART2, &itpDeviceUart2);
	ioctl(ITP_DEVICE_UART2, ITP_IOCTL_RESET, (void*)CFG_UART2_BAUDRATE);
#elif defined (CFG_UART3_ENABLE) && defined(CFG_DBG_UART3)
	itpRegisterDevice(ITP_DEVICE_UART3, &itpDeviceUart3);
	ioctl(ITP_DEVICE_UART3, ITP_IOCTL_RESET, (void*)CFG_UART3_BAUDRATE);
#endif

	fd = open(UART_PORT, O_RDONLY);
	
	if(fd < 0)	
		return;

	for(;;)
	{
	
		memset(rcvbuf, 0, 128);
		len = read(fd, rcvbuf, 128);

		if (len)
		{
			for (i = 0; i < len; i++)
			{			
				cmdbuf[wp] = rcvbuf[i];								
				wp++;					
				if (rcvbuf[i] == '\0')
				{					
					strlength = strlen(cmdbuf);					
					parseCommand(cmdbuf, strlength);
					memset(cmdbuf, 0, 128);
					wp = 0;						
				}		
			}
		}
		if(boot)
			break;
			
	}
	printf("Exit CommandReciver\n");
}
#endif
static void DoUpgrade(void)
{
    ITCStream* upgradeFile;

    LOG_INFO "Do Upgrade...\r\n" LOG_END

    upgradeFile = OpenUpgradePackage();
    if (upgradeFile)
    {
        int ret;

    #if !defined(CFG_LCD_ENABLE) && defined(CFG_CHIP_PKG_IT9910)
        //---light on red/green led task
        pthread_t task;
        pthread_create(&task, NULL, UgLedTask, NULL);
        //------
    #endif    
        // output messages to LCD console
    #if defined(CFG_LCD_ENABLE) && defined(CFG_BL_LCD_CONSOLE)
        if (!blLcdOn)
        {
        #if !defined(CFG_BL_SHOW_LOGO)
            extern uint32_t __lcd_base_a;
            extern uint32_t __lcd_base_b;
            extern uint32_t __lcd_base_c;
        
            itpRegisterDevice(ITP_DEVICE_SCREEN, &itpDeviceScreen);
            ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_RESET, NULL);
            ithLcdSetBaseAddrA((uint32_t) &__lcd_base_a);
            ithLcdSetBaseAddrB((uint32_t) &__lcd_base_b);        
        
        #ifdef CFG_BACKLIGHT_ENABLE
            itpRegisterDevice(ITP_DEVICE_BACKLIGHT, &itpDeviceBacklight);
            ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_INIT, NULL);
        #endif // CFG_BACKLIGHT_ENABLE        
        
        #endif // !defined(CFG_BL_SHOW_LOGO)
            
            ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
            ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_RESET, NULL);
            blLcdOn = true;
        }
        if (!blLcdConsoleOn)
        {
            itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceLcdConsole);
            itpRegisterDevice(ITP_DEVICE_LCDCONSOLE, &itpDeviceLcdConsole);
            ioctl(ITP_DEVICE_LCDCONSOLE, ITP_IOCTL_INIT, NULL);
            ioctl(ITP_DEVICE_LCDCONSOLE, ITP_IOCTL_CLEAR, NULL);
            blLcdConsoleOn = true;
        }
    #endif // defined(CFG_LCD_ENABLE) && defined(BL_LCD_CONSOLE)

        if (ugCheckCrc(upgradeFile, NULL))
            LOG_ERR "Upgrade failed.\n" LOG_END
        else
            ret = ugUpgradePackage(upgradeFile);

    #ifdef CFG_UPGRADE_DELETE_PKGFILE_AFTER_FINISH
        DeleteUpgradePackage();
    #endif

        #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
            LOG_INFO "Flushing NOR cache...\n" LOG_END
            ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
        #endif

        if (ret)
            LOG_INFO "Upgrade failed.\n" LOG_END
        else
        {
        	stop_led = true;
            LOG_INFO "Upgrade finished.\n" LOG_END
        }
    #if defined(CFG_UPGRADE_DELAY_AFTER_FINISH) && CFG_UPGRADE_DELAY_AFTER_FINISH > 0
        sleep(CFG_UPGRADE_DELAY_AFTER_FINISH);
    #endif

        exit(ret);
        while (1);
    }
}

void* BootloaderMain(void* arg)
{
    int ret;

#if defined(CFG_UPGRADE_PRESSKEY) || defined(CFG_UPGRADE_RESET_FACTORY) || defined(CFG_UPGRADE_RECOVERY)
    ITPKeypadEvent ev;
#endif

#ifdef CFG_WATCHDOG_ENABLE
    ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_DISABLE, NULL);
#endif

    ithMemDbgDisable(ITH_MEMDBG0);
    ithMemDbgDisable(ITH_MEMDBG1);

#ifdef CFG_BL_SHOW_LOGO
    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
    ShowLogo();
#ifdef CFG_BACKLIGHT_ENABLE
    usleep(100000);
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_RESET, NULL);
#endif // CFG_BACKLIGHT_ENABLE
    blLcdOn = true;
#endif // CFG_BL_SHOW_LOGO

#ifdef CFG_UPGRADE_USB_DEVICE
    if (DetectUsbDeviceMode())
    {
		ITCStream* upgradeFile = OpenUsbDevicePackage();
        if (upgradeFile)
        {
            if (ugCheckCrc(upgradeFile, NULL))
                LOG_ERR "Upgrade failed.\n" LOG_END
            else
                ret = ugUpgradePackage(upgradeFile);

            #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
                LOG_INFO "Flushing NOR cache...\n" LOG_END
                ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
            #endif

            if (ret)
                LOG_INFO "Upgrade failed.\n" LOG_END
            else
                LOG_INFO "Upgrade finished.\n" LOG_END

            exit(ret);
        }
    }
#endif // CFG_UPGRADE_USB_DEVICE

#if defined(CFG_UPGRADE_PRESSKEY) || defined(CFG_UPGRADE_RESET_FACTORY) || defined(CFG_UPGRADE_RECOVERY)
    ioctl(ITP_DEVICE_KEYPAD, ITP_IOCTL_PROBE, NULL);
    if (read(ITP_DEVICE_KEYPAD, &ev, sizeof (ITPKeypadEvent)) == sizeof (ITPKeypadEvent))
    {
        int key = ev.code, delay = 0;

#endif // defined(CFG_UPGRADE_PRESSKEY) || defined(CFG_UPGRADE_RESET_FACTORY) || defined(CFG_UPGRADE_RECOVERY)

    #ifdef CFG_UPGRADE_RECOVERY
        if (key == CFG_UPGRADE_RECOVERY_PRESSKEY_NUM)
        {
            struct timeval time = ev.time;

            // detect key pressed time
            for (;;)
            {
                if (ev.flags & ITP_KEYPAD_UP)
                    break;

                ioctl(ITP_DEVICE_KEYPAD, ITP_IOCTL_PROBE, NULL);
                if (read(ITP_DEVICE_KEYPAD, &ev, sizeof (ITPKeypadEvent)) == 0)
                    continue;

                delay += itpTimevalDiff(&time, &ev.time);
                time = ev.time;

                LOG_DBG "recovery key: time=%ld.%ld,code=%d,down=%d,up=%d,repeat=%d,delay=%d\r\n",
                    ev.time.tv_sec,
                    ev.time.tv_usec / 1000,
                    ev.code,
                    (ev.flags & ITP_KEYPAD_DOWN) ? 1 : 0,
                    (ev.flags & ITP_KEYPAD_UP) ? 1 : 0,
                    (ev.flags & ITP_KEYPAD_REPEAT) ? 1 : 0,
                    delay
                LOG_END

                if (delay >= CFG_UPGRADE_RECOVERY_PRESSKEY_DELAY)
                    break;
            };

            if (delay >= CFG_UPGRADE_RECOVERY_PRESSKEY_DELAY)
            {
                ITCStream* upgradeFile;
            #ifdef CFG_UPGRADE_RECOVERY_LED
                int fd = open(":led:" CFG_UPGRADE_RECOVERY_LED_NUM, O_RDONLY);
                ioctl(fd, ITP_IOCTL_FLICKER, (void*)500);
            #endif

                // output messages to LCD console
            #if defined(CFG_LCD_ENABLE) && defined(CFG_BL_LCD_CONSOLE)
                if (!blLcdConsoleOn)
                {
                    itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceLcdConsole);
                    itpRegisterDevice(ITP_DEVICE_LCDCONSOLE, &itpDeviceLcdConsole);
                    ioctl(ITP_DEVICE_LCDCONSOLE, ITP_IOCTL_INIT, NULL);
                    ioctl(ITP_DEVICE_LCDCONSOLE, ITP_IOCTL_CLEAR, NULL);
                    blLcdConsoleOn = true;
                }
                if (!blLcdOn)
                {
                    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
                    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_RESET, NULL);
                    blLcdOn = true;
                }
            #endif // defined(CFG_LCD_ENABLE) && defined(BL_LCD_CONSOLE)

                LOG_INFO "Do Recovery...\r\n" LOG_END

                InitFileSystem();

                upgradeFile = OpenRecoveryPackage();
                if (upgradeFile)
                {
                    if (ugCheckCrc(upgradeFile, NULL))
                        LOG_ERR "Recovery failed.\n" LOG_END
                    else
                        ret = ugUpgradePackage(upgradeFile);

                    #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
                        LOG_INFO "Flushing NOR cache...\n" LOG_END
                        ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
                    #endif

                    if (ret)
                        LOG_INFO "Recovery failed.\n" LOG_END
                    else
                        LOG_INFO "Recovery finished.\n" LOG_END

                    exit(ret);
                }
                else
                {
                #ifdef CFG_UPGRADE_RECOVERY_LED
                    ioctl(fd, ITP_IOCTL_OFF, NULL);
                #endif
                }
                while (1);
            }
        }
    #endif // CFG_UPGRADE_RECOVERY

    #ifdef CFG_UPGRADE_RESET_FACTORY
        if (key == CFG_UPGRADE_RESET_FACTORY_PRESSKEY_NUM)
        {
            struct timeval time = ev.time;

            // detect key pressed time
            for (;;)
            {
                if (ev.flags & ITP_KEYPAD_UP)
                    break;

                ioctl(ITP_DEVICE_KEYPAD, ITP_IOCTL_PROBE, NULL);
                if (read(ITP_DEVICE_KEYPAD, &ev, sizeof (ITPKeypadEvent)) == 0)
                    continue;

                delay += itpTimevalDiff(&time, &ev.time);
                time = ev.time;

                LOG_DBG "reset key: time=%ld.%ld,code=%d,down=%d,up=%d,repeat=%d,delay=%d\r\n",
                    ev.time.tv_sec,
                    ev.time.tv_usec / 1000,
                    ev.code,
                    (ev.flags & ITP_KEYPAD_DOWN) ? 1 : 0,
                    (ev.flags & ITP_KEYPAD_UP) ? 1 : 0,
                    (ev.flags & ITP_KEYPAD_REPEAT) ? 1 : 0,
                    delay
                LOG_END

                if (delay >= CFG_UPGRADE_RESET_FACTORY_PRESSKEY_DELAY)
                    break;
            };

            if (delay >= CFG_UPGRADE_RESET_FACTORY_PRESSKEY_DELAY)
            {
                // output messages to LCD console
            #if defined(CFG_LCD_ENABLE) && defined(CFG_BL_LCD_CONSOLE)
                if (!blLcdConsoleOn)
                {
                    itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceLcdConsole);
                    itpRegisterDevice(ITP_DEVICE_LCDCONSOLE, &itpDeviceLcdConsole);
                    ioctl(ITP_DEVICE_LCDCONSOLE, ITP_IOCTL_INIT, NULL);
                    ioctl(ITP_DEVICE_LCDCONSOLE, ITP_IOCTL_CLEAR, NULL);
                    blLcdConsoleOn = true;
                }
                if (!blLcdOn)
                {
                    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
                    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_RESET, NULL);
                    blLcdOn = true;
                }
            #endif // defined(CFG_LCD_ENABLE) && defined(BL_LCD_CONSOLE)

                LOG_INFO "Do Reset...\r\n" LOG_END

                InitFileSystem();
                ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_MOUNT, NULL);

                ret = ugResetFactory();

            #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
                LOG_INFO "Flushing NOR cache...\n" LOG_END
                ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
            #endif

                if (ret)
                    LOG_INFO "Reset failed.\n" LOG_END
                else
                    LOG_INFO "Reset finished.\n" LOG_END

                exit(ret);
                while (1);
            }
        }
    #endif // CFG_UPGRADE_RESET_FACTORY

    #ifdef CFG_UPGRADE_PRESSKEY
        if (key == CFG_UPGRADE_PRESSKEY_NUM)
        {
            struct timeval time = ev.time;

            // detect key pressed time
            for (;;)
            {
                if (ev.flags & ITP_KEYPAD_UP)
                    break;

                ioctl(ITP_DEVICE_KEYPAD, ITP_IOCTL_PROBE, NULL);
                if (read(ITP_DEVICE_KEYPAD, &ev, sizeof (ITPKeypadEvent)) == 0)
                    continue;

                delay += itpTimevalDiff(&time, &ev.time);
                time = ev.time;

                LOG_DBG "upgrade key: time=%ld.%ld,code=%d,down=%d,up=%d,repeat=%d,delay=%d\r\n",
                    ev.time.tv_sec,
                    ev.time.tv_usec / 1000,
                    ev.code,
                    (ev.flags & ITP_KEYPAD_DOWN) ? 1 : 0,
                    (ev.flags & ITP_KEYPAD_UP) ? 1 : 0,
                    (ev.flags & ITP_KEYPAD_REPEAT) ? 1 : 0,
                    delay
                LOG_END

                if (delay >= CFG_UPGRADE_PRESSKEY_DELAY)
                    break;
            };

            if (delay >= CFG_UPGRADE_PRESSKEY_DELAY)
            {
                InitFileSystem();
                DoUpgrade();
            }
        }
    #endif // CFG_UPGRADE_PRESSKEY
#if defined(CFG_UPGRADE_PRESSKEY) || defined(CFG_UPGRADE_RESET_FACTORY) || defined(CFG_UPGRADE_RECOVERY)
    }
#endif

#if defined(CFG_CHIP_PKG_IT9910)
	DetectKey();
#endif

#if !defined(CFG_UPGRADE_PRESSKEY) && defined(CFG_UPGRADE_OPEN_FILE)
    InitFileSystem();
    DoUpgrade();
#if defined (CFG_ENABLE_UART_CLI)	
	CommandReciver();
#endif
#endif

#ifdef CFG_UPGRADE_BACKUP_PACKAGE
	if (ugUpgradeCheck())
    {
        // output messages to LCD console
    #if defined(CFG_LCD_ENABLE) && defined(CFG_BL_LCD_CONSOLE)
        if (!blLcdConsoleOn)
        {
            itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceLcdConsole);
            itpRegisterDevice(ITP_DEVICE_LCDCONSOLE, &itpDeviceLcdConsole);
            ioctl(ITP_DEVICE_LCDCONSOLE, ITP_IOCTL_INIT, NULL);
            ioctl(ITP_DEVICE_LCDCONSOLE, ITP_IOCTL_CLEAR, NULL);
            blLcdConsoleOn = true;
        }
        if (!blLcdOn)
        {
            ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
            ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_RESET, NULL);
            blLcdOn = true;
        }
    #endif // defined(CFG_LCD_ENABLE) && defined(BL_LCD_CONSOLE)
                    
        LOG_INFO "Last upgrade failed, try to restore from internal package...\r\n" LOG_END        

        // init fat
    #ifdef CFG_FS_FAT
        itpRegisterDevice(ITP_DEVICE_FAT, &itpFSDeviceFat.dev);
        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_INIT, NULL);
        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_ENABLE, NULL);
    #endif

        // mount disks on booting
        ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_MOUNT, NULL);

        RestorePackage();
    }
#endif // CFG_UPGRADE_BACKUP_PACKAGE

#ifdef CFG_BL_SHOW_VIDEO
    itpInit();
    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_RESET, NULL);

    ituLcdInit(); //init itu
    
#ifdef CFG_M2D_ENABLE
    ituM2dInit();
#ifdef CFG_VIDEO_ENABLE
    ituFrameFuncInit();
#endif // CFG_VIDEO_ENABLE
#else
    ituSWInit();
#endif // CFG_M2D_ENABLE

    PlayVideo();
    WaitPlayVideoFinish();
#endif //CFG_BL_SHOW_VIDEO

    LOG_INFO "Do Booting...\r\n" LOG_END

    BootImage();

    return NULL;
}
/*
#if (CFG_CHIP_PKG_IT9079)
void
ithCodecPrintfWrite(
    char* string,
    int length)
{
    // this is a dummy function for linking with library itp_boot. (itp_swuart_codec.c)
}

#endif
*/

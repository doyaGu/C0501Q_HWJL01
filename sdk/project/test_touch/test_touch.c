#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include "SDL/SDL.h"

//#include "..\..\sdk\share\sdl\events\SDL_touch_c.h"
//#include "..\..\sdk\share\sdl\video\castor3\SDL_castor3touch.h"
#include "ite/itp.h"

#include "tslib.h"
#include <stdbool.h>


/********************************************
 * MACRO defination
 ********************************************/

static struct tsdev *ts;
/********************************************
 * global variable
 ********************************************/


/********************************************
 * private function 
 ********************************************/
/* TSLIB will load the config file before initialization 						*/
/* the config files of tslib are:												*/
/* \build\openrtos\test_touch\data\private\ts.conf 								*/
/* \sdk\target\touch\XXXX.conf (XXXX means the "TOUCH_MODULE" name)				*/
/*                   															*/
/* Put these 2 files into USB disk for TSLIB initialization						*/	
#ifdef CFG_MSC_ENABLE
static void _waitUsbMscInserted(void)
{
    ITPDriveStatus* driveStatusTable;
    ITPDriveStatus* driveStatus = NULL;
    int i, timeout = 500;
    bool found = false;
    
    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    while (!found)
    {
        ITPUsbInfo usbInfo;
        int i;
        
        usbInfo.host = true;
        
        for (i = 0; i < 1; i++)
        {
            usbInfo.usbIndex = i;
            ioctl(ITP_DEVICE_USB, ITP_IOCTL_GET_INFO, (void*)&usbInfo);
            if (usbInfo.ctxt)
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
            timeout -= 100;
            if (timeout <= 0)
            {
                printf( "USB device not found.\n" );
                return;
            }
            usleep(100000);
        }
    }

    found = false;
    timeout = CFG_UPGRADE_USB_TIMEOUT;

    while (!found)
    {
        for (i = 0; i < ITP_MAX_DRIVE; i++)
        {
            driveStatus = &driveStatusTable[i];
            if (driveStatus->disk >= ITP_DISK_MSC00 && driveStatus->disk <= ITP_DISK_MSC17 && driveStatus->avail)
            {
                printf( "drive[%d]:usb disk=%d\n", i, driveStatus->disk );
                found = true;
            }
        }
        if (!found)
        {
            timeout -= 100;
            if (timeout <= 0)
            {
                printf( "USB disk not found.\n" );
                break;
            }
            usleep(100000);
        }
    }
}
#endif

void tslib_test(void)
{
    int ret;
	struct ts_sample samp;
	int gLastP=0;
	
	#if (CFG_CHIP_FAMILY == 9070)	
	/* TO avoid the conflict of IIC & SPI bus */
	/* Removing SPI connector before countdown if they both occupied the GPIO2&3 */
	printf("! Ready to GO, 5 ...\n"); usleep(500000); usleep(500000);
	printf("! Ready to GO, 4 ...\n"); usleep(500000); usleep(500000);
	printf("! Ready to GO, 3 ...\n"); usleep(500000); usleep(500000);
	printf("! Ready to GO, 2 ...\n"); usleep(500000); usleep(500000);
	printf("! Ready to GO, 1 ...\n"); usleep(500000); usleep(500000);
	printf("! Ready to GO, 0 ...\n"); usleep(500000); usleep(500000);
	#endif
	
	itpInit();
	
#ifdef CFG_MSC_ENABLE
	_waitUsbMscInserted();	
#endif // CFG_MSC_ENABLE

	/* TILIB initial flow */
#if defined(CFG_TOUCH_I2C0)
    ts = ts_open(":i2c0",0);
#elif defined(CFG_TOUCH_I2C1)
    ts = ts_open(":i2c1",0);
#elif defined(CFG_TOUCH_SPI)
    ts = ts_open(":spi",0);
#endif // _WIN32

	if (!ts) 
    {
		perror("ts_open");
	}

	if (ts_config(ts)) 
    {
		perror("ts_config");
	}
	
	samp.x=0;
	samp.y=0;
	samp.pressure=0;

	while(1)
	{		
		ret = ts_read(ts, &samp, 1);
		
		if(gLastP || samp.pressure)
		{
			printf("p=%x, x=%f, y=%f\n",samp.pressure,(float)samp.x,(float)samp.y);
			gLastP = samp.pressure;
		}
		
		usleep(33000);
	}

	printf("End of ts_read() of TSLIB Test!!\n");
	while(1);
}

int TouchEvent_test(void)
{
    SDL_Event ev;
	
	#if (CFG_CHIP_FAMILY == 9070)	
	/* TO avoid the conflict of IIC & SPI bus */
	/* Removing SPI connector before countdown if IIC occupied the GPIO2&3 */
	printf("! Ready to GO, 5 ...\n"); usleep(500000); usleep(500000);
	printf("! Ready to GO, 4 ...\n"); usleep(500000); usleep(500000);
	printf("! Ready to GO, 3 ...\n"); usleep(500000); usleep(500000);
	printf("! Ready to GO, 2 ...\n"); usleep(500000); usleep(500000);
	printf("! Ready to GO, 1 ...\n"); usleep(500000); usleep(500000);
	printf("! Ready to GO, 0 ...\n"); usleep(500000); usleep(500000);
	#endif
	
	itpInit();	

#ifdef CFG_MSC_ENABLE
	_waitUsbMscInserted();	
#endif // CFG_MSC_ENABLE
	
	/* SDL initial */
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        
    for (;;)
    {
        bool result = false;

        while (SDL_PollEvent(&ev))
        {
            switch (ev.type)
            {
            case SDL_FINGERDOWN:
                printf("touch: down %d, %d\n", ev.tfinger.x, ev.tfinger.y);
                break;

            case SDL_FINGERUP:
                printf("touch: up %d, %d\n", ev.tfinger.x, ev.tfinger.y);
                break;

            case SDL_FINGERMOTION:
                printf("touch: move %d, %d\n", ev.tfinger.x, ev.tfinger.y);
                break;
                
            case SDL_SLIDEGESTURE:
                switch (ev.dgesture.gestureId)
                {
                case SDL_TG_LEFT:
                    printf("touch: slide to left\n");
                    break;

                case SDL_TG_UP:
                    printf("touch: slide to up\n");
                    break;

                case SDL_TG_RIGHT:
                    printf("touch: slide to right\n");
                    break;

                case SDL_TG_DOWN:
                    printf("touch: slide to down\n");
                    break;
                }
                break;
            }
        }
        SDL_Delay(1);
    }
}

void* TestFunc(void* arg)
{
	//tslib_test();
	TouchEvent_test();
	
	printf("END of touch test\n");
	while(1);

	return NULL;
}

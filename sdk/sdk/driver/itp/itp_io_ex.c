/** @file
 * IO EX functions.
 *
 * @author
 * @version 1.0
 * @date 2016
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
#include <errno.h>
#include <sys/socket.h>
#include <pthread.h>
#include "itp_cfg.h"

#ifndef _MSC_VER
#include "openrtos/FreeRTOS.h"
#include "openrtos/queue.h"
#endif

static uint8_t IoEx0,IoEx1,IoEx2,IoEx3,IoEx4,IoEx5,IoEx6,IoEx7;
static uint8_t MappingCount=0, OldStatus=0, NewStatus=0;
static ITHGpioIntrHandler itpIoExIntrHandler;
static pthread_mutex_t IoExInternalMutex  = PTHREAD_MUTEX_INITIALIZER;

void _IoEx_isr(unsigned int pin, void *arg)
{	
    uint32_t regValue;
	uint8_t  pinStauts;
    regValue = ithGpioGet(pin); //must check this for correct INTr
	if(regValue)
		pinStauts |= (0x01 << (pin - IoEx0));
	else
		pinStauts &= ~(0x01 << (pin - IoEx0));
	//ithPrintf("pin=%d,regValue=0x%x,NewStatus=0x%x\n",pin,regValue,NewStatus);   

	itpIoExIntrHandler(NULL,NULL);
    ithGpioClearIntr(pin);
}
static void IoExDefaultIntrHandler(void)
{
    // DO NOTHING
}

int IoExRead(int file, char *ptr, int len, void* info)
{
	int i;
	uint8_t temp_old     = 0;
    uint8_t temp_new     = 0;
	uint8_t SaveDiffPins = 0;
	uint8_t pinStatus=0;

	if(NewStatus != OldStatus)
	{
		temp_old  = OldStatus;
	    temp_new  = NewStatus;
		for (i = 0; i < MappingCount; i++)
	    {
	        if ( (temp_old & 0x1) != (temp_new & 0x1))
	        {
	            SaveDiffPins |= (0x1 << i);
	        }

	        temp_old >>= 1;
	        temp_new >>= 1;

	        if (temp_old == temp_new)
	        {
	            OldStatus = NewStatus;
	            break;
	        }
	    }
	    OldStatus = NewStatus;

		memcpy(ptr, &NewStatus, sizeof(uint8_t));
		//printf("change OldStatus=0x%x,NewStatus=0x%x,len=%d\n",OldStatus, NewStatus, sizeof(uint8_t));
		return SaveDiffPins; 	
	}
	else
	{
		memcpy(ptr, &OldStatus, sizeof(uint8_t));
		//printf("No OldStatus=0x%x,NewStatus=0x%x,len=%d\n",OldStatus, NewStatus, sizeof(uint8_t));
		return 0; 	
	}
}

static int IoExIoctl(int file, unsigned long request, void* ptr, void* info)
{
    int i;
	int32_t gpioState=0;
	ITHIOEXConfig *IoExConfig;

    switch (request)
    {
    case FIONREAD:
        break;

	case ITP_IOCTL_REG_IOEX_CB:
		itpIoExIntrHandler = (ITHGpioIntrHandler)ptr;
		break;
		
    case ITP_IOCTL_INIT:
		{
			IoExConfig	 =	(ITHIOEXConfig*)ptr;
			MappingCount = IoExConfig->MappingCount;
			
			printf("IoExConfig=>ealGPIONum[0]=%d,IoExConfig->MappingGPIONum[1]=%d\n",IoExConfig->MappingGPIONum[0],IoExConfig->MappingGPIONum[1]);
			IoEx0 = IoExConfig->MappingGPIONum[0];
			IoEx1 = IoExConfig->MappingGPIONum[1];
			IoEx2 = IoExConfig->MappingGPIONum[2];
			IoEx3 = IoExConfig->MappingGPIONum[3];
			IoEx4 = IoExConfig->MappingGPIONum[4];
			IoEx5 = IoExConfig->MappingGPIONum[5];
			IoEx6 = IoExConfig->MappingGPIONum[6];
			IoEx7 = IoExConfig->MappingGPIONum[7];
			
			for(i=IoEx0; i<=IoEx7; i++)
			{
				ithGpioSetMode(i, ITH_GPIO_MODE0);
				ithGpioClearIntr(i);
				ithGpioRegisterIntrHandler(i,_IoEx_isr, NULL);
				ithGpioSetIn(i);

				ithGpioCtrlEnable(i, ITH_GPIO_PULL_ENABLE); //To enable the PULL function of this GPIO pin
				ithGpioCtrlDisable(i, ITH_GPIO_PULL_UP);	//To set "PULL LOW" of this GPIO pin

				if(IoExConfig->LevelTrigger)
					ithGpioCtrlEnable(i, ITH_GPIO_INTR_LEVELTRIGGER);	// set to Level trigger
				if(IoExConfig->EdgeTrigger)
					ithGpioCtrlDisable(i, ITH_GPIO_INTR_LEVELTRIGGER);	// set to Edge trigger

				if(IoExConfig->BothEdge)
					ithGpioCtrlEnable(i, ITH_GPIO_INTR_BOTHEDGE);		//Both edge
				if(IoExConfig->SigleEdge)
					ithGpioCtrlDisable(i, ITH_GPIO_INTR_BOTHEDGE);		//Single edge

				if(IoExConfig->FallingTrigger)
					ithGpioCtrlEnable(i, ITH_GPIO_INTR_TRIGGERFALLING); // set to Falling  edge
				if(IoExConfig->RisingTrigger)
					ithGpioCtrlDisable(i, ITH_GPIO_INTR_TRIGGERFALLING); // set to Rising edge
			
				ithIntrEnableIrq(ITH_INTR_GPIO); //this has already setting on itp_init_openrtos.c
				ithGpioEnableIntr(i);
				ithGpioEnable(i);
				gpioState = ithGpioGet(i);
				if(gpioState)
					OldStatus |= (0x01 << (i - IoEx0));
				else
					OldStatus &= ~(0x01 << (i - IoEx0));
				
				printf("IoEx%d=%d\n",(i - IoEx0),i);
			}

			printf("default OldStatus=0x%x\n",OldStatus);
			itpIoExIntrHandler = IoExDefaultIntrHandler;
    	}	
        break;

    case ITP_IOCTL_ON:   
        break;

    case ITP_IOCTL_OFF:
        break;

    case ITP_IOCTL_RESET:
        break;

    default:
        errno = -1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceIoEX =
{
    ":io_ex",
    itpOpenDefault,
    itpCloseDefault,
    IoExRead,
    itpWriteDefault,
    itpLseekDefault,
    IoExIoctl,
    NULL
};


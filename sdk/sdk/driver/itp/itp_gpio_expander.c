/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL GPIO expander functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include <pthread.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/queue.h"
#include "itp_cfg.h"
#include "ite/itp.h"


#define GPIO_EXPANDER_TASK_PRIORITY 4

static ITPGpioExpanderDeferIntrHandler  itpGpioExpanderDeferIntrHandler;
#ifdef CFG_SECOND_GPIO_EXPANDER_ENABLE
static ITPSecondGpioExpanderDeferIntrHandler  itpSecondGpioExpanderDeferIntrHandler;
#endif

static uint32_t                   GPIOexpanderReferenceCount = 0;

void itpIOExpanderSetOut(ITP_IOExpander IOEXP_Port,unsigned int pin)
{
    if (GPIOexpanderReferenceCount)
    {
    	if(IOEXP_Port == IOEXPANDER_0)
        IOExpander_SetOutPortPin(pin);
#ifdef CFG_SECOND_GPIO_EXPANDER_ENABLE
		else
		IOExpander_SetOutPortPin_2(pin);
#endif
    }
    else
    {
        printf("GpioExpanderInit fail , Line = %d !!\n", __LINE__);
    }
}

void itpIOExpanderSetIn(ITP_IOExpander IOEXP_Port,unsigned int pin)
{
    if (GPIOexpanderReferenceCount)
    {
	    if(IOEXP_Port == IOEXPANDER_0)
        IOExpander_SetInPortPin(pin);
#ifdef CFG_SECOND_GPIO_EXPANDER_ENABLE
		else
		IOExpander_SetInPortPin_2(pin);	
#endif
    }
    else
    {
        printf("GpioExpanderInit fail , Line = %d !!\n", __LINE__);
    }
}

void itpIOExpanderSet(ITP_IOExpander IOEXP_Port,unsigned int pin)
{
    if (GPIOexpanderReferenceCount)
    {
	    if(IOEXP_Port == IOEXPANDER_0)
        IOExpander_SetPortPin(pin);
#ifdef CFG_SECOND_GPIO_EXPANDER_ENABLE
		else
		IOExpander_SetPortPin_2(pin);
#endif
    }
    else
    {
        printf("GpioExpanderInit fail , Line = %d !!\n", __LINE__);
    }
}

void itpIOExpanderClear(ITP_IOExpander IOEXP_Port,unsigned int pin)
{
    if (GPIOexpanderReferenceCount)
    {
	    if(IOEXP_Port == IOEXPANDER_0)
        IOExpander_ClrPortPin(pin);
#ifdef CFG_SECOND_GPIO_EXPANDER_ENABLE
		else
		IOExpander_ClrPortPin_2(pin);	
#endif
    }
    else
    {
        printf("GpioExpanderInit fail , Line = %d !!\n", __LINE__);
    }
}

uint8_t itpIOExpanderGet(ITP_IOExpander IOEXP_Port,unsigned int pin)
{
    uint8_t R_data;
    if (GPIOexpanderReferenceCount)
    {
     	if(IOEXP_Port == IOEXPANDER_0)
        R_data = IOExpander_ReadPortPin(pin);
#ifdef CFG_SECOND_GPIO_EXPANDER_ENABLE
		else
		R_data = IOExpander_ReadPortPin_2(pin);
#endif		
        return R_data;
    }
    else
    {
        printf("GpioExpanderInit fail , Line = %d !!\n", __LINE__);
    }
}

uint8_t itpIOExpanderGetAllPins(ITP_IOExpander IOEXP_Port)
{
    uint8_t R_data;
    if (GPIOexpanderReferenceCount)
    {
	    if(IOEXP_Port == IOEXPANDER_0)
        R_data = IOExpander_ReadPort();
#ifdef CFG_SECOND_GPIO_EXPANDER_ENABLE
		else
		R_data = IOExpander_ReadPort_2();	
#endif
		
        return R_data;
    }
    else
    {
        printf("GpioExpanderInit fail , Line = %d !!\n", __LINE__);
    }
}

uint8_t itpIOExpanderGetInputStatus(ITP_IOExpander IOEXP_Port)
{
    uint8_t R_data;
    if (GPIOexpanderReferenceCount)
    {
	    if(IOEXP_Port == IOEXPANDER_0)
        R_data = IOExpander_GetInputStatus();
#ifdef CFG_SECOND_GPIO_EXPANDER_ENABLE
		else
		R_data = IOExpander_GetInputStatus_2();
#endif
		
        return R_data;
    }
    else
    {
        printf("GpioExpanderInit fail , Line = %d !!\n", __LINE__);
    }
}
static void GpioExpanderIntrHandler(unsigned int pin, void *arg)
{
    //ithPrintf("GpioExpanderIntrHandler(%d)\n", pin);
    ithGpioClearIntr(pin);

    if(itpGpioExpanderDeferIntrHandler)
    {
        itpPendFunctionCallFromISR(itpGpioExpanderDeferIntrHandler , NULL,NULL);
    }
}
void itpRegisterIOExpanderDeferIntrHandler(ITPGpioExpanderDeferIntrHandler handler)
{
    itpGpioExpanderDeferIntrHandler  = handler;
}

#ifdef CFG_SECOND_GPIO_EXPANDER_ENABLE
static void GpioExpanderIntrHandler_2(unsigned int pin, void *arg)
{
    //ithPrintf("GpioExpanderIntrHandler(%d)\n", pin);
    ithGpioClearIntr(pin);

    if(itpSecondGpioExpanderDeferIntrHandler)
    {
        itpPendFunctionCallFromISR(itpSecondGpioExpanderDeferIntrHandler , NULL,NULL);
    }
}

void itpRegisterSecondIOExpanderDeferIntrHandler(ITPSecondGpioExpanderDeferIntrHandler handler)
{
    itpSecondGpioExpanderDeferIntrHandler  = handler;
}
#endif

static void GpioExpanderDefaultHandler(void)
{
    // DO NOTHING
}

void itpIOExpanderInit(void)
{
    pthread_t          task;
    pthread_attr_t     attr;
    struct sched_param param;
    int                i;

    GPIOexpanderReferenceCount++;
    if (GPIOexpanderReferenceCount == 1)
    {
        ithGpioRegisterIntrHandler(CFG_GPIO_EXPANDER, GpioExpanderIntrHandler, NULL);

        ithGpioSetIn(CFG_GPIO_EXPANDER);
        ithGpioCtrlDisable(CFG_GPIO_EXPANDER, ITH_GPIO_INTR_LEVELTRIGGER);  // set to edge trigger
        ithGpioCtrlDisable(CFG_GPIO_EXPANDER, ITH_GPIO_INTR_BOTHEDGE);      //single edge
        ithGpioCtrlEnable(CFG_GPIO_EXPANDER, ITH_GPIO_INTR_TRIGGERFALLING); // set to falling edge
        //ithGpioCtrlDisable(CFG_GPIO_EXPANDER,ITH_GPIO_INTR_TRIGGERFALLING); // set to rising edge

        ithGpioEnableIntr(CFG_GPIO_EXPANDER);
        ithGpioEnable(CFG_GPIO_EXPANDER);
        IOExpanderDriver_initial();
		
#ifdef CFG_SECOND_GPIO_EXPANDER_ENABLE
        ithGpioRegisterIntrHandler(CFG_SECOND_GPIO_EXPANDER, GpioExpanderIntrHandler_2, NULL);

        ithGpioSetIn(CFG_SECOND_GPIO_EXPANDER);
        ithGpioCtrlDisable(CFG_SECOND_GPIO_EXPANDER, ITH_GPIO_INTR_LEVELTRIGGER);  // set to edge trigger
        ithGpioCtrlDisable(CFG_SECOND_GPIO_EXPANDER, ITH_GPIO_INTR_BOTHEDGE);      //single edge
        ithGpioCtrlEnable(CFG_SECOND_GPIO_EXPANDER, ITH_GPIO_INTR_TRIGGERFALLING); // set to falling edge

        ithGpioEnableIntr(CFG_SECOND_GPIO_EXPANDER);
        ithGpioEnable(CFG_SECOND_GPIO_EXPANDER);
		IOExpanderDriver_initial_2();
#endif
    }
    else
    {
        return;
    }
}

static int GpioExpanderIoctl(int file, unsigned long request, void *ptr, void *info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
        break;

    default:
        errno = (ITP_DEVICE_GPIO_EXPANDER << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceGpioExpander =
{
    ":gpio_expander",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    itpWriteDefault,
    itpLseekDefault,
    GpioExpanderIoctl,
    NULL
};
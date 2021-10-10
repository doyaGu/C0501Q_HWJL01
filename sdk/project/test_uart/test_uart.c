#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "openrtos/FreeRTOS.h"
#include "openrtos/queue.h"

#define UartCommandLen 6
 
#define TEST_PORT ITP_DEVICE_UART0
//#define TEST_PORT ITP_DEVICE_UART1
//#define TEST_PORT ITP_DEVICE_UART2
//#define TEST_PORT ITP_DEVICE_UART3

static sem_t UartSem;

static void UartCallback(void* arg1, uint32_t arg2)
{
	uint8_t getstr1[256];
	uint8_t sendtr1[8] = {0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
	int len = 0;
	static int totalcount =0;	

#if 1  // Turn on  ITP_IOCTL_REG_UART_CB  flag , should using these codes.
	sem_post(&UartSem);
#else  // Turn on ITP_IOCTL_REG_UART_DEFER_CB flag , should using these codes.
	len = read(TEST_PORT,getstr1+totalcount,256);
	totalcount += len;
	
	if(totalcount >= UartCommandLen)
	{	  
		write(TEST_PORT,sendtr1, 8);
		totalcount =0;
		memset(getstr1, 0, 256);
	}
#endif
}

void timer_isr(void* data)
{
    uint32_t timer = (uint32_t)data;
        
    ithTimerClearIntr(timer);
	sem_post(&UartSem);
	
}


static void UartIntrHandler(unsigned int pin, void *arg)
{
	ITHTimer timer = (ITHTimer)arg;
	//ithPrintf("timer=%d\n", timer);
	ithGpioClearIntr(pin);
	ithTimerCtrlDisable(timer, ITH_TIMER_EN);	
	ithTimerSetTimeout(timer, 1000);//us
	ithTimerCtrlEnable(timer, ITH_TIMER_EN);	
}

/*
static void UartIntrHandler2(void * arg)
{
	ITHUartPort port = (ITHUartPort)arg;
	ithUartClearIntr(port);
	ithPrintf("-- uart handle2 --\n");
}*/

void InitUartIntr(ITHUartPort port)
{

	ithEnterCritical();
#if 1
    ithGpioClearIntr(CFG_GPIO_UART0_RX);
    ithGpioSetIn(CFG_GPIO_UART0_RX);
    ithGpioRegisterIntrHandler(CFG_GPIO_UART0_RX, UartIntrHandler, ITH_TIMER5);
	
    ithGpioCtrlDisable(CFG_GPIO_UART0_RX, ITH_GPIO_INTR_LEVELTRIGGER);   /* use edge trigger mode */
    ithGpioCtrlEnable(CFG_GPIO_UART0_RX, ITH_GPIO_INTR_BOTHEDGE); /* both edge */    
    ithIntrEnableIrq(ITH_INTR_GPIO);
    ithGpioEnableIntr(CFG_GPIO_UART0_RX);
#else
	//printf("port = 0x%x\n", port);
	ithIntrDisableIrq(ITH_INTR_UART0);
    ithUartClearIntr(ITH_UART0);
    ithIntrClearIrq(ITH_INTR_UART0);

    ithIntrSetTriggerModeIrq(ITH_INTR_UART0, ITH_INTR_LEVEL);
    ithIntrRegisterHandlerIrq(ITH_INTR_UART0, UartIntrHandler2, (void *)ITH_UART0);
    ithUartEnableIntr(ITH_UART0, ITH_UART_RX_READY);

    /* Enable the interrupts. */
    ithIntrEnableIrq(ITH_INTR_UART0);
#endif
    ithExitCritical();
}

void InitTimer(ITHTimer timer, ITHIntr intr)
{
	
	ithTimerReset(timer);
	
	// Initialize Timer IRQ
	ithIntrDisableIrq(intr);
	ithIntrClearIrq(intr);

	// register Timer Handler to IRQ
	ithIntrRegisterHandlerIrq(intr, timer_isr, (void*)timer);

	// set Timer IRQ to edge trigger
	ithIntrSetTriggerModeIrq(intr, ITH_INTR_EDGE);

	// set Timer IRQ to detect rising edge
	ithIntrSetTriggerLevelIrq(intr, ITH_INTR_HIGH_RISING);

	// Enable Timer IRQ
	ithIntrEnableIrq(intr);

}



void* TestFuncUseDMA(void* arg)
{
	int len = 0;
	int count = 0;
	char getstr[256];
	
	
	itpRegisterDevice(TEST_PORT, &itpDeviceUart0);
	ioctl(TEST_PORT, ITP_IOCTL_RESET, CFG_UART0_BAUDRATE);
	printf("Start uart DMA test!\n");
	InitTimer(ITH_TIMER5, ITH_INTR_TIMER5);
	InitUartIntr(TEST_PORT);
	memset(getstr, 0, 256);
	sem_init(&UartSem, 0, 0);
	
	while(1)
	{
		sem_wait(&UartSem);
		
		len = read(TEST_PORT, getstr+count, 256);
		printf("len = %d\n", len);
	    count += len;
	    if(count >= UartCommandLen)
	    {
		    printf("uart read: %s,count=%d\n",getstr,count);
		    count =0;
		    memset(getstr, 0, 256);		
	    }
	}
}


void* TestFunc(void* arg)
{
	int i;
    char getstr[256];
    char sendtr[256];
	int len = 0;
	int count =0;

	printf("Start uart test!\n");

	sem_init(&UartSem, 0, 0);

    itpRegisterDevice(TEST_PORT, &itpDeviceUart0);
    ioctl(TEST_PORT, ITP_IOCTL_INIT, NULL);
#ifdef CFG_UART0_ENABLE
    ioctl(TEST_PORT, ITP_IOCTL_RESET, CFG_UART0_BAUDRATE);
#endif
	ioctl(TEST_PORT, ITP_IOCTL_REG_UART_CB, (void*)UartCallback);
	//ioctl(TEST_PORT, ITP_IOCTL_REG_UART_DEFER_CB , (void*)UartCallback);

	memset(getstr, 0, 256);
	memset(sendtr, 0, 256);
	
    while(1)
    {
        sem_wait(&UartSem);
	    len = read(TEST_PORT,getstr+count,256);
	    count += len;
	   	 
	    if(count >= UartCommandLen)
	    {
		    printf("uart read: %s,count=%d\n",getstr,count);
		    count =0;
		    sprintf(sendtr,"%s\n",getstr);		  
            write(TEST_PORT, sendtr, 256);	
		    memset(getstr, 0, 256);
	    }
	}
}

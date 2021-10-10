/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 *
 * @file (%project)/project/test_gpio/test_gpio.c
 *
 * @author Joseph Chang
 * @version 1.0.0
 *
 * example code for setting GPIO interrupt function
 */
#include "ite/itp.h"	//for ITH_GPIO_MODE0 & GPIO ith driver

static char g_GPIO_INTR = 0;

void _gpio_isr(void* data)
{
	g_GPIO_INTR = 1;
}

void _initGpioPin(int pin)
{
	ithGpioSetMode(pin, ITH_GPIO_MODE0);	//set the GPIO pin as GPIO mode(mode0)
	ithGpioSetIn(pin);						//set this GPIO pin as input
	ithGpioCtrlEnable(pin, ITH_GPIO_PULL_ENABLE); 	//To enable the PULL function of this GPIO pin
	ithGpioCtrlEnable(pin, ITH_GPIO_PULL_UP);		//To set "PULL UP" of this GPIO pin
	ithGpioEnable(pin);						//TO enable this GPIO pin
}


void _initGpioIntr(int pin)
{  
    ithEnterCritical();						//to prevent from interrupt(suggestion)
    ithGpioClearIntr(pin);					//clear intr
    ithGpioRegisterIntrHandler(pin, _gpio_isr, NULL);		//To register ISR of this GPIO pin
    ithGpioCtrlDisable(pin, ITH_GPIO_INTR_LEVELTRIGGER);		//Set interrupt mode as "edge trigger"
    ithGpioCtrlEnable(pin, ITH_GPIO_INTR_BOTHEDGE);				//Set interrupt edge mode as "both edge"
    ithIntrEnableIrq(ITH_INTR_GPIO);		//To enable the IRQ of GPIO
    ithGpioEnableIntr(pin); 				//To enable the interrupt of this GPIO pin
    ithExitCritical();						//unlock spinlock(suggestion)

}

void _initGpio(int pin)
{
	_initGpioPin(pin);	
	_initGpioIntr(pin);
}

void* TestFunc(void* arg)
{
	int gpioPin = 13;	//use GPIO13(touch panel INT pin) to check the GPIO input status
	int i = 0;

	_initGpio(gpioPin);
	
	while(1)
	{
		//polling "g_GPIO_INTR"
		//printf("polling \"g_GPIO_INTR\"\n");
		if(g_GPIO_INTR)
		{
			unsigned int gpioState = ithGpioGet(gpioPin) ? 1 : 0;
			printf("current GPIO[%d] state=%x, index=%d\n", gpioPin, gpioState);
			g_GPIO_INTR = 0;
			ithGpioClearIntr(gpioPin);
		}
		usleep(100*1000);
	}

	return NULL;
}

/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 *
 * @file (%project)/project/test_gpio/test_gpio.c
 *
 * @author Joseph Chang
 * @version 1.0.0
 *
 * example code for reading GPIO input status
 */
#include "ite/itp.h"	//for all ith driver (include GPIO) & MACRO

static void _gpioPinInit(int pin)
{
	ithGpioSetMode(pin, ITH_GPIO_MODE0);
	ithGpioSetIn(pin);
	ithGpioEnable(pin);
}

void* TestFunc(void* arg)
{
	int gpioPin = 13;	//use GPIO13(touch panel INT pin) to check the GPIO input status
	int lastPinStatus = 0;

	//initial GPIO
	_gpioPinInit(gpioPin);

	//
	while(1)
	{
		//polling gpio pin
		if(ithGpioGet(gpioPin) != lastPinStatus)
		{
			// GPIO status has changed
			lastPinStatus = ithGpioGet(gpioPin);
			if(lastPinStatus)
			{
				printf("The lasted GPIO state is HIGH\n");
			}
			else
			{
				printf("The lasted GPIO state is LOW\n");
			}
		}
	}

	return NULL;
}


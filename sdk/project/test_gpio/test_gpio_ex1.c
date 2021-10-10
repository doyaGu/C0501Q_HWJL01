/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 *
 * @file (%project)/project/test_gpio/test_gpio_ex1.c
 *
 * @author Joseph Chang
 * @version 1.0.0
 *
 * example code for setting GPIO output status
 */
#include "ite/itp.h"	//for all ith driver (include GPIO) & MACRO

void* TestFunc(void* arg)
{
	int gpioPin = 4;	//GPIO 4 is LCD backlight @ doorbell indoor machine
	int i = 0;
	
	printf("GPIO test start~~~\n");
	
	//initial GPIO
	ithGpioSetOut(gpioPin);
	ithGpioSetMode(gpioPin, ITH_GPIO_MODE0);
	
	//
	while(1)
	{
		if(i++&0x1)
		{
			ithGpioClear(gpioPin);
		}
		else
		{
			ithGpioSet(gpioPin);
		}
		printf("current GPIO[%d] state=%x, index=%d\n",gpioPin,ithGpioGet(gpioPin),i);
		usleep(1000*1000);	//wait for 1 second
	}

	return NULL;
}

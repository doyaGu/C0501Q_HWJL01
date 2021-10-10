/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL PWM functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"
#include <pthread.h>

static uint32_t blCounts[6], blMatchs[6];

static unsigned int oldmatch1 = 0;
static pthread_mutex_t PWM_MUTEX  = PTHREAD_MUTEX_INITIALIZER;

void ithPwmInit(ITHPwm pwm, unsigned int freq, unsigned int duty)
{
    blCounts[pwm]   = ithGetBusClock() / freq;
    blMatchs[pwm]    = (uint64_t) blCounts[pwm] * duty / 100;
}

void ithPwmReset(ITHPwm pwm, unsigned int pin, unsigned int gpio_mode)
{
    ithGpioSetMode(pin, gpio_mode);
    ithTimerReset(pwm);
    ithTimerSetCounter(pwm, 0);
    ithTimerSetPwmMatch(pwm, blMatchs[pwm], blCounts[pwm]);

    ithTimerCtrlEnable(pwm, ITH_TIMER_UPCOUNT);
    ithTimerCtrlEnable(pwm, ITH_TIMER_PERIODIC);
    ithTimerCtrlEnable(pwm, ITH_TIMER_PWM);
    ithTimerEnable(pwm);
}

void ithPwmSetDutyCycle(ITHPwm pwm, unsigned int duty)
{
	uint32_t count, newmatch1,newcounter;
	
	ithTimerCtrlDisable(pwm, ITH_TIMER_EN);
	count = ithTimerGetCounter(pwm);	        
	newmatch1 = ((uint64_t) blCounts[pwm] * duty / 100);
	
	pthread_mutex_lock(&PWM_MUTEX);
	if ((oldmatch1 > count) && count > newmatch1)
	{		
		if (newmatch1 - (oldmatch1 - count) >= 0)
		{
			newcounter = newmatch1 - (oldmatch1 - count);			
		}
		else
		{
			newcounter =  blCounts[pwm] - ((oldmatch1 - count)- newmatch1);			
		}
		
    	ithTimerSetPwmMatch(pwm, newmatch1, blCounts[pwm]);
		ithTimerSetCounter(pwm, newcounter);				
	}
	else
		ithTimerSetPwmMatch(pwm, newmatch1, blCounts[pwm]);

	ithTimerCtrlEnable(pwm, ITH_TIMER_EN);
	oldmatch1 = newmatch1;
	pthread_mutex_unlock(&PWM_MUTEX);
}

void ithPwmEnable(ITHPwm pwm, unsigned int pin, unsigned int gpio_mode)
{
    ithGpioSetMode(pin, gpio_mode);
    ithTimerCtrlEnable(pwm, ITH_TIMER_PWM);
    ithTimerCtrlEnable(pwm, ITH_TIMER_EN);
}

void ithPwmDisable(ITHPwm pwm, unsigned int pin)
{
    ithGpioClear(pin);
    ithGpioEnable(pin);
    ithGpioSetOut(pin);
    ithTimerCtrlDisable(pwm, ITH_TIMER_EN);
    ithTimerCtrlDisable(pwm, ITH_TIMER_PWM);
}

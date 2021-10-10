#include <string.h>
#include "mmio.h"
#include "ith_risc.h"

///////////////////////////////////////////////////////////////////////////
unsigned long wg0_cardTempHigh2 = 0; // 93bits
unsigned long wg0_cardTempHigh = 0;  // 62bits
unsigned long wg0_cardTemp = 0;      // 31bits
int wg0_lastWiegand;
int wg0_bitCount = 0;	

unsigned long wg1_cardTempHigh2 = 0; // 93bits
unsigned long wg1_cardTempHigh = 0;  // 62bits
unsigned long wg1_cardTemp = 0;      // 31bits
int wg1_lastWiegand;
int wg1_bitCount = 0;

int w0_gpio0 = -1;
int w0_gpio1 = -1; 
int w1_gpio0 = -1;
int w1_gpio1 = -1;

void wg0_begin(int gpiod0, int gpiod1)
{
	wg0_cardTempHigh = 0;
	wg0_cardTemp = 0;
	wg0_bitCount = 0;  
	
	// set to input mode
	ithGpioSetIn(gpiod0);
	ithGpioSetIn(gpiod1);
	
	// set to edge trigger
	ithClearRegBitA(ITH_GPIO_BASE + ITH_GPIO1_INTRTRIG_REG, gpiod0);
	ithClearRegBitA(ITH_GPIO_BASE + ITH_GPIO1_INTRTRIG_REG, gpiod1);

	// set to single edge
	ithClearRegBitA(ITH_GPIO_BASE + 0x34, gpiod0);
    ithClearRegBitA(ITH_GPIO_BASE + 0x34, gpiod1);

	ithClearRegBitA(ITH_GPIO_BASE + 0x38, gpiod0);
	ithClearRegBitA(ITH_GPIO_BASE + 0x38, gpiod1);

	// set IntrMask 0
	ithSetRegBitA(ITH_GPIO_BASE + 0x28, gpiod0);
	ithSetRegBitA(ITH_GPIO_BASE + 0x28, gpiod1);
	
    // set bounce timer
    ithGpioSetDebounceClock(18000);

    // Enablebounce
    ithGpioEnableBounce(gpiod0);
    ithGpioEnableBounce(gpiod1);

    ithGpioEnableIntr(gpiod0);
	ithGpioEnableIntr(gpiod1);

	ithGpioClearIntr(gpiod0);
	ithGpioClearIntr(gpiod1);
	
	w0_gpio0 = gpiod0;
	w0_gpio1 = gpiod1;
}

void wg1_begin(int gpiod0, int gpiod1)
{
	wg1_cardTempHigh = 0;
	wg1_cardTemp = 0;
	wg1_bitCount = 0;  
		
	// set to input mode
	ithGpioSetIn(gpiod0);
	ithGpioSetIn(gpiod1);
	
	// set to edge trigger
	ithClearRegBitA(ITH_GPIO_BASE + ITH_GPIO1_INTRTRIG_REG, gpiod0);
	ithClearRegBitA(ITH_GPIO_BASE + ITH_GPIO1_INTRTRIG_REG, gpiod1);

	// set to single edge
	ithClearRegBitA(ITH_GPIO_BASE + 0x34, gpiod0);
    ithClearRegBitA(ITH_GPIO_BASE + 0x34, gpiod1);

	// set to rising edge
	ithClearRegBitA(ITH_GPIO_BASE + 0x38, gpiod0);
	ithClearRegBitA(ITH_GPIO_BASE + 0x38, gpiod1);

	// set IntrMask 0
	ithSetRegBitA(ITH_GPIO_BASE + 0x28, gpiod0);
	ithSetRegBitA(ITH_GPIO_BASE + 0x28, gpiod1);
	
	ithGpioEnableIntr(gpiod0);
	ithGpioEnableIntr(gpiod1);

    // set bounce timer
    ithGpioSetDebounceClock(18000);

    // Enablebounce
    ithGpioEnableBounce(gpiod0);
    ithGpioEnableBounce(gpiod1);

	ithGpioClearIntr(gpiod0);
	ithGpioClearIntr(gpiod1);
	
	w1_gpio0 = gpiod0;
	w1_gpio1 = gpiod1;
}

void wg0_ReadD0()
{
	wg0_bitCount++; // Increament bit count for Interrupt connected to D0
	if (wg0_bitCount>62) // If bit count more than 62, process high bits
	{
		wg0_cardTempHigh2 |= ((0x80000000 & wg0_cardTempHigh)>>31);	// shift value to high bits
		wg0_cardTempHigh2 <<= 1;
		wg0_cardTempHigh |= ((0x80000000 & wg0_cardTemp)>>31); // shift value to high bits
		wg0_cardTempHigh <<= 1;
		wg0_cardTemp <<=1;
	}
	else if (wg0_bitCount>31) // If bit count more than 31, process high bits
	{
		wg0_cardTempHigh |= ((0x80000000 & wg0_cardTemp)>>31); // shift value to high bits
		wg0_cardTempHigh <<= 1;
		wg0_cardTemp <<=1;
	}
	else
	{
		wg0_cardTemp <<= 1; // D0 represent binary 0, so just left shift card data
	}
}

void wg0_ReadD1()
{
	wg0_bitCount++; // Increment bit count for Interrupt connected to D1
	if (wg0_bitCount>62) // If bit count more than 62, process high bits
	{
		wg0_cardTempHigh2 |= ((0x80000000 & wg0_cardTempHigh)>>31); // shift value to high bits
		wg0_cardTempHigh2 <<= 1;
		wg0_cardTempHigh |= ((0x80000000 & wg0_cardTemp)>>31); // shift value to high bits
		wg0_cardTempHigh <<= 1;
		wg0_cardTemp |= 1;
		wg0_cardTemp <<=1;
	}
	else if (wg0_bitCount>31) // If bit count more than 31, process high bits
	{
		wg0_cardTempHigh |= ((0x80000000 & wg0_cardTemp)>>31); // shift value to high bits
		wg0_cardTempHigh <<= 1;
		wg0_cardTemp |= 1;
		wg0_cardTemp <<=1;
	}
	else
	{
		wg0_cardTemp |= 1; // D1 represent binary 1, so OR card data with 1 then
		wg0_cardTemp <<= 1; // left shift card data
	}
}

void wg1_ReadD0()
{
	wg1_bitCount++; // Increament bit count for Interrupt connected to D0
	if (wg1_bitCount>62) // If bit count more than 62, process high bits
	{
		wg1_cardTempHigh2 |= ((0x80000000 & wg1_cardTempHigh)>>31);	// shift value to high bits
		wg1_cardTempHigh2 <<= 1;
		wg1_cardTempHigh |= ((0x80000000 & wg1_cardTemp)>>31); // shift value to high bits
		wg1_cardTempHigh <<= 1;
		wg1_cardTemp <<=1;
	}
	else if (wg1_bitCount>31) // If bit count more than 31, process high bits
	{
		wg1_cardTempHigh |= ((0x80000000 & wg1_cardTemp)>>31); // shift value to high bits
		wg1_cardTempHigh <<= 1;
		wg1_cardTemp <<=1;
	}
	else
	{
		wg1_cardTemp <<= 1; // D0 represent binary 0, so just left shift card data
	}	
}

void wg1_ReadD1()
{
	wg1_bitCount++; // Increment bit count for Interrupt connected to D1
	if (wg1_bitCount>62) // If bit count more than 62, process high bits
	{
		wg1_cardTempHigh2 |= ((0x80000000 & wg1_cardTempHigh)>>31);	// shift value to high bits
		wg1_cardTempHigh2 <<= 1;
		wg1_cardTempHigh |= ((0x80000000 & wg1_cardTemp)>>31); // shift value to high bits
		wg1_cardTempHigh <<= 1;
		wg1_cardTemp |= 1;
		wg1_cardTemp <<=1;
	}
	else if (wg1_bitCount>31) // If bit count more than 31, process high bits
	{
		wg1_cardTempHigh |= ((0x80000000 & wg1_cardTemp)>>31); // shift value to high bits
		wg1_cardTempHigh <<= 1;
		wg1_cardTemp |= 1;
		wg1_cardTemp <<=1;
	}
	else
	{
		wg1_cardTemp |= 1; // D1 represent binary 1, so OR card data with 1 then
		wg1_cardTemp <<= 1; // left shift card data
	}
}

int wg0_DoWiegandConversion(unsigned long *card_id)
{
    int result = 0;
	int elapsedTime;
	volatile unsigned long gpioReg0, gpioReg1;	

    if (w0_gpio0 == -1 || w0_gpio1 == -1)
        return result;
    
    if (w0_gpio0 < 32)
        gpioReg0 = ithReadRegA(ITH_GPIO_BASE + ITH_GPIO1_INTRRAWSTATE_REG);
    else    
        gpioReg0 = ithReadRegA(ITH_GPIO_BASE + ITH_GPIO2_INTRRAWSTATE_REG);
        
    if (w0_gpio1 < 32)
        gpioReg1 = ithReadRegA(ITH_GPIO_BASE + ITH_GPIO1_INTRRAWSTATE_REG);
    else    
        gpioReg1 = ithReadRegA(ITH_GPIO_BASE + ITH_GPIO2_INTRRAWSTATE_REG);    

	if (gpioReg0 & (1 << w0_gpio0))
	{
	   wg0_ReadD0();
	   ithGpioClearIntr(w0_gpio0);	
	   wg0_lastWiegand = get_wiegand_timer(1); // timer 0 used by system, do not use timer 0 
	}
	if (gpioReg1 & (1 << w0_gpio1))
	{	   
	   wg0_ReadD1();
	   ithGpioClearIntr(w0_gpio1);
	   wg0_lastWiegand = get_wiegand_timer(1); // timer 0 used by system, do not use timer 0 
	}

    elapsedTime = (get_wiegand_timer(1) - wg0_lastWiegand)/(PalGetSysClock()/1000);
    if (elapsedTime > 25) // if no more signal coming through after 25ms
    {
        if (wg0_lastWiegand)
	{	
	    wg0_cardTemp >>= 1; // shift right 1 bit to get back the real value - interrupt done 1 left shift in advance
		if (wg0_bitCount>32) // bit count more than 32 bits, shift high bits right to make adjustment
			{
			wg0_cardTempHigh >>= 1;	
	            wg0_cardTemp |= (wg0_cardTempHigh & 0x1) << 31 ;
				wg0_cardTempHigh = (wg0_cardTempHigh & 0xFFFFFFFE) >> 1;
			}
	
		card_id[0] = wg0_cardTempHigh; // high 32 bits
		card_id[1] = wg0_cardTemp; // low 32 bits
		card_id[4] = wg0_bitCount;
		
		wg0_bitCount = 0;
		wg0_cardTemp = 0;
		wg0_cardTempHigh = 0;

        wg0_lastWiegand = 0;
		result = 1;
	}
		reset_wiegand_timer(1);
	}
		
    return result;
}

int wg1_DoWiegandConversion(unsigned long *card_id)
{
    int result = 0;
	int elapsedTime;
	volatile unsigned long gpioReg0, gpioReg1;

    if (w1_gpio0 == -1 || w1_gpio1 == -1)
        return result;

	if (w1_gpio0 < 32)
		gpioReg0 = ithReadRegA(ITH_GPIO_BASE + ITH_GPIO1_INTRRAWSTATE_REG);
	else
		gpioReg0 = ithReadRegA(ITH_GPIO_BASE + ITH_GPIO2_INTRRAWSTATE_REG);

	if (w1_gpio1 < 32)
		gpioReg1 = ithReadRegA(ITH_GPIO_BASE + ITH_GPIO1_INTRRAWSTATE_REG);
	else
		gpioReg1 = ithReadRegA(ITH_GPIO_BASE + ITH_GPIO2_INTRRAWSTATE_REG);

	if (gpioReg0 & (1 << w1_gpio0))
	{
	   wg1_ReadD0();
	   ithGpioClearIntr(w1_gpio0);	
	   wg1_lastWiegand = get_wiegand_timer(2); // timer 0 used by system, do not use timer 0 
	}
	if (gpioReg1 & (1 << w1_gpio1))
	{
	   wg1_ReadD1();
	   ithGpioClearIntr(w1_gpio1);
	   wg1_lastWiegand = get_wiegand_timer(2); // timer 0 used by system, do not use timer 0 
	}

    elapsedTime = (get_wiegand_timer(2) - wg1_lastWiegand)/(PalGetSysClock()/1000);
    if (elapsedTime > 25) // if no more signal coming through after 25ms
    {
        if (wg1_lastWiegand)
	{	    		
	    wg1_cardTemp >>= 1;	// shift right 1 bit to get back the real value - interrupt done 1 left shift in advance
		if (wg1_bitCount>32) // bit count more than 32 bits, shift high bits right to make adjustment
			{
			wg1_cardTempHigh >>= 1;	
	            wg1_cardTemp |= (wg1_cardTempHigh & 0x1) << 31 ;
				wg1_cardTempHigh = (wg1_cardTempHigh & 0xFFFFFFFE) >> 1;
			}
	
		card_id[2] = wg1_cardTempHigh; // high 32 bits
		card_id[3] = wg1_cardTemp; // low 32 bits
		card_id[5] = wg1_bitCount;
		
		wg1_bitCount = 0;
		wg1_cardTemp = 0;
		wg1_cardTempHigh = 0;
        wg1_lastWiegand = 0;
		result = 1;
	}
		reset_wiegand_timer(2);
	}

    return result;
}

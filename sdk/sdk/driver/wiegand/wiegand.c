#include "wiegand.h"
#include <sys/time.h>
#include "ite/ith.h"
#include "wiegand/wiegand.h"


#define WIEGAND_BASE    0xDE300000
#define WIEGAND_UART_BAUD 9600

#define WM0             0x000
#define WM1             0x010
#define WSR0            0x004
#define WSR1            0x014
#define WUSR0           0x008
#define WUSR1           0x018
#define WIR0       		0x00C

#define WIEGAND_RST_ENABLE		(1 << 31)
#define WIEGAND_RST_DISABLE 	(0 << 31)
#define WIEGAND_RESET_MASK  	(1 << 31)
#define WIEGAND_D0_SEL_MASK	 	0x7F
#define WIEGAND_D1_SEL_MASK		(0x7F << 8)



typedef struct WIEGAND_OBJECT_TAG
{
  WIEGANDID id;  
}WIEGAND_OBJECT;


static void _wiegand_reset(WIEGANDID id)
{
	ithWriteRegMaskA(WIEGAND_BASE + id*0x010, WIEGAND_RST_ENABLE, WIEGAND_RESET_MASK);
	usleep(1000);
	ithWriteRegMaskA(WIEGAND_BASE + id*0x010, WIEGAND_RST_DISABLE, WIEGAND_RESET_MASK);
}

static void _wiegand_set_pins(WIEGANDID id, int d0pin, int d1pin)
{		
	ithWriteRegMaskA(WIEGAND_BASE + id*0x010, d0pin, WIEGAND_D0_SEL_MASK);
	ithWriteRegMaskA(WIEGAND_BASE + id*0x010, d1pin << 8, WIEGAND_D1_SEL_MASK);
}

static void _wiegand_controller_enable(WIEGANDID id, int enable)
{
	if (enable)
		ithWriteRegMaskA(WIEGAND_BASE + id*0x010, (1 << 30), (1 << 30));
	else
		ithWriteRegMaskA(WIEGAND_BASE + id*0x010, (0 << 30), (1 << 30));
}

static void _wiegand_parity_enable(WIEGANDID id, int enable)
{
	if (enable)
		ithWriteRegMaskA(WIEGAND_BASE + id*0x010, (1 << 29), (1 << 29));
	else
		ithWriteRegMaskA(WIEGAND_BASE + id*0x010, (0 << 29), (1 << 29));
}

static void _wiegand_set_polarity(WIEGANDID id, int polarity)
{
	if (polarity)
		ithWriteRegMaskA(WIEGAND_BASE + id*0x010, (1 << 28), (1 << 28));
	else
		ithWriteRegMaskA(WIEGAND_BASE + id*0x010, (0 << 28), (1 << 28));

}

static void _wiegand_set_prescale(WIEGANDID id, int value)
{
	ithWriteRegMaskA(WIEGAND_BASE + WSR0 + id*0x010, (value << 16), (0x1FFF << 16));
}

static void _wiegand_set_timeout(WIEGANDID id, int value)
{
	ithWriteRegMaskA(WIEGAND_BASE + WSR0 + id*0x010, value, 0x1FFF);
}

static void _wiegand_set_debounce(WIEGANDID id, int value)
{
	ithWriteRegMaskA(WIEGAND_BASE + WSR0 + id*0x010, (value << 30), (0x3 << 30));
}

static void _wiegand_set_uartdiv(WIEGANDID id, int value)
{
	ithWriteRegMaskA(WIEGAND_BASE + WUSR0 + id*0x010, value, 0xFFFF);
}

static void _wiegand_set_uartprescale(WIEGANDID id, int value)
{
	ithWriteRegMaskA(WIEGAND_BASE + WUSR0 + id*0x010, (value << 16), (0x1F << 16));
}

static void _wiegand_set_intr_clear (WIEGANDID id, int mode)
{
	if (mode)
		ithWriteRegMaskA(WIEGAND_BASE + WIR0 + id*0x010, (1 << 31), (1 << 31));
	else
		ithWriteRegMaskA(WIEGAND_BASE + WIR0 + id*0x010, (0 << 31), (1 << 31));
}

static void _wiegand_set_probsel(WIEGANDID id, int mode)
{
	ithWriteRegMaskA(WIEGAND_BASE + WIR0 + id*0x010, (mode  << 6), (0x7 << 6));
}
	

static void _wiegand_set_mode(WIEGANDID id, int mode)
{
	ithWriteRegMaskA(WIEGAND_BASE + id*0x010, (mode << 16), (0x7 << 16));
}

static void _wiegand_set_uart2(void)
{
	uint32_t uartdiv, busclk;

	busclk = ithGetBusClock();
	uartdiv = busclk/(16*WIEGAND_UART_BAUD);

	//Uart2 Rx Source from wg0
	ithWriteRegMaskA(ITH_GPIO_BASE+0x1A0, (1 << 30), (1 << 30));
	
	ithWriteRegA(ITH_UART2_BASE + ITH_UART_LCR_REG, 0x0080);
	ithWriteRegA(ITH_UART2_BASE + ITH_UART_IIR_REG, 0x0001);
	ithWriteRegA(ITH_UART2_BASE + ITH_UART_THR_REG, uartdiv&0xFF);
	ithWriteRegA(ITH_UART2_BASE + ITH_UART_IER_REG, (uartdiv & 0xF00) >> 8);
	ithWriteRegA(ITH_UART2_BASE + ITH_UART_IIR_TXFIFOFULL, 0x000d);
	ithWriteRegA(ITH_UART2_BASE + ITH_UART_LCR_REG, 0x0003);
	ithWriteRegA(ITH_UART2_BASE + ITH_UART_FCR_REG, 0x0007);
	ithWriteRegA(ITH_UART2_BASE + ITH_UART_IER_REG, 0x0002);
	ithWriteRegA(ITH_UART2_BASE + 0x24, 0x0003);

}

static void _wiegand_set_uart3(void)
{
	uint32_t uartdiv, busclk;

	busclk = ithGetBusClock();
	uartdiv = busclk/(16*WIEGAND_UART_BAUD);

	//uart3 Rx Source from wg1
	ithWriteRegMaskA(ITH_GPIO_BASE+0x1A0, (1 << 31), (1 << 31));
	
	ithWriteRegA(ITH_UART3_BASE + ITH_UART_LCR_REG, 0x0080);
	ithWriteRegA(ITH_UART3_BASE + ITH_UART_IIR_REG, 0x0001);
	ithWriteRegA(ITH_UART3_BASE + ITH_UART_THR_REG, uartdiv&0xFF);
	ithWriteRegA(ITH_UART3_BASE + ITH_UART_IER_REG, (uartdiv & 0xF00) >> 8);
	ithWriteRegA(ITH_UART3_BASE + ITH_UART_IIR_TXFIFOFULL, 0x000d);
	ithWriteRegA(ITH_UART3_BASE + ITH_UART_LCR_REG, 0x0003);
	ithWriteRegA(ITH_UART3_BASE + ITH_UART_FCR_REG, 0x0007);
	ithWriteRegA(ITH_UART3_BASE + ITH_UART_IER_REG, 0x0002);
	ithWriteRegA(ITH_UART3_BASE + 0x24, 0x0003);
}


void wiegand_controller_enable(WIEGANDID id, int d0pin, int d1pin)
{
	if (id == wiegand_0)
	{		
		_wiegand_set_uart2();
		_wiegand_set_pins(id, d0pin, d1pin);//wg0 88,89
		_wiegand_controller_enable(id, 1);		
	}
	else
	{		
		_wiegand_set_uart3();
		_wiegand_set_pins(id, d0pin, d1pin);//wg1 86, 87
		_wiegand_controller_enable(id, 1);		
	}
}

void wiegand_set_bitcnt(WIEGANDID id, int bitcnt)
{
	int mode = 0;
	switch(bitcnt)
	{
	case 26:
		mode = 0x0;
		break;
	case 34:
		mode = 0x1;
		break;
	case 37:
		mode = 0x2;
		break;
	default:
		mode = 0x4; //auto detect
		break;
	}
	//printf("mode = %d\n", mode);
	_wiegand_set_mode(id, mode);
}

void wiegand_suspend(WIEGANDID id)
{
	
	uint32_t uartdiv, wuartdiv, prescale, timeout, busclk;


	busclk = ithGetBusClock();
	wuartdiv = busclk/WIEGAND_UART_BAUD;	
	prescale = (10 * busclk)/1000000;	
	uartdiv = busclk/(16*WIEGAND_UART_BAUD);

	_wiegand_set_uartdiv(id, wuartdiv); 
	_wiegand_set_prescale(id, prescale);
	
	if (id == wiegand_0)
	{
		ithWriteRegA(ITH_UART2_BASE + ITH_UART_LCR_REG, 0x0080);		
		ithWriteRegA(ITH_UART2_BASE + ITH_UART_THR_REG, uartdiv&0xFF); 
		ithWriteRegA(ITH_UART2_BASE + ITH_UART_IER_REG, (uartdiv & 0xF00) >> 8);
		ithWriteRegA(ITH_UART2_BASE + ITH_UART_LCR_REG, 0x0003);
	}
	else
	{
		ithWriteRegA(ITH_UART3_BASE + ITH_UART_LCR_REG, 0x0080);		
		ithWriteRegA(ITH_UART3_BASE + ITH_UART_THR_REG, uartdiv&0xFF);
		ithWriteRegA(ITH_UART3_BASE + ITH_UART_IER_REG, (uartdiv & 0xF00) >> 8);
		ithWriteRegA(ITH_UART3_BASE + ITH_UART_LCR_REG, 0x0003);
	}
}

void wiegand_resume(WIEGANDID id)
{
	uint32_t uartdiv, wuartdiv, prescale, timeout, busclk;
	
	busclk = ithGetBusClock();
	wuartdiv = busclk/WIEGAND_UART_BAUD;	
	prescale = (10 * busclk)/1000000;	
	uartdiv = busclk/(16*WIEGAND_UART_BAUD);

	_wiegand_set_uartdiv(id, wuartdiv); 
	_wiegand_set_prescale(id, prescale);
	
	if (id == wiegand_0)
	{
		ithWriteRegA(ITH_UART2_BASE + ITH_UART_LCR_REG, 0x0080);		
		ithWriteRegA(ITH_UART2_BASE + ITH_UART_THR_REG, uartdiv&0xFF); 
		ithWriteRegA(ITH_UART2_BASE + ITH_UART_IER_REG, (uartdiv & 0xF00) >> 8); 
		ithWriteRegA(ITH_UART2_BASE + ITH_UART_LCR_REG, 0x0003);
	}
	else
	{
		ithWriteRegA(ITH_UART3_BASE + ITH_UART_LCR_REG, 0x0080);		
		ithWriteRegA(ITH_UART3_BASE + ITH_UART_THR_REG, uartdiv&0xFF);
		ithWriteRegA(ITH_UART3_BASE + ITH_UART_IER_REG, (uartdiv & 0xF00) >> 8);
		ithWriteRegA(ITH_UART3_BASE + ITH_UART_LCR_REG, 0x0003);
	}
}

void init_wiegand_controller(WIEGANDID id)
{
	
	uint32_t busclk, prescale, uartdiv;
	
	busclk = ithGetBusClock();
	uartdiv = busclk/WIEGAND_UART_BAUD;	
	prescale = (10 * busclk)/1000000;
		
	_wiegand_reset(id);
	_wiegand_parity_enable(id, 1);
	_wiegand_set_prescale(id, prescale);
	_wiegand_set_uartdiv(id, uartdiv);
	_wiegand_set_timeout(id, 0x012C);	
	_wiegand_set_debounce(id, 0x1);	
	_wiegand_set_intr_clear(id, 1);
	_wiegand_set_probsel(id, 0x2);
	wiegand_set_bitcnt(id, 0x01);		
		
}


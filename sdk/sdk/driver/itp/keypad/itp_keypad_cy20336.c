/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * Castor3 keypad module.
 *
 * @author Joseph Chang
 * @version 1.0
 */
#include "../itp_cfg.h"
#include "iic/mmp_iic.h"
#include <errno.h>
#include <pthread.h>

#include "openrtos/FreeRTOS.h"		//must "include FreeRTOS.h" before "include queue.h"
#include "openrtos/queue.h"			//for using xQueue function



/**************************************************************************
** MACRO defination                                                      **
***************************************************************************/
#ifndef	CFG_TOUCH_KEY_NUM
#define CFG_TOUCH_KEY_NUM	(8)
#endif

//#define CY20336_IIC_ADDR 		(0x58>>1)
#define CY20336_IIC_ADDR 		0x00 //(0x58>>1)
//#define CY20336_IIC_ADDR 		0x68 //(0x58>>1)




#define CY_TOUCH_REG_ADRRESS 		0x00 //0x01 //(0x58>>1)
#define CY_LIGHT_REG_ADRRESS 		0x01


#define ENABLE_KEYPAD_DBG_MODE
#define ENABLE_KEYPAD_INT
//#define ENABLE_SHARE_GPIOEXTENDER_INTR
//#define ENABLE_CUTOFF_ALL_INTR
#define ENABLE_CLEAR_TP_INT

#ifdef	ENABLE_KEYPAD_INT
#define TK_GPIO_PIN	    CFG_GPIO_KEYPAD

#if (TK_GPIO_PIN<32)
#define TK_GPIO_MASK    (1<<TK_GPIO_PIN)
#else
#define TK_GPIO_MASK    (1<<(TK_GPIO_PIN-32))
#endif

#endif

#define QUEUE_LEN 256
/**************************************************************************
** global variable                                                      **
***************************************************************************/
//IT7235BR touch 16-key pad mapping table
//  x   80   40   02   01
//y	 ----------------------
//04 | [1]  [2]  [3]  [A] |
//08 | [4]  [5]  [6]  [B] |
//10 | [7]  [8]  [9]  [C] |
//20 | [*]  [0]  [#] [Cen]|
//	 ----------------------
//kay:: 	[0]  [1]  [2]  [3]  [4]  [5]  [6]  [7]  [8]  [9]  [A]  [B]  [C]  [Cen] [*]  [#]
//code::   	0x60,0x84,0x44,0x02,0x88,0x48,0x0a,0x90,0x50,0x12,0x05,0x09,0x11,0x21,0xa0,0x22
//SDL code: 13  ,0,  ,1   ,2   ,4   ,5   ,6   ,8   ,9   ,10  ,3   ,7   ,11  ,15   ,12, ,14 
//x:0x80,0x40,0x02,0x01
//y:0x04,0x08,0x10,0x20

static	pthread_mutex_t     keypad_mutex = PTHREAD_MUTEX_INITIALIZER;

static	const uint8_t gTotalTouchKeyNum = (uint8_t)(CFG_TOUCH_KEY_NUM);

static	uint8_t		gRegPage=0xFE;
static	uint8_t		gLastIicStatus=0;



static	uint8_t		gLastIndex = 0xFF;

/**************************************************************************
** private function                                                      **
***************************************************************************/
static uint8_t _checkSum(uint16_t sum)
{
	uint8_t	i;
	uint8_t	cnt=0;
	
	for(i=0; i<gTotalTouchKeyNum; i++)
	{
		if( (sum>>i)&0x01 )	cnt++;
	}
	return cnt;
}

#ifdef	ENABLE_KEYPAD_INT
static void _initTkGpioPin(void)
{
	ithGpioSetMode(TK_GPIO_PIN, ITH_GPIO_MODE1);
	ithGpioSetIn(TK_GPIO_PIN);
	ithGpioCtrlEnable(TK_GPIO_PIN, ITH_GPIO_PULL_ENABLE);
	ithGpioCtrlEnable(TK_GPIO_PIN, ITH_GPIO_PULL_UP);
	ithGpioEnable(TK_GPIO_PIN);	     
}
#endif




static uint16_t _getTouchKey_cy(void)
{
	uint16_t	KeyValue=0x00;
	unsigned int result = 0;
	uint8_t		buf_data =0;
	uint8_t		buf_reg =0;

	static uint8_t light_status=0x01;

/*
uint32_t
mmpIicReceiveData(
	IIC_PORT 	port,
    IIC_OP_MODE mode,
    uint8_t 	slaveAddr,
    uint8_t*	outData,
    uint32_t	outDataSize,
	uint8_t*	inData,
	uint32_t	inDataSize)
*/

//	result = mmpIicReceiveData(IIC_MASTER_MODE, CY20336_IIC_ADDR, CY_TOUCH_REG_ADRRESS, &buf, 1);
	buf_reg = CY_TOUCH_REG_ADRRESS;
	result = mmpIicReceiveData(IIC_PORT_1, IIC_MASTER_MODE, CY20336_IIC_ADDR, &buf_reg, 1, &buf_data, 1);
	if(result)	
	{
		printf("Err1\n"); 
		return 0;
	}
	KeyValue = (uint16_t) buf_data;    
	//printf("[xx=] %x, key=%x ",buf_data,KeyValue);

  	return KeyValue ; 
#if 0
	result = mmpIicReceiveData(IIC_MASTER_MODE, CY20336_IIC_ADDR, CY_LIGHT_REG_ADRRESS, &buf, 1);
		if(result)	printf("Err2\n");

      printf("[KEYPAD]light buf =%x light_status =%x \n",buf,light_status); 

	result = mmpIicSendData(IIC_MASTER_MODE, CY20336_IIC_ADDR, CY_LIGHT_REG_ADRRESS, &light_status, 1); 
      printf("[KEYPAD]send:  result=%x.\n",result); 
	light_status = light_status<<1; 
	if(	0 == light_status )
	{
		light_status = 0x01;
	}

	return 0;
#endif

}

void itp_cy_keypad_light_off_all()
{
	unsigned int result = 0;
	uint8_t light_status=0x00;
    pthread_mutex_lock(&keypad_mutex);
//	result = mmpIicSendData(IIC_MASTER_MODE, CY20336_IIC_ADDR, CY_LIGHT_REG_ADRRESS, &light_status, 1); 

	result = mmpIicSendData(IIC_PORT_1,IIC_MASTER_MODE, CY20336_IIC_ADDR, CY_LIGHT_REG_ADRRESS ,&light_status, 1); 

    pthread_mutex_unlock(&keypad_mutex);     

}


void itp_cy_keypad_light_on_all()
{
	unsigned int result = 0;
	uint8_t light_status=0xff;
    pthread_mutex_lock(&keypad_mutex);
//	result = mmpIicSendData(IIC_MASTER_MODE, CY20336_IIC_ADDR, CY_LIGHT_REG_ADRRESS, &light_status, 1); 

	result = mmpIicSendData(IIC_PORT_1,IIC_MASTER_MODE, CY20336_IIC_ADDR, CY_LIGHT_REG_ADRRESS ,&light_status, 1); 
	

    pthread_mutex_unlock(&keypad_mutex);     

}

void itp_cy_keypad_light_on_off(unsigned int led_num, bool on)
{
	unsigned int result = 0;
	uint8_t light_status=0xff;
	unsigned int mask= 0x1;
    pthread_mutex_lock(&keypad_mutex);
	uint8_t buf_reg;
	buf_reg = CY_LIGHT_REG_ADRRESS;
	result = mmpIicReceiveData(IIC_PORT_1, IIC_MASTER_MODE, CY20336_IIC_ADDR, &buf_reg, 1, &light_status, 1);

//	result = mmpIicReceiveData(IIC_MASTER_MODE, CY20336_IIC_ADDR, CY_LIGHT_REG_ADRRESS, &light_status, 1);
	    printf("[LED1]-status %x led_num %d \n,on=%d",light_status,led_num,on);

if(on)
{
	light_status|=(mask<<led_num);
}
else
{
	light_status&=~(mask<<led_num);
}
//	    printf("[LED2]-status %x\n",light_status);
	buf_reg = CY_LIGHT_REG_ADRRESS;
	result = mmpIicSendData(IIC_PORT_1,IIC_MASTER_MODE, CY20336_IIC_ADDR, buf_reg ,&light_status, 1); 
/*
mmpIicSendData(
	IIC_PORT 	port,
    IIC_OP_MODE mode,
    uint8_t 	slaveAddr,
    uint8_t 	regAddr,
    uint8_t* 	outData,
    uint32_t    outDataSize)
*/	
    pthread_mutex_unlock(&keypad_mutex);     

}





#define LED_CMD_SIZE	16
struct cy_led_cmd{
	     bool occupy;
	     bool on; //turn on or turn off
	     unsigned int led_num; // led number
	     unsigned int interval; 
 	     unsigned long start_ticks;
		 } st_cy_led_cmd[LED_CMD_SIZE]={0};

//led light off for a period interval_ms
int itp_cy_keypad_light_flash(unsigned int led_num, unsigned int interval_ms)
{
	int i;
	for(i=0;i<LED_CMD_SIZE;i++)
	{
		if(  0 == st_cy_led_cmd[i].occupy )
		{
			itp_cy_keypad_light_on_off(led_num,1); //'on' first and regist 'off' to queue
			st_cy_led_cmd[i].on =0;
			st_cy_led_cmd[i].led_num =led_num;
			st_cy_led_cmd[i].interval =interval_ms;
			st_cy_led_cmd[i].start_ticks = xTaskGetTickCount();
			st_cy_led_cmd[i].occupy =1;
			return 0;
		}		
	}

	if(i ==LED_CMD_SIZE )
	{
		printf("-W- itp_cy_keypad_light_flash, cmd full!");
		return -1;
	}
	else
	{
		return 0;
	}
}


void itp_cy_keypad_indoor_led_flash()
{
#define SCENCE_INDOOR_LED 		0
	itp_cy_keypad_light_flash(SCENCE_INDOOR_LED,500);
}

void itp_cy_keypad_sgate_call_led_flash()
{
	#define SCENCE_CALL_LED 		4
	itp_cy_keypad_light_flash(SCENCE_CALL_LED,500);
}

void itp_cy_keypad_sgate_manager_led_flash()
{
	#define SCENCE_MANAGER_LED 	3
	itp_cy_keypad_light_flash(SCENCE_MANAGER_LED,500);
}

void itp_cy_keypad_sgate_card_led_flash()
{
	#define SCENCE_CARD_LED 		2
	itp_cy_keypad_light_flash(SCENCE_CARD_LED,500);
}



void itp_cy_keypad_led_flash()
{
	itp_cy_keypad_light_flash(0,500);
	itp_cy_keypad_light_flash(1,500);
	itp_cy_keypad_light_flash(2,500);	
	itp_cy_keypad_light_flash(3,500);
	itp_cy_keypad_light_flash(4,500);
}



unsigned long itp_cy_keypad_get_elapsed_ms(unsigned long start_ticks)
{

	unsigned long tick = xTaskGetTickCount();
	unsigned long ret=0;


    if (tick >= start_ticks)
        ret = ((tick - start_ticks) / portTICK_PERIOD_MS);
    else
        ret =((0xFFFFFFFF - start_ticks + tick) / portTICK_PERIOD_MS);

	//printf(" [LIGHT  Elapsed]%d ,start_ticks %d,portTICK_RATE_MS %d\n", ret,start_ticks,portTICK_RATE_MS);
	return ret;
}



void itp_cy_keypad_light_cmd_queue()
{

	int i;
	unsigned long duration = 0;
	for(i=0;i<LED_CMD_SIZE;i++)
	{
		if(1 == st_cy_led_cmd[i].occupy)
		{
		//	printf(" [LIGHT  OCC]%d \n", i);

			duration = itp_cy_keypad_get_elapsed_ms(st_cy_led_cmd[i].start_ticks);
			if(duration >= st_cy_led_cmd[i].interval)
			{
//				printf(" [LIGHT  TIME]%d ,duration %d,interval %d,on=%d \n", i,   duration,st_cy_led_cmd[i].interval, st_cy_led_cmd[i].on);
				itp_cy_keypad_light_on_off( st_cy_led_cmd[i].led_num, st_cy_led_cmd[i].on);
				st_cy_led_cmd[i].occupy=0;				
			}
			else
			{

			}
		}
		else
		{
			// do nothing if no timer
		}

	}



	

}


uint8_t itp_cy_keypad_translation(uint8_t reg_val)
{
uint8_t  buttonID=0xff;
//case of 8 key
// =======
//  0x08	0x20
// =======
//  0x02	0x04
// =======
//  0x01	0x80
// =======
//  0x10	0x40
// =======

	switch(reg_val)
	{
		case 0x08:
		case 0x20:
			buttonID = 4;
			break;
		case 0x02:
		case 0x04:
			buttonID = 1;
			break;
		case 0x01:
		case 0x80:
			buttonID = 2;			
			break;
		case 0x10:
		case 0x40:
			buttonID = 3;
			break;
;
		default:
			buttonID = 0xff;
			break;

	}
	return buttonID;

}
void itp_cy_keypad_touch_led_flash(uint8_t reg_val)
{ //mapping touch value to led

	switch(reg_val)
	{
		case 0x08:
//			buttonID = 1;
			break;

		case 0x02:
			itp_cy_keypad_sgate_manager_led_flash();
			break;
			
		case 0x01:
//			buttonID = 3;			
			break;
		case 0x10:
//			buttonID = 4;
			itp_cy_keypad_sgate_call_led_flash();
			break;

		case 0x20:
//			buttonID = 5;			
			break;

		case 0x04:
//			buttonID = 6;			
			break;
		case 0x80:
//			buttonID = 7;			
			break;
		case 0x40:
//			buttonID = 8;			
			break;
		default:
//			buttonID = 0xff;
			break;

	}


}


/**************************************************************************
** public function(keypad API)                                           **
//ret = -1 : not valid
***************************************************************************/

int itpKeypadProbe(void)
{
    unsigned int i;
    uint16_t value=0;
    uint8_t ChkSum=0;
    uint8_t index;
    uint32_t	kpIntr = 0;
    

    pthread_mutex_lock(&keypad_mutex);
    itp_cy_keypad_light_cmd_queue();

	
#ifdef	ENABLE_KEYPAD_INT
        kpIntr = ithGpioGet(TK_GPIO_PIN);
#endif
    if(!kpIntr)
    {
//	    printf("[KEYPAD %d,%d]-INT",CFG_GPIO_KEYPAD,TK_GPIO_PIN);
    	value = _getTouchKey_cy();
    	if(!value)
    	{
		return -1;//invalid
    	}
	else
	{
		index = itp_cy_keypad_translation(value);

//		if()
			

		
		if( 0xff ==index)
		{
			return -1;//invalid
		}
		else
		{
    			 itp_cy_keypad_touch_led_flash(value);	  
		}
		
	}
     printf("[KEYPAD]key=%x,id=%x \n",value,index);

    }
    pthread_mutex_unlock(&keypad_mutex);     


    return (int)index;
}

void itpKeypadInit(void)
{
    printf("[KEYPAD]-itpKeypadInit");
	//TODO::
	//skip i2c init flow
	#ifdef	ENABLE_KEYPAD_INT
	_initTkGpioPin();
	#endif

	//itp_cy_keypad_light_off_all();
	
}

int itpKeypadGetMaxLevel(void)
{
    return gTotalTouchKeyNum;
}

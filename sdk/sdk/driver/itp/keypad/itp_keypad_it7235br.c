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


/**************************************************************************
** MACRO defination                                                      **
***************************************************************************/
#ifndef	CFG_TOUCH_KEY_NUM
#define CFG_TOUCH_KEY_NUM	16
#endif

#define IT7235_IIC_ADDR		(0x8C>>1)
//#define ENABLE_KEYPAD_DBG_MODE





/**************************************************************************
** global variable                                                      **
***************************************************************************/
//IT7235BR touch 16-key pad mapping table
// ----------------------
// | [1]  [2]  [3]  [A] |
// | [4]  [5]  [6]  [B] |
// | [7]  [8]  [9]  [C] |
// | [*]  [0]  [#] [Cen]|
// ----------------------
//kay:: 	[0]  [1]  [2]  [3]  [4]  [5]  [6]  [7]  [8]  [9]  [A]  [B]  [C]  [Cen] [*]  [#]
//code::   	0200,2000,0080,0004,1000,8000,0002,4000,0400,0001,0020,0010,0800,0040, 0100,0008
//bit::	 	9   ,13  ,7   ,2   ,12  ,15  ,1   ,14  ,10  ,0   ,5   ,4   ,11  ,6   , 8   ,3
//SDL code: 13  ,0,  ,1   ,2   ,4   ,5   ,6   ,8   ,9   ,10  ,3   ,7   ,11  ,15   ,12, ,14 
//(ref:keypad_doorbell_entrance.inc)

//so map table = { 10, 6, 2, 14, 7, 3, 15, 1, 12, 13, 9, 11, 4, 0, 8, 5 };
//i.e. bit 0 return 10(key[9]), bit 1 return 6(key[6]), bit 2 return 2(key[3]), bit 3 return 14(key[#]),...

//static const unsigned char kpTchKeyTable[] = { 10, 6, 2, 14, 7, 3, 15, 1, 12, 13, 9, 11, 4, 0, 8, 5 };
static const unsigned char kpTchKeyTable[] =    { 5, 9, 13, 1, 8, 12, 0, 14, 3, 2, 6, 4, 11, 15, 7, 10 };	//table of rotating 180 

static	pthread_mutex_t     keypad_mutex = PTHREAD_MUTEX_INITIALIZER;

static	uint8_t		gRegPage=0xFE;
static	uint8_t		gLastIicStatus=0;
static	uint8_t		g_kpI2cPort = IIC_PORT_0;
/**************************************************************************
** private function                                                      **
***************************************************************************/
static uint8_t _checkSum(uint16_t sum)
{
	uint8_t	i;
	uint8_t	cnt=0;
	
	for(i=0; i<CFG_TOUCH_KEY_NUM; i++)
	{
		if( (sum>>i)&0x01 )	cnt++;
	}
	return cnt;
}

static void _resetDevice(void)
{
	//pull down iic_SCL for 4ms

	#ifdef	ENABLE_KEYPAD_DBG_MODE
	printf("[Keypad error] IT7235BR hangup, and reset keypad device.\n");
	#endif
	
	//set GPIO3 as mode0	
	ithGpioSetMode(2, ITH_GPIO_MODE0);
	ithGpioSetMode(3, ITH_GPIO_MODE0);
	
	//set output mode
	ithGpioSetOut(2);
	ithGpioSetOut(3);
	
	//set GPIO output 0
	ithGpioClear(2);
	ithGpioClear(3);
	
	//for 4ms
	usleep(8000);
	ithGpioSet(2);	
	ithGpioSet(3);
	usleep(100);
	
	//set GPIO3 as mode1(IIC mode)
	ithGpioSetMode(2, ITH_GPIO_MODE3);
	ithGpioSetMode(3, ITH_GPIO_MODE3);
	usleep(100);
	
	#ifdef	ENABLE_KEYPAD_DBG_MODE
	printf(" end of keypad reset flow\n");
	#endif
}

static void _checkPage(uint8_t page)
{
	unsigned int result = 0;
	uint8_t		buf[2];
	uint32_t	reg32, cnt=0;
	
	if(gRegPage!=page)
	{
		result = mmpIicSendData(g_kpI2cPort, IIC_MASTER_MODE, IT7235_IIC_ADDR, 0xF0, &page, 1); 
		if(result)
	    {
	    	mmpIicStop(g_kpI2cPort);
	    	usleep(1000);
	    	
	    	if( result==I2C_WAIT_TRANSMIT_TIME_OUT )
	    	{
	    		printf("[KEYPAD]warning: IIC hang up(1), reset keypad device, result=%x.\n",result); 
	    		_resetDevice();
	    	}	    	
			
			#ifdef	ENABLE_KEYPAD_DBG_MODE
			printf("[KEYPAD]check iic status(busy/ready).01\n");
	   		while(1)
			{	
				reg32 = ithReadRegA((ITH_IIC_BASE + 0x04));
				if( !(reg32&0x0C) )	break;
				if( (cnt&0xFFFF)==0xFFFF )	printf(" iic busy.01,cnt=%X,reg=%x\n",cnt,reg32);
				if(++cnt>0xFFFFF){printf("STOP!!\n");	break;}
			}
		    printf("[KEYPAD]warning:: _checkPage() try again!!\n");				
			#endif		

		    result = mmpIicSendData(g_kpI2cPort, IIC_MASTER_MODE, IT7235_IIC_ADDR, 0xF0, &page, 1);
		    if(result)
		    {
	    		mmpIicStop(g_kpI2cPort);
	    		usleep(1000);	
	    		
	    		if( result==I2C_WAIT_TRANSMIT_TIME_OUT )
	    		{
	    			printf("[KEYPAD]warning: IIC hang up(2), reset keypad device, result=%x.\n",result); 
	    			_resetDevice();
	    		}
 				
		       	printf("%s[%d]ERROR = %x \n", __FILE__, __LINE__, result);   
		       	
		       	#ifdef	ENABLE_KEYPAD_DBG_MODE
		       	printf("[KEYPAD]check iic status(busy/ready).02\n");
		   		while(1)
				{	
					reg32 = ithReadRegA((ITH_IIC_BASE + 0x04));
					if( !(reg32&0x0C) )	break;
					if( (cnt&0xFFFF)==0xFFFF )	printf(" iic busy.02,cnt=%X,reg=%x\n",cnt,reg32);
					if(++cnt>0xFFFFF){printf("STOP!!\n");	break;}
				}
				printf("[KEYPAD]warning::for checking IIC busy status.\n");     
				#endif
				
		    	return;
		    }
        }

		#ifdef	ENABLE_KEYPAD_DBG_MODE
		printf("write page=0x%X to regAddr 0x%X\n", page, 0xF0);
		#endif
		
		gRegPage = page;
	}
}

static uint16_t _getTouchKey(void)
{
	uint8_t		page = 0;
	uint16_t	KeyValue;
	unsigned int result = 0;
	uint8_t		buf[2];
    uint32_t	regData1,regData2;
    uint32_t	reg32, cnt=0;

	_checkPage(page);
	
	buf[0] = 0xFE; 
	buf[1] = 0xFE; 
	
	result = mmpIicReceiveData(g_kpI2cPort, IIC_MASTER_MODE, IT7235_IIC_ADDR, buf, 1, buf, 2);
    if(result)
    {
    	mmpIicStop(g_kpI2cPort);
    	usleep(1000);    
    	
    	if( result==I2C_WAIT_TRANSMIT_TIME_OUT )
    	{
    		printf("[KEYPAD]warning: IIC hang up(3), reset keypad device, result=%x.\n",result); 
    		_resetDevice();
    		return 0;
    	}
		
		#ifdef	ENABLE_KEYPAD_DBG_MODE
    	printf("[KEYPAD]warning:: retry _getTouchKey(), result=%x,buf=[%x,%x]\n",result,buf[0],buf[1]);   
    	printf("[KEYPAD]check iic status(busy/ready).03\n");     		
   		while(1)
		{	
			reg32 = ithReadRegA((ITH_IIC_BASE + 0x04));
			if( !(reg32&0x0C) )	break;
			if( (cnt&0xFFFF)==0xFFFF )	printf(" iic busy.03,cnt=%X,reg=%x\n",cnt,reg32);
			if(++cnt>0xFFFFF){printf("STOP!!\n");	break;}
		}
		printf("[KEYPAD]warning::.03\n");
		#endif

    	buf[0] = 0xFE; 
		buf[1] = 0xFE; 
    	
    	result = mmpIicReceiveData(g_kpI2cPort, IIC_MASTER_MODE, IT7235_IIC_ADDR, buf, 1, buf, 2);
    	if(result)
    	{
        	mmpIicStop(g_kpI2cPort);    	
        	usleep(1000);
        	
	    	if( result==I2C_WAIT_TRANSMIT_TIME_OUT )
	    	{
	    		printf("[KEYPAD]warning: IIC hang up(4), reset keypad device, result=%x.\n",result); 
	    		_resetDevice();
	    	}
	    	
	    	printf("[KEYPAD][%x,%x]%s[%d]ERROR = %x \n",buf[0],buf[1] , __FILE__, __LINE__, result);
	    	gRegPage = 0xFE;
	    	
	    	#ifdef	ENABLE_KEYPAD_DBG_MODE
       		printf("[KEYPAD]check iic status(busy/ready).04\n"); 
       		while(1)
			{	
				reg32 = ithReadRegA((ITH_IIC_BASE + 0x04));
				if( !(reg32&0x0C) )	break;
				if( (cnt&0xFFF)==0xFFF )	printf(" iic busy.04,cnt=%X,reg=%x\n",cnt,reg32);
				if(++cnt>0xFFFF){printf("STOP!!\n");	break;}
			}
			printf("_getTouchKey.4\n");
			#endif
			
			return 0;
       	}
    }
    
    #ifdef	ENABLE_KEYPAD_DBG_MODE
    if( buf[0] || buf[1])
		printf("buf=[0x%X,0x%X], R=%x\n", buf[0], buf[1], result);
	#endif
	
	KeyValue = (uint16_t)buf[0] + (((uint16_t)buf[1]<<8)&0xFF00);

	return KeyValue;
}













/**************************************************************************
** public function(keypad API)                                           **
***************************************************************************/

int itpKeypadProbe(void)
{
    unsigned int i;
    uint16_t value;
    uint8_t ChkSum;
    
    //printf("itpKeyPadProbe\n");
    
    pthread_mutex_lock(&keypad_mutex);
    value = _getTouchKey();
    pthread_mutex_unlock(&keypad_mutex);
    
    ChkSum = _checkSum(value);
       
    if(ChkSum==0)	return -1;
    
    if(ChkSum>1)
    {
    	printf("[KEYPAD]warning:: multi-key, skip it(value=%x, chk=%d)\n",value, ChkSum);
    	return -1;
    }
    
	#ifdef	ENABLE_KEYPAD_DBG_MODE
    printf("[KEYPAD]:got key=%04x\n",value);
    #endif
    
    for (i = 0; i < CFG_TOUCH_KEY_NUM; i++)
    {    	
        if( (value>>i) & 0x1 )
        {        	
            return (int)kpTchKeyTable[(uint8_t)i];
        }
    }
}

void itpKeypadInit(void)
{
	//TODO::
	//skip i2c init flow
}

int itpKeypadGetMaxLevel(void)
{
    return CFG_TOUCH_KEY_NUM;
}

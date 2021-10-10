#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <alloca.h>
#include <pthread.h>	
#include "ite/ith.h"   // for 
#include "ite/itp.h"
#include "config.h"
#include "tslib-private.h"

#ifdef __OPENRTOS__
#include "openrtos/FreeRTOS.h"		//must "include FreeRTOS.h" before "include queue.h"
#include "openrtos/queue.h"			//for using xQueue function
#endif

//#define EN_DISABLE_ALL_INTR
#define	ENABLE_PATCH_Y_SHIFT_ISSUE
//#define	ENABLE_PATCH_INT_LOW_ISSUE
/****************************************************************************
 * ENABLE_TOUCH_POSITION_MSG :: just print X,Y coordination & 
 * 								touch-down/touch-up
 * ENABLE_TOUCH_IIC_DBG_MSG  :: show the IIC command 
 * ENABLE_TOUCH_PANEL_DBG_MSG:: show send-queue recieve-queue, 
 *                              and the xy value of each INTr
 ****************************************************************************/
//#define ENABLE_TOUCH_POSITION_MSG
//#define ENABLE_TOUCH_PANEL_DBG_MSG
//#define ENABLE_TOUCH_IIC_DBG_MSG
//#define ENABLE_SEND_FAKE_SAMPLE

/****************************************************************************
 * 0.pre-work(build a test project, set storage for tslib config file)
 * 1.INT pin response(OK)
 * 2.IIC send read/write command, register address, and response
 * 3.get x,y coordiantion(palse data buffer)
 * 4.celibration(rotate, scale, reverse...)
 * 5.get 4 corner touch point(should be (0,0)(800,0)(0,600)(800,600) )
 * 6.add interrupt, thread, Xqueue, Timer..
 * 7.check interrupt response time(NO event loss)
 * 8.check sample rate (33ms)
 * 9.check touch down/up event
 * 10.draw line test
 * 11.check IIC 400kHz and IIC acess procedure(sleep 300~500us)
 ****************************************************************************/

#ifdef	CFG_TOUCH_ADVANCE_CONFIG

#define ENABLE_SCALE_X
#define ENABLE_SCALE_Y

#ifdef	CFG_TOUCH_SWAP_XY
#define ENABLE_SWITCH_XY
#endif

#ifdef	CFG_TOUCH_REVERSE_X
#define ENABLE_REVERSE_X
#endif

#ifdef	CFG_TOUCH_REVERSE_Y
#define ENABLE_REVERSE_Y
#endif

#else

#define ENABLE_SWITCH_XY
#define ENABLE_SCALE_X
#define ENABLE_SCALE_Y
//#define ENABLE_REVERSE_X
#define ENABLE_REVERSE_Y

#endif
/***************************
 *
 **************************/
#define TP_GPIO_PIN	    CFG_GPIO_TOUCH_INT
#define TP_GPIO_MASK    (1<<(TP_GPIO_PIN%32))

#ifdef	CFG_GPIO_TOUCH_WAKE
#define TP_GPIO_WAKE_PIN	CFG_GPIO_TOUCH_WAKE
#endif 

#define TOUCH_DEVICE_ID     (0x48)
#define QUEUE_LEN 			(256)
#define TOUCH_SAMPLE_RATE	(33)

/***************************
 *
 **************************/
struct zt2083_ts_event { /* Used in the IBM Arctic II */
	signed short pressure;
	signed int x;
	signed int y;
	int millisecs;
	int flags;
};

//#ifdef CFG_TOUCH_INTR
static char g_TouchDownIntr = false;
//#endif

static char g_IsTpInitialized = false;
static char g_IsTpFirstPoint = 1;

static int lastp=0;
static int g_MaxRawX = 0xFFFF;
static int g_MaxRawY = 0xFFFF;

#ifdef	ENABLE_SCALE_X
static int g_xResolution=800;
#else
static int g_xResolution=CFG_TOUCH_X_MAX_VALUE;
#endif

#ifdef	ENABLE_SCALE_Y
static int g_yResolution=480;
#else
static int g_yResolution=CFG_TOUCH_Y_MAX_VALUE;
#endif

static unsigned int dur=0;
static unsigned char	gLastBuf[8]={0};
struct timeval startT, endT;

#ifdef	ENABLE_PATCH_Y_SHIFT_ISSUE
static int gInvalidZ1Z2 = 0;
#endif

#ifdef ENABLE_PATCH_INT_LOW_ISSUE
static unsigned int gIntDurTime = 10*1000;
static unsigned int gTpCnt1 = 0;
static unsigned int gIntDur1 = 0;
static unsigned int gTpCnt2 = 0;
static unsigned int gIntDur2 = 0;
struct timeval tv1, tv2;
struct timeval tvA, tvB;
#endif

static int g_tchDevFd=0;

#ifdef __OPENRTOS__
static QueueHandle_t tpQueue;
static pthread_mutex_t 	gTpMutex;
#endif

#ifdef	ENABLE_SEND_FAKE_SAMPLE
static int g_tpCntr = 0;
static unsigned char MAX_FAKE_NUM = 31;
static int gFakeTableX[] = {688,100,100,100,100,100,562,436,100,100,100,100,100,310,200,100};
static int gFakeTableY[] = {406,250,200,160,120, 70,406,406,250,200,160,120, 70,406,406,406};
//static int gFakeTableX[] = {688,100,100,100,100,100,562,436,100,100,100,100,100,310,200,100,100,100,100,688,562,436,310,200,100,688,562,436,310,200,100};
//static int gFakeTableY[] = {406,250,200,160,120, 70,406,406,250,200,160,120, 70,406,406,406,160,120, 70,406,406,406,406,406,406,406,406,406,406,406,406};
#endif
/*##################################################################################
 *                        the private function implementation
 ###################################################################################*/
#ifdef CFG_TOUCH_INTR
void _tp_isr(void* data)
{	
	unsigned int regValue;
	
	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	//ithPrintf("$in\n");
	#endif
	
	regValue = ithGpioGet(TP_GPIO_PIN);
	if ( (regValue & TP_GPIO_MASK) )
	{
		g_TouchDownIntr = false;
		#ifdef	ENABLE_PATCH_INT_LOW_ISSUE
		gTpCnt1 = 0;
		gIntDur1 = 0;
		#endif
	}
	else
	{
		g_TouchDownIntr = true;
	}

    ithGpioClearIntr(TP_GPIO_PIN);
	
	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	//ithPrintf("$out(%x,%x)\n",g_TouchDownIntr,regValue);
	#endif
}

static void _initTouchIntr(void)
{
	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
    printf("TP init in\n");	
    #endif
    
    ithEnterCritical();
    
    ithGpioClearIntr(TP_GPIO_PIN);
    ithGpioRegisterIntrHandler(TP_GPIO_PIN, _tp_isr, NULL);
    ithGpioCtrlEnable(TP_GPIO_PIN, ITH_GPIO_INTR_BOTHEDGE);
    ithIntrEnableIrq(ITH_INTR_GPIO);
    ithGpioEnableIntr(TP_GPIO_PIN); 
        
    ithExitCritical();
    
    #ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
    printf("TP init out\n");	
    #endif
}
#endif

void _initTouchGpioPin(void)
{
	ithGpioSetMode(TP_GPIO_PIN, ITH_GPIO_MODE0);
	ithGpioSetIn(TP_GPIO_PIN);
	ithGpioCtrlEnable(TP_GPIO_PIN, ITH_GPIO_PULL_ENABLE);
	ithGpioCtrlEnable(TP_GPIO_PIN, ITH_GPIO_PULL_UP);
	ithGpioEnable(TP_GPIO_PIN);	     
}

#ifdef ENABLE_PATCH_INT_LOW_ISSUE
void _tpWorkAroundIntLowIssue(void)
{
    int i=0;
    
    ithEnterCritical();
    ithIntrDisableIrq(ITH_INTR_GPIO);
    ithGpioDisableIntr(TP_GPIO_PIN); 
    ithExitCritical();    
    
	ithGpioCtrlDisable(TP_GPIO_PIN, ITH_GPIO_PULL_ENABLE);
	ithGpioCtrlDisable(TP_GPIO_PIN, ITH_GPIO_PULL_UP);
    
    while(i++<10)
    {
	    ithGpioSet(TP_GPIO_PIN);
    	ithGpioSetOut(TP_GPIO_PIN);  
    	usleep(100*1000);
    	
    	if ( (ithGpioGet(TP_GPIO_PIN) & TP_GPIO_MASK) )	
    	{
    	    printf("[TP] ~~~ GOT INT PULL HIGH ~~~\n");
    	    break;
    	}
    }
    
    if ( !(ithGpioGet(TP_GPIO_PIN) & TP_GPIO_MASK) )	printf("[TP] ~~~ INT pin is still LOW ~~~\n");    
    
	ithGpioSetIn(TP_GPIO_PIN);
	ithGpioCtrlEnable(TP_GPIO_PIN, ITH_GPIO_PULL_ENABLE);
	ithGpioCtrlEnable(TP_GPIO_PIN, ITH_GPIO_PULL_UP);
	ithGpioEnable(TP_GPIO_PIN);
	
    ithEnterCritical();
    ithGpioClearIntr(TP_GPIO_PIN);
    ithIntrEnableIrq(ITH_INTR_GPIO);
    ithGpioEnableIntr(TP_GPIO_PIN); 
    ithExitCritical();    
}
#endif
/******************************************************************************
 * the read flow for reading the zt2083's register by using iic repead start
 ******************************************************************************/
static int _readChipReg(int fd, unsigned char regAddr, unsigned char *dBuf, unsigned char dLen)
{
	ITPI2cInfo evt;
	unsigned char	I2cCmd;
	int 			i2cret;
	
	#ifdef	ENABLE_TOUCH_IIC_DBG_MSG
	printf("	RdTchIcReg(fd=%x, reg=%x, buf=%x, len=%x)\n", fd, regAddr, dBuf, dLen);
	#endif
	
	#ifdef EN_DISABLE_ALL_INTR
	portSAVEDISABLE_INTERRUPTS();
	#endif		
	
	I2cCmd = regAddr;	//1000 0010		
	evt.slaveAddress   = TOUCH_DEVICE_ID;
	evt.cmdBuffer      = &I2cCmd;
	evt.cmdBufferSize  = 1;	
	evt.dataBuffer     = dBuf;
	evt.dataBufferSize = dLen;	
	
	i2cret = read(fd, &evt, 1);
	
	#ifdef EN_DISABLE_ALL_INTR
    portRESTORE_INTERRUPTS();
	#endif	
		
	if(i2cret<0)
	{
		printf("[TOUCH ERROR].iic read fail\n");
		return -1;		
	}
	
	return 0;
}

/******************************************************************************
 * the write flow for writing the zt2083's register by using iic repead start
 ******************************************************************************/
static int _writeChipReg(int fd, unsigned char regAddr)
{
	ITPI2cInfo evt;
	unsigned char	I2cCmd;
	int 			i2cret;
	
	#ifdef	ENABLE_TOUCH_IIC_DBG_MSG
	printf("	WtTchIcReg(fd=%x, reg=%x)\n", fd, regAddr);
	#endif
	
	#ifdef EN_DISABLE_ALL_INTR
	portSAVEDISABLE_INTERRUPTS();
	#endif		
	
	I2cCmd = regAddr;	//1000 0010		
	evt.slaveAddress   = TOUCH_DEVICE_ID;
	evt.cmdBuffer      = &I2cCmd;
	evt.cmdBufferSize  = 1;
	evt.dataBuffer     = 0;
	evt.dataBufferSize = 0;	
	i2cret = write(fd, &evt, 1);
	
	#ifdef EN_DISABLE_ALL_INTR
    portRESTORE_INTERRUPTS();
	#endif	
		
	if(i2cret<0)
	{
		printf("[TOUCH ERROR].iic read fail\n");
		return -1;		
	}
	
	return 0;
}

/******************************************************************************
 * to read the Point Buffer by reading the register "0xE0"
 ******************************************************************************/
static int _readPointBuffer(int fd, unsigned char *Pbuf)
{
	ITPI2cInfo evt;
	unsigned char	I2cCmd;
	int 			i2cret;
	
	#ifdef	ENABLE_TOUCH_IIC_DBG_MSG
	printf("	RdPntbuf,%x,%x\n",fd,Pbuf);
	#endif
	
	while(1)
	{
		i2cret = _writeChipReg(fd, 0x80);
		if(i2cret<0)
		{
			printf("[TOUCH ERROR].Read Point buffer fail\n");
			return -1;
		}
		//usleep(500);
		i2cret = _readChipReg(fd, 0xC0, Pbuf, 2);
		if(i2cret<0)
		{
			printf("[TOUCH ERROR].Read Point buffer fail\n");
			return -1;
		}
    	
		i2cret = _writeChipReg(fd, 0x90);
		if(i2cret<0)
		{
			printf("[TOUCH ERROR].Read Point buffer fail\n");
			return -1;
		}
		//usleep(500);
		i2cret = _readChipReg(fd, 0xD0, &Pbuf[2], 2);
		if(i2cret<0)
		{
			printf("[TOUCH ERROR].Read Point buffer fail\n");
			return -1;
		}
    	
		i2cret = _writeChipReg(fd, 0xA0);
		if(i2cret<0)
		{
			printf("[TOUCH ERROR].Read Point buffer fail\n");
			return -1;
		}
		//usleep(500);
		i2cret = _readChipReg(fd, 0xE0, &Pbuf[4], 2);
		if(i2cret<0)
		{
			printf("[TOUCH ERROR].Read Point buffer fail\n");
			return -1;
		}
    	
		i2cret = _writeChipReg(fd, 0xA0);
		if(i2cret<0)
		{
			printf("[TOUCH ERROR].Read Point buffer fail\n");
			return -1;
		}
		//usleep(500);
		i2cret = _readChipReg(fd, 0xF0, &Pbuf[6], 2);	
		if(i2cret<0)
		{
			printf("[TOUCH ERROR].Read Point buffer fail\n");
			return -1;
		}
		
		#ifdef	ENABLE_PATCH_Y_SHIFT_ISSUE
		if ( (ithGpioGet(TP_GPIO_PIN) & TP_GPIO_MASK) )
		{
			#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
			printf("	##^[ invalid sample(INT=1, lastp=%x)!! ]^##\n",lastp);
			#endif
			return (-1);
		}

		//check Z1 & Z2 for valid point
		if(!g_IsTpFirstPoint)
		{
			unsigned int Z,Z1,Z2,X,Y;
			
			X = ((unsigned int)Pbuf[0] << 8) + (unsigned int)Pbuf[1];
			Y = ((unsigned int)Pbuf[2] << 8) + (unsigned int)Pbuf[3];			
			Z1 = ((unsigned int)Pbuf[4] << 8) + (unsigned int)Pbuf[5];
			Z2 = ((unsigned int)Pbuf[6] << 8) + (unsigned int)Pbuf[7];
			
			X = X>>4;	Z1 = Z1>>4;	Z2 = Z2>>4;	Y = Y>>4;
			if(Z1)
			{
				Z = (X*(Z2-Z1)) / (Z1);
				if( ((Z/16)<200) || ((Z/16)>2200) )	
				{		
					#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
					printf("	##^[ invalid (%d,%d)(%d,%d,%d) ]^##\n",X,Y, Z1,Z2,Z/16);
					#endif
					//printf("	##^[ invalid (%d,%d)(%d,%d,%d) ]^##\n",X,Y, Z1,Z2,Z/16);
					gInvalidZ1Z2 = 1;
					return (-1);
				}
			}
			else
			{
				#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
				printf("	##^[ invalid Z1=0,(%d,%d)(%d,%d) ]^##\n",X,Y, Z1,Z2);
				#endif
				//printf("	##^[ invalid Z1=0,(%d,%d)(%d,%d) ]^##\n",X,Y, Z1,Z2);
				gInvalidZ1Z2 = 1;
				return (-1);
			}
		}
		#endif	
		
		break;
	}	
	
	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	{
		int i;		
		printf("\nRdPbuf:");
		for(i=0; i<8; i++)	printf("%02x ",Pbuf[i]);
		printf("\n\n");		
	}
	#endif
	
	#ifdef	ENABLE_PATCH_Y_SHIFT_ISSUE
	gInvalidZ1Z2 = 0;
	#endif

	return 0;
}


//static int zt2083_read(struct tslib_module_info *inf, struct ts_sample *samp, int nr)
static void* _tpProbeHandler(void* arg)
{
    unsigned int regValue;
    
    printf("_tpProbeHandler.start~~\n");
    
	while(1)
	{
		if(g_IsTpInitialized==true)
		{
			#ifdef CFG_TOUCH_INTR
			if ( g_TouchDownIntr )
			#else
			regValue = ithGpioGet(TP_GPIO_PIN);
			if ( !(regValue & TP_GPIO_MASK) )
			#endif
			{
				struct ts_sample tmpSamp;
				
				#ifndef CFG_TOUCH_INTR
				ithEnterCritical();
				g_TouchDownIntr = true;
				ithExitCritical();
				#endif
				
				gettimeofday(&endT,NULL);
				dur = (unsigned int)itpTimevalDiff(&startT, &endT);		
						
				if( dur>TOUCH_SAMPLE_RATE )
				{
					#ifdef	ENABLE_PATCH_Y_SHIFT_ISSUE
					if( !_getTouchSample(&tmpSamp, 1) )
					{
						#ifdef __OPENRTOS__
						xQueueSend(tpQueue, &tmpSamp, 0);
						#endif
					
						#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
						printf("	EnQue:p=%x,xy=%d,%d\n", tmpSamp.pressure, tmpSamp.x, tmpSamp.y);
						#endif
						gettimeofday(&startT,NULL);
						
						lastp = 1;
					}
					else
					{
						#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
						printf("	get point error,(lastp=%x)(dur=%d)!\n",lastp,dur);
						#endif
						
						#ifdef	ENABLE_PATCH_INT_LOW_ISSUE
						printf("	get point error,(lastp=%x)(dur1=%d,%d)!\n",lastp,gIntDur1,gTpCnt1);
						#endif
					}
					#else
					_getTouchSample(&tmpSamp, 1);
					#ifdef __OPENRTOS__
					xQueueSend(tpQueue, &tmpSamp, 0);
					#endif
					
					#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
					printf("	EnQue:p=%x,xy=%d,%d\n", tmpSamp.pressure, tmpSamp.x, tmpSamp.y);
					#endif
					gettimeofday(&startT,NULL);
					#endif
				}
				#ifndef	ENABLE_PATCH_Y_SHIFT_ISSUE
				lastp = 1;
				#endif
				
				#ifdef ENABLE_PATCH_INT_LOW_ISSUE
			    if ( !(ithGpioGet(TP_GPIO_PIN) & TP_GPIO_MASK) )
			    {
			        if(!gTpCnt1++) gettimeofday(&tv1,NULL);
			        
			        gettimeofday(&tv2,NULL);
			        gIntDur1 = (unsigned int)itpTimevalDiff(&tv1, &tv2);	
			        if(gIntDur1 > gIntDurTime)
			        {
			            printf("[TP] trying to handle the INT-keep-low issue(1)\n");
			            _tpWorkAroundIntLowIssue();
			            gTpCnt1 = 0;
			        }			    
			    }
				#endif
				
				usleep(2000);							
			}
			#ifndef CFG_TOUCH_INTR
			else
			{
				ithEnterCritical();
				g_TouchDownIntr = false;
				ithExitCritical();

				#ifdef ENABLE_PATCH_INT_LOW_ISSUE
				if ( !(ithGpioGet(TP_GPIO_PIN) & TP_GPIO_MASK) )
				{
				    if(!gTpCnt2++) gettimeofday(&tvA,NULL);
				    gettimeofday(&tvB,NULL);
				    gIntDur2 = (unsigned int)itpTimevalDiff(&tvA, &tvB);	
				    if(gIntDur2 > gIntDurTime)
				    {
				        printf("[TP] trying to handle the INT-keep-low issue(2)\n");
				        _tpWorkAroundIntLowIssue();
				        gTpCnt2 = 0;
				    }
				}				
				#endif
			}
			#endif
			usleep(1000);
		}
		else
		{
			printf("touch has not init, yet~~~\n");
			usleep(100000);
		}
	}
	return NULL;
}

/******************************************************************************
 * do initial flow of zt2083
 * 1.indentify zt2083
 * 2.get 2D resolution 
 * 3.set interrupt information
 ******************************************************************************/
static int _initTouchChip(int fd)
{
	int i=0;
		
	//identify FW
	//TODO:

	//create thread
	if(g_IsTpInitialized==0)
	{
	    int res;
	    pthread_t task;
	    pthread_attr_t attr;	
	    printf("Create xQueue & pthread~~\n");
	    
	    #ifdef __OPENRTOS__
		tpQueue = xQueueCreate(QUEUE_LEN, (unsigned portBASE_TYPE) sizeof(struct ts_sample));

	    pthread_attr_init(&attr);
	    res = pthread_create(&task, &attr, _tpProbeHandler, NULL);
	        
	    if(res)
	    {
	    	printf( "[TouchPanel]%s() L#%ld: ERROR, create _tpProbeHandler() thread fail! res=%ld\n", res );
	    	return -1;
	    }
	    
	    res = pthread_mutex_init(&gTpMutex, NULL);
    	if(res)
    	{
    	    printf("[AudioLink]%s() L#%ld: ERROR, init touch mutex fail! res=%ld\r\n", __FUNCTION__, __LINE__, res);
    	    return -1;
    	}
		#endif    
    	return 0;
    }

	return -1;
}

static void _initSample(struct ts_sample *s, int nr)
{
	int i;
	struct ts_sample *samp=s;
	
	//samp=s;
	
	for(i = 0; i < nr; i++)
	{
		samp->x = 0;
		samp->y = 0;
		samp->pressure = 0;
		gettimeofday(&(samp->tv),NULL);		
		samp++;
	}
}

static int _palseTouchSample(unsigned char *buf, struct ts_sample *s, int nr)
{
	int i;
	int nRet=0;
	int tmpX=0;
	int tmpY=0;
	struct ts_sample *samp=s;
		
	for(i = 0; i < nr; i++)
	{
		//if(buf[0] & (1 << i))
		{
			nRet++;
			
			tmpX = ((int)buf[0] << 8) + (int)buf[1];
			tmpY = ((int)buf[2] << 8) + (int)buf[3];
			
			//printf("	##-Raw x,y = %d, %d ::",tmpX,tmpY);
			
			#ifdef	ENABLE_SWITCH_XY
			{
				int	tmp = tmpX;
				tmpX = tmpY;
				tmpY = tmp;
			}
			#endif		
			
			#ifdef	ENABLE_SCALE_X
			tmpX = (tmpX * g_xResolution)/g_MaxRawY;
			#endif
			
			#ifdef	ENABLE_SCALE_Y
			tmpY = (tmpY * g_yResolution)/g_MaxRawX;
			#endif					
			
			#ifdef	ENABLE_REVERSE_X				
			tmpX = g_xResolution - tmpX - 1;
			#else
			tmpX = tmpX;
			#endif
			
			#ifdef	ENABLE_REVERSE_Y
			tmpY = g_yResolution - tmpY - 1;
			#else
			tmpY = tmpY;
			#endif
			
			if(tmpX>=g_xResolution)	tmpX = g_xResolution - 1;
			if(tmpY>=g_yResolution)	tmpY = g_yResolution - 1;
			
			if(tmpX<0)	tmpX = 0;
			if(tmpY<0)	tmpY = 0;
			
			if( (tmpX>=g_xResolution) || (tmpY>=g_yResolution) || (tmpX<0) || (tmpY<0) )
				printf("[TP warning] XY are abnormal, x=%d,%d y=%d,%d\n",tmpX,g_xResolution,tmpY,g_yResolution);
						

			samp->pressure = 1;
			samp->x = tmpX;
			samp->y = tmpY;
			gettimeofday(&(samp->tv),NULL);
			
			//printf("modify x,y = %d, %d -##\n",samp->x,samp->y);
#ifdef DEBUG
        		fprintf(stderr,"	RAW-------------------->> %d %d %d\n",samp->x,samp->y,samp->pressure);
#endif /*DEBUG*/			
			
		}
		/*
		else
		{
			samp[i]->x = 0;
			samp[i]->y = 0;
			samp[i]->pressure = 0;
		}
		*/
		samp++;
	}
	return nRet;
}

int _getTouchSample(struct ts_sample *samp, int nr)
{
	int i2cret;
	int real_nr=0;
	unsigned char buf[14]={0};
	
	_initSample(samp, nr);
	#ifdef __OPENRTOS__
	pthread_mutex_lock(&gTpMutex);
	#endif
	i2cret = _readPointBuffer(g_tchDevFd,buf);
	if(i2cret<0)
	{
		#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
  		printf("tch.err.read_fail.1\n");
  		#endif
  		
  		#ifdef	ENABLE_PATCH_Y_SHIFT_ISSUE
  		//if INT active and get data Z1/Z2 error
  		//then keep last x/y sample
  		if(gInvalidZ1Z2)	_palseTouchSample(gLastBuf, samp, nr);
  		//printf("tch.err.read_fail.1,(%x,%d,%d)(z=%x) \n",samp->pressure, samp->x,samp->y,gInvalidZ1Z2);
  		#endif
  		
	    #ifdef __OPENRTOS__
        pthread_mutex_unlock(&gTpMutex);
	    #endif

		#ifdef	ENABLE_PATCH_Y_SHIFT_ISSUE
		return (-1);
		#else
		return;
		#endif
	}
	
	_palseTouchSample(buf, samp, nr);
	memcpy((void*)&gLastBuf[0], (void*)buf, 8);
	
	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	if(!lastp)	printf("\n      \\_tch_DOWN\n");
	printf("	P1.Buf[0~3]=[%x,%x,%x,%x], xy=[%d,%d]\n",buf[0],buf[1],buf[2],buf[3],samp->x,samp->y);
	#else
	#ifdef	ENABLE_TOUCH_POSITION_MSG
	if(!lastp)	printf("      ^^_tch_DOWN\n");
	printf("	P1.xy=[%d,%d]\n",samp->x,samp->y);
	#endif
	#endif
	#ifdef __OPENRTOS__
	pthread_mutex_unlock(&gTpMutex);
	#endif

	#ifdef	ENABLE_PATCH_Y_SHIFT_ISSUE
	return (0);	
	#endif
}

static void showAhbReg(unsigned int RegBase, unsigned int len)
{
	unsigned int i;
	printf("RegBase=%x, Len=%x\n",RegBase,len);
	for(i=RegBase; i<RegBase+len; i+=4)
	{
		printf("%08x ",ithReadRegA(i));
		if( (i&0x0C)==0x0C )	printf("\n");	
	}
	printf("\n");
}

#ifdef	ENABLE_SEND_FAKE_SAMPLE
static int _getFakeSample(struct ts_sample *samp, int nr)
{
	_initSample(samp, nr);
	
	printf("tp_getXY::cnt=%x\n",g_tpCntr);
	
	if(g_tpCntr++>0x100)
	{
		if( !(g_tpCntr&0x07) )
		{
			unsigned int i;
			i = (g_tpCntr>>3)&0x1F;
			if(i<MAX_FAKE_NUM)
			{
				samp->pressure = 1;
				samp->x = gFakeTableX[i];
				samp->y = gFakeTableY[i];
				printf("sendXY.=%d,%d\n",samp->x,samp->y);	
			}
		}
	}

	return nr;
}
#endif
/*##################################################################################
 *                           private function above
 ###################################################################################*/










/*##################################################################################
 #                       public function implementation
 ###################################################################################*/
static int zt2083_read(struct tslib_module_info *inf, struct ts_sample *samp, int nr)
{
	struct tsdev *ts = inf->dev;
	unsigned int regValue;
	int ret;
	int total = 0;
	int tchdev = ts->fd;
	struct ts_sample *s=samp;
	
	#ifdef	ENABLE_SEND_FAKE_SAMPLE
	return _getFakeSample(samp,nr);
	#endif	
	
	if(g_IsTpInitialized==false)
	{
		//_touchInit();
		
		printf("TP first init(INT is GPIO %d)\n",TP_GPIO_PIN);
		//showAhbReg(0xDE000000,0x100);
		
		gettimeofday(&startT,NULL);	
		
		#ifdef CFG_TOUCH_INTR	
		_initTouchIntr();	
		#endif
	
		//init touch GPIO pin 
		_initTouchGpioPin();

		ret = _initTouchChip(tchdev);
		if(ret<0)
		{
			printf("[TOUCH]warning:: touch panel initial fail\n");
			return -1;
		}
		usleep(100000);		//sleep 100ms for get the 1st touch event
		
		g_tchDevFd = tchdev;		
		g_IsTpInitialized = true;
		printf("## TP init OK, gTpIntr = %x\n",g_TouchDownIntr);
	}
	
	_initSample(s, nr);
	
	//to receive queue
	for(ret=0; ret<nr; ret++)
	{
		#ifdef __OPENRTOS__
		if( !xQueueReceive(tpQueue, s, 0) )
		{
			#ifdef CFG_TOUCH_INTR
			if(g_TouchDownIntr)			
			#else
			if ( !(ithGpioGet(TP_GPIO_PIN) & TP_GPIO_MASK) )
			#endif
			{
				if(lastp)
			{
					#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
					printf("	@@[TP] send lastP\n");
					#endif
					_palseTouchSample(gLastBuf, s, 1);
				}				
			}
		}
		#endif
		
		if(nr>1)	s++;
	}
	
	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	if( lastp || samp->pressure)	printf("	deQue-O:%x=(%x,%d,%d)r=%d\n", samp, samp->pressure, samp->x, samp->y, ret);
	#endif
	
	if( lastp && !samp->pressure)
	{
		#if	defined(ENABLE_TOUCH_PANEL_DBG_MSG) || defined(ENABLE_TOUCH_POSITION_MSG)
		printf("	__/tch-UP.2\n\n\n");	
		#endif
		
		if(g_IsTpFirstPoint)	g_IsTpFirstPoint=0;
		
		//printf("	><#[%x][%d,%d,%d]\n",lastp, samp->pressure, samp->x, samp->y);
		
		lastp = 0;		
	}
	
	//printf("	><#[%x][%d,%d,%d]\n",lastp, samp->pressure, samp->x, samp->y);
	
	return nr;
}

static const struct tslib_ops zt2083_ops =
{
	zt2083_read,
};

TSAPI struct tslib_module_info *zt2083_mod_init(struct tsdev *dev, const char *params)
{
	struct tslib_module_info *m;

	m = malloc(sizeof(struct tslib_module_info));
	if (m == NULL)
		return NULL;

	m->ops = &zt2083_ops;
	return m;
}

#ifndef TSLIB_STATIC_CASTOR3_MODULE
	TSLIB_MODULE_INIT(zt2083_mod_init);
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <alloca.h>
#include <pthread.h>	
#include "ite/ith.h"
#include "ite/itp.h"
#include "config.h"
#include "tslib-private.h"

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


/***************************
 *
 **************************/
#define TP_GPIO_PIN	    CFG_GPIO_TOUCH_INT
#define TP_RESET_PIN	(CFG_GPIO_TOUCH_WAKE)

#ifdef	CFG_GPIO_TOUCH_WAKE
#define TP_GPIO_WAKE_PIN	CFG_GPIO_TOUCH_WAKE
#endif 

#define TOUCH_DEVICE_ID (0x4C>>1)

#if (TP_GPIO_PIN<32)
#define TP_GPIO_MASK    (1<<TP_GPIO_PIN)
#else
#define TP_GPIO_MASK    (1<<(TP_GPIO_PIN-32))
#endif

#define QUEUE_LEN 256
//#define ENABLE_SWITCH_XY
#define ENABLE_SCALE_X
#define ENABLE_SCALE_Y
#define ENABLE_REVERSE_X
#define ENABLE_REVERSE_Y

struct msg22s_ts_event { /* Used in the IBM Arctic II */
	signed short pressure;
	signed int x;
	signed int y;
	int millisecs;
	int flags;
};

#ifdef CFG_TOUCH_INTR
static char g_TouchDownIntr = false;
#endif

static char g_IsTpInitialized = false;
static short g_xResolution=240;
static short g_yResolution=240;
static short g_MaxRawX = 2048;
static short g_MaxRawY = 2048;

static unsigned int dur=0;
struct timeval startT, endT;


static int g_tchDevFd=0;
static pthread_mutex_t 	gTpMutex;
static unsigned int gIntCnt=0;
static struct ts_sample g_sample;

#ifdef	ENABLE_SEND_FAKE_SAMPLE
static int g_tpCntr = 0;
static unsigned char MAX_FAKE_NUM = 31;
static int gFakeTableX[] = {688,100,100,100,100,100,562,436,100,100,100,100,100,310,200,100};
static int gFakeTableY[] = {406,250,200,160,120, 70,406,406,250,200,160,120, 70,406,406,406};
#endif
//static int gFakeTableX[] = {688,100,100,100,100,100,562,436,100,100,100,100,100,310,200,100,100,100,100,688,562,436,310,200,100,688,562,436,310,200,100};
//static int gFakeTableY[] = {406,250,200,160,120, 70,406,406,250,200,160,120, 70,406,406,406,160,120, 70,406,406,406,406,406,406,406,406,406,406,406,406};
/* //it's for 
static int gFakeTableX[] = {688,562,436,310,200,100};
static int gFakeTableY[] = {406,406,406,406,406,406};
*/
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
	
	g_TouchDownIntr = true;
	
	gIntCnt++;

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
    ithGpioCtrlDisable(TP_GPIO_PIN, ITH_GPIO_INTR_LEVELTRIGGER);
    //ithGpioCtrlEnable(TP_GPIO_PIN, ITH_GPIO_INTR_BOTHEDGE);
    ithGpioCtrlDisable(TP_GPIO_PIN, ITH_GPIO_INTR_BOTHEDGE);
    ithGpioCtrlDisable(TP_GPIO_PIN, ITH_GPIO_INTR_TRIGGERFALLING);
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
	
	ithGpioSetMode(TP_RESET_PIN, ITH_GPIO_MODE0);
	ithGpioSetOut(TP_RESET_PIN);
	ithGpioEnable(TP_RESET_PIN);	
	
	#ifdef	ENABLE_IIC_INTERNAL_PULL_UP
	ithGpioCtrlEnable(2, ITH_GPIO_PULL_ENABLE);
	ithGpioCtrlEnable(2, ITH_GPIO_PULL_UP);2
	
	ithGpioCtrlEnable(3, ITH_GPIO_PULL_ENABLE);
	ithGpioCtrlEnable(3, ITH_GPIO_PULL_UP);
	#endif
}

/******************************************************************************
 * 
 ******************************************************************************/
void _resetTouchChip(void)
{
	//TODO:	
	ithGpioClear(TP_RESET_PIN);
	usleep(10000);
	
	//set gpio14 as 1
	ithGpioSet(TP_RESET_PIN);
	usleep(100000);			//sleep 100ms, then read the TP controller 
}

/******************************************************************************
 * the read flow for reading the MSG22S's register by using iic repead start
 ******************************************************************************/
static unsigned char _cal8BitsChkSum( unsigned char *msg, int s32Length )
{
	int s32Checksum = 0;
	int i;

	for ( i = 0 ; i < s32Length; i++ )
	{
		s32Checksum += msg[i];
	}
	return (unsigned char)( ( -s32Checksum ) & 0xFF );
}

static unsigned char _checkPointBuffer(unsigned char *msg)
{
	if(msg[0]!=0x52)	return false;
	
	if(msg[7]!=_cal8BitsChkSum(msg,7))	return false;
	
	return true;
}

static int _readChipReg(int fd, unsigned char regAddr, unsigned char *dBuf, unsigned char dLen)
{
	ITPI2cInfo evt;
	unsigned char	I2cCmd;
	int 			i2cret;
	
	#ifdef	ENABLE_TOUCH_IIC_DBG_MSG
	printf("	RdIcReg(fd=%x, reg=%x, buf=%x, len=%x)\n", fd, regAddr, dBuf, dLen);
	#endif
	
	I2cCmd = regAddr;	//1000 0010		
	evt.slaveAddress   = TOUCH_DEVICE_ID;
	evt.cmdBuffer      = &I2cCmd;
	evt.cmdBufferSize  = 0;
	evt.dataBuffer     = dBuf;
	evt.dataBufferSize = dLen;	
	
	i2cret = read(fd, &evt, 1);

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
	printf("	RdPibuf,%x,%x\n",fd,Pbuf);
	#endif
	
	i2cret = _readChipReg(fd, 0xE0, Pbuf, 8);
	if(i2cret<0)
	{
		printf("[TOUCH ERROR].Read Point buffer fail\n");
		return -1;
	}

	return 0;
}

/******************************************************************************
 * read 6 bytes from msg22s via I2C bus
 * byte0:	0x52
 * byte1~3:	x & y coordination of point1
 * byte4~6:	x & y distance between point1 & point2
 * byte7:	check SUM
 ******************************************************************************/
static int _palseTouchSample(struct ts_sample *s,unsigned char *buf,  int nr)
{
	int i;
	int nRet=0;
	int tmpX=0;
	int tmpY=0;
	struct ts_sample *samp=s;
	struct ts_sample s2;
	
	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	printf("buf=[%x, %x, %x, %x][%x, %x, %x, %x]\n",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
	#endif
	
	if( (buf[1]==0xFF) && (buf[2]==0xFF) && (buf[3]==0xFF) ) return nRet;
	
	tmpX = ((buf[1] & 0xF0)<<4)|buf[2];
	tmpY = ((buf[1] & 0x0F)<<8)|buf[3];
	s2.x = ((buf[4] & 0xF0)<<4)|buf[5] + tmpX;
	s2.y = ((buf[4] & 0x0F)<<8)|buf[6] + tmpY;
	
	if( ( s2.x > 2047 ) && (buf[5] != 0xFF) )
	{
		switch( buf[5]&0x3F )
		{
			case 0x01:
				printf("Key1\n");
				break;
			case 0x02:
				printf("Key2\n");
				break;
			case 0x04:
				printf("Key3\n");
				break;
			case 0x08:
				printf("Key4\n");
				break;
			case 0x10:
				printf("Key5\n");
				break;
			case 0x20:
				printf("Key6\n");
				break;
			default:
				printf("No Key\n");
				break;
		}
		printf("Fingers = 0\n");
	}
	else if((tmpX == s2.x) && (tmpY == s2.y))
	{
		#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
		printf("Fingers = 1\n");
		#endif
		nRet = 1;
	}
	else
	{
		#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
		printf("Fingers = 2\n");
		#endif
		nRet = 1;
	}
	
	#ifdef	ENABLE_SWITCH_XY
	{
		int	tmp = tmpX;
	
		tmpX = tmpY;
		tmpY = tmp;
	}
	#endif	
	
	#ifdef	ENABLE_SCALE_X
	tmpX = (short)(((int)tmpX * g_xResolution)/g_MaxRawY);
	#endif
	
	#ifdef	ENABLE_SCALE_Y
	tmpY = (short)(((int)tmpY * g_yResolution)/g_MaxRawX);
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
				
	samp->x = tmpX;
	samp->y = tmpY;
	samp->pressure = 1;
	gettimeofday(&(samp->tv),NULL);
	
	#ifdef	ENABLE_TOUCH_POSITION_MSG
	printf("p1=%x, x=%d, y=%d, (%d,%d)\n",samp->pressure,samp->x,samp->y, s2.x,s2.y);
	#endif

	return nRet;
}

static int _getTouchSample(struct ts_sample *samp, int nr)
{
	int i2cret;
	int real_nr=0;
	unsigned char qbuf;
	unsigned char buf[8]={0};
	
	_initSample(samp, nr);
	
	//iic get 8 bytes
	i2cret = _readPointBuffer(g_tchDevFd, buf);
	if(i2cret<0)
	{
  		printf("tch.err.read_fail.1,qbuf=%x\n",buf[0]);
		return -1;
	}	
	
	if(_checkPointBuffer(buf)==false)	return -1;
	
	_palseTouchSample(samp, buf, nr);
	
	return 0;	
}

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
			printf("GPIO_reg=%x\n",regValue);
			if ( !(regValue & TP_GPIO_MASK) )
			#endif
			{
				struct ts_sample tmpSamp;
				
				#ifdef ENABLE_TOUCH_PANEL_DBG_MSG
				printf("got INTr!\n");
				#endif
				
				_getTouchSample(&tmpSamp, 1);
				
				pthread_mutex_lock(&gTpMutex);				
				memcpy((unsigned char*)&g_sample, (unsigned char*)&tmpSamp, sizeof(struct ts_sample));
				pthread_mutex_unlock(&gTpMutex);
								
				#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
				//printf("	lastp=%x, dur=%x,p=%x\n",lastp,dur,tmpSamp.pressure);
				#endif
				
				#ifdef CFG_TOUCH_INTR
				g_TouchDownIntr = false;
				ithGpioEnableIntr(TP_GPIO_PIN); 
				#endif
				
				usleep(2000);
			}
			else
			{
				usleep(1000);				
			}
		}
		else
		{
			printf("touch has not init, yet~~~\n");
			usleep(100000);
		}
	}
	return NULL;
}

void _doTpInit(int fd)
{
	int ret;
	
	printf("TP first init(INT is GPIO %d)\n",TP_GPIO_PIN);
	gettimeofday(&startT,NULL);
	
	//init touch GPIO pin 
	_initTouchGpioPin();
	
	#ifdef CFG_TOUCH_INTR	
	_initTouchIntr();	
	#endif
	
	_resetTouchChip();	
	
	//init thread and mutex
	{
	    int res;
	    pthread_t task;
	    pthread_attr_t attr;	

	    pthread_attr_init(&attr);
	    res = pthread_create(&task, &attr, _tpProbeHandler, NULL);
	        
	    if(res)
	    {
	    	printf( "[TouchPanel]%s() L#%ld: ERROR, create _tpProbeHandler() thread fail! res=%ld\n", res );
	    	return;
	    }
	    
	    res = pthread_mutex_init(&gTpMutex, NULL);
    	if(res)
    	{
    	    printf("[AudioLink]%s() L#%ld: ERROR, init touch mutex fail! res=%ld\r\n", __FUNCTION__, __LINE__, res);
    	    return;
    	}
    }
	
	g_tchDevFd = fd;		
	g_IsTpInitialized = true;
}

void _initSample(struct ts_sample *s, int nr)
{
	int i;
	struct ts_sample *samp=s;
	
	for(i = 0; i < nr; i++)
	{
		samp->x = 0;
		samp->y = 0;
		samp->pressure = 0;
		gettimeofday(&(samp->tv),NULL);		
		samp++;
	}
}

void showAhbReg(unsigned int RegBase, unsigned int len)
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
int _getFakeSample(struct ts_sample *samp, int nr)
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
static int msg22s_read(struct tslib_module_info *inf, struct ts_sample *samp, int nr)
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
	
	if(g_IsTpInitialized==false)	_doTpInit(tchdev);
	
	_initSample(s, nr);
	
	//get touch sample	
	pthread_mutex_lock(&gTpMutex);
	memcpy((unsigned char*)s,(unsigned char*)&g_sample, sizeof(struct ts_sample));
	pthread_mutex_unlock(&gTpMutex);
	
	return nr;
}

static const struct tslib_ops msg22s_ops =
{
	msg22s_read,
};

TSAPI struct tslib_module_info *msg22s_mod_init(struct tsdev *dev, const char *params)
{
	struct tslib_module_info *m;

	m = malloc(sizeof(struct tslib_module_info));
	if (m == NULL)
		return NULL;

	m->ops = &msg22s_ops;
	return m;
}

#ifndef TSLIB_STATIC_CASTOR3_MODULE
	TSLIB_MODULE_INIT(msg22s_mod_init);
#endif

/*
 you have to sure 3 things before porting TP driver
 1).INT is work normal
 2).I2C BUS can read data from TP chip
 3).Parse the X,Y coordination correctly

 These function are customized.
 You Have to modify these function with "(*)" mark.
 These functions(3&4) are almost without modification
 Function(5~7) will be modified deponding on chip's feature.
  0._tpInitSpec_vendor()           //set control config(*)
  1._tpReadPointBuffer_vendor()    //read point buffer(*)
  2._tpParseRawPxy_vendor()        //parse the touch point(*)
  3._tpIntActiveRule_vendor()      //touch-down RULE
  4._tpIntNotActiveRule_vendor()   //touch-up RULE
5._tpParseKey_vendor()           //depend on TP with key
6._tpDoPowerOnSeq_vendor();      //depend on TP with power-on sequence
7._tpDoInitProgram_vendor();         //depend on TP with initial programming


application notes:
 1.resolution:800*480
 2.reset pin: it's not necessary(>5ms)
 3.direction: normal(PAD is bottom)
 4.support 5 fingers touch, but it can report over 5 fingers
   (however the registers of 6~N fingers does not match the datasheet's format)
 5.current driver still has some problem that finger ID maybe not in order if single-touch.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <alloca.h>
#include <pthread.h>	
#include "openrtos/FreeRTOS.h"
#include "openrtos/queue.h"
#include "ite/ith.h" 
#include "ite/itp.h"
#include "config.h"
#include "tslib-private.h"

//#include "api-raw.h"

#ifdef	CFG_TOUCH_MULTI_FINGER
    #define TP_MULTI_FINGER_ENABLE
#endif

//#define USE_RAW_API
#define TP_USE_XQUEUE
/****************************************************************************
 * initial Kconfig setting
 ****************************************************************************/

#if	defined(CFG_TOUCH_I2C0) || defined(CFG_TOUCH_I2C1) || defined(CFG_TOUCH_I2C2) || defined(CFG_TOUCH_I2C3)
#define TP_INTERFACE_I2C   (0)
#endif

#if	defined(CFG_TOUCH_SPI) || defined(CFG_TOUCH_SPI0) || defined(CFG_TOUCH_SPI1)
#define TP_INTERFACE_SPI   (1)
#endif

#define TP_INT_PIN	    CFG_GPIO_TOUCH_INT
#define TP_GPIO_MASK    (1<<(TP_INT_PIN%32))

#ifdef	CFG_GPIO_TOUCH_WAKE
#if (CFG_GPIO_TOUCH_WAKE<128)
#define TP_GPIO_WAKE_PIN	CFG_GPIO_TOUCH_WAKE
#endif 
#endif 

#ifdef	CFG_GPIO_TOUCH_RESET
#if (CFG_GPIO_TOUCH_RESET<128)
#define TP_GPIO_RESET_PIN	CFG_GPIO_TOUCH_RESET
#endif 
#endif 

#ifdef	CFG_TOUCH_ADVANCE_CONFIG

#ifdef	CFG_TOUCH_SWAP_XY
#define	TP_SWAP_XY		(1)
#else
#define	TP_SWAP_XY		(0)
#endif

#ifdef	CFG_TOUCH_REVERSE_X
#define	TP_REVERSE_X	(1)
#else
#define	TP_REVERSE_X	(0)
#endif

#ifdef	CFG_TOUCH_REVERSE_Y
#define	TP_REVERSE_Y	(1)
#else
#define	TP_REVERSE_Y	(0)
#endif

#else

#define	TP_SWAP_XY		(0)
#define	TP_REVERSE_X	(0)
#define	TP_REVERSE_Y	(0)

#endif

#define	TOUCH_NO_CONTACT		(0)
#define	TOUCH_DOWN				(1)
#define	TOUCH_UP				(2)

#define	TP_ACTIVE_LOW           (0)
#define	TP_ACTIVE_HIGH          (1)

#ifdef	CFG_GPIO_TOUCH_INT_ACTIVE_HIGH
#define	TP_INT_ACTIVE_STATE     TP_ACTIVE_HIGH
#else
#define	TP_INT_ACTIVE_STATE     TP_ACTIVE_LOW
#endif

#define	TP_INT_LEVLE_TRIGGER    (1)
#define	TP_INT_EDGE_TRIGGER     (0)

#define	TP_INT_TYPE_KEEP_STATE  (0)
#define	TP_INT_TYPE_ZT2083      (0)
#define	TP_INT_TYPE_FT5XXX      (1)
#define	TP_INT_TYPE_IT7260      (2)

#define	TP_WITHOUT_KEY          (0)
#define	TP_HAS_TOUCH_KEY        (1)
#define	TP_GPIO_PIN_NO_DEF      (-1)

#ifdef	CFG_TOUCH_BUTTON
#define	TP_TOUCH_BUTTON		TP_HAS_TOUCH_KEY
#else
#define	TP_TOUCH_BUTTON		TP_WITHOUT_KEY
#endif

#ifdef CFG_TOUCH_INTR
#define	TP_ENABLE_INTERRUPT     (1)
#else
#define	TP_ENABLE_INTERRUPT     (0)
#endif

#ifdef TP_MULTI_FINGER_ENABLE
#define	MAX_FINGER_NUM	(5)		//depend on TP Native Max Finger Numbers  
#else
#define	MAX_FINGER_NUM	(1)
#endif

#ifdef TP_USE_XQUEUE
#define	TP_QUEUE_LEN	(64)
#endif
/****************************************************************************
 * touch cofig setting
 ****************************************************************************/
#define TP_IDLE_TIME                (2000)
#define TP_IDLE_TIME_NO_INITIAL     (100000)

/****************************************************************************
 * ENABLE_TOUCH_POSITION_MSG :: just print X,Y coordination & 
 * 								touch-down/touch-up
 * ENABLE_TOUCH_IIC_DBG_MSG  :: show the IIC command 
 * ENABLE_TOUCH_PANEL_DBG_MSG:: show send-queue recieve-queue, 
 *                              and the xy value of each INTr
 ****************************************************************************/
//#define ENABLE_TOUCH_POSITION_MSG
//#define ENABLE_TOUCH_RAW_POINT_MSG
//#define ENABLE_TOUCH_PANEL_DBG_MSG
//#define ENABLE_TOUCH_IIC_DBG_MSG
//#define ENABLE_SEND_FAKE_SAMPLE

/****************************************************************************
 * MACRO define of hy4633
 ****************************************************************************/
#define TP_I2C_DEVICE_ID       (0x38)
#define TP_SAMPLE_RATE	       (16)

#ifdef	CFG_LCD_ENABLE
#define	TP_SCREEN_WIDTH	    ithLcdGetWidth()
#define	TP_SCREEN_HEIGHT	ithLcdGetHeight()
#else
#define	TP_SCREEN_WIDTH	    (800)
#define	TP_SCREEN_HEIGHT	(480)
#endif
/****************************************
 *
 ***************************************/
typedef struct 
{
	char tpCurrINT;
	char tpStatus;
	char tpNeedToGetSample;
	char tpNeedUpdateSample;
	char tpFirstSampHasSend;
	char tpIntr4Probe;
	char tpIsInitFinished;
	int  tpDevFd;
	int  tpIntrCnt;
}tp_info_tag;//tp_gv_tag

typedef struct tp_spec_tag
{
    //TP H/W setting
    char tpIntPin;		    //INT signal GPIO pin number
    char tpIntActiveState;	//High=1, Low=0
    char tpIntTriggerType;  //interrupt trigger type. 0:edge trigger, 1:level trigger
    char tpWakeUpPin;		//Wake-Up pin GPIO pin number, -1: means NO Wake-Up pin.
    char tpResetPin;		//Reset pin GPIO pin number, -1: means NO reset pin.
    char tpIntrType;		//0:keep state when touch down(like ZT2083), 1:like FT5XXX type 2:like IT7260, 3:others....  
    char tpInterface; 		//0:I2C, 1:SPI, 2:other...
    char tpI2cDeviceId; 	//I2C device ID(slave address) if TP has I2C interface
    char tpHasTouchKey;		//0: NO touch key, 1:touch key type I, 2:touch key type II, ...
    char tpIntUseIsr;	    //0:polling INT siganl, 1:INT use interrupt, 
    char tpMaxFingerNum;	//The TP native maximun of finger numbers
    char tpIntActiveMaxIdleTime;    //default: 33ms, cytma568: 100ms
        
    //TP resolution
    int  tpMaxRawX;
    int  tpMaxRawY;
    int  tpScreenX;
    int  tpScreenY;
    
    //TP convert function
    char tpCvtSwapXY;		//0:Disable, 1:Enable
    char tpCvtReverseX;     //0:Disable, 1:Enable
    char tpCvtReverseY;     //0:Disable, 1:Enable 
    char tpCvtScaleX;		//0:Disable, 1:Enable
    char tpCvtScaleY;		//0:Disable, 1:Enable
    
    //TP sample specification
    char tpEnTchPressure;	//0:disable pressure info, 1:enable pressure info
    char tpSampleNum;		//0:NO scense, 1: single touch 2~10:multi-touch("tpSampleNum" must be <= "tpMaxFingerNum") 
    char tpSampleRate;		//UNIT: mill-second, range 8~16 ms(60~120 samples/per second)  
    
    //TP idle time
    int  tpIdleTime;		//sleep time for polling INT signal(even if interrupt mode).    
    int  tpIdleTimeB4Init;	//sleep time if TP not initial yet.       
    int  tpReadChipRegCnt;	//read register count for getting touch xy coordination
    //int  tpMoveDetectUnit;	//read register count for getting touch xy coordination
    
    //TP specific function
    char tpHasPowerOnSeq;	//0:NO power-on sequence, 1:TP has power-on sequence
    char tpNeedProgB4Init;	//0:TP IC works well without programe flow, 1:TP IC need program before operation.
    char tpNeedAutoTouchUp;
    char tpIntPullEnable;	//use internal pull up/down function    
} TP_SPEC;

/***************************
 * global variable
 **************************/
static struct ts_sample g_sample[MAX_FINGER_NUM];
static struct ts_sample gTmpSmp[10];

static char g_TouchDownIntr = false;
static char  g_IsTpInitialized = false;
static pthread_mutex_t 	gTpMutex;

#ifdef USE_RAW_API
static RA_TP_SPEC  gTpSpec;
static RA_GV       gTpInfo = { 0,RA_TOUCH_NO_CONTACT,1,0,0,0,0,0,0};
#else
static TP_SPEC     gTpSpec;
static tp_info_tag gTpInfo = { 0,TOUCH_NO_CONTACT,1,0,0,0,0,0,0};
#endif

static unsigned int dur=0;
static unsigned int iDur=0;
static unsigned int lowDur=0;

struct timeval T1, T2;
static int g_tpCntr = 0;
static unsigned int gLastNumFinger = 0;

//for the function "_tpFixIntHasNoResponseIssue()"
static int  g_IntrLowCnt = 0;
static int  g_IntrAtvCnt = 0;

struct timeval tv1, tv2;
static int  gNoEvtCnt = 0;

#ifdef TP_USE_XQUEUE
static QueueHandle_t tpQueue;
static int  SendQueCnt = 0;
#endif

/*************************************************
 global variable: gTpKeypadValue
 key0 is pressed if gTpKeypadValue's bit 0 is 1
 key1 is pressed if gTpKeypadValue's bit 1 is 1
   ...and so on

 NO key event if gTpKeypadValue = 0 
 MAX key number: 32 keys
***************************************************/
static uint32_t	gTpKeypadValue;

/*##################################################################################
 *                         the protocol of private function
 ###################################################################################*/
#ifdef CFG_TOUCH_INTR
static void _tp_isr(void* data);
static void _initTouchIntr(void);
#endif

static int _tpCheckMultiPressure(struct ts_sample *theSmp);
static int _checkIfSmpChg(struct ts_sample *s1, struct ts_sample *s2);
/* *************************************************************** */
static void _tpInitSpec_vendor(void);
static int _tpReadReg_vendor(unsigned char regAddr, unsigned char *dBuf, unsigned char dLen);
static int _tpWriteRegData_vendor(unsigned char *data, unsigned char len);
static int  _tpReadPointBuffer_vendor(unsigned char *buf, int cnt);
static int  _tpParseRawPxy_vendor(struct ts_sample *s, unsigned char *buf);
static void _tpParseKey_vendor(struct ts_sample *s, unsigned char *buf);

static void _tpIntActiveRule_vendor(struct ts_sample *tpSmp);
static void _tpIntNotActiveRule_vendor(struct ts_sample *tpSmp);

static void _tpDoPowerOnSeq_vendor(void);
static int _tpDoInitProgram_vendor(void);

/* *************************************************************** */

/* *************************************************************** */
static int  _tpDoInitial(void);
static void _tpInitWakeUpPin(void);
static void _tpInitResetPin(void);
static void _tpInitIntPin(void);
static void _tpInitTouchGpioPin(void);

static void _initSample(struct ts_sample *s, int nr);
static char _tpGetIntr(void);
/* *************************************************************** */

/* *************************************************************** */
static void _tpGetRawPoint(struct ts_sample *samp, int nr);
static void _tpConvertRawPoint(struct ts_sample *samp, int nr);
static void _tpUpdateLastXY(struct ts_sample *smp);

static bool _tpNeedToGetSample(void);
static void _tpGetSample(struct ts_sample *samp, int nr);
static void _tpUpdate(struct ts_sample *tpSmp);
/* *************************************************************** */

static void* _tpProbeHandler(void* arg);
static int  _tpProbeSample(struct ts_sample *samp, int nr);

/*##################################################################################
 *                        the private function implementation
 ###################################################################################*/
static void _tpInitSpec_vendor(void)
{
    gTpSpec.tpIntPin          	= (char)TP_INT_PIN;           //from Kconfig setting
    gTpSpec.tpWakeUpPin         = (char)TP_GPIO_PIN_NO_DEF;   //from Kconfig setting
    gTpSpec.tpResetPin          = (char)TP_GPIO_RESET_PIN;    //from Kconfig setting
    gTpSpec.tpIntUseIsr         = (char)TP_ENABLE_INTERRUPT;  //from Kconfig setting
    gTpSpec.tpIntActiveState    = (char)TP_INT_ACTIVE_STATE;        //from Kconfig setting    
    gTpSpec.tpIntTriggerType    = (char)TP_INT_EDGE_TRIGGER; //from Kconfig setting   level/edge
    
    gTpSpec.tpInterface         = (char)TP_INTERFACE_I2C;	  //from Kconfig setting
    gTpSpec.tpIntActiveMaxIdleTime = (char)TP_SAMPLE_RATE;	  //from Kconfig setting
        
    gTpSpec.tpMaxRawX           = (int)CFG_TOUCH_X_MAX_VALUE; //from Kconfig setting
    gTpSpec.tpMaxRawY           = (int)CFG_TOUCH_Y_MAX_VALUE; //from Kconfig setting
    gTpSpec.tpScreenX           = (int)TP_SCREEN_WIDTH;       //from Kconfig setting
    gTpSpec.tpScreenY           = (int)TP_SCREEN_HEIGHT;      //from Kconfig setting
    
    gTpSpec.tpCvtSwapXY        = (char)TP_SWAP_XY;            //from Kconfig setting
    gTpSpec.tpCvtReverseX      = (char)TP_REVERSE_X;          //from Kconfig setting
    gTpSpec.tpCvtReverseY      = (char)TP_REVERSE_Y;          //from Kconfig setting
    gTpSpec.tpCvtScaleX        = (char)0;                    //from Kconfig setting
    gTpSpec.tpCvtScaleY        = (char)0;                    //from Kconfig setting
    
    gTpSpec.tpI2cDeviceId       = (char)TP_I2C_DEVICE_ID;	  //from this driver setting
    gTpSpec.tpEnTchPressure     = (char)0;                    //from this driver setting
    gTpSpec.tpSampleNum         = (char)MAX_FINGER_NUM;       //from this driver setting
    gTpSpec.tpSampleRate        = (char)TP_SAMPLE_RATE;       //from this driver setting
    gTpSpec.tpIntrType          = (char)TP_INT_TYPE_FT5XXX;	  //from this driver setting
    gTpSpec.tpHasTouchKey       = (char)TP_WITHOUT_KEY;       //from this driver setting                                                               
    gTpSpec.tpIdleTime          = (int)TP_IDLE_TIME;          //from this driver setting
    gTpSpec.tpIdleTimeB4Init    = (int)TP_IDLE_TIME_NO_INITIAL;//from this driver setting    
    gTpSpec.tpReadChipRegCnt    = (int)33;
    //gTpSpec.tpMoveDetectUnit    = (int)1;

    //special initial flow
    gTpSpec.tpHasPowerOnSeq     = (char)1;
    gTpSpec.tpNeedProgB4Init    = (char)0;    
    gTpSpec.tpNeedAutoTouchUp	= (char)0; 

/*
    printf("gTpSpec.tpIntPin         = %d\n",gTpSpec.tpIntPin);
    printf("gTpSpec.tpIntActiveState = %x\n",gTpSpec.tpIntActiveState);
    printf("gTpSpec.tpWakeUpPin      = %d\n",gTpSpec.tpWakeUpPin);
    printf("gTpSpec.tpResetPin       = %d\n",gTpSpec.tpResetPin);
    printf("gTpSpec.tpIntrType       = %x\n",gTpSpec.tpIntrType);
    printf("gTpSpec.tpInterface      = %x\n",gTpSpec.tpInterface);
    printf("gTpSpec.tpI2cDeviceId    = %x\n",gTpSpec.tpI2cDeviceId);
    printf("gTpSpec.tpHasTouchKey    = %x\n",gTpSpec.tpHasTouchKey);
    printf("gTpSpec.tpIntUseIsr      = %x\n",gTpSpec.tpIntUseIsr);
    printf("gTpSpec.tpMaxRawX        = %d\n",gTpSpec.tpMaxRawX);
    printf("gTpSpec.tpMaxRawY        = %d\n",gTpSpec.tpMaxRawY);
    printf("gTpSpec.tpScreenX        = %d\n",gTpSpec.tpScreenX);
    printf("gTpSpec.tpScreenY        = %d\n",gTpSpec.tpScreenY);
    printf("gTpSpec.tpCvtSwapXY     = %x\n",gTpSpec.tpCvtSwapXY);
    printf("gTpSpec.tpCvtReverseX   = %x\n",gTpSpec.tpCvtReverseX);
    printf("gTpSpec.tpCvtReverseY   = %x\n",gTpSpec.tpCvtReverseY);
    printf("gTpSpec.tpCvtScaleX     = %x\n",gTpSpec.tpCvtScaleX);
    printf("gTpSpec.tpCvtScaleY     = %x\n",gTpSpec.tpCvtScaleY);
    printf("gTpSpec.tpEnTchPressure  = %x\n",gTpSpec.tpEnTchPressure);
    printf("gTpSpec.tpSampleNum      = %x\n",gTpSpec.tpSampleNum);
    printf("gTpSpec.tpSampleRate     = %x\n",gTpSpec.tpSampleRate);
    printf("gTpSpec.tpIdleTime       = %d\n",gTpSpec.tpIdleTime);
    printf("gTpSpec.tpIdleTimeB4Init = %d\n",gTpSpec.tpIdleTimeB4Init);
    printf("gTpSpec.tpHasPowerOnSeq  = %x\n",gTpSpec.tpHasPowerOnSeq);
    printf("gTpSpec.tpNeedProgB4Init = %x\n",gTpSpec.tpNeedProgB4Init);
	printf("gTpSpec.tpNeedAutoTouchUp= %x\n",gTpSpec.tpNeedAutoTouchUp);
*/
    //initial global variable "gTpInfo"
/*    
    printf("gTpInfo.tpCurrINT              = %x\n",gTpInfo.tpCurrINT);
    printf("gTpInfo.tpStatus               = %x\n",gTpInfo.tpStatus);
    printf("gTpInfo.tpNeedToGetSample      = %x\n",gTpInfo.tpNeedToGetSample);
    printf("gTpInfo.tpNeedUpdateSample     = %x\n",gTpInfo.tpNeedUpdateSample);
    printf("gTpInfo.tpFirstSampHasSend     = %x\n",gTpInfo.tpFirstSampHasSend);
    printf("gTpInfo.tpFirstSampHasSend     = %x\n",gTpInfo.tpIsInitFinished);
    printf("gTpInfo.tpIntr4Probe           = %x\n",gTpInfo.tpIntr4Probe);
    printf("gTpInfo.tpDevFd                = %x\n",gTpInfo.tpDevFd);    
    printf("gTpInfo.tpIntrCnt              = %x\n",gTpInfo.tpIntrCnt);
*/
}    

static void _tpDoPowerOnSeq_vendor(void)
{
	//for hy4633 power on sequence.	
    if(gTpSpec.tpResetPin == (char)-1)
    {
        //printf("SKIP POWER-ON sequence, reset pin:%d\n",gTpSpec.tpResetPin);	
    }
    else
    {
        printf("DO POWER-ON sequence, reset pin:%d\n",gTpSpec.tpResetPin);	        
    	//1.set "Reset pin" & "INT pin" are output-low for 10ms	
    	ithGpioSetMode( gTpSpec.tpResetPin, ITH_GPIO_MODE0);
    	ithGpioClear(gTpSpec.tpResetPin);
    	ithGpioSetOut(gTpSpec.tpResetPin);
    	ithGpioEnable(gTpSpec.tpResetPin);	
    	
    	//pull low for >5ms
    	usleep(5*1000);
    
        //2.set "Reset pin" output HIGH for 10ms
    	ithGpioSet(gTpSpec.tpResetPin);
    	usleep(5*1000);
    }
}

static int _tpDoInitProgram_vendor(void)
{
    //TODO: program touch IC for initial at first.(like IT7260)
    return 0;
}

static bool _tpChkIntActive(void)
{
	unsigned int regValue = ithGpioGet(TP_INT_PIN);
	
    if(gTpSpec.tpIntActiveState)
    {
    	if ( regValue & TP_GPIO_MASK )	return true;
    	else	return false;	    
    }
    else
    {
    	if ( !(regValue & TP_GPIO_MASK) )	return true;
    	else	return false;	
    }
}

static int _tpReadReg_vendor(unsigned char regAddr, unsigned char *dBuf, unsigned char dLen)
{
	ITPI2cInfo evt;
	unsigned char	I2cCmd;
	int 			i2cret;

	#ifdef EN_DISABLE_ALL_INTR
	portSAVEDISABLE_INTERRUPTS();
	#endif

	I2cCmd = regAddr;	//1000 0010
	evt.slaveAddress   = gTpSpec.tpI2cDeviceId;
	evt.cmdBuffer      = &I2cCmd;
	evt.cmdBufferSize  = 1;
	evt.dataBuffer     = dBuf;
	evt.dataBufferSize = dLen;

	i2cret = read(gTpInfo.tpDevFd, &evt, 1);

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

static int _tpWriteRegData_vendor(unsigned char *data, unsigned char len)
{
	ITPI2cInfo evt;
	int i2cret;

	#ifdef EN_DISABLE_ALL_INTR
	portSAVEDISABLE_INTERRUPTS();
	#endif

	evt.slaveAddress   = gTpSpec.tpI2cDeviceId;
	evt.cmdBuffer      = data;
	evt.cmdBufferSize  = len;
	evt.dataBuffer     = 0;
	evt.dataBufferSize = 0;
	i2cret = write(gTpInfo.tpDevFd, &evt, 1);

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

/****************************************************************
input: 
    buf: the buffer base, 
    cnt: the buffer size in bytes
output: 
    0: pass(got valid data)
    1: skip sample this time 
    -1: i2c error (upper-layer will send touch-up event)
*****************************************************************/
static int _tpReadPointBuffer_vendor(unsigned char *buf, int cnt)
{
    ITPI2cInfo *evt;
	unsigned char	I2cCmd;
    int i2cret;
	//unsigned char i;
	//unsigned char tmpId = 0x00;

    evt = alloca(sizeof(ITPI2cInfo));
    
    I2cCmd = 0x00;		//1100 0010
    evt->slaveAddress   = gTpSpec.tpI2cDeviceId;
    evt->cmdBuffer      = &I2cCmd;
    evt->cmdBufferSize  = 0;
    evt->dataBuffer     = buf;
    evt->dataBufferSize = cnt;
    i2cret = read(gTpInfo.tpDevFd, evt, 1);
    if(i2cret<0)	return -1;
    
    #ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
    {
        int i,c = 16;
        //if(cnt > 7) c = 1 + 6*(buf[2]&0x07);
        printf("\n	raw-buf[%02x]:",tmpId);
        for(i=0; i<c; i++)
        {
            printf("%02x ",buf[i]);
            if( (i&0x03)==0x03 )	printf(" ");
            if( (i&0x0F)==0x0F )	printf("\n		    ");
        }
        printf("\n\n");		
    }
    #endif	
    	
	return 0;
}                               


/*
 In multi-finger case, parse the registers of TP controller, event if fingers became zero.
 Because it still has its touch-up event need to be reported when finger is 0.
*/
static int _tpParseRawPxy_vendor(struct ts_sample *s, unsigned char *buf)
{
	int i=0;
	char device_mode = 0;
	char numOfTchPnt = 0;
	unsigned int lfnum=gLastNumFinger;
	char loopNum = 0;
	
	device_mode = buf[0];
	numOfTchPnt = buf[2] & 0x7;
	
	/* TP RUN MODE buf[0] == 0 :work mode; 0xC0:test mode */
	if(device_mode != 0) return (-1); 

	loopNum = numOfTchPnt;
	
	if(lfnum < numOfTchPnt)
	{
		//printf("fgr_chg++: %d, %d\n", lfnum, numOfTchPnt);
	}
	
	if(loopNum)
	{		
		struct ts_sample *smp = (struct ts_sample*)s;		

		for(i = 0; i < loopNum; i++)
		{
			if( (buf[3+6*i] >> 6) == 0x01 )	smp->pressure = 0;
			else	smp->pressure = 1;
				
			smp->id = (unsigned int)(buf[5+6*i] >> 4); //touch id
			smp->finger = loopNum;
			smp->x = (int)((((unsigned int)buf[3+6*i]<<8)&0x0F00) | ((unsigned int)buf[4+6*i]));
			smp->y = (int)((((unsigned int)buf[5+6*i]<<8)&0x0F00) | ((unsigned int)buf[6+6*i]));

            //skip the sample that finger id > MAX_FINGER_NUM
			if( (smp->id >= gTpSpec.tpSampleNum) && (gTpSpec.tpSampleNum > 1) )
			{
			    //clear this sample & continue
			    smp->finger = 0;			    
			    smp->pressure = 0;
			    smp->x = 0;
			    smp->y = 0;
			    continue;
			}

			printf("	RAW->[%d][%x, %d, %d]--> %d %d %d\n", i, smp, smp->id, smp->finger, smp->pressure, smp->x, smp->y);
			if(gTpSpec.tpSampleNum > 1)   smp++;
		}
		
		pthread_mutex_lock(&gTpMutex);
		gLastNumFinger = s->finger;
		pthread_mutex_unlock(&gTpMutex);	
			
		return 0;
	}
	
	return (-1);
}

static void _tpParseKey_vendor(struct ts_sample *s, unsigned char *buf)
{
    //TODO: get key information and input to xy sample...? as a special xy?
    //maybe define a special area for key
    //(like touch is 800x480, for example, y>500 for key, x=0~100 for keyA, x=100~200 for keyB... )
    //SDL layer could parse this special defination xy into key event(but this layer is not ready yet).
}

static void _tpIntActiveRule_vendor(struct ts_sample *tpSmp)
{
    gTpInfo.tpIntrCnt = 0;
    gTpInfo.tpNeedUpdateSample = 0;
    
    if(!gTpSpec.tpIntUseIsr)
    {
        //for prevent from the issue that polling INT signal will get the same sample.
        if(!gTpInfo.tpNeedToGetSample)	return;
        else    gTpInfo.tpNeedToGetSample = 0;
    }
    
    //status rule for TOUCH_DOWN/TOUCH_UP/TOUCH_NO_CONTACT
	switch(gTpInfo.tpStatus)
	{
		case TOUCH_NO_CONTACT:
			if (_tpCheckMultiPressure(tpSmp) )
			{
				//printf("\\__tpDn\n");
				gTpInfo.tpStatus = TOUCH_DOWN;
				gTpInfo.tpIntr4Probe = 1;
				gTpInfo.tpNeedUpdateSample = 1;
				gTpInfo.tpFirstSampHasSend = 0;
			}
			break;
		
		case TOUCH_DOWN:
			if ( !_tpCheckMultiPressure(tpSmp) )
			{
				//printf("	__/TchUp:1\n");
				gTpInfo.tpStatus = TOUCH_UP;
			}				
			if(gTpInfo.tpFirstSampHasSend)	gTpInfo.tpNeedUpdateSample = 1;
			break;
			
		case TOUCH_UP:
			if ( !_tpCheckMultiPressure(tpSmp) )
			{
				gTpInfo.tpStatus = TOUCH_NO_CONTACT;
				gTpInfo.tpIntr4Probe = 0;
			}
			else
			{
				gTpInfo.tpStatus = TOUCH_DOWN;
				gTpInfo.tpIntr4Probe = 1;
				gTpInfo.tpNeedUpdateSample = 1;
			}
			break;
			
		default:
			printf("ERROR touch STATUS, need to check it!!\n");
			break;				
	}

	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	printf("	tpStatus=%x, NSQ=%x, cINT=%x, send=%x\n", gTpInfo.tpStatus, gTpInfo.tpNeedUpdateSample, gTpInfo.tpIntr4Probe, gTpInfo.tpFirstSampHasSend);
	#endif		
	
	//use this flag to judge if update the touch sample
	//1.have to update the first TOUCH_DOWN event
	//2.don't update the touch event if UI does not get the first event
	//3.real-time update the X,Y point after send the 1st event
	//4.must send the touch event if last status is touch-up, and INT active again in this time.
	//  to handle the quickly touch case.
	//5.others...
	if(gTpInfo.tpNeedUpdateSample)
	{
		_tpUpdateLastXY(tpSmp);
	}		
	
	if(gTpSpec.tpIntUseIsr)
	{
	    //clear INT flag and enable interrupt if use ISR to handle INT signal
	    g_TouchDownIntr = 0;
	    ithGpioEnableIntr(TP_INT_PIN); 
	}
}

static void _tpIntNotActiveRule_vendor(struct ts_sample *tpSmp)
{
    if(!gTpSpec.tpIntUseIsr)
    {
        //if INT not active, then set this flag to call _tpGetSample() if next INT active
	    gTpInfo.tpNeedToGetSample = 1;
	}
	
	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	if( (gTpInfo.tpStatus != TOUCH_NO_CONTACT) )
		printf("	UpdateSmp0:INT=%x, ss=(%d,%d)\n",gTpInfo.tpCurrINT, gTpInfo.tpStatus, gTpInfo.tpFirstSampHasSend);
	#endif
	
	//In order to prevent from loss of the first touch event
	//Need To set "status=TOUCH_NO_CONTACT" if "last status=TOUCH_UP" + "first sample has send"
	if( (gTpInfo.tpStatus == TOUCH_UP) && (gTpInfo.tpFirstSampHasSend) )
	{
        _tpUpdateLastXY(NULL);
	    gTpInfo.tpStatus = TOUCH_NO_CONTACT;
	    gTpInfo.tpIntr4Probe = 0;

		#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
		printf("INT=0, force to set status=0!!\n");
		#endif
	}
	
	//For prevent from lossing the touch-up event
	//sometimes, S/W can not get TOUCH-UP event when INT is actived
	//So, this code will force to set touch-status as TOUCH_UP after INT is not actived for a specific time(16ms)
	if( gTpSpec.tpNeedAutoTouchUp && (gTpInfo.tpStatus == TOUCH_DOWN) && (_tpGetIntr()==false) )
	{
		static uint32_t tc1=0;
		
	    //printf("	UdSmp:s=%d, int=%x, ic=%d\n",gTpInfo.tpStatus,gTpInfo.tpCurrINT,gTpInfo.tpIntrCnt);
	    
	    if(!gTpInfo.tpIntrCnt)	tc1 =itpGetTickCount();
	    dur = itpGetTickDuration(tc1);

		if( gTpInfo.tpFirstSampHasSend && (gTpInfo.tpIntrCnt > 3) )
		{
			//when first smaple has send, or main-loop idle over 33 ms.
			//for fixing the FT5XXX's issue that sometimes it cannot get the TOUCH_UP EVENT
			//and need "gTpInfo.tpIntrCnt" > 3 times to prevent from main task idle issue
			if( (gTpSpec.tpIntrType == TP_INT_TYPE_ZT2083) || (dur > gTpSpec.tpSampleRate) )
			{
				//FORCE TOUCH_UP if TP_INT_TYPE_ZT2083 or dur > one-sample-rate-time
				//printf("	__/TchUp:2\n");
				gTpInfo.tpStatus = TOUCH_UP;
				gTpInfo.tpIntr4Probe = 0;
				_tpUpdateLastXY(NULL);					
				
				#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
				printf("INT=0, and dur>%dms, so force to set status=2!!\n",gTpSpec.tpSampleRate);
				#endif
				//printf("INT=0, and dur>%dms, force TOUCH_UP!!\n",gTpSpec.tpSampleRate);
			}
		}
		
		gTpInfo.tpIntrCnt++;
	}

	//to handle the INT actived, but g_TouchDownIntr doesn't become true.
	//need send a i2c read command to clear INT for IT7260.
	//If INT will keep active state until I2C send command to TP IC for clearing INT active state(like IT7260).
	//Then this workaround will be necessary for fixing the issue 
	//which TP's INT signal has NO response after suspend mode
    if(gTpSpec.tpIntrType == TP_INT_TYPE_IT7260)
    {
        //_tpFixIntHasNoResponseIssue();
    	if( gTpSpec.tpIntUseIsr && (_tpChkIntActive()==true) )
    	{
    		static uint32_t tc2=0;
    		if(!g_IntrLowCnt++)	tc2 =itpGetTickCount();
	    	iDur = itpGetTickDuration(tc2);
    	    
    	    if(iDur>gTpSpec.tpIntActiveMaxIdleTime)//need send read touch-data-command for INT Non-active time()
    	    {
     			unsigned char *buf = (unsigned char *)malloc(gTpSpec.tpReadChipRegCnt);
     			memset(buf, 0, gTpSpec.tpReadChipRegCnt);
     			if(_tpReadPointBuffer_vendor(buf, gTpSpec.tpReadChipRegCnt)>=0)
     			{     			    
     			    if(gTpSpec.tpHasTouchKey)
     			    {
     			    	struct ts_sample s1;
     			    	_tpParseKey_vendor(&s1, buf);
     			    }
     			}
     			g_IntrLowCnt = 0;
     			if(buf!=NULL)	free(buf);
     			g_TouchDownIntr = true;
     			//printf("read Sample while INT is active\n");
    		}
    	}
    	else
    	{
    	    g_IntrLowCnt = 0;
    	}
    }
}

/*##################################################################################
 *                middle APIs for handling raw x,y data
 ###################################################################################*/
#ifdef CFG_TOUCH_INTR
static void _tp_isr(void* data)
{	
	unsigned int regValue;
	
	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	//ithPrintf("$in\n");
	#endif

	g_TouchDownIntr = true;

    ithGpioClearIntr(TP_INT_PIN);
    if(gTpSpec.tpIntrType == TP_INT_TYPE_KEEP_STATE)
    {
        ithGpioDisableIntr(TP_INT_PIN); 
    }
	
	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	//ithPrintf("$out(%x)\n",g_TouchDownIntr);
	#endif
}

static void _initTouchIntr(void)
{
	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
    printf("TP init in\n");	
    #endif
    
    ithEnterCritical();
    
    ithGpioClearIntr(TP_INT_PIN);
    ithGpioRegisterIntrHandler(TP_INT_PIN, _tp_isr, NULL);
        
    if(gTpSpec.tpIntTriggerType==TP_INT_LEVLE_TRIGGER)     
        ithGpioCtrlEnable(TP_INT_PIN, ITH_GPIO_INTR_LEVELTRIGGER);
    else
        ithGpioCtrlDisable(TP_INT_PIN, ITH_GPIO_INTR_LEVELTRIGGER);
        
    if(gTpSpec.tpIntTriggerType==TP_INT_EDGE_TRIGGER)  //if edge trigger
        ithGpioCtrlDisable(TP_INT_PIN, ITH_GPIO_INTR_BOTHEDGE);		//set as single edge
//    else
//        ithGpioCtrlEnable(TP_INT_PIN, ITH_GPIO_INTR_BOTHEDGE);		//set as single edge
    
    if(gTpSpec.tpIntActiveState==TP_ACTIVE_HIGH)    
        ithGpioCtrlDisable(TP_INT_PIN, ITH_GPIO_INTR_TRIGGERFALLING);	//set as rising edge
    else
        ithGpioCtrlEnable(TP_INT_PIN, ITH_GPIO_INTR_TRIGGERFALLING);	//set as falling edge

    ithIntrEnableIrq(ITH_INTR_GPIO);
    ithGpioEnableIntr(TP_INT_PIN);
        
    ithExitCritical();
    
    #ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
    printf("TP init out\n");	
    #endif
}
#endif

static void _tpInitWakeUpPin(void)
{
	if( (gTpSpec.tpWakeUpPin>0) && (gTpSpec.tpWakeUpPin<128) )
	{
		ithGpioSetMode(gTpSpec.tpWakeUpPin,ITH_GPIO_MODE0);
   		ithGpioSetOut(gTpSpec.tpWakeUpPin);
   		ithGpioSet(gTpSpec.tpWakeUpPin);    	
   		ithGpioEnable(gTpSpec.tpWakeUpPin);
   	}
	else
	{
		printf("NOT initial TOUCH_GPIO_WAKE_PIN\n");
	}
}

static void _tpInitResetPin(void)
{
	if( (gTpSpec.tpResetPin>0) && (gTpSpec.tpResetPin<128) )
	{
		ithGpioSetMode(gTpSpec.tpResetPin,ITH_GPIO_MODE0);
   		ithGpioSetOut(gTpSpec.tpResetPin);
   		ithGpioSet(gTpSpec.tpResetPin);    	
   		ithGpioEnable(gTpSpec.tpResetPin);
   	}
	else
	{
		printf("NOT initial TOUCH_RESET_PIN(%d)\n",gTpSpec.tpResetPin);
	}
}

static void _tpInitIntPin(void)
{
	ithGpioSetMode(TP_INT_PIN, ITH_GPIO_MODE0);
	ithGpioSetIn(TP_INT_PIN);
	ithGpioCtrlEnable(TP_INT_PIN, ITH_GPIO_PULL_ENABLE);
	
	if(gTpSpec.tpIntActiveState)    ithGpioCtrlDisable(TP_INT_PIN, ITH_GPIO_PULL_UP);  
	else    ithGpioCtrlEnable(TP_INT_PIN, ITH_GPIO_PULL_UP);  
    
	ithGpioEnable(TP_INT_PIN);	
}

static void _tpInitTouchGpioPin(void)
{
	_tpInitWakeUpPin();
	
	_tpInitResetPin();
	
	_tpInitIntPin();
	
	#ifdef CFG_TOUCH_INTR
	_initTouchIntr();
	#endif
}

static char _tpGetIntr(void)
{
    if(gTpSpec.tpIntUseIsr)
    {
        //printf("GetInt1:%x\n",g_TouchDownIntr);
        return g_TouchDownIntr;
    }
    else
    {
    	return _tpChkIntActive();
    }
}

static void _initSample(struct ts_sample *s, int nr)
{
	int i;
	struct ts_sample *samp=s;	
	struct ts_sample *ns = (struct ts_sample*)s->next;	
	
	for(i = 0; i < nr; i++)
	{
		samp->finger = 0;
		samp->id = 0;
		samp->x = 0;
		samp->y = 0;
		samp->pressure = 0;
		gettimeofday(&(samp->tv),NULL);		
			
		if(i)   samp++;
		else    samp = (struct ts_sample*)ns;
	}
}

/*
send ts to os
*/
static void _tpSendQue2Sample(struct ts_sample *os, struct ts_sample *ts)
{
	int ret=0,i;
    int fn = (int)ts->finger;
	struct ts_sample *s = (struct ts_sample *)os;
	struct ts_sample *tts = (struct ts_sample *)ts;
	struct ts_sample *tmpNxt=(struct ts_sample *)os->next;
	int sz = (int)sizeof(struct ts_sample);
	int loop = fn;
	
	if( (os==NULL) || (ts==NULL) )
	{
		printf("NULL pointer, STOP!!\n");
		while(1);
	}
	
	//printf("loop = %d, %d\n",loop,fn);

    for(i=0; i<loop; i++)
    {
    	//printf("prb12:[%d,%d]:%d,%d,%d\n",tts->finger, tts->id, tts->pressure, tts->x, tts->y);
    	//if(tts->finger==2)	printf("prb2:[%d,%d]:%d,%d,%d\n",tts[1]->finger, tts[1]->id, tts[1]->pressure, tts[1]->x, tts[1]->y);

    	if(i>=gTpSpec.tpSampleNum)	break;

		if(!i)
		{
     	    memcpy((void *)s, (void *)ts, sz);
     	    s->next = (struct ts_sample *)tmpNxt;	
     	    gettimeofday(&s->tv,NULL);
     	    
     	    if(s->pressure)	gTpInfo.tpFirstSampHasSend = 1; 
     	    
     	    //printf("prb0:[%d,%d]:%d,%d,%d\n",s->finger, s->id, s->pressure, s->x, s->y);
		}
		else
    	{
    	 	struct ts_sample *s1 = (struct ts_sample *)(&tmpNxt[i-1]);
    	  	struct ts_sample *s2 = (struct ts_sample *)(&ts[i]);

    	   	//s2 = (struct ts_sample *)(++tts);

    	   	if(i >= fn)	break;
    	    		
    	   	if( (s1==NULL) || (s2==NULL) )
    	   	{
    	   		printf("	[TP ERROR]: incorrect pointer:s1=%x, s2=%x\n",s1,s2);
    	   		continue;
    	   	}
    	   	
    	   	s1->pressure = (unsigned int)s2->pressure;
    	   	s1->x = s2->x;
    	   	s1->y = s2->y;
    	   	s1->id = (unsigned int)s2->id;
    	   	//s1->finger = (unsigned int)s2->finger;
    	   	s1->finger = (unsigned int)fn;
    	   	//printf("prb3:[%d,%d,%d]:%d,%d,%d\n",i, s1->finger, s1->id, s1->pressure, s1->x, s1->y);
    	   	//printf("prb13:[%d,%d]:%d,%d,%d\n",s1->finger, s1->id, s1->pressure, s1->x, s1->y);
   	    }
   	}
}

static int _tpCheckMultiPressure(struct ts_sample *theSmp)
{
	struct ts_sample *s = theSmp;
	int cfgr = s->finger;
	int tpPressure = 0;
	int i;
	
	if(cfgr)
	{
		for(i=0; i<cfgr; i++)
		{
			if(s->pressure)	tpPressure++;
			s++;	
		}
	}
	
	if(tpPressure)	return 1;
	else			return 0;	
}

/*
return 0:no change, 1:sample has changed
*/
static int _checkIfSmpChg(struct ts_sample *s1, struct ts_sample *s2)
{
    int doChkDistance = 0;
    int deltaDist = 2;
    
	if(s1->finger != s2->finger)	return 1;
		
	{
		int i;
		int fn = s2->finger;
		
		for(i=0; i<fn; i++)
		{
			if(s1->pressure != s2->pressure)	return 1;
			
			if((fn==1) && doChkDistance)
			{
			    if(s1->x >= s2->x)
			    {
			        if((s1->x - s2->x) >= deltaDist)    return 1;
			    }
			    else
			    {
			        if((s2->x - s1->x) >= deltaDist)    return 1;
			    }
			        
			    if(s1->y >= s2->y)
			    {
			        if((s1->y - s2->y) >= deltaDist)    return 1;
			    }
			    else
			    {
			        if((s2->y - s1->y) >= deltaDist)    return 1;
			    }
			}
			else
			{
			    if(s1->x != s2->x)	return 1;
			
			    if(s1->y != s2->y)	return 1;
			}
			
			if(s1->id != s2->id)	return 1;
				
			s1++; s2++;
		}
	}
	
	return 0;
}

static void _tpGetRawPoint(struct ts_sample *samp, int nr)
{
	int real_nr=0;
	struct ts_sample *s=samp;
	unsigned char *buf = (unsigned char *)malloc(gTpSpec.tpReadChipRegCnt);
	
	_initSample(s, nr);	
	memset(buf, 0, gTpSpec.tpReadChipRegCnt);
	
	while(real_nr++<nr) 
	{
		if(_tpReadPointBuffer_vendor(buf, gTpSpec.tpReadChipRegCnt)<0)	break;
		
		if(_tpParseRawPxy_vendor(s, buf)<0)	break;
		
		if(gTpSpec.tpHasTouchKey)	_tpParseKey_vendor(s, buf);

        break;
	}
	if(buf!=NULL)	free(buf);
}

static void _tpConvertRawPoint(struct ts_sample *samp, int nr)
{
	int real_nr=0;
	int tmpMaxRawX = gTpSpec.tpMaxRawX;
	int tmpMaxRawY = gTpSpec.tpMaxRawY;
	int fgr_nr=0;
	struct ts_sample *s=samp;
	
	#ifdef ENABLE_TOUCH_RAW_POINT_MSG
	printf("	CvtRawPnt:%x,%d,%d\n",s->pressure,s->x,s->y);	
	#endif
	
#ifdef TP_MULTI_FINGER_ENABLE
	fgr_nr = s->finger;
#else
	fgr_nr = 1;
#endif
	
    while(real_nr++<fgr_nr)
    {
        if(!s->pressure)
        {
		    s++;
		    real_nr++;
            continue;
        }

        if(gTpSpec.tpCvtSwapXY)
        {
            int tmp = s->x;
            s->x = s->y;
            s->y = tmp;
            tmpMaxRawX = gTpSpec.tpMaxRawY;
            tmpMaxRawY = gTpSpec.tpMaxRawX;
        }

        if( (gTpSpec.tpHasTouchKey) && (s->y > tmpMaxRawX) )	
        {
            s++;
		    real_nr++;
            continue;//NEED TO VERIFY THIS CODE
        }
        
        if(gTpSpec.tpCvtReverseX)
        {
            if(s->x>=tmpMaxRawX)	s->x = 0;
            else					s->x = tmpMaxRawX - s->x;
        }

        if(gTpSpec.tpCvtReverseY)
        {
            if(s->y>=tmpMaxRawY)	s->y = 0;
            else					s->y = tmpMaxRawY - s->y;
        }

        if(gTpSpec.tpCvtScaleX)
        {
            s->x = (short)(((uint32_t)s->x*gTpSpec.tpScreenX)/tmpMaxRawX);
        }
        
        if(gTpSpec.tpCvtScaleY)
        {
            s->y = (short)(((uint32_t)s->y*gTpSpec.tpScreenY)/tmpMaxRawY);
        }
    			
/*
    	if( (s->x>=gTpSpec.tpScreenX) || (s->y>=gTpSpec.tpScreenY) || (s->x<0) || (s->y<0) )
    		printf("[TP warning] XY are abnormal, x=%d,%d y=%d,%d\n",s->x,gTpSpec.tpScreenX,s->y,gTpSpec.tpScreenY);
    			
    	if(s->x>=gTpSpec.tpScreenX)	s->x = gTpSpec.tpScreenX - 1;
    	if(s->y>=gTpSpec.tpScreenY)	s->y = gTpSpec.tpScreenY - 1;
    			
    	if(s->x<0)	s->x = 0;
    	if(s->y<0)	s->y = 0;
    				
    	//printf("modify x,y = %d, %d -##\n",s->x,s->y);		
*/ 
		
		#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
       	printf("	cvtPXY->--------> %d %d %d\n", s->pressure, s->x, s->y);
		#endif
        
		s++;
    }
}

static void _tpGetSample(struct ts_sample *samp, int nr)
{
	int real_nr=0;
	struct ts_sample *s=samp;
	
		_tpGetRawPoint(s, 1);		
		
		_tpConvertRawPoint(s, 1);
}

#if 0	
/*	
only report finger id 0 ~ gTpSpec.tpSampleNum
EX1: finger number = 5, gTpSpec.tpSampleNum = 2, current id list is 1,3,0,5,4 by sequence
    then report 0th & 2nd samples(exchange the sequence of samples "smp")

EX2: finger number = 4, gTpSpec.tpSampleNum = 2, current id list is 3,2,0,5 by sequence
    then report Only 2nd sample(ID=1 is not exist)
*/
static void _tpReduceSamples(struct ts_sample *smp)
{
	struct ts_sample *ts = (struct ts_sample *)smp;
	struct ts_sample *tsBase;
	struct ts_sample *tmpS;
	int currFgrNum = (int)smp->finger;
	int maxFgrNum = (int)gTpSpec.tpSampleNum;
	uint8_t valid_i[10] = {0xFF};
	uint8_t cId = 0;
	uint8_t got0thPressureIsZero = 0;
	uint8_t swapIndex = 0;
	int totalFgrNum = 0;
	int i;
	int k=0;
	int need_log = 0;
	
	//if(currFgrNum <= gTpSpec.tpSampleNum)	return;
	
	tsBase = (struct ts_sample *)( malloc(sizeof(struct ts_sample)*maxFgrNum) );
	if(tsBase==NULL)
	{
		printf("TP Error!! tsBase is out of memory!!\n");
		return;
	}
	
	tmpS = (struct ts_sample *)tsBase;
/*
	{
		struct ts_sample *tSp = (struct ts_sample *)smp;
		
		printf("origin Smp:\n");
		for(i=0; i<currFgrNum; i++)	
		{
			printf("	oriSp[%d] fn=%d, id=%d, pxy=%d,%d,%d\n", i,tSp->finger,tSp->id,tSp->pressure,tSp->x, tSp->y);
			tSp++;
		}
	}
*/
	//find out 0 ~ maxFgrNum id
	for(k=0; k<maxFgrNum; k++)
	{
		ts = (struct ts_sample *)smp;
		for(i=0; i<currFgrNum; i++)	
		{
			cId = (uint8_t)ts->id;
			
			if(cId == k)
			{
				valid_i[k] = i;	//IDth in ith sample
				totalFgrNum++;
				
				if( (k==0) && !ts->pressure )	got0thPressureIsZero = 1;
				if(got0thPressureIsZero && k && ts->pressure)	swapIndex = k;
					
				break;
			}
			ts++;
		}
	}
	
	if(got0thPressureIsZero && swapIndex)
	{
		uint8_t tmpIdx = valid_i[0];
		
		printf("Need Swap: %x, %x, %x\n",swapIndex,valid_i[0],valid_i[swapIndex]);
		
		valid_i[0] = valid_i[swapIndex];
		valid_i[swapIndex] = tmpIdx;
	}

	//ts = (struct ts_sample *)smp;
	
	//if the pressureof 0th sample  is 0, then swap sample sequence with the sampe(with p=1)

/*
	printf("valid ID index:[");
	for(i=0; i<gTpSpec.tpSampleNum; i++)	printf("%x, ", valid_i[i]);
	printf("]\n");
*/
	for(i=0; i<totalFgrNum; i++)	
	{
		char ci = valid_i[i];		
		
		if( ci != 0xFF )
		{
			ts = (struct ts_sample *)(&smp[ci]);	
			//printf("ci=%x, smp=%x, ts=%x\n",ci,smp,ts);	
			memcpy(tmpS, ts, sizeof(struct ts_sample));
			
			if(tmpS->id>2)	need_log = 1;
			if( tmpS->pressure!=0 && tmpS->pressure!=1 )	need_log = 1;
			if(tmpS->x>400)	need_log = 1;
			if(tmpS->y>1280)	need_log = 1;	
				
			if(need_log)
			{
				printf("	tmpS[%d,%d] bs=%x fn=%d, id=%d, pxy=%d,%d,%d\n", i,ci, tmpS,tmpS->finger,tmpS->id,tmpS->pressure,tmpS->x, tmpS->y);
				printf("	ts[%d,%d] bs=%x fn=%d, id=%d, pxy=%d,%d,%d\n", i,ci, ts, ts,ts->finger,ts->id,ts->pressure,ts->x, ts->y);
			}
			
			tmpS->finger = (unsigned int)totalFgrNum;
			tmpS++;
		}
	}
	
	if(0)//need_log)
	{
		struct ts_sample *tSp = (struct ts_sample *)smp;
		
		printf("newSmp: smp=%x\n",tSp);
		for(i=0; i<totalFgrNum; i++)	
		{
			printf("	smp[%d] bs=%x fn=%d, id=%d, pxy=%d,%d,%d\n", i,tSp,tSp->finger,tSp->id,tSp->pressure,tSp->x, tSp->y);
			tSp++;
		}
	}

	memcpy(smp, tsBase, sizeof(struct ts_sample)*totalFgrNum);

	{
		struct ts_sample *tSp = (struct ts_sample *)smp;
		int ShowLog = 0;
		
			if(tmpS->id>2)	ShowLog = 1;
			if( tmpS->pressure!=0 && tmpS->pressure!=1 )	ShowLog = 1;
			if(tmpS->x>400)	ShowLog = 1;
			if(tmpS->y>1280)	ShowLog = 1;	
		
		if(ShowLog)
		{
			printf("<newSmp>: smp=%x\n",tSp);
			for(i=0; i<totalFgrNum; i++)	
			{
				printf("	smpA[%d] bs=%x fn=%d, id=%d, pxy=%d,%d,%d\n", i,tSp,tSp->finger,tSp->id,tSp->pressure,tSp->x, tSp->y);
				tSp++;
			}
		}
	}

	{
		char need_show_log = 0;
		struct ts_sample *tSp = (struct ts_sample *)smp;
		
		//printf("newSmp:\n");
		for(i=0; i<totalFgrNum; i++)	
		{
			//printf("	smp[%d] fn=%d, id=%d, pxy=%d,%d,%d\n", i,tSp->finger,tSp->id,tSp->pressure,tSp->x, tSp->y);
			if(tSp->id >= 2)
			{
				need_show_log = 1;
				break;
			}
			tSp++;
		}
		
		
		if(need_show_log)
		{
			tSp = (struct ts_sample *)smp;
			printf("newSmp2: maxFN=%d, ttlFN=%d\n",maxFgrNum,totalFgrNum);
			for(i=0; i<maxFgrNum; i++)	
			{
				printf("	smpB[%d] bs=%x, fn=%d, id=%d, pxy=%d,%d,%d\n", i,tSp,tSp->finger,tSp->id,tSp->pressure,tSp->x, tSp->y);
				tSp++;
			}
		}
	}
	
	if(tsBase!=NULL)    free(tsBase);
}
#else
/*
if finger number > gTpSpec.tpSampleNum(N), then report only N samples that has the first N minimal ID
EX1: finger number = 5, gTpSpec.tpSampleNum = 2, current id list is 1,3,0,5,4 by sequence
    then report 0th & 2nd samples(exchange the sequence of samples "smp")

EX2: finger number = 4, gTpSpec.tpSampleNum = 2, current id list is 2,3,0,5 by sequence
    then report 0th & 2nd sample(the first 2 minimal ID)
*/
static void _tpReduceSamples(struct ts_sample *smp)
{
	struct ts_sample *ts = (struct ts_sample *)smp;
	struct ts_sample *tsBase;
	struct ts_sample *tmpS;
	int currFgrNum = (int)smp->finger;
	int maxFgrNum = (int)gTpSpec.tpSampleNum;
	uint8_t all_id[10];
	uint8_t valid_i[10];
	uint8_t max_id = 0;
	uint8_t min_id = 255;
	uint8_t cId;
	int i;
	int k=0;
	
	//do swap first sample
	if ( !smp->pressure && _tpCheckMultiPressure(smp) )
	{
	    //check if all p=1 but [0].p = 0
	    struct ts_sample tmpBkS;
	    int sz = (int)sizeof(struct ts_sample);

	    //search the sample with p=1
	    for(i=0; i<currFgrNum; i++)
	    {
	        if(ts->pressure)    break;
	        
	        ts++;
	    }
	    
	    if(!ts->pressure)   printf("TP ERROR: ts->p(%x) != 1, \n",ts->pressure);
        
        //swap data
	    memcpy( &tmpBkS, smp, sz);
	    memcpy( smp, ts, sz);
	    memcpy( ts, &tmpBkS, sz);
	    
	    //reset ts pointer
	    ts = (struct ts_sample *)smp;
	}

	if(currFgrNum <= gTpSpec.tpSampleNum)	return;
	
	tsBase = (struct ts_sample *)( malloc(sizeof(struct ts_sample)*maxFgrNum) );
	if(tsBase==NULL)
	{
		printf("TP Error!! tsBase is out of memory!!\n");
		return;
	}
	
	tmpS = (struct ts_sample *)tsBase;
	
	//find out all finger id & Max/Min ID
	for(i=0; i<currFgrNum; i++)	
	{
		all_id[i] = (unsigned char)ts->id;
		
		if(all_id[i] > max_id) max_id = (unsigned char)ts->id;
			
		if(all_id[i] < min_id) min_id = (unsigned char)ts->id;
			
		ts++;
	}
	
/*
{
	printf("minID=%d, maxID=%d\n",min_id,max_id);
	printf("all ID:[");
	for(i=0; i<currFgrNum; i++)	printf("%d, ", all_id[i]);
	printf("]\n");
}
*/

	ts = (struct ts_sample *)smp;
	
	//find out the N index with MIN finger id 
	for(cId=min_id; cId<(max_id+1); cId++)
	{
		for(i=0; i<currFgrNum; i++)	
		{
			if(cId == all_id[i])
			{
			    //choose ID 0 ~ maxFgrNum by sequence
				valid_i[k++] = i;
				//printf("cid=%d, i=%d, FN=%d, id=%d, k=%d, vi=%d\n",cId, i, currFgrNum, all_id[i], k-1, valid_i[k-1]);
				break;
			}
		}	
		if(k >= maxFgrNum)	break;
	}
/*
	printf("valid ID index:[");
	for(i=0; i<gTpSpec.tpSampleNum; i++)	printf("%d, ", valid_i[i]);
	printf("]\n");
*/
	for(i=0; i<maxFgrNum; i++)	
	{
		char ci = valid_i[i];
		ts = (struct ts_sample *)(&smp[ci]);		
		memcpy(tmpS, ts, sizeof(struct ts_sample));
		tmpS->finger = maxFgrNum;
		tmpS++;
	}

	memcpy(smp, tsBase, sizeof(struct ts_sample)*maxFgrNum);
/*
	{
		struct ts_sample *tSp = (struct ts_sample *)smp;
		
		printf("newSmp:\n");
		for(i=0; i<maxFgrNum; i++)	
		{
			printf("	smp[%d] fn=%d, id=%d, pxy=%d,%d,%d\n", i,tSp->finger,tSp->id,tSp->pressure,tSp->x, tSp->y);
			tSp++;
		}
	}
*/
    if(tsBase!=NULL)    free(tsBase);
}
#endif

static void _tpUpdateLastXY(struct ts_sample *smp)
{
	int smpChgFlag = 0;
	
	pthread_mutex_lock(&gTpMutex);
	if(smp!=NULL)
	{
#ifdef TP_MULTI_FINGER_ENABLE

		//to reduce the sample numbers
		_tpReduceSamples(smp);

        if(smp->finger == 0)
        {
            //if( smp->pressure || smp->x || smp->y)
            if(gTpInfo.tpStatus != TOUCH_UP)
            {
        	    struct ts_sample *tSmp = (struct ts_sample *)smp;
        	    printf("TP err4A:fn=%d, pxy=%d,%d,%d\n",tSmp->finger, tSmp->pressure, tSmp->x,tSmp->y);
        	    tSmp++;
        	    printf("TP err4B:fn=%d, pxy=%d,%d,%d\n",tSmp->finger, tSmp->pressure, tSmp->x,tSmp->y);                
            }  
		    smp->finger = 1;
        }
#else
        smp->finger = 1;
#endif

#ifdef TP_USE_XQUEUE
		smpChgFlag = _checkIfSmpChg(&g_sample[0], smp);
		if( smpChgFlag )
		{
			memcpy((void *)&g_sample[0] ,(void *)smp, sizeof(struct ts_sample)*smp->finger);
			SendQueCnt++;
			//printf("inQ1:%d\n",SendQueCnt);
        	if (xQueueSend(tpQueue, &g_sample[0], 0) != pdTRUE)
        	{
        		printf("	raQuSd1: send queue error QC=%d, (%x,%d,%d)\n",SendQueCnt, g_sample[0].pressure, g_sample[0].x, g_sample[0].y);
        	}			
		}

        gTpInfo.tpFirstSampHasSend = 1;		
#else
        memcpy((void *)&g_sample[0] ,(void *)smp, sizeof(struct ts_sample)*smp->finger);
#endif
	}
	else
	{

#ifdef TP_USE_XQUEUE
		smpChgFlag = _checkIfSmpChg(&g_sample[0], smp);
		
       	if(gTpInfo.tpStatus != TOUCH_NO_CONTACT)
       	{
       		if(smpChgFlag)
       		{
       			memset((void *)&g_sample[0] , 0, sizeof(struct ts_sample));
       			SendQueCnt++;
       			//printf("inQ2:%d\n",SendQueCnt);
       			if (xQueueSend(tpQueue, &g_sample[0], 0) != pdTRUE)	
       			{
       				printf("	raQuSd0: send queue error (%x,%d,%d)\n",g_sample[0].pressure, g_sample[0].x, g_sample[0].y);
       			}
       		}
       	}
#else
       	memset((void *)&g_sample[0] , 0, sizeof(struct ts_sample));
#endif
	}
	
	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	if(smp!=NULL)	printf("	EnQue:p=%x,xy=%d,%d\n", smp->pressure, smp->x, smp->y);
	else            printf("	EnQue:p=%x,xy=%d,%d\n", 0, 0, 0);
	#endif							

	pthread_mutex_unlock(&gTpMutex);
}

/**
 * to judge if S/W needs to get touch point
 *
 * @return: true for YES, false for NO
 * 
 * [NOTE]:return true if need to get touch sample via i2c bus(ex: when TP's INT signal actived, or other special rules)
          return false if NO need to get sample(NO TP's INT actived, or other special rules)
          in usual case, return true for INT actived, false for INT not actived..
          special rule1: To prevent from getting the same sample, when use "pulling INT" without interrupt.
          special rule2: DO NOT wanna loss the first point when finger just touch down(or quickly touch)
 */
static bool _tpNeedToGetSample(void)
{
    if(g_IsTpInitialized!=true)	return false;
    
    if(_tpGetIntr()==true)
    {
        gTpInfo.tpCurrINT = 1;
        
    	if(gTpSpec.tpIntUseIsr)
    	{
    		if( (gTpSpec.tpIntrType == TP_INT_TYPE_ZT2083) || (gTpSpec.tpIntrType == TP_INT_TYPE_IT7260) )
    		{
    			static uint32_t tc3=0;
	    		if(!g_IntrAtvCnt++)	tc3 =itpGetTickCount();
	    		lowDur = itpGetTickDuration(tc3);
		    	
		    	if(lowDur>gTpSpec.tpSampleRate)
		    	{
		    		//printf("	NGS:%x,%d,%d\n", gTpInfo.tpNeedToGetSample, g_IntrLowCnt, dur);	
	    			//printf("	  -ztInt:%x,%x\n",regValue,regValue & TP_GPIO_MASK);
	    			g_IntrAtvCnt = 0;
		    		gTpInfo.tpNeedToGetSample = 1;
		    		ithGpioEnableIntr(TP_INT_PIN); 
		    	}
		    	
	    		if ( gTpInfo.tpNeedToGetSample )	return true;
	    		else	return false;
    		}
    		else
    		{
	    		ithGpioDisableIntr(TP_INT_PIN);
    	        return true;
    		}
    	}
    	else
    	{
        	if( (gTpSpec.tpIntrType == TP_INT_TYPE_ZT2083) || (gTpSpec.tpIntrType == TP_INT_TYPE_IT7260) )
        	{
				//printf("IAR2:%x,%x,%d,%d\n",gTpInfo.tpNeedToGetSample,gTpInfo.tpFirstSampHasSend,g_IntrLowCnt,dur);
				if(gTpInfo.tpFirstSampHasSend)
				{
    				static uint32_t tc3=0;
	    			if(!g_IntrAtvCnt++)	tc3 =itpGetTickCount();
	    			lowDur = itpGetTickDuration(tc3);

			    	if(lowDur>gTpSpec.tpSampleRate)
			    	{
			    		gTpInfo.tpNeedToGetSample = 1;
			    		g_IntrAtvCnt = 0;
			    	}
				}
				else
				{
					gTpInfo.tpNeedToGetSample = 1;
				}
        	}
 
        	if( gTpInfo.tpNeedToGetSample )    return true;	
        	else    return false;    	
    	}
    }
    else
    {
    	g_IntrAtvCnt = 0;
        gTpInfo.tpCurrINT = 0;
        return false;
    }
}

/**
 * to update the touch status
 * 
 * [HINT 1]: when use "pulling INT", remember to prevent from getting the same sample
 * [HINT 2]: DO NOT loss the first point(each point) in quickly clicking case.
 * [HINT 3]: if not initial yet, TP will sleep 100ms
 * [HINT 4]: Basically, INT is active, then report "TOUCH DOWN" event; INT is not active, then report "TOUCH UP" event
 * [HINT 5]: INT has 2 action type:
    1). pull low until finger is not touch(contact) on TP
    2). pull low as a pulse which width about 2~20 micro-second until finger is not touch(contact) on TP
*/
static void _tpUpdate(struct ts_sample *tpSmp)
{
    if(g_IsTpInitialized!=true)
    {
        printf("WARNING:: TP has not initial, yet~~~\n");
        usleep(TP_IDLE_TIME_NO_INITIAL);
        return;
    }
    
    #ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
    if(gTpInfo.tpStatus != TOUCH_NO_CONTACT)
    	printf("	UpdateSmp:INT=%x, s=%x pxy=(%d,%d,%d)\n",gTpInfo.tpCurrINT, gTpInfo.tpStatus, tpSmp->pressure,tpSmp->x,tpSmp->y);
    #endif
    
    if(gTpInfo.tpCurrINT)
    {
        _tpIntActiveRule_vendor(tpSmp);
    }
    else
    {
    	_tpIntNotActiveRule_vendor(tpSmp);
    }
    
    usleep(TP_IDLE_TIME);	//sleep 2ms
}

/**
 * the thread for handling real-time touch event(<2ms)
 * 
 * [HINT]: use g_sample for comunication with function "_tpProbeSample()"
 */ 
static void* _tpProbeHandler(void* arg)
{
    struct ts_sample *tpSmp = (struct ts_sample *)&gTmpSmp[0];
    
    while(1)
    {
        if(_tpNeedToGetSample())	_tpGetSample(tpSmp, 1);
        
        _tpUpdate(tpSmp);           
    }
    return NULL;
}

/******************************************************************************
 * do initial flow
 ******************************************************************************/
/**
 * TP initial flow
 *
 * @return: 0 for success, -1 for failure
 *
  */ 
static int _tpDoInitial(void)
{
	int ret;
    int res;
    pthread_t task;
    pthread_attr_t attr;
    
    //initialize the TP SPEC first.
    _tpInitSpec_vendor();
    	
    if(gTpSpec.tpHasPowerOnSeq)     _tpDoPowerOnSeq_vendor();
    
    printf("try to init GPIO pin\n");  
    _tpInitTouchGpioPin();
    
    if(gTpSpec.tpNeedProgB4Init)
    {
    	if(_tpDoInitProgram_vendor()<0)
    	{
    		printf("[TOUCH]warning:: touch panel do initial progeram chip fail\n");
    		return -1;
    	}
	}
    
    //create touch mutex	
    res = pthread_mutex_init(&gTpMutex, NULL);
   	if(res)
   	{
   	    printf("[Touch Panel]%s() L#%ld: ERROR, init touch mutex fail! res=%ld\r\n", __FUNCTION__, __LINE__, res);
   	    return -1;
   	}
    
    printf("Create touch pthread~~\n");	
	//create thread	
    pthread_attr_init(&attr);
    res = pthread_create(&task, &attr, _tpProbeHandler, NULL);        
    if(res)
    {
    	printf( "[TouchPanel]%s() L#%ld: ERROR, create _tpProbeHandler() thread fail! res=%ld\n", res );
    	return -1;
    }

#ifdef TP_USE_XQUEUE
	tpQueue = xQueueCreate(TP_QUEUE_LEN, (unsigned portBASE_TYPE) sizeof(struct ts_sample)*gTpSpec.tpSampleNum);
    if(tpQueue == NULL)
    {
    	printf( "[TouchPanel]%s() L#%ld: ERROR, create xQueueCreate() fail! res=%ld\n", tpQueue );
    	return -1;
    }
#endif

    g_TouchDownIntr = false;
	g_IsTpInitialized = true;
	printf("TP initial has finished\n");  
	
	return 0;
}

/**
 * Send touch sample(samp->pressure, samp->x, samp->y, and samp->tv)
 *
 * @param samp: the touch samples
 * @param nr: the sample count that upper layer wanna get.
 * @return: the really total touch sample count
 *
 * [HINT 1]:this function will be called by it7260_read(). 
 * [HINT 2]:get the samples from the global variable(g_sample).
 */ 
static int _tpProbeSample(struct ts_sample *samp, int nr)
{
	struct ts_sample *s=samp;
#ifdef TP_USE_XQUEUE
	struct ts_sample *tSmpBase = (struct ts_sample *)malloc(sizeof(struct ts_sample)*gTpSpec.tpSampleNum);
#endif

	pthread_mutex_lock(&gTpMutex);	
	
#ifdef TP_MULTI_FINGER_ENABLE
	if(samp->next==NULL)	printf("	$$$### samp->next = NULL ###$$$\n");
#endif
	
	if(gTpInfo.tpIntr4Probe)
	{
        struct ts_sample *tSmp = (struct ts_sample *)&g_sample[0];

#ifdef TP_USE_XQUEUE
		if(tSmpBase==NULL)	printf("memory locate error\n");
        else    tSmp = (struct ts_sample *)tSmpBase;
		
		if(SendQueCnt>0)	SendQueCnt--;
		
		//printf("DeQ1:%d\n",SendQueCnt);
        if ( xQueueReceive(tpQueue, tSmp, 0)!=pdTRUE )
        {        	
        	tSmp = (struct ts_sample *)&g_sample[0];
        	//printf("xQue-NoQue:tSmp=%x, QC=%d\n",tSmp,SendQueCnt);
        }
#endif
        _tpSendQue2Sample(s, tSmp);
	}
	else
	{
#ifdef TP_USE_XQUEUE
        struct ts_sample *ts = (struct ts_sample *)tSmpBase;
        
		if(SendQueCnt>0)	SendQueCnt--;
		//printf("DeQ2:%d\n",SendQueCnt);

        if ( xQueueReceive(tpQueue, ts, 0) == pdTRUE )
        {
        	_tpSendQue2Sample(s, ts);
        }
        else
       	{
       		//if(SendQueCnt)	printf("Got No Que: QC=%d\n",SendQueCnt);
			if(s->finger)
			{
				printf("	###>>> s->fgr != 0, fn=%d\n",s->finger);
				s->finger = 0;
       	    }
        }
#else
		if(s->finger)
		{
			printf("	##>>> s->fgr != 0, fn=%d\n",s->finger);
			s->finger = 0;
		}
#endif
	}

	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	if(s->pressure)	gNoEvtCnt = 3;	
	if( gNoEvtCnt )	
	{
		printf("gfQ, INT=%x, fn=%d, id=%x, pxy=(%d,%d,%d)\n", gTpInfo.tpIntr4Probe, s->finger, s->id, s->pressure, s->x, s->y);
		if( !s->pressure )	gNoEvtCnt--;
	}
	#endif

#ifdef TP_MULTI_FINGER_ENABLE
    if( (s->finger > 1) && !(s->pressure) )
    {
    	printf("Warning: need to swap sample0 with other sample, sp=%x, fn=%x, p=%d\n",s,s->finger,s->pressure);
    }
#endif	
	
#ifdef TP_USE_XQUEUE
	if(tSmpBase!=NULL)  free(tSmpBase);
#endif

	pthread_mutex_unlock(&gTpMutex);
    
    if(s->finger)   return 1;
    else            return 0;
}

#ifdef USE_RAW_API
static void _tpRegRawApiFunc(int fd)
{
	//initial raw api function
	RA_FUNC *ra = (RA_FUNC *)&gTpSpec.rawApi;
	
   	ra->raInitSpec 			= _tpInitSpec_vendor;
   	ra->raParseRawPxy 		= _tpParseRawPxy_vendor;
   	ra->raReadPointBuffer 	= _tpReadPointBuffer_vendor;
   	ra->raParseKey 			= _tpParseKey_vendor;
   	ra->raDoInitProgram 	= _tpDoInitProgram_vendor;
   	ra->raDoPowerOnSeq 		= _tpDoPowerOnSeq_vendor;

   	gTpInfo.tpDevFd = fd;
   	gTpSpec.gTpSmpBase = (struct ts_sample *)&g_sample[0];
   	
   	gTpSpec.raInfoBase = &gTpInfo;
   	gTpSpec.raMutex = &gTpMutex;
   	gTpSpec.pTouchDownIntr = &g_TouchDownIntr;
   	gTpSpec.pTpInitialized = &g_IsTpInitialized;   	

	#ifdef	CFG_TOUCH_BUTTON
	gTpSpec.pTpKeypadValue = &gTpKeypadValue;
	#endif
   	
   	_raSetSpecBase(&gTpSpec);
}
#endif

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

/**
 * Send touch sample(samp->pressure, samp->x, samp->y, and samp->tv)
 *
 * @param inf: the module information of tslibo(just need to care "inf->dev")
 * @param samp: the touch samples
 * @param nr: the sample count that upper layer wanna get.
 * @return: the total touch sample count
 *
 * [HINT 1]:this function will be called by SDL every 33 ms. 
 * [HINT 2]:Upper layer(SDL) will judge finger-down(contact on TP) if samp->pressure>0, 
 *          finger-up(no touch) if samp->pressure=0.
 * [HINT 3]:please return either 0 or 1 (don't return other number for tslib rule, even if sample number is > 1) 
 */ 
static int hy4633_read(struct tslib_module_info *inf, struct ts_sample *samp, int nr)
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
	
	_initSample(s, nr);	
	
	if(g_IsTpInitialized==false)
	{
#ifdef	USE_RAW_API
    	_tpRegRawApiFunc(tchdev);

		if(!_raDoInitial())	return -1;
		else                return 0;
#else
		printf("TP first init(INT is GPIO %d)\n",TP_INT_PIN);
		gTpInfo.tpDevFd = tchdev;	
		if(!_tpDoInitial())	return -1;
		else                return 0;
#endif
	}
	
	//to probe touch sample 
#ifdef	USE_RAW_API
	ret = _raProbeSample(samp, nr);
#else
	ret = _tpProbeSample(samp, nr);
#endif
	
	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	if(ret)	printf("    deQue-O:fn=%d (%d,%d,%d)r=%d\n", samp->finger, samp->pressure, samp->x, samp->y, ret );
	#endif
	
	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	if(samp->pressure)	gNoEvtCnt = 3;	
	if( gNoEvtCnt )	
	{
		printf("    deQue-O1:[%x] fn=%d, id=%d (%d,%d,%d)r=%d\n", samp, samp->finger, samp->id, samp->pressure, samp->x, samp->y, ret );
		if(samp->finger>1)
		{
			struct ts_sample *tsp = (struct ts_sample *)samp->next;
			printf("    deQue-Q2:[%x] fn=%d, id=%d (%d,%d,%d)r=%d\n", tsp, tsp->finger, tsp->id, tsp->pressure, tsp->x, tsp->y, ret );
		}
		if( !samp->pressure )	gNoEvtCnt--;
	}
	#endif
	
	return ret;
}

static const struct tslib_ops hy4633_ops =
{
	hy4633_read,
};

TSAPI struct tslib_module_info *hy4633_mod_init(struct tsdev *dev, const char *params)
{
	struct tslib_module_info *m;

	m = malloc(sizeof(struct tslib_module_info));
	if (m == NULL)
		return NULL;

	m->ops = &hy4633_ops;
	return m;
}

#ifndef TSLIB_STATIC_CASTOR3_MODULE
	TSLIB_MODULE_INIT(hy4633_mod_init);
#endif

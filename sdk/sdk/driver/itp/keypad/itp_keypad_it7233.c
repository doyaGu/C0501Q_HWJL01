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
#include <errno.h>
#include <pthread.h>
//#include "openrtos/FreeRTOS.h"		//must "include FreeRTOS.h" before "include queue.h"
//#include "openrtos/queue.h"			//for using xQueue function

//#define ENABLE_TK_IIC_DBG_MSG
//#define ENABLE_KEYPAD_DBG_MSG
#define ENABLE_KEYPAD_INT
/**************************************************************************
** MACRO defination                                                      **
***************************************************************************/
#ifndef	CFG_TOUCH_KEY_NUM
#define CFG_TOUCH_KEY_NUM	(16)
#endif

#define IT7233_IIC_ADDR		(0x46)

#define TK_GPIO_PIN	    CFG_GPIO_KEYPAD

#define QUEUE_LEN 256

#define I2CReadByteData	_itpI2cReadByte
#define I2CWriteByteData	_itpI2cWriteByte

#define KEY_STATUS_POSITIVE_ADDRESS     0xFF
#define KEY_STATUS_NEGATIVE_ADDRESS     0xFE
#define IT7233_FIRMWARE_READY_ADDRESS   0xFA


//interrupt TYPE
//1.touch on : INT=low, touch off : INT=high
//2.touch on : INT=edge trigger, touch off : INT=edge trigger

//touch-UP TYPE
//1.BY INT for low to high
//2.BY get the last key status is UP
//3.
/**************************************************************************
** global variable                                                      **
***************************************************************************/

const uint8_t DATA_LENGTH = 195; 

const uint8_t CONFIG_DATA[195] = {
    0x02, 0x37, 0x00, 0x0F, 0x08, 0x1E, 0x05, 0x0F, 0xA0, 0x00, 0x40, 0x00, 
    0x01, 0x00, 0x05, 0xFF, 0x00, 0x00, 0x10, 0xFF, 0x02, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x01, 0x16, 0x44, 0x20, 0x01, 
    0x01, 0x00, 0x80, 0x10, 0x80, 0x20, 0x80, 0x40, 0x80, 0x60, 0x80, 0x70, 
    0x80, 0x90, 0x80, 0xC0, 0x80, 0x01, 0x16, 0x40, 0x50, 0x02, 0x20, 0x80, 
    0x83, 0x80, 0x83, 0x80, 0x83, 0x80, 0x83, 0x80, 0x83, 0x80, 0x83, 0x80, 
    0x83, 0x80, 0x83, 0x01, 0x16, 0x40, 0x52, 0x02, 0x20, 0x00, 0x50, 0x00, 
    0x50, 0x00, 0x50, 0x00, 0x50, 0x00, 0x50, 0x00, 0x50, 0x00, 0x50, 0x00, 
    0x50, 0x01, 0x15, 0x44, 0x03, 0x01, 0x01, 0x70, 0x00, 0x00, 0x00, 0xC0, 
    0x3F, 0x03, 0x0F, 0x01, 0x00, 0x00, 0xFF, 0x1F, 0x50, 0x00, 0x01, 0x0C, 
    0x63, 0x32, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1D, 
    0x63, 0x0c, 0xFF, 0x63, 0x16, 0x00, 0x63, 0x22, 0xE0, 0x63, 0x26, 0x30, 
    0x63, 0x86, 0x00, 0x63, 0x87, 0x00, 0x63, 0x92, 0x08, 0x64, 0x21, 0x09, 
    0x64, 0x22, 0x01, 0x49, 0x54, 0x37, 0x32, 0x33, 0x33, 0x00, 0x01, 0x00, 
    0x1A, 0x00, 0xC3
};

typedef struct it7233_fw_tag { /* Used in the IBM Arctic II */
	uint8_t fwCR;
	uint8_t fwStatus;
	uint8_t fwDevName[6];
	uint8_t fwVer[4];
	uint8_t fwOtpVer[2];
	uint16_t fwSramSize;
	uint16_t fwMtpSize;
	uint16_t fwCodeSize;
	uint8_t  fwBtnNum[1];
}FW_Info;

static	const uint8_t       gTotalTouchKeyNum = (uint8_t)(CFG_TOUCH_KEY_NUM);
static	const uint8_t       gFwVerCycle = 21;

static	FW_Info   gFwInfo;
static	uint8_t	  gRegPage=0xFE;

static	pthread_mutex_t     keypad_mutex = PTHREAD_MUTEX_INITIALIZER;

static	uint32_t  gCnt = 0;
static	int       gTkDev = 0;
static	uint8_t	  gCurrKeyStatus;
static	uint8_t	  gTkInitFinished = 0;
static	uint8_t	  specific_key_index_mapping = 0;

#ifdef ENABLE_KEYPAD_INT
static bool gTchKeyInterrupt = false;
#endif

/**************************************************************************
** private function                                                      **
***************************************************************************/
static uint8_t _checkSum(uint8_t sum)
{
	uint8_t	i;
	uint8_t	cnt=0;
	
	for(i=0; i<gTotalTouchKeyNum; i++)
	{
		if( (sum>>i)&0x01 )	cnt++;
	}
	return cnt;
}

/******************************************************************************
 * the read flow for reading the IT7233's register by using i2c repead start
 ******************************************************************************/
static int _readChipReg(uint8_t regAddr, uint8_t *dBuf, uint8_t dLen)
{
    ITPI2cInfo evt;
    uint8_t	I2cCmd;
    int 			i2cret;
    
    #ifdef	ENABLE_TK_IIC_DBG_MSG
    printf("	RdIcReg,fd=%x, [%x,", gTkDev, regAddr);
    #endif
    
    I2cCmd = regAddr;	//1000 0010
    evt.slaveAddress   = IT7233_IIC_ADDR;
    evt.cmdBuffer      = &I2cCmd;
    evt.cmdBufferSize  = 1;
    evt.dataBuffer     = dBuf;
    evt.dataBufferSize = dLen;	
    
    i2cret = read(gTkDev, &evt, 1);
    
    if(i2cret<0)
    {
        #ifdef	ENABLE_TK_IIC_DBG_MSG
        printf("%x]\n", 0);
        #endif
        printf("%s[%d]I2C read fail \n", __FILE__, __LINE__);
        return -1;		
    }
    
    #ifdef	ENABLE_TK_IIC_DBG_MSG
    printf("%x]\n", *dBuf);
    #endif
	
    return 0;
}

/******************************************************************************
 * the write flow for writing the IT7233's register
 ******************************************************************************/
static int _writeChipReg(uint8_t regAddr, uint8_t *regData, uint32_t len)
{
    ITPI2cInfo  evt;
    //uint8_t     I2cCmd[4]={0};
    uint8_t     *pI2cCmd;
    int         i2cret,i;
    
    #ifdef	ENABLE_TK_IIC_DBG_MSG
    printf("	WtTkIcReg(fd=%x,[%x,%x,%x]\n", gTkDev, regAddr, *regData, len);
    #endif
    
    pI2cCmd = (uint8_t*)malloc(len+4);
    
    pI2cCmd[0] = regAddr;	//1000 0010		
    
    for(i=0; i<len; i++)	pI2cCmd[i+1] = regData[i];
    
    evt.slaveAddress   = IT7233_IIC_ADDR;
    evt.cmdBuffer      = pI2cCmd;
    evt.cmdBufferSize  = len+1;
    evt.dataBuffer     = 0;
    evt.dataBufferSize = 0;	
    i2cret = write(gTkDev, &evt, 1);

    if(pI2cCmd)  free(pI2cCmd);

    if(i2cret<0)
    {
        printf("%s[%d]I2C write fail \n", __FILE__, __LINE__);
        return -1;		
    }
    
    return 0;
}

static uint8_t _itpI2cReadByte(uint8_t reg)
{
	uint32_t  result = 0;
	uint8_t	buf[2];
	uint32_t	reg32, cnt=0;
	int 			i2cret;
	
	i2cret = _readChipReg(reg, &buf[0], 1);	
	
	return buf[0];
}

static void _itpI2cWriteByte(uint8_t addr, uint8_t byte)
{
    int       i2cret;
    uint8_t   tmpB = byte;

    i2cret = _writeChipReg(addr, &tmpB, 1);
}

static void _initTkGpioPin(void)
{
    #ifdef	ENABLE_KEYPAD_DBG_MSG
    printf("_initTkGpioPin::in\n");
    #endif
    
    ithGpioSetMode(TK_GPIO_PIN, ITH_GPIO_MODE0);
    ithGpioSetIn(TK_GPIO_PIN);
    ithGpioCtrlEnable(TK_GPIO_PIN, ITH_GPIO_PULL_ENABLE);
    ithGpioCtrlEnable(TK_GPIO_PIN, ITH_GPIO_PULL_UP);
    ithGpioEnable(TK_GPIO_PIN);	  
    
    #ifdef	ENABLE_KEYPAD_DBG_MSG
    printf("itpKeypadIntr = %d\n",TK_GPIO_PIN);   
    #endif
}

#ifdef	ENABLE_KEYPAD_INT
void _tk_isr(void* data)
{	
    uint32_t regValue;
	
    #ifdef	ENABLE_KEYPAD_DBG_MSG
    ithPrintf("$in\n");
    #endif
	
    //must check this for correct INTr
    regValue = ithGpioGet(TK_GPIO_PIN);
    
    if(gTkInitFinished)
    {
        #ifndef	ENABLE_KEYPAD_DBG_MSG
        pthread_mutex_lock(&keypad_mutex); 
        #endif
        
        if (regValue)    gTchKeyInterrupt = false;
        else             gTchKeyInterrupt = true;
        
        #ifndef	ENABLE_KEYPAD_DBG_MSG
        pthread_mutex_unlock(&keypad_mutex); 
        #endif
    }
    ithGpioClearIntr(TK_GPIO_PIN);
	
    #ifdef	ENABLE_KEYPAD_DBG_MSG
    ithPrintf("$out(%x,%x)\n",gTchKeyInterrupt,regValue);
    #endif
}

static void _initTchKeyIntr(void)
{
    #ifdef	ENABLE_KEYPAD_DBG_MSG
    printf("TK init initial::in\n");	
    #endif
    
    ithEnterCritical();
    
    ithGpioClearIntr(TK_GPIO_PIN);
    ithGpioRegisterIntrHandler(TK_GPIO_PIN, _tk_isr, NULL);
    ithGpioCtrlDisable(TK_GPIO_PIN, ITH_GPIO_INTR_LEVELTRIGGER);    //edge trigger
    ithGpioCtrlEnable(TK_GPIO_PIN, ITH_GPIO_INTR_BOTHEDGE);			//must both edge(i don't know why?)
    ithIntrEnableIrq(ITH_INTR_GPIO);
    ithGpioEnableIntr(TK_GPIO_PIN);
        
    ithExitCritical();
    
    #ifdef	ENABLE_KEYPAD_DBG_MSG
    printf("TK init initial::out\n");	
    #endif
}
#endif

static bool _getTkIntr(void)
{
    #ifdef	ENABLE_KEYPAD_INT
    bool tmpINT;
    
    pthread_mutex_lock(&keypad_mutex); 
    tmpINT = gTchKeyInterrupt;
    pthread_mutex_unlock(&keypad_mutex); 
    
    return tmpINT;    
    
    #else
    
    if(!ithGpioGet(TK_GPIO_PIN))	return true;
    else                          return false;
    
    #endif
}

static uint8_t _getTouchKeyStatus(void)
{
    uint8_t		page = 0x60;
    uint8_t	  KeyValue;
    uint32_t result = 0;
    uint8_t		buf[2];
    uint32_t	regData1,regData2;
    uint32_t	reg32, cnt=0;
    
    uint8_t  bKeyStatus = 0;
    uint8_t  b2KeyStatus = 0;
    uint8_t  gbButtonFinal = 0;
	
    b2KeyStatus = I2CReadByteData(KEY_STATUS_NEGATIVE_ADDRESS);  //Read key status 1
    bKeyStatus  = I2CReadByteData(KEY_STATUS_POSITIVE_ADDRESS);  //Read key status 2
    
    I2CWriteByteData(0xF3, 0x00);

    b2KeyStatus = ~b2KeyStatus;

    if(bKeyStatus == b2KeyStatus)
    {
        KeyValue = bKeyStatus;
    }

    return KeyValue;
}

static uint8_t _tanslateTouchValue(uint8_t value, uint8_t totalKeyNum)
{
    uint8_t   ChkSum = _checkSum(value);
    uint8_t   i=0;
    uint16_t	flag;
	
    #ifdef	ENABLE_KEYPAD_DBG_MSG
    printf("parseTchKey(%x,%x), chkSum=%x\n",value,totalKeyNum,ChkSum);
    #endif
    
    if(specific_key_index_mapping)
    {
        switch(totalKeyNum)
        {
            case 5:
                switch(value)
                {
                    case 0x01:	return 0;
                    case 0x02:	return 0xFF;
                    case 0x04:	return 1;
                    case 0x08:	return 2;
                    case 0x10:	return 0xFF;
                    case 0x20:	return 3;
                    case 0x40:	return 4;
                    case 0x80:	return 0xFF;
                    default:
                        #ifdef	ENABLE_KEYPAD_DBG_MSG
                        printf("touch keypad error: totalKeyNum incorrect2, keyNum=%d, V=%x\n",totalKeyNum, value);
                        #endif
                        return 0xFF;
                }
            case 16:
            default:
                printf("touch keypad error: totalKeyNum incorrect, keyNum=%d, V=%x\n",totalKeyNum, value);
                return 0xFF;
        }
        return 0xFF;
    }
	
    if(ChkSum==0)	return 0xFF;
	
    switch(totalKeyNum)
    {
    case 5:
      
    	if(ChkSum>1)	return -1;
    	while(1)
    	{
    		flag = 0x01<<i;
    		if(value & flag)	return i;//index_table[i];
    		if(i++>totalKeyNum)	return -1;
    	}
    case 16:
    	{
    		uint8_t	BtnX,BtnY;
    		
    	    if(ChkSum==1)	return -1;
    	    
    	    if(ChkSum>2)
    	    {
    	    	printf("[KEYPAD]warning:: multi-key, skip it(value=%x, chk=%d)\n",value, ChkSum);
    	    	return -1;
    	    }
    	    switch(value&0xC3)
    	    {
    	    	case 0x80:	BtnX = 0;	break;
    	    	case 0x40:	BtnX = 1;	break;
    	    	case 0x02:	BtnX = 2;	break;
    	    	case 0x01:  BtnX = 3;	break;
    	    	default:
    	    		printf("[KEYPAD]warning:: incorrect X, skip it(value=%x, chk=%d)\n",value, value&0xC3);
    	    		return -1;
    	    }
    
    	    switch(value&0x3C)
    	    {
    	    	case 0x04:	BtnY = 0;	break;
    	    	case 0x08:	BtnY = 1;	break;
    	    	case 0x10:	BtnY = 2;	break;
    	    	case 0x20:  BtnY = 3;	break;
    	    	default:
    	    		printf("[KEYPAD]warning:: incorrect Y, skip it(value=%x, chk=%d)\n",value, value&0x3C);
    	    		return -1;
    	    }
    	    return (BtnY*4 + BtnX);	
        }
    default:
        printf("touch keypad error: totalKeyNum incorrect, keyNum=%d\n",totalKeyNum);
        break;
    }
    return (uint8_t)0xFF;
}

static uint8_t _getTkIndex(void)
{
    uint8_t index = 0xFF;    
    uint8_t tmp;
    
    pthread_mutex_lock(&keypad_mutex);    
    tmp = gCurrKeyStatus;
    pthread_mutex_unlock(&keypad_mutex);     

    index = _tanslateTouchValue(tmp, gTotalTouchKeyNum);
    
    #ifdef	ENABLE_KEYPAD_DBG_MSG
    printf("	cKey=%x,%x,%x\n",gTotalTouchKeyNum,gCurrKeyStatus,index);
    #endif
    
    if(index==0xFF)	return 0xFF;
    
    return index;
}

static void _checkPage(uint8_t page)
{
    uint32_t result = 0;
    uint8_t  buf[2];
    uint32_t reg32, cnt=0;
    
    if(gRegPage!=page)
    {
        I2CWriteByteData(0xF0, page);
    
        #ifdef	ENABLE_KEYPAD_DBG_MODE
        printf("write page=0x%X to regAddr 0x%X\n", page, 0xF0);
        #endif
    	
        gRegPage = page;
    }
}

/*!
 * Load_Config()
 * \n
 *  Download IT7233 Config
 *  return TRUE is config download OK
 *  return FALSE is config download fail  
 * \n
 * @see
 */
static bool _loadConfig(void)
{
    uint8_t DATA_INDEX = 0;
    uint8_t IT7233_REG_ADDRESS = 0xF0 - DATA_LENGTH;
    bool bOK_FLAG = true;

    I2CWriteByteData(0xF0, 0x07);//Change Page 7
    I2CWriteByteData(0xF0, 0x07);//Change Page 7, need to write twice

    //Write CONFIG Data to SRAM page 7
    while(DATA_INDEX < DATA_LENGTH)
    {
        I2CWriteByteData(IT7233_REG_ADDRESS, CONFIG_DATA[DATA_INDEX]);
        DATA_INDEX++;
        IT7233_REG_ADDRESS++;
    }

    //Check Config right or not
    DATA_INDEX = 0;
    IT7233_REG_ADDRESS = 0xF0 - DATA_LENGTH;
    while((DATA_INDEX < DATA_LENGTH)&&(bOK_FLAG))
    {
        if((CONFIG_DATA[DATA_INDEX]) == I2CReadByteData(IT7233_REG_ADDRESS)){
            DATA_INDEX++;
            IT7233_REG_ADDRESS++;
        }else{
            bOK_FLAG = false;
        }
    }

    if(bOK_FLAG){
        I2CWriteByteData(0xF0, 0x00);//Change Page 0
        I2CWriteByteData(0x40, 0x01);//Reset CMD
        I2CWriteByteData(0x41, 0x0b);
        I2CWriteByteData(0x42, 0x00);
        I2CWriteByteData(0x43, 0x00);
        I2CWriteByteData(0xF1, 0x40);//ACR_HOST_AFTER_ACCESS
        return bOK_FLAG;
    }else{
        //Config write fail.
        return bOK_FLAG;
    }
    
}

static void _getFirmwareVersion(FW_Info* fwInfo)
{
    uint8_t		page = 0x00;
    int 			i2cret,i;
    uint8_t		buf[24];
    uint8_t		cbuf[4];
    
    printf("get FW version:fwInfo=%x, size=%d:\n",fwInfo, gFwVerCycle);

    _checkPage(page);
    
    //write 0xF1 0x80
    cbuf[0]=0x80;
    i2cret = _writeChipReg(0xF1, cbuf, 1);
    
    //read 0xFA bit 0    
    buf[0]=0;
    while( !(buf[0]&0x1))
    {
    	buf[0] = I2CReadByteData(0xFA);
    }
    usleep(1000);    
    
    //write 0x40 0x01 0x01
    cbuf[0]=0x01;	cbuf[1]=0x01;
    i2cret = _writeChipReg(0x40, cbuf, 2);
    if(i2cret)	printf("Err3\n");	
    usleep(1000);
    
    //write 0xF1 0x40    
    cbuf[0]=0x40;
    i2cret = _writeChipReg(0xF1, cbuf, 1);
    if(i2cret)	printf("Err4\n");
    usleep(1000);  
    
    //wait INTr pull-down	
    //printf("Before INTr check\n");
    //while(ithGpioGet(TK_GPIO_PIN));
    printf("after INTr check\n");
    
    //write 0xF1 0x80    
    cbuf[0]=0x80;
    i2cret = _writeChipReg(0xF1, cbuf, 1);
    if(i2cret)	printf("Err5\n");
    usleep(1000);    
    
    //read 0xFA bit 0    
    buf[0]=0;
    while( !(buf[0]&0x1))
    {
    	buf[0] = I2CReadByteData(0xFA);
    }
    usleep(1000);
    usleep(1000);    
    
    //read 0x40 16BYTES
    i2cret = _readChipReg(0x40, &buf[0], 21);	
    if(i2cret)	printf("Err7\n");
    else		memcpy((uint8_t*)fwInfo, (uint8_t*)buf, gFwVerCycle );
    
    #ifdef	ENABLE_KEYPAD_DBG_MSG
    printf("FW info buffer::\n");
    for(i=0; i<gFwVerCycle; i++)
    {
        printf("%02x,",buf[i]);
        if( (i&0xF)==0xF )	printf("\n");
    }
    printf("\n");
    #endif
    
    usleep(1000);
    
    //write 0xF3 bit6 as 0    
    cbuf[0]=0x00;
    i2cret = _writeChipReg(0xF3, cbuf, 1);
    if(i2cret)	printf("Err8\n");
    usleep(1000);    
    
    //write 0xF1 bit6 as 1    
    cbuf[0]=0x40;
    i2cret = _writeChipReg(0xF3, cbuf, 1);
    if(i2cret)	printf("Err9\n");
    usleep(1000);    
}

static void* _tkProbeHandler(void* arg)
{
    uint8_t	temp=0;
    uint8_t	value;

    while(1)
    {
        if(gTkInitFinished)	
        {
            if(_getTkIntr()==true)
            {
                temp = _getTouchKeyStatus();
            
                pthread_mutex_lock(&keypad_mutex);
                if(temp != gCurrKeyStatus)	gCurrKeyStatus = temp;
                
                #ifdef	ENABLE_KEYPAD_INT
                gTchKeyInterrupt = false;
                #endif
                                
                pthread_mutex_unlock(&keypad_mutex); 
            }
        }
        
        #ifdef	ENABLE_KEYPAD_INT
        usleep(2000);	//sleep for 2 ms
        #else
        usleep(4000);	//sleep for 4 ms
        #endif
    }
}

/**************************************************************************
** public function(keypad API)                                           **
***************************************************************************/
int itpKeypadProbe(void)
{
    uint8_t index;
    
    #ifdef	ENABLE_KEYPAD_DBG_MSG
    printf("itpKeyPadProbe, cnt=%d\n",gCnt++);
    #endif

    index = _getTkIndex();
    
    #ifdef	ENABLE_KEYPAD_DBG_MSG
   	printf("[KEYPAD] send_key, index=%d\n",index);
   	#endif
    
    if(index==0xFF)	return -1;
    
    return (int)index;
}

void itpKeypadInit(void)
{
	  int ret;
    int res;
    int cnt = 0;    
    pthread_t task;
    pthread_attr_t attr;

    
    #ifdef	ENABLE_KEYPAD_DBG_MSG
    printf("itpKeypadInit.in\n");
    #endif
    
    //[1] initial GPIO
    _initTkGpioPin();

    #ifdef	ENABLE_KEYPAD_INT
    //[2] initial touch key interrupt    
    _initTchKeyIntr();
    #endif
    
    //[3] get I2C file device number
    #ifdef	CFG_TOUCH_KEYPAD_I2C0
    gTkDev = open(":i2c0", 0);
    #elif CFG_TOUCH_KEYPAD_I2C1
    gTkDev = open(":i2c1", 0);
    #else
    #error "error condition: It must select I2C0 or I2C1 for IT7233"
    #endif
    if(gTkDev!=0xFFFFFFFF)
    {
        printf("itpKeypadInit...SUCCESS!!\n");        
    }
    else
    {
        printf("%s[%d] get file device fail\n", __FILE__, __LINE__);
        return;
    }	

    //[4] load IT7233 Config
    while(1)
    {
        if( _loadConfig() == true )	break;
        if(cnt++>100)
        {
            printf("%s[%d] load IT7233 Config fail\n", __FILE__, __LINE__);
            return;
        }        
    }
    
    //[5] get IT7233 Config
    {
        uint8_t *b = (uint8_t*)&gFwInfo.fwVer[0];
        _getFirmwareVersion(&gFwInfo);
        printf("The %d-key FW version of IT7236:(%02X.%02X.%02X.%02X)\n", gTotalTouchKeyNum, b[0], b[1], b[2], b[3] );
    }
    
    //[6] Create a pthread for probing keypad event 
    #ifdef	ENABLE_KEYPAD_DBG_MSG
    printf("Create touch keypad pthread~~\n");    
    #endif
    pthread_attr_init(&attr);
    res = pthread_create(&task, &attr, _tkProbeHandler, NULL);
    if(res)
    {
        printf("%s[%d] Create thread fail, res=%x \n", __FILE__, __LINE__, res);
        return;
    }

    //[7] touch keypad initialization was finished
    gTkInitFinished = 1;    
	  
	  #ifdef	ENABLE_KEYPAD_DBG_MSG
	  printf("itpKeypadInit.out\n");
	  #endif
}

int itpKeypadGetMaxLevel(void)
{
    return gTotalTouchKeyNum;
}

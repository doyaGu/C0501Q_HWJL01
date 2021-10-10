#include <string.h>
#include "mmio.h"
#include "ith_risc.h"

#define TEST_CHECK_START_CODE 1
#define HEART_BEAT_TURN_ON 1

#define DATA_ONE_LOW_BOUND             800
#define DATA_ONE_UP_BOUND              1200
#define START_CODE_LOW_BOUND           1800
#define START_CODE_UP_BOUND            2200
#define START_CODE_HIGH_PLUSE          2000
#define START_CODE_LOW_PLUSE           1000
#define DATA_ONE_HIGH_PLUSE            1000
#define DATA_ONE_LOW_PLUSE             500
#define DATA_ZERO_HIGH_PLUSE           500
#define DATA_ZERO_LOW_PLUSE            500
#define END_CODE_HIGH_PLUSE            1000
#define END_CODE_LOW_PLUSE             0
#define RECEIVE_DATA_TIMEOUT           250000

#define ticker2us(time) (time/(tickToMicroConstant));

#define _COUNT_OF_MSG( w, r, max)   ((r <= w) ? (w - r) : (max - (r - w)))
#define _IS_MSGQ_FULL( w, r, max)   (_COUNT_OF_MSG(w, r, max) == max - 1)
#define _IS_MSGQ_EMPTY(w, r, max)   (_COUNT_OF_MSG(w, r, max) == 0)

#define ENDIAN_SWAP32(x) \
        (((x) & 0x000000FF) << 24) | \
        (((x) & 0x0000FF00) <<  8) | \
        (((x) & 0x00FF0000) >>  8) | \
        (((x) & 0xFF000000) >> 24)

/**
* Queue error codes.
*/
typedef enum QUEUE_ERROR_CODE_TAG
{
    QUEUE_INVALID_INPUT = -5,
    QUEUE_IS_EMPTY      = -4,
    QUEUE_IS_FULL       = -3,
    QUEUE_EXIST         = -2,
    QUEUE_NOT_EXIST     = -1,
    QUEUE_NO_ERROR      = 0
} QUEUE_ERROR_CODE;


typedef QUEUE_ERROR_CODE MSGQ_ERROR_CODE;

typedef enum HEARTBEAT_STATE_TAG
{
    STATE_START,
    STATE_LOW,
    STATE_HIGH,
    WAIT_RISING_EDGE,
    GET_HIGH_PERIOD,
} HEARTBEAT_STATE;

typedef enum SEND_DATA_STATE_TAG
{
    SEND_STRAT_CODE,
    SEND_DATA,
    SEND_END_CODE,
} SEND_DATA_STATE;

typedef enum RECEIVE_DATA_STATE_TAG
{
    WAIT_START,
    GET_START_CODE,
    GET_DATA,
    GET_END_CODE,
    ERROR_HANDLE,
} RECEIVE_DATA_STATE;

enum 
{
    RETURN_SUCCESS=0,
    RETURN_TIMEOUT=1,
    RETURN_RECEVING=2
};

///////////////////////////////
int Treadmill_output_gpio=0;
int Treadmill_input_gpio=0;
int Heartbeat_gpio=0;
int sendBitCounts=0;
int totalSendCounts=0;
int totalReceiveCounts=0;
uint32_t FirstRisingTime =0;
uint32_t SecondRisingTime=0;
static uint32_t RisingCount =0;
static uint32_t tickToMicroConstant = 360;
static uint32_t CountID= 0;

void ithGpioSetMode(unsigned int pin, ITHGpioMode mode)
{
    uint32_t value, mask;
    
    //ithEnterCritical();
    
	// for UART1 output
	if (mode == ITH_GPIO_MODE_TX)
	{
		if (pin < 32)
		{
		    ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_URTXSEL1_REG, pin);
		}
		else
		{
		    ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_URTXSEL2_REG, pin - 32);
		}

		mode = 0;
	}
	else if (mode == ITH_GPIO_MODE_RX)
	{
		if (pin < 32)
		{
		    ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_URRXSEL1_REG, pin);
		}
		else
		{
		    ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_URRXSEL2_REG, pin - 32);
		}
		mode = 0;
	}

	if (pin < 16)
	{
		value = mode << (pin * 2);
		mask = 0x3 << (pin * 2);
		ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO1_MODE_REG, value, mask);
	}
	else if (pin < 32)
	{
		value = mode << ((pin - 16) * 2);
		mask = 0x3 << ((pin - 16) * 2);
		ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO2_MODE_REG, value, mask);
	}
	else if (pin < 48)
	{
		value = mode << ((pin - 32) * 2);
		mask = 0x3 << ((pin - 32) * 2);
		ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO3_MODE_REG, value, mask);
	}
	else
	{
		value = mode << ((pin - 48) * 2);
		mask = 0x3 << ((pin - 48) * 2);
		ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO4_MODE_REG, value, mask);
	}

	//ithExitCritical();
}


void TreadmillGpioInit(int controlSendGpio , int controlRecvGpio , int heartBeatGpio)
{
 #define INVERSE_OUT
      ithGpioSetMode(controlSendGpio, ITH_GPIO_MODE0);
      ithGpioSetMode(controlRecvGpio, ITH_GPIO_MODE0);
      ithGpioSetMode(heartBeatGpio, ITH_GPIO_MODE0);

	// set controlSendGpio to output mode
	ithGpioSetOut(controlSendGpio);

#ifndef INVERSE_OUT  //default
	ithGpioSet(controlSendGpio);
#else
       ithGpioClear(controlSendGpio);  //default
#endif

      // set controlRecvGpio to input mode
      ithGpioSetIn(controlRecvGpio);
      ithGpioEnable(controlRecvGpio);

      // set heartBeatGpio to input mode
      ithGpioSetIn(heartBeatGpio);
      ithGpioEnable(heartBeatGpio);

      Treadmill_output_gpio = controlSendGpio;
      Treadmill_input_gpio = controlRecvGpio;
      Heartbeat_gpio = heartBeatGpio;
      tickToMicroConstant = PalGetSysClock() / (1000 * 1000);


}

int DoHeartBeat(int timeoutUs, HEARTBEAT_INFO* pHeartbeatinfo)
{    
   static HEARTBEAT_STATE gHeartBeatState = STATE_START;
   static int preHeartLevel = 0;
   static int HighStartTime = 0;
   static int PlusePeriod = 0;
   static int HighEndTime = 0;
   static int curHeartLevel = 0;
   static int startTimer = 0;
   static int bStart = 1;
   int doBreak=0;
   int CyclePeriod = 0;
   int getTimer =0;
   int period =0;
   int upTime =0;
   int result =RETURN_RECEVING;
   
   switch(gHeartBeatState)
   {
    case STATE_START:
            if (bStart)
            {
                reset_wiegand_timer(2);
                startTimer = get_wiegand_timer(2);
                bStart = 0;
            }
             curHeartLevel = ithGpioGet(Heartbeat_gpio);
            if(curHeartLevel)
            {               
            }
            else
            {
                gHeartBeatState = STATE_LOW;
            }
    break;

    case STATE_LOW:
             curHeartLevel = ithGpioGet(Heartbeat_gpio);
            if(curHeartLevel)
            {
                
                if(RisingCount >= 1)
                {
                    SecondRisingTime = get_wiegand_timer(1);
                    period = ticker2us((SecondRisingTime - FirstRisingTime));  //return this
                    upTime = PlusePeriod;
                    CountID ++;

                    pHeartbeatinfo->Buffersize = ENDIAN_SWAP32(MAX_BUFFER_COUNT);
                    pHeartbeatinfo->data[pHeartbeatinfo->WriteIndexForRISC].RisingPeriod = ENDIAN_SWAP32(period);
                    pHeartbeatinfo->data[pHeartbeatinfo->WriteIndexForRISC].upTime= ENDIAN_SWAP32(upTime);
                    pHeartbeatinfo->data[pHeartbeatinfo->WriteIndexForRISC].CountID = ENDIAN_SWAP32(CountID);

                    reset_wiegand_timer(1);
                    FirstRisingTime = get_wiegand_timer(1);
                
                    if(period  >timeoutUs)
                    {
                         result = RETURN_TIMEOUT;
                    }
                    else
                    {
                         result = RETURN_SUCCESS;
                    }
                    pHeartbeatinfo->data[pHeartbeatinfo->WriteIndexForRISC].HeartbeatTimeoutResult =ENDIAN_SWAP32(result);
                    
                    if (MAX_BUFFER_COUNT <= ++pHeartbeatinfo->WriteIndexForRISC)
                    {
                        pHeartbeatinfo->WriteIndexForRISC = 0;
                    }
                    pHeartbeatinfo->WriteIndex =ENDIAN_SWAP32(pHeartbeatinfo->WriteIndexForRISC);

                    reset_wiegand_timer(2);
                    startTimer = get_wiegand_timer(2);
                }
                else
                {
                    FirstRisingTime = get_wiegand_timer(1);
                }
                RisingCount++; 
                HighStartTime = get_wiegand_timer(1);
                gHeartBeatState = STATE_HIGH;
            }else
            {
            }
    break;

    case STATE_HIGH:
             curHeartLevel = ithGpioGet(Heartbeat_gpio);
            if(curHeartLevel)
            {
            }
            else
            {
                HighEndTime = get_wiegand_timer(1);
                PlusePeriod = ticker2us((HighEndTime - HighStartTime)); //return this.
                gHeartBeatState = STATE_LOW;
            }
    break;
   }

    getTimer = ticker2us((get_wiegand_timer(2) - startTimer));
    if ( getTimer > timeoutUs)
    {
            CountID ++;
            pHeartbeatinfo->Buffersize = ENDIAN_SWAP32(MAX_BUFFER_COUNT);
            pHeartbeatinfo->data[pHeartbeatinfo->WriteIndexForRISC].RisingPeriod = 0;
            pHeartbeatinfo->data[pHeartbeatinfo->WriteIndexForRISC].upTime= 0;
            pHeartbeatinfo->data[pHeartbeatinfo->WriteIndexForRISC].CountID =  ENDIAN_SWAP32(CountID);

            result = RETURN_TIMEOUT;
            pHeartbeatinfo->data[pHeartbeatinfo->WriteIndexForRISC].HeartbeatTimeoutResult =ENDIAN_SWAP32(result);
            
            if (MAX_BUFFER_COUNT <= ++pHeartbeatinfo->WriteIndexForRISC)
            {
                pHeartbeatinfo->WriteIndexForRISC = 0;
            }
            pHeartbeatinfo->WriteIndex =ENDIAN_SWAP32(pHeartbeatinfo->WriteIndexForRISC);
            bStart = 1;
            reset_wiegand_timer(2);
            startTimer = get_wiegand_timer(2);
    }
   return result;  //end of for return value 

}


void OutputSymbol(int highPeriod, int lowPeriod,int timeoutUs, HEARTBEAT_INFO* pHeartbeatinfo)
{

#define INVERSE_OUT
unsigned long start_time = 0;


#ifndef INVERSE_OUT  //default
	 ithGpioSet(Treadmill_output_gpio);
#else
       ithGpioClear(Treadmill_output_gpio);  //default
#endif
#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
    if (highPeriod > 0)
    {
#ifndef INVERSE_OUT
        ithGpioClear(Treadmill_output_gpio);
#else
    	 ithGpioSet(Treadmill_output_gpio);
#endif
#ifdef HEART_BEAT_TURN_ON
          DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
        reset_wiegand_timer(0);
        start_time = get_wiegand_timer(0);
        
    	while (1)
        {
             if (((get_wiegand_timer(0) - start_time)/(PalGetSysClock()/960000)) >= highPeriod)
             {                
#ifndef INVERSE_OUT 
                 ithGpioSet(Treadmill_output_gpio);
#else
                 ithGpioClear(Treadmill_output_gpio);
#endif
                 reset_wiegand_timer(0);
                 start_time = get_wiegand_timer(0);
                 break;
             }
#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
        }    	
    }
    
    if (lowPeriod > 0)
    {    	
    	while (1)
        {
             if (((get_wiegand_timer(0) - start_time)/(PalGetSysClock()/960000)) >= lowPeriod)
             {                
#ifndef INVERSE_OUT 
                 ithGpioClear(Treadmill_output_gpio);
#else
                 ithGpioSet(Treadmill_output_gpio);
#endif
                 reset_wiegand_timer(0);
                 break;
             }
#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
        }
    }               
}



int TMRSendData(uint8_t *inBuffer, int bufferSize ,int timeoutUs, HEARTBEAT_INFO* pHeartbeatinfo )
{
   SEND_DATA_STATE gTMSState = SEND_STRAT_CODE;      
   unsigned long test_start_time =0;
   int  sendBitCounts = 0;
   int  totalSendCounts = bufferSize * 8;
   int  index = 0;
   int i=0;
   int doBreak=0;
   int  SendBit  =0;


#ifdef HEART_BEAT_TURN_ON
            DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif

do
{
    switch (gTMSState)
    {
        case SEND_STRAT_CODE:
            OutputSymbol(START_CODE_HIGH_PLUSE , START_CODE_LOW_PLUSE,timeoutUs,pHeartbeatinfo);
            
            gTMSState = SEND_DATA;
            break;
                       
        case SEND_DATA:        
            for (i = 0; i < totalSendCounts; i++)
            {
             SendBit  = ((inBuffer[i /8] >> (i & 0x7) ) & 0x1) ;
            	if (SendBit)  
            		OutputSymbol(DATA_ONE_HIGH_PLUSE , DATA_ONE_LOW_PLUSE,timeoutUs,pHeartbeatinfo);
                else
                	OutputSymbol(DATA_ZERO_HIGH_PLUSE , DATA_ZERO_LOW_PLUSE,timeoutUs,pHeartbeatinfo);
            }
            
            gTMSState = SEND_END_CODE;
            break;

        case SEND_END_CODE:
            OutputSymbol(END_CODE_HIGH_PLUSE , END_CODE_LOW_PLUSE,timeoutUs,pHeartbeatinfo);
            
            gTMSState = SEND_STRAT_CODE;
            doBreak = true;
            {
                for(i=0;i<(10000*16);i++)
                {
                    asm("");
                }
            }
#ifdef HEART_BEAT_TURN_ON
            DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
            break;
    }      
  }while(!doBreak);  

   return ;
}



int ReceiveTMRData( uint8_t *OutputBuffer,int bufferSize, int timeoutMs ,uint8_t *recvState,int timeoutUs, HEARTBEAT_INFO* pHeartbeatinfo )
{
    
    RECEIVE_DATA_STATE gTreadMillState = WAIT_START;
    int preTMRLevel=0;
    int preTMRtTime=0;
    int timeoutTMRtime =0;
    int LowPeriod=0;
    int HighPeriod=0;
    int NoChangePeriod= 0;
    int Period=0;
    int curTMRTime;
    int curTMRLevel=0;
    int doBreak=0;
    int totalBitCounts =0;
    unsigned short ByteIdx =0;
    int bitIdx =0;
    short i=0;
    short j=0;

    #define INVERSE_OUT 
    #define INIT_LEVEL 0

    memset(OutputBuffer, 0x0, bufferSize);
#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
    preTMRLevel = INIT_LEVEL;
    reset_wiegand_timer(0);

    do
    {
        curTMRLevel = ithGpioGet(Treadmill_input_gpio);

#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
        
        #ifdef INVERSE_OUT  
        if (curTMRLevel)
        {
            curTMRLevel = 0;
        }
        else
        {
            curTMRLevel = 1;
        }
        #endif

#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
        switch (gTreadMillState)
        {
        case WAIT_START:
        
            if (curTMRLevel != preTMRLevel)
            {
                preTMRtTime = get_wiegand_timer(0);
#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif

#ifdef TEST_CHECK_START_CODE
                if (curTMRLevel)
                {
                    gTreadMillState = GET_START_CODE;
                } else {                    
                    reset_wiegand_timer(0);
                    gTreadMillState = WAIT_START;
                }
#else                
                if (!curTMRLevel)
                {
                    gTreadMillState = ERROR_HANDLE;
                } else {                    
                    gTreadMillState = GET_START_CODE;
                }
#endif
                preTMRLevel = curTMRLevel;
            } 
            else
            {
                curTMRTime = get_wiegand_timer(0);
                NoChangePeriod = ticker2us(curTMRTime);
                if (NoChangePeriod > RECEIVE_DATA_TIMEOUT)
                {
                	//receive time out
                	gTreadMillState = ERROR_HANDLE;
                }         		
            }
            break;
                       
        case GET_START_CODE:

            if (curTMRLevel != preTMRLevel)
            {

#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
                curTMRTime = get_wiegand_timer(0);
                if (curTMRLevel)
                {

#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
                    LowPeriod = ticker2us((curTMRTime - preTMRtTime));

                    // TODO : check High/Low Period, doing error handle
                    if(HighPeriod >=1800 && HighPeriod <=2200)
                    {

#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
                        gTreadMillState = GET_DATA; 
                        preTMRLevel = curTMRLevel;
                        preTMRtTime = curTMRTime;
                                  
                    }else
                    {

#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
                        reset_wiegand_timer(0);
                        gTreadMillState = WAIT_START;
                        preTMRLevel = INIT_LEVEL;
                    }
                    
                } 
                 else
                {

#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
                    HighPeriod = ticker2us((curTMRTime - preTMRtTime));
                    preTMRLevel = curTMRLevel;
                }            
            }
            else
            {

#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
                curTMRTime = get_wiegand_timer(0);
                NoChangePeriod = ticker2us(curTMRTime);
                if (NoChangePeriod > RECEIVE_DATA_TIMEOUT)
                {
                	//receive time out
                	gTreadMillState = ERROR_HANDLE;  
                }         		
            }
            break;

        case GET_DATA:
            if (curTMRLevel != preTMRLevel)
            {

#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
                curTMRTime = get_wiegand_timer(0);
                if (curTMRLevel)
                {

#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
                    LowPeriod = ticker2us((curTMRTime - preTMRtTime));
                    if ( (HighPeriod >= DATA_ONE_LOW_BOUND) && (HighPeriod <= DATA_ONE_UP_BOUND)) //0.8ms <= bit1 <= 1.2ms
                    {

#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
                        OutputBuffer[ByteIdx] |= (0x1 << bitIdx);
                        bitIdx =  (++bitIdx) & 0x7;

                        if(bitIdx == 0)
                        {
                            ByteIdx ++;
                        }
                       
                    }
                    else
                    {

#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
                        OutputBuffer[ByteIdx] |= (0x0 << bitIdx);
                        bitIdx =  (++bitIdx) & 0x7;

                        if(bitIdx == 0)
                        {
                            ByteIdx ++;
                        }
                    }
                 } 
                else
                {
                    HighPeriod = ticker2us((curTMRTime - preTMRtTime));
                }
#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
                preTMRLevel = curTMRLevel;
                preTMRtTime = curTMRTime;
            }
            else
             {

#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
                timeoutTMRtime =get_wiegand_timer(0);
                NoChangePeriod = ticker2us((timeoutTMRtime - preTMRtTime));

                if(NoChangePeriod >= 700  &&  !curTMRLevel )
                {

#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
                    if( (HighPeriod >= DATA_ONE_LOW_BOUND) && (HighPeriod <= DATA_ONE_UP_BOUND) )
                    {
                        gTreadMillState = GET_END_CODE;
                       
                    }else
                    {

#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
                        gTreadMillState = WAIT_START;
                        // TODO : check error handle
                    }
                }
            }
            break;
            
        case GET_END_CODE:

#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
             if (!curTMRLevel)
            {
                // TODO : check High/Low Period, doing error handle                    
                gTreadMillState = WAIT_START;
            } else {
                // TODO : error handle                    
                gTreadMillState = ERROR_HANDLE;
            }
            doBreak = true;    


            break;
        case ERROR_HANDLE:

#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif
            //write error status to API
            doBreak = true;
            gTreadMillState = WAIT_START;
            *recvState = RETURN_TIMEOUT;
            break;
        }
        
        //preTMRLevel = curTMRLevel;
    } while (!doBreak);      

#ifdef HEART_BEAT_TURN_ON
           DoHeartBeat(timeoutUs,pHeartbeatinfo);
#endif

*recvState = RETURN_SUCCESS;
return ByteIdx;
}



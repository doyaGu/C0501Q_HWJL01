/*
  This file is part of XXX
  
  Information here.
*/
/**
 * @file iic_sw.c
 * @brief  
 * @author
 * @author
 */

/********************************************
 * Include Header 
 ********************************************/
#include <stdio.h>
#include <pthread.h>
#include "iic_sw.h"

/********************************************
 * MACRO defination
 ********************************************/
#define IIC_SW_VERSION_2

#define SCLK_High() { _GpioSetHigh(IicDevice.gpioCLK); _IIC_Delay(IicDevice.delayTime); }
#define SCLK_Low()  { _GpioSetLow(IicDevice.gpioCLK);  _IIC_Delay(IicDevice.delayTime); }
#define SDA_High()  { _GpioSetHigh(IicDevice.gpioDATA);_IIC_Delay(IicDevice.delayTime); }
#define SDA_Low()   { _GpioSetLow(IicDevice.gpioDATA); _IIC_Delay(IicDevice.delayTime); }
 
 /********************************************
 * Definition
 ********************************************/
typedef enum _PIN_STATE
{
	PIN_LOW,
	PIN_HIGH
}PIN_STATE;

typedef struct tagIIC_DEVICE
{
	int32_t         refCount;
	pthread_mutex_t iicMutex;
	uint8_t         gpioCLK;
	uint8_t         gpioDATA;
	uint32_t        delayTime;
}IIC_DEVICE;

/********************************************
 * Global variable
 ********************************************/
static IIC_DEVICE IicDevice = {0, PTHREAD_MUTEX_INITIALIZER};

/********************************************
 * Prinvate function declaration
 ********************************************/

/********************************************
 * Prinvate function definition
 ********************************************/
/**
 * Function Brief Here
 *
 * @param XXX XXX
 * @return XXX
 */
static void
_GpioSetHigh(
	uint32_t pin)
{
	ithGpioSetIn(pin);
}

static void
_GpioSetLow(
	uint32_t pin)
{
	ithGpioClear(pin);
	ithGpioSetOut(pin);
}

static PIN_STATE 
SCLK_State()
{
	uint32_t value = 0;
	
	value = ithGpioGet(IicDevice.gpioCLK);
	if (value)
	{
		return PIN_HIGH;
	}
	return PIN_LOW;
}

static PIN_STATE 
SDA_State()
{
	uint32_t value = 0;
	
	value = ithGpioGet(IicDevice.gpioDATA);
	if (value)
	{
		return PIN_HIGH;
	}
	return PIN_LOW;
}

static void
WaitSCLKState(
	PIN_STATE state)
{
	if ( state == PIN_HIGH )
	{
		while ( SCLK_State() == PIN_LOW );
	}
	else
	{
		while ( SCLK_State() == PIN_HIGH );
	}
}

static void
WaitSDAState(
	PIN_STATE state)
{
	if ( state == PIN_HIGH )
	{
		while ( SDA_State() == PIN_LOW );
	}
	else
	{
		while ( SDA_State() == PIN_HIGH );
	}
}


static void
_IIC_Delay(
    uint32_t loop)
{
#if 1
	ithDelay(loop);
#else
	uint32_t _loop = loop;
	
    while(_loop)
	{
        _loop--;
	}
#endif
}

#ifdef IIC_SW_VERSION_2
static IIC_RESULT
_IIC_WaitAck()
{
    uint32_t ack = 0xFFFFFFFF;

    _IIC_Delay(IicDevice.delayTime);	// Delay 7

	// SCLK pull high 
	_GpioSetHigh(IicDevice.gpioCLK);
	_IIC_Delay(IicDevice.delayTime);	// Delay 7

	// Get SDA data
	ack = ithGpioGet(IicDevice.gpioDATA);

	// SCLK pull low
	_GpioSetLow(IicDevice.gpioCLK);
	_IIC_Delay(IicDevice.delayTime);	// Delay 8

	if( ack == 0 )
    {
        return IIC_RESULT_OK;
    }

	return IIC_RESULT_ERROR;
}
#else
static IIC_RESULT
_IIC_WaitAck()
{
    uint32_t ack = 0;

    //_IIC_Delay(IicDevice.delayTime);

    _GpioSetHigh(IicDevice.gpioDATA);
    _IIC_Delay(IicDevice.delayTime);

	// SCLK pull high 
	_GpioSetHigh(IicDevice.gpioCLK);
	_IIC_Delay(IicDevice.delayTime);

	// Get SDA data
	ack = ithGpioGet(IicDevice.gpioDATA);

	// SCLK pull low
	_GpioSetLow(IicDevice.gpioCLK);
	_IIC_Delay(IicDevice.delayTime);

	printf("ack = 0x%08X\n", ack);

	if( (ack & (1 << IicDevice.gpioDATA)) == 0 )
    {
        return IIC_RESULT_OK;
    }

	return IIC_RESULT_ERROR;
    

    // Set to input mode
    ithGpioSetIn(IicDevice.gpioDATA);

#if 1
	_IIC_Delay(IicDevice.delayTime);
	ack = ithGpioGet(IicDevice.gpioDATA);
#else
	ithGpioSet(IicDevice.gpioDATA);
#endif
    _IIC_Delay(IicDevice.delayTime);

    ithGpioClear(IicDevice.gpioCLK);
    _IIC_Delay(IicDevice.delayTime);

    ithGpioSet(IicDevice.gpioCLK);
    _IIC_Delay(IicDevice.delayTime);

    ack = ithGpioGet(IicDevice.gpioDATA);
    _IIC_Delay(IicDevice.delayTime);

    ithGpioClear(IicDevice.gpioCLK);

    // Set to input mode
    ithGpioSetIn(IicDevice.gpioDATA);

    if( (ack & IicDevice.gpioDATA) == 0 )
    {
        return IIC_RESULT_OK;
    }

    return IIC_RESULT_ERROR;
}
#endif

static IIC_RESULT
_IIC_SendAck(
    bool     sendAck)
{
    _GpioSetLow(IicDevice.gpioCLK);
    _IIC_Delay(IicDevice.delayTime);

    if (sendAck)
    {
        _GpioSetLow(IicDevice.gpioDATA);
        _IIC_Delay(IicDevice.delayTime);
    }
    else // NACK
    {
        _GpioSetHigh(IicDevice.gpioDATA);
        _IIC_Delay(IicDevice.delayTime);
    }

    _GpioSetHigh(IicDevice.gpioCLK);
    _IIC_Delay(IicDevice.delayTime);

    _GpioSetLow(IicDevice.gpioCLK);
    _IIC_Delay(IicDevice.delayTime);

    return IIC_RESULT_OK;
}

static void
_IIC_Start()
{
#if 1
	// Set GPIO to Mode 0
	ithGpioSetMode(IicDevice.gpioCLK ,ITH_GPIO_MODE0);
	ithGpioSetMode(IicDevice.gpioDATA ,ITH_GPIO_MODE0);
	//ithWriteRegMaskA(ITH_GPIO_BASE + 0x90, 0, (0x03 << (IicDevice.gpioCLK * 2)) | (0x03 << (IicDevice.gpioDATA * 2)));
	// Set GPIO data to 0
	ithGpioSetDataOut(IicDevice.gpioCLK);
	ithGpioSetDataOut(IicDevice.gpioDATA);
	//ithWriteRegMaskA(ITH_GPIO_BASE + 0x00, 1, (1 << IicDevice.gpioCLK) | (1 << IicDevice.gpioDATA));

		
    // SCLK/DATA pull high
    _GpioSetHigh(IicDevice.gpioCLK);
    _GpioSetHigh(IicDevice.gpioDATA);
    _IIC_Delay(IicDevice.delayTime);

	ithGpioSetOut(IicDevice.gpioCLK);
	ithGpioSetOut(IicDevice.gpioDATA);

    // DATA pull low
    _GpioSetLow(IicDevice.gpioDATA);
    _IIC_Delay(IicDevice.delayTime);

    // SCLK pull low
    _GpioSetLow(IicDevice.gpioCLK);
    _IIC_Delay(IicDevice.delayTime);
#else
	// Set GPIO to Mode 0
	ithWriteRegMaskA(ITH_GPIO_BASE + 0x90, 0, (0x03 << 20) | (0x03 << 26));
	
    // Set pins output
	ithGpioSetOut(IicDevice.gpioCLK);
	ithGpioSetOut(IicDevice.gpioDATA);

    // SCLK/DATA pull high
    ithGpioSet(IicDevice.gpioCLK);
    ithGpioSet(IicDevice.gpioDATA);
    _IIC_Delay(IicDevice.delayTime);

    // DATA pull low
    ithGpioClear(IicDevice.gpioDATA);
    _IIC_Delay(IicDevice.delayTime);

    // SCLK pull low
    ithGpioClear(IicDevice.gpioCLK);
    _IIC_Delay(IicDevice.delayTime);
#endif
}

static void
_IIC_Stop()
{
#if 1
	_IIC_Delay(IicDevice.delayTime);
    _GpioSetLow(IicDevice.gpioDATA);
    _IIC_Delay(IicDevice.delayTime);
    _GpioSetHigh(IicDevice.gpioCLK);
    _IIC_Delay(IicDevice.delayTime);
    _GpioSetHigh(IicDevice.gpioDATA);
    _IIC_Delay(IicDevice.delayTime);

    WaitSCLKState(PIN_HIGH);
    WaitSDAState(PIN_HIGH);
#else
	// Set pins output
	//ithGpioSetOut(IicDevice.gpioCLK);
	//ithGpioSetOut(IicDevice.gpioDATA);
	
    // SCLK/DATA pull low
	_GpioSetLow(IicDevice.gpioCLK);
    _GpioSetLow(IicDevice.gpioDATA);
    _IIC_Delay(IicDevice.delayTime);

    // SCLK pull high
    _GpioSetHigh(IicDevice.gpioCLK);
    _IIC_Delay(IicDevice.delayTime);

    // DATA pull high
    _GpioSetHigh(IicDevice.gpioDATA);
    _IIC_Delay(IicDevice.delayTime);
#endif
}

#ifdef IIC_SW_VERSION_2
static IIC_RESULT
_IIC_SendByte(
    uint8_t data)
{
	static uint32_t ccc = 0;
	uint8_t i = 0;

	++ccc;

    for( i = 0; i < 8; i++ )
    {
        if (data & 0x80)
        {
            _GpioSetHigh(IicDevice.gpioDATA);
        }
        else
        {
            _GpioSetLow(IicDevice.gpioDATA);
        }
		_GpioSetHigh(IicDevice.gpioCLK);
        _IIC_Delay(IicDevice.delayTime);	// Delay 4

        
        _IIC_Delay(IicDevice.delayTime);	// Delay 5
        if ( i == 0 )
        {
        	WaitSCLKState(PIN_HIGH);
        }

        _GpioSetLow(IicDevice.gpioCLK);
        if ( i == 7 )
        {
        	_GpioSetHigh(IicDevice.gpioDATA);
        }
        _IIC_Delay(IicDevice.delayTime);	// Delay 6
        
        data <<= 1;
    }

    if( _IIC_WaitAck() != IIC_RESULT_OK )
    {
        printf("Write to device error(NO ACK)\n");
        return IIC_RESULT_ERROR;
    }
    return IIC_RESULT_OK;
}
#else
static IIC_RESULT
_IIC_SendByte(
    uint8_t data)
{
    uint8_t i = 0;

    printf("_IIC_SendByte(), data = 0x%X\n", data);
    
    _IIC_Delay(IicDevice.delayTime);	// Delay 4
    // send address
    for( i = 0; i < 8; i++ )
    {
        if (data & 0x80)
        {
            _GpioSetHigh(IicDevice.gpioDATA);
            printf("1");
        }
        else
        {
            _GpioSetLow(IicDevice.gpioDATA);
            printf("0");
        }
        _IIC_Delay(IicDevice.delayTime);	// Delay 5

        // CLK pull low, DATA prepare
        _GpioSetLow(IicDevice.gpioCLK);
        _IIC_Delay(IicDevice.delayTime);	// Delay 6

        // CLK pull high, DATA transfer
        _GpioSetHigh(IicDevice.gpioCLK);
        _IIC_Delay(IicDevice.delayTime);	// Delay 7

        if ( i == 7 )
        {
        	// Set SDA output
        	_GpioSetHigh(IicDevice.gpioDATA);
			ithGpioSetIn(IicDevice.gpioDATA);
        }

        // CLK pull low, DATA ready
        _GpioSetLow(IicDevice.gpioCLK);
        _IIC_Delay(IicDevice.delayTime);	// Delay 8
        
        data <<= 1;
    }

    if( _IIC_WaitAck() != IIC_RESULT_OK )
    {
        IIC_LOG_MSG("Write to device error(NO ACK)\n");
		
		ithGpioSetOut(IicDevice.gpioDATA);
        return IIC_RESULT_ERROR;
    }
	
	ithGpioSetOut(IicDevice.gpioDATA);
    return IIC_RESULT_OK;
}
#endif

static IIC_RESULT
_IIC_ReceiveByte(
	uint8_t* data,
	bool     sendAck)
{
#if 1
	int8_t   i           = 0;
    uint32_t readBack    = 0x0000;
    uint32_t receiveData = 0;

    _GpioSetHigh(IicDevice.gpioDATA);
    //ithGpioSetIn(IicDevice.gpioDATA);
    for ( i = 7; i >= 0; i-- )
	{
        // CLK pull high
        _GpioSetHigh(IicDevice.gpioCLK);
        _IIC_Delay(IicDevice.delayTime);
        if ( i == 7 )
        {
        	WaitSCLKState(PIN_HIGH);
        }
        
        readBack = ithGpioGet(IicDevice.gpioDATA);
        _IIC_Delay(IicDevice.delayTime);
        
        // CLK pull low
        _GpioSetLow(IicDevice.gpioCLK);
        _IIC_Delay(IicDevice.delayTime);

        if( readBack )
        {
            receiveData |= (1 << i);
        }
    }
    
    *data = receiveData;
	//ithGpioSetOut(IicDevice.gpioDATA);
    _IIC_SendAck(sendAck);
    return IIC_RESULT_OK;
#else
    uint8_t  i           = 0;
    uint32_t readBack    = 0x0000;
    //uint32_t receiveData = 0;
    
    for( i = 0; i < 8; i++ )
	{
		_GpioSetHigh(IicDevice.gpioDATA);
        // CLK pull high
        _GpioSetHigh(IicDevice.gpioCLK);
        _IIC_Delay(IicDevice.delayTime);
        
        readBack = ithGpioGet(IicDevice.gpioDATA);
        //_IIC_Delay(IicDevice.delayTime);
        
        // CLK pull low
        _GpioSetLow(IicDevice.gpioCLK);
        _IIC_Delay(IicDevice.delayTime);

        if( readBack & (1 << IicDevice.gpioDATA) )
        {
            (*data) |= 1;
            //receiveData |= (1 << i);
            printf("*1");
        }
        else
        {
        	printf("*0");
        }

        (*data) <<= 1;
    }
    (*data) >>= 1;
    //printf("_IIC_ReceiveByte() get 0x%X\n", receiveData);
    //*data = receiveData;

    _IIC_SendAck(sendAck);
    return IIC_RESULT_OK;
#endif
}

/********************************************
 * Public function 
 ********************************************/
/**
 * Function Brief Here
 *
 * @param XXX XXX
 * @return XXX
 */
IIC_RESULT
IIC_Initial(
	uint8_t  gpioCLK,
	uint8_t  gpioDATA,
	uint32_t delay)
{
	IIC_RESULT iicResult = IIC_RESULT_ERROR;
	
	pthread_mutex_lock(&IicDevice.iicMutex);
	if ( IicDevice.refCount == 0 )
	{
		IicDevice.gpioCLK = gpioCLK;
		IicDevice.gpioDATA = gpioDATA;
		IicDevice.delayTime = delay;

		printf("IIC SCLK pin = %u\n", IicDevice.gpioCLK);
		printf("IIC SDA pin  = %u\n", IicDevice.gpioDATA);
		printf("IIC Delay    = %u\n", IicDevice.delayTime);

#if 0
		// Set IicDevice.gpioCLK to mode 0
		if ( IicDevice.gpioCLK <= 15 )
		{
			ithWriteRegMaskA(ITH_GPIO_BASE + 0x90, 0x00, 0x03 << (IicDevice.gpioCLK * 2));
			printf("For IIC CLK, Set GPIO + 0x90 to 0x%08X\n", ithReadRegA(ITH_GPIO_BASE + 0x90));
		}
		else
		{

			ithWriteRegMaskA(ITH_GPIO_BASE + 0x94, 0x00, 0x03 << ((IicDevice.gpioCLK - 16) * 2));
			printf("For IIC CLK, Set GPIO + 0x94 to 0x%08X\n", ithReadRegA(ITH_GPIO_BASE + 0x94));
		}

		// Set IicDevice.gpioDATA to mode 0
		if ( IicDevice.gpioDATA <= 15 )
		{
			ithWriteRegMaskA(ITH_GPIO_BASE + 0x90, 0x00, 0x03 << (IicDevice.gpioDATA * 2));
			printf("For IIC DATA, Set GPIO + 0x90 to 0x%08X\n", ithReadRegA(ITH_GPIO_BASE + 0x90));
		}
		else
		{
			ithWriteRegMaskA(ITH_GPIO_BASE + 0x94, 0x00, 0x03 << ((IicDevice.gpioDATA - 16) * 2));
			printf("For IIC DATA, Set GPIO + 0x94 to 0x%08X\n", ithReadRegA(ITH_GPIO_BASE + 0x94));
		}

		// Set ouput mode
		ithWriteRegMaskA(ITH_GPIO_BASE + 0x08, 
			(1 << IicDevice.gpioCLK) | (1 << IicDevice.gpioDATA),
			(1 << IicDevice.gpioCLK) | (1 << IicDevice.gpioDATA));
		// Set ouput 0
		ithWriteRegMaskA(ITH_GPIO_BASE + 0x00, 
			0,
			(1 << IicDevice.gpioCLK) | (1 << IicDevice.gpioDATA));
		// Set input mode
		ithWriteRegMaskA(ITH_GPIO_BASE + 0x08, 
			0,
			(1 << IicDevice.gpioCLK) | (1 << IicDevice.gpioDATA));
#else
		// Set IicDevice.gpioCLK to mode 0
		ithGpioSetMode(IicDevice.gpioCLK ,ITH_GPIO_MODE0);
		// Set IicDevice.gpioDATA to mode 0
		ithGpioSetMode(IicDevice.gpioDATA,ITH_GPIO_MODE0);

		// Set ouput mode
		ithGpioSetOut(IicDevice.gpioCLK);
		ithGpioSetOut(IicDevice.gpioDATA);
		// Set ouput 0
		ithGpioSetDataOut(IicDevice.gpioCLK);
		ithGpioSetDataOut(IicDevice.gpioDATA);
		// Set input mode
		ithGpioSetIn(IicDevice.gpioCLK);
		ithGpioSetIn(IicDevice.gpioDATA);
#endif
		
		iicResult = IIC_RESULT_OK;
	}
	else
	{
		IIC_LOG_MSG("IIC already inited! reference count = %d\n", IicDevice.refCount + 1);
	}
	IicDevice.refCount++;
	pthread_mutex_unlock(&IicDevice.iicMutex);

	return iicResult;
}
	
IIC_RESULT
IIC_Terminate()
{
	IIC_RESULT iicResult = IIC_RESULT_ERROR;

	pthread_mutex_lock(&IicDevice.iicMutex);
	IicDevice.refCount--;
	if ( IicDevice.refCount == 0 )
	{
		IicDevice.gpioCLK = 0;
		IicDevice.gpioDATA = 0;
		IicDevice.delayTime = 0;

		iicResult = IIC_RESULT_OK;
	}
	else if ( IicDevice.refCount < 0 )
	{
		IIC_LOG_MSG("Abnormal reference count, reference count = %d\n", IicDevice.refCount);
	}
	else
	{
		IIC_LOG_MSG("Reference count(%d) > 0, skip termination!\n", IicDevice.refCount);
	}
	pthread_mutex_unlock(&IicDevice.iicMutex);

	return iicResult;
}

IIC_RESULT
IIC_Start()
{
	IIC_RESULT iicResult = IIC_RESULT_OK;

	pthread_mutex_lock(&IicDevice.iicMutex);
	_IIC_Start();
	pthread_mutex_unlock(&IicDevice.iicMutex);

	return iicResult;
}

IIC_RESULT
IIC_Stop()
{
	IIC_RESULT iicResult = IIC_RESULT_OK;

	pthread_mutex_lock(&IicDevice.iicMutex);
	_IIC_Stop();
	pthread_mutex_unlock(&IicDevice.iicMutex);

	return iicResult;
}

IIC_RESULT
IIC_SendByte(
    uint8_t data)
{
	IIC_RESULT iicResult = IIC_RESULT_OK;

	pthread_mutex_lock(&IicDevice.iicMutex);
	iicResult = _IIC_SendByte(data);
	pthread_mutex_unlock(&IicDevice.iicMutex);

	return iicResult;
}
	
IIC_RESULT
IIC_RecieveByte(
	uint8_t* data,
	bool     sendAck)
{
	IIC_RESULT iicResult = IIC_RESULT_OK;

	pthread_mutex_lock(&IicDevice.iicMutex);
	iicResult = _IIC_ReceiveByte(data, sendAck);
	pthread_mutex_unlock(&IicDevice.iicMutex);

	return iicResult;
}


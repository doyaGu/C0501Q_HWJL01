#ifndef __IIC_SW_EEFD60E9_4F6C_4b73_BB55_C3FE3A2A682B__
#define __IIC_SW_EEFD60E9_4F6C_4b73_BB55_C3FE3A2A682B__

/********************************************
 * Include Header 
 ********************************************/
#include "ite/itp.h"

#ifdef __cplusplus 
extern "C" { 
#endif

/********************************************
 * MACRO defination
 ********************************************/
#define IIC_LOG_MSG(format, args...)			printf("[IIC_SW]"format, ##args)
#define IIC_ERROR_MSG(format, args...)			printf("[IIC_SW][ERROR]"format, ##args)

/********************************************
 * Declaration
 ********************************************/
typedef enum tagIIC_RESULT
{
	IIC_RESULT_ERROR,
	IIC_RESULT_OK
}IIC_RESULT;

IIC_RESULT
IIC_Initial(
	uint8_t  gpioCLK,
	uint8_t  gpioDATA,
	uint32_t delay);
	
IIC_RESULT
IIC_Terminate();

IIC_RESULT
IIC_Start();

IIC_RESULT
IIC_Stop();

IIC_RESULT
IIC_SendByte(
    uint8_t data);
	
IIC_RESULT
IIC_RecieveByte(
	uint8_t* data,
	bool     sendAck);

#ifdef __cplusplus 
} 
#endif 

#endif // __IIC_SW_EEFD60E9_4F6C_4b73_BB55_C3FE3A2A682B__
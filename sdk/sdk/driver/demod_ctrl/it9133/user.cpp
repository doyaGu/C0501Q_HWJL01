
#include <unistd.h>
#include <stdint.h>
#include "ite/itp.h"
#include "mmp_iic.h"
#include "user.h"
#include "pthread.h"


/**
 * Variable of critical section
 */

// Global Mutex for critical section protection
static pthread_mutex_t gAfaMutex = 0;

Dword User_memoryCopy (
    IN  Demodulator*    demodulator,
    IN  void*           dest,
    IN  void*           src,
    IN  Dword           count
) {
    /*
     *  ToDo:  Add code here
     *
     *  //Pseudo code
     *  memcpy(dest, src, (size_t)count);
     *  return (0);
     */
    return (Error_NO_ERROR);
}

Dword User_delay (
    IN  Demodulator*    demodulator,
    IN  Dword           dwMs
) {
    /*
     *  ToDo:  Add code here
     *
     *  //Pseudo code
     *  delay(dwMs);
     *  return (0);
     */
    usleep((uint32_t)dwMs*1000);
    return (Error_NO_ERROR);
}


Dword User_createCriticalSection (
    void
) {
    /*
     *  ToDo:  Add code here
     *
     *  //Pseudo code
     *  return (0);
     */  
 	if (gAfaMutex)
 		return Error_NO_ERROR;
 
 	pthread_mutex_init(&gAfaMutex, NULL);

	if (0 == gAfaMutex)
 		return Error_NULL_PTR;
   
    return (Error_NO_ERROR);
}

Dword User_deleteCriticalSection (
    void
) {
    /*
     *  ToDo:  Add code here
     *
     *  //Pseudo code
     *  return (0);
     */
 	if (0 == gAfaMutex)
 		return Error_NO_ERROR;
 
    // pthread_mutex_destroy(&gAfaMutex);
    
    return (Error_NO_ERROR);
}

Dword User_enterCriticalSection (
    IN  Demodulator*    demodulator
) {
    /*
     *  ToDo:  Add code here
     *
     *  //Pseudo code
     *  return (0);
     */
    if (0 == gAfaMutex)
 	{
 		printf((const char*)"%s(%d), The Afa Mutex is not created yet\n",
 			   (const char*) __FILE__,
 			   (const int) __LINE__);
 		return Error_NULL_PTR;
 	}

    pthread_mutex_lock(&gAfaMutex);
    
    return (Error_NO_ERROR);
}


Dword User_leaveCriticalSection (
    IN  Demodulator*    demodulator
) {
    /*
     *  ToDo:  Add code here
     *
     *  //Pseudo code
     *  return (0);
     */
    if (0 == gAfaMutex)
 	{
 		printf((const char*)"%s(%d), The Afa Mutex is not created yet\n",
 			   (const char*) __FILE__,
 			   (const int) __LINE__);
 		return Error_NULL_PTR;
 	}
 	
    pthread_mutex_unlock(&gAfaMutex);
    
    return (Error_NO_ERROR);
}


Dword User_mpegConfig (
    IN  Demodulator*    demodulator
) {
    /*
     *  ToDo:  Add code here
     *
     */
    return (Error_NO_ERROR);
}


Dword User_busTx (
    IN  Demodulator*    demodulator,
    IN  Dword           bufferLength,
    IN  Byte*           buffer
) {
    /*
     *  ToDo:  Add code here
     *
     *  //Pseudo code
     *  short i;
     *
     *  start();
     *  write_i2c(uc2WireAddr);
     *  ack();
     *  for (i = 0; i < bufferLength; i++) {
     *      write_i2c(*(ucpBuffer + i));
     *      ack();
     *  }
     *  stop();
     *
     *  // If no error happened return 0, else return error code.
     *  return (0);
     */
    uint32_t result = 0;
    DefaultDemodulator* demod = (DefaultDemodulator*)demodulator;
    result = mmpIicSendDataEx(IIC_MASTER_MODE, 
						    (demod->demodAddr >> 1),
						    (uint8_t*) buffer, 
						    (uint16_t) bufferLength);

	//printf("result = 0x%08x\n", result);
    if (0 != result)
    {
            //if (max_debug)
            //printf(" err! 0x%08x, i2cAddr = 0x%x, %s\n", result, demod->demodAddr, __FUNCTION__);
            mmpIicGenStop();
    }
     
    return result;
}


Dword User_busRx (
    IN  Demodulator*    demodulator,
    IN  Dword           bufferLength,
    OUT Byte*           buffer
) {
    /*
     *  ToDo:  Add code here
     *
     *  //Pseudo code
     *  short i;
     *
     *  start();
     *  write_i2c(uc2WireAddr | 0x01);
     *  ack();
     *  for (i = 0; i < bufferLength - 1; i++) {
     *      read_i2c(*(ucpBuffer + i));
     *      ack();
     *  }
     *  read_i2c(*(ucpBuffer + bufferLength - 1));
     *  nack();
     *  stop();
     *
     *  // If no error happened return 0, else return error code.
     *  return (0);
     */
    uint32_t result = 0;
	uint8_t* pDummyWrBuffer = 0;
	uint16_t dummyWrSize = 0;
    DefaultDemodulator* demod = (DefaultDemodulator*)demodulator;

	result = mmpIicReceiveDataEx(IIC_MASTER_MODE, 
							(demod->demodAddr >> 1),
							pDummyWrBuffer, 
							dummyWrSize,
							(uint8_t*) buffer,
							(uint16_t) bufferLength);

    if (0 != result)
    {
        //printf(" err! 0x%08x, i2cAddr = 0x%x, %s\n", result, demod->demodAddr, __FUNCTION__);
        mmpIicGenStop();
    }

    return result;
}


Dword User_busRxData (
    IN  Demodulator*    demodulator,
    IN  Dword           bufferLength,
    OUT Byte*           buffer
) {
    return (Error_NO_ERROR);
}

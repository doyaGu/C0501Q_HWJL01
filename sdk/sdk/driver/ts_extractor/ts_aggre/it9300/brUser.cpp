#include <unistd.h>
#include <stdint.h>
#include "brUser.h"
#include "mmp_iic.h"
#include "pthread.h"

// Global Mutex for critical section protection
static pthread_mutex_t gAfaMutex = 0;//PTHREAD_MUTEX_INITIALIZER;

/**
 * Variable of critical section
 */

Dword BrUser_delay (
    IN  Bridge*         bridge,
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
    return (BR_ERR_NO_ERROR);
}


Dword BrUser_enterCriticalSection (
    IN  Bridge*         bridge
) {
    /*
        *  ToDo:  Add code here
        *
        *  //Pseudo code
        *  return (0);
        */
    //pthread_mutex_lock(&gAfaMutex);
    return (BR_ERR_NO_ERROR);
}


Dword BrUser_leaveCriticalSection (
    IN  Bridge*         bridge
) {
    /*
        *  ToDo:  Add code here
        *
        *  //Pseudo code
        *  return (0);
        */

    //pthread_mutex_unlock(&gAfaMutex);
    return (BR_ERR_NO_ERROR);
}

Dword BrUser_busTx (
    IN  Bridge*         bridge,
    IN  Byte			i2cAddr,
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

    Dword      error = BR_ERR_NO_ERROR; 
    Endeavour* pendeavour = (Endeavour *)bridge;

#if !(_MSC_VER)
    switch( pendeavour->ctrlBus )
    {
        case BUS_I2C:  
            error = mmpIicSendDataEx(IIC_MASTER_MODE, 
                                      (i2cAddr >> 1),
                                      (uint8_t*) buffer, 
                                      (uint16_t) bufferLength);
            if (0 != error)
            { 
                //printf(" err! 0x%08x, i2cAddr = 0x%x, %s\n", error, i2cAddr, __FUNCTION__);
                mmpIicGenStop();
            }
            break;
 
        case BUS_USB:
            // error = Usb2_writeControlBus(bridge, bufferLength, buffer);
            break;
    }
#endif
    return (error);
}


Dword BrUser_busRx (
    IN  Bridge*         bridge,
    IN  Byte			i2cAddr,
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

    Dword       error = BR_ERR_NO_ERROR;
    Endeavour*  pendeavour = (Endeavour *)bridge;
    uint8_t*    pDummyWrBuffer = 0;
    uint16_t    dummyWrSize = 0;

#if !(_MSC_VER)
    switch( pendeavour->ctrlBus )
    {
        case BUS_I2C:
            error = mmpIicReceiveDataEx(IIC_MASTER_MODE, 
                                        (i2cAddr >> 1),
                                        pDummyWrBuffer, dummyWrSize,
                                        (uint8_t*) buffer, (uint16_t) bufferLength);
            if (0 != error)
            {
                //printf(" err! 0x%08x, i2cAddr = 0x%x, %s\n", error, i2cAddr, __FUNCTION__);
                mmpIicGenStop();
            }
            break;
        case BUS_USB:
            // error = Usb2_readControlBus(bridge, bufferLength, buffer);
            break;
    }
#endif
    return (error);
}


Dword BrUser_busRxData (
    IN  Bridge*         bridge,
    IN  Dword           bufferLength,
    OUT Byte*           buffer
) {
    return (BR_ERR_NO_ERROR);
}

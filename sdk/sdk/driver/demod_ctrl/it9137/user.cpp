#include <unistd.h>
#include <stdint.h>
#include "mmp_iic.h"
#include "user.h"
#include "tuner/tuner.h"
#include "pthread.h"
#include "it9130.h"
#include "ite/itp.h"

#define DEMOD_SWITCH_GPIO_MASK          0xFF
#define DEMOD_SWITCH_GPIO_0_SHIFT       24

#if !(_MSC_VER) && (CFG_DEMOD_SUPPORT_COUNT > 2) && (CFG_DEMOD_SWITCH_GPIO_0 >= 0)
    #define DEMOD_SWITCH_SET(gpio_0, demod_addr)    do{ (gpio_0) = ((demod_addr >> DEMOD_SWITCH_GPIO_0_SHIFT) & DEMOD_SWITCH_GPIO_MASK);\
                                                        if( (int)(gpio_0) >= 0 )      ithGpioSet((gpio_0));\
                                                    }while(0)

    #define DEMOD_SWITCH_CLR(gpio_0, demod_addr)    do{ if((int)gpio_0 >= 0 )\
                                                            ithGpioClear(gpio_0);\
                                                    }while(0)
#else
    #define DEMOD_SWITCH_SET(gpio_0, demod_addr)
    #define DEMOD_SWITCH_CLR(gpio_0, demod_addr)
#endif
/**
 * Variable of critical section
 */

// Global Mutex for critical section protection
static pthread_mutex_t gAfaMutex = 0;//PTHREAD_MUTEX_RECURSIVE;//0;

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
        //printf("%s(%d), The Afa Mutex is not created yet\n",
        //       (const char*) __FILE__, (const int) __LINE__);
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
        //printf("%s(%d), The Afa Mutex is not created yet\n",
        //       (const char*) __FILE__, (const int) __LINE__);
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

    uint32_t    result = 0;
    uint32_t    gpio_0 = (-1);
    IT9130*     demod = (IT9130*)demodulator;

#if !(_MSC_VER)
    switch( demod->busId )
    {
        case Bus_I2C:
            // set gpio for switch demod
            DEMOD_SWITCH_SET(gpio_0, demod->demodAddr);

            result = mmpIicSendDataEx(IIC_MASTER_MODE,
                                      ((demod->demodAddr & 0xFF) >> 1),
                                      (uint8_t*) buffer,
                                      (uint16_t) bufferLength);

            //printf("result = 0x%08x\n", result);
            if (0 != result)
            {
                //printf(" err! 0x%08x, i2cAddr = 0x%x, %s\n", result, demod->demodAddr, __FUNCTION__);
                mmpIicGenStop();
            }

            DEMOD_SWITCH_CLR(gpio_0, demod->demodAddr);

            break;
    }
#endif
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
    IT9130* demod = (IT9130*)demodulator;
    uint32_t    gpio_0 = (-1);

#if !(_MSC_VER)
    switch( demod->busId )
    {
        case Bus_I2C:
            // set gpio for switch demod
            DEMOD_SWITCH_SET(gpio_0, demod->demodAddr);

            result = mmpIicReceiveDataEx(IIC_MASTER_MODE,
                                         ((demod->demodAddr & 0xFF) >> 1),
                                         pDummyWrBuffer, dummyWrSize,
                                         (uint8_t*) buffer, (uint16_t) bufferLength);

            if (0 != result)
            {
                //printf(" err! 0x%08x, i2cAddr = 0x%x, %s\n", result, demod->demodAddr, __FUNCTION__);
                mmpIicGenStop();
            }

            DEMOD_SWITCH_CLR(gpio_0, demod->demodAddr);

            break;
    }
#endif
    return result;
}

Dword User_busRx_Data (
    IN  Demodulator*    demodulator,
    IN  Dword           bufferLength,
    OUT Byte*           buffer,
    IN  void*           pCallbackContext
) {

    uint32_t result = Error_NO_ERROR;
    return result;
}



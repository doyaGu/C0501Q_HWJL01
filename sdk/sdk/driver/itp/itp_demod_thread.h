#ifndef __ITP_DEMOD_THREAD_H__
#define __ITP_DEMOD_THREAD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "pthread.h"
#include "ite/itp.h"
//=============================================================================
//				  Structure Definition
//=============================================================================
typedef struct _TSI_THREAD_PARAM_TAG
{
    uint32_t        idx; 
    uint32_t        handle;
    
}TSI_THREAD_PARAM;
//=============================================================================
//				  Macro Definition
//=============================================================================
#define _mutex_init(mutex)        do{ if(!mutex){\
                                        pthread_mutex_init(&mutex, NULL);\
                                        /*printf(" mutex_init: %s, 0x%x\n", #mutex, mutex);*/}\
                                  }while(0)
#define _mutex_deinit(mutex)      do{if(mutex){pthread_mutex_destroy(&mutex);mutex=0;}}while(0)

#define _mutex_lock(mutex)        do{ if(mutex){\
                                        /* printf("lock %s()[#%d]\n", __FUNCTION__, __LINE__);*/\
                                        pthread_mutex_lock(&mutex);}\
                                  }while(0)
#define _mutex_unlock(mutex)      do{ if(mutex){ \
                                        pthread_mutex_unlock(&mutex); \
                                        /* printf("unlock %s()[#%d]\n", __FUNCTION__, __LINE__);*/}\
                                  }while(0)

//=============================================================================
//				  Public Function Definition
//=============================================================================
void* TsiBufferThread(void* args);

int TsiStopThread(uint32_t  index);
int TsiStartCache(uint32_t  index);
int TsiStopCache(uint32_t  index);

int 
TsiReadBuffer(
    uint32_t     index,
    uint8_t      *buffer, 
    unsigned int bufferLength);


int 
TsiSkipBuffer(
    uint32_t    tsi_id,
    bool        bSkip);


#ifdef __cplusplus
}
#endif   
#endif // __ITP_DEMOD_THREAD_H__
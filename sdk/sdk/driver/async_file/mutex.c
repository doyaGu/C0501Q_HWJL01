#include "config.h"
#include "async_file/pal.h"
#include "pthread.h"
//#include "sys/sys.h"
#include <malloc.h>

MMP_MUTEX
PalCreateMutex(
    MMP_INT name)
{
    MMP_MUTEX mutex;
    LOG_ENTER "PalCreateMutex(name=%d)\r\n", name LOG_END

    mutex = malloc(sizeof(pthread_mutex_t));
    
    if (mutex)
    {
        if (0 != pthread_mutex_init(mutex, NULL))
        {
            free(mutex);        
            mutex = NULL;
        }
    }

    LOG_LEAVE "PalCreateMutex()=0x%X\r\n", mutex LOG_END
    return mutex;
}

MMP_INT
PalDestroyMutex(
    MMP_MUTEX mutex)
{
    MMP_INT result = 0;
    LOG_ENTER "PalDestroyMutex(mutex=0x%X)\r\n", mutex LOG_END

    //assert(mutex);

    result = pthread_mutex_destroy(mutex);
    if (0 == result)    
        free(mutex);

    LOG_LEAVE "PalDestroyMutex()=%d\r\n", result LOG_END
    return result;
}

MMP_INT
PalWaitMutex(
    MMP_MUTEX mutex,
    MMP_ULONG timeout)
{
    MMP_INT result = 0;
    LOG_ENTER "PalWaitMutex(mutex=0x%X,timeout=%d)\r\n", mutex, timeout LOG_END

    //assert(mutex);

    result = pthread_mutex_lock(mutex);
    
    LOG_LEAVE "PalWaitMutex()=%d\r\n", result LOG_END
    return result;
}

MMP_INT
PalReleaseMutex(
    MMP_MUTEX mutex)
{
    MMP_INT result = 0;
    LOG_ENTER "PalReleaseMutex(mutex=0x%X)\r\n", mutex LOG_END

    //assert(mutex);

    result = pthread_mutex_unlock(mutex);

    LOG_LEAVE "PalReleaseMutex()=%d\r\n", result LOG_END
    return result;
}

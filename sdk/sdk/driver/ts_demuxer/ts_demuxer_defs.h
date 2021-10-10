#ifndef __TS_DEMUXER_DEFS_H_3U6LF050_R1LD_WDD9_F48B_2VQ5HR1XXOHN__
#define __TS_DEMUXER_DEFS_H_3U6LF050_R1LD_WDD9_F48B_2VQ5HR1XXOHN__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>
//=============================================================================
//                Constant Definition
//=============================================================================
#if _MSC_VER
    #define ENABLE_SW_SIMULATION    1
#else
    #define ENABLE_SW_SIMULATION    0
#endif

#define SHOW_TO_DO           0

#define ENABLE_PTHREAD_MUTEX    1
#define ENABLE_DBG_TRACE        0


//=============================================================================
//                Macro Definition
//=============================================================================
#ifndef MEMBER_OFFSET
    #define MEMBER_OFFSET(type, member)     (uint32_t)&(((type *)0)->member)
#endif

#ifndef DOWN_CAST
    #define DOWN_CAST(type, ptr, member)    (type*)((uint32_t)ptr - MEMBER_OFFSET(type, member))
#endif

#ifndef ARRAY_SIZE
    #define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

/**
 * waiting fucntion
 **/
#define WAIT_FOR(condition, timeoutInMs, sleepMs_func, time_gap, bOk) \
    do{ int timeRemain = (timeoutInMs); \
        while(timeRemain > 0) { \
            if(condition) break; \
            sleepMs_func(time_gap); \
            timeRemain-=(time_gap); \
        } \
        if(timeRemain > 0)  bOk = true; \
        else                bOk = false; \
    }while(0)


/**
 * irq fucntion
 **/
#if (ENABLE_SW_SIMULATION)
    #define _disable_irq()
    #define _enable_irq()
#else
    #include "ite/ith.h"
    #include "openrtos/FreeRTOS.h"
    #define _disable_irq()            portENTER_CRITICAL()
    #define _enable_irq()             portEXIT_CRITICAL()
#endif

/**
 *  Debug flag
 */
extern uint32_t  tsdMsgOnFlag;

typedef enum _TSD_MSG_TYPE
{
    TSD_MSG_TYPE_ERR             = (0x1 << 0),

    TSD_MSG_TYPE_TRACE_TSD       = (0X1 << 8),
    TSD_MSG_TYPE_TRACE_SRVC      = (0X1 << 9),
    TSD_MSG_TYPE_TRACE_EPG       = (0X1 << 10),
    TSD_MSG_TYPE_TRACE_PARSER    = (0X1 << 11),
    TSD_MSG_TYPE_TRACE_TSI       = (0X1 << 12),

}TSD_MSG_TYPE;

/**
 * debug message fucntion
 **/
#ifdef _MSC_VER // WIN32
    #ifndef trac
    #define trac(string, ...)               do{ printf(string, __VA_ARGS__); \
                                                printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                            }while(0)
    #endif

    #define tsd_msg(type, string, ...)      ((void)((type & tsdMsgOnFlag) ? printf(string, __VA_ARGS__) : 0))
    #define tsd_msg_ex(type, string, ...)   do{ if(type & tsdMsgOnFlag){ \
                                                   printf(string, __VA_ARGS__); \
                                                   printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                            }while(0)
    #if ENABLE_DBG_TRACE
        #define _trace_enter(string, ...)          do{ printf("enter: %s()[#%d]\n", __FUNCTION__, __LINE__);\
                                                       printf(string, __VA_ARGS__);\
                                                   }while(0)
        #define _trace_leave()                     printf("leave: %s()[#%d]\n", __FUNCTION__, __LINE__);
    #else
        #define _trace_enter(string, ...)
        #define _trace_leave()
    #endif

    #define _tsd_thread_pid()                   -1
#else
    #ifndef trac
    #define trac(string, args...)               do{ printf(string, ## args); \
                                                    printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                                }while(0)
    #endif

    #define tsd_msg(type, string, args...)      ((void)((type & tsdMsgOnFlag) ? printf(string, ## args) : 0))
    #define tsd_msg_ex(type, string, args...)   do{ if(type & tsdMsgOnFlag){ \
                                                       printf(string, ## args); \
                                                       printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                                }while(0)

    #if ENABLE_DBG_TRACE
        #define _trace_enter(string, args...)          do{ printf("  enter: %s()[#%d], ", __FUNCTION__, __LINE__);\
                                                           printf(string, ## args);\
                                                       }while(0)
        #define _trace_leave()                         printf("  leave: %s()[#%d]\n", __FUNCTION__, __LINE__)
    #else
        #define _trace_enter(string, args...)
        #define _trace_leave()
    #endif

    #define _tsd_thread_pid()                   getpid()
#endif

#define tsd_enable_msg(type)                (tsdMsgOnFlag |= type)
#define tsd_disable_msg(type)               (tsdMsgOnFlag &= ~(type))

/**
 * pthread fucntion
 **/
#if ENABLE_PTHREAD_MUTEX
    #include "pthread.h"

    #define _mutex_init(type, mutex)        do{ if(!mutex){\
                                                pthread_mutex_init(&mutex, NULL);\
                                                /*printf(" mutex_init: %s, 0x%x\n", #mutex, mutex);*/}\
                                             }while(0)
    #define _mutex_deinit(type, mutex)      do{if(mutex){pthread_mutex_destroy(&mutex);mutex=0;}}while(0)
    #define _mutex_lock(type, mutex)        do{ if(mutex){\
                                                tsd_msg(type, "  lock %s(mutex=0x%x, thread=0x%x)\n", __FUNCTION__, mutex, _tsd_thread_pid());\
                                                /*printf("  lock %s\n", __FUNCTION__);*/\
                                                pthread_mutex_lock(&mutex);}\
                                            }while(0)
    #define _mutex_unlock(type, mutex)      do{ if(mutex){\
                                                pthread_mutex_unlock(&mutex);\
                                                tsd_msg(type, "  unlock %s(mutex=0x%x, thread=0x%x)\n", __FUNCTION__, mutex, _tsd_thread_pid());\
                                                /*printf("  unlock %s\n", __FUNCTION__);*/}\
                                            }while(0)
#else
    #define pthread_mutex_t           uint32_t
    #define _mutex_init(type, mutex)
    #define _mutex_deinit(type, mutex)
    #define _mutex_lock(type, mutex)
    #define _mutex_unlock(type, mutex)
#endif

/**
 * handle check fucntion
 **/
#define _verify_handle(handle, err_code)   do{if(!handle){\
                                              tsd_msg_ex(TSD_MSG_TYPE_ERR, "Null pointer !!");return err_code;}\
                                           }while(0)

/**
 * Handle malloc memory. Compiler allow size = 0, but our sysetm can't.
 **/
#define tsd_malloc(size)                    ((size) ? malloc(size) : NULL)
//=============================================================================
//                Structure Definition
//=============================================================================


//=============================================================================
//                Global Data Definition
//=============================================================================


//=============================================================================
//                Private Function Definition
//=============================================================================
static void
_tsd_get_clock(
    struct timeval *startT)
{
    gettimeofday(startT, NULL);
}

static uint32_t
_tsd_get_duration(
    struct timeval *startT)
{
    struct timeval currT = {0};
    uint32_t  duration_time = 0;

    gettimeofday(&currT, NULL);
    duration_time = (currT.tv_sec - startT->tv_sec) * 1000;      // sec to ms
    //duration_time += ((currT.tv_usec - startT->tv_usec) / 1000); // us to ms
    return (uint32_t)duration_time;
}


//=============================================================================
//                Public Function Definition
//=============================================================================
// ts parser
#define tsp_msg             tsd_msg
#define tsp_msg_ex          tsd_msg_ex
#define tsp_enable_msg      tsd_enable_msg
#define tsp_disable_msg     tsd_disable_msg
#define TSP_MSG_TYPE_ERR    TSD_MSG_TYPE_ERR

// service info
#define srvc_msg             tsd_msg
#define srvc_msg_ex          tsd_msg_ex
#define srvc_enable_msg      tsd_enable_msg
#define srvc_disable_msg     tsd_disable_msg
#define SRVC_MSG_TYPE_ERR    TSD_MSG_TYPE_ERR

// channel info
#define chnl_msg             tsd_msg
#define chnl_msg_ex          tsd_msg_ex
#define chnl_enable_msg      tsd_enable_msg
#define chnl_disable_msg     tsd_disable_msg
#define CHNL_MSG_TYPE_ERR    TSD_MSG_TYPE_ERR

// epg info
#define epg_msg              tsd_msg
#define epg_msg_ex           tsd_msg_ex
#define epg_enable_msg       tsd_enable_msg
#define epg_disable_msg      tsd_disable_msg
#define EPG_MSG_TYPE_ERR     TSD_MSG_TYPE_ERR

// measure time
#define tsd_get_clock(pStartTime)       _tsd_get_clock(pStartTime)
#define tsd_get_duration(pStartTime)    _tsd_get_duration(pStartTime)

#ifdef __cplusplus
}
#endif

#endif

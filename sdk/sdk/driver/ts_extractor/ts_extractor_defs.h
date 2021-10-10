#ifndef __ts_extractor_defs_H_B8OxweTO_gUyK_gaus_BciU_YvJsNizJPpqk__
#define __ts_extractor_defs_H_B8OxweTO_gUyK_gaus_BciU_YvJsNizJPpqk__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CHECK_MEM_LEAK_ENABLE
    #define CHECK_MEM_LEAK_ENABLE       1
#endif

#if (_MSC_VER && CHECK_MEM_LEAK_ENABLE)
    #define _CRTDBG_MAP_ALLOC
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>
#include <string.h>

#if (_MSC_VER && CHECK_MEM_LEAK_ENABLE)
    #include <crtdbg.h>
#endif
//=============================================================================
//                Constant Definition
//=============================================================================
#if _MSC_VER
    #define ENABLE_SW_SIMULATION    1
#else
    #define ENABLE_SW_SIMULATION    0
#endif

#define ENABLE_PTHREAD_MUTEX    1
#define ENABLE_DBG_TRACE        0

/**
 * indepandant sub module macro
 **/
#define TSPA_LOCAL_MACRO_DISABLE
#define TSS_LOCAL_MACRO_DISABLE
#define TSEXT_LOCAL_MACRO_DISABLE
#define TSPD_LOCAL_MACRO_DISABLE
#define TSA_LOCAL_MACRO_DISABLE
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

//----------------------------
/**
 * bit field pull high/low
 */
#define DEFINE_BIT_OP(bit_size)\
    typedef struct ZONE_SET_T{\
        unsigned short bits_field[((bit_size)+0xF)>>4];\
    }ZONE_SET;

#define BOP_SET(pZone_set_member, bit_order)     ((pZone_set_member)->bits_field[(bit_order)>>4] |=  (1<<((bit_order)&0xF)))
#define BOP_CLR(pZone_set_member, bit_order)     ((pZone_set_member)->bits_field[(bit_order)>>4] &= ~(1<<((bit_order)&0xF)))
#define BOP_IS_SET(pZone_set_member, bit_order)  ((pZone_set_member)->bits_field[(bit_order)>>4] &   (1<<((bit_order)&0xF)))
#define BOP_ZERO(pZone_set_member)               memset((void*)(pZone_set_member),0,sizeof(ZONE_SET))

#define BIT_OP_T    ZONE_SET

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
extern uint32_t  tseMsgOnFlag;

typedef enum _TSE_MSG_TYPE
{
    TSE_MSG_ERR             = (0x1 << 0),

    TSE_MSG_TRACE_TSE       = (0X1 << 8),
    TSE_MSG_TRACE_TSAP      = (0X1 << 9),
    TSE_MSG_TRACE_TSAD      = (0x1 << 10),
    TSE_MSG_TRACE_TSEXT     = (0x1 << 11),
}tse_msg_TYPE;

/**
 * debug message fucntion
 **/
#ifdef _MSC_VER // WIN32
    #ifndef trace
    #define trace(string, ...)              do{ printf(string, __VA_ARGS__); \
                                                printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                            }while(0)
    #endif

    #define tse_msg(type, string, ...)      ((void)((type & tseMsgOnFlag) ? printf(string, __VA_ARGS__) : 0))
    #define tse_msg_ex(type, string, ...)   do{ if(type & tseMsgOnFlag){ \
                                                   printf(string, __VA_ARGS__); \
                                                   printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                            }while(0)
    #if ENABLE_DBG_TRACE
        #define _trace_enter(type, string, ...)    do{ if(type & tseMsgOnFlag){ \
                                                       printf("enter: %s()[#%d]\n", __FUNCTION__, __LINE__);\
                                                       printf(string, __VA_ARGS__);}\
                                                   }while(0)
        #define _trace_leave(type)                 tse_msg(type, "leave: %s()[#%d]\n", __FUNCTION__, __LINE__);
    #else /* ENABLE_DBG_TRACE */
        #define _trace_enter(string, ...)
        #define _trace_leave(type)
    #endif

    #define _tes_byte_align4                    __attribute__ ((aligned(4)))

    #define _tse_thread_pid()                   -1
#else /* _MSC_VER */
    #ifndef trace
    #define trace(string, args...)              do{ printf(string, ## args); \
                                                    printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                                }while(0)
    #endif

    #define tse_msg(type, string, args...)      ((void)((type & tseMsgOnFlag) ? printf(string, ## args) : 0))
    #define tse_msg_ex(type, string, args...)   do{ if(type & tseMsgOnFlag){ \
                                                       printf(string, ## args); \
                                                       printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                                }while(0)

    #if ENABLE_DBG_TRACE
        #define _trace_enter(type, string, args...)    do{ if(type & tseMsgOnFlag){ \
                                                           printf("  enter: %s()[#%d], ", __FUNCTION__, __LINE__);\
                                                           printf(string, ## args);}\
                                                       }while(0)
        #define _trace_leave(type)                     tse_msg(type, "  leave: %s()[#%d]\n", __FUNCTION__, __LINE__)
    #else /* ENABLE_DBG_TRACE */
        #define _trace_enter(string, args...)
        #define _trace_leave(type)
    #endif

    #define _tes_byte_align4                    __attribute__ ((aligned(4)))

    #define _tse_thread_pid()                   getpid()
#endif

#define tse_enable_msg(type)                (tseMsgOnFlag |= type)
#define tse_disable_msg(type)               (tseMsgOnFlag &= ~(type))

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
                                                tse_msg(type, "  lock %s(mutex=0x%x, thread=0x%x)\n", __FUNCTION__, mutex, _tse_thread_pid());\
                                                /*printf("  lock %s\n", __FUNCTION__);*/\
                                                pthread_mutex_lock(&mutex);}\
                                            }while(0)
    #define _mutex_unlock(type, mutex)      do{ if(mutex){\
                                                pthread_mutex_unlock(&mutex);\
                                                tse_msg(type, "  unlock %s(mutex=0x%x, thread=0x%x)\n", __FUNCTION__, mutex, _tse_thread_pid());\
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
                                              tse_msg_ex(TSE_MSG_ERR, "%s Null pointer !!", #handle);\
                                              return err_code;}\
                                           }while(0)

/**
 * Handle malloc memory. Compiler allow size = 0, but our sysetm can't.
 **/
#define tse_malloc(size)                    ((size) ? malloc(size) : NULL)
//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================
#if (_MSC_VER)
    // measure time
    #define tse_get_clock(pStartTime)       0
    #define tse_get_duration(pStartTime)    (-1)

#else
    static void
    _tse_get_clock(
        struct timeval *startT)
    {
        gettimeofday(startT, NULL);
    }

    static uint32_t
    _tse_get_duration(
        struct timeval *startT)
    {
        struct timeval currT = {0};
        uint32_t  duration_time = 0;

        gettimeofday(&currT, NULL);
        duration_time = (currT.tv_sec - startT->tv_sec) * 1000;      // sec to ms
        // duration_time += ((currT.tv_usec - startT->tv_usec) / 1000); // us to ms
        return (uint32_t)duration_time;
    }

    // measure time
    #define tse_get_clock(pStartTime)       _tse_get_clock(pStartTime)
    #define tse_get_duration(pStartTime)    _tse_get_duration(pStartTime)
#endif

//=============================================================================
//                Public Function Definition
//=============================================================================
#if defined(TSPA_LOCAL_MACRO_DISABLE)
// ts packet analysis
    #ifndef tspa_malloc
        #define tspa_malloc         tse_malloc
    #endif

    #define tspa_msg             tse_msg
    #define tspa_msg_ex          tse_msg_ex
    #define tspa_enable_msg      tse_enable_msg
    #define tspa_disable_msg     tse_disable_msg
    #define TSPA_MSG_ERR         TSE_MSG_ERR
#endif

#if defined(TSS_LOCAL_MACRO_DISABLE)
// ts split
    #ifndef tss_malloc
        #define tss_malloc          tse_malloc
    #endif

    #define tss_msg             tse_msg
    #define tss_msg_ex          tse_msg_ex
    #define tss_enable_msg      tse_enable_msg
    #define tss_disable_msg     tse_disable_msg
    #define TSS_MSG_ERR         TSE_MSG_ERR
#endif

#if defined(TSEXT_LOCAL_MACRO_DISABLE)
// ts extract
    #ifndef tsext_malloc
        #define tsext_malloc          tse_malloc
    #endif

    #define tsext_msg             tse_msg
    #define tsext_msg_ex          tse_msg_ex
    #define tsext_enable_msg      tse_enable_msg
    #define tsext_disable_msg     tse_disable_msg
    #define TSEXT_MSG_ERR         TSE_MSG_ERR
#endif

#if defined(TSPD_LOCAL_MACRO_DISABLE)
// ts packet demux
    #ifndef tspd_malloc
        #define tspd_malloc          tse_malloc
    #endif

    #define tspd_msg             tse_msg
    #define tspd_msg_ex          tse_msg_ex
    #define tspd_enable_msg      tse_enable_msg
    #define tspd_disable_msg     tse_disable_msg
    #define TSPD_MSG_ERR         TSE_MSG_ERR
#endif

#if defined(TSA_LOCAL_MACRO_DISABLE)
// ts aggregation
    #ifndef tsa_malloc
        #define tsa_malloc          tse_malloc
    #endif

    #define tsa_msg             tse_msg
    #define tsa_msg_ex          tse_msg_ex
    #define tsa_enable_msg      tse_enable_msg
    #define tsa_disable_msg     tse_disable_msg
    #define TSA_MSG_ERR         TSE_MSG_ERR
#endif




#ifdef __cplusplus
}
#endif

#endif

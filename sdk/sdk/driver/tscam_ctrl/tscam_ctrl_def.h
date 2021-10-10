#ifndef __tscam_ctrl_def_H_Ke62vODK_DbnM_kLzm_MH8G_CsUSSALtYXpZ__
#define __tscam_ctrl_def_H_Ke62vODK_DbnM_kLzm_MH8G_CsUSSALtYXpZ__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CHECK_MEM_LEAK_ENABLE
    #define CHECK_MEM_LEAK_ENABLE       1
#endif

#if (_MSC_VER && CHECK_MEM_LEAK_ENABLE) && !defined(_CRTDBG_MAP_ALLOC)
    #define _CRTDBG_MAP_ALLOC
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>

#if (_MSC_VER && CHECK_MEM_LEAK_ENABLE)
    #include <crtdbg.h>
#endif
//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================
#if _MSC_VER
    #define ENABLE_SW_SIMULATION    1
#else
    #define ENABLE_SW_SIMULATION    0
#endif

#define ENABLE_PTHREAD_MUTEX    1


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
extern uint32_t  tscmMsgOnFlag;

typedef enum _TSCM_MSG_TYPE
{
    TSCM_MSG_ERR               = (0x1 << 0),

    TSCM_MSG_TRACE_TSCM        = (0X1 << 8),

}TSCM_MSG_TYPE;

/**
 * debug message fucntion
 **/
#ifdef _MSC_VER // WIN32
    #ifndef trace
    #define trace(string, ...)              do{ printf(string, __VA_ARGS__); \
                                                printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                            }while(0)
    #endif

    #define tscm_msg(type, string, ...)      ((void)((type & tscmMsgOnFlag) ? printf(string, __VA_ARGS__) : 0))
    #define tscm_msg_ex(type, string, ...)   do{ if(type & tscmMsgOnFlag){ \
                                                   printf(string, __VA_ARGS__); \
                                                   printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                             }while(0)

    #define _tscm_thread_pid()               -1
#else
    #ifndef trace
    #define trace(string, args...)              do{ printf(string, ## args); \
                                                    printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                                }while(0)
    #endif

    #define tscm_msg(type, string, args...)      ((void)((type & tscmMsgOnFlag) ? printf(string, ## args) : 0))
    #define tscm_msg_ex(type, string, args...)   do{ if(type & tscmMsgOnFlag){ \
                                                       printf(string, ## args); \
                                                       printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                                 }while(0)

    #define _tscm_thread_pid()                   getpid()
#endif

#define tscm_enable_msg(type)                (tscmMsgOnFlag |= type)
#define tscm_disable_msg(type)               (tscmMsgOnFlag &= ~(type))


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
                                                tscm_msg(type, "  lock %s(mutex=0x%x, thread=0x%x)\n", __FUNCTION__, mutex, _tscm_thread_pid());\
                                                /*printf("  lock %s\n", __FUNCTION__);*/\
                                                pthread_mutex_lock(&mutex);}\
                                            }while(0)
    #define _mutex_unlock(type, mutex)      do{ if(mutex){\
                                                pthread_mutex_unlock(&mutex);\
                                                tscm_msg(type, "  unlock %s(mutex=0x%x, thread=0x%x)\n", __FUNCTION__, mutex, _tscm_thread_pid());\
                                                /*printf("  unlock %s\n", __FUNCTION__);*/}\
                                            }while(0)
#else
    #define pthread_mutex_t                 uint32_t
    #define _mutex_init(type, mutex)
    #define _mutex_deinit(type, mutex)
    #define _mutex_lock(type, mutex)
    #define _mutex_unlock(type, mutex)
#endif

/**
 * handle check fucntion
 **/
#define _verify_handle(handle, err_code)   do{if(!handle){\
                                              tscm_msg_ex(TSCM_MSG_ERR, "%s Null pointer !!", #handle);\
                                              return err_code;}\
                                           }while(0)

/**
 * Handle malloc memory. Compiler allow size = 0, but our sysetm can't.
 **/
#define tscm_malloc(size)                    ((size) ? malloc(size) : 0)
//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================
static uint8_t
_tscm_gen_check_sum(
    uint8_t     *pCur,
    uint32_t    size)
{
    uint32_t    rst = 0;
    uint32_t    i = 0;
    for(i= 0; i < size; ++i)
        rst += (*(pCur + i));
    return (uint8_t)(rst & 0xFF);
}

//=============================================================================
//                Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif

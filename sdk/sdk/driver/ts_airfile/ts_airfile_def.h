#ifndef __TS_AIRFILE_DEF_H_PUYSNGRO_T1OB_GN05_M5NH_DKETTQEREABT__
#define __TS_AIRFILE_DEF_H_PUYSNGRO_T1OB_GN05_M5NH_DKETTQEREABT__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>
//=============================================================================
//                Constant Definition
//=============================================================================

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
 *  Debug flag
 */
extern uint32_t  tsafMsgOnFlag;

typedef enum _TSAF_MSG_TYPE
{
    TSAF_MSG_TYPE_ERR             = (0x1 << 0),
    TSAF_MSG_TYPE_CACHE_BUF,
    TSAF_MSG_TYPE_TRACE_TSAF      = (0x1 << 11),
    TSAF_MSG_TYPE_TRACE_CACHE_BUF = (0x1 << 12),

}TSAF_MSG_TYPE;

/**
 * debug message fucntion
 **/
#ifdef _MSC_VER // WIN32
    #ifndef trace
    #define trace(string, ...)                      do{ printf(string, __VA_ARGS__); \
                                                        printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                                    }while(0)
    #endif

    #define tsaf_msg(type, string, ...)             ((void)((type & tsafMsgOnFlag) ? printf(string, __VA_ARGS__) : 0))
    #define tsaf_msg_ex(type, string, ...)          do{ if(type & tsafMsgOnFlag){ \
                                                        printf(string, __VA_ARGS__); \
                                                        printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                                    }while(0)
    #if ENABLE_DBG_TRACE
        #define _tsaf_trace_enter(type, string, ...)    do{ tsaf_msg(type, "enter: %s()[#%d]\n", __FUNCTION__, __LINE__);\
                                                             tsaf_msg(type, string, __VA_ARGS__);\
                                                        }while(0)
        #define _tsaf_trace_leave(type)                 tsaf_msg(type, "leave: %s()[#%d]\n", __FUNCTION__, __LINE__);
    #else
        #define _tsaf_trace_enter(type, string, ...)
        #define _tsaf_trace_leave(type)
    #endif

    #define _tsaf_thread_pid()                           -1
#else
    #ifndef trace
    #define trace(string, args...)                  do{ printf(string, ## args); \
                                                        printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                                    }while(0)
    #endif

    #define tsaf_msg(type, string, args...)         ((void)((type & tsafMsgOnFlag) ? printf(string, ## args) : 0))
    #define tsaf_msg_ex(type, string, args...)      do{ if(type & tsafMsgOnFlag){ \
                                                       printf(string, ## args); \
                                                       printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                                    }while(0)

    #if ENABLE_DBG_TRACE
        #define _tsaf_trace_enter(type, string, args...)    do{ tsaf_msg(type, "  enter: %s()[#%d], ", __FUNCTION__, __LINE__);\
                                                                tsaf_msg(type, string, ## args);\
                                                            }while(0)
        #define _tsaf_trace_leave(type)                     tsaf_msg(type, "  leave: %s()[#%d]\n", __FUNCTION__, __LINE__)
    #else
        #define _tsaf_trace_enter(type, string, args...)
        #define _tsaf_trace_leave(type)
    #endif

    #define _tsaf_thread_pid()                              getpid()
#endif


/**
 * pthread fucntion
 **/
#if ENABLE_PTHREAD_MUTEX
    #include "pthread.h"

    #define _tsaf_thread_exit(ptr)              pthread_exit(ptr)
    #define _tsaf_thread_join(pThread, attr)    do{ if(pThread)pthread_join((*pThread), attr);}while(0)

    #define _tsaf_mutex_init(type, mutex)       do{ if(!mutex){\
                                                    pthread_mutex_init(&mutex, NULL);\
                                                    tsaf_msg(type, " mutex_init: %s, 0x%x\n", #mutex, mutex);}\
                                                }while(0)
    #define _tsaf_mutex_deinit(type, mutex)     do{ if(mutex){\
                                                    pthread_mutex_destroy(&mutex);mutex=0;\
                                                    tsaf_msg(type, " mutex_deinit: %s\n", #mutex);}\
                                                }while(0)
    #define _tsaf_mutex_lock(type, mutex)       do{ if(mutex){\
                                                    tsaf_msg(type, "  lock %s(pid=0x%x)\n", __FUNCTION__, _tsaf_thread_pid());\
                                                    pthread_mutex_lock(&mutex);}\
                                                }while(0)
    #define _tsaf_mutex_unlock(type, mutex)     do{ if(mutex){\
                                                    pthread_mutex_unlock(&mutex);\
                                                    tsaf_msg(type, "  unlock %s(pid=0x%x)\n", __FUNCTION__, _tsaf_thread_pid());}\
                                                }while(0)
#else
    #define pthread_mutex_t                     uint32_t
    #define pthread_t                           uitn32_t
    #define _tsaf_thread_exit(ptr)
    #define _tsaf_mutex_init(type, mutex)
    #define _tsaf_mutex_deinit(type, mutex)
    #define _tsaf_mutex_lock(type, mutex)
    #define _tsaf_mutex_unlock(type, mutex)
#endif

/**
 * handle check fucntion
 **/
#define _tsaf_verify_handle(type, handle, mutex, err_code)    \
            do{ if(!handle){\
                /*_tsaf_mutex_deinit(type, mutex);*/\
                tsaf_msg(1, "Null pointer !!\n");\
                return err_code;}\
            }while(0)

/**
 * Handle malloc memory. Compiler allow size = 0, but our sysetm can't.
 **/
#define tsaf_malloc(size)                    ((size) ? malloc(size) : NULL)
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
tsaf_get_clock(
    struct timeval *startT)
{
    gettimeofday(startT, NULL);
}

static uint32_t
tsaf_get_duration(
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


#ifdef __cplusplus
}
#endif

#endif

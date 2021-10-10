#ifndef __jpg_defs_H_bMUPZPye_Z18T_1nOV_eySo_Wsq58wEedBg1__
#define __jpg_defs_H_bMUPZPye_Z18T_1nOV_eySo_Wsq58wEedBg1__

#ifdef __cplusplus
extern "C" {
#endif

//#define _CRTDBG_MAP_ALLOC

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "jpg_config.h"

//#include <crtdbg.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
#define ENABLE_PTHREAD_MUTEX        1
#define ENABLE_JDBG_TRACE           0
#define ENABLE_JDEBUG_MODE          0
#define ENABLE_JPG_SW_SIMULSTION    0

//=============================================================================
//                  Macro Definition
//=============================================================================
#define MEMBER_OFFSET(type, member)     (uint32_t)&(((type *)0)->member)
#define DOWN_CAST(type, ptr, member)    (type*)((uint32_t)ptr - MEMBER_OFFSET(type, member))

/**
 * some platform api
 **/
#if (CFG_ITP_PLATFORM) && !(ENABLE_JPG_SW_SIMULSTION)
    #include "jpg_platform_itp.h"
#elif (CFG_PAL_PLATFORM) && !(ENABLE_JPG_SW_SIMULSTION)
    #include "jpg_platform_pal.h"
#else
    #include "jpg_platform_null.h"
#endif

/**
 *  Debug flag
 */
extern uint32_t  jpgMsgOnFlag;

typedef enum _JPG_MSG_TYPE
{
    JPG_MSG_TYPE_ERR             = (0x1 << 0),
    JPG_MSG_TYPE_TRACE_ITEJPG    = (0X1 << 12),
    JPG_MSG_TYPE_TRACE_PARSER    = (0X1 << 13),
    JPG_MSG_TYPE_TRACE_JCOMM     = (0X1 << 14),
    JPG_MSG_TYPE_TRACE_DEC       = (0X1 << 15),
    JPG_MSG_TYPE_TRACE_ENC       = (0X1 << 16),
    JPG_MSG_TYPE_TRACE_DEC_MJPG  = (0X1 << 17),
    JPG_MSG_TYPE_TRACE_ENC_MJPG  = (0X1 << 18),
    JPG_MSG_TYPE_TRACE_DEC_JPG_CMD  = (0X1 << 19),
}JPG_MSG_TYPE;

/**
 * debug message fucntion
 **/
#if (_MSC_VER) // WIN32
    #ifndef trace
    #define trace(string, ...)                      do{ printf(string, __VA_ARGS__); \
                                                        printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                                    }while(0)
    #endif

    #define jpg_msg(type, string, ...)              ((void)((type & jpgMsgOnFlag) ? printf(string, __VA_ARGS__) : 0))
    #define jpg_msg_ex(type, string, ...)           do{ if(type & jpgMsgOnFlag){ \
                                                        printf(string, __VA_ARGS__); \
                                                        printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                                    }while(0)
    #if ENABLE_JDBG_TRACE
        #define _jpg_trace_enter(type, string, ...) do{ jpg_msg(type, "enter: %s()[#%d]\n", __FUNCTION__, __LINE__);\
                                                        jpg_msg(type, string, __VA_ARGS__);\
                                                    }while(0)
        #define _jpg_trace_leave(type)              jpg_msg(type, "leave: %s()[#%d]\n", __FUNCTION__, __LINE__);
    #else
        #define _jpg_trace_enter(type, string, ...)
        #define _jpg_trace_leave(type)
    #endif

    #define _jpg_byte_align4                        __attribute__ ((aligned(4)))

    #define _jpg_thread_pid()                       -1

#elif (__GNUC__)
    #ifndef trace
    #define trace(string, args...)                  do{ printf(string, ## args); \
                                                        printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                                    }while(0)
    #endif

    #define jpg_msg(type, string, args...)          ((void)((type & jpgMsgOnFlag) ? printf(string, ## args) : 0))
    #define jpg_msg_ex(type, string, args...)       do{ if(type & jpgMsgOnFlag){ \
                                                        printf(string, ## args); \
                                                        printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                                    }while(0)

    #if ENABLE_JDBG_TRACE
        #define _jpg_trace_enter(type, string, args...) do{ jpg_msg(type, "  enter: %s()[#%d], ", __FUNCTION__, __LINE__);\
                                                            jpg_msg(type, string, ## args);\
                                                        }while(0)
        #define _jpg_trace_leave(type)                  jpg_msg(type, "  leave: %s()[#%d]\n", __FUNCTION__, __LINE__)
    #else
        #define _jpg_trace_enter(type, string, args...)
        #define _jpg_trace_leave(type)
    #endif

    #define _jpg_byte_align4                        __attribute__ ((aligned(4)))

    #define _jpg_thread_pid()                       getpid()

#else
    #ifndef trace
    #define trace(string, args...)
    #endif
    #define jpg_msg(type, string, args...)
    #define jpg_msg_ex(type, string, args...)
    #define _jpg_trace_enter(type, string, args...)
    #define _jpg_trace_leave(type)
    #define _jpg_byte_align4
    #define _jpg_thread_pid()                           -1
#endif

/**
 * pthread fucntion
 **/
#if ENABLE_PTHREAD_MUTEX
    #include "pthread.h"

    #define _jpg_thread_exit(ptr)               pthread_exit(ptr)
    #define _jpg_thread_join(pThread, attr)     do{ if(pThread)pthread_join((*pThread), attr);}while(0)

    #define _jpg_mutex_init(type, mutex)        do{ if(!mutex){\
                                                    pthread_mutex_init(&mutex, NULL);\
                                                    jpg_msg(type, " mutex_init: %s, 0x%x\n", #mutex, mutex);}\
                                                }while(0)
    #define _jpg_mutex_deinit(type, mutex)      do{ if(mutex){\
                                                    pthread_mutex_destroy(&mutex);mutex=0;\
                                                    jpg_msg(type, " mutex_deinit: %s\n", #mutex);}\
                                                }while(0)
    #define _jpg_mutex_lock(type, mutex)        do{ if(mutex){\
                                                    jpg_msg(type, "  lock %s(pid=0x%x)\n", __FUNCTION__, _jpg_thread_pid());\
                                                    pthread_mutex_lock(&mutex);}\
                                                }while(0)
    #define _jpg_mutex_unlock(type, mutex)      do{ if(mutex){\
                                                    pthread_mutex_unlock(&mutex);\
                                                    jpg_msg(type, "  unlock %s(pid=0x%x)\n", __FUNCTION__, _jpg_thread_pid());}\
                                                }while(0)
#else
    #define pthread_mutex_t                     uint32_t
    #define pthread_t                           uitn32_t
    #define _jpg_thread_exit(ptr)
    #define _jpg_mutex_init(type, mutex)
    #define _jpg_mutex_deinit(type, mutex)
    #define _jpg_mutex_lock(type, mutex)
    #define _jpg_mutex_unlock(type, mutex)
#endif


/**
 * handle check fucntion
 **/
#define _jpg_verify_handle(type, handle, mutex, err_code)    \
            do{ if(!(handle)){\
                /*_jpg_mutex_deinit(type, mutex);*/\
                jpg_msg(1, "%s Null pointer !!\n", #handle);\
                return err_code;}\
            }while(0)

/**
 * Handle malloc memory. Compiler allow size = 0, but our sysetm can't.
 **/
#define jpg_malloc(size)                    ((size) ? malloc(size) : 0)
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif

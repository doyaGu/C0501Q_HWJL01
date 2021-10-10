#ifndef __ts_aggre_defs_H_j6A8GaK5_NJTk_H2QV_7w33_Cy3VxdoCl9yO__
#define __ts_aggre_defs_H_j6A8GaK5_NJTk_H2QV_7w33_Cy3VxdoCl9yO__

#ifdef __cplusplus
extern "C" {
#endif


#include "ts_extractor_defs.h"

#define TSA_CHECK_MEM_LEAK_ENABLE       0

#if (_MSC_VER && TSA_CHECK_MEM_LEAK_ENABLE)
    #define _CRTDBG_MAP_ALLOC
#endif


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#if (_MSC_VER && TSA_CHECK_MEM_LEAK_ENABLE)
    #include <crtdbg.h>
#endif
//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================
#if !defined(TSA_LOCAL_MACRO_DISABLE)

#ifndef MEMBER_OFFSET
    #define MEMBER_OFFSET(type, member)     (uint32_t)&(((type *)0)->member)
#endif

#ifndef DOWN_CAST
    #define DOWN_CAST(type, ptr, member)    (type*)((uint32_t)ptr - MEMBER_OFFSET(type, member))
#endif


#define ENABLE_PTHREAD_MUTEX    1
#define ENABLE_DBG_TRACE        0

/**
 *  Debug flag
 */
extern uint32_t  tsaMsgOnFlag;

typedef enum _TSA_MSG_TYPE
{
    TSA_MSG_ERR             = (0x1 << 0),

    TSA_MSG_TRACE_TSA       = (0x1 << 9),

}TSA_MSG_TYPE;

/**
 * debug message fucntion
 **/
#ifdef _MSC_VER // WIN32
    #ifndef trace
    #define trace(string, ...)              do{ printf(string, __VA_ARGS__); \
                                                printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                            }while(0)
    #endif

    #define tsa_msg(type, string, ...)      ((void)((type & tsaMsgOnFlag) ? printf(string, __VA_ARGS__) : 0))
    #define tsa_msg_ex(type, string, ...)   do{ if(type & tsaMsgOnFlag){ \
                                                   printf(string, __VA_ARGS__); \
                                                   printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                            }while(0)
    #if ENABLE_DBG_TRACE
        #define _trace_enter(type, string, ...)    do{ if(type & tsaMsgOnFlag){ \
                                                       printf("enter: %s()[#%d]\n", __FUNCTION__, __LINE__);\
                                                       printf(string, __VA_ARGS__);}\
                                                   }while(0)
        #define _trace_leave(type)                 tsa_msg(type, "leave: %s()[#%d]\n", __FUNCTION__, __LINE__);
    #else /* ENABLE_DBG_TRACE */
        #define _trace_enter(string, ...)
        #define _trace_leave(type)
    #endif

    #define _tsa_thread_pid()                   -1
#else /* _MSC_VER */
    #ifndef trace
    #define trace(string, args...)              do{ printf(string, ## args); \
                                                    printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                                }while(0)
    #endif

    #define tsa_msg(type, string, args...)      ((void)((type & tsaMsgOnFlag) ? printf(string, ## args) : 0))
    #define tsa_msg_ex(type, string, args...)   do{ if(type & tsaMsgOnFlag){ \
                                                       printf(string, ## args); \
                                                       printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                                }while(0)

    #if ENABLE_DBG_TRACE
        #define _trace_enter(type, string, args...)    do{ if(type & tsaMsgOnFlag){ \
                                                           printf("  enter: %s()[#%d], ", __FUNCTION__, __LINE__);\
                                                           printf(string, ## args);}\
                                                       }while(0)
        #define _trace_leave(type)                     tsa_msg(type, "  leave: %s()[#%d]\n", __FUNCTION__, __LINE__)
    #else /* ENABLE_DBG_TRACE */
        #define _trace_enter(string, args...)
        #define _trace_leave(type)
    #endif

    #define _tsa_thread_pid()                   getpid()
#endif

#define tspa_enable_msg(type)                (tsaMsgOnFlag |= type)
#define tspa_disable_msg(type)               (tsaMsgOnFlag &= ~(type))



/**
 * Handle malloc memory. Compiler allow size = 0, but our sysetm can't.
 **/
#ifndef tsa_malloc
    #define tsa_malloc(size)                    ((size) ? malloc(size) : NULL)
#endif

#endif /* !defined(TSA_LOCAL_MACRO_DISABLE) */
//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif

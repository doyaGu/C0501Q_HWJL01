#ifndef __ts_split_defs_H_xnQTwmoi_y0LT_se0r_cJnw_RCMlyIO3aQEI__
#define __ts_split_defs_H_xnQTwmoi_y0LT_se0r_cJnw_RCMlyIO3aQEI__

#ifdef __cplusplus
extern "C" {
#endif


#include "ts_extractor_defs.h"

#define TSS_CHECK_MEM_LEAK_ENABLE       0

#if (_MSC_VER && TSS_CHECK_MEM_LEAK_ENABLE)
    #define _CRTDBG_MAP_ALLOC
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
// #include <string.h>

#if (_MSC_VER && TSS_CHECK_MEM_LEAK_ENABLE)
    #include <crtdbg.h>
#endif
//=============================================================================
//                Constant Definition
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
//=============================================================================
//                Macro Definition
//=============================================================================
#if !defined(TSS_LOCAL_MACRO_DISABLE)

#define ENABLE_PTHREAD_MUTEX    1
#define ENABLE_DBG_TRACE        0

/**
 *  Debug flag
 */
extern uint32_t  tssMsgOnFlag;

typedef enum _TSS_MSG_TYPE
{
    TSS_MSG_ERR             = (0x1 << 0),

    TSS_MSG_TRACE_TSAP      = (0X1 << 9),

}TSS_MSG_TYPE;

/**
 * debug message fucntion
 **/
#ifdef _MSC_VER // WIN32
    #ifndef trace
    #define trace(string, ...)              do{ printf(string, __VA_ARGS__); \
                                                printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                            }while(0)
    #endif

    #define tss_msg(type, string, ...)      ((void)((type & tssMsgOnFlag) ? printf(string, __VA_ARGS__) : 0))
    #define tss_msg_ex(type, string, ...)   do{ if(type & tssMsgOnFlag){ \
                                                   printf(string, __VA_ARGS__); \
                                                   printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                            }while(0)
    #if ENABLE_DBG_TRACE
        #define _trace_enter(type, string, ...)    do{ if(type & tssMsgOnFlag){ \
                                                       printf("enter: %s()[#%d]\n", __FUNCTION__, __LINE__);\
                                                       printf(string, __VA_ARGS__);}\
                                                   }while(0)
        #define _trace_leave(type)                 tss_msg(type, "leave: %s()[#%d]\n", __FUNCTION__, __LINE__);
    #else /* ENABLE_DBG_TRACE */
        #define _trace_enter(string, ...)
        #define _trace_leave(type)
    #endif

    #define _tss_thread_pid()                   -1
#else /* _MSC_VER */
    #ifndef trace
    #define trace(string, args...)              do{ printf(string, ## args); \
                                                    printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                                }while(0)
    #endif

    #define tss_msg(type, string, args...)      ((void)((type & tssMsgOnFlag) ? printf(string, ## args) : 0))
    #define tss_msg_ex(type, string, args...)   do{ if(type & tssMsgOnFlag){ \
                                                       printf(string, ## args); \
                                                       printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                                }while(0)

    #if ENABLE_DBG_TRACE
        #define _trace_enter(type, string, args...)    do{ if(type & tssMsgOnFlag){ \
                                                           printf("  enter: %s()[#%d], ", __FUNCTION__, __LINE__);\
                                                           printf(string, ## args);}\
                                                       }while(0)
        #define _trace_leave(type)                     tss_msg(type, "  leave: %s()[#%d]\n", __FUNCTION__, __LINE__)
    #else /* ENABLE_DBG_TRACE */
        #define _trace_enter(string, args...)
        #define _trace_leave(type)
    #endif

    #define _tss_thread_pid()                   getpid()
#endif

#define tss_enable_msg(type)                (tssMsgOnFlag |= type)
#define tss_disable_msg(type)               (tssMsgOnFlag &= ~(type))



/**
 * Handle malloc memory. Compiler allow size = 0, but our sysetm can't.
 **/
#ifndef tss_malloc
    #define tss_malloc(size)                    ((size) ? malloc(size) : NULL)
#endif

#endif /* !defined(TSS_LOCAL_MACRO_DISABLE) */
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

#ifndef __demod_ctrl_defs_H_36uVjNeq_CHIX_BBfz_4tap_lI7REPHbkp8r__
#define __demod_ctrl_defs_H_36uVjNeq_CHIX_BBfz_4tap_lI7REPHbkp8r__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>
//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================
#ifndef MEMBER_OFFSET
    #define MEMBER_OFFSET(type, member)     (uint32_t)&(((type *)0)->member)
#endif

#ifndef DOWN_CAST
    #define DOWN_CAST(type, ptr, member)    (type*)((uint32_t)ptr - MEMBER_OFFSET(type, member))
#endif

/**
 *  Debug flag
 */
extern uint32_t  demMsgOnFlag;

typedef enum _DEM_MSG_TYPE
{
    DEM_MSG_TYPE_ERR             = (0x1 << 0),

}DEM_MSG_TYPE;

/**
 * debug message fucntion
 **/
#ifdef _MSC_VER // WIN32
    #ifndef trace
    #define trace(string, ...)              do{ printf(string, __VA_ARGS__); \
                                                printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                            }while(0)
    #endif

    #define dem_msg(type, string, ...)      ((void)((type & demMsgOnFlag) ? printf(string, __VA_ARGS__) : 0))
    #define dem_msg_ex(type, string, ...)   do{ if(type & demMsgOnFlag){ \
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
    #ifndef trace
    #define trace(string, args...)              do{ printf(string, ## args); \
                                                    printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                                }while(0)
    #endif

    #define dem_msg(type, string, args...)      ((void)((type & demMsgOnFlag) ? printf(string, ## args) : 0))
    #define dem_msg_ex(type, string, args...)   do{ if(type & demMsgOnFlag){ \
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

#define dem_enable_msg(type)                (demMsgOnFlag |= type)
#define dem_disable_msg(type)               (demMsgOnFlag &= ~(type))

/**
 * Handle malloc memory. Compiler allow size = 0, but our sysetm can't.
 **/
#define dem_malloc(size)                    ((size) ? malloc(size) : NULL)
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
_dem_get_clock(
    struct timeval *startT)
{
    gettimeofday(startT, NULL);
}

static uint32_t
_dem_get_duration(
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
// measure time
#define dem_get_clock(pStartTime)       _dem_get_clock(pStartTime)
#define dem_get_duration(pStartTime)    _dem_get_duration(pStartTime)


#ifdef __cplusplus
}
#endif

#endif

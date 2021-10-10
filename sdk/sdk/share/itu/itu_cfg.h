#ifndef ITU_CFG_H
#define ITU_CFG_H

#include <stdio.h>

#define PRINTF printf

/* Log fucntions definition */
#define STRINGIFY(x)    #x
#define TOSTRING(x)     STRINGIFY(x)

#define LOG_PREFIX   __FILE__ ":" TOSTRING(__LINE__) ": "

#ifdef CFG_ITU_ERR
    #define LOG_ERR     PRINTF("ERR:" LOG_PREFIX
#else
    #define LOG_ERR     (void)(1 ? 0 :
#endif

#ifdef CFG_ITU_WARN
    #define LOG_WARN    PRINTF("WARN:" LOG_PREFIX
#else
    #define LOG_WARN    (void)(1 ? 0 :
#endif

#ifdef CFG_ITU_INFO
    #define LOG_INFO    PRINTF("INFO:"
#else
    #define LOG_INFO    (void)(1 ? 0 :
#endif

#ifdef CFG_ITU_DBG
    #define LOG_DBG     PRINTF("DBG:"
#else
    #define LOG_DBG     (void)(1 ? 0 :
#endif

#ifdef CFG_ITU_LOAD
    #define LOG_LOAD    PRINTF("LOAD:"
#else
    #define LOG_LOAD    (void)(1 ? 0 :
#endif

#ifdef CFG_ITU_UPDATE
    #define LOG_UPDATE  PRINTF("UPDATE:"
#else
    #define LOG_UPDATE  (void)(1 ? 0 :
#endif

#ifdef CFG_ITU_DRAW
    #define LOG_DRAW    PRINTF("DRAW:"
#else
    #define LOG_DRAW    (void)(1 ? 0 :
#endif

#define LOG_END         );

#define ITU_SCROLL_DELAY                3
#define ITU_STOP_DELAY                  30

#ifndef CFG_M2D_ENABLE
    #undef CFG_CHIP_FAMILY
    #define CFG_CHIP_FAMILY 9070
#endif // CFG_M2D_ENABLE

#if defined(CFG_ITU_ASSERT_THREAD) || !defined(NDEBUG)

#define FUNCTIONIZE(a,b)    a(b)
#define STRINGIZE(a)        #a
#define INT2STRING(i)       FUNCTIONIZE(STRINGIZE,i)
#define ITU_FILE_POS        __FILE__ ":" INT2STRING(__LINE__)

#define ITU_ASSERT_THREAD() ituAssertThread(ITU_FILE_POS)

#else

#define ITU_ASSERT_THREAD()

#endif // defined(CFG_ITU_ASSERT_THREAD) || !defined(NDEBUG)

#endif // ITU_CFG_H

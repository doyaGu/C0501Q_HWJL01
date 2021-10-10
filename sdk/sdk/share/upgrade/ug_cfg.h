#ifndef UG_CFG_H
#define UG_CFG_H

#include <stdio.h>

#define PRINTF printf

/* Log fucntions definition */
#define STRINGIFY(x)    #x
#define TOSTRING(x)     STRINGIFY(x)

#define LOG_PREFIX   __FILE__ ":" TOSTRING(__LINE__) ": "

#ifdef CFG_UG_ERR
    #define LOG_ERR     PRINTF("ERR:" LOG_PREFIX
#else
    #define LOG_ERR     (void)(1 ? 0 :
#endif

#ifdef CFG_UG_WARN
    #define LOG_WARN    PRINTF("WARN:" LOG_PREFIX
#else
    #define LOG_WARN    (void)(1 ? 0 :
#endif

#ifdef CFG_UG_INFO
    #define LOG_INFO    PRINTF(
#else
    #define LOG_INFO    (void)(1 ? 0 :
#endif

#ifdef CFG_UG_DBG
    #define LOG_DBG     PRINTF("DBG:"
#else
    #define LOG_DBG     (void)(1 ? 0 :
#endif

#define LOG_END         );

#endif // UG_CFG_H

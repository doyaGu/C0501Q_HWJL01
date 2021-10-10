#ifndef __GFX_MSG_H__
#define __GFX_MSG_H__

#include <stdio.h>

//#define GFX_USE_VERBOSE_LOG
#define GFX_USE_ERROR_LOG

#ifdef _WIN32

#if defined(GFX_USE_ERROR_LOG) && defined(__GNUC__)
#define GFX_ERROR_MSG(format, args...)      printf("[GFX]%s(%d)[ERROR]: "format, __FUNCTION__, __LINE__, ##args)
#elif defined(GFX_USE_ERROR_LOG)
#define GFX_ERROR_MSG(format, ...)      printf("[GFX]%s(%d)[ERROR]: "format, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define GFX_ERROR_MSG(format, args...)
#endif

#define GFX_LOG_MSG(format, ...)        printf("[GFX]%s(%d) "format, __FUNCTION__, __LINE__, __VA_ARGS__)

#ifdef GFX_USE_VERBOSE_LOG
#define GFX_VERBOSE_LOG(format, ...)    printf("[GFX]%s(%d) "format, __FUNCTION__, __LINE__, __VA_ARGS__args)
#else
#define GFX_VERBOSE_LOG(format, ...)
#endif

#else

#ifdef GFX_USE_ERROR_LOG
#define GFX_ERROR_MSG(format, args...)      printf("[GFX]%s(%d)[ERROR]: "format, __FUNCTION__, __LINE__, ##args)
#else
#define GFX_ERROR_MSG(format, args...)
#endif

#define GFX_LOG_MSG(format, args...)        printf("[GFX]%s(%d) "format, __FUNCTION__, __LINE__, ##args)

#ifdef GFX_USE_VERBOSE_LOG
#define GFX_VERBOSE_LOG(format, args...)    printf("[GFX]%s(%d) "format, __FUNCTION__, __LINE__, ##args)
#else
#define GFX_VERBOSE_LOG(format, args...)
#endif

#endif

#if defined(GFX_USE_VERBOSE_LOG)
#define GFX_FUNC_ENTRY                      printf("%s() ENTER\n", __FUNCTION__, __LINE__)
#define GFX_FUNC_LEAVE                      printf("%s() LEAVE\n", __FUNCTION__, __LINE__)
#define GFX_HERE                            printf("%s()[%d]\n", __FUNCTION__, __LINE__)
#else
#define GFX_FUNC_ENTRY
#define GFX_FUNC_LEAVE
#define GFX_HERE
#endif
#endif  // __GFX_MSG_H__

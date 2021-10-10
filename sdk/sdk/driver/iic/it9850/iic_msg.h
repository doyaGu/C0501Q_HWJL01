#ifndef __IIC_MSG_H__
#define __IIC_MSG_H__

#include <stdio.h>

//#define IIC_USE_VERBOSE_LOG
#define IIC_USE_ERROR_LOG

#ifdef _WIN32

#ifdef IIC_USE_ERROR_LOG
#define IIC_ERROR_MSG(format, ...)		printf("[IIC]%s(%d)[ERROR]: "format, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define IIC_ERROR_MSG(format, args...)
#endif

#define IIC_LOG_MSG(format, ...)		printf("[IIC]%s(%d) "format, __FUNCTION__, __LINE__, __VA_ARGS__)

#ifdef IIC_USE_VERBOSE_LOG
#define IIC_VERBOSE_LOG(format, ...)	printf("[IIC]%s(%d) "format, __FUNCTION__, __LINE__, __VA_ARGS__args)
#else
#define IIC_VERBOSE_LOG(format, ...)
#endif

#define IIC_FUNC_ENTRY						printf("%s() ENTER\n", __FUNCTION__, __LINE__)
#define IIC_FUNC_LEAVE						printf("%s() LEAVE\n", __FUNCTION__, __LINE__)
#define IIC_HERE							printf("%s()[%d]\n", __FUNCTION__, __LINE__)

#else

#ifdef IIC_USE_ERROR_LOG
#define IIC_ERROR_MSG(format, args...)		printf("[IIC]%s(%d)[ERROR]: "format, __FUNCTION__, __LINE__, ##args)
#else
#define IIC_ERROR_MSG(format, args...)
#endif

#define IIC_LOG_MSG(format, args...)		printf("[IIC]%s(%d) "format, __FUNCTION__, __LINE__, ##args)

#ifdef IIC_USE_VERBOSE_LOG
#define IIC_VERBOSE_LOG(format, args...)	printf("[IIC]%s(%d) "format, __FUNCTION__, __LINE__, ##args)
#else
#define IIC_VERBOSE_LOG(format, args...)
#endif

#define IIC_FUNC_ENTRY						printf("%s() ENTER\n", __FUNCTION__, __LINE__)
#define IIC_FUNC_LEAVE						printf("%s() LEAVE\n", __FUNCTION__, __LINE__)
#define IIC_HERE							printf("%s()[%d]\n", __FUNCTION__, __LINE__)

#endif

#endif	// __IIC_MSG_H__

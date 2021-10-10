#ifndef __SPI_MSG_H__
#define __SPI_MSG_H__

#include <stdio.h>

//#define SPI_USE_VERBOSE_LOG
#define SPI_USE_ERROR_LOG

#ifdef _WIN32

#ifdef SPI_USE_ERROR_LOG
#define SPI_ERROR_MSG(format, ...)		printf("[SPI]%s(%d)[ERROR]: "format, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define SPI_ERROR_MSG(format, args...)
#endif

#define SPI_LOG_MSG(format, ...)		printf("[SPI]%s(%d) "format, __FUNCTION__, __LINE__, __VA_ARGS__)

#ifdef SPI_USE_VERBOSE_LOG
#define SPI_VERBOSE_LOG(format, ...)	printf("[SPI]%s(%d) "format, __FUNCTION__, __LINE__, __VA_ARGS__args)
#else
#define SPI_VERBOSE_LOG(format, ...)
#endif

#define SPI_FUNC_ENTRY						printf("%s() ENTER\n", __FUNCTION__, __LINE__)
#define SPI_FUNC_LEAVE						printf("%s() LEAVE\n", __FUNCTION__, __LINE__)
#define SPI_HERE							printf("%s()[%d]\n", __FUNCTION__, __LINE__)
#else

#ifdef SPI_USE_ERROR_LOG
#define SPI_ERROR_MSG(format, args...)		printf("[SPI]%s(%d)[ERROR]: "format, __FUNCTION__, __LINE__, ##args)
#else
#define SPI_ERROR_MSG(format, args...)
#endif

#define SPI_LOG_MSG(format, args...)		printf("[SPI]%s(%d) "format, __FUNCTION__, __LINE__, ##args)

#ifdef SPI_USE_VERBOSE_LOG
#define SPI_VERBOSE_LOG(format, args...)	printf("[SPI]%s(%d) "format, __FUNCTION__, __LINE__, ##args)
#else
#define SPI_VERBOSE_LOG(format, args...)
#endif

#define SPI_FUNC_ENTRY						printf("%s() ENTER\n", __FUNCTION__, __LINE__)
#define SPI_FUNC_LEAVE						printf("%s() LEAVE\n", __FUNCTION__, __LINE__)
#define SPI_HERE							printf("%s()[%d]\n", __FUNCTION__, __LINE__)

#endif

#endif	// __SPI_MSG_H__

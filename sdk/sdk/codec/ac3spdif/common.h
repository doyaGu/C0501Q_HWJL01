/*
 * common.h
 * Copyright (C) 2004-2007 Kuoping Hsu <kuoping@smediatech.com>
 */

#ifndef __COMMON_H__
#  define __COMMON_H__

#  define MAINBUF_SIZE    (640*6)  // 640 is the maximun bitrate.
#  define OUTPUT_CHANNELS 2
#  define OUTPUT_SAMPLES  (256*6)
#  define OUTPUT_BYTES    (OUTPUT_SAMPLES * OUTPUT_CHANNELS * sizeof(short))

typedef signed char             int8_t;
typedef short                   int16_t;
typedef long                    int32_t;
typedef unsigned char           uint8_t;
typedef unsigned short          uint16_t;
typedef unsigned long           uint32_t;

#  if defined(WIN32)
typedef __int64                 int64_t;
typedef unsigned __int64        uint64_t;
#  else
typedef long long               int64_t;
typedef unsigned long long      uint64_t;
#  endif

#endif /* __COMMON_H__ */


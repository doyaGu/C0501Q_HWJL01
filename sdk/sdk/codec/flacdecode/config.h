
//#define WIN32
#if defined (__OPENRTOS__)
#define __OR32__
#endif

#define ITE_RISC

#if defined(WIN32) || defined(__CYGWIN__)
    #include "win32.h"
#elif defined(__OR32__)
    #include "mmio.h"
#endif

#  if defined(WIN32)
    typedef __int64                 int64_t;
    typedef unsigned __int64        uint64_t;
    typedef unsigned int           size_t;    
    #define __inline
#  else
    typedef long long               int64_t;
    typedef unsigned long long      uint64_t;
#  endif

typedef signed char             int8_t;
typedef short                       int16_t;
typedef int                          int32_t;
typedef unsigned char           uint8_t;
typedef unsigned short          uint16_t;
typedef unsigned int           uint32_t;

#define av_cold
#ifndef INT_MAX
#define INT_MAX 0x7FFFFFFF
#endif
#define av_uninit(x) x

/** estimate for average size of a FLAC frame                                 */
#define FLAC_AVG_FRAME_SIZE 22000 // 8192

#define READBUF_BEGIN 0

/******************************
 The Buffer size of input stream
 ******************************/
#define READBUF_SIZE        (1024*32)
#define READBUF_LEN       (READBUF_SIZE - READBUF_BEGIN)

/******************************
 The Buffer size of output stream
 ******************************/
#define OUTBUF_SIZE        (192*1024)





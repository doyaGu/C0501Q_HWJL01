#ifndef __RSL_SLAVE__
#define __RSL_SLAVE__

#ifdef __cplusplus
extern "C" {
#endif

#include "alt_cpu/alt_cpu_device.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define MAX_CMD_DATA_BUFFER_SIZE            256
#define CMD_DATA_BUFFER_OFFSET              (16 * 1024 - MAX_CMD_DATA_BUFFER_SIZE)

#define INIT_CMD_ID                         1
#define READ_DATA_CMD_ID                    2
#define WRITE_DATA_CMD_ID                   3
#define READ_RAW_DATA_CMD_ID                4
#define WRITE_RAW_DATA_CMD_ID               5
#define SET_WRITE_COUNTER_CMD_ID            6

#define REQUEST_CMD_REG                     0x1698
#define RESPONSE_CMD_REG                    0x169A

#define ITP_IOCTL_ALT_CPU_READ_RAW_DATA         ITP_IOCTL_CUSTOM_CTL_ID1
#define ITP_IOCTL_ALT_CPU_WRITE_RAW_DATA        ITP_IOCTL_CUSTOM_CTL_ID2
#define ITP_IOCTL_ALT_CPU_SET_WRITE_COUNTER     ITP_IOCTL_CUSTOM_CTL_ID3

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct
{
    unsigned long cpuClock;
    unsigned long clkGpio;
    unsigned long dataGpio;
    unsigned long dataWriteGpio;
    unsigned long firstReadDelayUs;
    unsigned long readPeriod; //us
    unsigned long firstWriteDelayUs;
    unsigned long writePeriod; //us
} RSL_SLAVE_INIT_DATA;

typedef struct
{
    unsigned long bSuccess;
    unsigned char pReadBuffer[40];
} RSL_SLAVE_READ_DATA;

typedef struct
{
    unsigned char pWriteBuffer[40];
} RSL_SLAVE_WRITE_DATA;

typedef struct
{
    unsigned long bSuccess;
    unsigned char pReadBuffer[64];
} RSL_SLAVE_READ_RAW_DATA;

typedef struct
{
    unsigned char pWriteBuffer[64];
} RSL_SLAVE_WRITE_RAW_DATA;

typedef struct
{
    unsigned long writeCounter;
} RSL_SLAVE_WRITE_COUNTER;

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






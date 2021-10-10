#ifndef __RSL_MASTER__
#define __RSL_MASTER__

#ifdef __cplusplus
extern "C" {
#endif

#include "alt_cpu/alt_cpu_device.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define MAX_CMD_DATA_BUFFER_SIZE        256
#define CMD_DATA_BUFFER_OFFSET          (16 * 1024 - MAX_CMD_DATA_BUFFER_SIZE)

#define MAX_READ_BUFFER_SIZE            40
#define MAX_WRITE_BUFFER_SIZE           40

#define INIT_CMD_ID                     1
#define READ_DATA_CMD_ID                2
#define WRITE_DATA_CMD_ID               3
#define SEND_DATA_OUT_ID                4

#define REQUEST_CMD_REG                 0x1698
#define RESPONSE_CMD_REG                0x169A

#define ITP_IOCTL_ALT_CPU_SEND_DATA     ITP_IOCTL_CUSTOM_CTL_ID1


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
} RSL_MASTER_INIT_DATA;

typedef struct
{
    unsigned char pReadBuffer[40];
} RSL_MASTER_READ_DATA;

typedef struct
{
    unsigned char pWriteBuffer[40];
} RSL_MASTER_WRITE_DATA;

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






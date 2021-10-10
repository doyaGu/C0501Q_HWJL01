#ifndef __RSL_SLAVE__
#define __RSL_SLAVE__

#ifdef __cplusplus
extern "C" {
#endif

#include "alt_cpu/alt_cpu_device.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define MAX_CMD_DATA_BUFFER_SIZE    256
#define CMD_DATA_BUFFER_OFFSET      (16 * 1024 - MAX_CMD_DATA_BUFFER_SIZE)

#define INIT_CMD_ID                 1
#define RUN_CMD_ID                  2
#define STOP_CMD_ID                 3
#define UPDATE_DUTY_CYCLE_CMD_ID    4

#define REQUEST_CMD_REG             0x1698
#define RESPONSE_CMD_REG            0x169A

#define ITP_IOCTL_INIT_PWM_PARAM                    ITP_IOCTL_CUSTOM_CTL_ID1
#define ITP_IOCTL_SW_PWM_RUN                        ITP_IOCTL_CUSTOM_CTL_ID2
#define ITP_IOCTL_SW_PWM_STOP                       ITP_IOCTL_CUSTOM_CTL_ID3
#define ITP_IOCTL_SW_PWM_UPDATE_DUTY_CYCLE          ITP_IOCTL_CUSTOM_CTL_ID4

typedef enum
{
    SW_PWM0 = 0,
    SW_PWM1,
    SW_PWM2,
    SW_PWM3,
    SW_PWM_COUNT, //Must be the last entry;
} SW_PWM_ID;


//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct
{
    unsigned long pwmId;
    unsigned long cpuClock;
    unsigned long pwmGpio;
    unsigned long pwmClk;
    unsigned long pwmDutyCycle; //0 ~ 100
} SW_PWM_INIT_DATA;

typedef struct
{
    unsigned long pwmId;
} SW_PWM_RUN_CMD_DATA;

typedef struct
{
    unsigned long pwmId;
} SW_PWM_STOP_CMD_DATA;

typedef struct
{
    unsigned long pwmId;
    unsigned long pwmDutyCycle;
} SW_PWM_UPDATE_DUTY_CYCLE_DATA;

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






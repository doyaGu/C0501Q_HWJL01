#include <string.h>
#include "alt_cpu/swPwm/swPwm.h"

static uint8_t gpSwPwmImage[] =
{
    #include "swPwm.hex"
};


static void swPwmProcessCommand(int cmdId)
{
    int i = 0;
    ithWriteRegH(REQUEST_CMD_REG, cmdId);
    while(1)
    {
        if (ithReadRegH(RESPONSE_CMD_REG) != cmdId)
            continue;
        else
            break;
    }
    ithWriteRegH(REQUEST_CMD_REG, 0);
    for (i = 0; i < 1024; i++)
    {
        asm("");
    }
    ithWriteRegH(RESPONSE_CMD_REG, 0);
}

static int swPwmIoctl(int file, unsigned long request, void *ptr, void *info)
{
    uint8_t* pWriteAddress = (uint8_t*) (iteRiscGetTargetMemAddress(ALT_CPU_IMAGE_MEM_TARGET) + CMD_DATA_BUFFER_OFFSET);
    switch (request)
    {
        case ITP_IOCTL_INIT:
        {
            //Stop ALT CPU
            iteRiscResetCpu(ALT_CPU);

            //Clear Commuication Engine and command buffer
            memset(pWriteAddress, 0x0, MAX_CMD_DATA_BUFFER_SIZE);
            ithWriteRegH(REQUEST_CMD_REG, 0);
            ithWriteRegH(RESPONSE_CMD_REG, 0);

            //Load Engine First
            iteRiscLoadData(ALT_CPU_IMAGE_MEM_TARGET,gpSwPwmImage,sizeof(gpSwPwmImage));

            //Fire Alt CPU
            iteRiscFireCpu(ALT_CPU);
            break;
        }
        case ITP_IOCTL_INIT_PWM_PARAM:
        {
            SW_PWM_INIT_DATA* ptInitData = (SW_PWM_INIT_DATA*) ptr;
            memcpy(pWriteAddress, ptInitData, sizeof(SW_PWM_INIT_DATA));
            swPwmProcessCommand(INIT_CMD_ID);
            break;
        }
        case ITP_IOCTL_SW_PWM_RUN:
        {
            SW_PWM_RUN_CMD_DATA* ptRunCmd = (SW_PWM_RUN_CMD_DATA*) ptr;
            
            memcpy(pWriteAddress, ptRunCmd, sizeof(SW_PWM_RUN_CMD_DATA));
            swPwmProcessCommand(RUN_CMD_ID);
            break;
        }
        case ITP_IOCTL_SW_PWM_STOP:
        {
            SW_PWM_STOP_CMD_DATA* ptStopCmd = (SW_PWM_STOP_CMD_DATA*) ptr;
            
            memcpy(pWriteAddress, ptStopCmd, sizeof(SW_PWM_STOP_CMD_DATA));
            ithWriteRegH(REQUEST_CMD_REG, STOP_CMD_ID);
            swPwmProcessCommand(STOP_CMD_ID);
            break;
        }
        case ITP_IOCTL_SW_PWM_UPDATE_DUTY_CYCLE:
        {
            SW_PWM_UPDATE_DUTY_CYCLE_DATA* ptDutyCycleCmd = (SW_PWM_UPDATE_DUTY_CYCLE_DATA*) ptr;
            memcpy(pWriteAddress, ptDutyCycleCmd, sizeof(SW_PWM_UPDATE_DUTY_CYCLE_DATA));
            ithWriteRegH(REQUEST_CMD_REG, UPDATE_DUTY_CYCLE_CMD_ID);
            swPwmProcessCommand(UPDATE_DUTY_CYCLE_CMD_ID);
            break;
        }
        default:
            break;
    }
    return 0;
}

const ITPDevice itpDeviceSwPwm =
{
    ":swPwm",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    itpWriteDefault,
    itpLseekDefault,
    swPwmIoctl,
    NULL
};

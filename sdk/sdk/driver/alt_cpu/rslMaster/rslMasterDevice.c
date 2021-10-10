#include <string.h>
#include "alt_cpu/rslMaster/rslMaster.h"

static uint8_t gpRslMasterImage[] =
{
    #include "rslMaster.hex"
};

static void rslMasterProcessCommand(int cmdId)
{
    ithWriteRegH(REQUEST_CMD_REG, cmdId);
    while(1)
    {
        if (ithReadRegH(RESPONSE_CMD_REG) != cmdId)
            continue;
        else
            break;
    }
    ithWriteRegH(REQUEST_CMD_REG, 0);
    ithWriteRegH(RESPONSE_CMD_REG, 0);
}


static int rslMasterIoctl(int file, unsigned long request, void *ptr, void *info)
{
    uint8_t* pWriteAddress = (uint8_t*) (iteRiscGetTargetMemAddress(ALT_CPU_IMAGE_MEM_TARGET) + CMD_DATA_BUFFER_OFFSET);
    switch (request)
    {
        case ITP_IOCTL_INIT:
        {
            RSL_MASTER_INIT_DATA* ptInitData = (RSL_MASTER_INIT_DATA*) ptr;

            //Stop ALT CPU
            iteRiscResetCpu(ALT_CPU);

            //Clear Commuication Engine and command buffer
            memset(pWriteAddress, 0x0, MAX_CMD_DATA_BUFFER_SIZE);
            ithWriteRegH(REQUEST_CMD_REG, 0);
            ithWriteRegH(RESPONSE_CMD_REG, 0);

            //Load Engine First
            iteRiscLoadData(ALT_CPU_IMAGE_MEM_TARGET,gpRslMasterImage,sizeof(gpRslMasterImage));

            //Fir Alt CPU
            iteRiscFireCpu(ALT_CPU);

            //Send Init command
            memcpy(pWriteAddress, ptInitData, sizeof(RSL_MASTER_INIT_DATA));
            rslMasterProcessCommand(INIT_CMD_ID);
            break;
        }
        case ITP_IOCTL_ALT_CPU_SEND_DATA:
        {
            rslMasterProcessCommand(SEND_DATA_OUT_ID);
            break;
        }
        default:
            break;
    }
    return 0;
}

static int rslMasterRead(int file, char *ptr, int len, void* info)
{
    uint8_t* pWriteAddress = (uint8_t*) (iteRiscGetTargetMemAddress(ALT_CPU_IMAGE_MEM_TARGET) + CMD_DATA_BUFFER_OFFSET);
    int i = 0;
    
    memset(ptr, 0x0, 40);
    rslMasterProcessCommand(READ_DATA_CMD_ID);

    for (i = 0; i < 40; i++)
    {
        ptr[i] = pWriteAddress[i];
    }
    return 40;
}

static int rslMasterWrite(int file, char *ptr, int len, void* info)
{
    uint8_t* pWriteAddress = (uint8_t*) (iteRiscGetTargetMemAddress(ALT_CPU_IMAGE_MEM_TARGET) + CMD_DATA_BUFFER_OFFSET);
    int i = 0;

    for (i = 0; i < 40; i++)
    {
        pWriteAddress[i] = ptr[i];
    }
    rslMasterProcessCommand(WRITE_DATA_CMD_ID);
    return 40;
}

const ITPDevice itpDeviceRslMaster =
{
    ":rslMaster",
    itpOpenDefault,
    itpCloseDefault,
    rslMasterRead,
    rslMasterWrite,
    itpLseekDefault,
    rslMasterIoctl,
    NULL
};


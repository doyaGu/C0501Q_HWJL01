#include <string.h>
#include "alt_cpu/rslSlave/rslSlave.h"

static uint8_t gpRslSlaveImage[] =
{
    #include "rslSlave.hex"
};

static void rslSlaveProcessCommand(int cmdId)
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

static int rslSlaveIoctl(int file, unsigned long request, void *ptr, void *info)
{
    uint8_t* pWriteAddress = (uint8_t*) (iteRiscGetTargetMemAddress(ALT_CPU_IMAGE_MEM_TARGET) + CMD_DATA_BUFFER_OFFSET);

    switch (request)
    {
        case ITP_IOCTL_INIT:
        {
            RSL_SLAVE_INIT_DATA* ptInitData = (RSL_SLAVE_INIT_DATA*) ptr;

            //Stop ALT CPU
            iteRiscResetCpu(ALT_CPU);

            //Clear Commuication Engine and command buffer
            memset(pWriteAddress, 0x0, MAX_CMD_DATA_BUFFER_SIZE);
            ithWriteRegH(REQUEST_CMD_REG, 0);
            ithWriteRegH(RESPONSE_CMD_REG, 0);

            //Load Engine First
            iteRiscLoadData(ALT_CPU_IMAGE_MEM_TARGET,gpRslSlaveImage,sizeof(gpRslSlaveImage));

            //Fire Alt CPU
            iteRiscFireCpu(ALT_CPU);

            //Send Init command
            memcpy(pWriteAddress, ptInitData, sizeof(RSL_SLAVE_INIT_DATA));
            rslSlaveProcessCommand(INIT_CMD_ID);
            break;
        }
        case ITP_IOCTL_ALT_CPU_READ_RAW_DATA:
        {
            RSL_SLAVE_READ_RAW_DATA tReadRawData = { 0 };

            rslSlaveProcessCommand(READ_RAW_DATA_CMD_ID);
            memcpy(&tReadRawData, pWriteAddress, sizeof(RSL_SLAVE_READ_RAW_DATA));
            if (tReadRawData.bSuccess)
            {
                memcpy(ptr, tReadRawData.pReadBuffer, 64);
                return 64;
            }
            else
            {
                return 0;
            }
            break;
        }
        case ITP_IOCTL_ALT_CPU_WRITE_RAW_DATA:
        {
            int i = 0;
            uint8_t *pWriteRawData = (uint8_t*) ptr;

            for (i = 0; i < 64; i++)
            {
                pWriteAddress[i] = pWriteRawData[i];
            }
            rslSlaveProcessCommand(WRITE_RAW_DATA_CMD_ID);
            break;
        }
        case ITP_IOCTL_ALT_CPU_SET_WRITE_COUNTER:
        {
            memcpy(pWriteAddress, ptr, 4);
            rslSlaveProcessCommand(SET_WRITE_COUNTER_CMD_ID);
            break;
        }
        default:
            break;
    }
    return 0;
}

static int rslSlaveRead(int file, char *ptr, int len, void* info)
{
    uint8_t* pWriteAddress = (uint8_t*) (iteRiscGetTargetMemAddress(ALT_CPU_IMAGE_MEM_TARGET) + CMD_DATA_BUFFER_OFFSET);
    RSL_SLAVE_READ_DATA tSlaveData = { 0 };
    int i = 0;

    memset(tSlaveData.pReadBuffer, 0x0, 40);    

    rslSlaveProcessCommand(READ_DATA_CMD_ID);

    memcpy(&tSlaveData, pWriteAddress, sizeof(RSL_SLAVE_READ_DATA));
    if (tSlaveData.bSuccess)
    {
        memcpy(ptr, tSlaveData.pReadBuffer, 40);
        return 40;
    }
    else
    {
        return 0;
    }
}

static int rslSlaveWrite(int file, char *ptr, int len, void* info)
{
    uint8_t* pWriteAddress = (uint8_t*) (iteRiscGetTargetMemAddress(ALT_CPU_IMAGE_MEM_TARGET) + CMD_DATA_BUFFER_OFFSET);
    int i = 0;

    for (i = 0; i < 40; i++)
    {
        pWriteAddress[i] = ptr[i];
    }
    rslSlaveProcessCommand(WRITE_DATA_CMD_ID);
    return 40;
}

const ITPDevice itpDeviceRslSlave =
{
    ":rslSlave",
    itpOpenDefault,
    itpCloseDefault,
    rslSlaveRead,
    rslSlaveWrite,
    itpLseekDefault,
    rslSlaveIoctl,
    NULL
};

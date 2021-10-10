#include <stdarg.h>

#include "alt_cpu/rslMaster/rslMaster.h"

#define ENDIAN_SWAP16(x) \
        (((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8))

#define ENDIAN_SWAP32(x) \
        (((x & 0x000000FF) << 24) | \
        ((x & 0x0000FF00) <<  8) | \
        ((x & 0x00FF0000) >>  8) | \
        ((x & 0xFF000000) >> 24))

static char gRawWriteData[64] = { 0 };
static char gRawReadData[64] = { 0 };
static int gRawWriteIndex = 0;
static int gRawReadIndex = 0;

static uint32_t gCpuClock = 0;
static uint32_t gClkGpio = 0;
static uint32_t gDataGpio = 0;
static uint32_t gTickPerUs = 0;
static int gSendDataState = 0;
static uint32_t gSyncEndCheckTick = 0;

static void masterProcessInitCmd(void)
{
    RSL_MASTER_INIT_DATA* pInitData = (RSL_MASTER_INIT_DATA*) CMD_DATA_BUFFER_OFFSET;
    uint8_t* pAddr = (uint8_t*)pInitData;
    gClkGpio = ENDIAN_SWAP32(pInitData->clkGpio);
    gDataGpio = ENDIAN_SWAP32(pInitData->dataGpio);
    gCpuClock = ENDIAN_SWAP32(pInitData->cpuClock);

    gTickPerUs = gCpuClock / (1000 * 1000);
    setGpioDir(gClkGpio, 0);
    setGpioDir(gDataGpio, 0);
    setGpioMode(gClkGpio, 0);
    setGpioMode(gDataGpio, 0);
}

static void masterProcessWriteDataCmd(void)
{
    RSL_MASTER_WRITE_DATA* pWriteData = (RSL_MASTER_WRITE_DATA*) CMD_DATA_BUFFER_OFFSET;
    int rawWriteIndex = 0;
    int writeBufferIndex = 0;
    int i = 0;

    //Translate data to raw write data.
    for (i = 0; i < 64; i++)
    {
        uint8_t rawWriteData = 0;
        int bitOffset = rawWriteIndex & 0x7;
        if ((bitOffset + 5) > 7)
        {
            rawWriteData |= ((pWriteData->pWriteBuffer[writeBufferIndex] >> bitOffset) & 0xFF);
            rawWriteData |= ((pWriteData->pWriteBuffer[writeBufferIndex + 1] << (8 - bitOffset)) & 0xFF);
        }
        else
        {
            rawWriteData = ((pWriteData->pWriteBuffer[writeBufferIndex] >> bitOffset) & 0xFF);
        }
        rawWriteIndex += 5;
        writeBufferIndex = (rawWriteIndex >> 3);
        gRawWriteData[i] = ((rawWriteData << 2) & 0x7C);
    }
}

static void masterProcessReadDataCmd(void)
{
    RSL_MASTER_READ_DATA* pReadData = (RSL_MASTER_READ_DATA*) CMD_DATA_BUFFER_OFFSET;
    int readIndex = 0;
    int readBufferIndex = 0;
    int i = 0;

    //Clear command buffer first
    for (i = 0; i < 40; i++)
    {
        pReadData->pReadBuffer[i] = 0;
    }
    //Translate raw data to read data.
    for (i = 0; i < 64; i++)
    {
        uint8_t rawReadData = (gRawReadData[i] >> 2) & 0x1F;
        int bitOffset = (readIndex & 0x7);
        readBufferIndex = (readIndex >> 3);
        if ((bitOffset + 5) > 7)
        {
            pReadData->pReadBuffer[readBufferIndex] |= (rawReadData << bitOffset);
            pReadData->pReadBuffer[readBufferIndex + 1] |= (rawReadData >> 8 - bitOffset);
        }
        else
        {
            pReadData->pReadBuffer[readBufferIndex] |= (rawReadData << bitOffset);
        }
        readIndex += 5;
    }
}

static void masterProcessSendDataOutCmd(void)
{
    uint32_t nextCheckTime = 0;
    int i = 0, j = 0;
    int clkValue = 0;
    int dataValue = 0;
    int clockCount = 0;
    int bufferIndex = 0;

    if (gSendDataState == 1)
    {
        //1. Preparation
        gRawWriteIndex = 0;
        gRawReadIndex = 0;
        
        //Clear Read Data
        for (i = 0; i < 64; i++)
        {
            gRawReadData[i] = 0;
        }

        //Init Set both gpio 0
        setGpioValue(gClkGpio, clkValue);
        setGpioValue(gDataGpio, dataValue);

        //Start Timer
        startTimer(0);

        //2. Write Cycle
        //Set Data pin to output dir for later write.
        setGpioDir(gDataGpio, 0);
        //Genererate Clock and send data out
        for (clockCount = 0; clockCount < 64; clockCount++)
        {
            bufferIndex = (gRawWriteIndex >> 3);
            //Generate start signal
            setGpioValue(gClkGpio, 1);
            nextCheckTime += (gTickPerUs* 100);
            dataValue = gRawWriteData[bufferIndex];
            dataValue = ((dataValue >> (gRawWriteIndex & 0x7)) & 0x1);        
            setGpioValue(gDataGpio, dataValue);
            gRawWriteIndex++;
            while (getTimer(0) <= nextCheckTime);
            setGpioValue(gClkGpio, 0);

            for (j = 0; j < 7; j++)
            {
                dataValue = gRawWriteData[bufferIndex];
                dataValue = ((dataValue >> (gRawWriteIndex & 0x7)) & 0x1);
                gRawWriteIndex++;
                setGpioValue(gDataGpio, dataValue);
                nextCheckTime += (gTickPerUs* 100);
                while (getTimer(0) <= nextCheckTime);
            }
        }

        //3. Read Cycle
        //Set Data pin to input dir for later read.
        setGpioDir(gDataGpio, 1);
        //Genererate Clock and receive data
        for (clockCount = 0; clockCount < 64; clockCount++)
        {
            bufferIndex = (gRawReadIndex >> 3);
            nextCheckTime = getTimer(0);
            setGpioValue(gClkGpio, 1);    
            nextCheckTime += (gTickPerUs* 100);
            while (getTimer(0) <= nextCheckTime);
            setGpioValue(gClkGpio, 0);

            for (j = 0; j < 7; j++)
            {
                dataValue = getGpioValue(gDataGpio, 1);
                gRawReadData[bufferIndex] |= (dataValue << (gRawReadIndex & 0x7));
                gRawReadIndex++;
                nextCheckTime += (gTickPerUs* 100);
                while (getTimer(0) <= nextCheckTime);
            }
            dataValue = getGpioValue(gDataGpio, 1);
            gRawReadData[bufferIndex] |= (dataValue << (gRawReadIndex & 0x7));
            gRawReadIndex++;
        }
        gSyncEndCheckTick = getTimer(0) + 1600 * gTickPerUs;
        gSendDataState = 2;
        //stopTimer(0);
    }
    else if (gSendDataState == 2)
    {
        if (getTimer(0) >= gSyncEndCheckTick)
        {
            stopTimer(0);
            gSendDataState = 1;
        }
    }
}

int main(int argc, char **argv)
{
    //Set GPIO and Clock Setting
    int inputCmd = 0;

    while(1)
    {
        inputCmd = ithReadRegH(REQUEST_CMD_REG);
        if (inputCmd && ithReadRegH(RESPONSE_CMD_REG) == 0)
        {
            switch(inputCmd)
            {
                case INIT_CMD_ID:
                    masterProcessInitCmd();
                    break;
                case READ_DATA_CMD_ID:
                    masterProcessReadDataCmd();
                    break;
                case WRITE_DATA_CMD_ID:
                    masterProcessWriteDataCmd();
                    break;
                case SEND_DATA_OUT_ID:
                    gSendDataState = 1;
                    break;
                default:
                    break;
            }
            ithWriteRegH(RESPONSE_CMD_REG, (uint16_t) inputCmd);
        }
        masterProcessSendDataOutCmd();
    }
}

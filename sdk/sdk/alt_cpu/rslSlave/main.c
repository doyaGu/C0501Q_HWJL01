#include <stdarg.h>

#include "alt_cpu/rslSlave/rslSlave.h"

#define ENABLE_TIME_CHECK 0

#define ENDIAN_SWAP16(x) \
        (((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8))

#define ENDIAN_SWAP32(x) \
        (((x & 0x000000FF) << 24) | \
        ((x & 0x0000FF00) <<  8) | \
        ((x & 0x00FF0000) >>  8) | \
        ((x & 0xFF000000) >> 24))

#define MONITOR_STATE_NULL_STATE            0
#define MONITOR_STATE_WAITING_CLK           1
#define MONITOR_STATE_READ_DATA             2
#define MONITOR_STATE_WRITE_DATA            3

static char gRawWriteData[64] = { 0 };
static char gRawReadData[64] = { 0 };
static int gRawWriteIndex = 0;
static int gRawReadIndex = 0;

static char gReadData[40] = { 0 };
static char gSavedRawData[64] = { 0 };

static uint32_t gbValidData = 0;
static uint32_t gWriteCounter = 0;

static uint32_t gByPassClk = 0;

static uint32_t gCpuClock = 0;
static uint32_t gClkGpio = 0;
static uint32_t gDataGpio = 0;
static uint32_t gDataWriteGpio = 0;
static uint32_t gTickPerUs = 0;
static uint32_t gClkCount = 0;
static uint32_t gbStartMonitor = 0;
static uint32_t gFirstReadDelayUs = 0;
static uint32_t gFirstWriteDelayUs = 0;
static uint32_t gReadPeriod = 0;
static uint32_t gWritePeriod = 0;

static uint32_t gCurTime = 0;
static uint32_t gNextCheckTime = 0;
static uint32_t gPrevTime = 0;

static int gPreClkPinValue = 0;
static int gMointorState = MONITOR_STATE_NULL_STATE;

static void waitUs(int us)
{
    uint32_t leaveTime = getTimer(0) + (gTickPerUs * us);
    while (1)
    {
        uint32_t checkTime = getTimer(0);
        uint32_t remain = leaveTime - checkTime;
        if (gPrevTime == checkTime)
        {
            stopTimer(0);
            startTimer(0);
            leaveTime = remain;
        }
        if (checkTime >= leaveTime) break;
    
        gPrevTime = checkTime;
    }
}

static void slaveProcessInitCmd(void)
{
    RSL_SLAVE_INIT_DATA* pInitData = (RSL_SLAVE_INIT_DATA*) CMD_DATA_BUFFER_OFFSET;
    uint8_t* pAddr = (uint8_t*)pInitData;
    gClkGpio = ENDIAN_SWAP32(pInitData->clkGpio);
    gDataGpio = ENDIAN_SWAP32(pInitData->dataGpio);
    gDataWriteGpio = ENDIAN_SWAP32(pInitData->dataWriteGpio);
    gCpuClock = ENDIAN_SWAP32(pInitData->cpuClock);
    gFirstReadDelayUs = ENDIAN_SWAP32(pInitData->firstReadDelayUs);
    gFirstWriteDelayUs = ENDIAN_SWAP32(pInitData->firstWriteDelayUs);
    gReadPeriod = ENDIAN_SWAP32(pInitData->readPeriod);
    gWritePeriod = ENDIAN_SWAP32(pInitData->writePeriod);

    gTickPerUs = gCpuClock / (1000 * 1000);

    setGpioMode(gClkGpio, 0);
    setGpioMode(gDataGpio, 0);    
    setGpioMode(gDataWriteGpio, 0);
    setGpioDir(gClkGpio, 1);
    setGpioDir(gDataGpio, 1);
    setGpioDir(gDataWriteGpio, 1);

    gByPassClk = 70;
    setGpioMode(gByPassClk, 0);
    setGpioDir(gByPassClk, 0);

    gbStartMonitor = 1;
}

static void slaveProcessWriteDataCmd(void)
{
    RSL_SLAVE_WRITE_DATA* pWriteData = (RSL_SLAVE_WRITE_DATA*) CMD_DATA_BUFFER_OFFSET;
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
        gRawWriteData[i] = rawWriteData & 0x1F;
    }
}

static void slaveProcessReadDataCmd(void)
{
    RSL_SLAVE_READ_DATA* pReadData = (RSL_SLAVE_READ_DATA*) CMD_DATA_BUFFER_OFFSET;
    int readIndex = 0;
    int i = 0;

    if (gbValidData)
    {
        pReadData->bSuccess = ENDIAN_SWAP32(gbValidData);
        for (i = 0; i < 40; i++)
        {
            pReadData->pReadBuffer[i] = gReadData[i];
        }
        gbValidData = 0;
    }
    else
    {
        pReadData->bSuccess = 0;
        for (i = 0; i < 40; i++)
        {
            pReadData->pReadBuffer[i] = 0;
        }
    }
}

static void slaveProcessReadRawDataCmd(void)
{
    RSL_SLAVE_READ_RAW_DATA* pReadData = (RSL_SLAVE_READ_RAW_DATA*) CMD_DATA_BUFFER_OFFSET;
    int readIndex = 0;
    int i = 0;

    if (gbValidData)
    {
        pReadData->bSuccess = ENDIAN_SWAP32(gbValidData);
        for (i = 0; i < 64; i++)
        {
            pReadData->pReadBuffer[i] = gSavedRawData[i];
        }
        gbValidData = 0;
    }
    else
    {
        pReadData->bSuccess = 0;
        for (i = 0; i < 64; i++)
        {
            pReadData->pReadBuffer[i] = 0;
        }
    }
}


static void slaveRawReadDataToReadData(void)
{
    int readIndex = 0;
    int readBufferIndex = 0;
    int i = 0;

    //Clear command buffer first
    for (i = 0; i < 40; i++)
    {
        gReadData[i] = 0;
    }

    //Translate raw data to read data.
    for (i = 0; i < 64; i++)
    {
        uint8_t rawReadData = (gRawReadData[i] & 0x1F);
        int bitOffset = (readIndex & 0x7);
        readBufferIndex = (readIndex >> 3);
        if ((bitOffset + 5) > 7)
        {
            gReadData[readBufferIndex] |= (rawReadData << bitOffset);
            gReadData[readBufferIndex + 1] |= (rawReadData >> 8 - bitOffset);
        }
        else
        {
            gReadData[readBufferIndex] |= (rawReadData << bitOffset);
        }
        readIndex += 5;

        gSavedRawData[i] = gRawReadData[i];
    }
    gbValidData = 1;
}

static void slaveProcessWriteRawDataCmd(void)
{
    RSL_SLAVE_WRITE_RAW_DATA* pWriteRawData = (RSL_SLAVE_WRITE_RAW_DATA*) CMD_DATA_BUFFER_OFFSET;
    int i = 0;

    //Translate data to raw write data.
    for (i = 0; i < 64; i++)
    {
        gRawWriteData[i] = pWriteRawData->pWriteBuffer[i];
    }
}

static void slaveProcessSetWriteCounterCmd(void)
{
    RSL_SLAVE_WRITE_COUNTER* pWriteCounter = (RSL_SLAVE_WRITE_COUNTER*) CMD_DATA_BUFFER_OFFSET;
    gWriteCounter = ENDIAN_SWAP32(pWriteCounter->writeCounter);
}

static void slaveMonitorMasterInput(void)
{
    int i = 0;
    int clkValue = 0;
    int dataValue = 0;
    int clockCount = 0;
    int bufferIndex = 0;
    int clockValue = 0;

    if (!gbStartMonitor)
        return;


    switch(gMointorState)
    {
        case MONITOR_STATE_NULL_STATE:
            //1. Preparation
            gRawWriteIndex = 0;
            gRawReadIndex = 0;
            gPreClkPinValue = 0;
            gClkCount = 0;

            for (i = 0; i < 64; i++)
            {
                gRawReadData[i] = 0;
            }
            gCurTime = getTimer(0);
            gMointorState = MONITOR_STATE_READ_DATA;
            break;
        case MONITOR_STATE_READ_DATA:
        {
            clockValue = getGpioValue(gClkGpio,1);
            if (clockValue != gPreClkPinValue)
            {
                setGpioValue(gByPassClk, gPreClkPinValue);
            }

            if (clockValue != gPreClkPinValue && clockValue)
            {
                bufferIndex = (gRawReadIndex >> 3);
                if (gClkCount == 0)
                {
                    startTimer(0);
                }
                //Delay gFirstReadDelayUs us to start read data
                waitUs(gFirstReadDelayUs);
                for (i = 0; i < 4; i++)
                {
                    dataValue = getGpioValue(gDataGpio,1);
                    gRawReadData[bufferIndex] |= dataValue << (gRawReadIndex & 0x7);
                    gRawReadIndex++;
                    waitUs(gReadPeriod);
                }
                dataValue = getGpioValue(gDataGpio,1);

                gRawReadData[bufferIndex] |= dataValue << (gRawReadIndex & 0x7);
                gRawReadIndex += 4;
                gClkCount++;

                if (gClkCount == 64)
                {
                    gMointorState = MONITOR_STATE_WRITE_DATA;
                    if (ENABLE_TIME_CHECK)
                    {
                        gCurTime = getTimer(0);
                        ithWriteRegH(0x16E8, (gCurTime) & 0xFFFF);
                        ithWriteRegH(0x16EA, ((gCurTime) >> 16) & 0xFFFF);
                    }
                    gClkCount = 0;
                }
                
                gCurTime = getTimer(0);                
            }

            //checkTimeOut
            if ((getTimer(0) - gCurTime) > gTickPerUs * 1000)
            {
                gMointorState = MONITOR_STATE_NULL_STATE;
                stopTimer(0);
            }
            gPreClkPinValue = clockValue;
            break;
        }
        case MONITOR_STATE_WRITE_DATA:
        {
            clockValue = getGpioValue(gClkGpio,1);
            if (clockValue != gPreClkPinValue)
            {
                setGpioValue(gByPassClk, gPreClkPinValue);
            }
            if (clockValue != gPreClkPinValue && clockValue)
            {
                bufferIndex = (gRawWriteIndex >> 3);
                if (gClkCount == 0)
                {
                    if (gWriteCounter)
                    {
                        setGpioDir(gDataWriteGpio, 0);
                    }
                }

                //Delay gFirstWriteDelayUs us to write data
                waitUs(gFirstWriteDelayUs);
                for (i = 0; i < 5; i++)
                {
                    dataValue = ((gRawWriteData[bufferIndex] >> (gRawWriteIndex & 0x7)) & 0x1);
                    gRawWriteIndex++;
                    if (gWriteCounter)
                    {
                        setGpioValue(gDataWriteGpio,dataValue);
                        waitUs(gWritePeriod);
                        setGpioValue(gDataWriteGpio, 0);
                    }
                }
                gRawWriteIndex += 3;
                gClkCount++;

                if (gClkCount == 64)
                {
                    gMointorState = MONITOR_STATE_NULL_STATE;
                    if (ENABLE_TIME_CHECK)
                    {
                        uint32_t curTime  = getTimer(0);
                        ithWriteRegH(0x16EC, (curTime) & 0xFFFF);
                        ithWriteRegH(0x16EE, ((curTime) >> 16) & 0xFFFF);
                    }
                    slaveRawReadDataToReadData();

                    if (gWriteCounter)
                    {
                        --gWriteCounter;
                        if (gWriteCounter == 0)
                        {
                            for (i = 0; i < 64; i++)
                            {
                                gRawWriteData[i] = 0;
                            }
                        }
                    }
                    //After last bit is sent, then switch data pint to input mode again.
                    waitUs(gWritePeriod);
                    setGpioDir(gDataWriteGpio, 1);
                    stopTimer(0);
                    break;
                }
                gCurTime = getTimer(0);
            }

            //checkTimeOut
            if ((getTimer(0) - gCurTime) > gTickPerUs * 1000)
            {
                gMointorState = MONITOR_STATE_NULL_STATE;
                
                //In timeout case, switch data pint to input mode again.
                setGpioDir(gDataWriteGpio, 1);
                stopTimer(0);
            }
            gPreClkPinValue = clockValue;
            break;
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
                    slaveProcessInitCmd();
                    break;
                case READ_DATA_CMD_ID:
                    slaveProcessReadDataCmd();
                    break;
                case READ_RAW_DATA_CMD_ID:
                    slaveProcessReadRawDataCmd();
                    break;
                case WRITE_DATA_CMD_ID:
                    slaveProcessWriteDataCmd();
                    break;
                case WRITE_RAW_DATA_CMD_ID:
                    slaveProcessWriteRawDataCmd();
                    break;
                case SET_WRITE_COUNTER_CMD_ID:
                    slaveProcessSetWriteCounterCmd();
                    break;
                default:
                    break;
            }
            ithWriteRegH(RESPONSE_CMD_REG, (uint16_t) inputCmd);
        }
        
        slaveMonitorMasterInput();
    }
}

#include "ith_cfg.h"
#include <stdio.h>
#include "FTCSPI.h"

#define MAX_FREQ_93LC56B_CLOCK_DIVISOR 3   // equivalent to 1.5MHz

static CRITICAL_SECTION    CriticalSection;

static FTC_HANDLE ftHandle;
static FTC_CHIP_SELECT_PINS ChipSelectsDisableStates;
static FTC_INPUT_OUTPUT_PINS HighInputOutputPins;
static FTC_HIGHER_OUTPUT_PINS HighPinsWriteActiveStates;
static FTC_INIT_CONDITION ReadStartCondition;
static FTC_WAIT_DATA_WRITE WaitDataWriteComplete;
static FTC_INIT_CONDITION WriteStartCondition;

static uint8_t vramBuffer[CFG_RAM_SIZE] = {0xCC};
static uint8_t vramBufferCache[CFG_RAM_SIZE];

static int SpiClose(void)
{    
    if (ftHandle)
    {
        SPI_Close(ftHandle);
    }
    DeleteCriticalSection(&CriticalSection);
   
    return 0;
}

int SpiOpen(DWORD dwClockRate)
{
    FTC_STATUS Status = FTC_SUCCESS;
    DWORD dwNumDevices = 0;
    char szDeviceName[100];
    DWORD dwLocationID = 0;
    BOOL bPerformCommandSequence = false;
    DWORD dwClockFrequencyHz = 0;
    DWORD dwDividerValue;

    Status = SPI_GetNumDevices(&dwNumDevices);

    //Status = SPI_GetDllVersion(szDllVersion, 10);
    InitializeCriticalSection(&CriticalSection);

    if ((Status == FTC_SUCCESS) /*&& (dwNumDevices > 0)*/)
    {
        if(dwNumDevices == 0)
        {
              Status = SPI_OpenEx(szDeviceName, dwLocationID, &ftHandle);
        }
        else if (dwNumDevices == 1)
        {
            Status = SPI_GetDeviceNameLocID(0, szDeviceName, 100, &dwLocationID);

            if (Status == FTC_SUCCESS)
            {
                Status = SPI_OpenEx(szDeviceName, dwLocationID, &ftHandle);
                //Status = SPI_Open(&ftHandle);
            }
        }
        else
        {
            if (dwNumDevices == 2)
            {
                Status = SPI_GetDeviceNameLocID(1, szDeviceName, 100, &dwLocationID);

                if (Status == FTC_SUCCESS)
                {
                    Status = SPI_OpenEx(szDeviceName, dwLocationID, &ftHandle);
                    //Status = SPI_Open(&ftHandle);
                }
            }
        }
    }


    if ((Status == FTC_SUCCESS) && (dwNumDevices > 0))
    {
        if (Status == FTC_SUCCESS)
            Status = SPI_GetClock(MAX_FREQ_93LC56B_CLOCK_DIVISOR, &dwClockFrequencyHz);

        if (Status == FTC_SUCCESS)
        {
            //Status = SPI_InitDevice(ftHandle, MAX_FREQ_93LC56B_CLOCK_DIVISOR); //65535
            switch(dwClockRate)
            {
            case 6:
                dwDividerValue = 0;
                break;
            case 3:
                dwDividerValue = 1;
                break;
            case 2:
                dwDividerValue = 2;
                break;
            default:
                dwDividerValue = 2;
                break;
            }
        
            Status = SPI_InitDevice(ftHandle, dwDividerValue); // 12M / ((1+x) * 2)

            bPerformCommandSequence = true;               
            if (bPerformCommandSequence == true)
            {
                if (Status == FTC_SUCCESS)
                Status = SPI_ClearDeviceCmdSequence(ftHandle);
            }

            if (Status == FTC_SUCCESS)
            {
                  // Must set the chip select disable states for all the SPI devices connected to the FT2232C dual device
                  ChipSelectsDisableStates.bADBUS3ChipSelectPinState = false;
                  ChipSelectsDisableStates.bADBUS4GPIOL1PinState = false;
                  ChipSelectsDisableStates.bADBUS5GPIOL2PinState = false;
                  ChipSelectsDisableStates.bADBUS6GPIOL3PinState = false;
                  ChipSelectsDisableStates.bADBUS7GPIOL4PinState = false;

                  HighInputOutputPins.bPin1InputOutputState = true;
                  HighInputOutputPins.bPin1LowHighState = true;
                  HighInputOutputPins.bPin2InputOutputState = true;
                  HighInputOutputPins.bPin2LowHighState = true;
                  HighInputOutputPins.bPin3InputOutputState = true;
                  HighInputOutputPins.bPin3LowHighState = true;
                  HighInputOutputPins.bPin4InputOutputState = true;
                  HighInputOutputPins.bPin4LowHighState = true;

                  Status = SPI_SetGPIOs(ftHandle, &ChipSelectsDisableStates, &HighInputOutputPins);
            }

            if (Status == FTC_SUCCESS)
            {
                HighPinsWriteActiveStates.bPin1ActiveState = false;
                HighPinsWriteActiveStates.bPin1State = false;
                HighPinsWriteActiveStates.bPin2ActiveState = false;
                HighPinsWriteActiveStates.bPin2State = false;
                HighPinsWriteActiveStates.bPin3ActiveState = false;
                HighPinsWriteActiveStates.bPin3State = false;
                HighPinsWriteActiveStates.bPin4ActiveState = false;
                HighPinsWriteActiveStates.bPin4State = false;

                ReadStartCondition.bClockPinState = false;
                ReadStartCondition.bDataOutPinState = false;
                ReadStartCondition.bChipSelectPinState = true;
                ReadStartCondition.dwChipSelectPin = ADBUS3ChipSelect;

                WriteStartCondition.bClockPinState = false;
                WriteStartCondition.bDataOutPinState = false;
                WriteStartCondition.bChipSelectPinState = true;
                WriteStartCondition.dwChipSelectPin = ADBUS3ChipSelect;
            
                WaitDataWriteComplete.bWaitDataWriteComplete = false;
            }

        }
    }
    return 0;
}

static FTC_STATUS SpiRead(DWORD wrLen, PWriteControlByteBuffer pwrBuf, DWORD rdLen, PReadDataByteBuffer prdBuf)
{
    FTC_STATUS Status = FTC_SUCCESS;
    
    DWORD dwNumDataBytesReturned = 0;

    Status = SPI_Read(ftHandle, &ReadStartCondition, true, false, wrLen*8,
                      pwrBuf, wrLen, true, true, rdLen*8,
                      prdBuf, &dwNumDataBytesReturned,
                      &HighPinsWriteActiveStates);

    return(Status);
}

static ULONG SpiWrite(DWORD ctrlLen, PWriteControlByteBuffer pCtrlBuf, DWORD dataLen, PWriteDataByteBuffer pDataBuf)
{
    FTC_STATUS Status = FTC_SUCCESS;

    Status = SPI_Write(ftHandle, &WriteStartCondition, true, false,
                      ctrlLen*8, pCtrlBuf, ctrlLen,
                      true, dataLen*8, pDataBuf, dataLen, &WaitDataWriteComplete,
                      &HighPinsWriteActiveStates);
    
    return(Status);
}

#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9910) || (CFG_CHIP_FAMILY == 9850))
#define SECTION_SIZE        (0x10000)
#else
#define SECTION_SIZE        (0x10000-0x10)
#endif
#define MAX_PREREAD_COUNT   (0xF000)

#define MEM_ADDRESS_HI      0x206
#define	PREREAD_ADDRESS_LO  0x208
#define	PREREAD_ADDRESS_HI  0x20A
#define	PREREAD_LENGTH      0x20C
#define	PREREAD_FIRE        0x20E

static WriteControlByteBuffer wrBuf;
static ReadDataByteBuffer rdBuf;
static WriteDataByteBuffer wrDataBuf;

#ifdef CFG_DEV_TEST
DEFINE_COULD_BE_MOCKED_NOT_VOID_FUNC1(
uint16_t, ithReadRegH, uint16_t, addr)
#else
uint16_t ithReadRegH(uint16_t addr)
#endif
{
    uint16_t value;
    uint32_t error;
    uint32_t wrLen;
    uint32_t rdLen;

    EnterCriticalSection(&CriticalSection);

    addr /= 2;
    wrLen = 3;
    rdLen = 2;

    wrBuf[0] = 1;
    wrBuf[1] = (uint8_t)(addr & 0x00FF);
    wrBuf[2] = (uint8_t)((addr & 0xFF00) >> 8);

    error = SpiRead(wrLen, &wrBuf, rdLen, &rdBuf);
    if (error != 0)
    {
        printf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
        value = 0;
        printf("\nAck Error : 0x%x\n", rdBuf[0]);
    }
    else
    {
        value = (uint16_t)((rdBuf[1] << 8) + rdBuf[0]);
    }

    LeaveCriticalSection(&CriticalSection);

    return value;
}

#ifdef CFG_DEV_TEST
DEFINE_COULD_BE_MOCKED_VOID_FUNC2(
ithWriteRegH, uint16_t, addr, uint16_t, data)
#else
void ithWriteRegH(uint16_t addr, uint16_t data)
#endif
{
    uint32_t wrLen;
    uint32_t rdLen;
    uint32_t error;

    EnterCriticalSection(&CriticalSection);

    addr /= 2;
    wrLen = 3;
    rdLen = 2;

    wrBuf[0] = 0;
    wrBuf[1] = (uint8_t)(addr & 0x00FF);
    wrBuf[2] = (uint8_t)((addr & 0xFF00) >> 8);
    rdBuf[0] = (uint8_t)(data & 0x00FF);
    rdBuf[1] = (uint8_t)((data & 0xFF00) >> 8);

    error = SpiWrite(wrLen, &wrBuf, rdLen, &rdBuf);
  
    if ((addr == 0x08) && (data == 0x1000))
        goto end;
  
    if (error != 0)
    {
        printf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
        printf("\nAck Error : 0x%x\n", rdBuf[0]);
    }

end:
    LeaveCriticalSection(&CriticalSection);
}

#if CFG_CHIP_FAMILY != 9070 && CFG_CHIP_FAMILY != 9910 && CFG_CHIP_FAMILY != 9850

#define SPI_DELAY 0x7

uint32_t ithReadRegA(uint32_t addr)
{
    uint32_t value;
    uint32_t error;
    uint32_t wrLen = 5 + SPI_DELAY;
    uint32_t rdLen = 4;

    EnterCriticalSection(&CriticalSection);

	wrBuf[0]  = 0x05;
	wrBuf[0]  |= (SPI_DELAY << 4);
	wrBuf[1]  = (uint8_t)(addr & 0x000000FF);
	wrBuf[2]  = (uint8_t)((addr & 0x0000FF00) >> 8);
	wrBuf[3]  = (uint8_t)((addr & 0x00FF0000) >> 16);
	wrBuf[4]  = (uint8_t)((addr & 0xFF000000) >> 24);
	wrBuf[5]  = 0;
	wrBuf[6]  = 0;
	wrBuf[7]  = 0;
	wrBuf[8]  = 0;
	wrBuf[9]  = 0;
	wrBuf[10] = 0;
	wrBuf[11] = 0;
	wrBuf[12] = 0;

    error = SpiRead(wrLen, &wrBuf, rdLen, &rdBuf);
    if (error != 0)
    {
        printf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
        value = 0;
        printf("\nAck Error : 0x%x\n", rdBuf[0]);
    }
    else
    {
        value = (uint32_t)((rdBuf[3] << 24) + (rdBuf[2] << 16) +(rdBuf[1] << 8) + rdBuf[0]);
    }

    LeaveCriticalSection(&CriticalSection);

    return value;
}

void ithWriteRegA(uint32_t addr, uint32_t data)
{
    uint32_t wrLen = 5;
    uint32_t rdLen = 4;
    uint32_t error;

    EnterCriticalSection(&CriticalSection);

	wrBuf[0] = 4;
	wrBuf[1] = (uint8_t)(addr & 0x000000FF);
	wrBuf[2] = (uint8_t)((addr & 0x0000FF00) >> 8);
	wrBuf[3] = (uint8_t)((addr & 0x00FF0000) >> 16);
	wrBuf[4] = (uint8_t)((addr & 0xFF000000) >> 24);
	rdBuf[0] = (uint8_t)(data & 0x000000FF);
	rdBuf[1] = (uint8_t)((data & 0x0000FF00) >> 8);
	rdBuf[2] = (uint8_t)((data & 0x00FF0000) >> 16);
	rdBuf[3] = (uint8_t)((data & 0xFF000000) >> 24);

    error = SpiWrite(wrLen, &wrBuf, rdLen, &rdBuf);

    if ((addr == 0x08) && (data == 0x1000))
        goto end;
  
    if (error != 0)
    {
        printf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
        printf("\nAck Error : 0x%x\n", rdBuf[0]);
    }

end:
    LeaveCriticalSection(&CriticalSection);
}

uint8_t ithReadReg8(uint32_t addr)
{
    uint8_t value;
    uint32_t error;
    uint32_t wrLen = 5 + SPI_DELAY;
    uint32_t rdLen = 4;

    EnterCriticalSection(&CriticalSection);

	wrBuf[0]  = 0x05;
	wrBuf[0]  |= (SPI_DELAY << 4);
	wrBuf[1]  = (uint8_t)((addr & 0xfffffffc) & 0x000000FF);
	wrBuf[2]  = (uint8_t)(((addr & 0xfffffffc) & 0x0000FF00) >> 8);
	wrBuf[3]  = (uint8_t)(((addr & 0xfffffffc) & 0x00FF0000) >> 16);
	wrBuf[4]  = (uint8_t)(((addr & 0xfffffffc) & 0xFF000000) >> 24);
	wrBuf[5]  = 0;
	wrBuf[6]  = 0;
	wrBuf[7]  = 0;
	wrBuf[8]  = 0;
	wrBuf[9]  = 0;
	wrBuf[10] = 0;
	wrBuf[11] = 0;
	wrBuf[12] = 0;

    error = SpiRead(wrLen, &wrBuf, rdLen, &rdBuf);
    if (error != 0)
    {
        printf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
        value = 0;
        printf("\nAck Error : 0x%x\n", rdBuf[0]);
    }
    else
	{ 
		switch (addr & 3) {
			case 0: value = (uint8_t)rdBuf[0]; break;
			case 1: value = (uint8_t)rdBuf[1]; break;
			case 2: value = (uint8_t)rdBuf[2]; break;
			case 3: value = (uint8_t)rdBuf[3]; break;
		}
    }

    LeaveCriticalSection(&CriticalSection);

    return value;
}

void ithWriteReg8(uint32_t addr, uint8_t data)
{
    uint32_t wrLen = 5;
    uint32_t rdLen = 1;
    uint32_t error;

    EnterCriticalSection(&CriticalSection);

	wrBuf[0] = 4;
	wrBuf[1] = (uint8_t)(addr & 0x000000FF);
	wrBuf[2] = (uint8_t)((addr & 0x0000FF00) >> 8);
	wrBuf[3] = (uint8_t)((addr & 0x00FF0000) >> 16);
	wrBuf[4] = (uint8_t)((addr & 0xFF000000) >> 24);
	rdBuf[0] = (uint8_t)(data & 0x000000FF);
	rdBuf[1] = 0;
	rdBuf[2] = 0;
	rdBuf[3] = 0;

    error = SpiWrite(wrLen, &wrBuf, rdLen, &rdBuf);

    if ((addr == 0x08) && (data == 0x1000))
        goto end;
  
    if (error != 0)
    {
        printf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
        printf("\nAck Error : 0x%x\n", rdBuf[0]);
    }

end:
    LeaveCriticalSection(&CriticalSection);
}

uint16_t ithReadReg16(uint32_t addr)
{
    uint16_t value;
    uint32_t error;
    uint32_t wrLen = 5 + SPI_DELAY;
    uint32_t rdLen = 4;

    EnterCriticalSection(&CriticalSection);

	wrBuf[0]  = 0x05;
	wrBuf[0]  |= (SPI_DELAY << 4);
	wrBuf[1]  = (uint8_t)((addr & 0xfffffffc) & 0x000000FF);
	wrBuf[2]  = (uint8_t)(((addr & 0xfffffffc) & 0x0000FF00) >> 8);
	wrBuf[3]  = (uint8_t)(((addr & 0xfffffffc) & 0x00FF0000) >> 16);
	wrBuf[4]  = (uint8_t)(((addr & 0xfffffffc) & 0xFF000000) >> 24);
	wrBuf[5]  = 0;
	wrBuf[6]  = 0;
	wrBuf[7]  = 0;
	wrBuf[8]  = 0;
	wrBuf[9]  = 0;
	wrBuf[10] = 0;
	wrBuf[11] = 0;
	wrBuf[12] = 0;

    error = SpiRead(wrLen, &wrBuf, rdLen, &rdBuf);
    if (error != 0)
    {
        printf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
        value = 0;
        printf("\nAck Error : 0x%x\n", rdBuf[0]);
    }
    else
    {
		if (addr&2)
			value = (uint16_t)((rdBuf[3] << 8) + rdBuf[2]);
		else
			value = (uint16_t)((rdBuf[1] << 8) + rdBuf[0]);
    }

    LeaveCriticalSection(&CriticalSection);

    return value;
}

void ithWriteReg16(uint32_t addr, uint16_t data)
{
    uint32_t wrLen = 5;
    uint32_t rdLen = 2;
    uint32_t error;

    EnterCriticalSection(&CriticalSection);

	wrBuf[0] = 4;
	wrBuf[1] = (uint8_t)(addr & 0x000000FF);
	wrBuf[2] = (uint8_t)((addr & 0x0000FF00) >> 8);
	wrBuf[3] = (uint8_t)((addr & 0x00FF0000) >> 16);
	wrBuf[4] = (uint8_t)((addr & 0xFF000000) >> 24);
	rdBuf[0] = (uint8_t)(data & 0x000000FF);
	rdBuf[1] = (uint8_t)((data & 0x0000FF00) >> 8);
	rdBuf[2] = 0;
	rdBuf[3] = 0;

    error = SpiWrite(wrLen, &wrBuf, rdLen, &rdBuf);

    if ((addr == 0x08) && (data == 0x1000))
        goto end;
  
    if (error != 0)
    {
        printf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
        printf("\nAck Error : 0x%x\n", rdBuf[0]);
    }

end:
    LeaveCriticalSection(&CriticalSection);
}

#endif // CFG_CHIP_FAMILY != 9070 && CFG_CHIP_FAMILY != 9910 && CFG_CHIP_FAMILY != 9850

static void ReadMemory(uint32_t destAddress, uint32_t srcAddress, uint32_t sizeInByte)
{
    uint8_t* pdest = (uint8_t*)destAddress;
    uint32_t wrLen;
    uint32_t rdLen;
    uint32_t i;
    uint32_t error;

#if CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910 || CFG_CHIP_FAMILY == 9850

    unsigned int section = srcAddress / SECTION_SIZE;
    unsigned int preSize = 0;
    unsigned int data;
    uint32_t oddFlag = 0;

    EnterCriticalSection(&CriticalSection);

    while (sizeInByte)
    {
        preSize = (sizeInByte > MAX_PREREAD_COUNT)
                ? MAX_PREREAD_COUNT
                : sizeInByte;

        if (preSize & 1)
		{
            preSize += 1;
			oddFlag  = 1;
		}

        data = srcAddress % SECTION_SIZE;
        ithWriteRegH(PREREAD_ADDRESS_LO, data);

        data = srcAddress / SECTION_SIZE;
        ithWriteRegH(PREREAD_ADDRESS_HI, data);

		data = (preSize / 2) - 1;
		ithWriteRegH(PREREAD_LENGTH, data);
	
		data = 0x8007;
		ithWriteRegH(PREREAD_FIRE, data);

        wrLen = 3;
        rdLen = preSize;

        wrBuf[0] = 1;
        wrBuf[1] = (uint8_t)(0 & 0x00FF);
        wrBuf[2] = (uint8_t)((0 & 0xFF00) >> 8);
        wrBuf[2] |= 0x80;
    
        error = SpiRead(wrLen, &wrBuf, rdLen, &rdBuf);

        if (error != 0)
        {
            printf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
            printf("\nAck Error : 0x%x\n", rdBuf[0]);
        }

        if (preSize & 1)
		{
            preSize -=1;
			oddFlag = 0;
		}

        for (i = 0; i < preSize; i++)
        {
            *(pdest + i) = rdBuf[i];
        }

        sizeInByte -= preSize;
        srcAddress += preSize;
        pdest += preSize;
    }
    LeaveCriticalSection(&CriticalSection);

#else

    unsigned int preSize = 0;

    EnterCriticalSection(&CriticalSection);

    while (sizeInByte)
    {
        preSize = (sizeInByte > MAX_PREREAD_COUNT)
                ? MAX_PREREAD_COUNT
                : sizeInByte;

        wrLen = 9;
        rdLen = preSize;

        wrBuf[0] = 0x47;
        wrBuf[1] = (uint8_t)(srcAddress & 0x000000FF);
        wrBuf[2] = (uint8_t)((srcAddress & 0x0000FF00) >> 8);
        wrBuf[3] = (uint8_t)((srcAddress & 0x00FF0000) >> 16);
        wrBuf[4] = (uint8_t)((srcAddress & 0xFF000000) >> 24);
        wrBuf[5] = 0x00;
        wrBuf[6] = 0x00;
        wrBuf[7] = 0x00;
        wrBuf[8] = 0x00;
    
        error = SpiRead(wrLen, &wrBuf, rdLen, &rdBuf);

        if (error != 0)
        {
            printf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
            printf("\nAck Error : 0x%x\n", rdBuf[0]);
        }

        for (i = 0; i < preSize; i++)
        {
            *(pdest + i) = rdBuf[i];
        }

        sizeInByte -= preSize;
        srcAddress += preSize;
        pdest += preSize;
    }
    LeaveCriticalSection(&CriticalSection);

#endif // CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910 || CFG_CHIP_FAMILY == 9850
}

static void SecWriteMemory(uint16_t SecNum, uint16_t SecOffset, uint8_t *pDataBuffer, uint32_t DataLen)
{
    uint32_t ulRemainLen;
    uint16_t usSecOffset;
    uint16_t usTmpSecOffset;
    uint32_t usDataLen;
    uint32_t wrLen;
    uint32_t wrDataLen;
    uint32_t i;
    uint8_t *ptr;
    uint32_t error;

    if (DataLen > 0x10000)
    {
        printf("Top Half: Data length is too large\n");
        return;
    }

    ithWriteRegH(0x206, SecNum);

    ptr = pDataBuffer;
    
    ulRemainLen = DataLen;
    usSecOffset = SecOffset;
    usDataLen = (uint16_t)((ulRemainLen > (uint32_t)(0xFF00 - usSecOffset))?
                (uint32_t)(0xFF00 - usSecOffset) : ulRemainLen);
    
    if(usDataLen == 0)
    {
                    
        usDataLen = (ulRemainLen) ? ((uint16_t)ulRemainLen) : (0xFF00 - usSecOffset);
        if(usDataLen == 0)
            return;
    }

    wrLen = 3;
    wrDataLen = usDataLen;    

    for (i = 0; i < (usDataLen); i += 2)
    {
        wrDataBuf[i] = *ptr++;
        wrDataBuf[i + 1] = *ptr++;
    }

    wrBuf[0] = 0;
    usTmpSecOffset = (usSecOffset / 2);
      
    wrBuf[1] = (uint8_t)(usTmpSecOffset & 0x00FF);
    wrBuf[2] = (uint8_t)((usTmpSecOffset & 0xFF00) >> 8);
    wrBuf[2] |= 0x80;

    error = SpiWrite(wrLen, &wrBuf, wrDataLen, &wrDataBuf);
    if (error)
    {
        printf("\n%s:SpiWrite Error! Error: 0x%08x\n", __FUNCTION__, error);
    }

    ulRemainLen -= (uint32_t)usDataLen;

    if (ulRemainLen)
    {
        usDataLen = (uint16_t)ulRemainLen;    
        usSecOffset = 0xFF00;
        
        wrLen = 3;
        wrDataLen = usDataLen;

        if (usDataLen > 0x100)
        {
            printf("Buttom Half: Data Length is too large\n");
        }

        ithWriteRegH(0x206, SecNum);

        for (i = 0; i < (usDataLen); i += 2)
        {
            wrDataBuf[i] = *ptr++;
            wrDataBuf[i + 1] = *ptr++;
        }

        wrBuf[0] = 0;
        usTmpSecOffset = (usSecOffset / 2);
       
        wrBuf[1] = (uint8_t)(usTmpSecOffset & 0x00FF);
        wrBuf[2] = (uint8_t)((usTmpSecOffset & 0xFF00) >> 8);
        wrBuf[2] |= 0x80;

        error = SpiWrite(wrLen, &wrBuf, wrDataLen, &wrDataBuf);
        if (error)
        {
            printf("\n%s:SpiWrite Error! Error: 0x%08x\n", __FUNCTION__, error);
        }
    }
}

static void WriteMemory(uint32_t destAddress, uint32_t srcAddress, uint32_t sizeInByte)
{
#if CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910 || CFG_CHIP_FAMILY == 9850
    uint8_t *ptr;
    uint16_t usSecNum;
    uint16_t usSecOffset;
    uint32_t ulWriteLen;
    uint32_t ulRemainLen;

    EnterCriticalSection(&CriticalSection);

    sizeInByte  = ((sizeInByte + 3) >> 2) << 2;
    usSecNum    = (uint16_t)(destAddress / SECTION_SIZE);
    usSecOffset = (uint16_t)(destAddress % SECTION_SIZE);
    ulRemainLen = sizeInByte;
    ulWriteLen  = (ulRemainLen > (SECTION_SIZE - (uint32_t)usSecOffset))? (SECTION_SIZE - (uint32_t)usSecOffset) : ulRemainLen;
    ptr         = (uint8_t *)srcAddress;

    while(ulRemainLen > 0)
    {
        SecWriteMemory(usSecNum, usSecOffset, ptr, ulWriteLen);

        usSecNum++;
        usSecOffset = 0;
        ptr += ulWriteLen;
        ulRemainLen -= ulWriteLen;
        ulWriteLen = ((ulRemainLen > 0x10000)? 0x10000 : ulRemainLen);
    }
    LeaveCriticalSection(&CriticalSection);

#else

    unsigned int secSize = 0;

    EnterCriticalSection(&CriticalSection);

    while (sizeInByte)
    {
        uint32_t wrLen = 5;

        secSize = (sizeInByte > SECTION_SIZE)
                ? SECTION_SIZE
                : sizeInByte;

        wrBuf[0] = 4;
        wrBuf[1] = (uint8_t)(destAddress & 0x000000FF);
        wrBuf[2] = (uint8_t)((destAddress & 0x0000FF00) >> 8);
        wrBuf[3] = (uint8_t)((destAddress & 0x00FF0000) >> 16);
        wrBuf[4] = (uint8_t)((destAddress & 0xFF000000) >> 24);
	    wrBuf[5] = 0;
	    wrBuf[6] = 0;

        SpiWrite(wrLen, &wrBuf, secSize, (PWriteDataByteBuffer)srcAddress);

        sizeInByte -= secSize;
        srcAddress += secSize;
        destAddress += secSize;
    }
    LeaveCriticalSection(&CriticalSection);

#endif // CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910 || CFG_CHIP_FAMILY == 9850
}

void* ithMapVram(uint32_t vram_addr, uint32_t size, uint32_t flags)
{
    uint8_t* sys_addr;

    ASSERT(size > 0);
    ASSERT(vram_addr < CFG_RAM_SIZE && (vram_addr + size) <= CFG_RAM_SIZE);

    if (size == 0)
        return NULL;

    if (CFG_RAM_SIZE <= vram_addr || CFG_RAM_SIZE < (vram_addr + size))
        return NULL;

    if (flags & ITH_VRAM_READ)
        ReadMemory((uint32_t)&vramBuffer[vram_addr], vram_addr, size);
    memcpy(&vramBufferCache[vram_addr], &vramBuffer[vram_addr], size);
    sys_addr = &vramBufferCache[vram_addr];
    
    return sys_addr;
}

void ithUnmapVram(void* sys_addr, uint32_t size)
{
    uint32_t vram_addr;

    ASSERT(size > 0);
    ASSERT(((uint8_t*)vramBufferCache)   <= ((uint8_t*)sys_addr));
    ASSERT(((uint8_t*)sys_addr)          <= (((uint8_t*)vramBufferCache) + CFG_RAM_SIZE));
    ASSERT((((uint8_t*)sys_addr) + size) <= (((uint8_t*)vramBufferCache) + CFG_RAM_SIZE));

    vram_addr = (uint32_t)(((uint8_t*)sys_addr) - vramBufferCache);

    if (memcmp(&vramBuffer[vram_addr], &vramBufferCache[vram_addr], size) != 0)
    {
        memcpy(&vramBuffer[vram_addr], &vramBufferCache[vram_addr], size);
        WriteMemory(vram_addr, (uint32_t)&vramBuffer[vram_addr], size);
    }
}

void ithFlushDCacheRange(void* sys_addr, uint32_t size)
{
    ithUnmapVram(sys_addr, size);
}

uint32_t ithSysAddr2VramAddr(void* sys_addr)
{
    ASSERT(((uint8_t*)vramBufferCache)   <= ((uint8_t*)sys_addr));
    ASSERT(((uint8_t*)sys_addr)          <= (((uint8_t*)vramBufferCache) + CFG_RAM_SIZE));

    return (uint32_t)sys_addr - (uint32_t)vramBufferCache;
}

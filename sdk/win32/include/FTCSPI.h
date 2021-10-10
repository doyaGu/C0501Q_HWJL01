/*++

Copyright (c) 2005 Future Technology Devices International Ltd.

Module Name:

    ftcspi.h

Abstract:

    API DLL for FT2232C Dual Device setup to simulate the Serial Peripheral Interface(SPI) synchronous protocol.
    FTCSPI library definitions

Environment:

    kernel & user mode

Revision History:

    13/05/05    kra     Created.
	
--*/


#ifndef FTCSPI_H
#define FTCSPI_H

#include <windows.h>

//#define WINAPI
#ifndef false
#   define false FALSE
#endif
#ifndef true
#   define true TRUE
#endif

// The following ifdef block is the standard way of creating macros
// which make exporting from a DLL simpler.  All files within this DLL
// are compiled with the FTCSPI_EXPORTS symbol defined on the command line.
// This symbol should not be defined on any project that uses this DLL.
// This way any other project whose source files include this file see
// FTCSPI_API functions as being imported from a DLL, whereas this DLL
// sees symbols defined with this macro as being exported.

#ifdef FTCSPI_EXPORTS
#define FTCSPI_API __declspec(dllexport)
#else
#define FTCSPI_API __declspec(dllimport)
#endif

typedef DWORD FTC_HANDLE;
typedef ULONG FTC_STATUS;

#define ADBUS3ChipSelect 0
#define ADBUS4GPIOL1 1
#define ADBUS5GPIOL2 2
#define ADBUS6GPIOL3 3
#define ADBUS7GPIOL4 4

#define ADBUS2DataIn 0
#define ACBUS0GPIOH1 1
#define ACBUS1GPIOH2 2
#define ACBUS2GPIOH3 3
#define ACBUS3GPIOH4 4

#define FTC_SUCCESS 0 // FTC_OK
#define FTC_INVALID_HANDLE 1 // FTC_INVALID_HANDLE
#define FTC_DEVICE_NOT_FOUND 2 //FTC_DEVICE_NOT_FOUND
#define FTC_DEVICE_NOT_OPENED 3 //FTC_DEVICE_NOT_OPENED
#define FTC_IO_ERROR 4 //FTC_IO_ERROR
#define FTC_INSUFFICIENT_RESOURCES 5 // FTC_INSUFFICIENT_RESOURCES

#define FTC_FAILED_TO_COMPLETE_COMMAND 20          // cannot change, error code mapped from FT2232c classes
#define FTC_FAILED_TO_SYNCHRONIZE_DEVICE_MPSSE 21  // cannot change, error code mapped from FT2232c classes
#define FTC_INVALID_DEVICE_NAME_INDEX 22           // cannot change, error code mapped from FT2232c classes
#define FTC_NULL_DEVICE_NAME_BUFFER_POINTER 23     // cannot change, error code mapped from FT2232c classes 
#define FTC_DEVICE_NAME_BUFFER_TOO_SMALL 24        // cannot change, error code mapped from FT2232c classes
#define FTC_INVALID_DEVICE_NAME 25                 // cannot change, error code mapped from FT2232c classes
#define FTC_INVALID_LOCATION_ID 26                 // cannot change, error code mapped from FT2232c classes
#define FTC_DEVICE_IN_USE 27                       // cannot change, error code mapped from FT2232c classes
#define FTC_TOO_MANY_DEVICES 28                    // cannot change, error code mapped from FT2232c classes
#define FTC_INVALID_CLOCK_DIVISOR 29
#define FTC_NULL_INPUT_BUFFER_POINTER 30
#define FTC_NULL_CHIP_SELECT_BUFFER_POINTER 31
#define FTC_NULL_INPUT_OUTPUT_BUFFER_POINTER 32
#define FTC_NULL_OUTPUT_PINS_BUFFER_POINTER 33
#define FTC_NULL_INITIAL_CONDITION_BUFFER_POINTER 34
#define FTC_NULL_WRITE_CONTROL_BUFFER_POINTER 35
#define FTC_NULL_WRITE_DATA_BUFFER_POINTER 36
#define FTC_NULL_WAIT_DATA_WRITE_BUFFER_POINTER 37
#define FTC_NULL_READ_DATA_BUFFER_POINTER 38
#define FTC_NULL_READ_CMDS_DATA_BUFFER_POINTER 39
#define FTC_INVALID_NUMBER_CONTROL_BITS 40
#define FTC_INVALID_NUMBER_CONTROL_BYTES 41
#define FTC_NUMBER_CONTROL_BYTES_TOO_SMALL 42
#define FTC_INVALID_NUMBER_WRITE_DATA_BITS 43
#define FTC_INVALID_NUMBER_WRITE_DATA_BYTES 44
#define FTC_NUMBER_WRITE_DATA_BYTES_TOO_SMALL 45
#define FTC_INVALID_NUMBER_READ_DATA_BITS 46
#define FTC_INVALID_INIT_CLOCK_PIN_STATE 47
#define FTC_INVALID_FT2232C_CHIP_SELECT_PIN 48
#define FTC_INVALID_FT2232C_DATA_WRITE_COMPLETE_PIN 49
#define FTC_DATA_WRITE_COMPLETE_TIMEOUT 50
#define FTC_INVALID_CONFIGURATION_HIGHER_GPIO_PIN 51
#define FTC_COMMAND_SEQUENCE_BUFFER_FULL 52
#define FTC_NO_COMMAND_SEQUENCE 53
#define FTC_NULL_DLL_VERSION_BUFFER_POINTER 54
#define FTC_DLL_VERSION_BUFFER_TOO_SMALL 55
#define FTC_NULL_LANGUAGE_CODE_BUFFER_POINTER 56
#define FTC_NULL_ERROR_MESSAGE_BUFFER_POINTER 57
#define FTC_ERROR_MESSAGE_BUFFER_TOO_SMALL 58
#define FTC_INVALID_LANGUAGE_CODE 59
#define FTC_INVALID_STATUS_CODE 60

#ifdef __cplusplus
extern "C" {
#endif

FTCSPI_API
FTC_STATUS WINAPI SPI_GetNumDevices(LPDWORD lpdwNumDevices);

FTCSPI_API
FTC_STATUS WINAPI SPI_GetDeviceNameLocID(DWORD dwDeviceNameIndex, LPSTR lpDeviceNameBuffer, DWORD dwBufferSize, LPDWORD lpdwLocationID);

FTCSPI_API
FTC_STATUS WINAPI SPI_OpenEx(LPSTR lpDeviceName, DWORD dwLocationID, FTC_HANDLE *pftHandle);

FTCSPI_API
FTC_STATUS WINAPI SPI_Open(FTC_HANDLE *pftHandle);

FTCSPI_API
FTC_STATUS WINAPI SPI_Close(FTC_HANDLE ftHandle);

FTCSPI_API
FTC_STATUS WINAPI SPI_InitDevice(FTC_HANDLE ftHandle, DWORD dwClockDivisor);

FTCSPI_API
FTC_STATUS WINAPI SPI_GetClock(DWORD dwClockDivisor, LPDWORD lpdwClockFrequencyHz);

FTCSPI_API
FTC_STATUS WINAPI SPI_SetClock(FTC_HANDLE ftHandle, DWORD dwClockDivisor, LPDWORD lpdwClockFrequencyHz);

FTCSPI_API
FTC_STATUS WINAPI SPI_SetLoopback(FTC_HANDLE ftHandle, BOOL bLoopbackState);

typedef struct Ft_Chip_Select_Pins{
  BOOL  bADBUS3ChipSelectPinState;
  BOOL  bADBUS4GPIOL1PinState;
  BOOL  bADBUS5GPIOL2PinState;
  BOOL  bADBUS6GPIOL3PinState;
  BOOL  bADBUS7GPIOL4PinState;
}FTC_CHIP_SELECT_PINS, *PFTC_CHIP_SELECT_PINS;

typedef struct Ft_Input_Output_Pins{
  BOOL  bPin1InputOutputState;
  BOOL  bPin1LowHighState;
  BOOL  bPin2InputOutputState;
  BOOL  bPin2LowHighState;
  BOOL  bPin3InputOutputState;
  BOOL  bPin3LowHighState;
  BOOL  bPin4InputOutputState;
  BOOL  bPin4LowHighState;
}FTC_INPUT_OUTPUT_PINS, *PFTC_INPUT_OUTPUT_PINS;

FTCSPI_API
FTC_STATUS WINAPI SPI_SetGPIOs(FTC_HANDLE ftHandle, PFTC_CHIP_SELECT_PINS pChipSelectsDisableStates,
                               PFTC_INPUT_OUTPUT_PINS pHighInputOutputPinsData);

typedef struct Ft_Low_High_Pins{
  BOOL  bPin1LowHighState;
  BOOL  bPin2LowHighState;
  BOOL  bPin3LowHighState;
  BOOL  bPin4LowHighState;
}FTC_LOW_HIGH_PINS, *PFTC_LOW_HIGH_PINS;

FTCSPI_API
FTC_STATUS WINAPI SPI_GetGPIOs(FTC_HANDLE ftHandle, PFTC_LOW_HIGH_PINS pHighPinsInputData);

#define MAX_WRITE_CONTROL_BYTES_BUFFER_SIZE 256    // 256 bytes

typedef BYTE WriteControlByteBuffer[MAX_WRITE_CONTROL_BYTES_BUFFER_SIZE];
typedef WriteControlByteBuffer *PWriteControlByteBuffer;

#define MAX_WRITE_DATA_BYTES_BUFFER_SIZE 65536    // 64k bytes

typedef BYTE WriteDataByteBuffer[MAX_WRITE_DATA_BYTES_BUFFER_SIZE];
typedef WriteDataByteBuffer *PWriteDataByteBuffer;

typedef struct Ft_Init_Condition{
  BOOL  bClockPinState;
  BOOL  bDataOutPinState;
  BOOL  bChipSelectPinState;
  DWORD dwChipSelectPin;
}FTC_INIT_CONDITION, *PFTC_INIT_CONDITION;

typedef struct Ft_Wait_Data_Write{
  BOOL  bWaitDataWriteComplete;
  DWORD dwWaitDataWritePin;
  BOOL  bDataWriteCompleteState;
  DWORD dwDataWriteTimeoutmSecs;
}FTC_WAIT_DATA_WRITE, *PFTC_WAIT_DATA_WRITE;

typedef struct Ft_Higher_Output_Pins{
  BOOL  bPin1State;
  BOOL  bPin1ActiveState;
  BOOL  bPin2State;
  BOOL  bPin2ActiveState;
  BOOL  bPin3State;
  BOOL  bPin3ActiveState;
  BOOL  bPin4State;
  BOOL  bPin4ActiveState;
}FTC_HIGHER_OUTPUT_PINS, *PFTC_HIGHER_OUTPUT_PINS;

FTCSPI_API
FTC_STATUS WINAPI SPI_Write(FTC_HANDLE ftHandle, PFTC_INIT_CONDITION pWriteStartCondition, BOOL bClockOutDataBitsMSBFirst,
                            BOOL bClockOutDataBitsPosEdge, DWORD dwNumControlBitsToWrite, PWriteControlByteBuffer pWriteControlBuffer,
                            DWORD dwNumControlBytesToWrite, BOOL bWriteDataBits, DWORD dwNumDataBitsToWrite, PWriteDataByteBuffer pWriteDataBuffer,
                            DWORD dwNumDataBytesToWrite, PFTC_WAIT_DATA_WRITE pWaitDataWriteComplete, PFTC_HIGHER_OUTPUT_PINS pHighPinsWriteActiveStates);

#define MAX_READ_DATA_BYTES_BUFFER_SIZE 65536    // 64k bytes

typedef BYTE ReadDataByteBuffer[MAX_READ_DATA_BYTES_BUFFER_SIZE];
typedef ReadDataByteBuffer *PReadDataByteBuffer;

FTCSPI_API
FTC_STATUS WINAPI SPI_Read(FTC_HANDLE ftHandle, PFTC_INIT_CONDITION pReadStartCondition, BOOL bClockOutControlBitsMSBFirst,
                           BOOL bClockOutControlBitsPosEdge, DWORD dwNumControlBitsToWrite, PWriteControlByteBuffer pWriteControlBuffer,
                           DWORD dwNumControlBytesToWrite, BOOL bClockInDataBitsMSBFirst, BOOL bClockInDataBitsPosEdge,
                           DWORD dwNumDataBitsToRead, PReadDataByteBuffer pReadDataBuffer, LPDWORD lpdwNumDataBytesReturned,
                           PFTC_HIGHER_OUTPUT_PINS pHighPinsReadActiveStates);

FTCSPI_API
FTC_STATUS WINAPI SPI_ClearDeviceCmdSequence(FTC_HANDLE ftHandle);

FTCSPI_API
FTC_STATUS WINAPI SPI_AddDeviceWriteCmd(FTC_HANDLE ftHandle, PFTC_INIT_CONDITION pWriteStartCondition, BOOL bClockOutDataBitsMSBFirst,
                                        BOOL bClockOutDataBitsPosEdge, DWORD dwNumControlBitsToWrite, PWriteControlByteBuffer pWriteControlBuffer,
                                        DWORD dwNumControlBytesToWrite, BOOL bWriteDataBits, DWORD dwNumDataBitsToWrite,
                                        PWriteDataByteBuffer pWriteDataBuffer, DWORD dwNumDataBytesToWrite,
                                        PFTC_HIGHER_OUTPUT_PINS pHighPinsWriteActiveStates);

FTCSPI_API
FTC_STATUS WINAPI SPI_AddDeviceReadCmd(FTC_HANDLE ftHandle, PFTC_INIT_CONDITION pReadStartCondition, BOOL bClockOutControlBitsMSBFirst,
                                       BOOL bClockOutControlBitsPosEdge, DWORD dwNumControlBitsToWrite, PWriteControlByteBuffer pWriteControlBuffer,
                                       DWORD dwNumControlBytesToWrite, BOOL bClockInDataBitsMSBFirst, BOOL bClockInDataBitsPosEdge,
                                       DWORD dwNumDataBitsToRead, PFTC_HIGHER_OUTPUT_PINS pHighPinsReadActiveStates);

#define MAX_READ_CMDS_DATA_BYTES_BUFFER_SIZE 131071  // 128K bytes 

typedef BYTE ReadCmdSequenceDataByteBuffer[MAX_READ_CMDS_DATA_BYTES_BUFFER_SIZE];
typedef ReadCmdSequenceDataByteBuffer *PReadCmdSequenceDataByteBuffer;

FTCSPI_API
FTC_STATUS WINAPI SPI_ExecuteDeviceCmdSequence(FTC_HANDLE ftHandle, PReadCmdSequenceDataByteBuffer pReadCmdSequenceDataBuffer,
                                               LPDWORD lpdwNumBytesReturned);

FTCSPI_API
FTC_STATUS WINAPI SPI_GetDllVersion(LPSTR lpDllVersionBuffer, DWORD dwBufferSize);

FTCSPI_API
FTC_STATUS WINAPI SPI_GetErrorCodeString(LPSTR lpLanguage, FTC_STATUS StatusCode,
                                         LPSTR lpErrorMessageBuffer, DWORD dwBufferSize);


#ifdef __cplusplus
}
#endif


#endif  /* FTCSPI_H */
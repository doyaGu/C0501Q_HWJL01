#ifndef __EN_TYPE_H__
#define __EN_TYPE_H__

#include <stdlib.h>
#include <stdio.h>

#ifndef NULL
    #define NULL    0
#endif

#define IN
#define OUT
#define INOUT

/**
 * The type of handle.
 */
typedef void* Handle;


/**
 * The type defination of 8-bits unsigned type.
 */
typedef unsigned char Byte;


/**
 * The type defination of 16-bits unsigned type.
 */
typedef unsigned short Word;


/**
 * The type defination of 32-bits unsigned type.
 */
typedef unsigned long Dword;


/**
 * The type defination of 16-bits signed type.
 */
typedef short Short;


/**
 * The type defination of 32-bits signed type.
 */
typedef long Long;


/**
 * The type defination of Bool
 */
typedef enum {
    False = 0,
    True = 1
} Bool;

/**
 * The type defination of TS_in.
 */
typedef enum {
    CPU_LINK,
    CPU_OFDM = 8,
} BrProcessor;

/**
 * The type defination of abstract structure for Endeavour.
 */
typedef struct {
    Handle userData;
    Handle driver;
} Bridge;


/**
 * The type defination of BrSegment
 */
typedef struct {
    Byte    segmentType;           /** 0:Firmware download 1:Rom copy 2:Direct command */
    Dword   segmentLength;
} BrSegment;


/**
 * The type defination of BrValueSet.
 */
typedef struct {
    Dword   address;      /** The address of target register */
    Byte    value;         /** The value of target register   */
} BrValueSet;


/**
 * The type defination of StreamType.
 */
typedef enum {
    TS_PARALLEL = 0,
    TS_SERIAL
} TsType;

/**
 * The type defination of StreamFormat.
 */
typedef enum{    
    OUT_DATA_TS_PARALLEL = 0,       /** DVB-T mode, output via paralle interface */
    OUT_DATA_TS_SERIAL,         /** DVB-T mode, output via serial interface  */
    OUT_DATA_USB_DATAGRAM,
    OUT_DATA_I2S,
    OUT_DATA_601_640x480,
    OUT_DATA_601_720x576,
    OUT_DATA_601_320x240,
} OutDataType;

#if 0
/**
 * General modulator register-write function
 *
 * @param modulator the handle of modulator.
 * @param registerAddress address of register to be written.
 * @param bufferLength number, 1-8, of registers to be written.
 * @param buffer buffer used to store values to be written to specified
 *        registers.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
typedef Dword (*BrWriteRegisters) (
    IN  Bridge*         bridge,
    IN  Byte            chip,
    IN  BrProcessor     processor,
    IN  Dword           registerAddress,
    IN  Byte            registerAddressLength,
    IN  Dword           writeBufferLength,
    IN  Byte*           writeBuffer
);


/**
 * General tuner register-write function
 *
 * @param modulator the handle of modulator.
 * @param registerAddress address of register to be written.
 * @param bufferLength number, 1-8, of registers to be written.
 * @param buffer buffer used to store values to be written to specified
 *        registers.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
typedef Dword (*BrWriteTunerRegisters) (
    IN  Bridge*         bridge,
    IN  Byte            chip,
    IN  Byte            tunerAddress,
    IN  Word            registerAddress,
    IN  Byte            registerAddressLength,
    IN  Byte            writeBufferLength,
    IN  Byte*           writeBuffer
);


/**
 * General write EEPROM function
 *
 * @param modulator the handle of modulator.
 * @param registerAddress address of register to be read.
 * @param bufferLength number, 1-8, of registers to be written.
 * @param buffer buffer used to store values to be written to specified
 *        registers.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
typedef Dword (*BrWriteEepromValues) (
    IN  Bridge*         bridge,
    IN  Byte            chip,
    IN  Byte            eepromAddress,
    IN  Word            registerAddress,
    IN  Byte            registerAddressLength,
    IN  Byte            writeBufferLength,
    IN  Byte*           writeBuffer
);


/**
 * General modulator register-read function
 *
 * @param modulator the handle of modulator.
 * @param registerAddress address of register to be read.
 * @param bufferLength number, 1-8, of registers to be read.
 * @param buffer buffer used to store values to be read to specified
 *        registers.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
typedef Dword (*BrReadRegisters) (
    IN  Bridge*         bridge,
    IN  Byte            chip,
    IN  BrProcessor     processor,
    IN  Dword           registerAddress,
    IN  Byte            registerAddressLength,
    IN  Dword           readBufferLength,
    OUT Byte*           readBuffer
);


/**
 * General tuner register-read function
 *
 * @param modulator the handle of modulator.
 * @param registerAddress address of register to be read.
 * @param bufferLength number, 1-8, of registers to be read.
 * @param buffer buffer used to store values to be read to specified
 *        registers.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
typedef Dword (*BrReadTunerRegisters) (
    IN  Bridge*         bridge,
    IN  Byte            chip,
    IN  Byte            tunerAddress,
    IN  Word            registerAddress,
    IN  Byte            registerAddressLength,
    IN  Byte            readBufferLength,
    IN  Byte*           readBuffer
);


/**
 * General read EEPROM function
 *
 * @param modulator the handle of modulator.
 * @param registerAddress address of register to be read.
 * @param bufferLength number, 1-8, of registers to be read.
 * @param buffer buffer used to store values to be read to specified
 *        registers.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
typedef Dword (*BrReadEepromValues) (
    IN  Bridge*         bridge,
    IN  Byte            chip,
    IN  Byte            eepromAddress,
    IN  Word            registerAddress,
    IN  Byte            registerAddressLength,
    IN  Byte            readBufferLength,
    OUT Byte*           readBuffer
);


/**
 * General modulator register-read function
 *
 * @param modulator the handle of modulator.
 * @param registerAddress address of register to be read.
 * @param bufferLength number, 1-8, of registers to be read.
 * @param buffer buffer used to store values to be read to specified
 *        registers.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
typedef Dword (*BrModifyRegister) (
    IN  Bridge*         bridge,
    IN  Byte            chip,
    IN  BrProcessor     processor,
    IN  Dword           registerAddress,
    IN  Byte            registerAddressLength,
    IN  Byte            position,
    IN  Byte            length,
    IN  Byte            value
);


/**
 * General load firmware function
 *
 * @param modulator the handle of modulator.
 * @param length The length of firmware.
 * @param firmware The byte array of firmware.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
typedef Dword (*BrLoadFirmware) (
    IN  Bridge*         bridge,
    IN  Byte            chip,
    IN  Dword           firmwareLength,
    IN  Byte*           firmware
);


/**
 * General reboot function
 *
 * @param modulator the handle of modulator.
 * @param length The length of firmware.
 * @param firmware The byte array of firmware.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
typedef Dword (*BrReboot) (
    IN  Bridge*         bridge,
    IN  Byte            chip
);


/**
 * Find and Get bus handle used to control bus
 *
 * @param modulator the handle of modulator.
 * @param handle The bus handle.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
typedef Dword (*BrGetBus) (
    IN  Bridge*         bridge,
    OUT Handle*         handle
);


/**
 * Find and Get bus handle used to control bus
 *
 * @param modulator the handle of modulator.
 * @param bufferLength The length to transmit.
 * @param buffer The buffer which we store the data to send.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
typedef Dword (*BrBusTx) (
    IN  Bridge*         bridge,
    IN  Dword           bufferLength,
    IN  Byte*           buffer
);


/**
 * Find and Get bus handle used to control bus
 *
 * @param modulator the handle of modulator.
 * @param bufferLength The length to transmit.
 * @param buffer The buffer which we store the data to send.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
typedef Dword (*BrBusRx) (
    IN  Bridge*         bridge,
    IN  Dword           bufferLength,
    IN  Byte*           buffer
);


/**
 * Find and Get bus handle used to control bus
 *
 * @param modulator the handle of modulator.
 * @param registerAddress The starting address of memory to get.
 * @param readBufferLength The length of buffer to receive data.
 * @param readBuffer The buffer use to store received data
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
typedef Dword (*BrBusRxData) (
    IN  Bridge*         bridge,
    IN  Dword           readBufferLength,
    OUT Byte*           readBuffer
);

/**
 * General send command function
 *
 * @param modulator the handle of modulator.
 * @param command The command which you wan.
 * @param valueLength value length.
 * @param valueBuffer value buffer.
 * @param referenceLength reference length.
 * @param referenceBuffer reference buffer.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
typedef Dword (*BrSendCommand) (
    IN  Bridge*         bridge,
    IN  Word            command,
    IN  Byte            chip,
    IN  BrProcessor     processor,
    IN  Dword           writeBufferLength,
    IN  Byte*           writeBuffer,
    IN  Dword           readBufferLength,
    OUT Byte*           readBuffer
);


/**
 * General read EEPROM function
 *
 * @param modulator the handle of modulator.
 * @param registerAddress address of register to be read.
 * @param bufferLength number, 1-8, of registers to be read.
 * @param buffer buffer used to store values to be read to specified
 *        registers.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
typedef Dword (*BrReceiveData) (
    IN  Bridge*         bridge,
    IN  Byte            chip,
    IN  Dword           registerAddress,
    IN  Dword           readBufferLength,
    OUT Byte*           readBuffer
);


/**
 * The type defination of BrBusDescription
 */
typedef struct {
    BrGetBus            getBus;
    BrBusTx             busTx;
    BrBusRx             busRx;
    BrBusRxData         busRxData;
} BrBusDescription;


/**
 * The type defination of CmdDescription
 */
typedef struct {
    Dword                   mailBoxSize;
    BrBusDescription*       busDescription;
    BrWriteRegisters        writeRegisters;
    BrWriteTunerRegisters   writeTunerRegisters;
    BrWriteEepromValues     writeEepromValues;
    BrReadRegisters         readRegisters;
    BrReadTunerRegisters    readTunerRegisters;
    BrReadEepromValues      readEepromValues;
    BrModifyRegister        modifyRegister;
    BrLoadFirmware          loadFirmware;
    BrReboot                reboot;
    BrSendCommand           sendCommand;
    BrReceiveData           receiveData;
} BrCmdDescription;
#endif
/**
 * The type defination of TS_in.
 */
typedef enum {
    BUS_NULL,
    BUS_I2C,
    BUS_USB,
    BUS_9035U2I,
    BUS_USB11
} CtrlBus;


/**
 * The type defination of TS_in.
 */
typedef enum {
    TS_PORT_0 = 0,
    TS_PORT_1 = 1,
    TS_PORT_2,
    TS_PORT_3,
    TS_PORT_4
} TsPort;

/**
 * The type defination of PID_remap mode.
 */
typedef enum {
    REMAP_OFFSET_PASS,
    REMAP_TABLE_PASS,
    REMAP_OFFSET_BLOCK,
    REMAP_TABLE_BLOCK
} PidRemapMode;

/**
 * The type defination of TS Mux Method mode.
 */
typedef enum {    
    AGGRE_NONE      = 0,
    AGGRE_SYNC_BYTE = 1,
    AGGRE_TAG ,           
    AGGRE_PID_REMAP ,           
} TsAggreMode;

/**
 * The type defination of TS Mux Method mode.
 */
typedef enum {    
    PKT_LEN_188     = 0,
    PKT_LEN_192     = 1,
    PKT_LEN_204     = 2,
    PKT_LEN_208     = 3,
} TsPktLen;

/**
 * The type defination of chips connected to bridge.
 */
typedef struct {
    Bool            existed;
    TsType          tsType;
    Byte            i2cAddr;
    Byte            i2cBus;
    Byte            tsPktLen;
    Byte            syncByte;
    Byte            tag[4];
    PidRemapMode    pidRemapMode;
    Word            offset;
    Word            tableLen;
    Word*           orgPidTable;
    Word*           newPidTable;
    TsPort          tsPort;
    Handle          htsDev;
} TsSource;

/**
 * The type defination of chips connected to bridge.
 */
typedef struct {
    Bool        existed;
    Byte        i2cAddr;
    OutDataType outDataType;
    TsPktLen    outTsPktLen;
    Bool        booted;
    Bool        initialized;
} Gator;

//--------------------------
/**
 * The data structure of IT9300
 */
typedef struct {
    Handle              userData;
    Handle              driver;
    CtrlBus             ctrlBus;
    Dword               maxBusTxSize;
    Byte                chipCount;
    Gator               gator[4];
    TsSource            tsSource[4][5];
    Byte*               firmwareCodes;
    BrSegment*          firmwareSegments;
    Byte*               firmwarePartitions;
    Dword               firmwareVersion;
    Word*               scriptSets;
    BrValueSet*         scripts;
    Bool                bypassBoot;   
    Bool                bypassScript;      
    BrValueSet*         timing;    
} Endeavour;

#define BR_BIT_LEN 8
extern const Byte BR_BIT_MASK[BR_BIT_LEN];
#define BR_REG_MASK(pos, len)                (BR_BIT_MASK[len-1] << pos)
#define BR_REG_CLEAR(temp, pos, len)         (temp & (~BR_REG_MASK(pos, len)))
#define BR_REG_CREATE(val, temp, pos, len)   ((val << pos) | (BR_REG_CLEAR(temp, pos, len)))
#define BR_REG_GET(value, pos, len)          ((value & BR_REG_MASK(pos, len)) >> pos)
#define BR_LOWBYTE(w)                        ((Byte)((w) & 0xff))
#define BR_HIGHBYTE(w)                       ((Byte)((w >> 8) & 0xff))
#endif

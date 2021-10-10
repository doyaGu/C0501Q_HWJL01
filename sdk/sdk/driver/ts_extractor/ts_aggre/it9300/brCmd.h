#ifndef __IT9300CMD_H__
#define __IT9300CMD_H__

#include "brType.h"
#include "brUser.h"
#include "brError.h"

/**
 * Define commands
 */
#define Command_REG_DEMOD_READ          0x0000
#define Command_REG_DEMOD_WRITE         0x0001
#define Command_REG_TUNER_READ          0x0002
#define Command_REG_TUNER_WRITE         0x0003
#define Command_REG_EEPROM_READ         0x0004
#define Command_REG_EEPROM_WRITE        0x0005
#define Command_VAR_READ                0x0008
#define Command_VAR_WRITE               0x0009

#define Command_DATA_READ               0x0006

#define Command_PLATFORM_GET            0x000A
#define Command_PLATFORM_SET            0x000B
#define Command_IP_CACHE                0x000D
#define Command_IP_ADD                  0x000E
#define Command_IP_REMOVE               0x000F
#define Command_PID_ADD                 0x0010
#define Command_PID_REMOVE              0x0011
#define Command_SIPSI_GET               0x0012  /** Get SI/PSI table for specific PID "once". */
#define Command_SIPSI_MPE_RESET         0x0013
#define Command_H_PID_ADD               0x0015
#define Command_H_PID_REMOVE            0x0016
#define Command_ABORT                   0x0017
#define Command_IR_GET                  0x0018
#define Command_IR_SET                  0x0019
#define Command_FW_DOWNLOAD_BEGIN       0x0024
#define Command_FW_DOWNLOAD             0x0021
#define Command_FW_DOWNLOAD_END         0x0025
#define Command_QUERYINFO               0x0022
#define Command_BOOT                    0x0023
#define Command_REBOOT                  0x0023
#define Command_RUN_CODE                0x0026
#define Command_SCATTER_READ            0x0028
#define Command_SCATTER_WRITE           0x0029
#define Command_GENERIC_READ            0x002A
#define Command_GENERIC_WRITE           0x002B
#define Command_SHORT_REG_DEMOD_READ    0x02
#define Command_SHORT_REG_DEMOD_WRITE   0X03
#define Command_SHORT_REG_TUNER_READ    0x04
#define Command_SHORT_REG_TUNER_WRITE   0X05

#define Command_SERVICES_GET            0x0083
#define Command_COMPONENT_ADD           0x0086
#define Command_COMPONENT_REMOVE        0x0087
#define Command_FIG_ADD                 0x0088
#define Command_FIG_REMOVE              0x0089


#define Bus_MAX_WRITE_SIZE              254
#define Bus_MAX_READ_SIZE               254


#define BrCmd_buildCommand(command, chip)   ((endeavour->ctrlBus == BUS_USB)?   \
                                                (command + (Word) (chip << 12)):\
                                                 command)                       \


/**
 *
 */
Dword BrCmd_busTx (
    IN  Bridge*         bridge,
    IN  Byte            i2cAddr,
    IN  Dword           bufferLength,
    OUT Byte*           buffer
);
Dword BrCmd_busRx (
    IN  Bridge*         bridge,
    IN  Byte            chip,
    IN  Dword           bufferLength,
    OUT Byte*           buffer
) ;
Dword BrCmd_writeRegisters (
    IN  Bridge*         bridge,
    IN  Byte            chip,
    IN  Dword           registerAddress,
    IN  Byte            registerAddressLength,
    IN  Dword           writeBufferLength,
    IN  Byte*           writeBuffer
);


/**
 *
 */
Dword BrCmd_writeEepromValues (
    IN  Bridge*         bridge,
    IN  Byte            chip,
    IN  Byte            eepromAddress,
    IN  Word            registerAddress,
    IN  Byte            registerAddressLength,
    IN  Byte            writeBufferLength,
    IN  Byte*           writeBuffer
);


/**
 *
 */
Dword BrCmd_readRegisters (
    IN  Bridge*         bridge,
    IN  Byte            chip,
    IN  Dword           registerAddress,
    IN  Byte            registerAddressLength,
    IN  Dword           readBufferLength,
    OUT Byte*           readBuffer
);


/**
 *
 */
Dword BrCmd_readEepromValues (
    IN  Bridge*         bridge,
    IN  Byte            chip,
    IN  Byte            eepromAddress,
    IN  Word            registerAddress,
    IN  Byte            registerAddressLength,
    IN  Byte            readBufferLength,
    OUT Byte*           readBuffer
);


/**
 *
 */
Dword BrCmd_loadFirmware (
    IN  Bridge*         bridge,
    IN  Byte            chip,
    IN  Dword           length,
    IN  Byte*           firmware
);


/**
 *
 */
Dword BrCmd_reboot (
    IN  Bridge*         bridge,
    IN  Byte            chip
);


/**
 *
 */
Dword BrCmd_sendCommand (
    IN  Bridge*         bridge,
    IN  Word            command,
    IN  Byte            chip,
    IN  Dword           writeBufferLength,
    IN  Byte*           writeBuffer,
    IN  Dword           readBufferLength,
    OUT Byte*           readBuffer
);


Dword BrCmd_receiveData (
    IN  Bridge*         bridge,
    IN  Byte            chip,
    IN  Dword           registerAddress,
    IN  Dword           readBufferLength,
    OUT Byte*           readBuffer
);


#endif
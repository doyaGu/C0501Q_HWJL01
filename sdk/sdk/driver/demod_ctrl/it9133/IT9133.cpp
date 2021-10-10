﻿#include "IT9133.h"


Dword Demodulator_writeRegister (
    IN  Demodulator*    demodulator,
    IN  Processor       processor,
    IN  Dword           registerAddress,
    IN  Byte            value
) {
    return (Standard_writeRegister (demodulator, processor, registerAddress, value));
}


Dword Demodulator_writeRegisters (
    IN  Demodulator*    demodulator,
    IN  Processor       processor,
    IN  Dword           registerAddress,
    IN  Byte            bufferLength,
    IN  Byte*           buffer
) {
    return (Standard_writeRegisters (demodulator, processor, registerAddress, bufferLength, buffer));
}


Dword Demodulator_writeRegisterBits (
    IN  Demodulator*    demodulator,
    IN  Processor       processor,
    IN  Dword           registerAddress,
    IN  Byte            position,
    IN  Byte            length,
    IN  Byte            value
)
{
    return (Standard_writeRegisterBits (demodulator, processor, registerAddress, position, length, value));
}


Dword Demodulator_readRegister (
    IN  Demodulator*    demodulator,
    IN  Processor       processor,
    IN  Dword           registerAddress,
    OUT Byte*           value
) {
    return (Standard_readRegister (demodulator, processor, registerAddress, value));
}


Dword Demodulator_readRegisters (
    IN  Demodulator*    demodulator,
    IN  Processor       processor,
    IN  Dword           registerAddress,
    IN  Byte            bufferLength,
    OUT Byte*           buffer
) {
    return (Standard_readRegisters (demodulator, processor, registerAddress, bufferLength, buffer));
}


Dword Demodulator_readRegisterBits (
    IN  Demodulator*    demodulator,
    IN  Processor       processor,
    IN  Dword           registerAddress,
    IN  Byte            position,
    IN  Byte            length,
    OUT Byte*           value
) {
    return (Standard_readRegisterBits (demodulator, processor, registerAddress, position, length, value));
}


Dword Demodulator_getFirmwareVersion (
    IN  Demodulator*    demodulator,
    IN  Processor       processor,
    OUT Dword*          version
) {
    return (Standard_getFirmwareVersion (demodulator, processor, version));
}


Dword Demodulator_getPostVitBer (
    IN  Demodulator*    demodulator,
    OUT Dword*          postErrorCount,  /** 24 bits */
    OUT Dword*          postBitCount,    /** 16 bits */
    OUT Word*           abortCount
){
	return (Standard_getPostVitBer(demodulator, postErrorCount, postBitCount, abortCount));
}

Dword Demodulator_getSignalQualityIndication (
    IN  Demodulator*    demodulator,
    OUT Byte*           quality
) {
    return (Standard_getSignalQualityIndication (demodulator, quality));
}

Dword Demodulator_getSignalStrengthIndication (
    IN  Demodulator*    demodulator,
    OUT Byte*           strength
) {
    return (Standard_getSignalStrengthIndication (demodulator, strength));
}


Dword Demodulator_getSignalStrengthDbm (
    IN  Demodulator*    demodulator,
     OUT Long*           strengthDbm           /** DBm                                */
) {
    return (Standard_getSignalStrengthDbm (demodulator, strengthDbm));
}


Dword Demodulator_initialize (
    IN  Demodulator*    demodulator,
    IN  StreamType      streamType
) {
    return (Standard_initialize (demodulator, streamType));
}


Dword Demodulator_finalize (
    IN  Demodulator*    demodulator
) {
    return (Standard_finalize (demodulator));
}


Dword Demodulator_isLocked (
    IN  Demodulator*    demodulator,
    OUT Bool*           locked
)
{
    return (Standard_isLocked (demodulator, locked));
}


Dword Demodulator_getChannelModulation (
    IN  Demodulator*            demodulator,
    OUT ChannelModulation*      channelModulation
) {
    return (Standard_getChannelModulation (demodulator, channelModulation));
}


Dword Demodulator_acquireChannel (
    IN  Demodulator*    demodulator,
    IN  Word            bandwidth,
    IN  Dword           frequency
) {
    return (Standard_acquireChannel (demodulator, bandwidth, frequency));
}


Dword Demodulator_getStatistic (
    IN  Demodulator*    demodulator,
    OUT Statistic*      statistic
) {
    return (Standard_getStatistic (demodulator, statistic));
}


Dword Demodulator_controlPowerSaving (
    IN  Demodulator*    demodulator,
    IN  Byte            control
) {
    return (Standard_controlPowerSaving (demodulator, control));
}




Dword Demodulator_enableControlPowerSaving (
    IN  Demodulator*    demodulator,
	IN  Byte            enable
) {
    return (Standard_enableControlPowerSaving (demodulator, enable));
}

Dword Demodulator_it9136LoopThruEnable (
	IN  Demodulator*    demodulator,
	IN  Byte            enable
){
    return (Standard_it9136LoopThruEnable(demodulator, enable));
} 

Dword Demodulator_controlPidFilter (
    IN  Demodulator*    demodulator,
    IN  Byte            control
) {
    return (Standard_controlPidFilter (demodulator, control));
}


Dword Demodulator_resetPidFilter (
    IN  Demodulator*    demodulator
) {
    return (Standard_resetPidFilter (demodulator));
}


Dword Demodulator_addPidToFilter (
    IN  Demodulator*    demodulator,
    IN  Byte            index,
    IN  Word            value
) {
    return (Standard_addPidToFilter (demodulator, index, value));
}

Dword Demodulator_getSNR (
    IN  Demodulator*    demodulator,
    OUT Byte*           snr
) {
    return (Standard_getSNR (demodulator, snr));
    
}

Dword Demodulator_setMultiplier (
    IN  Demodulator*    demodulator,
    IN  Multiplier      multiplier
) {
    return (Standard_setMultiplier (demodulator, multiplier));
    
}

Dword Demodulator_setStreamPriority (
	IN Demodulator*  demodulator,
	IN Priority      priority
) {
    return (Standard_setStreamPriority (demodulator, priority));
    
}

Dword Demodulator_reboot (
    IN  Demodulator*    demodulator
)  {
    return (Standard_reboot (demodulator));
}


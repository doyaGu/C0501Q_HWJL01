#include "it9130.h"


Dword Demodulator_writeRegister (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    IN  Processor       processor,
    IN  Dword           registerAddress,
    IN  Byte            value
) {
    return (Standard_writeRegister (demodulator, chip, processor, registerAddress, value));
}


Dword Demodulator_writeRegisters (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    IN  Processor       processor,
    IN  Dword           registerAddress,
    IN  Byte            bufferLength,
    IN  Byte*           buffer
) {
    return (Standard_writeRegisters (demodulator, chip, processor, registerAddress, bufferLength, buffer));
}








Dword Demodulator_writeRegisterBits (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    IN  Processor       processor,
    IN  Dword           registerAddress,
    IN  Byte            position,
    IN  Byte            length,
    IN  Byte            value
)
{
    return (Standard_writeRegisterBits (demodulator, chip, processor, registerAddress, position, length, value));
}


Dword Demodulator_readRegister (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    IN  Processor       processor,
    IN  Dword           registerAddress,
    OUT Byte*           value
) {
    return (Standard_readRegister (demodulator, chip, processor, registerAddress, value));
}


Dword Demodulator_readRegisters (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    IN  Processor       processor,
    IN  Dword           registerAddress,
    IN  Byte            bufferLength,
    OUT Byte*           buffer
) {
    return (Standard_readRegisters (demodulator, chip, processor, registerAddress, bufferLength, buffer));
}








Dword Demodulator_readRegisterBits (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    IN  Processor       processor,
    IN  Dword           registerAddress,
    IN  Byte            position,
    IN  Byte            length,
    OUT Byte*           value
) {
    return (Standard_readRegisterBits (demodulator, chip, processor, registerAddress, position, length, value));
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
    IN  Byte            chip,
    OUT Dword*          postErrorCount,  /** 24 bits */
    OUT Dword*          postBitCount,    /** 16 bits */
    OUT Word*           abortCount
){
	return (Standard_getPostVitBer(demodulator, chip, postErrorCount, postBitCount, abortCount));
}
Dword Demodulator_getRfAgcGain (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    OUT Byte*           rfAgc
) {
    return (Standard_getRfAgcGain (demodulator, chip, rfAgc));
}


Dword Demodulator_getIfAgcGain (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    OUT Byte*           ifAgc
) {
    return (Standard_getIfAgcGain (demodulator, chip, ifAgc));
}


Dword Demodulator_getSignalQuality (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    OUT Byte*           quality
) {
    return (Standard_getSignalQuality (demodulator, chip, quality));
}



Dword Demodulator_getSignalQualityIndication (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    OUT Byte*           quality
) {
    return (Standard_getSignalQualityIndication (demodulator, chip, quality));
}

Dword Demodulator_getSignalStrengthIndication (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    OUT Byte*           strength
) {
    return (Standard_getSignalStrengthIndication (demodulator, chip, strength));
}

Dword Demodulator_getSignalStrength (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    OUT Byte*           strength
) {
    return (Standard_getSignalStrength (demodulator, chip, strength));
}


Dword Demodulator_getSignalStrengthDbm (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    OUT Long*           strengthDbm           /** DBm                                */
) {
    return (Standard_getSignalStrengthDbm (demodulator, chip, strengthDbm));
}



Dword Demodulator_initialize (
    IN  Demodulator*    demodulator,
    IN  Byte            chipNumber,
    IN  Word            sawBandwidth,
    IN  StreamType      streamType,
    IN  Architecture    architecture
) {
    return (Standard_initialize (demodulator, chipNumber, sawBandwidth, streamType, architecture));
}


Dword Demodulator_finalize (
    IN  Demodulator*    demodulator
) {
    return (Standard_finalize (demodulator));
}




Dword Demodulator_isTpsLocked (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    OUT Bool*           locked
) {
    return (Standard_isTpsLocked (demodulator, chip, locked));
}


Dword Demodulator_isMpeg2Locked (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    OUT Bool*           locked
) {
    return (Standard_isMpeg2Locked (demodulator, chip, locked));
}


Dword Demodulator_isLocked (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    OUT Bool*           locked
)
{
    return (Standard_isLocked (demodulator, chip, locked));
}




Dword Demodulator_reset (
    IN  Demodulator*    demodulator
) {
    return (Standard_reset (demodulator));
}


Dword Demodulator_getChannelModulation (
    IN  Demodulator*            demodulator,
    IN  Byte                    chip,
    OUT ChannelModulation*      channelModulation
) {
    return (Standard_getChannelModulation (demodulator, chip, channelModulation));
}




Dword Demodulator_acquireChannel (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    IN  Word            bandwidth,
    IN  Dword           frequency
) {
    return (Standard_acquireChannel (demodulator, chip, bandwidth, frequency));
}


Dword Demodulator_setStreamType (
    IN  Demodulator*    demodulator,
    IN  StreamType      streamType
) {
    return (Standard_setStreamType (demodulator, streamType));
}


Dword Demodulator_setArchitecture (
    IN  Demodulator*    demodulator,
    IN  Architecture    architecture
) {
    return (Standard_setArchitecture (demodulator, architecture));
}


Dword Demodulator_setViterbiRange (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    IN  Byte            superFrameCount,
    IN  Word            packetUnit
) {
    return (Standard_setViterbiRange (demodulator, chip, superFrameCount, packetUnit));
}



Dword Demodulator_getViterbiRange (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    IN  Byte*           superFrameCount,
    IN  Word*           packetUnit
) {
    return (Standard_getViterbiRange (demodulator, chip, superFrameCount, packetUnit));
}


Dword Demodulator_getStatistic (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    OUT Statistic*      statistic
) {
    return (Standard_getStatistic (demodulator, chip, statistic));
}






Dword Demodulator_reboot (
    IN  Demodulator*    demodulator
)  {
    return (Standard_reboot (demodulator));
}


Dword Demodulator_controlPowerSaving (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    IN  Byte            control
) {
    return (Standard_controlPowerSaving (demodulator, chip, control));
}

Dword Demodulator_controlTunerLeakage (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    IN  Byte            control
) {
    return (Standard_controlTunerLeakage (demodulator, chip, control));
}

Dword Demodulator_controlTunerPowerSaving (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
	IN  Byte            control
) {
    return (Standard_controlTunerPowerSaving (demodulator, chip, control));
}





Dword Demodulator_controlPidFilter (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    IN  Byte            control
) {
    return (Standard_controlPidFilter (demodulator, chip, control));
}


Dword Demodulator_resetPidFilter (
    IN  Demodulator*    demodulator,
    IN  Byte            chip
) {
    return (Standard_resetPidFilter (demodulator, chip));
}


Dword Demodulator_addPidToFilter (
    IN  Demodulator*    demodulator,
    IN  Byte            chip,
    IN  Byte            index,
    IN  Pid             pid
) {
    return (Standard_addPidToFilter (demodulator, chip, index, pid));
}


Dword Demodulator_getSNR (
    IN  Demodulator*    demodulator,
	IN  Byte            chip,
    OUT Byte*           snr
) {
    return (Standard_getSNR (demodulator, chip, snr));
    
}


Dword Demodulator_setMultiplier (
	IN  Demodulator*	demodulator,
	IN  Multiplier		multiplier
) {
	return (Standard_setMultiplier (demodulator, multiplier));
}

Dword Demodulator_setStreamPriority (
	IN  Demodulator*    demodulator,
	IN  Byte            chip,
	IN  Priority        priority
) {
    return (Standard_setStreamPriority (demodulator, chip, priority));
    
}



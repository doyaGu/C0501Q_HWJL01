#ifndef __ENDEAVOUR_H__
#define __ENDEAVOUR_H__


#include "brType.h"
#include "brUser.h"
#include "brError.h"
#include "brRegister.h"
#include "brVariable.h"
#include "brVersion.h"

#include "brCmd.h"
//#include ".\bus\i2cimpl.h"
//#include ".\bus\usb2impl.h"
//#include ".\IT9133\IT9133.h"
//#include ".\bus\af9035u2iimpl.h"
#ifdef __cplusplus
extern "C" {
#endif


/**
 * Write one byte (8 bits) to a specific register in modulator.
 *
 * @param modulator the handle of modulator.
 * @param processor The processor of specified register. Because each chip
 *        has two processor so user have to specify the processor. The
 *        possible values are Processor_LINK and Processor_OFDM.
 * @param registerAddress the address of the register to be written.
 * @param value the value to be written.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
Dword IT9300_writeRegister (
    IN  Endeavour*      endeavour,
    IN  Byte            chip,    
    IN  Dword           registerAddress,
    IN  Byte            value
);

/**
 * Write a sequence of bytes to the contiguous registers in modulator.
 * The maximum burst size is restricted by the capacity of bus. If bus
 * could transfer N bytes in one cycle, then the maximum value of
 * bufferLength would be N - 5.
 *
 * @param modulator the handle of modulator.
 * @param processor The processor of specified register. Because each chip
 *        has two processor so user have to specify the processor. The
 *        possible values are Processor_LINK and Processor_OFDM.
 * @param registerAddress the start address of the registers to be written.
 * @param bufferLength the number of registers to be written.
 * @param buffer a byte array which is used to store values to be written.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
Dword IT9300_writeRegisters (
    IN  Endeavour*      endeavour,
    IN  Byte            chip,    
    IN  Dword           registerAddress,
    IN  Byte            bufferLength,
    IN  Byte*           buffer
);

/**
 * Write a sequence of bytes to the contiguous cells in the EEPROM.
 * The maximum burst size is restricted by the capacity of bus. If bus
 * could transfer N bytes in one cycle, then the maximum value of
 * bufferLength would be N - 5 (firmware will detect EEPROM address).
 *
 * @param modulator the handle of modulator.
 * @param registerAddress the start address of the cells to be written.
 * @param bufferLength the number of cells to be written.
 * @param buffer a byte array which is used to store values to be written.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
Dword IT9300_writeEepromValues (
    IN  Endeavour*      endeavour,
    IN  Byte            chip,
    IN  Word            registerAddress,
    IN  Byte            bufferLength,
    IN  Byte*           buffer
);


/**
 * Modify bits in the specific register.
 *
 * @param modulator the handle of modulator.
 * @param processor The processor of specified register. Because each chip
 *        has two processor so user have to specify the processor. The
 *        possible values are Processor_LINK and Processor_OFDM.
 * @param registerAddress the address of the register to be written.
 * @param position the start position of bits to be modified (0 means the
 *        LSB of the specifyed register).
 * @param length the length of bits.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
Dword IT9300_writeRegisterBits (
    IN  Endeavour*      endeavour,
    IN  Byte            chip,    
    IN  Dword           registerAddress,
    IN  Byte            position,
    IN  Byte            length,
    IN  Byte            value
);

/**
 * Read one byte (8 bits) from a specific register in modulator.
 *
 * @param modulator the handle of modulator.
 * @param processor The processor of specified register. Because each chip
 *        has two processor so user have to specify the processor. The
 *        possible values are Processor_LINK and Processor_OFDM.
 * @param registerAddress the address of the register to be read.
 * @param value the pointer used to store the value read from modulator
 *        register.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
Dword IT9300_readRegister (
    IN  Endeavour*      endeavour,
    IN  Byte            chip,    
    IN  Dword           registerAddress,
    OUT Byte*           value
);

/**
 * Read a sequence of bytes from the contiguous registers in modulator.
 * The maximum burst size is restricted by the capacity of bus. If bus
 * could transfer N bytes in one cycle, then the maximum value of
 * bufferLength would be N - 5.
 *
 * @param modulator the handle of modulator.
 * @param processor The processor of specified register. Because each chip
 *        has two processor so user have to specify the processor. The
 *        possible values are Processor_LINK and Processor_OFDM.
 * @param registerAddress the address of the register to be read.
 * @param bufferLength the number of registers to be read.
 * @param buffer a byte array which is used to store values to be read.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
Dword IT9300_readRegisters (
    IN  Endeavour*      endeavour,
    IN  Byte            chip,    
    IN  Dword           registerAddress,
    IN  Byte            bufferLength,
    OUT Byte*           buffer
);

/**
 * Read a sequence of bytes from the contiguous cells in the EEPROM.
 * The maximum burst size is restricted by the capacity of bus. If bus
 * could transfer N bytes in one cycle, then the maximum value of
 * bufferLength would be N - 5 (firmware will detect EEPROM address).
 *
 * @param modulator the handle of modulator.
 * @param registerAddress the start address of the cells to be read.
 * @param registerAddressLength the valid bytes of registerAddress.
 * @param bufferLength the number of cells to be read.
 * @param buffer a byte array which is used to store values to be read.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
Dword IT9300_readEepromValues (
    IN  Endeavour*      endeavour,
    IN  Byte            chip,
    IN  Word            registerAddress,
    IN  Byte            bufferLength,
    OUT Byte*           buffer
);

/**
 * Read bits of the specified register.
 *
 * @param modulator the handle of modulator.
 * @param processor The processor of specified register. Because each chip
 *        has two processor so user have to specify the processor. The
 *        possible values are Processor_LINK and Processor_OFDM.
 * @param registerAddress the address of the register to be read.
 * @param position the start position of bits to be read (0 means the
 *        LSB of the specifyed register).
 * @param length the length of bits.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
Dword Endeavour_readRegisterBits (
    IN  Endeavour*      endeavour,
    IN  Byte            chip,     
    IN  Dword           registerAddress,
    IN  Byte            position,
    IN  Byte            length,
    OUT Byte*           value
);

/**
 * Get the version of firmware.
 *
 * @param modulator the handle of modulator.
 * @param version the version of firmware.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
Dword IT9300_getFirmwareVersion (
    IN  Endeavour*      endeavour,
    IN  Byte            chip
);

/**
 * First, download firmware from host to modulator. Actually, firmware is
 * put in firmware.h as a part of source code. Therefore, in order to
 * update firmware the host have to re-compile the source code.
 * Second, setting all parameters which will be need at the beginning.
 *
 * @param modulator the handle of modulator.
 * @param chipNumber The total number of demodulators.
 * @param sawBandwidth SAW filter bandwidth in MHz. The possible values
 *        are 6000, 7000, and 8000 (KHz).
 * @param streamType The format of output stream.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
Dword IT9300_initialize (
    IN  Endeavour*      endeavour,
    IN  Byte            chip
);


Dword IT9300_load601Timing (
	IN  Endeavour*    	modulator,
	IN  Byte          	chip
    );

Dword IT9300_setOutTsType (
    IN  Endeavour*      endeavour,
    IN  Byte            chip
	);

/**
 * Specify the bandwidth of channel and tune the channel to the specific
 * frequency. Afterwards, host could use output parameter dvbH to determine
 * if there is a DVB-H signal.
 * In DVB-T mode, after calling this function the output parameter dvbH
 * should return False and host could use output parameter "locked" to check
 * if the channel has correct TS output.
 * In DVB-H mode, after calling this function the output parameter dvbH should
 * return True and host could start get platform thereafter.
 *
 * @param modulator the handle of modulator.
 * @param bandwidth The channel bandwidth.
 *        DVB-T: 5000, 6000, 7000, and 8000 (KHz).
 *        DVB-H: 5000, 6000, 7000, and 8000 (KHz).
 *        T-DMB: 5000, 6000, 7000, and 8000 (KHz).
 *        FM: 100, and 200 (KHz).
 * @param frequency the channel frequency in KHz.
 * @return Error_NO_ERROR: successful, non-zero error code otherwise.
 */
Dword IT9300_setOutTsPktLen(
	IN  Endeavour*      endeavour,
	IN  Byte            chip
	);

Dword IT9300_setInTsPktLen(
    IN  Endeavour*      endeavour,
    IN  Byte            chip,
    IN  Byte            tsSrcIdx
    );

Dword IT9300_setInTsType(
	IN  Endeavour*    	endeavour,
	IN  Byte          	chip,
	IN  Byte            tsSrcIdx
	);

Dword IT9300_setSyncByteMode(
    IN  Endeavour*      endeavour,
    IN  Byte            chip,
    IN  Byte            tsSrcIdx
    );

Dword IT9300_setTagMode(
    IN  Endeavour*      endeavour,
    IN  Byte            chip,
    IN  Byte            tsSrcIdx
	);

Dword IT9300_setPidRemapMode(
    IN  Endeavour*      endeavour,
    IN  Byte            chip,
    IN  Byte            tsSrcIdx
    );

Dword IT9300_enPidFilter(
    IN  Endeavour*      endeavour,
    IN  Byte            chip,
    IN  Byte            tsSrcIdx
    );

Dword IT9300_disableTsPort(
    IN  Endeavour*      endeavour,
    IN  Byte            chip,
    IN  Byte            tsSrcIdx
    );

Dword IT9300_enableTsPort(
    IN  Endeavour*      endeavour,
    IN  Byte            chip,
    IN  Byte            tsSrcIdx
    );

Dword IT9300_setExternalclock (
    IN  Endeavour*      endeavour,
    IN  Byte            chip,
    IN  Bool            bvalue
    );

Dword IT9300_setNullpacket (
    IN  Endeavour*      endeavour,
    IN  Byte            chip,
    IN  Bool            bvalue
    ) ;

Dword IT9300_setCheckTEIerror (
    IN  Endeavour*      endeavour,
    IN  Byte            chip,
    IN  Bool            bvalue
    ) ;

Dword IT9300_setIgnoreFail (
    IN  Endeavour*      endeavour,
    IN  Byte            chip,
    IN  Bool            bvalue
    ) ;

Dword IT9300_setOutputTsType (
    IN  Endeavour*      endeavour,
    IN  Byte            chip,
    IN  TsType			tsoutType
    );

Dword IT9300_setOutputclock(
    IN  Endeavour*      endeavour,
    IN  Byte            chip,
    IN  Byte     		value
    );

Dword IT9300_settestmode(
    IN  Endeavour*      endeavour,
    IN  Byte            chip,
    IN  Byte            tsSrcIdx,
    IN  Byte     		mode
    );

Dword IT9300_setDataRate(
    IN  Endeavour*      endeavour,
    IN  Byte            chip,
    IN  Byte            tsSrcIdx,
    IN  Byte     		value
    ); 

Dword IT9300_writeGenericRegisters(
     IN  Endeavour*      endeavour,
     IN  Byte            chip,
     IN  Byte            bus,
     IN  Byte            slaveAddress,
     IN  Byte            bufferLength,
     IN  Byte*           buffer
    );

Dword IT9300_readGenericRegisters (
     IN  Endeavour*      endeavour,
     IN  Byte            chip,
     IN  Byte            bus,
     IN  Byte            slaveAddress,
     IN  Byte            bufferLength,
     OUT Byte*           buffer
    );
    
#ifdef __cplusplus
}
#endif


#endif

#ifndef __EN_USER_H__
#define __EN_USER_H__

#include "brType.h"
#include "brError.h"

#define IT9300User_MAX_PKT_SIZE               255

#define IT9300User_RETRY_MAX_LIMIT            250//10

/** Define I2C master speed, the default value 0x07 means 366KHz (1000000000 / (24.4 * 16 * IT9300User_I2C_SPEED)). */
#define IT9300User_I2C_SPEED				  0x07

/** Define I2C address of secondary chip when Diversity mode or PIP mode is active. */
#define IT9300User_I2C_ADDRESS				  0x38
#define IT9300User_DEVICETYPE			      1

/** Define USB frame size */
#define IT9300User_USB20_MAX_PACKET_SIZE      512
#define IT9300User_USB20_FRAME_SIZE           (188 * 816)//(188 * 348)
#define IT9300User_USB20_FRAME_SIZE_DW        (IT9300User_USB20_FRAME_SIZE / 4)
#define IT9300User_USB11_MAX_PACKET_SIZE      64
#define IT9300User_USB11_FRAME_SIZE           (188 * 21)
#define IT9300User_USB11_FRAME_SIZE_DW        (IT9300User_USB11_FRAME_SIZE / 4)


/**
 * Delay Function
 */
Dword BrUser_delay (
    IN  Bridge*			bridge,
    IN  Dword           dwMs
);


/**
 * Enter critical section
 */
Dword BrUser_enterCriticalSection (
    IN  Bridge*			bridge
);


/**
 * Leave critical section
 */
Dword BrUser_leaveCriticalSection (
    IN  Bridge*			bridge
);


/**
 * Write data via "Control Bus"
 * I2C mode : uc2WireAddr mean modulator chip address, the default value is 0x38
 * USB mode : uc2WireAddr is useless, don't have to send this data
 */
Dword BrUser_busTx (
    IN  Bridge*			bridge,
	IN  Byte			i2cAddr,
    IN  Dword           bufferLength,
    IN  Byte*           buffer
);


/**
 * Read data via "Control Bus"
 * I2C mode : uc2WireAddr mean modulator chip address, the default value is 0x38
 * USB mode : uc2WireAddr is useless, don't have to send this data
 */
Dword BrUser_busRx (
    IN  Bridge*			bridge,
	IN  Byte			i2cAddr,
    IN  Dword           bufferLength,
    OUT Byte*           buffer
);


/**
 * Read data via "Data Bus"
 * I2C mode : uc2WireAddr mean modulator chip address, the default value is 0x38
 * USB mode : uc2WireAddr is useless, don't have to send this data
 */
Dword BrUser_busRxData (
    IN  Bridge*			bridge,
    IN  Dword           bufferLength,
    OUT Byte*           buffer
);
#endif

#ifndef __USER_H__
#define __USER_H__


#include <stdio.h>
#include "type.h"
#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif


#define User_MAX_PKT_SIZE               255
#define User_USE_SHORT_CMD              0

#define User_RETRY_MAX_LIMIT            500

/** Define I2C master speed, the default value 0x07 means 366KHz (1000000000 / (24.4 * 16 * User_I2C_SPEED)). */
#define User_I2C_SPEED              0x07

/** Define I2C address of secondary chip when Diversity mode or PIP mode is active. */
#define User_I2C_ADDRESS            0x38
#define User_Chip2_I2C_ADDRESS      0x3A

/** Define USB frame size */
#define User_USB20_MAX_PACKET_SIZE      512
#define User_USB20_FRAME_SIZE           (188 * 87)
#define User_USB20_FRAME_SIZE_DW        (User_USB20_FRAME_SIZE / 4)
#define User_USB11_MAX_PACKET_SIZE      64
#define User_USB11_FRAME_SIZE           (188 * 21)
#define User_USB11_FRAME_SIZE_DW        (User_USB11_FRAME_SIZE / 4)








/**
 * Delay Function
 */
Dword User_delay (
    IN  Demodulator*    demodulator,
    IN  Dword           dwMs
);

/**
 * Create critical section
 */
Dword User_createCriticalSection (
    void
);

/**
 * Delete critical section
 */
Dword User_deleteCriticalSection (
    void
);

/**
 * Enter critical section
 */
Dword User_enterCriticalSection (
    IN  Demodulator*    demodulator
);


/**
 * Leave critical section
 */
Dword User_leaveCriticalSection (
    IN  Demodulator*    demodulator
);


/**
 * Config MPEG2 interface
 */
Dword User_mpegConfig (
    IN  Demodulator*    demodulator
);


/**
 * Write data via "Control Bus"
 * I2C mode : uc2WireAddr mean demodulator chip address, the default value is 0x38
 * USB mode : uc2WireAddr is useless, don't have to send this data
 */
Dword User_busTx (
    IN  Demodulator*    demodulator,
    IN  Dword           bufferLength,
    IN  Byte*           buffer
);


/**
 * Read data via "Control Bus"
 * I2C mode : uc2WireAddr mean demodulator chip address, the default value is 0x38
 * USB mode : uc2WireAddr is useless, don't have to send this data
 */
Dword User_busRx (
    IN  Demodulator*    demodulator,
    IN  Dword           bufferLength,
    OUT Byte*           buffer
);

Dword OMEGA_supportLNA_USB (
    IN  Demodulator*    demodulator,
    IN  Byte            supporttype
);

Dword User_busRx_Data (
    IN  Demodulator*    demodulator,
    IN  Dword           bufferLength,
    OUT Byte*           buffer,
    IN  void*           pCallbackContext
);

#ifdef __cplusplus
}
#endif

#endif

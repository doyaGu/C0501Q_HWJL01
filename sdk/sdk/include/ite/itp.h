/** @file
 * ITE Platform Library.
 *
 * @author Jim Tan
 * @version 1.0
 * @date 2011-2012
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
/** @defgroup itp ITE Platform Library
 *  @{
 */
#ifndef ITE_ITP_H
#define ITE_ITP_H

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/time.h>
#include <dirent.h>
#include <limits.h>
#include <mqueue.h>
#include <pthread.h>
#include <semaphore.h>
#include "ite/ith.h"
#include "ite/itp_codec.h"
#include "ite/itp_io_expander.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__GNUC__)
    #define __init      __attribute__ ((section(".init.text"), no_instrument_function))
    #define __initdata  __attribute__ ((section(".init.data")))
    #define __initconst __attribute__ ((section(".init.rodata")))
    #define __naked     __attribute__((naked))

#else
    #define __init
    #define __initdata
    #define __initconst
    #define __naked

#endif // defined(__GNUC__)

/** @defgroup itp_sys System
 *  @{
 */
/**
 * Initializes platform abstraction layer module.
 */
void itpInit(void);

/**
 * Exit platform abstraction layer module.
 */
void itpExit(void);

#define ITP_MAX_IDLE_HANDLER 5       ///< Maximum idle handler count

/**
 * Idle handler.
 */
typedef void (*ITPIdleHandler)(void);

/**
 * Registers idle handler.
 *
 * @param handler The handler function.
 */
void itpRegisterIdleHandler(ITPIdleHandler handler);

/**
 * Initializes command line interface module.
 */
void itpCliInit(void);

/** @} */ // end of itp_sys

/** @defgroup itp_posix POSIX Implementations
 *  @{
 */

/**
 * Pointer to function returning void. Used by POSIX timer.
 */
typedef void (*VOIDFUNCPTR) (timer_t timerid, int arg);

/**
 * POSIX timer handle implementation definition.
 */
typedef struct
{
    void        *pxTimer;   ///< Native timer.
    VOIDFUNCPTR routine;    ///< Timer function.
    int         arg;        ///< Timer function argument.
} itpTimer;

/**
 * Connects a user routine to the timer signal
 *
 * @param timerid timer ID
 * @param routine user routine
 * @param arg user argument
 * @return 0 (OK), or -1 (ERROR) if the timer is invalid or cannot bind the signal handler.
 */
int timer_connect(timer_t timerid, VOIDFUNCPTR routine, int arg);

/**
 * Gets the difference of two timeval, in msec.
 *
 * @param starttime start time
 * @param finishtime finish time
 * @return the difference, in msec.
 *
 * @code{.c}
 * struct timeval startTime, endTime;
 * gettimeofday(&startTime, NULL);
 * sleep(5);
 * gettimeofday(&endTime, NULL);
 * printf("duration: %ld ms\n", itpTimevalDiff(&startTime, &endTime));
 * @endcode
 */
long itpTimevalDiff(struct timeval *starttime, struct timeval *finishtime);

/**
 * Gets the tick count, in msec.
 */
uint32_t itpGetTickCount(void);

/**
 * Gets the tick duration, in msec.
 *
 * @param tick start time
 * @return the difference of the start time and current time, in mesc.
 *
 * @code{.c}
 * uint32_t startTime = itpGetTickCount();
 * sleep(5);
 * printf("duration: %u ms\n", itpGetTickDuration(startTime));
 * @endcode
 */
uint32_t itpGetTickDuration(uint32_t tick);

/**
 * Gets free heap size, in bytes.
 *
 * @return free heap size, in bytes.
 */
uint32_t itpGetFreeHeapSize(void);

/**
 * Posts semaphore from ISR.
 *
 * @param sem semaphore handle.
 */
void itpSemPostFromISR(sem_t *sem);

/**
 * Gets semaphore value from ISR.
 *
 * @param sem semaphore handle.
 * @param sval return value.
 * @return 0 (OK), or -1 (ERROR).
 */
int itpSemGetValueFromISR(sem_t *sem, int *sval);

/**
 * Waits semaphore with timeout.
 *
 * @param sem semaphore handle.
 * @param ms timeout value.
 * @return 0 (OK), or -1 (ERROR).
 */
int itpSemWaitTimeout(sem_t *sem, unsigned long ms);

/**
 * Notifies task.
 *
 * @param pthread task to notify.
 */
void itpTaskNotifyGive(pthread_t pthread);

/**
 * Notifies task from ISR.
 *
 * @param pthread task to notify.
 */
void itpTaskNotifyGiveFromISR(pthread_t pthread);

/**
 * Takes task notify.
 *
 * @param clearCountOnExit task's notification value is reset to 0 or not.
 * @param msToWait The maximum time to wait. -1 to wait inifite.
 */
int itpTaskNotifyTake(bool clearCountOnExit, long msToWait);

// Endian
#if BYTE_ORDER == LITTLE_ENDIAN
/**
 * Converts 16-bit big-enddian integer to host 16-bit integer.
 *
 * @param value The big-endian integer.
 * @return The result value.
 */
static inline uint16_t itpBetoh16(uint16_t value)
{
    return ithBswap16(value);
}

/**
 * Converts 32-bit big-enddian integer to host 32-bit integer.
 *
 * @param value The big-endian integer.
 * @return The result value.
 */
static inline uint32_t itpBetoh32(uint32_t value)
{
    return ithBswap32(value);
}

/**
 * Converts 64-bit big-enddian integer to host 64-bit integer.
 *
 * @param value The big-endian integer.
 * @return The result value.
 */
static inline uint64_t itpBetoh64(uint64_t value)
{
    return ithBswap64(value);
}

/**
 * Converts 16-bit little-enddian integer to host 16-bit integer.
 *
 * @param value The little-endian integer.
 * @return The result value.
 */
static inline uint16_t itpLetoh16(uint16_t value)
{
    return value;
}

/**
 * Converts 32-bit little-enddian integer to host 32-bit integer.
 *
 * @param value The little-endian integer.
 * @return The result value.
 */
static inline uint32_t itpLetoh32(uint32_t value)
{
    return value;
}

/**
 * Converts 64-bit little-enddian integer to host 64-bit integer.
 *
 * @param value The little-endian integer.
 * @return The result value.
 */
static inline uint64_t itpLetoh64(uint64_t value)
{
    return value;
}

#else
static inline uint16_t itpBetoh16(uint16_t value)
{
    return value;
}

static inline uint32_t itpBetoh32(uint32_t value)
{
    return value;
}

static inline uint64_t itpBetoh64(uint64_t value)
{
    return value;
}

static inline uint16_t itpLetoh16(uint16_t value)
{
    return ithBswap16(value);
}

static inline uint32_t itpLetoh32(uint32_t value)
{
    return ithBswap32(value);
}

static inline uint64_t itpLetoh64(uint64_t value)
{
    return ithBswap64(value);
}

#endif // BYTE_ORDER == LETTLE_ENDIAN

#if BYTE_ORDER == LITTLE_ENDIAN
    #ifndef cpu_to_le32
        #define cpu_to_le32(x) ((uint32_t)(x))
    #endif
    #ifndef cpu_to_be32
        #define cpu_to_be32(x) ithBswap32((uint32_t)(x))
    #endif
    #ifndef le32_to_cpu
        #define le32_to_cpu(x) ((uint32_t)(x))
    #endif
    #ifndef cpu_to_le16
        #define cpu_to_le16(x) ((uint16_t)(x))
    #endif
    #ifndef cpu_to_be16
        #define cpu_to_be16(x) ithBswap16((uint16_t)(x))
    #endif
    #ifndef le16_to_cpu
        #define le16_to_cpu(x) ((uint16_t)(x))
    #endif
    #ifndef be16_to_cpu
        #define be16_to_cpu(x) ithBswap16((uint16_t)(x))
    #endif
    #ifndef be32_to_cpu
        #define be32_to_cpu(x) ithBswap32((uint32_t)(x))
    #endif
#else
    #ifndef cpu_to_le32
        #define cpu_to_le32(x) ithBswap32((uint32_t)(x))
    #endif
    #ifndef cpu_to_be32
        #define cpu_to_be32(x) ((uint32_t)(x))
    #endif
    #ifndef le32_to_cpu
        #define le32_to_cpu(x) ithBswap32((uint32_t)(x))
    #endif
    #ifndef cpu_to_le16
        #define cpu_to_le16(x) ithBswap16((uint16_t)(x))
    #endif
    #ifndef cpu_to_be16
        #define cpu_to_be16(x) ((uint16_t)(x))
    #endif
    #ifndef le16_to_cpu
        #define le16_to_cpu(x) ithBswap16((uint16_t)(x))
    #endif
    #ifndef be16_to_cpu
        #define be16_to_cpu(x) ((uint16_t)(x))
    #endif
    #ifndef be32_to_cpu
        #define be32_to_cpu(x) ((uint32_t)(x))
    #endif
#endif // BYTE_ORDER == LETTLE_ENDIAN

/**
 * Print backtrace log.
 */
void itpPrintBacktrace(void);

/**
* Gets the number of UTF-8 word count
*
* @param s The UTF-8 string.
* @return The word count.
*/
static inline size_t strlen_utf8(const char* s)
{
	size_t len = 0;
	for (; *s; ++s) if ((*s & 0xC0) != 0x80) ++len;
	return len;
}

/** @} */ // end of itp_posix

/** @defgroup itp_memleak Memory Leak Tool
 *  @{
 *  How to use it:
 *  Add calls to dbg_mem_stat() or dbg_heap_dump() or others (see below)
 *  to any suspicious place of your code. You may use "keyword" argument
 *  to the dumping functions to reduce the amount of the output.
 */
#define CHK_FREED 1
#define CHK_ALLOC 2
#define CHK_ALL   (CHK_FREED | CHK_ALLOC)

/**
 * This initializes the memory leak debugger. @p history_length defines the
 * maximum number of stored headers of the freed data.
 *
 * @param history_length the maximum number of stored headers of the freed data.
 */
extern void dbg_init(int history_length);

/**
 * checks for addr in the tables given by opt. opt can be CHK_FREED or
 * CHK_ALLOC or CHK_ALL; returns CHK_FREED, CHK_ALLOC, or 0 if not found.
 * Also prints debugging message to stderr, prefixed with msg.
 */
extern int dbg_check_addr(char *msg, void *ptr, int opt);

/**
 * This function prints call statistics to stderr. The location of the
 * call is printed at the beginning of the status line. The status line
 * looks like this:<br>
 * <br>
 * file:line m:\<num\> c: \<num\> r:\<num\> f:\<num\> mem:\<num\><br>
 * <br>
 * where m, c, r, f count malloc(), calloc(), realloc(), and free() calls.
 * mem equals the total amount of requested and not yet freed memory (not
 * exactly the same as the amount of actually allocated physical memory).
 */
extern void dbg_mem_stat(void);

/**
 * clears statistics.
 */
extern void dbg_zero_stat(void);

extern void dbg_abort(char *msg);

/**
 * This function prints a dump of the current heap state to stderr. Every
 * line of the dump corresponds to an allocated and not yet freed piece of
 * data. Use "keyword" to narrower the dump: keyword = "" => entire heap
 * is dumped, if not "", strstr() is used to select strings with keyword
 * in them.
 */
extern void dbg_heap_dump(char *keyword);

/**
 * dumps history of malloc() - free() calls. keyword is the same as above.
 */
extern void dbg_history_dump(char *keyword);

/** @} */ // end of itp_memleak

/** @defgroup itp_device Device Drivers
 *  @{
 */
// Constant definitions
#define ITP_DEVICE_BIT       8                              ///< Device handle offset
#define ITP_DEVICE_MASK      (0xFF << ITP_DEVICE_BIT)       ///< Device handle mask
#define ITP_FILE_MASK        0xFF                           ///< File handle mask
#define ITP_DEVICE_ERRNO_BIT 8                              ///< Device error number bit

/**
 * Device types.
 */
typedef enum
{
    // Standard IO devices
    ITP_DEVICE_STD           = (0 << ITP_DEVICE_BIT),        ///< Standard IO
    ITP_DEVICE_SOCKET        = (1 << ITP_DEVICE_BIT),        ///< LWIP socket

    // File system devices
    ITP_DEVICE_FS            = (2 << ITP_DEVICE_BIT),        ///< File systems
    ITP_DEVICE_FAT           = (2 << ITP_DEVICE_BIT),        ///< FAT file system
    ITP_DEVICE_NTFS          = (3 << ITP_DEVICE_BIT),        ///< NTFS file system

    // Custom devices
    ITP_DEVICE_CUSTOM        = (4 << ITP_DEVICE_BIT),        ///< Custom devices
    ITP_DEVICE_PRINTBUF      = (5 << ITP_DEVICE_BIT),        ///< Print buffer
    ITP_DEVICE_SWUART        = (6 << ITP_DEVICE_BIT),        ///< Software UART
    ITP_DEVICE_UART0         = (7 << ITP_DEVICE_BIT),        ///< UART0
    ITP_DEVICE_UART1         = (8 << ITP_DEVICE_BIT),        ///< UART1
    ITP_DEVICE_LCDCONSOLE    = (9 << ITP_DEVICE_BIT),        ///< LCD console
    ITP_DEVICE_OSDCONSOLE    = (10 << ITP_DEVICE_BIT),       ///< OSD console
    ITP_DEVICE_SCREEN        = (11 << ITP_DEVICE_BIT),       ///< Screen
    ITP_DEVICE_I2C0          = (12 << ITP_DEVICE_BIT),       ///< I2C0
    ITP_DEVICE_I2C1          = (13 << ITP_DEVICE_BIT),       ///< I2C1
    ITP_DEVICE_SPI           = (14 << ITP_DEVICE_BIT),       ///< SPI
    ITP_DEVICE_IR            = (15 << ITP_DEVICE_BIT),       ///< IR
    ITP_DEVICE_NAND          = (16 << ITP_DEVICE_BIT),       ///< NAND
    ITP_DEVICE_NOR           = (17 << ITP_DEVICE_BIT),       ///< NOR
    ITP_DEVICE_SD0           = (18 << ITP_DEVICE_BIT),       ///< SD0
    ITP_DEVICE_SD1           = (19 << ITP_DEVICE_BIT),       ///< SD1
    ITP_DEVICE_USBDFSG       = (20 << ITP_DEVICE_BIT),       ///< USB acts as a USB Mass Storage device
    ITP_DEVICE_CARD          = (21 << ITP_DEVICE_BIT),       ///< Card
    ITP_DEVICE_DRIVE         = (22 << ITP_DEVICE_BIT),       ///< Drive
    ITP_DEVICE_KEYPAD        = (23 << ITP_DEVICE_BIT),       ///< Keypad
    ITP_DEVICE_POWER         = (24 << ITP_DEVICE_BIT),       ///< Power
    ITP_DEVICE_GSENSOR       = (25 << ITP_DEVICE_BIT),       ///< G-Sensor
    ITP_DEVICE_HEADSET       = (26 << ITP_DEVICE_BIT),       ///< Headset
    ITP_DEVICE_AMPLIFIER     = (27 << ITP_DEVICE_BIT),       ///< Audio amplifier
    ITP_DEVICE_STC           = (28 << ITP_DEVICE_BIT),       ///< STC
    ITP_DEVICE_DECOMPRESS    = (29 << ITP_DEVICE_BIT),       ///< Decompress
    ITP_DEVICE_CODEC         = (30 << ITP_DEVICE_BIT),       ///< Audio codec
    ITP_DEVICE_ETHERNET      = (31 << ITP_DEVICE_BIT),       ///< Ethernet
    ITP_DEVICE_WIFI          = (32 << ITP_DEVICE_BIT),       ///< WiFi
    ITP_DEVICE_FILE          = (33 << ITP_DEVICE_BIT),       ///< File
    ITP_DEVICE_DEMOD         = (34 << ITP_DEVICE_BIT),       ///< Demod
    ITP_DEVICE_WATCHDOG      = (35 << ITP_DEVICE_BIT),       ///< Watch Dog
    ITP_DEVICE_NETCONSOLE    = (36 << ITP_DEVICE_BIT),       ///< Network console
    ITP_DEVICE_USB           = (37 << ITP_DEVICE_BIT),       ///< USB
    ITP_DEVICE_DPU           = (38 << ITP_DEVICE_BIT),       ///< DPU (encryption/decryption)
    ITP_DEVICE_XD            = (39 << ITP_DEVICE_BIT),       ///< XD
    ITP_DEVICE_LED           = (40 << ITP_DEVICE_BIT),       ///< LED
    ITP_DEVICE_SWITCH        = (41 << ITP_DEVICE_BIT),       ///< Switch
    ITP_DEVICE_TUNER         = (42 << ITP_DEVICE_BIT),       ///< Tuner
    ITP_DEVICE_STNLCD        = (43 << ITP_DEVICE_BIT),       ///< STN LCD
    ITP_DEVICE_USBMOUSE      = (44 << ITP_DEVICE_BIT),       ///< USB Mouse
    ITP_DEVICE_USBKBD        = (45 << ITP_DEVICE_BIT),       ///< USB Keyboard
    ITP_DEVICE_RTC           = (46 << ITP_DEVICE_BIT),       ///< RTC
    ITP_DEVICE_BACKLIGHT     = (47 << ITP_DEVICE_BIT),       ///< Backlight
    ITP_DEVICE_GPIO_EXPANDER = (48 << ITP_DEVICE_BIT),       ///< GPIO Extender
    ITP_DEVICE_RS485_0       = (49 << ITP_DEVICE_BIT),       ///< RS485_0
    ITP_DEVICE_RS485_1       = (50 << ITP_DEVICE_BIT),       ///< RS485_1
    ITP_DEVICE_SWUART_CODEC  = (51 << ITP_DEVICE_BIT),       ///< Software UART codec
    ITP_DEVICE_WIEGAND0      = (52 << ITP_DEVICE_BIT),       ///< Wiegand0
    ITP_DEVICE_WIEGAND1      = (53 << ITP_DEVICE_BIT),       ///< Wiegand1
    ITP_DEVICE_CTRLBOARD     = (54 << ITP_DEVICE_BIT),       ///< CTRLBOARD
    //ITP_DEVICE_ARMSWUART        =  (55 << ITP_DEVICE_BIT),   ///< Armswuart
    ITP_DEVICE_SWUARTDBG     = (56 << ITP_DEVICE_BIT),       ///< SWUART DEBUG
    ITP_DEVICE_UART2         = (57 << ITP_DEVICE_BIT),       ///< UART2
    ITP_DEVICE_UART3         = (58 << ITP_DEVICE_BIT),       ///< UART3
    ITP_DEVICE_RS485_2       = (59 << ITP_DEVICE_BIT),       ///< RS485_2
    ITP_DEVICE_RS485_3       = (60 << ITP_DEVICE_BIT),       ///< RS485_3
    ITP_DEVICE_RS485_4       = (61 << ITP_DEVICE_BIT),       ///< RS485_4
    ITP_DEVICE_SPI1          = (62 << ITP_DEVICE_BIT),       ///< SPI
	ITP_DEVICE_SDIO 		 = (63 << ITP_DEVICE_BIT),		 ///< SDIO
	ITP_DEVICE_RGBTOMIPI 	 = (64 << ITP_DEVICE_BIT),		 ///< RGBToMIPI
	ITP_DEVICE_IOEX			 = (65 << ITP_DEVICE_BIT),		 ///< IoEx
    ITP_DEVICE_LOGDISK       = (66 << ITP_DEVICE_BIT),       ///< Log to disk
    ITP_DEVICE_USBDIDB       = (67 << ITP_DEVICE_BIT),       ///< USB acts as a USB device
    ITP_DEVICE_ALT_CPU       = (68 << ITP_DEVICE_BIT),       ///< Software peripheral on ALT CPU
    ITP_DEVICE_LAST
} ITPDeviceType;

#define ITP_DEVICE_MAX (((ITP_DEVICE_LAST - 1) >> ITP_DEVICE_BIT) + 1)

/**
 * Structure used to define a device driver
 */
typedef struct
{
    const char *name;  ///< Device name

    /**
     * Device open method.
     *
     * @param filename Device name.
     * @param oflag Type of operations allowed.
     * @param mode Permission mode.
     * @param info Device custom data.
     * @return Returns a file descriptor for the opened device. A return value of -1 indicates an error, in which case errno is set.
     */
    int (*open)(const char *name, int flags, int mode, void *info);

    /**
     * Device close method
     *
     * @param file File descriptor referring to the open device.
     * @param info Device custom data.
     * @return Returns 0 if the device was successfully closed. A return value of -1 indicates an error.
     */
    int (*close)(int file, void *info);

    /**
     * Device read method
     *
     * @param file File descriptor referring to the open device.
     * @param ptr Storage location for data.
     * @param len Maximum number of data unit.
     * @param info Device custom data.
     * @return Returns the number of data unit read. A return value of -1 indicates an error.
     */
    int (*read)(int file, char *ptr, int len, void *info);

    /**
     * Device write method
     *
     * @param file File descriptor of device into which data is written.
     * @param ptr Data to be written.
     * @param len Number of data unit.
     * @param info Device custom data.
     * @return If successful, it returns the number of data unit actually written. A return value of -1 indicates an error.
     */
    int (*write)(int file, char *ptr, int len, void *info);

    /**
     * Set position in a device.
     *
     * @param file File descriptor referring to an open device.
     * @param ptr Number of data unit from origin.
     * @param dir Initial position.
     * @param info Device custom data.
     * @return Returns the offset, in data unit, of the new position from the beginning of the file. The function returns -1 to indicate an error.
     */
    int (*lseek)(int file, int ptr, int dir, void *info);

    /**
     * Device ioctl method (controls device operating parameters).
     *
     * @param file File descriptor referring to an open device.
     * @param request Selects the control function to be performed.
     * @param ptr Additional information that is needed by this specific device to perform the requested function.
     * @param info Device custom data.
     * @return Upon successful completion, it shall return a value other than -1 that depends upon the device control function. Otherwise, it shall return -1 and set errno to indicate the error.
     */
    int (*ioctl)(int file, unsigned long request, void *ptr, void *info);

    void *info;
} ITPDevice;

// Default functions
/**
 * Default empty device open method.
 *
 * @param name Not use.
 * @param flags Not use.
 * @param mode Not use.
 * @param info Not use.
 * @return Always return -1.
 */
int itpOpenDefault(const char *name, int flags, int mode, void *info);

/**
 * Default empty device close method
 *
 * @param file Not use.
 * @param info Not use.
 * @return Always return -1.
 */
int itpCloseDefault(int file, void *info);

/**
 * Default empty device read method
 *
 * @param file Not use.
 * @param ptr Not use.
 * @param len Not use.
 * @param info Not use.
 * @return Always return 0.
 */
int itpReadDefault(int file, char *ptr, int len, void *info);

/**
 * Default empty device write method
 *
 * @param file Not use.
 * @param ptr Not use.
 * @param len Number of data unit.
 * @param info Not use.
 * @return Number of data unit.
 */
int itpWriteDefault(int file, char *ptr, int len, void *info);

/**
 * Default empty device lseek method
 *
 * @param file Not use.
 * @param ptr Not use.
 * @param dir Not use.
 * @param info Not use.
 * @return Always return 0.
 */
int itpLseekDefault(int file, int ptr, int dir, void *info);

/**
 * Default empty device ioctl method.
 *
 * @param file Not use.
 * @param request Not use.
 * @param ptr Not use.
 * @param info Not use.
 * @return Always return 0.
 */
int itpIoctlDefault(int file, unsigned long request, void *ptr, void *info);

/**
 * Registers device to system.
 *
 * @param type The device type.
 * @param device The device to register.
 */
void itpRegisterDevice(ITPDeviceType type, const ITPDevice *device);

/**
 * List of devices.  Newlib adapatation uses this to determine
 * what devices are available and how to control them.
 * Note:  Entries 0, 1, 2 are reserved to address stdin,
 * stdout and stderr. These must be able to work from a
 * standard open w/o needing further setup if they are to be
 * transparent, alternatively they must be setup before any I/O
 * is done.
 */
extern const ITPDevice *itpDeviceTable[ITP_DEVICE_MAX];

/**
 * File System device definition.
 */
typedef struct
{
    ITPDevice dev;  ///< Inherts common device structure

    /**
     * Removes a file.
     *
     * @param path points to the pathname of the file to be removed.
     * @return Upon successful completion, it shall return 0; otherwise, -1 shall be returned.
     */
    int (*remove)(const char *path);

    /**
     * Renames a file.
     *
     * @param oldname points to the pathname of the file to be renamed.
     * @param newname points to the new pathname of the file.
     * @return Upon successful completion, it shall return 0; otherwise, -1 shall be returned.
     */
    int (*rename)(const char *oldname, const char *newname);

    /**
     * Changes the current working directory.
     *
     * @param path Path of new working directory.
     * @return Upon successful completion, it shall return 0; otherwise, -1 shall be returned.
     */
    int (*chdir)(const char *path);

    /**
     * Changes the file-permission settings.
     *
     * @param path Name of the existing file.
     * @param mode Permission setting for the file.
     * @return Upon successful completion, it shall return 0; otherwise, -1 shall be returned.
     */
    int (*chmod)(const char *path, mode_t mode);

    /**
     * Creates a new directory.
     *
     * @param path Path for a new directory.
     * @param mode Permission setting for the path.
     * @return Upon successful completion, it shall return 0; otherwise, -1 shall be returned.
     */
    int (*mkdir)(const char *path, mode_t mode);

    /**
     * Get status information on a file.
     *
     * @param path Pointer to a string containing the path of existing file or directory.
     * @param buf Pointer to structure that stores results.
     * @return Upon successful completion, it shall return 0; otherwise, -1 shall be returned.
     */
    int (*stat)(const char *path, struct stat *buf);

    /**
     * Get VFS File System information on a file.
     *
     * @param path Pointer to a string containing the path of existing file or directory.
     * @param buf Pointer to structure that stores results.
     * @return Upon successful completion, it shall return 0; otherwise, -1 shall be returned.
     */
    int (*statvfs)(const char *path, struct statvfs *buf);

    /**
     * Gets information about an open file.
     *
     * @param file File descriptor of the open file.
     * @param st Pointer to the structure to store results.
     * @return Returns 0 if the file-status information is obtained. A return value of -1 indicates an error.
     */
    int (*fstat)(int file, struct stat *st);

    /**
     * Gets the current working directory.
     *
     * @param buf Storage location for the path.
     * @param size Maximum length of the path in characters.
     * @return Returns a pointer to buffer. A NULL return value indicates an error.
     */
    char * (*getcwd)(char *buf, size_t size);

    /**
     * Deletes a directory.
     *
     * @param path Path of the directory to be removed.
     * @return returns 0 if the directory is successfully deleted. A return value of ?V1 indicates an error.
     */
    int (*rmdir)(const char *path);

    /**
     * Closes a directory stream.
     *
     * @param dirp The directory stream.
     * @return Upon successful completion, it shall return 0; otherwise, -1 shall be returned.
     */
    int (*closedir)(DIR *dirp);

    /**
     * Opens a directory.
     *
     * @param dirname The directory name.
     * @return Upon successful completion, it shall return a pointer to an object of type DIR. Otherwise, a null pointer shall be returned.
     */
    DIR * (*opendir)(const char *dirname);

    /**
     * Reads a directory.
     *
     * @param dirp The directory stream.
     * @return Upon successful completion, it shall return a pointer to an object of type struct dirent. When an error is encountered, a null pointer shall be returned.
     */
    struct dirent * (*readdir)(DIR * dirp);

    /**
     * Resets the position of a directory stream to the beginning of a directory.
     *
     * @param dirp The directory stream.
     */
    void (*rewinddir)(DIR *dirp);

    /**
     * Gets the current position of a file pointer.
     *
     * @param file File descriptor of the open file.
     * @return Returns the current file position.
     */
    long (*ftell)(int file);

    /**
     * Flushes a stream.
     *
     * @param file File descriptor of the open file.
     * @return returns 0 if the buffer was successfully flushed.
     */
    int (*fflush)(int file);

    /**
     * Checks whether the end-of-File indicator associated with stream is set.
     *
     * @param file File descriptor of the open file.
     * @return A non-zero value is returned in the case that the end-of-file indicator associated with the stream is set.
     */
    int (*feof)(int file);

} ITPFSDevice;

// IOCTL definitions
#define ITP_IOCTL_INIT                 1   ///< Initialize
#define ITP_IOCTL_EXIT                 2   ///< Exit
#define ITP_IOCTL_ENABLE               3   ///< Enable
#define ITP_IOCTL_DISABLE              4   ///< Disable
#define ITP_IOCTL_MOUNT                5   ///< Mount
#define ITP_IOCTL_UNMOUNT              6   ///< Unmount
#define ITP_IOCTL_SET_FGCOLOR          7   ///< Sets foreground color
#define ITP_IOCTL_SET_BGCOLOR          8   ///< Sets background color
#define ITP_IOCTL_CLEAR                9   ///< Clear
#define ITP_IOCTL_GET_MAX_LEVEL        10  ///< Gets maximum level
#define ITP_IOCTL_RESUME               11  ///< Resume
#define ITP_IOCTL_STANDBY              12  ///< Standby
#define ITP_IOCTL_SLEEP                13  ///< Sleep
#define ITP_IOCTL_SUSPEND              14  ///< Suspend
#define ITP_IOCTL_OFF                  15  ///< Off
#define ITP_IOCTL_ON                   16  ///< On
#define ITP_IOCTL_SET_BRIGHTNESS       17  ///< Sets brightness
#define ITP_IOCTL_PROBE                18  ///< Probe
#define ITP_IOCTL_GET_TABLE            19  ///< Get table
#define ITP_IOCTL_PAUSE                20  ///< Pause
#define ITP_IOCTL_GET_PARTITION        21  ///< Gets partition
#define ITP_IOCTL_CREATE_PARTITION     22  ///< Create Partition
#define ITP_IOCTL_FORMAT               23  ///< Format
#define ITP_IOCTL_GET_SIZE             24  ///< Gets size
#define ITP_IOCTL_GET_BLOCK_SIZE       25  ///< Gets block size
#define ITP_IOCTL_MUTE                 26  ///< Mute
#define ITP_IOCTL_UNMUTE               27  ///< Un-mute
#define ITP_IOCTL_TVOUT                28  ///< TV-out
#define ITP_IOCTL_RESET                29  ///< Reset
#define ITP_IOCTL_SCAN                 30  ///< Scan
#define ITP_IOCTL_POLL                 31  ///< Poll
#define ITP_IOCTL_IS_AVAIL             32  ///< Is available
#define ITP_IOCTL_GET_INFO             33  ///< Gets information
#define ITP_IOCTL_RESTORE              34  ///< Restore
#define ITP_IOCTL_FLUSH                35  ///< Flush
#define ITP_IOCTL_GET_VOLUME           36  ///< Gets volume
#define ITP_IOCTL_SET_VOLUME           37  ///< Sets volume
#define ITP_IOCTL_POST_RESET           38  ///< Post Reset
#define ITP_IOCTL_GET_GAP_SIZE         39  ///< Gets gap size
#define ITP_IOCTL_SCAN_CHANNEL         40  ///< Demod scan channel
#define ITP_IOCTL_RECEIVE_TS           41  ///< Demod receive ts data
#define ITP_IOCTL_SKIP_TS              42  ///< Demod skip ts
#define ITP_IOCTL_GET_COUNRTY_NAME     43  ///< Get supported country name
#define ITP_IOCTL_IS_CONNECTED         44  ///< Is connected
#define ITP_IOCTL_SET_CIPHER           45  ///< Dpu set cipher mode
#define ITP_IOCTL_SET_DESCRYPT         46  ///< Dpu set encrypt/descrypt
#define ITP_IOCTL_SET_CRC_MASTER       47  ///< Dpu set CRC slave mode
#define ITP_IOCTL_SET_KEY_LENGTH       48  ///< Dpu set key length
#define ITP_IOCTL_SET_KEY_BUFFER       49  ///< Dpu set key buffer
#define ITP_IOCTL_SET_VECTOR_LENGTH    50  ///< Dpu set vector length
#define ITP_IOCTL_SET_VECTOR_BUFFER    51  ///< Dpu set vector buffer
#define ITP_IOCTL_FLICKER              52  ///< LED flicker
#define ITP_IOCTL_IS_DEVICE_READY      53  ///< Is Device Ready
#define ITP_IOCTL_GET_TS_HANDLE        54  ///< Get Ts demuxer handle
#define ITP_IOCTL_WIFI_LINK_AP         55  ///< Set ssid
#define ITP_IOCTL_TS_HANDLE_MERGE      56  ///< Merge serivce info of ts handles
#define ITP_IOCTL_SET_TS_SUSPEND       57  ///< Set TS demod suspend mode
#define ITP_IOCTL_HIBERNATION          58  ///< Hibernation
#define ITP_IOCTL_WIFI_RELINK          59  ///< Wifi re connection
#define ITP_IOCTL_WIFI_STATE           60  ///< Wifi driver state
#define ITP_IOCTL_FORCE_MOUNT          61  ///< Force mount
#define ITP_IOCTL_TS_SUSPEND_FIRE      62  ///< Enable/Disable TS demod suspend mode
#define ITP_IOCTL_WIFI_SOCKET          63  ///< Wifi ioctl
#define ITP_IOCTL_WIFIAP_ENABLE        64  ///< Wifi ap Enable
#define ITP_IOCTL_WIFIAP_DISABLE       65  ///< Wifi ap Disable
#define ITP_IOCTL_WIFIAP_GET_INFO      66  ///< Wifi ap Gets information
#define ITP_IOCTL_WIFI_START_DHCP      67  ///< Wifi start dhcp
#define ITP_IOCTL_WIFI_MODE            68  ///< Wifi mode ap/sta
#define ITP_IOCTL_INIT_TASK            69  ///< Initializes Task
#define ITP_IOCTL_GET_TIME             70  ///< Gets time
#define ITP_IOCTL_SET_TIME             71  ///< Sets time
#define ITP_IOCTL_WIRTE_MAC            72  ///< Writes MAC address
#define ITP_IOCTL_GET_BIT_COUNT        73  ///< Wiegand get bit count
#define ITP_IOCTL_SET_BIT_COUNT        74  ///< Wiegand set bit count
#define ITP_IOCTL_SET_GPIO_PIN         75  ///< Wiegand set gpio pin
#define ITP_IOCTL_SET_BAUDRATE         76  ///< SWUart codec/RS485/HWUart set baudrate
#define ITP_IOCTL_SET_PARITY           77  ///< SWUart codec/RS485/HWUart set parity
#define ITP_IOCTL_GET_HEARTBEAT        78  ///< CtrlBoard codec get heartbeat
#define ITP_IOCTL_WIFI_BEST_CHANNEL    79  ///<wifi find best channel
#define ITP_IOCTL_WIFI_STANUM          80  ///<wifi sta num
#define ITP_IOCTL_SWUARTTX_INIT        81  ///<arm swuart tx init
#define ITP_IOCTL_SWUARTRX_INIT        82  ///<arm swuart rx init
#define ITP_IOCTL_REG_RS485_CB         83  ///<Register RS485 callback.
#define ITP_IOCTL_REG_RS485_DEFER_CB   84  ///<Register RS485 deferred callback.
#define ITP_IOCTL_REG_UART_CB          85  ///<Register UART callback.
#define ITP_IOCTL_REG_UART_DEFER_CB    86  ///<Register UART deferred callback.
#define ITP_IOCTL_UART_TIMEOUT         87  ///<UART Timeout
#define ITP_IOCTL_ARP_REPLY            88  ///<ARP reply
#define ITP_IOCTL_IP_DUPLICATE         89  ///<IP Duplicate
#define ITP_IOCTL_NAND_CHECK_MAC_AREA  90  ///< NAND check MAC address area
#define ITP_IOCTL_REG_IOEX_CB          91  ///<Register GPIO(IoEx) callback.
#define ITP_IOCTL_RESET_DEFAULT        92  ///<Ethernet reset to netif default
#define ITP_IOCTL_WIFI_ADD_NETIF       93  ///< wifi add netif
#define ITP_IOCTL_WIFI_NETIF_STATUS    94  ///< wifi netif status
#define ITP_IOCTL_SLEEP_CONTINUE	   95  ///< SleepMode using double click.
#define ITP_IOCTL_UART_SET_BOOT	       96  ///< Uart set boot mode.
#define ITP_IOCTL_ETH_DHCP             97  ///< Ethernet open DHCP.
#define ITP_IOCTL_WIFI_DRV_REINIT      98  ///< Re-init driver after Wifi dongle hot plugin.
#define ITP_IOCTL_WIFI_NETIF_SHUTDOWN  99  ///< All Wifi netif shutdown and remove.
#define ITP_IOCTL_CUSTOM_CTL_ID0      100  ///< For Custom device usage.
#define ITP_IOCTL_CUSTOM_CTL_ID1      101  ///< For Custom device usage.
#define ITP_IOCTL_CUSTOM_CTL_ID2      102  ///< For Custom device usage.
#define ITP_IOCTL_CUSTOM_CTL_ID3      103  ///< For Custom device usage.
#define ITP_IOCTL_CUSTOM_CTL_ID4      104  ///< For Custom device usage.
#define ITP_IOCTL_CUSTOM_CTL_ID5      105  ///< For Custom device usage.
#define ITP_IOCTL_CUSTOM_CTL_ID6      106  ///< For Custom device usage.
#define ITP_IOCTL_CUSTOM_CTL_ID7      107  ///< For Custom device usage.


// Device definitions
extern const ITPDevice   itpDevicePrintBuf;       ///< Print buffer device
extern const ITPDevice   itpDeviceSwUart;         ///< Software UART device
extern const ITPDevice   itpDeviceUart0;          ///< UART #0 device
extern const ITPDevice   itpDeviceUart1;          ///< UART #1 device
extern const ITPDevice   itpDeviceUart2;          ///< UART #2 device
extern const ITPDevice   itpDeviceUart3;          ///< UART #3 device
extern const ITPDevice   itpDeviceLcdConsole;     ///< LCD console device
extern const ITPDevice   itpDeviceOsdConsole;     ///< LCD OSD console device
extern const ITPDevice   itpDeviceScreen;         ///< Screen device
extern const ITPDevice   itpDeviceI2c0;           ///< I2C #0 device
extern const ITPDevice   itpDeviceI2c1;           ///< I2C #1 device
extern const ITPDevice   itpDeviceI2c2;           ///< I2C #2 device (SW IIC)
extern const ITPDevice   itpDeviceSpi0;           ///< SPI #0 device
extern const ITPDevice   itpDeviceSpi1;           ///< SPI #1 device
extern const ITPDevice   itpDeviceIr;             ///< IR device
extern const ITPDevice   itpDeviceNand;           ///< NAND device
extern const ITPDevice   itpDeviceXd;             ///< XD device
extern const ITPDevice   itpDeviceNor;            ///< NOR device
extern const ITPDevice   itpDeviceSd0;            ///< SD #0 device
extern const ITPDevice   itpDeviceSd1;            ///< SD #1 device
extern const ITPDevice   itpDeviceUsb;            ///< USB device
extern const ITPDevice   itpDeviceUsbdFsg;        ///< USB acts as a USB Mass Storage device
extern const ITPDevice   itpDeviceCard;           ///< Card device
extern const ITPDevice   itpDeviceDrive;          ///< Drive device
extern const ITPDevice   itpDeviceKeypad;         ///< Keypad device
extern const ITPDevice   itpDevicePower;          ///< Power device
extern const ITPDevice   itpDeviceGSensor;        ///< G-Sensor device
extern const ITPDevice   itpDeviceHeadset;        ///< Headset device
extern const ITPDevice   itpDeviceAmplifier;      ///< Amplifier device
extern const ITPDevice   itpDeviceStc;            ///< STC device
extern const ITPDevice   itpDeviceDecompress;     ///< Decompress device
extern const ITPDevice   itpDeviceDpu;            ///< Dpu device(for encryption/descryption)
extern const ITPDevice   itpDeviceCodec;          ///< Audio Codec device
extern const ITPDevice   itpDeviceEthernet;       ///< Ethernet device
extern const ITPDevice   itpDeviceWifi;           ///< WiFi device
extern const ITPDevice   itpDeviceSocket;         ///< Socket device
extern const ITPFSDevice itpFSDeviceFile;         ///< File device
extern const ITPDevice   itpDeviceDemod;          ///< Demod device
extern const ITPDevice   itpDeviceWatchDog;       ///< Watch Dog device
extern const ITPDevice   itpDeviceNetConsole;     ///< Network console device
extern const ITPDevice   itpDeviceLed;            ///< LED device
extern const ITPDevice   itpDeviceSwitch;         ///< Switch device
extern const ITPDevice   itpDeviceTuner;          ///< Tuner device
extern const ITPDevice   itpDeviceStnLcd;         ///< STN LCD device
extern const ITPFSDevice itpFSDeviceFat;          ///< FAT device
extern const ITPFSDevice itpFSDeviceNtfs;         ///< NTFS device
extern const ITPDevice   itpDeviceUsbMouse;       ///< Usb Mouse device
extern const ITPDevice   itpDeviceUsbKbd;         ///< Usb Keyboard device
extern const ITPDevice   itpDeviceRtc;            ///< RTC device
extern const ITPDevice   itpDeviceBacklight;      ///< Backlight device
extern const ITPDevice   itpDeviceGpioExpander;   ///< GPIO expander device
extern const ITPDevice   itpDeviceRS485_0;        ///< RS485_0 device
extern const ITPDevice   itpDeviceRS485_1;        ///< RS485_1 device
extern const ITPDevice   itpDeviceRS485_2;        ///< RS485_2 device
extern const ITPDevice   itpDeviceRS485_3;        ///< RS485_3 device
extern const ITPDevice   itpDeviceRS485_4;        ///< RS485_4 device
extern const ITPDevice   itpDeviceSwUartCodec;    ///< Software UART codec device
extern const ITPDevice   itpDeviceWiegand0;       ///< Wiegand #0 device
extern const ITPDevice   itpDeviceWiegand1;       ///< Wiegand #1 device
extern const ITPDevice   itpCtrlBoard;            ///< CtrlBoard device
//extern const ITPDevice itpDeviceArmSwUartCodec;  ///< Arm swuart device
extern const ITPDevice   itpDeviceSwUartCodecDbg; ///< DBG swuart device
extern const ITPDevice   itpDeviceSdio;           ///< SDIO device
extern const ITPDevice   itpDeviceRGBtoMIPI; 	  ///< RGBToMIPI
extern const ITPDevice   itpDeviceIoEX; 	  	  ///< IoEX device
extern const ITPDevice   itpDeviceLogDisk;        ///< Log to disk device
extern const ITPDevice   itpDeviceUsbdIdb;        ///< USB acts as a USB device
extern const ITPDevice   itpDeviceAltCpu;         ///< Software peripheral on ALT CPU

/** @defgroup itp_card Card
 *  @{
 */

/**
 * Card type definitions.
 */
typedef enum
{
    ITP_CARD_SD0   = (0x1 << 0),    ///< SD #0
    ITP_CARD_SD1   = (0x1 << 1),    ///< SD #1
    ITP_CARD_CF    = (0x1 << 2),    ///< CF
    ITP_CARD_MS    = (0x1 << 3),    ///< MS
    ITP_CARD_XD    = (0x1 << 4),    ///< xD
    ITP_CARD_MSC00 = (0x1 << 5),    ///< USB #0, MSC #0
    ITP_CARD_MSC01 = (0x1 << 6),    ///< USB #0, MSC #1
    ITP_CARD_MSC02 = (0x1 << 7),    ///< USB #0, MSC #2
    ITP_CARD_MSC03 = (0x1 << 8),    ///< USB #0, MSC #3
    ITP_CARD_MSC04 = (0x1 << 9),    ///< USB #0, MSC #4
    ITP_CARD_MSC05 = (0x1 << 10),   ///< USB #0, MSC #5
    ITP_CARD_MSC06 = (0x1 << 11),   ///< USB #0, MSC #6
    ITP_CARD_MSC07 = (0x1 << 12),   ///< USB #0, MSC #7
    ITP_CARD_MSC10 = (0x1 << 13),   ///< USB #1, MSC #0
    ITP_CARD_MSC11 = (0x1 << 14),   ///< USB #1, MSC #1
    ITP_CARD_MSC12 = (0x1 << 15),   ///< USB #1, MSC #2
    ITP_CARD_MSC13 = (0x1 << 16),   ///< USB #1, MSC #3
    ITP_CARD_MSC14 = (0x1 << 17),   ///< USB #1, MSC #4
    ITP_CARD_MSC15 = (0x1 << 18),   ///< USB #1, MSC #5
    ITP_CARD_MSC16 = (0x1 << 19),   ///< USB #1, MSC #6
    ITP_CARD_MSC17 = (0x1 << 20),   ///< USB #0, MSC #7

    ITP_CARD_MAX   = 21
} ITPCard;

/**
 * Card status definition.
 */
typedef struct
{
    ITPCard card;     ///< Card type
    bool    inserted; ///< Inserted or not
} ITPCardStatus;

/** @} */ // end of ith_card

/** @defgroup itp_drive Drive
 *  @{
 */
#define ITP_MAX_DRIVE 26    ///< Maximum disk drive count

/**
 * Disk type definitions.
 */
typedef enum
{
    ITP_DISK_SD0   = 0,     ///< SD #0
    ITP_DISK_SD1   = 1,     ///< SD #1
    ITP_DISK_CF    = 2,     ///< CF
    ITP_DISK_MS    = 3,     ///< MS
    ITP_DISK_XD    = 4,     ///< xD
    ITP_DISK_NAND  = 5,     ///< NAND
    ITP_DISK_NOR   = 6,     ///< NOR
    ITP_DISK_MSC00 = 7,     ///< USB #0, MSC #0
    ITP_DISK_MSC01 = 8,     ///< USB #0, MSC #1
    ITP_DISK_MSC02 = 9,     ///< USB #0, MSC #2
    ITP_DISK_MSC03 = 10,    ///< USB #0, MSC #3
    ITP_DISK_MSC04 = 11,    ///< USB #0, MSC #4
    ITP_DISK_MSC05 = 12,    ///< USB #0, MSC #5
    ITP_DISK_MSC06 = 13,    ///< USB #0, MSC #6
    ITP_DISK_MSC07 = 14,    ///< USB #0, MSC #7
    ITP_DISK_MSC10 = 15,    ///< USB #1, MSC #0
    ITP_DISK_MSC11 = 16,    ///< USB #1, MSC #1
    ITP_DISK_MSC12 = 17,    ///< USB #1, MSC #2
    ITP_DISK_MSC13 = 18,    ///< USB #1, MSC #3
    ITP_DISK_MSC14 = 19,    ///< USB #1, MSC #4
    ITP_DISK_MSC15 = 20,    ///< USB #1, MSC #5
    ITP_DISK_MSC16 = 21,    ///< USB #1, MSC #6
    ITP_DISK_MSC17 = 22,    ///< USB #0, MSC #7
    ITP_DISK_RAM   = 23,    ///< RAMDISK

    ITH_DISK_MAX
} ITPDisk;

/**
 * Drive status definition.
 */
typedef struct
{
    ITPDisk       disk;      ///< Disk type
    ITPDeviceType device;    ///< Device type
    int           avail;     ///< Is presented or not
    char          name[4];   ///< Drive name. Ex: "C:/"
    int           writable;  ///< Is writable or not
    int           removable; ///< Is removable or not
} ITPDriveStatus;

// Partition
#define ITP_MAX_PARTITION 10    ///< Maximum disk partition count

/**
 * Disk partition definition.
 */
typedef struct
{
    ITPDisk  disk;                      ///< Disk type.
    int      count;                     ///< Partitions count.
    uint64_t size[ITP_MAX_PARTITION];   ///< Array of partitions size.
    uint64_t start[ITP_MAX_PARTITION];  ///< Array of partitions start.
} ITPPartition;

/** @} */ // end of ith_drive

/** @defgroup itp_nand
 *  @{
 */
/**
 * the definations of operating mode when open NAND device.
 */
#define	ITP_NAND_RAW_MODE	0x0		///< open ITP NAND device with raw data mode
#define	ITP_NAND_FTL_MODE	0x1		///< open ITP NAND device with FTL mode

/** @} */ // end of itp_nand

/** @defgroup itp_usbdevice Usb Device
 *  @{
 */
/**
 * USB device state definitions.
 */
typedef enum
{
    ITP_USB_DEVICE_UNKNOWN,     ///< USB device is unknown
    ITP_USB_DEVICE_PLUGGED,     ///< USB device is plugged
    ITP_USB_DEVICE_UNPLUGGED    ///< USB device is un-plugged
} ITPUsbDeviceState;

/** @} */ // end of itp_usbdevice

/** @defgroup itp_power Power
 *  @{
 */

/**
 * Power state definitions.
 */
typedef enum
{
    ITP_POWER_NORMAL,       ///< Normal
    ITP_POWER_STANDBY,      ///< Standby
    ITP_POWER_SLEEP,        ///< Sleep
    ITP_POWER_HIBERNATION,  ///< Hibernation
    ITP_POWER_SUSPEND,      ///< Suspend
    ITP_POWER_OFF           ///< Off
} ITPPowerState;

/**
 * Battery state definitions.
 */
typedef enum
{
    ITP_BATTERY_UNKNOWN,      ///< Cannot determine power status
    ITP_BATTERY_ON_BATTERY,   ///< Not plugged in, running on the battery
    ITP_BATTERY_NO_BATTERY,   ///< Plugged in, no battery available
    ITP_BATTERY_CHARGING,     ///< Plugged in, charging battery
    ITP_BATTERY_CHARGED       ///< Plugged in, battery charged
} ITPBatteryState;

/**
 * Power status definitions.
 */
typedef struct
{
    ITPPowerState   powerState;     ///< Power state
    ITPBatteryState batteryState;   ///< Battery state
    int             batteryPercent; ///< Battery percent (0-100)
} ITPPowerStatus;

/**
 * SWUART Parity definitions.
 */
typedef enum
{
    ITP_SWUART_NONE,
    ITP_SWUART_ODD,
    ITP_SWUART_EVEN,
} ITPSwuartParity;

/**
 * Initialize battery module.
 */
void itpBatteryInit(void);

/**
 * Gets battery state.
 *
 * @return Battery state.
 */
ITPBatteryState itpBatteryGetState(void);

/**
 * Gets battery percent.
 *
 * @return Battery percent, 0-100.
 */
int itpBatteryGetPercent(void);

/** @} */ // end of itp_power

/** @defgroup itp_keypad Keypad
 *  @{
 */
#define ITP_KEYPAD_DOWN   0x1   ///< Key down flag
#define ITP_KEYPAD_UP     0x2   ///< Key up flag
#define ITP_KEYPAD_REPEAT 0x4   ///< Key repeat flag

/**
 * Keypad event definition.
 */
typedef struct
{
    struct timeval time;    ///< Received time.
    int            code;    ///< Key code.
    uint32_t       flags;   ///< Key flags.
} ITPKeypadEvent;

/**
 * Initializes keypad module.
 */
void itpKeypadInit(void);

/**
 * Probes the input of keypad.
 */
int itpKeypadProbe(void);

/**
 * Gets the max keycode count of keypad.
 */
int itpKeypadGetMaxLevel(void);

/** @} */ // end of itp_keypad

/** @defgroup itp_usb_mouse Mouse
 *  @{
 */
#define ITP_MOUSE_LBTN_DOWN 0x01    ///< Mouse left button down
#define ITP_MOUSE_RBTN_DOWN 0x02    ///< Mouse right button down
#define ITP_MOUSE_MBTN_DOWN 0x04    ///< Mouse middle button down
#define ITP_MOUSE_SBTN_DOWN 0x08    ///< Mouse side button down
#define ITP_MOUSE_LBTN_UP   0x10    ///< Mouse left button up
#define ITP_MOUSE_RBTN_UP   0x20    ///< Mouse right button up
#define ITP_MOUSE_MBTN_UP   0x40    ///< Mouse middle button up
#define ITP_MOUSE_SBTN_UP   0x80    ///< Mouse side button up

/**
 * Mouse event definition.
 */
typedef struct
{
    uint8_t flags;          ///< Button flags.
    int8_t  x;              ///< X displacement
    int8_t  y;              ///< Y displacement
    int8_t  wheel;          ///< wheel
} ITPMouseEvent;

/** @} */ // end of itp_usb_mouse

/** @defgroup itp_keyboard Keyboard
 *  @{
 */
#define ITP_KEYDOWN 0x1     ///< Key down flag
#define ITP_KEYUP   0x2     ///< Key up flag

/**
 * Keyboard event definition.
 */
typedef struct
{
    uint32_t flags;         ///< Key flags.
    uint32_t code;          ///< Key code.
} ITPKeyboardEvent;

/** @} */ // end of itp_keyboard

/** @defgroup itp_gsensor G-Sensor
 *  @{
 */
/**
 * Initialize G-Sensor module.
 */
void itpGSensorInit(void);

/**
 * Gets G-Sensor value.
 *
 * @return G-Sensor value.
 */
int itpGSensorGetValue(void);

/** @} */ // end of itp_gsensor

/** @defgroup itp_headset Headset
 *  @{
 */
/**
 * Initializes headset module.
 */
void itpHeadsetInit(void);

/**
 * Gets headset value.
 *
 * @return headset value.
 */
int itpHeadsetGetValue(void);

/** @} */ // end of itp_headset

/** @defgroup itp_amplifier Amplifier
 *  @{
 */
/**
 * Initializes amplifier module.
 */
void itpAmplifierInit(void);

/**
 * Enables amplifier.
 */
void itpAmplifierEnable(void);

/**
 * Disables amplifier.
 */
void itpAmplifierDisable(void);

/**
 * Mutes amplifier.
 */
void itpAmplifierMute(void);

/**
 * Un-mutes amplifier.
 */
void itpAmplifierUnmute(void);

/** @} */ // end of itp_amplifier

/** @defgroup itp_rtc RTC
 *  @{
 */
/**
 * Initializes RTC module.
 */
void itpRtcInit(void);

/**
 * Gets RTC time.
 *
 * @param usec micro second to get if available.
 * @return rtc second value.
 */
long itpRtcGetTime(long *usec);

/**
 * Sets RTC time.
 *
 * @param sec second to set.
 * @param usec micro second to set.
 */
void itpRtcSetTime(long sec, long usec);

#ifdef	CFG_RTC_REDUCE_IO_ACCESS_ENABLE
/**
 * Initializes assist RTC.
 */
void assistRtcInit(void);

/**
 * Gets assist RTC time.
 *
 * @param us micro second to get if available.
 * @return rtc second value.
 */
long assistRtcGetTime(long *us);

/**
 * Sets assist RTC time.
 *
 * @param sec second to set.
 * @param us micro second to set.
 */
void assistRtcSetTime(long sec, long us);
#endif

/** @} */ // end of itp_rtc

/** @defgroup itp_screen Screen
 *  @{
 */
/**
 * TV-out type definitions.
 */
typedef enum
{
    ITP_TVOUT_LCD,      ///< LCD
    ITP_TVOUT_NTSC,     ///< NTSC
    ITP_TVOUT_PAL,      ///< PAL
    ITP_TVOUT_NTSC_I,   ///< Interleave NTSC
    ITP_TVOUT_PAL_I,    ///< Interleave PAL
    ITP_TVOUT_NTSC_P,   ///< Progress NTSC
    ITP_TVOUT_PAL_P     ///< Progress PAL
} ITPTvOut;

/**
 * STN LCD information definition.
 */
typedef struct
{
    uint32_t width;             ///< Width
    uint32_t height;            ///< Height
    uint32_t pitch;             ///< Pitch
    uint8_t  *buf;              ///< Frame buffer
} ITPStnLcdInfo;

/**
 * Initializes STN LCD module.
 */
void itpStnLcdInit(void);

/**
 * Enables STN LCD.
 */
void itpStnLcdEnable(void);

/**
 * Enables STN LCD.
 */
void itpStnLcdDisable(void);

/**
 * Sets STN LCD brightness.
 *
 * @param val the brightness value.
 */
void itpStnLcdSetBrightness(unsigned int val);

/**
 * Draws a pixel to STN LCD.
 *
 * @param x the x-coordinate.
 * @param y the y-coordinate.
 * @param color the color value.
 */
void itpStnLcdDrawPixel(int x, int y, unsigned int color);

/**
 * Draws a 1-bpp bitmap to STN LCD.
 *
 * @param x the x-coordinate.
 * @param y the y-coordinate.
 * @param bitmap the bitmap array.
 * @param w the bitmap width.
 * @param h the bitmap height.
 */
void itpStnLcdDrawBitmap(int x, int y, const uint8_t *bitmap, unsigned int w, unsigned int h);

/**
 * Draws a character to STN LCD.
 *
 * @param x the x-coordinate.
 * @param y the y-coordinate.
 * @param c the character.
 */
void itpStnLcdDrawChar(int x, int y, char c);

/**
 * Draws a string to STN LCD.
 *
 * @param x the x-coordinate.
 * @param y the y-coordinate.
 * @param text the string.
 */
void itpStnLcdDrawText(int x, int y, const char *text);

/**
 * Draws a line to STN LCD.
 *
 * @param x0 the x-coordinate of start point.
 * @param y0 the y-coordinate of start point.
 * @param x1 the x-coordinate of end point.
 * @param y1 the y-coordinate of end point.
 * @param color the color value.
 */
void itpStnLcdDrawLine(int x0, int y0, int x1, int y1, unsigned int color);

/**
 * Draws a rectangle to STN LCD.
 *
 * @param x the x-coordinate.
 * @param y the y-coordinate.
 * @param w the width.
 * @param h the height.
 * @param color the color value.
 */
void itpStnLcdDrawRect(int x, int y, unsigned int w, unsigned int h, unsigned int color);

/**
 * Fills a rectangle to STN LCD.
 *
 * @param x the x-coordinate.
 * @param y the y-coordinate.
 * @param w the width.
 * @param h the height.
 * @param color the color value.
 */
void itpStnLcdFillRect(int x, int y, unsigned int w, unsigned int h, unsigned int color);

/**
 * Writes STN LCD screen buffer.
 *
 * @param buf the screen buffer.
 */
void itpStnLcdWriteBuffer(uint8_t *buf);

/** @} */ // end of itp_screen

/** @defgroup itp_ethernet Ethernet
 *  @{
 */
#define ITP_ETH_LINKUP 0x1  ///< Ethernet link-up flag
#define ITP_ETH_ACTIVE 0x2  ///< Ethernet active flag

/**
 * Ethernet information definition.
 */
typedef struct
{
    int      index;              ///< Adapter index
    uint32_t flags;              ///< Flags
    uint32_t address;            ///< IP address
    uint32_t netmask;            ///< netmask
    char     displayName[256];   ///< display name
    uint8_t  hardwareAddress[8]; ///< hardware address
    char     name[16];           ///< short name
} ITPEthernetInfo;

/**
 * Ethernet setting definition.
 */
typedef struct
{
    int     index;      ///< Adapter index
    int     dhcp;       ///< Is DHCP enable or not
    int     autoip;     ///< Is AUTOIP enable or not
    uint8_t ipaddr[4];  ///< IP address
    uint8_t netmask[4]; ///< Netmask
    uint8_t gw[4];      ///< Gateway
} ITPEthernetSetting;

/**
 * Initializes Ethernet lwIP module.
 */
void itpEthernetLwipInit(void);

#define ITP_PHY_WOL     0x1

/** @} */ // end of itp_ethernet

/** @defgroup itp_phy PHY
*  @{
*/
/**
* Initializes PHY device.
*/
void PhyInit(int ethMode);

/**
* Check interrupt status for link change.
* Call from mac driver's internal ISR for phy's interrupt.
*/
extern int(*itpPhyLinkChange)(void);
/**
* Replace mac driver's ISR for phy's interrupt.
*/
extern ITHGpioIntrHandler itpPhylinkIsr;
/**
* Returns 0 if the device reports link status up/ok
*/
extern int(*itpPhyReadMode)(int* speed, int* duplex);
/**
* Get link status.
*/
extern uint32_t(*itpPhyLinkStatus)(void);

/** @} */ // end of itp_phy

/** @defgroup itp_wifi WiFi
 *  @{
 */
#define ITP_WIFI_LINKUP 0x1  ///< wifi link-up flag
#define ITP_WIFI_ACTIVE 0x2  ///< wifi active flag

/**
 * WiFi information definition.
 */
typedef struct
{
    int      active;              ///< Is active or not
    uint32_t address;             ///< IP address
    uint32_t netmask;             ///< netmask
    char     displayName[256];    ///< display name
    char     hardwareAddress[32]; ///< hardware address
    char     name[16];            ///< short name
} ITPWifiInfo;

/**
 * Wifi setting definition.
 */

#define WIFI_SEC_NOSEC        0
#define WIFI_SEC_WEP          1
#define WIFI_SEC_WPAPSK       2        /** default is TKIP */
#define WIFI_SEC_WPA2PSK      3        /** default is AES */
#define WIFI_SEC_WPAPSK_AES   4
#define WIFI_SEC_WPA2PSK_TKIP 5
#define WIFI_SEC_WPAPSK_MIX   6
#define WIFI_SEC_WPS          7

#define WIFI_WPS_PIN          0       /** not support */
#define WIFI_WPS_PBC          1
#define WIFI_WPS_PIN_START    2       /** not support */
#define WIFI_WPS_STOP         3

typedef struct {
    char          ssidName[32]; /*SSID name*/
    int           securityMode; /*Sec. Mode*/
    unsigned char preShareKey[64];
    int           wpsMode;
    void (*write_config_cb)(void *config);  /*for WPS write config to file*/
} ITPWifiSetting;

/**
 * Initializes WiFi lwIP module.
 */
void itpWifiLwipInit(void);
void itpWifiLwipInitNetif(void);

void itpWifiapLwipInit(void);

/** @} */ // end of itp_wifi

/** @defgroup itp_spi SPI
 *  @{
 */
typedef enum
{
    ITP_SPI_PIO_READ,
    ITP_SPI_DMA_READ,
    ITP_SPI_PIO_WRITE,
    ITP_SPI_DMA_WRITE,
} ITPSpiReadWriteFunc;

typedef struct
{
    ITPSpiReadWriteFunc readWriteFunc;
    void                *cmdBuffer;
    uint32_t            cmdBufferSize;
    void                *dataBuffer;
    uint32_t            dataBufferSize;
    uint8_t             dataLength;
} ITPSpiInfo;

/** @} */ // end of itp_spi

/** @defgroup itp_i2c I2C
 *  @{
 */
typedef struct
{
    uint8_t  slaveAddress;
    uint8_t  *cmdBuffer;
    uint32_t cmdBufferSize;
    uint8_t  *dataBuffer;
    uint32_t dataBufferSize;
    uint32_t errCode;
} ITPI2cInfo;

/** @} */ // end of itp_i2c

/** @defgroup itp_demod Demod
 *  @{
 */
// Demod channel
#define ITP_MAX_TS_SERVICE           500    ///< Maximum ts service count
#define ITP_MAX_TS_SERVICE_NAME_SIZE 512    ///< Maximum ts service name size
#define ITP_MAX_DEMOD_SUPPORT_CNT    2      ///< Maximum amount of demode supported

/** @defgroup itp_ts country frequency ID
 *  @{
 */
/**
 * Counrty ID definitions.
 */
typedef enum
{
    ITP_TS_COUNTRY_NONE,        ///< unknow
    ITP_TS_COUNTRY_AUSTRALIA,   ///< Australia
    ITP_TS_COUNTRY_AUSTRIA,     ///< Austria
    ITP_TS_COUNTRY_CHINA,       ///< China
    ITP_TS_COUNTRY_FRANCE,      ///< France
    ITP_TS_COUNTRY_GERMANY,     ///< Germany
    ITP_TS_COUNTRY_GREECE,      ///< Greece
    ITP_TS_COUNTRY_HUNGARY,     ///< Hungary
    ITP_TS_COUNTRY_ITALY,       ///< Italy
    ITP_TS_COUNTRY_NETHERLANDS, ///< Netherlands
    ITP_TS_COUNTRY_POLAND,      ///< Poland
    ITP_TS_COUNTRY_PORTUGAL,    ///< Portugal
    ITP_TS_COUNTRY_RUSSIAN,     ///< Russian
    ITP_TS_COUNTRY_SPAIN,       ///< Spain
    ITP_TS_COUNTRY_TAIWAN,      ///< Taiwan
    ITP_TS_COUNTRY_UK,          ///< Uk
    ITP_TS_COUNTRY_DENMARK,     ///< Denmark
    ITP_TS_COUNTRY_SWEDEN,      ///< Sweden
    ITP_TS_COUNTRY_FINLAND,     ///< Finland

    ITP_TS_COUNTRY_CNT,         ///< total country count (ITP_TS_COUNTRY_CNT - 1)
} ITP_TS_COUNTRY_ID;

/** @} */ // end of itp_ts

/**
 * Demod information definition.
 */
typedef struct
{
    uint32_t blockSize;             ///< Block size
    uint32_t frequency;             ///< ts channel frequency
    uint32_t bandwidth;             ///< ts channel bandwidth
    int32_t  ts_service_cnt;        ///< ts service count

    struct {                        ///< ts service info table
        uint32_t service_order_index;
        bool     bVideoService;
        uint16_t service_name[ITP_MAX_TS_SERVICE_NAME_SIZE];
        int      totalSoundTracks;
    } ts_service_table[ITP_MAX_TS_SERVICE];

    int32_t ts_country_cnt;         ///< ts country count
    struct {                        ///< ts country table
        ITP_TS_COUNTRY_ID countryId;
        char              country_name[32];
    } ts_country_table[ITP_TS_COUNTRY_CNT - 1];
} ITPDemodInfo;

/** @} */ // end of itp_demod

/** @defgroup itp_usb USB
 *  @{
 */
/**
 * USB information definition.
 */
typedef struct
{
    bool host;                   ///< true:get host's info, false:get device info
    int  usbIndex;               ///< 0:USB0, 1:USB1
    void *ctxt;                  ///< the usb device's context
    int  type;                   ///< type of the plugged usb device
    bool b_device;               ///< true: current role is device mode, false: current role is host mode. It is meaningful when host is set to false.
} ITPUsbInfo;

/** @} */ // end of itp_usb

/** @defgroup itp_dpu DPU
 *  @{
 */
typedef enum
{
    ITP_DPU_AES_MODE    = (0),
    ITP_DPU_DES_MODE    = (1),
    ITP_DPU_DES3_MODE   = (2),
    ITP_DPU_CSA_MODE    = (3),
    ITP_DPU_CRC_MODE    = (4),
    ITP_DPU_UNKNOW_MODE = (5),
} ITPDpuModeEnum;

typedef enum
{
    ITP_DPU_CIPHER_ECB    = (0),
    ITP_DPU_CIPHER_CBC    = (1),
    ITP_DPU_CIPHER_CFB    = (2),
    ITP_DPU_CIPHER_OFB    = (3),
    ITP_DPU_CIPHER_CTR    = (4),
    ITP_DPU_UNKNOW_CIPHER = (5),
} ITPDpuCipher;

typedef struct
{
    uint32_t keylen;
    uint32_t *pkey;
} ITPDpuKey;

typedef struct
{
    uint32_t ivlen;
    uint32_t *piv;
} ITPDpuVector;

typedef struct
{
    uint8_t  dpuMode;
    uint8_t  cipher;
    uint8_t  descrypt;
    uint8_t  crc_master;
    uint32_t dpuLen;
    uint32_t srcLen;
    uint32_t dstLen;
    uint32_t keyLen;
    uint32_t vctLen;
    uint8_t  *srcBuf;
    uint8_t  *dstBuf;
    uint8_t  *swBuf;
    uint8_t  *keyBuf;
    uint8_t  *vctBuf;
} ITP_DPU_INFO;

/** @} */ // end of itp_dpu

/** @} */ // end of itp_device

/** @defgroup itp_vmem Video Memory Management
 *  @{
 */

/**
 * Allocates video memory.
 *
 * @param size The amount of memory you want to allocate, in bytes.
 * @return Allocated memory address, or 0 if an error occurred
 */
uint32_t itpVmemAlloc(size_t size);

/**
 * The itpVmemAlignedAlloc function is similar to the memalign function in that
 * it returns a video memory of size bytes aligned to a multiple of alignment.
 *
 * @param alignment The alignment that you want to use for the memory. This must be a multiple of size( void *).
 * @param size The amount of memory you want to allocate, in bytes.
 * @return Allocated memory address, or 0 if an error occurred
 */
uint32_t itpVmemAlignedAlloc(size_t alignment, size_t size);

/**
 * Releases allocated video memory.
 *
 * @param addr The allocated video memory address.
 */
void itpVmemFree(uint32_t addr);

/**
 * Prints the video memory management status.
 */
void itpVmemStats(void);

/**
 * Allocates write-through memory.
 *
 * @param size The size to allocate.
 * @return Allocated memory address.
 */
uint32_t itpWTAlloc(size_t size);

/**
 * Releases allocated write-through memory.
 *
 * @param addr The allocated write-through memory address.
 */
void itpWTFree(uint32_t addr);

/**
 * Prints the write-through memory management status.
 */
void itpWTStats(void);

/** @} */ // end of itp_vmem

/** @defgroup itp_stats Statistic
 *  @{
 */

/**
 * Initializes statistic module.
 */
void itpStatsInit(void);

/**
 * File system statistic definition.
 */
typedef struct
{
    uint32_t readSize;      ///< Read size
    uint32_t readTime;      ///< Read time (ms)
    uint32_t writeSize;     ///< Written size
    uint32_t writeTime;     ///< Written time (ms)
} ITPFSStats;

extern ITPFSStats itpStatsFat;  ///< FAT statistic
extern ITPFSStats itpStatsNtfs; ///< NTFS statistic

/**
 * Initializes Task Dump
 */
void itpTaskVcdInit(void);

/**
 * Open Task Dump file
 * @par Example:
 * @code
    itpTaskVcdOpen("a:/dump.vcd", 8192); // record 8192 events maximum

    ...
    itpTaskVcdSetTag(0, 1);
    ...

    ...
    itpTaskVcdSetTag(1, 1);
    ...

    if (itpTaskVcdGetEventCount() == 8192)
        itpTaskVcdWrite();

 * @endcode
 */
void itpTaskVcdOpen(const char *filename, int count);

/**
 * Get Currnet Record Number
 */
int itpTaskVcdGetEventCount(void);

/**
 * Set Task Tag
 */
void itpTaskVcdSetTag(int id, int tag);

/**
 * Write Task Dump file
 */
void itpTaskVcdWrite(void);

/**
 * Close Task Dump file
 */
void itpTaskVcdClose(void);

/** @} */ // end of itp_stats

/** @defgroup itp_defer Deferred Interrupt Handling
 *  @{
 */

/**
 * Pointer to pend function.
 */
typedef void (*ITPPendFunction) (void* arg1, uint32_t arg2);

/**
 * Used from application interrupt service routines to defer the execution of a function to the RTOS daemon task.
 *
 * @param func The function to execute from the timer service/ daemon task.
 * @param arg1 The value of the callback function's first parameter.
 * @param arg2 The value of the callback function's second parameter.
 */
void itpPendFunctionCallFromISR(ITPPendFunction func, void* arg1, uint32_t arg2);

/** @} */ // end of itp_defer

#ifdef __cplusplus
}
#endif

#endif // ITE_ITP_H
/** @} */ // end of itp

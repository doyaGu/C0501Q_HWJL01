/** @file
 * ITE SD Driver API header file.
 *
 * @author Irene Lin
 * @copyright ITE Tech.Inc.All Rights Reserved.
 */

#ifndef ITE_SD_H
#define ITE_SD_H

#include "ite/ith.h"

#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup driver_sd SD
 *  @{
 */

/**
 * SD controller index definition.
 */
#define SD_0       0
#define SD_1       1
#define SD_NUM   2

/**
 * SD state.
 */
typedef enum SD_CARD_STATE_TAG
{
    SD_INIT_OK,
    SD_INSERTED
}SD_CARD_STATE;


/**
 * File system must call this API first when initializing a volume.
 *
 */
int iteSdInitializeEx(int index);

/**
 * This routine is used to release any resources associated with a drive when it is removed.
 *
 */
int iteSdTerminateEx(int index);

/**
 * This routine is used to read a series of sectors from the targe device.
 *
 * @param sector		 the start sector to be read.
 * @param count		 sector count.
 * @param data		 read buffer.

 * @return zero if succeed, otherwise return non-zero error codes.
 */
int iteSdReadMultipleSectorEx(int index, uint32_t sector, int count, void* data);

/**
 * This routine is used to write a series of sectors to the targe device.
 *
 * @param sector		 the start sector to be write.
 * @param count		 sector count.
 * @param data		 write buffer.

 * @return zero if succeed, otherwise return non-zero error codes.
 */
int iteSdWriteMultipleSectorEx(int index, uint32_t sector, int count, void* data);

/**
 * This routine is used to get the number of sectors and block size.
 *
 * @param sectorNum		 the total sector number.
 * @param blockLength		 the block length.
 */
int iteSdGetCapacityEx(int index, uint32_t* sectorNum, uint32_t* blockLength);

/**
 * This routine is used to get the device status.
 *
 * @param state		 the state to be get
 *.
 * @return the state status.
 */
int iteSdGetCardStateEx(int index, int state);   

/**
 * This routine is used to check if card is in write-protected status.
 */
bool iteSdIsLockEx(int index);

/** @} */ // end of driver_sd



#if defined(CFG_MMC_ENABLE)
struct sdio_func_info {
    int func_type;
#define SDIO_AMEBA	0x01

	void *ctxt;
};

/**
 * SD card information.
 */
typedef struct 
{
	int type;
#define SD_TYPE_MMC			0		/* MMC card */
#define SD_TYPE_SD			1		/* SD card */
#define SD_TYPE_SDIO		2		/* SDIO card */
#define SD_TYPE_SD_COMBO	3		/* SD combo (IO+mem) card */

	void *ctxt;

    struct sdio_func_info sdio_info[3];
    int sdio_drv_num;
	int sdc_idx;
} SD_CARD_INFO;


/**
 * Initialize the sd controller and get the card information.
 *
 * @param index		 the controller index.
 * @param card_info	 the related card information.
 */
int iteSdcInitialize(int index, SD_CARD_INFO *card_info);

/**
 * Terminate the related resource.
 *
 * @param index		 the controller index.
 */
int iteSdcTerminate(int index);

/**
 * Get the card type.
 *
 * @param index		 the controller index.
 */
int iteSdcGetType(int index);

#endif


#ifdef __cplusplus
}
#endif

#endif /* ITE_SD_H */

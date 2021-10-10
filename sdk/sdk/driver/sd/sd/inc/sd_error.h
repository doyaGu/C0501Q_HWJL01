/*
 * Copyright (c) 2007 ITE Corp. All Rights Reserved.
 */
/** @file
 *  Use as SD error code header file.
 *
 * @author Irene Lin
 */

#ifndef SD_ERROR_H
#define SD_ERROR_H


#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================

#define ERROR_SD_BASE                                  0xA000

#define ERROR_SD_CREATE_SEMAPHORE_FAIL                 (ERROR_SD_BASE + 0x0001)
#define ERROR_SD_NO_CARD_INSERTED                      (ERROR_SD_BASE + 0x0002)
#define ERROR_SD_IS_WRITE_PROTECT                      (ERROR_SD_BASE + 0x0003)
#define ERROR_SD_SEND_CMD_TIMEOUT                      (ERROR_SD_BASE + 0x0004)
#define ERROR_SD_SUPPORT_CMD8_BUT_FAIL                 (ERROR_SD_BASE + 0x0005)
#define ERROR_SD_SEND_ACMD41_TIMEOUT                   (ERROR_SD_BASE + 0x0006)
#define ERROR_MMC_SEND_CMD1_TIMEOUT                    (ERROR_SD_BASE + 0x0007)
#define ERROR_MMC_SEND_CMD2_TIMEOUT                    (ERROR_SD_BASE + 0x0008)
#define ERROR_SD_RCA_STATUS_ERROR                      (ERROR_SD_BASE + 0x0009)
#define ERROR_SD_SEND_RCA_TIMEOUT                      (ERROR_SD_BASE + 0x000A)
#define ERROR_MMC_SET_RCA_MORE_THAN_10                 (ERROR_SD_BASE + 0x000B)
#define ERROR_SD_SEND_CSD_FAIL                         (ERROR_SD_BASE + 0x000C)
#define ERROR_SD_SEND_CMD7_FAIL                        (ERROR_SD_BASE + 0x000D)
#define ERROR_SD_PUT_INTO_TRANSFER_STATE_FAIL          (ERROR_SD_BASE + 0x000E)
#define ERROR_SD_BUS_WIDTH_ERROR                       (ERROR_SD_BASE + 0x000F)
#define ERROR_SD_CMD6_ERROR                            (ERROR_SD_BASE + 0x0010)
#define ERROR_SD_SEND_CMD6_TIMEOUT                     (ERROR_SD_BASE + 0x0011)
#define ERROR_MMC_HC_TOTAL_SECTORS_ERROR               (ERROR_SD_BASE + 0x0012)
#define ERROR_SD_CRC_ERROR                             (ERROR_SD_BASE + 0x0013)
#define ERROR_SD_ALLOC_TMP_SYSTEM_MEMORY_FAIL          (ERROR_SD_BASE + 0x0014)
#define ERROR_SD_WAIT_FIFO_FULL_TIMEOUT                (ERROR_SD_BASE + 0x0015)
#define ERROR_SD_WAIT_FIFO_EMPTY_TIMEOUT               (ERROR_SD_BASE + 0x0016)
#define ERROR_SD_ALLOC_TMP_VRAM_FAIL                   (ERROR_SD_BASE + 0x0017)
#define ERROR_SD_GO_IDLE_STATE_FAIL                    (ERROR_SD_BASE + 0x0018)
#define ERROR_SD_WRAP_TIMEOUT                          (ERROR_SD_BASE + 0x0019)
#define ERROR_MMC_TEST_BUS_FAIL                        (ERROR_SD_BASE + 0x001A)
#define ERROR_SD_CMD_RESP_TIMEOUT                      (ERROR_SD_BASE + 0x001B)
#define ERROR_SD_REQUEST_DMA_FAIL                      (ERROR_SD_BASE + 0x001C)
#define ERROR_SD_DMA_TIMEOUT                           (ERROR_SD_BASE + 0x001D)
#define ERROR_SD_NO_INIT                               (ERROR_SD_BASE + 0x001E)


#ifdef __cplusplus
}
#endif

#endif

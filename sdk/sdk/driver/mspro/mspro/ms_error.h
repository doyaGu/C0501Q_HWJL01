/*
 * Copyright (c) 2007 ITE Corp. All Rights Reserved.
 */
/** @file
 *  Use as MS error code header file.
 *
 * @author Irene Lin
 */

#ifndef MS_ERROR_H
#define MS_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================

#define ERROR_MS_BASE                                 0x80000000  //(MMP_MODULE_MS << MMP_ERROR_OFFSET)

#define ERROR_MS_SW_RESET_FAIL                        (ERROR_MS_BASE + 0x0001)
#define ERROR_MS_NO_CARD_INSERTED                     (ERROR_MS_BASE + 0x0002)
#define ERROR_MS_WAIT_CMD_COMPLETE                    (ERROR_MS_BASE + 0x0003)
#define ERROR_MS_WAIT_BREQ_TIMEOUT_P                  (ERROR_MS_BASE + 0x0004)
#define ERROR_MS_WAIT_BREQ_FAIL_S                     (ERROR_MS_BASE + 0x0005)
#define ERROR_MS_WAIT_CED_TIMEOUT_P                   (ERROR_MS_BASE + 0x0006)
#define ERROR_MS_WAIT_CED_FAIL_S                      (ERROR_MS_BASE + 0x0007)
#define ERROR_MS_2STATE_ACCESS_MODE                   (ERROR_MS_BASE + 0x0008)
#define ERROR_MS_CRC_ERROR                            (ERROR_MS_BASE + 0x0009)
#define ERROR_MS_WAIT_FIFO_FULL_TIMEOUT               (ERROR_MS_BASE + 0x000A)
#define ERROR_MS_WAIT_FIFO_EMPTY_TIMEOUT              (ERROR_MS_BASE + 0x000B)
#define ERROR_MS_ALLOC_TMP_SYSTEM_MEMORY_FAIL         (ERROR_MS_BASE + 0x000C)
#define ERROR_MS_MEDIA_TYPE_ERROR                     (ERROR_MS_BASE + 0x000D)
//#define ERROR_MS_CREATE_SEMAPHORE_FAIL                  (ERROR_MS_BASE + 0x000E)
#define ERROR_MS_CONFIRM_CPU_STARTUP_FAIL1            (ERROR_MS_BASE + 0x000F)
//#define ERROR_MS_CONFIRM_CPU_STARTUP_FAIL2              (ERROR_MS_BASE + 0x0010)
#define ERROR_MS_UNABLE_CORRECT_DATA_FLAG             (ERROR_MS_BASE + 0x0011)
#define ERROR_MS_OVERWRITE_FLAG_ERROR                 (ERROR_MS_BASE + 0x0012)
#define ERROR_MS_MANAGEMENT_FLAG_ERROR                (ERROR_MS_BASE + 0x0013)
#define ERROR_MS_IS_WRITE_PROTECT                     (ERROR_MS_BASE + 0x0014)
#define ERROR_MS_GET_EXTRA_DATA_CMDNK                 (ERROR_MS_BASE + 0x0015)
#define ERROR_MS_GET_EXTRA_DATA_UNKNOWN_STATUS        (ERROR_MS_BASE + 0x0016)
#define ERROR_MS_READ_PAGE_CMDNK                      (ERROR_MS_BASE + 0x0017)
#define ERROR_MS_READ_PAGE_UNKNOWN_STATUS             (ERROR_MS_BASE + 0x0018)
#define ERROR_MS_READ_BLOCK_CMDNK                     (ERROR_MS_BASE + 0x0019)
#define ERROR_MS_READ_BLOCK_ERROR_END                 (ERROR_MS_BASE + 0x001A)
#define ERROR_MS_READ_BLOCK_UNKNOWN_STATUS            (ERROR_MS_BASE + 0x001B)
#define ERROR_MS_ALLOC_WRITE_TMP_BUF_FAIL             (ERROR_MS_BASE + 0x001C)
#define ERROR_MS_ERASE_BLOCK_CMDNK                    (ERROR_MS_BASE + 0x001D)
#define ERROR_MS_ERASE_BLOCK_FAIL                     (ERROR_MS_BASE + 0x001E)
#define ERROR_MS_ERASE_BLOCK_UNKNOWN_STATUS           (ERROR_MS_BASE + 0x001F)
#define ERROR_MS_WRITE_BLOCK_CMDNK                    (ERROR_MS_BASE + 0x0020)
#define ERROR_MS_WRITE_BLOCK_ERROR_END                (ERROR_MS_BASE + 0x0021)
#define ERROR_MS_WRITE_BLOCK_UNKNOWN_STATUS           (ERROR_MS_BASE + 0x0022)
#define ERROR_MS_WRITE_PAGE_CMDNK                     (ERROR_MS_BASE + 0x0023)
#define ERROR_MS_WRITE_PAGE_ERROR                     (ERROR_MS_BASE + 0x0024)
#define ERROR_MS_WRITE_PAGE_UNKNOWN_STATUS            (ERROR_MS_BASE + 0x0025)
#define ERROR_MS_MS_IO_EXPANDED_MODULE                (ERROR_MS_BASE + 0x0026)
#define ERROR_MS_COPY_BLOCK_ERROR_COUNT               (ERROR_MS_BASE + 0x0027)
#define ERROR_MS_SEARCH_SEGMENT_FAIL                  (ERROR_MS_BASE + 0x0028)
#define ERROR_MS_NO_AVAILABE_BLOCK                    (ERROR_MS_BASE + 0x0029)
#define ERROR_MS_WRITE_EXTRA_DATA_CMDNK               (ERROR_MS_BASE + 0x002A)
#define ERROR_MS_FLASH_WRITE_ERROR_OCCURRED           (ERROR_MS_BASE + 0x002B)
#define ERROR_MS_WRITE_EXTRA_DATA_UNKNOWN_STATUS      (ERROR_MS_BASE + 0x002C)
#define ERROR_MS_NOT_FOUND_BOOT_BLOCK                 (ERROR_MS_BASE + 0x002D)
#define ERROR_MS_BOOT_CONTENT_CLASS_ERROR             (ERROR_MS_BASE + 0x002E)
#define ERROR_MS_BOOT_CONTENT_DEVICE_TYPE_ERROR       (ERROR_MS_BASE + 0x002F)
#define ERROR_MS_BOOT_CONTENT_FORMAT_TYPE_ERROR       (ERROR_MS_BASE + 0x0030)
#define ERROR_MS_BOOT_CONTENT_BLOCK_NUM_ERROR         (ERROR_MS_BASE + 0x0031)
#define ERROR_MS_BOOT_CONTENT_EFFECT_NUM_ERROR        (ERROR_MS_BASE + 0x0032)
#define ERROR_MS_BOOT_CONTENT_BLOCK_SIZE_ERROR        (ERROR_MS_BASE + 0x0033)
#define ERROR_MS_WAIT_INT_TIMEOUT_P                   (ERROR_MS_BASE + 0x0034)
#define ERROR_MS_CMD_DEVICE_FAIL                      (ERROR_MS_BASE + 0x0035)
#define ERROR_MS_PRO_SIGNATURE_CODE_ERROR             (ERROR_MS_BASE + 0x0036)
#define ERROR_MS_PRO_ENTRY_COUNT_ERROR                (ERROR_MS_BASE + 0x0037)
#define ERROR_MS_PRO_NO_SYSTEM_INFO                   (ERROR_MS_BASE + 0x0038)
#define ERROR_MS_PRO_SYSTEM_INFO_SIZE_ERROR           (ERROR_MS_BASE + 0x0039)
#define ERROR_MS_PRO_SYSTEM_INFO_ADDR_ERROR           (ERROR_MS_BASE + 0x003A)
#define ERROR_MS_PRO_SYSTEM_INFO_ADDR_ERROR1          (ERROR_MS_BASE + 0x003B)
#define ERROR_MS_PRO_SYSTEM_INFO_CLASS_ERROR          (ERROR_MS_BASE + 0x003C)
#define ERROR_MS_PRO_SYSTEM_INFO_DEVICE_TYPE_ERROR    (ERROR_MS_BASE + 0x003D)
#define ERROR_MS_PRO_NO_VALID_ENTRY                   (ERROR_MS_BASE + 0x003E)
#define ERROR_MS_PRO_NO_IDENTIFY_DEVICE_INFO          (ERROR_MS_BASE + 0x003F)
#define ERROR_MS_PRO_IDENTIFY_DEVICE_INFO_SIZE_ERROR  (ERROR_MS_BASE + 0x0040)
#define ERROR_MS_PRO_IDENTIFY_DEVICE_INFO_ADDR_ERROR  (ERROR_MS_BASE + 0x0041)
#define ERROR_MS_PRO_IDENTIFY_DEVICE_INFO_ADDR_ERROR1 (ERROR_MS_BASE + 0x0042)
#define ERROR_MS_PRO_NO_PBR_FAT1216                   (ERROR_MS_BASE + 0x0043)
#define ERROR_MS_PRO_PBR_FAT1216_SIZE_ERROR           (ERROR_MS_BASE + 0x0044)
#define ERROR_MS_PRO_PBR_FAT1216_ADDR_ERROR           (ERROR_MS_BASE + 0x0045)
#define ERROR_MS_PRO_PBR_FAT1216_ADDR_ERROR1          (ERROR_MS_BASE + 0x0046)
#define ERROR_MS_PRO_NO_PBR_FAT32                     (ERROR_MS_BASE + 0x0047)
#define ERROR_MS_PRO_PBR_FAT32_SIZE_ERROR             (ERROR_MS_BASE + 0x0048)
#define ERROR_MS_PRO_PBR_FAT32_ADDR_ERROR             (ERROR_MS_BASE + 0x0049)
#define ERROR_MS_PRO_PBR_FAT32_ADDR_ERROR1            (ERROR_MS_BASE + 0x004A)
#define ERROR_MS_PRO_READ_CMDNK                       (ERROR_MS_BASE + 0x004B)
#define ERROR_MS_PRO_READ_UNKNOWN_STATUS              (ERROR_MS_BASE + 0x004C)
#define ERROR_MS_CONFIRM_CPU_STARTUP_MEDIA_ERROR      (ERROR_MS_BASE + 0x004D)
#define ERROR_MS_PRO_ENTRY_SIZE_ERROR                 (ERROR_MS_BASE + 0x004E)
#define ERROR_MS_PRO_ENTRY_ID_NOT_MATCH               (ERROR_MS_BASE + 0x004F)
#define ERROR_MS_PRO_ENTRY_START_ERROR                (ERROR_MS_BASE + 0x0050)
#define ERROR_MS_PRO_READ_ATTRIB_CMDNK                (ERROR_MS_BASE + 0x0051)
#define ERROR_MS_PRO_WRITE_CMDNK                      (ERROR_MS_BASE + 0x0052)
#define ERROR_MS_PRO_WRITE_ERROR                      (ERROR_MS_BASE + 0x0053)
#define ERROR_MS_PRO_WRITE_UNKNOWN_STATUS             (ERROR_MS_BASE + 0x0054)
#define ERROR_MS_WAIT_INT_TIMEOUT_S                   (ERROR_MS_BASE + 0x0055)
#define ERROR_MS_INVALID_OVERWRITE_METHOD             (ERROR_MS_BASE + 0x0056)
#define ERROR_MS_OVERWRITE_CMDNK                      (ERROR_MS_BASE + 0x0057)
#define ERROR_MS_OVERWRITE_UNKNOWN_STATUS             (ERROR_MS_BASE + 0x0058)
#define ERROR_MS_OVERWRITE_ERROR                      (ERROR_MS_BASE + 0x0059)
#define ERROR_MS_NOT_FIND_UNUSED_BLOCK                (ERROR_MS_BASE + 0x005A)
#define ERROR_MS_IS_READ_PROTECT                      (ERROR_MS_BASE + 0x005B)
#define ERROR_MS_ALLOC_SYS_INFO_MEM_FAIL              (ERROR_MS_BASE + 0x005C)
#define ERROR_MS_FILE_SYSTEM_FAIL                     (ERROR_MS_BASE + 0x005D)
#define ERROR_MS_ALLOC_TMP_VRAM_FAIL                  (ERROR_MS_BASE + 0x005E)
#define ERROR_MS_NOT_SUPPORT_MS_SERIAL_ONLY           (ERROR_MS_BASE + 0x005F)
#define ERROR_MS_CREATE_ISR_EVENT_FAIL                (ERROR_MS_BASE + 0x0060)
#define ERROR_MS_IRQ_WHAT_HAPPEN                      (ERROR_MS_BASE + 0x0061)
#define ERROR_MS_IRQ_DMA_TIMEOUT                      (ERROR_MS_BASE + 0x0062)
#define ERROR_MS_REQUEST_DMA_FAIL                     (ERROR_MS_BASE + 0x0063)

#ifdef __cplusplus
}
#endif

#endif
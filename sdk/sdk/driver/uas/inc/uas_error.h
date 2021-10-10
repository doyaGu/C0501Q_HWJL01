/*
 * Copyright (c) 2010 ITE Corp. All Rights Reserved.
 */
/** @file
 *  Use as UAS error code header file.
 *
 * @author Irene Lin
 */

#ifndef UAS_ERROR_H
#define UAS_ERROR_H


#define ERROR_UAS_BASE                                       0xCA000000

#define ERROR_UAS_NODEV                         (ERROR_UAS_BASE + 0x0001)
#define ERROR_UAS_INVALID_LUN                   (ERROR_UAS_BASE + 0x0002)
#define ERROR_UAS_ALLOC_SCSI_CMD                (ERROR_UAS_BASE + 0x0003)
#define ERROR_UAS_CMD_TIMEOUT                   (ERROR_UAS_BASE + 0x0004)
#define ERROR_UAS_DEVICE_BUSY                   (ERROR_UAS_BASE + 0x0005)
#define ERROR_UAS_ALLOC_SENSE_URB               (ERROR_UAS_BASE + 0x0006)
#define ERROR_UAS_SUBMIT_SENSE_URB              (ERROR_UAS_BASE + 0x0007)
#define ERROR_UAS_ALLOC_CMD_URB                 (ERROR_UAS_BASE + 0x0008)
#define ERROR_UAS_SUBMIT_CMD_URB                (ERROR_UAS_BASE + 0x0009)
#define ERROR_UAS_BUSY                          (ERROR_UAS_BASE + 0x000A)
#define ERROR_UAS_ALLOC_DATA_URB                (ERROR_UAS_BASE + 0x000B)
#define ERROR_UAS_SUBMIT_DATA_IN_URB            (ERROR_UAS_BASE + 0x000C)
#define ERROR_UAS_DEVICE_TYPE                   (ERROR_UAS_BASE + 0x000D)
#define ERROR_UAS_OUT_OF_RANGE                  (ERROR_UAS_BASE + 0x000E)
#define ERROR_UAS_SUBMIT_DATA_OUT_URB           (ERROR_UAS_BASE + 0x000F)
#define ERROR_UAS_TMF_SUBMIT_SENSE_URB          (ERROR_UAS_BASE + 0x0010)
#define ERROR_UAS_TMF_SUBMIT_TASK_MGNT_URB      (ERROR_UAS_BASE + 0x0011)
#define ERROR_UAS_ALLOC_TASK_MGMT_URB           (ERROR_UAS_BASE + 0x0012)
#define ERROR_UAS_ALLOC_TASK_MGMT_IU            (ERROR_UAS_BASE + 0x0013)
#define ERROR_UAS_UNSUPPORT_TASK_MGMT_FUNC      (ERROR_UAS_BASE + 0x0014)
#define ERROR_UAS_INVALID_SECTOR                (ERROR_UAS_BASE + 0x0015)






#endif

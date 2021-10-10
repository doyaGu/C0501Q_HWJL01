/*
 * Copyright (c) 2008 ITE Technology Corp. All Rights Reserved.
 */
/** @file scsi.h
 *
 * @author Irene Lin
 */
#ifndef SCSI_H
#define SCSI_H

//=============================================================================
//                              Constant Definition
//=============================================================================

/**
 * SCSI opcodes
 */
#define TEST_UNIT_READY                0x00
#define REZERO_UNIT                    0x01
#define REQUEST_SENSE                  0x03
#define FORMAT_UNIT                    0x04
#define READ_BLOCK_LIMITS              0x05
#define REASSIGN_BLOCKS                0x07
#define READ_6                         0x08
#define WRITE_6                        0x0a
#define SEEK_6                         0x0b
#define READ_REVERSE                   0x0f
#define WRITE_FILEMARKS                0x10
#define SPACE                          0x11
#define INQUIRY                        0x12
#define RECOVER_BUFFERED_DATA          0x14
#define MODE_SELECT                    0x15
#define RESERVE                        0x16
#define RELEASE                        0x17
#define COPY                           0x18
#define ERASE                          0x19
#define MODE_SENSE                     0x1a
#define START_STOP                     0x1b
#define RECEIVE_DIAGNOSTIC             0x1c
#define SEND_DIAGNOSTIC                0x1d
#define ALLOW_MEDIUM_REMOVAL           0x1e

#define SET_WINDOW                     0x24
#define READ_CAPACITY                  0x25
#define READ_10                        0x28
#define WRITE_10                       0x2a
#define SEEK_10                        0x2b
#define WRITE_VERIFY                   0x2e
#define VERIFY                         0x2f
#define SEARCH_HIGH                    0x30
#define SEARCH_EQUAL                   0x31
#define SEARCH_LOW                     0x32
#define SET_LIMITS                     0x33
#define PRE_FETCH                      0x34
#define READ_POSITION                  0x34
#define SYNCHRONIZE_CACHE              0x35
#define LOCK_UNLOCK_CACHE              0x36
#define READ_DEFECT_DATA               0x37
#define MEDIUM_SCAN                    0x38
#define COMPARE                        0x39
#define COPY_VERIFY                    0x3a
#define WRITE_BUFFER                   0x3b
#define READ_BUFFER                    0x3c
#define UPDATE_BLOCK                   0x3d
#define READ_LONG                      0x3e
#define WRITE_LONG                     0x3f
#define CHANGE_DEFINITION              0x40
#define WRITE_SAME                     0x41
#define READ_TOC                       0x43
#define LOG_SELECT                     0x4c
#define LOG_SENSE                      0x4d
#define MODE_SELECT_10                 0x55
#define RESERVE_10                     0x56
#define RELEASE_10                     0x57
#define MODE_SENSE_10                  0x5a
#define PERSISTENT_RESERVE_IN          0x5e
#define PERSISTENT_RESERVE_OUT         0x5f
#define REPORT_LUNS                    0xa0
#define MOVE_MEDIUM                    0xa5
#define READ_12                        0xa8
#define WRITE_12                       0xaa
#define WRITE_VERIFY_12                0xae
#define SEARCH_HIGH_12                 0xb0
#define SEARCH_EQUAL_12                0xb1
#define SEARCH_LOW_12                  0xb2
#define READ_ELEMENT_STATUS            0xb8
#define SEND_VOLUME_TAG                0xb6
#define WRITE_LONG_2                   0xea
#define READ_16                        0x88
#define WRITE_16                       0x8a
#define VERIFY_16                      0x8f
#define SERVICE_ACTION_IN              0x9e

/**
 * Device Types
 */
#define TYPE_DISK                      0x00
#define TYPE_TAPE                      0x01
#define TYPE_PRINTER                   0x02
#define TYPE_PROCESSOR                 0x03 /* HP scanners use this */
#define TYPE_WORM                      0x04 /* Treated as ROM by our system */
#define TYPE_ROM                       0x05
#define TYPE_SCANNER                   0x06
#define TYPE_MOD                       0x07 /* Magneto-optical disk - treated as TYPE_DISK */
#define TYPE_MEDIUM_CHANGER            0x08
#define TYPE_COMM                      0x09 /* Communications device */
#define TYPE_ENCLOSURE                 0x0d /* Enclosure Services Device */
#define TYPE_NO_LUN                    0x7f

/**
 * SCSI command data length
 */
#define DATA_LENGTH_INQUIRY            36
#define DATA_LENGTH_MODE_SENSE         192
#define DATA_LENGTH_REQUEST_SENSE      18
#define DATA_LENGTH_READ_CAPACITY      8

/**
 * SCSI command length
 */
#define CB_LENGTH_TEST_UNIT_READY      0x06
#define CB_LENGTH_REQUEST_SENSE        0x0C
#define CB_LENGTH_INQUIRY              0x06
#define CB_LENGTH_MODE_SENSE6          0x06
#define CB_LENGTH_READ_FORMAT_CAPACITY 0x0A
#define CB_LENGTH_READ_CAPACITY        0x0A
#define CB_LENGTH_ALLOW_MEDIUM_REMOVAL 0x06
#define CB_LENGTH_READ_10              0x0A
#define CB_LENGTH_WRITE_10             0x0A

/**
 * Status code from lower level driver, for scsi_cmnd->result "HIGH WORD"
 */
#define DID_OK                         0x00 /* NO error                                */
#define DID_NO_CONNECT                 0x01 /* Couldn't connect before timeout period  */
#define DID_BUS_BUSY                   0x02 /* BUS stayed busy through time out period */
#define DID_TIME_OUT                   0x03 /* TIMED OUT for other reason              */
#define DID_BAD_TARGET                 0x04 /* BAD target.                             */
#define DID_ABORT                      0x05 /* Told to abort for some other reason     */
#define DID_PARITY                     0x06 /* Parity error                            */
#define DID_ERROR                      0x07 /* Internal error                          */
#define DID_RESET                      0x08 /* Reset by somebody.                      */
#define DID_BAD_INTR                   0x09 /* Got an interrupt we weren't expecting.  */
#define DID_PASSTHROUGH                0x0a /* Force command past mid-layer            */
#define DID_SOFT_ERROR                 0x0b /* The low level driver just wish a retry  */
// +wlHsu
#define DID_HW_HANG                    0x0c
// -wlHsu
#define DRIVER_OK                      0x00 /* Driver status                           */

/**
 *  Status codes, for scsi_cmnd->result "LOW WORD"
 */
#define GOOD                           0x00
#define CHECK_CONDITION                0x01
#define CONDITION_GOOD                 0x02
#define BUSY                           0x04
#define STATUS_MASK                    0x0F

/*
 *  SENSE KEYS
 */
#define NO_SENSE                       0x00
#define RECOVERED_ERROR                0x01
#define NOT_READY                      0x02
#define MEDIUM_ERROR                   0x03
#define HARDWARE_ERROR                 0x04
#define ILLEGAL_REQUEST                0x05
#define UNIT_ATTENTION                 0x06
#define DATA_PROTECT                   0x07
#define BLANK_CHECK                    0x08
#define COPY_ABORTED                   0x0a
#define ABORTED_COMMAND                0x0b
#define VOLUME_OVERFLOW                0x0d
#define MISCOMPARE                     0x0e

/* Additional Sense Code (ASC) */
#define NO_ADDITIONAL_SENSE            0x0
#define LOGICAL_UNIT_NOT_READY         0x4
#define UNRECOVERED_READ_ERR           0x11
#define PARAMETER_LIST_LENGTH_ERR      0x1a
#define INVALID_OPCODE                 0x20
#define ADDR_OUT_OF_RANGE              0x21
#define INVALID_COMMAND_OPCODE         0x20
#define INVALID_FIELD_IN_CDB           0x24
#define INVALID_FIELD_IN_PARAM_LIST    0x26
#define POWERON_RESET                  0x29
#define SAVING_PARAMS_UNSUP            0x39
#define TRANSPORT_PROBLEM              0x4b
#define THRESHOLD_EXCEEDED             0x5d
#define LOW_POWER_COND_ON              0x5

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct scsi_cmnd
{
    MMP_UINT32 serial_number; /* CBW Tag */
    MMP_UINT8  lun;
    MMP_UINT8  cmd_len;

#define SCSI_DATA_WRITE       1
#define SCSI_DATA_READ        2
#define SCSI_DATA_NONE        3
    MMP_UINT8 sc_data_direction;

    /* These elements define the operation we are about to perform */
#define MAX_COMMAND_SIZE      16
    MMP_UINT8  cmnd[MAX_COMMAND_SIZE];
    MMP_UINT32 request_bufflen;             /* Actual request size */
    MMP_UINT8  *request_buffer;             /* Actual requested buffer */

#define SCSI_MAX_RESULT_SIZE  256
    MMP_UINT8 scsi_result[SCSI_MAX_RESULT_SIZE];

#define SCSI_SENSE_BUFFERSIZE 64
    MMP_UINT8 sense_buffer[SCSI_SENSE_BUFFERSIZE]; /* obtained by REQUEST SENSE
                                                    * when CHECK CONDITION is
                                                    * received on original command
                                                    * (auto-sense) */
    MMP_INT result;                                /* Status code from lower level driver */
} Scsi_Cmnd;

typedef struct ScsiDevice
{
    MMP_UINT8  vendorInfo[8];
    MMP_UINT8  productId[16];
    MMP_UINT8  productRev[4];
    MMP_UINT8  deviceType;
    MMP_UINT8  removable;
    MMP_UINT8  inquiried;

    MMP_UINT8  writeProtect;             /* Save Write Protection information */
    MMP_UINT32 blockSize;                /* Get by READ CAPACITY command */
    MMP_UINT32 blockTotalNum;
} Scsi_Device;

struct us_data;

MMP_INT scsi_test_unit_ready(struct us_data *us, MMP_UINT8 lun);
MMP_INT scsi_inquiry(struct us_data *us, MMP_UINT8 lun);
MMP_INT scsi_read_capacity(struct us_data *us, MMP_UINT8 lun);
MMP_INT scsi_mode_sense6(struct us_data *us, MMP_UINT8 lun);
MMP_INT scsi_read10(struct us_data *us, MMP_UINT8 lun, MMP_UINT32 sector, MMP_INT count, void *data);
MMP_INT scsi_write10(struct us_data *us, MMP_UINT8 lun, MMP_UINT32 sector, MMP_INT count, void *data);

#endif
#ifndef UAS_SCSI_H
#define UAS_SCSI_H


/**
 * SCSI opcodes
 */
#define TEST_UNIT_READY       0x00
#define REZERO_UNIT           0x01
#define REQUEST_SENSE         0x03
#define FORMAT_UNIT           0x04
#define READ_BLOCK_LIMITS     0x05
#define REASSIGN_BLOCKS       0x07
#define READ_6                0x08
#define WRITE_6               0x0a
#define SEEK_6                0x0b
#define READ_REVERSE          0x0f
#define WRITE_FILEMARKS       0x10
#define SPACE                 0x11
#define INQUIRY               0x12
#define RECOVER_BUFFERED_DATA 0x14
#define MODE_SELECT           0x15
#define RESERVE               0x16
#define RELEASE               0x17
#define COPY                  0x18
#define ERASE                 0x19
#define MODE_SENSE            0x1a
#define START_STOP            0x1b
#define RECEIVE_DIAGNOSTIC    0x1c
#define SEND_DIAGNOSTIC       0x1d
#define ALLOW_MEDIUM_REMOVAL  0x1e

#define SET_WINDOW            0x24
#define READ_CAPACITY         0x25
#define READ_10               0x28
#define WRITE_10              0x2a
#define SEEK_10               0x2b
#define WRITE_VERIFY          0x2e
#define VERIFY                0x2f
#define SEARCH_HIGH           0x30
#define SEARCH_EQUAL          0x31
#define SEARCH_LOW            0x32
#define SET_LIMITS            0x33
#define PRE_FETCH             0x34
#define READ_POSITION         0x34
#define SYNCHRONIZE_CACHE     0x35
#define LOCK_UNLOCK_CACHE     0x36
#define READ_DEFECT_DATA      0x37
#define MEDIUM_SCAN           0x38
#define COMPARE               0x39
#define COPY_VERIFY           0x3a
#define WRITE_BUFFER          0x3b
#define READ_BUFFER           0x3c
#define UPDATE_BLOCK          0x3d
#define READ_LONG             0x3e
#define WRITE_LONG            0x3f
#define CHANGE_DEFINITION     0x40
#define WRITE_SAME            0x41
#define READ_TOC              0x43
#define LOG_SELECT            0x4c
#define LOG_SENSE             0x4d
#define MODE_SELECT_10        0x55
#define RESERVE_10            0x56
#define RELEASE_10            0x57
#define MODE_SENSE_10         0x5a
#define PERSISTENT_RESERVE_IN 0x5e
#define PERSISTENT_RESERVE_OUT 0x5f
#define REPORT_LUNS           0xa0
#define MOVE_MEDIUM           0xa5
#define READ_12               0xa8
#define WRITE_12              0xaa
#define WRITE_VERIFY_12       0xae
#define SEARCH_HIGH_12        0xb0
#define SEARCH_EQUAL_12       0xb1
#define SEARCH_LOW_12         0xb2
#define READ_ELEMENT_STATUS   0xb8
#define SEND_VOLUME_TAG       0xb6
#define WRITE_LONG_2          0xea
#define READ_16               0x88
#define WRITE_16              0x8a
#define VERIFY_16	          0x8f
#define SERVICE_ACTION_IN     0x9e

/**
 * Device Types
 */
#define TYPE_DISK               0x00
#define TYPE_TAPE               0x01
#define TYPE_PRINTER            0x02
#define TYPE_PROCESSOR          0x03    /* HP scanners use this */
#define TYPE_WORM               0x04    /* Treated as ROM by our system */
#define TYPE_ROM                0x05
#define TYPE_SCANNER            0x06
#define TYPE_MOD                0x07    /* Magneto-optical disk - treated as TYPE_DISK */
#define TYPE_MEDIUM_CHANGER     0x08
#define TYPE_COMM               0x09    /* Communications device */
#define TYPE_ENCLOSURE          0x0d    /* Enclosure Services Device */
#define TYPE_NO_LUN             0x7f

/**
 * SCSI command data length
 */
#define DATA_LENGTH_INQUIRY			36
#define DATA_LENGTH_MODE_SENSE		192
#define DATA_LENGTH_REQUEST_SENSE	18
#define DATA_LENGTH_READ_CAPACITY	8

/**
 * SCSI command length
 */
#define CB_LENGTH_TEST_UNIT_READY       0x06
#define CB_LENGTH_REQUEST_SENSE         0x0C
#define CB_LENGTH_INQUIRY               0x06
#define CB_LENGTH_MODE_SENSE6           0x06
#define CB_LENGTH_READ_FORMAT_CAPACITY  0x0A
#define CB_LENGTH_READ_CAPACITY         0x0A
#define CB_LENGTH_ALLOW_MEDIUM_REMOVAL  0x06
#define CB_LENGTH_READ_10               0x0A
#define CB_LENGTH_WRITE_10              0x0A
#define CB_LENGTH_REPORT_LUNS           0x0C


/*
 * ScsiLun: 8 byte LUN.
 */
struct scsi_lun 
{
	uint8_t scsi_lun[8];
};

struct scsi_cmnd
{
#define MAX_COMMAND_SIZE    16
    uint8_t     cmnd[MAX_COMMAND_SIZE];
    uint8_t     *transfer_buffer;
    uint32_t    transfer_len;

    uint32_t    cmd_len:8;
#define DMA_BIDIRECTIONAL   0
#define DMA_TO_DEVICE       1
#define DMA_FROM_DEVICE     2
#define DMA_NONE            3
    uint32_t    sc_data_direction:4;
    uint32_t    lun:4;
    uint32_t    tagged:1;

    int         tag;
    int         result;

#define SCSI_SENSE_BUFFERSIZE   96
    uint8_t     *sense_buffer;
    uint8_t     *scsi_buffer;  /* internal used buffer, need free */

    sem_t       sem;
    void (*scsi_done)(struct scsi_cmnd *);
    struct uas_dev_info *devinfo;
    uint8_t cmdinfo[64];  /* struct uas_cmd_info */
};

struct scsi_device
{
    uint32_t device_type:8;
    uint32_t removable:1;
    uint32_t write_protect:1;

    uint32_t block_num;
    uint32_t block_size;
};

int uscsi_init(struct uas_dev_info *devinfo);
void uscsi_release(struct uas_dev_info*);
int uscsi_test_unit_ready(struct uas_dev_info *devinfo, int lun);
int uscsi_report_luns(struct uas_dev_info *devinfo);
int uscsi_inquiry(struct uas_dev_info *devinfo, int lun);
int uscsi_read_capacity(struct uas_dev_info *devinfo, int lun);
int uscsi_read10(struct uas_dev_info *devinfo, int lun, uint32_t sector, int count, void* data);
int uscsi_write10(struct uas_dev_info *devinfo, int lun, uint32_t sector, int count, void* data);


#endif

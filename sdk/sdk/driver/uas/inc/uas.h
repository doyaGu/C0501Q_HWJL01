#ifndef UAS_H
#define UAS_H

#include <linux/spinlock.h>

/*
 * spec is big-endian
 */

/* Common header for all IUs */
struct iu 
{
	uint8_t iu_id;
	uint8_t rsvd1;
	uint16_t tag;
};

enum 
{
	IU_ID_COMMAND		= 0x01,
	IU_ID_STATUS		= 0x03,
	IU_ID_RESPONSE		= 0x04,
	IU_ID_TASK_MGMT		= 0x05,
	IU_ID_READ_READY	= 0x06,
	IU_ID_WRITE_READY	= 0x07,
};

enum 
{
	TMF_ABORT_TASK          = 0x01,
	TMF_ABORT_TASK_SET      = 0x02,
	TMF_CLEAR_TASK_SET      = 0x04,
	TMF_LOGICAL_UNIT_RESET  = 0x08,
	TMF_I_T_NEXUS_RESET     = 0x10,
	TMF_CLEAR_ACA           = 0x40,
	TMF_QUERY_TASK          = 0x80,
	TMF_QUERY_TASK_SET      = 0x81,
	TMF_QUERY_ASYNC_EVENT   = 0x82,
};

enum 
{
	RC_TMF_COMPLETE         = 0x00,
	RC_INVALID_INFO_UNIT    = 0x02,
	RC_TMF_NOT_SUPPORTED    = 0x04,
	RC_TMF_FAILED           = 0x05,
	RC_TMF_SUCCEEDED        = 0x08,
	RC_INCORRECT_LUN        = 0x09,
	RC_OVERLAPPED_TAG       = 0x0a,
};

struct command_iu 
{
	uint8_t iu_id;
	uint8_t rsvd1;
	uint16_t tag;
	uint8_t prio_attr;
	uint8_t rsvd5;
	uint8_t len;
	uint8_t rsvd7;
	struct scsi_lun lun;
	uint8_t cdb[16];
};

struct task_mgmt_iu 
{
	uint8_t iu_id;
	uint8_t rsvd1;
	uint16_t tag;
	uint8_t function;
	uint8_t rsvd2;
	uint16_t task_tag;
	struct scsi_lun lun;
};


/*
 * Also used for the Read Ready and Write Ready IUs since they have the
 * same first four bytes
 */
struct sense_iu {
	uint8_t  iu_id;
	uint8_t  rsvd1;
	uint16_t tag;
	uint16_t status_qual;
	uint8_t  status;
	uint8_t  rsvd7[7];
	uint16_t len;
#define SCSI_SENSE_BUFFERSIZE   96
	uint8_t  sense[SCSI_SENSE_BUFFERSIZE];
};

struct response_ui 
{
	uint8_t iu_id;
	uint8_t rsvd1;
	uint16_t tag;
	uint16_t add_response_info;
	uint8_t response_code;
    uint8_t rsvd2;
};

enum 
{
	CMD_PIPE_ID		    = 1,
	STATUS_PIPE_ID		= 2,
	DATA_IN_PIPE_ID		= 3,
	DATA_OUT_PIPE_ID	= 4,

	UAS_SIMPLE_TAG		= 0,
	UAS_HEAD_TAG		= 1,
	UAS_ORDERED_TAG		= 2,
	UAS_ACA			    = 4,
};

/***************************************************************/
enum {
	SUBMIT_STATUS_URB	= (1 << 1),
	ALLOC_DATA_IN_URB	= (1 << 2),
	SUBMIT_DATA_IN_URB	= (1 << 3),
	ALLOC_DATA_OUT_URB	= (1 << 4),
	SUBMIT_DATA_OUT_URB	= (1 << 5),
	ALLOC_CMD_URB		= (1 << 6),
	SUBMIT_CMD_URB		= (1 << 7),
	COMMAND_INFLIGHT        = (1 << 8),
	DATA_IN_URB_INFLIGHT    = (1 << 9),
	DATA_OUT_URB_INFLIGHT   = (1 << 10),
	COMMAND_COMPLETED       = (1 << 11),
	COMMAND_ABORTED         = (1 << 12),
	UNLINK_DATA_URBS		= (1 << 13),
	IS_IN_WORK_LIST 		= (1 << 14),
};

struct uas_cmd_info
{
    uint32_t        state;
    uint32_t        stream;   // for USB3.0, not use now.
    struct urb      *cmd_urb;
    struct urb      *data_in_urb;
    struct urb      *data_out_urb;
};

#define UAS_MAX_LUN         2

struct uas_dev_info
{
    struct usb_device   *udev;
    struct usb_interface     *intf;
	struct usb_anchor cmd_urbs;
	struct usb_anchor sense_urbs;
	struct usb_anchor data_urbs;
	int qdepth, resetting;
    uint32_t cmd_pipe, status_pipe, data_in_pipe, data_out_pipe;
    struct scsi_cmnd *cmnd;
    struct response_ui response;
    spinlock_t      lock;

#define SCSI_CMDQ_DEPTH     6
    struct scsi_cmnd* cmd_list[SCSI_CMDQ_DEPTH];
    uint32_t cmd_list_tag;

    struct scsi_lun scsi_lun[UAS_MAX_LUN];
    struct scsi_device scsi_device[UAS_MAX_LUN];
    uint32_t lun_num:8;
    uint32_t use_streams:1;
};


static inline struct scsi_cmnd* find_cmnd_by_tag(struct uas_dev_info *devinfo, int tag)
{
    if((tag < 0) || (tag >= SCSI_CMDQ_DEPTH))
        return NULL;

    return devinfo->cmd_list[tag];
}

int uas_queuecommand(struct scsi_cmnd *cmnd);
void uas_log_cmd_state(struct scsi_cmnd *cmnd, const char *caller);
int uas_eh_task_mgmt(struct scsi_cmnd *cmnd, const char *fname, u8 function);

#endif

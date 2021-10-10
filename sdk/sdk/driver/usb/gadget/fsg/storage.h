
/*-------------------------------------------------------------------------*/


/* Bulk-only class specific requests */
#define USB_BULK_RESET_REQUEST			0xff
#define USB_BULK_GET_MAX_LUN_REQUEST	0xfe

/* Command Block Wrapper */
struct bulk_cb_wrap
{
    uint32_t Signature;		/* 'USBC' */
    uint32_t Tag;			/* unique per command id */
    uint32_t DataTransferLength; /* size of data */
    uint8_t	 Flags;			/* direction in bit 7 */
    uint8_t	 Lun;			/* LUN normally 0 */
    uint8_t	 Length;		/* of of the CDB */
    uint8_t  CDB[16];		/* command data block */
};

#define USB_BULK_CB_WRAP_LEN	31
#define USB_BULK_CB_SIG			0x43425355	// Spells out USBC
#define USB_BULK_IN_FLAG		0x80


/* Command Status Wrapper */
struct bulk_cs_wrap
{
    uint32_t	Signature;	/* Should = 'USBS' */
    uint32_t	Tag;		/* Same as original command */
    uint32_t	Residue;	/* Amount not transferred */
    uint8_t		Status;		/* Same as original command */
};

#define USB_BULK_CS_WRAP_LEN	13
#define USB_BULK_CS_SIG			0x53425355	// Spells out 'USBS'
#define USB_STATUS_PASS			0
#define USB_STATUS_FAIL			1
#define USB_STATUS_PHASE_ERROR	2



#define MAX_COMMAND_SIZE	16	// Length of a SCSI Command Data Block

/* SCSI commands that we recognize */
#define SC_FORMAT_UNIT					0x04
#define SC_INQUIRY						0x12
#define SC_MODE_SELECT_6				0x15
#define SC_MODE_SELECT_10				0x55
#define SC_MODE_SENSE_6					0x1a
#define SC_MODE_SENSE_10				0x5a
#define SC_PREVENT_ALLOW_MEDIUM_REMOVAL	0x1e
#define SC_READ_6						0x08
#define SC_READ_10						0x28
#define SC_READ_12						0xa8
#define SC_READ_CAPACITY				0x25
#define SC_READ_FORMAT_CAPACITIES		0x23
#define SC_RELEASE						0x17
#define SC_REQUEST_SENSE				0x03
#define SC_RESERVE						0x16
#define SC_SEND_DIAGNOSTIC				0x1d
#define SC_START_STOP_UNIT				0x1b
#define SC_SYNCHRONIZE_CACHE			0x35
#define SC_TEST_UNIT_READY				0x00
#define SC_VERIFY						0x2f
#define SC_WRITE_6						0x0a
#define SC_WRITE_10						0x2a
#define SC_WRITE_12						0xaa

/* SCSI Sense Key/Additional Sense Code/ASC Qualifier values */
#define SS_NO_SENSE								0
#define SS_COMMUNICATION_FAILURE				0x040800
#define SS_INVALID_COMMAND						0x052000
#define SS_INVALID_FIELD_IN_CDB					0x052400
#define SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE	0x052100
#define SS_LOGICAL_UNIT_NOT_SUPPORTED			0x052500
#define SS_MEDIUM_NOT_PRESENT					0x023a00
#define SS_MEDIUM_REMOVAL_PREVENTED				0x055302
#define SS_NOT_READY_TO_READY_TRANSITION		0x062800
#define SS_RESET_OCCURRED						0x062900
#define SS_SAVING_PARAMETERS_NOT_SUPPORTED		0x053900
#define SS_UNRECOVERED_READ_ERROR				0x031100
#define SS_WRITE_ERROR							0x030c02
#define SS_WRITE_PROTECTED						0x072700

#define SK(x)		((uint8_t) ((x) >> 16))	// Sense Key byte, etc.
#define ASC(x)		((uint8_t) ((x) >> 8))
#define ASCQ(x)		((uint8_t) (x))

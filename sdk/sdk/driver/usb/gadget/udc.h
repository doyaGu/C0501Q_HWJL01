/** @file
 * usb device controller header file.
 *
 * @author Irene Lin
 * @version 
 * @date
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
#ifndef UDC_H
#define UDC_H



#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                              Constants Definition
//=============================================================================
// Block Size define
#define BLK512BYTE		1
#define BLK1024BYTE		2

#define BLK64BYTE		1
#define BLK128BYTE		2

// Block toggle number define
#define SINGLE_BLK		1
#define DOUBLE_BLK		2
#define TRIBLE_BLK		3

// Endpoint transfer type
#define TF_TYPE_CONTROL			0
#define TF_TYPE_ISOCHRONOUS		1
#define TF_TYPE_BULK			2
#define TF_TYPE_INTERRUPT		3

// Endpoint or FIFO direction define
#define DIRECTION_IN	1
#define DIRECTION_OUT	0


// FIFO number define
#define FIFO0	0x0
#define FIFO1	0x1
#define FIFO2	0x2
#define FIFO3	0x3
#define FIFO4	0x4
#define FIFO5	0x5
#define FIFO6	0x6
#define FIFO7	0x7
#define FIFO8	0x8
#define FIFO9	0x9

// Endpoint number define
#define EP0         0x00
#define EP1         0x01
#define EP2         0x02
#define EP3         0x03
#define EP4         0x04
#define EP5         0x05
#define EP6         0x06
#define EP7         0x07
#define EP8         0x08
#define EP9         0x09
#define EP10        0x10
#define EP11        0x11
#define EP12        0x12
#define EP13        0x13
#define EP14        0x14
#define EP15        0x15



#define	MAX_FIFO_SIZE	64    // reset value for EP
#define	MAX_EP0_SIZE	0x40  // ep0 fifo size


#define ITE_MAX_EP				10  // 1 ~ 10
#define ITE_CURRENT_SUPPORT_EP	5	// ep0 ~ ep4


//=============================================================================
//                              Structure Definition
//=============================================================================
struct ite_ep
{
    struct usb_ep		ep;
    struct ite_udc		*dev;
    uint32_t			irqs;
    uint32_t			dma_set_len;

    uint32_t			num : 8;
    uint32_t			is_in : 1;
    uint32_t			stopped : 1;
    uint32_t			stall : 1;
    uint32_t			wedged : 1;


    struct list_head	queue;
    const struct usb_endpoint_descriptor	*desc;
};

struct ite_request
{
    struct usb_request	req;
    struct list_head	queue;
};

enum ep0state 
{
    EP0_DISCONNECT,	 /* no host */
    EP0_IDLE,		 /* between STATUS ack and SETUP report */
    EP0_IN, EP0_OUT, /* data stage */
    EP0_STATUS,		 /* status stage */
    EP0_STALL,		 /* data or status stages */
    EP0_SUSPEND,	 /* usb suspend */
};

typedef enum 
{
    CMD_VOID,				// No command
    CMD_GET_DESCRIPTOR,		// Get_Descriptor command
    CMD_SET_DESCRIPTOR,		// Set_Descriptor command
    CMD_TEST_MODE           // Test_Mode command
} CommandType;

struct ite_udc
{
    struct usb_gadget			gadget;
    _spinlock_t					lock;
    struct ite_ep				ep[ITE_MAX_EP];
    struct usb_gadget_driver	*driver;

    volatile enum ep0state   ep0state;
    CommandType eUsbCxCommand;

    struct usb_ep	*bulk_in;
    struct usb_ep	*bulk_out;

    uint8_t			intr_level1;
    uint8_t			intr_level1_save;
    uint8_t			intr_level1_mask;
    uint8_t			ep_use_dma;

    struct usb_request *ep0_req;
    uint32_t ep0_length;
    uint8_t ep0_data[2];
    uint8_t ep0_dir;
    uint8_t reserved;

    // some information
    uint32_t		irqs;
};



//=============================================================================
//                              EP/FIFO Config Definition
//=============================================================================

//*********************************************
//**** High Speed HW EP/FIFO Configurateion ***
#define HIGH_EP1_bBLKSIZE		BLK512BYTE
#define	HIGH_EP1_bBLKNO			SINGLE_BLK
#define HIGH_EP1_bDIRECTION		DIRECTION_IN
#define HIGH_EP1_bTYPE			TF_TYPE_BULK
#define HIGH_EP1_MAXPACKET		512

#define HIGH_EP2_bBLKSIZE		BLK512BYTE
#define	HIGH_EP2_bBLKNO			SINGLE_BLK
#define HIGH_EP2_bDIRECTION		DIRECTION_OUT
#define HIGH_EP2_bTYPE			TF_TYPE_BULK
#define HIGH_EP2_MAXPACKET		512

#define HIGH_EP3_bBLKSIZE		BLK512BYTE
#define	HIGH_EP3_bBLKNO			SINGLE_BLK
#define HIGH_EP3_bDIRECTION		DIRECTION_IN
#define HIGH_EP3_bTYPE			TF_TYPE_BULK
#define HIGH_EP3_MAXPACKET		512

#define HIGH_EP4_bBLKSIZE		BLK512BYTE
#define	HIGH_EP4_bBLKNO			SINGLE_BLK
#define HIGH_EP4_bDIRECTION		DIRECTION_OUT
#define HIGH_EP4_bTYPE			TF_TYPE_BULK
#define HIGH_EP4_MAXPACKET		512

//*********************************************
//**** Full Speed HW EP/FIFO Configurateion ***
#define FULL_EP1_bBLKSIZE		BLK512BYTE
#define	FULL_EP1_bBLKNO			SINGLE_BLK
#define FULL_EP1_bDIRECTION		DIRECTION_IN
#define FULL_EP1_bTYPE			TF_TYPE_BULK
#define FULL_EP1_MAXPACKET		64

#define FULL_EP2_bBLKSIZE		BLK512BYTE
#define	FULL_EP2_bBLKNO			SINGLE_BLK
#define FULL_EP2_bDIRECTION		DIRECTION_OUT
#define FULL_EP2_bTYPE			TF_TYPE_BULK
#define FULL_EP2_MAXPACKET		64

#define FULL_EP3_bBLKSIZE		BLK512BYTE
#define	FULL_EP3_bBLKNO			SINGLE_BLK
#define FULL_EP3_bDIRECTION		DIRECTION_IN
#define FULL_EP3_bTYPE			TF_TYPE_BULK
#define FULL_EP3_MAXPACKET		64

#define FULL_EP4_bBLKSIZE		BLK512BYTE
#define	FULL_EP4_bBLKNO			SINGLE_BLK
#define FULL_EP4_bDIRECTION		DIRECTION_OUT
#define FULL_EP4_bTYPE			TF_TYPE_BULK
#define FULL_EP4_MAXPACKET		64

//*********************************************
#define HIGH_EP1_FIFO_START		FIFO0
#define HIGH_EP2_FIFO_START		(HIGH_EP1_FIFO_START+(HIGH_EP1_bBLKSIZE*HIGH_EP1_bBLKNO))
#define HIGH_EP3_FIFO_START		(HIGH_EP2_FIFO_START+(HIGH_EP2_bBLKSIZE*HIGH_EP2_bBLKNO))
#define HIGH_EP4_FIFO_START		(HIGH_EP3_FIFO_START+(HIGH_EP3_bBLKSIZE*HIGH_EP3_bBLKNO))

#define HIGH_EP1_Map			(HIGH_EP1_FIFO_START|(HIGH_EP1_FIFO_START<<4) | (0xF0 >> (4*(1-HIGH_EP1_bDIRECTION))))
#define HIGH_EP1_FIFO_Map		((HIGH_EP1_bDIRECTION << 4) | EP1)
#define HIGH_EP1_FIFO_Config	(FIFOEnBit | ((HIGH_EP1_bBLKSIZE - 1) << 4) | ((HIGH_EP1_bBLKNO - 1) << 2) | HIGH_EP1_bTYPE)
#define HIGH_EP2_Map			(HIGH_EP2_FIFO_START|(HIGH_EP2_FIFO_START<<4) | (0xF0 >> (4*(1-HIGH_EP2_bDIRECTION))))
#define HIGH_EP2_FIFO_Map		((HIGH_EP2_bDIRECTION << 4) | EP2)
#define HIGH_EP2_FIFO_Config	(FIFOEnBit | ((HIGH_EP2_bBLKSIZE - 1) << 4) | ((HIGH_EP2_bBLKNO - 1) << 2) | HIGH_EP2_bTYPE)
#define HIGH_EP3_Map			(HIGH_EP3_FIFO_START|(HIGH_EP3_FIFO_START<<4) | (0xF0 >> (4*(1-HIGH_EP3_bDIRECTION))))
#define HIGH_EP3_FIFO_Map		((HIGH_EP3_bDIRECTION << 4) | EP3)
#define HIGH_EP3_FIFO_Config	(FIFOEnBit | ((HIGH_EP3_bBLKSIZE - 1) << 4) | ((HIGH_EP3_bBLKNO - 1) << 2) | HIGH_EP3_bTYPE)
#define HIGH_EP4_Map			(HIGH_EP4_FIFO_START|(HIGH_EP4_FIFO_START<<4) | (0xF0 >> (4*(1-HIGH_EP4_bDIRECTION))))
#define HIGH_EP4_FIFO_Map		((HIGH_EP4_bDIRECTION << 4) | EP4)
#define HIGH_EP4_FIFO_Config	(FIFOEnBit | ((HIGH_EP4_bBLKSIZE - 1) << 4) | ((HIGH_EP4_bBLKNO - 1) << 2) | HIGH_EP4_bTYPE)

#define FULL_EP1_FIFO_START		FIFO0
#define FULL_EP2_FIFO_START		(FULL_EP1_FIFO_START+(FULL_EP1_bBLKSIZE*FULL_EP1_bBLKNO))
#define FULL_EP3_FIFO_START		(FULL_EP2_FIFO_START+(FULL_EP2_bBLKSIZE*FULL_EP2_bBLKNO))
#define FULL_EP4_FIFO_START		(FULL_EP3_FIFO_START+(FULL_EP3_bBLKSIZE*FULL_EP3_bBLKNO))

#define FULL_EP1_Map			(FULL_EP1_FIFO_START|(FULL_EP1_FIFO_START<<4) | (0xF0 >> (4*(1-FULL_EP1_bDIRECTION))))
#define FULL_EP1_FIFO_Map		((FULL_EP1_bDIRECTION << 4) | EP1)
#define FULL_EP1_FIFO_Config	(FIFOEnBit | ((FULL_EP1_bBLKSIZE - 1) << 4) | ((FULL_EP1_bBLKNO - 1) << 2) | FULL_EP1_bTYPE)
#define FULL_EP2_Map			(FULL_EP2_FIFO_START|(FULL_EP2_FIFO_START<<4) | (0xF0 >> (4*(1-FULL_EP2_bDIRECTION))))
#define FULL_EP2_FIFO_Map		((FULL_EP2_bDIRECTION << 4) | EP2)
#define FULL_EP2_FIFO_Config	(FIFOEnBit | ((FULL_EP2_bBLKSIZE - 1) << 4) | ((FULL_EP2_bBLKNO - 1) << 2) | FULL_EP2_bTYPE)
#define FULL_EP3_Map			(FULL_EP3_FIFO_START|(FULL_EP3_FIFO_START<<4) | (0xF0 >> (4*(1-FULL_EP3_bDIRECTION))))
#define FULL_EP3_FIFO_Map		((FULL_EP3_bDIRECTION << 4) | EP3)
#define FULL_EP3_FIFO_Config	(FIFOEnBit | ((FULL_EP3_bBLKSIZE - 1) << 4) | ((FULL_EP3_bBLKNO - 1) << 2) | FULL_EP3_bTYPE)
#define FULL_EP4_Map			(FULL_EP4_FIFO_START|(FULL_EP4_FIFO_START<<4) | (0xF0 >> (4*(1-FULL_EP4_bDIRECTION))))
#define FULL_EP4_FIFO_Map		((FULL_EP4_bDIRECTION << 4) | EP4)
#define FULL_EP4_FIFO_Config	(FIFOEnBit | ((FULL_EP4_bBLKSIZE - 1) << 4) | ((FULL_EP4_bBLKNO - 1) << 2) | FULL_EP4_bTYPE)





#ifdef __cplusplus
}
#endif


#include  "udc_reg.h"



#endif // UDC_H
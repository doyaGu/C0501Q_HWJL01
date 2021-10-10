#ifndef _XD__H__
#define _XD__H__

#include "xd_type.h"
#include "ite\ith_defs.h"
#define	MM9070

#if !defined(MM9070) && !defined(MM230)
	#error "CHIP ID IS NOT DEFINED!"
#endif

////////////////////////////////////////////////////////////
//
// Compile Option
//
////////////////////////////////////////////////////////////
//#define XD_DEBUG_MSG

/*
#define	MMP_BOOL	unsigned char
#define	MMP_UINT8	unsigned char
#define	MMP_UINT16	unsigned short
#define	MMP_UINT32	unsigned int
#define	MMP_INT		int
#define	MMP_INLINE
#define	MMP_NULL    NULL
#define	MMP_TRUE    1
#define	MMP_FALSE   0
*/
////////////////////////////////////////////////////////////
//
// XD Register
//
////////////////////////////////////////////////////////////
// define MMIO register for XD //
#define SM_BASE_ADDR            0xDED00100	//(ITH_SD_BASE+0x100) //0xB100		//0xB100
  
  #define bit0                    0x01
  #define bit1                    0x02
  #define bit2                    0x04
  #define bit3                    0x08
  #define bit4                    0x10
  #define bit5                    0x20
  #define bit6                    0x40
  #define bit7                    0x80 
  
#define SMI_CTRL				(SM_BASE_ADDR)
	#define SMI_CTRL_RnB		bit0
	#define SMI_CTRL_WP			bit1
	#define SMI_CTRL_SM_CE		bit2
	#define SMI_CTRL_SM_CD		bit3
	#define SMI_CTRL_Ecc_En		bit5
	#define SMI_CTRL_SwRst		bit6
	#define SMI_CTRL_Clr_Rdnt 	bit7

#define SMI_RTRG				SM_BASE_ADDR+0x04
	#define RTrg_Rdnt			bit0
	#define RTrg_Dat			bit1
	#define RTrg_WaitRnB		bit2
	#define RTrg_Cmd2			bit3
	#define RTrg_Adr0			0x00
	#define RTrg_Adr1			0x10
	#define RTrg_Adr2			0x20
	#define RTrg_Adr3			0x30
	#define RTrg_Adr4			0x40
	#define RTrg_Adr5			0x50
	#define RTrg_Cmd1			bit7
#define SMI_WTRG				SM_BASE_ADDR+0x08
	#define WTrg_WaitRnB		bit0
	#define WTrg_Cmd2			bit1
	#define WTrg_Rdnt			bit2
	#define WTrg_Dat			bit3
	#define WTrg_Adr0			0x00
	#define WTrg_Adr1			0x10
	#define WTrg_Adr2			0x20
	#define WTrg_Adr3			0x30
	#define WTrg_Adr4			0x40
	#define WTrg_Adr5			0x50
	#define WTrg_Cmd1			bit7

#define SMI_ERR					SM_BASE_ADDR+0x0C
	#define Ecc1_1bit			bit0
	#define Ecc1_2bit			bit1
	#define Lba1_PrytErr		bit2
	#define Lba1_OvrRng			bit3
	#define Ecc2_1bit			bit4
	#define Ecc2_2bit			bit5
	#define Lba2_PrytErr		bit6
	#define Lba2_OvrRng			bit7

#define SMI_ADR5				SM_BASE_ADDR+0x10
#define SMI_ADR4				SM_BASE_ADDR+0x14
#define SMI_ADR3				SM_BASE_ADDR+0x18
#define SMI_ADR2				SM_BASE_ADDR+0x1C
#define SMI_ADR1				SM_BASE_ADDR+0x20
#define SMI_CMD1VLU				SM_BASE_ADDR+0x24
#define SMI_CMD2VLU				SM_BASE_ADDR+0x28
#define SMI_LBAH				SM_BASE_ADDR+0x2C
#define SMI_LBAL				SM_BASE_ADDR+0x30
#define SMI_RDNTLEN				SM_BASE_ADDR+0x34
#define SMI_DATALEN				SM_BASE_ADDR+0x38
#define SMI_CLOCK				SM_BASE_ADDR+0x3C
#define SMI_RDNT0				SM_BASE_ADDR+0x40
#define SMI_RDNT1				SM_BASE_ADDR+0x44
#define SMI_RDNT2				SM_BASE_ADDR+0x48
#define SMI_RDNT3				SM_BASE_ADDR+0x4C
#define SMI_RDNT4				SM_BASE_ADDR+0x50
#define SMI_RDNT5				SM_BASE_ADDR+0x54
#define SMI_RDNT6				SM_BASE_ADDR+0x58
#define SMI_RDNT7				SM_BASE_ADDR+0x5C
#define SMI_RDNT8				SM_BASE_ADDR+0x60
#define SMI_RDNT9				SM_BASE_ADDR+0x64
#define SMI_RDNTA				SM_BASE_ADDR+0x68
#define SMI_RDNTB				SM_BASE_ADDR+0x6C
#define SMI_RDNTC				SM_BASE_ADDR+0x70
#define SMI_RDNTD				SM_BASE_ADDR+0x74
#define SMI_RDNTE				SM_BASE_ADDR+0x78
#define SMI_RDNTF				SM_BASE_ADDR+0x7C

#define SMI_TRG_STATUS			SM_BASE_ADDR+0x80
	#define Trg_Status			bit0
	#define Trg_Rdn				bit1
	#define Rdn4addr			bit2

#define SMI_STATUS				SM_BASE_ADDR+0x84
	#define SMI_STATUS_Fail 	bit0
	#define SMI_STATUS_Ready	bit6
	#define SMI_STATUS_NotWP	bit7

#define	SMI_RCLOCK				SM_BASE_ADDR+0x90
#define	SMI_WCLOCK				SM_BASE_ADDR+0x94

#define SMI_BITCNT				SM_BASE_ADDR+0x98

#define SMI_WRAP_DATA_LENGTH	SM_BASE_ADDR+0x9C
#define SMI_WRAP_DATA_PORT		SM_BASE_ADDR+0xA0
#define SMI_WRAP_STATUS			SM_BASE_ADDR+0xA4
#define SMI_WRAP_CONTROL		SM_BASE_ADDR+0xA8

// define command of XD //
#define XD_READ_CMD_1ST        0x00
#define XD_READ_CMD_SPARE      0x50

#define XD_PROGRAM_CMD_1ST     0x80
#define XD_PROGRAM_CMD_2ND     0x10
 
#define XD_ERASE_CMD_1ST       0x60
#define XD_ERASE_CMD_2ND       0xD0

#define XD_READSTATUS_CMD_2ND  0X70

#define XD_READID90_CMD_1ST    0X90
#define XD_READID9A_CMD_1ST    0X9A

#define XD_RESET_CMD_2ST       0xFF

// define GPIO MMIO Space //
#define XD_GPIO_DATAOUT       0x7802
#define XD_GPIO_PINDIR        0x780A
       
////////////////////////////////////////////////////////////
//
// Const variable or Marco
//
////////////////////////////////////////////////////////////
#define XD_SMALLSIZE     0x0003 
#define XD_LARGESIZE     0x0004 
#define XD_ENABLE        0x0001 

#define XD_PAGE_SIZE     512
#define XD_SPARE_SIZE    16 
#define XD_DATA_SIZE     528
#define XD_PAGEPERBLOCK  32 

#define XD_IDLE          0x0001
#define XD_BLOCK_SIZE    XD_PAGE_SIZE * XD_PAGEPERBLOCK

#define XD_DATA_STATUS_FLAG  516
#define XD_BLOCK_STATUS_FLAG 517
#define XD_BLOCK_ADDR_1_LO   518
#define XD_BLOCK_ADDR_1_HI   519
#define XD_BLOCK_ADDR_2_LO   523
#define XD_BLOCK_ADDR_2_HI   524
#define XD_ECC_2_2           520
#define XD_ECC_2_1           521
#define XD_ECC_2_3           522
#define XD_ECC_1_2           525
#define XD_ECC_1_1           526
#define XD_ECC_1_3           527

#define XD_DATA_STATUS_FLAG_OFFSET  4  // XD_DATA_STATUS_FLAG(516) - 512 = 4
#define XD_BLOCK_STATUS_FLAG_OFFSET 5
#define XD_BLOCK_ADDR_1_LO_OFFSET   6
#define XD_BLOCK_ADDR_1_HI_OFFSET   7
#define XD_BLOCK_ADDR_2_LO_OFFSET   11
#define XD_BLOCK_ADDR_2_HI_OFFSET   12
#define XD_ECC_2_2_OFFSET           8
#define XD_ECC_2_1_OFFSET           9
#define XD_ECC_2_3_OFFSET           10
#define XD_ECC_1_2_OFFSET           13
#define XD_ECC_1_1_OFFSET           14
#define XD_ECC_1_3_OFFSET           15

#define WRAP_FULL_BIT           		0x00000001
#define WRAP_EMPTY_BIT           		0x00000002

#define WAIT_WRAP_FULL           		0x00000001
#define WAIT_WRAP_EMPTY          		0x00000000

#define WAIT_WRITE_TRIGGER         	0x00000001
#define WAIT_READ_TRIGGER          	0x00000000

enum {
	XD_OK,
	XD_ERASED,
	XD_ERROR
};

typedef enum XD_CMD_TYPE_TAG
{
  XD_ERASE,
  XD_WRITE,
  XD_READ,
  XD_READ_ID	
}XD_CMD_TYPE;
////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////
#define XD_nCE_Disable()		XD_WriteRegisterMask(SMI_CTRL, SMI_CTRL_SM_CE, SMI_CTRL_SM_CE)
#define XD_nCE_Enable()		    XD_WriteRegisterMask(SMI_CTRL, ~SMI_CTRL_SM_CE, SMI_CTRL_SM_CE)
#define XD_nWP_Disable()		XD_WriteRegisterMask(SMI_CTRL, SMI_CTRL_WP, SMI_CTRL_WP)
#define XD_nWP_Enable()  		XD_WriteRegisterMask(SMI_CTRL, ~SMI_CTRL_WP, SMI_CTRL_WP)
#define XD_SwReset()            XD_WriteRegisterMask(SMI_CTRL, SMI_CTRL_SwRst, SMI_CTRL_SwRst)
#define XD_WarpSoftReset()      XD_WriteRegisterMask(SMI_WRAP_CONTROL, 0x04,0x04);\
                                XD_WriteRegisterMask(SMI_WRAP_CONTROL, 0x00,0x04)

////////////////////////////////////////////////////////////
//
// Function Prototype
//
////////////////////////////////////////////////////////////
MMP_UINT8 GetXDId( unsigned long* pNumOfBlocks );
static MMP_INLINE MMP_UINT8 XD_WaitCmdReady(XD_CMD_TYPE type);
MMP_UINT8 XD_Initialize(unsigned int* pNumOfBlocks);
MMP_UINT8 XD_Terminate();
static MMP_INLINE MMP_UINT8 XD_WaitXDWrapFullEmpty(MMP_UINT8 Type);
MMP_UINT8 XD_WaitReadWriteTrigger(MMP_UINT8 Type);

static MMP_INLINE void XD_WriteRegisterMask(MMP_UINT32 RegisterAddress, MMP_UINT32 Data,MMP_UINT32 Mask);
static MMP_INLINE void XD_ReadRegister(MMP_UINT32 RegisterAddress, MMP_UINT32 *dRegValue);


//MMP_UINT8 XD_WarpSoftReset();
MMP_UINT8 XD_Erase(MMP_UINT32 pba);
MMP_UINT8 XD_WriteSeparate(MMP_UINT32 pba, MMP_UINT32 ppo, MMP_UINT8* pData, MMP_UINT8* pSpare,MMP_UINT32 LogicBlkAddr);
MMP_UINT8 XD_WriteSingle(MMP_UINT32 pba, MMP_UINT32 ppo, MMP_UINT8* pData,MMP_UINT32 LogicBlkAddr);
MMP_UINT8 XD_ReadSeparate(MMP_UINT32 pba, MMP_UINT32 ppo, MMP_UINT8* pDataBuffer, MMP_UINT8* pSparebuffer);
MMP_UINT8 XD_ReadSingle(MMP_UINT32 pba, MMP_UINT32 ppo, MMP_UINT8* pDataBuffer);
MMP_UINT8 XD_ReadPage(MMP_UINT32 pba, MMP_UINT32 ppo, MMP_UINT8* pDataBuffer);
MMP_UINT8 XD_ReadSpare(MMP_UINT32 pba, MMP_UINT32 ppo, MMP_UINT8* buffer);
MMP_UINT8 XD_WriteDummy(MMP_UINT32 pba, MMP_UINT32 ppo, MMP_UINT8* pData);

#endif

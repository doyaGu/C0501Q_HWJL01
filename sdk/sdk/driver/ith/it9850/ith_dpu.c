/*
 * Copyright (c) 2012 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL H/W decompress functions.
 *
 * @author Joseph Chang
 * @version 1.0
 */
#include "ite/ith.h"	
#include "ith_cfg.h"

//#define CPU_TYPE_LITTLE_ENDIAN

/* enable this option when CPU is RISC */
/* disable this option if WIN32 */
//#define ENABLE_PATCH_BIG_ENDIAN_ISSUE

//=============================================================================
//                              macro defination
//=============================================================================
#define ENABLE_DPU_LOG

#define DECOMPRESS_SW_ERR_DONEINDEX_OVER    0x00000001
#define DECOMPRESS_HW_FAIL                  0x00000004

#define DPU_INTR_MASK						0x00000001
#define DPU_INTR_ENABLE						0x00000001
#define DPU_INTR_DISABLE					0x00000000

#define DPU_INTR_CLEAR_MASK					0x00000002
#define DPU_INTR_ENCLEAR					0x00000002
#define DPU_INTR_DISCLEAR					0x00000000

#define	DPU_SET_ENCODE_DECODE_MASK			0x00000080	//(0x01<<2)
#define	DPU_SET_ENCODE						0x00000080
#define	DPU_SET_DECODE						0x00000000


#define DPU_FIRE_BIT	0x00000002
#define DPU_RESET_BIT	0x00000001

#define MAX_DPU_MODE_NUM   (20)

//=============================================================================
//                              DPU Register definition
//=============================================================================
#define ITH_DPU_REG_DCR			(ITH_DPU_BASE+0x00)
#define ITH_DPU_REG_SAR			(ITH_DPU_BASE+0x04)
#define ITH_DPU_REG_DAR			(ITH_DPU_BASE+0x08)
#define ITH_DPU_REG_SSR			(ITH_DPU_BASE+0x0C)
#define ITH_DPU_REG_INTCT		(ITH_DPU_BASE+0x10)
#define ITH_DPU_REG_DSR			(ITH_DPU_BASE+0x14)
#define ITH_DPU_REG_CRC0		(ITH_DPU_BASE+0x18)
#define ITH_DPU_REG_CRC1		(ITH_DPU_BASE+0x1C)
#define ITH_DPU_REG_CRCSD		(ITH_DPU_BASE+0x20)

#define ITH_DPU_REG_RANKEY		(ITH_DPU_BASE+0x24)
#define ITH_DPU_REG_RANKS		(ITH_DPU_BASE+0x28)
#define ITH_DPU_REG_KEY0		(ITH_DPU_BASE+0x2C)
#define ITH_DPU_REG_KEY1		(ITH_DPU_BASE+0x30)
#define ITH_DPU_REG_KEY2		(ITH_DPU_BASE+0x34)
#define ITH_DPU_REG_KEY3		(ITH_DPU_BASE+0x38)
#define ITH_DPU_REG_KEY4		(ITH_DPU_BASE+0x3C)
#define ITH_DPU_REG_KEY5		(ITH_DPU_BASE+0x40)
#define ITH_DPU_REG_VECTOR0		(ITH_DPU_BASE+0x44)
#define ITH_DPU_REG_VECTOR1		(ITH_DPU_BASE+0x48)
#define ITH_DPU_REG_VECTOR2		(ITH_DPU_BASE+0x4C)
#define ITH_DPU_REG_VECTOR3		(ITH_DPU_BASE+0x50)

#define ITH_AHB_REG_CLOCK2      (0x1A)
//=============================================================================
//                              DPU Register function mask Definition
//=============================================================================
//Burst size select
#define	DPU_BURST_WIDTH_MASK				0x70000000	//(0x07<<28)
#define	DPU_BURST_WIDTH_8_BIT				0x00000000
#define	DPU_BURST_WIDTH_16_BIT				0x10000000
#define	DPU_BURST_WIDTH_32_BIT				0x20000000

//Burst size select
#define	DPU_BURST_SIZE_MASK					0x07000000	//(0x07<<24)
#define	DPU_BURST_SIZE_1					0x00000000
#define	DPU_BURST_SIZE_4					0x01000000
#define	DPU_BURST_SIZE_8					0x02000000
#define	DPU_BURST_SIZE_16					0x03000000

//Random key free run
#define	DPU_RAND_KEY_FREERUN_MASK			0x00000800	//(0x01<<20)
#define	DPU_SET_RAND_KEY_FREERUN			0x00000800
#define	DPU_SET_RAND_KEY_DISABLE			0x00000000

//select CRC mode
#define	CRC_MODE_MASK						0x00080000	//(0x01<<19)
#define	CRC_SELECT_MASTER					0x00000000
#define	CRC_SELECT_SLAVE					0x00080000

//select CRC register
#define	CRC_REG_MASK						0x00040000	//(0x01<<18)
#define	CRC_SELECT_CRC_REG0					0x00000000
#define	CRC_SELECT_CRC_REG1					0x00040000

//Data process with CRC
#define	CRC_PROC_MASK						0x00030000	//(0x03<<16)
#define	CRC_PROC_USE_CRC_WHEN_DPU_MODE		0x00000000
#define	CRC_PROC_DATA_FROM_DATAIN			0x00010000
#define	CRC_PROC_DATA_FROM_DATAOUT			0x00020000
#define	CRC_PROC_NOT_DEFINE					0x00030000

//select the source of key
#define	DPU_SRC_OF_INIT_VCTR_MASK			0x0000C000	//(0x03<<14)
#define	DPU_SET_VECTOR_SRC0					0x00000000
#define	DPU_SET_VECTOR_SRC1					0x00004000
#define	DPU_SET_VECTOR_SRC2					0x00008000
#define	DPU_SET_VECTOR_SRC3					0x0000C000

//select the source of initial vector
#define	DPU_RAND_KEY_SOURCE_MASK			0x00003000	//(0x03<<12)
#define	DPU_SET_KEY_SRC0					0x00000000
#define	DPU_SET_KEY_SRC1					0x00001000
#define	DPU_SET_KEY_SRC2					0x00002000
#define	DPU_SET_KEY_SRC3					0x00003000

//DPU cipher mode select
#define	DPU_CIPHER_MODE_MASK				0x00000700	//(0x07<<8)
#define	DPU_SET_ECB_MODE					0x00000000
#define	DPU_SET_CBC_MODE					0x00000100
#define	DPU_SET_OFB_MODE					0x00000200
#define	DPU_SET_CFB_MODE					0x00000300
#define	DPU_SET_CTR_MODE					0x00000400

//DPU mode select
#define	DPU_MODE_MASK						0x00000070	//(0x07<<4)
#define	DPU_SET_CRC_MODE					0x00000000
#define	DPU_SET_AES_MODE					0x00000010
#define	DPU_SET_CSA_MODE					0x00000020
#define	DPU_SET_DES_MODE					0x00000030
#define	DPU_SET_DES3_MODE					0x00000040

//DPU endian select
#define	DPU_SET_ENDIAN_MASK					0x0030000C	//(0x01<<3)
#define	DPU_SET_BIG_ENDIAN					0x0000000C
#define	DPU_SET_LITTLE_ENDIAN				0x00000000

#define	DPU_INTR_ENABLE_MASK				0x00000001
#define	DPU_INTR_ENABLE						0x00000001
#define	DPU_INTR_DISABLE					0x00000000

#define AES_ECB_CTRL_INIT_VALUE		(DPU_BURST_WIDTH_32_BIT | DPU_BURST_SIZE_8 | DPU_SET_RAND_KEY_FREERUN | CRC_PROC_DATA_FROM_DATAIN |DPU_SET_LITTLE_ENDIAN |DPU_SET_VECTOR_SRC1 | DPU_SET_KEY_SRC1 | DPU_SET_AES_MODE | DPU_SET_ECB_MODE)
#define AES_CBC_CTRL_INIT_VALUE		(DPU_BURST_WIDTH_32_BIT | DPU_BURST_SIZE_8 | DPU_SET_RAND_KEY_FREERUN | CRC_PROC_DATA_FROM_DATAIN |DPU_SET_LITTLE_ENDIAN |DPU_SET_VECTOR_SRC1 | DPU_SET_KEY_SRC1 | DPU_SET_AES_MODE | DPU_SET_CBC_MODE)
#define AES_OFB_CTRL_INIT_VALUE		(DPU_BURST_WIDTH_32_BIT | DPU_BURST_SIZE_8 | DPU_SET_RAND_KEY_FREERUN | CRC_PROC_DATA_FROM_DATAIN |DPU_SET_LITTLE_ENDIAN |DPU_SET_VECTOR_SRC1 | DPU_SET_KEY_SRC1 | DPU_SET_AES_MODE | DPU_SET_OFB_MODE)
#define AES_CFB_CTRL_INIT_VALUE		(DPU_BURST_WIDTH_32_BIT | DPU_BURST_SIZE_8 | DPU_SET_RAND_KEY_FREERUN | CRC_PROC_DATA_FROM_DATAIN |DPU_SET_LITTLE_ENDIAN |DPU_SET_VECTOR_SRC1 | DPU_SET_KEY_SRC1 | DPU_SET_AES_MODE | DPU_SET_CFB_MODE)
#define AES_CTR_CTRL_INIT_VALUE		(DPU_BURST_WIDTH_32_BIT | DPU_BURST_SIZE_8 | DPU_SET_RAND_KEY_FREERUN | CRC_PROC_DATA_FROM_DATAIN |DPU_SET_LITTLE_ENDIAN |DPU_SET_VECTOR_SRC1 | DPU_SET_KEY_SRC1 | DPU_SET_AES_MODE | DPU_SET_CTR_MODE)

#define DES_ECB_CTRL_INIT_VALUE		(DPU_BURST_WIDTH_32_BIT | DPU_BURST_SIZE_8 | DPU_SET_RAND_KEY_DISABLE | CRC_PROC_USE_CRC_WHEN_DPU_MODE |DPU_SET_LITTLE_ENDIAN |DPU_SET_VECTOR_SRC1 | DPU_SET_KEY_SRC1 | DPU_SET_DES_MODE | DPU_SET_ECB_MODE)
#define DES_CBC_CTRL_INIT_VALUE		(DPU_BURST_WIDTH_32_BIT | DPU_BURST_SIZE_8 | DPU_SET_RAND_KEY_DISABLE | CRC_PROC_USE_CRC_WHEN_DPU_MODE |DPU_SET_LITTLE_ENDIAN |DPU_SET_VECTOR_SRC1 | DPU_SET_KEY_SRC1 | DPU_SET_DES_MODE | DPU_SET_CBC_MODE)
#define DES_OFB_CTRL_INIT_VALUE		(DPU_BURST_WIDTH_32_BIT | DPU_BURST_SIZE_8 | DPU_SET_RAND_KEY_DISABLE | CRC_PROC_USE_CRC_WHEN_DPU_MODE |DPU_SET_LITTLE_ENDIAN |DPU_SET_VECTOR_SRC1 | DPU_SET_KEY_SRC1 | DPU_SET_DES_MODE | DPU_SET_OFB_MODE)
#define DES_CFB_CTRL_INIT_VALUE		(DPU_BURST_WIDTH_32_BIT | DPU_BURST_SIZE_8 | DPU_SET_RAND_KEY_DISABLE | CRC_PROC_USE_CRC_WHEN_DPU_MODE |DPU_SET_LITTLE_ENDIAN |DPU_SET_VECTOR_SRC1 | DPU_SET_KEY_SRC1 | DPU_SET_DES_MODE | DPU_SET_CFB_MODE)
#define DES_CTR_CTRL_INIT_VALUE		(DPU_BURST_WIDTH_32_BIT | DPU_BURST_SIZE_8 | DPU_SET_RAND_KEY_DISABLE | CRC_PROC_USE_CRC_WHEN_DPU_MODE |DPU_SET_LITTLE_ENDIAN |DPU_SET_VECTOR_SRC1 | DPU_SET_KEY_SRC1 | DPU_SET_DES_MODE | DPU_SET_CTR_MODE)

#define DES3_ECB_CTRL_INIT_VALUE	(DPU_BURST_WIDTH_32_BIT | DPU_BURST_SIZE_8 | DPU_SET_RAND_KEY_DISABLE | CRC_PROC_USE_CRC_WHEN_DPU_MODE |DPU_SET_LITTLE_ENDIAN |DPU_SET_VECTOR_SRC1 | DPU_SET_KEY_SRC1 | DPU_SET_DES3_MODE | DPU_SET_ECB_MODE)
#define DES3_CBC_CTRL_INIT_VALUE	(DPU_BURST_WIDTH_32_BIT | DPU_BURST_SIZE_8 | DPU_SET_RAND_KEY_DISABLE | CRC_PROC_USE_CRC_WHEN_DPU_MODE |DPU_SET_LITTLE_ENDIAN |DPU_SET_VECTOR_SRC1 | DPU_SET_KEY_SRC1 | DPU_SET_DES3_MODE | DPU_SET_CBC_MODE)
#define DES3_OFB_CTRL_INIT_VALUE	(DPU_BURST_WIDTH_32_BIT | DPU_BURST_SIZE_8 | DPU_SET_RAND_KEY_DISABLE | CRC_PROC_USE_CRC_WHEN_DPU_MODE |DPU_SET_LITTLE_ENDIAN |DPU_SET_VECTOR_SRC1 | DPU_SET_KEY_SRC1 | DPU_SET_DES3_MODE | DPU_SET_OFB_MODE)
#define DES3_CFB_CTRL_INIT_VALUE	(DPU_BURST_WIDTH_32_BIT | DPU_BURST_SIZE_8 | DPU_SET_RAND_KEY_DISABLE | CRC_PROC_USE_CRC_WHEN_DPU_MODE |DPU_SET_LITTLE_ENDIAN |DPU_SET_VECTOR_SRC1 | DPU_SET_KEY_SRC1 | DPU_SET_DES3_MODE | DPU_SET_CFB_MODE)
#define DES3_CTR_CTRL_INIT_VALUE	(DPU_BURST_WIDTH_32_BIT | DPU_BURST_SIZE_8 | DPU_SET_RAND_KEY_DISABLE | CRC_PROC_USE_CRC_WHEN_DPU_MODE |DPU_SET_LITTLE_ENDIAN |DPU_SET_VECTOR_SRC1 | DPU_SET_KEY_SRC1 | DPU_SET_DES3_MODE | DPU_SET_CTR_MODE)

#define CSA_CTRL_INIT_VALUE			(DPU_BURST_WIDTH_32_BIT | DPU_BURST_SIZE_8 | DPU_SET_RAND_KEY_DISABLE | CRC_PROC_USE_CRC_WHEN_DPU_MODE |DPU_SET_LITTLE_ENDIAN |DPU_SET_VECTOR_SRC1 | DPU_SET_KEY_SRC1 | DPU_SET_CSA_MODE)
#define CRC_CTRL_INIT_VALUE			(DPU_BURST_WIDTH_32_BIT | DPU_BURST_SIZE_8 | DPU_SET_RAND_KEY_DISABLE | CRC_PROC_USE_CRC_WHEN_DPU_MODE |DPU_SET_LITTLE_ENDIAN |DPU_SET_VECTOR_SRC1 | DPU_SET_KEY_SRC1 | DPU_SET_CRC_MODE)

#define DPU_N5CLK_EN                 5
//=============================================================================
//                              LOG definition
//=============================================================================
#ifdef	ENABLE_DPU_LOG
#define ITH_DPU_LOG_INFO	    LOG_INFO
#define ITH_DPU_LOG_WARNING	    LOG_INFO
#define ITH_DPU_LOG_DEBUG	    LOG_INFO
#else
#define ITH_DPU_LOG_INFO	    LOG_INFO
#define ITH_DPU_LOG_WARNING	    LOG_WARN
#define ITH_DPU_LOG_DEBUG	    LOG_DBG
#endif

#define ITH_DPU_LOG_ERROR	    LOG_ERR
#define ITH_DPU_LOG_END	        LOG_END	

//#define ITH_DPU_LOG_ERROR	    printf(
//#define ITH_DPU_LOG_END	    );	

//=============================================================================
//                              MACRO definition
//=============================================================================
#ifdef	ENABLE_DPU_TYPE
#define DPU_BOOL	bool
#define DPU_UINT8	uint8_t
#define DPU_UINT16	uint16_t
#define DPU_UINT32	uint32_t
#else
#define DPU_BOOL	bool
#define DPU_UINT8	unsigned char
#define DPU_UINT16	unsigned int
#define DPU_UINT32	unsigned long
#endif

#define DPU_RESULT	DPU_BOOL




//=============================================================================
//                              type & structure definition
//=============================================================================

DPU_UINT32 DpuDsrInitArray[MAX_DPU_MODE_NUM] = //MAX_DPU_MODE_NUM = 20
{
	AES_ECB_CTRL_INIT_VALUE,
	AES_CBC_CTRL_INIT_VALUE,
	AES_OFB_CTRL_INIT_VALUE,
	AES_CFB_CTRL_INIT_VALUE,
	AES_CTR_CTRL_INIT_VALUE,

	DES_ECB_CTRL_INIT_VALUE,
	DES_CBC_CTRL_INIT_VALUE,
	DES_OFB_CTRL_INIT_VALUE,
	DES_CFB_CTRL_INIT_VALUE,
	DES_CTR_CTRL_INIT_VALUE,

	DES3_ECB_CTRL_INIT_VALUE,
	DES3_CBC_CTRL_INIT_VALUE,
	DES3_OFB_CTRL_INIT_VALUE,
	DES3_CFB_CTRL_INIT_VALUE,
	DES3_CTR_CTRL_INIT_VALUE,

	CSA_CTRL_INIT_VALUE,
	CRC_CTRL_INIT_VALUE,
	0,
	0,
	0
};
/*****************************************************************/
//globol variable
/*****************************************************************/

//static uint32_t g_DpuSetIndex;

static uint32_t DpuRegs[(ITH_DPU_REG_VECTOR3 - ITH_DPU_REG_DCR + 4) / 4];
/*****************************************************************/
//function protocol type
/*****************************************************************/

/*****************************************************************/
//function implement
/*****************************************************************/
void TransBig2LittleDw(uint8_t *pBuff1, uint8_t *pBuff2)
{
	uint8_t i;

	for(i=0;i<4;i++)
	{
		pBuff2[i]=pBuff1[3-i];
	}
}

void ithDpuClearCtrl(void)
{
	ithWriteRegA( ITH_DPU_REG_DCR, 0x00000000);
}

void ithDpuClearIntr(void)
{
	ithWriteRegMaskA( ITH_DPU_REG_INTCT, DPU_INTR_ENCLEAR, DPU_INTR_CLEAR_MASK);
	ithWriteRegMaskA( ITH_DPU_REG_INTCT, DPU_INTR_DISCLEAR, DPU_INTR_CLEAR_MASK);
}

void ithDpuInitCtrl(ITH_DPU_MODE DpuMode)
{
	uint32_t	CtrlRegMask = 0x777FF778;

	ithWriteRegMaskA( ITH_DPU_REG_DCR, DpuDsrInitArray[DpuMode], CtrlRegMask );
}

void ithDpuInitCrc(void)
{
	ithWriteRegA(ITH_DPU_REG_CRC0, 0xffffffff);	//set crc0
	ithWriteRegA(ITH_DPU_REG_CRC1, 0xffffffff);	//set crc1
}

void ithDpuEnableIntr(void)
{
	ithWriteRegMaskA( ITH_DPU_REG_INTCT, DPU_INTR_ENABLE, DPU_INTR_ENABLE_MASK);
}

void ithDpuDisableIntr(void)
{
	ithWriteRegMaskA( ITH_DPU_REG_INTCT, DPU_INTR_DISABLE, DPU_INTR_ENABLE_MASK);
}

void ithDpuSetSrcAddr(unsigned int SrcAddr)
{
	ithWriteRegA(ITH_DPU_REG_SAR, SrcAddr); 
}

void ithDpuGetCrcValue(unsigned int *Reg32)
{
	*Reg32 = ithReadRegA(ITH_DPU_REG_CRC0); 
}

void ithDpuGetStatus(unsigned int *Reg32)
{
	*Reg32 = ithReadRegA(ITH_DPU_REG_DSR); 
}

void ithDpuSetCrcData(unsigned int CrcData)
{
#ifdef	WIN32
	ithWriteRegA(ITH_DPU_BASE, 0x20000000);
	ithWriteRegH(0x7822, (CrcData>>16)&0xFFFF);
	ithWriteRegA(ITH_DPU_BASE, 0x20080000);
	ithWriteRegH(0x7820, CrcData&0xFFFF);
#else

#ifdef	ENABLE_PATCH_BIG_ENDIAN_ISSUE
	unsigned int	intCRC[1];
	unsigned char  dwCRC[4];
	unsigned int  *pdwCrc = (unsigned int *)dwCRC;
	
	intCRC[0]=CrcData;
	
	ithWriteRegA( ITH_DPU_REG_DCR, 0x20080000);	
	TransBig2LittleDw(&intCRC[0], &dwCRC[0]);
	ithWriteRegA( ITH_DPU_REG_CRCSD, pdwCrc[0]);
	//printf("crc1=%08x,crc2=%08x\n",CrcData, pdwCrc[0]);
#else
	ithWriteRegA( ITH_DPU_REG_DCR, 0x20080000);	
	ithWriteRegA( ITH_DPU_REG_CRCSD, CrcData); 
#endif

#endif	//WIN32
}

void ithDpuSetDstAddr(unsigned int DstAddr)
{
	ithWriteRegA(ITH_DPU_REG_DAR, DstAddr); 
}

void ithDpuSetEncrypt(void)
{
	ithWriteRegMaskA( ITH_DPU_REG_DCR, DPU_SET_ENCODE, DPU_SET_ENCODE_DECODE_MASK );
}

void ithDpuSetDescrypt(void)
{
	ithWriteRegMaskA( ITH_DPU_REG_DCR, DPU_SET_DECODE, DPU_SET_ENCODE_DECODE_MASK );
}

void ithDpuFire(void)
{
	ithWriteRegMaskA( ITH_DPU_REG_DCR, DPU_FIRE_BIT, DPU_FIRE_BIT );			//fire DPU engine 
}

void ithDpuSetSize(unsigned int size)
{
	ithWriteRegA(ITH_DPU_REG_SSR, size);
}

void ithDpuSetKey(uint32_t *key, uint8_t KeyNum)
{
	unsigned char KeyIndex;
#ifdef ENABLE_PATCH_BIG_ENDIAN_ISSUE	
	unsigned char  cKey2[4];
	unsigned int  *pKey=cKey2;
	#ifdef	CFG_CPU_WB
    ithFlushDCacheRange((void*)pKey, KeyNum*4);
    ithFlushMemBuffer();
    #endif
	for(KeyIndex=0; KeyIndex<KeyNum; KeyIndex++)
	{
		TransBig2LittleDw(&key[KeyIndex],&cKey2[0]);
		ithWriteRegA( ((ITH_DPU_REG_KEY0)+(KeyIndex*4)), pKey[0] );
	}
#else
	uint32_t *pKey = (uint32_t*)key;
	for(KeyIndex=0; KeyIndex<KeyNum; KeyIndex++)
	{
		ithWriteRegA( ((ITH_DPU_REG_KEY0)+(KeyIndex*4)), pKey[KeyIndex] ); 
	}
#endif
}

void ithDpuSetVector(uint32_t *vector, uint8_t len)
{
	unsigned char VectorIndex;
#ifdef ENABLE_PATCH_BIG_ENDIAN_ISSUE
	unsigned char  cKey2[4];
	unsigned int  *pIV=cKey2;
	#ifdef	CFG_CPU_WB
    ithFlushDCacheRange((void*)pIV, len*4);
    ithFlushMemBuffer();
    #endif
	for(VectorIndex=0; VectorIndex<len; VectorIndex++)
	{
		TransBig2LittleDw(&vector[VectorIndex],&cKey2[0]);
		ithWriteRegA( ((ITH_DPU_REG_VECTOR0)+(VectorIndex*4)), pIV[0] );
	}
#else
	uint32_t *pIV=(uint32_t *)vector;
	for(VectorIndex=0; VectorIndex<len; VectorIndex++)
	{
		ithWriteRegA( ((ITH_DPU_REG_VECTOR0)+(VectorIndex*4)), pIV[VectorIndex] ); 
	}
#endif
}

bool ithDpuWait(void)
{
	unsigned int    StatusReg32,TomeOutCnt=0x100000;
	unsigned char	Result = 1;

	do
	{
		StatusReg32 = ithReadRegA(ITH_DPU_REG_DSR);     //check DPU done bit

		if(TomeOutCnt--==0)
		{
			printf("ith DPU timeOut!!\n");
			Result = 0;
			break;
		}
	} while((StatusReg32&0x00000001) == 0x00000000);

	ithWriteRegA(ITH_DPU_REG_DCR, 0x00000000);	//clear DPU fire bit

	return	Result;

}

void ithDpuSuspend(void)
{
	unsigned int    StatusReg32,TomeOutCnt=0;
    int i;
    
    //wait DPU idle
	do
	{
		StatusReg32 = ithReadRegA(ITH_DPU_REG_DSR);     //check DPU done bit
		if(TomeOutCnt++>=10000)
		{
			printf("[dpu][warning]: TimeOutCnt,reg=%x\n",StatusReg32);
			TomeOutCnt=0;
		}
	} while((StatusReg32&0x00000002) == 0x00000002);	  
	
	//backup DPU registers
    for (i = ITH_DPU_REG_DCR; i < ITH_DPU_REG_VECTOR3; i+=4)
    {
        switch (i)
        {
        case ITH_DPU_REG_INTCT:        	
            DpuRegs[(i-ITH_DPU_REG_DCR)/4] = ithReadRegA(ITH_DPU_REG_INTCT);
            //printf("suspend, backup dpu reg,[%x,%08x][%x,%x,%x]\n",ITH_DPU_REG_INTCT,DpuRegs[(i-ITH_DPU_REG_DCR)/4], i, (i-ITH_DPU_REG_DCR)/4, ITH_DPU_REG_DCR);
            break;
            
        default:
            // don't need to backup 
            break;
        }
    }
}

void ithDpuResume(void)
{
    int i;
    
    //restore DPU registers
    for (i=ITH_DPU_REG_DCR; i<ITH_DPU_REG_VECTOR3; i+=4)
    {
        switch (i)
        {
        case ITH_DPU_REG_INTCT:
        	ithWriteRegA( ITH_DPU_REG_INTCT, DpuRegs[(i-ITH_DPU_REG_DCR)/4]);
            break;
            
        default:
            // don't need to backup 
            break;
        }
    }
}

void ithDpuSetDpuEndian(unsigned int EndianIndex)
{
	ithWriteRegMaskA( ITH_DPU_REG_DCR, EndianIndex, DPU_SET_ENDIAN_MASK );
}

void ithDpuEnableClock(void)
{
	ithSetRegBitH(ITH_AHB_REG_CLOCK2, DPU_N5CLK_EN);
}

void ithDpuDisableClock(void)
{
	ithClearRegBitH(ITH_AHB_REG_CLOCK2, DPU_N5CLK_EN);
}
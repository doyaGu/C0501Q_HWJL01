/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Decompress functions.
 *
 * @author Joseph
 * @version 1.0
 */
#include <errno.h>
#include <malloc.h>
#include <string.h>

#include "ite/ith.h"
#include "itp_cfg.h"

//#define ENABLE_ITP_DPU_DBG_MEASUREMENT
/*******************************************************************

 *******************************************************************/
#ifdef	CFG_CPU_WB
#define ENABLE_WRITE_BACK_CACHE
#endif

#define DPU_IRQ_ENABLE
#define ENABLE_HW_REVERSE
#define DPU_ALIGN_SIZE			(32)

#if CFG_CHIP_FAMILY == 9850

//#define ENABLE_9850_PATCH_ENDIAN_ISSUE
#define ENABLE_9850_PATCH_RESET_ISSUE

#ifdef ENABLE_9850_PATCH_ENDIAN_ISSUE
#define ITP_DPU_LITTLE_ENDIAN    0x00000000
#define ITP_DPU_BIG_ENDIAN       0x00300000
#else
#define ITP_DPU_LITTLE_ENDIAN    0x00000000
#define ITP_DPU_BIG_ENDIAN       0x0000000C
#endif
	
#else
	
/*******************************************************************
 "ENABLE_PATCH_DMA_ISSUE" is for fix the DPU H/W issue. 
 DPU can not work well with dma function (like: SPI...)
 *******************************************************************/
#define	ENABLE_PATCH_DMA_ISSUE

#define ITP_DPU_LITTLE_ENDIAN    0x00000000
#define ITP_DPU_BIG_ENDIAN       0x00000008

#endif    //CFG_CHIP_FAMILY == 9850

/*******************************************************************

 *******************************************************************/
static const ITPDpuModeEnum DpuModeTable[] =
{
    ITP_DPU_AES_MODE,
    ITP_DPU_DES_MODE,
    ITP_DPU_DES3_MODE,
    ITP_DPU_CSA_MODE,
    ITP_DPU_CRC_MODE,
	ITP_DPU_UNKNOW_MODE,
};

static const char* ItpDpuMode[] =
{
	"aes",
	"des",
	"des3",
	"csa",
	"crc",
};

static ITP_DPU_INFO 	g_DpuInfo;    
static unsigned int 	g_DpuSetIndex = 0;

#ifdef ENABLE_HW_REVERSE
static unsigned int 	g_DpuEndianSetting = ITP_DPU_BIG_ENDIAN;
#else
static unsigned int 	g_DpuEndianSetting = ITP_DPU_LITTLE_ENDIAN;
#endif

#if defined(DPU_IRQ_ENABLE)
static sem_t* 			DpuIsrSemaphor = NULL;
static unsigned int 	g_IsrCnt = 0;
#endif

#ifdef	ENABLE_ITP_DPU_DBG_MEASUREMENT
static struct timeval startT, endT;
#endif

#if defined(DPU_IRQ_ENABLE)
void dpu_isr(void* data)
{
	unsigned int tmp = 0;
	
	//ithPrintf("$in\n");	
	g_IsrCnt++;

	ithDpuGetStatus(&tmp);
	
	if( (tmp&0x00000001)== 0x00000001 )
	{		
		itpSemPostFromISR(DpuIsrSemaphor);		
    }
    else
    {
    	ithPrintf("$--ith dpu warning~~\n");
    }

    ithDpuClearIntr();
    	
	//ithPrintf("$[%d,%d]\n",g_CmdQueDoneIndex,g_CmdQueIndex);
	//ithPrintf("$out\n");
}

void DpuEnableIntr(void)
{
	// Initialize DPU IRQ
	printf("Enable DPU IRQ~~\n");
	ithIntrDisableIrq(ITH_INTR_DPU);
	ithIntrClearIrq(ITH_INTR_DPU);
	
	// register DPU Handler to IRQ
	ithIntrRegisterHandlerIrq(ITH_INTR_DPU, dpu_isr, NULL);

	// set IRQ to edge trigger
	ithIntrSetTriggerModeIrq(ITH_INTR_DPU, ITH_INTR_EDGE);

	// set IRQ to detect rising edge
	ithIntrSetTriggerLevelIrq(ITH_INTR_DPU, ITH_INTR_HIGH_RISING);

	// Enable IRQ
	ithDpuEnableIntr();
	ithIntrEnableIrq(ITH_INTR_DPU);
	    
    if(!DpuIsrSemaphor)
    {
        DpuIsrSemaphor = malloc(sizeof(sem_t));
	    sem_init(DpuIsrSemaphor, 0, 0);
    }
	
	//printf("DpuIsrSemaphor=%x\n",DpuIsrSemaphor);
	
	printf("Enable DPU IRQ~~leave\n");
}

void DpuDisableIntr(void)
{
	ithDpuDisableIntr();
    ithIntrDisableIrq(ITH_INTR_DPU);
    if(DpuIsrSemaphor)
    {
        sem_destroy(DpuIsrSemaphor);
        free(DpuIsrSemaphor);
        DpuIsrSemaphor = NULL;
    }
}
#endif

static void dpuReverse(unsigned char *buff, unsigned long size)
{
	int i;
	unsigned char tmpByte;

	for(i=0; i<(size/2); i++)
	{
		tmpByte = buff[size-i-1];
		buff[size-i-1] = buff[i];
		buff[i] = tmpByte;
	}
}

#ifdef ENABLE_HW_REVERSE
static void dpuReverseSrcData(unsigned char *buff, unsigned long size)
{
	int i;	
	unsigned char tmpByte;
	unsigned char *ptr8=(unsigned char *)( ((unsigned int)buff+3) & ~3);
	unsigned int *ptr32;
	unsigned int tmpDW;

	if(buff==ptr8)
	{
		ptr32 = (unsigned int *)buff;
	
		if(size == 8)
		{
			tmpDW = ptr32[1];	ptr32[1] = ptr32[0];	ptr32[0] = tmpDW;
		}
		else
		{
			tmpDW = ptr32[3];	ptr32[3] = ptr32[0];	ptr32[0] = tmpDW;			
			tmpDW = ptr32[2];	ptr32[2] = ptr32[1];	ptr32[1] = tmpDW;
		}      
	}
	else
	{
		ptr8 = (unsigned char *)buff;
	    if(size == 8)
	    {
	    	for(i=0; i<4; i++)
	    	{
				tmpByte = buff[i+4];	buff[i+4] = buff[i];	buff[i] = tmpByte;
			}
	    }
	    else
	    {
	    	for(i=0; i<4; i++)
	    	{
				tmpByte = buff[i+12];	buff[i+12] = buff[i];	buff[i] = tmpByte;
				tmpByte = buff[i+8];	buff[i+8] = buff[i+4];	buff[i+4] = tmpByte;
			}
	    }  
	}
}
#endif

static void dpuReverseAllData(unsigned char *buff, unsigned long size, unsigned long interval)
{
	int i;
	
	for(i=0; i<size; i+=interval)
	{
		#ifdef ENABLE_HW_REVERSE
		#ifdef ENABLE_9850_PATCH_ENDIAN_ISSUE
		#else
		dpuReverseSrcData(&buff[i], interval);
		#endif
		#else
		dpuReverse(&buff[i], interval);
		#endif
	}
}

static void _InvalidDpuBuffer(uint8_t* buf, uint32_t size)
{
    ithFlushDCacheRange((void*)buf, size);
    ithFlushMemBuffer();
}

static void DpuInit(void)  
{
    g_DpuInfo.srcLen = 0;
	g_DpuInfo.dstLen = 0;	
    g_DpuInfo.srcBuf = NULL;
    g_DpuInfo.dstBuf = NULL;
	//g_DpuInfo.BlkSize = 0x4000;	
	//g_DpuInfo.TotalCmdqCount = 0;
	
    //ithDpuInit(&g_DpuInfo);
	
    #if defined(DPU_IRQ_ENABLE) 
    DpuEnableIntr();
    #endif
}

ITH_DPU_MODE GetIthDpuMode(ITP_DPU_INFO *Info) 
{
	ITH_DPU_MODE res=ITP_DPU_UNKNOW_MODE; 
	switch(Info->dpuMode)
	{
		case ITP_DPU_AES_MODE:
			switch(Info->cipher)
			{
				case ITP_DPU_CIPHER_ECB:
					res = AES_ECB_MODE;
					break;
				case ITP_DPU_CIPHER_CBC:
					res = AES_CBC_MODE;
					break;
				case ITP_DPU_CIPHER_CFB:
					res = AES_CFB_MODE;
					break;
				case ITP_DPU_CIPHER_OFB:
					res = AES_OFB_MODE;
					break;
				case ITP_DPU_CIPHER_CTR:
					res = AES_CTR_MODE;
					break;
				default:
					printf("[DPU ERROR] incorrect cipher mode of AES\n");
					break;
			}
			break;
		case ITP_DPU_DES_MODE:
			switch(Info->cipher)
			{
				case ITP_DPU_CIPHER_ECB:
					res = DES_ECB_MODE;
					break;
				case ITP_DPU_CIPHER_CBC:
					res = DES_CBC_MODE;
					break;
				case ITP_DPU_CIPHER_CFB:
					res = DES_CFB_MODE;
					break;
				case ITP_DPU_CIPHER_OFB:
					res = DES_OFB_MODE;
					break;
				case ITP_DPU_CIPHER_CTR:
					res = DES_CTR_MODE;
					break;
				default:
					printf("[DPU ERROR] incorrect cipher mode of DES\n");
					break;
			}
			break;
		case ITP_DPU_DES3_MODE:
			switch(Info->cipher)
			{
				case ITP_DPU_CIPHER_ECB:
					res = DES3_ECB_MODE;
					break;
				case ITP_DPU_CIPHER_CBC:
					res = DES3_CBC_MODE;
					break;
				case ITP_DPU_CIPHER_CFB:
					res = DES3_CFB_MODE;
					break;
				case ITP_DPU_CIPHER_OFB:
					res = DES3_OFB_MODE;
					break;
				case ITP_DPU_CIPHER_CTR:
					res = DES3_CTR_MODE;
					break;
				default:
					printf("[DPU ERROR] incorrect cipher mode of DES3\n");
					break;
			}
			break;
		case ITP_DPU_CSA_MODE:
			res = CSA_MODE;
			break;
		case ITP_DPU_CRC_MODE:
			res = CRC_MODE;
			break;
		default:
			printf("[DPU ERROR] incorrect DPU mode\n");
			break;
	}
	return res;
}

static void DpuWaitIdle(ITP_DPU_INFO *Info) 
{
	#if defined(DPU_IRQ_ENABLE)	
	uint8_t     EventRst;	

	if( Info->dpuLen > 1000 )
	{
		EventRst = itpSemWaitTimeout(DpuIsrSemaphor, (unsigned long)(Info->dpuLen/500) );
	}
	else
	{
		EventRst = itpSemWaitTimeout(DpuIsrSemaphor, (unsigned long)2 );
	}
	if(EventRst)
	{
		printf("[ITP_DPU][ERR] itpSemWaitTimeout() error[%x]!!\n",EventRst);
	}
	EventRst = ithDpuWait();
	#else
	bool     EventRst;
	EventRst = ithDpuWait();
	#endif	
}

static bool HwDpuFire(ITP_DPU_INFO *Info)                 
{
	ITP_DPU_INFO TmpInfo;
	uint32_t	i=0, j = 0;
    uint32_t	in_len ;
    uint32_t	out_len;
    uint8_t     cipher_len=8;
	uint32_t	Reg32;
	uint8_t     *srcBuff = Info->srcBuf;
	uint8_t     *DstBuff = Info->dstBuf;
	bool		res=false;
	#if defined(__OPENRTOS__)
	#ifdef	ENABLE_WRITE_BACK_CACHE
	uint8_t *tem_sbuf_base = malloc( Info->dpuLen + DPU_ALIGN_SIZE - 1 );
	uint8_t *tem_dbuf_base = malloc( Info->dpuLen + DPU_ALIGN_SIZE - 1 );
	uint8_t *tem_sbuf = (uint8_t*)((uint32_t)(tem_sbuf_base + DPU_ALIGN_SIZE - 1) & (uint32_t)~(DPU_ALIGN_SIZE - 1));
	uint8_t *tem_dbuf = (uint8_t*)((uint32_t)(tem_dbuf_base + DPU_ALIGN_SIZE - 1) & (uint32_t)~(DPU_ALIGN_SIZE - 1));	
	#else
	uint8_t     *tem_sbuf=malloc(Info->dpuLen+4);
	uint8_t     *tem_dbuf=malloc(Info->dpuLen+4);  
	#endif
	#else
	uint8_t     *tem_sbuf=ithVmemAlloc(Info->dpuLen+4);
	uint8_t     *tem_dbuf=ithVmemAlloc(Info->dpuLen+4);
	#endif
	
	#ifdef	ENABLE_PATCH_DMA_ISSUE
	ithLockMutex(ithStorMutex);	
	#endif
	
	if(!tem_sbuf || !tem_dbuf)
	{
		printf("itp dpu driver out of memory\n");
		goto end;
	}	
	if( Info->dpuMode==ITP_DPU_AES_MODE )	cipher_len = 16;	
		
	#if defined(__OPENRTOS__)
	
	#ifdef	ENABLE_ITP_DPU_DBG_MEASUREMENT
	gettimeofday(&startT, NULL);
	#endif
	
	memcpy(tem_sbuf, srcBuff, Info->dpuLen);		
		
	#ifdef	ENABLE_ITP_DPU_DBG_MEASUREMENT
	gettimeofday(&endT, NULL);
	printf("mcp:Dur=%d,%d\n",itpTimevalDiff(&startT, &endT),Info->dpuLen );
	#endif
	
	if( Info->dpuMode!=ITP_DPU_CRC_MODE && Info->dpuMode!=ITP_DPU_CSA_MODE )
	{
		dpuReverseAllData(tem_sbuf, Info->dpuLen, cipher_len); 
	}
	#else
	if( Info->dpuMode!=ITP_DPU_CRC_MODE && Info->dpuMode!=ITP_DPU_CSA_MODE )
	{
		uint8_t     *buf = malloc(Info->dpuLen);
		memcpy(buf, srcBuff,Info->dpuLen);
		dpuReverseAllData(buf, Info->dpuLen, cipher_len); 
		ithWriteVram((uint32_t*)tem_sbuf, buf, Info->dpuLen);
		free(buf);
	}
	else
	{
		ithWriteVram((uint32_t*)tem_sbuf, Info->srcBuf, Info->dpuLen);
	}
	#endif
	
	if(Info->dpuMode==ITP_DPU_CRC_MODE)
	{
		if(Info->crc_master)
		{
			ithDpuClearCtrl();
			ithDpuInitCrc();
			ithDpuInitCtrl(GetIthDpuMode(Info));
			ithDpuSetSrcAddr((uint32_t)tem_sbuf);
			ithDpuSetSize(Info->dpuLen);
			ithDpuSetEncrypt();

			ithDpuSetDpuEndian(ITP_DPU_LITTLE_ENDIAN);

			#ifdef	ENABLE_WRITE_BACK_CACHE
			_InvalidDpuBuffer(tem_sbuf, Info->dpuLen);
			#endif

			#ifdef	ENABLE_ITP_DPU_DBG_MEASUREMENT
			gettimeofday(&startT, NULL);
			#endif

			ithDpuFire();
			if(ithDpuWait())	res=true;

			#ifdef	ENABLE_ITP_DPU_DBG_MEASUREMENT
			gettimeofday(&endT, NULL);
			printf("itpDpu:Dur=%d\n",itpTimevalDiff(&startT, &endT) );
			#endif
		}
		else
		{
			uint32_t     *ptr32=(uint32_t*)Info->srcBuf;
			//uint32_t     Reg32;
			
			ithDpuClearCtrl();
			ithDpuInitCrc();
			ithDpuSetDpuEndian(ITP_DPU_LITTLE_ENDIAN);	
					
			if( (uint32_t)ptr32 & 0x03 )
			{
				uint8_t *ptr_1 = (uint8_t*)malloc(Info->dpuLen+4);
				uint8_t *ptr8 = (uint8_t*)(((uint32_t)ptr_1 + 3)&~3);
				
				printf("crcS.byte alignment,ptr1=%x.ptr8=%x\n",ptr_1,ptr8);
				
				memcpy(ptr8, Info->srcBuf, Info->dpuLen);
				ptr32 = (uint32_t*)ptr8;
				for(i=0; i<(Info->dpuLen/4) ; i++)
				{
					ithDpuSetCrcData(ptr32[i]);
				}
				free(ptr_1);
			}
			else
			{
				for(i=0; i<(Info->dpuLen/4) ; i++)
				{
					ithDpuSetCrcData(ptr32[i]);
					//ithDpuGetCrcValue(&Reg32);
					//printf("CRC=%x\n",Reg32);
				}
			}
		}
		if((unsigned int)Info->dstBuf&3)
		{
			uint8_t *ptr_1 = (uint8_t*)malloc(8);
			uint32_t *ptr32 = (uint32_t*)(((uint32_t)ptr_1 + 3)&~3); 
			
			ithDpuGetCrcValue((unsigned int*)ptr32);
			memcpy((uint8_t*)Info->dstBuf, (uint8_t*)ptr32, 4);
			free(ptr_1);
		}
		else
		{
			ithDpuGetCrcValue((unsigned int*)Info->dstBuf);
		}
	}
	else
	{
		ithDpuClearCtrl();
		ithDpuInitCtrl(GetIthDpuMode(Info));
		ithDpuSetSrcAddr((uint32_t)tem_sbuf);
		ithDpuSetDstAddr((uint32_t)tem_dbuf);

		{
			#ifdef	ENABLE_WRITE_BACK_CACHE
			uint8_t     *buf = (uint8_t*)malloc( 32 + DPU_ALIGN_SIZE - 1 );
			uint8_t *align_buf = (uint8_t*)((uint32_t)(buf + DPU_ALIGN_SIZE - 1) & (uint32_t)~(DPU_ALIGN_SIZE - 1));
			#else		
			uint8_t     *buf = malloc(32+4);
			uint8_t     *align_buf = buf;
			uint32_t	buf_addr = (uint32_t)buf;
			#endif
			
			if(Info->keyLen)
			{
				#ifdef	ENABLE_WRITE_BACK_CACHE
				#else
				if(buf_addr&0x03)	align_buf = (uint8_t*)((buf_addr&~0x03)+4);
				#endif

				memcpy(align_buf, Info->keyBuf, Info->keyLen/8);

				if(Info->dpuMode==ITP_DPU_DES3_MODE)
				{
					dpuReverse(&align_buf[0], 8);
					dpuReverse(&align_buf[8], 8);
					dpuReverse(&align_buf[16], 8);
				}
				else
				{
					if(Info->dpuMode!=ITP_DPU_CSA_MODE)
					{
						dpuReverse(align_buf, Info->keyLen/8);
					}
				}
				ithDpuSetKey((uint32_t*)align_buf, Info->keyLen/32);
			}
			
			if(Info->vctLen)
			{
				memcpy(align_buf, Info->vctBuf, Info->vctLen/8);
				dpuReverse(align_buf, Info->vctLen/8);
				ithDpuSetVector((uint32_t*)align_buf, Info->vctLen/32);
			}

			free(buf);
		}

		ithDpuSetSize(Info->dpuLen);
		
		if(Info->descrypt)
			ithDpuSetDescrypt();
		else
			ithDpuSetEncrypt();
			
		if( Info->dpuMode==ITP_DPU_CSA_MODE )
		{
			ithDpuSetDpuEndian(ITP_DPU_LITTLE_ENDIAN);
		}
		else
		{
			ithDpuSetDpuEndian(g_DpuEndianSetting);
		}

		#ifdef	ENABLE_WRITE_BACK_CACHE
		_InvalidDpuBuffer(tem_sbuf, Info->dpuLen);
		#endif

		#ifdef	ENABLE_ITP_DPU_DBG_MEASUREMENT
		gettimeofday(&startT, NULL);
		#endif
			
		ithDpuFire();
		DpuWaitIdle(Info);
		
		#ifdef	ENABLE_ITP_DPU_DBG_MEASUREMENT
		gettimeofday(&endT, NULL);
		printf("itpDpu:Dur=%d\n",itpTimevalDiff(&startT, &endT) );
		#endif
			
		#if defined(__OPENRTOS__)
		ithInvalidateDCacheRange((uint32_t*)tem_dbuf, Info->dpuLen);
		memcpy(Info->dstBuf, tem_dbuf, Info->dpuLen); 
		#else
		ithReadVram((uint32_t*)Info->dstBuf, tem_dbuf, Info->dpuLen);
		#endif

		if(Info->dpuMode!=ITP_DPU_CSA_MODE)
		{
			dpuReverseAllData(Info->dstBuf, Info->dpuLen, cipher_len); 
		}
	}

end:
	#if defined(__OPENRTOS__)
	#ifdef	ENABLE_WRITE_BACK_CACHE
	if(tem_sbuf)
	{
		free(tem_sbuf_base);
		tem_sbuf_base=NULL;
		tem_sbuf=NULL;
	}
	if(tem_dbuf)
	{
		free(tem_dbuf_base);
		tem_dbuf_base=NULL;
		tem_dbuf=NULL;
	}
	#else
	if(tem_sbuf)
	{
		free(tem_sbuf);
		tem_sbuf=NULL;
	}
	if(tem_dbuf)
	{
		free(tem_dbuf);
		tem_dbuf=NULL;
	}
	#endif
	#else
	if(tem_sbuf)
	{
		ithVmemFree(tem_sbuf);
		tem_sbuf=NULL;
	}
	if(tem_dbuf)
	{
		ithVmemFree(tem_dbuf);
		tem_dbuf=NULL;
	}
	#endif

	#ifdef	ENABLE_9850_PATCH_RESET_ISSUE
	ithWriteRegA(ITH_DPU_BASE, 0x00000001);
	ithDpuEnableIntr();
	#endif	

	#ifdef	ENABLE_PATCH_DMA_ISSUE
	ithUnlockMutex(ithStorMutex);	
	#endif	

    return true;	
}


static int DpuOpen(const char* name, int flags, int mode, void* info)
{
   int i;

    if (name == NULL)
    {
        errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;

		LOG_ERR "[ITP_DPU_ERR][DpuOpen]name == NULL\n" LOG_END

        return -1;
    }

    for (i = 0; i < ITH_COUNT_OF(ItpDpuMode); i++)
    {
        if (strcmp(name, ItpDpuMode[i]) == 0)
		{
			//ITPDpuMode[i]
			g_DpuInfo.dpuMode = i;
			if(g_DpuInfo.dpuMode==ITP_DPU_CSA_MODE)
			{
				g_DpuInfo.descrypt=1;
			}
			else if(g_DpuInfo.dpuMode==ITP_DPU_CRC_MODE)
			{
				g_DpuInfo.crc_master=1;
			}
			else
			{
				g_DpuInfo.descrypt=0;
				g_DpuInfo.cipher=ITP_DPU_CIPHER_ECB;
			}
            return i;
		}
    }
    errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;

	LOG_ERR "[ITP_DPU_ERR][DpuOpen]dpu not exist: %s\n",name LOG_END

    return -1;
}

static int DpuClose(int file, void* info)
{
    g_DpuInfo.srcLen = 0;
	g_DpuInfo.dstLen = 0;	
	return 0;
}

static int DpuRead(int file, char *ptr, int len, void* info)
{
	int result=0;

	if( ptr==NULL || g_DpuInfo.srcBuf==NULL )
	{
		errno = (ITP_DEVICE_DECOMPRESS << ITP_DEVICE_ERRNO_BIT) | __LINE__;
		
		LOG_ERR "[ITP_DPU_ERR][DpuRead]buffer is NULL [%x,%x]\n", ptr, g_DpuInfo.srcBuf LOG_END
		
		result = 0;
		goto end;
	}

	if( !len || !g_DpuInfo.dpuLen)
	{
		errno = (ITP_DEVICE_DECOMPRESS << ITP_DEVICE_ERRNO_BIT) | __LINE__;

		LOG_ERR "[ITP_DPU_ERR][DpuRead]incorrect argument %x,%x,%x\n", len, g_DpuInfo.srcLen, g_DpuInfo.dstLen LOG_END

		result = 0;
		goto end;
	}

	g_DpuInfo.dstBuf = ptr;

	if( HwDpuFire(&g_DpuInfo) == true )
	{	
		result = g_DpuInfo.dpuLen;
	}
	else
	{
	    errno = (ITP_DEVICE_DECOMPRESS << ITP_DEVICE_ERRNO_BIT) | __LINE__;
	    
		LOG_ERR "[ITP_DPU_ERR][DpuRead]Decompress fail:\n" LOG_END

		result = 0;
		goto end;
	}

end:
	g_DpuInfo.dpuLen  = 0;
	g_DpuInfo.srcBuf=NULL;
	g_DpuInfo.dstBuf=NULL;

    return result;
}

static int DpuWrite(int file, char *ptr, int len, void* info)
{

	if( ptr==NULL || !len )
	{
		errno = (ITP_DEVICE_DECOMPRESS << ITP_DEVICE_ERRNO_BIT) | __LINE__;
		
		LOG_ERR "[ITP_DPU_ERR]incorrect argument %x,%x\n", ptr, len LOG_END
		
		goto end;
	}

	g_DpuInfo.dpuLen = len;
	g_DpuInfo.srcBuf = ptr;

end:
    return 0;
}

static int DpuIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
        DpuInit();
        break;

    case ITP_IOCTL_EXIT:
        //ithDpuExit();
        //DpuClose(int file, void* info);
        break;

	case ITP_IOCTL_SET_DESCRYPT:
		g_DpuInfo.descrypt = *(unsigned char *)ptr;
        break;

	case ITP_IOCTL_SET_CIPHER:
		g_DpuInfo.cipher = *(unsigned char *)ptr;
        break;

	case ITP_IOCTL_SET_KEY_LENGTH:
		g_DpuInfo.keyLen = *(uint32_t *)ptr;
        break;

	case ITP_IOCTL_SET_VECTOR_LENGTH:
		g_DpuInfo.vctLen = *(uint32_t *)ptr;
        break;

	case ITP_IOCTL_SET_KEY_BUFFER:
		g_DpuInfo.keyBuf = (unsigned char *)ptr;
        break;

	case ITP_IOCTL_SET_VECTOR_BUFFER:
		g_DpuInfo.vctBuf = (unsigned char *)ptr;
        break;

	case ITP_IOCTL_SET_CRC_MASTER:
		g_DpuInfo.crc_master = *(unsigned char *)ptr;
        break;

    default:
        errno = (ITP_DEVICE_DECOMPRESS << ITP_DEVICE_ERRNO_BIT) | 1;
        
		LOG_ERR "[ITP_DPU_ERR]DpuIoctl incorrect request (%x)\n", request LOG_END
		
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceDpu =
{
    ":dpu",
    DpuOpen,
    DpuClose,
    DpuRead,
    DpuWrite,
    itpLseekDefault,
    DpuIoctl,
    NULL
};

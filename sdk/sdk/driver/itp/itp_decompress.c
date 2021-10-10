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
#include <pthread.h>
#include "itp_cfg.h"

#define DCPS_IRQ_ENABLE
#define ENABLE_DCPS_MUTEX

//#define ENABLE_DCPS_WB_CACHE
//#define BNABLE_MEASURE_TIME
//#define ENABLE_DCPS_DBG_MSG
/*******************************************************************
 "ENABLE_PATCH_DCPS_DMA_ISSUE" is for fixing the DCPS H/W issue. 
 DPU can not work well with dma function (like: SPI...)
 *******************************************************************/
#if (CFG_CHIP_FAMILY != 9850)
	#define	ENABLE_PATCH_DCPS_DMA_ISSUE
#endif

/*************************************************************************
 "ENABLE_PATCH_DST_BASE_ALIGNMENT_ISSUE" is for workaround the DCPS H/W issue. 
 DCPS will fail if dstination length is not 4 byte alignenmt.
 But it will workaround if dstination buffer address was shift (4-n) bytes.
 ( n = dstination_length % 4 )
 *************************************************************************/
#define ENABLE_PATCH_DST_BASE_ALIGNMENT_ISSUE

#define DCPS_ALIGN_SIZE			(32)
/* ************************************************************ */
/* GLOBAL VARIABLES												*/  
/* ************************************************************ */

static ITH_DCPS_INFO 	g_DcpsInfo;    
static unsigned int 	g_DcpsSetIndex = 0;

#if defined(DCPS_IRQ_ENABLE)
static sem_t* 			DcpsIsrSemaphor = NULL;
static unsigned int 	g_IsrCounter = 0;
#endif

#ifdef	ENABLE_DCPS_WB_CACHE
static unsigned char gDcpsSrcBase = NULL;
#endif

static struct timeval startT, endT;

#ifdef	ENABLE_DCPS_MUTEX
static pthread_mutex_t  dcpsMutex;
static unsigned char gDcps1stInit = 1;
#endif

/* ************************************************************ */
/* function implementation										*/  
/* ************************************************************ */
#if defined(DCPS_IRQ_ENABLE)
static void dcps_isr(void* data)
{
	unsigned int tmp = 0;
	unsigned int QueCnt = 0;
	
	//ithPrintf("$in\n");	
	g_IsrCounter++;

	ithDcpsGetStatus(&g_DcpsInfo);
	tmp = g_DcpsInfo.RegDcpsStatus;
	
	if(tmp&0x04)
	{
	   //decompress fail
	   ithPrintf("decompress fail!!\n");
	   //g_DecompressStatus |= DECOMPRESS_HW_FAIL;   //DECOMPRESS_FAIL  0x0004       
    }


    ithDcpsClearIntr();

    itpSemPostFromISR(DcpsIsrSemaphor);
    	
	//ithPrintf("$[%d,%d]\n",g_CmdQueDoneIndex,g_CmdQueIndex);
	//ithPrintf("$out\n");
}

static void DcpsEnableIntr(void)
{
	// Initialize DCPS IRQ
	//printf("Enable DCPS IRQ~~\n");

	// register DCPS Handler to IRQ
	ithIntrRegisterHandlerIrq(ITH_INTR_DECOMPRESS, dcps_isr, NULL);

	// set IRQ to edge trigger
	ithIntrSetTriggerModeIrq(ITH_INTR_DECOMPRESS, ITH_INTR_EDGE);

	// set IRQ to detect rising edge
	ithIntrSetTriggerLevelIrq(ITH_INTR_NAND, ITH_INTR_HIGH_RISING);

	// Enable IRQ
	ithDcpsEnIntr();
	ithIntrEnableIrq(ITH_INTR_DECOMPRESS);
	    
    if(!DcpsIsrSemaphor)
    {
        DcpsIsrSemaphor = malloc(sizeof(sem_t));
	    sem_init(DcpsIsrSemaphor, 0, 0);
    }
	
	//printf("DcpsIsrSemaphor=%x\n",DcpsIsrSemaphor);
	
	//printf("Enable NAND IRQ~~leave\n");
}

static void DcpsDisableIntr(void)
{
	ithDcpsDisIntr();
    ithIntrDisableIrq(ITH_INTR_DECOMPRESS);
    if(DcpsIsrSemaphor)
    {
        sem_destroy(DcpsIsrSemaphor);
        free(DcpsIsrSemaphor);
        DcpsIsrSemaphor = NULL;
    }
}
#endif

static void DcpsInit(void)  
{
	#ifdef	ENABLE_DCPS_DBG_MSG
	LOG_INFO "DCPS:init\n" LOG_END
	#endif

	#ifdef	ENABLE_DCPS_MUTEX
	if(gDcps1stInit)
	{
		//printf("first init DCPS's mutex!!\n");
		pthread_mutex_init(&dcpsMutex, NULL);
		pthread_mutex_lock(&dcpsMutex);			
		gDcps1stInit = 0;
	}
	else
	{
		pthread_mutex_lock(&dcpsMutex);
	}
	#endif
		
    g_DcpsInfo.srcLen = 0;
	g_DcpsInfo.dstLen = 0;	
    g_DcpsInfo.srcbuf = NULL;
    g_DcpsInfo.dstbuf = NULL;
	g_DcpsInfo.BlkSize = 0x4000;	
	g_DcpsInfo.TotalCmdqCount = 0;
	
    ithDcpsInit(&g_DcpsInfo);
	
    #if defined(DCPS_IRQ_ENABLE) 
    DcpsEnableIntr();
    #endif
}

static void DcpsWaitIdle(ITH_DCPS_INFO *Info) 
{
	#if defined(DCPS_IRQ_ENABLE)	
	uint8_t     EventRst;	

	if( Info->srcLen > 4 )
	{
		EventRst = itpSemWaitTimeout(DcpsIsrSemaphor, (unsigned long)(Info->srcLen/4) );
	}
	else
	{
		EventRst = itpSemWaitTimeout(DcpsIsrSemaphor, (unsigned long)10 );
	}
	if(EventRst)
	{
		printf("[ITP_DCPS][ERR] itpSemWaitTimeout() error[%x]!!\n",EventRst);
		#if (CFG_CHIP_FAMILY == 9070)
		ithPrintRegA((ITH_DPU_BASE+0x100), 0x40);
		#endif
	}
	ithDcpsWait(Info);	
	#else
	ithDcpsWait(Info);			
	#endif	
}

static uint32_t DcpsGetSize( uint8_t *Src, uint32_t SrcLen )
{
	uint32_t DcpsSize=0;
	uint32_t i=0;
	uint32_t out_len=0;
	uint32_t in_len=0;

	while(i<SrcLen)
	{
		out_len = ((Src[i+3]) | (Src[i+2]<<8) | (Src[i+1]<<16) | (Src[i]<<24));
        i=i+4;
        in_len = ((Src[i+3]) | (Src[i+2]<<8) | (Src[i+1]<<16) | (Src[i]<<24));
        i=i+4;

        if (out_len == 0)
		{
			//ITH_DCPS_LOG_ERROR "End of DecompressData, InLen=%08x, OutLen=%08x!\n", in_len, out_len ITH_DCPS_LOG_END
			break;
		}
		   
        if( in_len < out_len)	DcpsSize += out_len;
        else					DcpsSize += in_len;	

		i += in_len;
	}

	return	DcpsSize;	
}

static bool DoDcps(ITH_DCPS_INFO *Info)                 
{
	ITH_DCPS_INFO TmpInfo;
	uint32_t	i=0, j = 0;
    uint32_t	in_len ;
    uint32_t	out_len;
	uint32_t	Reg32;
	uint8_t     *SrcBuff = Info->srcbuf;
	uint8_t     *DstBuff = Info->dstbuf;	
	#ifdef ENABLE_PATCH_DST_BASE_ALIGNMENT_ISSUE
	uint8_t     *alignDstBuff = NULL;
	#endif
	//uint32_t	cnt=0;

	#if defined(DCPS_IRQ_ENABLE)
	uint8_t     EventRst;
	#endif
	
	#if !defined(__OPENRTOS__)
	uint8_t     *SrcVbuf = (uint8_t *)ithVmemAlloc(Info->srcLen+16); 
	uint8_t     *DstVbuf = (uint8_t *)ithVmemAlloc(Info->dstLen+16); 
	#endif
	
	#ifdef	ENABLE_DCPS_DBG_MSG
	printf("src=%x, sL=%x\n",Info->srcbuf,Info->srcLen);
	printf("dst=%x, dL=%x\n",Info->dstbuf,Info->dstLen);
	#endif

	/*
	if(Info->srcLen > Info->dstLen) 
	{
		printf("ERROR: Src is larger than Dest[%d,%d]\n",Info->srcLen ,Info->dstLen);
		return false;
	}
	*/
	if( Info->srcLen>0x4000000 )
	{
		printf("Warning: src data is over 64MB[%d],(Not make sense)\n",Info->srcLen);
	}
	
	if( Info->dstLen>0x4000000 )
	{
		printf("Warning: dst data is over 64MB[%d](Not make sense)\n",Info->dstLen);
	}

    #ifdef	CFG_CPU_WB
    ithFlushDCacheRange((void*)Info->srcbuf, Info->srcLen);
    ithFlushMemBuffer();
    #endif
    
    for (;;)
    {    
        int r = 0;

		if ( (i >= Info->srcLen+8) || (j >= Info->dstLen) )
		{
			if ( (i > Info->srcLen+8) || (j > Info->dstLen) )
			{
				printf("warning: DCPS over Length[%d,%d][%d,%d]\n",i,Info->srcLen ,j ,Info->dstLen);
				printf("src=%x, ",Info->srcbuf);
				printf("srcBuf=[%02x,%02x,%02x,%02x][%02,%02x,%02x,%02x]\n",SrcBuff[0],SrcBuff[1],SrcBuff[2],SrcBuff[3],SrcBuff[4],SrcBuff[5],SrcBuff[6],SrcBuff[7]);
			}			
			break;
		}  

        //get src data length & get output data length
		out_len = ((SrcBuff[i+3]) | (SrcBuff[i+2]<<8) | (SrcBuff[i+1]<<16) | (SrcBuff[i]<<24));
        i=i+4;
        in_len = ((SrcBuff[i+3]) | (SrcBuff[i+2]<<8) | (SrcBuff[i+1]<<16) | (SrcBuff[i]<<24));
        i=i+4;

        //printf("dcps%x.src=%x,%x dst=%x,%x\n",cnt,i,in_len,j,out_len);
        //cnt++;

        if (out_len == 0)
		{
			//ITH_DCPS_LOG_WARING "End of DecompressData, InLen=%08x, OutLen=%08x!\n", in_len, out_len ITH_DCPS_LOG_END
			break;
		}

        if (in_len < out_len)
        {
        	TmpInfo.srcLen = in_len;
        	TmpInfo.dstLen = out_len;

        	#if defined(WIN32)
        	TmpInfo.srcbuf = &SrcVbuf[i];
        	TmpInfo.dstbuf = &DstVbuf[j];
        	#else
        	TmpInfo.srcbuf = &SrcBuff[i];
        	TmpInfo.dstbuf = &DstBuff[j];
			#endif
        	
			#ifdef ENABLE_PATCH_DST_BASE_ALIGNMENT_ISSUE
			if(TmpInfo.dstLen%4)
			{
				alignDstBuff = (uint8_t*)malloc(Info->dstLen+64);
				TmpInfo.dstbuf = (uint8_t*)&alignDstBuff[4-(Info->dstLen%4)];
			}
			#endif
        	
			#ifdef	ENABLE_PATCH_DCPS_DMA_ISSUE
			ithLockMutex(ithStorMutex);	
			#endif

			#ifdef	BNABLE_MEASURE_TIME
			gettimeofday(&startT, NULL);
			#endif

			#ifdef ENABLE_PATCH_DST_BASE_ALIGNMENT_ISSUE
			if (TmpInfo.dstLen%4)
				ithInvalidateDCacheRange((uint32_t*)alignDstBuff, out_len + 4-(Info->dstLen%4));
			else
				ithInvalidateDCacheRange((uint32_t*)TmpInfo.dstbuf, out_len);
			#else
			ithInvalidateDCacheRange((uint32_t*)TmpInfo.dstbuf, out_len);
			#endif

			ithDcpsFire(&TmpInfo);
			DcpsWaitIdle(Info);

			#ifdef	BNABLE_MEASURE_TIME
			gettimeofday(&endT, NULL);
			printf("dcps dur=%d ms\n",itpTimevalDiff(&startT, &endT) );
			#endif

			#ifdef	ENABLE_PATCH_DCPS_DMA_ISSUE
			ithUnlockMutex(ithStorMutex);
			#endif
				
			Info->TotalCmdqCount++;
			
			#ifdef ENABLE_PATCH_DST_BASE_ALIGNMENT_ISSUE
			if(TmpInfo.dstLen%4)
			{
				memcpy(&DstBuff[j], TmpInfo.dstbuf, Info->dstLen);
#ifdef	CFG_CPU_WB
                ithFlushDCacheRange(&DstBuff[j], Info->dstLen);
                ithFlushMemBuffer();
#endif
				if(alignDstBuff != NULL)
				{
					free(alignDstBuff);
					alignDstBuff = NULL;
				}
			}
			#endif
			
			if(Info->RegDcpsStatus&0x04)
			{
				printf("[DCPS ERR] Check DCPS error bit fail!!\n");
				return false;
			}
			ithDcpsGetDoneLen(&out_len);
			
            i += in_len;
            j += out_len;	
        }
        else 
        {
            //#if defined(__OPENRTOS__)
            #if defined(WIN32)
			ithWriteVram((uint32_t)&DstVbuf[j], (void*)&SrcBuff[i], in_len);
            #else
            memcpy( (void*)&DstBuff[j], (void*)&SrcBuff[i], in_len);
            #ifdef	CFG_CPU_WB
            ithFlushDCacheRange((void*)&DstBuff[j], in_len);
            ithFlushMemBuffer();
            #endif
			#endif
            i += in_len;
            j += in_len;
        }
    }
	#if defined(WIN32)
	ithReadVram((uint32_t)Info->dstbuf, (void*)DstVbuf, Info->dstLen);
	#endif

	#ifdef	CFG_CPU_WB
	ithInvalidateDCacheRange((uint32_t*)Info->dstbuf, Info->dstLen);
	#endif

	g_DcpsSetIndex++;

    return true;	
}

static int DecompressRead(int file, char *ptr, int len, void* info)
{
	int result=0;

#ifdef	ENABLE_DCPS_DBG_MSG
	uint8_t*	b=(uint8_t*)g_DcpsInfo.srcbuf;
	LOG_INFO "DCPS:rd %x,%x,%x,[%02x,%02x,%02x,%02x][%02x,%02x,%02x,%02x]\n", ptr, len, b, b[0],b[01],b[2],b[3], b[4],b[5],b[6],b[7] LOG_END
#endif

	if( ptr==NULL || g_DcpsInfo.srcbuf==NULL )
	{
		errno = (ITP_DEVICE_DECOMPRESS << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        LOG_ERR "buffer is NULL [%x,%x]\n", ptr, g_DcpsInfo.srcbuf LOG_END
		result = 0;
		goto end;
	}

	if( !len || !g_DcpsInfo.srcLen || !g_DcpsInfo.dstLen)
	{
		errno = (ITP_DEVICE_DECOMPRESS << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        LOG_ERR "incorrect argument %x,%x,%x\n", len, g_DcpsInfo.srcLen, g_DcpsInfo.dstLen LOG_END
		result = 0;
		goto end;
	}

	if(len != g_DcpsInfo.dstLen)
	{
		errno = (ITP_DEVICE_DECOMPRESS << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        LOG_ERR "len dose not match the calculated value %x,%x,%x\n",len ,g_DcpsInfo.dstLen,g_DcpsInfo.srcbuf LOG_END
        {
        	uint8_t *b=(uint8_t*)g_DcpsInfo.srcbuf;
        	LOG_ERR "srcBuf=%x, [%x,%x,%x,%x][%x,%x,%x,%x]\n",g_DcpsInfo.srcbuf, b[0],b[1],b[2],b[3], b[4],b[5],b[6],b[7] LOG_END
        }
		result = 0;
		goto end;
	}

	g_DcpsInfo.dstbuf = ptr;
	g_DcpsInfo.IsEnableComQ = 0;	//don't use command queue

	if( DoDcps(&g_DcpsInfo) == true )
	{	
		result = g_DcpsInfo.dstLen;
	}
	else
	{
	    errno = (ITP_DEVICE_DECOMPRESS << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        LOG_ERR "Decompress fail:\n" LOG_END
		result = 0;
		goto end;
	}

end:
	g_DcpsInfo.srcLen  = 0;
	g_DcpsInfo.dstLen  = 0;
	g_DcpsInfo.srcbuf=NULL;
	g_DcpsInfo.dstbuf=NULL;
	
	#ifdef	ENABLE_DCPS_WB_CACHE
	if(gDcpsSrcBase)
	{
		free(gDcpsSrcBase);
		gDcpsSrcBase=NULL;
	}
	#endif
/*
	if(g_DcpsInfo.srcbuf)
	{
		free(g_DcpsInfo.srcbuf);
		g_DcpsInfo.srcbuf=NULL;
	}

	if(g_DcpsInfo.dstbuf)
	{
		free(g_DcpsInfo.dstbuf);
		g_DcpsInfo.dstbuf=NULL;
	}
*/
    return result;
}

static int DecompressWrite(int file, char *ptr, int len, void* info)
{
#ifdef	ENABLE_DCPS_DBG_MSG
	uint8_t*	b=(uint8_t*)ptr;
	LOG_INFO "DCPS:wt %x,%x,%x,[%02x,%02x,%02x,%02x][%02x,%02x,%02x,%02x]\n", ptr, len, b, b[0],b[01],b[2],b[3], b[4],b[5],b[6],b[7] LOG_END
#endif
	
	if( ptr==NULL || !len )
	{
		errno = (ITP_DEVICE_DECOMPRESS << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        LOG_ERR "incorrect argument %x,%x\n", ptr, len LOG_END
		goto end;
	}
	
	g_DcpsInfo.dstLen = DcpsGetSize(ptr, len);

	if(!g_DcpsInfo.dstLen)
	{
		errno = (ITP_DEVICE_DECOMPRESS << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        LOG_ERR "decompress length is 0 !!\n" LOG_END
		goto end;
	}

	#ifdef	ENABLE_DCPS_WB_CACHE
	gDcpsSrcBase = malloc(len + DCPS_ALIGN_SIZE - 1 );
	g_DcpsInfo.srcbuf = (uint8_t*)((uint32_t)(gDcpsSrcBase + DCPS_ALIGN_SIZE - 1) & (uint32_t)~(DCPS_ALIGN_SIZE - 1));
	memcpy(g_DcpsInfo.srcbuf, ptr, len);
	g_DcpsInfo.srcLen = len;
	#else
	g_DcpsInfo.srcbuf = ptr;
	g_DcpsInfo.srcLen = len;
	#endif

#ifdef	ENABLE_DCPS_DBG_MSG
	LOG_INFO "dcpsWt:%x,%d\n",g_DcpsInfo.srcbuf,g_DcpsInfo.srcLen LOG_END
#endif

	#ifdef	ENABLE_DCPS_WB_CACHE
	ithFlushDCacheRange((void*)g_DcpsInfo.srcbuf, g_DcpsInfo.srcLen);
	ithFlushMemBuffer();
	#endif

	return len;

end:
    return 0;
}

static int DecompressIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
        DcpsInit();
        break;

    case ITP_IOCTL_EXIT:
    	#if defined(DCPS_IRQ_ENABLE)
    	DcpsDisableIntr();
    	#endif
        ithDcpsExit();
        
      	#ifdef	ENABLE_DCPS_MUTEX
		pthread_mutex_unlock(&dcpsMutex);
		#endif
				
        break;

    case ITP_IOCTL_GET_SIZE:
		*(unsigned long *)ptr = g_DcpsInfo.dstLen;
        break;

    case ITP_IOCTL_SET_VOLUME:
		g_DcpsInfo.BlkSize = *(unsigned long *)ptr;
        break;

    default:
        errno = (ITP_DEVICE_DECOMPRESS << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceDecompress =
{
    ":decompress",
    itpOpenDefault,
    itpCloseDefault,
    DecompressRead,
    DecompressWrite,
    itpLseekDefault,
    DecompressIoctl,
    NULL
};

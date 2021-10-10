#include <malloc.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "../../include/ssp/mmp_spi.h"
#include "../../include/nor/ftssp.h"
#include "../../include/nor/mmp_nor.h"
#include "../../include/nor/nor.h"
#include "ite/itp.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define NOR_BUS					SPI_0
#define INVALID_SIZE            (-1)
#define SSP_POLLING_COUNT       (0x100000)
#define WRITE_DATA_COUNTS       8
#define WRITE_DATA_BYTES        28      //according to fifo length
#ifdef _WIN32
#define PER_READ_BYTES          28      //14*2 (fifo size - input size)
#else
#define PER_READ_BYTES          60      //15*4 (fifo size - input size)
#endif
#define PER_DMA_BYTES			2048

#ifdef __FREERTOS__
#define WriteReg    ithWriteRegA
#define ReadReg     ithReadRegA
#else
#define WriteReg    ithWriteRegH
#define ReadReg     ithReadRegH
#endif

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct
{
    uint16_t sectorNum;
    uint32_t startAddr;
    uint32_t endAddr;
}NOR_ADDR_MAP;

typedef enum
{
    AMIC_A25L032,
    AMIC_A25LQ32A,
    ATMEL_AT26DF161,
    ATMEL_AT26D321,
    EON_EN25P32,
    EON_EN25B16,
    EON_EN25B32,
    EON_EN25B64,
    EON_EN25F16,
    EON_EN25F32,
    EON_EN25Q16,
    EON_EN25Q32A,
    EON_EN25F80,
    EON_EN25Q80A,
    EON_EN25Q64,
    EON_EN25QH32,
    ES_ES25M16A,
    ESMT_F25L16A,
    ESMT_F25L32Q,
    ESMT_F25L32PA,//kenny
    Micron_N25Q032A,//Aileen
    MX_25L1605A,
    MX_25L3205D,
    MX_25L3235D,
    MX_25L1635D,
    MX_25L6445E,
    MX_25L12835F,
    MX_25L25635F,
    MX_25L25735F,
    GD_GD25Q64B,
    GD_GD25Q32,
    GD_GD25Q16,
    NUMON_M25P20,
    NUMON_M25P32,
    PMC_PM25LQ032C,//MY
	SPAN_S25FL016A,
	SPAN_S25FL032A,
    SST_25VF016B,
    WIN_W25X16A,
    WIN_W25X32V,
    WIN_W25Q32BV,
    WIN_W25Q64BV,
    GD_GD25Q128C,
    SPAN_S25FL127S,
    EON_EN25QH128,
    GD_GD25Q256C,
    UNKNOW_VENDOR = 0xFFFF
}NOR_VENDOR_ID;

typedef struct
{
    uint8_t       manufatureID;
    uint16_t      deviceID;
   	uint8_t       deviceID2;
    uint8_t*      deviceName;
    NOR_VENDOR_ID vendorID;
}NOR_VENDOR_CONTEXT;

typedef enum
{
    NOR_BIG_ENDIAN,
    NOR_LITTLE_ENDIAN
}NOR_ENDIAN_TYPE;

typedef struct
{
    uint16_t bytesPerPage;
    uint16_t pagesPerSector;
	int32_t  bytesPerSector;
	uint16_t sectorsPerBlock;
    uint16_t totalBlocks;
    // Operation
    bool use4BytesAddress;
}NOR_ADDR_CONTEXT;


//=============================================================================
//                              Global Data Definition
//=============================================================================
SPI_CONTEXT            g_SpiContext;
NOR_ADDR_CONTEXT       g_norAddrMap = {0};
uint16_t               g_initCount;
NOR_VENDOR_ID          g_vendor = UNKNOW_VENDOR;
static NOR_ENDIAN_TYPE g_endian;
static pthread_mutex_t NorInternalMutex  = PTHREAD_MUTEX_INITIALIZER;

NOR_VENDOR_CONTEXT nor_support_vendor[] = {
    {0x37, 0x3016, 0x15, "AMIC__A25L032",   AMIC_A25L032},
    {0x37, 0x4016, 0x15, "AMIC_A25LQ32A",   AMIC_A25LQ32A},
    {0x1F, 0x4600, 0xFF, "ATMEL_AT26DF161", ATMEL_AT26DF161},
    {0x1F, 0x4700, 0xFF, "ATMEL__AT26D321", ATMEL_AT26D321},
    {0x1C, 0x2016, 0x15, "EON__EN25P32",    EON_EN25P32},
    {0x1C, 0x2015, 0x34, "EON__EN25B16",    EON_EN25B16},
    {0x1C, 0x2016, 0x35, "EON__EN25B32",    EON_EN25B32},
    {0x1C, 0x2017, 0x36, "EON__EN25B64",    EON_EN25B64},
    {0x1C, 0x3115, 0x14, "EON__EN25F16",    EON_EN25F16},
    {0x1C, 0x3116, 0x15, "EON__EN25F32",    EON_EN25F32},
    {0x1C, 0x3015, 0x14, "EON_EN25Q16",     EON_EN25Q16},
    {0x1C, 0x3016, 0x15, "EON_EN25Q32A",    EON_EN25Q32A},
    {0x1C, 0x3114, 0x13, "EON_EN25F80",     EON_EN25F80},
    {0x1C, 0x3014, 0x13, "EON_EN25Q80A",    EON_EN25Q80A},
    {0x1C, 0x3017, 0x16, "EON_EN25Q64",     EON_EN25Q64},
    {0x1C, 0x7016, 0x15, "EON_EN25QH32",    EON_EN25QH32},
    {0x4A, 0x3215, 0x14, "ES__ES25M16A",    ES_ES25M16A},
    {0x8C, 0x2015, 0x14, "ESMT_F25L16PA",   ESMT_F25L16A},
    {0x8C, 0x4116, 0x15, "ESMT_F25L32Q",    ESMT_F25L32Q},
    {0x8C, 0x2016, 0x15, "ESMT_F25L32PA",    ESMT_F25L32PA},//kenny
    {0x20, 0xBA16, 0xFF, "Micron_N25Q032A",    Micron_N25Q032A},//Aileen
    {0xC2, 0x2015, 0x14, "MX__25L1605A",    MX_25L1605A},
    {0xC2, 0x2016, 0x15, "MX__25L3205D",    MX_25L3205D},
    {0xC2, 0x5E16, 0x5E, "MX__25L3235D",    MX_25L3235D},
    {0xC2, 0x2415, 0x24, "MX__25L1635D",    MX_25L1635D},
    {0xC2, 0x2017, 0x16, "MX_25L6445E",     MX_25L6445E},
    {0xC2, 0x2018, 0x17, "MX_25L12835F",    MX_25L12835F},
    {0xC2, 0x2019, 0x18, "MX_25L25635F",    MX_25L25635F},
    {0xC2, 0x2019, 0x18, "MX_25L25735F",    MX_25L25735F},
    {0xC8, 0x4017, 0x16, "GD_GD25Q64B",     GD_GD25Q64B},
    {0xC8, 0x4016, 0x15, "GD_GD25Q32",      GD_GD25Q32},
    {0xC8, 0x4015, 0x14, "GD_GD25Q16",      GD_GD25Q16},
    {0x20, 0x2012, 0xFF, "NUMON_M25P20",    NUMON_M25P20},
    {0x20, 0x2016, 0xFF, "NUMON_M25P32",    NUMON_M25P32},
    {0x7F, 0x9D46, 0x15, "PMC_PM25LQ032C", PMC_PM25LQ032C},    //MY    
	{0x01, 0x0214, 0xFF, "SPAN_S25FL016A",  SPAN_S25FL016A},
	{0x01, 0x0215, 0xFF, "SPAN_S25FL032A",  SPAN_S25FL032A},
    {0xBF, 0x2541, 0x41, "SST_25VF016B",    SST_25VF016B},
    {0xEF, 0x3015, 0x14, "WIN__W25X16A",    WIN_W25X16A},
    {0xEF, 0x3016, 0x15, "WIN_W25X32V",     WIN_W25X32V},
    {0xEF, 0x4016, 0x15, "WIN_W25Q32BV",    WIN_W25Q32BV},
    {0xEF, 0x4017, 0x16, "WIN__W25Q64BV",   WIN_W25Q64BV},
    {0xc8, 0x4018, 0x17, "GD_GD25Q128C",    GD_GD25Q128C},
    {0x01, 0x0017, 0xFF, "SPAN_S25FL127S",  SPAN_S25FL127S},
    {0x1C, 0x7018, 0x17, "EON_EN25QH128",  EON_EN25QH128},
    {0xC8, 0x4019, 0x18, "GD_GD25Q256C",   GD_GD25Q256C},
};

//=============================================================================
//                              Private Function Definition
//=============================================================================
static int32_t
NorSendCommand(
    FLASH_CMD_TYPE cmdType,
    uint32_t       arg,
	uint32_t       data,
    uint32_t       size)

{
    int32_t  result        = 0;
    uint8_t  cmd           = 0;
    uint8_t  inputData[5]  = {0};
    uint32_t inputDataSize = 0;

    switch(cmdType)
    {
    case ERASE_ALL:
        //C7h/ 60h
        cmd = ERASE_BULK_CMD;
        inputData[0] = cmd;
        result = mmpSpiPioWrite(NOR_BUS , inputData, 1, 0, 0, 8);
        break;

	case WRITE_EN:
        //06h
		cmd = WRITE_EN_CMD;
        inputData[0] = cmd;
        result = mmpSpiPioWrite(NOR_BUS, inputData, 1, 0, 0, 8);
		break;

	case WRITE_DIS:
        //04h
        cmd = WRITE_DIS_CMD;
        inputData[0] = cmd;
        result = mmpSpiPioWrite(NOR_BUS, inputData, 1, 0, 0, 8);
        break;

	case WRITE_STATUS_EN:
        cmd = WRITE_STATUS_EN_CMD;
        inputData[0] = cmd;
        result = mmpSpiPioWrite(NOR_BUS, inputData, 1, 0, 0, 8);
		break;

	case WRITE_STATUS:
        //01h S7-S0
        cmd = WRITE_STATUS_CMD;
        inputData[0] = cmd;
        inputData[1] = arg;
        result = mmpSpiPioWrite(NOR_BUS, inputData, 2, 0, 0, 16);
		break;

    case ERASE_SECTOR:
        //20h A23-A16 A15-A8 A7-A0
        if ( g_norAddrMap.use4BytesAddress == true )
        {
            cmd = ERASE_SECOTR4B_CMD;
        }
        else
        {
            cmd = ERASE_SECOTR_CMD;
        }
        inputData[0] = cmd;
		if ( g_norAddrMap.use4BytesAddress == true )
	    {
	    	inputData[1] = arg >> 24;
		    inputData[2] = arg >> 16;
		    inputData[3] = arg >> 8;
	    	inputData[4] = arg;
	    	inputDataSize = 5;
	    }
	    else
	    {
		    inputData[1] = arg >> 16;
		    inputData[2] = arg >> 8;
		    inputData[3] = arg;
		    inputDataSize = 4;
	    }
#if defined(_WIN32) || (defined(__OPENRTOS__) || defined(__FREERTOS__))
        if ( g_norAddrMap.use4BytesAddress == true )
        {
            result = mmpSpiPioWrite(NOR_BUS, inputData, inputDataSize, 0, 0, 8);
        }
        else
        {
            result = mmpSpiPioWrite(NOR_BUS, inputData, 4, 0, 0, 16);
        }
#else
        if ( g_norAddrMap.use4BytesAddress == true )
        {
            result = mmpSpiPioWrite(NOR_BUS, inputData, inputDataSize, 0, 0, 8);
        }
        else
        {
            result = mmpSpiPioWrite(NOR_BUS, inputData, 4, 0, 0, 32);
        }
#endif
        break;

	case ERASE_BLOCK:
        //D8h/ 52h A23-A16 A15-A8 A7-A0
        if ( g_norAddrMap.use4BytesAddress == true )
        {
            cmd = ERASE_BLOCK4B_CMD;
        }
        else
        {
            cmd = ERASE_BLOCK_CMD;
        }
        inputData[0] = cmd;
		if ( g_norAddrMap.use4BytesAddress == true )
	    {
	    	inputData[1] = arg >> 24;
		    inputData[2] = arg >> 16;
		    inputData[3] = arg >> 8;
	    	inputData[4] = arg;
	    	inputDataSize = 5;
	    }
	    else
	    {
		    inputData[1] = arg >> 16;
		    inputData[2] = arg >> 8;
		    inputData[3] = arg;
		    inputDataSize = 4;
	    }
#if defined(_WIN32) || (defined(__OPENRTOS__) || defined(__FREERTOS__))
        if ( g_norAddrMap.use4BytesAddress == true )
        {
            result = mmpSpiPioWrite(NOR_BUS, inputData, inputDataSize, 0, 0, 8);
        }
        else
        {
            result = mmpSpiPioWrite(NOR_BUS, inputData, 4, 0, 0, 16);
        }
#else
        if ( g_norAddrMap.use4BytesAddress == true )
        {
            result = mmpSpiPioWrite(NOR_BUS, inputData, inputDataSize, 0, 0, 8);
        }
        else
        {
            result = mmpSpiPioWrite(NOR_BUS, inputData, 4, 0, 0, 32);
        }
#endif
        break;

    case AUTO_WRITE:
        break;

	case AUTO_WRITE_DATA_ONLY:
		break;

    case WRITE:
        //02h A23-A16 A15-A8 A7-A0 D7-D0 (Next byte) continuous
        if ( g_norAddrMap.use4BytesAddress == true )
        {
            cmd = PAGE_PROGRAM4B_CMD;
        }
        else
        {
            cmd = PAGE_PROGRAM_CMD;
        }
        inputData[0] = cmd;
		if ( g_norAddrMap.use4BytesAddress == true )
	    {
	    	inputData[1] = arg >> 24;
		    inputData[2] = arg >> 16;
		    inputData[3] = arg >> 8;
	    	inputData[4] = arg;
	    	inputDataSize = 5;
	    }
	    else
	    {
		    inputData[1] = arg >> 16;
		    inputData[2] = arg >> 8;
		    inputData[3] = arg;
		    inputDataSize = 4;
	    }
#if defined(_WIN32) || (defined(__OPENRTOS__) || defined(__FREERTOS__))
        if ( g_norAddrMap.use4BytesAddress == true )
        {
            if(size >= 16 || (size & 1))
            {
    			result = mmpSpiDmaWrite(NOR_BUS, inputData, inputDataSize, (void*)data, size, 8);
            }
            else
            {
    			result = mmpSpiPioWrite(NOR_BUS, inputData, inputDataSize, (void*)data, size, 8);

    		}
        }
        else
        {
            if(size >= 16 /*g_norAddrMap.bytesPerPage*/ || (size & 1))
            {
    			result = mmpSpiDmaWrite(NOR_BUS, inputData, 4, (void*)data, size, 8);
            }
            else
            {
                result = mmpSpiPioWrite(NOR_BUS, inputData, 4, (void*)data, size, 16);
    			//result = mmpSpiPioWrite(NOR_BUS, inputData, 4, (void*)data, size, 8);
    		}
        }
#else
        if ( g_norAddrMap.use4BytesAddress == true )
        {
            result = mmpSpiPioWrite(NOR_BUS, inputData, inputDataSize, (void*)data, size, 32);
        }
        else
        {
            result = mmpSpiPioWrite(NOR_BUS, inputData, 4, (void*)data, size, 32);
        }
#endif
        break;

#if 1
	case WRITE_MULTI:
        //02h A23-A16 A15-A8 A7-A0 D7-D0 (Next byte) continuous
        if ( size >= (4 * 1024 * 1024) )
        {
        	uint32_t remainderSize = size;
			uint32_t writeAddr = arg;
			uint32_t writeData = data;
			uint32_t writeSize = size;

            if ( g_norAddrMap.use4BytesAddress == true )
            {
                cmd = PAGE_PROGRAM4B_CMD;
            }
            else
            {
                cmd = PAGE_PROGRAM_CMD;
            }
	        inputData[0] = cmd;

	        while ( remainderSize )
	        {
	        	if ( remainderSize >= (4 * 1024 * 1024) )
	        	{
	        		writeSize = (4 * 1024 * 1024);
	        	}
	        	else
	        	{
	        		writeSize = remainderSize;
	        	}

	        	if ( g_norAddrMap.use4BytesAddress == true )
			    {
			    	inputData[1] = writeAddr >> 24;
				    inputData[2] = writeAddr >> 16;
				    inputData[3] = writeAddr >> 8;
			    	inputData[4] = writeAddr;
			    	inputDataSize = 5;
			    }
			    else
			    {
				    inputData[1] = writeAddr >> 16;
				    inputData[2] = writeAddr >> 8;
				    inputData[3] = writeAddr;
				    inputDataSize = 4;
			    }

				printf("%s[%d]writeAddr = 0x%08X, writeData = 0x%08X, remainderSize = %d Bytes\n", __FUNCTION__, __LINE__,
					writeAddr, writeData, remainderSize);
			    result = mmpSpiDmaWriteMulti(NOR_BUS, inputData, inputDataSize, (void*)writeData, writeSize, 8, g_norAddrMap.bytesPerPage);

			    writeAddr += writeSize;
			    writeData += writeSize;
			    remainderSize -= writeSize;
	        }
        }
        else
        {
            if ( g_norAddrMap.use4BytesAddress == true )
            {
                cmd = PAGE_PROGRAM4B_CMD;
            }
            else
            {
                cmd = PAGE_PROGRAM_CMD;
            }
	        inputData[0] = cmd;
			if ( g_norAddrMap.use4BytesAddress == true )
		    {
		    	inputData[1] = arg >> 24;
			    inputData[2] = arg >> 16;
			    inputData[3] = arg >> 8;
		    	inputData[4] = arg;
		    	inputDataSize = 5;
		    }
		    else
		    {
			    inputData[1] = arg >> 16;
			    inputData[2] = arg >> 8;
			    inputData[3] = arg;
			    inputDataSize = 4;
		    }
			if ( size >= 256 )
			{
				printf("GGGGGGGGGGGGg size = %d\n", size);
				result = mmpSpiDmaWriteMulti(NOR_BUS, inputData, inputDataSize, (void*)data, size, 8, g_norAddrMap.bytesPerPage);
			}
        }
        break;
#else
	case WRITE_MULTI:
        //02h A23-A16 A15-A8 A7-A0 D7-D0 (Next byte) continuous
        if ( g_norAddrMap.use4BytesAddress == true )
        {
            cmd = PAGE_PROGRAM4B_CMD;
        }
        else
        {
            cmd = PAGE_PROGRAM_CMD;
        }
        inputData[0] = cmd;
		if ( g_norAddrMap.use4BytesAddress == true )
	    {
	    	inputData[1] = arg >> 24;
		    inputData[2] = arg >> 16;
		    inputData[3] = arg >> 8;
	    	inputData[4] = arg;
	    	inputDataSize = 5;
	    }
	    else
	    {
		    inputData[1] = arg >> 16;
		    inputData[2] = arg >> 8;
		    inputData[3] = arg;
		    inputDataSize = 4;
	    }
		if ( size >= 256 )
		{
			result = mmpSpiDmaWriteMulti(NOR_BUS, inputData, inputDataSize, (void*)data, size, 8, g_norAddrMap.bytesPerPage);
		}
        break;
#endif

    case READ:
        //03h A23-A16 A15-A8 A7-A0 (D7-D0) (Next byte) continuous
        if ( g_norAddrMap.use4BytesAddress == true )
        {
            cmd = READ4B_CMD;
        }
        else
        {
            cmd = READ_CMD;
        }
        inputData[0] = cmd;
        if ( g_norAddrMap.use4BytesAddress == true )
	    {
	    	inputData[1] = arg >> 24;
		    inputData[2] = arg >> 16;
		    inputData[3] = arg >> 8;
	    	inputData[4] = arg;
	    	inputDataSize = 5;
	    }
	    else
	    {
		    inputData[1] = arg >> 16;
		    inputData[2] = arg >> 8;
		    inputData[3] = arg;
		    inputDataSize = 4;
	    }
#if defined(_WIN32) || (defined(__OPENRTOS__) || defined(__FREERTOS__))
        if ( g_norAddrMap.use4BytesAddress == true )
        {
           if(size >= 16 || (size&1))
           {
               result = mmpSpiDmaRead(NOR_BUS, inputData, inputDataSize, (void*)data, size, 8);
           }
           else
           {
               result = mmpSpiPioRead(NOR_BUS, inputData, inputDataSize, (void*)data, size, 8);
           }
        }
        else
        {
            if(size >= 16 || (size&1))
            {
                result = mmpSpiDmaRead(NOR_BUS, inputData, 4, (void*)data, size, 8);
            }
            else
            {
                result = mmpSpiPioRead(NOR_BUS, inputData, 4, (void*)data, size, 16);
            }
        }
#else
        if ( g_norAddrMap.use4BytesAddress == true )
        {
            result = mmpSpiPioRead(NOR_BUS, inputData, inputDataSize, (void*)data, size, 32);
        }
        else
        {
            result = mmpSpiPioRead(NOR_BUS, inputData, 4, (void*)data, size, 32);
        }
#endif
        break;

    case FAST_READ:
        //0Bh A23-A16 A15-A8 A7-A0 dummy (D7-D0) (Next Byte) continuous
        //TODO
        if ( g_norAddrMap.use4BytesAddress == true )
        {
            cmd = FAST_READ4B_CMD;
        }
        else
        {
            cmd = FAST_READ_CMD;
        }
        inputData[0] = cmd;
		if ( g_norAddrMap.use4BytesAddress == true )
	    {
	    	inputData[1] = arg >> 24;
		    inputData[2] = arg >> 16;
		    inputData[3] = arg >> 8;
	    	inputData[4] = arg;
	    	inputDataSize = 5;
	    }
	    else
	    {
		    inputData[1] = arg >> 16;
		    inputData[2] = arg >> 8;
		    inputData[3] = arg;
		    inputDataSize = 4;
	    }
#if defined(_WIN32) || (defined(__OPENRTOS__) || defined(__FREERTOS__))
        if ( g_norAddrMap.use4BytesAddress == true )
        {
            result = mmpSpiPioRead(NOR_BUS, inputData, inputDataSize, (void*)data, size, 8);
        }
        else
        {
            result = mmpSpiPioRead(NOR_BUS, inputData, 4, (void*)data, size, 16);
        }
#else
        if ( g_norAddrMap.use4BytesAddress == true )
        {
            result = mmpSpiPioRead(NOR_BUS, inputData, inputDataSize, (void*)data, size, 8);
        }
        else
        {
            result = mmpSpiPioRead(NOR_BUS, inputData, 4, (void*)data, size, 32);
        }
#endif
        break;

    case READ_STATUS:
        //05h (S7-S0)(1)
        cmd = READ_STATUS_CMD;
        inputData[0] = cmd;
        mmpSpiPioRead(NOR_BUS, inputData, 1, (void*)data, 1, 8);
        break;

    case READ_ID:
        //9Fh (M7-M0) (ID15-ID8) (ID7-ID0)
        cmd = JEDEC_READ_ID;
        inputData[0] = cmd;
        result = mmpSpiPioRead(NOR_BUS, inputData, 1, (void*)data, 5, 8);
        break;

    case READ_DEVICE:
   		//90h dummy dummy 00h (M7-M0) (ID7-ID0)
		cmd = READ_ID_CMD;
		inputData[0] = cmd;
		inputData[1] = arg>>16;
		inputData[2] = arg>>8;
		inputData[3] = arg;
        result = mmpSpiPioRead(NOR_BUS, inputData, 4, (void*)data, 2, 8);
		break;
    case EN4B_MODE:
   		//B7h
		cmd = EN4B_MODE;
		inputData[0] = cmd;
        result = mmpSpiPioWrite(NOR_BUS, inputData, 1, 0, 0, 8);
        break;
    case EX4B_MODE:
        //E9h
		cmd = EX4B_MODE;
		inputData[0] = cmd;
        result = mmpSpiPioWrite(NOR_BUS, inputData, 1, 0, 0, 8);
        break;
    default:
        break;
    }

    return 0;
}

static void
SetPadSel(void)
{
	uint32_t data;

	data = ithReadRegA(ITH_GPIO_BASE + 0x90);

	// GPIO9
	data &= (3<<18);
	data |= (2<<18);
	ithWriteRegA(ITH_GPIO_BASE + 0x90,data);
	// MUDEX
	data = ithReadRegA(ITH_GPIO_BASE + 0xD0);
	// GPIO9
	data &= ~0x7;
	data |= 0x0006;
	ithWriteRegA(ITH_GPIO_BASE + 0xD0,data);
}

static uint32_t
NorCheckReady(
    void)
{
    uint32_t i     = 0;
    uint32_t state = NOR_ERROR_STATUS_BUSY;
    uint8_t  temp  = 0;

	i = SSP_POLLING_COUNT;
    while(--i)
    {
        NorSendCommand(READ_STATUS, 0, (uint32_t)&temp, 1);
        if(temp & NOR_DEVICE_BUSY)
        {
            continue;
        }
		else
		{
			state = NOR_DEVICE_READY;
            goto done;
		}
    }

    state = NOR_ERROR_DEVICE_TIMEOUT;

done:
    return state;

}

static uint32_t
NorCheckReadyEx(
    void)
{
    uint32_t i     = 0;
    uint32_t state = NOR_ERROR_STATUS_BUSY;
    uint8_t  temp  = 0;

	i = SSP_POLLING_COUNT;
    while(--i)
    {
        NorSendCommand(READ_STATUS, 0, (uint32_t)&temp, 1);
        if(temp & NOR_DEVICE_BUSY)
        {
            usleep(100);
            continue;
        }
		else
		{
			state = NOR_DEVICE_READY;
            goto done;
		}
    }

    state = NOR_ERROR_DEVICE_TIMEOUT;

done:
    return state;

}

static NOR_VENDOR_ID
Nor_ReadID(
    void)
{
    uint16_t      i            = 0;
    uint8_t       manufatureID = 0;
    uint16_t      deviceID     = 0;
    uint8_t       deviceID2    = 0;
    uint8_t       data[8]      = {0};
    NOR_VENDOR_ID vendorID     = UNKNOW_VENDOR;

    NorSendCommand(READ_ID, 0, (uint32_t)data, 0);
    manufatureID = (uint8_t)(data[0]);
    deviceID     = (uint16_t)((data[1]<<8) | data[2]);
    if( manufatureID == 0x1F )	// ATMEL manufacture ID
    {
        deviceID2 = 0xFF;
    }
	else if(manufatureID == 0x01) // SPANSION manufacture ID
	{
		deviceID2 = 0xFF;
	}
	else if(manufatureID == 0x20) // Numonyx manufacture ID
	{
		deviceID2 = 0xFF;
	}
    else
    {
        NorSendCommand(READ_DEVICE, 0, (uint32_t)data, 2);
        deviceID2 = (uint8_t)(data[1]);
    }

	printf("manufatureID = 0x%X, deviceID = 0x%X, deviceID2 = 0x%X\n", manufatureID, deviceID, deviceID2);

    for(i = 0; i < (sizeof(nor_support_vendor)/sizeof(nor_support_vendor[0])); ++i)
    {
        if(manufatureID == nor_support_vendor[i].manufatureID)
        {
            if(deviceID == nor_support_vendor[i].deviceID)
            {
                if(deviceID2 == nor_support_vendor[i].deviceID2)
                {
                    vendorID = nor_support_vendor[i].vendorID;
                    printf("[NOR][INFO] DEVICE IS %s \n", nor_support_vendor[i].deviceName);
                    break;
                }
            }
        }
    }

    if(vendorID == UNKNOW_VENDOR)
    {
        printf("[NOR][ERROR] DEVICE UNKNOW  \n");
    }

    return vendorID;

}

static NOR_RESULT
InitNorFlash(
    void)
{
    NOR_RESULT result = 0;

	//SetPadSel();
    g_vendor = Nor_ReadID();
    if(g_vendor == UNKNOW_VENDOR)
    {
        result = 1;
        goto end;
    }

    switch(g_vendor)
    {
    case NUMON_M25P20:
        g_norAddrMap.bytesPerPage		= 256;
        g_norAddrMap.pagesPerSector		= 256;
        g_norAddrMap.bytesPerSector		= 64*1024;
        g_norAddrMap.sectorsPerBlock	= 1;
        g_norAddrMap.totalBlocks		= 4;
        break;

    case ESMT_F25L16A:
        g_norAddrMap.bytesPerPage		= 256;
        g_norAddrMap.pagesPerSector		= 256;
        g_norAddrMap.bytesPerSector		= 64*1024;
        g_norAddrMap.sectorsPerBlock	= 1;
        g_norAddrMap.totalBlocks		= 32;
        break;

    case EON_EN25P32:
    case NUMON_M25P32:
        g_norAddrMap.bytesPerPage		= 256;
        g_norAddrMap.pagesPerSector		= 256;
        g_norAddrMap.bytesPerSector		= 64*1024;
        g_norAddrMap.sectorsPerBlock	= 1;
        g_norAddrMap.totalBlocks		= 64;
        break;

    case EON_EN25B16:
        g_norAddrMap.bytesPerPage		= 256;
        g_norAddrMap.pagesPerSector		= 256;
        g_norAddrMap.bytesPerSector		= 64*1024;
        g_norAddrMap.sectorsPerBlock	= 1;
        g_norAddrMap.totalBlocks		= 32;   //31x64k + 1x32k + 1x16k + 1x8k + 2x4k
        break;

    case MX_25L1605A:
    case MX_25L1635D:
    case WIN_W25X16A:
    case ATMEL_AT26DF161:
	case SPAN_S25FL016A:
        g_norAddrMap.bytesPerPage		= 256;
        g_norAddrMap.pagesPerSector		= 256;
        g_norAddrMap.bytesPerSector		= 64*1024;
        g_norAddrMap.sectorsPerBlock	= 1;
        g_norAddrMap.totalBlocks		= 32;
        break;

    case EON_EN25F16:
    case EON_EN25Q16:
        g_norAddrMap.bytesPerPage		= 256;
        g_norAddrMap.pagesPerSector		= 256;
        g_norAddrMap.bytesPerSector		= 64*1024;
        g_norAddrMap.sectorsPerBlock	= 1;
        g_norAddrMap.totalBlocks		= 32;   //31x64k + 1x32k + 1x16k + 1x8k + 2x4k
        break;

    case EON_EN25B32:
    case EON_EN25F32:
    case EON_EN25Q32A:
    case EON_EN25QH32:
    case Micron_N25Q032A:  //Aileen
    case MX_25L3235D:
    case MX_25L3205D:
    case PMC_PM25LQ032C://MY
	case SPAN_S25FL032A:
    case WIN_W25X32V:
    case WIN_W25Q32BV:
    case ESMT_F25L32Q:
   case ESMT_F25L32PA://kenny
        g_norAddrMap.bytesPerPage		= 256;
        g_norAddrMap.pagesPerSector		= 256;
        g_norAddrMap.bytesPerSector		= 64*1024;
        g_norAddrMap.sectorsPerBlock	= 1;
        g_norAddrMap.totalBlocks		= 64;
        break;

    case SST_25VF016B:
        g_norAddrMap.bytesPerPage		= 512;
        g_norAddrMap.pagesPerSector		= 128;
        g_norAddrMap.bytesPerSector		= 64*1024;
        g_norAddrMap.sectorsPerBlock	= 1;
        g_norAddrMap.totalBlocks		= 32;
        break;

    case AMIC_A25L032:
    case AMIC_A25LQ32A:
        g_norAddrMap.bytesPerPage		= 256;
        g_norAddrMap.pagesPerSector		= 256;
        g_norAddrMap.bytesPerSector		= 64*1024;
        g_norAddrMap.sectorsPerBlock	= 1;
        g_norAddrMap.totalBlocks		= 64;
        break;

    case ATMEL_AT26D321:
        g_norAddrMap.bytesPerPage		= 256;
        g_norAddrMap.pagesPerSector		= 256;
        g_norAddrMap.bytesPerSector		= 64*1024;
        g_norAddrMap.sectorsPerBlock	= 1;
        g_norAddrMap.totalBlocks		= 64;
        break;

    case ES_ES25M16A:
        g_norAddrMap.bytesPerPage		= 256;
        g_norAddrMap.pagesPerSector		= 256;
        g_norAddrMap.bytesPerSector		= 64*1024;
        g_norAddrMap.sectorsPerBlock	= 1;
        g_norAddrMap.totalBlocks		= 32;
        break;

    case EON_EN25B64:
    case EON_EN25Q64:
    case WIN_W25Q64BV:
    case MX_25L6445E:
        g_norAddrMap.bytesPerPage		= 256;
        g_norAddrMap.pagesPerSector		= 256;
        g_norAddrMap.bytesPerSector		= 64*1024;
        g_norAddrMap.sectorsPerBlock	= 1;
        g_norAddrMap.totalBlocks		= 128;
        break;

    case MX_25L12835F:
    	g_norAddrMap.bytesPerPage		= 256;
        g_norAddrMap.pagesPerSector		= 256;
        g_norAddrMap.bytesPerSector		= 64*1024;
        g_norAddrMap.sectorsPerBlock	= 1;
        g_norAddrMap.totalBlocks		= 256;
    	break;

    case MX_25L25735F:
    case MX_25L25635F:
    	g_norAddrMap.bytesPerPage		= 256;
        g_norAddrMap.pagesPerSector		= 256;
        g_norAddrMap.bytesPerSector		= 64*1024;
        g_norAddrMap.sectorsPerBlock	= 1;
        g_norAddrMap.totalBlocks		= 512;
        g_norAddrMap.use4BytesAddress   = true;
        
    	break;

    case EON_EN25F80:
    case EON_EN25Q80A:
        g_norAddrMap.bytesPerPage		= 256;
        g_norAddrMap.pagesPerSector		= 256;
        g_norAddrMap.bytesPerSector		= 64*1024;
        g_norAddrMap.sectorsPerBlock	= 1;
        g_norAddrMap.totalBlocks		= 16;
        break;

    case GD_GD25Q64B:	// 8 MB
	    g_norAddrMap.bytesPerPage		= 256;
        g_norAddrMap.pagesPerSector		= 256;
        g_norAddrMap.bytesPerSector		= 64 * 1024;
        g_norAddrMap.sectorsPerBlock	= 1;
        g_norAddrMap.totalBlocks		= 128;
    	break;

    case GD_GD25Q32:	// 4 MB
	    g_norAddrMap.bytesPerPage		= 256;
        g_norAddrMap.pagesPerSector		= 256;
        g_norAddrMap.bytesPerSector		= 64 * 1024;
        g_norAddrMap.sectorsPerBlock	= 1;
        g_norAddrMap.totalBlocks		= 64;
    	break;

    case GD_GD25Q16:	// 2 MB
	    g_norAddrMap.bytesPerPage		= 256;
        g_norAddrMap.pagesPerSector		= 256;
        g_norAddrMap.bytesPerSector		= 64 * 1024;
        g_norAddrMap.sectorsPerBlock	= 1;
        g_norAddrMap.totalBlocks		= 32;
    	break;

    case GD_GD25Q128C:    // 16 MB
        g_norAddrMap.bytesPerPage       = 256;
        g_norAddrMap.pagesPerSector     = 16;
        g_norAddrMap.bytesPerSector     = 4*1024;
        g_norAddrMap.sectorsPerBlock    = 16;
        g_norAddrMap.totalBlocks        = 256;
        break;
    case SPAN_S25FL127S:    // 16 MB
        g_norAddrMap.bytesPerPage		= 256;
        g_norAddrMap.pagesPerSector		= 256;
        g_norAddrMap.bytesPerSector		= 64*1024;
        g_norAddrMap.sectorsPerBlock	= 1;
        g_norAddrMap.totalBlocks		= 256;
        break;
    case EON_EN25QH128:    // 16 MB
        g_norAddrMap.bytesPerPage		= 256;
        g_norAddrMap.pagesPerSector		= 16;
        g_norAddrMap.bytesPerSector		= 4*1024;
        g_norAddrMap.sectorsPerBlock    = 16;
        g_norAddrMap.totalBlocks		= 256;
        break;
    case GD_GD25Q256C:    // 32 MB
        g_norAddrMap.bytesPerPage		= 256;
        g_norAddrMap.pagesPerSector		= 16;
        g_norAddrMap.bytesPerSector		= 4*1024;
        g_norAddrMap.sectorsPerBlock    = 16;
        g_norAddrMap.totalBlocks		= 512;
        break;
    default:
        break;
    }

    if (g_norAddrMap.use4BytesAddress)
    {
        printf("4 byte mode is on+\n");
        NorSendCommand(EN4B_MODE, 0, 0, 0);
        printf("4 byte mode is on-\n");
    }
end:
    return result;
}

//=============================================================================
//                              Public Function Definition
//=============================================================================
NOR_RESULT
FTSPI_Read(
    void* pdes,
    uint32_t addr,
    int32_t size)
{

    uint8_t*   pbuf    = (uint8_t*)(pdes);
    NOR_RESULT result  = 0;
    uint32_t   perSize = 0;
    int32_t    remainderSize = size;

#ifdef ARM_NOR_WRITER
	SetArmNorWriterAppStatus(ANW_APP_STATUS_READING_PAGES);
	SetArmNorWriterAppProgressMin(0);
	SetArmNorWriterAppProgressMax(size);
#endif
	while(remainderSize > 0)
	{
#if 0
	    perSize = (size > PER_READ_BYTES) ? (PER_READ_BYTES) : size;
#endif
		perSize = (remainderSize > PER_DMA_BYTES) ? (PER_DMA_BYTES) : remainderSize;
		NorSendCommand(READ, addr, (uint32_t)pbuf, perSize);
		remainderSize -= perSize;
		addr += perSize;
		pbuf += perSize;
#ifdef ARM_NOR_WRITER
		SetArmNorWriterAppProgressCurr(size - remainderSize);
#endif
	}
    
    return result;
}

NOR_RESULT
FTSPI_Write(
    void* psrc,
    uint32_t addr,
    int32_t size)
{
    int32_t    i           = 0;
    uint8_t    temp        = 0;
    uint8_t*   pbuf        = (uint8_t*)(psrc);
    int32_t    blockSize = g_norAddrMap.bytesPerSector * g_norAddrMap.sectorsPerBlock;
    int32_t    numOfSector = (size / blockSize);
    int32_t    wSize       = 0;
    int32_t    addrOffset  = addr % (blockSize);
    uint32_t   perSize     = 0;
    NOR_RESULT result      = 0;
    uint32_t   endAddr     = addr + size;
    int32_t    endOffset   = (endAddr % blockSize);
    int        totalWriteSize = size;
    uint32_t   writeStartAddr = addr - addrOffset;
	uint32_t   addrTemp    = writeStartAddr;
    uint8_t*   pWritebuf   = pbuf;
    int        headSector = 0;
    int        tailSector = 0;

    if (addrOffset || endOffset)
    {
        if (addrOffset)
        {
            headSector = 1;
            if ((writeStartAddr / blockSize) != (endAddr /blockSize) && endOffset)
            {
                tailSector = 1;
				if ( ((blockSize - addrOffset)  + endOffset) >= blockSize && numOfSector)
				{
					numOfSector -= 1;
				}
            }
        }
        else
        {
            tailSector = 1;
        }
		
		
        totalWriteSize = (numOfSector + headSector + tailSector) * blockSize;

        pWritebuf = (uint8_t*) malloc(totalWriteSize);
        if (headSector)
        {
            FTSPI_Read(pWritebuf, writeStartAddr, blockSize);
        }
        if (tailSector)
        {
            FTSPI_Read(&pWritebuf[totalWriteSize - blockSize], (endAddr - endOffset), blockSize);
        }
        memcpy(&pWritebuf[addrOffset], pbuf, size);
    }
    // printf("totalWriteSize: %u, head: %u, tail: %u, addr: 0x%X, size: %u\, byteperSec: %u\n",
           // totalWriteSize, headSector, tailSector, addr, size, g_norAddrMap.bytesPerSector);
    numOfSector += (headSector + tailSector);

	// Unlock write protected
    NorSendCommand(WRITE_EN, 0, 0, 0);
    NorSendCommand(WRITE_STATUS, 0, 0, 0);
    result = NorCheckReady();
    if(result)
    {
		goto done;
    }

    NorSendCommand(READ_STATUS, 0, (uint32_t)&temp, 1);
	if((temp & NOR_WRITE_PROTECT)  != 0)
	{
		result = NOR_ERROR_STATUS_PROTECT;
		printf("[ERROR] NOR_STATUS_PROTECT \n");
		goto done;
	}

    // Erase sectors
#ifdef ARM_NOR_WRITER
	printf("Erasing nor......\n");
	SetArmNorWriterAppStatus(ANW_APP_STATUS_WRITING_ERASE_NOR);
	SetArmNorWriterAppProgressMin(0);
	SetArmNorWriterAppProgressMax(numOfSector - 1);
#endif
    for(i = 0; i < numOfSector; ++i)
	{
#ifdef ARM_NOR_WRITER
		SetArmNorWriterAppProgressCurr(i);
#endif
	    // EON EN25B16 sector 0 devide 5 partial sectors
	    if( (g_vendor == EON_EN25B16 || g_vendor == EON_EN25B32 || g_vendor == EON_EN25B64) &&
            (writeStartAddr == 0) )
	    {
	        int32_t j              = 0;
	        int32_t bytesPerSector = 0;

	        for(j = 0; j < 5; ++j)
	        {
	            switch(j)
	            {
                case 0:
                    bytesPerSector = 0x1000;
                    break;
                case 1:
                    bytesPerSector = 0x1000;
                    break;
                case 2:
                    bytesPerSector = 0x2000;
                    break;
                case 3:
                    bytesPerSector = 0x4000;
                    break;
                case 4:
                    bytesPerSector = 0x8000;
                    break;
	            }

	            // Write enable
    			NorSendCommand(WRITE_EN, 0, 0, 0);
                // Erase
    			NorSendCommand(ERASE_BLOCK, addrTemp, 0, 0);

                // Wait erase finished
                result = NorCheckReadyEx();
                if(result)
                {
                    goto done;
                }
                addrTemp += bytesPerSector;
	        }
	    }
	    else
	    {
			// Write enable
			NorSendCommand(WRITE_EN, 0, 0 , 0);
            // Erase
			NorSendCommand(ERASE_BLOCK, addrTemp, 0, 0);

            // Wait erase finished
            result = NorCheckReadyEx();
            if(result)
            {
                goto done;
            }
            addrTemp += blockSize;
        }
	}
#ifdef ARM_NOR_WRITER
	SetArmNorWriterAppProgressCurr(GetArmNorWriterAppProgressMax());
#endif

    // Write state
    {
        uint32_t bufAddr = (uint32_t)pWritebuf;
        int      remainderSize = totalWriteSize;
        addrTemp   = writeStartAddr;
        while(remainderSize > 0)
        {
            wSize = (remainderSize < g_norAddrMap.bytesPerPage) ? (remainderSize) : (g_norAddrMap.bytesPerPage);
#ifdef ARM_NOR_WRITER
			if ( remainderSize < g_norAddrMap.bytesPerPage )
			{
				wSize = remainderSize;
			}
			else
			{
				wSize = remainderSize / g_norAddrMap.bytesPerPage * g_norAddrMap.bytesPerPage;
			}
#endif

			//test only
            //if(wSize == g_norAddrMap.bytesPerPage)
#ifdef ARM_NOR_WRITER
			if ( wSize > 256 )
			{
                NorSendCommand(WRITE_MULTI, addrTemp, bufAddr, wSize);
                result = NorCheckReady();
                if(result)
                {
                    goto done;
                }
                bufAddr  += wSize;
                addrTemp += wSize;
                remainderSize -= wSize;
			}
			else
#endif
			if( wSize >= 32 )
            {
                //write enable
	            NorSendCommand(WRITE_EN, 0, 0, 0);
                NorSendCommand(WRITE, addrTemp, bufAddr, wSize);
                result = NorCheckReady();
                if(result)
                {
                    goto done;
                }
                bufAddr  += wSize;
                addrTemp += wSize;
                remainderSize -= wSize;
            }
            else
            {
                for(i = 0; i< wSize; i +=WRITE_DATA_BYTES)
                {
                    if((wSize - i) >= WRITE_DATA_BYTES)
                    {
                        perSize = WRITE_DATA_BYTES;
                    }
                    else
                    {
                        perSize = wSize - i;
                    }

                    // Write enable
	                NorSendCommand(WRITE_EN, 0, 0, 0);
                    // 32 bytes each wrtie cmd
                    NorSendCommand(WRITE, addrTemp, bufAddr, perSize);

                    bufAddr += perSize;
                    addrTemp += perSize;

                    result = NorCheckReady();
                    if(result)
                    {
                        goto done;
                    }
                }
                remainderSize -= wSize;
            }
        }
    }

    if (pWritebuf != pbuf)
    {
        free(pWritebuf);
    }

done:
	// Write disable
	NorSendCommand(WRITE_DIS, 0, 0, 0);

	if(result == 0)
    {
		result = NorCheckReady();
    }

    return result;
}

NOR_RESULT
FTSPI_WriteWithoutErase(
    void*    psrc,
    uint32_t addr,
    int32_t  size)
{
    int32_t    i      = 0;
    uint8_t    temp   = 0;
    uint8_t*   pbuf        = (uint8_t*)(psrc);
    uint16_t   data   = 0;
	int32_t numOfSector  = (size / g_norAddrMap.bytesPerSector);
    int32_t   wOffset    = size % (g_norAddrMap.bytesPerSector);
    int32_t   wSize      = (size < g_norAddrMap.bytesPerSector) ? (size) : (g_norAddrMap.bytesPerSector) ;
	int32_t   addrOffset = addr % (g_norAddrMap.bytesPerSector);
	uint32_t  addrTemp   = addr;
    uint32_t   perSize     = 0;
    NOR_RESULT result      = 0;

    if ( wOffset )
    {
        numOfSector++;
    }

	// Unlock write protected
    NorSendCommand(WRITE_EN, 0, 0, 0);
    NorSendCommand(WRITE_STATUS, 0, 0, 0);
    result = NorCheckReady();
    if(result)
    {
        goto done;
    }

    NorSendCommand(READ_STATUS, 0, (uint32_t)&temp, 1);
	if((temp & NOR_WRITE_PROTECT)  != 0)
	{
		result = NOR_ERROR_STATUS_PROTECT;
		printf("[ERROR] NOR_STATUS_PROTECT \n");
		goto done;
	}

    // Write state
    {
        uint32_t bufAddr = (uint32_t)pbuf;
        int      remainderSize = size;

        addrOffset = addr % (g_norAddrMap.bytesPerPage);
        addrTemp   = addr;
        while(remainderSize > 0)
        {
            if(addrOffset)
            {
                wSize =  g_norAddrMap.bytesPerPage - addrOffset;
                if(wSize > remainderSize)
                {
                    wSize = remainderSize;
                }
                addrOffset = 0;
            }
            else
            {
                wSize = (size < g_norAddrMap.bytesPerPage) ? (size) : (g_norAddrMap.bytesPerPage);
            }

			if( wSize >= 32 )
            {
                //write enable
	            NorSendCommand(WRITE_EN, 0, 0, 0);
                NorSendCommand(WRITE, addrTemp, bufAddr, wSize);
                result = NorCheckReady();
                if(result)
                {
                    goto done;
                }
                bufAddr  += wSize;
                addrTemp += wSize;
                remainderSize -= wSize;
            }
            else
            {
            for(i = 0; i< wSize; i +=WRITE_DATA_BYTES)
            {
                    if((wSize - i) >= WRITE_DATA_BYTES)
                    {
                        perSize = WRITE_DATA_BYTES;
                    }
                    else
                    {
                        perSize = wSize - i;
                    }

                // Write enable
	            NorSendCommand(WRITE_EN, 0, 0, 0);
                    // 32 bytes each wrtie cmd
                    NorSendCommand(WRITE, addrTemp, bufAddr, perSize);

                    bufAddr += perSize;
                    addrTemp += perSize;

                result = NorCheckReady();
                if(result)
                {
                    goto done;
                }
            }
                remainderSize -= wSize;
            }
        }
    }

done:
	// Write disable
	NorSendCommand(WRITE_DIS, 0, 0, 0);

	if(result == 0)
    {
		result = NorCheckReady();
    }

    return result;
}

NOR_RESULT
NorBulkErase(
	void)
{
    NOR_RESULT result = 0;
	uint32_t   j      = 0;
    uint8_t    temp   = 0;

    pthread_mutex_lock(&NorInternalMutex);

	//Nnlock write protected
	NorSendCommand(WRITE_EN, 0, 0, 0);

	// Erase
	NorSendCommand(ERASE_ALL, 0, 0, 0);

	// Wwait erase finished
	j = 0x800000;
	do
    {
		NorSendCommand(READ_STATUS, 0, (uint32_t)&temp, 1);
    } while ((temp & NOR_DEVICE_BUSY)&&(--j));

	if (j == 0)
    {
		result = NOR_ERROR_DEVICE_TIMEOUT;
		goto end;
    }

	// Write disable
	NorSendCommand(WRITE_DIS, 0, 0, 0);

end:
	pthread_mutex_unlock(&NorInternalMutex);
	return result;
}

NOR_RESULT
NorInitial(
    SPI_PORT port,
    SPI_CSN  chipSelectPin)
{
    NOR_RESULT result = 0;

    union
    {
        uint32_t dwData;
        uint8_t byteData[4];
    }pat;

    pthread_mutex_lock(&NorInternalMutex);

    if(g_initCount == 0)
    {
        pat.dwData = 0x12345678;
        if( pat.byteData[0] != (pat.dwData&0xFF) )
        {
            g_endian = NOR_BIG_ENDIAN;
        }
        else
        {
            g_endian = NOR_LITTLE_ENDIAN;
        }

        result = mmpSpiInitialize(NOR_BUS, NOR_CSN_0, CPO_0_CPH_0, SPI_CLK_20M);
        if(result == 0)
        {
            result = InitNorFlash();
			if (!result)			
				NorSendCommand(WRITE_DIS, 0, 0, 0);
				
        }
    }

    g_initCount++;

    pthread_mutex_unlock(&NorInternalMutex);

    return result;
}

NOR_RESULT
NorTerminate(
    SPI_PORT port,
	SPI_CSN  chipSelectPin)
{
    NOR_RESULT result = 0;

    pthread_mutex_lock(&NorInternalMutex);

    if(--g_initCount == 0)
    {
        mmpSpiTerminate(NOR_BUS);
    }

    pthread_mutex_unlock(&NorInternalMutex);
	
    return ;
}

NOR_RESULT
NorRead(
	SPI_PORT	port,
	SPI_CSN		chipSelectPin,
	uint32_t 	addr,
    uint8_t*    pdes,    
    uint32_t  	size)
{
    NOR_RESULT result     = 0;
    uint8_t*   norTempBuf = NULL;
    uint32_t   des        = (uint32_t)pdes;
    int32_t    pageOffset = addr & 0xFF;
	pthread_mutex_lock(&NorInternalMutex);

    //ensure addr is page align
    if(des & 0x3 || pageOffset)
    {
        norTempBuf = (uint8_t *)itpVmemAlloc(size + 256);
        if(!norTempBuf)
        {
            result = -1;
            goto end;
        }

        if (pageOffset)
        {
            FTSPI_Read(norTempBuf, addr - pageOffset, size + pageOffset);
            memcpy(pdes, &norTempBuf[pageOffset], size);
        }
        else
        {
            result = FTSPI_Read(norTempBuf, addr, size);
            if(!result)
            {
    #ifdef WIN32
                uint8_t* mappedSysRam = NULL;

                mappedSysRam = ithMapVram((uint32_t)norTempBuf, size, ITH_VRAM_READ);
                memcpy(pdes, mappedSysRam, size);
                ithUnmapVram(mappedSysRam, size);
    #else
                memcpy(pdes, norTempBuf, size);
    #endif
            }
        }
        itpVmemFree((uint32_t)norTempBuf);
    }
    else
    {
        result = FTSPI_Read(pdes, addr, size);
    }

end:
	pthread_mutex_unlock(&NorInternalMutex);
    return result;
}

NOR_RESULT
NorWrite(
	SPI_PORT	port,
	SPI_CSN		chipSelectPin,
	uint32_t addr,
    uint8_t* psrc,
    uint32_t size)
{
    NOR_RESULT result;

    pthread_mutex_lock(&NorInternalMutex);
    result = FTSPI_Write(psrc, addr, size);
    pthread_mutex_unlock(&NorInternalMutex);

    return result;
}

NOR_RESULT
NorWriteWithoutErase(
	SPI_PORT	port,
	SPI_CSN		chipSelectPin,
	uint32_t addr,
    uint8_t* psrc,
    uint32_t size)
{
    NOR_RESULT result;

    pthread_mutex_lock(&NorInternalMutex);
    result = FTSPI_WriteWithoutErase(psrc, addr, size);
    pthread_mutex_unlock(&NorInternalMutex);

    return result;
}

uint32_t
NorCapacity(
    void)
{
	uint32_t result = 0;

	pthread_mutex_lock(&NorInternalMutex);
	result = (uint32_t)(g_norAddrMap.bytesPerSector * g_norAddrMap.sectorsPerBlock * g_norAddrMap.totalBlocks);
	pthread_mutex_unlock(&NorInternalMutex);

    return result;
}


uint32_t
NorGetAttitude(
	SPI_PORT port,
	SPI_CSN  chipSelectPin,
    NOR_ATTITUDE atti)
{
    uint32_t data = 0;

	pthread_mutex_lock(&NorInternalMutex);

    switch(atti)
    {
    case NOR_ATTITUDE_ERASE_UNIT:
        data = (uint32_t)g_norAddrMap.bytesPerSector;
        break;

    case NOR_ATTITUDE_DEVICE_COUNT:
        data = sizeof(nor_support_vendor)/sizeof(nor_support_vendor[0]);
        break;

    case NOR_ATTITUDE_CURRENT_DEVICE_ID:
        data = g_vendor;
        break;

    case NOR_ATTITUDE_PAGE_SIZE:
    	data = (uint32_t)g_norAddrMap.bytesPerPage;
    	break;

   	case NOR_ATTITUDE_PAGE_PER_SECTOR:
   		data = (uint32_t)g_norAddrMap.pagesPerSector;
   		break;

   	case NOR_ATTITUDE_SECTOR_PER_BLOCK:
   		data = (uint32_t)g_norAddrMap.sectorsPerBlock;
   		break;

   	case NOR_ATTITUDE_BLOCK_SIZE:
   		data = (uint32_t)g_norAddrMap.totalBlocks;
   		break;

   	case NOR_ATTITUDE_TOTAL_SIZE:
   		data = (uint32_t)(g_norAddrMap.totalBlocks * g_norAddrMap.sectorsPerBlock * g_norAddrMap.bytesPerSector);
   		break;

    default:
        break;
    }

    pthread_mutex_unlock(&NorInternalMutex);

    return data;
}

uint8_t*
NorGetDeviceName(
    uint8_t num)
{
    uint8_t *ptr = NULL;
    uint8_t i;

    pthread_mutex_lock(&NorInternalMutex);

    for(i = 0; i < sizeof(nor_support_vendor)/sizeof(nor_support_vendor[0]);++i)
    {
    	if(num == nor_support_vendor[i].vendorID)
    	{
    		ptr = nor_support_vendor[i].deviceName;
    		break;
    	}
    }

    pthread_mutex_unlock(&NorInternalMutex);

    return ptr;
}

uint32_t
NorGetDeviceCount()
{
	uint32_t result = 0;

	pthread_mutex_lock(&NorInternalMutex);
	result = (sizeof(nor_support_vendor)/sizeof(nor_support_vendor[0]));
	pthread_mutex_unlock(&NorInternalMutex);

	return result;
}

uint32_t
NorGetDeviceInfo(
	uint32_t  deviceIndex,
	uint32_t* pId1,
	uint32_t* pId2,
	uint32_t* pId3,
	char**    deviceName)
{
	uint32_t funcResult  = 0;
	uint32_t deviceCount = NorGetDeviceCount();

	pthread_mutex_lock(&NorInternalMutex);
	if ( deviceIndex >= deviceCount )
	{
		goto end;
	}
	if ( pId1 == NULL || pId2 == NULL || pId3 == NULL || deviceName == NULL )
	{
		goto end;
	}

	*pId1 = nor_support_vendor[deviceIndex].manufatureID;
	*pId2 = nor_support_vendor[deviceIndex].deviceID;
	*pId3 = nor_support_vendor[deviceIndex].deviceID2;
	*deviceName = nor_support_vendor[deviceIndex].deviceName;
	funcResult = 1;

end:
	pthread_mutex_unlock(&NorInternalMutex);
	return funcResult;
}

void
NorGetBuildInfo(
    uint8_t* version,
    uint8_t* date)
{
    uint8_t* ptr;
    uint8_t cnt = 0;

	pthread_mutex_lock(&NorInternalMutex);
    *version = BUILD_VERSION;
    ptr = __DATE__;
    cnt = strlen(ptr);
    memcpy(date, ptr, cnt);
    pthread_mutex_unlock(&NorInternalMutex);
}

const uint32_t
NorGetCapacity(
	SPI_PORT port,
	SPI_CSN  chipSelectPin)
{	
	uint32_t	result = 0;

	pthread_mutex_lock(&NorInternalMutex);
	result = (uint32_t)(g_norAddrMap.bytesPerSector * g_norAddrMap.sectorsPerBlock * g_norAddrMap.totalBlocks);
	pthread_mutex_unlock(&NorInternalMutex);

	return result;
}

NOR_RESULT
NorEraseAll(
	SPI_PORT	port,
	SPI_CSN		chipSelectPin)
{
    NOR_RESULT result = 0;
	uint32_t   j      = 0;
    uint8_t    temp   = 0;

    pthread_mutex_lock(&NorInternalMutex);

	//Nnlock write protected
	NorSendCommand(WRITE_EN, 0, 0, 0);

	// Erase
	NorSendCommand(ERASE_ALL, 0, 0, 0);

	// Wwait erase finished
	j = 0x800000;
	do
    {
		NorSendCommand(READ_STATUS, 0, (uint32_t)&temp, 1);
    } while ((temp & NOR_DEVICE_BUSY)&&(--j));

	if (j == 0)
    {
		result = NOR_ERROR_DEVICE_TIMEOUT;
		goto end;
    }

	// Write disable
	NorSendCommand(WRITE_DIS, 0, 0, 0);

end:
	pthread_mutex_unlock(&NorInternalMutex);
	return result;
}



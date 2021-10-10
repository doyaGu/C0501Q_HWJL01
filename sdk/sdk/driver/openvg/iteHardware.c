/*
 */

#include <stdio.h>
#include <string.h>
#include "openvg.h"
#include "iteDefs.h"
#include "iteContext.h"
#include "itePath.h"
#include "iteImage.h"
#include "iteGeometry.h"
#include "itePaint.h"
#include "iteVectors.h"
#include "iteHardware.h"
#include "../../include/ite/ith.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define ITE_VG_USE_CMDQ
#define ITE_VG_CMDQ_BURST_MODE
//#define ITE_VG_USE_INTR
//#define ITE_VG_NULL_CMDQ
//#define ITE_VG_DEBUG_MSG

#ifdef ITE_VG_DEBUG_MSG
#define vgPrint	printf
#else
#define vgPrint
#endif

//=============================================================================
//                              Structure Definition
//=============================================================================


//=============================================================================
//                              Global Data Definition
//=============================================================================
static ITEuint32 VgFrameID   = 1;
static ITEuint32 VgObjectID  = 1;
static ITEuint32 VgCurrObjID = 0;
static ITEuint32 VgInitCount = 0;

//=============================================================================
//                              Private Function Declaration
//=============================================================================


//=============================================================================
//                              Private Function Definition
//=============================================================================
static void
_DumpPathBuffer(
	ITEHardwareRegister* hwReg,
	char*                filepath)
{
#ifdef _WIN32
	ITEuint8* pathSRamBuffer = NULL;
	ITEuint32 pathVRamBuffer = hwReg->REG_PBR_BASE;
	ITEuint32 pathVRamLength = hwReg->REG_PLR_BASE;
	ITEuint32 i              = 0;
	FILE*     pFile          = fopen(filepath, "a+");
	char      buf[256]       = {0};

	if ( pFile )
	{
		if ( pathVRamLength )
		{
			pathSRamBuffer = ithMapVram(pathVRamBuffer, pathVRamLength, ITH_VRAM_READ);

			/* Write address */
			sprintf(buf, "@%08X\n", pathVRamBuffer);
			fwrite(buf, 1, strlen(buf), pFile);

			/* Write data */
			for ( i = 0; i < pathVRamLength; i++ )
			{
				sprintf(buf, "%02X ", pathSRamBuffer[i]);
				fwrite(buf, 1, strlen(buf), pFile);
			}
			sprintf(buf, "\n\n");
			fwrite(buf, 1, strlen(buf), pFile);

			ithUnmapVram(pathSRamBuffer, pathVRamLength);
		}

		fclose(pFile);
	}
#else
	ITEuint32 pathVRamBuffer = hwReg->REG_PBR_BASE;
	ITEuint32 pathVRamLength = hwReg->REG_PLR_BASE;
	ITEuint32 i              = 0;
	FILE*     pFile          = fopen(filepath, "a+");
	char      buf[256]       = {0};

	if ( pFile )
	{
		if ( pathVRamLength )
		{
			/* Write address */
			sprintf(buf, "@0x%08X\n", pathVRamBuffer);
			fwrite(buf, 1, strlen(buf), pFile);

			/* Write data */
			for ( i = 0; i < pathVRamLength; i++ )
			{
				sprintf(buf, "%02X ", ((ITEuint8*)pathVRamBuffer)[i]);
				fwrite(buf, 1, strlen(buf), pFile);
			}
			sprintf(buf, "\n\n");
			fwrite(buf, 1, strlen(buf), pFile);
		}

		fclose(pFile);
	}
#endif
}

static void
_DumpSourceBuffer(
	ITEHardwareRegister* hwReg,
	char*                filepath)
{
#ifdef _WIN32
	ITEuint8* srcSRamBuffer = NULL;
	ITEuint32 srcVRamBuffer = hwReg->REG_SBR_BASE;
	ITEuint32 srcPicth      = 0;
	ITEuint32 srcHeight     = 0;
	ITEuint32 i             = 0;
	FILE*     pFile         = fopen(filepath, "a+");
	char      buf[256]      = {0};

	if ( pFile )
	{
		srcPicth = (hwReg->REG_SDPR_BASE & 0xFFFF0000) >> 16;
		srcHeight = hwReg->REG_SHWR_BASE & 0x0000FFFF;

		if (srcPicth * srcHeight)
		{
			srcSRamBuffer = ithMapVram(srcVRamBuffer, srcPicth * srcHeight, ITH_VRAM_READ);

			/* Write address */
			sprintf(buf, "@%08X\n", srcVRamBuffer);
			fwrite(buf, 1, strlen(buf), pFile);

			/* Write data */
			for ( i = 0; i < (srcPicth * srcHeight); i++ )
			{
				sprintf(buf, "%02X ", srcSRamBuffer[i]);
				fwrite(buf, 1, strlen(buf), pFile);
			}
			sprintf(buf, "\n\n");
			fwrite(buf, 1, strlen(buf), pFile);

			ithUnmapVram(srcSRamBuffer, srcPicth * srcHeight);
		}

		fclose(pFile);
	}
#else
	ITEuint32 srcVRamBuffer = hwReg->REG_SBR_BASE;
	ITEuint32 srcPicth      = 0;
	ITEuint32 srcHeight     = 0;
	ITEuint32 i             = 0;
	FILE*     pFile         = fopen(filepath, "a+");
	char      buf[256]      = {0};

	if ( pFile )
	{
		srcPicth = (hwReg->REG_SDPR_BASE & 0xFFFF0000) >> 16;
		srcHeight = hwReg->REG_SHWR_BASE & 0x0000FFFF;

		if (srcPicth * srcHeight)
		{
			/* Write address */
			sprintf(buf, "@0x%08X\n", srcVRamBuffer);
			fwrite(buf, 1, strlen(buf), pFile);

			/* Write data */
			for ( i = 0; i < (srcPicth * srcHeight); i++ )
			{
				sprintf(buf, "%02X ", ((ITEuint8*)srcVRamBuffer)[i]);
				fwrite(buf, 1, strlen(buf), pFile);
			}
			sprintf(buf, "\n\n");
			fwrite(buf, 1, strlen(buf), pFile);
		}

		fclose(pFile);
	}
#endif
}

static void
_DumpBuffer(
	ITEHardwareRegister* hwReg,
	char*                filepath)
{
	/* Dump path command */
	_DumpPathBuffer(hwReg, filepath);
	/* Dump source image */
	_DumpSourceBuffer(hwReg, filepath);
}

static void
_DumpReg(
	ITEHardwareRegister* hwReg,
	ITEuint32            count,
	char*                filepath)
{
	char       buf[256] = {0};
	FILE*      pFile    = fopen(filepath, "a+");
	ITEuint32* startReg = &hwReg->REG_TCR_BASE;
	ITEuint32* endReg   = &hwReg->REG_PPCR1_BASE;
	ITEuint32* currReg  = NULL;

	if ( pFile )
	{
		sprintf(buf, "// ========== VG Register %d =========\n", count);
		fwrite(buf, 1, strlen(buf), pFile);

		for ( currReg = startReg; currReg != endReg; currReg++ )
		{
			ITEuint32 offset = (ITEuint32)currReg - (ITEuint32)startReg;

			sprintf(buf, "0x%08X 0x%08X 0x2 1\n", offset, *currReg);
			fwrite(buf, 1, strlen(buf), pFile);
		}
		sprintf(buf, "0x%08X 0x%08X 0x2 1\n", ITE_VG_REG_CMDR_BASE, 0);						 fwrite(buf, 1, strlen(buf), pFile);
		sprintf(buf, "0x%08X 0x%08X 0x2 1\n", ITE_VG_REG_BID1_BASE, hwReg->REG_BID1_BASE);	 fwrite(buf, 1, strlen(buf), pFile);
		sprintf(buf, "0x%08X 0x%08X 0x2 1\n\n", ITE_VG_REG_BID2_BASE, hwReg->REG_BID2_BASE); fwrite(buf, 1, strlen(buf), pFile);

		fclose(pFile);
	}
}

static void
_DumpPattern(
	ITEHardwareRegister* hwReg,
	ITEuint32            count)
{
	_DumpReg(hwReg, count, "D:/ite01054/regDump.txt");
	_DumpBuffer(hwReg, "D:/ite01054/bufDump.txt");
}

static void
iteAhbWriteRegister(
	ITEHardwareRegister* hwReg)
{
	ITEuint32* startReg = &hwReg->REG_TCR_BASE;
	ITEuint32* endReg   = &hwReg->REG_ICR_BASE;
	ITEuint32* currReg  = NULL;

	/* Fill register */
	for ( currReg = startReg; currReg != endReg; currReg++ )
	{
		ITEuint32 offset = (ITEuint32)currReg - (ITEuint32)startReg;

		ithWriteRegA(ITH_OPENVG_BASE + offset, *currReg);
	}

	/* Engine fire */
	ithWriteRegA(ITH_OPENVG_BASE + ITE_VG_REG_CMDR_BASE, 0);
}

static void
iteHostWriteRegister(
	ITEHardwareRegister* hwReg,
	ITEuint32            count)
{
	ITEuint32* startReg = &hwReg->REG_TCR_BASE;
	ITEuint32* endReg   = &hwReg->REG_PPCR1_BASE;
	ITEuint32* currReg  = NULL;

#ifdef ITE_VG_NULL_CMDQ
	return;
#endif

	/* Fill register */
    vgPrint("========== VG Register %d =========\n", count++);
	for ( currReg = startReg; currReg != endReg; currReg++ )
	{
		ITEuint32 offset = (ITEuint32)currReg - (ITEuint32)startReg;

		ithWriteRegA(ITH_OPENVG_BASE + offset, *currReg);

        if(*currReg != 0)
        {
        	vgPrint("0x%08X = 0x%08X\n", offset, *currReg);
        }
	}

	/* Engine fire */
	//HOST_WriteRegister(ITE_VG_HOST_REG_BASE + ITE_VG_REG_CMDR_BASE, 2);
	ithWriteRegA(ITH_OPENVG_BASE + ITE_VG_REG_CMDR_BASE, 2);

    /* Wait engine idle */
    {
        ITEuint32 regValue;

        do
        {
			regValue = ithReadRegA(ITH_OPENVG_BASE + ITE_VG_REG_ESR1_BASE);
			if (regValue & 1)
            {
                usleep(5);
            }
			else
            {
				break;
            }
        }while(1);
    }

	/* Write object ID */
	ithWriteRegA(ITH_OPENVG_BASE + ITE_VG_REG_BID2_BASE, hwReg->REG_BID2_BASE);
	vgPrint("0x%08X = 0x%08X\n", ITE_VG_REG_BID2_BASE, hwReg->REG_BID2_BASE);

    vgPrint("=======================================\n");
    vgPrint("    Finish CMD Fire\n");
    vgPrint("=======================================\n\n");
}


#ifdef ITE_VG_USE_CMDQ
static void
iteCmdqWriteRegister(
	ITEHardwareRegister* hwReg,
	ITEuint32            count)
{
	ITEuint32* startReg = &hwReg->REG_TCR_BASE;
	ITEuint32* endReg   = &hwReg->REG_CMDR_BASE;
	ITEuint32* currReg  = NULL;
	uint32_t*  cmdqAddr = NULL;

#ifdef ITE_VG_NULL_CMDQ
	return;
#endif

	ithCmdQLock();
#ifdef ITE_VG_CMDQ_BURST_MODE
		cmdqAddr = ithCmdQWaitSize(((uint32_t)endReg - (uint32_t)startReg) + 3 * 8);		// + fire command and object ID

		ITH_CMDQ_BURST_CMD(cmdqAddr, ITH_OPENVG_BASE, (uint32_t)endReg - (uint32_t)startReg);
		{
			int i = 0;
			int size = ((uint32_t)endReg - (uint32_t)startReg)/4;
			uint32_t alignLength = 0;

			for ( i = 0, currReg = startReg; i < size; i++, currReg++ )
			{
				*cmdqAddr = *currReg;
				cmdqAddr++;
			}

			alignLength = ((uint32_t)cmdqAddr + 0x7) & (~0x7); // length must align to 64bit
			if ( alignLength != (uint32_t)cmdqAddr )
        	{
        	    int i = 0;
        		uint32_t sss = (alignLength - (uint32_t)cmdqAddr) / 4;
        		for ( i = 0; i < sss; i++)
        		{
        		    *cmdqAddr = 0;
        		    cmdqAddr++;
        		}
        	}
		}

		/* Engine fire */
		ITH_CMDQ_SINGLE_CMD(cmdqAddr, ITH_OPENVG_BASE + ITE_VG_REG_CMDR_BASE, 0x0);

		/* Write object ID */
		ITH_CMDQ_SINGLE_CMD(cmdqAddr, ITH_OPENVG_BASE + ITE_VG_REG_BID2_BASE, hwReg->REG_BID2_BASE);
#else
		cmdqAddr = ithCmdQWaitSize(((uint32_t)endReg - (uint32_t)startReg) * 2 + 2 * 8);	// + fire command and object ID

		/* Fill register */
		for ( currReg = startReg; currReg != endReg; currReg++ )
		{
			ITH_CMDQ_SINGLE_CMD(cmdqAddr, ITH_OPENVG_BASE + ((uint32_t)currReg - (uint32_t)startReg),  *currReg);
		}

		/* Engine fire */
		ITH_CMDQ_SINGLE_CMD(cmdqAddr, ITH_OPENVG_BASE + ITE_VG_REG_CMDR_BASE, 0);

		/* Write object ID */
		ITH_CMDQ_SINGLE_CMD(cmdqAddr, ITH_OPENVG_BASE + ITE_VG_REG_BID2_BASE, hwReg->REG_BID2_BASE);
#endif
		ithCmdQFlush(cmdqAddr);
	ithCmdQUnlock();

    vgPrint("=======================================\n");
    vgPrint("    Finish CMD Fire(%u)\n", count);
    vgPrint("=======================================\n\n");
}
#endif

//static ITEHardwareRegister* g_hardware = NULL;
static ITEHardwareRegister PhysicalHwReg = {0};

/*
void iteHardwareRender()
{
	ITEHardware* h;
	VG_GETCONTEXT(VG_NO_RETVAL);
	h = context->hardware;

	if( h->enCoverage )
	{
		iteCoverageEngine();
		iteRenderEngine();
	}
	else
		iteRenderEngine();

}
*/

//=============================================================================
//                              Function Definition
//=============================================================================
void iteHardwareFire(
	ITEHardwareRegister* hwReg)
{
	if ( hwReg )
	{
		static ITEuint32 count = 1;

		/* Dump*/
		//_DumpPattern(hwReg, count);

#if 0	// Test code
		/* Screen Flip setting */
		{
			/* Get Destination Height */
			uint32_t destHeight = (hwReg->REG_DHWR_BASE & ITE_VG_CMDMASK_DSTHEIGHT) >> ITE_VG_CMDSHIFT_DSTHEIGHT;


			hwReg->REG_CCR_BASE |= ITE_VG_CMD_DSTFLIP_EN;
			hwReg->REG_SFR_BASE |= (destHeight << ITE_VG_CMDSHIFT_DSTFLIP_H) & ITE_VG_CMDMASK_DSTFLIP_H;
		}
#endif

		/* Flush path buffer */
		// Path Buffer
    #ifndef _WIN32
		ithFlushDCacheRange((void*)hwReg->REG_PBR_BASE, hwReg->REG_PLR_BASE);
    #endif

		// Source Base
		{
			ITEuint8* srcBuffer = hwReg->REG_SBR_BASE;
			ITEuint32 srcPitch = (hwReg->REG_SDPR_BASE & ITE_VG_CMDMASK_SRCPITCH0) >> ITE_VG_CMDSHIFT_SRCPITCH0;
			ITEuint32 srcHeight = (hwReg->REG_SHWR_BASE & ITE_VG_CMDMASK_SRCHEIGHT) >> ITE_VG_CMDSHIFT_SRCHEIGHT;

        #ifndef _WIN32
			ithFlushDCacheRange((void*)srcBuffer, srcPitch * srcHeight);
        #endif
		}

		/* Doesn't need it.
		// Coverage Plane Base Address
		{
			ITEuint8* coverageBuffer = hwReg->REG_CPBR_BASE;
			ITEuint32 coveragePitch = hwReg->REG_VPBR_BASE & ITE_VG_CMDMASK_PLANPITCH;
			ITEuint32 coverageHeight = (hwReg->REG_PYCR_BASE & ITE_VG_CMDMASK_PYCR_CLIPXEND) >> ITE_VG_CMDSHIFT_PYCR_CLIPXEND;

			ithFlushDCacheRange((void*)coverageBuffer, coveragePitch * coverageHeight);
		}
		*/

#ifdef ITE_VG_USE_CMDQ
		iteCmdqWriteRegister(hwReg, count);
#else
		/* Use HOST to write all register to engine */
		iteHostWriteRegister(hwReg, count);
#endif
		count++;
	}
}

void iteHardwareNullFire(
	ITEuint32 frameID)
{
	static ITEuint32 count = 0;
	uint32_t* cmdqAddr = NULL;

#ifdef ITE_VG_NULL_CMDQ
	return;
#endif

#ifdef ITE_VG_USE_CMDQ
	ithCmdQLock();
		cmdqAddr = ithCmdQWaitSize(2 * 8);	// 1 fire + 1 framID

		/* Engine fire */
		ITH_CMDQ_SINGLE_CMD(cmdqAddr, ITH_OPENVG_BASE + ITE_VG_REG_CMDR_BASE, 0x03);

		/* Write frame ID */
		ITH_CMDQ_SINGLE_CMD(cmdqAddr, ITH_OPENVG_BASE + ITE_VG_REG_BID1_BASE, frameID);

		ithCmdQFlush(cmdqAddr);
	ithCmdQUnlock();

	vgPrint("=======================================\n");
    vgPrint("    Null Command(%d)\n", frameID);
    vgPrint("=======================================\n\n");
#else
	/* Engine fire */
	ithWriteRegA(ITH_OPENVG_BASE + ITE_VG_REG_CMDR_BASE, 0x03);

	/* Wait engine idle */
    {
        ITEuint32 regValue;

        do
        {
            regValue = ithReadRegA(ITH_OPENVG_BASE + ITE_VG_REG_ESR1_BASE);
			if (regValue & 1)
            {
                usleep(1);
            }
			else
            {
				break;
            }
        }while(1);
    }

	/* Write frame ID */
	ithWriteRegA(ITH_OPENVG_BASE + ITE_VG_REG_BID1_BASE, frameID);
	vgPrint("0x%08X = 0x%08X\n", ITE_VG_REG_BID1_BASE, frameID);

    vgPrint("=======================================\n");
    vgPrint("    Null Command(%d)\n", frameID);
    vgPrint("=======================================\n\n");
#endif
}

void
ITEHardware_ctor(
	ITEHardwareRegister* hw)
{
#if 1
    ITEint i;
	VG_Memset(hw, 0x00, sizeof(ITEHardwareRegister));

	/* This value from HW guy */
	hw->REG_ACR_BASE = 0x01e40003;
	hw->REG_RFR_BASE = ITE_VG_CMD_MASKEXTEND_EN |
		               ITE_VG_CMD_DSTEXTEND_EN |
		               ITE_VG_CMD_SRCEXTEND_EN;

    if ( VgInitCount == 0 )
    {
    	/* Enable TCLK divider */
    	ithWriteRegMaskH(ITH_OPVG_CLK2_REG, (1<<15), (1<<15));
    
    	/* Enable VCLK divider */
    	ithWriteRegMaskH(ITH_OPVG_CLK3_REG, (1<<15), (1<<15));

    	/* Reset OpenVG and enable N9CLK, M16CLK, VCLK, and TCLK */
    	ithWriteRegMaskH(ITH_OPVG_CLK1_REG, (1<<15)|(1<<7)|(1<<5)|(1<<3)|(1<<1), (1<<15)|(1<<7)|(1<<5)|(1<<3)|(1<<1));
#ifndef _WIN32
        for(i=0; i<100; i++) asm("");
#endif
    	ithWriteRegMaskH(ITH_OPVG_CLK1_REG, (0<<15)|(1<<7)|(1<<5)|(1<<3)|(1<<1), (1<<15)|(1<<7)|(1<<5)|(1<<3)|(1<<1));  	
    }
    VgInitCount++;

#else
	ITEint i,j;

	/* Tessellation */
	h->paintMode = HW_FILL_PATH;
	h->lineWidth = 0;
	h->min.x = 32767;
	h->min.y = 32767;
	h->max.x = 0;
	h->max.y = 0;
	SETMAT(h->pathTransform, 0x10000,0,0, 0,0x10000,0, 0,0,0x10000);

	/* Mode settings */
	h->fillRule = HW_EVEN_ODD;
	h->imageQuality = HW_IMAGE_QUALITY_FASTER;
	h->renderingQuality = HW_RENDERING_QUALITY_BETTER;
	h->blendMode = HW_BLEND_SRC_OVER;
	h->maskMode = HW_INTERSECT_RENDERMASK;
	h->imageMode = HW_DRAW_IMAGE_NORMAL;
	h->paintMode = HW_FILL_PATH;

   /* enabling */
	h->enCoverage = HW_TRUE;  // enable coverage
	h->enScissor = HW_FALSE;
	h->enMask = HW_FALSE;
	h->enTexture = HW_FALSE;
	h->enLookup = HW_FALSE;
	h->enBlend = HW_TRUE;
	h->enColorTransform = HW_FALSE;
	h->enLookup = HW_FALSE;
	h->enSrcMultiply = HW_TRUE;
	h->enSrcUnMultiply = HW_TRUE;
	h->enDstMultiply = HW_TRUE;
	h->enPerspective = HW_FALSE;

	/* color */
	CSET(h->tileFillColor, 0,0,0,0);  //Edge fill color for vgConvolve and pattern paint
	CSET(h->clearColor,  128, 128, 128, 0);  // vgClear,

	/* Matrices */
	SETMAT(h->fillTransform, 0x10000,0,0, 0,0x10000,0, 0,0,0x10000);
	SETMAT(h->strokeTransform, 0x10000,0,0, 0,0x10000,0, 0,0,0x10000);
	SETMAT(h->imageTransform, 0x10000,0,0, 0,0x10000,0, 0,0,0x10000);

  /* Image */
	h->coverageData = NULL;
	h->coverageWidth = 0;
	h->coverageHeight = 0;
	h->textureData = NULL;
	h->textureWidth = 0;
	h->textureHeight = 0;
	h->maskData = NULL;
	h->maskWidth = 0;
	h->maskHeight = 0;
	h->surfaceData = NULL;
	h->surfaceWidth = 0;
	h->surfaceHeight = 0;
	h->gradientLen = 10;
	h->gradientData = (ITEuint8*)malloc(1<<(h->gradientLen+2));

	// ColorTransform
    for(i=0;i<4;i++)
		for(j=0;j<4;j++)
			h->colorTransform[i][j] = (i==j) ? 0x100 : 0;  //s7.8
    for(i=0;i<4;i++)
		h->colorBias[i] = 0;  //s8

	// Lookup Table
    for(i=0;i<4;i++)
		for(j=0;j<256;j++)
			h->lookupTable[i][j] = j;
#endif
}

/*-----------------------------------------------------
 * VGContext constructor
 *-----------------------------------------------------*/
void
ITEHardware_dtor(
	ITEHardwareRegister* hw)
{
#if 1
	VgInitCount--;

	if ( VgInitCount == 0 )
	{
       	/* Disable TCLK divider */
       	ithWriteRegMaskH(ITH_OPVG_CLK2_REG, (0<<15), (1<<15));
       
       	/* Disable VCLK divider */
       	ithWriteRegMaskH(ITH_OPVG_CLK3_REG, (0<<15), (1<<15));
       
       	/* Disable N9CLK, M16CLK, VCLK, and TCLK */
       	ithWriteRegMaskH(ITH_OPVG_CLK1_REG, (0<<15)|(0<<7)|(0<<5)|(0<<3)|(0<<1), (1<<15)|(1<<7)|(1<<5)|(1<<3)|(1<<1));
	}
#else
	free(h->coverageData);
	h->coverageData = NULL;
	free(h->textureData);
	h->textureData = NULL;
	free(h->patternData);
	h->patternData = NULL;
#endif
}

#if 1
void
iteCreateHardware(
	ITEHardwareRegister* hwReg)
{
	ITEHardware_ctor(hwReg);
}
#else
ITEHardware*
iteCreateHardware()
{
  /* return if already created */
  if (g_hardware) return g_hardware;

  /* create new context */
  ITE_NEWOBJ(ITEHardware, g_hardware);
  if (!g_hardware) return NULL;

  return g_hardware;
}
#endif

#if 1
void
iteDestroyHardware(
	ITEHardwareRegister* hw)
{
	ITEHardware_dtor(hw);
}
#else
void iteDestroyHardware()
{
	/* return if already released */
	if (!g_hardware) return;

	/* delete context object */
	ITE_DELETEOBJ(ITEHardware, g_hardware);
	g_hardware = NULL;
}
#endif

ITEuint32
iteHardwareGenFrameID()
{
	if ( VgFrameID == INVALID_FRAME_ID )
	{
		return (VgFrameID = 0);
	}
	else
	{
		return VgFrameID++;
	}
}

ITEuint32
iteHardwareGenObjectID()
{
	if ( VgObjectID == INVALID_OBJECT_ID )
	{
		return (VgObjectID = 0);
	}
	else
	{
		return VgObjectID++;
	}
}

ITEuint32
iteHardwareGetCurrentFrameID()
{
	if ( VgFrameID == 0 )
	{
		return 0xFFFFFFFE;
	}
	else
	{
		return VgFrameID - 1;
	}
}

ITEuint32
iteHardwareGetCurrentObjectID()
{
	if ( VgObjectID == 0 )
	{
		return 0xFFFFFFFE;
	}
	else
	{
		return VgObjectID - 1;
	}
}

ITEuint32
iteHardwareGetNextFrameID()
{
	return VgFrameID;
}

ITEuint32
iteHardwareGetNextObjectID()
{
	return VgObjectID;
}

void
iteHardwareWaitObjID(
	ITEuint32 objID)
{
#ifdef ITE_VG_USE_INTR
#else
	uint32_t currObjID = 0;
	uint16_t objId0    = 0;
	uint16_t objId1    = 0;

#ifdef ITE_VG_NULL_CMDQ
	return;
#endif

	if (   (objID == INVALID_OBJECT_ID)
		|| (VgCurrObjID >= objID) )
	{
		return;
	}

	do
	{
		currObjID = ithReadRegA(ITH_OPENVG_BASE + ITE_VG_REG_BID2_BASE);
	}while( currObjID < objID );

	VgCurrObjID = currObjID;
#endif
}

void
iteHardwareWaitFrameID(
	ITEuint32 frameID)
{
#ifdef ITE_VG_USE_INTR
#else
	uint32_t currFrameID = 0;
	uint16_t frameId0    = 0;
	uint16_t frameId1    = 0;

#ifdef ITE_VG_NULL_CMDQ
	return;
#endif

	do
	{
		currFrameID = ithReadRegA(ITH_OPENVG_BASE + ITE_VG_REG_BID1_BASE);
	}while( currFrameID < frameID );
#endif
}


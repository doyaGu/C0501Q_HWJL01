/*
 * Copyright (c) 2004 SMedia technology Corp. All Rights Reserved.
 */
/** @file
 * Used as implementatoin template file.
 *
 * @author Erica Chang
 * @version 0.1
 */
#include <string.h>
#include <unistd.h>
#include "m2d/m2d_graphics.h"
#include "m2d/m2d_engine.h"
#include "m2d/m2d_util.h"

#include "stdio.h"
#include "m2d/iteM2dDefs.h"
#include "m2d/iteM2dVectors.h"
#include "m2d/iteM2dPath.h"
#include "m2d/m2dvgmem.h"

#include "../../include/ite/ith.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define ITEM2D_VG_USE_CMDQ
#define ITEM2D_VG_CMDQ_BURST_MODE
//#define ITE_VG_USE_INTR

#ifdef ITEM2D_VG_DEBUG_MSG
    #define m2dvgPrint printf
#else
    #define m2dvgPrint
#endif

//=============================================================================
//                              Macro Definition
//=============================================================================
#define IS_A_CATCHUP_WITH_B(a, b, boundary) ((((uint32_t)(a) - (uint32_t)(b)) < ((boundary) / 4)) \
                                          || (((int32_t )(b) - (int32_t )(a)) > ((boundary) / 4 * 3)))

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
static ITEM2Duint32 M2dVgFrameID   = 0;
static ITEM2Duint32 M2dVgObjectID  = 0;
static ITEM2Duint32 M2dVgCurrObjID = 0;
static ITEM2Duint32 M2dVgInitCount = 0;

//MMP_UINT16       bitbltId2Value = 0;  //Record the current value of bitbltId2

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Private Function Definition
//=============================================================================
static void
_DumpPathBuffer(
    ITEM2DHardwareRegister *hwReg,
    char                   *filepath)
{
#ifdef _WIN32
    ITEM2Duint8  *pathSRamBuffer = NULL;
    ITEM2Duint32 pathVRamBuffer  = hwReg->REG_PBR_BASE;
    ITEM2Duint32 pathVRamLength  = hwReg->REG_PLR_BASE;
    ITEM2Duint32 i               = 0;
    FILE         *pFile          = fopen(filepath, "a+");
    char         buf[256]        = {0};

    if (pFile)
    {
        if (pathVRamLength)
        {
            pathSRamBuffer = ithMapVram(pathVRamBuffer, pathVRamLength, ITH_VRAM_READ);

            /* Write address */
            sprintf(buf, "@%08X\n", pathVRamBuffer);
            fwrite(buf, 1, strlen(buf), pFile);

            /* Write data */
            for (i = 0; i < pathVRamLength; i++)
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
    ITEM2Duint32 pathVRamBuffer = hwReg->REG_PBR_BASE;
    ITEM2Duint32 pathVRamLength = hwReg->REG_PLR_BASE;
    ITEM2Duint32 i              = 0;
    //FILE*     pFile          = fopen(filepath, "a+");
    char         buf[256]       = {0};

    //if ( pFile )
    {
        if (pathVRamLength)
        {
            /* Write address */
            sprintf(buf, "@0x%08X\n", pathVRamBuffer);
            //fwrite(buf, 1, strlen(buf), pFile);
            printf("%s", buf);

            /* Write data */
            for (i = 0; i < pathVRamLength; i++)
            {
                sprintf(buf, "%02X ", ((ITEM2Duint8 *)pathVRamBuffer)[i]);
                //fwrite(buf, 1, strlen(buf), pFile);
                printf("%s", buf);
            }
            sprintf(buf, "\n\n");
            //fwrite(buf, 1, strlen(buf), pFile);
            printf("%s", buf);
        }

        //fclose(pFile);
    }
#endif
}

static void
_DumpSourceBuffer(
    ITEM2DHardwareRegister *hwReg,
    char                   *filepath)
{
#ifdef _WIN32
    ITEM2Duint8  *srcSRamBuffer = NULL;
    ITEM2Duint32 srcVRamBuffer  = hwReg->REG_SBR_BASE;
    ITEM2Duint32 srcPicth       = 0;
    ITEM2Duint32 srcHeight      = 0;
    ITEM2Duint32 i              = 0;
    FILE         *pFile         = fopen(filepath, "a+");
    char         buf[256]       = {0};

    if (pFile)
    {
        srcPicth  = (hwReg->REG_SDPR_BASE & 0xFFFF0000) >> 16;
        srcHeight = hwReg->REG_SHWR_BASE & 0x0000FFFF;

        if (srcPicth * srcHeight)
        {
            srcSRamBuffer = ithMapVram(srcVRamBuffer, srcPicth * srcHeight, ITH_VRAM_READ);

            /* Write address */
            sprintf(buf, "@%08X\n", srcVRamBuffer);
            fwrite(buf, 1, strlen(buf), pFile);

            /* Write data */
            for (i = 0; i < (srcPicth * srcHeight); i++)
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
    ITEM2Duint32 srcVRamBuffer = hwReg->REG_SBR_BASE;
    ITEM2Duint32 srcPicth      = 0;
    ITEM2Duint32 srcHeight     = 0;
    ITEM2Duint32 i             = 0;
    //FILE*     pFile         = fopen(filepath, "a+");
    char         buf[256]      = {0};

    //if ( pFile )
    {
        srcPicth  = (hwReg->REG_SDPR_BASE & 0xFFFF0000) >> 16;
        srcHeight = hwReg->REG_SHWR_BASE & 0x0000FFFF;

        if (srcPicth * srcHeight)
        {
            /* Write address */
            sprintf(buf, "@0x%08X\n", srcVRamBuffer);
            //fwrite(buf, 1, strlen(buf), pFile);
            printf("%s", buf);

            /* Write data */
            for (i = 0; i < (srcPicth * srcHeight); i++)
            {
                sprintf(buf, "%02X ", ((ITEM2Duint8 *)srcVRamBuffer)[i]);
                //fwrite(buf, 1, strlen(buf), pFile);
                printf("%s", buf);
            }
            sprintf(buf, "\n\n");
            //fwrite(buf, 1, strlen(buf), pFile);
            printf("%s", buf);
        }

        //fclose(pFile);
    }
#endif
}

static void
_DumpBuffer(
    ITEM2DHardwareRegister *hwReg,
    char                   *filepath)
{
    /* Dump path command */
    _DumpPathBuffer(hwReg, filepath);
    /* Dump source image */
    _DumpSourceBuffer(hwReg, filepath);
}

static void
_DumpReg(
    ITEM2DHardwareRegister *hwReg,
    ITEM2Duint32           count,
    char                   *filepath)
{
    char         buf[256]  = {0};
    //FILE*      pFile    = fopen(filepath, "a+");
    ITEM2Duint32 *startReg = &hwReg->REG_TCR_BASE;
    ITEM2Duint32 *endReg   = &hwReg->REG_PPCR1_BASE;
    ITEM2Duint32 *currReg  = NULL;

    //if ( pFile )
    {
        sprintf(buf, "// ========== VG Register %d =========\n", count);
        //fwrite(buf, 1, strlen(buf), pFile);
        printf("%s", buf);

        for (currReg = startReg; currReg != endReg; currReg++)
        {
            ITEM2Duint32 offset = (ITEM2Duint32)currReg - (ITEM2Duint32)startReg;

            sprintf(buf, "0x%08X 0x%08X 0x2 1\n", offset, *currReg);
            //fwrite(buf, 1, strlen(buf), pFile);
            printf("%s", buf);
        }
        sprintf(buf, "0x%08X 0x%08X 0x2 1\n", ITEM2D_VG_REG_CMDR_BASE, 0);
        //fwrite(buf, 1, strlen(buf), pFile);
        printf("%s", buf);
        sprintf(buf, "0x%08X 0x%08X 0x2 1\n", ITEM2D_VG_REG_BID1_BASE, hwReg->REG_BID1_BASE);
        //fwrite(buf, 1, strlen(buf), pFile);
        printf("%s", buf);
        sprintf(buf, "0x%08X 0x%08X 0x2 1\n\n", ITEM2D_VG_REG_BID2_BASE, hwReg->REG_BID2_BASE);
        //fwrite(buf, 1, strlen(buf), pFile);
        printf("%s", buf);

        //fclose(pFile);
    }
}

static void
_DumpPattern(
    ITEM2DHardwareRegister *hwReg,
    ITEM2Duint32           count)
{
    char filePath[256];

    sprintf(filePath, "D:/regDump_%08d_%08d.txt", M2dVgFrameID, count);
    _DumpReg(hwReg, count, filePath);
    sprintf(filePath, "D:/bufDump_%08d_%08d.txt", M2dVgFrameID, count);
    _DumpBuffer(hwReg, filePath);
}

static HWImageFormat 
_M2dImgFormat2HwImgFormat(
    MMP_M2D_IMAGE_FORMAT m2dImageFormat)
{
    switch (m2dImageFormat)
    {
    case MMP_M2D_IMAGE_FORMAT_ARGB8888:
        return HW_sARGB_8888;

    case MMP_M2D_IMAGE_FORMAT_ARGB1555:
        return HW_sARGB_1555;

    case MMP_M2D_IMAGE_FORMAT_ARGB4444:
        return HW_sARGB_4444;

    case MMP_M2D_IMAGE_FORMAT_RGB565:
    default:
        return HW_sRGB_565;
    }
}

static void
iteM2dAhbWriteRegister(
    ITEM2DHardwareRegister *hwReg)
{
    ITEM2Duint32 *startReg = &hwReg->REG_TCR_BASE;
    ITEM2Duint32 *endReg   = &hwReg->REG_ICR_BASE;
    ITEM2Duint32 *currReg  = NULL;

    /* Fill register */
    for (currReg = startReg; currReg != endReg; currReg++)
    {
        ITEM2Duint32 offset = (ITEM2Duint32)currReg - (ITEM2Duint32)startReg;
        ithWriteRegA(ITH_OPENVG_BASE + offset, *currReg);
    }

    /* Engine fire */
    ithWriteRegA(ITH_OPENVG_BASE + ITEM2D_VG_REG_CMDR_BASE, 0);
}

static void
iteM2dHostWriteRegister(
    ITEM2DHardwareRegister *hwReg,
    ITEM2Duint32           count)
{
    ITEM2Duint32 *startReg = &hwReg->REG_TCR_BASE;
    ITEM2Duint32 *endReg   = &hwReg->REG_CMDR_BASE;
    ITEM2Duint32 *currReg  = NULL;

#ifdef ITEM2D_VG_NULL_CMDQ
    return;
#endif

    /* Fill register */
    m2dvgPrint("========== VG Register %d =========\n", count++);
    for (currReg = startReg; currReg != endReg; currReg++)
    {
        ITEM2Duint32 offset = (ITEM2Duint32)currReg - (ITEM2Duint32)startReg;

        ithWriteRegA(ITH_OPENVG_BASE + offset, *currReg);

        if (*currReg != 0)
        {
            m2dvgPrint("0x%08X = 0x%08X\n", offset, *currReg);
        }
    }

    /* Engine fire */
    ithWriteRegA(ITH_OPENVG_BASE + ITEM2D_VG_REG_CMDR_BASE, 2);

    /* Wait engine idle */
    {
        ITEM2Duint32 regValue;

        do
        {
            regValue = ithReadRegA(ITH_OPENVG_BASE + ITEM2D_VG_REG_ESR1_BASE);
            if (regValue & 1)
            {
                usleep(5);
            }
            else
            {
                break;
            }
        } while (1);
    }

    /* Write object ID */
    ithWriteRegA(ITH_OPENVG_BASE + ITEM2D_VG_REG_BID2_BASE, hwReg->REG_BID2_BASE);
    m2dvgPrint("0x%08X = 0x%08X\n", ITEM2D_VG_REG_BID2_BASE, hwReg->REG_BID2_BASE);

    m2dvgPrint("=======================================\n");
    m2dvgPrint("    Finish CMD Fire\n");
    m2dvgPrint("=======================================\n\n");
}

#ifdef ITEM2D_VG_USE_CMDQ
static void
iteM2dCmdqWriteRegister(
    ITEM2DHardwareRegister *hwReg,
    ITEM2Duint32           count)
{
    ITEM2Duint32 *startReg   = &hwReg->REG_TCR_BASE;
    ITEM2Duint32 *endReg     = &hwReg->REG_CMDR_BASE;
    ITEM2Duint32 *currReg    = NULL;
    uint32_t     *cmdqAddr   = NULL;
    ITEM2Duint32 requestSize = 0;

    #ifdef ITEM2D_VG_NULL_CMDQ
    return;
    #endif

    ithCmdQLock();
    #ifdef ITEM2D_VG_CMDQ_BURST_MODE
    requestSize = ((uint32_t)endReg - (uint32_t)startReg) + 3 * 8;
    requestSize = (requestSize + 0x07) & (~0x07);
    cmdqAddr    = ithCmdQWaitSize(requestSize);         // + fire command and object ID
    //cmdqAddr = ithCmdQWaitSize(((uint32_t)endReg - (uint32_t)startReg) + 3 * 8);      // + fire command and object ID
    if (cmdqAddr == NULL)
    {
        printf("************************ cmdqAddr == NULL\n");
    }
    ITH_CMDQ_BURST_CMD(cmdqAddr, ITH_OPENVG_BASE, (uint32_t)endReg - (uint32_t)startReg);
    {
        int      i           = 0;
        int      size        = ((uint32_t)endReg - (uint32_t)startReg) / 4;
        uint32_t alignLength = 0;

        for (i = 0, currReg = startReg; i < size; i++, currReg++)
        {
            *cmdqAddr = *currReg;
            cmdqAddr++;
        }

        alignLength = ((uint32_t)cmdqAddr + 0x7) & (~0x7);     // length must align to 64bit
        if (alignLength != (uint32_t)cmdqAddr)
        {
            int      i   = 0;
            uint32_t sss = (alignLength - (uint32_t)cmdqAddr) / 4;

            printf("************************ cmdq no align\n");
            for (i = 0; i < sss; i++)
            {
                *cmdqAddr = 0;
                cmdqAddr++;
            }
        }
    }

    /* Engine fire */
    ITH_CMDQ_SINGLE_CMD(cmdqAddr, ITH_OPENVG_BASE + ITEM2D_VG_REG_CMDR_BASE, 0x0);

    /* Write object ID */
    ITH_CMDQ_SINGLE_CMD(cmdqAddr, ITH_OPENVG_BASE + ITEM2D_VG_REG_BID2_BASE, hwReg->REG_BID2_BASE);
    #else
    cmdqAddr = ithCmdQWaitSize(((uint32_t)endReg - (uint32_t)startReg) * 2 + 2 * 8);        // + fire command and object ID

    /* Fill register */
    for (currReg = startReg; currReg != endReg; currReg++)
    {
        ITH_CMDQ_SINGLE_CMD(cmdqAddr, ITH_OPENVG_BASE + ((uint32_t)currReg - (uint32_t)startReg),  *currReg);
    }

    /* Engine fire */
    ITH_CMDQ_SINGLE_CMD(cmdqAddr, ITH_OPENVG_BASE + ITEM2D_VG_REG_CMDR_BASE, 0);

    /* Write object ID */
    ITH_CMDQ_SINGLE_CMD(cmdqAddr, ITH_OPENVG_BASE + ITEM2D_VG_REG_BID2_BASE, hwReg->REG_BID2_BASE);
    #endif
    ithCmdQFlush(cmdqAddr);
    ithCmdQUnlock();

    m2dvgPrint("=======================================\n");
    m2dvgPrint("    Finish CMD Fire(%u)\n", count);
    m2dvgPrint("=======================================\n\n");
}
#endif

//static ITEHardwareRegister* g_hardware = NULL;
static ITEM2DHardwareRegister PhysicalHwReg = {0};

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
static int dumpFlag = 0;

void iteM2dHardwareFire(
    ITEM2DHardwareRegister *hwReg)
{
    if (hwReg)
    {
        static ITEM2Duint32 count = 1;

        /* Dump*/
        if (dumpFlag == 1)
            _DumpPattern(hwReg, count);

#if 0   // Test code
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
        ithFlushDCacheRange((void *)hwReg->REG_PBR_BASE, hwReg->REG_PLR_BASE);
        ithFlushMemBuffer();
#endif

/*
        if ( !(hwReg->REG_TCR_BASE & 0x01) )
        // Source Base
        {
            ITEM2Duint8* srcBuffer = hwReg->REG_SBR_BASE;
            ITEM2Duint32 srcPitch = (hwReg->REG_SDPR_BASE & ITEM2D_VG_CMDMASK_SRCPITCH0) >> ITEM2D_VG_CMDSHIFT_SRCPITCH0;
            ITEM2Duint32 srcHeight = (hwReg->REG_SHWR_BASE & ITEM2D_VG_CMDMASK_SRCHEIGHT) >> ITEM2D_VG_CMDSHIFT_SRCHEIGHT;

   #ifndef _WIN32
            ithFlushDCacheRange((void*)srcBuffer, srcPitch * srcHeight);
            ithFlushMemBuffer();
   #endif
        }
 */

#ifdef ITEM2D_VG_USE_CMDQ
        iteM2dCmdqWriteRegister(hwReg, count);
#else
        /* Use HOST to write all register to engine */
        iteM2dHostWriteRegister(hwReg, count);
#endif
        count++;
    }
}

void iteM2dHardwareNullFire(
    ITEM2Duint32 frameID)
{
    static ITEM2Duint32 count     = 0;
    uint32_t            *cmdqAddr = NULL;

#ifdef ITEM2D_VG_NULL_CMDQ
    return;
#endif

#ifdef ITEM2D_VG_USE_CMDQ
    ithCmdQLock();
    cmdqAddr = ithCmdQWaitSize(2 * 8);      // 1 fire + 1 framID

    /* Engine fire */
    ITH_CMDQ_SINGLE_CMD(cmdqAddr, ITH_OPENVG_BASE + ITEM2D_VG_REG_CMDR_BASE, 0x03);

    /* Write frame ID */
    ITH_CMDQ_SINGLE_CMD(cmdqAddr, ITH_OPENVG_BASE + ITEM2D_VG_REG_BID1_BASE, frameID);

    ithCmdQFlush(cmdqAddr);
    ithCmdQUnlock();

    m2dvgPrint("=======================================\n");
    m2dvgPrint("    Null Command(%d)\n", frameID);
    m2dvgPrint("=======================================\n\n");
#else
    /* Engine fire */
    ithWriteRegA(ITH_OPENVG_BASE + ITEM2D_VG_REG_CMDR_BASE, 0x03);

    /* Wait engine idle */
    {
        ITEM2Duint32 regValue;

        do
        {
            regValue = ithReadRegA(ITH_OPENVG_BASE + ITEM2D_VG_REG_ESR1_BASE);
            if (regValue & 1)
            {
                usleep(1);
            }
            else
            {
                break;
            }
        } while (1);
    }

    /* Write frame ID */
    ithWriteRegA(ITH_OPENVG_BASE + ITEM2D_VG_REG_BID1_BASE, frameID);
    m2dvgPrint("0x%08X = 0x%08X\n", ITEM2D_VG_REG_BID1_BASE, frameID);

    m2dvgPrint("=======================================\n");
    m2dvgPrint("    Null Command(%d)\n", frameID);
    m2dvgPrint("=======================================\n\n");
#endif
}

void
ITEM2DHardware_ctor(
    ITEM2DHardwareRegister *hw)
{
#if 1
    ITEM2Dint i;
    memset(hw, 0x00, sizeof(ITEM2DHardwareRegister));

    /* This value from HW guy */
    hw->REG_ACR_BASE = 0x01e40003;
    //hw->REG_RFR_BASE = ITEM2D_VG_CMD_MASKEXTEND_EN |
    //                   ITEM2D_VG_CMD_DSTEXTEND_EN |
    //                   ITEM2D_VG_CMD_SRCEXTEND_EN;

    if (M2dVgInitCount == 0)
    {
        /* Enable TCLK divider */
        ithWriteRegMaskH(ITH_OPVG_CLK2_REG, (1 << 15), (1 << 15));

        /* Enable VCLK divider */
        ithWriteRegMaskH(ITH_OPVG_CLK3_REG, (1 << 15), (1 << 15));

        /* Reset OpenVG and enable N9CLK, M16CLK, VCLK, and TCLK */
        ithWriteRegMaskH(ITH_OPVG_CLK1_REG, (1 << 15) | (1 << 7) | (1 << 5) | (1 << 3) | (1 << 1), (1 << 15) | (1 << 7) | (1 << 5) | (1 << 3) | (1 << 1));
    #ifndef _WIN32
        for (i = 0; i < 100; i++)
            asm ("");
    #endif
        ithWriteRegMaskH(ITH_OPVG_CLK1_REG, (0 << 15) | (1 << 7) | (1 << 5) | (1 << 3) | (1 << 1), (1 << 15) | (1 << 7) | (1 << 5) | (1 << 3) | (1 << 1));
    }
    M2dVgInitCount++;

#else
    ITEint i, j;

    /* Tessellation */
    h->paintMode = HW_FILL_PATH;
    h->lineWidth = 0;
    h->min.x     = 32767;
    h->min.y     = 32767;
    h->max.x     = 0;
    h->max.y     = 0;
    M2DSETMAT(h->pathTransform, 0x10000, 0, 0, 0, 0x10000, 0, 0, 0, 0x10000);

    /* Mode settings */
    h->fillRule         = HW_EVEN_ODD;
    h->imageQuality     = HW_IMAGE_QUALITY_FASTER;
    h->renderingQuality = HW_RENDERING_QUALITY_BETTER;
    h->blendMode        = HW_BLEND_SRC_OVER;
    h->maskMode         = HW_INTERSECT_RENDERMASK;
    h->imageMode        = HW_DRAW_IMAGE_NORMAL;
    h->paintMode        = HW_FILL_PATH;

    /* enabling */
    h->enCoverage       = HW_TRUE; // enable coverage
    h->enScissor        = HW_FALSE;
    h->enMask           = HW_FALSE;
    h->enTexture        = HW_FALSE;
    h->enLookup         = HW_FALSE;
    h->enBlend          = HW_TRUE;
    h->enColorTransform = HW_FALSE;
    h->enLookup         = HW_FALSE;
    h->enSrcMultiply    = HW_TRUE;
    h->enSrcUnMultiply  = HW_TRUE;
    h->enDstMultiply    = HW_TRUE;
    h->enPerspective    = HW_FALSE;

    /* color */
    CSET(h->tileFillColor, 0, 0, 0, 0);     //Edge fill color for vgConvolve and pattern paint
    CSET(h->clearColor,  128, 128, 128, 0); // vgClear,

    /* Matrices */
    M2DSETMAT(h->fillTransform, 0x10000, 0, 0, 0, 0x10000, 0, 0, 0, 0x10000);
    M2DSETMAT(h->strokeTransform, 0x10000, 0, 0, 0, 0x10000, 0, 0, 0, 0x10000);
    M2DSETMAT(h->imageTransform, 0x10000, 0, 0, 0, 0x10000, 0, 0, 0, 0x10000);

    /* Image */
    h->coverageData   = NULL;
    h->coverageWidth  = 0;
    h->coverageHeight = 0;
    h->textureData    = NULL;
    h->textureWidth   = 0;
    h->textureHeight  = 0;
    h->maskData       = NULL;
    h->maskWidth      = 0;
    h->maskHeight     = 0;
    h->surfaceData    = NULL;
    h->surfaceWidth   = 0;
    h->surfaceHeight  = 0;
    h->gradientLen    = 10;
    h->gradientData   = (ITEuint8 *)malloc(1 << (h->gradientLen + 2));

    // ColorTransform
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            h->colorTransform[i][j] = (i == j) ? 0x100 : 0;  //s7.8
    for (i = 0; i < 4; i++)
        h->colorBias[i] = 0;  //s8

    // Lookup Table
    for (i = 0; i < 4; i++)
        for (j = 0; j < 256; j++)
            h->lookupTable[i][j] = j;
#endif
}

/*-----------------------------------------------------
 * VGContext constructor
 *-----------------------------------------------------*/
void
ITEM2DHardware_dtor(
    ITEM2DHardwareRegister *hw)
{
#if 1
    M2dVgInitCount--;

    if (M2dVgInitCount == 0)
    {
        /* Disable TCLK divider */
        ithWriteRegMaskH(ITH_OPVG_CLK2_REG, (0 << 15), (1 << 15));

        /* Disable VCLK divider */
        ithWriteRegMaskH(ITH_OPVG_CLK3_REG, (0 << 15), (1 << 15));

        /* Disable N9CLK, M16CLK, VCLK, and TCLK */
        ithWriteRegMaskH(ITH_OPVG_CLK1_REG, (0 << 15) | (0 << 7) | (0 << 5) | (0 << 3) | (0 << 1), (1 << 15) | (1 << 7) | (1 << 5) | (1 << 3) | (1 << 1));
    }
#else
    free(h->coverageData);
    h->coverageData = NULL;
    free(h->textureData);
    h->textureData  = NULL;
    free(h->patternData);
    h->patternData  = NULL;
#endif
}

#if 1
void
iteM2dCreateHardware(
    ITEM2DHardwareRegister *hwReg)
{
    ITEM2DHardware_ctor(hwReg);
}
#else
ITEM2DHardware *
iteCreateHardware()
{
    /* return if already created */
    if (g_hardware)
        return g_hardware;

    /* create new context */
    ITEM2D_NEWOBJ(ITEM2DHardware, g_hardware);
    if (!g_hardware)
        return NULL;

    return g_hardware;
}
#endif

#if 1
void
iteM2dDestroyHardware(
    ITEM2DHardwareRegister *hw)
{
    ITEM2DHardware_dtor(hw);
}
#else
void iteDestroyHardware()
{
    /* return if already released */
    if (!g_hardware)
        return;

    /* delete context object */
    ITEM2D_DELETEOBJ(ITEM2DHardware, g_hardware);
    g_hardware = NULL;
}
#endif

ITEM2Duint32
iteM2dHardwareGenFrameID()
{
    M2dVgFrameID++;

    if (M2dVgFrameID == INVALID_FRAME_ID)
    {
        M2dVgFrameID = 0;
    }

    return M2dVgFrameID;
}

ITEM2Duint32
iteM2dHardwareGenObjectID()
{
    M2dVgObjectID++;

    if (M2dVgObjectID == INVALID_OBJECT_ID)
    {
        M2dVgObjectID = 0;
    }

    return M2dVgObjectID;
}

ITEM2Duint32
iteM2dHardwareGetCurrentFrameID()
{
    return M2dVgFrameID;
}

ITEM2Duint32
iteM2dHardwareGetCurrentObjectID()
{
    return M2dVgObjectID;
}

ITEM2Duint32
iteM2dHardwareGetNextFrameID()
{
    if (M2dVgFrameID + 1 == INVALID_FRAME_ID)
    {
        return 0;
    }
    else
    {
        return M2dVgFrameID + 1;
    }
}

ITEM2Duint32
iteM2dHardwareGetNextObjectID()
{
    if (M2dVgObjectID + 1 == INVALID_OBJECT_ID)
    {
        return 0;
    }
    else
    {
        return M2dVgObjectID + 1;
    }
}

void
iteM2dHardwareWaitObjID(
    ITEM2Duint32 objID)
{
#ifdef ITEM2D_VG_USE_INTR
#else
    uint32_t currObjID = 0;
    uint16_t objId0    = 0;
    uint16_t objId1    = 0;

    #ifdef ITEM2D_VG_NULL_CMDQ
    return;
    #endif

    if (objID == 0)
    {
        //printf("obj Retrace!\n");
        while (1)
        {
            currObjID = ithReadRegA(ITH_OPENVG_BASE + ITEM2D_VG_REG_BID2_BASE);
            if (currObjID == objID
                || ((currObjID < 1000) && (currObjID > objID)))
            {
                break;
            }
            //printf("    hw obj id = %d, waitID = %d\n", currObjID, objID);
            usleep(100 * 1000);
        }
    }
    else
    {
        do
        {
            currObjID = ithReadRegA(ITH_OPENVG_BASE + ITEM2D_VG_REG_BID2_BASE);
        } while (!IS_A_CATCHUP_WITH_B(currObjID, objID, INVALID_OBJECT_ID));
    }
#endif
}

void
iteM2dHardwareWaitFrameID(
    ITEM2Duint32 frameID)
{
#ifdef ITEM2D_VG_USE_INTR
#else
    uint32_t currFrameID = 0;
    uint16_t frameId0    = 0;
    uint16_t frameId1    = 0;

    #ifdef ITEM2D_VG_NULL_CMDQ
    return;
    #endif

    if (frameID == 0)
    {
        //printf("frame Retrace!\n");
        while (1)
        {
            currFrameID = ithReadRegA(ITH_OPENVG_BASE + ITEM2D_VG_REG_BID1_BASE);
            if (currFrameID == frameID
                || ((currFrameID > frameID) && ((currFrameID - frameID) < 100)))
            {
                break;
            }
            //printf("    hw frame id = %d, waitID = %d\n", currFrameID, frameID);
            usleep(100 * 1000);
        }
    }
    else
    {
        do
        {
            currFrameID = ithReadRegA(ITH_OPENVG_BASE + ITEM2D_VG_REG_BID1_BASE);
        } while (!IS_A_CATCHUP_WITH_B(currFrameID, frameID, INVALID_FRAME_ID));
    }
#endif
}

//todo: check clipping
//todo: check rotate
MMP_RESULT
m2dAlphaBlend(
    M2D_SURFACE *destSurface,
    MMP_INT     destX,
    MMP_INT     destY,
    MMP_UINT    destWidth,
    MMP_UINT    destHeight,
    M2D_SURFACE *srcSurface,
    MMP_INT     srcX,
    MMP_INT     srcY,
    HWBlendMode blendMode,
    MMP_INT     srcConstAlpha)
{
    ITEM2DHardwareRegister hwReg                = {0};
    ITEM2Dboolean          srcPreMultiFormat    = ITEM2D_FALSE;
    ITEM2Dboolean          dstPreMultiFormat    = ITEM2D_FALSE;
    ITEM2Dboolean          EnPreMultiForTexture = ITEM2D_FALSE;  // Reg 0x0AC, Bit 28
    ITEM2Dboolean          EnUnpreMultiForDst   = ITEM2D_FALSE;  // Reg 0x0AC, Bit 31
    ITEM2Dboolean          EnPreMultiForBlend   = ITEM2D_FALSE;  // Reg 0x0AC, Bit 29
    ITEM2Dboolean          EnUnpreMultiForCT    = ITEM2D_FALSE;  // Reg 0x0AC, Bit 30
    ITEM2Dboolean          enBlend              = ITEM2D_TRUE;
    HWImageFormat          destImageFormat      = HW_sRGB_565;
    HWImageFormat          srcImageFormat       = HW_sRGB_565;
    ITEM2DMatrix3x3        rotateMat            = {0};
    ITEM2DMatrix3x3        rotateinverseMat     = {0};
    ITEM2DMatrix3x3        transMat             = {1, 0, 0, 0, 1, 0, 0, 0, 1};
    ITEM2DMatrix3x3        transinverseMat      = {0};

    ITEM2D_INITOBJ(ITEM2DHardware, hwReg);

    TRANSLATEMATR(transMat, destX, destY);

    iteM2dInvertMatrix(&transMat, &transinverseMat);

    dstPreMultiFormat = ITEM2D_FALSE;

    if (srcPreMultiFormat == dstPreMultiFormat)
    {
        EnPreMultiForTexture = EnUnpreMultiForDst = ITEM2D_FALSE;
    }

    /* Disable tesslellation */
    hwReg.REG_TCR_BASE = ITEM2D_VG_CMD_TRANSFORM_EN;

    /* Set clipping range */
    hwReg.REG_CCR_BASE = ITEM2D_VG_CMD_CLIPPING |
                         ITEM2D_VG_CMD_FULLRDY_EN |
                         ITEM2D_VG_CMD_TIMERDY_EN;

    if (((M2D_SURFACE *)destSurface)->clipSet.clipRegionCount == 1)
    {
        hwReg.REG_PXCR_BASE = ((((M2D_SURFACE *)destSurface)->clipSet.clipRegion->right) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND) | (((M2D_SURFACE *)destSurface)->clipSet.clipRegion->left);
        hwReg.REG_PYCR_BASE = ((((M2D_SURFACE *)destSurface)->clipSet.clipRegion->bottom) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND) | (((M2D_SURFACE *)destSurface)->clipSet.clipRegion->top);
        //printf("m2dAlphaBlend right = %d,left = %d,bottom = %d,top = %d\n",((M2D_SURFACE*)destSurface)->clipSet.clipRegion->right,((M2D_SURFACE*)destSurface)->clipSet.clipRegion->left,((M2D_SURFACE*)destSurface)->clipSet.clipRegion->bottom,((M2D_SURFACE*)destSurface)->clipSet.clipRegion->top);
        //hwReg.REG_PXCR_BASE = (destX + destWidth - 1) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND;
        //hwReg.REG_PYCR_BASE = (destY + destHeight - 1) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND;
        //printf("destW = %d,destX = %d,destH = %d,destY = %d\n",(destX + destWidth - 1),destX,(destY + destHeight - 1),destY);
    }
    else
    {
        hwReg.REG_PXCR_BASE = (destX + destWidth - 1) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND;
        hwReg.REG_PYCR_BASE = (destY + destHeight - 1) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND;
    }

    /* Setup render engine */
    // Render Control Register
    hwReg.REG_RCR_BASE = (EnUnpreMultiForDst ? ITEM2D_VG_CMD_DST_PRE_EN : 0) |
                         ITEM2D_VG_CMD_TEX_PRE_EN |
                         ITEM2D_VG_CMD_DESTINATION_EN |
                         (enBlend ? ITEM2D_VG_CMD_BLEND_EN : 0) |
                         ITEM2D_VG_CMD_TEXCACHE_EN |
                         ITEM2D_VG_CMD_TEXTURE_EN |
                         ITEM2D_VG_CMD_RENDERMODE_1 |
                         ITEM2D_VG_CMD_DITHER_EN;  // always enable dither

    hwReg.REG_ACR_BASE |= ITEM2D_VG_CMD_CLRCOVCACHE | ITEM2D_VG_CMD_CLRRDRCACHE;

    // Render Mode Register
    hwReg.REG_RMR_BASE  = ITEM2D_VG_CMD_AUTOSCAN_EN |
        (((ITEM2Duint32)blendMode & 0x1F) << ITEM2D_VG_CMDSHIFT_BLENDMODE) |
        ITEM2D_VG_CMD_MASKMODE_INTERSECT;

    destImageFormat = _M2dImgFormat2HwImgFormat(destSurface->imageFormat);
    srcImageFormat  = _M2dImgFormat2HwImgFormat(srcSurface->imageFormat);

    hwReg.REG_RFR_BASE |= (destImageFormat << 8) | srcImageFormat;

    hwReg.REG_CAR_BASE  = 0xff | (srcConstAlpha << 8) | (srcConstAlpha << 16) | (srcConstAlpha << 24);

    // Destination Coordinate Register
    hwReg.REG_DCR_BASE  = (((ITEM2Ds12p3)destY << ITEM2D_VG_CMDSHIFT_DSTY) & ITEM2D_VG_CMDMASK_DSTY) |
                          ((ITEM2Ds12p3)destX & ITEM2D_VG_CMDMASK_DSTX);

    // Destination Height/Width Register
    hwReg.REG_DHWR_BASE = (((ITEM2Dint16)destSurface->height << ITEM2D_VG_CMDSHIFT_DSTHEIGHT) & ITEM2D_VG_CMDMASK_DSTHEIGHT) |
                          ((ITEM2Dint16)destSurface->width & ITEM2D_VG_CMDMASK_DSTWIDTH);
    // Destination Base Register
    hwReg.REG_DBR_BASE  = (ITEM2Duint32)(destSurface->baseScanPtr) & ITEM2D_VG_CMDMASK_SRCBASE;

    // Src/Dst Pitch Register
    hwReg.REG_SDPR_BASE = (((ITEM2Dint16)srcSurface->pitch << ITEM2D_VG_CMDSHIFT_SRCPITCH0) & ITEM2D_VG_CMDMASK_SRCPITCH0) |
                          (destSurface->pitch & ITEM2D_VG_CMDMASK_DSTPITCH);
    // Source Coordinate Register
    hwReg.REG_SCR_BASE  = (((ITEM2Ds12p3)0 << ITEM2D_VG_CMDSHIFT_DSTY) & ITEM2D_VG_CMDMASK_DSTY) |
                          ((ITEM2Ds12p3)0 & ITEM2D_VG_CMDMASK_DSTX);
    // Source Height/Width Register
    hwReg.REG_SHWR_BASE = (((ITEM2Dint16)srcSurface->height << ITEM2D_VG_CMDSHIFT_DSTHEIGHT) & ITEM2D_VG_CMDMASK_DSTHEIGHT) |
                          ((ITEM2Dint16)srcSurface->width & ITEM2D_VG_CMDMASK_DSTWIDTH);
    // Source Base Register
    //hwReg.REG_SBR_BASE = (ITEM2Duint32)(srcSurface->baseScanPtr)& ITEM2D_VG_CMDMASK_SRCBASE;
    if (srcSurface->imageFormat == MMP_M2D_IMAGE_FORMAT_ARGB8888)
        hwReg.REG_SBR_BASE = (ITEM2Duint32)(srcSurface->baseScanPtr + srcY * srcSurface->pitch + srcX * 4) & ITEM2D_VG_CMDMASK_SRCBASE;
    else
        hwReg.REG_SBR_BASE = (ITEM2Duint32)(srcSurface->baseScanPtr + srcY * srcSurface->pitch + srcX * 2) & ITEM2D_VG_CMDMASK_SRCBASE;

    hwReg.REG_UTR00_BASE  = (ITEM2Ds15p16)(transMat.m[0][0] * 0x10000);
    hwReg.REG_UTR01_BASE  = (ITEM2Ds15p16)(transMat.m[0][1] * 0x10000);
    hwReg.REG_UTR02_BASE  = (ITEM2Ds15p16)(transMat.m[0][2] * 0x10000);
    hwReg.REG_UTR10_BASE  = (ITEM2Ds15p16)(transMat.m[1][0] * 0x10000);
    hwReg.REG_UTR11_BASE  = (ITEM2Ds15p16)(transMat.m[1][1] * 0x10000);
    hwReg.REG_UTR12_BASE  = (ITEM2Ds15p16)(transMat.m[1][2] * 0x10000);
    hwReg.REG_UTR20_BASE  = (ITEM2Ds15p16)(transMat.m[2][0] * 0x10000);
    hwReg.REG_UTR21_BASE  = (ITEM2Ds15p16)(transMat.m[2][1] * 0x10000);
    hwReg.REG_UTR22_BASE  = (ITEM2Ds15p16)(transMat.m[2][2] * 0x10000);

    // User Inverse Transform
    hwReg.REG_UITR00_BASE = (ITEM2Ds15p16)(transinverseMat.m[0][0] * 0x10000);
    hwReg.REG_UITR01_BASE = (ITEM2Ds15p16)(transinverseMat.m[0][1] * 0x10000);
    hwReg.REG_UITR02_BASE = (ITEM2Ds15p16)(transinverseMat.m[0][2] * 0x10000);
    hwReg.REG_UITR10_BASE = (ITEM2Ds15p16)(transinverseMat.m[1][0] * 0x10000);
    hwReg.REG_UITR11_BASE = (ITEM2Ds15p16)(transinverseMat.m[1][1] * 0x10000);
    hwReg.REG_UITR12_BASE = (ITEM2Ds15p16)(transinverseMat.m[1][2] * 0x10000);
    hwReg.REG_UITR20_BASE = (ITEM2Ds15p16)(transinverseMat.m[2][0] * 0x10000);
    hwReg.REG_UITR21_BASE = (ITEM2Ds15p16)(transinverseMat.m[2][1] * 0x10000);
    hwReg.REG_UITR22_BASE = (ITEM2Ds15p16)(transinverseMat.m[2][2] * 0x10000);

    hwReg.REG_PITR00_BASE = (ITEM2Ds15p16)(1 * 0x10000);
    hwReg.REG_PITR01_BASE = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_PITR02_BASE = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_PITR10_BASE = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_PITR11_BASE = (ITEM2Ds15p16)(1 * 0x10000);
    hwReg.REG_PITR12_BASE = (ITEM2Ds15p16)(0 * 0x10000);

    /* Set image object ID */
    iteM2dHardwareGenObjectID();
    hwReg.REG_BID2_BASE    = iteM2dHardwareGetCurrentObjectID();
    destSurface->bitbltId2 = hwReg.REG_BID2_BASE;
    srcSurface->bitbltId2  = hwReg.REG_BID2_BASE;

    /* Set HW to cmdq */
    iteM2dHardwareFire(&hwReg);

    return MMP_SUCCESS;
}

MMP_RESULT
m2dSourceCopy(
    M2D_SURFACE *destDisplay,
    MMP_INT     destX,
    MMP_INT     destY,
    MMP_UINT    destWidth,
    MMP_UINT    destHeight,
    M2D_SURFACE *srcDisplay,
    MMP_INT     srcX,
    MMP_INT     srcY)
{
    ITEM2DHardwareRegister hwReg                 = {0};
    ITEM2Dboolean          srcPreMultiFormat     = ITEM2D_FALSE;
    ITEM2Dboolean          dstPreMultiFormat     = ITEM2D_FALSE;
    ITEM2Dboolean          EnPreMultiForTexture  = ITEM2D_FALSE;  // Reg 0x0AC, Bit 28
    ITEM2Dboolean          EnUnpreMultiForDst    = ITEM2D_FALSE;  // Reg 0x0AC, Bit 31
    ITEM2Dboolean          EnPreMultiForBlend    = ITEM2D_FALSE;  // Reg 0x0AC, Bit 29
    ITEM2Dboolean          EnUnpreMultiForCT     = ITEM2D_FALSE;  // Reg 0x0AC, Bit 30
    ITEM2Dboolean          enBlend               = ITEM2D_TRUE;
    HWImageFormat          destImageFormat       = HW_sRGB_565;
    HWImageFormat          srcImageFormat        = HW_sRGB_565;
    ITEM2DMatrix3x3        rotateMat             = {0};
    ITEM2DMatrix3x3        rotateinverseMat      = {0};
    ITEM2DMatrix3x3        transMat              = {1, 0, 0, 0, 1, 0, 0, 0, 1};
    ITEM2DMatrix3x3        transinverseMat       = {0};
    ITEM2Duint32           hwRenderQuality       = ITEM2D_VG_CMD_RENDERQ_BETTER;
    ITEM2Duint32           hwImageQuality        = ITEM2D_VG_CMD_IMAGEQ_FASTER;
    ITEM2Duint32           hwFillRule            = ITEM2D_VG_CMD_FILLRULE_ODDEVEN;
    ITEM2Duint32           hwCoverageFormatBytes = ITEM2D_VG_CMD_PLANFORMAT_TWOBYTES;
    ITEM2DImage            *hwCoverageImage      = NULL;
    ITEM2DImage            *hwValidImage         = NULL;

    ITEM2D_INITOBJ(ITEM2DHardware, hwReg);
    TRANSLATEMATR(transMat, destX, destY);

#if 0
    {
        if (destX != 0)
        {
            ITEM2Dfloat a;

            a = ITEM2D_DEG2RAD(30);
            ROTATEMATR(transMat, a);
            printf("%f , %f, %f\n", transMat.m[0][0], transMat.m[0][1], transMat.m[0][2]);
            printf("%f , %f, %f\n", transMat.m[1][0], transMat.m[1][1], transMat.m[1][2]);
            printf("%f , %f, %f\n", transMat.m[2][0], transMat.m[2][1], transMat.m[2][2]);
        }
    }
#endif

    iteM2dInvertMatrix(&transMat, &transinverseMat);

    /* Disable tesslellation */
    hwReg.REG_TCR_BASE = ITEM2D_VG_CMD_TRANSFORM_EN;

    /* Set clipping range */
    hwReg.REG_CCR_BASE = ITEM2D_VG_CMD_CLIPPING |
                         ITEM2D_VG_CMD_FULLRDY_EN |
                         ITEM2D_VG_CMD_TIMERDY_EN;

    if (((M2D_SURFACE *)destDisplay)->clipSet.clipRegionCount == 1)
    {
        hwReg.REG_PXCR_BASE = ((((M2D_SURFACE *)destDisplay)->clipSet.clipRegion->right) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND) | (((M2D_SURFACE *)destDisplay)->clipSet.clipRegion->left);
        hwReg.REG_PYCR_BASE = ((((M2D_SURFACE *)destDisplay)->clipSet.clipRegion->bottom) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND) | (((M2D_SURFACE *)destDisplay)->clipSet.clipRegion->top);
    }
    else
    {
        hwReg.REG_PXCR_BASE = (destX + destWidth - 1) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND;
        hwReg.REG_PYCR_BASE = (destY + destHeight - 1) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND;
    }

    /* Setup render engine */
    // Render Control Register
    hwReg.REG_RCR_BASE = (EnUnpreMultiForDst ? ITEM2D_VG_CMD_DST_PRE_EN : 0) |
                         (EnUnpreMultiForDst ? ITEM2D_VG_CMD_TEX_PRE_EN : 0) |
                         ITEM2D_VG_CMD_DESTINATION_EN |
                         ITEM2D_VG_CMD_TEXCACHE_EN |
                         ITEM2D_VG_CMD_TEXTURE_EN |
                         ITEM2D_VG_CMD_SRCCOPY_EN |
                         ITEM2D_VG_CMD_DITHER_EN | // always enable dither
                         ITEM2D_VG_CMD_RENDERMODE_1;

    hwReg.REG_RMR_BASE  = ITEM2D_VG_CMD_AUTOSCAN_EN;
    hwReg.REG_ACR_BASE |= ITEM2D_VG_CMD_CLRCOVCACHE | ITEM2D_VG_CMD_CLRRDRCACHE;

    /* Render format register */
    hwReg.REG_RFR_BASE &= 0x00FFFFFF;

    destImageFormat = _M2dImgFormat2HwImgFormat(destDisplay->imageFormat);
    srcImageFormat  = _M2dImgFormat2HwImgFormat(srcDisplay->imageFormat);

    hwReg.REG_RFR_BASE |= (destImageFormat << 8) | srcImageFormat;

    // Destination Coordinate Register
    hwReg.REG_DCR_BASE  = ((destY << ITEM2D_VG_CMDSHIFT_DSTY) & ITEM2D_VG_CMDMASK_DSTY) |
                          (destX & ITEM2D_VG_CMDMASK_DSTX);

    // Destination Height/Width Register
    hwReg.REG_DHWR_BASE = (((ITEM2Dint16)destDisplay->height << ITEM2D_VG_CMDSHIFT_DSTHEIGHT) & ITEM2D_VG_CMDMASK_DSTHEIGHT) |
                          ((ITEM2Dint16)destDisplay->width & ITEM2D_VG_CMDMASK_DSTWIDTH);
    // Destination Base Register
    hwReg.REG_DBR_BASE  = (ITEM2Duint32)(destDisplay->baseScanPtr) & ITEM2D_VG_CMDMASK_SRCBASE;

    // Src/Dst Pitch Register
    hwReg.REG_SDPR_BASE = (((ITEM2Dint16)srcDisplay->pitch << ITEM2D_VG_CMDSHIFT_SRCPITCH0) & ITEM2D_VG_CMDMASK_SRCPITCH0) |
                          (destDisplay->pitch & ITEM2D_VG_CMDMASK_DSTPITCH);

    // Source Coordinate Register
    hwReg.REG_SCR_BASE = ((0 << ITEM2D_VG_CMDSHIFT_SRCY) & ITEM2D_VG_CMDMASK_SRCY) |
                         (0 & ITEM2D_VG_CMDMASK_SRCX);

    // Source Height/Width Register
    hwReg.REG_SHWR_BASE = (((ITEM2Dint16)srcDisplay->height << ITEM2D_VG_CMDSHIFT_DSTHEIGHT) & ITEM2D_VG_CMDMASK_DSTHEIGHT) |
                          ((ITEM2Dint16)srcDisplay->width & ITEM2D_VG_CMDMASK_DSTWIDTH);

    // Source Base Register
    //hwReg.REG_SBR_BASE = (ITEM2Duint32)(srcDisplay->baseScanPtr)& ITEM2D_VG_CMDMASK_SRCBASE;
    if (srcDisplay->imageFormat == MMP_M2D_IMAGE_FORMAT_ARGB8888)
        hwReg.REG_SBR_BASE = (ITEM2Duint32)(srcDisplay->baseScanPtr + srcY * srcDisplay->pitch + srcX * 4) & ITEM2D_VG_CMDMASK_SRCBASE;
    else
        hwReg.REG_SBR_BASE = (ITEM2Duint32)(srcDisplay->baseScanPtr + srcY * srcDisplay->pitch + srcX * 2) & ITEM2D_VG_CMDMASK_SRCBASE;

    hwReg.REG_UTR00_BASE  = (ITEM2Ds15p16)(transMat.m[0][0] * 0x10000);
    hwReg.REG_UTR01_BASE  = (ITEM2Ds15p16)(transMat.m[0][1] * 0x10000);
    hwReg.REG_UTR02_BASE  = (ITEM2Ds15p16)(transMat.m[0][2] * 0x10000);
    hwReg.REG_UTR10_BASE  = (ITEM2Ds15p16)(transMat.m[1][0] * 0x10000);
    hwReg.REG_UTR11_BASE  = (ITEM2Ds15p16)(transMat.m[1][1] * 0x10000);
    hwReg.REG_UTR12_BASE  = (ITEM2Ds15p16)(transMat.m[1][2] * 0x10000);
    hwReg.REG_UTR20_BASE  = (ITEM2Ds15p16)(transMat.m[2][0] * 0x10000);
    hwReg.REG_UTR21_BASE  = (ITEM2Ds15p16)(transMat.m[2][1] * 0x10000);
    hwReg.REG_UTR22_BASE  = (ITEM2Ds15p16)(transMat.m[2][2] * 0x10000);

    // User Inverse Transform
    hwReg.REG_UITR00_BASE = (ITEM2Ds15p16)(transinverseMat.m[0][0] * 0x10000);
    hwReg.REG_UITR01_BASE = (ITEM2Ds15p16)(transinverseMat.m[0][1] * 0x10000);
    hwReg.REG_UITR02_BASE = (ITEM2Ds15p16)(transinverseMat.m[0][2] * 0x10000);
    hwReg.REG_UITR10_BASE = (ITEM2Ds15p16)(transinverseMat.m[1][0] * 0x10000);
    hwReg.REG_UITR11_BASE = (ITEM2Ds15p16)(transinverseMat.m[1][1] * 0x10000);
    hwReg.REG_UITR12_BASE = (ITEM2Ds15p16)(transinverseMat.m[1][2] * 0x10000);
    hwReg.REG_UITR20_BASE = (ITEM2Ds15p16)(transinverseMat.m[2][0] * 0x10000);
    hwReg.REG_UITR21_BASE = (ITEM2Ds15p16)(transinverseMat.m[2][1] * 0x10000);
    hwReg.REG_UITR22_BASE = (ITEM2Ds15p16)(transinverseMat.m[2][2] * 0x10000);

    hwReg.REG_PITR00_BASE = (ITEM2Ds15p16)(1 * 0x10000);
    hwReg.REG_PITR01_BASE = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_PITR02_BASE = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_PITR10_BASE = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_PITR11_BASE = (ITEM2Ds15p16)(1 * 0x10000);
    hwReg.REG_PITR12_BASE = (ITEM2Ds15p16)(0 * 0x10000);

    /* Set image object ID */
    iteM2dHardwareGenObjectID();
    hwReg.REG_BID2_BASE    = iteM2dHardwareGetCurrentObjectID();
    srcDisplay->bitbltId2  = hwReg.REG_BID2_BASE;
    destDisplay->bitbltId2 = hwReg.REG_BID2_BASE;

    /* Set HW to cmdq */
    iteM2dHardwareFire(&hwReg);

    return MMP_SUCCESS;
}

MMP_RESULT
m2dStretchSrcCopy(
    M2D_SURFACE *destSurface,
    MMP_INT     destX,
    MMP_INT     destY,
    MMP_UINT    destWidth,
    MMP_UINT    destHeight,
    M2D_SURFACE *srcSurface,
    MMP_INT     srcX,
    MMP_INT     srcY,
    MMP_UINT    srcWidth,
    MMP_UINT    srcHeight)
{
    ITEM2DHardwareRegister hwReg                = {0};
    ITEM2Dboolean          srcPreMultiFormat    = ITEM2D_FALSE;
    ITEM2Dboolean          dstPreMultiFormat    = ITEM2D_FALSE;
    ITEM2Dboolean          EnPreMultiForTexture = ITEM2D_FALSE; // Reg 0x0AC, Bit 28
    ITEM2Dboolean          EnUnpreMultiForDst   = ITEM2D_FALSE; // Reg 0x0AC, Bit 31
    HWMatrix3x3            transMat             = {0};
    ITEM2Dfloat            floatinverseWidth    = (ITEM2Dfloat)destWidth / srcWidth;
    ITEM2Dfloat            floatinverseHeight   = (ITEM2Dfloat)destHeight / srcHeight;
    ITEM2Duint             inverseWidth         = _mathFloatToS12_15((ITEM2Duint *)&floatinverseWidth);
    ITEM2Duint             inverseHeight        = _mathFloatToS12_15((ITEM2Duint *)&floatinverseHeight);
    HWImageFormat          destImageFormat      = HW_sRGB_565;
    HWImageFormat          srcImageFormat       = HW_sRGB_565;
    ITEM2Dboolean          enScissor            = ITEM2D_FALSE;
    ITEM2DMatrix3x3        rotateMat            = {0};
    ITEM2DMatrix3x3        rotateinverseMat     = {0};
    HWMatrix3x3            transinverseMat      = {0};
    ITEM2Dboolean          enBlend              = ITEM2D_FALSE;
    HWBlendMode            blendMode            = HW_BLEND_SRC;

    int                    shx                  = (float)(srcX - destX) * ((float)srcWidth / destWidth) * (1 << 16);
    int                    shy                  = (float)(srcY - destY) * ((float)srcHeight / destHeight) * (1 << 16);

    // stretch blt with alphablend
    if (srcSurface->imageFormat != MMP_M2D_IMAGE_FORMAT_RGB565)
    {
        blendMode            = HW_BLEND_SRC_OVER;
        EnPreMultiForTexture = ITEM2D_TRUE;
        enBlend              = ITEM2D_TRUE;
    }

    ITEM2D_INITOBJ(ITEM2DHardware, hwReg);

    M2DSETMAT(transMat,
              ((float)srcWidth / destWidth) * (1 << 16), 0,             shx,
              0,            ((float)srcHeight / destHeight) * (1 << 16), shy,
              0,            0,             0x10000);

    M2DSETMAT(rotateMat,
              1.0f,    0,       0,
              0,       1.0f,    0,
              0,       0,       1.0f);

    switch (destSurface->rotateOp)
    {
    case  MMP_M2D_ROTATE_OP_90:
        ROTATEMATR(rotateMat, 90);
        break;
    case  MMP_M2D_ROTATE_OP_180:
        ROTATEMATR(rotateMat, 180);
        break;
    case  MMP_M2D_ROTATE_OP_270:
        ROTATEMATR(rotateMat, 270);
        break;
    }

    iteM2dInvertMatrix(&rotateMat, &rotateinverseMat);

    MULMATMAT(rotateinverseMat, transMat, transinverseMat);

    /* Disable tesslellation */
    hwReg.REG_TCR_BASE = ITEM2D_VG_CMD_TRANSFORM_EN;

    /* Set clipping range */
    hwReg.REG_CCR_BASE = ITEM2D_VG_CMD_CLIPPING |
                         ITEM2D_VG_CMD_FULLRDY_EN |
                         ITEM2D_VG_CMD_TIMERDY_EN;

    if (((M2D_SURFACE *)destSurface)->clipSet.clipRegionCount == 1)
    {
        hwReg.REG_PXCR_BASE = ((((M2D_SURFACE *)destSurface)->clipSet.clipRegion->right) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND) | (((M2D_SURFACE *)destSurface)->clipSet.clipRegion->left);
        hwReg.REG_PYCR_BASE = ((((M2D_SURFACE *)destSurface)->clipSet.clipRegion->bottom) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND) | (((M2D_SURFACE *)destSurface)->clipSet.clipRegion->top);
        //printf("m2dStretchSrcCopy right = %d,left = %d,bottom = %d,top = %d\n",((M2D_SURFACE*)destSurface)->clipSet.clipRegion->right,((M2D_SURFACE*)destSurface)->clipSet.clipRegion->left,((M2D_SURFACE*)destSurface)->clipSet.clipRegion->bottom,((M2D_SURFACE*)destSurface)->clipSet.clipRegion->top);
    }
    else
    {
        hwReg.REG_PXCR_BASE = (destX + destWidth - 1) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND;
        hwReg.REG_PYCR_BASE = (destY + destHeight - 1) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND;
    }

    /* Setup render engine */
    // Render Control Register
    hwReg.REG_RCR_BASE = (EnUnpreMultiForDst ? ITEM2D_VG_CMD_DST_PRE_EN : 0) |
                         (EnPreMultiForTexture ? ITEM2D_VG_CMD_TEX_PRE_EN : 0) |
                         ITEM2D_VG_CMD_TEXCACHE_EN |
                         ITEM2D_VG_CMD_TEXTURE_EN |
                         (enBlend ? ITEM2D_VG_CMD_BLEND_EN : 0) |
                         ITEM2D_VG_CMD_DESTINATION_EN |
                         ITEM2D_VG_CMD_DITHER_EN | // always enable dither
                         ITEM2D_VG_CMD_RENDERMODE_1;

    hwReg.REG_ACR_BASE |= ITEM2D_VG_CMD_CLRCOVCACHE | ITEM2D_VG_CMD_CLRRDRCACHE;

    // Render Mode Register
    hwReg.REG_RMR_BASE  = ITEM2D_VG_CMD_AUTOSCAN_EN |
                          (((ITEM2Duint32)blendMode & 0x1F) << ITEM2D_VG_CMDSHIFT_BLENDMODE) |
                          ITEM2D_VG_CMD_MASKMODE_INTERSECT;

    /* Render format register */
    hwReg.REG_RFR_BASE &= 0x00FFFFFF;

    destImageFormat = _M2dImgFormat2HwImgFormat(destSurface->imageFormat);
    srcImageFormat  = _M2dImgFormat2HwImgFormat(srcSurface->imageFormat);

    hwReg.REG_RFR_BASE |= (destImageFormat << 8) | srcImageFormat;

    // Destination Coordinate Register
    hwReg.REG_DCR_BASE  = (((ITEM2Ds12p3)destY << ITEM2D_VG_CMDSHIFT_DSTY) & ITEM2D_VG_CMDMASK_DSTY) |
                          ((ITEM2Ds12p3)destX & ITEM2D_VG_CMDMASK_DSTX);
    // Destination Height/Width Register
    hwReg.REG_DHWR_BASE = (((ITEM2Dint16)destSurface->height << ITEM2D_VG_CMDSHIFT_DSTHEIGHT) & ITEM2D_VG_CMDMASK_DSTHEIGHT) |
                          ((ITEM2Dint16)destSurface->width & ITEM2D_VG_CMDMASK_DSTWIDTH);
    // Destination Base Register
    hwReg.REG_DBR_BASE  = (ITEM2Duint32)(destSurface->baseScanPtr) & ITEM2D_VG_CMDMASK_SRCBASE;
    // Src/Dst Pitch Register
    hwReg.REG_SDPR_BASE = (((ITEM2Dint16)srcSurface->pitch << ITEM2D_VG_CMDSHIFT_SRCPITCH0) & ITEM2D_VG_CMDMASK_SRCPITCH0) |
                          (destSurface->pitch & ITEM2D_VG_CMDMASK_DSTPITCH);
    // Source Coordinate Register
    hwReg.REG_SCR_BASE  = (((ITEM2Ds12p3)0 << ITEM2D_VG_CMDSHIFT_DSTY) & ITEM2D_VG_CMDMASK_DSTY) |
                          ((ITEM2Ds12p3)0 & ITEM2D_VG_CMDMASK_DSTX);
    // Source Height/Width Register
    hwReg.REG_SHWR_BASE = (((ITEM2Dint16)srcSurface->height << ITEM2D_VG_CMDSHIFT_DSTHEIGHT) & ITEM2D_VG_CMDMASK_DSTHEIGHT) |
                          ((ITEM2Dint16)srcSurface->width & ITEM2D_VG_CMDMASK_DSTWIDTH);
    // Source Base Register
    //hwReg.REG_SBR_BASE |= (ITEM2Duint32)(srcSurface->baseScanPtr)& ITEM2D_VG_CMDMASK_SRCBASE;
    if (srcSurface->imageFormat == MMP_M2D_IMAGE_FORMAT_ARGB8888)
        hwReg.REG_SBR_BASE = (ITEM2Duint32)(srcSurface->baseScanPtr + srcY * srcSurface->pitch + srcX * 4) & ITEM2D_VG_CMDMASK_SRCBASE;
    else
        hwReg.REG_SBR_BASE = (ITEM2Duint32)(srcSurface->baseScanPtr + srcY * srcSurface->pitch + srcX * 2) & ITEM2D_VG_CMDMASK_SRCBASE;

    // User Inverse Transform
    hwReg.REG_UITR00_BASE = (transinverseMat.m[0][0] << ITEM2D_VG_CMDSHIFT_USRINV00) & ITEM2D_VG_CMDMASK_USRINV00;
    hwReg.REG_UITR01_BASE = (transinverseMat.m[0][1] << ITEM2D_VG_CMDSHIFT_USRINV01) & ITEM2D_VG_CMDMASK_USRINV01;
    hwReg.REG_UITR02_BASE = (transinverseMat.m[0][2] << ITEM2D_VG_CMDSHIFT_USRINV02) & ITEM2D_VG_CMDMASK_USRINV02;
    hwReg.REG_UITR10_BASE = (transinverseMat.m[1][0] << ITEM2D_VG_CMDSHIFT_USRINV10) & ITEM2D_VG_CMDMASK_USRINV10;
    hwReg.REG_UITR11_BASE = (transinverseMat.m[1][1] << ITEM2D_VG_CMDSHIFT_USRINV11) & ITEM2D_VG_CMDMASK_USRINV11;
    hwReg.REG_UITR12_BASE = (transinverseMat.m[1][2] << ITEM2D_VG_CMDSHIFT_USRINV12) & ITEM2D_VG_CMDMASK_USRINV12;
    hwReg.REG_UITR20_BASE = (transinverseMat.m[2][0] << ITEM2D_VG_CMDSHIFT_USRINV20) & ITEM2D_VG_CMDMASK_USRINV20;
    hwReg.REG_UITR21_BASE = (transinverseMat.m[2][1] << ITEM2D_VG_CMDSHIFT_USRINV21) & ITEM2D_VG_CMDMASK_USRINV21;
    hwReg.REG_UITR22_BASE = (transinverseMat.m[2][2] << ITEM2D_VG_CMDSHIFT_USRINV22) & ITEM2D_VG_CMDMASK_USRINV22;

    /* Set image object ID */
    iteM2dHardwareGenObjectID();
    hwReg.REG_BID2_BASE    = iteM2dHardwareGetCurrentObjectID();
    srcSurface->bitbltId2  = hwReg.REG_BID2_BASE;
    destSurface->bitbltId2 = hwReg.REG_BID2_BASE;

    /* Set HW to cmdq */
    iteM2dHardwareFire(&hwReg);

    return MMP_SUCCESS;
}

MMP_RESULT
m2dTransferBlock(
    M2D_SURFACE *destDisplay,
    MMP_INT     destX,
    MMP_INT     destY,
    MMP_UINT    destWidth,
    MMP_UINT    destHeight,
    M2D_SURFACE *srcDisplay,
    MMP_INT     srcX,
    MMP_INT     srcY,
    MMP_INT     rop)
{
    //todo: define brush??
    //do TransferBlock
    ITEM2DHardwareRegister hwReg                = {0};
    ITEM2Dboolean          srcPreMultiFormat    = ITEM2D_FALSE;
    ITEM2Dboolean          dstPreMultiFormat    = ITEM2D_FALSE;
    ITEM2Dboolean          EnPreMultiForTexture = ITEM2D_FALSE; // Reg 0x0AC, Bit 28
    ITEM2Dboolean          EnUnpreMultiForDst   = ITEM2D_FALSE; // Reg 0x0AC, Bit 31
    HWMatrix3x3            transMat             = {0};
    HWImageFormat          destImageFormat      = HW_sRGB_565;
    HWImageFormat          srcImageFormat       = HW_sRGB_565;
    ITEM2Dboolean          enScissor            = ITEM2D_FALSE;
    MMP_M2D_COLOR          fgColor              = destDisplay->brush->realizedBrush->foregroundColor;
    ITEM2DMatrix3x3        rotateMat            = {0};
    ITEM2DMatrix3x3        rotateinverseMat     = {0};
    HWMatrix3x3            transinverseMat      = {0};
    ITEM2DImage            *hwCoverageImage     = NULL;
    ITEM2DImage            *hwValidImage        = NULL;
    ITEM2Dboolean          textureEnable        = ITEM2D_FALSE;

    /* Get coverage image parameter */
    if (vgSurface->coverageIndex)
    {
        hwCoverageImage          = vgSurface->coverageImageB;
        hwValidImage             = vgSurface->validImageB;
        vgSurface->coverageIndex = 0;
    }
    else
    {
        hwCoverageImage          = vgSurface->coverageImageA;
        hwValidImage             = vgSurface->validImageA;
        vgSurface->coverageIndex = 1;
    }

    //need source
    if ((rop ^ (rop >> 2)) & 0x33)
    {
        textureEnable = ITEM2D_TRUE;
    }

    ITEM2D_INITOBJ(ITEM2DHardware, hwReg);

    M2DSETMAT(transMat,
              0x10000, 0,       (ITEM2Ds15p16)((srcX - destX) << 16),
              0,       0x10000, (ITEM2Ds15p16)((srcY - destY) << 16),
              0,       0,       0x10000);

    M2DSETMAT(rotateMat,
              1.0f,    0,       0,
              0,       1.0f,    0,
              0,       0,       1.0f);

    switch (destDisplay->rotateOp)
    {
    case MMP_M2D_ROTATE_OP_90:  ROTATEMATR(rotateMat, 90);  break;
    case MMP_M2D_ROTATE_OP_180: ROTATEMATR(rotateMat, 180); break;
    case MMP_M2D_ROTATE_OP_270: ROTATEMATR(rotateMat, 270); break;
    }

    iteM2dInvertMatrix(&rotateMat, &rotateinverseMat);

    MULMATMAT(rotateinverseMat, transMat, transinverseMat);

    /* Disable tesslellation */
    hwReg.REG_TCR_BASE = ((textureEnable == ITEM2D_TRUE) ? ITEM2D_VG_CMD_TRANSFORM_EN : 0) |
                         ITEM2D_VG_CMD_READMEM |
                         ITEM2D_VG_CMD_TELWORK;

    /* Set clipping range */
    hwReg.REG_CCR_BASE = ITEM2D_VG_CMD_CLIPPING |
                         ITEM2D_VG_CMD_RENDERQ_BETTER |
                         ITEM2D_VG_CMD_PLANFORMAT_TWOBYTES |
                         ITEM2D_VG_CMD_FULLRDY_EN |
                         ITEM2D_VG_CMD_TIMERDY_EN;

    if (((M2D_SURFACE *)destDisplay)->clipSet.clipRegionCount == 1)
    {
        hwReg.REG_PXCR_BASE = ((((M2D_SURFACE *)destDisplay)->clipSet.clipRegion->right) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND) | (((M2D_SURFACE *)destDisplay)->clipSet.clipRegion->left);
        hwReg.REG_PYCR_BASE = ((((M2D_SURFACE *)destDisplay)->clipSet.clipRegion->bottom) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND) | (((M2D_SURFACE *)destDisplay)->clipSet.clipRegion->top);
        //printf("m2dTransferBlock right = %d,left = %d,bottom = %d,top = %d\n",((M2D_SURFACE*)destDisplay)->clipSet.clipRegion->right,((M2D_SURFACE*)destDisplay)->clipSet.clipRegion->left,((M2D_SURFACE*)destDisplay)->clipSet.clipRegion->bottom,((M2D_SURFACE*)destDisplay)->clipSet.clipRegion->top);
    }
    else
    {
        hwReg.REG_PXCR_BASE = (hwCoverageImage->width - 1) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND;
        hwReg.REG_PYCR_BASE = (hwCoverageImage->height - 1) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND;
    }

    hwReg.REG_CPBR_BASE  = ((ITEM2Duint32)hwCoverageImage->data) >> ITEM2D_VG_CMDSHIFT_PLANBASE;
    hwReg.REG_CVPPR_BASE = (hwValidImage->pitch << ITEM2D_VG_CMDSHIFT_VALIDPITCH) |
                           (hwCoverageImage->pitch << ITEM2D_VG_CMDSHIFT_PLANPITCH);
    hwReg.REG_VPBR_BASE  = ((ITEM2Duint32)hwValidImage->data) << ITEM2D_VG_CMDSHIFT_VALIDBASE;
    hwReg.REG_ACR_BASE  |= ITEM2D_VG_CMD_CLRCOVCACHE | ITEM2D_VG_CMD_CLRRDRCACHE;

    /* Setup render engine */
    // Render Control Register
    hwReg.REG_RCR_BASE =
        ITEM2D_VG_CMD_DST_PRE_EN |
        ITEM2D_VG_CMD_SRC_NONPRE_EN |
        ITEM2D_VG_CMD_SRC_PRE_EN |
        ITEM2D_VG_CMD_TEX_PRE_EN |
        ITEM2D_VG_CMD_WR_NONPRE_EN |
        //ITEM2D_VG_CMD_DITHER_EN | // always enable dither
        ITEM2D_VG_CMD_DESTINATION_EN |
        ITEM2D_VG_CMD_DSTCOLOR_EN |
        ITEM2D_VG_CMD_TEXCACHE_EN |
        ((textureEnable == ITEM2D_TRUE) ? ITEM2D_VG_CMD_TEXTURE_EN : 0) |
        ITEM2D_VG_CMD_ROP3_EN |
        ((enScissor == ITEM2D_TRUE) ? ITEM2D_VG_CMD_SCISSOR_EN : 0) |
        ITEM2D_VG_CMD_RDPLN_VLD_EN |
        ITEM2D_VG_CMD_RENDERMODE_1;

    hwReg.REG_CAR_BASE   = rop | (rop << 8) | (rop << 16) | (rop << 24);

    // ROP3 pattern paint color
    hwReg.REG_PPCR0_BASE = (((fgColor & 0x0000001F) * 255) / 31) << 20 | (0xFF);
    hwReg.REG_PPCR1_BASE = ((((fgColor & 0x0000F800) >> 11) * 255) / 31) << 20 | (((((fgColor & 0x000007E0) >> 5) * 255) / 63) << 4);

    hwReg.REG_RMR_BASE   = ITEM2D_VG_CMD_AUTOSCAN_EN | ITEM2D_VG_CMD_PAINTTYPE_COLOR;

    /* Render format register */
    //hwReg.REG_RFR_BASE &= 0x00FFFFFF;

    destImageFormat = _M2dImgFormat2HwImgFormat(destDisplay->imageFormat);
    if (srcDisplay)
        srcImageFormat = _M2dImgFormat2HwImgFormat(srcDisplay->imageFormat);

    hwReg.REG_RFR_BASE |= (destImageFormat << 8) | srcImageFormat;

    // Destination Coordinate Register
    hwReg.REG_DCR_BASE  = (((ITEM2Ds12p3)destY << ITEM2D_VG_CMDSHIFT_DSTY) & ITEM2D_VG_CMDMASK_DSTY) |
                          ((ITEM2Ds12p3)destX & ITEM2D_VG_CMDMASK_DSTX);

    // Destination Height/Width Register
    hwReg.REG_DHWR_BASE = (((ITEM2Dint16)destHeight << ITEM2D_VG_CMDSHIFT_DSTHEIGHT) & ITEM2D_VG_CMDMASK_DSTHEIGHT) |
                          ((ITEM2Dint16)destWidth & ITEM2D_VG_CMDMASK_DSTWIDTH);

    // Destination Base Register
    hwReg.REG_DBR_BASE = (ITEM2Duint32)(destDisplay->baseScanPtr) & ITEM2D_VG_CMDMASK_SRCBASE;

    if (srcDisplay)
    {
        // Src/Dst Pitch Register
        hwReg.REG_SDPR_BASE = (((ITEM2Dint16)srcDisplay->pitch << ITEM2D_VG_CMDSHIFT_SRCPITCH0) & ITEM2D_VG_CMDMASK_SRCPITCH0) |
                              (destDisplay->pitch & ITEM2D_VG_CMDMASK_DSTPITCH);

        // Source Height/Width Register
        hwReg.REG_SHWR_BASE = (((ITEM2Dint16)srcDisplay->height << ITEM2D_VG_CMDSHIFT_DSTHEIGHT) & ITEM2D_VG_CMDMASK_DSTHEIGHT) |
                              ((ITEM2Dint16)srcDisplay->width & ITEM2D_VG_CMDMASK_DSTWIDTH);

        // Source Base Register
        hwReg.REG_SBR_BASE = (ITEM2Duint32)(srcDisplay->baseScanPtr) & ITEM2D_VG_CMDMASK_SRCBASE;
    }
    else
    {
        // Src/Dst Pitch Register
        hwReg.REG_SDPR_BASE = (destDisplay->pitch & ITEM2D_VG_CMDMASK_DSTPITCH);
    }
    // Source Coordinate Register
    hwReg.REG_SCR_BASE    = (((ITEM2Ds12p3)0 << ITEM2D_VG_CMDSHIFT_DSTY) & ITEM2D_VG_CMDMASK_DSTY) |
                            ((ITEM2Ds12p3)0 & ITEM2D_VG_CMDMASK_DSTX);

    hwReg.REG_UTR00_BASE  = (ITEM2Ds15p16)(1.0f * 0x10000);
    hwReg.REG_UTR01_BASE  = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_UTR02_BASE  = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_UTR10_BASE  = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_UTR11_BASE  = (ITEM2Ds15p16)(1.0f * 0x10000);
    hwReg.REG_UTR12_BASE  = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_UTR20_BASE  = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_UTR21_BASE  = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_UTR22_BASE  = (ITEM2Ds15p16)(1.0f * 0x10000);

    // User Inverse Transform
    hwReg.REG_UITR00_BASE = (transinverseMat.m[0][0] << ITEM2D_VG_CMDSHIFT_USRINV00) & ITEM2D_VG_CMDMASK_USRINV00;
    hwReg.REG_UITR01_BASE = (transinverseMat.m[0][1] << ITEM2D_VG_CMDSHIFT_USRINV01) & ITEM2D_VG_CMDMASK_USRINV01;
    hwReg.REG_UITR02_BASE = (transinverseMat.m[0][2] << ITEM2D_VG_CMDSHIFT_USRINV02) & ITEM2D_VG_CMDMASK_USRINV02;
    hwReg.REG_UITR10_BASE = (transinverseMat.m[1][0] << ITEM2D_VG_CMDSHIFT_USRINV10) & ITEM2D_VG_CMDMASK_USRINV10;
    hwReg.REG_UITR11_BASE = (transinverseMat.m[1][1] << ITEM2D_VG_CMDSHIFT_USRINV11) & ITEM2D_VG_CMDMASK_USRINV11;
    hwReg.REG_UITR12_BASE = (transinverseMat.m[1][2] << ITEM2D_VG_CMDSHIFT_USRINV12) & ITEM2D_VG_CMDMASK_USRINV12;
    hwReg.REG_UITR20_BASE = (transinverseMat.m[2][0] << ITEM2D_VG_CMDSHIFT_USRINV20) & ITEM2D_VG_CMDMASK_USRINV20;
    hwReg.REG_UITR21_BASE = (transinverseMat.m[2][1] << ITEM2D_VG_CMDSHIFT_USRINV21) & ITEM2D_VG_CMDMASK_USRINV21;
    hwReg.REG_UITR22_BASE = (transinverseMat.m[2][2] << ITEM2D_VG_CMDSHIFT_USRINV22) & ITEM2D_VG_CMDMASK_USRINV22;

    hwReg.REG_CTBR0_BASE  = (ITEM2Dint32)(0 * 0x100);
    hwReg.REG_CTBR1_BASE  = (ITEM2Dint32)(0 * 0x100);
    hwReg.REG_CTBR2_BASE  = (ITEM2Dint32)(0 * 0x100);
    hwReg.REG_CTBR3_BASE  = (ITEM2Dint32)(0 * 0x100);

    hwReg.REG_CTSR0_BASE  = (((ITEM2Dint16)(0 * 0x100) << ITEM2D_VG_CMDSHIFT_SCOLXFM00) & ITEM2D_VG_CMDMASK_SCOLXFM00) |
                            (ITEM2Dint16)(1 * 0x100);

    hwReg.REG_CTSR1_BASE  = (((ITEM2Dint16)(0 * 0x100) << ITEM2D_VG_CMDSHIFT_SCOLXFM00) & ITEM2D_VG_CMDMASK_SCOLXFM00) |
                            (ITEM2Dint16)(1 * 0x100);

    /* Set image object ID */
    iteM2dHardwareGenObjectID();
    hwReg.REG_BID2_BASE    = iteM2dHardwareGetCurrentObjectID();
    //srcDisplay->bitbltId2= hwReg.REG_BID2_BASE;
    destDisplay->bitbltId2 = hwReg.REG_BID2_BASE;

    /* Set HW to cmdq */
    iteM2dHardwareFire(&hwReg);

    return MMP_SUCCESS;
}

MMP_RESULT
m2dDrawTransparentBlock(
    M2D_SURFACE   *destSurface,
    MMP_INT       destX,
    MMP_INT       destY,
    MMP_UINT      destWidth,
    MMP_UINT      destHeight,
    M2D_SURFACE   *srcSurface,
    MMP_INT       srcX,
    MMP_INT       srcY,
    MMP_M2D_COLOR destHighColor,
    MMP_M2D_COLOR destLowColor,
    MMP_M2D_COLOR srcHighColor,
    MMP_M2D_COLOR srcLowColor,
    MMP_M2D_TROP  transparentRop)
{
    ITEM2DHardwareRegister hwReg                = {0};
    ITEM2Dboolean          srcPreMultiFormat    = ITEM2D_FALSE;
    ITEM2Dboolean          dstPreMultiFormat    = ITEM2D_FALSE;
    ITEM2Dboolean          EnPreMultiForTexture = ITEM2D_FALSE; // Reg 0x0AC, Bit 28
    ITEM2Dboolean          EnUnpreMultiForDst   = ITEM2D_FALSE; // Reg 0x0AC, Bit 31
    HWMatrix3x3            transMat             = {0};
    HWImageFormat          destImageFormat      = HW_sRGB_565;
    HWImageFormat          srcImageFormat       = HW_sRGB_565;
    ITEM2Dboolean          enScissor            = ITEM2D_FALSE;
    ITEM2DMatrix3x3        rotateMat            = {0};
    ITEM2DMatrix3x3        rotateinverseMat     = {0};
    HWMatrix3x3            transinverseMat      = {0};
    ITEM2Duint8            ropinverse           = 0;
    ITEM2DMatrix3x3        imageMatrix          = {0};

    ropinverse = ((transparentRop << 3) & 0x8) | ((transparentRop << 1) & 0x4) | ((transparentRop >> 1) & 0x2) | ((transparentRop >> 3) & 0x1);

    ITEM2D_INITOBJ(ITEM2DHardware, hwReg);

    /* Disable tesslellation */
    hwReg.REG_TCR_BASE = ITEM2D_VG_CMD_TRANSFORM_EN |
                         ITEM2D_VG_CMD_READMEM |
                         ITEM2D_VG_CMD_TELWORK;

    /* Set clipping range */
    hwReg.REG_CCR_BASE = ITEM2D_VG_CMD_CLIPPING |
                         ITEM2D_VG_CMD_FULLRDY_EN |
                         ITEM2D_VG_CMD_TIMERDY_EN;
    if (((M2D_SURFACE *)destSurface)->clipSet.clipRegionCount == 1)
    {
        hwReg.REG_PXCR_BASE = ((((M2D_SURFACE *)destSurface)->clipSet.clipRegion->right) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND) | (((M2D_SURFACE *)destSurface)->clipSet.clipRegion->left);
        hwReg.REG_PYCR_BASE = ((((M2D_SURFACE *)destSurface)->clipSet.clipRegion->bottom) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND) | (((M2D_SURFACE *)destSurface)->clipSet.clipRegion->top);
        //printf("m2dDrawTransparentBlock right = %d,left = %d,bottom = %d,top = %d\n",((M2D_SURFACE*)destSurface)->clipSet.clipRegion->right,((M2D_SURFACE*)destSurface)->clipSet.clipRegion->left,((M2D_SURFACE*)destSurface)->clipSet.clipRegion->bottom,((M2D_SURFACE*)destSurface)->clipSet.clipRegion->top);
    }
    else
    {
        hwReg.REG_PXCR_BASE = (destX + destWidth - 1) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND;
        hwReg.REG_PYCR_BASE = (destY + destHeight - 1) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND;
    }
    hwReg.REG_ACR_BASE |= ITEM2D_VG_CMD_CLRCOVCACHE | ITEM2D_VG_CMD_CLRRDRCACHE;

    /* Setup render engine */
    // Render Control Register
    hwReg.REG_RCR_BASE = ITEM2D_VG_CMD_DST_PRE_EN |
                         ITEM2D_VG_CMD_SRC_NONPRE_EN |
                         ITEM2D_VG_CMD_SRC_PRE_EN |
                         ITEM2D_VG_CMD_TEX_PRE_EN |
                         ITEM2D_VG_CMD_WR_NONPRE_EN |
                         ITEM2D_VG_CMD_DITHER_EN | // always enable dither
                         ITEM2D_VG_CMD_DESTINATION_EN |
                         ITEM2D_VG_CMD_TEXCACHE_EN |
                         ITEM2D_VG_CMD_TEXTURE_EN |
                         ITEM2D_VG_CMD_TBITBLT_EN |
                         ((enScissor == ITEM2D_TRUE) ? ITEM2D_VG_CMD_SCISSOR_EN : 0) |
                         ITEM2D_VG_CMD_RDPLN_VLD_EN |
                         ITEM2D_VG_CMD_RENDERMODE_1;

    hwReg.REG_CAR_BASE   = ropinverse;

    // tBitBlt Source HIGH color key
    hwReg.REG_RCR10_BASE = (((srcHighColor & 0x0000001F) * 255) / 31) << 20 | (0xFF);
    hwReg.REG_RCR11_BASE = ((((srcHighColor & 0x0000F800) >> 11) * 255) / 31) << 20 | (((((srcHighColor & 0x000007E0) >> 5) * 255) / 63) << 4);

    //tBitBlt Source LOW color key
    hwReg.REG_RCR20_BASE = (((srcLowColor & 0x0000001F) * 255) / 31) << 20 | (0xFF);
    hwReg.REG_RCR21_BASE = ((((srcLowColor & 0x0000F800) >> 11) * 255) / 31) << 20 | (((((srcLowColor & 0x000007E0) >> 5) * 255) / 63) << 4);

    //tBitBlt Destination HIGH color key
    hwReg.REG_RCR30_BASE = (((destHighColor & 0x0000001F) * 255) / 31) << 20 | (0xFF);
    hwReg.REG_RCR31_BASE = ((((destHighColor & 0x0000F800) >> 11) * 255) / 31) << 20 | (((((destHighColor & 0x000007E0) >> 5) * 255) / 63) << 4);

    //tBitBlt Destination LOW color key
    hwReg.REG_RCR40_BASE = (((destLowColor & 0x0000001F) * 255) / 31) << 20 | (0xFF);
    hwReg.REG_RCR41_BASE = ((((destLowColor & 0x0000F800) >> 11) * 255) / 31) << 20 | (((((destLowColor & 0x000007E0) >> 5) * 255) / 63) << 4);

    hwReg.REG_RMR_BASE   = ITEM2D_VG_CMD_AUTOSCAN_EN;

    /* Render format register */

    destImageFormat = _M2dImgFormat2HwImgFormat(destSurface->imageFormat);
    if (srcSurface)
        srcImageFormat = _M2dImgFormat2HwImgFormat(srcSurface->imageFormat);

    hwReg.REG_RFR_BASE |= (destImageFormat << 8) | srcImageFormat;

    // Destination Coordinate Register
    hwReg.REG_DCR_BASE  = (((ITEM2Ds12p3)destY << ITEM2D_VG_CMDSHIFT_DSTY) & ITEM2D_VG_CMDMASK_DSTY) |
                          ((ITEM2Ds12p3)destX & ITEM2D_VG_CMDMASK_DSTX);

    // Destination Height/Width Register
    hwReg.REG_DHWR_BASE = (((ITEM2Dint16)destSurface->height << ITEM2D_VG_CMDSHIFT_DSTHEIGHT) & ITEM2D_VG_CMDMASK_DSTHEIGHT) |
                          ((ITEM2Dint16)destSurface->width & ITEM2D_VG_CMDMASK_DSTWIDTH);

    // Destination Base Register
    hwReg.REG_DBR_BASE = (ITEM2Duint32)(destSurface->baseScanPtr) & ITEM2D_VG_CMDMASK_SRCBASE;

    if (srcSurface)
    {
        // Src/Dst Pitch Register
        hwReg.REG_SDPR_BASE = (((ITEM2Dint16)srcSurface->pitch << ITEM2D_VG_CMDSHIFT_SRCPITCH0) & ITEM2D_VG_CMDMASK_SRCPITCH0) |
                              (destSurface->pitch & ITEM2D_VG_CMDMASK_DSTPITCH);

        // Source Height/Width Register
        hwReg.REG_SHWR_BASE = (((ITEM2Dint16)srcSurface->height << ITEM2D_VG_CMDSHIFT_DSTHEIGHT) & ITEM2D_VG_CMDMASK_DSTHEIGHT) |
                              ((ITEM2Dint16)srcSurface->width & ITEM2D_VG_CMDMASK_DSTWIDTH);

        // Source Base Register
        hwReg.REG_SBR_BASE = (ITEM2Duint32)(srcSurface->baseScanPtr) & ITEM2D_VG_CMDMASK_SRCBASE;
    }
    else
    {
        // Src/Dst Pitch Register
        hwReg.REG_SDPR_BASE = (destSurface->pitch & ITEM2D_VG_CMDMASK_DSTPITCH);
    }
    // Source Coordinate Register
    hwReg.REG_SCR_BASE = (((ITEM2Ds12p3)0 << ITEM2D_VG_CMDSHIFT_DSTY) & ITEM2D_VG_CMDMASK_DSTY) |
                         ((ITEM2Ds12p3)0 & ITEM2D_VG_CMDMASK_DSTX);

    M2DSETMAT(transMat,
              0x10000, 0,       (ITEM2Ds15p16)((srcX - destX) << 16),
              0,       0x10000, (ITEM2Ds15p16)((srcY - destY) << 16),
              0,       0,       0x10000);

    M2DSETMAT(rotateMat,
              1.0f,    0,       0,
              0,       1.0f,    0,
              0,       0,       1.0f);

    switch (destSurface->rotateOp)
    {
    case  MMP_M2D_ROTATE_OP_90:
        ROTATEMATR(rotateMat, 90);
        break;
    case  MMP_M2D_ROTATE_OP_180:
        ROTATEMATR(rotateMat, 180);
        break;
    case  MMP_M2D_ROTATE_OP_270:
        ROTATEMATR(rotateMat, 270);
        break;
    }

    iteM2dInvertMatrix(&rotateMat, &rotateinverseMat);

    MULMATMAT(rotateinverseMat, transMat, transinverseMat);

    hwReg.REG_UITR00_BASE = (transinverseMat.m[0][0] << ITEM2D_VG_CMDSHIFT_USRINV00) & ITEM2D_VG_CMDMASK_USRINV00;
    hwReg.REG_UITR01_BASE = (transinverseMat.m[0][1] << ITEM2D_VG_CMDSHIFT_USRINV01) & ITEM2D_VG_CMDMASK_USRINV01;
    hwReg.REG_UITR02_BASE = (transinverseMat.m[0][2] << ITEM2D_VG_CMDSHIFT_USRINV02) & ITEM2D_VG_CMDMASK_USRINV02;
    hwReg.REG_UITR10_BASE = (transinverseMat.m[1][0] << ITEM2D_VG_CMDSHIFT_USRINV10) & ITEM2D_VG_CMDMASK_USRINV10;
    hwReg.REG_UITR11_BASE = (transinverseMat.m[1][1] << ITEM2D_VG_CMDSHIFT_USRINV11) & ITEM2D_VG_CMDMASK_USRINV11;
    hwReg.REG_UITR12_BASE = (transinverseMat.m[1][2] << ITEM2D_VG_CMDSHIFT_USRINV12) & ITEM2D_VG_CMDMASK_USRINV12;
    hwReg.REG_UITR20_BASE = (transinverseMat.m[2][0] << ITEM2D_VG_CMDSHIFT_USRINV20) & ITEM2D_VG_CMDMASK_USRINV20;
    hwReg.REG_UITR21_BASE = (transinverseMat.m[2][1] << ITEM2D_VG_CMDSHIFT_USRINV21) & ITEM2D_VG_CMDMASK_USRINV21;
    hwReg.REG_UITR22_BASE = (transinverseMat.m[2][2] << ITEM2D_VG_CMDSHIFT_USRINV22) & ITEM2D_VG_CMDMASK_USRINV22;

    /* Set image object ID */
    iteM2dHardwareGenObjectID();
    hwReg.REG_BID2_BASE = iteM2dHardwareGetCurrentObjectID();

    if (srcSurface)
    {
        srcSurface->bitbltId2 = hwReg.REG_BID2_BASE;
    }
    destSurface->bitbltId2 = hwReg.REG_BID2_BASE;

    /* Set HW to cmdq */
    iteM2dHardwareFire(&hwReg);
}

MMP_RESULT
m2dGradientFill(
    M2D_SURFACE          *destDisplay,
    MMP_UINT             destX,
    MMP_UINT             destY,
    MMP_UINT             destWidth,
    MMP_UINT             destHeight,
    MMP_M2D_COLOR        initColor,
    MMP_M2D_COLOR        endColor,
    MMP_M2D_GF_DIRECTION direction)
{
    ITEM2Dfloat            coords[8]            = {0};
    ITEM2Duint8            segments[]           = { M2DVG_MOVE_TO_ABS, M2DVG_LINE_TO_ABS, M2DVG_LINE_TO_ABS, M2DVG_LINE_TO_ABS, M2DVG_CLOSE_PATH };
    ITEM2DPath             p                    = {0};
    ITEM2Dboolean          hwWaitObjID          = ITEM2D_FALSE;
    ITEM2DHardwareRegister hwReg                = {0};
    ITEM2Dboolean          srcPreMultiFormat    = ITEM2D_FALSE;
    ITEM2Dboolean          dstPreMultiFormat    = ITEM2D_FALSE;
    ITEM2Dboolean          EnPreMultiForTexture = ITEM2D_FALSE; // Reg 0x0AC, Bit 28
    ITEM2Dboolean          EnUnpreMultiForDst   = ITEM2D_FALSE; // Reg 0x0AC, Bit 31
    HWMatrix3x3            transMat             = {0};
    HWImageFormat          destImageFormat      = HW_sRGB_565;
    HWImageFormat          srcImageFormat       = HW_sRGB_565;
    ITEM2Dboolean          enScissor            = ITEM2D_FALSE;
    ITEM2DImage            *hwCoverageImage     = NULL;
    ITEM2DImage            *hwValidImage        = NULL;
    ITEM2Duint8            *tessellateCmdBuffer = NULL;

    /* Get coverage image parameter */
    if (vgSurface->coverageIndex)
    {
        hwCoverageImage          = vgSurface->coverageImageB;
        hwValidImage             = vgSurface->validImageB;
        vgSurface->coverageIndex = 0;
    }
    else
    {
        hwCoverageImage          = vgSurface->coverageImageA;
        hwValidImage             = vgSurface->validImageA;
        vgSurface->coverageIndex = 1;
    }

    ITEM2D_INITOBJ(ITEM2DHardware, hwReg);

    M2DSETMAT(transMat,
              0x10000, 0,       0,
              0,       0x10000, 0,
              0,       0,       0x10000);

    coords[0]   = (ITEM2Dfloat)destX;
    coords[1]   = (ITEM2Dfloat)destY;
    coords[2]   = (ITEM2Dfloat)destX + destWidth;
    coords[3]   = (ITEM2Dfloat)destY;
    coords[4]   = (ITEM2Dfloat)destX + destWidth;
    coords[5]   = (ITEM2Dfloat)destY + destHeight;
    coords[6]   = (ITEM2Dfloat)destX;
    coords[7]   = (ITEM2Dfloat)destY + destHeight;

    p.format    = M2DVG_PATH_FORMAT_STANDARD;
    p.scale     = 1.0f;
    p.bias      = 0.0f;
    p.segHint   = 0;
    p.dataHint  = 0;
    p.datatype  = M2DVG_PATH_DATATYPE_F;
    p.caps      = M2DVG_PATH_CAPABILITY_ALL;
    p.segCount  = 5;
    p.segs      = segments;
    p.dataCount = 8;
    p.data      = coords;

    iteM2dFlattenPath(&p, 1, &p.pathCommand);

    hwReg.REG_TCR_BASE = ITEM2D_VG_CMD_TRANSFORM_EN |
                         ITEM2D_VG_CMD_JOINROUND |
                         ITEM2D_VG_CMD_READMEM |
                         ITEM2D_VG_CMD_TELWORK;

    hwReg.REG_PLR_BASE = p.pathCommand.size * sizeof(ITEM2Duint32);
    //tessellateCmdBuffer
    //allocate vram buffer

    if ( (p.pathCommand.size * sizeof(ITEM2Duint32)) <= ITEM2D_PATH_COPY_SIZE_THRESHOLD)
    {
        ITEM2Duint8  *mappedSysRam = NULL;
        ITEM2Duint32 allocSize     = p.pathCommand.size * sizeof(ITEM2Duint32);

        tessellateCmdBuffer = (ITEM2Duint8 *)m2dvgMemalign(4, allocSize, iteM2dHardwareGenObjectID());
        mappedSysRam        = ithMapVram((uint32_t)tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
        memcpy(mappedSysRam, p.pathCommand.items, allocSize);
        ithFlushDCacheRange(mappedSysRam, allocSize);
        ithUnmapVram(mappedSysRam, allocSize);
    }
    else
    {
#ifdef _WIN32
        ITEM2Duint8  *mappedSysRam = NULL;
        ITEM2Duint32 allocSize     = p.pathCommand.size * sizeof(ITEM2Duint32);

        tessellateCmdBuffer = (ITEM2Duint8 *)m2dvgMemalign(4, p.pathCommand.size * sizeof(ITEM2Duint32), iteM2dHardwareGenObjectID());
        mappedSysRam        = ithMapVram(tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
        memcpy(mappedSysRam, p.pathCommand.items, allocSize);
        ithFlushDCacheRange(mappedSysRam, allocSize);
        ithUnmapVram(mappedSysRam, allocSize);
#else
        tessellateCmdBuffer = (ITEM2Duint8 *)p.pathCommand.items;
#endif
        hwWaitObjID         = ITEM2D_TRUE;
    }

    hwReg.REG_PBR_BASE    = ((ITEM2Duint32)tessellateCmdBuffer) << ITEM2D_VG_CMDSHIFT_PATHBASE;

    hwReg.REG_UTR00_BASE  = (ITEM2Ds15p16)(1.0f * 0x10000);
    hwReg.REG_UTR01_BASE  = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_UTR02_BASE  = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_UTR10_BASE  = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_UTR11_BASE  = (ITEM2Ds15p16)(1.0f * 0x10000);
    hwReg.REG_UTR12_BASE  = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_UTR20_BASE  = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_UTR21_BASE  = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_UTR22_BASE  = (ITEM2Ds15p16)(1.0f * 0x10000);

    // User Inverse Transform
    hwReg.REG_UITR00_BASE = (transMat.m[0][0] << ITEM2D_VG_CMDSHIFT_USRINV00) & ITEM2D_VG_CMDMASK_USRINV00;
    hwReg.REG_UITR01_BASE = (transMat.m[0][1] << ITEM2D_VG_CMDSHIFT_USRINV01) & ITEM2D_VG_CMDMASK_USRINV01;
    hwReg.REG_UITR02_BASE = (transMat.m[0][2] << ITEM2D_VG_CMDSHIFT_USRINV02) & ITEM2D_VG_CMDMASK_USRINV02;
    hwReg.REG_UITR10_BASE = (transMat.m[1][0] << ITEM2D_VG_CMDSHIFT_USRINV10) & ITEM2D_VG_CMDMASK_USRINV10;
    hwReg.REG_UITR11_BASE = (transMat.m[1][1] << ITEM2D_VG_CMDSHIFT_USRINV11) & ITEM2D_VG_CMDMASK_USRINV11;
    hwReg.REG_UITR12_BASE = (transMat.m[1][2] << ITEM2D_VG_CMDSHIFT_USRINV12) & ITEM2D_VG_CMDMASK_USRINV12;
    hwReg.REG_UITR20_BASE = (transMat.m[2][0] << ITEM2D_VG_CMDSHIFT_USRINV20) & ITEM2D_VG_CMDMASK_USRINV20;
    hwReg.REG_UITR21_BASE = (transMat.m[2][1] << ITEM2D_VG_CMDSHIFT_USRINV21) & ITEM2D_VG_CMDMASK_USRINV21;
    hwReg.REG_UITR22_BASE = (transMat.m[2][2] << ITEM2D_VG_CMDSHIFT_USRINV22) & ITEM2D_VG_CMDMASK_USRINV22;

    /* Set clipping range */
    hwReg.REG_CCR_BASE    = ITEM2D_VG_CMD_CLIPPING |
                            ITEM2D_VG_CMD_RENDERQ_BETTER |
                            ITEM2D_VG_CMD_PLANFORMAT_TWOBYTES;

    hwReg.REG_CPBR_BASE  = ((ITEM2Duint32)hwCoverageImage->data) >> ITEM2D_VG_CMDSHIFT_PLANBASE;
    hwReg.REG_CVPPR_BASE = (hwValidImage->pitch << ITEM2D_VG_CMDSHIFT_VALIDPITCH) |
                           (hwCoverageImage->pitch << ITEM2D_VG_CMDSHIFT_PLANPITCH);
    hwReg.REG_VPBR_BASE  = ((ITEM2Duint32)hwValidImage->data) << ITEM2D_VG_CMDSHIFT_VALIDBASE;

    if (((M2D_SURFACE *)destDisplay)->clipSet.clipRegionCount == 1)
    {
        hwReg.REG_PXCR_BASE = ((((M2D_SURFACE *)destDisplay)->clipSet.clipRegion->right) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND) | (((M2D_SURFACE *)destDisplay)->clipSet.clipRegion->left);
        hwReg.REG_PYCR_BASE = ((((M2D_SURFACE *)destDisplay)->clipSet.clipRegion->bottom) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND) | (((M2D_SURFACE *)destDisplay)->clipSet.clipRegion->top);
        //printf("m2dGradientFill right = %d,left = %d,bottom = %d,top = %d\n",((M2D_SURFACE*)destDisplay)->clipSet.clipRegion->right,((M2D_SURFACE*)destDisplay)->clipSet.clipRegion->left,((M2D_SURFACE*)destDisplay)->clipSet.clipRegion->bottom,((M2D_SURFACE*)destDisplay)->clipSet.clipRegion->top);
    }
    else
    {
        hwReg.REG_PXCR_BASE = (hwCoverageImage->width - 1) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND;
        hwReg.REG_PYCR_BASE = (hwCoverageImage->height - 1) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND;
    }

    hwReg.REG_ACR_BASE |= ITEM2D_VG_CMD_CLRCOVCACHE | ITEM2D_VG_CMD_CLRRDRCACHE;

    /* Setup render engine */
    // Render Control Register
    hwReg.REG_RCR_BASE = ITEM2D_VG_CMD_DST_PRE_EN |
                         ITEM2D_VG_CMD_SRC_NONPRE_EN |
                         ITEM2D_VG_CMD_SRC_PRE_EN |
                         ITEM2D_VG_CMD_DITHER_EN | // always enable dither
                         ITEM2D_VG_CMD_DESTINATION_EN |
                         ITEM2D_VG_CMD_TEXCACHE_EN |
                         ITEM2D_VG_CMD_TEXTURE_EN |
                         ((enScissor == ITEM2D_TRUE) ? ITEM2D_VG_CMD_SCISSOR_EN : 0) |
                         ITEM2D_VG_CMD_COVERAGE_EN |
                         ITEM2D_VG_CMD_FILLRULE_NONZERO |
                         ITEM2D_VG_CMD_IMAGEQ_FASTER |
                         ITEM2D_VG_CMD_RDPLN_VLD_EN |
                         ITEM2D_VG_CMD_RENDERMODE_0;

    hwReg.REG_RMR_BASE = ITEM2D_VG_CMD_AUTOSCAN_EN |
                         ITEM2D_VG_CMD_MASKMODE_INTERSECT |
                         ITEM2D_VG_CMD_GAMMAMODE_INVERSE |
                         ITEM2D_VG_CMD_BLENDMODE_SRC_OVER_DST |
                         ITEM2D_VG_CMD_PAINTTYPE_LINEAR |
                         ITEM2D_VG_CMD_RAMPMODE_PAD;

    /* Render format register */
    hwReg.REG_RFR_BASE &= 0x00FFFFFF;
    destImageFormat = _M2dImgFormat2HwImgFormat(destDisplay->imageFormat);

    hwReg.REG_RFR_BASE |= (destImageFormat << 8);

    // Destination Coordinate Register
    hwReg.REG_DCR_BASE  = (((ITEM2Ds12p3)0 << ITEM2D_VG_CMDSHIFT_DSTY) & ITEM2D_VG_CMDMASK_DSTY) |
                          ((ITEM2Ds12p3)0 & ITEM2D_VG_CMDMASK_DSTX);

    // Destination Height/Width Register
    hwReg.REG_DHWR_BASE = (((ITEM2Dint16)destDisplay->height << ITEM2D_VG_CMDSHIFT_DSTHEIGHT) & ITEM2D_VG_CMDMASK_DSTHEIGHT) |
                          ((ITEM2Dint16)destDisplay->width & ITEM2D_VG_CMDMASK_DSTWIDTH);

    // Destination Base Register
    hwReg.REG_DBR_BASE  = (ITEM2Duint32)(destDisplay->baseScanPtr) & ITEM2D_VG_CMDMASK_SRCBASE;

    // Src/Dst Pitch Register
    hwReg.REG_SDPR_BASE = (destDisplay->pitch & ITEM2D_VG_CMDMASK_DSTPITCH);

    {
        ITEM2Dfloat   R;
        ITEM2DVector2 u, v, w;

        switch (direction)
        {
        case MMP_M2D_GF_HORIZONTAL:
            u.x = (ITEM2Dfloat)destX;
            u.y = (ITEM2Dfloat)destY;
            v.x = (ITEM2Dfloat)destWidth + destX;
            v.y = (ITEM2Dfloat)destY;
            break;
        case MMP_M2D_GF_VERTICAL:
            u.x = (ITEM2Dfloat)destX;
            u.y = (ITEM2Dfloat)destY;
            v.x = (ITEM2Dfloat)destX;
            v.y = (ITEM2Dfloat)destHeight + destY;
            break;
        case MMP_M2D_GF_DIAGONAL_LT:
            u.x = (ITEM2Dfloat)destX;
            u.y = (ITEM2Dfloat)destY;
            v.x = (ITEM2Dfloat)destWidth + destX;
            v.y = (ITEM2Dfloat)destHeight + destY;
            break;
        case MMP_M2D_GF_DIAGONAL_RT:
            u.x = (ITEM2Dfloat)destWidth + destX;
            u.y = (ITEM2Dfloat)destY;
            v.x = (ITEM2Dfloat)destX;
            v.y = (ITEM2Dfloat)destHeight + destY;
            break;
        case MMP_M2D_GF_DIAGONAL_LB:
            u.x = (ITEM2Dfloat)destX;
            u.y = (ITEM2Dfloat)destHeight + destY;
            v.x = (ITEM2Dfloat)destWidth + destX;
            v.y = (ITEM2Dfloat)destY;
            break;
        case MMP_M2D_GF_DIAGONAL_RB:
            u.x = (ITEM2Dfloat)destWidth + destX;
            u.y = (ITEM2Dfloat)destHeight + destY;
            v.x = (ITEM2Dfloat)destX;
            v.y = (ITEM2Dfloat)destY;
            break;
        }

        SET2(w, v.x - u.x, v.y - u.y);
        R = DOT2(w, w);
        if (R <= 0.0f)
        {
            R = 1.0f;
        }
        R                   = 1.0f / R;

        hwReg.REG_GPRA_BASE = (ITEM2Ds15p16)(R * w.x * 0x10000);
        hwReg.REG_GPRB_BASE = (ITEM2Ds15p16)(R * w.y * 0x10000);
        hwReg.REG_GPRC_BASE = (ITEM2Ds15p16)(-1 * R * (w.x * u.x + w.y * u.y) * 0x10000);
    }

    hwReg.REG_CTSR0_BASE  = (((ITEM2Dint16)(0 * 0x100) << ITEM2D_VG_CMDSHIFT_SCOLXFM00) & ITEM2D_VG_CMDMASK_SCOLXFM00) |
                            (ITEM2Dint16)(1 * 0x100);

    hwReg.REG_CTSR1_BASE  = (((ITEM2Dint16)(0 * 0x100) << ITEM2D_VG_CMDSHIFT_SCOLXFM00) & ITEM2D_VG_CMDMASK_SCOLXFM00) |
                            (ITEM2Dint16)(1 * 0x100);

    hwReg.REG_PITR00_BASE = (ITEM2Ds15p16)(1 * 0x10000);
    hwReg.REG_PITR01_BASE = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_PITR02_BASE = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_PITR10_BASE = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_PITR11_BASE = (ITEM2Ds15p16)(1 * 0x10000);
    hwReg.REG_PITR12_BASE = (ITEM2Ds15p16)(0 * 0x10000);

    /* Fill gradient parameter */
    {
        ITEM2Dfloat   lastOffset        = 0.0f;
        ITEM2Dint32   itemIndex         = 0;
        ITEM2Dint32   startIndex        = 0;
        ITEM2Dboolean rampEnd           = ITEM2D_FALSE;
        ITEM2Dboolean even              = ITEM2D_TRUE;
        ITEM2Duint32  *currRampStopReg  = &hwReg.REG_RSR01_BASE;
        ITEM2Duint32  *currRampColorReg = &hwReg.REG_RCR00_BASE;
        ITEM2Duint32  *currDividerReg   = &hwReg.REG_RDR01_BASE;

        /* Disable all ramp stop registers */
        for (itemIndex = 0; itemIndex < 8; itemIndex++)
        {
            *currRampStopReg = 0;
            currRampStopReg++;
        }

        /* Restore */
        currRampStopReg  = &hwReg.REG_RSR01_BASE;
        currRampColorReg = &hwReg.REG_RCR00_BASE;
        currDividerReg   = &hwReg.REG_RDR01_BASE;

        /* Fill first 8 ramp stop value */
        for (startIndex = 0, even = ITEM2D_TRUE, itemIndex = 0;
             itemIndex < 2;
             itemIndex++, even = !even)
        {
            if (rampEnd == ITEM2D_TRUE)
            {
                rampEnd    = ITEM2D_FALSE;
                startIndex = itemIndex;
            }

            /* Offset */
            if (even)
            {
                *currRampStopReg = ((ITEM2Duint32)(0.0f * (1 << 12))) | ITEM2D_VG_CMD_RAMPSTOP0VLD;

                if (   (startIndex == itemIndex)
                       && (currRampStopReg == &hwReg.REG_RSR01_BASE) )
                {
                    *currRampStopReg |= ITEM2D_VG_CMD_RAMPSTOP0EQ;
                }
            }
            else
            {
                *currRampStopReg |= (((ITEM2Duint32)(1.0f * (1 << 12))) << ITEM2D_VG_CMDSHIFT_RAMPSTOP1) | ITEM2D_VG_CMD_RAMPSTOP1VLD;
                currRampStopReg++;
            }

            /* Color */

            if (itemIndex == 0)
            {
                *currRampColorReg = (((initColor & 0x0000001F) * 255) / 31) << 20 | (0xFF);
                currRampColorReg++;
                *currRampColorReg = ((((initColor & 0x0000F800) >> 11) * 255) / 31) << 20 | (((((initColor & 0x000007E0) >> 5) * 255) / 63) << 4);
                currRampColorReg++;
            }
            else
            {
                *currRampColorReg = (((endColor & 0x0000001F) * 255) / 31) << 20 | (0xFF);
                currRampColorReg++;
                *currRampColorReg = ((((endColor & 0x0000F800) >> 11) * 255) / 31) << 20 | (((((endColor & 0x000007E0) >> 5) * 255) / 63) << 4);
            }

            /* Divider */
            if (itemIndex > startIndex)
            {
                ITEM2Ds15p16 delta = (ITEM2Ds15p16)((1.0f - lastOffset) * (1 << 12));

                if (delta)
                {
                    *currDividerReg = ((ITEM2Ds15p16)(1 << 24)) / delta;
                }
                else
                {
                    *currDividerReg = 0;
                }
                currDividerReg++;
            }

            lastOffset = 0.0f;
        }
    }

    /* Set image object ID */
    if (hwWaitObjID == ITEM2D_TRUE)
    {
        iteM2dHardwareGenObjectID();
    }
    hwReg.REG_BID2_BASE    = iteM2dHardwareGetCurrentObjectID();
    destDisplay->bitbltId2 = hwReg.REG_BID2_BASE;

    iteM2dHardwareFire(&hwReg);

    if (hwWaitObjID == ITEM2D_TRUE)
    {
        iteM2dHardwareWaitObjID(iteM2dHardwareGetCurrentObjectID());
    }

    free(p.pathCommand.items);
}

static void
iteM2dGenDashParamter(
    float        dashPhase,             /* IN */
    float        *contextDashArray,     /* IN */
    ITEM2Duint32 contextDashArraySize,
    ITEM2Duint32 hwDashArraySize,       /* IN */
    ITEM2Ds15p16 *hwDashArray,          /* OUT */
    ITEM2Duint32 *hwDashArrayUsedSize)  /* OUT */
{
    if (contextDashArraySize != 0)
    {
        int i = 0, /*j=0,*/ phstart = 0, phfirst = 1, OtherSize = 0;

        *hwDashArrayUsedSize = 0;

        for (i = 0; i < contextDashArraySize; i++)
        {
            if (dashPhase <= contextDashArray[i] || phstart)
            {
                if (phfirst)
                {
                    if (i & 1)
                    {
                        hwDashArray[*hwDashArrayUsedSize] = (ITEM2Ds15p16)((contextDashArray[i] - dashPhase) * (1 << POINTPREC)); // s12.11
                    }
                    else
                    {
                        hwDashArray[*hwDashArrayUsedSize] = (ITEM2Ds15p16)((contextDashArray[i] - dashPhase) * (1 << POINTPREC) + (1 << (POINTPREC + 12)));   // s12.11
                    }
                }
                else
                {
                    if (i & 1)
                    {
                        hwDashArray[*hwDashArrayUsedSize] = (ITEM2Ds15p16)(contextDashArray[i] * (1 << POINTPREC) );    // s12.11
                    }
                    else
                    {
                        hwDashArray[*hwDashArrayUsedSize] = (ITEM2Ds15p16)(contextDashArray[i] * (1 << POINTPREC) + (1 << (POINTPREC + 12)));   // s12.11
                    }
                }
                phstart = 1;
                phfirst = 0;
                (*hwDashArrayUsedSize)++;

                if (*hwDashArrayUsedSize >= hwDashArraySize)
                {
                    return;
                }
            }
            else
            {
                dashPhase -= contextDashArray[i];
            }
        }

        OtherSize = contextDashArraySize - (*hwDashArrayUsedSize) + 1;

        for (i = 0; i < OtherSize; i++)
        {
            if (i & 1)
            {
                hwDashArray[*hwDashArrayUsedSize] = (ITEM2Ds15p16)(contextDashArray[i] * (1 << POINTPREC)); // s12.11
            }
            else
            {
                hwDashArray[*hwDashArrayUsedSize] = (ITEM2Ds15p16)(contextDashArray[i] * (1 << POINTPREC) + (1 << (POINTPREC + 12)));   // s12.11
            }

            *hwDashArrayUsedSize++;

            if (*hwDashArrayUsedSize >= hwDashArraySize)
            {
                return;
            }
        }
    }
    else
    {
        memset(hwDashArray, 0x00, sizeof(float) * hwDashArraySize);
    }
}

MMP_RESULT
m2dDrawLine(
    M2D_SURFACE       *destDisplay,
    MMP_UINT          startX,
    MMP_UINT          startY,
    MMP_UINT          endX,
    MMP_UINT          endY,
    MMP_M2D_COLOR     color,
    MMP_M2D_PEN_STYLE style,
    MMP_UINT          lineWidth)
{
    ITEM2Dfloat            coords[4]            = {0};
    ITEM2Duint8            segments[]           = {M2DVG_MOVE_TO_ABS, M2DVG_LINE_TO_ABS};
    ITEM2DPath             p                    = {0};
    ITEM2Dboolean          hwWaitObjID          = ITEM2D_FALSE;
    ITEM2DHardwareRegister hwReg                = {0};
    ITEM2Dboolean          srcPreMultiFormat    = ITEM2D_FALSE;
    ITEM2Dboolean          dstPreMultiFormat    = ITEM2D_FALSE;
    ITEM2Dboolean          EnPreMultiForTexture = ITEM2D_FALSE; // Reg 0x0AC, Bit 28
    ITEM2Dboolean          EnUnpreMultiForDst   = ITEM2D_FALSE; // Reg 0x0AC, Bit 31
    HWMatrix3x3            transMat             = {0};
    HWImageFormat          destImageFormat      = HW_sRGB_565;
    HWImageFormat          srcImageFormat       = HW_sRGB_565;
    ITEM2Dboolean          enScissor            = ITEM2D_FALSE;
    ITEM2Dint              minterLength         = 100 * 16;
    ITEM2DImage            *hwCoverageImage     = NULL;
    ITEM2DImage            *hwValidImage        = NULL;
    ITEM2Duint8            *tessellateCmdBuffer = NULL;
    MMP_UINT               destWidth            = 0;
    MMP_UINT               destHeight           = 0;
    MMP_UINT               destX                = 0;
    MMP_UINT               destY                = 0;
    MMP_M2D_COLOR          fgColor;

    if (startX == endX || startY == endY)
    {
        if (startX == endX)
        {
            destWidth = lineWidth;
            if (startY > endY)
            {
                destHeight = (startY - endY);
                destX      = endX;
                destY      = endY;
            }
            else
            {
                destHeight = (endY - startY);
                destX      = startX;
                destY      = startY;
            }
        }
        else
        {
            destHeight = lineWidth;
            if (startX > endX)
            {
                destWidth = (startX - endX);
                destX     = endX;
                destY     = endY;
            }
            else
            {
                destWidth = (endX - startX);
                destX     = startX;
                destY     = startY;
            }
        }

        fgColor                                            = destDisplay->brush->realizedBrush->foregroundColor;
        destDisplay->brush->realizedBrush->foregroundColor = color;

        m2dTransferBlock(
            destDisplay,
            destX,
            destY,
            destWidth,
            destHeight,
            MMP_NULL,
            0,
            0,
            0xF0);

        destDisplay->brush->realizedBrush->foregroundColor = fgColor;
        return 0;
    }

    /* Get coverage image parameter */
    if (vgSurface->coverageIndex)
    {
        hwCoverageImage          = vgSurface->coverageImageB;
        hwValidImage             = vgSurface->validImageB;
        vgSurface->coverageIndex = 0;
    }
    else
    {
        hwCoverageImage          = vgSurface->coverageImageA;
        hwValidImage             = vgSurface->validImageA;
        vgSurface->coverageIndex = 1;
    }

    ITEM2D_INITOBJ(ITEM2DHardware, hwReg);

    M2DSETMAT(transMat,
              0x10000, 0,       0,
              0,       0x10000, 0,
              0,       0,       0x10000);

    coords[0]   = (ITEM2Dfloat)startX;
    coords[1]   = (ITEM2Dfloat)startY;
    coords[2]   = (ITEM2Dfloat)endX;
    coords[3]   = (ITEM2Dfloat)endY;

    p.format    = M2DVG_PATH_FORMAT_STANDARD;
    p.scale     = 1.0f;
    p.bias      = 0.0f;
    p.segHint   = 0;
    p.dataHint  = 0;
    p.datatype  = M2DVG_PATH_DATATYPE_F;
    p.caps      = M2DVG_PATH_CAPABILITY_ALL;
    p.segCount  = 2;
    p.segs      = segments;
    p.dataCount = 4;
    p.data      = coords;

    iteM2dFlattenPath(&p, 1, &p.pathCommand);

    hwReg.REG_TCR_BASE = ITEM2D_VG_CMD_CAPTYPE_BUTT |
                         ITEM2D_VG_CMD_JOINTYPE_MITER |
                         ITEM2D_VG_CMD_TRANSFORM_EN |
                         ITEM2D_VG_CMD_STROKEPATH |
                         ITEM2D_VG_CMD_READMEM |
                         ITEM2D_VG_CMD_TELWORK;

    hwReg.REG_LWR_BASE = ((ITEM2Duint32)(lineWidth * (1 << 12)) & ITEM2D_VG_CMDMASK_LINEWIDTH);

    {
        ITEM2Ds15p16 hwDashArray[ITEM2D_MAX_DASH_COUNT] = {0};
        ITEM2Duint32 hwDashArrayUsedSize                = 0;
        ITEM2Duint32 dashIndex                          = 0;
        ITEM2Duint32 *dprBase                           = &hwReg.REG_DPR00_BASE;
        ITEM2Duint32 dashLength                         = 0;
        float        strokeDashPattern[16]              = {0};

        switch (style)
        {
        case MMP_M2D_PEN_STYLE_SOLID:
            dashLength           = 0;
            break;

        case MMP_M2D_PEN_STYLE_DASH:
            dashLength           = 2;
            strokeDashPattern[0] = 8 * lineWidth;
            strokeDashPattern[1] = 8 * lineWidth;
            break;

        case MMP_M2D_PEN_STYLE_DOT:
            dashLength           = 2;
            strokeDashPattern[0] = 2 * lineWidth;
            strokeDashPattern[1] = 2 * lineWidth;
            break;

        case MMP_M2D_PEN_STYLE_DASH_DOT:
            dashLength           = 4;
            strokeDashPattern[0] = 8 * lineWidth;
            strokeDashPattern[1] = 4 * lineWidth;
            strokeDashPattern[2] = 2 * lineWidth;
            strokeDashPattern[3] = 4 * lineWidth;
            break;

        case MMP_M2D_PEN_STYLE_DASH_DOT_DOT:
            dashLength           = 6;
            strokeDashPattern[0] = 8 * lineWidth;
            strokeDashPattern[1] = 4 * lineWidth;
            strokeDashPattern[2] = 2 * lineWidth;
            strokeDashPattern[3] = 4 * lineWidth;
            strokeDashPattern[4] = 2 * lineWidth;
            strokeDashPattern[5] = 4 * lineWidth;
            break;
        }

        iteM2dGenDashParamter(
            0,
            strokeDashPattern,
            dashLength,
            ITEM2D_MAX_DASH_COUNT,
            hwDashArray,
            &hwDashArrayUsedSize);

        hwReg.REG_TCR_BASE |= (hwDashArrayUsedSize << ITEM2D_VG_CMDSHIFT_DASHCOUNT) & ITEM2D_VG_CMDMASK_DASHCOUNT;

        hwReg.REG_SRNR_BASE = (((ITEM2Duint32)(minterLength * lineWidth * (1 << 8) + 0.5f) << ITEM2D_VG_CMDSHIFT_MITERLIMIT) & ITEM2D_VG_CMDMASK_MITERLIMIT) |
                              ITEM2D_MAX_STROKE_DIVIDE_NUMBER;

        for (dashIndex = 0; (dashIndex <= hwDashArrayUsedSize) && (dashIndex < ITEM2D_MAX_DASH_COUNT); dashIndex++)
        {
            *dprBase = hwDashArray[dashIndex];
            dprBase++;
        }
    }

    hwReg.REG_PLR_BASE = p.pathCommand.size * sizeof(ITEM2Duint32);

    if ( (p.pathCommand.size * sizeof(ITEM2Duint32)) <= ITEM2D_PATH_COPY_SIZE_THRESHOLD)
    {
        ITEM2Duint8  *mappedSysRam = NULL;
        ITEM2Duint32 allocSize     = p.pathCommand.size * sizeof(ITEM2Duint32);

        tessellateCmdBuffer = (ITEM2Duint8 *)m2dvgMemalign(4, allocSize, iteM2dHardwareGenObjectID());
        mappedSysRam        = ithMapVram((uint32_t)tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
        memcpy(mappedSysRam, p.pathCommand.items, allocSize);
        ithFlushDCacheRange(mappedSysRam, allocSize);
        ithUnmapVram(mappedSysRam, allocSize);
    }
    else
    {
#ifdef _WIN32
        ITEM2Duint8  *mappedSysRam = NULL;
        ITEM2Duint32 allocSize     = p.pathCommand.size * sizeof(ITEM2Duint32);

        tessellateCmdBuffer = (ITEM2Duint8 *)m2dvgMemalign(4, p.pathCommand.size * sizeof(ITEM2Duint32), iteM2dHardwareGenObjectID());
        mappedSysRam        = ithMapVram(tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
        memcpy(mappedSysRam, p.pathCommand.items, allocSize);
        ithFlushDCacheRange(mappedSysRam, allocSize);
        ithUnmapVram(mappedSysRam, allocSize);
#else
        tessellateCmdBuffer = (ITEM2Duint8 *)p.pathCommand.items;
#endif
        hwWaitObjID         = ITEM2D_TRUE;
    }

    hwReg.REG_PBR_BASE    = ((ITEM2Duint32)tessellateCmdBuffer) << ITEM2D_VG_CMDSHIFT_PATHBASE;

    hwReg.REG_UTR00_BASE  = (ITEM2Ds15p16)(1.0f * 0x10000);
    hwReg.REG_UTR01_BASE  = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_UTR02_BASE  = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_UTR10_BASE  = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_UTR11_BASE  = (ITEM2Ds15p16)(1.0f * 0x10000);
    hwReg.REG_UTR12_BASE  = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_UTR20_BASE  = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_UTR21_BASE  = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_UTR22_BASE  = (ITEM2Ds15p16)(1.0f * 0x10000);

    // User Inverse Transform
    hwReg.REG_UITR00_BASE = (transMat.m[0][0] << ITEM2D_VG_CMDSHIFT_USRINV00) & ITEM2D_VG_CMDMASK_USRINV00;
    hwReg.REG_UITR01_BASE = (transMat.m[0][1] << ITEM2D_VG_CMDSHIFT_USRINV01) & ITEM2D_VG_CMDMASK_USRINV01;
    hwReg.REG_UITR02_BASE = (transMat.m[0][2] << ITEM2D_VG_CMDSHIFT_USRINV02) & ITEM2D_VG_CMDMASK_USRINV02;
    hwReg.REG_UITR10_BASE = (transMat.m[1][0] << ITEM2D_VG_CMDSHIFT_USRINV10) & ITEM2D_VG_CMDMASK_USRINV10;
    hwReg.REG_UITR11_BASE = (transMat.m[1][1] << ITEM2D_VG_CMDSHIFT_USRINV11) & ITEM2D_VG_CMDMASK_USRINV11;
    hwReg.REG_UITR12_BASE = (transMat.m[1][2] << ITEM2D_VG_CMDSHIFT_USRINV12) & ITEM2D_VG_CMDMASK_USRINV12;
    hwReg.REG_UITR20_BASE = (transMat.m[2][0] << ITEM2D_VG_CMDSHIFT_USRINV20) & ITEM2D_VG_CMDMASK_USRINV20;
    hwReg.REG_UITR21_BASE = (transMat.m[2][1] << ITEM2D_VG_CMDSHIFT_USRINV21) & ITEM2D_VG_CMDMASK_USRINV21;
    hwReg.REG_UITR22_BASE = (transMat.m[2][2] << ITEM2D_VG_CMDSHIFT_USRINV22) & ITEM2D_VG_CMDMASK_USRINV22;

    /* Set clipping range */
    hwReg.REG_CCR_BASE    = ITEM2D_VG_CMD_CLIPPING |
                            ITEM2D_VG_CMD_RENDERQ_BETTER |
                            ITEM2D_VG_CMD_PLANFORMAT_TWOBYTES |
                            ITEM2D_VG_CMD_FULLRDY_EN |
                            ITEM2D_VG_CMD_TIMERDY_EN;

    hwReg.REG_CPBR_BASE  = ((ITEM2Duint32)hwCoverageImage->data) >> ITEM2D_VG_CMDSHIFT_PLANBASE;
    hwReg.REG_CVPPR_BASE = (hwValidImage->pitch << ITEM2D_VG_CMDSHIFT_VALIDPITCH) |
                           (hwCoverageImage->pitch << ITEM2D_VG_CMDSHIFT_PLANPITCH);
    hwReg.REG_VPBR_BASE  = ((ITEM2Duint32)hwValidImage->data) << ITEM2D_VG_CMDSHIFT_VALIDBASE;
    if (((M2D_SURFACE *)destDisplay)->clipSet.clipRegionCount == 1)
    {
        hwReg.REG_PXCR_BASE = ((((M2D_SURFACE *)destDisplay)->clipSet.clipRegion->right) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND) | (((M2D_SURFACE *)destDisplay)->clipSet.clipRegion->left);
        hwReg.REG_PYCR_BASE = ((((M2D_SURFACE *)destDisplay)->clipSet.clipRegion->bottom) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND) | (((M2D_SURFACE *)destDisplay)->clipSet.clipRegion->top);
    }
    else
    {
        hwReg.REG_PXCR_BASE = (hwCoverageImage->width - 1) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND;
        hwReg.REG_PYCR_BASE = (hwCoverageImage->height - 1) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND;
    }
    hwReg.REG_ACR_BASE |= ITEM2D_VG_CMD_CLRCOVCACHE | ITEM2D_VG_CMD_CLRRDRCACHE;

    /* Setup render engine */
    // Render Control Register
    hwReg.REG_RCR_BASE = ITEM2D_VG_CMD_DST_PRE_EN |
                         ITEM2D_VG_CMD_SRC_NONPRE_EN |
                         ITEM2D_VG_CMD_SRC_PRE_EN |
                         ITEM2D_VG_CMD_DITHER_EN | // always enable dither
                         ITEM2D_VG_CMD_DESTINATION_EN |
                         ITEM2D_VG_CMD_TEXCACHE_EN |
                         ITEM2D_VG_CMD_TEXTURE_EN |
                         ((enScissor == ITEM2D_TRUE) ? ITEM2D_VG_CMD_SCISSOR_EN : 0) |
                         ITEM2D_VG_CMD_COVERAGE_EN |
                         ITEM2D_VG_CMD_FILLRULE_NONZERO |
                         ITEM2D_VG_CMD_IMAGEQ_FASTER |
                         ITEM2D_VG_CMD_RDPLN_VLD_EN |
                         ITEM2D_VG_CMD_RENDERMODE_0;

    hwReg.REG_RMR_BASE = ITEM2D_VG_CMD_AUTOSCAN_EN |
                         ITEM2D_VG_CMD_BLENDMODE_SRC_OVER_DST |
                         ITEM2D_VG_CMD_GAMMAMODE_INVERSE;

    /* Render format register */
    hwReg.REG_RFR_BASE &= 0x00FFFFFF;
    destImageFormat = _M2dImgFormat2HwImgFormat(destDisplay->imageFormat);

    hwReg.REG_RFR_BASE |= (destImageFormat << 8);

    // Destination Coordinate Register
    hwReg.REG_DCR_BASE  = (((ITEM2Ds12p3)0 << ITEM2D_VG_CMDSHIFT_DSTY) & ITEM2D_VG_CMDMASK_DSTY) |
                          ((ITEM2Ds12p3)0 & ITEM2D_VG_CMDMASK_DSTX);

    // Destination Height/Width Register
    hwReg.REG_DHWR_BASE = (((ITEM2Dint16)destDisplay->height << ITEM2D_VG_CMDSHIFT_DSTHEIGHT) & ITEM2D_VG_CMDMASK_DSTHEIGHT) |
                          ((ITEM2Dint16)destDisplay->width & ITEM2D_VG_CMDMASK_DSTWIDTH);

    // Destination Base Register
    hwReg.REG_DBR_BASE   = (ITEM2Duint32)(destDisplay->baseScanPtr) & ITEM2D_VG_CMDMASK_SRCBASE;

    // Src/Dst Pitch Register
    hwReg.REG_SDPR_BASE  = (destDisplay->pitch & ITEM2D_VG_CMDMASK_DSTPITCH);

    hwReg.REG_RCR00_BASE = (((color & 0x0000001F) * 255) / 31) << 20 | (0xFF);
    hwReg.REG_RCR01_BASE = ((((color & 0x0000F800) >> 11) * 255) / 31) << 20 | (((((color & 0x000007E0) >> 5) * 255) / 63) << 4);

    hwReg.REG_CTBR0_BASE = (ITEM2Dint32)(0 * 0x100);
    hwReg.REG_CTBR1_BASE = (ITEM2Dint32)(0 * 0x100);
    hwReg.REG_CTBR2_BASE = (ITEM2Dint32)(0 * 0x100);
    hwReg.REG_CTBR3_BASE = (ITEM2Dint32)(0 * 0x100);

    hwReg.REG_CTSR0_BASE = (((ITEM2Dint16)(0 * 0x100) << ITEM2D_VG_CMDSHIFT_SCOLXFM00) & ITEM2D_VG_CMDMASK_SCOLXFM00) |
                           (ITEM2Dint16)(1 * 0x100);

    hwReg.REG_CTSR1_BASE = (((ITEM2Dint16)(0 * 0x100) << ITEM2D_VG_CMDSHIFT_SCOLXFM00) & ITEM2D_VG_CMDMASK_SCOLXFM00) |
                           (ITEM2Dint16)(1 * 0x100);

    /* Set image object ID */
    if (hwWaitObjID == ITEM2D_TRUE)
    {
        iteM2dHardwareGenObjectID();
    }
    hwReg.REG_BID2_BASE    = iteM2dHardwareGetCurrentObjectID();
    destDisplay->bitbltId2 = hwReg.REG_BID2_BASE;

    iteM2dHardwareFire(&hwReg);

    if (hwWaitObjID == ITEM2D_TRUE)
    {
        iteM2dHardwareWaitObjID(iteM2dHardwareGetCurrentObjectID());
    }

    free(p.pathCommand.items);
}

void
m2dSetImage(
    M2D_SURFACE *im,
    ITEM2Dint   x,
    ITEM2Dint   y,
    ITEM2Dint   width,
    ITEM2Dint   height,
    ITEM2DColor color)
{
    ITEM2DHardwareRegister hwReg                = {0};
    ITEM2Dboolean          srcPreMultiFormat    = ITEM2D_FALSE;
    ITEM2Dboolean          dstPreMultiFormat    = ITEM2D_FALSE;
    ITEM2Dboolean          EnPreMultiForTexture = ITEM2D_FALSE; // Reg 0x0AC, Bit 28
    ITEM2Dboolean          EnUnpreMultiForDst   = ITEM2D_FALSE; // Reg 0x0AC, Bit 31
    HWImageFormat          destImageFormat      = HW_sRGB_565;
    HWImageFormat          srcImageFormat       = HW_sRGB_565;
    ITEM2Dboolean          enScissor            = ITEM2D_FALSE;

    ITEM2D_INITOBJ(ITEM2DHardware, hwReg);

    /* Disable tesslellation */
    hwReg.REG_TCR_BASE = 0;

    /* Disable coverage engine */
    hwReg.REG_RCR_BASE = 0;

    /* Setup render engine */
    // Render Control Register
    hwReg.REG_RCR_BASE = (EnUnpreMultiForDst ? ITEM2D_VG_CMD_DST_PRE_EN : 0) |
                         (EnUnpreMultiForDst ? ITEM2D_VG_CMD_TEX_PRE_EN : 0) |
                         ((enScissor == ITEM2D_TRUE) ? ITEM2D_VG_CMD_SCISSOR_EN : 0) |
                         ITEM2D_VG_CMD_DITHER_EN | // always enable dither
                         ITEM2D_VG_CMD_RENDERMODE_0;

    /* Set clipping range */
    hwReg.REG_CCR_BASE = ITEM2D_VG_CMD_CLIPPING |
                         ITEM2D_VG_CMD_FULLRDY_EN |
                         ITEM2D_VG_CMD_TIMERDY_EN;

    if (((M2D_SURFACE *)im)->clipSet.clipRegionCount == 1)
    {
        hwReg.REG_PXCR_BASE = ((((M2D_SURFACE *)im)->clipSet.clipRegion->right) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND) | (((M2D_SURFACE *)im)->clipSet.clipRegion->left);
        hwReg.REG_PYCR_BASE = ((((M2D_SURFACE *)im)->clipSet.clipRegion->bottom) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND) | (((M2D_SURFACE *)im)->clipSet.clipRegion->top);
    }
    else
    {
        hwReg.REG_PXCR_BASE = (width - 1) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND;
        hwReg.REG_PYCR_BASE = (height - 1) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND;
    }

    hwReg.REG_RMR_BASE  = ITEM2D_VG_CMD_PAINTTYPE_COLOR;
    hwReg.REG_RFR_BASE &= 0x0000FF00;

    /* Render format register */
    destImageFormat = _M2dImgFormat2HwImgFormat(im->imageFormat);

    hwReg.REG_RFR_BASE  |= destImageFormat << 8;

    hwReg.REG_RCR00_BASE = (((ITEM2Ds8p4)(color.b << 4) << ITEM2D_VG_CMDSHIFT_RAMPCOLOR0B) & ITEM2D_VG_CMDMASK_RAMPCOLOR0B) |
                           (color.a & ITEM2D_VG_CMDMASK_RAMPCOLOR0A);

    hwReg.REG_RCR01_BASE = (((ITEM2Ds8p4)(color.r << 4) << ITEM2D_VG_CMDSHIFT_RAMPCOLOR0R) & ITEM2D_VG_CMDMASK_RAMPCOLOR0R) |
                           ((ITEM2Ds8p4)(color.g << 4) & ITEM2D_VG_CMDMASK_RAMPCOLOR0G);

    // Destination Coordinate Register
    hwReg.REG_DCR_BASE  = (((ITEM2Ds12p3)y << ITEM2D_VG_CMDSHIFT_DSTY) & ITEM2D_VG_CMDMASK_DSTY) |
                          ((ITEM2Ds12p3)x & ITEM2D_VG_CMDMASK_DSTX);
    // Destination Height/Width Register
    hwReg.REG_DHWR_BASE = (((ITEM2Dint16)height << ITEM2D_VG_CMDSHIFT_DSTHEIGHT) & ITEM2D_VG_CMDMASK_DSTHEIGHT) |
                          ((ITEM2Dint16)width & ITEM2D_VG_CMDMASK_DSTWIDTH);
    // Destination Base Register
    hwReg.REG_DBR_BASE  = (ITEM2Duint32)(im->baseScanPtr) & ITEM2D_VG_CMDMASK_SRCBASE;
    // Src/Dst Pitch Register
    hwReg.REG_SDPR_BASE = (((ITEM2Dint16)im->pitch << ITEM2D_VG_CMDSHIFT_SRCPITCH0) & ITEM2D_VG_CMDMASK_SRCPITCH0) |
                          (im->pitch & ITEM2D_VG_CMDMASK_DSTPITCH);

    /* Set image object ID */
    iteM2dHardwareGenObjectID();
    hwReg.REG_BID2_BASE = iteM2dHardwareGetCurrentObjectID();
    im->bitbltId2       = hwReg.REG_BID2_BASE;

    /* Set HW to cmdq */
    iteM2dHardwareFire(&hwReg);
}

MMP_RESULT
m2dDrawBmpTextTtx2(
    M2D_SURFACE *destSurface,
    MMP_INT     destX,
    MMP_INT     destY,
    void        *srcPtr,
    MMP_UINT    srcWidth,
    MMP_UINT    srcHeight,
    MMP_UINT    srcPitch)
{
    ITEM2DHardwareRegister hwReg                 = {0};
    ITEM2Dboolean          srcPreMultiFormat     = ITEM2D_FALSE;
    ITEM2Dboolean          dstPreMultiFormat     = ITEM2D_FALSE;
    ITEM2Dboolean          EnPreMultiForTexture  = ITEM2D_FALSE;       // Reg 0x0AC, Bit 28
    ITEM2Dboolean          EnUnpreMultiForDst    = ITEM2D_FALSE;       // Reg 0x0AC, Bit 31
    HWImageFormat          destImageFormat       = HW_sRGB_565;
    HWImageFormat          srcImageFormat        = HW_sRGB_565;
    ITEM2Dboolean          enScissor             = ITEM2D_FALSE;
    ITEM2Duint32           hwRenderQuality       = ITEM2D_VG_CMD_RENDERQ_BETTER;
    ITEM2Duint32           hwImageQuality        = ITEM2D_VG_CMD_IMAGEQ_FASTER;
    ITEM2Duint32           hwFillRule            = ITEM2D_VG_CMD_FILLRULE_ODDEVEN;
    ITEM2Duint32           hwCoverageFormatBytes = ITEM2D_VG_CMD_PLANFORMAT_TWOBYTES;
    ITEM2Duint32           hwPaintType           = ITEM2D_VG_CMD_PAINTTYPE_COLOR;
    ITEM2DImage            *hwCoverageImage      = NULL;
    ITEM2DImage            *hwValidImage         = NULL;
    ITEM2DImage            srcImage              = {0};
    ITEM2DMatrix3x3        imageMatrix           = {0};
    ITEM2Dfloat            coords[5]             = {0};
    ITEM2Duint8            segments[]            = { M2DVG_MOVE_TO_ABS, M2DVG_HLINE_TO_REL, M2DVG_VLINE_TO_REL, M2DVG_HLINE_TO_REL, M2DVG_CLOSE_PATH };
    ITEM2DPath             p                     = {0};
    ITEM2Dboolean          hwWaitObjID           = ITEM2D_FALSE;
    ITEM2Duint8            *tessellateCmdBuffer  = NULL;

    srcImage.width    = srcWidth;
    srcImage.height   = srcHeight;
    srcImage.pitch    = srcPitch;
    srcImage.vgformat = ITEM2D_VG_CMD_SRCFORMAT_A_1;
    srcImage.data     = (ITEM2Duint8 *)srcPtr;

    coords[0]         = 0.0f;
    coords[1]         = 0.0f;
    coords[2]         = (ITEM2Dfloat)(srcImage.width);
    coords[3]         = (ITEM2Dfloat)(srcImage.height);
    coords[4]         = (ITEM2Dfloat)(-srcImage.width);

    p.format          = M2DVG_PATH_FORMAT_STANDARD;
    p.scale           = 1.0f;
    p.bias            = 0.0f;
    p.segHint         = 0;
    p.dataHint        = 0;
    p.datatype        = M2DVG_PATH_DATATYPE_F;
    p.caps            = M2DVG_PATH_CAPABILITY_ALL;
    p.segCount        = 5;
    p.segs            = segments;
    p.dataCount       = 5;
    p.data            = coords;

    ITEM2D_INITOBJ(ITEM2DHardware, hwReg);

    iteM2dFlattenPath(&p, 1, &p.pathCommand);

    hwReg.REG_PLR_BASE = p.pathCommand.size * sizeof(ITEM2Duint32);

    //tessellateCmdBuffer
    //allocate vram buffer

    if ( (p.pathCommand.size * sizeof(ITEM2Duint32)) <= ITEM2D_PATH_COPY_SIZE_THRESHOLD)
    {
        ITEM2Duint8  *mappedSysRam = NULL;
        ITEM2Duint32 allocSize     = p.pathCommand.size * sizeof(ITEM2Duint32);

        tessellateCmdBuffer = (ITEM2Duint8 *)m2dvgMemalign(4, allocSize, iteM2dHardwareGenObjectID());
        mappedSysRam        = ithMapVram((uint32_t)tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
        memcpy(mappedSysRam, p.pathCommand.items, allocSize);
        ithFlushDCacheRange(mappedSysRam, allocSize);
        ithUnmapVram(mappedSysRam, allocSize);
    }
    else
    {
#ifdef _WIN32
        ITEM2Duint8  *mappedSysRam = NULL;
        ITEM2Duint32 allocSize     = p.pathCommand.size * sizeof(ITEM2Duint32);

        tessellateCmdBuffer = (ITEM2Duint8 *)m2dvgMemalign(4, p.pathCommand.size * sizeof(ITEM2Duint32), iteM2dHardwareGenObjectID());
        mappedSysRam        = ithMapVram(tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
        memcpy(mappedSysRam, p.pathCommand.items, allocSize);
        ithFlushDCacheRange(mappedSysRam, allocSize);
        ithUnmapVram(mappedSysRam, allocSize);
#else
        tessellateCmdBuffer = (ITEM2Duint8 *)p.pathCommand.items;
#endif
        hwWaitObjID         = ITEM2D_TRUE;
    }

    hwReg.REG_PBR_BASE  = ((ITEM2Duint32)tessellateCmdBuffer) << ITEM2D_VG_CMDSHIFT_PATHBASE;
    hwReg.REG_BID2_BASE = iteM2dHardwareGetCurrentObjectID();

    /* Get coverage image parameter */
    if (vgSurface->coverageIndex)
    {
        hwCoverageImage          = vgSurface->coverageImageB;
        hwValidImage             = vgSurface->validImageB;
        vgSurface->coverageIndex = 0;
    }
    else
    {
        hwCoverageImage          = vgSurface->coverageImageA;
        hwValidImage             = vgSurface->validImageA;
        vgSurface->coverageIndex = 1;
    }

    {
        ITEM2DMatrix3x3 glyphUserToSurfaceMatrix;
        ITEM2DMatrix3x3 localTransformMatrix, tempMatrix;
        ITEM2DVector2   deltaVector;
        ITEM2DMatrix3x3 imageInvMatrix;

        IDMAT(glyphUserToSurfaceMatrix);
        deltaVector.x = destX;
        deltaVector.y = destY;
        IDMAT(localTransformMatrix);
        TRANSLATEMATL(localTransformMatrix, deltaVector.x, deltaVector.y);
        MULMATMAT(glyphUserToSurfaceMatrix, localTransformMatrix, tempMatrix);
        glyphUserToSurfaceMatrix = tempMatrix;
        /* force affinity */
        M2DSETMAT(glyphUserToSurfaceMatrix,
                  glyphUserToSurfaceMatrix.m[0][0], glyphUserToSurfaceMatrix.m[0][1], glyphUserToSurfaceMatrix.m[0][2],
                  glyphUserToSurfaceMatrix.m[1][0], glyphUserToSurfaceMatrix.m[1][1], glyphUserToSurfaceMatrix.m[1][2],
                  0, 0, 1);

        hwReg.REG_UTR00_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[0][0] * 0x10000);
        hwReg.REG_UTR01_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[0][1] * 0x10000);
        hwReg.REG_UTR02_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[0][2] * 0x10000);
        hwReg.REG_UTR10_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[1][0] * 0x10000);
        hwReg.REG_UTR11_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[1][1] * 0x10000);
        hwReg.REG_UTR12_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[1][2] * 0x10000);
        hwReg.REG_UTR20_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[2][0] * 0x10000);
        hwReg.REG_UTR21_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[2][1] * 0x10000);
        hwReg.REG_UTR22_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[2][2] * 0x10000);

        iteM2dInvertMatrix(&glyphUserToSurfaceMatrix, &imageInvMatrix);
        hwReg.REG_UITR00_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[0][0] * 0x10000);
        hwReg.REG_UITR01_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[0][1] * 0x10000);
        hwReg.REG_UITR02_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[0][2] * 0x10000);
        hwReg.REG_UITR10_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[1][0] * 0x10000);
        hwReg.REG_UITR11_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[1][1] * 0x10000);
        hwReg.REG_UITR12_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[1][2] * 0x10000);
        hwReg.REG_UITR20_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[2][0] * 0x10000);
        hwReg.REG_UITR21_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[2][1] * 0x10000);
        hwReg.REG_UITR22_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[2][2] * 0x10000);
    }

    hwReg.REG_TCR_BASE = ITEM2D_VG_CMD_TRANSFORM_EN |
                         ITEM2D_VG_CMD_READMEM |
                         ITEM2D_VG_CMD_TELWORK;

    hwReg.REG_CCR_BASE = ITEM2D_VG_CMD_CLIPPING |
                         hwRenderQuality |
                         hwCoverageFormatBytes |
                         ITEM2D_VG_CMD_FULLRDY_EN |
                         ITEM2D_VG_CMD_TIMERDY_EN;

    hwReg.REG_CPBR_BASE  = ((ITEM2Duint32)hwCoverageImage->data) >> ITEM2D_VG_CMDSHIFT_PLANBASE;
    hwReg.REG_CVPPR_BASE = (hwValidImage->pitch << ITEM2D_VG_CMDSHIFT_VALIDPITCH) |
                           (hwCoverageImage->pitch << ITEM2D_VG_CMDSHIFT_PLANPITCH);
    hwReg.REG_VPBR_BASE  = ((ITEM2Duint32)hwValidImage->data) << ITEM2D_VG_CMDSHIFT_VALIDBASE;
    if (((M2D_SURFACE *)destSurface)->clipSet.clipRegionCount == 1)
    {
        hwReg.REG_PXCR_BASE = ((((M2D_SURFACE *)destSurface)->clipSet.clipRegion->right) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND) | (((M2D_SURFACE *)destSurface)->clipSet.clipRegion->left);
        hwReg.REG_PYCR_BASE = ((((M2D_SURFACE *)destSurface)->clipSet.clipRegion->bottom) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND) | (((M2D_SURFACE *)destSurface)->clipSet.clipRegion->top);
    }
    else
    {
        hwReg.REG_PXCR_BASE = (hwCoverageImage->width - 1) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND;
        hwReg.REG_PYCR_BASE = (hwCoverageImage->height - 1) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND;
    }
    hwReg.REG_ACR_BASE |= ITEM2D_VG_CMD_CLRCOVCACHE | ITEM2D_VG_CMD_CLRRDRCACHE;

    hwReg.REG_RCR_BASE  = ITEM2D_VG_CMD_DST_PRE_EN |
                          ITEM2D_VG_CMD_SRC_NONPRE_EN |
                          ITEM2D_VG_CMD_SRC_PRE_EN |
                          ITEM2D_VG_CMD_TEX_PRE_EN |
                          ITEM2D_VG_CMD_WR_NONPRE_EN |
                          ITEM2D_VG_CMD_DITHER_EN |  // always enable dither
                          ITEM2D_VG_CMD_BLEND_EN |
                          ITEM2D_VG_CMD_DESTINATION_EN |
                          ITEM2D_VG_CMD_TEXCACHE_EN |
                          ITEM2D_VG_CMD_TEXTURE_EN |
                          ITEM2D_VG_CMD_COVERAGE_EN |
                          hwImageQuality |
                          hwFillRule |
                          ITEM2D_VG_CMD_RENDERMODE_1 |
                          ITEM2D_VG_CMD_RDPLN_VLD_EN;

    hwReg.REG_RMR_BASE = ITEM2D_VG_CMD_AUTOSCAN_EN |
                         ITEM2D_VG_CMD_GAMMAMODE_INVERSE |
                         ITEM2D_VG_CMD_TILEMODE_FILL |
                         ITEM2D_VG_CMD_BLENDMODE_SRC_OVER_DST |
                         ITEM2D_VG_CMD_PAINTTYPE_COLOR |
                         ITEM2D_VG_CMD_IMAGEMODE_STENCIL |
                         ITEM2D_VG_CMD_MASKMODE_INTERSECT;

    /* Render format register */
    destImageFormat = _M2dImgFormat2HwImgFormat(destSurface->imageFormat);

    //color image format???
    hwReg.REG_RFR_BASE |= //ITEM2D_VG_CMD_SRCEXTEND_EN |
                          //ITEM2D_VG_CMD_DSTEXTEND_EN |
                          //ITEM2D_VG_CMD_MASKEXTEND_EN |
                          //(vgSurface->maskImage->vgformat << 16) |
                          destImageFormat << 8 |
                          srcImage.vgformat;

    hwReg.REG_RCR00_BASE = (((destSurface->font->fontColor & 0x0000001F) * 255) / 31) << 20 | (0xFF);
    hwReg.REG_RCR01_BASE = ((((destSurface->font->fontColor & 0x0000F800) >> 11) * 255) / 31) << 20 | (((((destSurface->font->fontColor & 0x000007E0) >> 5) * 255) / 63) << 4);

    // Destination Coordinate Register
    //hwReg.REG_DCR_BASE = (((ITEs12p3)destY << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
    //((ITEs12p3)destX & ITE_VG_CMDMASK_DSTX);

    // Destination Height/Width Register
    hwReg.REG_DHWR_BASE = (((ITEM2Dint16)destSurface->height << ITEM2D_VG_CMDSHIFT_DSTHEIGHT) & ITEM2D_VG_CMDMASK_DSTHEIGHT) |
                          ((ITEM2Dint16)destSurface->width & ITEM2D_VG_CMDMASK_DSTWIDTH);

    // Destination Base Register
    hwReg.REG_DBR_BASE  = (ITEM2Duint32)(destSurface->baseScanPtr) & ITEM2D_VG_CMDMASK_SRCBASE;

    // Src/Dst Pitch Register
    hwReg.REG_SDPR_BASE = (((ITEM2Dint16)srcPitch << ITEM2D_VG_CMDSHIFT_SRCPITCH0) & ITEM2D_VG_CMDMASK_SRCPITCH0) |
                          (destSurface->pitch & ITEM2D_VG_CMDMASK_DSTPITCH);

    // Source Coordinate Register
    hwReg.REG_SCR_BASE = (((ITEM2Ds12p3)0 << ITEM2D_VG_CMDSHIFT_DSTY) & ITEM2D_VG_CMDMASK_DSTY) |
                         ((ITEM2Ds12p3)0 & ITEM2D_VG_CMDMASK_DSTX);

    // Source Height/Width Register
    hwReg.REG_SHWR_BASE = (((ITEM2Dint16)srcHeight << ITEM2D_VG_CMDSHIFT_DSTHEIGHT) & ITEM2D_VG_CMDMASK_DSTHEIGHT) |
                          ((ITEM2Dint16)srcWidth & ITEM2D_VG_CMDMASK_DSTWIDTH);

    // Source Base Register
    hwReg.REG_SBR_BASE    = (ITEM2Duint32)(srcImage.data) & ITEM2D_VG_CMDMASK_SRCBASE;

    hwReg.REG_MBR_BASE    = (ITEM2Duint32)vgSurface->maskImage->data;

    hwReg.REG_SMPR_BASE   = vgSurface->maskImage->pitch;

    hwReg.REG_CTSR0_BASE  = (((ITEM2Dint16)(0 * 0x100) << ITEM2D_VG_CMDSHIFT_SCOLXFM00) & ITEM2D_VG_CMDMASK_SCOLXFM00) |
                            (ITEM2Dint16)(1 * 0x100);

    hwReg.REG_CTSR1_BASE  = (((ITEM2Dint16)(0 * 0x100) << ITEM2D_VG_CMDSHIFT_SCOLXFM00) & ITEM2D_VG_CMDMASK_SCOLXFM00) |
                            (ITEM2Dint16)(1 * 0x100);

    hwReg.REG_PITR00_BASE = (ITEM2Ds15p16)(1 * 0x10000);
    hwReg.REG_PITR01_BASE = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_PITR02_BASE = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_PITR10_BASE = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_PITR11_BASE = (ITEM2Ds15p16)(1 * 0x10000);
    hwReg.REG_PITR12_BASE = (ITEM2Ds15p16)(0 * 0x10000);

    /* Set image object ID */
    iteM2dHardwareGenObjectID();
    hwReg.REG_BID2_BASE    = iteM2dHardwareGetCurrentObjectID();
    destSurface->bitbltId2 = hwReg.REG_BID2_BASE;

    /* Set HW to cmdq */
    iteM2dHardwareFire(&hwReg);

    if (hwWaitObjID == ITEM2D_TRUE)
    {
        iteM2dHardwareWaitObjID(iteM2dHardwareGetCurrentObjectID());
    }

    free(p.pathCommand.items);
}

MMP_RESULT
m2dDrawAABmpTextTtx2(
    M2D_SURFACE *destSurface,
    MMP_INT     destX,
    MMP_INT     destY,
    void        *srcPtr,
    MMP_UINT    srcWidth,
    MMP_UINT    srcHeight,
    MMP_UINT    srcPitch)
{
    ITEM2DHardwareRegister hwReg                 = {0};
    ITEM2Dboolean          srcPreMultiFormat     = ITEM2D_FALSE;
    ITEM2Dboolean          dstPreMultiFormat     = ITEM2D_FALSE;
    ITEM2Dboolean          EnPreMultiForTexture  = ITEM2D_FALSE;       // Reg 0x0AC, Bit 28
    ITEM2Dboolean          EnUnpreMultiForDst    = ITEM2D_FALSE;       // Reg 0x0AC, Bit 31
    HWImageFormat          destImageFormat       = HW_sRGB_565;
    HWImageFormat          srcImageFormat        = HW_sRGB_565;
    ITEM2Dboolean          enScissor             = ITEM2D_FALSE;
    ITEM2Duint32           hwRenderQuality       = ITEM2D_VG_CMD_RENDERQ_BETTER;
    ITEM2Duint32           hwImageQuality        = ITEM2D_VG_CMD_IMAGEQ_FASTER;
    ITEM2Duint32           hwFillRule            = ITEM2D_VG_CMD_FILLRULE_ODDEVEN;
    ITEM2Duint32           hwCoverageFormatBytes = ITEM2D_VG_CMD_PLANFORMAT_TWOBYTES;
    ITEM2Duint32           hwPaintType           = ITEM2D_VG_CMD_PAINTTYPE_COLOR;
    ITEM2DImage            *hwCoverageImage      = NULL;
    ITEM2DImage            *hwValidImage         = NULL;
    ITEM2DImage            srcImage              = {0};
    ITEM2DMatrix3x3        imageMatrix           = {0};
    ITEM2Dfloat            coords[5]             = {0};
    ITEM2Duint8            segments[]            = { M2DVG_MOVE_TO_ABS, M2DVG_HLINE_TO_REL, M2DVG_VLINE_TO_REL, M2DVG_HLINE_TO_REL, M2DVG_CLOSE_PATH };
    ITEM2DPath             p                     = {0};
    ITEM2Dboolean          hwWaitObjID           = ITEM2D_FALSE;
    ITEM2Duint8            *tessellateCmdBuffer  = NULL;

    srcImage.width    = srcWidth;
    srcImage.height   = srcHeight;
    srcImage.pitch    = srcPitch;
    srcImage.vgformat = ITEM2D_VG_CMD_SRCFORMAT_A_4;
    srcImage.data     = (ITEM2Duint8 *)srcPtr;

    coords[0]         = 0.0f;
    coords[1]         = 0.0f;
    coords[2]         = (ITEM2Dfloat)(srcImage.width);
    coords[3]         = (ITEM2Dfloat)(srcImage.height);
    coords[4]         = (ITEM2Dfloat)(-srcImage.width);

    p.format          = M2DVG_PATH_FORMAT_STANDARD;
    p.scale           = 1.0f;
    p.bias            = 0.0f;
    p.segHint         = 0;
    p.dataHint        = 0;
    p.datatype        = M2DVG_PATH_DATATYPE_F;
    p.caps            = M2DVG_PATH_CAPABILITY_ALL;
    p.segCount        = 5;
    p.segs            = segments;
    p.dataCount       = 5;
    p.data            = coords;

    ITEM2D_INITOBJ(ITEM2DHardware, hwReg);

    iteM2dFlattenPath(&p, 1, &p.pathCommand);
    hwReg.REG_PLR_BASE = p.pathCommand.size * sizeof(ITEM2Duint32);

    //tessellateCmdBuffer
    //allocate vram buffer

    if ( (p.pathCommand.size * sizeof(ITEM2Duint32)) <= ITEM2D_PATH_COPY_SIZE_THRESHOLD)
    {
        ITEM2Duint8  *mappedSysRam = NULL;
        ITEM2Duint32 allocSize     = p.pathCommand.size * sizeof(ITEM2Duint32);

        tessellateCmdBuffer = (ITEM2Duint8 *)m2dvgMemalign(4, allocSize, iteM2dHardwareGenObjectID());
        mappedSysRam        = ithMapVram((uint32_t)tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
        memcpy(mappedSysRam, p.pathCommand.items, allocSize);
        ithFlushDCacheRange(mappedSysRam, allocSize);
        ithUnmapVram(mappedSysRam, allocSize);
    }
    else
    {
#ifdef _WIN32
        ITEM2Duint8  *mappedSysRam = NULL;
        ITEM2Duint32 allocSize     = p.pathCommand.size * sizeof(ITEM2Duint32);

        tessellateCmdBuffer = (ITEM2Duint8 *)m2dvgMemalign(4, p.pathCommand.size * sizeof(ITEM2Duint32), iteM2dHardwareGenObjectID());
        mappedSysRam        = ithMapVram(tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
        memcpy(mappedSysRam, p.pathCommand.items, allocSize);
        ithFlushDCacheRange(mappedSysRam, allocSize);
        ithUnmapVram(mappedSysRam, allocSize);
#else
        tessellateCmdBuffer = (ITEM2Duint8 *)p.pathCommand.items;
#endif
        hwWaitObjID         = ITEM2D_TRUE;
    }

    hwReg.REG_PBR_BASE  = ((ITEM2Duint32)tessellateCmdBuffer) << ITEM2D_VG_CMDSHIFT_PATHBASE;
    hwReg.REG_BID2_BASE = iteM2dHardwareGetCurrentObjectID();

    /* Get coverage image parameter */
    if (vgSurface->coverageIndex)
    {
        hwCoverageImage          = vgSurface->coverageImageB;
        hwValidImage             = vgSurface->validImageB;
        vgSurface->coverageIndex = 0;
    }
    else
    {
        hwCoverageImage          = vgSurface->coverageImageA;
        hwValidImage             = vgSurface->validImageA;
        vgSurface->coverageIndex = 1;
    }

    {
        ITEM2DMatrix3x3 glyphUserToSurfaceMatrix;
        ITEM2DMatrix3x3 localTransformMatrix, tempMatrix;
        ITEM2DVector2   deltaVector;
        ITEM2DMatrix3x3 imageInvMatrix;

        IDMAT(glyphUserToSurfaceMatrix);
        deltaVector.x = destX;
        deltaVector.y = destY;
        IDMAT(localTransformMatrix);
        TRANSLATEMATL(localTransformMatrix, deltaVector.x, deltaVector.y);
        MULMATMAT(glyphUserToSurfaceMatrix, localTransformMatrix, tempMatrix);
        glyphUserToSurfaceMatrix = tempMatrix;
        /* force affinity */
        M2DSETMAT(glyphUserToSurfaceMatrix,
                  glyphUserToSurfaceMatrix.m[0][0], glyphUserToSurfaceMatrix.m[0][1], glyphUserToSurfaceMatrix.m[0][2],
                  glyphUserToSurfaceMatrix.m[1][0], glyphUserToSurfaceMatrix.m[1][1], glyphUserToSurfaceMatrix.m[1][2],
                  0, 0, 1);

        hwReg.REG_UTR00_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[0][0] * 0x10000);
        hwReg.REG_UTR01_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[0][1] * 0x10000);
        hwReg.REG_UTR02_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[0][2] * 0x10000);
        hwReg.REG_UTR10_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[1][0] * 0x10000);
        hwReg.REG_UTR11_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[1][1] * 0x10000);
        hwReg.REG_UTR12_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[1][2] * 0x10000);
        hwReg.REG_UTR20_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[2][0] * 0x10000);
        hwReg.REG_UTR21_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[2][1] * 0x10000);
        hwReg.REG_UTR22_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[2][2] * 0x10000);

        iteM2dInvertMatrix(&glyphUserToSurfaceMatrix, &imageInvMatrix);
        hwReg.REG_UITR00_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[0][0] * 0x10000);
        hwReg.REG_UITR01_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[0][1] * 0x10000);
        hwReg.REG_UITR02_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[0][2] * 0x10000);
        hwReg.REG_UITR10_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[1][0] * 0x10000);
        hwReg.REG_UITR11_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[1][1] * 0x10000);
        hwReg.REG_UITR12_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[1][2] * 0x10000);
        hwReg.REG_UITR20_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[2][0] * 0x10000);
        hwReg.REG_UITR21_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[2][1] * 0x10000);
        hwReg.REG_UITR22_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[2][2] * 0x10000);
    }

    hwReg.REG_TCR_BASE = ITEM2D_VG_CMD_TRANSFORM_EN |
                         ITEM2D_VG_CMD_READMEM |
                         ITEM2D_VG_CMD_TELWORK;

    hwReg.REG_CCR_BASE = ITEM2D_VG_CMD_CLIPPING |
                         hwRenderQuality |
                         hwCoverageFormatBytes |
                         ITEM2D_VG_CMD_FULLRDY_EN |
                         ITEM2D_VG_CMD_TIMERDY_EN;

    hwReg.REG_CPBR_BASE  = ((ITEM2Duint32)hwCoverageImage->data) >> ITEM2D_VG_CMDSHIFT_PLANBASE;
    hwReg.REG_CVPPR_BASE = (hwValidImage->pitch << ITEM2D_VG_CMDSHIFT_VALIDPITCH) |
                           (hwCoverageImage->pitch << ITEM2D_VG_CMDSHIFT_PLANPITCH);
    hwReg.REG_VPBR_BASE  = ((ITEM2Duint32)hwValidImage->data) << ITEM2D_VG_CMDSHIFT_VALIDBASE;
    if (((M2D_SURFACE *)destSurface)->clipSet.clipRegionCount == 1)
    {
        hwReg.REG_PXCR_BASE = ((((M2D_SURFACE *)destSurface)->clipSet.clipRegion->right) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND) | (((M2D_SURFACE *)destSurface)->clipSet.clipRegion->left);
        hwReg.REG_PYCR_BASE = ((((M2D_SURFACE *)destSurface)->clipSet.clipRegion->bottom) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND) | (((M2D_SURFACE *)destSurface)->clipSet.clipRegion->top);
    }
    else
    {
        hwReg.REG_PXCR_BASE = (hwCoverageImage->width - 1) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND;
        hwReg.REG_PYCR_BASE = (hwCoverageImage->height - 1) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND;
    }
    hwReg.REG_ACR_BASE |= ITEM2D_VG_CMD_CLRCOVCACHE | ITEM2D_VG_CMD_CLRRDRCACHE;

    hwReg.REG_RCR_BASE  = ITEM2D_VG_CMD_DST_PRE_EN |
                          ITEM2D_VG_CMD_SRC_NONPRE_EN |
                          ITEM2D_VG_CMD_SRC_PRE_EN |
                          ITEM2D_VG_CMD_TEX_PRE_EN |
                          ITEM2D_VG_CMD_WR_NONPRE_EN |
                          ITEM2D_VG_CMD_DITHER_EN |  // always enable dither
                          ITEM2D_VG_CMD_BLEND_EN |
                          ITEM2D_VG_CMD_DESTINATION_EN |
                          ITEM2D_VG_CMD_TEXCACHE_EN |
                          ITEM2D_VG_CMD_TEXTURE_EN |
                          ITEM2D_VG_CMD_COVERAGE_EN |
                          hwImageQuality |
                          hwFillRule |
                          ITEM2D_VG_CMD_RENDERMODE_1 |
                          ITEM2D_VG_CMD_RDPLN_VLD_EN;

    hwReg.REG_RMR_BASE = ITEM2D_VG_CMD_AUTOSCAN_EN |
                         ITEM2D_VG_CMD_GAMMAMODE_INVERSE |
                         ITEM2D_VG_CMD_TILEMODE_FILL |
                         ITEM2D_VG_CMD_BLENDMODE_SRC_OVER_DST |
                         ITEM2D_VG_CMD_PAINTTYPE_COLOR |
                         ITEM2D_VG_CMD_IMAGEMODE_STENCIL |
                         ITEM2D_VG_CMD_MASKMODE_INTERSECT;

    /* Render format register */
    destImageFormat = _M2dImgFormat2HwImgFormat(destSurface->imageFormat);

    //color image format???
    hwReg.REG_RFR_BASE |= //ITEM2D_VG_CMD_SRCEXTEND_EN |
                          //ITEM2D_VG_CMD_DSTEXTEND_EN |
                          //ITEM2D_VG_CMD_MASKEXTEND_EN |
                          //(vgSurface->maskImage->vgformat << 16) |
                          destImageFormat << 8 |
                          srcImage.vgformat;

    hwReg.REG_RCR00_BASE = (((destSurface->font->fontColor & 0x0000001F) * 255) / 31) << 20 | (0xFF);
    hwReg.REG_RCR01_BASE = ((((destSurface->font->fontColor & 0x0000F800) >> 11) * 255) / 31) << 20 | (((((destSurface->font->fontColor & 0x000007E0) >> 5) * 255) / 63) << 4);

    // Destination Coordinate Register
    //hwReg.REG_DCR_BASE = (((ITEs12p3)destY << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
    //((ITEs12p3)destX & ITE_VG_CMDMASK_DSTX);

    // Destination Height/Width Register
    hwReg.REG_DHWR_BASE = (((ITEM2Dint16)destSurface->height << ITEM2D_VG_CMDSHIFT_DSTHEIGHT) & ITEM2D_VG_CMDMASK_DSTHEIGHT) |
                          ((ITEM2Dint16)destSurface->width & ITEM2D_VG_CMDMASK_DSTWIDTH);

    // Destination Base Register
    hwReg.REG_DBR_BASE  = (ITEM2Duint32)(destSurface->baseScanPtr) & ITEM2D_VG_CMDMASK_SRCBASE;

    // Src/Dst Pitch Register
    hwReg.REG_SDPR_BASE = (((ITEM2Dint16)srcPitch << ITEM2D_VG_CMDSHIFT_SRCPITCH0) & ITEM2D_VG_CMDMASK_SRCPITCH0) |
                          (destSurface->pitch & ITEM2D_VG_CMDMASK_DSTPITCH);

    // Source Coordinate Register
    hwReg.REG_SCR_BASE = (((ITEM2Ds12p3)0 << ITEM2D_VG_CMDSHIFT_DSTY) & ITEM2D_VG_CMDMASK_DSTY) |
                         ((ITEM2Ds12p3)0 & ITEM2D_VG_CMDMASK_DSTX);

    // Source Height/Width Register
    hwReg.REG_SHWR_BASE = (((ITEM2Dint16)srcHeight << ITEM2D_VG_CMDSHIFT_DSTHEIGHT) & ITEM2D_VG_CMDMASK_DSTHEIGHT) |
                          ((ITEM2Dint16)srcWidth & ITEM2D_VG_CMDMASK_DSTWIDTH);

    // Source Base Register
    hwReg.REG_SBR_BASE    = (ITEM2Duint32)(srcImage.data) & ITEM2D_VG_CMDMASK_SRCBASE;

    hwReg.REG_MBR_BASE    = (ITEM2Duint32)vgSurface->maskImage->data;

    hwReg.REG_SMPR_BASE   = vgSurface->maskImage->pitch;

    hwReg.REG_CTSR0_BASE  = (((ITEM2Dint16)(0 * 0x100) << ITEM2D_VG_CMDSHIFT_SCOLXFM00) & ITEM2D_VG_CMDMASK_SCOLXFM00) |
                            (ITEM2Dint16)(1 * 0x100);

    hwReg.REG_CTSR1_BASE  = (((ITEM2Dint16)(0 * 0x100) << ITEM2D_VG_CMDSHIFT_SCOLXFM00) & ITEM2D_VG_CMDMASK_SCOLXFM00) |
                            (ITEM2Dint16)(1 * 0x100);

    hwReg.REG_PITR00_BASE = (ITEM2Ds15p16)(1 * 0x10000);
    hwReg.REG_PITR01_BASE = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_PITR02_BASE = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_PITR10_BASE = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_PITR11_BASE = (ITEM2Ds15p16)(1 * 0x10000);
    hwReg.REG_PITR12_BASE = (ITEM2Ds15p16)(0 * 0x10000);

    /* Set image object ID */
    if (hwWaitObjID == ITEM2D_TRUE)
    {
        iteM2dHardwareGenObjectID();
    }
    hwReg.REG_BID2_BASE    = iteM2dHardwareGetCurrentObjectID();
    destSurface->bitbltId2 = hwReg.REG_BID2_BASE;

    /* Set HW to cmdq */
    iteM2dHardwareFire(&hwReg);

    if (hwWaitObjID == ITEM2D_TRUE)
    {
        iteM2dHardwareWaitObjID(iteM2dHardwareGetCurrentObjectID());
    }

    free(p.pathCommand.items);
}

MMP_RESULT
m2dDrawAABmpTextTtx3(
    M2D_SURFACE *destSurface,
    MMP_INT     destX,
    MMP_INT     destY,
    void        *srcPtr,
    MMP_UINT    srcWidth,
    MMP_UINT    srcHeight,
    MMP_UINT    srcPitch)
{
    ITEM2DHardwareRegister hwReg                 = {0};
    ITEM2Dboolean          srcPreMultiFormat     = ITEM2D_FALSE;
    ITEM2Dboolean          dstPreMultiFormat     = ITEM2D_FALSE;
    ITEM2Dboolean          EnPreMultiForTexture  = ITEM2D_FALSE;       // Reg 0x0AC, Bit 28
    ITEM2Dboolean          EnUnpreMultiForDst    = ITEM2D_FALSE;       // Reg 0x0AC, Bit 31
    HWImageFormat          destImageFormat       = HW_sRGB_565;
    HWImageFormat          srcImageFormat        = HW_sRGB_565;
    ITEM2Dboolean          enScissor             = ITEM2D_FALSE;
    ITEM2Duint32           hwRenderQuality       = ITEM2D_VG_CMD_RENDERQ_BETTER;
    ITEM2Duint32           hwImageQuality        = ITEM2D_VG_CMD_IMAGEQ_FASTER;
    ITEM2Duint32           hwFillRule            = ITEM2D_VG_CMD_FILLRULE_ODDEVEN;
    ITEM2Duint32           hwCoverageFormatBytes = ITEM2D_VG_CMD_PLANFORMAT_TWOBYTES;
    ITEM2DImage            *hwCoverageImage      = NULL;
    ITEM2DImage            *hwValidImage         = NULL;
    ITEM2DImage            srcImage              = {0};
    ITEM2DMatrix3x3        imageMatrix           = {0};
    ITEM2Dfloat            coords[5]             = {0};
    ITEM2Duint8            segments[]            = { M2DVG_MOVE_TO_ABS, M2DVG_HLINE_TO_REL, M2DVG_VLINE_TO_REL, M2DVG_HLINE_TO_REL, M2DVG_CLOSE_PATH };
    ITEM2DPath             p                     = {0};
    ITEM2Dboolean          hwWaitObjID           = ITEM2D_FALSE;
    ITEM2Duint8            *tessellateCmdBuffer  = NULL;

    srcImage.width    = srcWidth;
    srcImage.height   = srcHeight;
    srcImage.pitch    = srcPitch;
    srcImage.vgformat = ITEM2D_VG_CMD_SRCFORMAT_A_8;
    srcImage.data     = (ITEM2Duint8 *)srcPtr;

    coords[0]         = 0.0f;
    coords[1]         = 0.0f;
    coords[2]         = (ITEM2Dfloat)(srcImage.width);
    coords[3]         = (ITEM2Dfloat)(srcImage.height);
    coords[4]         = (ITEM2Dfloat)(-srcImage.width);

    p.format          = M2DVG_PATH_FORMAT_STANDARD;
    p.scale           = 1.0f;
    p.bias            = 0.0f;
    p.segHint         = 0;
    p.dataHint        = 0;
    p.datatype        = M2DVG_PATH_DATATYPE_F;
    p.caps            = M2DVG_PATH_CAPABILITY_ALL;
    p.segCount        = 5;
    p.segs            = segments;
    p.dataCount       = 5;
    p.data            = coords;

    ITEM2D_INITOBJ(ITEM2DHardware, hwReg);

    iteM2dFlattenPath(&p, 1, &p.pathCommand);
    hwReg.REG_PLR_BASE = p.pathCommand.size * sizeof(ITEM2Duint32);

    if ( (p.pathCommand.size * sizeof(ITEM2Duint32)) <= ITEM2D_PATH_COPY_SIZE_THRESHOLD)
    {
        ITEM2Duint8  *mappedSysRam = NULL;
        ITEM2Duint32 allocSize     = p.pathCommand.size * sizeof(ITEM2Duint32);

        tessellateCmdBuffer = (ITEM2Duint8 *)m2dvgMemalign(4, allocSize, iteM2dHardwareGenObjectID());
        mappedSysRam        = ithMapVram((uint32_t)tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
        memcpy(mappedSysRam, p.pathCommand.items, allocSize);
        ithFlushDCacheRange(mappedSysRam, allocSize);
        ithUnmapVram(mappedSysRam, allocSize);
    }
    else
    {
#ifdef _WIN32
        ITEM2Duint8  *mappedSysRam = NULL;
        ITEM2Duint32 allocSize     = p.pathCommand.size * sizeof(ITEM2Duint32);

        tessellateCmdBuffer = (ITEM2Duint8 *)m2dvgMemalign(4, p.pathCommand.size * sizeof(ITEM2Duint32), iteM2dHardwareGenObjectID());
        mappedSysRam        = ithMapVram(tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
        memcpy(mappedSysRam, p.pathCommand.items, allocSize);
        ithFlushDCacheRange(mappedSysRam, allocSize);
        ithUnmapVram(mappedSysRam, allocSize);
#else
        tessellateCmdBuffer = (ITEM2Duint8 *)p.pathCommand.items;
#endif
        hwWaitObjID         = ITEM2D_TRUE;
    }

    hwReg.REG_PBR_BASE  = ((ITEM2Duint32)tessellateCmdBuffer) << ITEM2D_VG_CMDSHIFT_PATHBASE;
    hwReg.REG_BID2_BASE = iteM2dHardwareGetCurrentObjectID();

    /* Get coverage image parameter */
    if (vgSurface->coverageIndex)
    {
        hwCoverageImage          = vgSurface->coverageImageB;
        hwValidImage             = vgSurface->validImageB;
        vgSurface->coverageIndex = 0;
    }
    else
    {
        hwCoverageImage          = vgSurface->coverageImageA;
        hwValidImage             = vgSurface->validImageA;
        vgSurface->coverageIndex = 1;
    }

    /* Prevent to modify not processed data */
    {
        ITEM2DMatrix3x3 glyphUserToSurfaceMatrix;
        ITEM2DMatrix3x3 localTransformMatrix, tempMatrix;
        ITEM2DVector2   deltaVector;
        ITEM2DMatrix3x3 imageInvMatrix;

        IDMAT(glyphUserToSurfaceMatrix);
        deltaVector.x = destX;
        deltaVector.y = destY;
        IDMAT(localTransformMatrix);
        TRANSLATEMATL(localTransformMatrix, deltaVector.x, deltaVector.y);
        MULMATMAT(glyphUserToSurfaceMatrix, localTransformMatrix, tempMatrix);
        glyphUserToSurfaceMatrix = tempMatrix;
        /* force affinity */
        M2DSETMAT(glyphUserToSurfaceMatrix,
                  glyphUserToSurfaceMatrix.m[0][0], glyphUserToSurfaceMatrix.m[0][1], glyphUserToSurfaceMatrix.m[0][2],
                  glyphUserToSurfaceMatrix.m[1][0], glyphUserToSurfaceMatrix.m[1][1], glyphUserToSurfaceMatrix.m[1][2],
                  0, 0, 1);

        hwReg.REG_UTR00_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[0][0] * 0x10000);
        hwReg.REG_UTR01_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[0][1] * 0x10000);
        hwReg.REG_UTR02_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[0][2] * 0x10000);
        hwReg.REG_UTR10_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[1][0] * 0x10000);
        hwReg.REG_UTR11_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[1][1] * 0x10000);
        hwReg.REG_UTR12_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[1][2] * 0x10000);
        hwReg.REG_UTR20_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[2][0] * 0x10000);
        hwReg.REG_UTR21_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[2][1] * 0x10000);
        hwReg.REG_UTR22_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[2][2] * 0x10000);

        iteM2dInvertMatrix(&glyphUserToSurfaceMatrix, &imageInvMatrix);
        hwReg.REG_UITR00_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[0][0] * 0x10000);
        hwReg.REG_UITR01_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[0][1] * 0x10000);
        hwReg.REG_UITR02_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[0][2] * 0x10000);
        hwReg.REG_UITR10_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[1][0] * 0x10000);
        hwReg.REG_UITR11_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[1][1] * 0x10000);
        hwReg.REG_UITR12_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[1][2] * 0x10000);
        hwReg.REG_UITR20_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[2][0] * 0x10000);
        hwReg.REG_UITR21_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[2][1] * 0x10000);
        hwReg.REG_UITR22_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[2][2] * 0x10000);
    }

    hwReg.REG_TCR_BASE = ITEM2D_VG_CMD_TRANSFORM_EN |
                         ITEM2D_VG_CMD_READMEM |
                         ITEM2D_VG_CMD_TELWORK;

    hwReg.REG_CCR_BASE = ITEM2D_VG_CMD_CLIPPING |
                         hwRenderQuality |
                         hwCoverageFormatBytes |
                         ITEM2D_VG_CMD_FULLRDY_EN |
                         ITEM2D_VG_CMD_TIMERDY_EN;

    hwReg.REG_CPBR_BASE  = ((ITEM2Duint32)hwCoverageImage->data) >> ITEM2D_VG_CMDSHIFT_PLANBASE;
    hwReg.REG_CVPPR_BASE = (hwValidImage->pitch << ITEM2D_VG_CMDSHIFT_VALIDPITCH) |
                           (hwCoverageImage->pitch << ITEM2D_VG_CMDSHIFT_PLANPITCH);
    hwReg.REG_VPBR_BASE  = ((ITEM2Duint32)hwValidImage->data) << ITEM2D_VG_CMDSHIFT_VALIDBASE;
    if (((M2D_SURFACE *)destSurface)->clipSet.clipRegionCount == 1)
    {
        hwReg.REG_PXCR_BASE = ((((M2D_SURFACE *)destSurface)->clipSet.clipRegion->right) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND) | (((M2D_SURFACE *)destSurface)->clipSet.clipRegion->left);
        hwReg.REG_PYCR_BASE = ((((M2D_SURFACE *)destSurface)->clipSet.clipRegion->bottom) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND) | (((M2D_SURFACE *)destSurface)->clipSet.clipRegion->top);

        //printf("m2dDrawAABmpTextTtx3 right = %d,left = %d,bottom = %d,top = %d\n",((M2D_SURFACE*)destSurface)->clipSet.clipRegion->right,((M2D_SURFACE*)destSurface)->clipSet.clipRegion->left,((M2D_SURFACE*)destSurface)->clipSet.clipRegion->bottom,((M2D_SURFACE*)destSurface)->clipSet.clipRegion->top);
    }
    else
    {
        hwReg.REG_PXCR_BASE = (hwCoverageImage->width - 1) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND;
        hwReg.REG_PYCR_BASE = (hwCoverageImage->height - 1) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND;
    }
    hwReg.REG_ACR_BASE |= ITEM2D_VG_CMD_CLRCOVCACHE | ITEM2D_VG_CMD_CLRRDRCACHE;

    hwReg.REG_RCR_BASE  = ITEM2D_VG_CMD_DST_PRE_EN |
                          ITEM2D_VG_CMD_SRC_NONPRE_EN |
                          ITEM2D_VG_CMD_SRC_PRE_EN |
                          ITEM2D_VG_CMD_TEX_PRE_EN |
                          ITEM2D_VG_CMD_WR_NONPRE_EN |
                          //ITEM2D_VG_CMD_DITHER_EN | // always enable dither
                          ITEM2D_VG_CMD_BLEND_EN |
                          ITEM2D_VG_CMD_DESTINATION_EN |
                          ITEM2D_VG_CMD_TEXCACHE_EN |
                          ITEM2D_VG_CMD_TEXTURE_EN |
                          ITEM2D_VG_CMD_COVERAGE_EN |
                          hwImageQuality |
                          hwFillRule |
                          ITEM2D_VG_CMD_RENDERMODE_1 |
                          ITEM2D_VG_CMD_RDPLN_VLD_EN;

    hwReg.REG_RMR_BASE = ITEM2D_VG_CMD_AUTOSCAN_EN |
                         ITEM2D_VG_CMD_MASKMODE_INTERSECT |
                         ITEM2D_VG_CMD_GAMMAMODE_INVERSE |
                         ITEM2D_VG_CMD_TILEMODE_FILL |
                         ITEM2D_VG_CMD_BLENDMODE_SRC_OVER_DST |
                         ITEM2D_VG_CMD_PAINTTYPE_COLOR |
                         ITEM2D_VG_CMD_IMAGEMODE_STENCIL;

    /* Render format register */
    destImageFormat = _M2dImgFormat2HwImgFormat(destSurface->imageFormat);

    //color image format???
    hwReg.REG_RFR_BASE |= //ITEM2D_VG_CMD_SRCEXTEND_EN |
                          //ITEM2D_VG_CMD_DSTEXTEND_EN |
                          //ITEM2D_VG_CMD_MASKEXTEND_EN |
                          //(vgSurface->maskImage->vgformat << 16) |
                          destImageFormat << 8 |
                          srcImage.vgformat;

    hwReg.REG_RCR00_BASE = (((destSurface->font->fontColor & 0x0000001F) * 255) / 31) << 20 | (0xFF);
    hwReg.REG_RCR01_BASE = ((((destSurface->font->fontColor & 0x0000F800) >> 11) * 255) / 31) << 20 | (((((destSurface->font->fontColor & 0x000007E0) >> 5) * 255) / 63) << 4);

    // Destination Coordinate Register
    //hwReg.REG_DCR_BASE = (((ITEs12p3)destY << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
    //((ITEs12p3)destX & ITE_VG_CMDMASK_DSTX);

    // Destination Height/Width Register
    hwReg.REG_DHWR_BASE = (((ITEM2Dint16)destSurface->height << ITEM2D_VG_CMDSHIFT_DSTHEIGHT) & ITEM2D_VG_CMDMASK_DSTHEIGHT) |
                          ((ITEM2Dint16)destSurface->width & ITEM2D_VG_CMDMASK_DSTWIDTH);

    // Destination Base Register
    hwReg.REG_DBR_BASE  = (ITEM2Duint32)(destSurface->baseScanPtr) & ITEM2D_VG_CMDMASK_SRCBASE;

    // Src/Dst Pitch Register
    hwReg.REG_SDPR_BASE = (((ITEM2Dint16)srcPitch << ITEM2D_VG_CMDSHIFT_SRCPITCH0) & ITEM2D_VG_CMDMASK_SRCPITCH0) |
                          (destSurface->pitch & ITEM2D_VG_CMDMASK_DSTPITCH);

    // Source Coordinate Register
    hwReg.REG_SCR_BASE = (((ITEM2Ds12p3)0 << ITEM2D_VG_CMDSHIFT_DSTY) & ITEM2D_VG_CMDMASK_DSTY) |
                         ((ITEM2Ds12p3)0 & ITEM2D_VG_CMDMASK_DSTX);

    // Source Height/Width Register
    hwReg.REG_SHWR_BASE = (((ITEM2Dint16)srcHeight << ITEM2D_VG_CMDSHIFT_DSTHEIGHT) & ITEM2D_VG_CMDMASK_DSTHEIGHT) |
                          ((ITEM2Dint16)srcWidth & ITEM2D_VG_CMDMASK_DSTWIDTH);

    // Source Base Register
    hwReg.REG_SBR_BASE    = (ITEM2Duint32)(srcImage.data) & ITEM2D_VG_CMDMASK_SRCBASE;

    hwReg.REG_MBR_BASE    = vgSurface->maskImage ? (ITEM2Duint32)vgSurface->maskImage->data : 0;

    hwReg.REG_SMPR_BASE   = vgSurface->maskImage ? vgSurface->maskImage->pitch : 0;

    hwReg.REG_CTSR0_BASE  = (((ITEM2Dint16)(0 * 0x100) << ITEM2D_VG_CMDSHIFT_SCOLXFM00) & ITEM2D_VG_CMDMASK_SCOLXFM00) |
                            (ITEM2Dint16)(1 * 0x100);

    hwReg.REG_CTSR1_BASE  = (((ITEM2Dint16)(0 * 0x100) << ITEM2D_VG_CMDSHIFT_SCOLXFM00) & ITEM2D_VG_CMDMASK_SCOLXFM00) |
                            (ITEM2Dint16)(1 * 0x100);

    hwReg.REG_PITR00_BASE = (ITEM2Ds15p16)(1 * 0x10000);
    hwReg.REG_PITR01_BASE = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_PITR02_BASE = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_PITR10_BASE = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_PITR11_BASE = (ITEM2Ds15p16)(1 * 0x10000);
    hwReg.REG_PITR12_BASE = (ITEM2Ds15p16)(0 * 0x10000);

    /* Set image object ID */
    if (hwWaitObjID == ITEM2D_TRUE)
    {
        iteM2dHardwareGenObjectID();
    }
    hwReg.REG_BID2_BASE    = iteM2dHardwareGetCurrentObjectID();
    destSurface->bitbltId2 = hwReg.REG_BID2_BASE;

    /* Set HW to cmdq */
    iteM2dHardwareFire(&hwReg);

    if (hwWaitObjID == ITEM2D_TRUE)
    {
        iteM2dHardwareWaitObjID(iteM2dHardwareGetCurrentObjectID());
    }

    free(p.pathCommand.items);
}

void
m2dDrawFont(
    M2D_SURFACE *destDisplay,
    void        *path,
    MMP_UINT    x,
    MMP_UINT    y)
{
    ITEM2Dboolean          hwWaitObjID          = ITEM2D_FALSE;
    ITEM2DHardwareRegister hwReg                = {0};
    ITEM2Dboolean          srcPreMultiFormat    = ITEM2D_FALSE;
    ITEM2Dboolean          dstPreMultiFormat    = ITEM2D_FALSE;
    ITEM2Dboolean          EnPreMultiForTexture = ITEM2D_FALSE; // Reg 0x0AC, Bit 28
    ITEM2Dboolean          EnUnpreMultiForDst   = ITEM2D_FALSE; // Reg 0x0AC, Bit 31
    HWMatrix3x3            transMat             = {0};
    HWImageFormat          destImageFormat      = HW_sRGB_565;
    HWImageFormat          srcImageFormat       = HW_sRGB_565;
    ITEM2Dboolean          enScissor            = ITEM2D_FALSE;
    ITEM2DImage            *hwCoverageImage     = NULL;
    ITEM2DImage            *hwValidImage        = NULL;
    ITEM2Duint8            *tessellateCmdBuffer = NULL;
    ITEM2DPath             *p2                  = (ITEM2DPath *)path;

    if (p2 == MMP_NULL)
        return;

    /* Get coverage image parameter */
    if (vgSurface->coverageIndex)
    {
        hwCoverageImage          = vgSurface->coverageImageB;
        hwValidImage             = vgSurface->validImageB;
        vgSurface->coverageIndex = 0;
    }
    else
    {
        hwCoverageImage          = vgSurface->coverageImageA;
        hwValidImage             = vgSurface->validImageA;
        vgSurface->coverageIndex = 1;
    }

    ITEM2D_INITOBJ(ITEM2DHardware, hwReg);

    //x,y
    {
        ITEM2DMatrix3x3 glyphUserToSurfaceMatrix;
        ITEM2DMatrix3x3 localTransformMatrix, tempMatrix;
        ITEM2DVector2   deltaVector;
        ITEM2DMatrix3x3 imageInvMatrix;

        IDMAT(glyphUserToSurfaceMatrix);
        deltaVector.x = x;
        deltaVector.y = ((ITEM2Dint16)destDisplay->height) - y;
        IDMAT(localTransformMatrix);
        TRANSLATEMATL(localTransformMatrix, deltaVector.x, deltaVector.y);
        MULMATMAT(glyphUserToSurfaceMatrix, localTransformMatrix, tempMatrix);
        glyphUserToSurfaceMatrix = tempMatrix;
        /* force affinity */
        M2DSETMAT(glyphUserToSurfaceMatrix,
                  glyphUserToSurfaceMatrix.m[0][0], glyphUserToSurfaceMatrix.m[0][1], glyphUserToSurfaceMatrix.m[0][2],
                  glyphUserToSurfaceMatrix.m[1][0], glyphUserToSurfaceMatrix.m[1][1], glyphUserToSurfaceMatrix.m[1][2],
                  0, 0, 1);

        hwReg.REG_UTR00_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[0][0] * 0x10000);
        hwReg.REG_UTR01_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[0][1] * 0x10000);
        hwReg.REG_UTR02_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[0][2] * 0x10000);
        hwReg.REG_UTR10_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[1][0] * 0x10000);
        hwReg.REG_UTR11_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[1][1] * 0x10000);
        hwReg.REG_UTR12_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[1][2] * 0x10000);
        hwReg.REG_UTR20_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[2][0] * 0x10000);
        hwReg.REG_UTR21_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[2][1] * 0x10000);
        hwReg.REG_UTR22_BASE  = (ITEM2Ds15p16)(glyphUserToSurfaceMatrix.m[2][2] * 0x10000);

        iteM2dInvertMatrix(&glyphUserToSurfaceMatrix, &imageInvMatrix);
        hwReg.REG_UITR00_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[0][0] * 0x10000);
        hwReg.REG_UITR01_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[0][1] * 0x10000);
        hwReg.REG_UITR02_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[0][2] * 0x10000);
        hwReg.REG_UITR10_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[1][0] * 0x10000);
        hwReg.REG_UITR11_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[1][1] * 0x10000);
        hwReg.REG_UITR12_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[1][2] * 0x10000);
        hwReg.REG_UITR20_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[2][0] * 0x10000);
        hwReg.REG_UITR21_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[2][1] * 0x10000);
        hwReg.REG_UITR22_BASE = (ITEM2Ds15p16)(imageInvMatrix.m[2][2] * 0x10000);
    }

    if (p2->cmdDirty == ITEM2D_TRUE)
    {
        iteM2dPathCommandArrayReserve(&p2->pathCommand, 256);
        iteM2dFlattenPath(p2, 1, &p2->pathCommand);
        p2->cmdDirty = ITEM2D_FALSE;
    }

    hwReg.REG_TCR_BASE = ITEM2D_VG_CMD_TRANSFORM_EN |
                         ITEM2D_VG_CMD_SHIFTROUNDING |
                         ITEM2D_VG_CMD_JOINROUND |
                         ITEM2D_VG_CMD_READMEM |
                         ITEM2D_VG_CMD_TELWORK;

    hwReg.REG_PLR_BASE = p2->pathCommand.size * sizeof(ITEM2Duint32);

    //tessellateCmdBuffer
    //allocate vram buffer

    if ( (p2->pathCommand.size * sizeof(ITEM2Duint32)) <= ITEM2D_PATH_COPY_SIZE_THRESHOLD)
    {
        ITEM2Duint8  *mappedSysRam = NULL;
        ITEM2Duint32 allocSize     = p2->pathCommand.size * sizeof(ITEM2Duint32);

        tessellateCmdBuffer = (ITEM2Duint8 *)m2dvgMemalign(4, allocSize, iteM2dHardwareGenObjectID());
        mappedSysRam        = ithMapVram((uint32_t)tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
        memcpy(mappedSysRam, p2->pathCommand.items, allocSize);
        ithFlushDCacheRange(mappedSysRam, allocSize);
        ithUnmapVram(mappedSysRam, allocSize);
    }
    else
    {
#ifdef _WIN32
        ITEM2Duint8  *mappedSysRam = NULL;
        ITEM2Duint32 allocSize     = p2->pathCommand.size * sizeof(ITEM2Duint32);

        tessellateCmdBuffer = (ITEM2Duint8 *)m2dvgMemalign(4, p2->pathCommand.size * sizeof(ITEM2Duint32), iteM2dHardwareGenObjectID());
        mappedSysRam        = ithMapVram(tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
        memcpy(mappedSysRam, p2->pathCommand.items, allocSize);
        ithFlushDCacheRange(mappedSysRam, allocSize);
        ithUnmapVram(mappedSysRam, allocSize);
#else
        tessellateCmdBuffer = (ITEM2Duint8 *)p2->pathCommand.items;
#endif
        hwWaitObjID         = ITEM2D_TRUE;
    }

    hwReg.REG_PBR_BASE = ((ITEM2Duint32)tessellateCmdBuffer) << ITEM2D_VG_CMDSHIFT_PATHBASE;

    /* Set clipping range */
    hwReg.REG_CCR_BASE = ITEM2D_VG_CMD_CLIPPING |
                         ITEM2D_VG_CMD_RENDERQ_BETTER |
                         ITEM2D_VG_CMD_PLANFORMAT_TWOBYTES |
                         (1 << 6) |
                         (1 << 7);

    hwReg.REG_CPBR_BASE  = ((ITEM2Duint32)hwCoverageImage->data) >> ITEM2D_VG_CMDSHIFT_PLANBASE;
    hwReg.REG_CVPPR_BASE = (hwValidImage->pitch << ITEM2D_VG_CMDSHIFT_VALIDPITCH) |
                           (hwCoverageImage->pitch << ITEM2D_VG_CMDSHIFT_PLANPITCH);
    hwReg.REG_VPBR_BASE  = ((ITEM2Duint32)hwValidImage->data) << ITEM2D_VG_CMDSHIFT_VALIDBASE;
    if (((M2D_SURFACE *)destDisplay)->clipSet.clipRegionCount == 1)
    {
        hwReg.REG_PXCR_BASE = ((((M2D_SURFACE *)destDisplay)->clipSet.clipRegion->right) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND) | (((M2D_SURFACE *)destDisplay)->clipSet.clipRegion->left);
        hwReg.REG_PYCR_BASE = ((((M2D_SURFACE *)destDisplay)->clipSet.clipRegion->bottom) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND) | (((M2D_SURFACE *)destDisplay)->clipSet.clipRegion->top);
    }
    else
    {
        hwReg.REG_PXCR_BASE = (hwCoverageImage->width - 1) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND;
        hwReg.REG_PYCR_BASE = (hwCoverageImage->height - 1) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND;
    }
    hwReg.REG_ACR_BASE |= ITEM2D_VG_CMD_CLRCOVCACHE | ITEM2D_VG_CMD_CLRRDRCACHE;

    /* Setup render engine */
    // Render Control Register
    hwReg.REG_RCR_BASE = ITEM2D_VG_CMD_DST_PRE_EN |
                         ITEM2D_VG_CMD_SRC_NONPRE_EN |
                         ITEM2D_VG_CMD_SRC_PRE_EN |
                         ITEM2D_VG_CMD_DITHER_EN | // always enable dither
                         ITEM2D_VG_CMD_BLEND_EN |
                         ITEM2D_VG_CMD_DESTINATION_EN |
                         ITEM2D_VG_CMD_TEXCACHE_EN |
                         ITEM2D_VG_CMD_TEXTURE_EN |
                         ITEM2D_VG_CMD_COVERAGE_EN |
                         ITEM2D_VG_CMD_FILLRULE_NONZERO |
                         ITEM2D_VG_CMD_IMAGEQ_FASTER |
                         ITEM2D_VG_CMD_RDPLN_VLD_EN |
                         ITEM2D_VG_CMD_RENDERMODE_0;

    hwReg.REG_RMR_BASE = ITEM2D_VG_CMD_AUTOSCAN_EN |
                         ITEM2D_VG_CMD_GAMMAMODE_INVERSE |
                         ITEM2D_VG_CMD_BLENDMODE_SRC_OVER_DST |
                         ITEM2D_VG_CMD_PAINTTYPE_COLOR |
                         ITEM2D_VG_CMD_RAMPMODE_PAD |
                         ITEM2D_VG_CMD_IMAGEMODE_STENCIL;

    /* Render format register */
    hwReg.REG_RFR_BASE &= 0x00FFFFFF;

    destImageFormat = _M2dImgFormat2HwImgFormat(destDisplay->imageFormat);
    hwReg.REG_RFR_BASE  |= (destImageFormat << 8);

    hwReg.REG_RCR00_BASE = (((destDisplay->font->fontColor & 0x0000001F) * 255) / 31) << 20 | (0xFF);
    hwReg.REG_RCR01_BASE = ((((destDisplay->font->fontColor & 0x0000F800) >> 11) * 255) / 31) << 20 | (((((destDisplay->font->fontColor & 0x000007E0) >> 5) * 255) / 63) << 4);

    // Destination Coordinate Register
    hwReg.REG_DCR_BASE   = (((ITEM2Ds12p3)0 << ITEM2D_VG_CMDSHIFT_DSTY) & ITEM2D_VG_CMDMASK_DSTY) |
                           ((ITEM2Ds12p3)0 & ITEM2D_VG_CMDMASK_DSTX);

    // Destination Height/Width Register
    hwReg.REG_DHWR_BASE = (((ITEM2Dint16)destDisplay->height << ITEM2D_VG_CMDSHIFT_DSTHEIGHT) & ITEM2D_VG_CMDMASK_DSTHEIGHT) |
                          ((ITEM2Dint16)destDisplay->width & ITEM2D_VG_CMDMASK_DSTWIDTH);

    // Destination Base Register
    hwReg.REG_DBR_BASE    = (ITEM2Duint32)(destDisplay->baseScanPtr) & ITEM2D_VG_CMDMASK_SRCBASE;

    // Src/Dst Pitch Register
    hwReg.REG_SDPR_BASE   = (destDisplay->pitch & ITEM2D_VG_CMDMASK_DSTPITCH);

    hwReg.REG_CTSR0_BASE  = (((ITEM2Dint16)(0 * 0x100) << ITEM2D_VG_CMDSHIFT_SCOLXFM00) & ITEM2D_VG_CMDMASK_SCOLXFM00) |
                            (ITEM2Dint16)(1 * 0x100);

    hwReg.REG_CTSR1_BASE  = (((ITEM2Dint16)(0 * 0x100) << ITEM2D_VG_CMDSHIFT_SCOLXFM00) & ITEM2D_VG_CMDMASK_SCOLXFM00) |
                            (ITEM2Dint16)(1 * 0x100);

    hwReg.REG_PITR00_BASE = (ITEM2Ds15p16)(1 * 0x10000);
    hwReg.REG_PITR01_BASE = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_PITR02_BASE = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_PITR10_BASE = (ITEM2Ds15p16)(0 * 0x10000);
    hwReg.REG_PITR11_BASE = (ITEM2Ds15p16)(1 * 0x10000);
    hwReg.REG_PITR12_BASE = (ITEM2Ds15p16)(0 * 0x10000);

    /* Set image object ID */
    if (hwWaitObjID == ITEM2D_TRUE)
    {
        iteM2dHardwareGenObjectID();
    }
    hwReg.REG_BID2_BASE    = iteM2dHardwareGetCurrentObjectID();
    destDisplay->bitbltId2 = hwReg.REG_BID2_BASE;

    //font flip
    hwReg.REG_SFR_BASE     = ((ITEM2Dint16)destDisplay->height) << 16;
    hwReg.REG_CCR_BASE    |= (1 << 30);

    iteM2dHardwareFire(&hwReg);

    if (hwWaitObjID == ITEM2D_TRUE)
    {
        iteM2dHardwareWaitObjID(iteM2dHardwareGetCurrentObjectID());
    }
}

MMP_RESULT
m2dTransformations(
    M2D_SURFACE *destSurface,
    MMP_INT     destX,
    MMP_INT     destY,
    M2D_SURFACE *srcSurface,
    MMP_INT     cX,
    MMP_INT     cY,
    float       angle,
    float       scale)
{
    ITEM2DHardwareRegister hwReg                 = {0};
    ITEM2Dboolean          srcPreMultiFormat     = ITEM2D_FALSE;
    ITEM2Dboolean          dstPreMultiFormat     = ITEM2D_FALSE;
    ITEM2Dboolean          EnPreMultiForTexture  = ITEM2D_FALSE; // Reg 0x0AC, Bit 28
    ITEM2Dboolean          EnUnpreMultiForDst    = ITEM2D_FALSE; // Reg 0x0AC, Bit 31
    HWImageFormat          destImageFormat       = HW_sRGB_565;
    HWImageFormat          srcImageFormat        = HW_sRGB_565;
    ITEM2Dboolean          enScissor             = ITEM2D_FALSE;
    ITEM2Duint32           hwRenderQuality       = ITEM2D_VG_CMD_RENDERQ_FASTER;
    ITEM2Duint32           hwImageQuality        = ITEM2D_VG_CMD_IMAGEQ_NONAA;
    ITEM2Duint32           hwFillRule            = ITEM2D_VG_CMD_FILLRULE_ODDEVEN;
    ITEM2Duint32           hwCoverageFormatBytes = ITEM2D_VG_CMD_PLANFORMAT_ONEBYTE;
    ITEM2DImage            *hwCoverageImage      = NULL;
    ITEM2DImage            *hwValidImage         = NULL;
    ITEM2DImage            srcImage              = {0};
    ITEM2Dfloat            coords[5]             = {0};
    ITEM2Duint8            segments[]            = { M2DVG_MOVE_TO_ABS, M2DVG_HLINE_TO_REL, M2DVG_VLINE_TO_REL, M2DVG_HLINE_TO_REL, M2DVG_CLOSE_PATH };
    ITEM2DPath             p                     = {0};
    ITEM2Dboolean          hwWaitObjID           = ITEM2D_FALSE;
    ITEM2Duint8            *tessellateCmdBuffer  = NULL;
    ITEM2DMatrix3x3        transMat              = {1, 0, 0, 0, 1, 0, 0, 0, 1};
    ITEM2DMatrix3x3        transinverseMat       = {0};
    ITEM2DMatrix3x3        filltransMat          = {1, 0, 0, 0, 1, 0, 0, 0, 1};
    ITEM2DMatrix3x3        filltransinverseMat   = {0};
    ITEM2Dfloat            a;

    srcImage.width     = srcSurface->width;
    srcImage.height    = srcSurface->height;
    srcImage.pitch     = srcSurface->pitch;

    //printf("m2dTransformations width=%d,height=%d,pitch=%d\n",srcImage.width,srcImage.height,srcImage.pitch);

    srcImage.vgformat = _M2dImgFormat2HwImgFormat(srcSurface->imageFormat);
    srcImage.data = (ITEM2Duint8 *)srcSurface->baseScanPtr;

    destX         = destX - cX;
    destY         = destY - cY;

    TRANSLATEMATR(filltransMat, destX, destY);

    a = ITEM2D_DEG2RAD(angle);

    TRANSLATEMATR(transMat, ( destX + cX), (destY + cY));
    ROTATEMATR(transMat, a);
    TRANSLATEMATR(transMat, 0 - ( destX + cX), 0 - (destY + cY));
    iteM2dInvertMatrix(&filltransMat, &filltransinverseMat);
    iteM2dInvertMatrix(&transMat, &transinverseMat);

    coords[0]   = destX;
    coords[1]   = destY;
    coords[2]   = srcImage.width;
    coords[3]   = srcImage.height;
    coords[4]   = 0 - srcImage.width;

    p.format    = M2DVG_PATH_FORMAT_STANDARD;
    p.scale     = 1.0f;
    p.bias      = 0.0f;
    p.segHint   = 0.0f;
    p.dataHint  = 0.0f;
    p.datatype  = M2DVG_PATH_DATATYPE_F;
    p.caps      = M2DVG_PATH_CAPABILITY_ALL;
    p.segCount  = 5;
    p.segs      = segments;
    p.dataCount = 5;
    p.data      = coords;

    ITEM2D_INITOBJ(ITEM2DHardware, hwReg);
    iteM2dFlattenPath(&p, 1, &p.pathCommand);
    hwReg.REG_PLR_BASE = p.pathCommand.size * sizeof(ITEM2Duint32);

    //tessellateCmdBuffer
    //allocate vram buffer

    if ( (p.pathCommand.size * sizeof(ITEM2Duint32)) <= ITEM2D_PATH_COPY_SIZE_THRESHOLD)
    {
        ITEM2Duint8  *mappedSysRam = NULL;
        ITEM2Duint32 allocSize     = p.pathCommand.size * sizeof(ITEM2Duint32);

        tessellateCmdBuffer = (ITEM2Duint8 *)m2dvgMemalign(4, allocSize, iteM2dHardwareGenObjectID());
        mappedSysRam        = ithMapVram((uint32_t)tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
        memcpy(mappedSysRam, p.pathCommand.items, allocSize);
        ithFlushDCacheRange(mappedSysRam, allocSize);
        ithUnmapVram(mappedSysRam, allocSize);
    }
    else
    {
#ifdef _WIN32
        ITEM2Duint8  *mappedSysRam = NULL;
        ITEM2Duint32 allocSize     = p.pathCommand.size * sizeof(ITEM2Duint32);

        tessellateCmdBuffer = (ITEM2Duint8 *)m2dvgMemalign(4, p.pathCommand.size * sizeof(ITEM2Duint32), iteM2dHardwareGenObjectID());
        mappedSysRam        = ithMapVram(tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
        memcpy(mappedSysRam, p.pathCommand.items, allocSize);
        ithFlushDCacheRange(mappedSysRam, allocSize);
        ithUnmapVram(mappedSysRam, allocSize);
#else
        tessellateCmdBuffer = (ITEM2Duint8 *)p.pathCommand.items;
#endif
        hwWaitObjID         = ITEM2D_TRUE;
    }

    hwReg.REG_PBR_BASE  = ((ITEM2Duint32)tessellateCmdBuffer) << ITEM2D_VG_CMDSHIFT_PATHBASE;
    hwReg.REG_BID2_BASE = iteM2dHardwareGetCurrentObjectID();

    /* Get coverage image parameter */
    if (vgSurface->coverageIndex)
    {
        hwCoverageImage          = vgSurface->coverageImageB;
        hwValidImage             = vgSurface->validImageB;
        vgSurface->coverageIndex = 0;
    }
    else
    {
        hwCoverageImage          = vgSurface->coverageImageA;
        hwValidImage             = vgSurface->validImageA;
        vgSurface->coverageIndex = 1;
    }

    hwReg.REG_TCR_BASE = //ITE_VG_CMD_PERSPECTIVE_EN |
                         //ITEM2D_VG_CMD_CAPTYPE_ROUND    |
                         0x10 |
                         ITEM2D_VG_CMD_TRANSFORM_EN |
                         ITEM2D_VG_CMD_READMEM |
                         ITEM2D_VG_CMD_TELWORK;

    hwReg.REG_CCR_BASE = ITEM2D_VG_CMD_CLIPPING |
                         hwRenderQuality |
                         hwCoverageFormatBytes |
                         ITEM2D_VG_CMD_FULLRDY_EN |
                         ITEM2D_VG_CMD_TIMERDY_EN;

    if (((M2D_SURFACE *)destSurface)->clipSet.clipRegionCount == 1)
    {
        hwReg.REG_PXCR_BASE = ((((M2D_SURFACE *)destSurface)->clipSet.clipRegion->right) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND) | (((M2D_SURFACE *)destSurface)->clipSet.clipRegion->left);
        hwReg.REG_PYCR_BASE = ((((M2D_SURFACE *)destSurface)->clipSet.clipRegion->bottom) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND) | (((M2D_SURFACE *)destSurface)->clipSet.clipRegion->top);
        //printf("m2dTransformations right = %d,left = %d,bottom = %d,top = %d\n",((M2D_SURFACE*)destSurface)->clipSet.clipRegion->right,((M2D_SURFACE*)destSurface)->clipSet.clipRegion->left,((M2D_SURFACE*)destSurface)->clipSet.clipRegion->bottom,((M2D_SURFACE*)destSurface)->clipSet.clipRegion->top);
    }
    else
    {
        hwReg.REG_PXCR_BASE = (hwCoverageImage->width - 1) << ITEM2D_VG_CMDSHIFT_PXCR_CLIPXEND;
        hwReg.REG_PYCR_BASE = (hwCoverageImage->height - 1) << ITEM2D_VG_CMDSHIFT_PYCR_CLIPXEND;
    }

    hwReg.REG_CPBR_BASE  = ((ITEM2Duint32)hwCoverageImage->data) >> ITEM2D_VG_CMDSHIFT_PLANBASE;
    hwReg.REG_CVPPR_BASE = (hwValidImage->pitch << ITEM2D_VG_CMDSHIFT_VALIDPITCH) |
                           (hwCoverageImage->pitch << ITEM2D_VG_CMDSHIFT_PLANPITCH);
    hwReg.REG_VPBR_BASE  = ((ITEM2Duint32)hwValidImage->data) << ITEM2D_VG_CMDSHIFT_VALIDBASE;
    hwReg.REG_ACR_BASE  |= ITEM2D_VG_CMD_CLRCOVCACHE | ITEM2D_VG_CMD_CLRRDRCACHE;

    hwReg.REG_RCR_BASE   = ITEM2D_VG_CMD_DST_PRE_EN |
                           ITEM2D_VG_CMD_SRC_NONPRE_EN |
                           ITEM2D_VG_CMD_SRC_PRE_EN |
                           ITEM2D_VG_CMD_TEX_PRE_EN |
                           //ITEM2D_VG_CMD_DITHER_EN | // always enable dither
                           ITEM2D_VG_CMD_BLEND_EN |
                           ITEM2D_VG_CMD_DESTINATION_EN |
                           ITEM2D_VG_CMD_TEXCACHE_EN |
                           ITEM2D_VG_CMD_TEXTURE_EN |
                           ITEM2D_VG_CMD_COVERAGE_EN |
                           hwImageQuality |
                           hwFillRule |
                           ITEM2D_VG_CMD_RENDERMODE_0 |
                           ITEM2D_VG_CMD_RDPLN_VLD_EN;

    hwReg.REG_RMR_BASE = ITEM2D_VG_CMD_AUTOSCAN_EN |
                         ITEM2D_VG_CMD_MASKMODE_INTERSECT |
                         ITEM2D_VG_CMD_GAMMAMODE_INVERSE |
                         ITEM2D_VG_CMD_TILEMODE_FILL |
                         ITEM2D_VG_CMD_BLENDMODE_SRC_OVER_DST |
                         ITEM2D_VG_CMD_PAINTTYPE_PATTERN |
                         ITEM2D_VG_CMD_IMAGEMODE_NORMAL;

    /* Render format register */
    destImageFormat = _M2dImgFormat2HwImgFormat(destSurface->imageFormat);

    //color image format???
    hwReg.REG_RFR_BASE |= destImageFormat << 8 |
                          srcImage.vgformat;

    // Destination Height/Width Register
    hwReg.REG_DHWR_BASE = (((ITEM2Dint16)destSurface->height << ITEM2D_VG_CMDSHIFT_DSTHEIGHT) & ITEM2D_VG_CMDMASK_DSTHEIGHT) |
                          ((ITEM2Dint16)destSurface->width & ITEM2D_VG_CMDMASK_DSTWIDTH);

    // Destination Base Register
    hwReg.REG_DBR_BASE  = (ITEM2Duint32)(destSurface->baseScanPtr) & ITEM2D_VG_CMDMASK_SRCBASE;

    // Src/Dst Pitch Register
    hwReg.REG_SDPR_BASE = (((ITEM2Dint16)srcSurface->pitch << ITEM2D_VG_CMDSHIFT_SRCPITCH0) & ITEM2D_VG_CMDMASK_SRCPITCH0) |
                          (destSurface->pitch & ITEM2D_VG_CMDMASK_DSTPITCH);

    // Source Coordinate Register
    hwReg.REG_SCR_BASE = (((ITEM2Ds12p3)0 << ITEM2D_VG_CMDSHIFT_DSTY) & ITEM2D_VG_CMDMASK_DSTY) |
                         ((ITEM2Ds12p3)0 & ITEM2D_VG_CMDMASK_DSTX);

    // Source Height/Width Register
    hwReg.REG_SHWR_BASE = (((ITEM2Dint16)srcSurface->height << ITEM2D_VG_CMDSHIFT_DSTHEIGHT) & ITEM2D_VG_CMDMASK_DSTHEIGHT) |
                          ((ITEM2Dint16)srcSurface->width & ITEM2D_VG_CMDMASK_DSTWIDTH);

    // Source Base Register
    hwReg.REG_SBR_BASE   = (ITEM2Duint32)(srcImage.data) & ITEM2D_VG_CMDMASK_SRCBASE;

    hwReg.REG_RCR00_BASE = 0x000000ff;
    hwReg.REG_RCR01_BASE = 0x00100010;

    hwReg.REG_CTSR0_BASE = (((ITEM2Dint16)(0 * 0x100) << ITEM2D_VG_CMDSHIFT_SCOLXFM00) & ITEM2D_VG_CMDMASK_SCOLXFM00) |
                           (ITEM2Dint16)(1 * 0x100);

    hwReg.REG_CTSR1_BASE = (((ITEM2Dint16)(0 * 0x100) << ITEM2D_VG_CMDSHIFT_SCOLXFM00) & ITEM2D_VG_CMDMASK_SCOLXFM00) |
                           (ITEM2Dint16)(1 * 0x100);

#if 1
    hwReg.REG_UTR00_BASE  = (ITEM2Ds15p16)(transMat.m[0][0] * 0x10000 + 1.0f);
    hwReg.REG_UTR01_BASE  = (ITEM2Ds15p16)(transMat.m[0][1] * 0x10000 + 1.0f);
    hwReg.REG_UTR02_BASE  = (ITEM2Ds15p16)(transMat.m[0][2] * 0x10000 + 1.0f);
    hwReg.REG_UTR10_BASE  = (ITEM2Ds15p16)(transMat.m[1][0] * 0x10000 + 1.0f);
    hwReg.REG_UTR11_BASE  = (ITEM2Ds15p16)(transMat.m[1][1] * 0x10000 + 1.0f);
    hwReg.REG_UTR12_BASE  = (ITEM2Ds15p16)(transMat.m[1][2] * 0x10000 + 1.0f);
    hwReg.REG_UTR20_BASE  = (ITEM2Ds15p16)(transMat.m[2][0] * 0x10000 + 1.0f);
    hwReg.REG_UTR21_BASE  = (ITEM2Ds15p16)(transMat.m[2][1] * 0x10000 + 1.0f);
    hwReg.REG_UTR22_BASE  = (ITEM2Ds15p16)(transMat.m[2][2] * 0x10000 + 1.0f);

    // User Inverse Transform
    hwReg.REG_UITR00_BASE = (ITEM2Ds15p16)(transinverseMat.m[0][0] * 0x10000 + 1.0f);
    hwReg.REG_UITR01_BASE = (ITEM2Ds15p16)(transinverseMat.m[0][1] * 0x10000 + 1.0f);
    hwReg.REG_UITR02_BASE = (ITEM2Ds15p16)(transinverseMat.m[0][2] * 0x10000 + 1.0f);
    hwReg.REG_UITR10_BASE = (ITEM2Ds15p16)(transinverseMat.m[1][0] * 0x10000 + 1.0f);
    hwReg.REG_UITR11_BASE = (ITEM2Ds15p16)(transinverseMat.m[1][1] * 0x10000 + 1.0f);
    hwReg.REG_UITR12_BASE = (ITEM2Ds15p16)(transinverseMat.m[1][2] * 0x10000 + 1.0f);
    hwReg.REG_UITR20_BASE = (ITEM2Ds15p16)(transinverseMat.m[2][0] * 0x10000 + 1.0f);
    hwReg.REG_UITR21_BASE = (ITEM2Ds15p16)(transinverseMat.m[2][1] * 0x10000 + 1.0f);
    hwReg.REG_UITR22_BASE = (ITEM2Ds15p16)(transinverseMat.m[2][2] * 0x10000 + 1.0f);

    hwReg.REG_PITR00_BASE = (ITEM2Ds15p16)(filltransinverseMat.m[0][0] * 0x10000 + 1.0f);
    hwReg.REG_PITR01_BASE = (ITEM2Ds15p16)(filltransinverseMat.m[0][1] * 0x10000 + 1.0f);
    hwReg.REG_PITR02_BASE = (ITEM2Ds15p16)(filltransinverseMat.m[0][2] * 0x10000 + 1.0f);
    hwReg.REG_PITR10_BASE = (ITEM2Ds15p16)(filltransinverseMat.m[1][0] * 0x10000 + 1.0f);
    hwReg.REG_PITR11_BASE = (ITEM2Ds15p16)(filltransinverseMat.m[1][1] * 0x10000 + 1.0f);
    hwReg.REG_PITR12_BASE = (ITEM2Ds15p16)(filltransinverseMat.m[1][2] * 0x10000 + 1.0f);
#else
    hwReg.REG_UTR00_BASE  = 0x0000b505;
    hwReg.REG_UTR01_BASE  = 0xffff4afd;
    hwReg.REG_UTR02_BASE  = 0x012996e1;
    hwReg.REG_UTR10_BASE  = 0x0000b505;
    hwReg.REG_UTR11_BASE  = 0x0000b505;
    hwReg.REG_UTR12_BASE  = 0xffd9d70d;
    hwReg.REG_UTR20_BASE  = 0x00000001;
    hwReg.REG_UTR21_BASE  = 0x00000001;
    hwReg.REG_UTR22_BASE  = 0x00010001;

    hwReg.REG_UITR00_BASE = 0x0000b505;
    hwReg.REG_UITR01_BASE = 0x0000b505;
    hwReg.REG_UITR02_BASE = 0xff488e45;
    hwReg.REG_UITR10_BASE = 0xffff4afd;
    hwReg.REG_UITR11_BASE = 0x0000b505;
    hwReg.REG_UITR12_BASE = 0x00ed6921;
    hwReg.REG_UITR20_BASE = 0x00000001;
    hwReg.REG_UITR21_BASE = 0x00000001;
    hwReg.REG_UITR22_BASE = 0x00010001;

    hwReg.REG_PITR00_BASE = 0x00010001;
    hwReg.REG_PITR01_BASE = 0x00000001;
    hwReg.REG_PITR02_BASE = 0xff310001;
    hwReg.REG_PITR10_BASE = 0x00000001;
    hwReg.REG_PITR11_BASE = 0x00010001;
    hwReg.REG_PITR12_BASE = 0xff180001;
#endif
    /* Set image object ID */
    if (hwWaitObjID == ITEM2D_TRUE)
    {
        iteM2dHardwareGenObjectID();
    }
    hwReg.REG_BID2_BASE    = iteM2dHardwareGetCurrentObjectID();
    destSurface->bitbltId2 = hwReg.REG_BID2_BASE;
    srcSurface->bitbltId2  = hwReg.REG_BID2_BASE;

    /* Set HW to cmdq */
    //dumpflag = 1;
    iteM2dHardwareFire(&hwReg);
    //while(1);
    if (hwWaitObjID == ITEM2D_TRUE)
    {
        iteM2dHardwareWaitObjID(iteM2dHardwareGetCurrentObjectID());
    }

    free(p.pathCommand.items);

    return MMP_SUCCESS;
}
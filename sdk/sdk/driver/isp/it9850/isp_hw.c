#include "isp_hw.h"
#include "isp_reg.h"
#include "isp_util.h"

//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================
MMP_UINT8 ISPTilingTable[5][32] =
{ {  0,  1,  2,  9, 10, 11, 12,  3,  4,  5, 13,  6, 14,  7,  8, 15,  // pitch = 512
     16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
  {  0,  1,  2, 10, 11, 12, 13,  3,  4,  5, 14,  6, 15,  7,  8,  9,  // pitch = 1024
     16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
  {  0,  1,  2, 11, 12, 13, 14,  3,  4,  5, 15,  6, 16,  7,  8,  9,  // pitch = 2048
     10, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
  {  0,  1,  2, 12, 13, 14, 15,  3,  4,  5, 16,  6, 7,  8,  9,  10,  // pitch = 4096
     11, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
  //{  0,  1,  2, 12, 13, 14, 15,  3,  4,  5, 16,  6, 17,  7,  8,  9,  // pitch = 4096
  //   10, 11, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },     
  {  0,  1,  2, 13, 14, 15, 16,  3,  4,  5, 17,  6, 18,  7,  8,  9,  // pitch = 8096
     10, 11, 12, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 } };

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
void
ISP_LogReg(
    void)
{
    MMP_UINT16 reg, p;
    MMP_UINT   i, j, count;

    reg   = ISP_REG_BASE;
    count = (0x700 - reg) / sizeof(MMP_UINT16);

    j     = 0;
    p     = reg;
    //printf("\r\n");
    printf("\n\t   0    2    4    6    8    A    C    E\r\n");

    for (i = 0; i < count; ++i)
    {
        MMP_UINT16 value = 0;

        isp_ReadHwReg(p, &value);
        if (j == 0)
            printf("0x%04X:", p);

        printf(" %04X", value);

        if (j >= 7)
        {
            printf("\r\n");
            j = 0;
        }
        else
            j++;

        p += 2;
    }

    if (j > 0)
        printf("\r\n");
}

//=============================================================================
/**
 * Clear ISP Interrupt.
 */
//=============================================================================
void
ISP_ClearInterrupt_Reg(
    void)
{
    MMP_UINT16 Value = 0;

    Value = ((0x1 & ISP_BIT_ISP_INTERRUPT_CLEAR) << ISP_SHT_ISP_INTERRUPT_CLEAR);

    isp_WriteHwReg(ISP_REG_SET500, (MMP_UINT16)Value);
}

//=============================================================================
/**
 * Driver Fire ISP Engine.
 */
//=============================================================================
void
ISP_DriverFire_Reg(
    void)
{
    isp_WriteHwReg(ISP_REG_SET500, (MMP_UINT16)((0x1 & ISP_BIT_DRIVER_FIRE_EN) << ISP_SHT_DRIVER_FIRE_EN));
}

//=============================================================================
/**
 * Set Input Format
 */
//=============================================================================
void
ISP_SetInputParameter_Reg(
    const ISP_INPUT_INFO *pInInfo)
{
    MMP_UINT16 Value = 0;

    //Set ISP_REG_SET502
    Value = ((pInInfo->EnableRemapUVAddr   & ISP_BIT_REMAP_CHROMAADDR_EN) << ISP_SHT_REMAP_CHROMAADDR_EN) |
            ((pInInfo->EnableRemapYAddr    & ISP_BIT_REMAP_LUMAADDR_EN)   << ISP_SHT_REMAP_LUMAADDR_EN) |
            ((pInInfo->EnableInYUV255Range & ISP_BIT_IN_YUV255RANGE_EN)   << ISP_SHT_IN_YUV255RANGE_EN) |
            ((pInInfo->UVRepeatMode        & ISP_BIT_UVREPEAT_MODE)       << ISP_SHT_UVREPEAT_MODE) |
            ((pInInfo->EnableCCFun         & ISP_BIT_COLOR_CORRECT_EN)    << ISP_SHT_COLOR_CORRECT_EN) |
            ((pInInfo->EnableCSFun         & ISP_BIT_COLOR_SPECE_EN)      << ISP_SHT_COLOR_SPECE_EN) |
            ((pInInfo->EnableDSFun         & ISP_BIT_DOWN_SAMPLE_EN)      << ISP_SHT_DOWN_SAMPLE_EN);

    isp_WriteHwReg(ISP_REG_SET502, (MMP_UINT16)Value);

    Value = (pInInfo->PlaneFormat & ISP_BIT_IN_PLANE_FORMAT) << ISP_SHT_IN_PLANE_FORMAT;

    isp_WriteHwReg(ISP_REG_INPUT_FORMAT, (MMP_UINT16)Value);
}

//=============================================================================
/**
 * Set ISP input buffer relate parameters.
 */
//=============================================================================
void
ISP_SetInputBuf_Reg(
    const ISP_INPUT_INFO *pInInfo)
{
    MMP_UINT16 Value = 0;

    isp_WriteHwReg(ISP_REG_INPUT_WIDTH,    (MMP_UINT16)(pInInfo->SrcWidth  & ISP_BIT_INPUT_WIDTH));
    isp_WriteHwReg(ISP_REG_INPUT_HEIGHT,   (MMP_UINT16)(pInInfo->SrcHeight & ISP_BIT_INPUT_HEIGHT));
    isp_WriteHwReg(ISP_REG_INPUT_PITCH_Y,  (MMP_UINT16)(pInInfo->PitchY    & ISP_BIT_INPUT_PITCH_Y));
    isp_WriteHwReg(ISP_REG_INPUT_PITCH_UV, (MMP_UINT16)(pInInfo->PitchUV   & ISP_BIT_INPUT_PITCH_UV));
}

//=============================================================================
/**
 * Set ISP input buffer address relate parameters
 */
//=============================================================================
void
ISP_SetInputAddr_Reg(
    const ISP_INPUT_INFO *pInInfo)
{
    MMP_UINT32 Value        = 0;
    MMP_UINT32 vramBaseAddr = (MMP_UINT32)isp_GetVramBaseAddr();

    //CurFrame
    Value = (MMP_UINT32)pInInfo->AddrY - vramBaseAddr;
    isp_WriteHwReg(ISP_REG_INPUT_ADDR_YL, (MMP_UINT16)(Value & ISP_BIT_INPUT_ADDR_L));
    isp_WriteHwReg(ISP_REG_INPUT_ADDR_YH, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));

    Value = (MMP_UINT32)pInInfo->AddrU - vramBaseAddr;
    isp_WriteHwReg(ISP_REG_INPUT_ADDR_UL, (MMP_UINT16)(Value & ISP_BIT_INPUT_ADDR_L));
    isp_WriteHwReg(ISP_REG_INPUT_ADDR_UH, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));

    Value = (MMP_UINT32)pInInfo->AddrV - vramBaseAddr;
    isp_WriteHwReg(ISP_REG_INPUT_ADDR_VL, (MMP_UINT16)(Value & ISP_BIT_INPUT_ADDR_L));
    isp_WriteHwReg(ISP_REG_INPUT_ADDR_VH, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));

    //PreFrame
    Value = (MMP_UINT32)pInInfo->AddrYp - vramBaseAddr;
    isp_WriteHwReg(ISP_REG_INPUT_ADDR_YPL, (MMP_UINT16)(Value & ISP_BIT_INPUT_ADDR_L));
    isp_WriteHwReg(ISP_REG_INPUT_ADDR_YPH, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));
}

//=============================================================================
/**
 * Set Remap Y Address
 */
//=============================================================================
void
ISP_SetRemapYAddress_Reg(
    const MMP_UINT8 tableIdx)
{
    MMP_UINT32 Value = 0;

    Value = ((ISPTilingTable[tableIdx][4]) << 8) | (ISPTilingTable[tableIdx][3]);
    isp_WriteHwReg(ISP_REG_MAPADR_Y_0403, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][6]) << 8) | (ISPTilingTable[tableIdx][5]);
    isp_WriteHwReg(ISP_REG_MAPADR_Y_0605, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][8]) << 8) | (ISPTilingTable[tableIdx][7]);
    isp_WriteHwReg(ISP_REG_MAPADR_Y_0807, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][10]) << 8) | (ISPTilingTable[tableIdx][9]);
    isp_WriteHwReg(ISP_REG_MAPADR_Y_1009, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][12]) << 8) | (ISPTilingTable[tableIdx][11]);
    isp_WriteHwReg(ISP_REG_MAPADR_Y_1211, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][14]) << 8) | (ISPTilingTable[tableIdx][13]);
    isp_WriteHwReg(ISP_REG_MAPADR_Y_1413, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][16]) << 8) | (ISPTilingTable[tableIdx][15]);
    isp_WriteHwReg(ISP_REG_MAPADR_Y_1615, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][18]) << 8) | (ISPTilingTable[tableIdx][17]);
    isp_WriteHwReg(ISP_REG_MAPADR_Y_1817, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][20]) << 8) | (ISPTilingTable[tableIdx][19]);
    isp_WriteHwReg(ISP_REG_MAPADR_Y_2019, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][22]) << 8) | (ISPTilingTable[tableIdx][21]);
    isp_WriteHwReg(ISP_REG_MAPADR_Y_2221, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][24]) << 8) | (ISPTilingTable[tableIdx][23]);
    isp_WriteHwReg(ISP_REG_MAPADR_Y_2423, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][26]) << 8) | (ISPTilingTable[tableIdx][25]);
    isp_WriteHwReg(ISP_REG_MAPADR_Y_2625, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][28]) << 8) | (ISPTilingTable[tableIdx][27]);
    isp_WriteHwReg(ISP_REG_MAPADR_Y_2827, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][30]) << 8) | (ISPTilingTable[tableIdx][29]);
    isp_WriteHwReg(ISP_REG_MAPADR_Y_3029, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = (ISPTilingTable[tableIdx][31]);
    isp_WriteHwReg(ISP_REG_MAPADR_Y_XX31, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
}

//=============================================================================
/**
 * Set Remap UV Address
 */
//=============================================================================
void
ISP_SetRemapUVAddress_Reg(
    const MMP_UINT8 tableIdx)
{
    MMP_UINT32 Value = 0;

    Value = ((ISPTilingTable[tableIdx][4]) << 8) | (ISPTilingTable[tableIdx][3]);
    isp_WriteHwReg(ISP_REG_MAPADR_UV_0403, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][6]) << 8) | (ISPTilingTable[tableIdx][5]);
    isp_WriteHwReg(ISP_REG_MAPADR_UV_0605, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][8]) << 8) | (ISPTilingTable[tableIdx][7]);
    isp_WriteHwReg(ISP_REG_MAPADR_UV_0807, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][10]) << 8) | (ISPTilingTable[tableIdx][9]);
    isp_WriteHwReg(ISP_REG_MAPADR_UV_1009, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][12]) << 8) | (ISPTilingTable[tableIdx][11]);
    isp_WriteHwReg(ISP_REG_MAPADR_UV_1211, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][14]) << 8) | (ISPTilingTable[tableIdx][13]);
    isp_WriteHwReg(ISP_REG_MAPADR_UV_1413, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][16]) << 8) | (ISPTilingTable[tableIdx][15]);
    isp_WriteHwReg(ISP_REG_MAPADR_UV_1615, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][18]) << 8) | (ISPTilingTable[tableIdx][17]);
    isp_WriteHwReg(ISP_REG_MAPADR_UV_1817, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][20]) << 8) | (ISPTilingTable[tableIdx][19]);
    isp_WriteHwReg(ISP_REG_MAPADR_UV_2019, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][22]) << 8) | (ISPTilingTable[tableIdx][21]);
    isp_WriteHwReg(ISP_REG_MAPADR_UV_2221, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][24]) << 8) | (ISPTilingTable[tableIdx][23]);
    isp_WriteHwReg(ISP_REG_MAPADR_UV_2423, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][26]) << 8) | (ISPTilingTable[tableIdx][25]);
    isp_WriteHwReg(ISP_REG_MAPADR_UV_2625, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][28]) << 8) | (ISPTilingTable[tableIdx][27]);
    isp_WriteHwReg(ISP_REG_MAPADR_UV_2827, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = ((ISPTilingTable[tableIdx][30]) << 8) | (ISPTilingTable[tableIdx][29]);
    isp_WriteHwReg(ISP_REG_MAPADR_UV_3029, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));

    Value = (ISPTilingTable[tableIdx][31]);
    isp_WriteHwReg(ISP_REG_MAPADR_UV_XX31, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
}

//=============================================================================
/**
 * Set Deinterlace Parameter.
 */
//=============================================================================
void
ISP_SetDeInterlaceParam_Reg(
    ISP_DEVICE                 ptDev,
    const ISP_DEINTERLACE_CTRL *pDeInterlace)
{
    MMP_UINT16 Value = 0;

    Value = ((pDeInterlace->LowLevelBypassBlend & ISP_BIT_LOWLEVELBYPASSBLEND) << ISP_SHT_LOWLEVELBYPASSBLEND) |
            ((pDeInterlace->Disable30MotionDetect & ISP_BIT_30LOWLEVELEDGE_DISABLE) << ISP_SHT_30LOWLEVELEDGE_DISABLE) |
            ((pDeInterlace->EnLowLevelOutside & ISP_BIT_LOWLEVELOUTSIDE_EN) << ISP_SHT_LOWLEVELOUTSIDE_EN) |
            ((pDeInterlace->LowLevelMode & ISP_BIT_LOWLEVELMODE) << ISP_SHT_LOWLEVELMODE) |
            ((pDeInterlace->EnLowLevelEdge & ISP_BIT_LOWLEVELEDGE_EN) << ISP_SHT_LOWLEVELEDGE_EN) |
            ((pDeInterlace->EnChromaEdgeDetect & ISP_BIT_CHROMA_EDGEDET_EN) << ISP_SHT_CHROMA_EDGEDET_EN) |
            ((pDeInterlace->EnLummaEdgeDetect & ISP_BIT_LUMA_EDGEDET_EN) << ISP_SHT_LUMA_EDGEDET_EN) |
            ((pDeInterlace->Enable & ISP_BIT_DEINTERLACE_EN) << ISP_SHT_DEINTERLACE_EN) |
            ((pDeInterlace->DeinterMode & ISP_BIT_2D_DEINTER_MODE_EN) << ISP_SHT_2D_DEINTER_MODE_EN) |
            ((pDeInterlace->EnSrcBottomFieldFirst & ISP_BIT_SRC_BOTTOM_FIELD_FIRST) << ISP_SHT_SRC_BOTTOM_FIELD_FIRST) |
            ((pDeInterlace->EnDeinterBottomField & ISP_BIT_DEINTER_BOTTOM_EN) << ISP_SHT_DEINTER_BOTTOM_EN) |
            ((pDeInterlace->EnSrcLPF & ISP_BIT_SRC_LPFITR_EN) << ISP_SHT_SRC_LPFITR_EN) |
            ((pDeInterlace->EnUV2DMethod & ISP_BIT_UV2D_METHOD_EN) << ISP_SHT_UV2D_METHOD_EN);

    isp_WriteHwReg(ISP_REG_SET_DEINTERLACE,      (MMP_UINT16)Value);

    isp_WriteHwReg(ISP_REG_LOWLEVELEDGE_START_X, (MMP_UINT16)pDeInterlace->LowLevelPosX);
    isp_WriteHwReg(ISP_REG_LOWLEVELEDGE_START_Y, (MMP_UINT16)pDeInterlace->LowLevelPosY);
    isp_WriteHwReg(ISP_REG_LOWLEVELEDGE_WIDTH,   (MMP_UINT16)pDeInterlace->LowLevelWidth);
    isp_WriteHwReg(ISP_REG_LOWLEVELEDGE_HEIGHT,  (MMP_UINT16)pDeInterlace->LowLevelHeight);

    if (pDeInterlace->DeinterMode == DEINTER3D)
        ISP_Set3DDeInterlaceParm_Reg(ptDev);
    else if (pDeInterlace->DeinterMode == DEINTER2D)
        ISP_Set2DDeInterlaceParam_Reg(ptDev);
}

//=============================================================================
/**
 * Set 3D-Deinterlace parameters.
 */
//=============================================================================
void
ISP_Set3DDeInterlaceParm_Reg(
    ISP_DEVICE ptDev)
{
    MMP_UINT16           Value         = 0;
    ISP_CONTEXT          *pISPctxt     = (ISP_CONTEXT *)ptDev;
    ISP_DEINTERLACE_CTRL *pDeInterlace = &pISPctxt->DeInterlace;

    //Parameter 1
    Value = ((pDeInterlace->MDThreshold_High & ISP_BIT_3D_MDTHRED_HIGH) << ISP_SHT_3D_MDTHRED_HIGH) |
            ((pDeInterlace->MDThreshold_Low & ISP_BIT_3D_MDTHRED_LOW) << ISP_SHT_3D_MDTHRED_LOW);

    isp_WriteHwReg(ISP_REG_3D_DEINTER_PARM_1, (MMP_UINT16)Value);

    //Parameter 2
    Value = ((pDeInterlace->DisableMV_A & ISP_BIT_DISABLE_MOTIONVALUE_A) << ISP_SHT_DISABLE_MOTIONVALUE_A) |
            ((pDeInterlace->DisableMV_B & ISP_BIT_DISABLE_MOTIONVALUE_B) << ISP_SHT_DISABLE_MOTIONVALUE_B) |
            ((pDeInterlace->DisableMV_C & ISP_BIT_DISABLE_MOTIONVALUE_C) << ISP_SHT_DISABLE_MOTIONVALUE_C) |
            ((pDeInterlace->DisableMV_D & ISP_BIT_DISABLE_MOTIONVALUE_D) << ISP_SHT_DISABLE_MOTIONVALUE_D) |
            ((pDeInterlace->DisableMV_E & ISP_BIT_DISABLE_MOTIONVALUE_E) << ISP_SHT_DISABLE_MOTIONVALUE_E) |
            ((pDeInterlace->DisableMV_F & ISP_BIT_DISABLE_MOTIONVALUE_F) << ISP_SHT_DISABLE_MOTIONVALUE_F) |
            ((pDeInterlace->DisableMV_G & ISP_BIT_DISABLE_MOTIONVALUE_G) << ISP_SHT_DISABLE_MOTIONVALUE_G) |
            ((pDeInterlace->EnLPFWeight & ISP_BIT_LPF_WEIGHT_EN) << ISP_SHT_LPF_WEIGHT_EN) |
            ((pDeInterlace->EnLPFBlend & ISP_BIT_LPF_BLEND_EN) << ISP_SHT_LPF_BLEND_EN) |
            ((pDeInterlace->MDThreshold_Step & ISP_BIT_3D_MDTHRED_STEP) << ISP_SHT_3D_MDTHRED_STEP);

    isp_WriteHwReg(ISP_REG_3D_DEINTER_PARM_2, (MMP_UINT16)Value);

    //Parameter 3
    Value = ((pDeInterlace->EnLPFStaticPixel & ISP_BIT_LPF_STATICPIXEL_EN) << ISP_SHT_LPF_STATICPIXEL_EN);

    isp_WriteHwReg(ISP_REG_3D_DEINTER_PARM_3, (MMP_UINT16)Value);
}

//=============================================================================
/**
 * Set 2D-Deinterlace parameters.
 */
//=============================================================================
void
ISP_Set2DDeInterlaceParam_Reg(
    ISP_DEVICE ptDev)
{
    MMP_UINT16           Value         = 0;
    ISP_CONTEXT          *pISPctxt     = (ISP_CONTEXT *)ptDev;
    ISP_DEINTERLACE_CTRL *pDeInterlace = &pISPctxt->DeInterlace;

    Value = ((pDeInterlace->D2EdgeBlendWeight & ISP_BIT_2D_EDGE_WEIGHT) << ISP_SHT_2D_EDGE_WEIGHT) |
            ((pDeInterlace->D2OrgBlendWeight & ISP_BIT_2D_ORG_WEIGHT) << ISP_SHT_2D_ORG_WEIGHT);

    isp_WriteHwReg(ISP_REG_2D_DEINTER_PARM_1, (MMP_UINT16)Value);
}

//=============================================================================
/**
 * YUV to RGB transfer matrix.
 */
//=============================================================================
void
ISP_SetYUVtoRGBMatrix_Reg(
    const ISP_YUV_TO_RGB *pYUVtoRGB)
{
    isp_WriteHwReg(ISP_REG_YUV_TO_RGB_11,      (MMP_UINT16)(pYUVtoRGB->_11 & ISP_BIT_YUV_TO_RGB));
    isp_WriteHwReg(ISP_REG_YUV_TO_RGB_12,      (MMP_UINT16)(pYUVtoRGB->_12 & ISP_BIT_YUV_TO_RGB));
    isp_WriteHwReg(ISP_REG_YUV_TO_RGB_13,      (MMP_UINT16)(pYUVtoRGB->_13 & ISP_BIT_YUV_TO_RGB));

    isp_WriteHwReg(ISP_REG_YUV_TO_RGB_21,      (MMP_UINT16)(pYUVtoRGB->_21 & ISP_BIT_YUV_TO_RGB));
    isp_WriteHwReg(ISP_REG_YUV_TO_RGB_22,      (MMP_UINT16)(pYUVtoRGB->_22 & ISP_BIT_YUV_TO_RGB));
    isp_WriteHwReg(ISP_REG_YUV_TO_RGB_23,      (MMP_UINT16)(pYUVtoRGB->_23 & ISP_BIT_YUV_TO_RGB));

    isp_WriteHwReg(ISP_REG_YUV_TO_RGB_31,      (MMP_UINT16)(pYUVtoRGB->_31 & ISP_BIT_YUV_TO_RGB));
    isp_WriteHwReg(ISP_REG_YUV_TO_RGB_32,      (MMP_UINT16)(pYUVtoRGB->_32 & ISP_BIT_YUV_TO_RGB));
    isp_WriteHwReg(ISP_REG_YUV_TO_RGB_33,      (MMP_UINT16)(pYUVtoRGB->_33 & ISP_BIT_YUV_TO_RGB));

    isp_WriteHwReg(ISP_REG_YUV_TO_RGB_CONST_R, (MMP_UINT16)(pYUVtoRGB->ConstR & ISP_BIT_YUV_TO_RGB_CONST));
    isp_WriteHwReg(ISP_REG_YUV_TO_RGB_CONST_G, (MMP_UINT16)(pYUVtoRGB->ConstG & ISP_BIT_YUV_TO_RGB_CONST));
    isp_WriteHwReg(ISP_REG_YUV_TO_RGB_CONST_B, (MMP_UINT16)(pYUVtoRGB->ConstB & ISP_BIT_YUV_TO_RGB_CONST));
}

//=============================================================================
/**
 * Set color correction matrix and constant
 */
//=============================================================================
void
ISP_SetCCMatrix_Reg(
    const ISP_COLOR_CORRECTION *pColorCorrect)
{
    isp_WriteHwReg(ISP_REG_CC_IN_OFFSET_R,  (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->OffsetR, 8, 0) & ISP_BIT_IN_OFFSET));
    isp_WriteHwReg(ISP_REG_CC_IN_OFFSET_G,  (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->OffsetG, 8, 0) & ISP_BIT_IN_OFFSET));
    isp_WriteHwReg(ISP_REG_CC_IN_OFFSET_B,  (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->OffsetB, 8, 0) & ISP_BIT_IN_OFFSET));

    isp_WriteHwReg(ISP_REG_COL_COR_11,      (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_11, 4, 8) & ISP_BIT_COL_COR));
    isp_WriteHwReg(ISP_REG_COL_COR_12,      (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_12, 4, 8) & ISP_BIT_COL_COR));
    isp_WriteHwReg(ISP_REG_COL_COR_13,      (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_13, 4, 8) & ISP_BIT_COL_COR));
    isp_WriteHwReg(ISP_REG_COL_COR_21,      (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_21, 4, 8) & ISP_BIT_COL_COR));
    isp_WriteHwReg(ISP_REG_COL_COR_22,      (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_22, 4, 8) & ISP_BIT_COL_COR));
    isp_WriteHwReg(ISP_REG_COL_COR_23,      (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_23, 4, 8) & ISP_BIT_COL_COR));
    isp_WriteHwReg(ISP_REG_COL_COR_31,      (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_31, 4, 8) & ISP_BIT_COL_COR));
    isp_WriteHwReg(ISP_REG_COL_COR_32,      (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_32, 4, 8) & ISP_BIT_COL_COR));
    isp_WriteHwReg(ISP_REG_COL_COR_33,      (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_33, 4, 8) & ISP_BIT_COL_COR));

    isp_WriteHwReg(ISP_REG_COL_COR_DELTA_R, (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->DeltaR, 8, 0) & ISP_BIT_COL_CORR_DELTA));
    isp_WriteHwReg(ISP_REG_COL_COR_DELTA_G, (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->DeltaG, 8, 0) & ISP_BIT_COL_CORR_DELTA));
    isp_WriteHwReg(ISP_REG_COL_COR_DELTA_B, (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->DeltaB, 8, 0) & ISP_BIT_COL_CORR_DELTA));
}

//=============================================================================
/*
 * Set Pre-Scale Register
 */
//=============================================================================
void
ISP_SetPreScaleParam_Reg(
    const ISP_PRESCALE_CTRL *pPreScaleFun)
{
    MMP_UINT32 HCI;

    HCI = ISP_FloatToFix(pPreScaleFun->HCI, 6, 14);

    //HCI
    isp_WriteHwReg(ISP_REG_PRESCALE_HCI_L, (MMP_UINT16)(HCI & ISP_BIT_PRESCALE_HCI_L));
    isp_WriteHwReg(ISP_REG_PRESCALE_HCI_H, (MMP_UINT16)((HCI >> 16) & ISP_BIT_PRESCALE_HCI_H));

    //PreScale Output Width
    isp_WriteHwReg(ISP_REG_PRESCALE_WIDTH, (MMP_UINT16)(pPreScaleFun->DstWidth & ISP_BIT_PRESCALE_WIDTH));
}

//=============================================================================
/**
 * Set Pre-Scale Weight.
 */
//=============================================================================
void
ISP_SetPreScaleMatrix_Reg(
    const ISP_PRESCALE_CTRL *pPreScaleFun)
{
    MMP_UINT32 Value = 0;

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[0][0], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[0][1], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    isp_WriteHwReg(ISP_REG_PRESCALE_WX0100, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[0][2], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[0][3], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    isp_WriteHwReg(ISP_REG_PRESCALE_WX0302, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[1][0], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[1][1], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    isp_WriteHwReg(ISP_REG_PRESCALE_WX1110, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[1][2], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[1][3], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    isp_WriteHwReg(ISP_REG_PRESCALE_WX1312, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[2][0], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[2][1], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    isp_WriteHwReg(ISP_REG_PRESCALE_WX2120, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[2][2], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[2][3], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    isp_WriteHwReg(ISP_REG_PRESCALE_WX2322, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[3][0], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[3][1], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    isp_WriteHwReg(ISP_REG_PRESCALE_WX3130, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[3][2], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[3][3], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    isp_WriteHwReg(ISP_REG_PRESCALE_WX3332, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[4][0], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[4][1], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    isp_WriteHwReg(ISP_REG_PRESCALE_WX4140, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[4][2], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[4][3], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    isp_WriteHwReg(ISP_REG_PRESCALE_WX4342, (MMP_UINT16)Value);
}

void
ISP_SetIntPreScaleMatrix_Reg(
    MMP_UINT8 WeightMatX[][ISP_SCALE_TAP])
{
    MMP_UINT16 Value;

    Value = ((WeightMatX[0][0] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[0][1] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    isp_WriteHwReg(ISP_REG_PRESCALE_WX0100, (MMP_UINT16)Value);

    Value = ((WeightMatX[0][2] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[0][3] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    isp_WriteHwReg(ISP_REG_PRESCALE_WX0302, (MMP_UINT16)Value);

    Value = ((WeightMatX[1][0] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[1][1] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    isp_WriteHwReg(ISP_REG_PRESCALE_WX1110, (MMP_UINT16)Value);

    Value = ((WeightMatX[1][2] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[1][3] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    isp_WriteHwReg(ISP_REG_PRESCALE_WX1312, (MMP_UINT16)Value);

    Value = ((WeightMatX[2][0] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[2][1] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    isp_WriteHwReg(ISP_REG_PRESCALE_WX2120, (MMP_UINT16)Value);

    Value = ((WeightMatX[2][2] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[2][3] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    isp_WriteHwReg(ISP_REG_PRESCALE_WX2322, (MMP_UINT16)Value);

    Value = ((WeightMatX[3][0] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[3][1] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    isp_WriteHwReg(ISP_REG_PRESCALE_WX3130, (MMP_UINT16)Value);

    Value = ((WeightMatX[3][2] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[3][3] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    isp_WriteHwReg(ISP_REG_PRESCALE_WX3332, (MMP_UINT16)Value);

    Value = ((WeightMatX[4][0] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[4][1] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    isp_WriteHwReg(ISP_REG_PRESCALE_WX4140, (MMP_UINT16)Value);

    Value = ((WeightMatX[4][2] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[4][3] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    isp_WriteHwReg(ISP_REG_PRESCALE_WX4342, (MMP_UINT16)Value);
}

//=============================================================================
/*
 * Set Scale Factor
 */
//=============================================================================
void
ISP_SetScaleParam_Reg(
    const ISP_SCALE_CTRL *pScaleFun)
{
    MMP_UINT16 Value = 0;
    MMP_UINT32 HCI;
    MMP_UINT32 VCI;

    HCI = ISP_FloatToFix(pScaleFun->HCI, 6, 14);
    VCI = ISP_FloatToFix(pScaleFun->VCI, 6, 14);

    //HCI
    isp_WriteHwReg(ISP_REG_SCALE_HCI_L, (MMP_UINT16)(HCI & ISP_BIT_SCALE_L));
    isp_WriteHwReg(ISP_REG_SCALE_HCI_H, (MMP_UINT16)((HCI >> 16) & ISP_BIT_SCALE_H));

    //VCI
    isp_WriteHwReg(ISP_REG_SCALE_VCI_L, (MMP_UINT16)(VCI & ISP_BIT_SCALE_L));
    isp_WriteHwReg(ISP_REG_SCALE_VCI_H, (MMP_UINT16)((VCI >> 16) & ISP_BIT_SCALE_H));

    //Background Area Color
    Value = ((pScaleFun->BGColorR & ISP_BIT_BGCOLOR) << ISP_SHT_BGCOLOR_R) |
            ((pScaleFun->BGColorG & ISP_BIT_BGCOLOR) << ISP_SHT_BGCOLOR_G);

    isp_WriteHwReg(ISP_REG_BGCOLOR_RG,      (MMP_UINT16)Value);
    isp_WriteHwReg(ISP_REG_BGCOLOR_B,       (MMP_UINT16)(pScaleFun->BGColorB & ISP_BIT_BGCOLOR));

    //Scale Output Position X and Y
    isp_WriteHwReg(ISP_REG_SCALE_DSTPOS_X,  (MMP_UINT16)(pScaleFun->DstPosX & ISP_BIT_SCALE_DSTPOS_X));
    isp_WriteHwReg(ISP_REG_SCALE_DSTPOS_Y,  (MMP_UINT16)(pScaleFun->DstPosY & ISP_BIT_SCALE_DSTPOS_Y));

    //Scale Output Width and Height
    isp_WriteHwReg(ISP_REG_SCALE_DSTWIDTH,  (MMP_UINT16)(pScaleFun->DstWidth & ISP_BIT_SCALE_DSTWIDTH));
    isp_WriteHwReg(ISP_REG_SCALE_DSTHEIGHT, (MMP_UINT16)(pScaleFun->DstHeight & ISP_BIT_SCALE_DSTHEIGHT));
}

//=============================================================================
/**
 * Set Frame Function 0
 */
//=============================================================================
void
ISP_SetFrameFun0_Reg(
    const ISP_FRMFUN_CTRL *pFrameFun)
{
    MMP_UINT32 Value        = 0;
    MMP_UINT32 vramBaseAddr = (MMP_UINT32)isp_GetVramBaseAddr();

    //Starting address
    Value = (MMP_UINT32)pFrameFun->Addr - vramBaseAddr; // byte align

    isp_WriteHwReg(ISP_REG_FRMFUN_0_ADDR_L,  (MMP_UINT16)(Value & ISP_BIT_FRMFUN_ADDR_L));
    isp_WriteHwReg(ISP_REG_FRMFUN_0_ADDR_H,  (MMP_UINT16)((Value >> 16) & ISP_BIT_FRMFUN_ADDR_H));

    //width, height, pitch
    isp_WriteHwReg(ISP_REG_FRMFUN_0_WIDTH,   (MMP_UINT16)(pFrameFun->Width & ISP_BIT_FRMFUN_WIDTH));
    isp_WriteHwReg(ISP_REG_FRMFUN_0_HEIGHT,  (MMP_UINT16)(pFrameFun->Height & ISP_BIT_FRMFUN_HEIGHT));
    isp_WriteHwReg(ISP_REG_FRMFUN_0_PITCH,   (MMP_UINT16)(pFrameFun->Pitch & ISP_BIT_FRMFUN_PITCH));

    //start X/Y
    isp_WriteHwReg(ISP_REG_FRMFUN_0_START_X, (MMP_UINT16)(pFrameFun->StartX & ISP_BIT_FRMFUN_START_X));
    isp_WriteHwReg(ISP_REG_FRMFUN_0_START_Y, (MMP_UINT16)(pFrameFun->StartY & ISP_BIT_FRMFUN_START_Y));

    //color key
    Value = ((pFrameFun->ColorKeyR & ISP_BIT_FRMFUN_KEY) << ISP_SHT_FRMFUN_KEY_R) |
            ((pFrameFun->ColorKeyG & ISP_BIT_FRMFUN_KEY) << ISP_SHT_FRMFUN_KEY_G);

    isp_WriteHwReg(ISP_REG_FRMFUN_0_KEY_RG, (MMP_UINT16)Value);
    isp_WriteHwReg(ISP_REG_FRMFUN_0_KEY_B,  (MMP_UINT16)(pFrameFun->ColorKeyB & ISP_BIT_FRMFUN_KEY));

    //Constant Alpha Value
    isp_WriteHwReg(ISP_REG_CONST_ALPHA_0,   (MMP_UINT16)(pFrameFun->ConstantAlpha & ISP_BIT_CONST_ALPHA));

    //format ARGB4444 or Constant Alpha with RGB565
    if (pFrameFun->Format == ARGB4444 || pFrameFun->Format == ARGB8888)
    {
        Value = ((0x1 & ISP_BIT_FRMFUN_ALPHA_BLEND_EN) << ISP_SHT_FRMFUN_ALPHA_BLEND_EN) |
                ((0x1 & ISP_BIT_FRMFUN_MODE) << ISP_SHT_FRMFUN_MODE);
    }
    else if (pFrameFun->Format == CARGB565)
    {
        if (pFrameFun->ConstantAlpha != 0)
        {
            Value = ((0x1 & ISP_BIT_FRMFUN_ALPHA_BLEND_EN) << ISP_SHT_FRMFUN_ALPHA_BLEND_EN) |
                    ((0x0 & ISP_BIT_FRMFUN_MODE) << ISP_SHT_FRMFUN_MODE);
        }
        else
        {
            Value = ((0x0 & ISP_BIT_FRMFUN_ALPHA_BLEND_EN) << ISP_SHT_FRMFUN_ALPHA_BLEND_EN) |
                    ((0x0 & ISP_BIT_FRMFUN_MODE) << ISP_SHT_FRMFUN_MODE);
        }
    }

    //FrameFun Enable
    Value = Value |
            ((pFrameFun->Enable & ISP_BIT_FRMFUN_EN) << ISP_SHT_FRMFUN_EN);

    //Enable frame function, set format
    isp_WriteHwReg(ISP_REG_SET_FRMFUN_0, (MMP_UINT16)Value);
}

//=============================================================================
/**
 * Set Output Format
 */
//=============================================================================
void
ISP_SetOutParameter_Reg(
    const ISP_OUTPUT_INFO *pOutInfo)
{
    MMP_UINT16 Value = 0;

    //Set ISP_REG_OUTPUT_FORMAT
    Value = ((pOutInfo->RGBFormat & ISP_BIT_OUT_RGB_FORMAT) << ISP_SHT_OUT_RGB_FORMAT) |
            ((pOutInfo->DitherMode & ISP_BIT_OUT_DITHER_MODE) << ISP_SHT_OUT_DITHER_MODE);

    isp_WriteHwReg(ISP_REG_OUTPUT_FORMAT, (MMP_UINT16)Value);
}

//=============================================================================
/**
 * Set Output Information
 */
//=============================================================================
void
ISP_SetOutBufInfo_Reg(
    const ISP_OUTPUT_INFO *pOutInfo)
{
    //width, height, pitch
    isp_WriteHwReg(ISP_REG_OUT_WIDTH,  (MMP_UINT16)(pOutInfo->Width & ISP_BIT_OUT_WIDTH ));
    isp_WriteHwReg(ISP_REG_OUT_HEIGHT, (MMP_UINT16)(pOutInfo->Height & ISP_BIT_OUT_HEIGHT));
    isp_WriteHwReg(ISP_REG_OUT_PITCH,  (MMP_UINT16)(pOutInfo->Pitch & ISP_BIT_OUT_PITCH));
}

//=============================================================================
/**
 * Set Output Address
 */
//=============================================================================
void
ISP_SetOutAddress_Reg(
    const ISP_OUTPUT_INFO *pOutInfo)
{
    MMP_UINT32 Value        = 0;
    MMP_UINT32 vramBaseAddr = (MMP_UINT32)isp_GetVramBaseAddr();

    //buffer address
    Value = (MMP_UINT32)pOutInfo->Addr - vramBaseAddr; // byte align
    isp_WriteHwReg(ISP_REG_OUT_ADDR_0L, (MMP_UINT16)(Value & ISP_BIT_OUT_ADDR_L));
    isp_WriteHwReg(ISP_REG_OUT_ADDR_0H, (MMP_UINT16)((Value >> 16) & ISP_BIT_OUT_ADDR_H));
}

//=============================================================================
/**
 * Set ISP Engine Mode.
 */
//=============================================================================
void
ISP_SetEngineMode_Reg(
    const ISP_ENGINE_MODE_CTRL *pEngineMode)
{
    MMP_UINT16 Value = 0;

    isp_WriteHwRegMask(ISP_REG_RUNLENENC_PARM, ((pEngineMode->EnableJPEGDECODE & ISP_BIT_JPEGDECODE_MODE) << ISP_SHT_JPEGDECODE_MODE), ISP_BIT_JPEGDECODE_MODE << ISP_SHT_JPEGDECODE_MODE);

    Value = ((pEngineMode->TotalSliceNum & ISP_BIT_TOTALSLICENUM) << ISP_SHT_TOTALSLICENUM);

    isp_WriteHwReg(ISP_REG_SET50C, (MMP_UINT16)Value);
}

//=============================================================================
/**
 * Set Run-Length Encoder
 */
//=============================================================================
void
ISP_SetRunLengthEnc_Reg(
    const ISP_RUNLEN_ENC_CTRL *pCtrl)
{
    MMP_UINT32 Value        = 0;
    MMP_UINT32 vramBaseAddr = (MMP_UINT32)isp_GetVramBaseAddr();

    isp_ReadHwReg(ISP_REG_RUNLENENC_PARM, (MMP_UINT16 *)&Value);

    Value = Value & (ISP_BIT_JPEGDECODE_MODE << ISP_SHT_JPEGDECODE_MODE);

    //Set ISP_REG_RUNLENENC_PARM
    Value = ((pCtrl->RunSize & ISP_BIT_RUNLENENC_RUNSIZE) << ISP_SHT_RUNLENENC_RUNSIZE) |
            ((pCtrl->EnableRejectBit & ISP_BIT_RUNLENENC_REJECT_EN) << ISP_SHT_RUNLENENC_REJECT_EN) |
            ((pCtrl->UnitSize & ISP_BIT_RUNLENENC_UNITSIZE) << ISP_SHT_RUNLENENC_UNITSIZE) |
            ((pCtrl->Enable & ISP_BIT_RUNLENENC_EN) << ISP_SHT_RUNLENENC_EN) | Value;

    isp_WriteHwReg(ISP_REG_RUNLENENC_PARM, (MMP_UINT16)Value);

    //buffer address
    Value = (MMP_UINT32)pCtrl->Addr - vramBaseAddr; // byte align
    isp_WriteHwReg(ISP_REG_RUNLENENC_ADDR_L,   (MMP_UINT16)(Value & ISP_BIT_RUNLENENC_ADDR_L));
    isp_WriteHwReg(ISP_REG_RUNLENENC_ADDR_H,   (MMP_UINT16)((Value >> 16) & ISP_BIT_RUNLENENC_ADDR_H));

    isp_WriteHwReg(ISP_REG_RUNLENENC_LINEBYTE, (MMP_UINT16)pCtrl->LineByte);
    isp_WriteHwReg(ISP_REG_RUNLENENC_PITCH,    (MMP_UINT16)pCtrl->Pitch);

    Value = pCtrl->MaxBit;
    isp_WriteHwReg(ISP_REG_RUNLENENC_MAXBIT_L, (MMP_UINT16)(Value & ISP_BIT_RUNLENENC_MAXBIT_L));
    isp_WriteHwReg(ISP_REG_RUNLENENC_MAXBIT_H, (MMP_UINT16)((Value >> 16) & ISP_BIT_RUNLENENC_MAXBIT_H));
}

//=============================================================================
/**
 * Wait ISP engine idle!  //for JPG module use
 */
//=============================================================================
ISP_RESULT
ISP_WaitEngineIdle(
    void)
{
    ISP_RESULT result  = ISP_SUCCESS;
    MMP_UINT16 status  = 0;
    MMP_UINT16 timeOut = 0;

    //
    //  Wait ISP engine idle!   0x6FC D[0]  0: idle, 1: busy
    //
    isp_ReadHwReg(ISP_REG_ISP_ENGINE_STATUS, (MMP_UINT16 *)&status);
    while (status & 0x0001)
    {
        isp_sleep(1);
        if (++timeOut > 2000)
        {
            //ISP_LogReg();
            isp_msg_ex(ISP_MSG_TYPE_ERR, "ERROR_ISP_NOT_IDLE \n");
            result = ISP_ERR_NOT_IDLE;
            goto end;
        }
        isp_ReadHwReg(ISP_REG_ISP_ENGINE_STATUS, (MMP_UINT16 *)&status);
    }

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0%x !", __FUNCTION__, result);

    return result;
}

//=============================================================================
/**
 * Wait ISP interrupt idle!
 */
//=============================================================================
ISP_RESULT
ISP_WaitInterruptIdle(
    void)
{
    ISP_RESULT result  = ISP_SUCCESS;
    MMP_UINT16 status  = 0;
    MMP_UINT16 timeOut = 0;

    //
    //  Wait ISP interrupt idle!   0x6FE D[8]  0: idle, 1: busy
    //
    isp_ReadHwReg(ISP_REG_ISP_ENGINE_STATUS_2, (MMP_UINT16 *)&status);
    while (status & 0x0100)
    {
        isp_sleep(1);
        if (++timeOut > 2000)
        {
            //ISP_LogReg();
            isp_msg_ex(ISP_MSG_TYPE_ERR, "ERROR_ISP_INTERRUPT_NOT_IDLE \n");
            result = ISP_ERR_NOT_IDLE;
            goto end;
        }
        isp_ReadHwReg(ISP_REG_ISP_ENGINE_STATUS_2, (MMP_UINT16 *)&status);
    }

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0%x !", __FUNCTION__, result);

    return result;
}

//=============================================================================
/**
 * Set Interrupt Information
 */
//=============================================================================
void
ISP_SetInterruptParameter_Reg(
    const ISP_CONTEXT *pISPctxt)
{
    MMP_UINT16 Value = 0;

    //Set ISP_REG_SET50E
    Value = ((pISPctxt->EnableInterrupt & ISP_BIT_ISP_INTERRUPT_EN)   << ISP_SHT_ISP_INTERRUPT_EN) |
            ((pISPctxt->InterruptMode   & ISP_BIT_ISP_INTERRUPT_MODE) << ISP_SHT_ISP_INTERRUPT_MODE);

    isp_WriteHwReg(ISP_REG_SET50E, (MMP_UINT16)Value);
    //isp_WriteHwRegMask(ISP_REG_SET50E, (pISPctxt->InterruptMode & ISP_BIT_ISP_INTERRUPT_MODE) << ISP_SHT_ISP_INTERRUPT_MODE, (ISP_BIT_ISP_INTERRUPT_MODE << ISP_SHT_ISP_INTERRUPT_MODE));
}

//=============================================================================
/**
 * ISP engine clock related.
 */
//=============================================================================
void
ISP_PowerUp(
    void)
{
    isp_EnginClockOn();
    isp_EnginReset();
}

void
ISP_PowerDown(
    void)
{
    isp_EnginReset();
    isp_EnginClockOff();
}
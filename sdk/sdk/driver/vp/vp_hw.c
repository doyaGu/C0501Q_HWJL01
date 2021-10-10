//#include "host/host.h"
//#include "pal/pal.h"
//#include "sys/sys.h"
//#include "mmp_types.h"

#include "vp/vp_config.h"
#include "vp/vp_hw.h"
#include "vp/vp_reg.h"
#include "vp/vp_util.h"
//=============================================================================
//                Constant Definition
//=============================================================================
#define MMP_ISP_CLOCK_REG_30                0x0030
#define MMP_ISP_CLOCK_REG_32                0x0032
#define MMP_ISP_EN_DIV_ICLK                 0x8000  // [15]
#define MMP_ISP_SRC_ICLK                    0x3000  // [13:12]
#define MMP_ISP_RAT_ICLK                    0x000F  // [ 3: 0]
#define MMP_ISPCMQGBL_RESET                 0x4000  // [14]
#define MMP_ISPGBL_RESET                    0x2000  // [13]
#define MMP_ISP_RESET                       0x1000  // [12]
#define MMP_ISP_EN_N5CLK                    0x0020  // [ 5] AHB clock
#define MMP_ISP_EN_M5CLK                    0x0008  // [ 3] memory clock in ISP
#define MMP_ISP_EN_ICLK                     0x0002  // [ 1] ISP clock
#define MMP_HOST_BUS_CONTROLLER_REG_202     0x0202
#define MMP_HOST_BUS_EN_MMIO_ISP            0x2000

//=============================================================================
//                Macro Definition
//=============================================================================


//=============================================================================
//                Structure Definition
//=============================================================================


//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
void
VP_LogReg(
    void)
{
    MMP_UINT16  reg, p;
    MMP_UINT    i, j, count;

    reg     = VP_REG_BASE;
    count   = (0x06FE - reg) / sizeof(MMP_UINT16);

    j = 0;
    p = reg;
    //printf("\r\n");
    printf( "\n\t   0    2    4    6    8    A    C    E\r\n");

    for(i = 0; i < count; ++i)
    {
        MMP_UINT16 value = 0;

        HOST_ReadRegister(p, &value);
        if( j == 0 )
            printf("0x%04X:", p);

        printf(" %04X", value);

        if( j >= 7 )
        {
            printf("\r\n");
            j = 0;
        }
        else
            j++;

        p += 2;
    }

    if( j > 0 )
        printf("\r\n");

}
//=============================================================================
/**
 * Clear ISP Interrupt.
 */
//=============================================================================
void
VP_ClearInterrupt_Reg(
    void)
{   
    MMP_UINT16  Value = 0;
    
    Value = ((0x1 & VP_BIT_VP_INTERRUPT_CLEAR) << VP_SHT_VP_INTERRUPT_CLEAR);
            
    HOST_WriteRegister(VP_REG_SET500, (MMP_UINT16)Value);
}

//=============================================================================
/**
 * Driver Fire ISP Engine.
 */
//=============================================================================
void
VP_DriverFire_Reg(
    void)
{   
    MMP_UINT16  Value = 0;
    
    Value = ((0x1 & VP_BIT_DRIVER_FIRE_EN) << VP_SHT_DRIVER_FIRE_EN);     
            
    HOST_WriteRegister(VP_REG_SET500, (MMP_UINT16)Value);
}

//=============================================================================
/**
 * Driver Refresh ISP Parameter.
 */
//=============================================================================
void
VP_RefreshFire_Reg(
    void)
{   
    MMP_UINT16  Value = 0;
    
    Value = ((0x1 & VP_BIT_VP_REFRESH_PARM) << VP_SHT_VP_REFRESH_PARM);
            
    HOST_WriteRegister(VP_REG_SET500, (MMP_UINT16)Value);
}

//=============================================================================
/**
* Driver Update ISP Parameter.
*/
//=============================================================================
void
VP_UpdateFire_Reg(
    void)
{
    HOST_WriteRegister(VP_REG_SET500, (MMP_UINT16)((0x1 & VP_BIT_VP_UPDATE_PARM_EN) << VP_SHT_VP_UPDATE_PARM_EN));
}

//=============================================================================
/**
* Set Input Format
*/
//=============================================================================
void
VP_SetInputParameter_Reg(
    const VP_INPUT_INFO    *pInInfo)
{
    MMP_UINT16  Value = 0;

    Value = ((pInInfo->EnableReadMemoryMode & VP_BIT_READ_MEM_MODE_EN) << VP_SHT_READ_MEM_MODE_EN) | 
            ((pInInfo->EnableInYUV255Range & VP_BIT_IN_YUV255RANGE_EN) << VP_SHT_IN_YUV255RANGE_EN) | 
            ((pInInfo->EnableCCFun & VP_BIT_COLOR_CORRECT_EN) << VP_SHT_COLOR_CORRECT_EN) |    
            ((pInInfo->NVFormat & VP_BIT_IN_NV_FORMAT) << VP_SHT_IN_NV_FORMAT) |
            ((0x0 & VP_BIT_BYPASS_SCALE_EN) << VP_SHT_BYPASS_SCALE_EN) |
            ((pInInfo->InputBufferNum & VP_BIT_RDBUFFER_NUM) << VP_SHT_RDBUFFER_NUM) |
            ((pInInfo->DisableCaptureCtrl & VP_BIT_CAPTURE_CRTL_DISABLE) << VP_SHT_CAPTURE_CRTL_DISABLE) |
            ((pInInfo->EnableSceneChg & VP_BIT_SCENE_CHANGE_EN) << VP_SHT_SCENE_CHANGE_EN);

    HOST_WriteRegister(VP_REG_SET502,  (MMP_UINT16)Value);
}

//=============================================================================
/**
 * Set ISP input buffer relate parameters.
 */
//=============================================================================
void
VP_SetInputBuf_Reg(
    const VP_INPUT_INFO    *pInInfo)
{
    MMP_UINT16  Value = 0;

    HOST_WriteRegister(VP_REG_INPUT_WIDTH, (MMP_UINT16)(pInInfo->SrcWidth & VP_BIT_INPUT_WIDTH));
    HOST_WriteRegister(VP_REG_INPUT_HEIGHT, (MMP_UINT16)(pInInfo->SrcHeight & VP_BIT_INPUT_HEIGHT));

    HOST_WriteRegister(VP_REG_INPUT_PITCH_Y, (MMP_UINT16)(pInInfo->PitchY & VP_BIT_INPUT_PITCH_Y));
    HOST_WriteRegister(VP_REG_INPUT_PITCH_UV, (MMP_UINT16)(pInInfo->PitchUV & VP_BIT_INPUT_PITCH_UV));
}

//=============================================================================
/**
 * Set ISP input buffer address relate parameters
 */
//=============================================================================
void
VP_SetInputAddr_Reg(
    const VP_INPUT_INFO    *pInInfo)
{
    MMP_UINT32      Value = 0;
    MMP_UINT32      vramBaseAddr = (MMP_UINT32)HOST_GetVramBaseAddress();

    //CurFrame 0
    Value = (MMP_UINT32)pInInfo->AddrY[0] - vramBaseAddr;
    HOST_WriteRegister(VP_REG_INPUT_ADDR_Y0L, (MMP_UINT16)(Value & VP_BIT_INPUT_ADDR_L));
    HOST_WriteRegister(VP_REG_INPUT_ADDR_Y0H, (MMP_UINT16)((Value >> 16) & VP_BIT_INPUT_ADDR_H));

    Value = (MMP_UINT32)pInInfo->AddrUV[0] - vramBaseAddr;
    HOST_WriteRegister(VP_REG_INPUT_ADDR_UV0L, (MMP_UINT16)(Value & VP_BIT_INPUT_ADDR_L));
    HOST_WriteRegister(VP_REG_INPUT_ADDR_UV0H, (MMP_UINT16)((Value >> 16) & VP_BIT_INPUT_ADDR_H));

    if (pInInfo->InputBufferNum != 0)
    {
    //CurFrame 1
    Value = (MMP_UINT32)pInInfo->AddrY[1] - vramBaseAddr;
    HOST_WriteRegister(VP_REG_INPUT_ADDR_Y1L, (MMP_UINT16)(Value & VP_BIT_INPUT_ADDR_L));
    HOST_WriteRegister(VP_REG_INPUT_ADDR_Y1H, (MMP_UINT16)((Value >> 16) & VP_BIT_INPUT_ADDR_H));

    Value = (MMP_UINT32)pInInfo->AddrUV[1] - vramBaseAddr;
    HOST_WriteRegister(VP_REG_INPUT_ADDR_UV1L, (MMP_UINT16)(Value & VP_BIT_INPUT_ADDR_L));
    HOST_WriteRegister(VP_REG_INPUT_ADDR_UV1H, (MMP_UINT16)((Value >> 16) & VP_BIT_INPUT_ADDR_H));

    //CurFrame 2
    Value = (MMP_UINT32)pInInfo->AddrY[2] - vramBaseAddr;
    HOST_WriteRegister(VP_REG_INPUT_ADDR_Y2L, (MMP_UINT16)(Value & VP_BIT_INPUT_ADDR_L));
    HOST_WriteRegister(VP_REG_INPUT_ADDR_Y2H, (MMP_UINT16)((Value >> 16) & VP_BIT_INPUT_ADDR_H));

    Value = (MMP_UINT32)pInInfo->AddrUV[2] - vramBaseAddr;
    HOST_WriteRegister(VP_REG_INPUT_ADDR_UV2L, (MMP_UINT16)(Value & VP_BIT_INPUT_ADDR_L));
    HOST_WriteRegister(VP_REG_INPUT_ADDR_UV2H, (MMP_UINT16)((Value >> 16) & VP_BIT_INPUT_ADDR_H));

    //CurFrame 3
    Value = (MMP_UINT32)pInInfo->AddrY[3] - vramBaseAddr;
    HOST_WriteRegister(VP_REG_INPUT_ADDR_Y3L, (MMP_UINT16)(Value & VP_BIT_INPUT_ADDR_L));
    HOST_WriteRegister(VP_REG_INPUT_ADDR_Y3H, (MMP_UINT16)((Value >> 16) & VP_BIT_INPUT_ADDR_H));

    Value = (MMP_UINT32)pInInfo->AddrUV[3] - vramBaseAddr;
    HOST_WriteRegister(VP_REG_INPUT_ADDR_UV3L, (MMP_UINT16)(Value & VP_BIT_INPUT_ADDR_L));
    HOST_WriteRegister(VP_REG_INPUT_ADDR_UV3H, (MMP_UINT16)((Value >> 16) & VP_BIT_INPUT_ADDR_H));
    
    //CurFrame 4
    Value = (MMP_UINT32)pInInfo->AddrY[4] - vramBaseAddr;
    HOST_WriteRegister(VP_REG_INPUT_ADDR_Y4L, (MMP_UINT16)(Value & VP_BIT_INPUT_ADDR_L));
    HOST_WriteRegister(VP_REG_INPUT_ADDR_Y4H, (MMP_UINT16)((Value >> 16) & VP_BIT_INPUT_ADDR_H));

    Value = (MMP_UINT32)pInInfo->AddrUV[4] - vramBaseAddr;
    HOST_WriteRegister(VP_REG_INPUT_ADDR_UV4L, (MMP_UINT16)(Value & VP_BIT_INPUT_ADDR_L));
    HOST_WriteRegister(VP_REG_INPUT_ADDR_UV4H, (MMP_UINT16)((Value >> 16) & VP_BIT_INPUT_ADDR_H));
    }
    
    //PreFrame
    Value = (MMP_UINT32)pInInfo->AddrYp - vramBaseAddr;
    HOST_WriteRegister(VP_REG_INPUT_ADDR_YPL, (MMP_UINT16)(Value & VP_BIT_INPUT_ADDR_L));
    HOST_WriteRegister(VP_REG_INPUT_ADDR_YPH, (MMP_UINT16)((Value >> 16) & VP_BIT_INPUT_ADDR_H));
}

//=============================================================================
/**
* Set Deinterlace Parameter.
*/
//=============================================================================
void
VP_SetDeInterlaceParam_Reg(
    const VP_DEINTERLACE_CTRL  *pDeInterlace)
{
    MMP_UINT16  Value = 0;

    Value = ((pDeInterlace->EnableOutMotion & VP_BIT_OUTMOTIONDETECT_EN) << VP_SHT_OUTMOTIONDETECT_EN) |
            ((pDeInterlace->LowLevelBypassBlend & VP_BIT_LOWLEVELBYPASSBLEND) << VP_SHT_LOWLEVELBYPASSBLEND) |
            ((pDeInterlace->Disable30MotionDetect & VP_BIT_30LOWLEVELEDGE_DISABLE) << VP_SHT_30LOWLEVELEDGE_DISABLE) |
            ((pDeInterlace->EnLowLevelOutside & VP_BIT_LOWLEVELOUTSIDE_EN) << VP_SHT_LOWLEVELOUTSIDE_EN) |
            ((pDeInterlace->LowLevelMode & VP_BIT_LOWLEVELMODE) << VP_SHT_LOWLEVELMODE) |
            ((pDeInterlace->EnLowLevelEdge & VP_BIT_LOWLEVELEDGE_EN) << VP_SHT_LOWLEVELEDGE_EN) |
            ((pDeInterlace->UVRepeatMode & VP_BIT_UVREPEAT_MODE) << VP_SHT_UVREPEAT_MODE) |
            ((pDeInterlace->EnChromaEdgeDetect & VP_BIT_CHROMA_EDGEDET_EN) << VP_SHT_CHROMA_EDGEDET_EN) |
            ((pDeInterlace->EnLummaEdgeDetect & VP_BIT_LUMA_EDGEDET_EN) << VP_SHT_LUMA_EDGEDET_EN) |
            ((pDeInterlace->Enable & VP_BIT_DEINTERLACE_EN) << VP_SHT_DEINTERLACE_EN) |
            ((pDeInterlace->DeinterMode & VP_BIT_2D_DEINTER_MODE_EN) << VP_SHT_2D_DEINTER_MODE_EN) |
            ((pDeInterlace->EnSrcBottomFieldFirst & VP_BIT_SRC_BOTTOM_FIELD_FIRST) << VP_SHT_SRC_BOTTOM_FIELD_FIRST) |
            ((pDeInterlace->EnDeinterBottomField & VP_BIT_DEINTER_BOTTOM_EN) << VP_SHT_DEINTER_BOTTOM_EN) |
            ((pDeInterlace->EnSrcLPF & VP_BIT_SRC_LPFITR_EN) << VP_SHT_SRC_LPFITR_EN) |
            ((pDeInterlace->EnUV2DMethod & VP_BIT_UV2D_METHOD_EN) << VP_SHT_UV2D_METHOD_EN);
          
    HOST_WriteRegister(VP_REG_SET504, (MMP_UINT16)Value);

    HOST_WriteRegister(VP_REG_LOWLEVELEDGE_START_X, (MMP_UINT16)pDeInterlace->LowLevelPosX);
    HOST_WriteRegister(VP_REG_LOWLEVELEDGE_START_Y, (MMP_UINT16)pDeInterlace->LowLevelPosY);
    HOST_WriteRegister(VP_REG_LOWLEVELEDGE_WIDTH, (MMP_UINT16)pDeInterlace->LowLevelWidth);
    HOST_WriteRegister(VP_REG_LOWLEVELEDGE_HEIGHT, (MMP_UINT16)pDeInterlace->LowLevelHeight);

    if(pDeInterlace->DeinterMode == DEINTER3D)
        VP_Set3DDeInterlaceParm_Reg();
    else if(pDeInterlace->DeinterMode == DEINTER2D)
        VP_Set2DDeInterlaceParam_Reg();
}


//=============================================================================
/**
* Set 3D-Deinterlace parameters.
*/
//=============================================================================
void
VP_Set3DDeInterlaceParm_Reg(
    void)
{
    MMP_UINT16              Value = 0;
    VP_DEINTERLACE_CTRL    *pDeInterlace = &VPctxt->DeInterlace;

    //Parameter 1
    Value = ((pDeInterlace->MDThreshold_High & VP_BIT_3D_MDTHRED_HIGH) << VP_SHT_3D_MDTHRED_HIGH) |
            ((pDeInterlace->MDThreshold_Low & VP_BIT_3D_MDTHRED_LOW) << VP_SHT_3D_MDTHRED_LOW);

    HOST_WriteRegister(VP_REG_3D_DEINTER_PARM_1, (MMP_UINT16)Value);

    //Parameter 2
    Value = ((pDeInterlace->DisableMV_A & VP_BIT_DISABLE_MOTIONVALUE_A) << VP_SHT_DISABLE_MOTIONVALUE_A) |
            ((pDeInterlace->DisableMV_B & VP_BIT_DISABLE_MOTIONVALUE_B) << VP_SHT_DISABLE_MOTIONVALUE_B) |
            ((pDeInterlace->DisableMV_C & VP_BIT_DISABLE_MOTIONVALUE_C) << VP_SHT_DISABLE_MOTIONVALUE_C) |
            ((pDeInterlace->DisableMV_D & VP_BIT_DISABLE_MOTIONVALUE_D) << VP_SHT_DISABLE_MOTIONVALUE_D) |
            ((pDeInterlace->DisableMV_E & VP_BIT_DISABLE_MOTIONVALUE_E) << VP_SHT_DISABLE_MOTIONVALUE_E) |
            ((pDeInterlace->DisableMV_F & VP_BIT_DISABLE_MOTIONVALUE_F) << VP_SHT_DISABLE_MOTIONVALUE_F) |
            ((pDeInterlace->DisableMV_G & VP_BIT_DISABLE_MOTIONVALUE_G) << VP_SHT_DISABLE_MOTIONVALUE_G) |
            ((pDeInterlace->EnLPFWeight & VP_BIT_LPF_WEIGHT_EN) << VP_SHT_LPF_WEIGHT_EN) |
            ((pDeInterlace->EnLPFBlend & VP_BIT_LPF_BLEND_EN) << VP_SHT_LPF_BLEND_EN) |
            ((pDeInterlace->MDThreshold_Step & VP_BIT_3D_MDTHRED_STEP) << VP_SHT_3D_MDTHRED_STEP);

    HOST_WriteRegister(VP_REG_3D_DEINTER_PARM_2, (MMP_UINT16)Value);

    //Parameter 3
    Value = ((pDeInterlace->EnLPFStaticPixel & VP_BIT_LPF_STATICPIXEL_EN) << VP_SHT_LPF_STATICPIXEL_EN);

    HOST_WriteRegister(VP_REG_3D_DEINTER_PARM_3, (MMP_UINT16)Value);

}

//=============================================================================
/**
* Set 2D-Deinterlace parameters.
*/
//=============================================================================
void
VP_Set2DDeInterlaceParam_Reg(
    void)
{
    MMP_UINT16              Value = 0;
    VP_DEINTERLACE_CTRL    *pDeInterlace = &VPctxt->DeInterlace;

    Value = ((pDeInterlace->D2EdgeBlendWeight & VP_BIT_2D_EDGE_WEIGHT) << VP_SHT_2D_EDGE_WEIGHT) |
            ((pDeInterlace->D2OrgBlendWeight & VP_BIT_2D_ORG_WEIGHT) << VP_SHT_2D_ORG_WEIGHT);

    HOST_WriteRegister(VP_REG_2D_DEINTER_PARM_1, (MMP_UINT16)Value);
}

//=============================================================================
/**
 * Set Jpeg encode parameters
 */
//=============================================================================
void
VP_SetJpegEncode_Reg(
    const VP_JEPG_ENCODE_CTRL  *pJpegEncode)
{
    MMP_UINT16              Value = 0;
    
    Value = ((pJpegEncode->EnableJPEGEncode & VP_BIT_JPEGDECODE_EN) << VP_SHT_JPEGDECODE_EN) |
            ((pJpegEncode->TotalSliceNum & VP_BIT_TOTALSLICENUM) << VP_SHT_TOTALSLICENUM);
            
    HOST_WriteRegister(VP_REG_SET506, (MMP_UINT16)Value);
}

//=============================================================================
/**
 * Set color correction matrix and constant
 */
//=============================================================================
void
VP_SetCCMatrix_Reg(
    const VP_COLOR_CORRECTION  *pColorCorrect)
{
    HOST_WriteRegister(VP_REG_CC_IN_OFFSET_R, (MMP_UINT16)(pColorCorrect->OffsetR & VP_BIT_IN_OFFSET));
    HOST_WriteRegister(VP_REG_CC_IN_OFFSET_G, (MMP_UINT16)(pColorCorrect->OffsetG & VP_BIT_IN_OFFSET));
    HOST_WriteRegister(VP_REG_CC_IN_OFFSET_B, (MMP_UINT16)(pColorCorrect->OffsetB & VP_BIT_IN_OFFSET));

    HOST_WriteRegister(VP_REG_COL_COR_11, (MMP_UINT16)(pColorCorrect->_11 & VP_BIT_COL_COR));
    HOST_WriteRegister(VP_REG_COL_COR_12, (MMP_UINT16)(pColorCorrect->_12 & VP_BIT_COL_COR));
    HOST_WriteRegister(VP_REG_COL_COR_13, (MMP_UINT16)(pColorCorrect->_13 & VP_BIT_COL_COR));
    HOST_WriteRegister(VP_REG_COL_COR_21, (MMP_UINT16)(pColorCorrect->_21 & VP_BIT_COL_COR));
    HOST_WriteRegister(VP_REG_COL_COR_22, (MMP_UINT16)(pColorCorrect->_22 & VP_BIT_COL_COR));
    HOST_WriteRegister(VP_REG_COL_COR_23, (MMP_UINT16)(pColorCorrect->_23 & VP_BIT_COL_COR));
    HOST_WriteRegister(VP_REG_COL_COR_31, (MMP_UINT16)(pColorCorrect->_31 & VP_BIT_COL_COR));
    HOST_WriteRegister(VP_REG_COL_COR_32, (MMP_UINT16)(pColorCorrect->_32 & VP_BIT_COL_COR));
    HOST_WriteRegister(VP_REG_COL_COR_33, (MMP_UINT16)(pColorCorrect->_33 & VP_BIT_COL_COR));

    HOST_WriteRegister(VP_REG_COL_COR_DELTA_R, (MMP_UINT16)(pColorCorrect->DeltaR & VP_BIT_COL_CORR_DELTA));
    HOST_WriteRegister(VP_REG_COL_COR_DELTA_G, (MMP_UINT16)(pColorCorrect->DeltaG & VP_BIT_COL_CORR_DELTA));
    HOST_WriteRegister(VP_REG_COL_COR_DELTA_B, (MMP_UINT16)(pColorCorrect->DeltaB & VP_BIT_COL_CORR_DELTA));
}

//=============================================================================
/*
* Set Scale Factor
*/
//=============================================================================
void
VP_SetScaleParam_Reg(
    const VP_SCALE_CTRL    *pScaleFun)
{
    MMP_UINT16  Value = 0;
    MMP_UINT32  HCI;
    MMP_UINT32  VCI;

    HCI = MMP_FLOATToFix(pScaleFun->HCI, 6, 14);
    VCI = MMP_FLOATToFix(pScaleFun->VCI, 6, 14);

    //HCI
    HOST_WriteRegister(VP_REG_SCALE_HCI_L, (MMP_UINT16)(HCI & VP_BIT_SCALE_L));
    HOST_WriteRegister(VP_REG_SCALE_HCI_H, (MMP_UINT16)((HCI >> 16) & VP_BIT_SCALE_H));

    //VCI
    HOST_WriteRegister(VP_REG_SCALE_VCI_L, (MMP_UINT16)(VCI & VP_BIT_SCALE_L));
    HOST_WriteRegister(VP_REG_SCALE_VCI_H, (MMP_UINT16)((VCI >> 16) & VP_BIT_SCALE_H));
}


//=============================================================================
/**
* Set Scale Horizontal Weight.
*/
//=============================================================================
void
VP_SetScaleMatrixH_Reg(
    const VP_SCALE_CTRL    *pScaleFun)
{
    MMP_UINT16  Value;

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[0][0], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[0][1], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWX0100, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[0][2], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[0][3], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWX0302, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[1][0], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[1][1], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWX1110, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[1][2], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[1][3], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWX1312, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[2][0], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[2][1], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWX2120, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[2][2], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[2][3], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWX2322, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[3][0], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[3][1], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWX3130, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[3][2], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[3][3], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWX3332, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[4][0], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[4][1], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWX4140, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[4][2], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[4][3], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWX4342, (MMP_UINT16)Value);
}

void
VP_SetIntScaleMatrixH_Reg(
    MMP_UINT8  WeightMatX[][VP_SCALE_TAP])
{
    MMP_UINT16  Value;

    Value = ((WeightMatX[0][0] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[0][1] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWX0100, (MMP_UINT16)Value);

    Value = ((WeightMatX[0][2] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[0][3] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWX0302, (MMP_UINT16)Value);

    Value = ((WeightMatX[1][0] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[1][1] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWX1110, (MMP_UINT16)Value);

    Value = ((WeightMatX[1][2] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[1][3] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWX1312, (MMP_UINT16)Value);

    Value = ((WeightMatX[2][0] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[2][1] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWX2120, (MMP_UINT16)Value);

    Value = ((WeightMatX[2][2] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[2][3] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWX2322, (MMP_UINT16)Value);

    Value = ((WeightMatX[3][0] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[3][1] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWX3130, (MMP_UINT16)Value);

    Value = ((WeightMatX[3][2] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[3][3] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWX3332, (MMP_UINT16)Value);

    Value = ((WeightMatX[4][0] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[4][1] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWX4140, (MMP_UINT16)Value);

    Value = ((WeightMatX[4][2] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[4][3] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWX4342, (MMP_UINT16)Value);
}

//=============================================================================
/**
* Set Scale Vertical Weight.
*/
//=============================================================================
void
VP_SetScaleMatrixV_Reg(
    const VP_SCALE_CTRL    *pScaleFun)
{
    MMP_UINT16 Value;

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[0][0], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[0][1], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWY0100, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[0][2], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[0][3], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWY0302, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[1][0], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[1][1], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWY1110, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[1][2], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[1][3], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWY1312, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[2][0], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[2][1], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWY2120, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[2][2], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[2][3], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWY2322, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[3][0], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[3][1], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWY3130, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[3][2], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[3][3], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWY3332, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[4][0], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[4][1], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWY4140, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[4][2], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[4][3], 1, 6) & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWY4342, (MMP_UINT16)Value);
}

void
VP_SetIntScaleMatrixV_Reg(
    MMP_UINT8  WeightMatY[][VP_SCALE_TAP])
{
    MMP_UINT16 Value;

    Value = ((WeightMatY[0][0] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[0][1] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWY0100, (MMP_UINT16)Value);

    Value = ((WeightMatY[0][2] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[0][3] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWY0302, (MMP_UINT16)Value);

    Value = ((WeightMatY[1][0] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[1][1] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWY1110, (MMP_UINT16)Value);

    Value = ((WeightMatY[1][2] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[1][3] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWY1312, (MMP_UINT16)Value);

    Value = ((WeightMatY[2][0] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[2][1] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWY2120, (MMP_UINT16)Value);

    Value = ((WeightMatY[2][2] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[2][3] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWY2322, (MMP_UINT16)Value);

    Value = ((WeightMatY[3][0] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[3][1] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWY3130, (MMP_UINT16)Value);

    Value = ((WeightMatY[3][2] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[3][3] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWY3332, (MMP_UINT16)Value);

    Value = ((WeightMatY[4][0] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[4][1] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWY4140, (MMP_UINT16)Value);

    Value = ((WeightMatY[4][2] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[4][3] & VP_BIT_SCALEWEIGHT) << VP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(VP_REG_SCALEWY4342, (MMP_UINT16)Value);
}


//=============================================================================
/**
* Frmfun RGB to YUV transfer matrix.
*/
//=============================================================================
void
VP_SetFrmMatrix_Reg(
    const VP_RGB_TO_YUV    *pMatrix)
{
    HOST_WriteRegister(VP_REG_FRM_RGB2YUV_11, (MMP_UINT16)(pMatrix->_11 & VP_BIT_FRM_RGB2YUV));
    HOST_WriteRegister(VP_REG_FRM_RGB2YUV_12, (MMP_UINT16)(pMatrix->_12 & VP_BIT_FRM_RGB2YUV));
    HOST_WriteRegister(VP_REG_FRM_RGB2YUV_13, (MMP_UINT16)(pMatrix->_13 & VP_BIT_FRM_RGB2YUV));

    HOST_WriteRegister(VP_REG_FRM_RGB2YUV_21, (MMP_UINT16)(pMatrix->_21 & VP_BIT_FRM_RGB2YUV));
    HOST_WriteRegister(VP_REG_FRM_RGB2YUV_22, (MMP_UINT16)(pMatrix->_22 & VP_BIT_FRM_RGB2YUV));
    HOST_WriteRegister(VP_REG_FRM_RGB2YUV_23, (MMP_UINT16)(pMatrix->_23 & VP_BIT_FRM_RGB2YUV));

    HOST_WriteRegister(VP_REG_FRM_RGB2YUV_31, (MMP_UINT16)(pMatrix->_31 & VP_BIT_FRM_RGB2YUV));
    HOST_WriteRegister(VP_REG_FRM_RGB2YUV_32, (MMP_UINT16)(pMatrix->_32 & VP_BIT_FRM_RGB2YUV));
    HOST_WriteRegister(VP_REG_FRM_RGB2YUV_33, (MMP_UINT16)(pMatrix->_33 & VP_BIT_FRM_RGB2YUV));

    HOST_WriteRegister(VP_REG_FRM_RGB2YUV_CONST_Y, (MMP_UINT16)(pMatrix->ConstY & VP_BIT_FRM_RGB2YUV_CONST));
    HOST_WriteRegister(VP_REG_FRM_RGB2YUV_CONST_U, (MMP_UINT16)(pMatrix->ConstU & VP_BIT_FRM_RGB2YUV_CONST));
    HOST_WriteRegister(VP_REG_FRM_RGB2YUV_CONST_V, (MMP_UINT16)(pMatrix->ConstV & VP_BIT_FRM_RGB2YUV_CONST));
}


//=============================================================================
/**
* Set Frame Function 0
*/
//=============================================================================
void
VP_SetFrameFun0_Reg(
    const VP_FRMFUN_CTRL   *pFrameFun)
{
    MMP_UINT32      Value = 0;
    MMP_UINT32      vramBaseAddr = (MMP_UINT32)HOST_GetVramBaseAddress();

    //Starting address
    Value = (MMP_UINT32)pFrameFun->Addr - vramBaseAddr; // byte align
    HOST_WriteRegister(VP_REG_FRMFUN_0_ADDR_L, (MMP_UINT16)(Value & VP_BIT_FRMFUN_ADDR_L));
    HOST_WriteRegister(VP_REG_FRMFUN_0_ADDR_H, (MMP_UINT16)((Value >> 16) & VP_BIT_FRMFUN_ADDR_H));

    //width, height, pitch
    HOST_WriteRegister(VP_REG_FRMFUN_0_WIDTH,  (MMP_UINT16)(pFrameFun->Width & VP_BIT_FRMFUN_WIDTH));
    HOST_WriteRegister(VP_REG_FRMFUN_0_HEIGHT, (MMP_UINT16)(pFrameFun->Height & VP_BIT_FRMFUN_HEIGHT));
    HOST_WriteRegister(VP_REG_FRMFUN_0_PITCH,  (MMP_UINT16)(pFrameFun->Pitch & VP_BIT_FRMFUN_PITCH));

    //start X/Y
    HOST_WriteRegister(VP_REG_FRMFUN_0_START_X, (MMP_UINT16)(pFrameFun->StartX & VP_BIT_FRMFUN_START_X));
    HOST_WriteRegister(VP_REG_FRMFUN_0_START_Y, (MMP_UINT16)(pFrameFun->StartY & VP_BIT_FRMFUN_START_Y));

    //color key
    Value = ((pFrameFun->ColorKeyR & VP_BIT_FRMFUN_KEY) << VP_SHT_FRMFUN_KEY_R) |
            ((pFrameFun->ColorKeyG & VP_BIT_FRMFUN_KEY) << VP_SHT_FRMFUN_KEY_G);

    HOST_WriteRegister(VP_REG_FRMFUN_0_KEY_RG, (MMP_UINT16)Value);
    HOST_WriteRegister(VP_REG_FRMFUN_0_KEY_B,  (MMP_UINT16)(pFrameFun->ColorKeyB & VP_BIT_FRMFUN_KEY));

    //Constant Alpha Value
    HOST_WriteRegister(VP_REG_CONST_ALPHA_0, (MMP_UINT16)(pFrameFun->ConstantAlpha & VP_BIT_CONST_ALPHA));


    //format ARGB4444 or Constant Alpha with RGB565
    if(pFrameFun->Format == ARGB4444)
    {
        Value = ((0x1 & VP_BIT_FRMFUN_ALPHA_BLEND_EN) << VP_SHT_FRMFUN_ALPHA_BLEND_EN) |
                ((0x1 & VP_BIT_FRMFUN_MODE) << VP_SHT_FRMFUN_MODE);
    }
    else if(pFrameFun->Format == CARGB565)
    {
        if(pFrameFun->ConstantAlpha != 0)
        {
            Value = ((0x1 & VP_BIT_FRMFUN_ALPHA_BLEND_EN) << VP_SHT_FRMFUN_ALPHA_BLEND_EN) |
                    ((0x0 & VP_BIT_FRMFUN_MODE) << VP_SHT_FRMFUN_MODE);
        }
        else
        {
            Value = ((0x0 & VP_BIT_FRMFUN_ALPHA_BLEND_EN) << VP_SHT_FRMFUN_ALPHA_BLEND_EN) |
                    ((0x0 & VP_BIT_FRMFUN_MODE) << VP_SHT_FRMFUN_MODE);

        }
    }

    //FrameFun Enable, BlendConst
    Value = Value |
            ((pFrameFun->Enable & VP_BIT_FRMFUN_EN) << VP_SHT_FRMFUN_EN) |
            ((pFrameFun->EnableFieldMode & VP_BIT_FRMFUN_FIELDMODE_EN) << VP_SHT_FRMFUN_FIELDMODE_EN) |
            ((pFrameFun->EnableGobang & VP_BIT_FRMFUN_GOBANG_EN) << VP_SHT_FRMFUN_GOBANG_EN) |
            ((pFrameFun->EnableRGB2YUV & VP_BIT_FRMFUN_RGB2YUV_EN) << VP_SHT_FRMFUN_RGB2YUV_EN);

    //Enable frame function, set format
    HOST_WriteRegister(VP_REG_SET_FRMFUN_0, (MMP_UINT16)Value);
}

//=============================================================================
/**
* RGB to YUV transfer matrix.
*/
//=============================================================================
void
VP_SetRGBtoYUVMatrix_Reg(
    const VP_RGB_TO_YUV    *pRGBtoYUV)
{
    HOST_WriteRegister(VP_REG_RGB_TO_YUV_11, (MMP_UINT16)(pRGBtoYUV->_11 & VP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(VP_REG_RGB_TO_YUV_12, (MMP_UINT16)(pRGBtoYUV->_12 & VP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(VP_REG_RGB_TO_YUV_13, (MMP_UINT16)(pRGBtoYUV->_13 & VP_BIT_RGB_TO_YUV));

    HOST_WriteRegister(VP_REG_RGB_TO_YUV_21, (MMP_UINT16)(pRGBtoYUV->_21 & VP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(VP_REG_RGB_TO_YUV_22, (MMP_UINT16)(pRGBtoYUV->_22 & VP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(VP_REG_RGB_TO_YUV_23, (MMP_UINT16)(pRGBtoYUV->_23 & VP_BIT_RGB_TO_YUV));

    HOST_WriteRegister(VP_REG_RGB_TO_YUV_31, (MMP_UINT16)(pRGBtoYUV->_31 & VP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(VP_REG_RGB_TO_YUV_32, (MMP_UINT16)(pRGBtoYUV->_32 & VP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(VP_REG_RGB_TO_YUV_33, (MMP_UINT16)(pRGBtoYUV->_33 & VP_BIT_RGB_TO_YUV));

    HOST_WriteRegister(VP_REG_RGB_TO_YUV_CONST_Y, (MMP_UINT16)(pRGBtoYUV->ConstY & VP_BIT_RGB_TO_YUV_CONST));
    HOST_WriteRegister(VP_REG_RGB_TO_YUV_CONST_U, (MMP_UINT16)(pRGBtoYUV->ConstU & VP_BIT_RGB_TO_YUV_CONST));
    HOST_WriteRegister(VP_REG_RGB_TO_YUV_CONST_V, (MMP_UINT16)(pRGBtoYUV->ConstV & VP_BIT_RGB_TO_YUV_CONST));
}

//=============================================================================
/**
* Set Output Format
*/
//=============================================================================
void
VP_SetOutParameter_Reg(
    const VP_OUTPUT_INFO   *pOutInfo)
{

    MMP_UINT16  Value = 0;

    //Set VP_REG_SET508
    
    Value = ((pOutInfo->EngineDelay & VP_BIT_ENGINEDELAY) << VP_SHT_ENGINEDELAY) |
            ((pOutInfo->OutputBufferNum & VP_BIT_WRBUFFER_NUM) << VP_SHT_WRBUFFER_NUM) |
            ((pOutInfo->EnableSWCtrlRdAddr & VP_BIT_SWCTRL_RDADDR_EN) << VP_SHT_SWCTRL_RDADDR_EN) |
            ((pOutInfo->EnableSWFlipMode & VP_BIT_SWWRFLIP_EN) << VP_SHT_SWWRFLIP_EN) |
            ((pOutInfo->SWWrFlipNum & VP_BIT_SWWRFLIP_NUM) << VP_SHT_SWWRFLIP_NUM);
 
    HOST_WriteRegister(VP_REG_SET508,  (MMP_UINT16)Value); 
    Value = 0;


    //Set VP_REG_SET50A

    Value = ((pOutInfo->EnableUVBiDownsample & VP_BIT_OUT_BILINEAR_DOWNSAMPLE_EN) << VP_SHT_OUT_BILINEAR_DOWNSAMPLE_EN) |
            ((pOutInfo->PlaneFormat & VP_SHT_OUT_YUVPLANE_FORMAT) << VP_BIT_OUT_YUVPLANE_FORMAT) |
            ((pOutInfo->NVFormat & VP_BIT_OUT_NV_FORMAT) << VP_SHT_OUT_NV_FORMAT) |
            ((pOutInfo->OutFormat & VP_BIT_OUT_FORMAT) << VP_SHT_OUT_FORMAT) |             
            ((pOutInfo->DisableOutMatrix & VP_BIT_OUTMATRIX_DISABLE) << VP_SHT_OUTMATRIX_DISABLE) |
            ((pOutInfo->EnableFieldMode & VP_BIT_OUTPUT_FIELD_MODE) << VP_SHT_OUTPUT_FIELD_MODE) |
            ((pOutInfo->EnableRemapUVAddr & VP_BIT_REMAP_CHROMAADDR_EN) << VP_SHT_REMAP_CHROMAADDR_EN) |
            ((pOutInfo->EnableRemapYAddr & VP_BIT_REMAP_LUMAADDR_EN) << VP_SHT_REMAP_LUMAADDR_EN);

    HOST_WriteRegister(VP_REG_SET50A,  (MMP_UINT16)Value);
    Value = 0;
}

//=============================================================================
/**
 * Set Output Information
 */
//=============================================================================
void
VP_SetOutBufInfo_Reg(
    const VP_OUTPUT_INFO   *pOutInfo)
{
    //width, height, pitch
    HOST_WriteRegister(VP_REG_OUT_WIDTH,  (MMP_UINT16)(pOutInfo->Width & VP_BIT_OUT_WIDTH ));
    HOST_WriteRegister(VP_REG_OUT_HEIGHT, (MMP_UINT16)(pOutInfo->Height & VP_BIT_OUT_HEIGHT));
    HOST_WriteRegister(VP_REG_OUT_Y_PITCH,  (MMP_UINT16)(pOutInfo->PitchY & VP_BIT_OUT_PITCH));
    HOST_WriteRegister(VP_REG_OUT_UV_PITCH,  (MMP_UINT16)(pOutInfo->PitchUV & VP_BIT_OUT_PITCH));
}

//=============================================================================
/**
* Set Output Address
*/
//=============================================================================
void
VP_SetOutAddress_Reg(
    const VP_OUTPUT_INFO   *pOutInfo)
{
    MMP_UINT32  Value = 0;
    MMP_UINT32  vramBaseAddr = (MMP_UINT32)HOST_GetVramBaseAddress();

    // byte align
    //buffer address 0
    Value = (MMP_UINT32)pOutInfo->AddrY[0] - vramBaseAddr; 
    HOST_WriteRegister(VP_REG_OUT_ADDR_Y0L, (MMP_UINT16)(Value & VP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(VP_REG_OUT_ADDR_Y0H, (MMP_UINT16)((Value >> 16) & VP_BIT_OUT_ADDR_H));

    Value = (MMP_UINT32)pOutInfo->AddrU[0] - vramBaseAddr;
    HOST_WriteRegister(VP_REG_OUT_ADDR_U0L, (MMP_UINT16)(Value & VP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(VP_REG_OUT_ADDR_U0H, (MMP_UINT16)((Value >> 16) & VP_BIT_OUT_ADDR_H));

    Value = (MMP_UINT32)pOutInfo->AddrV[0] - vramBaseAddr;
    HOST_WriteRegister(VP_REG_OUT_ADDR_V0L, (MMP_UINT16)(Value & VP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(VP_REG_OUT_ADDR_V0H, (MMP_UINT16)((Value >> 16) & VP_BIT_OUT_ADDR_H));
    
    
    if (pOutInfo->OutputBufferNum != 0)    
    {
    //buffer address 1
    Value = (MMP_UINT32)pOutInfo->AddrY[1] - vramBaseAddr; 
    HOST_WriteRegister(VP_REG_OUT_ADDR_Y1L, (MMP_UINT16)(Value & VP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(VP_REG_OUT_ADDR_Y1H, (MMP_UINT16)((Value >> 16) & VP_BIT_OUT_ADDR_H));

    Value = (MMP_UINT32)pOutInfo->AddrU[1] - vramBaseAddr;
    HOST_WriteRegister(VP_REG_OUT_ADDR_U1L, (MMP_UINT16)(Value & VP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(VP_REG_OUT_ADDR_U1H, (MMP_UINT16)((Value >> 16) & VP_BIT_OUT_ADDR_H));

    Value = (MMP_UINT32)pOutInfo->AddrV[1] - vramBaseAddr;
    HOST_WriteRegister(VP_REG_OUT_ADDR_V1L, (MMP_UINT16)(Value & VP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(VP_REG_OUT_ADDR_V1H, (MMP_UINT16)((Value >> 16) & VP_BIT_OUT_ADDR_H));
    
    //buffer address 2
    Value = (MMP_UINT32)pOutInfo->AddrY[2] - vramBaseAddr; 
    HOST_WriteRegister(VP_REG_OUT_ADDR_Y2L, (MMP_UINT16)(Value & VP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(VP_REG_OUT_ADDR_Y2H, (MMP_UINT16)((Value >> 16) & VP_BIT_OUT_ADDR_H));

    Value = (MMP_UINT32)pOutInfo->AddrU[2] - vramBaseAddr;
    HOST_WriteRegister(VP_REG_OUT_ADDR_U2L, (MMP_UINT16)(Value & VP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(VP_REG_OUT_ADDR_U2H, (MMP_UINT16)((Value >> 16) & VP_BIT_OUT_ADDR_H));

    Value = (MMP_UINT32)pOutInfo->AddrV[2] - vramBaseAddr;
    HOST_WriteRegister(VP_REG_OUT_ADDR_V2L, (MMP_UINT16)(Value & VP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(VP_REG_OUT_ADDR_V2H, (MMP_UINT16)((Value >> 16) & VP_BIT_OUT_ADDR_H));
    
    //buffer address 3
    Value = (MMP_UINT32)pOutInfo->AddrY[3] - vramBaseAddr; 
    HOST_WriteRegister(VP_REG_OUT_ADDR_Y3L, (MMP_UINT16)(Value & VP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(VP_REG_OUT_ADDR_Y3H, (MMP_UINT16)((Value >> 16) & VP_BIT_OUT_ADDR_H));

    Value = (MMP_UINT32)pOutInfo->AddrU[3] - vramBaseAddr;
    HOST_WriteRegister(VP_REG_OUT_ADDR_U3L, (MMP_UINT16)(Value & VP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(VP_REG_OUT_ADDR_U3H, (MMP_UINT16)((Value >> 16) & VP_BIT_OUT_ADDR_H));

    Value = (MMP_UINT32)pOutInfo->AddrV[3] - vramBaseAddr;
    HOST_WriteRegister(VP_REG_OUT_ADDR_V3L, (MMP_UINT16)(Value & VP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(VP_REG_OUT_ADDR_V3H, (MMP_UINT16)((Value >> 16) & VP_BIT_OUT_ADDR_H));
    
    //buffer address 4
    Value = (MMP_UINT32)pOutInfo->AddrY[4] - vramBaseAddr; 
    HOST_WriteRegister(VP_REG_OUT_ADDR_Y4L, (MMP_UINT16)(Value & VP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(VP_REG_OUT_ADDR_Y4H, (MMP_UINT16)((Value >> 16) & VP_BIT_OUT_ADDR_H));

    Value = (MMP_UINT32)pOutInfo->AddrU[4] - vramBaseAddr;
    HOST_WriteRegister(VP_REG_OUT_ADDR_U4L, (MMP_UINT16)(Value & VP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(VP_REG_OUT_ADDR_U4H, (MMP_UINT16)((Value >> 16) & VP_BIT_OUT_ADDR_H));

    Value = (MMP_UINT32)pOutInfo->AddrV[4] - vramBaseAddr;
    HOST_WriteRegister(VP_REG_OUT_ADDR_V4L, (MMP_UINT16)(Value & VP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(VP_REG_OUT_ADDR_V4H, (MMP_UINT16)((Value >> 16) & VP_BIT_OUT_ADDR_H));
    }
}

//=============================================================================
/**
* Set Remap Y Address
*/
//=============================================================================
void
VP_SetRemapYAddress_Reg(
    const VP_REMAP_ADDR   *pRemapAddr)
{
    MMP_UINT32  Value = 0;
    
    Value = ((pRemapAddr->Addr_04 & 0x3F) << 8) | (pRemapAddr->Addr_03 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_Y_0403, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y)); 
    
    Value = ((pRemapAddr->Addr_06 & 0x3F) << 8) | (pRemapAddr->Addr_05 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_Y_0605, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y)); 
        
    Value = ((pRemapAddr->Addr_08 & 0x3F) << 8) | (pRemapAddr->Addr_07 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_Y_0807, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_10 & 0x3F) << 8) | (pRemapAddr->Addr_09 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_Y_1009, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_12 & 0x3F) << 8) | (pRemapAddr->Addr_11 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_Y_1211, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_14 & 0x3F) << 8) | (pRemapAddr->Addr_13 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_Y_1413, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_16 & 0x3F) << 8) | (pRemapAddr->Addr_15 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_Y_1615, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_18 & 0x3F) << 8) | (pRemapAddr->Addr_17 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_Y_1817, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_20 & 0x3F) << 8) | (pRemapAddr->Addr_19 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_Y_2019, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_22 & 0x3F) << 8) | (pRemapAddr->Addr_21 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_Y_2221, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_24 & 0x3F) << 8) | (pRemapAddr->Addr_23 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_Y_2423, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_26 & 0x3F) << 8) | (pRemapAddr->Addr_25 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_Y_2625, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_28 & 0x3F) << 8) | (pRemapAddr->Addr_27 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_Y_2827, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_30 & 0x3F) << 8) | (pRemapAddr->Addr_29 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_Y_3029, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = (pRemapAddr->Addr_31 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_Y_XX31, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
}

//=============================================================================
/**
* Set Remap UV Address
*/
//=============================================================================
void
VP_SetRemapUVAddress_Reg(
    const VP_REMAP_ADDR   *pRemapAddr)
{
    MMP_UINT32  Value = 0;
    
    Value = ((pRemapAddr->Addr_04 & 0x3F) << 8) | (pRemapAddr->Addr_03 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_UV_0403, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));    
    
    Value = ((pRemapAddr->Addr_06 & 0x3F) << 8) | (pRemapAddr->Addr_05 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_UV_0605, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));    
        
    Value = ((pRemapAddr->Addr_08 & 0x3F) << 8) | (pRemapAddr->Addr_07 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_UV_0807, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_10 & 0x3F) << 8) | (pRemapAddr->Addr_09 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_UV_1009, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_12 & 0x3F) << 8) | (pRemapAddr->Addr_11 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_UV_1211, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_14 & 0x3F) << 8) | (pRemapAddr->Addr_13 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_UV_1413, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_16 & 0x3F) << 8) | (pRemapAddr->Addr_15 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_UV_1615, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_18 & 0x3F) << 8) | (pRemapAddr->Addr_17 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_UV_1817, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_20 & 0x3F) << 8) | (pRemapAddr->Addr_19 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_UV_2019, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_22 & 0x3F) << 8) | (pRemapAddr->Addr_21 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_UV_2221, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_24 & 0x3F) << 8) | (pRemapAddr->Addr_23 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_UV_2423, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_26 & 0x3F) << 8) | (pRemapAddr->Addr_25 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_UV_2625, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_28 & 0x3F) << 8) | (pRemapAddr->Addr_27 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_UV_2827, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_30 & 0x3F) << 8) | (pRemapAddr->Addr_29 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_UV_3029, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
    Value = (pRemapAddr->Addr_31 & 0x3F);
    HOST_WriteRegister(VP_REG_MAPADR_UV_XX31, (MMP_UINT16)(Value & VP_BIT_MAPADDR_Y));
    
}
    
//=============================================================================
/**
* Set Scene Change Parameter
*/
//=============================================================================
void
VP_SetSceneChange_Reg(
    const VP_SCENE_CHANGE   *pSceneChg)
{
    HOST_WriteRegister(VP_REG_SCENE_HOFFSET, (MMP_UINT16)(pSceneChg->H_Offset & VP_BIT_SCENE_HOFFSET));
    HOST_WriteRegister(VP_REG_SCENE_VOFFSET, (MMP_UINT16)(pSceneChg->V_Offset & VP_BIT_SCENE_VOFFSET));
    HOST_WriteRegister(VP_REG_SCENE_HSTEP, (MMP_UINT16)(pSceneChg->H_Step & VP_BIT_SCENE_HSTEP));
    HOST_WriteRegister(VP_REG_SCENE_VSTEP, (MMP_UINT16)(pSceneChg->V_Step & VP_BIT_SCENE_VSTEP));
    HOST_WriteRegister(VP_REG_SCENE_STEPNO, (MMP_UINT16)(pSceneChg->Step_No & VP_BIT_SCENE_STEPNO));
    //HOST_WriteRegister(VP_REG_SCENE_INITVALUE, (MMP_UINT16)(pSceneChg->BufInitValue & VP_BIT_SCENE_INITVALUE));
}

void
VP_InitSceneChange_Reg(
    const VP_SCENE_CHANGE   *pSceneChg)
{
    HOST_WriteRegister(VP_REG_SCENE_INITVALUE, (MMP_UINT16)(pSceneChg->BufInitValue & VP_BIT_SCENE_INITVALUE));
}

//=============================================================================
/**
 * Return ISP Scene Change total diff
 */
//=============================================================================
MMP_UINT16
VP_SceneChgTotalDiff_Reg(
    void)
{
    MMP_UINT16  Value = 0;

    HOST_ReadRegister(VP_REG_VP_ENGINE_STATUS_3, &Value);

    return (MMP_UINT16)Value;
}

//=============================================================================
/**
 * Wait ISP engine idle!  //for JPG module use
 */
//=============================================================================
MMP_RESULT
VP_WaitEngineIdle(
    void)
{
    VP_RESULT  result = VP_SUCCESS;
    MMP_UINT16  status = 0;
    MMP_UINT16  timeOut = 0;

    //
    //  Wait ISP engine idle!   0x6FC D[0]  0: idle, 1: busy
    //
    HOST_ReadRegister(VP_REG_VP_ENGINE_STATUS, (MMP_UINT16*)&status);
    while( status & 0x0001 )
    {
        PalSleep(1);
        if( ++timeOut > 2000 )
        {
            //VP_LogReg();
            VP_msg_ex(VP_MSG_TYPE_ERR, "ERROR_VP_NOT_IDLE \n");
            result = VP_ERR_NOT_IDLE;
            goto end;
        }
        HOST_ReadRegister(VP_REG_VP_ENGINE_STATUS, (MMP_UINT16*)&status);
    }

end:
    if( result )
        VP_msg_ex(VP_MSG_TYPE_ERR, " %s() err 0%x !", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * Is ISP engine idle! 
 */
//=============================================================================
MMP_BOOL
VP_IsEngineIdle(
    void)
{
    MMP_UINT16  status = 0, status2 = 0;

    HOST_ReadRegister(VP_REG_VP_ENGINE_STATUS, (MMP_UINT16*)&status);
    HOST_ReadRegister(VP_REG_VP_ENGINE_STATUS_2, (MMP_UINT16*)&status2);
    
    if (((status & 0xFF0F) == 0x7504) && ((status2 & 0xFFFF) == 0x0000))
        return MMP_TRUE;
    else
        return MMP_FALSE;       
}

//=============================================================================
/**
 * Wait ISP interrupt idle!
 */
//=============================================================================
MMP_RESULT
VP_WaitInterruptIdle(
    void)
{
    VP_RESULT  result = VP_SUCCESS;
    MMP_UINT16  status = 0;
    MMP_UINT16  timeOut = 0;

    //
    //  Wait ISP interrupt idle!   0x6FE D[8]  0: idle, 1: busy
    //
    HOST_ReadRegister(VP_REG_VP_ENGINE_STATUS_2, (MMP_UINT16*)&status);
    while( status & 0x0100 )
    {
        PalSleep(1);
        if( ++timeOut > 2000 )
        {
            //VP_LogReg();
            VP_msg_ex(VP_MSG_TYPE_ERR, "ERROR_VP_INTERRUPT_NOT_IDLE \n");
            result = VP_ERR_NOT_IDLE;
            goto end;
        }
        HOST_ReadRegister(VP_REG_VP_ENGINE_STATUS_2, (MMP_UINT16*)&status);
    }

end:
    if( result )
        VP_msg_ex(VP_MSG_TYPE_ERR, " %s() err 0%x !", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
* Set Interrupt Information
*/
//=============================================================================
void
VP_SetInterruptParameter_Reg(
    const VP_CONTEXT *pISPctxt)
{
    MMP_UINT16  Value = 0;
    
    //Set VP_REG_SET50E
    Value = ((pISPctxt->EnableInterrupt & VP_BIT_VP_INTERRUPT_EN) << VP_SHT_VP_INTERRUPT_EN) |
            ((pISPctxt->InterruptMode & VP_BIT_VP_INTERRUPT_MODE) << VP_SHT_VP_INTERRUPT_MODE);
            
    HOST_WriteRegister(VP_REG_SET50E, (MMP_UINT16)Value);
    //HOST_WriteRegisterMask(VP_REG_SET50E, (pISPctxt->InterruptMode & VP_BIT_VP_INTERRUPT_MODE) << VP_SHT_VP_INTERRUPT_MODE, (VP_BIT_VP_INTERRUPT_MODE << VP_SHT_VP_INTERRUPT_MODE));
}

//=============================================================================
/**
 * Return ISP Write Buffer Index.
 */
//=============================================================================
MMP_UINT16
VP_RetrunWrBufIndex_Reg(
    void)
{   
    MMP_UINT16  Value = 0;
                
    HOST_ReadRegister(VP_REG_VP_ENGINE_STATUS, &Value);
    
    Value = (Value >> 4) & 0x7;
    
    return (MMP_UINT16)Value;
}

//=============================================================================
/**
 * Wait ISP change idle!
 */
//=============================================================================
MMP_RESULT
VP_WaitISPChangeIdle(
    void)
{
    VP_RESULT  result = VP_SUCCESS;
    MMP_UINT16  status = 0;
    MMP_UINT16  timeOut = 0;

    //
    //  Wait ISP change idle!   0x6FC D[3]  0: idle, 1: busy
    //
    HOST_ReadRegister(VP_REG_VP_ENGINE_STATUS, (MMP_UINT16*)&status);

    while( (status & 0x0008) )
    {
        PalSleep(1);

        if( ++timeOut > 2000 )
        {
            //VP_LogReg();
            VP_msg_ex(VP_MSG_TYPE_ERR, "ERROR_VP_CHANGE_NOT_IDLE \n");
            result = VP_ERR_NOT_IDLE;
            goto end;
        }
        HOST_ReadRegister(VP_REG_VP_ENGINE_STATUS, (MMP_UINT16*)&status);
    }

end:
    if( result )
        VP_msg_ex(VP_MSG_TYPE_ERR, " %s() err 0%x !", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

void 
HOST_VP_EnableClock(
    void)
{
    HOST_WriteRegisterMask(MMP_ISP_CLOCK_REG_32, 0xFFFF, MMP_ISP_EN_N5CLK | MMP_ISP_EN_M5CLK | MMP_ISP_EN_ICLK);
    HOST_WriteRegisterMask(MMP_ISP_CLOCK_REG_30, 0xFFFF, MMP_ISP_EN_DIV_ICLK);
    HOST_WriteRegisterMask(MMP_HOST_BUS_CONTROLLER_REG_202, 0xFFFF, MMP_HOST_BUS_EN_MMIO_ISP);
}

void 
HOST_VP_DisableClock(
    void)
{
    HOST_WriteRegisterMask(MMP_ISP_CLOCK_REG_32, 0x0000, MMP_ISP_EN_N5CLK | MMP_ISP_EN_M5CLK | MMP_ISP_EN_ICLK);
    HOST_WriteRegisterMask(MMP_ISP_CLOCK_REG_30, 0x0000, MMP_ISP_EN_DIV_ICLK);
    HOST_WriteRegisterMask(MMP_HOST_BUS_CONTROLLER_REG_202, 0x0000, MMP_HOST_BUS_EN_MMIO_ISP);
}

void 
HOST_VP_Reset(
    void)
{
	HOST_WriteRegisterMask(MMP_ISP_CLOCK_REG_32, 0xFFFF, MMP_ISP_RESET | MMP_ISPGBL_RESET);
	MMP_Sleep(1);
	HOST_WriteRegisterMask(MMP_ISP_CLOCK_REG_32, 0x0000, MMP_ISP_RESET | MMP_ISPGBL_RESET);
	MMP_Sleep(1);
}
//=============================================================================
/**
 * ISP engine clock related.
 */
//=============================================================================
void
VP_PowerUp(
    void)
{
   HOST_VP_EnableClock();
   HOST_VP_Reset();
}

void
VP_PowerDown(
    void)
{
    HOST_VP_Reset();
    HOST_VP_DisableClock();
}

void
VP_EnableClock(
    void)
{
    HOST_VP_EnableClock();  
}

void
VP_DisableClock(
    void)
{    
    HOST_VP_DisableClock();
}

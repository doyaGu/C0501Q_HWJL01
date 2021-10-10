#if defined(ISSUE_CODE)
    #include "cmq/cmd_queue.h"
#endif

#include "isp_types.h"
#include "isp_hw.h"
#include "isp_reg.h"
#include "isp_queue.h"
#include "isp_util.h"
#include "isp_hw_op.h"
#include "../../ith/it9070/ith_lcd.h"

//=============================================================================
//                        Constant Definition
//=============================================================================

//=============================================================================
//                        Macro Definition
//=============================================================================

//=============================================================================
//                        Structure Definition
//=============================================================================

//=============================================================================
//                        Global Data Definition
//=============================================================================

//=============================================================================
//                        Private Function Definition
//=============================================================================

//=============================================================================
//                        Public Function Definition
//=============================================================================
void
ISP_LogReg(
    void)
{
    MMP_UINT16 reg, p;
    MMP_UINT   i, j, count;

    reg   = ISP_REG_BASE;
    count = (0x06FE - reg) / sizeof(MMP_UINT16);

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
 * Driver Fire ISP Engine.
 */
//=============================================================================
void
ISP_DriverFire_Reg(
    void)
{
    ISP_FireIspQueue();

    ISP_CMD_QUEUE_WAIT(1 * 2 * 4); // 1 register = (addr + value), addr/value = 4 byte

    ISP_WriteRegister(ISP_REG_SET500, (MMP_UINT16)((0x1 & ISP_BIT_DRIVER_FIRE_EN) << ISP_SHT_DRIVER_FIRE_EN));

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Driver Update ISP Parameter.
 */
//=============================================================================
void
ISP_UpdateFire_Reg(
    void)
{
    ISP_FireIspQueue();

    ISP_CMD_QUEUE_WAIT(1 * 2 * 4);

    ISP_WriteRegister(ISP_REG_SET500, (MMP_UINT16)((0x1 & ISP_BIT_ISP_UPDATE_PARM_EN) << ISP_SHT_ISP_UPDATE_PARM_EN));

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Driver Write Raw Slice Buffer ISP Engine.
 */
//=============================================================================
void
ISP_WirteRawSliceFire_Reg(
    void)
{
    ISP_CMD_QUEUE_WAIT(1 * 2 * 4);

    ISP_WriteRegister(ISP_REG_SET500, (MMP_UINT16)((0x1 & ISP_BIT_ISP_WIRTE_SLICE_FIRE_EN) << ISP_SHT_ISP_WIRTE_SLICE_FIRE_EN));

    ISP_CMD_QUEUE_FIRE();
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

    ISP_CMD_QUEUE_WAIT(2 * 2 * 4);

    Value = ((pEngineMode->TotalSliceNum     & ISP_BIT_TOTALSLICENUM)        << ISP_SHT_TOTALSLICENUM) |
            ((pEngineMode->EnableJPEGDECODE  & ISP_BIT_JPEGDECODE_MODE)      << ISP_SHT_JPEGDECODE_MODE) |
            ((pEngineMode->EnableBlockMode   & ISP_BIT_BLOCK_MODE)           << ISP_SHT_BLOCK_MODE);
    ISP_WriteRegister(ISP_REG_ENGINEMODE_PARM, (MMP_UINT16)Value);

    Value = ((pEngineMode->WriteRawSliceNum  & ISP_BIT_RAWDATA_SLICE_NUM)    << ISP_SHT_RAWDATA_SLICE_NUM) |
            ((pEngineMode->EnableRawDataMode & ISP_BIT_RAWDATA_HANDSHARK_EN) << ISP_SHT_RAWDATA_HANDSHARK_EN);
    ISP_WriteRegister(ISP_REG_RAWDATA_HANDSHARK, (MMP_UINT16)Value);

    ISP_CMD_QUEUE_FIRE();
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

    ISP_CMD_QUEUE_WAIT(1 * 2 * 4);

    if (pInInfo->EnableYUVPlaneMode)
    {
        Value = (0x1 & ISP_BIT_IN_YUVPLANE_EN) << ISP_SHT_IN_YUVPLANE_EN;
        Value = Value | (pInInfo->PlaneFormat & ISP_BIT_IN_PLANE_FORMAT) << ISP_SHT_IN_PLANE_FORMAT;
    }
    else if (pInInfo->EnableYUVPackMode == YUVPacket)
    {
        Value = (0x1 & ISP_BIT_IN_YUVPACKET_EN) << ISP_SHT_IN_YUVPACKET_EN;
        Value = Value | (pInInfo->PacketFormat & ISP_BIT_IN_PACKET_FORMAT) << ISP_SHT_IN_PACKET_FORMAT;
    }
    else if (pInInfo->EnableRGB565)
    {
        Value = (0x1 & ISP_BIT_IN_RGB565_EN) << ISP_SHT_IN_RGB565_EN;
    }
    else if (pInInfo->EnableRGB888)
    {
        Value = (0x1 & ISP_BIT_IN_RGB888_EN) << ISP_SHT_IN_RGB888_EN;
    }
    else if (pInInfo->EnableNVMode)
    {
        Value = (0x1 & ISP_BIT_IN_NV_EN) << ISP_SHT_IN_NV_EN;
        Value = Value | (pInInfo->NVFormat & ISP_BIT_IN_NV_FORMAT) << ISP_SHT_IN_NV_FORMAT;
    }

    Value = Value |
            ((pInInfo->EnableRdRqDoubleLine & ISP_BIT_IN_RDRQ_DOUBLE_LINE) << ISP_SHT_IN_RDRQ_DOUBLE_LINE) |
            ((pInInfo->EnableInYUV255Range & ISP_BIT_IN_YUV255RANGE_EN) << ISP_SHT_IN_YUV255RANGE_EN);
    ISP_WriteRegister(ISP_REG_INPUT_FORMAT, (MMP_UINT16)Value);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Set ISP input buffer relate parameters.
 */
//=============================================================================
void
ISP_SetInputBuf_Reg(
    ISP_DEVICE           ptDev,
    const ISP_INPUT_INFO *pInInfo)
{
    MMP_UINT16  Value     = 0;
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    ISP_CMD_QUEUE_WAIT(10 * 2 * 4);

    ISP_WriteRegister(ISP_REG_INPUT_WIDTH,  (MMP_UINT16)(pInInfo->SrcWidth  & ISP_BIT_INPUT_WIDTH));
    ISP_WriteRegister(ISP_REG_INPUT_HEIGHT, (MMP_UINT16)(pInInfo->SrcHeight & ISP_BIT_INPUT_HEIGHT));

    if (pISPctxt->OutInfo.EnableFieldScale)
    {
        if (pISPctxt->OutInfo.EnableProgFieldMode)
            ISP_WriteRegister(ISP_REG_INPUT_PITCH_Y, (MMP_UINT16)((pInInfo->PitchY) & ISP_BIT_INPUT_PITCH_Y));
        else
            ISP_WriteRegister(ISP_REG_INPUT_PITCH_Y, (MMP_UINT16)((pInInfo->PitchY << 1) & ISP_BIT_INPUT_PITCH_Y));

        if (pISPctxt->OutInfo.EnableProgFieldMode)
            ISP_WriteRegister(ISP_REG_INPUT_PITCH_UV, (MMP_UINT16)((pInInfo->PitchUV) & ISP_BIT_INPUT_PITCH_UV));
        else if (pISPctxt->DeInterlace.UVRepeatMode)
            ISP_WriteRegister(ISP_REG_INPUT_PITCH_UV, (MMP_UINT16)((pInInfo->PitchUV << 1) & ISP_BIT_INPUT_PITCH_UV));
        else if (!pISPctxt->DeInterlace.UVRepeatMode)
        {
            if (pInInfo->PlaneFormat == YUV420 || pInInfo->PlaneFormat == YUV422R)
                ISP_WriteRegister(ISP_REG_INPUT_PITCH_UV, (MMP_UINT16)(pInInfo->PitchUV & ISP_BIT_INPUT_PITCH_UV));
            else
                ISP_WriteRegister(ISP_REG_INPUT_PITCH_UV, (MMP_UINT16)((pInInfo->PitchUV << 1) & ISP_BIT_INPUT_PITCH_UV));
        }
    }
    else
    {
        ISP_WriteRegister(ISP_REG_INPUT_PITCH_Y,  (MMP_UINT16)(pInInfo->PitchY  & ISP_BIT_INPUT_PITCH_Y));
        ISP_WriteRegister(ISP_REG_INPUT_PITCH_UV, (MMP_UINT16)(pInInfo->PitchUV & ISP_BIT_INPUT_PITCH_UV));
    }

    ISP_WriteRegister(ISP_REG_PANEL_SRCPOS_X,   (MMP_UINT16)(pInInfo->SrcPosX     & ISP_BIT_PANEL_SRCPOS_X));
    ISP_WriteRegister(ISP_REG_PANEL_SRCPOS_Y,   (MMP_UINT16)(pInInfo->SrcPosY     & ISP_BIT_PANEL_SRCPOS_Y));
    ISP_WriteRegister(ISP_REG_PANEL_SRC_WIDTH,  (MMP_UINT16)(pInInfo->PanelWidth  & ISP_BIT_PANEL_SRC_WIDTH));
    ISP_WriteRegister(ISP_REG_PANEL_SRC_HEIGHT, (MMP_UINT16)(pInInfo->PanelHeight & ISP_BIT_PANEL_SRC_HEIGHT));

    // color
    Value = ((pInInfo->PanelColorY & ISP_BIT_PANEL_COLOR) << ISP_SHT_PANEL_COLOR_Y) |
            ((pInInfo->PanelColorU & ISP_BIT_PANEL_COLOR) << ISP_SHT_PANEL_COLOR_U);

    ISP_WriteRegister(ISP_REG_PANEL_COLOR_YU, (MMP_UINT16)Value);
    ISP_WriteRegister(ISP_REG_PANEL_COLOR_V,  (MMP_UINT16)(pInInfo->PanelColorV & ISP_BIT_PANEL_COLOR));

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Set ISP input buffer address relate parameters
 */
//=============================================================================
void
ISP_SetInputAddr_Reg(
    ISP_DEVICE           ptDev,
    const ISP_INPUT_INFO *pInInfo)
{
    MMP_UINT32  Value        = 0;
    MMP_UINT32  vramBaseAddr = (MMP_UINT32)isp_GetVramBaseAddr();
    ISP_CONTEXT *pISPctxt    = (ISP_CONTEXT *)ptDev;

    ISP_CMD_QUEUE_WAIT(12 * 2 * 4);

    if (pISPctxt->OutInfo.EnableFieldScale == MMP_TRUE)
    {
        //Top Field Y
        if (pISPctxt->OutInfo.EnableKeepLastField)
        {
            Value = (pISPctxt->top_field_first)
                    ? (MMP_UINT32)pInInfo->AddrY - vramBaseAddr  + pInInfo->PitchY
                    : (MMP_UINT32)pInInfo->AddrYp - vramBaseAddr + pInInfo->PitchY;
        }
        else
            Value = (MMP_UINT32)pInInfo->AddrY - vramBaseAddr;

        ISP_WriteRegister(ISP_REG_INPUT_ADDR_YL, (MMP_UINT16)(Value         & ISP_BIT_INPUT_ADDR_L));
        ISP_WriteRegister(ISP_REG_INPUT_ADDR_YH, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));

        //Bottom Field Y
        Value = (pISPctxt->top_field_first)
                ? (MMP_UINT32)pInInfo->AddrY  - vramBaseAddr + pInInfo->PitchY
                : (MMP_UINT32)pInInfo->AddrYp - vramBaseAddr + pInInfo->PitchY;

        ISP_WriteRegister(ISP_REG_INPUT_ADDR_YPL, (MMP_UINT16)(Value         & ISP_BIT_INPUT_ADDR_L));
        ISP_WriteRegister(ISP_REG_INPUT_ADDR_YPH, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));

        //Top Field UV
        if (pISPctxt->OutInfo.EnableKeepLastField)
        {
            Value = (pISPctxt->top_field_first)
                    ? (MMP_UINT32)pInInfo->AddrU  - vramBaseAddr + pInInfo->PitchUV
                    : (MMP_UINT32)pInInfo->AddrUp - vramBaseAddr + pInInfo->PitchUV;
        }
        else
            Value = (MMP_UINT32)pInInfo->AddrU - vramBaseAddr;

        ISP_WriteRegister(ISP_REG_INPUT_ADDR_UL, (MMP_UINT16)(Value         & ISP_BIT_INPUT_ADDR_L));
        ISP_WriteRegister(ISP_REG_INPUT_ADDR_UH, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));

        if (pISPctxt->OutInfo.EnableKeepLastField)
        {
            Value = (pISPctxt->top_field_first)
                    ? (MMP_UINT32)pInInfo->AddrV  - vramBaseAddr + pInInfo->PitchUV
                    : (MMP_UINT32)pInInfo->AddrVp - vramBaseAddr + pInInfo->PitchUV;
        }
        else
            Value = (MMP_UINT32)pInInfo->AddrV - vramBaseAddr;

        ISP_WriteRegister(ISP_REG_INPUT_ADDR_VL, (MMP_UINT16)(Value         & ISP_BIT_INPUT_ADDR_L));
        ISP_WriteRegister(ISP_REG_INPUT_ADDR_VH, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));

        if (pISPctxt->DeInterlace.UVRepeatMode == MMP_TRUE)
        {
            //Bottom Field UV
            Value = (pISPctxt->top_field_first)
                    ? (MMP_UINT32)pInInfo->AddrU  - vramBaseAddr + pInInfo->PitchUV
                    : (MMP_UINT32)pInInfo->AddrUp - vramBaseAddr + pInInfo->PitchUV;

            ISP_WriteRegister(ISP_REG_INPUT_ADDR_UPL, (MMP_UINT16)(Value         & ISP_BIT_INPUT_ADDR_L));
            ISP_WriteRegister(ISP_REG_INPUT_ADDR_UPH, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));

            Value = (pISPctxt->top_field_first)
                    ? (MMP_UINT32)pInInfo->AddrV  - vramBaseAddr + pInInfo->PitchUV
                    : (MMP_UINT32)pInInfo->AddrVp - vramBaseAddr + pInInfo->PitchUV;

            ISP_WriteRegister(ISP_REG_INPUT_ADDR_VPL, (MMP_UINT16)(Value         & ISP_BIT_INPUT_ADDR_L));
            ISP_WriteRegister(ISP_REG_INPUT_ADDR_VPH, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));
        }
        else if (pISPctxt->DeInterlace.UVRepeatMode == MMP_FALSE)
        {
            if (pInInfo->PlaneFormat == YUV420 || pInInfo->PlaneFormat == YUV422R)
            {
                //Bottom Field UV
                Value = (pISPctxt->top_field_first)
                        ? (MMP_UINT32)pInInfo->AddrU - vramBaseAddr
                        : (MMP_UINT32)pInInfo->AddrUp - vramBaseAddr;

                ISP_WriteRegister(ISP_REG_INPUT_ADDR_UPL, (MMP_UINT16)(Value         & ISP_BIT_INPUT_ADDR_L));
                ISP_WriteRegister(ISP_REG_INPUT_ADDR_UPH, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));

                Value = (pISPctxt->top_field_first)
                        ? (MMP_UINT32)pInInfo->AddrV - vramBaseAddr
                        : (MMP_UINT32)pInInfo->AddrVp - vramBaseAddr;

                ISP_WriteRegister(ISP_REG_INPUT_ADDR_VPL, (MMP_UINT16)(Value         & ISP_BIT_INPUT_ADDR_L));
                ISP_WriteRegister(ISP_REG_INPUT_ADDR_VPH, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));
            }
            else
            {
                //Bottom Field UV
                Value = (pISPctxt->top_field_first)
                        ? (MMP_UINT32)pInInfo->AddrU  - vramBaseAddr + pInInfo->PitchUV
                        : (MMP_UINT32)pInInfo->AddrUp - vramBaseAddr + pInInfo->PitchUV;

                ISP_WriteRegister(ISP_REG_INPUT_ADDR_UPL, (MMP_UINT16)(Value         & ISP_BIT_INPUT_ADDR_L));
                ISP_WriteRegister(ISP_REG_INPUT_ADDR_UPH, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));

                Value = (pISPctxt->top_field_first)
                        ? (MMP_UINT32)pInInfo->AddrV  - vramBaseAddr + pInInfo->PitchUV
                        : (MMP_UINT32)pInInfo->AddrVp - vramBaseAddr + pInInfo->PitchUV;

                ISP_WriteRegister(ISP_REG_INPUT_ADDR_VPL, (MMP_UINT16)(Value         & ISP_BIT_INPUT_ADDR_L));
                ISP_WriteRegister(ISP_REG_INPUT_ADDR_VPH, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));
            }
        }
    }
    else
    {
        // CurFrame
        Value = (MMP_UINT32)pInInfo->AddrY - vramBaseAddr;
        ISP_WriteRegister(ISP_REG_INPUT_ADDR_YL, (MMP_UINT16)(Value         & ISP_BIT_INPUT_ADDR_L));
        ISP_WriteRegister(ISP_REG_INPUT_ADDR_YH, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));

        Value = (MMP_UINT32)pInInfo->AddrU - vramBaseAddr;
        ISP_WriteRegister(ISP_REG_INPUT_ADDR_UL, (MMP_UINT16)(Value         & ISP_BIT_INPUT_ADDR_L));
        ISP_WriteRegister(ISP_REG_INPUT_ADDR_UH, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));

        Value = (MMP_UINT32)pInInfo->AddrV - vramBaseAddr;
        ISP_WriteRegister(ISP_REG_INPUT_ADDR_VL, (MMP_UINT16)(Value         & ISP_BIT_INPUT_ADDR_L));
        ISP_WriteRegister(ISP_REG_INPUT_ADDR_VH, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));

        // PreFrame
        Value = (MMP_UINT32)pInInfo->AddrYp - vramBaseAddr;
        ISP_WriteRegister(ISP_REG_INPUT_ADDR_YPL, (MMP_UINT16)(Value         & ISP_BIT_INPUT_ADDR_L));
        ISP_WriteRegister(ISP_REG_INPUT_ADDR_YPH, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));

        Value = (MMP_UINT32)pInInfo->AddrUp - vramBaseAddr;
        ISP_WriteRegister(ISP_REG_INPUT_ADDR_UPL, (MMP_UINT16)(Value         & ISP_BIT_INPUT_ADDR_L));
        ISP_WriteRegister(ISP_REG_INPUT_ADDR_UPH, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));

        Value = (MMP_UINT32)pInInfo->AddrVp - vramBaseAddr;
        ISP_WriteRegister(ISP_REG_INPUT_ADDR_VPL, (MMP_UINT16)(Value         & ISP_BIT_INPUT_ADDR_L));
        ISP_WriteRegister(ISP_REG_INPUT_ADDR_VPH, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));
    }

    ISP_CMD_QUEUE_FIRE();
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

    ISP_CMD_QUEUE_WAIT(5 * 2 * 4);

    Value = ((pDeInterlace->LowLevelBypassBlend   & ISP_BIT_LOWLEVELBYPASSBLEND)    << ISP_SHT_LOWLEVELBYPASSBLEND) |
            ((pDeInterlace->EnLowLevelOutside     & ISP_BIT_LOWLEVELOUTSIDE_EN)     << ISP_SHT_LOWLEVELOUTSIDE_EN) |
            ((pDeInterlace->LowLevelMode          & ISP_BIT_LOWLEVELMODE)           << ISP_SHT_LOWLEVELMODE) |
            ((pDeInterlace->EnLowLevelEdge        & ISP_BIT_LOWLEVELEDGE_EN)        << ISP_SHT_LOWLEVELEDGE_EN) |
            ((pDeInterlace->UVRepeatMode          & ISP_BIT_UVREPEAT_MODE)          << ISP_SHT_UVREPEAT_MODE) |
            ((pDeInterlace->EnChromaEdgeDetect    & ISP_BIT_CHROMA_EDGEDET_EN)      << ISP_SHT_CHROMA_EDGEDET_EN) |
            ((pDeInterlace->EnLummaEdgeDetect     & ISP_BIT_LUMA_EDGEDET_EN)        << ISP_SHT_LUMA_EDGEDET_EN) |
            ((pDeInterlace->Enable                & ISP_BIT_DEINTERLACE_EN)         << ISP_SHT_DEINTERLACE_EN) |
            ((pDeInterlace->DeinterMode           & ISP_BIT_2D_DEINTER_MODE_EN)     << ISP_SHT_2D_DEINTER_MODE_EN) |
            ((pDeInterlace->EnSrcBottomFieldFirst & ISP_BIT_SRC_BOTTOM_FIELD_FIRST) << ISP_SHT_SRC_BOTTOM_FIELD_FIRST) |
            ((pDeInterlace->EnDeinterBottomField  & ISP_BIT_DEINTER_BOTTOM_EN)      << ISP_SHT_DEINTER_BOTTOM_EN) |
            ((pDeInterlace->EnSrcLPF              & ISP_BIT_SRC_LPFITR_EN)          << ISP_SHT_SRC_LPFITR_EN) |
            ((pDeInterlace->EnAutoSwapField       & ISP_BIT_AUTO_SWAP_FIELD)        << ISP_SHT_AUTO_SWAP_FIELD);

    ISP_WriteRegister(ISP_REG_SET_DEINTERLACE,      (MMP_UINT16)Value);

    ISP_WriteRegister(ISP_REG_LOWLEVELEDGE_START_X, (MMP_UINT16)pDeInterlace->LowLevelPosX);
    ISP_WriteRegister(ISP_REG_LOWLEVELEDGE_START_Y, (MMP_UINT16)pDeInterlace->LowLevelPosY);
    ISP_WriteRegister(ISP_REG_LOWLEVELEDGE_WIDTH,   (MMP_UINT16)pDeInterlace->LowLevelWidth);
    ISP_WriteRegister(ISP_REG_LOWLEVELEDGE_HEIGHT,  (MMP_UINT16)pDeInterlace->LowLevelHeight);

    ISP_CMD_QUEUE_FIRE();

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

    ISP_CMD_QUEUE_WAIT(3 * 2 * 4);

    //Parameter 1
    Value = ((pDeInterlace->MDThreshold_High & ISP_BIT_3D_MDTHRED_HIGH) << ISP_SHT_3D_MDTHRED_HIGH) |
            ((pDeInterlace->MDThreshold_Low  & ISP_BIT_3D_MDTHRED_LOW)  << ISP_SHT_3D_MDTHRED_LOW);

    ISP_WriteRegister(ISP_REG_3D_DEINTER_PARM_1, (MMP_UINT16)Value);

    //Parameter 2
    Value = ((pDeInterlace->DisableMV_A      & ISP_BIT_DISABLE_MOTIONVALUE_A) << ISP_SHT_DISABLE_MOTIONVALUE_A) |
            ((pDeInterlace->DisableMV_B      & ISP_BIT_DISABLE_MOTIONVALUE_B) << ISP_SHT_DISABLE_MOTIONVALUE_B) |
            ((pDeInterlace->DisableMV_C      & ISP_BIT_DISABLE_MOTIONVALUE_C) << ISP_SHT_DISABLE_MOTIONVALUE_C) |
            ((pDeInterlace->DisableMV_D      & ISP_BIT_DISABLE_MOTIONVALUE_D) << ISP_SHT_DISABLE_MOTIONVALUE_D) |
            ((pDeInterlace->DisableMV_E      & ISP_BIT_DISABLE_MOTIONVALUE_E) << ISP_SHT_DISABLE_MOTIONVALUE_E) |
            ((pDeInterlace->DisableMV_F      & ISP_BIT_DISABLE_MOTIONVALUE_F) << ISP_SHT_DISABLE_MOTIONVALUE_F) |
            ((pDeInterlace->DisableMV_G      & ISP_BIT_DISABLE_MOTIONVALUE_G) << ISP_SHT_DISABLE_MOTIONVALUE_G) |
            ((pDeInterlace->EnLPFWeight      & ISP_BIT_LPF_WEIGHT_EN)         << ISP_SHT_LPF_WEIGHT_EN) |
            ((pDeInterlace->EnLPFBlend       & ISP_BIT_LPF_BLEND_EN)          << ISP_SHT_LPF_BLEND_EN) |
            ((pDeInterlace->MDThreshold_Step & ISP_BIT_3D_MDTHRED_STEP)       << ISP_SHT_3D_MDTHRED_STEP);

    ISP_WriteRegister(ISP_REG_3D_DEINTER_PARM_2, (MMP_UINT16)Value);

    //Parameter 3
    Value = ((pDeInterlace->EnLPFStaticPixel & ISP_BIT_LPF_STATICPIXEL_EN) << ISP_SHT_LPF_STATICPIXEL_EN);

    ISP_WriteRegister(ISP_REG_3D_DEINTER_PARM_3, (MMP_UINT16)Value);

    ISP_CMD_QUEUE_FIRE();
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

    ISP_CMD_QUEUE_WAIT(1 * 2 * 4);

    Value = ((pDeInterlace->D2EdgeBlendWeight & ISP_BIT_2D_EDGE_WEIGHT) << ISP_SHT_2D_EDGE_WEIGHT) |
            ((pDeInterlace->D2OrgBlendWeight  & ISP_BIT_2D_ORG_WEIGHT)  << ISP_SHT_2D_ORG_WEIGHT);
    ISP_WriteRegister(ISP_REG_2D_DEINTER_PARM_1, (MMP_UINT16)Value);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Set Subtitle 0.
 */
//=============================================================================
void
ISP_SetSubTitle0_Reg(
    const ISP_SUBTITLE_CTRL *pSubTitle)
{
    MMP_UINT32 Value        = 0;
    MMP_UINT32 vramBaseAddr = (MMP_UINT32)isp_GetVramBaseAddr();
    MMP_UINT32 HCI;
    MMP_UINT32 VCI;

    HCI = ISP_FloatToFix(pSubTitle->HCI, 4, 10);
    VCI = ISP_FloatToFix(pSubTitle->VCI, 4, 10);

    ISP_CMD_QUEUE_WAIT(15 * 2 * 4);

    // Starting Address
    Value = (pSubTitle->Enable)
            ? (MMP_UINT32)pSubTitle->Addr - vramBaseAddr // byte align
            : 0;                                         // for Onfly

    ISP_WriteRegister(ISP_REG_SUBTITLE_0_ADDR_L,          (MMP_UINT16)(Value & ISP_BIT_SUBTITLE_ADDR_L));
    ISP_WriteRegister(ISP_REG_SUBTITLE_0_ADDR_H,          (MMP_UINT16)((Value >> 16) & ISP_BIT_SUBTITLE_ADDR_H));

    //SrcWidth, SrcHeight
    ISP_WriteRegister(ISP_REG_SUBTITLE_0_SRCWIDTH,        (MMP_UINT16)(pSubTitle->SrcWidth & ISP_BIT_SUBTITLE_SRCWIDTH));
    ISP_WriteRegister(ISP_REG_SUBTITLE_0_SRCHEIGHT,       (MMP_UINT16)(pSubTitle->SrcHeight & ISP_BIT_SUBTITLE_SRCHEIGHT));

    //HCI, VCI
    ISP_WriteRegister(ISP_REG_SUBTITLE_0_HCI,             (MMP_UINT16)(HCI & ISP_BIT_SUBTITLE_HCI));
    ISP_WriteRegister(ISP_REG_SUBTITLE_0_VCI,             (MMP_UINT16)(VCI & ISP_BIT_SUBTITLE_VCI));

    // DstWidth, DstHeight, Pitch
    ISP_WriteRegister(ISP_REG_SUBTITLE_0_DSTWIDTH,        (MMP_UINT16)(pSubTitle->DstWidth & ISP_BIT_SUBTITLE_DSTWIDTH));
    ISP_WriteRegister(ISP_REG_SUBTITLE_0_DSTHEIGHT,       (MMP_UINT16)(pSubTitle->DstHeight & ISP_BIT_SUBTITLE_DSTHEIGHT));
    ISP_WriteRegister(ISP_REG_SUBTITLE_0_PITCH,           (MMP_UINT16)(pSubTitle->Pitch & ISP_BIT_SUBTITLE_PITCH));

    //Start X/Y
    ISP_WriteRegister(ISP_REG_SUBTITLE_0_START_X,         (MMP_UINT16)(pSubTitle->StartX & ISP_BIT_SUBTITLE_START_X));
    ISP_WriteRegister(ISP_REG_SUBTITLE_0_START_Y,         (MMP_UINT16)(pSubTitle->StartY & ISP_BIT_SUBTITLE_START_Y));

    // ui decompress
    ISP_WriteRegister(ISP_REG_SUBTITLE_0_DEC_LINEBYTE,    (MMP_UINT16)(pSubTitle->UiDecLineBytes & ISP_BIT_SUBTITLE_DEC_LINEBYTE));
    ISP_WriteRegister(ISP_REG_SUBTITLE_0_DEC_TOTALBYTE_L, (MMP_UINT16)(pSubTitle->UiDecTotalBytes & ISP_REG_SUBTITLE_DEC_TOTALBYTE_L));
    ISP_WriteRegister(ISP_REG_SUBTITLE_0_DEC_TOTALBYTE_H, (MMP_UINT16)((pSubTitle->UiDecTotalBytes >> 16) & ISP_REG_SUBTITLE_DEC_TOTALBYTE_H));

    //Mode and Enable
    Value = ((pSubTitle->EnableUiDec & ISP_BIT_SUBTITLE_UIDEC_EN) << ISP_SHT_SUBTITLE_UIDEC_EN) |
            ((pSubTitle->Format      & ISP_BIT_SUBTITLE_MODE)     << ISP_SHT_SUBTITLE_MODE) |
            ((pSubTitle->Enable      & ISP_BIT_SUBTITLE_EN)       << ISP_SHT_SUBTITLE_EN);

    ISP_WriteRegister(ISP_REG_SET_SUBTITLE_0, (MMP_UINT16)Value);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Set Subtitle 1
 */
//=============================================================================
void
ISP_SetSubTitle1_Reg(
    const ISP_SUBTITLE_CTRL *pSubTitle)
{
    MMP_UINT32 Value        = 0;
    MMP_UINT32 vramBaseAddr = (MMP_UINT32)isp_GetVramBaseAddr();

    MMP_UINT32 HCI;
    MMP_UINT32 VCI;

    HCI = ISP_FloatToFix(pSubTitle->HCI, 4, 10);
    VCI = ISP_FloatToFix(pSubTitle->VCI, 4, 10);

    ISP_CMD_QUEUE_WAIT(15 * 2 * 4);

    //Starting Address
    Value = (pSubTitle->Enable)
            ? (MMP_UINT32)pSubTitle->Addr - vramBaseAddr // byte align
            : 0;                                         // for Onfly
    ISP_WriteRegister(ISP_REG_SUBTITLE_1_ADDR_L,          (MMP_UINT16)(Value & ISP_BIT_SUBTITLE_ADDR_L));
    ISP_WriteRegister(ISP_REG_SUBTITLE_1_ADDR_H,          (MMP_UINT16)((Value >> 16) & ISP_BIT_SUBTITLE_ADDR_H));

    //SrcWidth, SrcHeight
    ISP_WriteRegister(ISP_REG_SUBTITLE_1_SRCWIDTH,        (MMP_UINT16)(pSubTitle->SrcWidth & ISP_BIT_SUBTITLE_SRCWIDTH));
    ISP_WriteRegister(ISP_REG_SUBTITLE_1_SRCHEIGHT,       (MMP_UINT16)(pSubTitle->SrcHeight & ISP_BIT_SUBTITLE_SRCHEIGHT));

    //HCI, VCI
    ISP_WriteRegister(ISP_REG_SUBTITLE_1_HCI,             (MMP_UINT16)(HCI & ISP_BIT_SUBTITLE_HCI));
    ISP_WriteRegister(ISP_REG_SUBTITLE_1_VCI,             (MMP_UINT16)(VCI & ISP_BIT_SUBTITLE_VCI));

    // DstWidth, DstHeight, Pitch
    ISP_WriteRegister(ISP_REG_SUBTITLE_1_DSTWIDTH,        (MMP_UINT16)(pSubTitle->DstWidth & ISP_BIT_SUBTITLE_DSTWIDTH));
    ISP_WriteRegister(ISP_REG_SUBTITLE_1_DSTHEIGHT,       (MMP_UINT16)(pSubTitle->DstHeight & ISP_BIT_SUBTITLE_DSTHEIGHT));
    ISP_WriteRegister(ISP_REG_SUBTITLE_1_PITCH,           (MMP_UINT16)(pSubTitle->Pitch & ISP_BIT_SUBTITLE_PITCH));

    //Start X/Y
    ISP_WriteRegister(ISP_REG_SUBTITLE_1_START_X,         (MMP_UINT16)(pSubTitle->StartX & ISP_BIT_SUBTITLE_START_X));
    ISP_WriteRegister(ISP_REG_SUBTITLE_1_START_Y,         (MMP_UINT16)(pSubTitle->StartY & ISP_BIT_SUBTITLE_START_Y));

    // ui decompress
    ISP_WriteRegister(ISP_REG_SUBTITLE_1_DEC_LINEBYTE,    (MMP_UINT16)(pSubTitle->UiDecLineBytes & ISP_BIT_SUBTITLE_DEC_LINEBYTE));
    ISP_WriteRegister(ISP_REG_SUBTITLE_1_DEC_TOTALBYTE_L, (MMP_UINT16)(pSubTitle->UiDecTotalBytes & ISP_REG_SUBTITLE_DEC_TOTALBYTE_L));
    ISP_WriteRegister(ISP_REG_SUBTITLE_1_DEC_TOTALBYTE_H, (MMP_UINT16)((pSubTitle->UiDecTotalBytes >> 16) & ISP_REG_SUBTITLE_DEC_TOTALBYTE_H));

    //Mode and Enable
    Value = ((pSubTitle->EnableUiDec & ISP_BIT_SUBTITLE_UIDEC_EN) << ISP_SHT_SUBTITLE_UIDEC_EN) |
            ((pSubTitle->Format      & ISP_BIT_SUBTITLE_MODE)     << ISP_SHT_SUBTITLE_MODE) |
            ((pSubTitle->Enable      & ISP_BIT_SUBTITLE_EN)       << ISP_SHT_SUBTITLE_EN);

    ISP_WriteRegister(ISP_REG_SET_SUBTITLE_1, (MMP_UINT16)Value);

    ISP_CMD_QUEUE_FIRE();
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
    ISP_CMD_QUEUE_WAIT(12 * 2 * 4);

    ISP_WriteRegister(ISP_REG_YUV_TO_RGB_11,      (MMP_UINT16)(pYUVtoRGB->_11    & ISP_BIT_YUV_TO_RGB));
    ISP_WriteRegister(ISP_REG_YUV_TO_RGB_12,      (MMP_UINT16)(pYUVtoRGB->_12    & ISP_BIT_YUV_TO_RGB));
    ISP_WriteRegister(ISP_REG_YUV_TO_RGB_13,      (MMP_UINT16)(pYUVtoRGB->_13    & ISP_BIT_YUV_TO_RGB));
    ISP_WriteRegister(ISP_REG_YUV_TO_RGB_21,      (MMP_UINT16)(pYUVtoRGB->_21    & ISP_BIT_YUV_TO_RGB));
    ISP_WriteRegister(ISP_REG_YUV_TO_RGB_22,      (MMP_UINT16)(pYUVtoRGB->_22    & ISP_BIT_YUV_TO_RGB));
    ISP_WriteRegister(ISP_REG_YUV_TO_RGB_23,      (MMP_UINT16)(pYUVtoRGB->_23    & ISP_BIT_YUV_TO_RGB));
    ISP_WriteRegister(ISP_REG_YUV_TO_RGB_31,      (MMP_UINT16)(pYUVtoRGB->_31    & ISP_BIT_YUV_TO_RGB));
    ISP_WriteRegister(ISP_REG_YUV_TO_RGB_32,      (MMP_UINT16)(pYUVtoRGB->_32    & ISP_BIT_YUV_TO_RGB));
    ISP_WriteRegister(ISP_REG_YUV_TO_RGB_33,      (MMP_UINT16)(pYUVtoRGB->_33    & ISP_BIT_YUV_TO_RGB));
    ISP_WriteRegister(ISP_REG_YUV_TO_RGB_CONST_R, (MMP_UINT16)(pYUVtoRGB->ConstR & ISP_BIT_YUV_TO_RGB_CONST));
    ISP_WriteRegister(ISP_REG_YUV_TO_RGB_CONST_G, (MMP_UINT16)(pYUVtoRGB->ConstG & ISP_BIT_YUV_TO_RGB_CONST));
    ISP_WriteRegister(ISP_REG_YUV_TO_RGB_CONST_B, (MMP_UINT16)(pYUVtoRGB->ConstB & ISP_BIT_YUV_TO_RGB_CONST));

    ISP_CMD_QUEUE_FIRE();
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
    ISP_CMD_QUEUE_WAIT(12 * 2 * 4);

    ISP_WriteRegister(ISP_REG_COL_COR_11,      (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_11,    4, 8) & ISP_BIT_COL_COR));
    ISP_WriteRegister(ISP_REG_COL_COR_12,      (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_12,    4, 8) & ISP_BIT_COL_COR));
    ISP_WriteRegister(ISP_REG_COL_COR_13,      (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_13,    4, 8) & ISP_BIT_COL_COR));
    ISP_WriteRegister(ISP_REG_COL_COR_21,      (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_21,    4, 8) & ISP_BIT_COL_COR));
    ISP_WriteRegister(ISP_REG_COL_COR_22,      (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_22,    4, 8) & ISP_BIT_COL_COR));
    ISP_WriteRegister(ISP_REG_COL_COR_23,      (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_23,    4, 8) & ISP_BIT_COL_COR));
    ISP_WriteRegister(ISP_REG_COL_COR_31,      (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_31,    4, 8) & ISP_BIT_COL_COR));
    ISP_WriteRegister(ISP_REG_COL_COR_32,      (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_32,    4, 8) & ISP_BIT_COL_COR));
    ISP_WriteRegister(ISP_REG_COL_COR_33,      (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_33,    4, 8) & ISP_BIT_COL_COR));
    ISP_WriteRegister(ISP_REG_COL_COR_DELTA_R, (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->DeltaR, 8, 0) & ISP_BIT_COL_CORR_DELTA));
    ISP_WriteRegister(ISP_REG_COL_COR_DELTA_G, (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->DeltaG, 8, 0) & ISP_BIT_COL_CORR_DELTA));
    ISP_WriteRegister(ISP_REG_COL_COR_DELTA_B, (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->DeltaB, 8, 0) & ISP_BIT_COL_CORR_DELTA));

    ISP_CMD_QUEUE_FIRE();
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

    ISP_CMD_QUEUE_WAIT(3 * 2 * 4);

    //HCI
    ISP_WriteRegister(ISP_REG_PRESCALE_HCI_L, (MMP_UINT16)(HCI         & ISP_BIT_PRESCALE_HCI_L));
    ISP_WriteRegister(ISP_REG_PRESCALE_HCI_H, (MMP_UINT16)((HCI >> 16) & ISP_BIT_PRESCALE_HCI_H));

    //PreScale Output Width
    ISP_WriteRegister(ISP_REG_PRESCALE_WIDTH, (MMP_UINT16)(pPreScaleFun->DstWidth & ISP_BIT_PRESCALE_WIDTH));

    ISP_CMD_QUEUE_FIRE();
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

    ISP_CMD_QUEUE_WAIT(10 * 2 * 4);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[0][0], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[0][1], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX0100, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[0][2], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[0][3], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX0302, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[1][0], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[1][1], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX1110, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[1][2], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[1][3], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX1312, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[2][0], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[2][1], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX2120, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[2][2], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[2][3], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX2322, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[3][0], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[3][1], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX3130, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[3][2], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[3][3], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX3332, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[4][0], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[4][1], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX4140, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[4][2], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[4][3], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX4342, (MMP_UINT16)Value);

    ISP_CMD_QUEUE_FIRE();
}

void
ISP_SetIntPreScaleMatrix_Reg(
    MMP_UINT8 WeightMatX[][ISP_SCALE_TAP])
{
    MMP_UINT16 Value;

    ISP_CMD_QUEUE_WAIT(10 * 2 * 4);

    Value = ((WeightMatX[0][0] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[0][1] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX0100, (MMP_UINT16)Value);

    Value = ((WeightMatX[0][2] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[0][3] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX0302, (MMP_UINT16)Value);

    Value = ((WeightMatX[1][0] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[1][1] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX1110, (MMP_UINT16)Value);

    Value = ((WeightMatX[1][2] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[1][3] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX1312, (MMP_UINT16)Value);

    Value = ((WeightMatX[2][0] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[2][1] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX2120, (MMP_UINT16)Value);

    Value = ((WeightMatX[2][2] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[2][3] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX2322, (MMP_UINT16)Value);

    Value = ((WeightMatX[3][0] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[3][1] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX3130, (MMP_UINT16)Value);

    Value = ((WeightMatX[3][2] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[3][3] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX3332, (MMP_UINT16)Value);

    Value = ((WeightMatX[4][0] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[4][1] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX4140, (MMP_UINT16)Value);

    Value = ((WeightMatX[4][2] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[4][3] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX4342, (MMP_UINT16)Value);

    ISP_CMD_QUEUE_FIRE();
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
    MMP_INT    OPNPLB;
    MMP_UINT32 HCIP;
    MMP_UINT32 HCI;
    MMP_UINT32 VCI;

    OPNPLB = (MMP_INT)((MMP_FLOAT)ISP_SCALE_MAX_BLOCK_WIDTH / pScaleFun->HCI);

    if (OPNPLB % 2 == 1)
        OPNPLB = OPNPLB - 1;

    HCIP = ISP_FloatToFix((pScaleFun->HCI * OPNPLB), 9, 14);
    HCI  = ISP_FloatToFix(pScaleFun->HCI, 6, 14);
    VCI  = ISP_FloatToFix(pScaleFun->VCI, 6, 14);

    ISP_CMD_QUEUE_WAIT(14 * 2 * 4);

    //HCIP
    ISP_WriteRegister(ISP_REG_SCALE_HCIP_L, (MMP_UINT16)(HCIP & ISP_BIT_SCLAE_HCIP_L));
    ISP_WriteRegister(ISP_REG_SCLAE_HCIP_H, (MMP_UINT16)((HCIP >> 16) & ISP_BIT_SCLAE_HCIP_H));

    //VCIP
    ISP_WriteRegister(ISP_REG_SCALE_VCIP,   (MMP_UINT16)ISP_SCALE_MAX_BLOCK_HEIGHT);

    //Output pixel counts per line in a block
    ISP_WriteRegister(ISP_REG_SCLAE_OPNPLB, (MMP_UINT16)(OPNPLB & ISP_BIT_SCLAE_OPNPLB));

    //HCI
    ISP_WriteRegister(ISP_REG_SCALE_HCI_L,  (MMP_UINT16)(HCI & ISP_BIT_SCALE_L));
    ISP_WriteRegister(ISP_REG_SCALE_HCI_H,  (MMP_UINT16)((HCI >> 16) & ISP_BIT_SCALE_H));

    //VCI
    ISP_WriteRegister(ISP_REG_SCALE_VCI_L,  (MMP_UINT16)(VCI & ISP_BIT_SCALE_L));
    ISP_WriteRegister(ISP_REG_SCALE_VCI_H,  (MMP_UINT16)((VCI >> 16) & ISP_BIT_SCALE_H));

    //Background Area Color
    Value = ((pScaleFun->BGColorR & ISP_BIT_BGCOLOR) << ISP_SHT_BGCOLOR_R) |
            ((pScaleFun->BGColorG & ISP_BIT_BGCOLOR) << ISP_SHT_BGCOLOR_G);

    ISP_WriteRegister(ISP_REG_BGCOLOR_RG,      (MMP_UINT16)Value);
    ISP_WriteRegister(ISP_REG_BGCOLOR_B,       (MMP_UINT16)(pScaleFun->BGColorB & ISP_BIT_BGCOLOR));

    //Scale Output Position X and Y
    ISP_WriteRegister(ISP_REG_SCALE_DSTPOS_X,  (MMP_UINT16)(pScaleFun->DstPosX & ISP_BIT_SCALE_DSTPOS_X));
    ISP_WriteRegister(ISP_REG_SCALE_DSTPOS_Y,  (MMP_UINT16)(pScaleFun->DstPosY & ISP_BIT_SCALE_DSTPOS_Y));

    //Scale Output Width and Height
    ISP_WriteRegister(ISP_REG_SCALE_DSTWIDTH,  (MMP_UINT16)(pScaleFun->DstWidth  & ISP_BIT_SCALE_DSTWIDTH));
    ISP_WriteRegister(ISP_REG_SCALE_DSTHEIGHT, (MMP_UINT16)(pScaleFun->DstHeight & ISP_BIT_SCALE_DSTHEIGHT));

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Set Scale Horizontal Weight.
 */
//=============================================================================
void
ISP_SetScaleMatrixH_Reg(
    const ISP_SCALE_CTRL *pScaleFun)
{
    MMP_UINT16 Value;

    ISP_CMD_QUEUE_WAIT(10 * 2 * 4);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[0][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[0][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX0100, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[0][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[0][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX0302, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[1][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[1][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX1110, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[1][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[1][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX1312, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[2][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[2][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX2120, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[2][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[2][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX2322, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[3][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[3][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX3130, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[3][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[3][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX3332, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[4][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[4][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX4140, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[4][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[4][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX4342, (MMP_UINT16)Value);

    ISP_CMD_QUEUE_FIRE();
}

void
ISP_SetIntScaleMatrixH_Reg(
    MMP_UINT8 WeightMatX[][ISP_SCALE_TAP])
{
    MMP_UINT16 Value;

    ISP_CMD_QUEUE_WAIT(10 * 2 * 4);

    Value = ((WeightMatX[0][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[0][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX0100, (MMP_UINT16)Value);

    Value = ((WeightMatX[0][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[0][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX0302, (MMP_UINT16)Value);

    Value = ((WeightMatX[1][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[1][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX1110, (MMP_UINT16)Value);

    Value = ((WeightMatX[1][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[1][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX1312, (MMP_UINT16)Value);

    Value = ((WeightMatX[2][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[2][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX2120, (MMP_UINT16)Value);

    Value = ((WeightMatX[2][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[2][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX2322, (MMP_UINT16)Value);

    Value = ((WeightMatX[3][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[3][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX3130, (MMP_UINT16)Value);

    Value = ((WeightMatX[3][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[3][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX3332, (MMP_UINT16)Value);

    Value = ((WeightMatX[4][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[4][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX4140, (MMP_UINT16)Value);

    Value = ((WeightMatX[4][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[4][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX4342, (MMP_UINT16)Value);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Set Scale Vertical Weight.
 */
//=============================================================================
void
ISP_SetScaleMatrixV_Reg(
    const ISP_SCALE_CTRL *pScaleFun)
{
    MMP_UINT16 Value;

    ISP_CMD_QUEUE_WAIT(10 * 2 * 4);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[0][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[0][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY0100, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[0][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[0][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY0302, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[1][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[1][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY1110, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[1][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[1][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY1312, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[2][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[2][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY2120, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[2][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[2][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY2322, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[3][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[3][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY3130, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[3][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[3][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY3332, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[4][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[4][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY4140, (MMP_UINT16)Value);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[4][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[4][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY4342, (MMP_UINT16)Value);

    ISP_CMD_QUEUE_FIRE();
}

void
ISP_SetIntScaleMatrixV_Reg(
    MMP_UINT8 WeightMatY[][ISP_SCALE_TAP])
{
    MMP_UINT16 Value;

    ISP_CMD_QUEUE_WAIT(10 * 2 * 4);

    Value = ((WeightMatY[0][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[0][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY0100, (MMP_UINT16)Value);

    Value = ((WeightMatY[0][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[0][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY0302, (MMP_UINT16)Value);

    Value = ((WeightMatY[1][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[1][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY1110, (MMP_UINT16)Value);

    Value = ((WeightMatY[1][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[1][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY1312, (MMP_UINT16)Value);

    Value = ((WeightMatY[2][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[2][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY2120, (MMP_UINT16)Value);

    Value = ((WeightMatY[2][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[2][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY2322, (MMP_UINT16)Value);

    Value = ((WeightMatY[3][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[3][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY3130, (MMP_UINT16)Value);

    Value = ((WeightMatY[3][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[3][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY3332, (MMP_UINT16)Value);

    Value = ((WeightMatY[4][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[4][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY4140, (MMP_UINT16)Value);

    Value = ((WeightMatY[4][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[4][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY4342, (MMP_UINT16)Value);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Frmfun RGB to YUV transfer matrix.
 */
//=============================================================================
void
ISP_SetFrmMatrix_Reg(
    const ISP_RGB_TO_YUV *pMatrix)
{
    ISP_CMD_QUEUE_WAIT(12 * 2 * 4);

    ISP_WriteRegister(ISP_REG_FRM_RGB2YUV_11,      (MMP_UINT16)(pMatrix->_11    & ISP_BIT_FRM_RGB2YUV));
    ISP_WriteRegister(ISP_REG_FRM_RGB2YUV_12,      (MMP_UINT16)(pMatrix->_12    & ISP_BIT_FRM_RGB2YUV));
    ISP_WriteRegister(ISP_REG_FRM_RGB2YUV_13,      (MMP_UINT16)(pMatrix->_13    & ISP_BIT_FRM_RGB2YUV));
    ISP_WriteRegister(ISP_REG_FRM_RGB2YUV_21,      (MMP_UINT16)(pMatrix->_21    & ISP_BIT_FRM_RGB2YUV));
    ISP_WriteRegister(ISP_REG_FRM_RGB2YUV_22,      (MMP_UINT16)(pMatrix->_22    & ISP_BIT_FRM_RGB2YUV));
    ISP_WriteRegister(ISP_REG_FRM_RGB2YUV_23,      (MMP_UINT16)(pMatrix->_23    & ISP_BIT_FRM_RGB2YUV));
    ISP_WriteRegister(ISP_REG_FRM_RGB2YUV_31,      (MMP_UINT16)(pMatrix->_31    & ISP_BIT_FRM_RGB2YUV));
    ISP_WriteRegister(ISP_REG_FRM_RGB2YUV_32,      (MMP_UINT16)(pMatrix->_32    & ISP_BIT_FRM_RGB2YUV));
    ISP_WriteRegister(ISP_REG_FRM_RGB2YUV_33,      (MMP_UINT16)(pMatrix->_33    & ISP_BIT_FRM_RGB2YUV));
    ISP_WriteRegister(ISP_REG_FRM_RGB2YUV_CONST_Y, (MMP_UINT16)(pMatrix->ConstY & ISP_BIT_FRM_RGB2YUV_CONST));
    ISP_WriteRegister(ISP_REG_FRM_RGB2YUV_CONST_U, (MMP_UINT16)(pMatrix->ConstU & ISP_BIT_FRM_RGB2YUV_CONST));
    ISP_WriteRegister(ISP_REG_FRM_RGB2YUV_CONST_V, (MMP_UINT16)(pMatrix->ConstV & ISP_BIT_FRM_RGB2YUV_CONST));

    ISP_CMD_QUEUE_FIRE();
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

    ISP_CMD_QUEUE_WAIT(18 * 2 * 4);

    //Starting address
    Value = (pFrameFun->Enable)
            ? (MMP_UINT32)pFrameFun->Addr - vramBaseAddr  // byte align
            : 0;                                          // for Onfly
    ISP_WriteRegister(ISP_REG_FRMFUN_0_ADDR_L,  (MMP_UINT16)(Value         & ISP_BIT_FRMFUN_ADDR_L));
    ISP_WriteRegister(ISP_REG_FRMFUN_0_ADDR_H,  (MMP_UINT16)((Value >> 16) & ISP_BIT_FRMFUN_ADDR_H));

    //width, height, pitch
    ISP_WriteRegister(ISP_REG_FRMFUN_0_WIDTH,   (MMP_UINT16)(pFrameFun->Width  & ISP_BIT_FRMFUN_WIDTH));
    ISP_WriteRegister(ISP_REG_FRMFUN_0_HEIGHT,  (MMP_UINT16)(pFrameFun->Height & ISP_BIT_FRMFUN_HEIGHT));
    ISP_WriteRegister(ISP_REG_FRMFUN_0_PITCH,   (MMP_UINT16)(pFrameFun->Pitch  & ISP_BIT_FRMFUN_PITCH));

    //start X/Y
    ISP_WriteRegister(ISP_REG_FRMFUN_0_START_X, (MMP_UINT16)(pFrameFun->StartX & ISP_BIT_FRMFUN_START_X));
    ISP_WriteRegister(ISP_REG_FRMFUN_0_START_Y, (MMP_UINT16)(pFrameFun->StartY & ISP_BIT_FRMFUN_START_Y));

    //color key
    Value = ((pFrameFun->ColorKeyR & ISP_BIT_FRMFUN_KEY) << ISP_SHT_FRMFUN_KEY_R) |
            ((pFrameFun->ColorKeyG & ISP_BIT_FRMFUN_KEY) << ISP_SHT_FRMFUN_KEY_G);

    ISP_WriteRegister(ISP_REG_FRMFUN_0_KEY_RG, (MMP_UINT16)Value);
    ISP_WriteRegister(ISP_REG_FRMFUN_0_KEY_B,  (MMP_UINT16)(pFrameFun->ColorKeyB & ISP_BIT_FRMFUN_KEY));

    //Constant Alpha Value
    ISP_WriteRegister(ISP_REG_CONST_ALPHA_0,   (MMP_UINT16)(pFrameFun->ConstantAlpha & ISP_BIT_CONST_ALPHA));

    //format ARGB4444 or Constant Alpha with RGB565
    if (pFrameFun->Format == ARGB4444 || pFrameFun->Format == ARGB8888)
    {
        Value = ((0x1 & ISP_BIT_FRMFUN_ALPHA_BLEND_EN) << ISP_SHT_FRMFUN_ALPHA_BLEND_EN) |
                ((0x1 & ISP_BIT_FRMFUN_MODE)           << ISP_SHT_FRMFUN_MODE);
    }
    else if (pFrameFun->Format == CARGB565)
    {
        if (pFrameFun->ConstantAlpha != 0)
        {
            Value = ((0x1 & ISP_BIT_FRMFUN_ALPHA_BLEND_EN) << ISP_SHT_FRMFUN_ALPHA_BLEND_EN) |
                    ((0x0 & ISP_BIT_FRMFUN_MODE)           << ISP_SHT_FRMFUN_MODE);
        }
        else
        {
            Value = ((0x0 & ISP_BIT_FRMFUN_ALPHA_BLEND_EN) << ISP_SHT_FRMFUN_ALPHA_BLEND_EN) |
                    ((0x0 & ISP_BIT_FRMFUN_MODE)           << ISP_SHT_FRMFUN_MODE);
        }
    }

    //FrameFun Enable, BlendConst
    Value = Value |
            ((pFrameFun->Enable           & ISP_BIT_FRMFUN_EN)                  << ISP_SHT_FRMFUN_EN) |
            ((pFrameFun->EnableUiDec      & ISP_BIT_FRMFUN_UIDEC_EN)            << ISP_SHT_FRMFUN_UIDEC_EN) |
            ((pFrameFun->EnableBlendConst & ISP_BIT_FRMFUN_CONST_DATA_BLEND_EN) << ISP_SHT_FRMFUN_CONST_DATA_BLEND_EN) |
            ((pFrameFun->EnableGridConst  & ISP_BIT_FRMFUN_GRID_CONST_DATA_EN)  << ISP_SHT_FRMFUN_GRID_CONST_DATA_EN) |
            ((pFrameFun->GridDataMode     & ISP_BIT_FRMFUN_GRID_PIXEL_MODE)     << ISP_SHT_FRMFUN_GRID_PIXEL_MODE) |
            ((pFrameFun->EnableRGB2YUV    & ISP_BIT_FRMFUN_RGB2YUV_EN)          << ISP_SHT_FRMFUN_RGB2YUV_EN);

    //Enable frame function, set format
    ISP_WriteRegister(ISP_REG_SET_FRMFUN_0, (MMP_UINT16)Value);

    //Const Data
    Value = ((pFrameFun->ConstColorR0 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_LOW) |
            ((pFrameFun->ConstColorG0 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_HI);
    ISP_WriteRegister(ISP_REG_CONST_DATA_R0G0_0, (MMP_UINT16)Value);

    Value = ((pFrameFun->ConstColorB0 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_LOW) |
            ((pFrameFun->ConstColorR1 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_HI);
    ISP_WriteRegister(ISP_REG_CONST_DATA_B0R1_0, (MMP_UINT16)Value);

    Value = ((pFrameFun->ConstColorG1 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_LOW) |
            ((pFrameFun->ConstColorB1 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_HI);
    ISP_WriteRegister(ISP_REG_CONST_DATA_G1B1_0,        (MMP_UINT16)Value); //14

    // ui decompress
    ISP_WriteRegister(ISP_REG_FRMFUN_0_DEC_LINEBYTE,    (MMP_UINT16)(pFrameFun->UiDecLineBytes          & ISP_REG_FRMFUN_DEC_LINEBYTE));
    ISP_WriteRegister(ISP_REG_FRMFUN_0_DEC_TOTALBYTE_L, (MMP_UINT16)(pFrameFun->UiDecTotalBytes         & ISP_REG_FRMFUN_DEC_TOTALBYTE_L));
    ISP_WriteRegister(ISP_REG_FRMFUN_0_DEC_TOTALBYTE_H, (MMP_UINT16)((pFrameFun->UiDecTotalBytes >> 16) & ISP_REG_FRMFUN_DEC_TOTALBYTE_H));
    ISP_WriteRegister(ISP_REG_UPDATE_PARA_1,            pFrameFun->UIBufferIndex);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Set Frame Function 1
 */
//=============================================================================
void
ISP_SetFrameFun1_Reg(
    const ISP_FRMFUN_CTRL *pFrameFun)
{
    MMP_UINT32 Value        = 0;
    MMP_UINT32 vramBaseAddr = (MMP_UINT32)isp_GetVramBaseAddr();

    ISP_CMD_QUEUE_WAIT(18 * 2 * 4);

    // starting address
    Value = (pFrameFun->Enable)
            ? (MMP_UINT32)pFrameFun->Addr - vramBaseAddr // byte align
            : 0;                                         // for Onfly
    ISP_WriteRegister(ISP_REG_FRMFUN_1_ADDR_L,  (MMP_UINT16)(Value         & ISP_BIT_FRMFUN_ADDR_L));
    ISP_WriteRegister(ISP_REG_FRMFUN_1_ADDR_H,  (MMP_UINT16)((Value >> 16) & ISP_BIT_FRMFUN_ADDR_H));

    // width, height, pitch
    ISP_WriteRegister(ISP_REG_FRMFUN_1_WIDTH,   (MMP_UINT16)(pFrameFun->Width  & ISP_BIT_FRMFUN_WIDTH));
    ISP_WriteRegister(ISP_REG_FRMFUN_1_HEIGHT,  (MMP_UINT16)(pFrameFun->Height & ISP_BIT_FRMFUN_HEIGHT));
    ISP_WriteRegister(ISP_REG_FRMFUN_1_PITCH,   (MMP_UINT16)(pFrameFun->Pitch  & ISP_BIT_FRMFUN_PITCH));

    // start X/Y
    ISP_WriteRegister(ISP_REG_FRMFUN_1_START_X, (MMP_UINT16)(pFrameFun->StartX & ISP_BIT_FRMFUN_START_X));
    ISP_WriteRegister(ISP_REG_FRMFUN_1_START_Y, (MMP_UINT16)(pFrameFun->StartY & ISP_BIT_FRMFUN_START_Y));

    // color key
    Value = ((pFrameFun->ColorKeyR & ISP_BIT_FRMFUN_KEY) << ISP_SHT_FRMFUN_KEY_R) |
            ((pFrameFun->ColorKeyG & ISP_BIT_FRMFUN_KEY) << ISP_SHT_FRMFUN_KEY_G);

    ISP_WriteRegister(ISP_REG_FRMFUN_1_KEY_RG, (MMP_UINT16)Value);
    ISP_WriteRegister(ISP_REG_FRMFUN_1_KEY_B,  (MMP_UINT16)(pFrameFun->ColorKeyB & ISP_BIT_FRMFUN_KEY));

    ISP_WriteRegister(ISP_REG_CONST_ALPHA_1,   (MMP_UINT16)(pFrameFun->ConstantAlpha & ISP_BIT_CONST_ALPHA));

    // format ARGB4444 or Constant Alpha with RGB565
    if (pFrameFun->Format == ARGB4444)
    {
        Value = ((0x1 & ISP_BIT_FRMFUN_ALPHA_BLEND_EN) << ISP_SHT_FRMFUN_ALPHA_BLEND_EN) |
                ((0x1 & ISP_BIT_FRMFUN_MODE)           << ISP_SHT_FRMFUN_MODE);
    }
    else if (pFrameFun->Format == CARGB565)
    {
        if (pFrameFun->ConstantAlpha != 0)
        {
            Value = ((0x1 & ISP_BIT_FRMFUN_ALPHA_BLEND_EN) << ISP_SHT_FRMFUN_ALPHA_BLEND_EN) |
                    ((0x0 & ISP_BIT_FRMFUN_MODE)           << ISP_SHT_FRMFUN_MODE);
        }
        else
        {
            Value = ((0x0 & ISP_BIT_FRMFUN_ALPHA_BLEND_EN) << ISP_SHT_FRMFUN_ALPHA_BLEND_EN) |
                    ((0x0 & ISP_BIT_FRMFUN_MODE)           << ISP_SHT_FRMFUN_MODE);
        }
    }

    //FrameFun Enable, BlendConst
    Value = Value |
            ((pFrameFun->Enable           & ISP_BIT_FRMFUN_EN)                  << ISP_SHT_FRMFUN_EN) |
            ((pFrameFun->EnableUiDec      & ISP_BIT_FRMFUN_UIDEC_EN)            << ISP_SHT_FRMFUN_UIDEC_EN) |
            ((pFrameFun->EnableBlendConst & ISP_BIT_FRMFUN_CONST_DATA_BLEND_EN) << ISP_SHT_FRMFUN_CONST_DATA_BLEND_EN) |
            ((pFrameFun->EnableGridConst  & ISP_BIT_FRMFUN_GRID_CONST_DATA_EN)  << ISP_SHT_FRMFUN_GRID_CONST_DATA_EN) |
            ((pFrameFun->GridDataMode     & ISP_BIT_FRMFUN_GRID_PIXEL_MODE)     << ISP_SHT_FRMFUN_GRID_PIXEL_MODE) |
            ((pFrameFun->EnableRGB2YUV    & ISP_BIT_FRMFUN_RGB2YUV_EN)          << ISP_SHT_FRMFUN_RGB2YUV_EN);

    //Enable frame function, set format
    ISP_WriteRegister(ISP_REG_SET_FRMFUN_1, (MMP_UINT16)Value);

    //Const Data
    Value = ((pFrameFun->ConstColorR0 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_LOW) |
            ((pFrameFun->ConstColorG0 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_HI);
    ISP_WriteRegister(ISP_REG_CONST_DATA_R0G0_1, (MMP_UINT16)Value);

    Value = ((pFrameFun->ConstColorB0 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_LOW) |
            ((pFrameFun->ConstColorR1 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_HI);
    ISP_WriteRegister(ISP_REG_CONST_DATA_B0R1_1, (MMP_UINT16)Value);

    Value = ((pFrameFun->ConstColorG1 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_LOW) |
            ((pFrameFun->ConstColorB1 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_HI);
    ISP_WriteRegister(ISP_REG_CONST_DATA_G1B1_1,        (MMP_UINT16)Value);

    // ui decompress
    ISP_WriteRegister(ISP_REG_FRMFUN_1_DEC_LINEBYTE,    (MMP_UINT16)(pFrameFun->UiDecLineBytes          & ISP_REG_FRMFUN_DEC_LINEBYTE));
    ISP_WriteRegister(ISP_REG_FRMFUN_1_DEC_TOTALBYTE_L, (MMP_UINT16)(pFrameFun->UiDecTotalBytes         & ISP_REG_FRMFUN_DEC_TOTALBYTE_L));
    ISP_WriteRegister(ISP_REG_FRMFUN_1_DEC_TOTALBYTE_H, (MMP_UINT16)((pFrameFun->UiDecTotalBytes >> 16) & ISP_REG_FRMFUN_DEC_TOTALBYTE_H));

    ISP_WriteRegister(ISP_REG_UPDATE_PARA_1,            pFrameFun->UIBufferIndex);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * RGB to YUV transfer matrix.
 */
//=============================================================================
void
ISP_SetRGBtoYUVMatrix_Reg(
    const ISP_RGB_TO_YUV *pRGBtoYUV)
{
    ISP_CMD_QUEUE_WAIT(12 * 2 * 4);

    ISP_WriteRegister(ISP_REG_RGB_TO_YUV_11,      (MMP_UINT16)(pRGBtoYUV->_11    & ISP_BIT_RGB_TO_YUV));
    ISP_WriteRegister(ISP_REG_RGB_TO_YUV_12,      (MMP_UINT16)(pRGBtoYUV->_12    & ISP_BIT_RGB_TO_YUV));
    ISP_WriteRegister(ISP_REG_RGB_TO_YUV_13,      (MMP_UINT16)(pRGBtoYUV->_13    & ISP_BIT_RGB_TO_YUV));
    ISP_WriteRegister(ISP_REG_RGB_TO_YUV_21,      (MMP_UINT16)(pRGBtoYUV->_21    & ISP_BIT_RGB_TO_YUV));
    ISP_WriteRegister(ISP_REG_RGB_TO_YUV_22,      (MMP_UINT16)(pRGBtoYUV->_22    & ISP_BIT_RGB_TO_YUV));
    ISP_WriteRegister(ISP_REG_RGB_TO_YUV_23,      (MMP_UINT16)(pRGBtoYUV->_23    & ISP_BIT_RGB_TO_YUV));
    ISP_WriteRegister(ISP_REG_RGB_TO_YUV_31,      (MMP_UINT16)(pRGBtoYUV->_31    & ISP_BIT_RGB_TO_YUV));
    ISP_WriteRegister(ISP_REG_RGB_TO_YUV_32,      (MMP_UINT16)(pRGBtoYUV->_32    & ISP_BIT_RGB_TO_YUV));
    ISP_WriteRegister(ISP_REG_RGB_TO_YUV_33,      (MMP_UINT16)(pRGBtoYUV->_33    & ISP_BIT_RGB_TO_YUV));
    ISP_WriteRegister(ISP_REG_RGB_TO_YUV_CONST_Y, (MMP_UINT16)(pRGBtoYUV->ConstY & ISP_BIT_RGB_TO_YUV_CONST));
    ISP_WriteRegister(ISP_REG_RGB_TO_YUV_CONST_U, (MMP_UINT16)(pRGBtoYUV->ConstU & ISP_BIT_RGB_TO_YUV_CONST));
    ISP_WriteRegister(ISP_REG_RGB_TO_YUV_CONST_V, (MMP_UINT16)(pRGBtoYUV->ConstV & ISP_BIT_RGB_TO_YUV_CONST));

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Set clip0 function.
 */
//=============================================================================
void
ISP_SetClip0Fun_Reg(
    const ISP_CLIP_FN_CTRL *pClipFun)
{
    MMP_UINT16 Value = 0;

    ISP_CMD_QUEUE_WAIT(5 * 2 * 4);

    Value = ((pClipFun->Enable & ISP_BIT_CLIP_EN)     << ISP_SHT_CLIP_EN) |
            ((pClipFun->Format & ISP_BIT_CLIP_REGION) << ISP_SHT_CLIP_REGION);
    ISP_WriteRegister(ISP_REG_SET_CLIP_0,    (MMP_UINT16)Value);

    ISP_WriteRegister(ISP_REG_CLIP_0_LEFT,   (MMP_UINT16)(pClipFun->ClipLeft   & ISP_REG_CLIP_LEFT));
    ISP_WriteRegister(ISP_REG_CLIP_0_RIGHT,  (MMP_UINT16)(pClipFun->ClipRight  & ISP_REG_CLIP_RIGHT));
    ISP_WriteRegister(ISP_REG_CLIP_0_TOP,    (MMP_UINT16)(pClipFun->ClipTop    & ISP_REG_CLIP_TOP));
    ISP_WriteRegister(ISP_REG_CLIP_0_BOTTOM, (MMP_UINT16)(pClipFun->ClipBottom & ISP_REG_CLIP_BOTTOM));

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Set clip1 function.
 */
//=============================================================================
void
ISP_SetClip1Fun_Reg(
    ISP_DEVICE             ptDev,
    const ISP_CLIP_FN_CTRL *pClipFun)
{
    MMP_UINT16  Value     = 0;
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    ISP_CMD_QUEUE_WAIT(5 * 2 * 4);

    Value = ((pISPctxt->Mpeg2BotBufferIndex & 0x1f)   << 4) |
            ((pClipFun->Enable & ISP_BIT_CLIP_EN)     << ISP_SHT_CLIP_EN) |
            ((pClipFun->Format & ISP_BIT_CLIP_REGION) << ISP_SHT_CLIP_REGION);
    ISP_WriteRegister(ISP_REG_SET_CLIP_1,    (MMP_UINT16)Value);

    ISP_WriteRegister(ISP_REG_CLIP_1_LEFT,   (MMP_UINT16)(pClipFun->ClipLeft   & ISP_REG_CLIP_LEFT));
    ISP_WriteRegister(ISP_REG_CLIP_1_RIGHT,  (MMP_UINT16)(pClipFun->ClipRight  & ISP_REG_CLIP_RIGHT));
    ISP_WriteRegister(ISP_REG_CLIP_1_TOP,    (MMP_UINT16)(pClipFun->ClipTop    & ISP_REG_CLIP_TOP));
    ISP_WriteRegister(ISP_REG_CLIP_1_BOTTOM, (MMP_UINT16)(pClipFun->ClipBottom & ISP_REG_CLIP_BOTTOM));

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Set clip2 function.
 */
//=============================================================================
void
ISP_SetClip2Fun_Reg(
    ISP_DEVICE             ptDev,
    const ISP_CLIP_FN_CTRL *pClipFun)
{
    MMP_UINT16  Value     = 0;
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    ISP_CMD_QUEUE_WAIT(5 * 2 * 4);

    Value = ((pISPctxt->Mpeg2TopBufferIndex & 0x1f) << 4) |
            ((pClipFun->Enable & ISP_BIT_CLIP_EN)     << ISP_SHT_CLIP_EN) |
            ((pClipFun->Format & ISP_BIT_CLIP_REGION) << ISP_SHT_CLIP_REGION);
    ISP_WriteRegister(ISP_REG_SET_CLIP_2,    (MMP_UINT16)Value);

    ISP_WriteRegister(ISP_REG_CLIP_2_LEFT,   (MMP_UINT16)(pClipFun->ClipLeft   & ISP_REG_CLIP_LEFT));
    ISP_WriteRegister(ISP_REG_CLIP_2_RIGHT,  (MMP_UINT16)(pClipFun->ClipRight  & ISP_REG_CLIP_RIGHT));
    ISP_WriteRegister(ISP_REG_CLIP_2_TOP,    (MMP_UINT16)(pClipFun->ClipTop    & ISP_REG_CLIP_TOP));
    ISP_WriteRegister(ISP_REG_CLIP_2_BOTTOM, (MMP_UINT16)(pClipFun->ClipBottom & ISP_REG_CLIP_BOTTOM));

    ISP_CMD_QUEUE_FIRE();
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

    ISP_CMD_QUEUE_WAIT(3 * 2 * 4);

    //Set ISP_REG_OUTPUT_FORMAT

    if (pOutInfo->OutFormat == RGBPacket)
    {
        Value = (pOutInfo->RGBFormat & ISP_BIT_OUT_RGB_FORMAT) << ISP_SHT_OUT_RGB_FORMAT;
    }
    else if (pOutInfo->OutFormat == YUVPacket)
    {
        Value = (pOutInfo->PacketFormat & ISP_BIT_OUT_YUVPACKET_FORMAT) << ISP_SHT_OUT_YUVPACKET_FORMAT;
    }
    else if (pOutInfo->OutFormat == YUVPlane)
    {
        Value = (pOutInfo->PlaneFormat & ISP_BIT_OUT_YUVPLANE_FORMAT) << ISP_SHT_OUT_YUVPLANE_FORMAT;
    }

    Value = Value |
            ((pOutInfo->OutFormat             & ISP_BIT_OUT_FORMAT)          << ISP_SHT_OUT_FORMAT) |
            ((pOutInfo->DitherMode            & ISP_BIT_OUT_DITHER_MODE)     << ISP_SHT_OUT_DITHER_MODE) |
            ((pOutInfo->EnableQueueFire       & ISP_BIT_QUEUE_FIRE_EN)       << ISP_SHT_QUEUE_FIRE_EN) |
            ((0x0                             & ISP_BIT_KEEP_LAST_FIELD_EN)  << ISP_SHT_KEEP_LAST_FIELD_EN) |
            ((pOutInfo->EnableLcdOnFly        & ISP_BIT_LCD_ONFLY_EN)        << ISP_SHT_LCD_ONFLY_EN) |
            ((pOutInfo->EnableOnFlyWriteMem   & ISP_BIT_ONFLY_WRITE_MEM_EN)  << ISP_SHT_ONFLY_WRITE_MEM_EN) |
            ((pOutInfo->EnableDoubleFrameRate & ISP_BIT_DOUBLE_FRAMERATE_EN) << ISP_SHT_DOUBLE_FRAMERATE_EN);
    ISP_WriteRegister(ISP_REG_OUTPUT_FORMAT, (MMP_UINT16)Value);

    //Set ISP_REG_SET502
    Value = ((pOutInfo->RotateType & ISP_BIT_WRITE_ROTATE_DIR) << ISP_SHT_WRITE_ROTATE_DIR);

    Value = Value |
            ((pOutInfo->SWWrFlipNum           & ISP_BIT_SWWRFLIPNUM)              << ISP_SHT_SWWRFLIPNUM) |
            ((pOutInfo->EnableSWFlipMode      & ISP_BIT_SWWRFLIP_EN)              << ISP_SHT_SWWRFLIP_EN) |
            ((pOutInfo->RotateType            & ISP_BIT_WRITE_ROTATE_DIR)         << ISP_SHT_WRITE_ROTATE_DIR) |
            ((pOutInfo->EnableTripleBuf       & ISP_BIT_OUT_TRI_BUFFER_EN)        << ISP_SHT_OUT_TRI_BUFFER_EN) |
            ((pOutInfo->EnableDoubleBuf       & ISP_BIT_OUT_DUAL_BUFFER_EN)       << ISP_SHT_OUT_DUAL_BUFFER_EN) |
            ((pOutInfo->DisableToLCDFlip      & ISP_BIT_DISABLE_TO_LCD_FLIP_EN)   << ISP_SHT_DISABLE_TO_LCD_FLIP_EN) |
            ((pOutInfo->EnableProgFieldMode   & ISP_BIT_PROGFIELD_MODE_EN)        << ISP_SHT_PROGFIELD_MODE_EN) |
            ((pOutInfo->EnableNegative        & ISP_BIT_NEGATIVE_EFFECT_EN)       << ISP_SHT_NEGATIVE_EFFECT_EN) |
            ((pOutInfo->EnableFieldScale      & ISP_BIT_FIELD_SCALE_EN)           << ISP_SHT_FIELD_SCALE_EN) |
            ((pOutInfo->BottomFieldScaleFirst & ISP_BIT_BOTTOM_FIELD_SCALE_FIRST) << ISP_SHT_BOTTOM_FIELD_SCALE_FIRST) |
            ((pOutInfo->EnableCCFun           & ISP_BIT_COLOR_CORRECT_EN)         << ISP_SHT_COLOR_CORRECT_EN) |
            ((pOutInfo->EnableCSFun           & ISP_BIT_COLOR_SPECE_EN)           << ISP_SHT_COLOR_SPECE_EN) |
            ((pOutInfo->DisbleVideoOut        & ISP_BIT_VIDEO_OUT_EN)             << ISP_SHT_VIDEO_OUT_EN);

    ISP_WriteRegister(ISP_REG_SET502, (MMP_UINT16)Value);

    //Set ISP_REG_SET50E
    Value = ((pOutInfo->EnableOutYUV235Range & ISP_BIT_OUTYUV235RANGE_EN) << ISP_SHT_OUTYUV235RANGE_EN) |
            ((0x0                            & ISP_BIT_ISP_INTERRUPT_EN)  << ISP_SHT_ISP_INTERRUPT_EN) |
            ((0x0                            & ISP_BIT_POST_SUBTITLE_EN)  << ISP_SHT_POST_SUBTITLE_EN);
    ISP_WriteRegister(ISP_REG_SET50E, (MMP_UINT16)Value);

    ISP_CMD_QUEUE_FIRE();
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
    ISP_CMD_QUEUE_WAIT(4 * 2 * 4);

    // width, height, pitch
    ISP_WriteRegister(ISP_REG_OUT_WIDTH,      (MMP_UINT16)(pOutInfo->Width     & ISP_BIT_OUT_WIDTH ));
    ISP_WriteRegister(ISP_REG_OUT_HEIGHT,     (MMP_UINT16)(pOutInfo->Height    & ISP_BIT_OUT_HEIGHT));
    ISP_WriteRegister(ISP_REG_OUT_YRGB_PITCH, (MMP_UINT16)(pOutInfo->PitchYRGB & ISP_BIT_OUT_PITCH));
    ISP_WriteRegister(ISP_REG_OUT_UV_PITCH,   (MMP_UINT16)(pOutInfo->PitchUV   & ISP_BIT_OUT_PITCH));

    ISP_CMD_QUEUE_FIRE();
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

    ISP_CMD_QUEUE_WAIT(6 * 2 * 4);

    //buffer address
    Value = (MMP_UINT32)pOutInfo->Addr0 - vramBaseAddr; // byte align
    ISP_WriteRegister(ISP_REG_OUT_ADDR_0L, (MMP_UINT16)(Value         & ISP_BIT_OUT_ADDR_L));
    ISP_WriteRegister(ISP_REG_OUT_ADDR_0H, (MMP_UINT16)((Value >> 16) & ISP_BIT_OUT_ADDR_H));

    Value = (MMP_UINT32)pOutInfo->Addr1 - vramBaseAddr; // byte align
    ISP_WriteRegister(ISP_REG_OUT_ADDR_1L, (MMP_UINT16)(Value         & ISP_BIT_OUT_ADDR_L));
    ISP_WriteRegister(ISP_REG_OUT_ADDR_1H, (MMP_UINT16)((Value >> 16) & ISP_BIT_OUT_ADDR_H));

    Value = (MMP_UINT32)pOutInfo->Addr2 - vramBaseAddr; // byte align
    ISP_WriteRegister(ISP_REG_OUT_ADDR_2L, (MMP_UINT16)(Value         & ISP_BIT_OUT_ADDR_L));
    ISP_WriteRegister(ISP_REG_OUT_ADDR_2H, (MMP_UINT16)((Value >> 16) & ISP_BIT_OUT_ADDR_H));

    ISP_CMD_QUEUE_FIRE();
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
 * Wait ISP change idle!
 */
//=============================================================================
ISP_RESULT
ISP_WaitISPChangeIdle(
    void)
{
    ISP_RESULT result  = ISP_SUCCESS;
    MMP_UINT16 status  = 0;
    MMP_UINT16 timeOut = 0;

    //
    //  Wait ISP change idle!   0x6FC D[3]  0: idle, 1: busy
    //
    isp_ReadHwReg(ISP_REG_ISP_ENGINE_STATUS, (MMP_UINT16 *)&status);

    while ( (status & 0x0008) )
    {
        isp_sleep(1);

        if (++timeOut > 2000)
        {
            //ISP_LogReg();
            isp_msg_ex(ISP_MSG_TYPE_ERR, "ERROR_ISP_CHANGE_NOT_IDLE \n");
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
 * Wait LCD Frame Buffer Idle in Queue Fire Mode
 */
//=============================================================================
ISP_RESULT
ISP_WaitQueueFireIdle(
    void)
{
    ISP_RESULT result    = ISP_SUCCESS;
    MMP_UINT16 statusLCD = 0, statusISP = 0;
    MMP_UINT16 timeOut   = 0;

    isp_ReadHwReg(ITH_LCD_READ_STATUS2_REG,     (MMP_UINT16 *)&statusLCD); // get Lcd flip index
    isp_ReadHwReg(ISP_REG_ISP_CURR_FLIP_STATUS, (MMP_UINT16 *)&statusISP);

    while ( ((statusISP & 0x0100) >> 8) != ((statusLCD & 0x1000) >> 12) )
    {
        isp_sleep(1);
        if (++timeOut > 2000)
        {
            //ISP_LogReg();
            isp_msg_ex(ISP_MSG_TYPE_ERR, "ERROR_ISP_QUEUE_FIRE_NOT_IDLE \n");
            result = ISP_ERR_QUEUE_FIRE_NOT_IDLE;
            goto end;
        }

        isp_ReadHwReg(ITH_LCD_READ_STATUS2_REG,     (MMP_UINT16 *)&statusLCD);
        isp_ReadHwReg(ISP_REG_ISP_CURR_FLIP_STATUS, (MMP_UINT16 *)&statusISP);
    }

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0%x !", __FUNCTION__, result);

    return result;
}

//=============================================================================
/**
 * Wait ISP engine slice buffer empty!  //for JPG module use
 */
//=============================================================================
ISP_RESULT
ISP_WaitRawSliceBufferEmpty(
    void)
{
    ISP_RESULT result  = ISP_SUCCESS;
    MMP_UINT16 status  = 0;
    MMP_UINT16 timeOut = 0;

    //
    //  Wait ISP engine idle!   0x6FC D[9] 1: empty
    //
    isp_ReadHwReg(ISP_REG_ISP_ENGINE_STATUS, (MMP_UINT16 *)&status);
    while (!(status & 0x0200) )
    {
        isp_sleep(1);
        if (++timeOut > 2000)
        {
            //ISP_LogReg();
            isp_msg_ex(ISP_MSG_TYPE_ERR, "ERROR_ISP_SLICEBUF_NOT_EMPTY\n");
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
 * Set LCD OnFly register
 */
//=============================================================================
void
ISP_EnableLCD_OnFly(
    void)
{
    // set Lcd on-fly on
    isp_WriteHwRegMask(ITH_LCD_SET1_REG,        (MMP_UINT16)0x0800, 0x0800); // enable Lcd on-fly flag
    isp_WriteHwRegMask(ITH_LCD_COLORCON_OUT1_1, (MMP_UINT16)0x8000, 0x8000); // Enable Port1 color convert
    isp_WriteHwRegMask(ITH_LCD_COLORCON_OUT2_1, (MMP_UINT16)0x8000, 0x8000); // Enable Port2 color convert
    isp_WriteHwRegMask(ITH_LCD_UPDATE_REG,      (MMP_UINT16)0x8003, 0x8003); // Lcd layer 1 update parameters
}

void
ISP_DisableLCD_OnFly(
    void)
{
    // set Lcd on-fly off
    isp_WriteHwRegMask(ITH_LCD_SET1_REG,        (MMP_UINT16)0x0000, 0x0800); // disable Lcd on-fly flag
    isp_WriteHwRegMask(ITH_LCD_COLORCON_OUT1_1, (MMP_UINT16)0x0000, 0x8000); // Disable Port1 color convert
    isp_WriteHwRegMask(ITH_LCD_COLORCON_OUT2_1, (MMP_UINT16)0x0000, 0x8000); // Disable Port2 color convert
    isp_WriteHwRegMask(ITH_LCD_UPDATE_REG,      (MMP_UINT16)0x8003, 0x8003); // Lcd layer 1 update parameters
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
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
#define REG_ADDR_TRANSFER(ispCore, reg_addr)                              \
        ispCore == MMP_ISP_CORE_0 ? reg_addr : reg_addr + ISP_CORE_1_BASE

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
    const MMP_ISP_CORE ispCore)
{
    MMP_UINT32 reg, end, i;

    reg = REG_ADDR_TRANSFER(ispCore, ISP_REG_BASE);
    end = REG_ADDR_TRANSFER(ispCore, 0xd0300200);

    ispCore == MMP_ISP_CORE_0
            ? printf("====================LogVP0Reg====================\n")
            : printf("====================LogVP1Reg====================\n");

    for (i = reg; i < end; i += 4)
        printf("WRITE(0x%x,0x%08x);\n", i, ithReadRegA(i));
}

//=============================================================================
/**
 * Driver Fire ISP Engine.
 */
//=============================================================================
void
ISP_DriverFire_Reg(
    const ISP_OUTPUT_INFO *pOutInfo,
    const MMP_ISP_CORE    ispCore)
{
    ISP_FireIspQueue();

    ISP_CMD_QUEUE_WAIT(1 * 2 * 4); // 1 register = (addr + value), addr/value = 4 byte

    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_SET500), (MMP_UINT32)(
            ((pOutInfo->RotateType & ISP_BIT_WRITE_ROTATE_DIR) << ISP_SHT_WRITE_ROTATE_DIR) |
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
            ((pOutInfo->DisbleVideoOut        & ISP_BIT_VIDEO_OUT_EN)             << ISP_SHT_VIDEO_OUT_EN) |
            ((0x1 & ISP_BIT_DRIVER_FIRE_EN) << ISP_SHT_DRIVER_FIRE_EN)));

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Driver Update ISP Parameter.
 */
//=============================================================================
void
ISP_UpdateFire_Reg(
    const ISP_OUTPUT_INFO *pOutInfo,
    const MMP_ISP_CORE    ispCore)
{
    ISP_FireIspQueue();

    ISP_CMD_QUEUE_WAIT(1 * 2 * 4);

    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_SET500), (MMP_UINT32)(
            ((pOutInfo->RotateType & ISP_BIT_WRITE_ROTATE_DIR) << ISP_SHT_WRITE_ROTATE_DIR) |
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
            ((pOutInfo->DisbleVideoOut        & ISP_BIT_VIDEO_OUT_EN)             << ISP_SHT_VIDEO_OUT_EN) |
            ((0x1 & ISP_BIT_ISP_UPDATE_PARM_EN) << ISP_SHT_ISP_UPDATE_PARM_EN)));

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Driver Write Raw Slice Buffer ISP Engine.
 */
//=============================================================================
void
ISP_WirteRawSliceFire_Reg(
    const ISP_OUTPUT_INFO *pOutInfo,
    const MMP_ISP_CORE    ispCore)
{
    ISP_CMD_QUEUE_WAIT(1 * 2 * 4);

    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_SET500), (MMP_UINT32)(
            ((pOutInfo->RotateType & ISP_BIT_WRITE_ROTATE_DIR) << ISP_SHT_WRITE_ROTATE_DIR) |
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
            ((pOutInfo->DisbleVideoOut        & ISP_BIT_VIDEO_OUT_EN)             << ISP_SHT_VIDEO_OUT_EN) |
            ((0x1 & ISP_BIT_ISP_WIRTE_SLICE_FIRE_EN) << ISP_SHT_ISP_WIRTE_SLICE_FIRE_EN)));

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Set ISP Engine Mode and Output Format.
 */
//=============================================================================
void
ISP_SetEngineModeOutParameter_Reg(
    const ISP_ENGINE_MODE_CTRL *pEngineMode,
    const ISP_OUTPUT_INFO      *pOutInfo,
    const MMP_ISP_CORE         ispCore)
{
    MMP_UINT32 Value = 0;

    ISP_CMD_QUEUE_WAIT(2 * 2 * 4);

    Value = ((pEngineMode->TotalSliceNum        & ISP_BIT_TOTALSLICENUM)        << ISP_SHT_TOTALSLICENUM) |
            ((pEngineMode->EnableJPEGDECODE     & ISP_BIT_JPEGDECODE_MODE)      << ISP_SHT_JPEGDECODE_MODE) |
            ((pOutInfo->NVFormat                & ISP_BIT_OUT_NV_FORMAT)        << ISP_SHT_OUT_NV_FORMAT) |
            ((pOutInfo->EnableNVMode            & ISP_BIT_OUT_NV_EN)            << ISP_SHT_OUT_NV_EN) |
            ((pEngineMode->EnableCaptureOnFly   & ISP_BIT_CAPTURE_ONFLY_EN)     << ISP_SHT_CAPTURE_ONFLY_EN) |
            ((pEngineMode->EnableReadMemoryMode & ISP_BIT_READ_MEMORY_MODE)     << ISP_SHT_READ_MEMORY_MODE) |
            ((pEngineMode->EnableBlockMode      & ISP_BIT_BLOCK_MODE)           << ISP_SHT_BLOCK_MODE);

    if (pOutInfo->OutFormat == RGBPacket)
    {
        Value |= (pOutInfo->RGBFormat & ISP_BIT_OUT_RGB_FORMAT) << ISP_SHT_OUT_RGB_FORMAT;
    }
    else if (pOutInfo->OutFormat == YUVPacket)
    {
        Value |= (pOutInfo->PacketFormat & ISP_BIT_OUT_YUVPACKET_FORMAT) << ISP_SHT_OUT_YUVPACKET_FORMAT;
    }
    else if (pOutInfo->OutFormat == YUVPlane)
    {
        Value |= (pOutInfo->PlaneFormat & ISP_BIT_OUT_YUVPLANE_FORMAT) << ISP_SHT_OUT_YUVPLANE_FORMAT;
    }

    Value |= ((pOutInfo->OutFormat             & ISP_BIT_OUT_FORMAT)          << ISP_SHT_OUT_FORMAT) |
             ((pOutInfo->DitherMode            & ISP_BIT_OUT_DITHER_MODE)     << ISP_SHT_OUT_DITHER_MODE) |
             ((pOutInfo->EnableQueueFire       & ISP_BIT_QUEUE_FIRE_EN)       << ISP_SHT_QUEUE_FIRE_EN) |
             ((0x0                             & ISP_BIT_KEEP_LAST_FIELD_EN)  << ISP_SHT_KEEP_LAST_FIELD_EN) |
             ((pOutInfo->EnableLcdOnFly        & ISP_BIT_LCD_ONFLY_EN)        << ISP_SHT_LCD_ONFLY_EN) |
             ((pOutInfo->EnableOnFlyWriteMem   & ISP_BIT_ONFLY_WRITE_MEM_EN)  << ISP_SHT_ONFLY_WRITE_MEM_EN) |
             ((pOutInfo->EnableDoubleFrameRate & ISP_BIT_DOUBLE_FRAMERATE_EN) << ISP_SHT_DOUBLE_FRAMERATE_EN);

    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_ENGINEMODE_PARM), Value);

    Value = ((pEngineMode->WriteRawSliceNum  & ISP_BIT_RAWDATA_SLICE_NUM)    << ISP_SHT_RAWDATA_SLICE_NUM) |
            ((pEngineMode->EnableRawDataMode & ISP_BIT_RAWDATA_HANDSHARK_EN) << ISP_SHT_RAWDATA_HANDSHARK_EN);

    Value |= ((pOutInfo->EnableOutYUV235Range & ISP_BIT_OUTYUV235RANGE_EN) << ISP_SHT_OUTYUV235RANGE_EN) |
             ((0x0                            & ISP_BIT_ISP_INTERRUPT_EN)  << ISP_SHT_ISP_INTERRUPT_EN) |
             ((0x0                            & ISP_BIT_POST_SUBTITLE_EN)  << ISP_SHT_POST_SUBTITLE_EN);

    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_RAWDATA_HANDSHARK), Value);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * VP1 API
 */
//=============================================================================
void
ISP_SetPVPreviewMode_Reg(
    ISP_DEVICE ptDev)
{
    MMP_UINT16  Value = 0;
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    ISP_CMD_QUEUE_WAIT(2 * 2 * 4);

    Value |= (pISPctxt->EnPreview & ISP_BIT_PREVIEW_EN) << ISP_SHT_PREVIEW_EN |
             (pISPctxt->PreScaleSel & ISP_BIT_PRESCALE_SEL) << ISP_SHT_PRESCALE_SEL;

    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_PANEL_COLOR_V), (MMP_UINT16)Value << 16, ISP_MASK_LOWBIT);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * VP1 API
 */
//=============================================================================
void
ISP_SetPVOutParameter_Reg(
    const ISP_PV_OUTPUT_INFO *pOutInfo,
    const MMP_ISP_CORE       ispCore)
{
    MMP_UINT16 Value = 0;

    ISP_CMD_QUEUE_WAIT(2 * 2 * 4);

    if (pOutInfo->OutFormat == RGBPacket)
    {
        Value |= 0x1 << ISP_SHT_PVOUT_RGB_EN |
                 (pOutInfo->RGBFormat & ISP_BIT_PVOUT_RGB_FORMAT) << ISP_SHT_PVOUT_RGB_FORMAT |
                 (pOutInfo->DitherMode & ISP_BIT_PVOUT_DITHER_MODE) << ISP_SHT_PVOUT_DITHER_MODE;
    }
    else if (pOutInfo->OutFormat == YUVPacket)
    {
        Value |= 0x1 << ISP_SHT_PVOUT_YUVPACKET_EN |
                 (pOutInfo->PacketFormat & ISP_BIT_PVOUT_YUVPACKET_FORMAT) << ISP_SHT_PVOUT_YUVPACKET_FORMAT;
    }

    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_PVOUTPUT_FORMAT), (MMP_UINT32)Value, ISP_MASK_HIGHBIT);

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
    MMP_UINT32  Value     = 0;
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    ISP_CMD_QUEUE_WAIT(10 * 2 * 4);

    Value  = (MMP_UINT16)(pInInfo->SrcHeight & ISP_BIT_INPUT_HEIGHT);
    Value |= (MMP_UINT16)(pInInfo->SrcWidth  & ISP_BIT_INPUT_WIDTH) << 16;
    ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_INPUT_HEIGHT), Value);

    if (pISPctxt->OutInfo.EnableFieldScale)
    {
        Value = 0;
        if (pISPctxt->OutInfo.EnableProgFieldMode)
            Value = (MMP_UINT16)((pInInfo->PitchY) & ISP_BIT_INPUT_PITCH_Y);
        else
            Value = (MMP_UINT16)((pInInfo->PitchY << 1) & ISP_BIT_INPUT_PITCH_Y);

        if (pISPctxt->OutInfo.EnableProgFieldMode)
            Value |= (MMP_UINT16)((pInInfo->PitchUV) & ISP_BIT_INPUT_PITCH_UV) << 16;
        else if (pISPctxt->DeInterlace.UVRepeatMode)
            Value |= (MMP_UINT16)((pInInfo->PitchUV << 1) & ISP_BIT_INPUT_PITCH_UV) << 16;
        else if (!pISPctxt->DeInterlace.UVRepeatMode)
        {
            if (pInInfo->PlaneFormat == YUV420 || pInInfo->PlaneFormat == YUV422R)
                Value |= (MMP_UINT16)(pInInfo->PitchUV & ISP_BIT_INPUT_PITCH_UV) << 16;
            else
                Value |= (MMP_UINT16)((pInInfo->PitchUV << 1) & ISP_BIT_INPUT_PITCH_UV) << 16;
        }

        ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_INPUT_PITCH_Y), Value);
    }
    else
    {
        Value  = (MMP_UINT16)(pInInfo->PitchY & ISP_BIT_INPUT_PITCH_Y);
        Value |= (MMP_UINT16)(pInInfo->PitchUV & ISP_BIT_INPUT_PITCH_UV) << 16;
        ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_INPUT_PITCH_Y), Value);
    }

    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_SET_VC1),
            (MMP_UINT16)(pInInfo->SrcPosX & ISP_BIT_PANEL_SRCPOS_X) << 16, ISP_MASK_LOWBIT);

    ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_PANEL_SRCPOS_Y),
            (MMP_UINT16)(pInInfo->SrcPosY     & ISP_BIT_PANEL_SRCPOS_Y) |
            (MMP_UINT16)(pInInfo->PanelWidth  & ISP_BIT_PANEL_SRC_WIDTH) << 16);

    // height and color
    Value = ((pInInfo->PanelColorY & ISP_BIT_PANEL_COLOR) << ISP_SHT_PANEL_COLOR_Y) |
            ((pInInfo->PanelColorU & ISP_BIT_PANEL_COLOR) << ISP_SHT_PANEL_COLOR_U);

    ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_PANEL_SRC_HEIGHT),
            (MMP_UINT16)(pInInfo->PanelHeight & ISP_BIT_PANEL_SRC_HEIGHT) |
            (MMP_UINT16)Value << 16);

    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_PANEL_COLOR_V),
            (MMP_UINT32)(pInInfo->PanelColorV & ISP_BIT_PANEL_COLOR), ISP_MASK_HIGHBIT);

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
        // Top Field Y
        if (pISPctxt->OutInfo.EnableKeepLastField)
        {
            Value = (pISPctxt->top_field_first)
                    ? (MMP_UINT32)pInInfo->AddrY - vramBaseAddr  + pInInfo->PitchY
                    : (MMP_UINT32)pInInfo->AddrYp - vramBaseAddr + pInInfo->PitchY;
        }
        else
            Value = (MMP_UINT32)pInInfo->AddrY - vramBaseAddr;

        ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_INPUT_ADDR_YL), Value);

        // Bottom Field Y
        Value = (pISPctxt->top_field_first)
                ? (MMP_UINT32)pInInfo->AddrY  - vramBaseAddr + pInInfo->PitchY
                : (MMP_UINT32)pInInfo->AddrYp - vramBaseAddr + pInInfo->PitchY;

        ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_INPUT_ADDR_YPL), Value);

        // Top Field UV
        if (pISPctxt->OutInfo.EnableKeepLastField)
        {
            Value = (pISPctxt->top_field_first)
                    ? (MMP_UINT32)pInInfo->AddrU  - vramBaseAddr + pInInfo->PitchUV
                    : (MMP_UINT32)pInInfo->AddrUp - vramBaseAddr + pInInfo->PitchUV;
        }
        else
            Value = (MMP_UINT32)pInInfo->AddrU - vramBaseAddr;

        ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_INPUT_ADDR_UL), Value);

        if (pISPctxt->OutInfo.EnableKeepLastField)
        {
            Value = (pISPctxt->top_field_first)
                    ? (MMP_UINT32)pInInfo->AddrV  - vramBaseAddr + pInInfo->PitchUV
                    : (MMP_UINT32)pInInfo->AddrVp - vramBaseAddr + pInInfo->PitchUV;
        }
        else
            Value = (MMP_UINT32)pInInfo->AddrV - vramBaseAddr;

        ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_INPUT_ADDR_VL), Value);

        if (pISPctxt->DeInterlace.UVRepeatMode == MMP_TRUE)
        {
            // Bottom Field UV
            Value = (pISPctxt->top_field_first)
                    ? (MMP_UINT32)pInInfo->AddrU  - vramBaseAddr + pInInfo->PitchUV
                    : (MMP_UINT32)pInInfo->AddrUp - vramBaseAddr + pInInfo->PitchUV;

            ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_INPUT_ADDR_UPL), Value);

            Value = (pISPctxt->top_field_first)
                    ? (MMP_UINT32)pInInfo->AddrV  - vramBaseAddr + pInInfo->PitchUV
                    : (MMP_UINT32)pInInfo->AddrVp - vramBaseAddr + pInInfo->PitchUV;

            ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_INPUT_ADDR_VPL), Value);
        }
        else if (pISPctxt->DeInterlace.UVRepeatMode == MMP_FALSE)
        {
            if (pInInfo->PlaneFormat == YUV420 || pInInfo->PlaneFormat == YUV422R)
            {
                // Bottom Field UV
                Value = (pISPctxt->top_field_first)
                        ? (MMP_UINT32)pInInfo->AddrU - vramBaseAddr
                        : (MMP_UINT32)pInInfo->AddrUp - vramBaseAddr;

                ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_INPUT_ADDR_UPL), Value);

                Value = (pISPctxt->top_field_first)
                        ? (MMP_UINT32)pInInfo->AddrV - vramBaseAddr
                        : (MMP_UINT32)pInInfo->AddrVp - vramBaseAddr;

                ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_INPUT_ADDR_VPL), Value);
            }
            else
            {
                // Bottom Field UV
                Value = (pISPctxt->top_field_first)
                        ? (MMP_UINT32)pInInfo->AddrU  - vramBaseAddr + pInInfo->PitchUV
                        : (MMP_UINT32)pInInfo->AddrUp - vramBaseAddr + pInInfo->PitchUV;

                ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_INPUT_ADDR_UPL), Value);

                Value = (pISPctxt->top_field_first)
                        ? (MMP_UINT32)pInInfo->AddrV  - vramBaseAddr + pInInfo->PitchUV
                        : (MMP_UINT32)pInInfo->AddrVp - vramBaseAddr + pInInfo->PitchUV;

                ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_INPUT_ADDR_VPL), Value);
            }
        }
    }
    else
    {
        // CurFrame
        Value = (MMP_UINT32)pInInfo->AddrY - vramBaseAddr;
        ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_INPUT_ADDR_YL), Value);

        Value = (MMP_UINT32)pInInfo->AddrU - vramBaseAddr;
        ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_INPUT_ADDR_UL), Value);

        Value = (MMP_UINT32)pInInfo->AddrV - vramBaseAddr;
        ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_INPUT_ADDR_VL), Value);

        // PreFrame
        Value = (MMP_UINT32)pInInfo->AddrYp - vramBaseAddr;
        ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_INPUT_ADDR_YPL), Value);

        Value = (MMP_UINT32)pInInfo->AddrUp - vramBaseAddr;
        ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_INPUT_ADDR_UPL), Value);

        Value = (MMP_UINT32)pInInfo->AddrVp - vramBaseAddr;
        ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_INPUT_ADDR_VPL), Value);
    }

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Set Deinterlace Parameter and Input Format.
 */
//=============================================================================
void
ISP_SetDeInterlaceInputParam_Reg(
    ISP_DEVICE                 ptDev,
    const ISP_DEINTERLACE_CTRL *pDeInterlace,
    const ISP_INPUT_INFO       *pInInfo)
{
    MMP_UINT32  Value     = 0;
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

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

    if (pInInfo->EnableYUVPlaneMode)
    {
        Value |= (0x1 & ISP_BIT_IN_YUVPLANE_EN) << ISP_SHT_IN_YUVPLANE_EN;
        Value |= (pInInfo->PlaneFormat & ISP_BIT_IN_PLANE_FORMAT) << ISP_SHT_IN_PLANE_FORMAT;
    }
    else if (pInInfo->EnableYUVPackMode)
    {
        Value |= (0x1 & ISP_BIT_IN_YUVPACKET_EN) << ISP_SHT_IN_YUVPACKET_EN;
        Value |= (pInInfo->PacketFormat & ISP_BIT_IN_PACKET_FORMAT) << ISP_SHT_IN_PACKET_FORMAT;
    }
    else if (pInInfo->EnableRGB565)
    {
        Value |= (0x1 & ISP_BIT_IN_RGB565_EN) << ISP_SHT_IN_RGB565_EN;
    }
    else if (pInInfo->EnableRGB888)
    {
        Value |= (0x1 & ISP_BIT_IN_RGB888_EN) << ISP_SHT_IN_RGB888_EN;
    }
    else if (pInInfo->EnableNVMode)
    {
        Value |= (0x1 & ISP_BIT_IN_NV_EN) << ISP_SHT_IN_NV_EN;
        Value |= (pInInfo->NVFormat & ISP_BIT_IN_NV_FORMAT) << ISP_SHT_IN_NV_FORMAT;
    }

    Value |= ((pInInfo->EnableRdRqDoubleLine & ISP_BIT_IN_RDRQ_DOUBLE_LINE) << ISP_SHT_IN_RDRQ_DOUBLE_LINE) |
             ((pInInfo->EnableInYUV255Range & ISP_BIT_IN_YUV255RANGE_EN) << ISP_SHT_IN_YUV255RANGE_EN);

    ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_SET_DEINTERLACE), Value);

    ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_LOWLEVELEDGE_START_X),
            (MMP_UINT16)pDeInterlace->LowLevelPosX |
            (MMP_UINT16)pDeInterlace->LowLevelPosY << 16);
    ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_LOWLEVELEDGE_WIDTH),
            (MMP_UINT16)pDeInterlace->LowLevelWidth |
            (MMP_UINT16)pDeInterlace->LowLevelHeight << 16);

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
    MMP_UINT32           Value         = 0;
    ISP_CONTEXT          *pISPctxt     = (ISP_CONTEXT *)ptDev;
    ISP_DEINTERLACE_CTRL *pDeInterlace = &pISPctxt->DeInterlace;

    ISP_CMD_QUEUE_WAIT(3 * 2 * 4);

    // Parameter 1
    Value = ((pDeInterlace->MDThreshold_High & ISP_BIT_3D_MDTHRED_HIGH) << ISP_SHT_3D_MDTHRED_HIGH) |
            ((pDeInterlace->MDThreshold_Low  & ISP_BIT_3D_MDTHRED_LOW)  << ISP_SHT_3D_MDTHRED_LOW);

    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_OUT_UV_PITCH), (MMP_UINT16)Value << 16, ISP_MASK_LOWBIT);

    // Parameter 2 and Parameter 3
    Value = ((pDeInterlace->DisableMV_A      & ISP_BIT_DISABLE_MOTIONVALUE_A) << ISP_SHT_DISABLE_MOTIONVALUE_A) |
            ((pDeInterlace->DisableMV_B      & ISP_BIT_DISABLE_MOTIONVALUE_B) << ISP_SHT_DISABLE_MOTIONVALUE_B) |
            ((pDeInterlace->DisableMV_C      & ISP_BIT_DISABLE_MOTIONVALUE_C) << ISP_SHT_DISABLE_MOTIONVALUE_C) |
            ((pDeInterlace->DisableMV_D      & ISP_BIT_DISABLE_MOTIONVALUE_D) << ISP_SHT_DISABLE_MOTIONVALUE_D) |
            ((pDeInterlace->DisableMV_E      & ISP_BIT_DISABLE_MOTIONVALUE_E) << ISP_SHT_DISABLE_MOTIONVALUE_E) |
            ((pDeInterlace->DisableMV_F      & ISP_BIT_DISABLE_MOTIONVALUE_F) << ISP_SHT_DISABLE_MOTIONVALUE_F) |
            ((pDeInterlace->DisableMV_G      & ISP_BIT_DISABLE_MOTIONVALUE_G) << ISP_SHT_DISABLE_MOTIONVALUE_G) |
            ((pDeInterlace->EnLPFWeight      & ISP_BIT_LPF_WEIGHT_EN)         << ISP_SHT_LPF_WEIGHT_EN) |
            ((pDeInterlace->EnLPFBlend       & ISP_BIT_LPF_BLEND_EN)          << ISP_SHT_LPF_BLEND_EN) |
            ((pDeInterlace->MDThreshold_Step & ISP_BIT_3D_MDTHRED_STEP)       << ISP_SHT_3D_MDTHRED_STEP) |
            (MMP_UINT16)((pDeInterlace->EnLPFStaticPixel & ISP_BIT_LPF_STATICPIXEL_EN) << ISP_SHT_LPF_STATICPIXEL_EN) << 16;

    ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_3D_DEINTER_PARM_2), Value);

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

    Value = ((pDeInterlace->PostLPFilterEnable & ISP_BIT_2D_POST_LPF_EN) << ISP_SHT_2D_POST_LPF_EN) |
            ((pDeInterlace->D2EdgeBlendWeight  & ISP_BIT_2D_EDGE_WEIGHT) << ISP_SHT_2D_EDGE_WEIGHT) |
            ((pDeInterlace->D2OrgBlendWeight   & ISP_BIT_2D_ORG_WEIGHT)  << ISP_SHT_2D_ORG_WEIGHT);

    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_2D_DEINTER_PARM_1), (MMP_UINT32)Value, ISP_MASK_HIGHBIT);

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

    ISP_WriteRegister(ISP_REG_SUBTITLE_0_ADDR_L,          (MMP_UINT16)(Value & ISP_BIT_SUBTITLE_ADDR_L) |
                                                          (MMP_UINT16)((Value >> 16) & ISP_BIT_SUBTITLE_ADDR_H) << 16);

    // SrcWidth, SrcHeight
    ISP_WriteRegister(ISP_REG_SUBTITLE_0_SRCWIDTH,        (MMP_UINT16)(pSubTitle->SrcWidth & ISP_BIT_SUBTITLE_SRCWIDTH) |
                                                          (MMP_UINT16)(pSubTitle->SrcHeight & ISP_BIT_SUBTITLE_SRCHEIGHT) << 16);

    // HCI, VCI
    ISP_WriteRegister(ISP_REG_SUBTITLE_0_HCI,             (MMP_UINT16)(HCI & ISP_BIT_SUBTITLE_HCI) |
                                                          (MMP_UINT16)(VCI & ISP_BIT_SUBTITLE_VCI) << 16);

    // DstWidth, DstHeight, Pitch
    ISP_WriteRegister(ISP_REG_SUBTITLE_0_DSTWIDTH,        (MMP_UINT16)(pSubTitle->DstWidth & ISP_BIT_SUBTITLE_DSTWIDTH) |
                                                          (MMP_UINT16)(pSubTitle->DstHeight & ISP_BIT_SUBTITLE_DSTHEIGHT) << 16);

    ISP_WriteRegisterMask(ISP_REG_SUBTITLE_0_PITCH, (MMP_UINT32)(pSubTitle->Pitch & ISP_BIT_SUBTITLE_PITCH), ISP_MASK_HIGHBIT);

    // Start X/Y
    ISP_WriteRegister(ISP_REG_SUBTITLE_0_START_X,         (MMP_UINT16)(pSubTitle->StartX & ISP_BIT_SUBTITLE_START_X) |
                                                          (MMP_UINT16)(pSubTitle->StartY & ISP_BIT_SUBTITLE_START_Y) << 16);

    // ui decompress
    ISP_WriteRegister(ISP_REG_SUBTITLE_0_DEC_LINEBYTE,    (MMP_UINT16)(pSubTitle->UiDecLineBytes & ISP_BIT_SUBTITLE_DEC_LINEBYTE) |
                                                          (MMP_UINT16)(pSubTitle->UiDecTotalBytes & ISP_REG_SUBTITLE_DEC_TOTALBYTE_L) << 16);

    ISP_WriteRegisterMask(ISP_REG_SUBTITLE_0_DEC_TOTALBYTE_H, (MMP_UINT32)((pSubTitle->UiDecTotalBytes >> 16) & ISP_REG_SUBTITLE_DEC_TOTALBYTE_H), ISP_MASK_HIGHBIT);

    // Mode and Enable
    Value = ((pSubTitle->EnableUiDec & ISP_BIT_SUBTITLE_UIDEC_EN) << ISP_SHT_SUBTITLE_UIDEC_EN) |
            ((pSubTitle->Format      & ISP_BIT_SUBTITLE_MODE)     << ISP_SHT_SUBTITLE_MODE) |
            ((pSubTitle->Enable      & ISP_BIT_SUBTITLE_EN)       << ISP_SHT_SUBTITLE_EN);

    ISP_WriteRegisterMask(ISP_REG_COL_COR_DELTA_B, (MMP_UINT16)Value << 16, ISP_MASK_LOWBIT);

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

    // Starting Address
    Value = (pSubTitle->Enable)
            ? (MMP_UINT32)pSubTitle->Addr - vramBaseAddr // byte align
            : 0;                                         // for Onfly
    ISP_WriteRegister(ISP_REG_SUBTITLE_1_ADDR_L,          (MMP_UINT16)(Value & ISP_BIT_SUBTITLE_ADDR_L) |
                                                          (MMP_UINT16)((Value >> 16) & ISP_BIT_SUBTITLE_ADDR_H) << 16);

    // SrcWidth, SrcHeight
    ISP_WriteRegister(ISP_REG_SUBTITLE_1_SRCWIDTH,        (MMP_UINT16)(pSubTitle->SrcWidth & ISP_BIT_SUBTITLE_SRCWIDTH) |
                                                          (MMP_UINT16)(pSubTitle->SrcHeight & ISP_BIT_SUBTITLE_SRCHEIGHT) << 16);

    // HCI, VCI
    ISP_WriteRegister(ISP_REG_SUBTITLE_1_HCI,             (MMP_UINT16)(HCI & ISP_BIT_SUBTITLE_HCI) |
                                                          (MMP_UINT16)(VCI & ISP_BIT_SUBTITLE_VCI) << 16);

    // DstWidth, DstHeight, Pitch
    ISP_WriteRegister(ISP_REG_SUBTITLE_1_DSTWIDTH,        (MMP_UINT16)(pSubTitle->DstWidth & ISP_BIT_SUBTITLE_DSTWIDTH) |
                                                          (MMP_UINT16)(pSubTitle->DstHeight & ISP_BIT_SUBTITLE_DSTHEIGHT) << 16);

    ISP_WriteRegisterMask(ISP_REG_SUBTITLE_1_PITCH, (MMP_UINT32)(pSubTitle->Pitch & ISP_BIT_SUBTITLE_PITCH), ISP_MASK_HIGHBIT);

    // Start X/Y
    ISP_WriteRegister(ISP_REG_SUBTITLE_1_START_X,         (MMP_UINT16)(pSubTitle->StartX & ISP_BIT_SUBTITLE_START_X) |
                                                          (MMP_UINT16)(pSubTitle->StartY & ISP_BIT_SUBTITLE_START_Y) << 16);

    // ui decompress
    ISP_WriteRegisterMask(ISP_REG_SUBTITLE_0_DEC_TOTALBYTE_H, (MMP_UINT16)(pSubTitle->UiDecLineBytes & ISP_BIT_SUBTITLE_DEC_LINEBYTE) << 16, ISP_MASK_LOWBIT);

    ISP_WriteRegister(ISP_REG_SUBTITLE_1_DEC_TOTALBYTE_L, (MMP_UINT16)(pSubTitle->UiDecTotalBytes & ISP_REG_SUBTITLE_DEC_TOTALBYTE_L) |
                                                          (MMP_UINT16)((pSubTitle->UiDecTotalBytes >> 16) & ISP_REG_SUBTITLE_DEC_TOTALBYTE_H) << 16);

    // Mode and Enable
    Value = ((pSubTitle->EnableUiDec & ISP_BIT_SUBTITLE_UIDEC_EN) << ISP_SHT_SUBTITLE_UIDEC_EN) |
            ((pSubTitle->Format      & ISP_BIT_SUBTITLE_MODE)     << ISP_SHT_SUBTITLE_MODE) |
            ((pSubTitle->Enable      & ISP_BIT_SUBTITLE_EN)       << ISP_SHT_SUBTITLE_EN);

    ISP_WriteRegisterMask(ISP_REG_SUBTITLE_0_PITCH, (MMP_UINT16)Value << 16, ISP_MASK_LOWBIT);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * YUV to RGB transfer matrix.
 */
//=============================================================================
void
ISP_SetYUVtoRGBMatrix_Reg(
    const ISP_YUV_TO_RGB *pYUVtoRGB,
    const MMP_ISP_CORE   ispCore)
{
    ISP_CMD_QUEUE_WAIT(12 * 2 * 4);

    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_UV_INTERPOLATION),
            (MMP_UINT16)(pYUVtoRGB->_11 & ISP_BIT_YUV_TO_RGB) << 16, ISP_MASK_LOWBIT);

    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_YUV_TO_RGB_12),
            (MMP_UINT16)(pYUVtoRGB->_12    & ISP_BIT_YUV_TO_RGB) |
            (MMP_UINT16)(pYUVtoRGB->_13    & ISP_BIT_YUV_TO_RGB) << 16);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_YUV_TO_RGB_21),
            (MMP_UINT16)(pYUVtoRGB->_21    & ISP_BIT_YUV_TO_RGB) |
            (MMP_UINT16)(pYUVtoRGB->_22    & ISP_BIT_YUV_TO_RGB) << 16);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_YUV_TO_RGB_23),
            (MMP_UINT16)(pYUVtoRGB->_23    & ISP_BIT_YUV_TO_RGB) |
            (MMP_UINT16)(pYUVtoRGB->_31    & ISP_BIT_YUV_TO_RGB) << 16);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_YUV_TO_RGB_32),
            (MMP_UINT16)(pYUVtoRGB->_32    & ISP_BIT_YUV_TO_RGB) |
            (MMP_UINT16)(pYUVtoRGB->_33    & ISP_BIT_YUV_TO_RGB) << 16);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_YUV_TO_RGB_CONST_R),
            (MMP_UINT16)(pYUVtoRGB->ConstR & ISP_BIT_YUV_TO_RGB_CONST) |
            (MMP_UINT16)(pYUVtoRGB->ConstG & ISP_BIT_YUV_TO_RGB_CONST) << 16);

    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_YUV_TO_RGB_CONST_B),
            (MMP_UINT32)(pYUVtoRGB->ConstB & ISP_BIT_YUV_TO_RGB_CONST), ISP_MASK_HIGHBIT);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Set color correction matrix and constant
 */
//=============================================================================
void
ISP_SetCCMatrix_Reg(
    const ISP_COLOR_CORRECTION *pColorCorrect,
    const MMP_ISP_CORE         ispCore)
{
    ISP_CMD_QUEUE_WAIT(12 * 2 * 4);

    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_YUV_TO_RGB_CONST_B),
            (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_11, 4, 8) & ISP_BIT_COL_COR) << 16, ISP_MASK_LOWBIT);

    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_COL_COR_12),
            (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_12, 4, 8) & ISP_BIT_COL_COR) |
            (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_13, 4, 8) & ISP_BIT_COL_COR) << 16);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_COL_COR_21),
            (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_21, 4, 8) & ISP_BIT_COL_COR) |
            (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_22, 4, 8) & ISP_BIT_COL_COR) << 16);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_COL_COR_23),
            (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_23, 4, 8) & ISP_BIT_COL_COR) |
            (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_31, 4, 8) & ISP_BIT_COL_COR) << 16);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_COL_COR_32),
            (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_32, 4, 8) & ISP_BIT_COL_COR) |
            (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->_33, 4, 8) & ISP_BIT_COL_COR) << 16);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_COL_COR_DELTA_R),
            (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->DeltaR, 8, 0) & ISP_BIT_COL_CORR_DELTA) |
            (MMP_UINT16)(ISP_FloatToFix(pColorCorrect->DeltaG, 8, 0) & ISP_BIT_COL_CORR_DELTA) << 16);

    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_COL_COR_DELTA_B),
            (MMP_UINT32)(ISP_FloatToFix(pColorCorrect->DeltaB, 8, 0) & ISP_BIT_COL_CORR_DELTA), ISP_MASK_HIGHBIT);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/*
 * Set Pre-Scale Register
 */
//=============================================================================
void
ISP_SetPreScaleParam_Reg(
    const ISP_PRESCALE_CTRL *pPreScaleFun,
    const MMP_ISP_CORE      ispCore)
{
    MMP_UINT32 HCI;

    HCI = ISP_FloatToFix(pPreScaleFun->HCI, 6, 14);

    ISP_CMD_QUEUE_WAIT(3 * 2 * 4);

    // HCI
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_PRESCALE_HCI_L),
            (MMP_UINT16)(HCI & ISP_BIT_PRESCALE_HCI_L) |
            (MMP_UINT16)((HCI >> 16) & ISP_BIT_PRESCALE_HCI_H) << 16);

    // PreScale Output Width
    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_PRESCALE_WIDTH),
            (MMP_UINT32)(pPreScaleFun->DstWidth & ISP_BIT_PRESCALE_WIDTH), ISP_MASK_HIGHBIT);

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
    MMP_UINT32 Value = 0, Value2 = 0;

    ISP_CMD_QUEUE_WAIT(10 * 2 * 4);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[0][0], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[0][1], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegisterMask(ISP_REG_PRESCALE_WIDTH, (MMP_UINT16)Value << 16, ISP_MASK_LOWBIT);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[0][2], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[0][3], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    Value2 = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[1][0], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
             (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[1][1], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX0302, (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[1][2], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[1][3], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    Value2 = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[2][0], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
             (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[2][1], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX1312, (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[2][2], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[2][3], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    Value2 = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[3][0], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
             (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[3][1], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX2322, (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[3][2], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[3][3], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    Value2 = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[4][0], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
             (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[4][1], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_PRESCALE_WX3332, (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[4][2], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pPreScaleFun->WeightMatX[4][3], 1, 6) & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegisterMask(ISP_REG_PRESCALE_WX4342, (MMP_UINT32)Value, ISP_MASK_HIGHBIT);

    ISP_CMD_QUEUE_FIRE();
}

void
ISP_SetIntPreScaleMatrix_Reg(
    MMP_UINT8          WeightMatX[][ISP_SCALE_TAP],
    const MMP_ISP_CORE ispCore)
{
    MMP_UINT16 Value, Value2;

    ISP_CMD_QUEUE_WAIT(10 * 2 * 4);

    Value = ((WeightMatX[0][0] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[0][1] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_PRESCALE_WIDTH), (MMP_UINT16)Value << 16, ISP_MASK_LOWBIT);

    Value = ((WeightMatX[0][2] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[0][3] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    Value2 = ((WeightMatX[1][0] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
             ((WeightMatX[1][1] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_PRESCALE_WX0302), (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = ((WeightMatX[1][2] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[1][3] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    Value2 = ((WeightMatX[2][0] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
             ((WeightMatX[2][1] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_PRESCALE_WX1312), (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = ((WeightMatX[2][2] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[2][3] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    Value2 = ((WeightMatX[3][0] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
             ((WeightMatX[3][1] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_PRESCALE_WX2322), (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = ((WeightMatX[3][2] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[3][3] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    Value2 = ((WeightMatX[4][0] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
             ((WeightMatX[4][1] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_PRESCALE_WX3332), (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = ((WeightMatX[4][2] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_L) |
            ((WeightMatX[4][3] & ISP_BIT_PRESCALEWEIGHT) << ISP_SHT_PRESCALEWEIGHT_H);
    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_PRESCALE_WX4342), (MMP_UINT32)Value, ISP_MASK_HIGHBIT);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/*
 * Set Scale Factor
 */
//=============================================================================
void
ISP_SetScaleParam_Reg(
    const ISP_SCALE_CTRL *pScaleFun,
    const MMP_ISP_CORE   ispCore)
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

    // HCIP
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_SCALE_HCIP_L),
            (MMP_UINT16)(HCIP & ISP_BIT_SCLAE_HCIP_L) |
            (MMP_UINT16)((HCIP >> 16) & ISP_BIT_SCLAE_HCIP_H) << 16);

    // VCIP and Output pixel counts per line in a block
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_SCALE_VCIP),
            (MMP_UINT16)ISP_SCALE_MAX_BLOCK_HEIGHT |
            (MMP_UINT16)(OPNPLB & ISP_BIT_SCLAE_OPNPLB) << 16);

    // HCI
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_SCALE_HCI_L),
            (MMP_UINT16)(HCI & ISP_BIT_SCALE_L) |
            (MMP_UINT16)((HCI >> 16) & ISP_BIT_SCALE_H) << 16);

    // VCI
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_SCALE_VCI_L),
            (MMP_UINT16)(VCI & ISP_BIT_SCALE_L) |
            (MMP_UINT16)((VCI >> 16) & ISP_BIT_SCALE_H) << 16);

    // Background Area Color
    Value = ((pScaleFun->BGColorR & ISP_BIT_BGCOLOR) << ISP_SHT_BGCOLOR_R) |
            ((pScaleFun->BGColorG & ISP_BIT_BGCOLOR) << ISP_SHT_BGCOLOR_G);

    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_BGCOLOR_RG),
            (MMP_UINT16)Value |
            (MMP_UINT16)(pScaleFun->BGColorB & ISP_BIT_BGCOLOR) << 16);

    // Scale Output Position X and Y
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_SCALE_DSTPOS_X),
            (MMP_UINT16)(pScaleFun->DstPosX & ISP_BIT_SCALE_DSTPOS_X) |
            (MMP_UINT16)(pScaleFun->DstPosY & ISP_BIT_SCALE_DSTPOS_Y) << 16);

    // Scale Output Width and Height
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_SCALE_DSTWIDTH),
            (MMP_UINT16)(pScaleFun->DstWidth  & ISP_BIT_SCALE_DSTWIDTH) |
            (MMP_UINT16)(pScaleFun->DstHeight & ISP_BIT_SCALE_DSTHEIGHT) << 16);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/*
 * VP1 API
 */
//=============================================================================
void
ISP_SetPVScaleParam_Reg(
    const ISP_SCALE_CTRL *pScaleFun,
    const MMP_ISP_CORE   ispCore)
{
    MMP_UINT16 Value = 0;
    MMP_UINT32 HCI;
    MMP_UINT32 VCI;

    HCI  = ISP_FloatToFix(pScaleFun->HCI, 6, 14);
    VCI  = ISP_FloatToFix(pScaleFun->VCI, 6, 14);

    ISP_CMD_QUEUE_WAIT(14 * 2 * 4);

    // HCI
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_PVSCALE_HCI_L),
            (MMP_UINT16)(HCI & ISP_BIT_PVSCALE_L) |
            (MMP_UINT16)((HCI >> 16) & ISP_BIT_PVSCALE_H) << 16);

    // VCI
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_PVSCALE_VCI_L),
            (MMP_UINT16)(VCI & ISP_BIT_PVSCALE_L) |
            (MMP_UINT16)((VCI >> 16) & ISP_BIT_PVSCALE_H) << 16);

    // Scale Output Width and Height
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_PVSCALE_DSTWIDTH),
            (MMP_UINT16)(pScaleFun->DstWidth  & ISP_BIT_PVSCALE_DSTWIDTH) |
            (MMP_UINT16)(pScaleFun->DstHeight & ISP_BIT_PVSCALE_DSTHEIGHT) << 16);

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
    MMP_UINT16 Value, Value2;

    ISP_CMD_QUEUE_WAIT(10 * 2 * 4);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[0][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[0][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    Value2 = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[0][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
             (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[0][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX0100, (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[1][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[1][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    Value2 = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[1][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
             (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[1][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX1110, (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[2][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[2][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    Value2 = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[2][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
             (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[2][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX2120, (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[3][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[3][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    Value2 = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[3][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
             (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[3][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX3130, (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[4][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[4][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    Value2 = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[4][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
             (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatX[4][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWX4140, (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    ISP_CMD_QUEUE_FIRE();
}

void
ISP_SetIntScaleMatrixH_Reg(
    MMP_UINT8          WeightMatX[][ISP_SCALE_TAP],
    const MMP_ISP_CORE ispCore)
{
    MMP_UINT16 Value, Value2;

    ISP_CMD_QUEUE_WAIT(10 * 2 * 4);

    Value = ((WeightMatX[0][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[0][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    Value2 = ((WeightMatX[0][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
             ((WeightMatX[0][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_SCALEWX0100), (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = ((WeightMatX[1][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[1][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    Value2 = ((WeightMatX[1][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
             ((WeightMatX[1][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_SCALEWX1110), (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = ((WeightMatX[2][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[2][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    Value2 = ((WeightMatX[2][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
             ((WeightMatX[2][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_SCALEWX2120), (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = ((WeightMatX[3][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[3][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    Value2 = ((WeightMatX[3][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
             ((WeightMatX[3][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_SCALEWX3130), (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = ((WeightMatX[4][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[4][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    Value2 = ((WeightMatX[4][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
             ((WeightMatX[4][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_SCALEWX4140), (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

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
    MMP_UINT16 Value, Value2;

    ISP_CMD_QUEUE_WAIT(10 * 2 * 4);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[0][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[0][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    Value2 = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[0][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
             (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[0][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY0100, (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[1][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[1][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    Value2 = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[1][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
             (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[1][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY1110, (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[2][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[2][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    Value2 = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[2][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
             (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[2][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY2120, (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[3][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[3][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    Value2 = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[3][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
             (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[3][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY3130, (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[4][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[4][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    Value2 = (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[4][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
             (((MMP_UINT8)ISP_FloatToFix(pScaleFun->WeightMatY[4][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(ISP_REG_SCALEWY4140, (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    ISP_CMD_QUEUE_FIRE();
}

void
ISP_SetIntScaleMatrixV_Reg(
    MMP_UINT8          WeightMatY[][ISP_SCALE_TAP],
    const MMP_ISP_CORE ispCore)
{
    MMP_UINT16 Value, Value2;

    ISP_CMD_QUEUE_WAIT(10 * 2 * 4);

    Value = ((WeightMatY[0][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[0][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    Value2 = ((WeightMatY[0][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
             ((WeightMatY[0][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_SCALEWY0100), (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = ((WeightMatY[1][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[1][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    Value2 = ((WeightMatY[1][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
             ((WeightMatY[1][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_SCALEWY1110), (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = ((WeightMatY[2][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[2][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    Value2 = ((WeightMatY[2][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
             ((WeightMatY[2][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_SCALEWY2120), (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = ((WeightMatY[3][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[3][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    Value2 = ((WeightMatY[3][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
             ((WeightMatY[3][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_SCALEWY3130), (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = ((WeightMatY[4][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[4][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    Value2 = ((WeightMatY[4][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
             ((WeightMatY[4][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_SCALEWY4140), (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * VP1 API
 */
//=============================================================================
void
ISP_SetIntPVScaleMatrixH_Reg(
    MMP_UINT8 WeightMatX[][ISP_SCALE_TAP],
    const     MMP_ISP_CORE ispCore)
{
    MMP_UINT16 Value, Value2;

    ISP_CMD_QUEUE_WAIT(10 * 2 * 4);

    Value = ((WeightMatX[0][0] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_L) |
            ((WeightMatX[0][1] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_H);
    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_COL_COR_DELTA_B), (MMP_UINT16)Value << 16, ISP_MASK_LOWBIT);

    Value = ((WeightMatX[0][2] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_L) |
            ((WeightMatX[0][3] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_H);
    Value2 = ((WeightMatX[1][0] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_L) |
             ((WeightMatX[1][1] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_H);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_PVSCALEWX0302), (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = ((WeightMatX[1][2] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_L) |
            ((WeightMatX[1][3] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_H);
    Value2 = ((WeightMatX[2][0] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_L) |
             ((WeightMatX[2][1] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_H);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_PVSCALEWX1312), (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = ((WeightMatX[2][2] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_L) |
            ((WeightMatX[2][3] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_H);
    Value2 = ((WeightMatX[3][0] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_L) |
             ((WeightMatX[3][1] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_H);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_PVSCALEWX2322), (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = ((WeightMatX[3][2] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_L) |
            ((WeightMatX[3][3] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_H);
    Value2 = ((WeightMatX[4][0] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_L) |
             ((WeightMatX[4][1] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_H);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_PVSCALEWX3332), (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = ((WeightMatX[4][2] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_L) |
            ((WeightMatX[4][3] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_H);
    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_PVSCALEWX4342), (MMP_UINT32)Value, ISP_MASK_HIGHBIT);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * VP1 API
 */
//=============================================================================
void
ISP_SetIntPVScaleMatrixV_Reg(
    MMP_UINT8 WeightMatY[][ISP_SCALE_TAP],
    const     MMP_ISP_CORE ispCore)
{
    MMP_UINT16 Value, Value2;

    ISP_CMD_QUEUE_WAIT(10 * 2 * 4);

    Value = ((WeightMatY[0][0] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_L) |
            ((WeightMatY[0][1] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_H);
    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_PVSCALEWX4342), (MMP_UINT16)Value << 16, ISP_MASK_LOWBIT);

    Value = ((WeightMatY[0][2] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_L) |
            ((WeightMatY[0][3] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_H);
    Value2 = ((WeightMatY[1][0] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_L) |
             ((WeightMatY[1][1] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_H);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_PVSCALEWY0302), (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = ((WeightMatY[1][2] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_L) |
            ((WeightMatY[1][3] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_H);
    Value2 = ((WeightMatY[2][0] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_L) |
             ((WeightMatY[2][1] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_H);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_PVSCALEWY1312), (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = ((WeightMatY[2][2] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_L) |
            ((WeightMatY[2][3] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_H);
    Value2 = ((WeightMatY[3][0] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_L) |
             ((WeightMatY[3][1] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_H);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_PVSCALEWY2322), (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = ((WeightMatY[3][2] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_L) |
            ((WeightMatY[3][3] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_H);
    Value2 = ((WeightMatY[4][0] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_L) |
             ((WeightMatY[4][1] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_H);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_PVSCALEWY3332), (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = ((WeightMatY[4][2] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_L) |
            ((WeightMatY[4][3] & ISP_BIT_PVSCALEWEIGHT) << ISP_SHT_PVSCALEWEIGHT_H);
    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_PVSCALEWY4342), (MMP_UINT32)Value, ISP_MASK_HIGHBIT);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Frmfun RGB to YUV transfer matrix.
 */
//=============================================================================
void
ISP_SetFrmMatrix_Reg(
    const ISP_RGB_TO_YUV *pMatrix,
    const MMP_ISP_CORE   ispCore)
{
    ISP_CMD_QUEUE_WAIT(12 * 2 * 4);

    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_FRM_RGB2YUV_11),
            (MMP_UINT16)(pMatrix->_11    & ISP_BIT_FRM_RGB2YUV) |
            (MMP_UINT16)(pMatrix->_12    & ISP_BIT_FRM_RGB2YUV) << 16);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_FRM_RGB2YUV_13),
            (MMP_UINT16)(pMatrix->_13    & ISP_BIT_FRM_RGB2YUV) |
            (MMP_UINT16)(pMatrix->_21    & ISP_BIT_FRM_RGB2YUV) << 16);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_FRM_RGB2YUV_22),
            (MMP_UINT16)(pMatrix->_22    & ISP_BIT_FRM_RGB2YUV) |
            (MMP_UINT16)(pMatrix->_23    & ISP_BIT_FRM_RGB2YUV) << 16);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_FRM_RGB2YUV_31),
            (MMP_UINT16)(pMatrix->_31    & ISP_BIT_FRM_RGB2YUV) |
            (MMP_UINT16)(pMatrix->_32    & ISP_BIT_FRM_RGB2YUV) << 16);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_FRM_RGB2YUV_33),
            (MMP_UINT16)(pMatrix->_33    & ISP_BIT_FRM_RGB2YUV) |
            (MMP_UINT16)(pMatrix->ConstY & ISP_BIT_FRM_RGB2YUV_CONST) << 16);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_FRM_RGB2YUV_CONST_U),
            (MMP_UINT16)(pMatrix->ConstU & ISP_BIT_FRM_RGB2YUV_CONST) |
            (MMP_UINT16)(pMatrix->ConstV & ISP_BIT_FRM_RGB2YUV_CONST) << 16);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Set Frame Function 0
 */
//=============================================================================
void
ISP_SetFrameFun0_Reg(
    const ISP_FRMFUN_CTRL *pFrameFun,
    const MMP_ISP_CORE    ispCore)
{
    MMP_UINT32 Value        = 0;
    MMP_UINT32 Value2       = 0;
    MMP_UINT32 vramBaseAddr = (MMP_UINT32)isp_GetVramBaseAddr();

    ISP_CMD_QUEUE_WAIT(18 * 2 * 4);

    // Starting address
    Value = (pFrameFun->Enable)
            ? (MMP_UINT32)pFrameFun->Addr - vramBaseAddr  // byte align
            : 0;                                          // for Onfly
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_FRMFUN_0_ADDR_L),
            (MMP_UINT16)(Value & ISP_BIT_FRMFUN_ADDR_L) |
            (MMP_UINT16)((Value >> 16) & ISP_BIT_FRMFUN_ADDR_H) << 16);

    // width, height, pitch
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_FRMFUN_0_WIDTH),
            (MMP_UINT16)(pFrameFun->Width  & ISP_BIT_FRMFUN_WIDTH) |
            (MMP_UINT16)(pFrameFun->Height & ISP_BIT_FRMFUN_HEIGHT) << 16);

    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_FRMFUN_0_PITCH),
            (MMP_UINT32)(pFrameFun->Pitch & ISP_BIT_FRMFUN_PITCH), ISP_MASK_HIGHBIT);

    // start X/Y
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_FRMFUN_0_START_X),
            (MMP_UINT16)(pFrameFun->StartX & ISP_BIT_FRMFUN_START_X) |
            (MMP_UINT16)(pFrameFun->StartY & ISP_BIT_FRMFUN_START_Y) << 16);

    // color key
    Value = ((pFrameFun->ColorKeyR & ISP_BIT_FRMFUN_KEY) << ISP_SHT_FRMFUN_KEY_R) |
            ((pFrameFun->ColorKeyG & ISP_BIT_FRMFUN_KEY) << ISP_SHT_FRMFUN_KEY_G);

    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_FRMFUN_0_KEY_RG),
            (MMP_UINT16)Value |
            (MMP_UINT16)(pFrameFun->ColorKeyB & ISP_BIT_FRMFUN_KEY) << 16);

    // format ARGB or Constant Alpha with RGB565
    Value = ((pFrameFun->EnableAlphaBlend & ISP_BIT_FRMFUN_ALPHA_BLEND_EN) << ISP_SHT_FRMFUN_ALPHA_BLEND_EN) |
            ((pFrameFun->Format           & ISP_BIT_FRMFUN_MODE)           << ISP_SHT_FRMFUN_MODE);

    // FrameFun Enable, BlendConst
    Value |= ((pFrameFun->Enable           & ISP_BIT_FRMFUN_EN)                  << ISP_SHT_FRMFUN_EN) |
             ((pFrameFun->EnableUiDec      & ISP_BIT_FRMFUN_UIDEC_EN)            << ISP_SHT_FRMFUN_UIDEC_EN) |
             ((pFrameFun->EnableBlendConst & ISP_BIT_FRMFUN_CONST_DATA_BLEND_EN) << ISP_SHT_FRMFUN_CONST_DATA_BLEND_EN) |
             ((pFrameFun->EnableGridConst  & ISP_BIT_FRMFUN_GRID_CONST_DATA_EN)  << ISP_SHT_FRMFUN_GRID_CONST_DATA_EN) |
             ((pFrameFun->GridDataMode     & ISP_BIT_FRMFUN_GRID_PIXEL_MODE)     << ISP_SHT_FRMFUN_GRID_PIXEL_MODE) |
             ((pFrameFun->EnableRGB2YUV    & ISP_BIT_FRMFUN_RGB2YUV_EN)          << ISP_SHT_FRMFUN_RGB2YUV_EN);

    // Enable frame function, set format, and Constant Alpha Value
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_SET_FRMFUN_0),
            (MMP_UINT16)Value |
            (MMP_UINT16)(pFrameFun->ConstantAlpha & ISP_BIT_CONST_ALPHA) << 16);

    // Const Data
    Value = ((pFrameFun->ConstColorR0 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_LOW) |
            ((pFrameFun->ConstColorG0 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_HI);
    Value2 = ((pFrameFun->ConstColorB0 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_LOW) |
             ((pFrameFun->ConstColorR1 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_HI);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_CONST_DATA_R0G0_0),
            (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    Value = ((pFrameFun->ConstColorG1 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_LOW) |
            ((pFrameFun->ConstColorB1 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_HI);
    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_CONST_DATA_G1B1_0), (MMP_UINT32)Value, ISP_MASK_HIGHBIT); //14

    // ui decompress
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_FRMFUN_0_DEC_LINEBYTE),
            (MMP_UINT16)(pFrameFun->UiDecLineBytes  & ISP_REG_FRMFUN_DEC_LINEBYTE) |
            (MMP_UINT16)(pFrameFun->UiDecTotalBytes & ISP_REG_FRMFUN_DEC_TOTALBYTE_L) << 16);

    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_FRMFUN_0_DEC_TOTALBYTE_H),
            (MMP_UINT32)((pFrameFun->UiDecTotalBytes >> 16) & ISP_REG_FRMFUN_DEC_TOTALBYTE_H), ISP_MASK_HIGHBIT);

    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_UPDATE_PARA_1), (MMP_UINT32)pFrameFun->UIBufferIndex, ISP_MASK_HIGHBIT);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Set Frame Function 1
 */
//=============================================================================
void
ISP_SetFrameFun1_Reg(
    const ISP_FRMFUN_CTRL *pFrameFun,
    const MMP_ISP_CORE    ispCore)
{
    MMP_UINT32 Value        = 0;
    MMP_UINT32 Value2       = 0;
    MMP_UINT32 vramBaseAddr = (MMP_UINT32)isp_GetVramBaseAddr();

    ISP_CMD_QUEUE_WAIT(18 * 2 * 4);

    // starting address and start X
    Value = (pFrameFun->Enable)
            ? (MMP_UINT32)pFrameFun->Addr - vramBaseAddr // byte align
            : 0;                                         // for Onfly
    Value2 = (MMP_UINT16)(Value & ISP_BIT_FRMFUN_ADDR_L);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_FRMFUN_1_ADDR_H),
            (MMP_UINT16)((Value >> 16) & ISP_BIT_FRMFUN_ADDR_H) |
            (MMP_UINT16)(pFrameFun->StartX & ISP_BIT_FRMFUN_START_X) << 16);

    // height, pitch
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_FRMFUN_1_HEIGHT),
            (MMP_UINT16)(pFrameFun->Height & ISP_BIT_FRMFUN_HEIGHT) |
            (MMP_UINT16)(pFrameFun->Pitch  & ISP_BIT_FRMFUN_PITCH) << 16);

    // start Y and width
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_FRMFUN_1_START_Y),
            (MMP_UINT16)(pFrameFun->StartY & ISP_BIT_FRMFUN_START_Y) |
            (MMP_UINT16)(pFrameFun->Width  & ISP_BIT_FRMFUN_WIDTH) << 16);

    // color key and starting address
    Value = ((pFrameFun->ColorKeyR & ISP_BIT_FRMFUN_KEY) << ISP_SHT_FRMFUN_KEY_R) |
            ((pFrameFun->ColorKeyG & ISP_BIT_FRMFUN_KEY) << ISP_SHT_FRMFUN_KEY_G);

    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_FRMFUN_1_KEY_B),
            (MMP_UINT16)(pFrameFun->ColorKeyB & ISP_BIT_FRMFUN_KEY) |
            Value2 << 16);

    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_CONST_ALPHA_1),
            (MMP_UINT16)(pFrameFun->ConstantAlpha & ISP_BIT_CONST_ALPHA) |
            (MMP_UINT16)Value << 16);

    // format ARGB or Constant Alpha with RGB565
    Value = ((pFrameFun->EnableAlphaBlend & ISP_BIT_FRMFUN_ALPHA_BLEND_EN) << ISP_SHT_FRMFUN_ALPHA_BLEND_EN) |
            ((pFrameFun->Format           & ISP_BIT_FRMFUN_MODE)           << ISP_SHT_FRMFUN_MODE);

    // FrameFun Enable, BlendConst
    Value |= ((pFrameFun->Enable           & ISP_BIT_FRMFUN_EN)                  << ISP_SHT_FRMFUN_EN) |
             ((pFrameFun->EnableUiDec      & ISP_BIT_FRMFUN_UIDEC_EN)            << ISP_SHT_FRMFUN_UIDEC_EN) |
             ((pFrameFun->EnableBlendConst & ISP_BIT_FRMFUN_CONST_DATA_BLEND_EN) << ISP_SHT_FRMFUN_CONST_DATA_BLEND_EN) |
             ((pFrameFun->EnableGridConst  & ISP_BIT_FRMFUN_GRID_CONST_DATA_EN)  << ISP_SHT_FRMFUN_GRID_CONST_DATA_EN) |
             ((pFrameFun->GridDataMode     & ISP_BIT_FRMFUN_GRID_PIXEL_MODE)     << ISP_SHT_FRMFUN_GRID_PIXEL_MODE) |
             ((pFrameFun->EnableRGB2YUV    & ISP_BIT_FRMFUN_RGB2YUV_EN)          << ISP_SHT_FRMFUN_RGB2YUV_EN);

    // Enable frame function, set format
    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_FRMFUN_0_PITCH), (MMP_UINT16)Value << 16, ISP_MASK_LOWBIT);

    // Const Data
    Value = ((pFrameFun->ConstColorR0 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_LOW) |
            ((pFrameFun->ConstColorG0 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_HI);

    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_CONST_DATA_G1B1_0), (MMP_UINT16)Value << 16, ISP_MASK_LOWBIT);

    Value = ((pFrameFun->ConstColorB0 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_LOW) |
            ((pFrameFun->ConstColorR1 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_HI);
    Value2 = ((pFrameFun->ConstColorG1 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_LOW) |
             ((pFrameFun->ConstColorB1 & ISP_BIT_CONST_DATA) << ISP_SHT_CONST_DATA_HI);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_CONST_DATA_B0R1_1), (MMP_UINT16)Value | (MMP_UINT16)Value2 << 16);

    // ui decompress
    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_FRMFUN_0_DEC_TOTALBYTE_H),
            (MMP_UINT16)(pFrameFun->UiDecLineBytes & ISP_REG_FRMFUN_DEC_LINEBYTE) << 16, ISP_MASK_LOWBIT);

    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_FRMFUN_1_DEC_TOTALBYTE_L),
            (MMP_UINT16)(pFrameFun->UiDecTotalBytes         & ISP_REG_FRMFUN_DEC_TOTALBYTE_L) |
            (MMP_UINT16)((pFrameFun->UiDecTotalBytes >> 16) & ISP_REG_FRMFUN_DEC_TOTALBYTE_H) << 16);

    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_UPDATE_PARA_1), (MMP_UINT32)pFrameFun->UIBufferIndex, ISP_MASK_HIGHBIT);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * RGB to YUV transfer matrix.
 */
//=============================================================================
void
ISP_SetRGBtoYUVMatrix_Reg(
    const ISP_RGB_TO_YUV *pRGBtoYUV,
    const MMP_ISP_CORE   ispCore)
{
    ISP_CMD_QUEUE_WAIT(12 * 2 * 4);

    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_RGB_TO_YUV_11),
            (MMP_UINT16)(pRGBtoYUV->_11 & ISP_BIT_RGB_TO_YUV) |
            (MMP_UINT16)(pRGBtoYUV->_12 & ISP_BIT_RGB_TO_YUV) << 16);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_RGB_TO_YUV_13),
            (MMP_UINT16)(pRGBtoYUV->_13 & ISP_BIT_RGB_TO_YUV) |
            (MMP_UINT16)(pRGBtoYUV->_21 & ISP_BIT_RGB_TO_YUV) << 16);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_RGB_TO_YUV_22),
            (MMP_UINT16)(pRGBtoYUV->_22 & ISP_BIT_RGB_TO_YUV) |
            (MMP_UINT16)(pRGBtoYUV->_23 & ISP_BIT_RGB_TO_YUV) << 16);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_RGB_TO_YUV_31),
            (MMP_UINT16)(pRGBtoYUV->_31 & ISP_BIT_RGB_TO_YUV) |
            (MMP_UINT16)(pRGBtoYUV->_32 & ISP_BIT_RGB_TO_YUV) << 16);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_RGB_TO_YUV_33),
            (MMP_UINT16)(pRGBtoYUV->_33 & ISP_BIT_RGB_TO_YUV) |
            (MMP_UINT16)(pRGBtoYUV->ConstY & ISP_BIT_RGB_TO_YUV_CONST) << 16);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_RGB_TO_YUV_CONST_U),
            (MMP_UINT16)(pRGBtoYUV->ConstU & ISP_BIT_RGB_TO_YUV_CONST) |
            (MMP_UINT16)(pRGBtoYUV->ConstV & ISP_BIT_RGB_TO_YUV_CONST) << 16);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Set clip0 function.
 */
//=============================================================================
void
ISP_SetClip0Fun_Reg(
    const ISP_CLIP_FN_CTRL *pClipFun,
    const MMP_ISP_CORE     ispCore)
{
    MMP_UINT16 Value = 0;

    ISP_CMD_QUEUE_WAIT(5 * 2 * 4);

    Value = ((pClipFun->Enable & ISP_BIT_CLIP_EN)     << ISP_SHT_CLIP_EN) |
            ((pClipFun->Format & ISP_BIT_CLIP_REGION) << ISP_SHT_CLIP_REGION);
    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_UPDATE_PARA_3), (MMP_UINT16)Value << 16, ISP_MASK_LOWBIT);

    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_CLIP_0_LEFT),
            (MMP_UINT16)(pClipFun->ClipLeft & ISP_REG_CLIP_LEFT) |
            (MMP_UINT16)(pClipFun->ClipRight & ISP_REG_CLIP_RIGHT) << 16);
    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_CLIP_0_TOP),
            (MMP_UINT16)(pClipFun->ClipTop & ISP_REG_CLIP_TOP) |
            (MMP_UINT16)(pClipFun->ClipBottom & ISP_REG_CLIP_BOTTOM) << 16);

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
    ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_SET_CLIP_1),
            (MMP_UINT16)Value |
            (MMP_UINT16)(pClipFun->ClipLeft & ISP_REG_CLIP_LEFT) << 16);

    ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_CLIP_1_RIGHT),
            (MMP_UINT16)(pClipFun->ClipRight & ISP_REG_CLIP_RIGHT) |
            (MMP_UINT16)(pClipFun->ClipTop & ISP_REG_CLIP_TOP) << 16);

    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_CLIP_1_BOTTOM),
            (MMP_UINT32)(pClipFun->ClipBottom & ISP_REG_CLIP_BOTTOM), ISP_MASK_HIGHBIT);

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

    Value = ((pISPctxt->Mpeg2TopBufferIndex & 0x1f)   << 4) |
            ((pClipFun->Enable & ISP_BIT_CLIP_EN)     << ISP_SHT_CLIP_EN) |
            ((pClipFun->Format & ISP_BIT_CLIP_REGION) << ISP_SHT_CLIP_REGION);
    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_CLIP_1_BOTTOM), (MMP_UINT16)Value << 16, ISP_MASK_LOWBIT);

    ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_CLIP_2_LEFT),
            (MMP_UINT16)(pClipFun->ClipLeft & ISP_REG_CLIP_LEFT) |
            (MMP_UINT16)(pClipFun->ClipRight & ISP_REG_CLIP_RIGHT) << 16);
    ISP_WriteRegister(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_CLIP_2_TOP),
            (MMP_UINT16)(pClipFun->ClipTop & ISP_REG_CLIP_TOP) |
            (MMP_UINT16)(pClipFun->ClipBottom & ISP_REG_CLIP_BOTTOM) << 16);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Set Output Information
 */
//=============================================================================
void
ISP_SetOutAddressBufInfo_Reg(
    const ISP_OUTPUT_INFO *pOutInfo,
    const MMP_ISP_CORE    ispCore)
{
    MMP_UINT32 Value        = 0;
    MMP_UINT32 Value2       = 0;
    MMP_UINT32 vramBaseAddr = (MMP_UINT32)isp_GetVramBaseAddr();

    ISP_CMD_QUEUE_WAIT(6 * 2 * 4);

    // buffer address
    Value = (MMP_UINT32)pOutInfo->Addr0 - vramBaseAddr; // byte align

    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_SUBTITLE_1_PITCH),
            (MMP_UINT16)(Value & ISP_BIT_OUT_ADDR_L) << 16, ISP_MASK_LOWBIT);

    Value2  = (MMP_UINT32)((Value >> 16) & ISP_BIT_OUT_ADDR_H);
    Value   = (MMP_UINT32)pOutInfo->Addr1 - vramBaseAddr; // byte align
    Value2 |= (MMP_UINT16)(Value & ISP_BIT_OUT_ADDR_L) << 16;

    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_OUT_ADDR_0H), Value2);

    Value2  = (MMP_UINT32)((Value >> 16) & ISP_BIT_OUT_ADDR_H);
    Value   = (MMP_UINT32)pOutInfo->Addr2 - vramBaseAddr; // byte align
    Value2 |= (MMP_UINT16)(Value & ISP_BIT_OUT_ADDR_L) << 16;

    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_OUT_ADDR_1H), Value2);

    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_OUT_ADDR_2H),
            (MMP_UINT16)((Value >> 16) & ISP_BIT_OUT_ADDR_H) |
            (MMP_UINT16)(pOutInfo->Width & ISP_BIT_OUT_WIDTH) << 16);

    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_OUT_HEIGHT),
            (MMP_UINT16)(pOutInfo->Height & ISP_BIT_OUT_HEIGHT) |
            (MMP_UINT16)(pOutInfo->PitchYRGB & ISP_BIT_OUT_PITCH) << 16);

    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_OUT_UV_PITCH),
            (MMP_UINT32)(pOutInfo->PitchUV & ISP_BIT_OUT_PITCH), ISP_MASK_HIGHBIT);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * VP 1
 */
//=============================================================================
void
ISP_SetPVOutAddressBufInfo_Reg(
    const ISP_PV_OUTPUT_INFO *pOutInfo,
    const MMP_ISP_CORE       ispCore)
{
    MMP_UINT32 Value        = 0;
    MMP_UINT32 vramBaseAddr = (MMP_UINT32)isp_GetVramBaseAddr();

    ISP_CMD_QUEUE_WAIT(6 * 2 * 4);

    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_PVSCALEWY4342),
            (MMP_UINT16)(pOutInfo->Width & ISP_BIT_PVOUT_WIDTH) << 16, ISP_MASK_LOWBIT);

    Value = (MMP_UINT32)pOutInfo->Addr0 - vramBaseAddr;

    ISP_WriteRegister(REG_ADDR_TRANSFER(ispCore, ISP_REG_PVOUT_HEIGHT),
            (MMP_UINT16)(pOutInfo->Height & ISP_BIT_PVOUT_HEIGHT) |
            (MMP_UINT16)(Value & ISP_BIT_PVOUT_ADDR_L) << 16);

    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_PVOUT_ADDR_0H),
            (MMP_UINT32)((Value >> 16) & ISP_BIT_PVOUT_ADDR_H), ISP_MASK_HIGHBIT);

    ISP_WriteRegisterMask(REG_ADDR_TRANSFER(ispCore, ISP_REG_2D_DEINTER_PARM_1),
            (MMP_UINT16)(pOutInfo->PitchYRGB & ISP_BIT_PVOUT_PITCH) << 16, ISP_MASK_LOWBIT);

    ISP_CMD_QUEUE_FIRE();
}

//=============================================================================
/**
 * Wait ISP engine idle!  //for JPG module use
 */
//=============================================================================
ISP_RESULT
ISP_WaitEngineIdle(
    ISP_DEVICE ptDev)
{
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;
    ISP_RESULT result     = ISP_SUCCESS;
    MMP_UINT32 status     = 0;
    MMP_UINT16 timeOut    = 0;
    MMP_BOOL   busy       = MMP_TRUE;

    //
    //  Wait ISP engine idle!   0x6FC VP0 D[5] , D[10]  0: idle, 1: busy
    //                                VP1 D[21], D[26]  0: idle, 1: busy
    //
    isp_ReadHwReg(ISP_REG_ISP_ENGINE_STATUS, (MMP_UINT32 *)&status);
    //isp_ReadHwReg(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_ISP_ENGINE_STATUS), (MMP_UINT32 *)&status);

    if (pISPctxt->ispCore == MMP_ISP_CORE_0)
        busy = (status >> 5) & 1 | (status >> 10) & 1;
    else if (pISPctxt->ispCore == MMP_ISP_CORE_1)
        busy = (status >> 21) & 1 | (status >> 26) & 1;

    while (busy)
    {
        isp_sleep(1);
        if (++timeOut > 2000)
        {
            //ISP_LogReg();
            isp_msg_ex(ISP_MSG_TYPE_ERR, "ERROR_ISP_NOT_IDLE \n");
            result = ISP_ERR_NOT_IDLE;
            goto end;
        }
        isp_ReadHwReg(ISP_REG_ISP_ENGINE_STATUS, (MMP_UINT32 *)&status);
        //isp_ReadHwReg(REG_ADDR_TRANSFER(pISPctxt->ispCore, ISP_REG_ISP_ENGINE_STATUS), (MMP_UINT32 *)&status);

        if (pISPctxt->ispCore == MMP_ISP_CORE_0)
            busy = (status >> 5) & 1 | (status >> 10) & 1;
        else if (pISPctxt->ispCore == MMP_ISP_CORE_1)
            busy = (status >> 21) & 1 | (status >> 26) & 1;
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
    MMP_UINT32 status  = 0;
    MMP_UINT16 timeOut = 0;

    //
    //  Wait ISP change idle!   0x6FC D[3]  0: idle, 1: busy
    //
    isp_ReadHwReg(ISP_REG_ISP_ENGINE_STATUS, (MMP_UINT32 *)&status);

    while (status & 0x00000008)
    {
        isp_sleep(1);

        if (++timeOut > 2000)
        {
            //ISP_LogReg();
            isp_msg_ex(ISP_MSG_TYPE_ERR, "ERROR_ISP_CHANGE_NOT_IDLE \n");
            result = ISP_ERR_NOT_IDLE;
            goto end;
        }
        isp_ReadHwReg(ISP_REG_ISP_ENGINE_STATUS, (MMP_UINT32 *)&status);
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
    MMP_UINT32 statusLCD = 0, statusISP = 0;
    MMP_UINT16 timeOut   = 0;

    isp_ReadHwReg(ITH_LCD_READ_STATUS2_REG,     (MMP_UINT32 *)&statusLCD); // get Lcd flip index
    isp_ReadHwReg(ISP_REG_ISP_CURR_FLIP_STATUS, (MMP_UINT32 *)&statusISP);

    while (((statusISP & 0x00000100) >> 8) != ((statusLCD & 0x00001000) >> 12))
    {
        isp_sleep(1);
        if (++timeOut > 2000)
        {
            //ISP_LogReg();
            isp_msg_ex(ISP_MSG_TYPE_ERR, "ERROR_ISP_QUEUE_FIRE_NOT_IDLE \n");
            result = ISP_ERR_QUEUE_FIRE_NOT_IDLE;
            goto end;
        }

        isp_ReadHwReg(ITH_LCD_READ_STATUS2_REG,     (MMP_UINT32 *)&statusLCD);
        isp_ReadHwReg(ISP_REG_ISP_CURR_FLIP_STATUS, (MMP_UINT32 *)&statusISP);
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
    MMP_UINT32 status  = 0;
    MMP_UINT16 timeOut = 0;

    //
    //  Wait ISP engine idle!   0x6FC D[9] 1: empty
    //
    isp_ReadHwReg(ISP_REG_ISP_ENGINE_STATUS, (MMP_UINT32 *)&status);
    while (!(status & 0x00000200))
    {
        isp_sleep(1);
        if (++timeOut > 2000)
        {
            //ISP_LogReg();
            isp_msg_ex(ISP_MSG_TYPE_ERR, "ERROR_ISP_SLICEBUF_NOT_EMPTY\n");
            result = ISP_ERR_NOT_IDLE;
            goto end;
        }
        isp_ReadHwReg(ISP_REG_ISP_ENGINE_STATUS, (MMP_UINT32 *)&status);
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
#ifndef CFG_VP_TEST_MODULE_ENABLE
    // set Lcd on-fly on
    isp_WriteHwRegMask(ITH_LCD_SET1_REG,        (MMP_UINT16)0x0800, 0x0800); // enable Lcd on-fly flag
    isp_WriteHwRegMask(ITH_LCD_COLORCON_OUT1_1, (MMP_UINT16)0x8000, 0x8000); // Enable Port1 color convert
    isp_WriteHwRegMask(ITH_LCD_COLORCON_OUT2_1, (MMP_UINT16)0x8000, 0x8000); // Enable Port2 color convert
    isp_WriteHwRegMask(ITH_LCD_UPDATE_REG,      (MMP_UINT16)0x8003, 0x8003); // Lcd layer 1 update parameters
#endif
}

void
ISP_DisableLCD_OnFly(
    void)
{
#ifndef CFG_VP_TEST_MODULE_ENABLE
    // set Lcd on-fly off
    isp_WriteHwRegMask(ITH_LCD_SET1_REG,        (MMP_UINT16)0x0000, 0x0800); // disable Lcd on-fly flag
    isp_WriteHwRegMask(ITH_LCD_COLORCON_OUT1_1, (MMP_UINT16)0x0000, 0x8000); // Disable Port1 color convert
    isp_WriteHwRegMask(ITH_LCD_COLORCON_OUT2_1, (MMP_UINT16)0x0000, 0x8000); // Disable Port2 color convert
    isp_WriteHwRegMask(ITH_LCD_UPDATE_REG,      (MMP_UINT16)0x8003, 0x8003); // Lcd layer 1 update parameters
#endif
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
#ifndef CFG_VP_TEST_MODULE_ENABLE
    isp_EnginClockOn();
    isp_EnginReset();
#endif
}

void
ISP_PowerDown(
    void)
{
#ifndef CFG_VP_TEST_MODULE_ENABLE
    isp_EnginReset();
    isp_EnginClockOff();
#endif
}
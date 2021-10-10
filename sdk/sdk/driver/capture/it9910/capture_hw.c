//#include "host/host.h"
//#include "sys/sys.h"
//#include "mmp_types.h"

#include "capture_config.h"
#include "capture_hw.h"
#include "capture_reg.h"
#include "capture_util.h"

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

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================

void
Cap_Fire(void)
{
    HOST_WriteRegister(GEN_SETTING_REG_13 , 0x8000);
}

void
Cap_UnFire(void)
{
    HOST_WriteRegister(GEN_SETTING_REG_13, 0x0000);
}

void
Cap_TurnOnClock_Reg(MMP_BOOL flag)
{
    MMP_UINT16 value;
    HOST_ReadRegister(GEN_SETTING_REG_43, &value);

    if (flag == MMP_TRUE && (value & 0x1000) == 0x0000)
        HOST_WriteRegisterMask(GEN_SETTING_REG_43, 0x1000, 0x1000);
    else if (flag == MMP_FALSE && (value & 0x1000) == 0x1000)
        HOST_WriteRegisterMask(GEN_SETTING_REG_43, 0x0000, 0x1000);
}

/* Input Related Reg Setting */
MMP_RESULT
Cap_Set_Input_Pin_Mux_Reg(CAP_INPUT_MUX_INFO *pininfo)
{
    MMP_RESULT result = MMP_TRUE;
    MMP_UINT16 data = 0, mask = 0;

    data = pininfo->Y_Pin_Num[0];
    data |= (pininfo->Y_Pin_Num[1] << 8);
    HOST_WriteRegister(GEN_SETTING_REG_16 , data);

    data = pininfo->Y_Pin_Num[2];
    data |= (pininfo->Y_Pin_Num[3] << 8);
    HOST_WriteRegister(GEN_SETTING_REG_17 , data);

    data = pininfo->Y_Pin_Num[4];
    data |= (pininfo->Y_Pin_Num[5] << 8);
    HOST_WriteRegister(GEN_SETTING_REG_18 , data);

    data = pininfo->Y_Pin_Num[6];
    data |= (pininfo->Y_Pin_Num[7] << 8);
    HOST_WriteRegister(GEN_SETTING_REG_19 , data);

    data = pininfo->Y_Pin_Num[8];
    data |= (pininfo->Y_Pin_Num[9] << 8);
    HOST_WriteRegister(GEN_SETTING_REG_20 , data);

    data = pininfo->Y_Pin_Num[10];
    data |= (pininfo->Y_Pin_Num[11] << 8);
    HOST_WriteRegister(GEN_SETTING_REG_21 , data);

    data = pininfo->U_Pin_Num[0];
    data |= (pininfo->U_Pin_Num[1] << 8);
    HOST_WriteRegister(GEN_SETTING_REG_22 , data);

    data = pininfo->U_Pin_Num[2];
    data |= (pininfo->U_Pin_Num[3] << 8);
    HOST_WriteRegister(GEN_SETTING_REG_23 , data);

    data = pininfo->U_Pin_Num[4];
    data |= (pininfo->U_Pin_Num[5] << 8);
    HOST_WriteRegister(GEN_SETTING_REG_24 , data);

    data = pininfo->U_Pin_Num[6];
    data |= (pininfo->U_Pin_Num[7] << 8);
    HOST_WriteRegister(GEN_SETTING_REG_25 , data);

    data = pininfo->U_Pin_Num[8];
    data |= (pininfo->U_Pin_Num[9] << 8);
    HOST_WriteRegister(GEN_SETTING_REG_26 , data);

    data = pininfo->U_Pin_Num[10];
    data |= (pininfo->U_Pin_Num[11] << 8);
    HOST_WriteRegister(GEN_SETTING_REG_27 , data);

    data = pininfo->V_Pin_Num[0];
    data |= (pininfo->V_Pin_Num[1] << 8);
    HOST_WriteRegister(GEN_SETTING_REG_28 , data);

    data = pininfo->V_Pin_Num[2];
    data |= (pininfo->V_Pin_Num[3] << 8);
    HOST_WriteRegister(GEN_SETTING_REG_29 , data);

    data = pininfo->V_Pin_Num[4];
    data |= (pininfo->V_Pin_Num[5] << 8);
    HOST_WriteRegister(GEN_SETTING_REG_30 , data);

    data = pininfo->V_Pin_Num[6];
    data |= (pininfo->V_Pin_Num[7] << 8);
    HOST_WriteRegister(GEN_SETTING_REG_31 , data);

    data = pininfo->V_Pin_Num[8];
    data |= (pininfo->V_Pin_Num[9] << 8);
    HOST_WriteRegister(GEN_SETTING_REG_32 , data);

    data = pininfo->V_Pin_Num[10];
    data |= (pininfo->V_Pin_Num[11] << 8);
    HOST_WriteRegister(GEN_SETTING_REG_33 , data);

    //Set Input clk mode
    data = (pininfo->UCLKSrc & 0x07) |
            ((pininfo->UCLKInv & 0x1) << 3) |
            ((pininfo->UCLKDly & 0x0F) << 4) |
            ((pininfo->UCLKRatio & 0x0F) << 7) |
            ((pininfo->EnUCLK & 0x1) << 12);

    mask = (0x07) |
           (0x1 << 3) |
           (0xF << 4) |
           (0xF << 7) |
           (0x1 << 12);
    HOST_WriteRegisterMask(GEN_SETTING_REG_43, data, mask);

    HOST_ReadRegister(GEN_SETTING_REG_124, (MMP_UINT16*)&data);
    data |= (pininfo->UCLKVDSel << 8);
    HOST_WriteRegister(GEN_SETTING_REG_124, data);

    return result;
}


/* Output Related Reg Setting */
void
Cap_Set_Output_Pin_Mux_Reg(CAP_OUTPUT_PIN_SELECT *pininfo)
{
    MMP_UINT16 data = 0;

    data = pininfo->CAPIOVDOUTSEL[0];
    data |= (pininfo->CAPIOVDOUTSEL[1] << 8);
    HOST_WriteRegister(HDMI_LOOP_THROUGH_SETTING_REG_106, data);

    data = pininfo->CAPIOVDOUTSEL[2];
    data |= (pininfo->CAPIOVDOUTSEL[3] << 8);
    HOST_WriteRegister(HDMI_LOOP_THROUGH_SETTING_REG_107, data);

    data = pininfo->CAPIOVDOUTSEL[4];
    data |= (pininfo->CAPIOVDOUTSEL[5] << 8);
    HOST_WriteRegister(HDMI_LOOP_THROUGH_SETTING_REG_108, data);

    data = pininfo->CAPIOVDOUTSEL[6];
    data |= (pininfo->CAPIOVDOUTSEL[7] << 8);
    HOST_WriteRegister(HDMI_LOOP_THROUGH_SETTING_REG_109, data);

    data = pininfo->CAPIOVDOUTSEL[8];
    data |= (pininfo->CAPIOVDOUTSEL[9] << 8);
    HOST_WriteRegister(HDMI_LOOP_THROUGH_SETTING_REG_110, data);

    data = pininfo->CAPIOVDOUTSEL[10];
    data |= (pininfo->CAPIOVDOUTSEL[11] << 8);
    HOST_WriteRegister(HDMI_LOOP_THROUGH_SETTING_REG_111, data);

    data = pininfo->CAPIOVDOUTSEL[12];
    data |= (pininfo->CAPIOVDOUTSEL[13] << 8);
    HOST_WriteRegister(HDMI_LOOP_THROUGH_SETTING_REG_112, data);

    data = pininfo->CAPIOVDOUTSEL[14];
    data |= (pininfo->CAPIOVDOUTSEL[15] << 8);
    HOST_WriteRegister(HDMI_LOOP_THROUGH_SETTING_REG_113, data);

    data = pininfo->CAPIOVDOUTSEL[16];
    data |= (pininfo->CAPIOVDOUTSEL[17] << 8);
    HOST_WriteRegister(HDMI_LOOP_THROUGH_SETTING_REG_114, data);

    data = pininfo->CAPIOVDOUTSEL[18];
    data |= (pininfo->CAPIOVDOUTSEL[19] << 8);
    HOST_WriteRegister(HDMI_LOOP_THROUGH_SETTING_REG_115, data);

    data = pininfo->CAPIOVDOUTSEL[20];
    data |= (pininfo->CAPIOVDOUTSEL[21] << 8);
    HOST_WriteRegister(HDMI_LOOP_THROUGH_SETTING_REG_116, data);

    data = pininfo->CAPIOVDOUTSEL[22];
    data |= (pininfo->CAPIOVDOUTSEL[23] << 8);
    HOST_WriteRegister(HDMI_LOOP_THROUGH_SETTING_REG_117, data);

    data = pininfo->CAPIOVDOUTSEL[24];
    data |= (pininfo->CAPIOVDOUTSEL[25] << 8);
    HOST_WriteRegister(HDMI_LOOP_THROUGH_SETTING_REG_118, data);

    data = pininfo->CAPIOVDOUTSEL[26];
    data |= (pininfo->CAPIOVDOUTSEL[27] << 8);
    HOST_WriteRegister(HDMI_LOOP_THROUGH_SETTING_REG_119, data);

    data = pininfo->CAPIOVDOUTSEL[28];
    data |= (pininfo->CAPIOVDOUTSEL[29] << 8);
    HOST_WriteRegister(HDMI_LOOP_THROUGH_SETTING_REG_120, data);

    data = pininfo->CAPIOVDOUTSEL[30];
    data |= (pininfo->CAPIOVDOUTSEL[31] << 8);
    HOST_WriteRegister(HDMI_LOOP_THROUGH_SETTING_REG_121, data);

    data = pininfo->CAPIOVDOUTSEL[32];
    data |= (pininfo->CAPIOVDOUTSEL[33] << 8);
    HOST_WriteRegister(HDMI_LOOP_THROUGH_SETTING_REG_122, data);

    data = pininfo->CAPIOVDOUTSEL[34];
    data |= (pininfo->CAPIOVDOUTSEL[35] << 8);
    HOST_WriteRegister(HDMI_LOOP_THROUGH_SETTING_REG_123, data);

    HOST_ReadRegister(GEN_SETTING_REG_124,(MMP_UINT16*) &data);
    data &= 0xFFF0;
    data|= pininfo->CAPIOVDOUTSEL_X;
    HOST_WriteRegister(GEN_SETTING_REG_124, data);

}

MMP_RESULT
Cap_Set_Output_Clk_Mode_Reg(MMP_UINT16 value)
{
    MMP_RESULT result = MMP_TRUE;
    MMP_UINT16 data = 0;

    HOST_ReadRegister(GEN_SETTING_REG_124,(MMP_UINT16 *)&data);

    data &= 0xC0FF;
    data |= (value << 8);

    HOST_WriteRegister(GEN_SETTING_REG_124, data);

    return result;
}

void
Cap_Set_Color_Format_Reg (CAP_YUV_INFO *pYUVinfo)
{
    MMP_UINT16 Value;

    Value = ((pYUVinfo->InputMode & 0x1) << 12) |
            ((pYUVinfo->ClockPerPixel & 0x1) << 13) |
            ((pYUVinfo->YUV422Format & 0x3) << 14) ;

    HOST_WriteRegisterMask(GEN_SETTING_REG_2, Value, 0xF000);
}

MMP_RESULT
Cap_Set_IO_Mode_Reg(CAP_IO_MODE_INFO *io_config)
{
    // ask H.C
    MMP_RESULT result = MMP_TRUE;
    MMP_UINT16 data;

    HOST_WriteRegister(GEN_SETTING_REG_36, io_config->CAPIOEN_VD_15_0);
    HOST_WriteRegister(GEN_SETTING_REG_37, io_config->CAPIOEN_VD_31_16);

    data = 0;
    data |= ( ((io_config->CAPIOEN_VD_32_35 & 0xF) << 0) |
              ((io_config->CAPIOEN_X & 0x1) << 8) |
              ((io_config->CAPIOEN_HS & 0x1) << 9) |
              ((io_config->CAPIOEN_VS & 0x1) << 10) |
              ((io_config->CAPIOEN_DE & 0x1) << 11) |
              ((io_config->CAPIOEN_Field & 0x1) << 12) |
              ((io_config->CAPIOEN_PMCLK & 0x1) << 13) );
    HOST_WriteRegister(GEN_SETTING_REG_38, data);


    //CAP IO FF
    HOST_WriteRegister(GEN_SETTING_REG_39, io_config->CAPIOFFEn_VD_00_15);
    HOST_WriteRegister(GEN_SETTING_REG_40, io_config->CAPIOFFEn_VD_16_31);
    HOST_WriteRegister(GEN_SETTING_REG_41, io_config->CAPIOFFEn_VD_32_47);
    data = 0;
    data |= (((io_config->HDMICLKDly & 0xF)  << 12)| (io_config->HDMICLKInv << 11) |
             (io_config->CAPIOFFEn_Field << 10) | (io_config->CAPIOFFEn_DE << 9) |
             (io_config->CAPIOFFEn_VS << 8) | (io_config->CAPIOFFEn_HS << 7)|
              io_config->CAPIOFFEn_VD_48_54);
    //HOST_WriteRegisterMask(GEN_SETTING_REG_42, data, 0x07FF);
    HOST_WriteRegister(GEN_SETTING_REG_42, data);

    HOST_WriteRegisterMask (GEN_SETTING_REG_34, io_config->EnInternalRxHDMI, 0x0001);

    return result;
}


void
Cap_Set_Input_Data_Info_Reg(CAP_INPUT_INFO *pIninfo)
{
    MMP_UINT16 VDUHeight, data;

    /* Set Interlace or Progressive */
    HOST_WriteRegisterMask(GEN_SETTING_REG_1, (pIninfo->Interleave << 9), 0x0200);

    /* Set Hsync & Vsync Porlarity */
    HOST_WriteRegisterMask(GEN_SETTING_REG_1, (pIninfo->HSyncPol << 7), 0x0080);
    HOST_WriteRegisterMask(GEN_SETTING_REG_1, (pIninfo->VSyncPol << 8), 0x0100);


    /* Set YUV pitch */
    HOST_WriteRegister  (GEN_SETTING_REG_95, pIninfo->PitchY);
    HOST_WriteRegister  (GEN_SETTING_REG_96, pIninfo->PitchUV);

    /*  Set Active Region  Set CapWidth & Cap Height  */
    HOST_WriteRegister(GEN_SETTING_REG_2, (pIninfo->capwidth) &0x0FFF);
    HOST_WriteRegister(GEN_SETTING_REG_3, (pIninfo->capheight)&0x0FFF);
    HOST_WriteRegister(GEN_SETTING_REG_7, pIninfo->HNum1);
    HOST_WriteRegister(GEN_SETTING_REG_8, pIninfo->HNum2);
    HOST_WriteRegister(GEN_SETTING_REG_9, pIninfo->LineNum1);
    HOST_WriteRegister(GEN_SETTING_REG_10, pIninfo->LineNum2);
    HOST_WriteRegister(GEN_SETTING_REG_11, pIninfo->LineNum3);
    HOST_WriteRegister(GEN_SETTING_REG_12, pIninfo->LineNum4);

    /* Set ROI */
    /* The source frame start X position */
    HOST_WriteRegister(GEN_SETTING_REG_44, pIninfo->ROIPosX);
    /* The source frame start Y position */
    HOST_WriteRegister(GEN_SETTING_REG_45, pIninfo->ROIPosY);
    /* The width size of ROI image */
    HOST_WriteRegister(GEN_SETTING_REG_46, pIninfo->ROIWidth);
    /* The height size of ROI image */
    HOST_WriteRegister(GEN_SETTING_REG_47, pIninfo->ROIHeight);

    //Color Depth
    HOST_WriteRegisterMask(GEN_SETTING_REG_1, (pIninfo->ColorDepth << 5), 0x0060);

    if (Capctxt->funen.EnCrossLineDE == MMP_TRUE)
    {
        if (pIninfo->Interleave)
            VDUHeight = pIninfo->capheight + 2;
        else
            VDUHeight = pIninfo->capheight + 1;
    }
    else
    {
        VDUHeight = pIninfo->capheight;
    }

    HOST_WriteRegisterMask(GEN_SETTING_REG_126 , (0x1 & 0x0FFF), 0x0FFF);

    if (Capctxt->YUVinfo.ClockPerPixel == CPP_1P2T)
        HOST_WriteRegisterMask(GEN_SETTING_REG_127 , ((pIninfo->capwidth << 1) & 0x0FFF), 0x0FFF);
    else
        HOST_WriteRegisterMask(GEN_SETTING_REG_127 , (pIninfo->capwidth & 0x0FFF), 0x0FFF);

    HOST_WriteRegisterMask(AV_SYNC_SETTING_REG_129 , (VDUHeight & 0x07FF) << 0x5, (0x07FF << 0x5));


    HOST_WriteRegisterMask(AV_SYNC_SETTING_REG_130,
        ((Capctxt->funen.EnHSPosEdge & 0x1) << 14) | ((Capctxt->funen.EnYPbPrTopVSMode & 0x1) << 13) | ((Capctxt->funen.EnCrossLineDE & 0x1) << 12), (0x1 << 14) | (0x1 << 13) |(0x1 << 12));

    HOST_WriteRegisterMask(GEN_SETTING_REG_6, ((Capctxt->funen.EnDlyVS & 0x1) << 14), (0x1 << 14));
}

void
Cap_Set_HorScale_Width_Reg(CAP_OUTPUT_INFO* pOutInfo)
{
    HOST_WriteRegisterMask(GEN_SETTING_REG_63, (MMP_UINT16)(pOutInfo->OutWidth<< 4), 0xFFF0);
}

/* Frame rate control */
void Cap_Set_Skip_Pattern_Reg (MMP_UINT16 pattern, MMP_UINT16 period)
{
    HOST_WriteRegister(GEN_SETTING_REG_4, pattern);
    HOST_WriteRegister(COLOR_SETTING_REG_105, period);
}

MMP_BOOL
IsCapFire(void)
{
    MMP_UINT16 data;
    HOST_ReadRegister(GEN_SETTING_REG_13,(MMP_UINT16 *)& data);

    if ((data&0x8000) == 0x8000)
        return MMP_TRUE;
    else
        return MMP_FALSE;
}

MMP_RESULT
Cap_WaitEngineIdle(void)
{
    MMP_RESULT  result = MMP_SUCCESS;
    MMP_UINT16  status = 0;
    MMP_UINT16  timeOut = 0;

    HOST_ReadRegister(ENGINE_STATUS_REG_8, (MMP_UINT16*)&status);
    while(!(status&0x0001))
    {
        PalSleep(1);
        if( ++timeOut > 2000 )
        {

            cap_msg_ex(CAP_MSG_TYPE_ERR, "Capture Not Idle !!!!!\n");
            result = MMP_RESULT_ERROR;
            goto end;
        }
        HOST_ReadRegister(ENGINE_STATUS_REG_8, (MMP_UINT16*)&status);
    }

end:
    if( result )
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) ERROR !!!!!\n",  __FUNCTION__, __LINE__);

    return (MMP_RESULT)result;

}

/* Get Capture Lane error status */
MMP_UINT16
Cap_Get_Lane_status(CAP_LANE_STATUS lanenum)
{
    MMP_UINT16 data = 0;

    switch (lanenum)
    {
    case CAP_LANE0_STATUS:
    HOST_ReadRegister(ENGINE_STATUS_REG_8, (MMP_UINT16*)&data);
    break;
    case CAP_LANE1_STATUS:
    HOST_ReadRegister(ENGINE_STATUS_REG_28, (MMP_UINT16*)&data);
    break;
    case CAP_LANE2_STATUS:
    HOST_ReadRegister(ENGINE_STATUS_REG_29, (MMP_UINT16*)&data);
    break;
    case CAP_LANE3_STATUS:
    HOST_ReadRegister(ENGINE_STATUS_REG_30, (MMP_UINT16*)&data);
    break;
    default:
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) ERROR\n", __FUNCTION__, __LINE__);
    }
    //cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) ERROR\n", __FUNCTION__, __LINE__);

    return (MMP_UINT16)data;
}

MMP_UINT16
Cap_Get_Hsync_Polarity(void)
{
    MMP_UINT16 data;
    HOST_ReadRegister(GEN_SETTING_REG_1, (MMP_UINT16 *)&data);
    data &= 0x0080;
    cap_msg_ex(CAP_MSG_TYPE_ERR, "%s data = 0x%x\n", data);
    return data;
}

MMP_UINT16
Cap_Get_Vsync_Polarity(void)
{
    MMP_UINT16 data;
    HOST_ReadRegister(GEN_SETTING_REG_1, (MMP_UINT16 *)&data);
    data &= 0x0010;
    cap_msg_ex(CAP_MSG_TYPE_ERR, "%s data = 0x%x\n", data);
    return data;
}

MMP_UINT16
Cap_Get_Hsync_Polarity_Status(void)
{
    MMP_UINT16 data;
    HOST_ReadRegister(ENGINE_STATUS_REG_8, (MMP_UINT16*)&data);
    data &= 0x0010;
    cap_msg_ex(CAP_MSG_TYPE_ERR, "%s data = 0x%x\n", data);
    return data;
}

MMP_UINT16
Cap_Get_Vsync_Polarity_Status(void)
{
    MMP_UINT16 data;
    HOST_ReadRegister(GEN_SETTING_REG_1, (MMP_UINT16 *)&data);
    data &= 0x0020;
    cap_msg_ex(CAP_MSG_TYPE_ERR, "%s data = 0x%x\n", data);
    return data;
}


void
Cap_Engine_Reset(void)
{
    HOST_WriteRegisterMask(0x0062, 0xFFFF, 0x0008);
    MMP_Sleep(1);
    HOST_WriteRegisterMask(0x0062, 0x0000, 0x0008);
    MMP_Sleep(1);

    Cap_Set_EnableFrameRate_Reg();
    MMP_Sleep(50);

}

void
Cap_Engine_Register_Reset(void)
{
    HOST_WriteRegisterMask(0x0062, 0xFFFF, 0x000C);
    MMP_Sleep(1);
    HOST_WriteRegisterMask(0x0062, 0x0000, 0x000C);
    MMP_Sleep(1);
}

void
Cap_Set_Enable_Interrupt(MMP_BOOL flag)
{
    if (flag == MMP_TRUE)
        HOST_WriteRegisterMask(GEN_SETTING_REG_43, 0x8000, M_CAP_ENABLE_INT);
    else
        HOST_WriteRegisterMask(GEN_SETTING_REG_43, 0x0000, M_CAP_ENABLE_INT);
}

void
Cap_Set_Interrupt_Mode(MMP_UINT8 Intr_Mode)
{
    MMP_UINT16 data;

    switch (Intr_Mode)
    {
    case CAP_INT_MODE_ERR_FRAMEEND:
            data = (CAP_INT_MODE_ERR_FRAMEEND << 13);
        break;
    case CAP_INT_MODE_ERR:
            data = (CAP_INT_MODE_ERR << 13);
        break;
    case CAP_INT_MODE_MULTI_CHANNEL:
            data = (CAP_INT_MODE_MULTI_CHANNEL << 13);
        break;
    default:
        break;
    }

    HOST_WriteRegisterMask(GEN_SETTING_REG_43, data, (0x3 << 13));
}

void
Cap_Set_Color_Bar(CAP_COLOR_BAR_CONFIG color_info)
{
    MMP_UINT16 data = 0;

    /* color bar reg97 */
    if (color_info.Enable_colorbar)
        data |= B_CAP_COLOR_BAR_ENABLE;

    if (color_info.Hsync_pol)
        data |= B_CAP_COLOR_BAR_HSPOL;

    if (color_info.Vsync_pol)
        data |= B_CAP_COLOR_BAR_VSPOL;

    data |= (color_info.VS_act_start_line & 0x0FFF);

    HOST_WriteRegister(COLOR_SETTING_REG_97, data);


    /* color bar reg98 */
    HOST_WriteRegister(COLOR_SETTING_REG_98, (color_info.VS_act_line&0x0FFF));
    /* color bar reg 99 */
    HOST_WriteRegister(COLOR_SETTING_REG_99, (color_info.blank_line1&0x0FFF));
    /* color bar reg 100 */
    HOST_WriteRegister(COLOR_SETTING_REG_100, (color_info.act_line&0x0FFF));
    /* color bar reg 101 */
    HOST_WriteRegister(COLOR_SETTING_REG_101, (color_info.blank_line2&0x0FFF));
    /* color bar reg 102 */
    HOST_WriteRegister(COLOR_SETTING_REG_102, (color_info.Hs_act&0x0FFF));
    /* color bar reg 103 */
    HOST_WriteRegister(COLOR_SETTING_REG_103, (color_info.blank_pix1&0x0FFF));
    /* color bar reg 104 */
    HOST_WriteRegister(COLOR_SETTING_REG_104, (color_info.act_pix&0x0FFF));
    /* color bar reg 105 */
    HOST_WriteRegister(COLOR_SETTING_REG_105, (color_info.blank_pix2&0x0FFF));

}

MMP_RESULT
Cap_Get_BT601_SyncCnt(void)
{
    MMP_RESULT result = MMP_TRUE;
    MMP_UINT16 data;

    HOST_ReadRegister(ENGINE_STATUS_REG_13, (MMP_UINT16*)&data);
    cap_msg_ex(CAP_MSG_TYPE_ERR, "RawHtotal = 0x%x\n", data);
    HOST_ReadRegister(ENGINE_STATUS_REG_14, (MMP_UINT16*)&data);
    return result;
}

MMP_RESULT
Cap_Get_Detected_Region(void)
{
    MMP_RESULT result = MMP_TRUE;
    MMP_INT16 data;

    HOST_ReadRegister(ENGINE_STATUS_REG_9, (MMP_UINT16*)&data);
    cap_msg_ex(CAP_MSG_TYPE_ERR, "HTotal = 0x%x\n", data);
    HOST_ReadRegister(ENGINE_STATUS_REG_10, (MMP_UINT16*)&data);
    cap_msg_ex(CAP_MSG_TYPE_ERR, "VTotal = 0x%x\n", data);

    HOST_ReadRegister(ENGINE_STATUS_REG_11, (MMP_UINT16*)&data);
    cap_msg_ex(CAP_MSG_TYPE_ERR, "Width = 0x%x\n", data);
    HOST_ReadRegister(ENGINE_STATUS_REG_12,(MMP_UINT16*)&data);
    cap_msg_ex(CAP_MSG_TYPE_ERR, "Height = 0x%x\n", data);

    HOST_ReadRegister(ENGINE_STATUS_REG_15,(MMP_UINT16*)&data);
    cap_msg_ex(CAP_MSG_TYPE_ERR, "H1 = 0x%x\n", data);
    HOST_ReadRegister(ENGINE_STATUS_REG_16, (MMP_UINT16*)&data);
    cap_msg_ex(CAP_MSG_TYPE_ERR, "H2 = 0x%x\n", data);
    HOST_ReadRegister(ENGINE_STATUS_REG_17, (MMP_UINT16*)&data);
    cap_msg_ex(CAP_MSG_TYPE_ERR, "L1 = 0x%x\n", data);

    HOST_ReadRegister(ENGINE_STATUS_REG_18, (MMP_UINT16*)&data);
    cap_msg_ex(CAP_MSG_TYPE_ERR, "L2 = 0x%x\n", data);

    HOST_ReadRegister(ENGINE_STATUS_REG_19, (MMP_UINT16*)&data);
    cap_msg_ex(CAP_MSG_TYPE_ERR, "L3 = 0x%x\n", data);
    HOST_ReadRegister(ENGINE_STATUS_REG_20, (MMP_UINT16*)&data);
    cap_msg_ex(CAP_MSG_TYPE_ERR, "L4 = 0x%x\n", data);

    return result;
}

MMP_UINT16
Cap_Get_Sync_Status(void)
{
    MMP_UINT16 data;
    /* REG: 0x1F22 */
    HOST_ReadRegister(ENGINE_STATUS_REG_8, (MMP_UINT16*)&data);
    return data;
}

MMP_UINT16
Cap_Get_Error_Status(void)
{
    /* ENGINE_STATUS_REG_8, EverErrld, HoltforFire */
    MMP_UINT16 data;
    HOST_ReadRegister(ENGINE_STATUS_REG_8, (MMP_UINT16 *)&data);
    cap_msg_ex(CAP_MSG_TYPE_ERR, "ENGINE STATUS = 0x%x\n", data);
    return data;
}

void Cap_Dump_Reg(void)
{
    MMP_UINT16 i, data;

    for(i=0 ; i<= 246; i=i+2)
    {
        char buf[20];
        HOST_ReadRegister(0x2000+i, (MMP_UINT16 *) &data);
        cap_msg_ex(CAP_MSG_TYPE_ERR, buf, "reg 0x%x", 0x2000+i);
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s val=0x%04x \n", buf, data);
    }

    HOST_ReadRegister(0x2018 , (MMP_UINT16*)&data);
    cap_msg_ex(CAP_MSG_TYPE_ERR, "reg 0x2018 val = 0x%04x\n", data);
}

MMP_UINT16
Cap_Clean_Intr(void)
{
    MMP_UINT16 data;
    HOST_ReadRegister(ENGINE_STATUS_REG_7, (MMP_UINT16*)&data);
    return data;
}

void
Cap_Set_EnableFrameRate_Reg(void)
{
    MMP_UINT16 Value;

    HOST_ReadRegister(GEN_SETTING_REG_6, (MMP_UINT16*)&Value);

    if ((Value & 0x8000) == 0x0000)
    {
    //Enable frame rate counter
        Value |= ((0x1 & 0x1) << 15) ;
        HOST_WriteRegister(GEN_SETTING_REG_6, Value);
    }
}

void
Cap_Set_Enable_Reg(CAP_ENFUN_INFO* pFunEn)
{
    MMP_UINT16 Value;

    // Enable DDR mode
    HOST_WriteRegisterMask(GEN_SETTING_REG_1, (pFunEn->EnDDRMode << 15), 0x8000);
    // Data Enable mode in BT601
    HOST_WriteRegisterMask(GEN_SETTING_REG_1, (pFunEn->EnDEMode << 2), 0x0004);
    // BT601 or BT656
    HOST_WriteRegisterMask(GEN_SETTING_REG_1, (pFunEn->EnInBT656 << 1), 0x0002);

    if (Capctxt->YUVinfo.ClockPerPixel == CPP_1P2T)
        HOST_WriteRegisterMask(GEN_SETTING_REG_1, (0x1 << 14), 0x4000);
    else
        HOST_WriteRegisterMask(GEN_SETTING_REG_1, (0x0 << 14), 0x4000);

    // Enable CS fun or CC fun
    HOST_WriteRegisterMask(GEN_SETTING_REG_48, ((pFunEn->EnCSFun | pFunEn->EnCCFun) << 15), 0x8000);

    //Enable frame rate counter / NoHSyncForSensor
    Value = ((pFunEn->EnNoHSyncForSensor & 0x1) << 11) |
            ((0x1 & 0x1) << 15) ;
    HOST_WriteRegisterMask (GEN_SETTING_REG_6, Value, 0x8800);



    //Enable Use external HSYNC or H bit after sync
    //Enable Use external VSYNC or V bit after sync
    //Enable Use external DE after sync
    Value = ((pFunEn->EnUseExtDE & 0x1) << 12) |
            ((pFunEn->EnUseExtVRst & 0x1) << 13) |
            ((pFunEn->EnUseExtHRst & 0x1) << 14) ;
    HOST_WriteRegisterMask(GEN_SETTING_REG_5, Value, 0x7000);

    HOST_WriteRegisterMask(GEN_SETTING_REG_34, ((pFunEn->EnProgressiveToField & 0x1) << 15), 0x8000);

    //Enable  Port1UV2LineDS, interpolate even and odd lines
    HOST_WriteRegisterMask(GEN_SETTING_REG_74, ((pFunEn->EnPort1UV2LineDS & 0x1) << 4) , 0x0010);

}

MMP_RESULT
Cap_Set_ISP_HandShaking(CAP_ISP_HANDSHAKING_MODE mode, MMP_UINT8 preloadnum)
{
    MMP_RESULT result;
    MMP_UINT16 data;

    HOST_ReadRegister(GEN_SETTING_REG_1, (MMP_UINT16*)&data);

    if (mode == ONFLY_MODE)//onfly mode
    {
        data |= B_CAP_ONFLY_MODE;
        HOST_WriteRegister(GEN_SETTING_REG_1, data);
        HOST_ReadRegister(GEN_SETTING_REG_1, (MMP_UINT16*)&data);
        return (data & B_CAP_ONFLY_MODE)? MMP_TRUE : MMP_FALSE;
    }
    else if (mode == MEMORY_MODE)//memory mode
    {
        data &= ~B_CAP_ONFLY_MODE;
        HOST_WriteRegister(GEN_SETTING_REG_1, data);

        // Set PreLoadNum
        HOST_WriteRegisterMask(GEN_SETTING_REG_6, (preloadnum << 12), 0x3000);
        HOST_ReadRegister(GEN_SETTING_REG_1, (MMP_UINT16*)&data);
        return (data & B_CAP_ONFLY_MODE)? MMP_FALSE : MMP_TRUE;
    }
    else
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, "ISP HandShaking error\n");
        return MMP_FALSE;
    }
}

MMP_RESULT
Cap_Set_Error_Handleing(MMP_UINT16 errDetectEn)
{
    MMP_RESULT result = MMP_TRUE;

    HOST_WriteRegister(GEN_SETTING_REG_15, errDetectEn);

#ifdef SENSOR_DEV
        HOST_WriteRegister(GEN_SETTING_REG_14, 0x8000);
#else
        HOST_WriteRegister(GEN_SETTING_REG_14, 0x0080);
#endif

    return result;
}

MMP_RESULT
Cap_Set_Wait_Error_Reset(
    void)
{
    MMP_RESULT result = MMP_TRUE;

    HOST_WriteRegister(GEN_SETTING_REG_125, 0xFFFF);

    return result;
}

//FrameBase Reg
void
Cap_Set_FrameBase_0_Reg(MMP_UINT32 dst_Yaddr , MMP_UINT32 dst_UVaddr)
{
    HOST_WriteRegister(GEN_SETTING_REG_75 , dst_Yaddr&0xFFFF);
    HOST_WriteRegister(GEN_SETTING_REG_76, (dst_Yaddr&0xFFFF0000) >> 16);
    HOST_WriteRegister(GEN_SETTING_REG_77,  dst_UVaddr & 0xFFFF);
    HOST_WriteRegister(GEN_SETTING_REG_78, (dst_UVaddr&0xFFFF0000) >> 16);
}

void
Cap_Set_FrameBase_1_Reg(MMP_UINT32 dst_Yaddr , MMP_UINT32 dst_UVaddr)
{
    HOST_WriteRegister(GEN_SETTING_REG_79 , dst_Yaddr&0xFFFF);
    HOST_WriteRegister(GEN_SETTING_REG_80, (dst_Yaddr&0xFFFF0000) >> 16);
    HOST_WriteRegister(GEN_SETTING_REG_81,  dst_UVaddr & 0xFFFF);
    HOST_WriteRegister(GEN_SETTING_REG_82, (dst_UVaddr&0xFFFF0000) >> 16);
}

void
Cap_Set_FrameBase_2_Reg(MMP_UINT32 dst_Yaddr , MMP_UINT32 dst_UVaddr)
{
    HOST_WriteRegister(GEN_SETTING_REG_83 , dst_Yaddr&0xFFFF);
    HOST_WriteRegister(GEN_SETTING_REG_84, (dst_Yaddr&0xFFFF0000) >> 16);
    HOST_WriteRegister(GEN_SETTING_REG_85,  dst_UVaddr);
    HOST_WriteRegister(GEN_SETTING_REG_86, (dst_UVaddr&0xFFFF0000) >> 16);
}

void
Cap_Set_FrameBase_3_Reg(MMP_UINT32 dst_Yaddr , MMP_UINT32 dst_UVaddr)
{
    HOST_WriteRegister(GEN_SETTING_REG_87 , dst_Yaddr&0xFFFF);
    HOST_WriteRegister(GEN_SETTING_REG_88, (dst_Yaddr&0xFFFF0000) >> 16);
    HOST_WriteRegister(GEN_SETTING_REG_89,  dst_UVaddr);
    HOST_WriteRegister(GEN_SETTING_REG_90, (dst_UVaddr&0xFFFF0000) >> 16);
}

void
Cap_Set_FrameBase_4_Reg(MMP_UINT32 dst_Yaddr , MMP_UINT32 dst_UVaddr)
{
    HOST_WriteRegister(GEN_SETTING_REG_91 , dst_Yaddr&0xFFFF);
    HOST_WriteRegister(GEN_SETTING_REG_92, (dst_Yaddr&0xFFFF0000) >> 16);
    HOST_WriteRegister(GEN_SETTING_REG_93,  dst_UVaddr);
    HOST_WriteRegister(GEN_SETTING_REG_94, (dst_UVaddr&0xFFFF0000) >> 16);
}

void
Cap_Set_Buffer_addr_Reg(
    MMP_UINT32 *pAddr,
    MMP_UINT32 addrOffset)
{
    //Y0
    HOST_WriteRegister(GEN_SETTING_REG_75, ((pAddr[0] + addrOffset) & 0xFFFF));
    HOST_WriteRegister(GEN_SETTING_REG_76, (((pAddr[0] + addrOffset) >> 16) & 0xFFFF));

    //UV0
    HOST_WriteRegister(GEN_SETTING_REG_77, ((pAddr[1] + addrOffset) & 0xFFFF));
    HOST_WriteRegister(GEN_SETTING_REG_78, (((pAddr[1] + addrOffset) >> 16) & 0xFFFF));

    //Y1
    HOST_WriteRegister(GEN_SETTING_REG_79, ((pAddr[2] + addrOffset) & 0xFFFF));
    HOST_WriteRegister(GEN_SETTING_REG_80, (((pAddr[2] + addrOffset) >> 16) & 0xFFFF));

    //UV1
    HOST_WriteRegister(GEN_SETTING_REG_81, ((pAddr[3] + addrOffset) & 0xFFFF));
    HOST_WriteRegister(GEN_SETTING_REG_82, (((pAddr[3] + addrOffset) >> 16) & 0xFFFF));

    //Y2
    HOST_WriteRegister(GEN_SETTING_REG_83, ((pAddr[4] + addrOffset) & 0xFFFF));
    HOST_WriteRegister(GEN_SETTING_REG_84, (((pAddr[4] + addrOffset) >> 16) & 0xFFFF));

    //UV2
    HOST_WriteRegister(GEN_SETTING_REG_85, ((pAddr[5] + addrOffset) & 0xFFFF));
    HOST_WriteRegister(GEN_SETTING_REG_86, (((pAddr[5] + addrOffset) >> 16) & 0xFFFF));

    // Set framebuf num , memory mode
    HOST_WriteRegisterMask(GEN_SETTING_REG_74, (CAPTURE_MEM_BUF_COUNT >> 1) - 1, 0x0007);
}

void
CAP_SetScaleParam_Reg(
    const CAP_SCALE_CTRL    *pScaleFun)
{
    MMP_UINT16  Value = 0;
    MMP_UINT32  HCI;

    HCI = CAP_FLOATToFix(pScaleFun->HCI, 6, 14);

    HOST_WriteRegister(GEN_SETTING_REG_62, (MMP_UINT16)(HCI & 0xFFFF));
    HOST_WriteRegisterMask(GEN_SETTING_REG_63, (MMP_UINT16)((MMP_UINT32)(HCI & 0xF0000) >> 16), 0x000F);

}

void
Cap_SetIntScaleMatrixH_Reg(
    MMP_UINT16  WeightMatX[][CAP_SCALE_TAP])
{
    MMP_UINT16  Value;

    Value = ((WeightMatX[0][0] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[0][1] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(GEN_SETTING_REG_64, (MMP_UINT16)Value);

    Value = ((WeightMatX[0][2] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[0][3] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(GEN_SETTING_REG_65, (MMP_UINT16)Value);

    Value = ((WeightMatX[1][0] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[1][1] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(GEN_SETTING_REG_66, (MMP_UINT16)Value);

    Value = ((WeightMatX[1][2] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[1][3] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(GEN_SETTING_REG_67, (MMP_UINT16)Value);

    Value = ((WeightMatX[2][0] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[2][1] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(GEN_SETTING_REG_68, (MMP_UINT16)Value);

    Value = ((WeightMatX[2][2] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[2][3] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(GEN_SETTING_REG_69, (MMP_UINT16)Value);

    Value = ((WeightMatX[3][0] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[3][1] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(GEN_SETTING_REG_70, (MMP_UINT16)Value);

    Value = ((WeightMatX[3][2] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[3][3] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(GEN_SETTING_REG_71, (MMP_UINT16)Value);

    Value = ((WeightMatX[4][0] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[4][1] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(GEN_SETTING_REG_72, (MMP_UINT16)Value);

    Value = ((WeightMatX[4][2] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[4][3] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(GEN_SETTING_REG_73, (MMP_UINT16)Value);
}


//=============================================================================
/**
* RGB to YUV transfer matrix.
*/
//=============================================================================
void
Cap_SetRGBtoYUVMatrix_Reg(
    const CAP_RGB_TO_YUV    *pRGBtoYUV)
{
    HOST_WriteRegisterMask(GEN_SETTING_REG_48, 0x0, 0x00FF);
    HOST_WriteRegister(GEN_SETTING_REG_49, (MMP_UINT16)(0x0));

    HOST_WriteRegister(GEN_SETTING_REG_50, (MMP_UINT16)(pRGBtoYUV->_11 & CAP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(GEN_SETTING_REG_51, (MMP_UINT16)(pRGBtoYUV->_12 & CAP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(GEN_SETTING_REG_52, (MMP_UINT16)(pRGBtoYUV->_13 & CAP_BIT_RGB_TO_YUV));

    HOST_WriteRegister(GEN_SETTING_REG_53, (MMP_UINT16)(pRGBtoYUV->_21 & CAP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(GEN_SETTING_REG_54, (MMP_UINT16)(pRGBtoYUV->_22 & CAP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(GEN_SETTING_REG_55, (MMP_UINT16)(pRGBtoYUV->_23 & CAP_BIT_RGB_TO_YUV));

    HOST_WriteRegister(GEN_SETTING_REG_56, (MMP_UINT16)(pRGBtoYUV->_31 & CAP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(GEN_SETTING_REG_57, (MMP_UINT16)(pRGBtoYUV->_32 & CAP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(GEN_SETTING_REG_58, (MMP_UINT16)(pRGBtoYUV->_33 & CAP_BIT_RGB_TO_YUV));

    HOST_WriteRegister(GEN_SETTING_REG_59, (MMP_UINT16)(pRGBtoYUV->ConstY & CAP_BIT_RGB_TO_YUV_CONST));
    HOST_WriteRegister(GEN_SETTING_REG_60, (MMP_UINT16)(pRGBtoYUV->ConstU & CAP_BIT_RGB_TO_YUV_CONST));
    HOST_WriteRegister(GEN_SETTING_REG_61, (MMP_UINT16)(pRGBtoYUV->ConstV & CAP_BIT_RGB_TO_YUV_CONST));
}

//=============================================================================
/**
* Set color correction matrix and constant.
*/
//=============================================================================
void
Cap_SetCCMatrix_Reg(
    const CAP_COLOR_CORRECTION    *pColorCorrect)
{
    MMP_UINT16 value;

    value = pColorCorrect->OffsetR & 0xFF;
    HOST_WriteRegisterMask(GEN_SETTING_REG_48, value, 0x00FF);

    value = ((pColorCorrect->OffsetB & 0xFF) << 8) | (pColorCorrect->OffsetG & 0xFF);
    HOST_WriteRegister(GEN_SETTING_REG_49, (MMP_UINT16)(value));

    HOST_WriteRegister(GEN_SETTING_REG_50, (MMP_UINT16)(pColorCorrect->_11 & CAP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(GEN_SETTING_REG_51, (MMP_UINT16)(pColorCorrect->_12 & CAP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(GEN_SETTING_REG_52, (MMP_UINT16)(pColorCorrect->_13 & CAP_BIT_RGB_TO_YUV));

    HOST_WriteRegister(GEN_SETTING_REG_53, (MMP_UINT16)(pColorCorrect->_21 & CAP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(GEN_SETTING_REG_54, (MMP_UINT16)(pColorCorrect->_22 & CAP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(GEN_SETTING_REG_55, (MMP_UINT16)(pColorCorrect->_23 & CAP_BIT_RGB_TO_YUV));

    HOST_WriteRegister(GEN_SETTING_REG_56, (MMP_UINT16)(pColorCorrect->_31 & CAP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(GEN_SETTING_REG_57, (MMP_UINT16)(pColorCorrect->_32 & CAP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(GEN_SETTING_REG_58, (MMP_UINT16)(pColorCorrect->_33 & CAP_BIT_RGB_TO_YUV));

    HOST_WriteRegister(GEN_SETTING_REG_59, (MMP_UINT16)(pColorCorrect->DeltaR & CAP_BIT_RGB_TO_YUV_CONST));
    HOST_WriteRegister(GEN_SETTING_REG_60, (MMP_UINT16)(pColorCorrect->DeltaG & CAP_BIT_RGB_TO_YUV_CONST));
    HOST_WriteRegister(GEN_SETTING_REG_61, (MMP_UINT16)(pColorCorrect->DeltaB & CAP_BIT_RGB_TO_YUV_CONST));
}

//=============================================================================
/**
* Audio/Video/Mute Counter control function
*/
//=============================================================================
void
AVSync_CounterCtrl(AV_SYNC_COUNTER_CTRL mode, MMP_UINT16 divider)
{
    if (mode & AUDIO_COUNTER_SEL)
    {
        HOST_WriteRegisterMask(AV_SYNC_SETTING_REG_130, divider, 0x007F);
    }
    else if (mode & VIDEO_COUNTER_SEL)
    {
          HOST_WriteRegister(AV_SYNC_SETTING_REG_131, (divider & 0x7F));
    }
    else if (mode & MUTE_COUNTER_SEL)
    {
          HOST_WriteRegister(AV_SYNC_SETTING_REG_132, divider & 0x7F);
    }

    if (mode & I2S_SOURCE_SEL)
          HOST_WriteRegisterMask(AV_SYNC_SETTING_REG_130, 0, 0x8000);

    if (mode & SPDIF_SOURCE_SEL)
          HOST_WriteRegisterMask(AV_SYNC_SETTING_REG_130, 0x8000, 0x8000);
}

void
AVSync_CounterReset(AV_SYNC_COUNTER_CTRL mode)
{
    HOST_WriteRegisterMask(AV_SYNC_SETTING_REG_129, (mode & 0x001F), 0x001F);
}

MMP_UINT32
AVSync_CounterLatch(AV_SYNC_COUNTER_CTRL mode)
{
    HOST_WriteRegisterMask(AV_SYNC_SETTING_REG_129, (mode & 0x001F), 0x001F);
}

MMP_UINT32
AVSync_CounterRead(AV_SYNC_COUNTER_CTRL mode)
{
    MMP_UINT16 value0;
    MMP_UINT16 value1;

    if (mode == AUDIO_COUNTER_SEL)
    {
        HOST_ReadRegister(ENGINE_STATUS_REG_0, &value0);
        HOST_ReadRegister(ENGINE_STATUS_REG_1, &value1);
    }
    else if (mode == VIDEO_COUNTER_SEL)
    {
        HOST_ReadRegister(ENGINE_STATUS_REG_2, &value0);
        HOST_ReadRegister(ENGINE_STATUS_REG_3, &value1);
    }
    else if (mode == MUTE_COUNTER_SEL)
    {
        HOST_ReadRegister(ENGINE_STATUS_REG_5, &value0);
        HOST_ReadRegister(ENGINE_STATUS_REG_6, &value1);
    }

    return value1 << 16 | value0;
}

MMP_BOOL
AVSync_MuteDetect(void)
{
    MMP_UINT16 value;

    HOST_ReadRegister(ENGINE_STATUS_REG_4, &value);

    if (value)
          return MMP_TRUE;
    else
          return MMP_FALSE;
}

void
Cap_Set_Output_Driving_Strength_Reg(MMP_UINT16 driving)
{
    MMP_UINT16 value;

    value = (((driving & 0x3) << 12) |
             ((driving & 0x3) << 10) |
             ((driving & 0x3) << 8) |
             ((driving & 0x3) << 6) |
             ((0x3 & 0x3) << 4) );  //VDX

    HOST_WriteRegisterMask(GEN_SETTING_REG_34 , value, 0x3FF0);
}

void 
Cap_EnableClock(
    void)
{
    HOST_WriteRegisterMask(MMP_CAP_CLOCK_REG_62, 0xFFFF, MMP_CAP_EN_M17CLK);
    HOST_WriteRegisterMask(MMP_CAP_CLOCK_REG_64, 0xFFFF, MMP_CAP_EN_DIV_CAPCLK);
}

void 
Cap_DisableClock(
    void)
{
    HOST_WriteRegisterMask(MMP_CAP_CLOCK_REG_62, 0x0000, MMP_CAP_EN_M17CLK);
    HOST_WriteRegisterMask(MMP_CAP_CLOCK_REG_64, 0x0000, MMP_CAP_EN_DIV_CAPCLK);
}

void 
Cap_Reset(
    void)
{
	  HOST_WriteRegisterMask(MMP_CAP_CLOCK_REG_62, 0xFFFF, MMP_CAP_RESET);
	  MMP_Sleep(1);
	  HOST_WriteRegisterMask(MMP_CAP_CLOCK_REG_62, 0x0000, MMP_CAP_RESET);
	  MMP_Sleep(1);
}

void 
Cap_Reg_Reset(
    void)
{
	  HOST_WriteRegisterMask(MMP_CAP_CLOCK_REG_62, 0xFFFF, MMP_CAP_REG_RESET);
	  MMP_Sleep(1);
	  HOST_WriteRegisterMask(MMP_CAP_CLOCK_REG_62, 0x0000, MMP_CAP_REG_RESET);
	  MMP_Sleep(1);
}


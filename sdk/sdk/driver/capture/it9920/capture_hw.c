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
Cap_Fire(
	CAPTURE_DEV_ID DEV_ID)
{
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_128] , 0x0003);
}

void
Cap_UnFire(
	CAPTURE_DEV_ID DEV_ID)
{
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_128], 0x0000);
}

void
Cap_TurnOnClock_Reg(
	CAPTURE_DEV_ID DEV_ID,MMP_BOOL flag)
{
    MMP_UINT32 value;

    if (flag == MMP_TRUE && (value & 0x08) == 0x00)
        ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_01C], 0x08, 0x08);
    else if (flag == MMP_FALSE && (value & 0x08) == 0x08)
        ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_01C], 0x00, 0x08);
}

/* Input Related Reg Setting */
MMP_RESULT
Cap_Set_Input_Pin_Mux_Reg(
	CAPTURE_DEV_ID DEV_ID,CAP_INPUT_MUX_INFO *pininfo)
{
    MMP_RESULT result = MMP_TRUE;
    MMP_UINT32 data = 0, mask = 0;

	// Setting Y Pin mux
    data = pininfo->Y_Pin_Num[0];
    data |= (pininfo->Y_Pin_Num[1] << 8);
	data |= (pininfo->Y_Pin_Num[2] << 16);
	data |= (pininfo->Y_Pin_Num[3] << 24);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_02C] , data);

    data = pininfo->Y_Pin_Num[4];
    data |= (pininfo->Y_Pin_Num[5] << 8);
	data |= (pininfo->Y_Pin_Num[6] << 16);
	data |= (pininfo->Y_Pin_Num[7] << 24);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_030] , data);

    data = pininfo->Y_Pin_Num[8];
    data |= (pininfo->Y_Pin_Num[9]  << 8);
	data |= (pininfo->Y_Pin_Num[10] << 16);
	data |= (pininfo->Y_Pin_Num[11] << 24);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_034] , data);

	// Setting U Pin mux
    data = pininfo->U_Pin_Num[0];
    data |= (pininfo->U_Pin_Num[1] << 8);
	data |= (pininfo->U_Pin_Num[2] << 16);
    data |= (pininfo->U_Pin_Num[3] << 24);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_038] , data);

    data = pininfo->U_Pin_Num[4];
    data |= (pininfo->U_Pin_Num[5] << 8);
	data |= (pininfo->U_Pin_Num[6] << 16);
    data |= (pininfo->U_Pin_Num[7] << 24);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_03C] , data);

	data = pininfo->U_Pin_Num[8];
    data |= (pininfo->U_Pin_Num[9]  << 8);
	data |= (pininfo->U_Pin_Num[10] << 16);
    data |= (pininfo->U_Pin_Num[11] << 24);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_040] , data);

	// Setting V Pin mux
    data = pininfo->V_Pin_Num[0];
    data |= (pininfo->V_Pin_Num[1] << 8);
	data |= (pininfo->V_Pin_Num[2] << 16);
    data |= (pininfo->V_Pin_Num[3] << 24);	
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_044] , data);

	data = pininfo->V_Pin_Num[4];
    data |= (pininfo->V_Pin_Num[5] << 8);
	data |= (pininfo->V_Pin_Num[6] << 16);
    data |= (pininfo->V_Pin_Num[7] << 24);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_048] , data);

	data = pininfo->V_Pin_Num[8];
    data |= (pininfo->V_Pin_Num[9] << 8);
	data |= (pininfo->V_Pin_Num[10] << 16);
    data |= (pininfo->V_Pin_Num[11] << 24);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_04C] , data);

	//Set HS,VS,DE, Pin mux //Benson
	data = pininfo->HS_Pin_Num;
    data |= (pininfo->VS_Pin_Num << 8);
	data |= (pininfo->DE_Pin_Num << 16);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_028], data);

    //Set Input clk mode
    data = (MMP_UINT32)(pininfo->UCLKSrc & 0x07) |
		    ((MMP_UINT32)(pininfo->EnUCLK & 0x1) << 3)   |
		    ((MMP_UINT32)(pininfo->UCLKRatio & 0x0F) << 4) |
		    ((MMP_UINT32)(pininfo->UCLKDly & 0x0F) << 8)|
            ((MMP_UINT32)(pininfo->UCLKInv & 0x1) << 12)|
            ((MMP_UINT32)(pininfo->UCLKVDSel & 0x3F) << 16) |
			((MMP_UINT32)(pininfo->UCLKAutoDlyEn & 0x1) << 31);

    mask = (0x07) |
           (0x1 << 3) |
           (0xF << 4) |
           (0xF << 8) |
           (0x1 << 12)|
           (0x3F << 16)|
           (0x1 << 31);
	ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_01C], data, mask);

	#if 0
	//for 1080i and 720p FPGA.
	ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_01C], 0x4 <<4, 0xF<<4);
	ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_050], 0x0 <<28 , 0x1 <<28);
	#endif

	return result;
}


/* Output Related Reg Setting */
void
Cap_Set_Output_Pin_Mux_Reg(
	CAPTURE_DEV_ID DEV_ID,CAP_OUTPUT_PIN_SELECT *pininfo)
{

}

MMP_RESULT
Cap_Set_Output_Clk_Mode_Reg(
	CAPTURE_DEV_ID DEV_ID,MMP_UINT16 value)
{

}

void
Cap_Set_Color_Format_Reg (
	CAPTURE_DEV_ID DEV_ID,CAP_YUV_INFO *pYUVinfo)
{
    MMP_UINT32 Value;

    Value = (pYUVinfo->InputMode & 0x3) |
			((pYUVinfo->ColorOrder & 0x3) << 2)|
         	((pYUVinfo->ColorDepth & 0x3) << 4) |
             ((pYUVinfo->InputWidth & 0x3) << 6);

	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_014], Value);
}

MMP_RESULT
Cap_Set_IO_Mode_Reg(
	CAPTURE_DEV_ID DEV_ID,CAP_IO_MODE_INFO *io_config)
{
    // ask H.C
    MMP_RESULT result = MMP_TRUE;
    MMP_UINT32 data;

    //CAP IO FF
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_020], io_config->CAPIOFFEn_VD_00_31);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_024], (io_config->CAPIOFFEn_VD_35_32 & 0xF));	
    return result;
}

void
Cap_Set_Input_Data_Info_Reg(
	CAPTURE_DEV_ID DEV_ID,CAP_INPUT_INFO *pIninfo)
{
    MMP_UINT32 VDUHeight, data = 0,mask = 0;

    /* Set Interlace or Progressive */
    ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_000], (pIninfo->Interleave << 1), 0x00000002);

    /* Set Hsync & Vsync Porlarity */
	data  = (pIninfo->HSyncPol & 0x1);
	data |= ((pIninfo->VSyncPol & 0x1) << 1);
	
	/* Set VsyncSkip & HsyncSkip */
	data |= ((pIninfo->VSyncSkip & 0x7) << 8);
	data |= ((pIninfo->HSyncSkip & 0x3F) << 12);
	
	/* Set sample Vsync by Hsync */
	data |= ((pIninfo->HSnapV & 0x1) << 27);

	/* Set CheckHsync &  CheckVsync & CheckDE enable */
	data |= ((pIninfo->CheckHS & 0x1) << 28);
	data |= ((pIninfo->CheckVS & 0x1) << 29);
	data |= ((pIninfo->CheckDE & 0x1)  << 30);

	mask = (0x1)|
           (0x1 <<  1)|
           (0x7 <<  8)|
           (0x3F << 12)|
           (0x1 << 27)|
           (0x1 << 28)|
           (0x1 << 29)|
           (0x1 << 30);
	ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_050], data, mask);

	/* Set memory write threshould & NV12Format */ 
	data  = ((pIninfo->NV12Format & 0x1) << 5); 
	data |= ((pIninfo->WrMergeThresld & 0x3F) << 8);
	ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_0A8], data, ((0x003F << 8)|(0x1 << 5)));
	
    /* Set YUV pitch */
	data  = ((pIninfo->PitchY >> 3  & 0x3FF) << 3);
	data |= ((pIninfo->PitchUV >> 3 & 0x3FF) << 19);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0DC], data);

    /*  Set Active Region  Set CapWidth & Cap Height  */
	data  = (pIninfo->capwidth & 0x1FFF);
	data |= ((pIninfo->capheight & 0x1FFF) << 16);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_004], data);
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_008], (pIninfo->HNum1 & 0x1FFF));
	
	data  = (pIninfo->LineNum1 & 0xFFF);
	data |= ((pIninfo->LineNum2 & 0xFFF) << 16);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_00C], data);

	data =  (pIninfo->LineNum3 & 0xFFF);
	data |=  ((pIninfo->LineNum4 & 0xFFF) << 16);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_010], data);

    /* Set ROI */ /* The source frame start X position and start Y position */
	data =  (pIninfo->ROIPosX & 0x1FFF);
	data |= ((pIninfo->ROIPosY & 0x1FFF) << 16);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_068], data);
	
    /* The width size and height size of ROI image */
	data =  (pIninfo->ROIWidth & 0x1FFF);
	data |= ((pIninfo->ROIHeight & 0x1FFF) << 16);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_06C], data);

}

void
Cap_Set_HorScale_Width_Reg(
	CAPTURE_DEV_ID DEV_ID,CAP_OUTPUT_INFO* pOutInfo)
{
    ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_08C], (MMP_UINT32)(pOutInfo->OutWidth), 0x1FFF);
}

/* Frame rate control */
void Cap_Set_Skip_Pattern_Reg (
	CAPTURE_DEV_ID DEV_ID,MMP_UINT16 pattern, MMP_UINT16 period)
{
	MMP_UINT32 data = 0;
	data = (pattern & 0xFFFF);
	data |=((period & 0xF) << 16); 
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_058], data);
}

Cap_Set_Hsync_Polarity(
	CAPTURE_DEV_ID DEV_ID,MMP_UINT16 Hsync)
{
	ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_050], Hsync, CAP_MSK_PHS);
}

Cap_Set_Vsync_Polarity(
	CAPTURE_DEV_ID DEV_ID,MMP_UINT16 Vsync)
{
	ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_050], (Vsync << 1), CAP_MSK_VHS);
}

MMP_BOOL
IsCapFire(
	CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 data;
    data = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_128]);

    if ((data & 0x0003) == 0x0003)
        return MMP_TRUE;
    else
        return MMP_FALSE;
}

MMP_RESULT
Cap_WaitEngineIdle(
	CAPTURE_DEV_ID DEV_ID)
{
    MMP_RESULT  result = MMP_SUCCESS;
    MMP_UINT32  status = 0;
    MMP_UINT32  timeOut = 0;

	//change to look the engine busy bit.
    status = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_200]);
    while(!(status & 0x1000))
    {
        usleep(1000);
        if( ++timeOut > 2000 )
        {

            cap_msg_ex(CAP_MSG_TYPE_ERR, "Capture still busy !!!!!\n");
            result = MMP_RESULT_ERROR;
            goto end;
        }
        status = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_200]);
    }

end:
    if( result )
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) ERROR !!!!!\n",  __FUNCTION__, __LINE__);

    return (MMP_RESULT)result;

}

/* Get Capture Lane error status */
MMP_UINT32
Cap_Get_Lane_status(
	CAPTURE_DEV_ID DEV_ID,CAP_LANE_STATUS lanenum)
{
    MMP_UINT32 data = 0;

    switch (lanenum)
    {
    case CAP_LANE0_STATUS:
    	data = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_200]);
    break;
    case CAP_LANE1_STATUS:
		data = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_204]);
    break;
    case CAP_LANE2_STATUS:
    	data = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_208]);
    break;
    case CAP_LANE3_STATUS:
    	data = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_20C]);
    break;
    default:
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) ERROR\n", __FUNCTION__, __LINE__);
    }

    return (MMP_UINT32)data;
}

MMP_UINT32
Cap_Get_Hsync_Polarity(
	CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 data;
	data = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_050]);
    data = ((data & 0x01) >> 0);
    return data;
}

MMP_UINT32
Cap_Get_Vsync_Polarity(
	CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 data;
    data = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_050]);
    data = ((data & 0x02 ) >> 1);
    return data;
}

MMP_UINT32
Cap_Get_Hsync_Polarity_Status(
	CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 data; //change to check Hsync Stable
	data = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_200]);
    data &= 0x01;
    cap_msg_ex(CAP_MSG_TYPE_ERR, "%s data = 0x%x\n", data);
    return data;
}

MMP_UINT32
Cap_Get_Vsync_Polarity_Status(
	CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 data; //change to check Vsync Stable.
    data = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_200]);
    data &= 0x02;
    cap_msg_ex(CAP_MSG_TYPE_ERR, "%s data = 0x%x\n", data);
    return data;
}

MMP_UINT32
Cap_Get_Revision(
	CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 data;
    data = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_018]);
    data &= 0x001F;
    cap_msg_ex(CAP_MSG_TYPE_ERR, "%s data = 0x%x\n", data);
    return data;
}

void
Cap_Engine_Reset(void)
{
	printf("Cap_Engine_Reset\n");
    ithWriteRegMaskA(0x0060, 0x40000000, 0x40000000);
    usleep(1000);
    ithWriteRegMaskA(0x0060, 0x00000000, 0x40000000);
    usleep(1000);
}

void
Cap_Engine_Register_Reset(void)
{
	printf("Cap_Engine_Register_Reset\n");
    ithWriteRegMaskA(0x0060, 0x80000000, 0x80000000);
    usleep(1000);
    ithWriteRegMaskA(0x0060, 0x00000000, 0x80000000);
    usleep(1000);
}

void
Cap_Set_Enable_Interrupt(CAPTURE_DEV_ID DEV_ID,MMP_BOOL flag)
{
    //9920 don`t need this, just choise the interrup mode that will enable it.
}

void
Cap_Set_Interrupt_Mode(CAPTURE_DEV_ID DEV_ID,MMP_UINT16 Intr_Mode,MMP_BOOL flag)
{
    MMP_UINT32 data = 0,i=0;

	if (flag == MMP_TRUE)
	{
	    if(Intr_Mode & CAP_INT_MODE_FRAME_END )
	    	data |= 0x1;
		if(Intr_Mode & CAP_INT_MODE_SYNC_ERR)
			data |= (0x1 << 1);
		if(Intr_Mode & CAP_INT_MODE_DCLK_PHASE_DRIFTED)
			data |= (0x1 << 2);
		if(Intr_Mode & CAP_INT_MODE_MUTE_DETECT)
			data |= (0x1 << 3);

    	ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_0E0], data, 0x0000000F);
	}
}

void
Cap_Set_Color_Bar(CAPTURE_DEV_ID DEV_ID,CAP_COLOR_BAR_CONFIG color_info)
{
    MMP_UINT32 data = 0;

    if (color_info.Enable_colorbar)
        data |= B_CAP_COLOR_BAR_ROLLING_EN;

    data |= (color_info.VS_act_start_line & 0x0FFF);
	data |= ((color_info.VS_act_line & 0x0FFF) << 16);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0E4], data);

	data = (color_info.blank_line1 & 0x0FFF);
	data |= ((color_info.act_line & 0x0FFF) << 16);
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0E8], data);

	data = (color_info.blank_line2 & 0x0FFF);
	data |= ((color_info.Hs_act & 0x0FFF) << 16);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0EC], data);

	data = (color_info.blank_pix1 &0x0FFF);
	data |= ((color_info.act_pix & 0x0FFF) << 16);
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0F0], data);

	data = (color_info.blank_pix2 & 0x0FFF);
	
    if (color_info.Hsync_pol)
        data |= B_CAP_COLOR_BAR_HSPOL;

    if (color_info.Vsync_pol)
        data |= B_CAP_COLOR_BAR_VSPOL;
	
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0F4],data);
}

MMP_UINT32
Cap_Get_MRawVTotal(CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 data;
    data = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_20C]);
    return ((data & 0xFFFF0000) >> 16);
}

MMP_UINT32
Cap_Get_Detectd_Hsync_Polarity(
	CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 data;
	data = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_050]);
    data = ((data & 0x00000040) >> 6);
    printf("detected Hsync Polarity = 0x%x\n", data);
    return data;
}

MMP_UINT32
Cap_Get_Detectd_Vsync_Polarity(
	CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 data;
	data = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_050]);
	data = ((data & 0x00000080) >> 7);
    printf("detected Vsync Polarity = 0x%x\n", data);
    return data;
}

MMP_RESULT
Cap_Get_Detected_Region(CAPTURE_DEV_ID DEV_ID)
{
    MMP_RESULT result = MMP_TRUE;
    MMP_UINT32 data;

	data = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_200]);
	printf("Capture status =0x%x\n", data);

    data = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_204]);
    printf("HTotal = 0x%x ,VTotal = 0x%x\n", data &0x00000FFF,(data & 0x1FFF0000)>>16);

	data = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_208]);
    printf("Width = 0x%x ,Height = 0x%x\n", data &0x00000FFF,(data & 0x1FFF0000)>>16);

    data = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_20C]);
    printf("MRawHTotal = 0x%x,MRawVTotal = 0x%x\n", data &0x0000FFFF, (data &0xFFFF0000)>>16);

    return result;
}

MMP_UINT32
Cap_Get_Detected_Width(CAPTURE_DEV_ID DEV_ID)
{
	MMP_RESULT result = MMP_TRUE;
	MMP_UINT32 data ,InputBitWidth;

	data = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_208]);
	InputBitWidth = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_014]);
	InputBitWidth = (InputBitWidth & 0xC0) >> 6;

	if(InputBitWidth)
		data /= InputBitWidth;
	
	return ((data & 0x0FFF));
}

MMP_UINT32
Cap_Get_Detected_Height(CAPTURE_DEV_ID DEV_ID)
{
	MMP_RESULT result = MMP_TRUE;
	MMP_UINT32 data;

	data = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_208]);
	return ((data & 0x1FFF0000) >> 16);
}

MMP_UINT32
Cap_Get_Detected_Interleave(CAPTURE_DEV_ID DEV_ID)
{
	MMP_RESULT result = MMP_TRUE;
	MMP_UINT32 data;

	data = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_200]);
	data = (data & 0x08000000) >> 27;
	return data;
}

void
Cap_Set_Interleave(CAPTURE_DEV_ID DEV_ID,MMP_UINT32 Interleave)
{
	MMP_RESULT result = MMP_TRUE;
	MMP_UINT32 data;

	/*	Set Interleave or Progressive   */
	ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_000], (Interleave << 1), 0x0002);
}

void
Cap_Set_Width_Height(CAPTURE_DEV_ID DEV_ID,MMP_UINT32 width,MMP_UINT32 height)
{
	MMP_RESULT result = MMP_TRUE;
	MMP_UINT32 data = 0;

	/*	Set Active Region  Set CapWidth & Cap Height   */
	data  = (width & 0x1FFF);
	data |= ((height & 0x1FFF) << 16);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_004], data);

	/* Set YUV pitch */
	data  = ((width >> 3 & 0x3FF) << 3);
	data |= ((width >> 3 & 0x3FF) << 19);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0DC], data);
	
	 /* The width & height size of ROI image */
	data =  (width & 0x1FFF);
	data |= ((height & 0x1FFF) << 16);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_06C], data);
   
	/*Set HorScale width */
	ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_08C], (MMP_UINT32)(width), 0x1FFF);
}

void
Cap_Set_ROI_Width_Height(CAPTURE_DEV_ID DEV_ID,MMP_UINT32 width,MMP_UINT32 height)
{	
	/* The width & height size of ROI image */
	MMP_UINT32 data = 0;
	
	data =  (width & 0x1FFF);
	data |= ((height & 0x1FFF) << 16);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_06C], data);
}
	
MMP_UINT16
Cap_Get_Sync_Status(CAPTURE_DEV_ID DEV_ID)
{
  
}

MMP_UINT32
Cap_Get_Error_Status(CAPTURE_DEV_ID DEV_ID)
{
	MMP_UINT32 data = 0;
	data =    ithReadRegA(CapRegTable[DEV_ID][CAP_REG_200]);
	if (data & 0xF00)
	{
		printf("Capture Get Error!! , data=0x%x\n", data);
		return (data & 0xF00) >>8;
	}else
		return 0;
  
}

void Cap_Dump_Reg(CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 i, data;

    for(i=0 ; i<= 544; i+=4)
    {
        data  = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_000]+i);
        printf("reg=0x%x ,val=0x%x\n",CapRegTable[DEV_ID][CAP_REG_000]+i , data);

    }
	printf("\n");
}

void 
Cap_Clean_Intr(CAPTURE_DEV_ID DEV_ID)
{
	ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_0E0],(0x1<<4),(0x1<<4));
    //ithPrintf("clear cap %d int\n", DEV_ID);
}

void
Cap_Set_EnableFrameRate_Reg(CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT16 Value;
}

void
Cap_Set_Enable_Reg(CAPTURE_DEV_ID DEV_ID,CAP_ENFUN_INFO* pFunEn)
{
    MMP_UINT32 Value, data = 0;

	//Enable Hsync &  BT601 or BT656
	data  = (pFunEn->EnInBT656);
	data |= ((pFunEn->EnHSync & 0x1) << 2);
	
	 // Data Enable mode in BT601
	data |=	((pFunEn->EnDEMode & 0x1 ) << 3);
	ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_000], data, 0xD);

    // Enable CS fun or CC fun
    ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_070], ((pFunEn->EnCSFun | pFunEn->EnCCFun) << 24), (0x1 << 24));

	//AutoDected Hsync & AutoDected Vsync or not
	data  = ((pFunEn->EnAutoDetHSPol & 0x1) << 6);
	data |= ((pFunEn->EnAutoDetVSPol & 0x1) << 7);
	data |= ((pFunEn->EnDumpMode & 0x1) << 31);
	ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_050], data, (CAP_MSK_HSPOL_DET | CAP_MSK_VSPOL_DET | CAP_MSK_DUMPMODE));
	
	data  = ((pFunEn->EnMemContinousDump & 0x1) << 7);
	data |= ((pFunEn->EnSramNap & 0x1) << 8);
	ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_054], data, ((0x1 << 7)|(0x1 << 8)));

	data  = ((pFunEn->EnPort1UV2LineDS & 0x1) << 6);
	data |= ((pFunEn->EnMemLimit & 0x1) << 31);
	ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_0A8], data , ((0x1 << 6)|(0x1 << 31)));
}

void
Cap_Set_Enable_Dither(
	CAPTURE_DEV_ID DEV_ID,CAP_INPUT_DITHER_INFO *pDitherinfo)
{
	MMP_UINT32 data = 0;

	data  = (pDitherinfo->DitherMode & 0x3);
	data |= ((pDitherinfo->EnDither & 0x1) << 3);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_064], data);
}

MMP_RESULT
Cap_Set_ISP_HandShaking(CAPTURE_DEV_ID DEV_ID,CAP_ISP_HANDSHAKING_MODE mode, CAP_OUTPUT_INFO* pOutInfo)
{
    MMP_RESULT result;
    MMP_UINT32 data = 0;

    if (mode == ONFLY_MODE)//onfly mode
    {
        data |= B_CAP_ONFLY_MODE;
        ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_054],data);
		
        data = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_054]);
        return (data & B_CAP_ONFLY_MODE)? MMP_TRUE : MMP_FALSE;
    }
    else if (mode == MEMORY_MODE)//memory mode
    {
        data |= B_CAP_MEM_MODE;
		data |= ((pOutInfo->OutMemFormat & 0x3) << 4);
        ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_054], data);

        // Set PreLoadNum
        data = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_054]);
        return (data & B_CAP_MEM_MODE)? MMP_FALSE : MMP_TRUE;
    }
    else
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, "ISP HandShaking error\n");
        return MMP_FALSE;
    }
}

MMP_RESULT
Cap_Set_Error_Handleing(CAPTURE_DEV_ID DEV_ID,MMP_UINT32 errDetectEn)
{
    MMP_RESULT result = MMP_TRUE;

    ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_05C], errDetectEn,0x0000FFFF);
    return result;
}

MMP_RESULT
Cap_Set_Wait_Error_Reset(
CAPTURE_DEV_ID DEV_ID)
{
	MMP_RESULT result = MMP_TRUE;
	MMP_UINT32 data;
	data = 0xFFFF0000;// & (!B_CAP_ERR_RST_STOP_ENGINE);
	ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_05C], data, 0xFFFF0000);

	return result;
}

void
Cap_Set_Memory_AddressLimit_Reg(
	CAPTURE_DEV_ID DEV_ID,MMP_UINT32 memUpBound , MMP_UINT32 memLoBound)
{
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0AC], memUpBound);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0B0], memLoBound);
}

//FrameBase Reg
void
Cap_Set_FrameBase_0_Reg(CAPTURE_DEV_ID DEV_ID,MMP_UINT32 dst_Yaddr , MMP_UINT32 dst_UVaddr)
{
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0B4], dst_Yaddr);
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0B8], dst_UVaddr);
}

void
Cap_Set_FrameBase_1_Reg(CAPTURE_DEV_ID DEV_ID,MMP_UINT32 dst_Yaddr , MMP_UINT32 dst_UVaddr)
{
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0BC],  dst_Yaddr);
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0C0],  dst_UVaddr);
}

void
Cap_Set_FrameBase_2_Reg(CAPTURE_DEV_ID DEV_ID,MMP_UINT32 dst_Yaddr , MMP_UINT32 dst_UVaddr)
{
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0C4],  dst_Yaddr);
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0C8],  dst_UVaddr);
}

void
Cap_Set_FrameBase_3_Reg(CAPTURE_DEV_ID DEV_ID,MMP_UINT32 dst_Yaddr , MMP_UINT32 dst_UVaddr)
{
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0CC], dst_Yaddr);
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0D0], dst_UVaddr);
}

void
Cap_Set_FrameBase_4_Reg(CAPTURE_DEV_ID DEV_ID,MMP_UINT32 dst_Yaddr , MMP_UINT32 dst_UVaddr)
{
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0D4], dst_Yaddr);
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0D8], dst_UVaddr);
}

void
Cap_Set_Buffer_addr_Reg(
	CAPTURE_DEV_ID DEV_ID,
    MMP_UINT32 *pAddr,
    MMP_UINT32 addrOffset)
{
	uint32_t vram_addr = 0;

    //Y0
    vram_addr = ithSysAddr2VramAddr(pAddr[0]);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0B4], (vram_addr + addrOffset));
   
    //UV0
    vram_addr = ithSysAddr2VramAddr(pAddr[1]);
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0B8],  (vram_addr + addrOffset));

    //Y1
    vram_addr = ithSysAddr2VramAddr(pAddr[2]);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0BC], (vram_addr + addrOffset));
   
    //UV1
    vram_addr = ithSysAddr2VramAddr(pAddr[3]);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0C0],  (vram_addr + addrOffset));

    //Y2
    vram_addr = ithSysAddr2VramAddr(pAddr[4]);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0C4], (vram_addr + addrOffset));
   
    //UV2
    vram_addr = ithSysAddr2VramAddr(pAddr[5]);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0C8],  (vram_addr + addrOffset));

    // Set framebuf num , memory mode
    ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_0A8], (CAPTURE_MEM_BUF_COUNT >>1) -1, 0x0007);
}

void
CAP_SetScaleParam_Reg(
	CAPTURE_DEV_ID DEV_ID,
    const CAP_SCALE_CTRL    *pScaleFun)
{
    MMP_UINT32  Value = 0;
    MMP_UINT32  HCI;

    HCI = CAP_FLOATToFix(pScaleFun->HCI, 6, 14);

    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_090], (MMP_UINT32)(HCI & 0x001FFFFF));
}

void
Cap_SetIntScaleMatrixH_Reg(
	CAPTURE_DEV_ID DEV_ID,
    MMP_UINT32  WeightMatX[][CAP_SCALE_TAP])
{
    MMP_UINT32  Value;

    Value = (MMP_UINT32)((WeightMatX[0][0] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[0][1] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_H) |
    		((WeightMatX[0][2] & CAP_BIT_SCALEWEIGHT) << 16) |
            ((WeightMatX[0][3] & CAP_BIT_SCALEWEIGHT) << 24);
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_094], (MMP_UINT32)Value);

    Value = (MMP_UINT32)((WeightMatX[1][0] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[1][1] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_H)|
   			((WeightMatX[1][2] & CAP_BIT_SCALEWEIGHT) << 16) |
            ((WeightMatX[1][3] & CAP_BIT_SCALEWEIGHT) << 24);
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_098], (MMP_UINT32)Value);

    Value = (MMP_UINT32)((WeightMatX[2][0] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[2][1] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_H)|
    		((WeightMatX[2][2] & CAP_BIT_SCALEWEIGHT) << 16) |
            ((WeightMatX[2][3] & CAP_BIT_SCALEWEIGHT) << 24);
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_09C], (MMP_UINT32)Value);

    Value = (MMP_UINT32)((WeightMatX[3][0] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[3][1] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_H)|
    		((WeightMatX[3][2] & CAP_BIT_SCALEWEIGHT) << 16) |
            ((WeightMatX[3][3] & CAP_BIT_SCALEWEIGHT) << 24);
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0A0], (MMP_UINT32)Value);

    Value = (MMP_UINT32)((WeightMatX[4][0] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[4][1] & CAP_BIT_SCALEWEIGHT) << CAP_SHT_SCALEWEIGHT_H)|
    		((WeightMatX[4][2] & CAP_BIT_SCALEWEIGHT) << 16) |
            ((WeightMatX[4][3] & CAP_BIT_SCALEWEIGHT) << 24);
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_0A4], (MMP_UINT32)Value);
}

//=============================================================================
/**
* RGB to YUV transfer matrix.
*/
//=============================================================================
void
Cap_SetRGBtoYUVMatrix_Reg(
	CAPTURE_DEV_ID DEV_ID,
    const CAP_RGB_TO_YUV    *pRGBtoYUV)
{
	MMP_UINT32	data = 0;

	//CSOffset 1~3 all setting to zero, why?
	ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_070], 0x0 ,0x00FFFFFF);

	data = (MMP_UINT32)(pRGBtoYUV->_11 & CAP_BIT_RGB_TO_YUV);
	data |=(MMP_UINT32)((pRGBtoYUV->_12 & CAP_BIT_RGB_TO_YUV)<<16);
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_074], data);

	data = (MMP_UINT32)(pRGBtoYUV->_13 & CAP_BIT_RGB_TO_YUV);
	data |=(MMP_UINT32)((pRGBtoYUV->_21 & CAP_BIT_RGB_TO_YUV)<<16);
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_078], data);

	data = (MMP_UINT32)(pRGBtoYUV->_22 & CAP_BIT_RGB_TO_YUV);
	data |=(MMP_UINT32)((pRGBtoYUV->_23 & CAP_BIT_RGB_TO_YUV)<<16);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_07C], data);

	data = (MMP_UINT32)(pRGBtoYUV->_31 & CAP_BIT_RGB_TO_YUV);
	data |=(MMP_UINT32)((pRGBtoYUV->_32 & CAP_BIT_RGB_TO_YUV)<<16);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_080], data);
	
    data = (MMP_UINT32)(pRGBtoYUV->_33 & CAP_BIT_RGB_TO_YUV);
    data |=(MMP_UINT32)((pRGBtoYUV->ConstY & CAP_BIT_RGB_TO_YUV_CONST)<<16);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_084], data);
	
    data = (MMP_UINT32)(pRGBtoYUV->ConstU & CAP_BIT_RGB_TO_YUV_CONST);
    data |=(MMP_UINT32)((pRGBtoYUV->ConstV & CAP_BIT_RGB_TO_YUV_CONST)<<16);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_088], data);
}

//=============================================================================
/**
* Set color correction matrix and constant.
*/
//=============================================================================
void
Cap_SetCCMatrix_Reg(
	CAPTURE_DEV_ID DEV_ID,
    const CAP_COLOR_CORRECTION    *pColorCorrect)
{
    MMP_UINT32 value = 0;

    value = pColorCorrect->OffsetR & 0xFF;
	value |= ((pColorCorrect->OffsetG & 0xFF) << 8);
	value |= ((pColorCorrect->OffsetB & 0xFF) << 16);
    ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_070], value, 0x00FFFFFF);

	value = (MMP_UINT32)(pColorCorrect->_11  & CAP_BIT_RGB_TO_YUV);
	value |=(MMP_UINT32)((pColorCorrect->_12 & CAP_BIT_RGB_TO_YUV)<<16);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_074], value);

	value = (MMP_UINT32)(pColorCorrect->_13 & CAP_BIT_RGB_TO_YUV);
	value |=(MMP_UINT32)((pColorCorrect->_21 & CAP_BIT_RGB_TO_YUV)<<16);
    ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_078], value);

	value = (MMP_UINT32)(pColorCorrect->_22 & CAP_BIT_RGB_TO_YUV);
	value |=(MMP_UINT32)((pColorCorrect->_23 & CAP_BIT_RGB_TO_YUV)<<16);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_07C], value);

	value = (MMP_UINT32)(pColorCorrect->_31 & CAP_BIT_RGB_TO_YUV);
	value |=(MMP_UINT32)((pColorCorrect->_32 & CAP_BIT_RGB_TO_YUV)<<16);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_080], value);

	value = (MMP_UINT32)(pColorCorrect->_33 & CAP_BIT_RGB_TO_YUV);
    value |=(MMP_UINT32)((pColorCorrect->DeltaR & CAP_BIT_RGB_TO_YUV_CONST)<<16);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_084], value);

	value = (MMP_UINT32)(pColorCorrect->DeltaG & CAP_BIT_RGB_TO_YUV_CONST);
    value |=(MMP_UINT32)((pColorCorrect->DeltaB & CAP_BIT_RGB_TO_YUV_CONST)<<16);
	ithWriteRegA(CapRegTable[DEV_ID][CAP_REG_088], value);
}

//=============================================================================
/**
* Audio/Video/Mute Counter control function
*/
//=============================================================================
void
AVSync_CounterCtrl(CAPTURE_DEV_ID DEV_ID,AV_SYNC_COUNTER_CTRL mode, MMP_UINT16 divider)
{
    if (mode & AUDIO_COUNTER_SEL)
    {
        ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_130], divider, 0x000007FF);
    }
    else if (mode & VIDEO_COUNTER_SEL)
    {
     	ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_130], ((divider & 0x000007FF)<<16), 0x000007FF <<16);
    }
    else if (mode & MUTE_COUNTER_SEL)
    {
        ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_12C],((divider & 0x000007FF)<<16), 0x000007FF <<16);
    }
}

void
AVSync_CounterReset(CAPTURE_DEV_ID DEV_ID,AV_SYNC_COUNTER_CTRL mode)
{
    ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_12C], (mode & 0x001F), 0x001F);
}

MMP_UINT32
AVSync_CounterLatch(CAPTURE_DEV_ID DEV_ID,AV_SYNC_COUNTER_CTRL mode)
{
    ithWriteRegMaskA(CapRegTable[DEV_ID][CAP_REG_12C], (mode & 0x001F), 0x001F);
}

MMP_UINT32
AVSync_CounterRead(CAPTURE_DEV_ID DEV_ID,AV_SYNC_COUNTER_CTRL mode)
{
    MMP_UINT32 value = 0;

    if (mode == AUDIO_COUNTER_SEL)
    {
        value = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_210]);
    }
    else if (mode == VIDEO_COUNTER_SEL)
    {
		value = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_214]);
    }
    else if (mode == MUTE_COUNTER_SEL)
    {
    	value = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_218]);
    }
	else if(mode == MUTEPRE_COUNTER_SEL)
    {
		value = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_21C]);
    }

    return value;
}

MMP_BOOL
AVSync_MuteDetect(CAPTURE_DEV_ID DEV_ID)
{
    MMP_UINT32 value;

    value = ithReadRegA(CapRegTable[DEV_ID][CAP_REG_200]);

    if (value & 0x01000000)
          return MMP_TRUE;
    else
          return MMP_FALSE;
}

void
Cap_Set_Output_Driving_Strength_Reg(CAPTURE_DEV_ID DEV_ID,MMP_UINT16 driving)
{
	//9920 seems like no use this.
}

void 
Cap_EnableClock(
    void)
{
    ithWriteRegMaskA(MMP_CAP_CLOCK_REG_60, MMP_CAP_EN_M12CLK, MMP_CAP_EN_M12CLK);
    ithWriteRegMaskA(MMP_CAP_CLOCK_REG_60, MMP_CAP_EN_DIV_CAPCLK, MMP_CAP_EN_DIV_CAPCLK);
}

void 
Cap_DisableClock(
    void)
{
    ithWriteRegMaskA(MMP_CAP_CLOCK_REG_60, 0x0000, MMP_CAP_EN_M12CLK);
    ithWriteRegMaskA(MMP_CAP_CLOCK_REG_60, 0x0000, MMP_CAP_EN_DIV_CAPCLK);
}

void 
Cap_Reset(
    void)
{
	  ithWriteRegMaskA(MMP_CAP_CLOCK_REG_60, MMP_CAP_RESET, MMP_CAP_RESET);
	  sleep(1);
	  ithWriteRegMaskA(MMP_CAP_CLOCK_REG_60, 0x0000, MMP_CAP_RESET);
	  sleep(1);
}

void 
Cap_Reg_Reset(
    void)
{
	  ithWriteRegMaskA(MMP_CAP_CLOCK_REG_60, MMP_CAP_REG_RESET, MMP_CAP_REG_RESET);
	  sleep(1);
	  ithWriteRegMaskA(MMP_CAP_CLOCK_REG_60, 0x0000, MMP_CAP_REG_RESET);
	  sleep(1);
}


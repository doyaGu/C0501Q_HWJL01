//#include "host/host.h"
//#include "pal/pal.h"
//#include "sys/sys.h"
//#include "mmp_types.h"

#include "vp/vp_types.h"
#include "vp/vp_config.h"
#include "vp/vp_hw.h"
#include "vp/vp.h"
#include "vp/vp_util.h"

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

/* 1080P
Input   Output  Ratio(In/Out)
1920	720	    2.666666667
1080	480	    2.25
1080	576	    1.875
1280	720	    1.777777778
1920	1280	1.5
720	    576	    1.25
576     480	    1.2
1920    1920    1

480	    576	    0.833333333
576	    720	    0.8
1280    1920    0.666666667
720	    1280	0.5625
576     1080    0.533333333
480     1080    0.444444444
720     1920    0.375
*/

/* 720P
Input   Output  Ratio(In/Out)
768     320     2.4
768	    352	    2.181818182
480     240     2
1280	720	    1.777777778
1280	768	    1.666666667
720  	480 	1.5
768     720     1.066666667
1280    1280    1

480	    576	    0.833333333
576	    720	    0.8
1280    1920    0.666666667
720	    1280	0.5625
576     1080    0.533333333
480     1080    0.444444444
720     1920    0.375
*/

#define     WEIGHT_NUM 15

#if defined (RESOLUTION_1080P)
MMP_FLOAT   VP_ScaleRatio[WEIGHT_NUM] = {(1920.0f/720), (1080.0f/480), (1080.0f/576), (1280.0f/720), (1920.0f/1280), (720.0f/576), (576.0f/480), (1920.0f/1920),
                                      (480.0f/576), (576.0f/720), (1280.0f/1920), (720.0f/1280), (576.0f/1080), (480.0f/1080), (720.0f/1920)};
#elif defined (RESOLUTION_720P)
MMP_FLOAT   VP_ScaleRatio[WEIGHT_NUM] = {(768.0f/320), (768.0f/352), (480.0f/240), (1280.0f/720), (1280.0f/768), (720.0f/480), (768.0f/720), (1280.0f/1280),
                                      (480.0f/576), (576.0f/720), (1280.0f/1920), (720.0f/1280), (576.0f/1080), (480.0f/1080), (720.0f/1920)};
#else //TODO 720x480
MMP_FLOAT   VP_ScaleRatio[WEIGHT_NUM] = {(768.0f/320), (768.0f/352), (480.0f/240), (1280.0f/720), (1280.0f/768), (720.0f/480), (768.0f/720), (1280.0f/1280),
                                      (480.0f/576), (576.0f/720), (1280.0f/1920), (720.0f/1280), (576.0f/1080), (480.0f/1080), (720.0f/1920)};                                      
#endif


MMP_UINT8   WeightMatInt[WEIGHT_NUM][VP_SCALE_TAP_SIZE][VP_SCALE_TAP];
MMP_UINT8   MotionWeightMatInt_H[VP_SCALE_TAP_SIZE][VP_SCALE_TAP];
MMP_UINT8   MotionWeightMatInt_V[VP_SCALE_TAP_SIZE][VP_SCALE_TAP];

MMP_UINT8   WeightMatInt_2TAP[WEIGHT_NUM][VP_SCALE_TAP_SIZE][VP_SCALE_TAP];

//=============================================================================
//                Private Function Definition
//=============================================================================
//=============================================================================
/**
* Calculate Scale Factor
*/
//=============================================================================
static MMP_FLOAT
_VP_ScaleFactor(
    MMP_UINT16  Input,
    MMP_UINT16  Output)
{
    return (MMP_FLOAT) (((MMP_INT) (16384.0f*Input/(MMP_FLOAT)Output))/16384.0f);
}

//=============================================================================
/**
* Calculate ISP Deinterlace.
*/
//=============================================================================
static void
_VP_Deinter_Param(
    VP_DEINTERLACE_CTRL    *pDeInterlace)
{
    pDeInterlace->DeinterMode = DEINTER3D;
    pDeInterlace->EnableOutMotion = MMP_FALSE;
    pDeInterlace->Disable30MotionDetect = MMP_FALSE;

    pDeInterlace->EnSrcBottomFieldFirst = MMP_FALSE;
    pDeInterlace->EnDeinterBottomField = MMP_TRUE;

    pDeInterlace->EnChromaEdgeDetect = MMP_TRUE;
    pDeInterlace->EnLummaEdgeDetect = MMP_TRUE;
    pDeInterlace->EnSrcLPF = MMP_TRUE;

    pDeInterlace->UVRepeatMode = MMP_FALSE;

    pDeInterlace->EnLowLevelEdge = MMP_FALSE;
    pDeInterlace->LowLevelMode = 0;
    pDeInterlace->EnLowLevelOutside = MMP_FALSE;
    pDeInterlace->LowLevelBypassBlend = 0;

    pDeInterlace->LowLevelPosX = 0;
    pDeInterlace->LowLevelPosY = 425;
    pDeInterlace->LowLevelWidth = 704;
    pDeInterlace->LowLevelHeight = 55;

    if (pDeInterlace->DeinterMode == DEINTER3D)
    	pDeInterlace->EnUV2DMethod = MMP_TRUE;
    else
    	pDeInterlace->EnUV2DMethod = MMP_FALSE;
}

static void
_VP_Deinter3D_Param(
    VP_DEINTERLACE_CTRL    *pDeInterlace)
{
    MMP_UINT16      MDThreshold_High;
    MMP_UINT16      MDThreshold_Low;

    MDThreshold_Low = 8;
    MDThreshold_High = 16;

    pDeInterlace->MDThreshold_Low = MDThreshold_Low;
    pDeInterlace->MDThreshold_High = MDThreshold_High;
    pDeInterlace->MDThreshold_Step = (MMP_INT)((MMP_FLOAT)128.0f * 1.0f / (MDThreshold_High - MDThreshold_Low));

    pDeInterlace->EnLPFWeight = MMP_TRUE;
    pDeInterlace->EnLPFWeight = MMP_TRUE;
    pDeInterlace->EnLPFStaticPixel = MMP_TRUE;

    pDeInterlace->DisableMV_A = MMP_FALSE;
    pDeInterlace->DisableMV_B = MMP_FALSE;
    pDeInterlace->DisableMV_C = MMP_FALSE;
    pDeInterlace->DisableMV_D = MMP_FALSE;
    pDeInterlace->DisableMV_E = MMP_FALSE;
    pDeInterlace->DisableMV_F = MMP_FALSE;
    pDeInterlace->DisableMV_G = MMP_FALSE;
}

static void
_VP_Deinter2D_Param(
    VP_DEINTERLACE_CTRL    *pDeInterlace)
{
    MMP_UINT16      EdgeBlendWeight;
    MMP_UINT16      OrgBlendWeight;

    EdgeBlendWeight = 8;
    OrgBlendWeight = 8;

    pDeInterlace->D2EdgeBlendWeight = (MMP_INT)((MMP_FLOAT)64.0f * EdgeBlendWeight / (EdgeBlendWeight + OrgBlendWeight));
    pDeInterlace->D2OrgBlendWeight = (MMP_INT)((MMP_FLOAT)64.0f * OrgBlendWeight / (EdgeBlendWeight + OrgBlendWeight));
}

//=============================================================================
/**
* Calculate ISP Scaling Factor.
*/
//=============================================================================
static void
_VP_CalScaleHCI_VCI(
    void)
{
    MMP_FLOAT   HCI;
    MMP_FLOAT   VCI;

    HCI = _VP_ScaleFactor(VPctxt->InInfo.SrcWidth, VPctxt->OutInfo.Width);
    VCI = _VP_ScaleFactor(VPctxt->InInfo.SrcHeight, VPctxt->OutInfo.Height);

    if(HCI != VPctxt->ScaleFun.HCI)
        VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_ScaleMatrixH;

    if(VCI != VPctxt->ScaleFun.VCI)
        VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_ScaleMatrixV;

    VPctxt->ScaleFun.HCI = HCI;
    VPctxt->ScaleFun.VCI = VCI;
}

//=============================================================================
/**
 * Create weighting for the matrix of scaling.
*/
//=============================================================================
static void
_VP_CreateWeighting(
    MMP_FLOAT   scale,
    MMP_UINT8   taps,
    MMP_UINT8   tapSize,
    //MMP_FLOAT   weightMatrix[][VP_SCALE_TAP])
    MMP_FLOAT   **weightMatrix)
{
    MMP_UINT8   i, j;
    MMP_FLOAT   WW;
    MMP_FLOAT   W[32];
    MMP_INT16   point;
    MMP_UINT8   adjust;
    MMP_INT16   sharp;
    MMP_UINT8   method;
    MMP_FLOAT   precision = 64.0f;
    MMP_FLOAT   fscale = 1.0f;

    //Kevin:TODO temp solution
    adjust = 0;
    method = 11;
    sharp = 0;

    switch(sharp)
    {
        case 4:     fscale = 0.6f;  break;
        case 3:     fscale = 0.7f;  break;
        case 2:     fscale = 0.8f;  break;
        case 1:     fscale = 0.9f;  break;
        case 0:     fscale = 1.0f;  break;
        case -1:    fscale = 1.1f;  break;
        case -2:    fscale = 1.2f;  break;
        case -3:    fscale = 1.3f;  break;
        case -4:    fscale = 1.4f;  break;
        default:    fscale = 1.5f;  break;
    };

    if (adjust == 0)
    {
        if (scale < 1.0f)
        {
            scale = fscale;
        }
        else
        {
            scale *= fscale;
        }
    }
    else if (adjust == 1)
    {
        //Last update (2002/04/24) by WKLIN]
        // For Low Pass
        if (scale < 1.0f)
        {
            scale = 1.2f;
        }
        else if (scale > 1.0f)
        {
            scale *= 1.1f;
        }
    }
    else if (adjust == 2)
    {
        //Last update (2003/08/17) by WKLIN in Taipei
        //For including more high frequency details
        if (scale < 1.0f)
        {
            scale = 0.9f;
        }
        else if (scale > 1.0f)
        {
            scale *= 0.9f;
        }
    }
    else if (adjust == 3)
    {
        //Last update (2003/09/10) by WKLIN in Hsin-Chu
        //For excluding more high frequency details
        if (scale < 1.0f)
        {
            scale = 1.2f;
        }
        else if (scale >= 1.0f)
        {
            scale *= 1.3f;
        }
    }

    if (method == 10)
    {
        //Sinc
        for (i=0; i <= (tapSize>>1); i++)
        {
            WW = 0.0f;
            point = (MMP_INT)(taps>>1) - 1;
            for (j = 0; j < taps; j++)
            {
                W[j] = vp_sinc( ((MMP_FLOAT)point + i/((MMP_FLOAT)tapSize)) / (MMP_FLOAT)scale);
                point--;
                WW += W[j];
            }

            //for (j=0; j< taps; j++)
            //  WeightMat[i][j] = ((int)(W[j]/WW*precision + 0.5 ))/precision;

            //Changed: 2004/02/24
            weightMatrix[i][taps-1] = 1.0;
            for (j = 0; j < taps-1; j++)
            {
                weightMatrix[i][j] = ((MMP_INT)(W[j]/WW*precision + 0.5)) / precision;
                weightMatrix[i][taps-1] -= weightMatrix[i][j];
            }
        }

        for (i = ((tapSize>>1)+1); i < tapSize; i++)
        {
            for (j = 0; j < taps; j++)
            {
                weightMatrix[i][j] = weightMatrix[tapSize-i][taps-1-j];
            }
        }
    }
    else if (method == 11)
    {
        //rcos
        for (i = 0; i <= (tapSize>>1); i++)
        {
            WW =0.0;
            point = (MMP_INT) (taps>>1)-1;
            for (j=0; j < taps; j++)
            {
                W[j] = rcos( ((MMP_FLOAT)point + i/((MMP_FLOAT)tapSize)) / (MMP_FLOAT)scale);
                point--;
                WW += W[j];
            }

            //for (j=0; j< taps; j++)
            //  weightMatrix[i][j] = ((int)(W[j]/WW*precision + 0.5 ))/precision;

            //Changed: 2004/02/24
            weightMatrix[i][taps-1] = 1.0;
            for (j = 0; j < taps-1; j++)
            {
                weightMatrix[i][j] = ((MMP_INT)(W[j]/WW*precision + 0.5)) / precision;
                weightMatrix[i][taps-1] -= weightMatrix[i][j];
            }

        }
        for (i = ((tapSize>>1)+1); i < tapSize; i++)
        {
            for (j = 0; j < taps; j++)
            {
                weightMatrix[i][j] = weightMatrix[tapSize-i][taps-1-j];
            }
        }
    }
    else if (method == 12)
    {
        // Catmull-Rom Cubic interpolation
        for (i = 0; i <= (tapSize>>1); i++)
        {
            WW = 0.0f;
            point = (MMP_INT)(taps>>1)-1;
            for (j = 0; j < taps; j++)
            {
                W[j] = cubic01( ((MMP_FLOAT)point + i/((MMP_FLOAT)tapSize)) / (MMP_FLOAT)scale);
                point--;
                WW += W[j];

                //printf("i:%2d   W=%6.3f   point=%2d    WW=%6.3f\n",
                //  i, W[j], point, WW);
            }


            //Changed: 2004/02/24
            weightMatrix[i][taps-1] = 1.0;
            for (j = 0; j < taps-1; j++)
            {
                weightMatrix[i][j] = ((MMP_INT)(W[j]/WW*precision + 0.5)) / precision;
                weightMatrix[i][taps-1] -= weightMatrix[i][j];
            }
        }
        for (i = ((tapSize>>1)+1); i < tapSize; i++)
        {
            for (j = 0; j < taps; j++)
            {
                weightMatrix[i][j] = weightMatrix[tapSize-i][taps-1-j];
            }
        }
    }
    else
    {
        VP_msg_ex(VP_MSG_TYPE_ERR, " %s() unknow error !\n", __FUNCTION__);
    }
}

//=============================================================================
/**
* Calculate ISP Scene Change Parameter.
*/
//=============================================================================
static void
_VP_SceneChange_Param(
    void)
{
    MMP_UINT16          SourceHeight = VPctxt->InInfo.SrcHeight;
    MMP_UINT16          SourceWidth = VPctxt->InInfo.SrcWidth;
	MMP_FLOAT           H_Step, V_Step;
    VP_SCENE_CHANGE    *pSceneChg = &VPctxt->SceneChg;
    MMP_UINT16          SceneChgMode = 0;

    pSceneChg->BufInitValue = 128;

    switch (SceneChgMode)
    {
        case 0: // total 16 x 16 regions, 16 horizontal region are vertically aligned
        	pSceneChg->V_Offset = (SourceHeight + 256) >> (8+1);
        	pSceneChg->H_Offset = (SourceWidth + 16) >> (4+1);
        	V_Step              = SourceHeight >> 8;
        	H_Step              = SourceWidth >> 4;
        	pSceneChg->Step_No  = 256;
        	break;

        case 1: // total 16 x 16 regions, 16 horizontal region are vertically aligned with right shift 1
        	pSceneChg->V_Offset = (SourceHeight + 256) >> 9;
        	pSceneChg->H_Offset = ((SourceWidth + 16) >> 5) - 8;
        	V_Step              = SourceHeight >> 8;
        	H_Step              = (SourceWidth + 1) >> 4;
        	pSceneChg->Step_No  = 256;
        	break;

        case 2: // total 16 x 16 regions, 16 horizontal region are vertically aligned with left shift 1
        	pSceneChg->V_Offset = (SourceHeight + 256) >> 9;
        	pSceneChg->H_Offset = ((SourceWidth + 16) >> 5) + 8;
        	V_Step              = SourceHeight >> 8;
        	H_Step              = (SourceWidth - 1) >> 4;
        	pSceneChg->Step_No  = 256;
        	break;

        case 3: // total 8 x 8 regions, 8 horizontal region are vertically aligned
        	pSceneChg->V_Offset = (SourceHeight + 64) >> 7;
        	pSceneChg->H_Offset = (SourceWidth + 8) >> 4;
        	V_Step              = SourceHeight >> 6;
        	H_Step              = SourceWidth >> 3;
        	pSceneChg->Step_No  = 64;
        	break;

        case 4: // total 8 x 8 regions, 8 horizontal region are vertically aligned with right shift 1
        	pSceneChg->V_Offset = (SourceHeight + 64) >> 7;
        	pSceneChg->H_Offset = ((SourceWidth + 8) >> 4) - 4;
        	V_Step              = SourceHeight >> 6;
        	H_Step              = (SourceWidth + 1) >> 3;
        	pSceneChg->Step_No  = 64;
        	break;

        case 5: // total 8 x 8 regions, 8 horizontal region are vertically aligned with left shift 1
        	pSceneChg->V_Offset = (SourceHeight + 64) >> 7;
        	pSceneChg->H_Offset = ((SourceWidth + 8) >> 4) + 4;
        	V_Step              = SourceHeight >> 6;
        	H_Step              = (SourceWidth - 1) >> 3;
        	pSceneChg->Step_No  = 64;
        	break;

        case 6: // total 4 x 4 regions, 4 horizontal region are vertically aligned
        	pSceneChg->V_Offset = (SourceHeight + 16) >> 5;
        	pSceneChg->H_Offset = (SourceWidth + 4) >> 3;
        	V_Step              = SourceHeight >> 4;
        	H_Step              = SourceWidth >> 2;
        	pSceneChg->Step_No  = 16;
        	break;

        case 7: // total 4 x 4 regions, 4 horizontal region are vertically aligned with right shift 1
        	pSceneChg->V_Offset = (SourceHeight + 16) >> 5;
        	pSceneChg->H_Offset = ((SourceWidth + 4) >> 3) - 2;
        	V_Step              = SourceHeight >> 4;
        	H_Step              = (SourceWidth + 1) >> 2;
        	pSceneChg->Step_No  = 16;
        	break;

        case 8: // total 4 x 4 regions, 4 horizontal region are vertically aligned with left shift 1
        	pSceneChg->V_Offset = (SourceHeight + 16) >> 5;
        	pSceneChg->H_Offset = ((SourceWidth + 4) >> 3) + 2;
        	V_Step              = SourceHeight >> 4;
        	H_Step              = (SourceWidth - 1) >> 2;
        	pSceneChg->Step_No  = 16;
        	break;

        default:
            VP_msg_ex(VP_MSG_TYPE_ERR, " %s() unknow error !\n", __FUNCTION__);
        	break;
    }

	pSceneChg->H_Step = (MMP_UINT16)(MMP_FLOATToFix(H_Step, 8, 4));
	pSceneChg->V_Step = (MMP_UINT16)(MMP_FLOATToFix(V_Step, 8, 4));
}


//=============================================================================
/**
 * ISP update hardware register.
 */
//=============================================================================
static void
_VP_UpdateHwReg(
    void)
{
    MMP_UINT16  index;

    //
    //Input Parameter
    //
    if(VPctxt->UpdateFlags & VP_FLAGS_UPDATE_InputParameter)
        VP_SetInputParameter_Reg(&VPctxt->InInfo);

    //
    //Input Width, Height, Pitch
    //
    if(VPctxt->UpdateFlags & VP_FLAGS_UPDATE_InputBuf)
        VP_SetInputBuf_Reg(&VPctxt->InInfo);

    //
    //Input Address
    //
    if(VPctxt->UpdateFlags & VP_FLAGS_UPDATE_InputAddr)
        VP_SetInputAddr_Reg(&VPctxt->InInfo);

    //
    //Deinterlace Parameter
    //
    if(VPctxt->UpdateFlags & VP_FLAGS_UPDATE_DeInterlaceParam)
        VP_SetDeInterlaceParam_Reg(&VPctxt->DeInterlace);

    //
    //Jpeg Encode Parameter
    //
    if(VPctxt->UpdateFlags & VP_FLAGS_UPDATE_JpegEncode)
        VP_SetJpegEncode_Reg(&VPctxt->JpegEncode);

    //
    //Color Correction Parameter
    //
    if(VPctxt->UpdateFlags & VP_FLAGS_UPDATE_CCMatrix)
    {
        VP_SetCCMatrix_Reg(&VPctxt->CCFun);
        VPctxt->UpdateFlags &= (~VP_FLAGS_UPDATE_CCMatrix);
    }

    //
    //Scale Parameter
    //
    if(VPctxt->UpdateFlags & VP_FLAGS_UPDATE_ScaleParam)
    {
        _VP_CalScaleHCI_VCI();
        VP_SetScaleParam_Reg(&VPctxt->ScaleFun);
    }

    //
    //Scale Horizontal Matrix
    //
    if(VPctxt->UpdateFlags & VP_FLAGS_UPDATE_ScaleMatrixH)
    {
        if (VPctxt->DeInterlace.EnableOutMotion)
        {
            VP_SetIntScaleMatrixH_Reg(MotionWeightMatInt_H);
        }
        else
        {
        for(index = 0; index < WEIGHT_NUM; index++)
        {
            if(VPctxt->ScaleFun.HCI >= VP_ScaleRatio[index])
            {
                if (VPctxt->OutInfo.EnableFieldMode == MMP_TRUE)
                    VP_SetIntScaleMatrixH_Reg(WeightMatInt_2TAP[index]);
                else
                VP_SetIntScaleMatrixH_Reg(WeightMatInt[index]);
                break;
            }
            else if(index == WEIGHT_NUM - 1)
            {
                if (VPctxt->OutInfo.EnableFieldMode == MMP_TRUE)
                    VP_SetIntScaleMatrixH_Reg(WeightMatInt_2TAP[index]);
                else
                VP_SetIntScaleMatrixH_Reg(WeightMatInt[index]);
        }
    }
    }
    }

    //
    //Scale Vertical Matrix
    //
    if(VPctxt->UpdateFlags & VP_FLAGS_UPDATE_ScaleMatrixV)
    {
        if (VPctxt->DeInterlace.EnableOutMotion)
        {
            VP_SetIntScaleMatrixV_Reg(MotionWeightMatInt_V);
        }
        else
        {
        for(index = 0; index < WEIGHT_NUM; index++)
        {
            if(VPctxt->ScaleFun.VCI >= VP_ScaleRatio[index])
            {
                if (VPctxt->OutInfo.EnableFieldMode == MMP_TRUE)
                    VP_SetIntScaleMatrixV_Reg(WeightMatInt_2TAP[index]);
                else
                VP_SetIntScaleMatrixV_Reg(WeightMatInt[index]);
                break;
            }
            else if(index == WEIGHT_NUM - 1)
            {
                if (VPctxt->OutInfo.EnableFieldMode == MMP_TRUE)
                    VP_SetIntScaleMatrixV_Reg(WeightMatInt_2TAP[index]);
                else
                VP_SetIntScaleMatrixV_Reg(WeightMatInt[index]);
        }
    }
    }
    }

    //
    //YUV to RGB Parameter
    //
    if(VPctxt->UpdateFlags & VP_FLAGS_UPDATE_FrmMatrix)
        VP_SetFrmMatrix_Reg(&VPctxt->FrmMatrix);

    //
    //Frame Function 0
    //
    if(VPctxt->UpdateFlags & VP_FLAGS_UPDATE_FrameFun0)
        VP_SetFrameFun0_Reg(&VPctxt->FrameFun0);

    //
    //YUV to RGB Parameter
    //
    if(VPctxt->UpdateFlags & VP_FLAGS_UPDATE_RGBtoYUVMatrix)
        VP_SetRGBtoYUVMatrix_Reg(&VPctxt->RGB2YUVFun);

    //
    //Output Parameter
    //
    if(VPctxt->UpdateFlags & VP_FLAGS_UPDATE_OutParameter)
        VP_SetOutParameter_Reg(&VPctxt->OutInfo);

    //
    //Output Width, Height and Pitch
    //
    if(VPctxt->UpdateFlags & VP_FLAGS_UPDATE_OutBufInfo)
        VP_SetOutBufInfo_Reg(&VPctxt->OutInfo);

    //
    //Output Address
    //
    if(VPctxt->UpdateFlags & VP_FLAGS_UPDATE_OutAddress)
        VP_SetOutAddress_Reg(&VPctxt->OutInfo);

    //
    //Remap Address
    //
    if(VPctxt->UpdateFlags & VP_FLAGS_UPDATE_RemapAddr)
    {
        VP_SetRemapYAddress_Reg(&VPctxt->RemapYAddr);
        VP_SetRemapUVAddress_Reg(&VPctxt->RemapUVAddr);
    }

    //
    //Scene Change
    //
    if (VPctxt->UpdateFlags & VP_FLAGS_UPDATE_SceneChange)
    {
        _VP_SceneChange_Param();

        if (VPctxt->UpdateFlags & VP_FLAGS_UPDATE_InitSceneChange)
            VP_InitSceneChange_Reg(&VPctxt->SceneChg);

        VP_SetSceneChange_Reg(&VPctxt->SceneChg);
    }

    //
    //ISP Interrupt
    //
    if(VPctxt->UpdateFlags & VP_FLAGS_UPDATE_Interrupt)
    {
        VP_SetInterruptParameter_Reg(VPctxt);
    }

    VP_RefreshFire_Reg();
}

//=============================================================================
//                Public Function Definition
//=============================================================================


//=============================================================================
/**
 * ISP default value initialization.
 */
//=============================================================================
MMP_RESULT
VP_ContextInitialize(
    MMP_BOOL initialMatrix)
{
    VP_RESULT  result = VP_SUCCESS;
    MMP_UINT16  i, j, index;
    MMP_FLOAT   **WeightMat;
    MMP_FLOAT   **WeightMat_2TAP;

    if (VPctxt == MMP_NULL)
    {
        result = VP_ERR_NOT_INITIALIZE;
        goto end;
    }

    //Input format
    VPctxt->InInfo.NVFormat = NV12;
    VPctxt->InInfo.EnableInYUV255Range = MMP_FALSE;
    VPctxt->InInfo.DisableCaptureCtrl = MMP_TRUE;
    VPctxt->InInfo.InputBufferNum = 0;

    //Deinterlace Paramter
    _VP_Deinter_Param(&VPctxt->DeInterlace);
    _VP_Deinter3D_Param(&VPctxt->DeInterlace);
    _VP_Deinter2D_Param(&VPctxt->DeInterlace);

    //Color Correction
    VPctxt->CCFun.OffsetR  = MMP_FLOATToFix(0.0f, 8, 0);
    VPctxt->CCFun.OffsetG  = MMP_FLOATToFix(0.0f, 8, 0);
    VPctxt->CCFun.OffsetB  = MMP_FLOATToFix(0.0f, 8, 0);
    VPctxt->CCFun._11      = MMP_FLOATToFix(1.0f, 4, 8);
    VPctxt->CCFun._12      = MMP_FLOATToFix(0.0f, 4, 8);
    VPctxt->CCFun._13      = MMP_FLOATToFix(0.0f, 4, 8);
    VPctxt->CCFun._21      = MMP_FLOATToFix(0.0f, 4, 8);
    VPctxt->CCFun._22      = MMP_FLOATToFix(1.0f, 4, 8);
    VPctxt->CCFun._23      = MMP_FLOATToFix(0.0f, 4, 8);
    VPctxt->CCFun._31      = MMP_FLOATToFix(0.0f, 4, 8);
    VPctxt->CCFun._32      = MMP_FLOATToFix(0.0f, 4, 8);
    VPctxt->CCFun._33      = MMP_FLOATToFix(1.0f, 4, 8);
    VPctxt->CCFun.DeltaR   = MMP_FLOATToFix(0.0f, 8, 0);
    VPctxt->CCFun.DeltaG   = MMP_FLOATToFix(0.0f, 8, 0);
    VPctxt->CCFun.DeltaB   = MMP_FLOATToFix(0.0f, 8, 0);
    VPctxt->InInfo.EnableCCFun = MMP_TRUE;

    //Scale
    VPctxt->ScaleFun.HCI = 0.0f;
    VPctxt->ScaleFun.VCI = 0.0f;

    //FrmFun RGB to YUV   p 0-255 Output-> 0-255
    VPctxt->FrmMatrix._11 = 0x004D;
    VPctxt->FrmMatrix._12 = 0x0096;
    VPctxt->FrmMatrix._13 = 0x001D;
    VPctxt->FrmMatrix._21 = 0x03d4;
    VPctxt->FrmMatrix._22 = 0x03a9;
    VPctxt->FrmMatrix._23 = 0x0083;
    VPctxt->FrmMatrix._31 = 0x0083;
    VPctxt->FrmMatrix._32 = 0x0392;
    VPctxt->FrmMatrix._33 = 0x03eb;
    VPctxt->FrmMatrix.ConstY = 0x0000;
    VPctxt->FrmMatrix.ConstU = 0x0080;
    VPctxt->FrmMatrix.ConstV = 0x0080;

    //Output format
    VPctxt->OutInfo.OutFormat = NVMode;
    VPctxt->OutInfo.NVFormat = NV12;
    VPctxt->OutInfo.SWWrFlipNum = 0;
    VPctxt->OutInfo.EnableSWFlipMode = MMP_FALSE;
    VPctxt->OutInfo.OutputBufferNum = 0;
    VPctxt->OutInfo.EnableFieldMode = MMP_FALSE;

    VPctxt->OutInfo.EngineDelay = 0;  // 0 - 15

    VPctxt->OutInfo.EnableUVBiDownsample = MMP_TRUE;
    VPctxt->OutInfo.EnableRemapYAddr = MMP_FALSE;
    VPctxt->OutInfo.EnableRemapUVAddr = MMP_FALSE;
    VPctxt->OutInfo.EnableSWCtrlRdAddr = MMP_FALSE;
    VPctxt->OutInfo.DisableOutMatrix = MMP_FALSE;

    if (VPctxt->InInfo.EnableInYUV255Range)
    {
        //YUV 0-255 ---> 16-235
        VPctxt->RGB2YUVFun._11 = 0x00DC;
        VPctxt->RGB2YUVFun._12 = 0x0000;
        VPctxt->RGB2YUVFun._13 = 0x0000;
        VPctxt->RGB2YUVFun._21 = 0x0000;
        VPctxt->RGB2YUVFun._22 = 0x00E1;
        VPctxt->RGB2YUVFun._23 = 0x0000;
        VPctxt->RGB2YUVFun._31 = 0x0000;
        VPctxt->RGB2YUVFun._32 = 0x0000;
        VPctxt->RGB2YUVFun._33 = 0x00E1;
        VPctxt->RGB2YUVFun.ConstY = 0x0010;
        VPctxt->RGB2YUVFun.ConstU = 0x0010;
        VPctxt->RGB2YUVFun.ConstV = 0x0010;
    }
    else
    {
        //YUV 0-255 ---> 0-255
        VPctxt->RGB2YUVFun._11 = 0x0100;
        VPctxt->RGB2YUVFun._12 = 0x0000;
        VPctxt->RGB2YUVFun._13 = 0x0000;
        VPctxt->RGB2YUVFun._21 = 0x0000;
        VPctxt->RGB2YUVFun._22 = 0x0100;
        VPctxt->RGB2YUVFun._23 = 0x0000;
        VPctxt->RGB2YUVFun._31 = 0x0000;
        VPctxt->RGB2YUVFun._32 = 0x0000;
        VPctxt->RGB2YUVFun._33 = 0x0100;
        VPctxt->RGB2YUVFun.ConstY = 0x0000;
        VPctxt->RGB2YUVFun.ConstU = 0x0000;
        VPctxt->RGB2YUVFun.ConstV = 0x0000;
    }

    //Type2 Frame based Tiled Map, Vertical Addressing Luma
    VPctxt->RemapYAddr.Addr_03 = (0x0 << 5) | 11;
    VPctxt->RemapYAddr.Addr_04 = (0x0 << 5) | 12;
    VPctxt->RemapYAddr.Addr_05 = (0x0 << 5) | 13;
    VPctxt->RemapYAddr.Addr_06 = (0x0 << 5) | 14;
    VPctxt->RemapYAddr.Addr_07 = (0x0 << 5) | 3;
    VPctxt->RemapYAddr.Addr_08 = (0x0 << 5) | 4;
    VPctxt->RemapYAddr.Addr_09 = (0x0 << 5) | 5;
    VPctxt->RemapYAddr.Addr_10 = (0x0 << 5) | 15;
    VPctxt->RemapYAddr.Addr_11 = (0x0 << 5) | 6;
    VPctxt->RemapYAddr.Addr_12 = (0x0 << 5) | 16;
    VPctxt->RemapYAddr.Addr_13 = (0x0 << 5) | 7;
    VPctxt->RemapYAddr.Addr_14 = (0x0 << 5) | 8;
    VPctxt->RemapYAddr.Addr_15 = (0x0 << 5) | 9;
    VPctxt->RemapYAddr.Addr_16 = (0x0 << 5) | 10;
    VPctxt->RemapYAddr.Addr_17 = (0x0 << 5) | 17;
    VPctxt->RemapYAddr.Addr_18 = (0x0 << 5) | 18;
    VPctxt->RemapYAddr.Addr_19 = (0x0 << 5) | 19;
    VPctxt->RemapYAddr.Addr_20 = (0x0 << 5) | 20;
    VPctxt->RemapYAddr.Addr_21 = (0x0 << 5) | 21;
    VPctxt->RemapYAddr.Addr_22 = (0x0 << 5) | 22;
    VPctxt->RemapYAddr.Addr_23 = (0x0 << 5) | 23;
    VPctxt->RemapYAddr.Addr_24 = (0x0 << 5) | 24;
    VPctxt->RemapYAddr.Addr_25 = (0x0 << 5) | 25;
    VPctxt->RemapYAddr.Addr_26 = (0x0 << 5) | 26;
    VPctxt->RemapYAddr.Addr_27 = (0x0 << 5) | 27;
    VPctxt->RemapYAddr.Addr_28 = (0x0 << 5) | 28;
    VPctxt->RemapYAddr.Addr_29 = (0x0 << 5) | 29;
    VPctxt->RemapYAddr.Addr_30 = (0x0 << 5) | 30;
    VPctxt->RemapYAddr.Addr_31 = (0x0 << 5) | 31;

    //Type2 Frame based Tiled Map, Vertical Addressing Chroma
    VPctxt->RemapUVAddr.Addr_03 = (0x0 << 5) | 11;
    VPctxt->RemapUVAddr.Addr_04 = (0x0 << 5) | 12;
    VPctxt->RemapUVAddr.Addr_05 = (0x0 << 5) | 13;
    VPctxt->RemapUVAddr.Addr_06 = (0x0 << 5) | 14;
    VPctxt->RemapUVAddr.Addr_07 = (0x0 << 5) | 3;
    VPctxt->RemapUVAddr.Addr_08 = (0x0 << 5) | 4;
    VPctxt->RemapUVAddr.Addr_09 = (0x0 << 5) | 5;
    VPctxt->RemapUVAddr.Addr_10 = (0x0 << 5) | 16;
    VPctxt->RemapUVAddr.Addr_11 = (0x1 << 5) | 6;
    VPctxt->RemapUVAddr.Addr_12 = (0x1 << 5) | 15;
    VPctxt->RemapUVAddr.Addr_13 = (0x0 << 5) | 7;
    VPctxt->RemapUVAddr.Addr_14 = (0x0 << 5) | 8;
    VPctxt->RemapUVAddr.Addr_15 = (0x0 << 5) | 9;
    VPctxt->RemapUVAddr.Addr_16 = (0x0 << 5) | 10;
    VPctxt->RemapUVAddr.Addr_17 = (0x0 << 5) | 17;
    VPctxt->RemapUVAddr.Addr_18 = (0x0 << 5) | 18;
    VPctxt->RemapUVAddr.Addr_19 = (0x0 << 5) | 19;
    VPctxt->RemapUVAddr.Addr_20 = (0x0 << 5) | 20;
    VPctxt->RemapUVAddr.Addr_21 = (0x0 << 5) | 21;
    VPctxt->RemapUVAddr.Addr_22 = (0x0 << 5) | 22;
    VPctxt->RemapUVAddr.Addr_23 = (0x0 << 5) | 23;
    VPctxt->RemapUVAddr.Addr_24 = (0x0 << 5) | 24;
    VPctxt->RemapUVAddr.Addr_25 = (0x0 << 5) | 25;
    VPctxt->RemapUVAddr.Addr_26 = (0x0 << 5) | 26;
    VPctxt->RemapUVAddr.Addr_27 = (0x0 << 5) | 27;
    VPctxt->RemapUVAddr.Addr_28 = (0x0 << 5) | 28;
    VPctxt->RemapUVAddr.Addr_29 = (0x0 << 5) | 29;
    VPctxt->RemapUVAddr.Addr_30 = (0x0 << 5) | 30;
    VPctxt->RemapUVAddr.Addr_31 = (0x0 << 5) | 31;

    //ISP Interrupt
    VPctxt->EnableInterrupt = MMP_FALSE;
    VPctxt->InterruptMode = 0x0;

    //Color Contrl
    VPctxt->ColorCtrl.brightness = 0;
    VPctxt->ColorCtrl.contrast = 1.0;
    VPctxt->ColorCtrl.hue = 0;
    VPctxt->ColorCtrl.saturation = 1.0;
    VPctxt->ColorCtrl.colorEffect[0] = 0;
    VPctxt->ColorCtrl.colorEffect[1] = 0;

    //Scene Change
    VPctxt->InInfo.EnableSceneChg = MMP_FALSE;
    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_InitSceneChange;

    if (initialMatrix)
    {
        vp_get_mem2Dfloat(&WeightMat, VP_SCALE_TAP_SIZE, VP_SCALE_TAP);
        vp_get_mem2Dfloat(&WeightMat_2TAP, VP_SCALE_TAP_SIZE, 2);

        //Initial Video Weight Matrix
        for(index = 0; index < WEIGHT_NUM; index++)
        {
            _VP_CreateWeighting(VP_ScaleRatio[index], VP_SCALE_TAP, VP_SCALE_TAP_SIZE, WeightMat);

            for(j = 0; j < VP_SCALE_TAP_SIZE; j++)
                for(i = 0; i < VP_SCALE_TAP; i++)
                    WeightMatInt[index][j][i] = (MMP_UINT8)MMP_FLOATToFix(WeightMat[j][i], 1, 6);
        }

        //Initial Video Weight Matrix 2Tap
        for(index = 0; index < WEIGHT_NUM; index++)
        {
            _VP_CreateWeighting(VP_ScaleRatio[index], 2, VP_SCALE_TAP_SIZE, WeightMat_2TAP);

            for(j = 0; j < VP_SCALE_TAP_SIZE; j++)
                for(i = 0; i < VP_SCALE_TAP; i++)
                {
                    if (i == 1 || i == 2)
                        WeightMatInt_2TAP[index][j][i] = (MMP_UINT8)MMP_FLOATToFix(WeightMat_2TAP[j][i-1], 1, 6);
                    else
                        WeightMatInt_2TAP[index][j][i] = 0x00;
                }
        }

        vp_free_mem2Dfloat(WeightMat);
        vp_free_mem2Dfloat(WeightMat_2TAP);
    }

    //setting motion parameter
    VPctxt->MotionParam.MDThreshold_Low = 12;
    VPctxt->MotionParam.MDThreshold_High = 12;
    VPctxt->MotionParam.EnLPFWeight = MMP_TRUE;
    VPctxt->MotionParam.EnLPFStaticPixel = MMP_FALSE;
    VPctxt->MotionParam.DisableMV_A = MMP_TRUE;
    VPctxt->MotionParam.DisableMV_B = MMP_TRUE;
    VPctxt->MotionParam.DisableMV_C = MMP_TRUE;
    VPctxt->MotionParam.DisableMV_D = MMP_FALSE;
    VPctxt->MotionParam.DisableMV_E = MMP_FALSE;
    VPctxt->MotionParam.DisableMV_F = MMP_FALSE;
    VPctxt->MotionParam.DisableMV_G = MMP_FALSE;

    VPctxt->UpdateFlags = 0xFFFFFFFF;
end:
    if(result)
        VP_msg_ex(VP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
* Set Motion Detection Parameter.
**/
//=============================================================================
void
VP_EnableMotionDetectionParameter(
    const VP_DEINTERLACE_CTRL *pMotion)
{
    //Enable Motion Detection
    VPctxt->DeInterlace.Enable = MMP_TRUE;
    VPctxt->DeInterlace.EnableOutMotion = MMP_TRUE;
    VPctxt->DeInterlace.DeinterMode = DEINTER3D;
    VPctxt->DeInterlace.EnUV2DMethod = MMP_FALSE;
    VPctxt->InInfo.EnableInYUV255Range = MMP_TRUE;

    VPctxt->DeInterlace.MDThreshold_Low = pMotion->MDThreshold_Low;
    VPctxt->DeInterlace.MDThreshold_High = pMotion->MDThreshold_High;
    VPctxt->DeInterlace.MDThreshold_Step = (MMP_INT)((MMP_FLOAT)128.0f * 1.0f / (VPctxt->DeInterlace.MDThreshold_High - VPctxt->DeInterlace.MDThreshold_Low));

    VPctxt->DeInterlace.EnLPFWeight = pMotion->EnLPFWeight;
    VPctxt->DeInterlace.EnLPFStaticPixel = pMotion->EnLPFStaticPixel;

    VPctxt->DeInterlace.DisableMV_A = pMotion->DisableMV_A;
    VPctxt->DeInterlace.DisableMV_B = pMotion->DisableMV_B;
    VPctxt->DeInterlace.DisableMV_C = pMotion->DisableMV_C;
    VPctxt->DeInterlace.DisableMV_D = pMotion->DisableMV_D;
    VPctxt->DeInterlace.DisableMV_E = pMotion->DisableMV_E;
    VPctxt->DeInterlace.DisableMV_F = pMotion->DisableMV_F;
    VPctxt->DeInterlace.DisableMV_G = pMotion->DisableMV_G;

    VPctxt->UpdateFlags &= (~VP_FLAGS_UPDATE_MotionParameter);

    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_DeInterlaceParam;

    //Color Correction
    VPctxt->CCFun.OffsetR  = MMP_FLOATToFix(0.0f, 8, 0);
    VPctxt->CCFun.OffsetG  = MMP_FLOATToFix(0.0f, 8, 0);
    VPctxt->CCFun.OffsetB  = MMP_FLOATToFix(0.0f, 8, 0);
    VPctxt->CCFun._11      = MMP_FLOATToFix(-1.0f, 4, 8);
    VPctxt->CCFun._12      = MMP_FLOATToFix(0.0f, 4, 8);
    VPctxt->CCFun._13      = MMP_FLOATToFix(0.0f, 4, 8);
    VPctxt->CCFun._21      = MMP_FLOATToFix(0.0f, 4, 8);
    VPctxt->CCFun._22      = MMP_FLOATToFix(0.0f, 4, 8);
    VPctxt->CCFun._23      = MMP_FLOATToFix(0.0f, 4, 8);
    VPctxt->CCFun._31      = MMP_FLOATToFix(0.0f, 4, 8);
    VPctxt->CCFun._32      = MMP_FLOATToFix(0.0f, 4, 8);
    VPctxt->CCFun._33      = MMP_FLOATToFix(0.0f, 4, 8);
    VPctxt->CCFun.DeltaR   = MMP_FLOATToFix(255.0f, 8, 0);
    VPctxt->CCFun.DeltaG   = MMP_FLOATToFix(0.0f, 8, 0);
    VPctxt->CCFun.DeltaB   = MMP_FLOATToFix(0.0f, 8, 0);
    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_CCMatrix;

    MotionWeightMatInt_H[0][0] = 0x3;
    MotionWeightMatInt_H[0][1] = 0x3;
    MotionWeightMatInt_H[0][2] = 0x3;
    MotionWeightMatInt_H[0][3] = 0x3;

    MotionWeightMatInt_H[1][0] = 0x3;
    MotionWeightMatInt_H[1][1] = 0x3;
    MotionWeightMatInt_H[1][2] = 0x3;
    MotionWeightMatInt_H[1][3] = 0x3;

    MotionWeightMatInt_H[2][0] = 0x3;
    MotionWeightMatInt_H[2][1] = 0x3;
    MotionWeightMatInt_H[2][2] = 0x3;
    MotionWeightMatInt_H[2][3] = 0x3;

    MotionWeightMatInt_H[3][0] = 0x3;
    MotionWeightMatInt_H[3][1] = 0x3;
    MotionWeightMatInt_H[3][2] = 0x3;
    MotionWeightMatInt_H[3][3] = 0x3;

    MotionWeightMatInt_H[4][0] = 0x3;
    MotionWeightMatInt_H[4][1] = 0x3;
    MotionWeightMatInt_H[4][2] = 0x3;
    MotionWeightMatInt_H[4][3] = 0x3;

    MotionWeightMatInt_V[0][0] = 0x40;
    MotionWeightMatInt_V[0][1] = 0x40;
    MotionWeightMatInt_V[0][2] = 0x40;
    MotionWeightMatInt_V[0][3] = 0x40;

    MotionWeightMatInt_V[1][0] = 0x40;
    MotionWeightMatInt_V[1][1] = 0x40;
    MotionWeightMatInt_V[1][2] = 0x40;
    MotionWeightMatInt_V[1][3] = 0x40;

    MotionWeightMatInt_V[2][0] = 0x40;
    MotionWeightMatInt_V[2][1] = 0x40;
    MotionWeightMatInt_V[2][2] = 0x40;
    MotionWeightMatInt_V[2][3] = 0x40;

    MotionWeightMatInt_V[3][0] = 0x40;
    MotionWeightMatInt_V[3][1] = 0x40;
    MotionWeightMatInt_V[3][2] = 0x40;
    MotionWeightMatInt_V[3][3] = 0x40;

    MotionWeightMatInt_V[4][0] = 0x40;
    MotionWeightMatInt_V[4][1] = 0x40;
    MotionWeightMatInt_V[4][2] = 0x40;
    MotionWeightMatInt_V[4][3] = 0x40;

    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_ScaleMatrixH;
    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_ScaleMatrixV;

    //YUV 0-255 ---> 16-235
    VPctxt->RGB2YUVFun._11 = 0x0016;
    VPctxt->RGB2YUVFun._12 = 0x0000;
    VPctxt->RGB2YUVFun._13 = 0x0000;
    VPctxt->RGB2YUVFun._21 = 0x0000;
    VPctxt->RGB2YUVFun._22 = 0x0000;
    VPctxt->RGB2YUVFun._23 = 0x0000;
    VPctxt->RGB2YUVFun._31 = 0x0000;
    VPctxt->RGB2YUVFun._32 = 0x0000;
    VPctxt->RGB2YUVFun._33 = 0x0000;
    VPctxt->RGB2YUVFun.ConstY = 0x0000;
    VPctxt->RGB2YUVFun.ConstU = 0x0000;
    VPctxt->RGB2YUVFun.ConstV = 0x0000;
    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_RGBtoYUVMatrix;
}

//=============================================================================
/**
* Clear Motion Detection Parameter.
**/
//=============================================================================
void
VP_DisableMotionDetectionParameter(
    void)
{
    //Disable Motion Detection
    VPctxt->DeInterlace.Enable = MMP_FALSE;
    VPctxt->InInfo.EnableInYUV255Range = MMP_FALSE;

    //Deinterlace Paramter
    _VP_Deinter_Param(&VPctxt->DeInterlace);
    _VP_Deinter3D_Param(&VPctxt->DeInterlace);
    _VP_Deinter2D_Param(&VPctxt->DeInterlace);

    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_DeInterlaceParam;

    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_ScaleMatrixH;
    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_ScaleMatrixV;

    //Color Correction
    VPctxt->CCFun.OffsetR  = MMP_FLOATToFix(0.0f, 8, 0);
    VPctxt->CCFun.OffsetG  = MMP_FLOATToFix(0.0f, 8, 0);
    VPctxt->CCFun.OffsetB  = MMP_FLOATToFix(0.0f, 8, 0);
    VPctxt->CCFun._11      = MMP_FLOATToFix(1.0f, 4, 8);
    VPctxt->CCFun._12      = MMP_FLOATToFix(0.0f, 4, 8);
    VPctxt->CCFun._13      = MMP_FLOATToFix(0.0f, 4, 8);
    VPctxt->CCFun._21      = MMP_FLOATToFix(0.0f, 4, 8);
    VPctxt->CCFun._22      = MMP_FLOATToFix(1.0f, 4, 8);
    VPctxt->CCFun._23      = MMP_FLOATToFix(0.0f, 4, 8);
    VPctxt->CCFun._31      = MMP_FLOATToFix(0.0f, 4, 8);
    VPctxt->CCFun._32      = MMP_FLOATToFix(0.0f, 4, 8);
    VPctxt->CCFun._33      = MMP_FLOATToFix(1.0f, 4, 8);
    VPctxt->CCFun.DeltaR   = MMP_FLOATToFix(0.0f, 8, 0);
    VPctxt->CCFun.DeltaG   = MMP_FLOATToFix(0.0f, 8, 0);
    VPctxt->CCFun.DeltaB   = MMP_FLOATToFix(0.0f, 8, 0);

    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_CCMatrix;

    if (VPctxt->InInfo.EnableInYUV255Range)
    {
        //YUV 0-255 ---> 16-235
        VPctxt->RGB2YUVFun._11 = 0x00DC;
        VPctxt->RGB2YUVFun._12 = 0x0000;
        VPctxt->RGB2YUVFun._13 = 0x0000;
        VPctxt->RGB2YUVFun._21 = 0x0000;
        VPctxt->RGB2YUVFun._22 = 0x00E1;
        VPctxt->RGB2YUVFun._23 = 0x0000;
        VPctxt->RGB2YUVFun._31 = 0x0000;
        VPctxt->RGB2YUVFun._32 = 0x0000;
        VPctxt->RGB2YUVFun._33 = 0x00E1;
        VPctxt->RGB2YUVFun.ConstY = 0x0010;
        VPctxt->RGB2YUVFun.ConstU = 0x0010;
        VPctxt->RGB2YUVFun.ConstV = 0x0010;
    }
    else
    {
        //YUV 0-255 ---> 0-255
        VPctxt->RGB2YUVFun._11 = 0x0100;
        VPctxt->RGB2YUVFun._12 = 0x0000;
        VPctxt->RGB2YUVFun._13 = 0x0000;
        VPctxt->RGB2YUVFun._21 = 0x0000;
        VPctxt->RGB2YUVFun._22 = 0x0100;
        VPctxt->RGB2YUVFun._23 = 0x0000;
        VPctxt->RGB2YUVFun._31 = 0x0000;
        VPctxt->RGB2YUVFun._32 = 0x0000;
        VPctxt->RGB2YUVFun._33 = 0x0100;
        VPctxt->RGB2YUVFun.ConstY = 0x0000;
        VPctxt->RGB2YUVFun.ConstU = 0x0000;
        VPctxt->RGB2YUVFun.ConstV = 0x0000;
    }
    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_RGBtoYUVMatrix;
}

//=============================================================================
/**
* Set isp input format.
**/
//=============================================================================
MMP_RESULT
VP_SetInputFormat(
    MMP_VP_INFORMAT    format)
{
    VP_RESULT  result = VP_SUCCESS;

    if(VPctxt == MMP_NULL)
    {
        result = VP_ERR_NOT_INITIALIZE;
        goto end;
    }
    switch(format)
    {
        case MMP_VP_IN_NV12:
            VPctxt->InInfo.NVFormat = NV12;
            break;

        case MMP_VP_IN_NV21:
            VPctxt->InInfo.NVFormat = NV21;
            break;

        default:
            result = VP_ERR_NO_MATCH_INPUT_FORMAT;
            break;
    }

    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_InputParameter;
end:

    if(result)
        VP_msg_ex(VP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}


//=============================================================================
/**
* Set isp output format.
**/
//=============================================================================
MMP_RESULT
VP_SetOutputFormat(
    MMP_VP_OUTFORMAT   format)
{
    VP_RESULT   result = VP_SUCCESS;

    if (VPctxt == MMP_NULL)
    {
        result = VP_ERR_NOT_INITIALIZE;
        VP_msg_ex(VP_MSG_TYPE_ERR, " NULL pointer !\n");
        goto end;
    }

    switch(format)
    {
        case MMP_VP_OUT_YUV422:
            VPctxt->OutInfo.OutFormat = YUVPlane;
            VPctxt->OutInfo.PlaneFormat = VP_YUV422;
            break;

        case MMP_VP_OUT_YUV420:
            VPctxt->OutInfo.OutFormat = YUVPlane;
            VPctxt->OutInfo.PlaneFormat = VP_YUV420;
            break;

        case MMP_VP_OUT_YUV444:
            VPctxt->OutInfo.OutFormat = YUVPlane;
            VPctxt->OutInfo.PlaneFormat = VP_YUV444;
            break;

        case MMP_VP_OUT_YUV422R:
            VPctxt->OutInfo.OutFormat = YUVPlane;
            VPctxt->OutInfo.PlaneFormat = VP_YUV422R;
            break;

        case MMP_VP_OUT_NV12:
            VPctxt->OutInfo.OutFormat = NVMode;
            VPctxt->OutInfo.NVFormat = NV12;
            break;

        case MMP_VP_OUT_NV21:
            VPctxt->InInfo.NVFormat = NVMode;
            VPctxt->OutInfo.NVFormat = NV21;
            break;

        default:
            result = VP_ERR_NO_MATCH_OUTPUT_FORMAT;
            break;
    }

    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_OutParameter;

end:
    if(result)
        VP_msg_ex(VP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * Update ISP device.
 *
 * @return VP_RESULT_SUCCESS if succeed, error codes of VP_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
VP_Update(
    void)
{
    VP_RESULT  result = VP_SUCCESS;
    MMP_UINT32 PreUpdateFlags;

    if(VPctxt == MMP_NULL)
    {
        result = VP_ERR_NOT_INITIALIZE;
        goto end;
    }

    // Update ISP Scale Parameter
    if ((VPctxt->UpdateFlags & VP_FLAGS_UPDATE_InputBuf) ||
        (VPctxt->UpdateFlags & VP_FLAGS_UPDATE_OutBufInfo))
    {
        VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_ScaleParam;
    }

    if (VPctxt->UpdateFlags & VP_FLAGS_UPDATE_InputBuf)
        VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_SceneChange;

    // Update ISP hardware register
    if(VPctxt->UpdateFlags)
        _VP_UpdateHwReg();

    // Clear Update Flags
    PreUpdateFlags = VPctxt->UpdateFlags;
    VPctxt->UpdateFlags = PreUpdateFlags & (0x0 | VP_FLAGS_UPDATE_CCMatrix | VP_FLAGS_UPDATE_MotionParameter);

end:

    if(result)
        VP_msg_ex(VP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
// brightness:     -128 ~ 127     default : 0
// contrast:       0.0 ~ 4.0      default : 1.0
// hue:            0 ~ 359        default : 0
// saturation:     0.0 ~ 4.0      default : 1.0
// colorEffect[2]: -128 ~ 128     default : 0, 0

// preOff:  S8
// M:       S4.8
// postOff: S8
*/
//=============================================================================
#if defined (USE_COLOR_EFFECT)
void
VP_SetColorCorrMatrix(
    VP_COLOR_CORRECTION  *pColorCorrect,
    MMP_INT32 brightness,
    MMP_FLOAT contrast,
    MMP_INT32 hue,
    MMP_FLOAT saturation,
    MMP_INT32 colorEffect[2])
{
    MMP_INT32 preOff[3];
    MMP_INT32 M[3][3];
    MMP_INT32 postOff[3];
    MMP_FLOAT cosTh, sinTh;

    preOff[0] = preOff[1] = preOff[2] = -128;

    M[0][0] = (int)(contrast * 256 + 0.5);
    M[0][1] = M[0][2] = 0;
    getSinCos(hue, &sinTh, &cosTh);
    M[1][0] = 0;
    M[1][1] = (int)(saturation * cosTh * 256 + 0.5);
    M[1][2] = (int)(saturation * -sinTh * 256 + 0.5);
    M[2][0] = 0;
    M[2][1] = (int)(saturation * sinTh * 256 + 0.5);
    M[2][2] = (int)(saturation * cosTh * 256 + 0.5);

    postOff[0] = (int)(contrast * brightness + 128.5);
    postOff[1] = colorEffect[0] + 128;
    postOff[2] = colorEffect[1] + 128;

    pColorCorrect->OffsetR  = preOff[0];
    pColorCorrect->OffsetG  = preOff[1];
    pColorCorrect->OffsetB  = preOff[2];
    pColorCorrect->_11      = M[0][0];
    pColorCorrect->_12      = M[0][1];
    pColorCorrect->_13      = M[0][2];
    pColorCorrect->_21      = M[1][0];
    pColorCorrect->_22      = M[1][1];
    pColorCorrect->_23      = M[1][2];
    pColorCorrect->_31      = M[2][0];
    pColorCorrect->_32      = M[2][1];
    pColorCorrect->_33      = M[2][2];
    pColorCorrect->DeltaR   = postOff[0];
    pColorCorrect->DeltaG   = postOff[1];
    pColorCorrect->DeltaB   = postOff[2];
}

//=============================================================================
/**
 * Update ISP color matrix device.
 *
 * @return VP_RESULT_SUCCESS if succeed, error codes of VP_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
VP_UpdateColorMatrix(
    void)
{
    VP_RESULT  result = VP_SUCCESS;

    if(VPctxt == MMP_NULL)
    {
        result = VP_ERR_NOT_INITIALIZE;
        goto end;
    }

    //
    //Color Correction Parameter
    //
    if(VPctxt->UpdateFlags & VP_FLAGS_UPDATE_CCMatrix)
    {
        VP_SetCCMatrix_Reg(&VPctxt->CCFun);
        VPctxt->UpdateFlags &= (~VP_FLAGS_UPDATE_CCMatrix);
    }

end:
    if(result)
        VP_msg_ex(VP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}
#endif

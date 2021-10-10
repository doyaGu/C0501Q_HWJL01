#include "isp_types.h"
#include "isp_hw.h"
#include "isp.h"
#include "isp_util.h"

//=============================================================================
//				  Constant Definition
//=============================================================================

//=============================================================================
//				  Macro Definition
//=============================================================================
#define _MAX(a, b) (((a) >= (b)) ? (a) : (b))
#define _MIN(a, b) (((a) >= (b)) ? (b) : (a))

//=============================================================================
//				  Structure Definition
//=============================================================================

//=============================================================================
//				  Global Data Definition
//=============================================================================
#define     WEIGHT_NUM        10
#define     ISP_LINE_BUF_SIZE 1280
MMP_FLOAT ScaleRatio[WEIGHT_NUM] = {(8192.0f / ISP_LINE_BUF_SIZE), (6144.0f / ISP_LINE_BUF_SIZE), (4096.0f / ISP_LINE_BUF_SIZE),
                                    (3456.0f / ISP_LINE_BUF_SIZE), (3072.0f / ISP_LINE_BUF_SIZE), (2056.0f / ISP_LINE_BUF_SIZE),
                                    (2048.0f / ISP_LINE_BUF_SIZE), (1920.0f / ISP_LINE_BUF_SIZE), (1600.0f / ISP_LINE_BUF_SIZE),
                                    (1280.0f / ISP_LINE_BUF_SIZE)};
MMP_UINT8 WeightMatInt[WEIGHT_NUM][ISP_SCALE_TAP_SIZE][ISP_SCALE_TAP];

//=============================================================================
//				  Private Function Definition
//=============================================================================
static MMP_INLINE MMP_FLOAT
_MAX3(
    MMP_FLOAT a,
    MMP_FLOAT b,
    MMP_FLOAT c)
{
    MMP_FLOAT max = (a >= b) ? a : b;
    if (c > max)
        max = c;
    return max;
}

//=============================================================================
/**
 * Calculate Scale Factor
 */
//=============================================================================
static MMP_FLOAT
_ISP_ScaleFactor(
    MMP_UINT16 Input,
    MMP_UINT16 Output)
{
    return (MMP_FLOAT) (((MMP_INT) (16384.0f * Input / (MMP_FLOAT)Output)) / 16384.0f);
}

//=============================================================================
/**
 * Calculate ISP Deinterlace.
 */
//=============================================================================
static void
_ISP_Deinter_Param(
    ISP_DEINTERLACE_CTRL *pDeInterlace)
{
    pDeInterlace->DeinterMode           = DEINTER3D;
    pDeInterlace->Disable30MotionDetect = MMP_FALSE;
    pDeInterlace->EnUV2DMethod          = MMP_FALSE;

    pDeInterlace->EnSrcBottomFieldFirst = 0;
    pDeInterlace->EnDeinterBottomField  = 1;

    pDeInterlace->EnChromaEdgeDetect    = 1;
    pDeInterlace->EnLummaEdgeDetect     = 1;
    pDeInterlace->EnSrcLPF              = 1;

    pDeInterlace->EnLowLevelEdge        = MMP_FALSE;
    pDeInterlace->LowLevelMode          = 0;
    pDeInterlace->EnLowLevelOutside     = MMP_FALSE;
    pDeInterlace->LowLevelBypassBlend   = 0;

    pDeInterlace->LowLevelPosX          = 0;
    pDeInterlace->LowLevelPosY          = 425;
    pDeInterlace->LowLevelWidth         = 704;
    pDeInterlace->LowLevelHeight        = 55;
}

static void
_ISP_Deinter3D_Param(
    ISP_DEINTERLACE_CTRL *pDeInterlace)
{
    MMP_UINT16 MDThreshold_High = 40;
    MMP_UINT16 MDThreshold_Low  = 8;

    pDeInterlace->MDThreshold_Low  = MDThreshold_Low;
    pDeInterlace->MDThreshold_High = MDThreshold_High;
    pDeInterlace->MDThreshold_Step = (MMP_INT)((MMP_FLOAT)128.0f * 1.0f / (MDThreshold_High - MDThreshold_Low));

    pDeInterlace->EnLPFWeight      = 0;
    pDeInterlace->EnLPFWeight      = 0;
    pDeInterlace->EnLPFStaticPixel = 0;

    pDeInterlace->DisableMV_A      = 0;
    pDeInterlace->DisableMV_B      = 0;
    pDeInterlace->DisableMV_C      = 0;
    pDeInterlace->DisableMV_D      = 0;
    pDeInterlace->DisableMV_E      = 0;
    pDeInterlace->DisableMV_F      = 0;
    pDeInterlace->DisableMV_G      = 0;
}

static void
_ISP_Deinter2D_Param(
    ISP_DEINTERLACE_CTRL *pDeInterlace)
{
    MMP_UINT16 EdgeBlendWeight = 14;
    MMP_UINT16 OrgBlendWeight  = 2;

    pDeInterlace->D2EdgeBlendWeight = (MMP_INT)((MMP_FLOAT)64.0f * EdgeBlendWeight / (EdgeBlendWeight + OrgBlendWeight));
    pDeInterlace->D2OrgBlendWeight  = (MMP_INT)((MMP_FLOAT)64.0f * OrgBlendWeight / (EdgeBlendWeight + OrgBlendWeight));
}

//=============================================================================
/**
 * Calculate ISP Pre-Scaling Factor.
 */
//=============================================================================
static void
_ISP_CalPreScaleHCI(
    ISP_DEVICE ptDev)
{
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;
    MMP_FLOAT   HCI;

    HCI = _ISP_ScaleFactor(pISPctxt->InInfo.SrcWidth, pISPctxt->PreScaleFun.DstWidth);

    if (HCI != pISPctxt->PreScaleFun.HCI)
        pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_PreScaleMatrix;

    pISPctxt->PreScaleFun.HCI = HCI;
}

//=============================================================================
/**
 * Calculate ISP Scaling Factor.
 */
//=============================================================================
static void
_ISP_CalScaleHCI_VCI(
    ISP_DEVICE ptDev)
{
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;
    MMP_FLOAT   HCI;
    MMP_FLOAT   VCI;

    HCI                    = _ISP_ScaleFactor(pISPctxt->PreScaleFun.DstWidth, pISPctxt->ScaleFun.DstWidth);
    VCI                    = _ISP_ScaleFactor(pISPctxt->InInfo.SrcHeight, pISPctxt->ScaleFun.DstHeight);

    pISPctxt->ScaleFun.HCI = HCI;
    pISPctxt->ScaleFun.VCI = VCI;
}

//=============================================================================
/**
 * Create weighting for the matrix of scaling.
 */
//=============================================================================
static void
_ISP_CreateWeighting(
    ISP_DEVICE ptDev,
    MMP_FLOAT  scale,
    MMP_UINT8  taps,
    MMP_UINT8  tapSize,
    MMP_FLOAT  weightMatrix[][ISP_SCALE_TAP])
{
    MMP_UINT8   i, j;
    MMP_FLOAT   WW;
    MMP_FLOAT   W[32];
    MMP_INT16   point;
    MMP_UINT8   adjust;
    MMP_UINT8   method;
    MMP_FLOAT   precision = 64.0f;
    MMP_FLOAT   fscale    = 1.0f;
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    //Kevin:TODO temp solution
    adjust = 0;
    method = 11;

    switch (pISPctxt->sharp)
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
    }

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
        for (i = 0; i <= (tapSize >> 1); i++)
        {
            WW    = 0.0f;
            point = (MMP_INT)(taps >> 1) - 1;
            for (j = 0; j < taps; j++)
            {
                W[j] = sinc( ((MMP_FLOAT)point + i / ((MMP_FLOAT)tapSize)) / (MMP_FLOAT)scale);
                point--;
                WW  += W[j];
            }

            //for (j=0; j< taps; j++)
            //  WeightMat[i][j] = ((int)(W[j]/WW*precision + 0.5 ))/precision;

            //Changed: 2004/02/24
            weightMatrix[i][taps - 1] = 1.0;
            for (j = 0; j < taps - 1; j++)
            {
                weightMatrix[i][j]         = ((MMP_INT)(W[j] / WW * precision + 0.5)) / precision;
                weightMatrix[i][taps - 1] -= weightMatrix[i][j];
            }
        }

        for (i = ((tapSize >> 1) + 1); i < tapSize; i++)
        {
            for (j = 0; j < taps; j++)
            {
                weightMatrix[i][j] = weightMatrix[tapSize - i][taps - 1 - j];
            }
        }
    }
    else if (method == 11)
    {
        //rcos
        for (i = 0; i <= (tapSize >> 1); i++)
        {
            WW    = 0.0;
            point = (MMP_INT) (taps >> 1) - 1;
            for (j = 0; j < taps; j++)
            {
                W[j] = rcos( ((MMP_FLOAT)point + i / ((MMP_FLOAT)tapSize)) / (MMP_FLOAT)scale);
                point--;
                WW  += W[j];
            }

            //for (j=0; j< taps; j++)
            //  weightMatrix[i][j] = ((int)(W[j]/WW*precision + 0.5 ))/precision;

            //Changed: 2004/02/24
            weightMatrix[i][taps - 1] = 1.0;
            for (j = 0; j < taps - 1; j++)
            {
                weightMatrix[i][j]         = ((MMP_INT)(W[j] / WW * precision + 0.5)) / precision;
                weightMatrix[i][taps - 1] -= weightMatrix[i][j];
            }
        }
        for (i = ((tapSize >> 1) + 1); i < tapSize; i++)
        {
            for (j = 0; j < taps; j++)
            {
                weightMatrix[i][j] = weightMatrix[tapSize - i][taps - 1 - j];
            }
        }
    }
    else if (method == 12)
    {
        // Catmull-Rom Cubic interpolation
        for (i = 0; i <= (tapSize >> 1); i++)
        {
            WW    = 0.0f;
            point = (MMP_INT)(taps >> 1) - 1;
            for (j = 0; j < taps; j++)
            {
                W[j] = cubic01( ((MMP_FLOAT)point + i / ((MMP_FLOAT)tapSize)) / (MMP_FLOAT)scale);
                point--;
                WW  += W[j];

                //printf("i:%2d   W=%6.3f   point=%2d    WW=%6.3f\n",
                //  i, W[j], point, WW);
            }

            //Changed: 2004/02/24
            weightMatrix[i][taps - 1] = 1.0;
            for (j = 0; j < taps - 1; j++)
            {
                weightMatrix[i][j]         = ((MMP_INT)(W[j] / WW * precision + 0.5)) / precision;
                weightMatrix[i][taps - 1] -= weightMatrix[i][j];
            }
        }
        for (i = ((tapSize >> 1) + 1); i < tapSize; i++)
        {
            for (j = 0; j < taps; j++)
            {
                weightMatrix[i][j] = weightMatrix[tapSize - i][taps - 1 - j];
            }
        }
    }
    else
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() unknow error !\n", __FUNCTION__);
    }
}

//=============================================================================
/**
 * ISP color matrix.
 */
//=============================================================================
void
ISP_SetColorMatrix(
    ISP_DEVICE ptDev)
{
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    {
        //Input 16 - 235 -> Output 0 - 255
        pISPctxt->YUV2RGBFun._11    = 0x012A;
        pISPctxt->YUV2RGBFun._12    = 0x0000;
        pISPctxt->YUV2RGBFun._13    = 0x0199;
        pISPctxt->YUV2RGBFun._21    = 0x012A;
        pISPctxt->YUV2RGBFun._22    = 0x079C;
        pISPctxt->YUV2RGBFun._23    = 0x0730;
        pISPctxt->YUV2RGBFun._31    = 0x012A;
        pISPctxt->YUV2RGBFun._32    = 0x0205;
        pISPctxt->YUV2RGBFun._33    = 0x0000;
        pISPctxt->YUV2RGBFun.ConstR = 0x0322;
        pISPctxt->YUV2RGBFun.ConstG = 0x0087;
        pISPctxt->YUV2RGBFun.ConstB = 0x02EC;
    }
    /*
       {
        pISPctxt->YUV2RGBFun._11 = 0x0100;
        pISPctxt->YUV2RGBFun._12 = 0x0000;
        pISPctxt->YUV2RGBFun._13 = 0x0167;
        pISPctxt->YUV2RGBFun._21 = 0x0100;
        pISPctxt->YUV2RGBFun._22 = 0x07A8;
        pISPctxt->YUV2RGBFun._23 = 0x0749;
        pISPctxt->YUV2RGBFun._31 = 0x0100;
        pISPctxt->YUV2RGBFun._32 = 0x01C6;
        pISPctxt->YUV2RGBFun._33 = 0x0000;
        pISPctxt->YUV2RGBFun.ConstR = 0x034D;
        pISPctxt->YUV2RGBFun.ConstG = 0x0089;
        pISPctxt->YUV2RGBFun.ConstB = 0x031E;
       }*/

    /*
       {
        //Input 16 - 235 -> Output 16 - 235
        //Input 0 - 255 -> Output 0 - 255
        pISPctxt->YUV2RGBFun._11 = 0x0100;
        pISPctxt->YUV2RGBFun._13 = 0x015F;
        pISPctxt->YUV2RGBFun._21 = 0x0100;
        pISPctxt->YUV2RGBFun._22 = 0x07AA;
        pISPctxt->YUV2RGBFun._23 = 0x074D;
        pISPctxt->YUV2RGBFun._31 = 0x0100;
        pISPctxt->YUV2RGBFun._32 = 0x01BB;
        pISPctxt->YUV2RGBFun.ConstR = 0x0351;
        pISPctxt->YUV2RGBFun.ConstG = 0x0084;
        pISPctxt->YUV2RGBFun.ConstB = 0x0322;

       }*/

    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_YUVtoRGBMatrix;
}

//=============================================================================
/**
 * ISP update hardware register.
 */
//=============================================================================
static void
_ISP_UpdateHwReg(
    ISP_DEVICE ptDev)
{
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;
    MMP_UINT16  index;
    //
    //Set Engine Mode
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_EngineMode)
        ISP_SetEngineMode_Reg(&pISPctxt->EngineMode);

    //
    //Input Parameter
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_InputParameter)
        ISP_SetInputParameter_Reg(&pISPctxt->InInfo);

    //
    //Input Width, Height, Pitch
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_InputBuf)
        ISP_SetInputBuf_Reg(&pISPctxt->InInfo);

    //
    //Input Address
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_InputAddr)
        ISP_SetInputAddr_Reg(&pISPctxt->InInfo);

    //
    //Remap Address
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_RemapAddr)
    {
        ISP_SetRemapYAddress_Reg(pISPctxt->RemapTableYIdx);
        ISP_SetRemapUVAddress_Reg(pISPctxt->RemapTableUVIdx);
    }

    //
    //Deinterlace Parameter
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_DeInterlaceParam)
        ISP_SetDeInterlaceParam_Reg(pISPctxt, &pISPctxt->DeInterlace);

    //
    //YUV to RGB Parameter
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_YUVtoRGBMatrix)
        ISP_SetYUVtoRGBMatrix_Reg(&pISPctxt->YUV2RGBFun);

    //
    //Color Correction Parameter
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_CCMatrix)
        ISP_SetCCMatrix_Reg(&pISPctxt->CCFun);

    //
    //Pre-Scale Parameter
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_PreScaleParam)
    {
        _ISP_CalPreScaleHCI(pISPctxt);
        ISP_SetPreScaleParam_Reg(&pISPctxt->PreScaleFun);
    }

    //
    //Pre-Scale Matrix
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_PreScaleMatrix)
    {
        //if(pISPctxt->ispMode == MMP_ISP_MODE_PLAY_VIDEO)
        {
            for (index = 0; index < WEIGHT_NUM; index++)
            {
                if (pISPctxt->PreScaleFun.HCI >= ScaleRatio[index])
                {
                    ISP_SetIntPreScaleMatrix_Reg(WeightMatInt[index]);
                    break;
                }
                else if (index == WEIGHT_NUM - 1)
                    ISP_SetIntPreScaleMatrix_Reg(WeightMatInt[index]);
            }
        }
        //else
        //{
        //    _ISP_CreateWeighting(pISPctxt, pISPctxt->PreScaleFun.HCI, ISP_SCALE_TAP, ISP_SCALE_TAP_SIZE, ISPctxt->PreScaleFun.WeightMatX);
        //    ISP_SetPreScaleMatrix_Reg(&pISPctxt->PreScaleFun);
        //}
    }

    //
    //Scale Parameter
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_ScaleParam)
    {
        _ISP_CalScaleHCI_VCI(pISPctxt);
        ISP_SetScaleParam_Reg(&pISPctxt->ScaleFun);
    }

    //
    //Frame Function 0
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_FrameFun0)
        ISP_SetFrameFun0_Reg(&pISPctxt->FrameFun0);

    //
    //Output Parameter
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_OutParameter)
        ISP_SetOutParameter_Reg(&pISPctxt->OutInfo);

    //
    //Output Width, Height and Pitch
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_OutBufInfo)
        ISP_SetOutBufInfo_Reg(&pISPctxt->OutInfo);

    //
    //Output Address
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_OutAddress)
        ISP_SetOutAddress_Reg(&pISPctxt->OutInfo);

    //
    //Run-Length Enc
    //
    //if(pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_RunLenEnc)
    //    ISP_SetRunLengthEnc_Reg(&pISPctxt->RunLenEnc);

    //
    //ISP Interrupt
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_Interrupt)
    {
        ISP_SetInterruptParameter_Reg(pISPctxt);
    }
}

//=============================================================================
//				  Public Function Definition
//=============================================================================
//=============================================================================
/**
 * ISP default value initialization.
 */
//=============================================================================
ISP_RESULT
ISP_ContextInitialize(
    ISP_DEVICE ptDev)
{
    ISP_RESULT  result    = ISP_SUCCESS;
    MMP_UINT16  i, j, index;
    MMP_FLOAT   WeightMat[ISP_SCALE_TAP_SIZE][ISP_SCALE_TAP];
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    if (pISPctxt == MMP_NULL)
    {
        result = ISP_ERR_NOT_INITIALIZE;
        goto end;
    }

    //Input format
    pISPctxt->InInfo.PlaneFormat         = YUV420;
    pISPctxt->InInfo.EnableDSFun         = MMP_FALSE;
    pISPctxt->InInfo.UVRepeatMode        = MMP_FALSE;
    pISPctxt->InInfo.EnableRemapYAddr    = MMP_FALSE;
    pISPctxt->InInfo.EnableRemapUVAddr   = MMP_FALSE;
    pISPctxt->InInfo.EnableInYUV255Range = MMP_FALSE;

    //Deinterlace Paramter
    _ISP_Deinter_Param(&pISPctxt->DeInterlace);
    _ISP_Deinter3D_Param(&pISPctxt->DeInterlace);
    _ISP_Deinter2D_Param(&pISPctxt->DeInterlace);

    //YUV to RGB
    ISP_SetColorMatrix(ptDev);
    pISPctxt->InInfo.EnableCSFun        = MMP_TRUE;

    //Color Correction
    pISPctxt->CCFun.OffsetR             = 0.0f;
    pISPctxt->CCFun.OffsetG             = 0.0f;
    pISPctxt->CCFun.OffsetB             = 0.0f;
    pISPctxt->CCFun._11                 = 1.0f;
    pISPctxt->CCFun._12                 = 0.0f;
    pISPctxt->CCFun._13                 = 0.0f;
    pISPctxt->CCFun._21                 = 0.0f;
    pISPctxt->CCFun._22                 = 1.0f;
    pISPctxt->CCFun._23                 = 0.0f;
    pISPctxt->CCFun._31                 = 0.0f;
    pISPctxt->CCFun._32                 = 0.0f;
    pISPctxt->CCFun._33                 = 1.0f;
    pISPctxt->CCFun.DeltaR              = 0.0f;
    pISPctxt->CCFun.DeltaG              = 0.0f;
    pISPctxt->CCFun.DeltaB              = 0.0f;
    pISPctxt->InInfo.EnableCCFun        = MMP_FALSE;

    //PreScale
    pISPctxt->PreScaleFun.HCI           = 0.0f;
    pISPctxt->UpdateFlags              |= ISP_FLAGS_UPDATE_PreScaleParam;

    //Scale
    pISPctxt->ScaleFun.HCI              = 0.0f;
    pISPctxt->ScaleFun.VCI              = 0.0f;
    pISPctxt->ScaleFun.BGColorR         = 0x00;
    pISPctxt->ScaleFun.BGColorG         = 0x80;
    pISPctxt->ScaleFun.BGColorB         = 0x80;

    //some parameter for color correction matrix.
    pISPctxt->hue                       = 0;
    pISPctxt->saturation                = 128;
    pISPctxt->contrast                  = 0;
    pISPctxt->midPoint                  = 128;
    pISPctxt->colorEffect               = 0;
    pISPctxt->brightness                = 0;
    pISPctxt->sharp                     = 0;

    //Output format
    pISPctxt->OutInfo.RGBFormat         = Dither565;
    pISPctxt->OutInfo.DitherMode        = 0;

#if (CFG_CHIP_PKG_IT9856 || CFG_CHIP_PKG_IT9854)  // pitch = 2048
    pISPctxt->RemapYAddr.Addr[0]        = (0x0 << 5) | 0;
    pISPctxt->RemapYAddr.Addr[1]        = (0x0 << 5) | 1;
    pISPctxt->RemapYAddr.Addr[2]        = (0x0 << 5) | 2;
    pISPctxt->RemapYAddr.Addr[3]        = (0x0 << 5) | 11;
    pISPctxt->RemapYAddr.Addr[4]        = (0x0 << 5) | 12;
    pISPctxt->RemapYAddr.Addr[5]        = (0x0 << 5) | 13;
    pISPctxt->RemapYAddr.Addr[6]        = (0x0 << 5) | 14;
    pISPctxt->RemapYAddr.Addr[7]        = (0x0 << 5) | 3;
    pISPctxt->RemapYAddr.Addr[8]        = (0x0 << 5) | 4;
    pISPctxt->RemapYAddr.Addr[9]        = (0x0 << 5) | 5;
    pISPctxt->RemapYAddr.Addr[10]       = (0x0 << 5) | 15;
    pISPctxt->RemapYAddr.Addr[11]       = (0x0 << 5) | 6;
    pISPctxt->RemapYAddr.Addr[12]       = (0x0 << 5) | 16;
    pISPctxt->RemapYAddr.Addr[13]       = (0x0 << 5) | 7;
    pISPctxt->RemapYAddr.Addr[14]       = (0x0 << 5) | 8;
    pISPctxt->RemapYAddr.Addr[15]       = (0x0 << 5) | 9;
    pISPctxt->RemapYAddr.Addr[16]       = (0x0 << 5) | 10;
    pISPctxt->RemapYAddr.Addr[17]       = (0x0 << 5) | 17;
    pISPctxt->RemapYAddr.Addr[18]       = (0x0 << 5) | 18;
    pISPctxt->RemapYAddr.Addr[19]       = (0x0 << 5) | 19;
    pISPctxt->RemapYAddr.Addr[20]       = (0x0 << 5) | 20;
    pISPctxt->RemapYAddr.Addr[21]       = (0x0 << 5) | 21;
    pISPctxt->RemapYAddr.Addr[22]       = (0x0 << 5) | 22;
    pISPctxt->RemapYAddr.Addr[23]       = (0x0 << 5) | 23;
    pISPctxt->RemapYAddr.Addr[24]       = (0x0 << 5) | 24;
    pISPctxt->RemapYAddr.Addr[25]       = (0x0 << 5) | 25;
    pISPctxt->RemapYAddr.Addr[26]       = (0x0 << 5) | 26;
    pISPctxt->RemapYAddr.Addr[27]       = (0x0 << 5) | 27;
    pISPctxt->RemapYAddr.Addr[28]       = (0x0 << 5) | 28;
    pISPctxt->RemapYAddr.Addr[29]       = (0x0 << 5) | 29;
    pISPctxt->RemapYAddr.Addr[30]       = (0x0 << 5) | 30;
    pISPctxt->RemapYAddr.Addr[31]       = (0x0 << 5) | 31;

    pISPctxt->RemapUVAddr.Addr[0]       = (0x0 << 5) | 0;
    pISPctxt->RemapUVAddr.Addr[1]       = (0x0 << 5) | 1;
    pISPctxt->RemapUVAddr.Addr[2]       = (0x0 << 5) | 2;
    pISPctxt->RemapUVAddr.Addr[3]       = (0x0 << 5) | 11;
    pISPctxt->RemapUVAddr.Addr[4]       = (0x0 << 5) | 12;
    pISPctxt->RemapUVAddr.Addr[5]       = (0x0 << 5) | 13;
    pISPctxt->RemapUVAddr.Addr[6]       = (0x0 << 5) | 14;
    pISPctxt->RemapUVAddr.Addr[7]       = (0x0 << 5) | 3;
    pISPctxt->RemapUVAddr.Addr[8]       = (0x0 << 5) | 4;
    pISPctxt->RemapUVAddr.Addr[9]       = (0x0 << 5) | 5;
    pISPctxt->RemapUVAddr.Addr[10]      = (0x0 << 5) | 15;
    pISPctxt->RemapUVAddr.Addr[11]      = (0x0 << 5) | 6;
    pISPctxt->RemapUVAddr.Addr[12]      = (0x0 << 5) | 16;
    pISPctxt->RemapUVAddr.Addr[13]      = (0x0 << 5) | 7;
    pISPctxt->RemapUVAddr.Addr[14]      = (0x0 << 5) | 8;
    pISPctxt->RemapUVAddr.Addr[15]      = (0x0 << 5) | 9;
    pISPctxt->RemapUVAddr.Addr[16]      = (0x0 << 5) | 10;
    pISPctxt->RemapUVAddr.Addr[17]      = (0x0 << 5) | 17;
    pISPctxt->RemapUVAddr.Addr[18]      = (0x0 << 5) | 18;
    pISPctxt->RemapUVAddr.Addr[19]      = (0x0 << 5) | 19;
    pISPctxt->RemapUVAddr.Addr[20]      = (0x0 << 5) | 20;
    pISPctxt->RemapUVAddr.Addr[21]      = (0x0 << 5) | 21;
    pISPctxt->RemapUVAddr.Addr[22]      = (0x0 << 5) | 22;
    pISPctxt->RemapUVAddr.Addr[23]      = (0x0 << 5) | 23;
    pISPctxt->RemapUVAddr.Addr[24]      = (0x0 << 5) | 24;
    pISPctxt->RemapUVAddr.Addr[25]      = (0x0 << 5) | 25;
    pISPctxt->RemapUVAddr.Addr[26]      = (0x0 << 5) | 26;
    pISPctxt->RemapUVAddr.Addr[27]      = (0x0 << 5) | 27;
    pISPctxt->RemapUVAddr.Addr[28]      = (0x0 << 5) | 28;
    pISPctxt->RemapUVAddr.Addr[29]      = (0x0 << 5) | 29;
    pISPctxt->RemapUVAddr.Addr[30]      = (0x0 << 5) | 30;
    pISPctxt->RemapUVAddr.Addr[31]      = (0x0 << 5) | 31;
#else // pitch = 1024
    pISPctxt->RemapYAddr.Addr[0]        = (0x0 << 5) | 0;
    pISPctxt->RemapYAddr.Addr[1]        = (0x0 << 5) | 1;
    pISPctxt->RemapYAddr.Addr[2]        = (0x0 << 5) | 2;
    pISPctxt->RemapYAddr.Addr[3]        = (0x0 << 5) | 10;
    pISPctxt->RemapYAddr.Addr[4]        = (0x0 << 5) | 11;
    pISPctxt->RemapYAddr.Addr[5]        = (0x0 << 5) | 12;
    pISPctxt->RemapYAddr.Addr[6]        = (0x0 << 5) | 13;
    pISPctxt->RemapYAddr.Addr[7]        = (0x0 << 5) | 3;
    pISPctxt->RemapYAddr.Addr[8]        = (0x0 << 5) | 4;
    pISPctxt->RemapYAddr.Addr[9]        = (0x0 << 5) | 5;
    pISPctxt->RemapYAddr.Addr[10]       = (0x0 << 5) | 14;
    pISPctxt->RemapYAddr.Addr[11]       = (0x0 << 5) | 6;
    pISPctxt->RemapYAddr.Addr[12]       = (0x0 << 5) | 15;
    pISPctxt->RemapYAddr.Addr[13]       = (0x0 << 5) | 7;
    pISPctxt->RemapYAddr.Addr[14]       = (0x0 << 5) | 8;
    pISPctxt->RemapYAddr.Addr[15]       = (0x0 << 5) | 9;
    pISPctxt->RemapYAddr.Addr[16]       = (0x0 << 5) | 16;
    pISPctxt->RemapYAddr.Addr[17]       = (0x0 << 5) | 17;
    pISPctxt->RemapYAddr.Addr[18]       = (0x0 << 5) | 18;
    pISPctxt->RemapYAddr.Addr[19]       = (0x0 << 5) | 19;
    pISPctxt->RemapYAddr.Addr[20]       = (0x0 << 5) | 20;
    pISPctxt->RemapYAddr.Addr[21]       = (0x0 << 5) | 21;
    pISPctxt->RemapYAddr.Addr[22]       = (0x0 << 5) | 22;
    pISPctxt->RemapYAddr.Addr[23]       = (0x0 << 5) | 23;
    pISPctxt->RemapYAddr.Addr[24]       = (0x0 << 5) | 24;
    pISPctxt->RemapYAddr.Addr[25]       = (0x0 << 5) | 25;
    pISPctxt->RemapYAddr.Addr[26]       = (0x0 << 5) | 26;
    pISPctxt->RemapYAddr.Addr[27]       = (0x0 << 5) | 27;
    pISPctxt->RemapYAddr.Addr[28]       = (0x0 << 5) | 28;
    pISPctxt->RemapYAddr.Addr[29]       = (0x0 << 5) | 29;
    pISPctxt->RemapYAddr.Addr[30]       = (0x0 << 5) | 30;
    pISPctxt->RemapYAddr.Addr[31]       = (0x0 << 5) | 31;

    pISPctxt->RemapUVAddr.Addr[0]       = (0x0 << 5) | 0;
    pISPctxt->RemapUVAddr.Addr[1]       = (0x0 << 5) | 1;
    pISPctxt->RemapUVAddr.Addr[2]       = (0x0 << 5) | 2;
    pISPctxt->RemapUVAddr.Addr[3]       = (0x0 << 5) | 10;
    pISPctxt->RemapUVAddr.Addr[4]       = (0x0 << 5) | 11;
    pISPctxt->RemapUVAddr.Addr[5]       = (0x0 << 5) | 12;
    pISPctxt->RemapUVAddr.Addr[6]       = (0x0 << 5) | 13;
    pISPctxt->RemapUVAddr.Addr[7]       = (0x0 << 5) | 3;
    pISPctxt->RemapUVAddr.Addr[8]       = (0x0 << 5) | 4;
    pISPctxt->RemapUVAddr.Addr[9]       = (0x0 << 5) | 5;
    pISPctxt->RemapUVAddr.Addr[10]      = (0x0 << 5) | 14;
    pISPctxt->RemapUVAddr.Addr[11]      = (0x0 << 5) | 6;
    pISPctxt->RemapUVAddr.Addr[12]      = (0x0 << 5) | 15;
    pISPctxt->RemapUVAddr.Addr[13]      = (0x0 << 5) | 7;
    pISPctxt->RemapUVAddr.Addr[14]      = (0x0 << 5) | 8;
    pISPctxt->RemapUVAddr.Addr[15]      = (0x0 << 5) | 9;
    pISPctxt->RemapUVAddr.Addr[16]      = (0x0 << 5) | 16;
    pISPctxt->RemapUVAddr.Addr[17]      = (0x0 << 5) | 17;
    pISPctxt->RemapUVAddr.Addr[18]      = (0x0 << 5) | 18;
    pISPctxt->RemapUVAddr.Addr[19]      = (0x0 << 5) | 19;
    pISPctxt->RemapUVAddr.Addr[20]      = (0x0 << 5) | 20;
    pISPctxt->RemapUVAddr.Addr[21]      = (0x0 << 5) | 21;
    pISPctxt->RemapUVAddr.Addr[22]      = (0x0 << 5) | 22;
    pISPctxt->RemapUVAddr.Addr[23]      = (0x0 << 5) | 23;
    pISPctxt->RemapUVAddr.Addr[24]      = (0x0 << 5) | 24;
    pISPctxt->RemapUVAddr.Addr[25]      = (0x0 << 5) | 25;
    pISPctxt->RemapUVAddr.Addr[26]      = (0x0 << 5) | 26;
    pISPctxt->RemapUVAddr.Addr[27]      = (0x0 << 5) | 27;
    pISPctxt->RemapUVAddr.Addr[28]      = (0x0 << 5) | 28;
    pISPctxt->RemapUVAddr.Addr[29]      = (0x0 << 5) | 29;
    pISPctxt->RemapUVAddr.Addr[30]      = (0x0 << 5) | 30;
    pISPctxt->RemapUVAddr.Addr[31]      = (0x0 << 5) | 31;
#endif
    //Run-Length Encoder
    pISPctxt->RunLenEnc.Enable          = MMP_FALSE;
    pISPctxt->RunLenEnc.EnableRejectBit = MMP_FALSE;
    pISPctxt->RunLenEnc.UnitSize        = 0;
    pISPctxt->RunLenEnc.RunSize         = 8 - 3;

    //Frame Function
    pISPctxt->FrameFun0.Enable          = MMP_FALSE;

    //ISP Interrupt
    pISPctxt->EnableInterrupt           = MMP_FALSE;
    pISPctxt->InterruptMode             = 0x0;

    pISPctxt->UpdateFlags               = 0xFFFFFFFF;

    //Initial Video Weight Matrix
    for (index = 0; index < WEIGHT_NUM; index++)
    {
        _ISP_CreateWeighting(pISPctxt, ScaleRatio[index], ISP_SCALE_TAP, ISP_SCALE_TAP_SIZE, WeightMat);

        for (j = 0; j < ISP_SCALE_TAP_SIZE; j++)
            for (i = 0; i < ISP_SCALE_TAP; i++)
                WeightMatInt[index][j][i] = (MMP_UINT8)ISP_FloatToFix(WeightMat[j][i], 1, 6);
    }

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return result;
}

//=============================================================================
/**
 * Update ISP device.
 *
 * @return ISP_RESULT_SUCCESS if succeed, error codes of ISP_RESULT_ERROR otherwise.
 */
//=============================================================================
ISP_RESULT
ISP_Update(
    ISP_DEVICE ptDev)
{
    ISP_RESULT  result    = ISP_SUCCESS;
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    if (pISPctxt == MMP_NULL)
    {
        result = ISP_ERR_NOT_INITIALIZE;
        goto end;
    }

    //video windows parameter
    if (pISPctxt->ScaleFun.DstWidth == 0 || pISPctxt->ScaleFun.DstHeight == 0)
    {
        pISPctxt->ScaleFun.DstPosX   = 0;
        pISPctxt->ScaleFun.DstPosY   = 0;
        pISPctxt->ScaleFun.DstWidth = pISPctxt->OutInfo.Width;
        pISPctxt->ScaleFun.DstHeight = pISPctxt->OutInfo.Height;
    }

    if ((pISPctxt->ScaleFun.DstPosX + pISPctxt->ScaleFun.DstWidth) > pISPctxt->OutInfo.Width)
    {
        result = ISP_ERR_INVALID_PARAM;
        goto end;
    }

    if ((pISPctxt->ScaleFun.DstPosY + pISPctxt->ScaleFun.DstHeight) > pISPctxt->OutInfo.Height)
    {
        result = ISP_ERR_INVALID_PARAM;
        goto end;
    }

#if 0
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_RunLenEnc)
    {
        pISPctxt->RunLenEnc.EnableRejectBit = MMP_TRUE;

        if (pISPctxt->OutInfo.RGBFormat == NoDither888)
        {
            pISPctxt->RunLenEnc.UnitSize = 1;
            pISPctxt->RunLenEnc.MaxBit   = 4 * pISPctxt->OutInfo.Width * pISPctxt->OutInfo.Height * 8;
        }
        else
        {
            pISPctxt->RunLenEnc.UnitSize = 0;
            pISPctxt->RunLenEnc.MaxBit   = 2 * pISPctxt->OutInfo.Width * pISPctxt->OutInfo.Height * 8;
        }
    }
#endif

    //Set PreScaling Parameter in Line Mode
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_InputBuf)
    {
        //Horizontal Pre-scaling
        if (pISPctxt->InInfo.SrcWidth > ISP_SCALE_MAX_LINE_BUFF_LEN)
            pISPctxt->PreScaleFun.DstWidth = ISP_SCALE_MAX_LINE_BUFF_LEN;
        else
            pISPctxt->PreScaleFun.DstWidth = pISPctxt->InInfo.SrcWidth;

        pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_PreScaleParam;
    }
    else
        pISPctxt->UpdateFlags &= (~ISP_FLAGS_UPDATE_PreScaleParam);

    //Update ISP Scale Parameter
    if ((pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_InputBuf) ||
        (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_OutParameter) ||
        (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_PreScaleParam))
    {
        pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_ScaleParam;
    }

    // Update ISP hardware register
    if (pISPctxt->UpdateFlags)
        _ISP_UpdateHwReg(pISPctxt);

    // Clear Update Flags
    pISPctxt->UpdateFlags = 0;

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return result;
}

//=============================================================================
/**
    Typical settings:
      angle: default is 0, range is 0 ~ 359
      saturation: default is 128, range is 0 ~ 255
      contrast: default is 0, range is -64 ~ 63
      midPt: default is 128
      color_op: 0 is no color operation, 1 ~ 6 mapping to R G B C M Y effect
      rgb_gain_tbl[3]: default is {1.0, 1.0, 1.0} in display function
      brightness: default is 0, range is -64 ~ 63
      colorTemp: 0 is no color temperature adjustment, range is 4000 ~ 10000
      CT_Table: a piece-wised contineous table for look-up a R/G/B gain for a color temperature
        CT_Num: entry number
        CT: color temperature
        R_gain/G_gain/B_gain: R/G/B gain for the corresponding color temperature
        if colorTemp = 0, the CT_Table can be set to NULL

    Note:
      1. for sepia effect, select a desired "color_op" and set "saturation" to zero.
      2. if "contrast", "brightness" and "color_op" are enabled simultaneously, there would be
         un-predictable effect occured.
      3. use the "angle" setting if you know what are you doing.
 */
//=============================================================================
ISP_RESULT
ISP_SetColorCorrMatrix(
    void      *matrix,
    MMP_INT16 angle,
    MMP_INT16 saturation,
    MMP_INT16 contrast,
    MMP_INT16 midPt,
    MMP_INT16 color_op,
    MMP_INT16 brightness,
    MMP_BOOL  useyuvformat)
{
    EV_MATRIX            CC_MatrixDefault;
    EV_MATRIX            CC_MatrixCurrect;
    MMP_FLOAT            *colorMatrix      = (MMP_FLOAT *)matrix;
    MMP_UINT16           ui16CCRegTemp[12] = {
        256,           0,   0,
        0,           256,   0,
        0,             0, 256,
        0,             0,   0
    };

    MMP_INT16            i, j;

    MMP_FLOAT            rgb_gain_tbl[3]       = {1.0f, 1.0f, 1.0f};
    MMP_INT16            colorTemp             = 0;
    MMP_ISP_CT_TableType *CT_Table             = MMP_NULL;

    MMP_INT16            color_variation[7][2] = {
        {  0,   0 },                //  %0: Black
        { 15,  -5 },                //  %1: R
        {-15, -15 },                //  %2: G
        {  4,  -6 },                //  %3: B
        {-15,  +5 },                //  %4: C
        {+15, +15 },                //  %5: M
        { +8, -16 }
    };                              //  %6: Y

    MMP_FLOAT            M_ycrb2rgb[3][3];
    MMP_FLOAT            M_rgb2ycrb[3][3];

    MMP_FLOAT            M_contrast[3][3]          = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    MMP_FLOAT            M_hsv_angle[3][3]         = {{1, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    MMP_FLOAT            M_hsv_saturation[3][3]    = {{1, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    MMP_FLOAT            M_color_temperature[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};

    MMP_FLOAT            V_contrast[3][1];
    MMP_FLOAT            V1[3][1], V2[3][1], V3[3][1];
    MMP_FLOAT            V_hsv_sepia[3][1];
    MMP_FLOAT            V_brightness[3][1];

    MMP_FLOAT            M[3][3];

    MMP_FLOAT            M_temp1[3][3], M_temp2[3][3];

    MMP_FLOAT            AS, adj;
    MMP_INT16            precision = 8;
    MMP_FLOAT            Acr, Acb;
    MMP_FLOAT            a, b;

    useyuvformat = MMP_TRUE;
    if (useyuvformat)
    {
        M_ycrb2rgb[0][0] = 1.0f;
        M_ycrb2rgb[0][1] = 0.0f;
        M_ycrb2rgb[0][2] = 0.0f;
        M_ycrb2rgb[1][0] = 0.0f;
        M_ycrb2rgb[1][1] = 1.0f;
        M_ycrb2rgb[1][2] = 0.0f;
        M_ycrb2rgb[2][0] = 0.0f;
        M_ycrb2rgb[2][1] = 0.0f;
        M_ycrb2rgb[2][2] = 1.0f;

        M_rgb2ycrb[0][0] = 1.0f;
        M_rgb2ycrb[0][1] = 0.0f;
        M_rgb2ycrb[0][2] = 0.0f;
        M_rgb2ycrb[1][0] = 0.0f;
        M_rgb2ycrb[1][1] = 1.0f;
        M_rgb2ycrb[1][2] = 0.0f;
        M_rgb2ycrb[2][0] = 0.0f;
        M_rgb2ycrb[2][1] = 0.0f;
        M_rgb2ycrb[2][2] = 1.0f;
    }
    else
    {
        M_ycrb2rgb[0][0] = 1.0f;
        M_ycrb2rgb[0][1] = 1.402f;
        M_ycrb2rgb[0][2] = 0.0f;
        M_ycrb2rgb[1][0] = 1.0f;
        M_ycrb2rgb[1][1] = -0.714136f;
        M_ycrb2rgb[1][2] = -0.344136f;
        M_ycrb2rgb[2][0] = 1.0f;
        M_ycrb2rgb[2][1] = 0.0f;
        M_ycrb2rgb[2][2] = 1.772f;

        M_rgb2ycrb[0][0] = 0.299f;
        M_rgb2ycrb[0][1] = 0.587f;
        M_rgb2ycrb[0][2] = 0.114f;
        M_rgb2ycrb[1][0] = 0.5f;
        M_rgb2ycrb[1][1] = -0.418688f;
        M_rgb2ycrb[1][2] = -0.081312f;
        M_rgb2ycrb[2][0] = -0.168736f;
        M_rgb2ycrb[2][1] = -0.331264f;
        M_rgb2ycrb[2][2] = 0.5f;
    }

    if (color_op != 0)
    {
        if (contrast < -48)
            contrast = -48;
        else if (contrast > 47)
            contrast = 47;
        if (brightness < -48)
            brightness = -48;
        else if (brightness > 47)
            brightness = 47;
    }

    adj                    = (MMP_FLOAT) contrast;
    AS                     = (MMP_FLOAT)(saturation / 128.0);

    Acr                    = (MMP_FLOAT) color_variation[color_op][0];
    Acb                    = (MMP_FLOAT) color_variation[color_op][1];

    a                      = (MMP_FLOAT)  cos(angle / 180.0 * 3.1516159);
    b                      = (MMP_FLOAT) -sin(angle / 180.0 * 3.1516159);

    M_contrast[0][0]       = (MMP_FLOAT) (1.0 + adj / 128.0f);
    M_contrast[1][1]       = (MMP_FLOAT) (1.0 + adj / 128.0f);
    M_contrast[2][2]       = (MMP_FLOAT) (1.0 + adj / 128.0f);

    V_contrast[0][0]       = (MMP_FLOAT) (-midPt * adj / 128.0f);
    V_contrast[1][0]       = (MMP_FLOAT) (-midPt * adj / 128.0f);
    V_contrast[2][0]       = (MMP_FLOAT) (-midPt * adj / 128.0f);

    M_hsv_angle[1][1]      = a;
    M_hsv_angle[1][2]      = -b;
    M_hsv_angle[2][1]      = b;
    M_hsv_angle[2][2]      = a;

    M_hsv_saturation[1][1] = AS;
    M_hsv_saturation[2][2] = AS;

    V_hsv_sepia[0][0]      = 0.0;
    V_hsv_sepia[1][0]      = Acr;
    V_hsv_sepia[2][0]      = Acb;

    V_brightness[0][0]     = V_brightness[1][0] = V_brightness[2][0] = (MMP_FLOAT)brightness * 256;

    // interpolation for color temperature gain
    if (colorTemp == 0 || CT_Table == MMP_NULL)
    {
        M_color_temperature[0][0] = 1.0;
        M_color_temperature[1][1] = 1.0;
        M_color_temperature[2][2] = 1.0;
    }
    else if (colorTemp <= CT_Table->CT[0])
    {
        M_color_temperature[0][0] = CT_Table->R_gain[0];
        M_color_temperature[1][1] = CT_Table->G_gain[0];
        M_color_temperature[2][2] = CT_Table->B_gain[0];
    }
    else if (colorTemp >= CT_Table->CT[CT_Table->CT_Num - 1])
    {
        M_color_temperature[0][0] = CT_Table->R_gain[CT_Table->CT_Num - 1];
        M_color_temperature[1][1] = CT_Table->G_gain[CT_Table->CT_Num - 1];
        M_color_temperature[2][2] = CT_Table->B_gain[CT_Table->CT_Num - 1];
    }
    else
    {
        MMP_INT   distL, distH, dist;
        MMP_FLOAT newR_gain, newG_gain, newB_gain, maxValue;

        for (i = 1; i < CT_Table->CT_Num; i++)
        {
            if (colorTemp < CT_Table->CT[i])
                break;
        }
        dist                      = CT_Table->CT[i] - CT_Table->CT[i - 1];
        distL                     = colorTemp - CT_Table->CT[i - 1];
        distH                     = CT_Table->CT[i] - colorTemp;
        newR_gain                 = (distH * CT_Table->R_gain[i - 1] + distL * CT_Table->R_gain[i]) / dist;
        newG_gain                 = (distH * CT_Table->G_gain[i - 1] + distL * CT_Table->G_gain[i]) / dist;
        newB_gain                 = (distH * CT_Table->B_gain[i - 1] + distL * CT_Table->B_gain[i]) / dist;
        maxValue                  = _MAX3(newR_gain, newG_gain, newB_gain);
        // normalize
        M_color_temperature[0][0] = newR_gain / maxValue;
        M_color_temperature[1][1] = newG_gain / maxValue;
        M_color_temperature[2][2] = newB_gain / maxValue;
    }

    i = 0;
    while (i < 12)
    {
        if (i < 9)
        {
            //Read Matrix
            if ( (ui16CCRegTemp[i] & 0x1000) )
            {
                //Negative
                ui16CCRegTemp[i]                          = ((~ui16CCRegTemp[i]) & 0x0FFF) + 1;
                CC_MatrixDefault.M_color_correction[0][i] = ((MMP_FLOAT)ui16CCRegTemp[i] / 256.0f) * (-1.0f);
            }
            else
            {
                //Positive
                CC_MatrixDefault.M_color_correction[0][i] = (MMP_FLOAT)ui16CCRegTemp[i] / 256.0f;
            }
        }
        else
        {
            //Read Vector
            if (ui16CCRegTemp[i] & 0x0010)
            {
                // -ve
                ui16CCRegTemp[i]                              = ((~ui16CCRegTemp[i]) & 0x007F) + 1;
                CC_MatrixDefault.V_color_correction[i - 9][0] = (MMP_FLOAT)ui16CCRegTemp[i];
            }
            else
            {
                // +ve
                CC_MatrixDefault.V_color_correction[i - 9][0] = (MMP_FLOAT)ui16CCRegTemp[i];
            }
        }

        ++i;
    }

    //Calculate M
    matrix_multiply_33(M_ycrb2rgb, M_hsv_angle,      M_temp1);
    matrix_multiply_33(M_temp1,    M_hsv_saturation, M_temp2);
    matrix_multiply_33(M_temp2,    M_rgb2ycrb,       M);
    //V1
    matrix_multiply_33(M,          M_contrast,       M_temp1);
    matrix_multiply_31(M_temp1,    CC_MatrixDefault.V_color_correction, V1);
    //V2
    matrix_multiply_31(M,          V_contrast,                          V2);
    //V3
    matrix_multiply_31(M_ycrb2rgb, V_hsv_sepia,                         V3);

    for (i = 0; i < 3; i++)
    {
        CC_MatrixCurrect.V_color_correction[i][0] = (MMP_FLOAT) ((MMP_INT32)(V1[i][0] + V2[i][0] + V3[i][0] + V_brightness[i][0] + 0.0f));
    }

    matrix_multiply_33(M,       M_contrast,                          M_temp1);
    matrix_multiply_33(M_temp1, M_color_temperature,                 M_temp2);
    matrix_multiply_33(M_temp2, CC_MatrixDefault.M_color_correction, CC_MatrixCurrect.M_color_correction);
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            CC_MatrixCurrect.M_color_correction[i][j] = (MMP_FLOAT) (((MMP_INT32)(CC_MatrixCurrect.M_color_correction[i][j] * pow(2, precision) + 0.5f)) / pow(2, precision));
        }
    }

    i = 0;
    while (i < 12)
    {
        if (i < 9)
        {
            //Color Matrix
            if (CC_MatrixCurrect.M_color_correction[0][i] >= 0)
            {
                if ((i == 0) || (i == 4) || (i == 8))
                {
                    ui16CCRegTemp[i] = (MMP_UINT16) (CC_MatrixCurrect.M_color_correction[0][i] * 256 * rgb_gain_tbl[i / 3]);  // For Tuning API (2005/06/23)
                }
                else
                {
                    ui16CCRegTemp[i] = (MMP_UINT16) (CC_MatrixCurrect.M_color_correction[0][i] * 256);
                }
            }
            else
            {
                ui16CCRegTemp[i] = (MMP_UINT16) (CC_MatrixCurrect.M_color_correction[0][i] * -256);
                ui16CCRegTemp[i] = ((~(ui16CCRegTemp[i] - 1)) & 0x1FFF);
            }
            colorMatrix[i] = (MMP_FLOAT)((MMP_FLOAT)ui16CCRegTemp[i] / 256.0f);
        }
        else
        {
            //Write Vector (Vector values are signed numers)
            if (CC_MatrixCurrect.M_color_correction[0][i] >= 0)
            {
                ui16CCRegTemp[i] = (MMP_UINT16) (CC_MatrixCurrect.V_color_correction[i - 9][0]);
                colorMatrix[i]   = (MMP_FLOAT)((MMP_FLOAT)ui16CCRegTemp[i] / 256.0f);
            }
            else
            {
                ui16CCRegTemp[i] = (MMP_UINT16) (256 * 256 - CC_MatrixCurrect.V_color_correction[i - 9][0]);
                colorMatrix[i]   = (MMP_FLOAT)((MMP_FLOAT)ui16CCRegTemp[i] / 256.0f * -1);
            }
        }

        ++i;
    }

    return 0;
}

//=============================================================================
/**
 * Set isp input format.
 **/
//=============================================================================
ISP_RESULT
ISP_SetInputFormat(
    ISP_DEVICE       ptDev,
    MMP_ISP_INFORMAT format)
{
    ISP_RESULT  result    = ISP_SUCCESS;
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    if (pISPctxt == MMP_NULL)
    {
        result = ISP_ERR_NOT_INITIALIZE;
        goto end;
    }

    switch (format)
    {
    case MMP_ISP_IN_YUV422:
        pISPctxt->InInfo.PlaneFormat = YUV422;
        pISPctxt->InInfo.EnableCSFun = MMP_TRUE;
        break;

    case MMP_ISP_IN_YUV420:
        pISPctxt->InInfo.PlaneFormat = YUV420;
        pISPctxt->InInfo.EnableCSFun = MMP_TRUE;
        break;

    case MMP_ISP_IN_YUV444:
        pISPctxt->InInfo.PlaneFormat = YUV422;
        pISPctxt->InInfo.EnableCSFun = MMP_TRUE;
        break;
    default:
        result                       = ISP_ERR_NO_MATCH_INPUT_FORMAT;
        break;
    }

    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_InputParameter;

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return result;
}

//=============================================================================
/**
 * Set isp output format.
 **/
//=============================================================================
ISP_RESULT
ISP_SetOutputFormat(
    ISP_DEVICE        ptDev,
    MMP_ISP_OUTFORMAT format)
{
    ISP_RESULT  result    = ISP_SUCCESS;
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    if (pISPctxt == MMP_NULL)
    {
        result = ISP_ERR_NOT_INITIALIZE;
        goto end;
    }

    switch (format)
    {
    case MMP_ISP_OUT_DITHER565:
        pISPctxt->OutInfo.RGBFormat = Dither565;
        break;

    case MMP_ISP_OUT_DITHER444:
        pISPctxt->OutInfo.RGBFormat = Dither444;
        break;

    case MMP_ISP_OUT_RGB888:
        pISPctxt->OutInfo.RGBFormat = NoDither888;
        break;

    default:
        result = ISP_ERR_NO_MATCH_OUTPUT_FORMAT;
        break;
    }

    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_OutParameter;

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return result;
}

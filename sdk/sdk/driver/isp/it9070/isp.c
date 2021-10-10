#include "isp_types.h"
#include "isp_hw.h"
#include "isp.h"
#include "isp_util.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================
#define _MAX(a, b) (((a) >= (b)) ? (a) : (b))
#define _MIN(a, b) (((a) >= (b)) ? (b) : (a))

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
MMP_UINT32 ispMsgOnFlag = 0x1;

/*
   Input   Output  Ratio(In/Out)
   1088    1080    1.007407407
   1920    1920    1
   1280    1920    0.666666667
   576     1080    0.533333333
   480     1080    0.444444444
   720     1920    0.375
   640     1920    0.333333333
   288     1080    0.266666667
   352     1920    0.183333333
 */
#define     WEIGHT_NUM 9
#if defined(CFG_LCD_ENABLE) && defined(CFG_LCD_HEIGHT) && defined(CFG_LCD_WIDTH) && !defined(CFG_LCD_MULTIPLE)
MMP_FLOAT ScaleRatio[WEIGHT_NUM] = {(1088.0f / CFG_LCD_HEIGHT), (1920.0f / CFG_LCD_WIDTH), (1280.0f / CFG_LCD_WIDTH), (576.0f / CFG_LCD_HEIGHT), (480.0f / CFG_LCD_HEIGHT), (720.0f / CFG_LCD_WIDTH), (640.0f / CFG_LCD_WIDTH), (288.0f / CFG_LCD_HEIGHT), (352.0f / CFG_LCD_WIDTH)};
#else
MMP_FLOAT ScaleRatio[WEIGHT_NUM] = {(1088.0f / 1080), (1920.0f / 1920), (1280.0f / 1920), (576.0f / 1080), (480.0f / 1080), (720.0f / 1920), (640.0f / 1920), (288.0f / 1080), (352.0f / 1920)};
#endif
// the values of WeightMatInt table only need to be cacluated once at initial time.
MMP_UINT8 WeightMatInt[WEIGHT_NUM][ISP_SCALE_TAP_SIZE][ISP_SCALE_TAP];

// Input 16 - 235 -> Output 0 - 255s
static const ISP_YUV_TO_RGB gtIspYuv2RgbColorMatrix_ForPlayVideoMode =
{
    // _11      // _12      // _13
    0x012A,     0x0000,     0x0199,
    // _21      // _22      // _23
    0x012A,     0x079C,     0x0730,
    // _31      // _32      // _33
    0x012A,     0x0205,     0x0000,
    // ConstR   // ConstG   // ConstB
    0x0322,     0x0087,     0x02EC,
};

static const ISP_RGB_TO_YUV gtIspRgb2YuvColorMatrix_ForPlayVideoMode =
{
    // _11      // _12      // _13
    0x004D,     0x0096,     0x001D,
    // _21      // _22      // _23
    0x03d4,     0x03a9,     0x0083,
    // _31      // _32      // _33
    0x0083,     0x0392,     0x03eb,
    // ConstY   // ConstU   // ConstV
    0x0000,     0x0080,     0x0080,
};

static const ISP_YUV_TO_RGB gtIspYuv2RgbColorMatrix_ForNonePlayVideoMode =
{
    // _11      // _12      // _13
    0x0100,     0x0000,     0x0167,
    // _21      // _22      // _23
    0x0100,     0x07A8,     0x0749,
    // _31      // _32      // _33
    0x0100,     0x01C6,     0x0000,
    // ConstR   // ConstG   // ConstB
    0x034D,     0x0089,     0x031E,
};

static const ISP_RGB_TO_YUV gtIspRgb2YuvColorMatrix_ForNonePlayVideoMode =
{
    // _11      // _12      // _13
    0x004D,     0x0096,     0x001D,
    // _21      // _22      // _23
    0x03d4,     0x03a9,     0x0083,
    // _31      // _32      // _33
    0x0083,     0x0392,     0x03eb,
    // ConstY   // ConstU   // ConstV
    0x0000,     0x0080,     0x0080,
};

//=============================================================================
//                              Private Function Definition
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
 * Calculate ISP Extern Source.
 */
//=============================================================================
static void
_ISP_CalExternSource(
    ISP_DEVICE ptDev)
{
    static MMP_FLOAT HCI;
    static MMP_FLOAT VCI;
    MMP_UINT16       TopExternPixel;
    MMP_UINT16       LeftExternPixel;
    MMP_UINT16       DownExternPixel;
    MMP_UINT16       RightExternPixel;
    ISP_CONTEXT      *pISPctxt = (ISP_CONTEXT *)ptDev;

    HCI                          = _ISP_ScaleFactor(pISPctxt->InInfo.SrcWidth, pISPctxt->ScaleFun.DstWidth);
    VCI                          = _ISP_ScaleFactor(pISPctxt->InInfo.SrcHeight, pISPctxt->ScaleFun.DstHeight);

    LeftExternPixel              = (MMP_UINT16)((MMP_FLOAT)pISPctxt->InInfo.DstExtedLeft * HCI);
    RightExternPixel             = (MMP_UINT16)((MMP_FLOAT)pISPctxt->InInfo.DstExtedRight * HCI);

    TopExternPixel               = (MMP_UINT16)((MMP_FLOAT)pISPctxt->InInfo.DstExtedTop * VCI);
    DownExternPixel              = (MMP_UINT16)((MMP_FLOAT)pISPctxt->InInfo.DstExtedDown * VCI);

    pISPctxt->InInfo.SrcPosX     = pISPctxt->InInfo.SrcExtedLeft + LeftExternPixel;
    pISPctxt->InInfo.SrcPosY     = pISPctxt->InInfo.SrcExtedTop + TopExternPixel;

    pISPctxt->InInfo.PanelWidth  = pISPctxt->InInfo.SrcWidth + pISPctxt->InInfo.SrcExtedLeft + pISPctxt->InInfo.SrcExtedRight +
                                   LeftExternPixel + RightExternPixel;

    pISPctxt->InInfo.PanelHeight = pISPctxt->InInfo.SrcHeight + pISPctxt->InInfo.SrcExtedTop + pISPctxt->InInfo.SrcExtedDown +
                                   TopExternPixel + DownExternPixel;
}

//=============================================================================
/**
 * Calculate ISP Subtitle Position.
 */
//=============================================================================
static void
_ISP_CalSubtitlePosition(
    ISP_DEVICE        ptDev,
    ISP_SUBTITLE_CTRL *pIspSubTitle)
{
    static MMP_UINT16 PreScrW = 0, PrePanelW = 0, PreScaleDstW = 0;
    static MMP_UINT16 PreScrH = 0, PrePanelH = 0, PreScaleDstH = 0;
    static MMP_FLOAT  InvHCI;
    static MMP_FLOAT  InvVCI;

    static MMP_FLOAT  HCI;
    static MMP_FLOAT  VCI;

    MMP_UINT16        FS_StartX;
    MMP_UINT16        FS_StartY;
    MMP_UINT16        FS_DstWidth;
    MMP_UINT16        FS_DstHeight;
    ISP_CONTEXT       *pISPctxt = (ISP_CONTEXT *)ptDev;

    //Full Screen Position
    if (PreScrW != pISPctxt->InInfo.SrcWidth || PreScaleDstW != pISPctxt->ScaleFun.DstWidth)
        InvHCI = (MMP_FLOAT)1.0 / _ISP_ScaleFactor(pISPctxt->InInfo.SrcWidth, pISPctxt->ScaleFun.DstWidth);

    if (PreScrH != pISPctxt->InInfo.SrcHeight || PreScaleDstH != pISPctxt->ScaleFun.DstHeight)
        InvVCI = (MMP_FLOAT)1.0 / _ISP_ScaleFactor(pISPctxt->InInfo.SrcHeight, pISPctxt->ScaleFun.DstHeight);

    FS_StartX    = (MMP_UINT16)((MMP_FLOAT)pIspSubTitle->StartX * InvHCI) & 0xFFFE;
    FS_DstWidth  = (MMP_UINT16)((MMP_FLOAT)pIspSubTitle->DstWidth * InvHCI) & 0xFFFE;

    FS_StartY    = (MMP_UINT16)((MMP_FLOAT)pIspSubTitle->StartY * InvVCI);
    FS_DstHeight = (MMP_UINT16)((MMP_FLOAT)pIspSubTitle->DstHeight * InvVCI);

    //New Display Type : 16:9 or 4:3 or Full screen
    if (PrePanelW != pISPctxt->InInfo.PanelWidth)
        HCI = _ISP_ScaleFactor(pISPctxt->InInfo.PanelWidth, pISPctxt->ScaleFun.DstWidth);

    if (PrePanelH != pISPctxt->InInfo.PanelHeight)
        VCI = _ISP_ScaleFactor(pISPctxt->InInfo.PanelHeight, pISPctxt->ScaleFun.DstHeight);

    pIspSubTitle->StartX    = (MMP_UINT16)((MMP_FLOAT)FS_StartX * HCI) & 0xFFFE;
    pIspSubTitle->DstWidth  = (MMP_UINT16)((MMP_FLOAT)FS_DstWidth * HCI) & 0xFFFE;
    pIspSubTitle->StartY    = (MMP_UINT16)((MMP_FLOAT)FS_StartY * VCI);
    pIspSubTitle->DstHeight = (MMP_UINT16)((MMP_FLOAT)FS_DstHeight * VCI);

    PreScrW                 = pISPctxt->InInfo.SrcWidth;
    PreScaleDstW            = pISPctxt->ScaleFun.DstWidth;
    PreScrH                 = pISPctxt->InInfo.SrcHeight;
    PreScaleDstH            = pISPctxt->ScaleFun.DstHeight;

    PrePanelW               = pISPctxt->InInfo.PanelWidth;
    PrePanelH               = pISPctxt->InInfo.PanelHeight;
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
    pDeInterlace->DeinterMode           = DEINTER2D;

    pDeInterlace->EnSrcBottomFieldFirst = 0;
    pDeInterlace->EnDeinterBottomField  = 1;

    pDeInterlace->EnChromaEdgeDetect    = 1;
    pDeInterlace->EnLummaEdgeDetect     = 1;
    pDeInterlace->EnSrcLPF              = 1;

    pDeInterlace->UVRepeatMode          = MMP_FALSE;

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
 * Calculate ISP Subtitle Scale Factor.
 */
//=============================================================================
static void
_ISP_CalSubtitleHCI_VCI(
    ISP_SUBTITLE_CTRL *pSubTitle)
{
    pSubTitle->HCI = _ISP_ScaleFactor(pSubTitle->SrcWidth, pSubTitle->DstWidth);
    pSubTitle->VCI = _ISP_ScaleFactor(pSubTitle->SrcHeight, pSubTitle->DstHeight);
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

    HCI = _ISP_ScaleFactor(pISPctxt->InInfo.PanelWidth, pISPctxt->PreScaleFun.DstWidth);

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

    HCI = (pISPctxt->EngineMode.EnableBlockMode)
          ? _ISP_ScaleFactor(pISPctxt->InInfo.SrcWidth, pISPctxt->ScaleFun.DstWidth)
          : _ISP_ScaleFactor(pISPctxt->PreScaleFun.DstWidth, pISPctxt->ScaleFun.DstWidth);

    VCI = (pISPctxt->EngineMode.EnableBlockMode)
          ? _ISP_ScaleFactor(pISPctxt->InInfo.SrcHeight, pISPctxt->ScaleFun.DstHeight)
          : _ISP_ScaleFactor(pISPctxt->InInfo.PanelHeight, pISPctxt->ScaleFun.DstHeight);

    if (HCI != pISPctxt->ScaleFun.HCI)
        pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_ScaleMatrixH;

    if (VCI != pISPctxt->ScaleFun.VCI)
        pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_ScaleMatrixV;

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
 * Transform coordinate for clipping window when display window rotating makes
 * original point change
 */
//=============================================================================
static void
_ISP_CalClipWndRange(
    ISP_OUTPUT_INFO  *pOutInfo,
    ISP_CLIP_FN_CTRL *pClipFun)
{
    MMP_UINT16 ClipStartX, ClipStartY;
    MMP_UINT16 ClipWidth, ClipHeight;

    ClipStartX = pClipFun->StartX;
    ClipStartY = pClipFun->StartY;
    ClipWidth  = pClipFun->Width;
    ClipHeight = pClipFun->Height;

    switch (pOutInfo->RotateType)
    {
    case Deg90:
        pClipFun->ClipLeft   = (MMP_UINT16)ClipStartY;
        pClipFun->ClipTop    = (MMP_UINT16)(pOutInfo->Width - ClipStartX - ClipWidth);
        pClipFun->ClipRight  = (MMP_UINT16)(ClipStartY + ClipHeight - 1);
        pClipFun->ClipBottom = (MMP_UINT16)(pOutInfo->Width - ClipStartX - 1);
        break;

    case Deg270:
        pClipFun->ClipLeft   = (MMP_UINT16)(pOutInfo->Height - ClipStartY - ClipHeight);
        pClipFun->ClipTop    = (MMP_UINT16)ClipStartX;
        pClipFun->ClipRight  = (MMP_UINT16)(pOutInfo->Height - ClipStartY - 1);
        pClipFun->ClipBottom = (MMP_UINT16)(ClipStartX + ClipWidth - 1);
        break;

    case Deg180:
        pClipFun->ClipLeft   = (MMP_UINT16)(pOutInfo->Width - ClipStartX - ClipWidth);
        pClipFun->ClipTop    = (MMP_UINT16)(pOutInfo->Height - ClipStartY - ClipHeight);
        pClipFun->ClipRight  = (MMP_UINT16)(pOutInfo->Width - ClipStartX - 1);
        pClipFun->ClipBottom = (MMP_UINT16)(pOutInfo->Height - ClipStartY - 1);
        break;

    case Mirror:
        pClipFun->ClipLeft   = (MMP_UINT16)(pOutInfo->Width - ClipStartX - ClipWidth);
        pClipFun->ClipTop    = (MMP_UINT16)ClipStartY;
        pClipFun->ClipRight  = (MMP_UINT16)(pOutInfo->Width - ClipStartX - 1);
        pClipFun->ClipBottom = (MMP_UINT16)(ClipStartY + ClipHeight - 1);
        break;

    case Flip:
        pClipFun->ClipLeft   = (MMP_UINT16)ClipStartX;
        pClipFun->ClipTop    = (MMP_UINT16)(pOutInfo->Height - ClipStartY - ClipHeight);
        pClipFun->ClipRight  = (MMP_UINT16)(ClipStartX + ClipWidth - 1);
        pClipFun->ClipBottom = (MMP_UINT16)(pOutInfo->Height - ClipStartY - 1);
        break;

    default:
        pClipFun->ClipLeft   = (MMP_UINT16)ClipStartX;
        pClipFun->ClipTop    = (MMP_UINT16)ClipStartY;
        pClipFun->ClipRight  = (MMP_UINT16)(ClipStartX + ClipWidth - 1);
        pClipFun->ClipBottom = (MMP_UINT16)(ClipStartY + ClipHeight - 1);
        break;
    }
}

//=============================================================================
//                              Public Function Definition
//=============================================================================
//=============================================================================
/**
 * ISP default value initialization.
 */
//=============================================================================
void
ISP_ContextInitialize(
    ISP_DEVICE ptDev)
{
    MMP_UINT16  i, j, index;
    MMP_FLOAT   WeightMat[ISP_SCALE_TAP_SIZE][ISP_SCALE_TAP];
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    if (pISPctxt == MMP_NULL)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, "NULL pointer !!");
        goto end;
    }

    isp_Memset((void *)pISPctxt, 0, sizeof(ISP_CONTEXT));

    // Source Extend
    pISPctxt->InInfo.PanelColorY = 0x10;
    pISPctxt->InInfo.PanelColorU = 0x80;
    pISPctxt->InInfo.PanelColorV = 0x80;
    pISPctxt->UpdateFlags       |= ISP_FLAGS_UPDATE_InputBuf;

    // Deinterlace Paramter
    _ISP_Deinter_Param(&pISPctxt->DeInterlace);
    _ISP_Deinter3D_Param(&pISPctxt->DeInterlace);
    _ISP_Deinter2D_Param(&pISPctxt->DeInterlace);
    pISPctxt->UpdateFlags      |= ISP_FLAGS_UPDATE_DeInterlaceParam;

    // Color Correction
    pISPctxt->CCFun._11         = 1.0f;
    pISPctxt->CCFun._12         = 0.0f;
    pISPctxt->CCFun._13         = 0.0f;
    pISPctxt->CCFun._21         = 0.0f;
    pISPctxt->CCFun._22         = 1.0f;
    pISPctxt->CCFun._23         = 0.0f;
    pISPctxt->CCFun._31         = 0.0f;
    pISPctxt->CCFun._32         = 0.0f;
    pISPctxt->CCFun._33         = 1.0f;
    pISPctxt->UpdateFlags      |= ISP_FLAGS_UPDATE_CCMatrix;
    pISPctxt->UpdateFlags      |= ISP_FLAGS_UPDATE_OutParameter;

    // PreScale
    pISPctxt->PreScaleFun.HCI   = 0.0f;
    pISPctxt->UpdateFlags      |= ISP_FLAGS_UPDATE_PreScaleParam;

    // Scale
    pISPctxt->ScaleFun.BGColorR = 0x00;
    pISPctxt->ScaleFun.BGColorG = 0x80;
    pISPctxt->ScaleFun.BGColorB = 0x80;
    pISPctxt->UpdateFlags      |= ISP_FLAGS_UPDATE_ScaleParam;

    // Frame Function RGB to YUV, Input-> 0-255 Output-> 0-255
    pISPctxt->FrmMatrix._11     = 0x004D;
    pISPctxt->FrmMatrix._12     = 0x0096;
    pISPctxt->FrmMatrix._13     = 0x001D;
    pISPctxt->FrmMatrix._21     = 0x03d4;
    pISPctxt->FrmMatrix._22     = 0x03a9;
    pISPctxt->FrmMatrix._23     = 0x0083;
    pISPctxt->FrmMatrix._31     = 0x0083;
    pISPctxt->FrmMatrix._32     = 0x0392;
    pISPctxt->FrmMatrix._33     = 0x03eb;
    pISPctxt->FrmMatrix.ConstY  = 0x0000;
    pISPctxt->FrmMatrix.ConstU  = 0x0080;
    pISPctxt->FrmMatrix.ConstV  = 0x0080;
    pISPctxt->UpdateFlags      |= ISP_FLAGS_UPDATE_FrmMatrix;

    // some parameter for color correction matrix.
    pISPctxt->hue               = 0;
    pISPctxt->saturation        = 128;
    pISPctxt->contrast          = 0;
    pISPctxt->midPoint          = 128;
    pISPctxt->colorEffect       = 0;
    pISPctxt->brightness        = 0;
    pISPctxt->sharp             = 0;

    ISP_SetColorMatrix(ptDev);

    // Output format
    pISPctxt->OutInfo.OutFormat   = RGBPacket;
    pISPctxt->OutInfo.RGBFormat   = Dither565;
    pISPctxt->UpdateFlags        |= ISP_FLAGS_UPDATE_OutParameter;

    // YUV to RGB
    pISPctxt->OutInfo.EnableCSFun = MMP_TRUE;
    pISPctxt->UpdateFlags        |= ISP_FLAGS_UPDATE_OutParameter;
    pISPctxt->top_field_first     = 1;

    // Initial Video Weight Matrix
    for (index = 0; index < WEIGHT_NUM; index++)
    {
        _ISP_CreateWeighting(pISPctxt, ScaleRatio[index], ISP_SCALE_TAP, ISP_SCALE_TAP_SIZE, WeightMat);

        for (j = 0; j < ISP_SCALE_TAP_SIZE; j++)
            for (i = 0; i < ISP_SCALE_TAP; i++)
                WeightMatInt[index][j][i] = (MMP_UINT8)ISP_FloatToFix(WeightMat[j][i], 1, 6);
    }

end:
    return;
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

    if (pISPctxt->ispMode == MMP_ISP_MODE_PLAY_VIDEO) //For MPEG Video
    {
        pISPctxt->YUV2RGBFun = &gtIspYuv2RgbColorMatrix_ForPlayVideoMode;
        pISPctxt->RGB2YUVFun = &gtIspRgb2YuvColorMatrix_ForPlayVideoMode;
    }
    else
    {
        pISPctxt->YUV2RGBFun = &gtIspYuv2RgbColorMatrix_ForNonePlayVideoMode;
        pISPctxt->RGB2YUVFun = &gtIspRgb2YuvColorMatrix_ForNonePlayVideoMode;
    }

    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_YUVtoRGBMatrix;
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
        256,       0,   0,
        0,       256,   0,
        0,         0, 256,
        0,         0,   0
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

    pISPctxt->InInfo.EnableRGB565         = MMP_FALSE;
    pISPctxt->InInfo.EnableRGB888         = MMP_FALSE;
    pISPctxt->InInfo.EnableYUVPackMode    = MMP_FALSE;
    pISPctxt->InInfo.EnableYUVPlaneMode   = MMP_FALSE;
    pISPctxt->InInfo.EnableNVMode         = MMP_FALSE;
    pISPctxt->InInfo.EnableRdRqDoubleLine = MMP_FALSE;

    switch (format)
    {
    case MMP_ISP_IN_RGB565:
        pISPctxt->InInfo.EnableRGB565 = MMP_TRUE;
        pISPctxt->OutInfo.EnableCSFun = MMP_FALSE;
        break;

    case MMP_ISP_IN_RGB888:
        pISPctxt->InInfo.EnableRGB888 = MMP_TRUE;
        pISPctxt->OutInfo.EnableCSFun = MMP_FALSE;
        break;

    case MMP_ISP_IN_YUYV:
        pISPctxt->InInfo.EnableYUVPackMode = MMP_TRUE;
        pISPctxt->InInfo.PacketFormat      = YUYV;
        pISPctxt->OutInfo.EnableCSFun      = MMP_TRUE;
        break;

    case MMP_ISP_IN_YVYU:
        pISPctxt->InInfo.EnableYUVPackMode = MMP_TRUE;
        pISPctxt->InInfo.PacketFormat      = YVYU;
        pISPctxt->OutInfo.EnableCSFun      = MMP_TRUE;
        break;

    case MMP_ISP_IN_UYVY:
        pISPctxt->InInfo.EnableYUVPackMode = MMP_TRUE;
        pISPctxt->InInfo.PacketFormat      = UYVY;
        pISPctxt->OutInfo.EnableCSFun      = MMP_TRUE;
        break;

    case MMP_ISP_IN_VYUY:
        pISPctxt->InInfo.EnableYUVPackMode = MMP_TRUE;
        pISPctxt->InInfo.PacketFormat      = VYUY;
        pISPctxt->OutInfo.EnableCSFun      = MMP_TRUE;
        break;

    case MMP_ISP_IN_YUV422:
        pISPctxt->InInfo.EnableYUVPlaneMode   = MMP_TRUE;
        pISPctxt->InInfo.PlaneFormat          = YUV422;
        pISPctxt->OutInfo.EnableCSFun         = MMP_TRUE;
        pISPctxt->InInfo.EnableRdRqDoubleLine = (pISPctxt->ispMode == MMP_ISP_MODE_PLAY_VIDEO);
        break;

    case MMP_ISP_IN_YUV420:
        pISPctxt->InInfo.EnableYUVPlaneMode   = MMP_TRUE;
        pISPctxt->InInfo.PlaneFormat          = YUV420;
        pISPctxt->OutInfo.EnableCSFun         = MMP_TRUE;
        pISPctxt->InInfo.EnableRdRqDoubleLine = (pISPctxt->ispMode == MMP_ISP_MODE_PLAY_VIDEO);
        break;

    case MMP_ISP_IN_YUV444:
        pISPctxt->InInfo.EnableYUVPlaneMode = MMP_TRUE;
        pISPctxt->InInfo.PlaneFormat        = YUV444;
        pISPctxt->OutInfo.EnableCSFun       = MMP_TRUE;
        break;

    case MMP_ISP_IN_YUV422R:
        pISPctxt->InInfo.EnableYUVPlaneMode = MMP_TRUE;
        pISPctxt->InInfo.PlaneFormat        = YUV422R;
        pISPctxt->OutInfo.EnableCSFun       = MMP_TRUE;
        break;

    case MMP_ISP_IN_NV12:
        pISPctxt->InInfo.EnableNVMode         = MMP_TRUE;
        pISPctxt->InInfo.NVFormat             = NV12;
        pISPctxt->OutInfo.EnableCSFun         = MMP_TRUE;
        pISPctxt->InInfo.EnableRdRqDoubleLine = (pISPctxt->ispMode == MMP_ISP_MODE_PLAY_VIDEO);
        break;

    case MMP_ISP_IN_NV21:
        pISPctxt->InInfo.EnableNVMode         = MMP_TRUE;
        pISPctxt->InInfo.NVFormat             = NV21;
        pISPctxt->OutInfo.EnableCSFun         = MMP_TRUE;
        pISPctxt->InInfo.EnableRdRqDoubleLine = (pISPctxt->ispMode == MMP_ISP_MODE_PLAY_VIDEO);
        break;

    default:
        result = ISP_ERR_NO_MATCH_INPUT_FORMAT;
        break;
    }

    pISPctxt->InInfo.EnableInYUV255Range   = MMP_FALSE;
    pISPctxt->OutInfo.EnableOutYUV235Range = MMP_FALSE;
    if (pISPctxt->EnableYUVProcess)
        pISPctxt->OutInfo.EnableCSFun = MMP_FALSE;

    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_InputParameter;
    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_OutParameter;

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

    // Adjust the Scaling Parameter
    if (pISPctxt->OutInfo.Width < (pISPctxt->ScaleFun.DstPosX + pISPctxt->ScaleFun.DstWidth))
    {
        if (pISPctxt->ScaleFun.DstPosX > pISPctxt->OutInfo.Width)
            pISPctxt->ScaleFun.DstPosX = pISPctxt->ScaleFun.DstWidth = 
            pISPctxt->ScaleFun.DstPosY = pISPctxt->ScaleFun.DstHeight = 0;
        else
            pISPctxt->ScaleFun.DstWidth = pISPctxt->OutInfo.Width - pISPctxt->ScaleFun.DstPosX;
        pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_ScaleParam;
    }
    if (pISPctxt->OutInfo.Height < (pISPctxt->ScaleFun.DstPosY + pISPctxt->ScaleFun.DstHeight))
    {
        if (pISPctxt->ScaleFun.DstPosY > pISPctxt->OutInfo.Height)
            pISPctxt->ScaleFun.DstPosX = pISPctxt->ScaleFun.DstWidth =
            pISPctxt->ScaleFun.DstPosY = pISPctxt->ScaleFun.DstHeight = 0;
        else
            pISPctxt->ScaleFun.DstHeight = pISPctxt->OutInfo.Height - pISPctxt->ScaleFun.DstPosY;
        pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_ScaleParam;
    }

    //Set Extern Source Parameter
    if (pISPctxt->OutInfo.DisbleVideoOut == MMP_FALSE)
    {
        if (pISPctxt->InInfo.DstExtedLeft != 0 || pISPctxt->InInfo.DstExtedRight != 0 ||
            pISPctxt->InInfo.DstExtedTop != 0 || pISPctxt->InInfo.DstExtedDown != 0)
            _ISP_CalExternSource(ptDev);
    }

    //Set PreScaling Parameter in Line Mode
    if (pISPctxt->EngineMode.EnableBlockMode == MMP_FALSE &&
        (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_InputBuf))
    {
        //Horizontal Pre-scaling
        pISPctxt->PreScaleFun.DstWidth = _MIN(pISPctxt->InInfo.PanelWidth, ISP_SCALE_MAX_LINE_BUFF_LEN);
        pISPctxt->UpdateFlags         |= ISP_FLAGS_UPDATE_PreScaleParam;
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
        ISP_UpdateHwReg(pISPctxt);

    // Clear Update Flags
    pISPctxt->UpdateFlags = 0;

end:

    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return result;
}

//=============================================================================
/**
 * ISP update hardware register.
 */
//=============================================================================
void
ISP_UpdateHwReg(
    ISP_DEVICE ptDev)
{
    MMP_UINT16  index;
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

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
        ISP_SetInputBuf_Reg(pISPctxt, &pISPctxt->InInfo);

    //
    //Input Address
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_InputAddr)
        ISP_SetInputAddr_Reg(pISPctxt, &pISPctxt->InInfo);

    //
    //Deinterlace Parameter
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_DeInterlaceParam)
        ISP_SetDeInterlaceParam_Reg(pISPctxt, &pISPctxt->DeInterlace);

    //
    //Subtitle 0 Parameter
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_SubTitle0)
    {
        _ISP_CalSubtitlePosition(pISPctxt, &pISPctxt->SubTitle0);
        _ISP_CalSubtitleHCI_VCI(&pISPctxt->SubTitle0);
        ISP_SetSubTitle0_Reg(&pISPctxt->SubTitle0);
    }

    //
    //Subtitle 1 Parameter
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_SubTitle1)
    {
        _ISP_CalSubtitlePosition(pISPctxt, &pISPctxt->SubTitle1);
        _ISP_CalSubtitleHCI_VCI(&pISPctxt->SubTitle1);
        ISP_SetSubTitle1_Reg(&pISPctxt->SubTitle1);
    }

    //
    //YUV to RGB Parameter
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_YUVtoRGBMatrix)
        ISP_SetYUVtoRGBMatrix_Reg(pISPctxt->YUV2RGBFun);

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
        //    _ISP_CreateWeighting(pISPctxt->PreScaleFun.HCI, ISP_SCALE_TAP, ISP_SCALE_TAP_SIZE, pISPctxt->PreScaleFun.WeightMatX);
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
    //Scale Horizontal Matrix
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_ScaleMatrixH)
    {
        //if(pISPctxt->ispMode == MMP_ISP_MODE_PLAY_VIDEO)
        {
            for (index = 0; index < WEIGHT_NUM; index++)
            {
                if (pISPctxt->ScaleFun.HCI >= ScaleRatio[index])
                {
                    ISP_SetIntScaleMatrixH_Reg(WeightMatInt[index]);
                    break;
                }
                else if (index == WEIGHT_NUM - 1)
                    ISP_SetIntScaleMatrixH_Reg(WeightMatInt[index]);
            }
        }
        //else
        //{
        //    _ISP_CreateWeighting(pISPctxt->ScaleFun.HCI, ISP_SCALE_TAP, ISP_SCALE_TAP_SIZE, pISPctxt->ScaleFun.WeightMatX);
        //    ISP_SetScaleMatrixH_Reg(&pISPctxt->ScaleFun);
        //}
    }

    //
    //Scale Vertical Matrix
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_ScaleMatrixV)
    {
        //if(pISPctxt->ispMode == MMP_ISP_MODE_PLAY_VIDEO)
        {
            for (index = 0; index < WEIGHT_NUM; index++)
            {
                if (pISPctxt->ScaleFun.VCI >= ScaleRatio[index])
                {
                    ISP_SetIntScaleMatrixV_Reg(WeightMatInt[index]);
                    break;
                }
                else if (index == WEIGHT_NUM - 1)
                    ISP_SetIntScaleMatrixV_Reg(WeightMatInt[index]);
            }
        }
        //else
        //{
        //    _ISP_CreateWeighting(pISPctxt->ScaleFun.VCI, ISP_SCALE_TAP, ISP_SCALE_TAP_SIZE, pISPctxt->ScaleFun.WeightMatY);
        //    ISP_SetScaleMatrixV_Reg(&pISPctxt->ScaleFun);
        //}
    }

    //
    // YUV to RGB Parameter
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_FrmMatrix)
        ISP_SetFrmMatrix_Reg(&pISPctxt->FrmMatrix);

    //
    // Frame Function 0
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_FrameFun0)
        ISP_SetFrameFun0_Reg(&pISPctxt->FrameFun0);

    //
    // Frame Function 1
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_FrameFun1)
        ISP_SetFrameFun1_Reg(&pISPctxt->FrameFun1);

    //
    // YUV to RGB Parameter
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_RGBtoYUVMatrix)
        ISP_SetRGBtoYUVMatrix_Reg(pISPctxt->RGB2YUVFun);

    //
    // Clip 0 Function
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_Clip0Fun)
    {
        _ISP_CalClipWndRange(&pISPctxt->OutInfo, &pISPctxt->ClipFun0);
        ISP_SetClip0Fun_Reg(&pISPctxt->ClipFun0);
    }

    //
    // Clip 1 Function
    //
    if ((pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_Clip1Fun) || (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_Mpeg2BufferIdx))
    {
        _ISP_CalClipWndRange(&pISPctxt->OutInfo, &pISPctxt->ClipFun1);
        ISP_SetClip1Fun_Reg(pISPctxt, &pISPctxt->ClipFun1);
    }

    //
    // Clip 2 Function
    //
    if ((pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_Clip2Fun) || (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_Mpeg2BufferIdx))
    {
        _ISP_CalClipWndRange(&pISPctxt->OutInfo, &pISPctxt->ClipFun2);
        ISP_SetClip2Fun_Reg(pISPctxt, &pISPctxt->ClipFun2);
    }

    //
    // Output Parameter
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_OutParameter)
        ISP_SetOutParameter_Reg(&pISPctxt->OutInfo);

    //
    // Output Width, Height and Pitch
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_OutBufInfo)
        ISP_SetOutBufInfo_Reg(&pISPctxt->OutInfo);

    //
    // Output Address
    //
    if (pISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_OutAddress)
        ISP_SetOutAddress_Reg(&pISPctxt->OutInfo);
}
#include "jpg_extern_link.h"
#include "jpg_common.h"
#include "jpg_defs.h"

#if !(ENABLE_JPG_SW_SIMULSTION)
    #include "mmp_isp.h"

    #if !defined(__arm__) && !(_MSC_VER)
//#include "or32.h"
    #endif
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static ISP_DEVICE gIspDev;
static int MjpegFullScreen;

//=============================================================================
//                  Private Function Definition
//=============================================================================
static JPG_ERR
_SetIspDecodeParam(
    JCOMM_HANDLE *pHJComm)
{
    JPG_ERR        result          = JPG_ERR_OK;
    int            ispRst          = ISP_SUCCESS;
    JPG_DISP_INFO  *pJDispInfo     = pHJComm->pJDispInfo;
    JPG_COLOR_CTRL *pJColorCtrl    = &pJDispInfo->colorCtrl;
    JPG_BUF_INFO   *pJOutBufInfo_0 = &pHJComm->jOutBufInfo[0];
    JPG_BUF_INFO   *pJOutBufInfo_1 = &pHJComm->jOutBufInfo[1];
    JPG_BUF_INFO   *pJOutBufInfo_2 = &pHJComm->jOutBufInfo[2];

    jpg_msg(0, "set to Isp out(x, y, w, h, pitch, outAddr, format, rotType) = (%d, %d, %d, %d, %d, 0x%x, %d, %d)\n",
            pJDispInfo->dstX, pJDispInfo->dstY, pJDispInfo->dstW, pJDispInfo->dstH,
            pJOutBufInfo_0->pitch, pJOutBufInfo_0->pBufAddr, pJDispInfo->outColorSpace, pJDispInfo->rotType);

    // set rotation
    switch (pJDispInfo->rotType)
    {
    default:
    case JPG_ROT_TYPE_0:
        mmpIspSetDisplayRotateType(gIspDev, MMP_ISP_ROTATE_0);
        break;
    case JPG_ROT_TYPE_90:
        mmpIspSetDisplayRotateType(gIspDev, MMP_ISP_ROTATE_90);
        break;
    case JPG_ROT_TYPE_180:
        mmpIspSetDisplayRotateType(gIspDev, MMP_ISP_ROTATE_180);
        break;
    case JPG_ROT_TYPE_270:
        mmpIspSetDisplayRotateType(gIspDev, MMP_ISP_ROTATE_270);
        break;
    }

    do
    {
        bool bSkip = false;

        //set display window or window of off-screen surface.
        if (pJOutBufInfo_0->pBufAddr)
        {
            MMP_ISP_OUTPUT_INFO outInfo;

            memset((void *)&outInfo, 0, sizeof(MMP_ISP_OUTPUT_INFO));
            outInfo.startX = pJDispInfo->dstX;
            outInfo.startY = pJDispInfo->dstY;
            outInfo.width  = pJDispInfo->dstW;
            outInfo.height = pJDispInfo->dstH;

			//delete rotate codes.
			if(MjpegFullScreen) //for Mjpeg handshake Decoder.
			{
				outInfo.startX = 0;
				outInfo.startY = 0;
				outInfo.width  = ithLcdGetWidth(); 
				outInfo.height = ithLcdGetHeight(); 
				MjpegFullScreen  = 0;
			}

            switch (pJDispInfo->outColorSpace)
            {
            case JPG_COLOR_SPACE_ARGB4444:
            case JPG_COLOR_SPACE_ARGB8888:
            case JPG_COLOR_SPACE_RGB565:
                outInfo.addrRGB  = (uint32_t)pJOutBufInfo_0->pBufAddr;
                outInfo.pitchRGB = pJOutBufInfo_0->pitch;
                outInfo.format   = (pJDispInfo->outColorSpace == JPG_COLOR_SPACE_RGB565)
                                   ? MMP_ISP_OUT_DITHER565 :
                                   ((pJDispInfo->outColorSpace == JPG_COLOR_SPACE_ARGB4444) ?
                                    MMP_ISP_OUT_DITHER444 : MMP_ISP_OUT_RGB888);

                //Benson
                mmpIspSetDisplayWindow(gIspDev, 0, 0, outInfo.width, outInfo.height);
                mmpIspSetVideoWindow(gIspDev, 0, 0, outInfo.width, outInfo.height);
                mmpIspSetOutputFormat(gIspDev, MMP_ISP_OUT_DITHER565);
                //end of benson
                break;

            case JPG_COLOR_SPACE_YUV422:
            case JPG_COLOR_SPACE_YUV420:
                outInfo.pitchY  = (uint16_t)pJOutBufInfo_0->pitch;
                outInfo.pitchUv = (uint16_t)pJOutBufInfo_1->pitch;
                outInfo.addrY   = (uint32_t)pJOutBufInfo_0->pBufAddr;
                outInfo.addrU   = (uint32_t)pJOutBufInfo_1->pBufAddr;
                outInfo.addrV   = (uint32_t)pJOutBufInfo_2->pBufAddr;
                outInfo.format  = (pJDispInfo->outColorSpace == JPG_COLOR_SPACE_YUV420)
                                  ? MMP_ISP_OUT_YUV420 : MMP_ISP_OUT_YUV422;
                printf(" isp output: y=0x%x, u=0x%x, v=0x%x, pitchY=%d, pitchUv=%d\n",
                       outInfo.addrY, outInfo.addrU, outInfo.addrV,
                       outInfo.pitchY, outInfo.pitchUv);
                break;

            default:
                jpg_msg_ex(JPG_MSG_TYPE_ERR, "Wrong output format !! ");
                result = JPG_ERR_INVALID_PARAMETER;
                bSkip  = true;
                break;
            }

            if (bSkip == true)
                break;

            ispRst = mmpIspSetOutputWindow(gIspDev, &outInfo);
            if (ispRst != ISP_SUCCESS)
            {
                jpg_msg_ex(JPG_MSG_TYPE_ERR, "mmpIspSetRgbOutputWindow() err (0x%x) !! ", ispRst);
                result = JPG_ERR_ISP_HW_FAIL;
                break;
            }
        }
        else
        {
            ispRst = mmpIspSetDisplayWindow(
                gIspDev,
                pJDispInfo->dstX,
                pJDispInfo->dstY,
                pJDispInfo->dstW,
                pJDispInfo->dstH);
            if (ispRst != ISP_SUCCESS)
            {
                jpg_msg_ex(JPG_MSG_TYPE_ERR, "mmpIspSetDisplayWindow() err (0x%x) !!", ispRst);
                result = JPG_ERR_ISP_HW_FAIL;
                break;
            }
        }

        // Set Color Control
        if (pJColorCtrl->bColorCtl == true)
        {
            if (pJColorCtrl->ctrlFlag & COLOR_COMP_HUE)        // Hue
            {
                ispRst = mmpIspSetColorCtrl(gIspDev, MMP_ISP_HUE, pJColorCtrl->hue);
                if (ispRst != ISP_SUCCESS)
                {
                    jpg_msg_ex(JPG_MSG_TYPE_ERR, "mmpIspSetColorControl( Hue ) err (0x%x) !! \n", ispRst);
                    result = JPG_ERR_ISP_HW_FAIL;
                    break;
                }
            }

            if (pJColorCtrl->ctrlFlag & COLOR_COMP_CONTRAST)        // Contrast
            {
                ispRst = mmpIspSetColorCtrl(gIspDev, MMP_ISP_CONTRAST, pJColorCtrl->contrast);
                if (ispRst != ISP_SUCCESS)
                {
                    jpg_msg_ex(JPG_MSG_TYPE_ERR, "mmpIspSetColorControl( Contrast ) err (0x%x) !! \n", ispRst);
                    result = JPG_ERR_ISP_HW_FAIL;
                    break;
                }
            }

            if (pJColorCtrl->ctrlFlag & COLOR_COMP_SATURATION)        // Saturation
            {
                ispRst = mmpIspSetColorCtrl(gIspDev, MMP_ISP_SATURATION, pJColorCtrl->saturation);
                if (ispRst != ISP_SUCCESS)
                {
                    jpg_msg_ex(JPG_MSG_TYPE_ERR, "mmpIspSetColorControl( Saturation ) err (0x%x) !! \n", ispRst);
                    result = JPG_ERR_ISP_HW_FAIL;
                    break;
                }
            }

            if (pJColorCtrl->ctrlFlag & COLOR_COMP_BRIGHTNESS)        // Brightness
            {
                ispRst = mmpIspSetColorCtrl(gIspDev, MMP_ISP_BRIGHTNESS, pJColorCtrl->brightness);
                if (ispRst != ISP_SUCCESS)
                {
                    jpg_msg_ex(JPG_MSG_TYPE_ERR, "mmpIspSetColorControl( Brightness ) err (0x%x) !! \n", ispRst);
                    result = JPG_ERR_ISP_HW_FAIL;
                    break;
                }
            }
        }
    } while (0);

    if (result != JPG_ERR_OK)
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);

    return result;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
JPG_ERR
Jpg_Ext_Link_Ctrl(
    JEL_CTRL_CMD cmd,
    uint32_t     *value,
    void         *extraData)
{
    JPG_ERR result = JPG_ERR_OK;
    int     ispRst = 0;

    switch (cmd)
    {
    case JEL_CTRL_ISP_INIT:
        mmpIspTerminate(&gIspDev);      // Benson
        jpg_sleep(1);
        ispRst = mmpIspInitialize(&gIspDev);
        if (ispRst != ISP_SUCCESS)
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, "mmpIspInitialize() fail (rst=0x%x) !!", ispRst);
            result = JPG_ERR_ISP_HW_FAIL;
            break;
        }

        switch ( (JEL_SET_ISP_MODE)value)
        {
        case JEL_SET_ISP_COLOR_TRANSFORM:
#if (CFG_CHIP_FAMILY == 9850)
			ispRst = mmpIspSetMode(gIspDev, MMP_ISP_MODE_JPEG_TRANSFORM);
#else
			ispRst = mmpIspSetMode(gIspDev, MMP_ISP_MODE_TRANSFORM);
#endif
            break;

        case JEL_SET_ISP_SHOW_IMAGE:
            ispRst = mmpIspSetMode(gIspDev, MMP_ISP_MODE_SHOW_IMAGE);
            break;

        default:
            ispRst = ISP_ERR_INVALID_PARAM;
            break;
        }

        if (ispRst != ISP_SUCCESS)
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, "mmpIspSetMode() fail (rst=0x%x) !!", ispRst);
            result = JPG_ERR_ISP_HW_FAIL;
        }
        break;

    case JEL_CTRL_ISP_TERMINATE:
        ispRst = mmpIspTerminate(&gIspDev);
        if (ispRst != ISP_SUCCESS)
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, "mmpIspTerminate() fail (rst=0x%x) !!", ispRst);
            result = JPG_ERR_ISP_HW_FAIL;
        }
        break;

    case JEL_CTRL_ISP_POWERDOWN:
        ispRst = mmpIspPowerdown(gIspDev);
        if (ispRst != ISP_SUCCESS)
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, "mmpIspPowerdown() fail (rst=0x%x) !!", ispRst);
            result = JPG_ERR_ISP_HW_FAIL;
        }
        break;

    case JEL_CTRL_ISP_SET_DISP_INFO:
        if (extraData)
        {
            JCOMM_HANDLE *pHJComm = (JCOMM_HANDLE *)extraData;
			MjpegFullScreen = 0;
			
			if(value) 
				MjpegFullScreen = 1;  //Powei says don`t fullscreen it.
			
            result = _SetIspDecodeParam(pHJComm);
        }
        break;

    case JEL_CTRL_ISP_HW_RESET:
        ispResetHwEngine();
        ispResetHwReg();
        break;

    case JEL_CTRL_ISP_WAIT_IDLE:
        ispRst = mmpIspWaitEngineIdle();
        if (ispRst != ISP_SUCCESS)
        {
            //jpg_msg_ex(JPG_MSG_TYPE_ERR, "mmpIspTerminate() fail (rst=0x%x) !!", ispRst);
            result = JPG_ERR_ISP_HW_FAIL;
        }
        break;

    case JEL_CTRL_ISP_IMG_PROC:
        if (extraData)
        {
            JPG_SHARE_DATA *pShare2Isp = (JPG_SHARE_DATA *)extraData;
            MMP_ISP_SHARE  ispShare    = {0};

            ispShare.width        = pShare2Isp->width;
            ispShare.height       = pShare2Isp->height;
            ispShare.addrY        = pShare2Isp->addrY;
            ispShare.pitchY       = pShare2Isp->pitchY;
            ispShare.addrU        = pShare2Isp->addrU;
            ispShare.addrV        = pShare2Isp->addrV;
            ispShare.pitchUv      = pShare2Isp->pitchUv;
            ispShare.sliceCount   = pShare2Isp->sliceCount;
            ispShare.isAdobe_CMYK = pShare2Isp->bCMYK;

            switch (pShare2Isp->colorSpace)
            {
            case JPG_COLOR_SPACE_RGB565:    ispShare.format = MMP_ISP_IN_RGB565;    break;
            case JPG_COLOR_SPACE_ARGB8888:  ispShare.format = MMP_ISP_IN_RGB888;    break;
            case JPG_COLOR_SPACE_YUV444:    ispShare.format = MMP_ISP_IN_YUV444;    break;
            case JPG_COLOR_SPACE_YUV422:    ispShare.format = MMP_ISP_IN_YUV422;    break;
            case JPG_COLOR_SPACE_YUV420:    ispShare.format = MMP_ISP_IN_YUV420;    break;
            case JPG_COLOR_SPACE_YUV422R:   ispShare.format = MMP_ISP_IN_YUV422R;   break;
            case JPG_COLOR_SPACE_YUV411:    ispShare.format = MMP_ISP_IN_YUV422;    break;
            default:
                jpg_msg_ex(JPG_MSG_TYPE_ERR, "Wrong input format !! ");
                result = JPG_ERR_INVALID_PARAMETER;
                break;
            }
            if (result == JPG_ERR_OK)
                result = mmpIspPlayImageProcess(gIspDev, &ispShare);
        }
        break;

    case JEL_CTRL_HOST_R_MEM:
        if (extraData)
        {
            JPG_MEM_MOVE_INFO *info = (JPG_MEM_MOVE_INFO *)extraData;
            jpgReadVram(info->dstAddr, info->srcAddr, info->sizeByByte);
        }
        break;

    case JEL_CTRL_HOST_W_MEM:
        if (extraData)
        {
            JPG_MEM_MOVE_INFO *info = (JPG_MEM_MOVE_INFO *)extraData;
            jpgWriteVram(info->dstAddr, info->srcAddr, info->sizeByByte);
        }
        break;

    case JEL_CTRL_CPU_INVALD_CACHE:
        if (extraData)
        {
            JPG_MEM_MOVE_INFO *info = (JPG_MEM_MOVE_INFO *)extraData;

    #if defined(__arm__) && !(_MSC_VER)
            ithInvalidateDCacheRange((void *)info->srcAddr, info->sizeByByte);
    #elif !(_MSC_VER)
            ithInvalidateDCacheRange((void *)info->srcAddr, info->sizeByByte);
    #endif
        }
        break;
    }

    if (result != JPG_ERR_OK)
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !! ", __FUNCTION__, result);
    return result;
}

#else
JPG_ERR
Jpg_Ext_Link_Ctrl(
    JEL_CTRL_CMD cmd,
    uint32_t     *value,
    void         *extraData)
{
    JPG_ERR result = 0;

    return result;
}
#endif
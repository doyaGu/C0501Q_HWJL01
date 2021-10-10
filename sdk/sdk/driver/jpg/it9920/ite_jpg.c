

#include "ite_jpg.h"
#include "jpg_defs.h"
#include "jpg_parser.h"
#include "jpg_common.h"
#include "jpg_codec.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define JPG_MAX_WIDTH                  16376
#define JPG_MAX_HEIGHT                 16376

#define MJPG_MAX_WIDTH                 800
#define MJPG_MAX_HEIGHT                600

#if (JPG_CHECK_CALLER)
    #undef iteJpg_CreateHandle
    #undef iteJpg_DestroyHandle
    #undef iteJpg_SetStreamInfo
    #undef iteJpg_Parsing
    #undef iteJpg_Setup
    #undef iteJpg_Process
    #undef iteJpg_SetBaseOutInfo
    #undef iteJpg_Reset
    #undef iteJpg_WaitIdle

    #define iteJpg_CreateHandle         iteJpg_CreateHandle_dbg
    #define iteJpg_DestroyHandle        iteJpg_DestroyHandle_dbg
    #define iteJpg_SetStreamInfo        iteJpg_SetStreamInfo_dbg
    #define iteJpg_Parsing              iteJpg_Parsing_dbg
    #define iteJpg_Setup                iteJpg_Setup_dbg
    #define iteJpg_Process              iteJpg_Process_dbg
    #define iteJpg_SetBaseOutInfo       iteJpg_SetBaseOutInfo_dbg
    #define iteJpg_Reset                iteJpg_Reset_dbg
    #define iteJpg_WaitIdle             iteJpg_WaitIdle_dbg
#endif
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

typedef struct JPG_DEV_TAG
{
    JPG_INIT_PARAM      initParam;

    // it maybe need to pretect parsing
    pthread_mutex_t     jPsr_mutex;

    // I/O handler
    JPG_STREAM_HANDLE   hJInStream;
    JPG_STREAM_HANDLE   hJOutStream;

    // JPG_USER_INFO       userInfo;
    JPG_STATUS          status;
    JPG_RECT            jpgRect;

    // jpg common -> H/W handler
    JCOMM_HANDLE        *pHJComm;

    // jpg parser info
    JPG_PARSER_INFO     jPrsInfo;

    // output display info
    JPG_DISP_INFO       jDispInfo;

    // encode
    uint32_t            encSectHight;
    bool                bPartialEnc;
    uint8_t             *pExifInfo;
    uint32_t            exifInfoSize;

}JPG_DEV;
//=============================================================================
//                  Global Data Definition
//=============================================================================
uint32_t  jpgMsgOnFlag = (JPG_MSG_TYPE_ERR);// | JPG_MSG_TYPE_TRACE_ITEJPG | JPG_MSG_TYPE_TRACE_JCOMM);  // | JPG_MSG_TYPE_TRACE_PARSER);
uint16_t  PITCH_ARR[6] = {0,512,1024,2048,4096,8192};  //get the correct pitch for jpg decode.

extern JPG_STREAM_DESC jpg_stream_file_desc;
extern JPG_STREAM_DESC jpg_stream_mem_desc;

static pthread_mutex_t  g_jpg_codec_mutex = 0; // only one jpg/isp H/W module
//=============================================================================
//                  Private Function Definition
//=============================================================================
static JPG_ERR
_Verify_DecType(
    JPG_STREAM_HANDLE   *pHJStream,
    JPG_PARSER_INFO     *pJPrsInfo,
    JPG_INIT_PARAM      *pInitParam)
{
    JPG_ERR             result = JPG_ERR_OK;
    JPG_ATTRIB          *pJAttrib = &pJPrsInfo->jAttrib;
    JPG_PRS_BS_CTRL     *pJPrsBsCtrl = &pJPrsInfo->jPrsBsCtrl;

    switch( pInitParam->decType )
    {
        case JPG_DEC_PRIMARY:
            if( pJAttrib->flag & JATT_STATUS_UNSUPPORT_PRIMARY )
            {
                result = JPG_ERR_HW_NOT_SUPPORT;
                goto end;
            }

            jPrs_Seek2SectionStart(pHJStream, pJPrsInfo, pJAttrib->primaryOffset, pJAttrib->primaryLength);
            pJAttrib->decThumbType = JPG_DEC_PRIMARY;
            break;

        case JPG_DEC_LARGE_THUMB:
        case JPG_DEC_SMALL_THUMB:
            // ------------------------------
            // select decode type dependent on parsing result
            switch( pJAttrib->flag & (JATT_STATUS_HAS_SMALL_THUMB |
                                     JATT_STATUS_HAS_LARGE_THUMB |
                                     JATT_STATUS_UNSUPPORT_SMALL_THUMB |
                                     JATT_STATUS_UNSUPPORT_LARGE_THUMB))
            {
                case (JATT_STATUS_HAS_SMALL_THUMB |
                      JATT_STATUS_HAS_LARGE_THUMB): //Support both small thumbnail and large thumbnail.
                    if( pJAttrib->jBaseLiteInfo[JPG_DEC_SMALL_THUMB].width < pJAttrib->jBaseLiteInfo[JPG_DEC_LARGE_THUMB].width )
                        pJAttrib->decThumbType = JPG_DEC_LARGE_THUMB;
                    else
                        pJAttrib->decThumbType = JPG_DEC_SMALL_THUMB;
                    break;

                case (JATT_STATUS_HAS_SMALL_THUMB |
                      JATT_STATUS_HAS_LARGE_THUMB |
                      JATT_STATUS_UNSUPPORT_LARGE_THUMB): // Has 2 thumbnail but large thumb is not supported.
                case JATT_STATUS_HAS_SMALL_THUMB: // Only support small thumbnail.
                    pJAttrib->decThumbType = JPG_DEC_SMALL_THUMB;
                    break;

                case (JATT_STATUS_HAS_LARGE_THUMB |
                      JATT_STATUS_HAS_SMALL_THUMB |
                      JATT_STATUS_UNSUPPORT_SMALL_THUMB): // Has 2 thumbnail but small thumb is not supported.
                case JATT_STATUS_HAS_LARGE_THUMB: // Only support large thumbnail.
                    pJAttrib->decThumbType = JPG_DEC_LARGE_THUMB;
                    break;

                default:
                case (JATT_STATUS_HAS_SMALL_THUMB |
                      JATT_STATUS_UNSUPPORT_SMALL_THUMB): //Has thumbnail1 but not supported.
                case (JATT_STATUS_HAS_SMALL_THUMB |
                      JATT_STATUS_UNSUPPORT_SMALL_THUMB |
                      JATT_STATUS_HAS_LARGE_THUMB |
                      JATT_STATUS_UNSUPPORT_LARGE_THUMB): // Has 2 thumbnail but both can't supported.
                    if( pJAttrib->flag & JATT_STATUS_UNSUPPORT_PRIMARY )
                    {
                        result = JPG_ERR_HW_NOT_SUPPORT;
                        goto end;
                    }

                    jPrs_Seek2SectionStart(pHJStream, pJPrsInfo, pJAttrib->primaryOffset, pJAttrib->primaryLength);
                    pJAttrib->decThumbType = JPG_DEC_PRIMARY;
                    goto end;
                    break;

            }

            if( pJAttrib->decThumbType == JPG_DEC_SMALL_THUMB )
            {
                // small thumbnail
                jPrs_Seek2SectionStart(
                    pHJStream, pJPrsInfo,
                    pJAttrib->jBaseLiteInfo[JPG_DEC_SMALL_THUMB].offset,
                    pJAttrib->jBaseLiteInfo[JPG_DEC_SMALL_THUMB].size);

                pJPrsInfo->bMultiSection = false;
            }
            else
            {
                // large thumbnail
                // only std file I/O can work, mem I/O maybe crash
                uint32_t      thumbSize = 0;
                uint32_t      i;

                for(i = 0; i < pJAttrib->app2StreamCnt; i++)
                    thumbSize += pJAttrib->app2StreamSize[i];

                pJAttrib->jBaseLiteInfo[JPG_DEC_LARGE_THUMB].size = thumbSize;

                if( thumbSize > pJPrsBsCtrl->bsPrsBufMaxLeng )
                {
                    if( pJAttrib->flag & JATT_STATUS_UNSUPPORT_PRIMARY )
                    {
                        result = JPG_ERR_HW_NOT_SUPPORT;
                        goto end;
                    }

                    jPrs_Seek2SectionStart(pHJStream, pJPrsInfo, pJAttrib->primaryOffset, pJAttrib->primaryLength);
                    pJAttrib->decThumbType = JPG_DEC_PRIMARY;
                }
                else
                {
                    uint32_t            realSize = 0;
                    uint8_t             *tmpAddr = pJPrsBsCtrl->bsPrsBuf;
                    JPG_STREAM_DESC     *pJStreamDesc = &pHJStream->jStreamDesc;

                    for(i = 0; i < pJAttrib->app2StreamCnt; i++)
                    {
                        if( pJStreamDesc->jSeek_stream )
                            pJStreamDesc->jSeek_stream(pHJStream, pJAttrib->app2StreamOffset[i], JPG_SEEK_SET, 0);

                        if( pJStreamDesc->jFull_buf )
                            pJStreamDesc->jFull_buf(pHJStream, tmpAddr,
                                                    pJAttrib->app2StreamSize[i],
                                                    &realSize, 0);
                        tmpAddr += pJAttrib->app2StreamSize[i];
                    }

                    pJPrsBsCtrl->curPos = 0;
                    pJPrsInfo->bMultiSection = false;
                }
            }
            break;
    }

end:
    return result;
}

static void
_Set_Exif_Orientation(
    JPG_DEC_TYPE     decType,
    JPG_ROT_TYPE     rotType,
    bool             bExifOrientation,
    uint32_t         exifOrientationType,
    JPG_DISP_INFO    *pJDispInfo)
{

    if( bExifOrientation == true && decType == JPG_DEC_PRIMARY )
    {
        switch( exifOrientationType )
        {
            default :
            case 0:             break;
            case 1: pJDispInfo->rotType = JPG_ROT_TYPE_0;    break; // (row, column) = (T, L)
            case 2:             break;                              // (row, column) = (T, R) => MIRROR
            case 3: pJDispInfo->rotType = JPG_ROT_TYPE_180;  break; // (row, column) = (B, R)
            case 4:             break;                              // (row, column) = (B, L) => FLIP;
            case 5:             break;                              // (row, column) = (L, T)
            case 6: pJDispInfo->rotType = JPG_ROT_TYPE_90;   break; // (row, column) = (R, T)
            case 7:             break;                              // (row, column) = (R, B)
            case 8: pJDispInfo->rotType = JPG_ROT_TYPE_270;  break; // (row, column) = (L, B)
        }

        pJDispInfo->rotType = (JPG_ROT_TYPE)(((uint32_t)rotType + (uint32_t)pJDispInfo->rotType) & 0x3);
    }
    else
    {
        pJDispInfo->rotType = rotType;
    }
}

static void
_genParam_Fit (
    JPG_DISP_INFO     *pJDispInfo,
    uint32_t          dispWidth,
    uint32_t          dispHeight,
    uint32_t          realWidth,
    uint32_t          realHeight)
{
    pJDispInfo->srcW = realWidth;
    pJDispInfo->srcH = realHeight;
    pJDispInfo->srcX = pJDispInfo->srcY = 0;

    pJDispInfo->dstW = dispWidth;
    pJDispInfo->dstH = dispHeight;
    pJDispInfo->dstX = pJDispInfo->dstY = 0;
}


static void
_genParam_CutPart(
     JPG_DISP_INFO     *pJDispInfo,
     uint32_t          dispWidth,
     uint32_t          dispHeight,
     uint32_t          realWidth,
     uint32_t          realHeight,
     uint32_t          keepPercentage)
{
    uint32_t    scalWidth = 0, scalHeight = 0;
    uint32_t    picWidth  = 0;
    uint32_t    picHeight = 0;

    picWidth  = realWidth;
    picHeight = realHeight;

    switch( pJDispInfo->rotType )
    {
        default:
        case JPG_ROT_TYPE_0:
        case JPG_ROT_TYPE_180:
            // First, according to the aspect ratio of the JPEG picture to decide how to fit the decoded picture
            scalWidth  = dispWidth;
            scalHeight = scalWidth * picHeight / picWidth;

            if( scalHeight < dispHeight )
            {
                // Cut left and right of the source picture
                scalWidth = dispHeight * picWidth / picHeight;
                if( (scalWidth * keepPercentage) >= (100 * dispWidth) )
                {
                    // reserve 60% case
                    scalHeight = (dispWidth * 100 * picHeight) / (keepPercentage * picWidth);
                    pJDispInfo->srcW = picWidth * keepPercentage / 100;
                    pJDispInfo->srcH = picHeight;
                    pJDispInfo->srcX = (picWidth - pJDispInfo->srcW) >> 1;
                    pJDispInfo->srcY = 0;

                    pJDispInfo->dstX = 0;
                    pJDispInfo->dstY = (dispHeight - scalHeight) >> 1;
                    pJDispInfo->dstW = dispWidth;
                    pJDispInfo->dstH = scalHeight;
                }
                else
                {
                    uint32_t    tmpH = picHeight;
                    uint32_t    tmpW = picHeight * dispWidth / dispHeight;

                    pJDispInfo->srcW = tmpW;
                    pJDispInfo->srcH = tmpH;
                    pJDispInfo->srcX = (picWidth - tmpW) >> 1;
                    pJDispInfo->srcY = 0;

                    pJDispInfo->dstX = 0;
                    pJDispInfo->dstY = 0;
                    pJDispInfo->dstW = dispWidth;
                    pJDispInfo->dstH = dispHeight;
                }
            }
            else
            {
                // Cut top and bottom of the source picture
                if ( (scalHeight * keepPercentage) >= (100 * dispHeight) )
                {
                    // reserve 60% case
                    scalWidth = (dispHeight * 100 * picWidth) / (keepPercentage * picHeight);

                    pJDispInfo->srcW = picWidth;
                    pJDispInfo->srcH = picHeight * keepPercentage / 100;
                    pJDispInfo->srcX = 0;
                    pJDispInfo->srcY = (picHeight - pJDispInfo->srcH) >> 1;

                    pJDispInfo->dstX = (dispWidth - scalWidth) >> 1;
                    pJDispInfo->dstY = 0;
                    pJDispInfo->dstW = scalWidth;
                    pJDispInfo->dstH = dispHeight;
                }
                else
                {
                    uint32_t    tmpW = picWidth;
                    uint32_t    tmpH = picWidth * dispHeight / dispWidth;

                    pJDispInfo->srcW = tmpW;
                    pJDispInfo->srcH = tmpH;
                    pJDispInfo->srcX = 0;
                    pJDispInfo->srcY = (picHeight - tmpH) >> 1;

                    pJDispInfo->dstX = 0;
                    pJDispInfo->dstY = 0;
                    pJDispInfo->dstW = dispWidth;
                    pJDispInfo->dstH = dispHeight;
                }
            }
            break;

        case JPG_ROT_TYPE_90:
        case JPG_ROT_TYPE_270:
            // First, according to the aspect ratio of the JPEG picture
            //        to decide how to fit the decoded picture
            scalWidth = dispWidth;
            scalHeight = scalWidth * picHeight / picWidth;

            if( scalHeight < dispHeight )
            {
                // Cut top and bottom of the source picture
                scalWidth = dispHeight * picWidth / picHeight;
                if( (scalWidth * keepPercentage) >= (100 * dispWidth) )
                {
                    // reserve 60% case
                    scalHeight = (dispWidth * 100 * picHeight) / (keepPercentage * picWidth);

                    pJDispInfo->srcW = picHeight;
                    pJDispInfo->srcH = picWidth * keepPercentage / 100;
                    pJDispInfo->srcX = 0;
                    pJDispInfo->srcY = (picWidth - pJDispInfo->srcH) >> 1;

                    pJDispInfo->dstX = 0;
                    pJDispInfo->dstY = (dispHeight - scalHeight) >> 1;
                    pJDispInfo->dstW = dispWidth;
                    pJDispInfo->dstH = scalHeight;
                }
                else
                {
                    uint32_t    tmpW = picHeight;
                    uint32_t    tmpH = picHeight * dispWidth / dispHeight;

                    pJDispInfo->srcW = tmpW;
                    pJDispInfo->srcH = tmpH;
                    pJDispInfo->srcX = 0;
                    pJDispInfo->srcY = (picWidth - tmpH) >> 1;

                    pJDispInfo->dstX = 0;
                    pJDispInfo->dstY = 0;
                    pJDispInfo->dstW = dispWidth;
                    pJDispInfo->dstH = dispHeight;
                }
            }
            else
            {
                // Cut left and right of the source picture
                if( (scalHeight * keepPercentage) >= (100 * dispHeight) )
                {
                    // reserve 60% case
                    scalWidth = (dispHeight * 100 * picWidth) / (keepPercentage * picHeight);

                    pJDispInfo->srcW = picHeight * keepPercentage / 100;
                    pJDispInfo->srcH = picWidth;
                    pJDispInfo->srcX = (picHeight - pJDispInfo->srcW) >> 1;
                    pJDispInfo->srcY = 0;

                    pJDispInfo->dstX = (dispWidth - scalWidth) >> 1;
                    pJDispInfo->dstY = 0;
                    pJDispInfo->dstW = scalWidth;
                    pJDispInfo->dstH = dispHeight;
                }
                else
                {
                    uint32_t    tmpH = picWidth;
                    uint32_t    tmpW = picWidth * dispHeight / dispWidth;

                    pJDispInfo->srcW = tmpW;
                    pJDispInfo->srcH = tmpH;
                    pJDispInfo->srcX = (picHeight - tmpW) >> 1;
                    pJDispInfo->srcY = 0;

                    pJDispInfo->dstX = 0;
                    pJDispInfo->dstY = 0;
                    pJDispInfo->dstW = dispWidth;
                    pJDispInfo->dstH = dispHeight;
                }
            }
            break;
    }

    return;
}

static void
_calc_partial_enc_heigth(
    JPG_DEV     *pJpgDev)
{
    uint32_t             ratio = 0;
    JCOMM_HANDLE        *pHJComm = pJpgDev->pHJComm;

    // general compression ratio
    if( pJpgDev->initParam.encQuality > 90 )        ratio = 4;
    else if( pJpgDev->initParam.encQuality > 85 )   ratio = 10;
    else                                            ratio = 15;

    pJpgDev->encSectHight = (pHJComm->sysBsBufSize * ratio) / (3 * pJpgDev->initParam.width);

    pJpgDev->encSectHight &= (~0xF); // MCI 16x8

    // switch( pJpgDev->initParam.outColorSpace )
    // {
    //     // MCU 16x8
    //     case JPG_COLOR_SPACE_YUV422:  pJpgDev->encSectHight &= (~0x7); break;
    //
    //     // MCI 16x8
    //     case JPG_COLOR_SPACE_YUV420:  pJpgDev->encSectHight &= (~0xF);  break;
    // }

    if( pJpgDev->encSectHight > pJpgDev->initParam.height )
    {
        pJpgDev->bPartialEnc = false;
        pJpgDev->encSectHight = pJpgDev->initParam.height;
    }
    else
        pJpgDev->bPartialEnc = true;

    pHJComm->encSectHight = pJpgDev->encSectHight;

    return;
}

static JPG_ERR
_jpg_dec_setup(
    JPG_DEV     *pJpgDev)
{
    JPG_ERR             result = JPG_ERR_OK;
    JCOMM_HANDLE        *pHJComm = pJpgDev->pHJComm;
    JPG_PARSER_INFO     *pJPrsInfo = &pJpgDev->jPrsInfo;
    JPG_PRS_BS_CTRL     *pJPrsBsCtrl = &pJPrsInfo->jPrsBsCtrl;
    JPG_STREAM_HANDLE   *pHInJStream = &pJpgDev->hJInStream;
    JPG_STREAM_HANDLE   *pHOutJStream = &pJpgDev->hJOutStream;
    JPG_STREAM_DESC     *pJInStreamDesc = &pHInJStream->jStreamDesc;
    JPG_DISP_INFO       *pJDispInfo = &pJpgDev->jDispInfo;
    JPG_DEC_TYPE        decType = pJpgDev->initParam.decType;
    JCOMM_INIT_PARAM    jCommInitParam = {0};

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ITEJPG, "0x%x\n", pJpgDev);

    do{
        int     i;

        //-----------------------------------------------
        // jpg H/W init
        jCommInitParam.pHInJStream  = &pJpgDev->hJInStream;
        jCommInitParam.pHOutJStream = &pJpgDev->hJOutStream;
        jCommInitParam.pJDispInfo   = pJDispInfo;

        if( pJPrsInfo->jAttrib.jBaseLiteInfo[decType].bJprog == true )
            jCommInitParam.codecType = JPG_CODEC_DEC_JPROG;
        else
            jCommInitParam.codecType = JPG_CODEC_DEC_JPG;

        //---------------------------------  //Benson
        // assign info from parser
        pHJComm->pJFrmComp  = &pJPrsInfo->jFrmComp;

        result = jComm_Init(pHJComm, &jCommInitParam, 0);
        if( result != JPG_ERR_OK )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " error !!");
            result = JPG_ERR_ALLOCATE_FAIL;
            break;
        }

        //------------------------------------------------
        // jpg H/W setup
        if( pJPrsInfo->bMultiSection == true &&
            pJPrsInfo->remainSize > 0 )
        {
            // multi-section
            uint8_t     *tmpAddr = pJPrsBsCtrl->bsPrsBuf + pJPrsBsCtrl->curPos;
            int         tmpLength = pJPrsBsCtrl->realSize - pJPrsBsCtrl->curPos;

            // move and fill bs buffer
            memcpy(pJPrsBsCtrl->bsPrsBuf, tmpAddr, tmpLength);
            tmpAddr = pJPrsBsCtrl->bsPrsBuf + tmpLength;
            if( pJInStreamDesc->jFull_buf )
                pJInStreamDesc->jFull_buf(pHInJStream, tmpAddr,
                                          pJPrsBsCtrl->curPos,
                                          &pHJComm->realSzie, 0);

            pHJComm->remainSize = pJPrsInfo->remainSize - pHJComm->realSzie;

            if( pHJComm->remainSize == 0 )
            {
                pHJComm->jInBufInfo[0].pBufAddr = pHJComm->jInBufInfo[1].pBufAddr = pJPrsBsCtrl->bsPrsBuf;
                pHJComm->jInBufInfo[0].bufLength = pHJComm->jInBufInfo[1].bufLength
                                                 = pJPrsInfo->fileLength - pJPrsBsCtrl->curPos;
                pJPrsInfo->bMultiSection = false;
            }
            else
            {
                // for pipe line decode, bsBufMaxLeng_A should be alignment
                pHJComm->jInBufInfo[0].pBufAddr  = pJPrsBsCtrl->bsPrsBuf;
                pHJComm->jInBufInfo[1].pBufAddr  = pJPrsBsCtrl->bsPrsBuf + (pJPrsBsCtrl->bsPrsBufMaxLeng >> 1);

                pHJComm->jInBufInfo[0].bufLength = pHJComm->jInBufInfo[1].bufLength
                                                 = (pJPrsBsCtrl->bsPrsBufMaxLeng >> 1);
            }

            pJPrsBsCtrl->curPos = 0;
        }
        else
        {
            // signal section
            JPG_ATTRIB          *pJAttrib = &pJPrsInfo->jAttrib;

            pHJComm->jInBufInfo[0].pBufAddr = pHJComm->jInBufInfo[1].pBufAddr
                                            = pJPrsBsCtrl->bsPrsBuf + pJPrsBsCtrl->curPos;
            switch( pJAttrib->decThumbType )
            {
                case JPG_DEC_SMALL_THUMB:
                case JPG_DEC_LARGE_THUMB:
                    pHJComm->jInBufInfo[0].bufLength = pJAttrib->jBaseLiteInfo[pJAttrib->decThumbType].size;
                    break;

                default:
                    pHJComm->jInBufInfo[0].bufLength = pJPrsBsCtrl->realSize - pJPrsBsCtrl->curPos;
                    break;
            }
            pHJComm->jInBufInfo[1].bufLength = pHJComm->jInBufInfo[0].bufLength;
            pHJComm->remainSize = 0;
        }

        pHJComm->actIOBuf_idx = 0;

        switch( pJpgDev->initParam.outColorSpace )
        {
            case JPG_COLOR_SPACE_ARGB4444:
            case JPG_COLOR_SPACE_ARGB8888:
            case JPG_COLOR_SPACE_RGB565:
                pHOutJStream->compCnt = 1;
                pHJComm->jOutBufInfo[0].pBufAddr  = pHOutJStream->jStreamInfo.jstream.mem[0].pAddr;
                pHJComm->jOutBufInfo[0].bufLength = pHOutJStream->jStreamInfo.jstream.mem[0].length;
                pHJComm->jOutBufInfo[0].pitch     = pHOutJStream->jStreamInfo.jstream.mem[0].pitch;
                break;

            case JPG_COLOR_SPACE_YUV422:
            case JPG_COLOR_SPACE_YUV420:
			case JPG_COLOR_SPACE_YUV444:
                // set output buffer info
                pHOutJStream->compCnt = pHOutJStream->jStreamInfo.validCompCnt;
                for(i = 0; i < pHOutJStream->compCnt; i++)
                {
                    pHJComm->jOutBufInfo[i].pBufAddr  = pHOutJStream->jStreamInfo.jstream.mem[i].pAddr;
                    pHJComm->jOutBufInfo[i].bufLength = pHOutJStream->jStreamInfo.jstream.mem[i].length;
                    pHJComm->jOutBufInfo[i].pitch     = pHOutJStream->jStreamInfo.jstream.mem[i].pitch;
                }
                break;
        }

        result = jComm_Setup(pHJComm, 0);

#if !(_MSC_VER)
        pHJComm->actIOBuf_idx = ((pHJComm->actIOBuf_idx+1) & 0x1);
#endif

    }while(0);

    if( result != JPG_ERR_OK )
    {
        jpg_msg_ex(JPG_MSG_TYPE_ERR,"%s() err 0x%x !", __FUNCTION__, result);
    }
    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ITEJPG);
    return result;
}

static JPG_ERR
_jpg_dec_fire(
    JPG_DEV     *pJpgDev)
{
    JPG_ERR             result = JPG_ERR_OK;
    JCOMM_HANDLE        *pHJComm = pJpgDev->pHJComm;
    JPG_STREAM_HANDLE   *pHInJStream = &pJpgDev->hJInStream;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ITEJPG, "0x%x\n", pJpgDev);

    do{
        bool                b1stFire = true;
        JPG_STREAM_DESC     *pJInStreamDesc = &pHInJStream->jStreamDesc;
        JPG_PARSER_INFO     *pJPrsInfo = &pJpgDev->jPrsInfo;

        result = jComm_Fire(pHJComm, ((pHJComm->remainSize == 0) ? true : false), 0);
        if( result != JPG_ERR_OK )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg err 0x%x !! ", result);
            break;
        }

        pHJComm->actIOBuf_idx = ((pHJComm->actIOBuf_idx+1) & 0x1);
        //--------------------------------------------
        // jpg H/W fire
        while( pJPrsInfo->bMultiSection == true && pHJComm->remainSize > 0 )
        {
            uint8_t     *pCurBsBuf = 0;
            uint32_t    curBsBufSize = 0;

            jpg_sleep(1); //5
            if( b1stFire == true )      b1stFire = false;
            else                        pHJComm->remainSize -= pHJComm->realSzie;

            result = jComm_Fire(pHJComm, ((pHJComm->remainSize == 0) ? true : false), 0);
            if( result != JPG_ERR_OK )
            {
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg err 0x%x !! ", result);
                break;
            }

            if( pHJComm->remainSize > 0 )
            {
                // fill buff
        #if (_MSC_VER) // win32
                pHJComm->actIOBuf_idx = ((pHJComm->actIOBuf_idx+1) & 0x1);
                pCurBsBuf    = pHJComm->jInBufInfo[pHJComm->actIOBuf_idx].pBufAddr;
                curBsBufSize = pHJComm->jInBufInfo[pHJComm->actIOBuf_idx].bufLength;
                if( pJInStreamDesc->jFull_buf )
                    pJInStreamDesc->jFull_buf(pHInJStream, pCurBsBuf,
                                              curBsBufSize,
                                              &pHJComm->realSzie, 0);
        #else
                pCurBsBuf    = pHJComm->jInBufInfo[pHJComm->actIOBuf_idx].pBufAddr;
                curBsBufSize = pHJComm->jInBufInfo[pHJComm->actIOBuf_idx].bufLength;
                if( pJInStreamDesc->jFull_buf )
                    pJInStreamDesc->jFull_buf(pHInJStream, pCurBsBuf,
                                              curBsBufSize,
                                              &pHJComm->realSzie, 0);
                pHJComm->actIOBuf_idx = ((pHJComm->actIOBuf_idx+1) & 0x1);
        #endif
            }
        }
    }while(0);

    if( result != JPG_ERR_OK )
    {
        jpg_msg_ex(JPG_MSG_TYPE_ERR,"%s() err 0x%x !", __FUNCTION__, result);
    }

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ITEJPG);
    return result;
}

static JPG_ERR
_jpg_dec_cmd_setup(
    JPG_DEV     *pJpgDev)
{
    JPG_ERR             result = JPG_ERR_OK;
    JCOMM_HANDLE        *pHJComm = pJpgDev->pHJComm;
    JPG_PARSER_INFO     *pJPrsInfo = &pJpgDev->jPrsInfo;
    JPG_PRS_BS_CTRL     *pJPrsBsCtrl = &pJPrsInfo->jPrsBsCtrl;
    JPG_STREAM_HANDLE   *pHOutJStream = &pJpgDev->hJOutStream;
    JPG_DISP_INFO       *pJDispInfo = &pJpgDev->jDispInfo;
    JCOMM_INIT_PARAM    jCommInitParam = {0};

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ITEJPG, "0x%x\n", pJpgDev);

    do{
        bool    bBreak = false;
        int     i;

        //-----------------------------------------------
        // jpg H/W init
        jCommInitParam.pHInJStream  = &pJpgDev->hJInStream;
        jCommInitParam.pHOutJStream = &pJpgDev->hJOutStream;
		jCommInitParam.pJDispInfo	= &pJpgDev->jDispInfo;
        jCommInitParam.codecType    = JPG_CODEC_DEC_JPG_CMD;

        result = jComm_Init(pHJComm, &jCommInitParam, 0);
        if( result != JPG_ERR_OK )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " error !!");
            result = JPG_ERR_ALLOCATE_FAIL;
            break;
        }

        //---------------------------------
        // assign info from parser
        pHJComm->pJDispInfo = pJDispInfo;
        pHJComm->pJFrmComp  = &pJPrsInfo->jFrmComp;

        {
            // signal section
            JPG_ATTRIB          *pJAttrib = &pJPrsInfo->jAttrib;

            pHJComm->jInBufInfo[0].pBufAddr = pHJComm->jInBufInfo[1].pBufAddr
                                            = pJPrsBsCtrl->bsPrsBuf + pJPrsBsCtrl->curPos;

            pHJComm->jInBufInfo[0].bufLength = pJPrsBsCtrl->realSize - pJPrsBsCtrl->curPos;

            pHJComm->jInBufInfo[1].bufLength = pHJComm->jInBufInfo[0].bufLength;
            pHJComm->remainSize = 0;
        }

        pHJComm->actIOBuf_idx = 0;

        pHOutJStream->compCnt = pHOutJStream->jStreamInfo.validCompCnt;
        for(i = 0; i < pHOutJStream->compCnt; i++)
        {
            pHJComm->jOutBufInfo[i].pBufAddr  = pHOutJStream->jStreamInfo.jstream.mem[i].pAddr;
            pHJComm->jOutBufInfo[i].bufLength = pHOutJStream->jStreamInfo.jstream.mem[i].length;
            pHJComm->jOutBufInfo[i].pitch     = pHOutJStream->jStreamInfo.jstream.mem[i].pitch;
        }

        if( bBreak == true )        break;

        result = jComm_Setup(pHJComm, 0);

    }while(0);

    if( result != JPG_ERR_OK )
    {
        jpg_msg_ex(JPG_MSG_TYPE_ERR,"%s() err 0x%x !", __FUNCTION__, result);
    }
    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ITEJPG);
    return result;

}

static JPG_ERR
_jpg_dec_cmd_fire(
    JPG_DEV         *pJpgDev,
    JPG_BUF_INFO    *pEntropyBufInfo)
{
    JPG_ERR             result = JPG_ERR_OK;
    JCOMM_HANDLE        *pHJComm = pJpgDev->pHJComm;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ITEJPG, "0x%x\n", pJpgDev);

    do{
        JPG_STREAM_INFO     *pJOutStreamInfo = &pJpgDev->hJOutStream.jStreamInfo;

        // set input stream info
        pHJComm->jInBufInfo[0].pBufAddr  = pEntropyBufInfo->pBufAddr;
        pHJComm->jInBufInfo[0].bufLength = pEntropyBufInfo->bufLength;
        pHJComm->jInBufInfo[0].pitch     = pEntropyBufInfo->pitch;

        // mjpg only support memory out
        if( pJOutStreamInfo->streamType != JPG_STREAM_MEM ||
            pJOutStreamInfo->streamIOType != JPG_STREAM_IO_WRITE )
        {
            result = JPG_ERR_NO_IMPLEMENT;
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg err 0x%x !! ", result);
            break;
        }

        result = jComm_Fire(pHJComm, true, 0);
        if( result != JPG_ERR_OK )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg err 0x%x !! ", result);
            break;
        }

    }while(0);

    if( result != JPG_ERR_OK )
    {
        jpg_msg_ex(JPG_MSG_TYPE_ERR,"%s() err 0x%x !", __FUNCTION__, result);
    }
    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ITEJPG);
    return result;

}

#if (CONFIG_JPG_CODEC_DESC_DEC_MJPG_DESC)
static JPG_ERR
_jpg_dec_mjpg_setup(
    JPG_DEV     *pJpgDev)
{
    JPG_ERR             result = JPG_ERR_OK;
    JCOMM_HANDLE        *pHJComm = pJpgDev->pHJComm;
    JPG_PARSER_INFO     *pJPrsInfo = &pJpgDev->jPrsInfo;
	JPG_PRS_BS_CTRL     *pJPrsBsCtrl = &pJPrsInfo->jPrsBsCtrl;
    JPG_STREAM_HANDLE   *pHOutJStream = &pJpgDev->hJOutStream;
    JPG_DISP_INFO       *pJDispInfo = &pJpgDev->jDispInfo;
    JCOMM_INIT_PARAM    jCommInitParam = {0};


    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ITEJPG, "0x%x\n", pJpgDev);

    do{
        bool    bBreak = false;
        int     i;

        //-----------------------------------------------
        // jpg H/W init
        jCommInitParam.pHInJStream  = &pJpgDev->hJInStream;
        jCommInitParam.pHOutJStream = &pJpgDev->hJOutStream;
		jCommInitParam.pJDispInfo	= &pJpgDev->jDispInfo;
        jCommInitParam.codecType    = JPG_CODEC_DEC_MJPG;

        result = jComm_Init(pHJComm, &jCommInitParam, 0);
        if( result != JPG_ERR_OK )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " error !!");
            result = JPG_ERR_ALLOCATE_FAIL;
            break;
        }

		//---------------------------------
        // assign info from parser
        pHJComm->pJDispInfo = pJDispInfo;
        pHJComm->pJFrmComp  = &pJPrsInfo->jFrmComp;

        {
            // signal section
            JPG_ATTRIB          *pJAttrib = &pJPrsInfo->jAttrib;

            pHJComm->jInBufInfo[0].pBufAddr = pHJComm->jInBufInfo[1].pBufAddr
                                            = pJPrsBsCtrl->bsPrsBuf + pJPrsBsCtrl->curPos;

            pHJComm->jInBufInfo[0].bufLength = pJPrsBsCtrl->realSize - pJPrsBsCtrl->curPos;

            pHJComm->jInBufInfo[1].bufLength = pHJComm->jInBufInfo[0].bufLength;
            pHJComm->remainSize = 0;
        }

        pHJComm->actIOBuf_idx = 0;

        switch( pJpgDev->initParam.outColorSpace )
        {
             case JPG_COLOR_SPACE_RGB565:     //For isp handshake  ,Benson
			 	pHOutJStream->compCnt = 1;
                pHJComm->jOutBufInfo[0].pBufAddr  = pHOutJStream->jStreamInfo.jstream.mem[0].pAddr;
                pHJComm->jOutBufInfo[0].bufLength = pHOutJStream->jStreamInfo.jstream.mem[0].length;
                pHJComm->jOutBufInfo[0].pitch     = pHOutJStream->jStreamInfo.jstream.mem[0].pitch;
			 	break;
            case JPG_COLOR_SPACE_YUV422:
            case JPG_COLOR_SPACE_YUV420:
                // set output buffer info
                pHOutJStream->compCnt = pHOutJStream->jStreamInfo.validCompCnt;
                for(i = 0; i < pHOutJStream->compCnt; i++)
                {
                    pHJComm->jOutBufInfo[i].pBufAddr  = pHOutJStream->jStreamInfo.jstream.mem[i].pAddr;
                    pHJComm->jOutBufInfo[i].bufLength = pHOutJStream->jStreamInfo.jstream.mem[i].length;
                    pHJComm->jOutBufInfo[i].pitch     = pHOutJStream->jStreamInfo.jstream.mem[i].pitch;
                }
                break;

            default:
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " error !!");
                result = JPG_ERR_HW_NOT_SUPPORT;
                bBreak = true;
                break;
        }

        if( bBreak == true )        break;

        result = jComm_Setup(pHJComm, 0);

    }while(0);

    if( result != JPG_ERR_OK )
    {
        jpg_msg_ex(JPG_MSG_TYPE_ERR,"%s() err 0x%x !", __FUNCTION__, result);
    }
    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ITEJPG);
    return result;
}

static JPG_ERR
_jpg_dec_mjpg_fire(
    JPG_DEV         *pJpgDev,
    JPG_BUF_INFO    *pEntropyBufInfo)
{
    JPG_ERR             result = JPG_ERR_OK;
    JCOMM_HANDLE        *pHJComm = pJpgDev->pHJComm;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ITEJPG, "0x%x\n", pJpgDev);

    do{
        JPG_STREAM_INFO     *pJOutStreamInfo = &pJpgDev->hJOutStream.jStreamInfo;

        // set input stream info
        pHJComm->jInBufInfo[0].pBufAddr  = pEntropyBufInfo->pBufAddr;
        pHJComm->jInBufInfo[0].bufLength = pEntropyBufInfo->bufLength;
        pHJComm->jInBufInfo[0].pitch     = pEntropyBufInfo->pitch;

        // mjpg only support memory out
        if( pJOutStreamInfo->streamType != JPG_STREAM_MEM ||
            pJOutStreamInfo->streamIOType != JPG_STREAM_IO_WRITE )
        {
            result = JPG_ERR_NO_IMPLEMENT;
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg err 0x%x !! ", result);
            break;
        }

        result = jComm_Fire(pHJComm, true, 0);
        if( result != JPG_ERR_OK )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg err 0x%x !! ", result);
            break;
        }

    }while(0);

    if( result != JPG_ERR_OK )
    {
        jpg_msg_ex(JPG_MSG_TYPE_ERR,"%s() err 0x%x !", __FUNCTION__, result);
    }
    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ITEJPG);
    return result;
}
#endif

#if (CONFIG_JPG_CODEC_DESC_ENCODER_DESC)
static JPG_ERR
_jpg_enc_setup(
    JPG_DEV     *pJpgDev)
{
    JPG_ERR             result = JPG_ERR_OK;
    JCOMM_INIT_PARAM    jCommInitParam = {0};

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ITEJPG, "0x%x\n", pJpgDev);

    do{
        JCOMM_HANDLE        *pHJComm = pJpgDev->pHJComm;
        JPG_STREAM_HANDLE   *pHInJStream = &pJpgDev->hJInStream;
        int                 i;

        //-----------------------------------------------
        // jpg H/W init
        jCommInitParam.pHInJStream  = &pJpgDev->hJInStream;
        jCommInitParam.pHOutJStream = &pJpgDev->hJOutStream;
        jCommInitParam.codecType    = JPG_CODEC_ENC_JPG;
        jCommInitParam.encQuality   = pJpgDev->initParam.encQuality;
        jCommInitParam.encWidth     = pJpgDev->initParam.width;
        jCommInitParam.encHeight    = pJpgDev->initParam.height;
        jCommInitParam.encColorSpace = pJpgDev->initParam.outColorSpace;

        result = jComm_Init(pHJComm, &jCommInitParam, 0);
        if( result != JPG_ERR_OK )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " error !!");
            result = JPG_ERR_ALLOCATE_FAIL;
            break;
        }

        pHJComm->pJFrmComp  = &pJpgDev->jPrsInfo.jFrmComp;
        pHJComm->pJDispInfo = &pJpgDev->jDispInfo;

        //--------------------------------
        // calculate section height for encoding
        _calc_partial_enc_heigth(pJpgDev);
        jpg_msg(1, "\t.... bPartialEnc = %d\r\n", pJpgDev->bPartialEnc); //Benson

        //---------------------------------
        // set input stream info
        pHInJStream->compCnt = pHInJStream->jStreamInfo.validCompCnt;
        for(i = 0; i < pHInJStream->compCnt; i++)
        {
            pHJComm->jInBufInfo[i].pBufAddr  = pHInJStream->jStreamInfo.jstream.mem[i].pAddr;
            pHJComm->jInBufInfo[i].bufLength = pHInJStream->jStreamInfo.jstream.mem[i].length;
            pHJComm->jInBufInfo[i].pitch     = pHInJStream->jStreamInfo.jstream.mem[i].pitch;
        }

        result = jComm_Setup(pHJComm, 0);

    }while(0);

    if( result != JPG_ERR_OK )
    {
        jpg_msg_ex(JPG_MSG_TYPE_ERR,"%s() err 0x%x !", __FUNCTION__, result);
    }

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ITEJPG);
    return result;
}

static JPG_ERR
_jpg_enc_fire(
    JPG_DEV         *pJpgDev,
    JPG_BUF_INFO    *pSysBsBufInfo,
    uint32_t        *pJpgSize)
{
    JPG_ERR             result = JPG_ERR_OK;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ITEJPG, "0x%x\n", pJpgDev);

    do{
        JCOMM_HANDLE        *pHJComm = pJpgDev->pHJComm;
        JPG_STREAM_HANDLE   *pHOutJStream = &pJpgDev->hJOutStream;
        JPG_STREAM_DESC     *pJOutStreamDesc = &pHOutJStream->jStreamDesc;
        int                 remain_H = (int)pJpgDev->initParam.height;
        uint32_t            totalEncLeng = 0;
        uint8_t             *AsyncSaveAllAddr = NULL;

        if( !pJOutStreamDesc->jOut_buf )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg err, No out stream API!! ");
            result = JPG_ERR_INVALID_PARAMETER;
            break;
        }

        result = jComm_Fire(pHJComm, !pJpgDev->bPartialEnc, 0);
        if( result != JPG_ERR_OK )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg err 0x%x !! ", result);
            break;
        }

        //---------------------------------
        // save enc leng
        totalEncLeng = pJpgDev->pHJComm->sysValidBsBufSize + pJpgDev->pHJComm->jHdrDataSize + pJpgDev->exifInfoSize;

        //---------------------------------
        // output jpg heander
        if(pHOutJStream->jStreamInfo.streamType == JPG_STREAM_MEM) //Benson add for async mode side effect.
        {  
            //Original coding.
            if( pJpgDev->pExifInfo )   //it seems all to change
            {
                pJOutStreamDesc->jOut_buf(pHOutJStream, pJpgDev->pHJComm->pJHdrData, 2, 0);
                pJOutStreamDesc->jOut_buf(pHOutJStream, pJpgDev->pExifInfo, pJpgDev->exifInfoSize, 0);
                pJOutStreamDesc->jOut_buf(pHOutJStream, pJpgDev->pHJComm->pJHdrData, (pJpgDev->pHJComm->jHdrDataSize - 2), 0);
            }
            else
                pJOutStreamDesc->jOut_buf(pHOutJStream, pJpgDev->pHJComm->pJHdrData, pJpgDev->pHJComm->jHdrDataSize, 0);

            // output entropy code data
            pJOutStreamDesc->jOut_buf(pHOutJStream, pJpgDev->pHJComm->pSysBsBufAddr, pJpgDev->pHJComm->sysValidBsBufSize, 0);
         }
         else
         {  
            //change to async mode , save all header and data to fwrite once time.  
            if(AsyncSaveAllAddr) {printf("AsyncSaveAllAddr has memory leak issue , addr= 0x%x\n",AsyncSaveAllAddr);}

            if( pJpgDev->pExifInfo ) 
            {
                AsyncSaveAllAddr =malloc(totalEncLeng);    
                if(AsyncSaveAllAddr ==NULL)        
                {printf("open AsyncSaveAllAddr fail\n");}

                memcpy(AsyncSaveAllAddr ,  pJpgDev->pHJComm->pJHdrData ,2);
                memcpy(AsyncSaveAllAddr+2 ,  pJpgDev->pExifInfo ,pJpgDev->exifInfoSize);
                memcpy(AsyncSaveAllAddr+2+pJpgDev->exifInfoSize ,  pJpgDev->pHJComm->pJHdrData,(pJpgDev->pHJComm->jHdrDataSize - 2));

                memcpy(AsyncSaveAllAddr+2+pJpgDev->exifInfoSize+(pJpgDev->pHJComm->jHdrDataSize - 2) ,  pJpgDev->pHJComm->pSysBsBufAddr,pJpgDev->pHJComm->sysValidBsBufSize);
                pJOutStreamDesc->jOut_buf(pHOutJStream, AsyncSaveAllAddr, totalEncLeng, 0);
            }else
            {
                AsyncSaveAllAddr =malloc(totalEncLeng);
                if(AsyncSaveAllAddr ==NULL)        
                {printf("open AsyncSaveAllAddr fail\n");}

                memcpy(AsyncSaveAllAddr ,  pJpgDev->pHJComm->pJHdrData ,pJpgDev->pHJComm->jHdrDataSize);
                memcpy(AsyncSaveAllAddr+pJpgDev->pHJComm->jHdrDataSize ,  pJpgDev->pHJComm->pSysBsBufAddr,pJpgDev->pHJComm->sysValidBsBufSize);
                pJOutStreamDesc->jOut_buf(pHOutJStream, AsyncSaveAllAddr, totalEncLeng, 0);
            }
        }


        pJpgDev->pHJComm->sysValidBsBufSize = 0; // for next section encode
        remain_H -= pJpgDev->encSectHight;

        // ------------------------------------
        // multi-encode not verify
        while( pJpgDev->bPartialEnc == true && remain_H > 0 )
        {
            JCOMM_HANDLE        *pHJComm = pJpgDev->pHJComm;
            JPG_STREAM_INFO     *pJInStreamInfo = &pJpgDev->hJInStream.jStreamInfo;

#if 0
            // reserve code, read new yuv source (ex. scanner) not verify
            JPG_STREAM_DESC     *pJInStreamDesc = &pJpgDev->hJInStream.jStreamDesc;
            if( pJInStreamDesc->jFull_buf )
                pJInStreamDesc->jFull_buf(&pJpgDev->hJInStream, pYuvBufAddr,
                                          slice*N, 0, 0);
#else
            // update input addrY/U/V
            pHJComm->jInBufInfo[0].pBufAddr += (pJpgDev->encSectHight * pJInStreamInfo->jstream.mem[0].pitch);
            switch( pJpgDev->initParam.outColorSpace )
            {
                case JPG_COLOR_SPACE_YUV422:
                    pHJComm->jInBufInfo[1].pBufAddr += (pJpgDev->encSectHight * pJInStreamInfo->jstream.mem[1].pitch);
                    pHJComm->jInBufInfo[2].pBufAddr += (pJpgDev->encSectHight * pJInStreamInfo->jstream.mem[2].pitch);
                    break;

                case JPG_COLOR_SPACE_YUV420:
                    pHJComm->jInBufInfo[1].pBufAddr += ((pJpgDev->encSectHight * pJInStreamInfo->jstream.mem[1].pitch) >> 1);
                    pHJComm->jInBufInfo[2].pBufAddr += ((pJpgDev->encSectHight * pJInStreamInfo->jstream.mem[2].pitch) >> 1);
                    break;
            }
#endif
            // encode fire
            pJpgDev->encSectHight = (remain_H < (int)pJpgDev->encSectHight) ? remain_H : pJpgDev->encSectHight;

            remain_H -= pJpgDev->encSectHight;

            result = jComm_Fire(pHJComm, (remain_H > 0) ? false : true, 0);
            if( result != JPG_ERR_OK )
            {
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg err 0x%x !! ", result);
                break;
            }

            // out stream
            pJOutStreamDesc->jOut_buf(pHOutJStream, pJpgDev->pHJComm->pSysBsBufAddr, pJpgDev->pHJComm->sysValidBsBufSize, 0);

            totalEncLeng += pJpgDev->pHJComm->sysValidBsBufSize;
            pJpgDev->pHJComm->sysValidBsBufSize = 0; // for next section encode
            jpg_sleep(1); 
        }

        jpg_msg(1, " enc size = %f kb\r\n", (float)totalEncLeng/1024);
        *pJpgSize = totalEncLeng;

    }while(0);

    if( result != JPG_ERR_OK )
    {
        jpg_msg_ex(JPG_MSG_TYPE_ERR,"%s() err 0x%x !", __FUNCTION__, result);
    }

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ITEJPG);
    return result;
}
#endif

#if (CONFIG_JPG_CODEC_DESC_ENC_MJPG_DESC)
static JPG_ERR
_jpg_enc_mjpg_setup(
    JPG_DEV     *pJpgDev)
{
    JPG_ERR             result = JPG_ERR_OK;
    JCOMM_INIT_PARAM    jCommInitParam = {0};

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ITEJPG, "0x%x\n", pJpgDev);

    do{
        JCOMM_HANDLE        *pHJComm = pJpgDev->pHJComm;
        JPG_STREAM_HANDLE   *pHInJStream = &pJpgDev->hJInStream;
        int                 i;

        //-----------------------------------------------
        // jpg H/W init
        jCommInitParam.pHInJStream  = &pJpgDev->hJInStream;
        jCommInitParam.pHOutJStream = &pJpgDev->hJOutStream;
        jCommInitParam.codecType    = JPG_CODEC_ENC_MJPG;
        jCommInitParam.encQuality   = pJpgDev->initParam.encQuality;
        jCommInitParam.encWidth     = pJpgDev->initParam.width;
        jCommInitParam.encHeight    = pJpgDev->initParam.height;
        jCommInitParam.encColorSpace = pJpgDev->initParam.outColorSpace;

        result = jComm_Init(pHJComm, &jCommInitParam, 0);
        if( result != JPG_ERR_OK )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " error !!");
            result = JPG_ERR_ALLOCATE_FAIL;
            break;
        }

        pHJComm->pJFrmComp  = &pJpgDev->jPrsInfo.jFrmComp;
        pHJComm->pJDispInfo = &pJpgDev->jDispInfo;

        //---------------------------------
        // set input stream info
        pHInJStream->compCnt = pHInJStream->jStreamInfo.validCompCnt;
        for(i = 0; i < pHInJStream->compCnt; i++)
        {
            pHJComm->jInBufInfo[i].pBufAddr  = pHInJStream->jStreamInfo.jstream.mem[i].pAddr;
            pHJComm->jInBufInfo[i].bufLength = pHInJStream->jStreamInfo.jstream.mem[i].length;
            pHJComm->jInBufInfo[i].pitch     = pHInJStream->jStreamInfo.jstream.mem[i].pitch;
        }

        result = jComm_Setup(pHJComm, 0);

    }while(0);

    if( result != JPG_ERR_OK )
    {
        jpg_msg_ex(JPG_MSG_TYPE_ERR,"%s() err 0x%x !", __FUNCTION__, result);
    }

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ITEJPG);
    return result;
}

static JPG_ERR
_jpg_enc_mjpg_fire(
    JPG_DEV         *pJpgDev,
    JPG_BUF_INFO    *pSysBsBufInfo,
    uint32_t        *pJpgSize)
{
    JPG_ERR             result = JPG_ERR_OK;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ITEJPG, "0x%x\n", pJpgDev);

    do{
        uint32_t            totalEncLeng = 0;
        JCOMM_HANDLE        *pHJComm = pJpgDev->pHJComm;
        JPG_STREAM_HANDLE   *pHOutJStream = &pJpgDev->hJOutStream;
        JPG_STREAM_DESC     *pJOutStreamDesc = &pHOutJStream->jStreamDesc;

        if( !pJOutStreamDesc->jOut_buf )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg err, No out stream API!! ");
            result = JPG_ERR_INVALID_PARAMETER;
            break;
        }

        // output jpg header info
        pJOutStreamDesc->jOut_buf(pHOutJStream, pJpgDev->pHJComm->pJHdrData, pJpgDev->pHJComm->jHdrDataSize, 0);

        // for performance, make jpg H/W directly write to AP Bit Stream buffer.
        pJpgDev->pHJComm->pSysBsBufAddr = (uint8_t*)pHOutJStream->curBsPos;
        pJpgDev->pHJComm->sysBsBufSize  = pHOutJStream->streamSize - pJpgDev->pHJComm->jHdrDataSize;

        result = jComm_Fire(pHJComm, true, 0);
        if( result != JPG_ERR_OK )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg err 0x%x !! ", result);
            break;
        }

        totalEncLeng = pJpgDev->pHJComm->sysValidBsBufSize + pJpgDev->pHJComm->jHdrDataSize;

        jpg_msg(1, " enc size = %f kb\r\n", (float)totalEncLeng/1024);
        *pJpgSize = totalEncLeng;

    }while(0);

    if( result != JPG_ERR_OK )
    {
        jpg_msg_ex(JPG_MSG_TYPE_ERR,"%s() err 0x%x !", __FUNCTION__, result);
    }

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ITEJPG);
    return result;
}
#endif

//=============================================================================
//                  Public Function Definition
//=============================================================================
JPG_ERR
iteJpg_CreateHandle(
    HJPG            **pHJpeg,
    JPG_INIT_PARAM  *pInitParam,
    void            *extraData
    JPG_EXTRA_INFO)
{
    JPG_ERR     result = JPG_ERR_OK;
    JPG_DEV     *pJpgDev = 0;

    _jpg_trace_caller(" %s: caller=%s()[#%d]\n", __FUNCTION__, _caller, _line);
    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ITEJPG, "0x%x, 0x%x, 0x%x\n", pHJpeg, pInitParam, extraData);

    do{
        if (g_jpg_codec_mutex == 0)
        {
            _jpg_mutex_init(JPG_MSG_TYPE_TRACE_ITEJPG, g_jpg_codec_mutex);
        }
        _jpg_mutex_lock(JPG_MSG_TYPE_TRACE_ITEJPG, g_jpg_codec_mutex);

        if( *pHJpeg != 0 )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " error, Exist handle !!");
            result = JPG_ERR_INVALID_PARAMETER;
            break;
        }

        // ------------------------
        // craete dev info
        pJpgDev = jpg_malloc(sizeof(JPG_DEV));
        if( !pJpgDev )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " error, allocate fail !!");
            result = JPG_ERR_ALLOCATE_FAIL;
            break;
        }
        if(pJpgDev)
            memset(pJpgDev, 0x0, sizeof(JPG_DEV));
        if( pInitParam )
            memcpy(&pJpgDev->initParam, pInitParam, sizeof(JPG_INIT_PARAM));

        //--------------------------------------
        // set handle descriptor by codec type
        switch( pInitParam->codecType )
        {
        #if (CONFIG_JPG_CODEC_DESC_ENC_MJPG_DESC)
            case JPG_CODEC_ENC_MJPG:
        #endif
        #if (CONFIG_JPG_CODEC_DESC_ENCODER_DESC)
            case JPG_CODEC_ENC_JPG:
        #endif
        #if (CONFIG_JPG_CODEC_DESC_ENC_MJPG_DESC) || (CONFIG_JPG_CODEC_DESC_ENCODER_DESC)
                if( pInitParam->encQuality > 99 || pInitParam->encQuality < 1 )
                {
                    result = JPG_ERR_INVALID_PARAMETER;
                    jpg_msg_ex(JPG_MSG_TYPE_ERR, " err,  1 < quality(%d) < 99 !", pInitParam->encQuality);
                }

                switch( pInitParam->outColorSpace )
                {
                    case JPG_COLOR_SPACE_YUV422:
                        pJpgDev->initParam.width &= ~0xF;
                        pJpgDev->initParam.height &= ~0xF; // ~0x7;
                        break;
                    case JPG_COLOR_SPACE_YUV420:
                        // yuv420 => enc width or height must be 32 alignment in it9070
                        // I guess that is H/W bug.
                        #if 1
                        if( (pJpgDev->initParam.width & 0x1F) < (pJpgDev->initParam.height & 0x1F) )
                        {
                            pJpgDev->initParam.width &= ~0x1F;
                            pJpgDev->initParam.height &= ~0xF;
                        }
                        else
                        {
                            pJpgDev->initParam.height &= ~0x1F;
                            pJpgDev->initParam.width &= ~0xF;
                        }
						#endif
                        break;
                    default:
                        result = JPG_ERR_INVALID_PARAMETER;
                        jpg_msg_ex(JPG_MSG_TYPE_ERR, " err, enc only support yuv422 or yuv420 !");
                        break;
                }

                if( result == JPG_ERR_OK )
                {
                    // Benson
                    //jpg_msg(1, " alignment (W, H) from (%d, %d) to (%d, %d) !",
                   //     pInitParam->width, pInitParam->height,
                   //     pJpgDev->initParam.width, pJpgDev->initParam.height);
                }
        #endif
        #if (CONFIG_JPG_CODEC_DESC_DEC_MJPG_DESC)
            case JPG_CODEC_DEC_MJPG:
        #endif
            case JPG_CODEC_DEC_JPG_CMD:
            case JPG_CODEC_DEC_JPG:
                jComm_CreateHandle(&pJpgDev->pHJComm, 0);
                break;

            default:
                result = JPG_ERR_INVALID_PARAMETER;
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " err, wrong parameters !");
                break;
        }

    }while(0);

    if( result != JPG_ERR_OK )
    {
        if( pJpgDev )
            iteJpg_DestroyHandle((HJPG**)&pJpgDev, 0 jpg_extra_param);

        _jpg_mutex_unlock(JPG_MSG_TYPE_TRACE_ITEJPG, g_jpg_codec_mutex);
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    (*pHJpeg) = (HJPG)pJpgDev;
    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ITEJPG);
    return result;
}

JPG_ERR
iteJpg_DestroyHandle(
    HJPG            **pHJpeg,
    void            *extraData
    JPG_EXTRA_INFO)
{
    JPG_ERR     result = JPG_ERR_OK;
    JPG_DEV     *pJpgDev = (JPG_DEV*)(*pHJpeg);

    _jpg_trace_caller(" %s: caller=%s()[#%d]\n", __FUNCTION__, _caller, _line);
    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ITEJPG, "0x%x, 0x%x\n", pHJpeg, extraData);
    _jpg_verify_handle(JPG_MSG_TYPE_TRACE_ITEJPG, pHJpeg, 0, result);
    _jpg_verify_handle(JPG_MSG_TYPE_TRACE_ITEJPG, (*pHJpeg), 0, result);

    if( pJpgDev )
    {
        JPG_STREAM_HANDLE   *pHInJStream = &pJpgDev->hJInStream;
        JPG_STREAM_DESC     *pJInStreamDesc = &pJpgDev->hJInStream.jStreamDesc;
        JPG_STREAM_HANDLE   *pHOutJStream = &pJpgDev->hJOutStream;
        JPG_STREAM_DESC     *pJOutStreamDesc = &pJpgDev->hJOutStream.jStreamDesc;
        JPG_PRS_BS_CTRL     *pJPrsBsCtrl = &pJpgDev->jPrsInfo.jPrsBsCtrl;
        JPG_HEAP_TYPE       heapType = JPG_HEAP_DEF;
        JPG_PARSER_INFO     *pJPrsInfo = &pJpgDev->jPrsInfo;
        int                 i;

        switch( pJpgDev->initParam.codecType )
        {
        #if (CONFIG_JPG_CODEC_DESC_DEC_MJPG_DESC)
            case JPG_CODEC_DEC_MJPG:
        #endif
            case JPG_CODEC_DEC_JPG_CMD:
            case JPG_CODEC_DEC_JPG:
                // jpg parser terminate
                heapType = (pJPrsInfo->jFrmComp.bFindHDT == true) ? JPG_HEAP_DEF : JPG_HEAP_STATIC;

                for(i = 0; i < 2; i++)
                {
                    if( pJPrsInfo->jFrmComp.huff_DC[i].pHuffmanTable )
                    {
                        if( pJInStreamDesc->jFree_mem )
                        {
                            pJInStreamDesc->jFree_mem(pHInJStream, heapType, pJPrsInfo->jFrmComp.huff_DC[i].pHuffmanTable);
                        }
                        pJPrsInfo->jFrmComp.huff_DC[i].pHuffmanTable = 0;
                    }

                    if( pJPrsInfo->jFrmComp.huff_AC[i].pHuffmanTable )
                    {
                        if( pJInStreamDesc->jFree_mem )
                        {
                            pJInStreamDesc->jFree_mem(pHInJStream, heapType, pJPrsInfo->jFrmComp.huff_AC[i].pHuffmanTable);
                        }

                        pJPrsInfo->jFrmComp.huff_AC[i].pHuffmanTable = 0;
                    }
                }
                break;

        #if (CONFIG_JPG_CODEC_DESC_ENCODER_DESC)
            case JPG_CODEC_ENC_JPG:
                break;
        #endif

        #if (CONFIG_JPG_CODEC_DESC_ENC_MJPG_DESC)
            case JPG_CODEC_ENC_MJPG:
                break;
        #endif
        }

        jComm_deInit(pJpgDev->pHJComm, 0);

        jComm_DestroyHandle(&pJpgDev->pHJComm, 0);

        if( pJPrsBsCtrl->bsPrsBuf &&
            pJInStreamDesc->jFree_mem )
        {
            pJInStreamDesc->jFree_mem(pHInJStream, JPG_HEAP_BS_BUF, pJPrsBsCtrl->bsPrsBuf);
            pJPrsBsCtrl->bsPrsBuf = 0;
        }

        if( pJInStreamDesc->jClose_stream )
            pJInStreamDesc->jClose_stream(pHInJStream, 0);

        if( pJOutStreamDesc->jClose_stream )
            pJOutStreamDesc->jClose_stream(pHOutJStream, 0);

        free(pJpgDev);
        *pHJpeg = 0;
    }
    _jpg_mutex_unlock(JPG_MSG_TYPE_TRACE_ITEJPG, g_jpg_codec_mutex);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ITEJPG);
    return result;
}

JPG_ERR
iteJpg_SetStreamInfo(
    HJPG            *pHJpeg,
    JPG_STREAM_INFO *pInStreamInfo,
    JPG_STREAM_INFO *pOutStreamInfo,
    void            *extraData
    JPG_EXTRA_INFO)
{
    JPG_ERR     result = JPG_ERR_OK;
    JPG_DEV     *pJpgDev = (JPG_DEV*)pHJpeg;

    _jpg_trace_caller(" %s: caller=%s()[#%d]\n", __FUNCTION__, _caller, _line);
    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ITEJPG, "0x%x, 0x%x, 0x%x, 0x%x\n", pHJpeg, pInStreamInfo, pOutStreamInfo, extraData);
    _jpg_verify_handle(JPG_MSG_TYPE_TRACE_ITEJPG, pHJpeg, 0, result);

    if( pJpgDev && pJpgDev->status != JPG_STATUS_FAIL )
    {
    #define SET_UNKNOW_INFO     0xFF
    #define SET_INPUT_INFO      0x11
    #define SET_OUTPUT_INFO     0xCC

        int                 curState = SET_INPUT_INFO;
        JPG_STREAM_INFO     *pJStreamInfo = 0;
        JPG_STREAM_HANDLE   *pHJtStream = 0;

        while( curState != SET_UNKNOW_INFO )
        {
            JPG_STREAM_DESC     *pJStreamDesc = 0;
            JPG_STREAM_INFO     *pStreamInfo = 0;
            bool                bSkip = true;

            switch( curState )
            {
                case SET_INPUT_INFO:
                    if( pInStreamInfo )
                    {
                        pHJtStream   = &pJpgDev->hJInStream;
                        pJStreamInfo = &pJpgDev->hJInStream.jStreamInfo;
                        pJStreamInfo->streamIOType = JPG_STREAM_IO_READ;
                        bSkip = false;
                    }

                    pStreamInfo = pInStreamInfo;
                    curState = SET_OUTPUT_INFO;
                    break;

                case SET_OUTPUT_INFO:
                    if( pOutStreamInfo )
                    {
                        pHJtStream   = &pJpgDev->hJOutStream;
                        pJStreamInfo = &pJpgDev->hJOutStream.jStreamInfo;
                        pJStreamInfo->streamIOType = JPG_STREAM_IO_WRITE;
                        bSkip = false;
                    }

                    pStreamInfo = pOutStreamInfo;
                    curState = SET_UNKNOW_INFO;
                    break;

                default :
                    break;
            }

            if( bSkip == false )
            {
                //--------------------------------
                // close old stream
                if( pStreamInfo->streamType != JPG_STREAM_UNKNOW )
                {
                    pJStreamDesc = &pHJtStream->jStreamDesc;
                    if( pJStreamDesc->jClose_stream )
                    {
                        while( pHJtStream->bOpened == true )
                        {
	                        result = pJStreamDesc->jClose_stream(pHJtStream, 0);
							pHJtStream->bOpened = false;
	  					 	jpg_sleep(1); // 3
                        }
                    }
                }

                if( pStreamInfo )
                    memcpy(&pHJtStream->jStreamInfo, pStreamInfo, sizeof(JPG_STREAM_INFO));

                //--------------------------------------
                // reset default handle descriptor by input stream type
                switch( pStreamInfo->streamType )
                {
                    case JPG_STREAM_FILE:   pHJtStream->jStreamDesc = jpg_stream_file_desc;   break;
                    case JPG_STREAM_MEM:    pHJtStream->jStreamDesc = jpg_stream_mem_desc;    break;

                    case JPG_STREAM_CUSTOMER:       break;
                    default:
                        result = JPG_ERR_INVALID_PARAMETER;
                        jpg_msg_ex(JPG_MSG_TYPE_ERR, " err, wrong parameters !");
                        break;
                }

                // reset default handler if customer want to do.
                if( pHJtStream->jStreamInfo.jpg_reset_stream_info )
                    pHJtStream->jStreamInfo.jpg_reset_stream_info(pHJtStream, 0);

                //-------------------------------
                // open new stream
                pJStreamDesc = &pHJtStream->jStreamDesc;
                if( pJStreamDesc->jOpen_stream )
                {
                    result = pJStreamDesc->jOpen_stream(pHJtStream, 0);
                    if( result == JPG_ERR_OK )      pHJtStream->bOpened = true;
                }
            }
        }
    }

    if( result != JPG_ERR_OK )
    {
        pJpgDev->status = JPG_STATUS_FAIL;
        jpg_msg_ex(JPG_MSG_TYPE_ERR,"%s() err 0x%x !", __FUNCTION__, result);
    }

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ITEJPG);
    return result;
}

JPG_ERR
iteJpg_Parsing(
    HJPG            *pHJpeg,
    JPG_BUF_INFO    *pEntropyBufInfo,
    void            *extraData)
    //JPG_EXTRA_INFO)
{
    JPG_ERR     result = JPG_ERR_OK;
    JPG_DEV     *pJpgDev = (JPG_DEV*)pHJpeg;

    _jpg_trace_caller(" %s: caller=%s()[#%d]\n", __FUNCTION__, _caller, _line);
    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ITEJPG, "0x%x, 0x%x, 0x%x\n", pHJpeg, pEntropyBufInfo, extraData);
    _jpg_verify_handle(JPG_MSG_TYPE_TRACE_ITEJPG, pHJpeg, 0, result);

    if( pJpgDev && pJpgDev->status != JPG_STATUS_FAIL )
    {
        JPG_PARSER_INFO     *pJPrsInfo = &pJpgDev->jPrsInfo;
        JPG_PRS_BS_CTRL     *pJPrsBsCtrl = &pJPrsInfo->jPrsBsCtrl;
        JPG_STREAM_HANDLE   *pHInJStream = &pJpgDev->hJInStream;
        JPG_STREAM_DESC     *pJInStreamDesc = &pHInJStream->jStreamDesc;
        JPG_FRM_COMP        *pJFrmComp = &pJPrsInfo->jFrmComp;
        JPG_DISP_INFO       *pJDispInfo = &pJpgDev->jDispInfo;
        JPG_EXIF_INFO       *pExifInfo = &pJPrsInfo->jAttrib.exifInfo;
        JPG_RECT                *pJrect = (JPG_RECT*)extraData; //Benson
        uint32_t            max_width = 0, max_height = 0;

        if( pJpgDev->initParam.codecType != JPG_CODEC_DEC_JPG &&
            pJpgDev->initParam.codecType != JPG_CODEC_DEC_JPG_CMD &&
            pJpgDev->initParam.codecType != JPG_CODEC_DEC_MJPG )
        {
            goto end;
        }

        //--------------------------------------
        // check parser bs buf
        if( pJPrsBsCtrl->bsPrsBuf == 0 ) 
        {
            uint32_t      realSzie = 0;

            // check endian
            pJPrsInfo->bLEndian = false;//_isLEndian();

            // heap parser bs buf
            if( pJInStreamDesc->jControl )
                pJInStreamDesc->jControl(pHInJStream, (uint32_t)JPG_STREAM_CMD_GET_BS_BUF_SIZE,
                                       &pJPrsBsCtrl->bsPrsBufMaxLeng, 0);

            if( pJInStreamDesc->jHeap_mem )
            {
                pJPrsBsCtrl->bsPrsBuf = pJInStreamDesc->jHeap_mem(pHInJStream, JPG_HEAP_BS_BUF,
                                                                  pJPrsBsCtrl->bsPrsBufMaxLeng, &realSzie);
            }
            if( pJPrsBsCtrl->bsPrsBuf == 0 )
            {
                result = JPG_ERR_ALLOCATE_FAIL;
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " mallocate fail !!");
                goto end;
            }

            pJPrsBsCtrl->bsPrsBufMaxLeng = realSzie;  // Real heap size can be used to debug, but I just replay to bsBufMaxLeng_A
        }

        //-------------------------------------------------
        // get stream total length
        pJPrsInfo->fileLength = pHInJStream->streamSize;  //>256KB ,using multi section
        pJPrsInfo->remainSize = pJPrsInfo->fileLength;

        //-------------------------------------------------
        // fill buf
        if( pJPrsInfo->fileLength > pJPrsBsCtrl->bsPrsBufMaxLeng &&
            pJpgDev->initParam.codecType != JPG_CODEC_DEC_MJPG )
        {
            // need multi-section
            if( pJInStreamDesc->jFull_buf )
                pJInStreamDesc->jFull_buf(pHInJStream, pJPrsBsCtrl->bsPrsBuf,
                                        pJPrsBsCtrl->bsPrsBufMaxLeng,
                                        &pJPrsBsCtrl->realSize, 0);
            pJPrsInfo->bMultiSection = true;
            if( pJPrsBsCtrl->realSize != pJPrsBsCtrl->bsPrsBufMaxLeng )
            {
                jpg_msg_ex(JPG_MSG_TYPE_ERR, "Fill buffer something wrong !! (%d, %d) ", pJPrsInfo->remainSize, pJPrsBsCtrl->bsPrsBufMaxLeng);
            }
        }
        else
        {
            // one section
            if( pJInStreamDesc->jFull_buf )
                pJInStreamDesc->jFull_buf(pHInJStream, (void*)pJPrsBsCtrl->bsPrsBuf,
                                        pJPrsInfo->fileLength,
                                        &pJPrsBsCtrl->realSize, 0);
            pJPrsInfo->bMultiSection = false;
        }

        pJPrsInfo->remainSize -= pJPrsBsCtrl->realSize;

        pJPrsBsCtrl->b1stSection = true;
        pJPrsBsCtrl->curPos = 0;

        switch( pJpgDev->initParam.codecType )
        {
            case JPG_CODEC_DEC_JPG_CMD:
                max_width  = JPG_MAX_WIDTH;
                max_height = JPG_MAX_HEIGHT;

                pJpgDev->initParam.decType = JPG_DEC_PRIMARY;
                break;

            case JPG_CODEC_DEC_JPG:
                max_width  = JPG_MAX_WIDTH;
                max_height = JPG_MAX_HEIGHT;

                if( pJpgDev->initParam.decType != JPG_DEC_PRIMARY ||
                    pJpgDev->initParam.bExifParsing == true )
                {
                    //----------------------------------------
                    // parsing APP section
                    result = jPrs_AppParser(pHInJStream, pJPrsInfo);
                    if( result != JPG_ERR_OK )
                    {
                        jpg_msg_ex(JPG_MSG_TYPE_ERR, " err ! ");
                        goto end;
                    }

                    //----------------------------------------
                    // set decode bs buffer info and select decode type (primary or thumbnail)
                    // If CMYK jpg, always decode primary.
                    if( pJPrsInfo->jAttrib.bCMYK == true )
                        pJpgDev->initParam.decType = JPG_DEC_PRIMARY;

                    result = _Verify_DecType(pHInJStream, pJPrsInfo, &pJpgDev->initParam);
                    if( result != JPG_ERR_OK )
                    {
                        jpg_msg_ex(JPG_MSG_TYPE_ERR, " err ! ");
                        goto end;
                    }
                }
                break;

            case JPG_CODEC_DEC_MJPG:
                max_width  = MJPG_MAX_WIDTH;
                max_height = MJPG_MAX_HEIGHT;

                pJpgDev->initParam.decType = JPG_DEC_PRIMARY;
                break;

            default:
                result = JPG_ERR_INVALID_PARAMETER;
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " err, wrong parameters !");
                goto end;
                break;
        }

        //----------------------------------------------
        // parsing Base section
        do{
            result = jPrs_BaseParser(pHInJStream, pJPrsInfo);

            // clear table info
            if( result == JPG_ERR_HDER_ONLY_TABLES )
            {
                int              i;
                JPG_HEAP_TYPE    heapType = JPG_HEAP_DEF;

                heapType = (pJPrsInfo->jFrmComp.bFindHDT == true) ? JPG_HEAP_DEF : JPG_HEAP_STATIC;
                for(i = 0; i < 2; i++)
                {
                    if( pJPrsInfo->jFrmComp.huff_DC[i].pHuffmanTable )
                    {
                        if( pJInStreamDesc->jFree_mem )
                        {
                            pJInStreamDesc->jFree_mem(pHInJStream, heapType, pJPrsInfo->jFrmComp.huff_DC[i].pHuffmanTable);
                        }
                        pJPrsInfo->jFrmComp.huff_DC[i].pHuffmanTable = 0;
                    }

                    if( pJPrsInfo->jFrmComp.huff_AC[i].pHuffmanTable )
                    {
                        if( pJInStreamDesc->jFree_mem )
                        {
                            pJInStreamDesc->jFree_mem(pHInJStream, heapType, pJPrsInfo->jFrmComp.huff_AC[i].pHuffmanTable);
                        }
                        pJPrsInfo->jFrmComp.huff_AC[i].pHuffmanTable = 0;
                    }
                }
            }
            else if( result == JPG_ERR_JPROG_STREAM)
            {
              goto end;
            }

        }while( result == JPG_ERR_HDER_ONLY_TABLES );

        if( result != JPG_ERR_OK )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " err ! ");
            goto end;
        }

        if( pJFrmComp->imgWidth == 0 || pJFrmComp->imgHeight == 0 )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " err, jpg stream get wrong W(%d)/H(%d) !! ", pJFrmComp->imgWidth, pJFrmComp->imgHeight);
            result = JPG_ERR_INVALID_PARAMETER;
            goto end;
        }

        // error handle, 1:1 scaling
        if( pJpgDev->initParam.width == 0 || pJpgDev->initParam.height == 0 )
        {
            if( pJpgDev->initParam.codecType != JPG_CODEC_DEC_JPG_CMD )
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " Waring, jpg wrong output resolution W(%d)/H(%d), reset to W(%d)/H(%d) !!",
                                    pJpgDev->initParam.width, pJpgDev->initParam.height,
                                    pJFrmComp->imgWidth, pJFrmComp->imgHeight);

            pJpgDev->initParam.width  = pJFrmComp->imgWidth;
            pJpgDev->initParam.height = pJFrmComp->imgHeight;
        }

        if( pJFrmComp->imgWidth > max_width || pJFrmComp->imgHeight > max_height )
        {
            result = JPG_ERR_HW_NOT_SUPPORT;
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Resolution not support !!");
            goto end;
        }

        if( pJpgDev->initParam.codecType == JPG_CODEC_DEC_MJPG ||
            pJpgDev->initParam.codecType == JPG_CODEC_DEC_JPG_CMD )
        {
            if( pEntropyBufInfo )
            {
                pEntropyBufInfo->pBufAddr  = pJPrsBsCtrl->bsPrsBuf + pJPrsBsCtrl->curPos;
                pEntropyBufInfo->bufLength = pJPrsBsCtrl->realSize - pJPrsBsCtrl->curPos;
            }

            pJDispInfo->dstW = pJDispInfo->srcW = pJFrmComp->imgWidth;
            pJDispInfo->dstH = pJDispInfo->srcH = pJFrmComp->imgHeight;
            pJDispInfo->srcX = pJDispInfo->srcY = 0;
            pJDispInfo->dstX = pJDispInfo->dstY = 0;

			//if(pJpgDev->initParam.codecType == JPG_CODEC_DEC_MJPG )
            //goto end; // when MJPG, jpg parsing finish  
        }

        //----------------------------------
        // Set Orientation, performance issue
        _Set_Exif_Orientation(
            pJpgDev->initParam.decType,
            pJpgDev->initParam.rotType,
            pJpgDev->initParam.bExifOrientation,
            pExifInfo->primaryOrientation,
            pJDispInfo);

        //----------------------------------
        // set display info (fit/cut/center) for H/W module
        switch( pJpgDev->initParam.outColorSpace )
        {
            case JPG_COLOR_SPACE_RGB565:
            case JPG_COLOR_SPACE_ARGB4444:
            case JPG_COLOR_SPACE_ARGB8888:
            case JPG_COLOR_SPACE_YUV422:        // for transcode
            case JPG_COLOR_SPACE_YUV420:        // for transcode
		    case JPG_COLOR_SPACE_YUV444:
                pJDispInfo->outColorSpace = pJpgDev->initParam.outColorSpace;
                break;

            default:
                result = JPG_ERR_INVALID_PARAMETER;
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " err, wrong parameters !");
                break;
        }

        pJDispInfo->colorCtrl = pJpgDev->initParam.colorCtl;
        pJDispInfo->bCMYK     = pJPrsInfo->jAttrib.bCMYK;
        // calculate src/dest range

        switch( pJpgDev->initParam.dispMode )
        {
            case JPG_DISP_FIT:
                {
                    _genParam_Fit(pJDispInfo,
                                  pJpgDev->initParam.width, pJpgDev->initParam.height,
                                  pJFrmComp->imgWidth, pJFrmComp->imgHeight);
                    pJrect->x = pJDispInfo->dstX;
                    pJrect->y = pJDispInfo->dstY;
                    pJrect->h = pJDispInfo->dstH;
                    pJrect->w = pJDispInfo->dstW;
                }
                break;

            case JPG_DISP_CUT:
            case JPG_DISP_CENTER:
            case JPG_DISP_CUT_PART:
                {
                    uint32_t keepPercentage = (pJpgDev->initParam.dispMode == JPG_DISP_CUT_PART) ?
                                              pJpgDev->initParam.keepPercentage :
                                                  ((pJpgDev->initParam.dispMode == JPG_DISP_CENTER) ? 100 : 0);

                    _genParam_CutPart(pJDispInfo,
                                      pJpgDev->initParam.width, pJpgDev->initParam.height,
                                      pJFrmComp->imgWidth, pJFrmComp->imgHeight,
                                      keepPercentage);
                    pJrect->x = pJDispInfo->dstX;
                    pJrect->y = pJDispInfo->dstY;
                    pJrect->h = pJDispInfo->dstH;
                    pJrect->w = pJDispInfo->dstW;
                }
                break;

            default:
                result = JPG_ERR_INVALID_PARAMETER;
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " err, wrong parameters !");
                break;
        }
    }

end:
    if( result != JPG_ERR_OK && result != JPG_ERR_JPROG_STREAM)
    {
        pJpgDev->status = JPG_STATUS_FAIL;
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }
    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ITEJPG);
    return result;
}

JPG_ERR
iteJpg_Setup(
    HJPG            *pHJpeg,
    void            *extraData
    JPG_EXTRA_INFO)
{
    JPG_ERR     result = JPG_ERR_OK;
    JPG_DEV     *pJpgDev = (JPG_DEV*)pHJpeg;

    _jpg_trace_caller(" %s: caller=%s()[#%d]\n", __FUNCTION__, _caller, _line);
    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ITEJPG, "0x%x, 0x%x\n", pHJpeg, extraData);
    _jpg_verify_handle(JPG_MSG_TYPE_TRACE_ITEJPG, pHJpeg, 0, result);

    if( pJpgDev && pJpgDev->status != JPG_STATUS_FAIL )
    {
        switch( pJpgDev->initParam.codecType )
        {
            case JPG_CODEC_DEC_JPG_CMD: result = _jpg_dec_cmd_setup(pJpgDev);   break;
            case JPG_CODEC_DEC_JPG:     result = _jpg_dec_setup(pJpgDev);       break;

        #if (CONFIG_JPG_CODEC_DESC_DEC_MJPG_DESC)
            case JPG_CODEC_DEC_MJPG:    result = _jpg_dec_mjpg_setup(pJpgDev);  break;
        #endif

        #if (CONFIG_JPG_CODEC_DESC_ENCODER_DESC)
            case JPG_CODEC_ENC_JPG:     result = _jpg_enc_setup(pJpgDev);       break;
        #endif

        #if (CONFIG_JPG_CODEC_DESC_ENC_MJPG_DESC)
            case JPG_CODEC_ENC_MJPG:    result = _jpg_enc_mjpg_setup(pJpgDev);  break;
        #endif

            default:
                result = JPG_ERR_INVALID_PARAMETER;
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " err, wrong parameters !");
                break;
        }
    }

    if( result != JPG_ERR_OK )
    {
        pJpgDev->status = JPG_STATUS_FAIL;
        jpg_msg_ex(JPG_MSG_TYPE_ERR,"%s() err 0x%x !", __FUNCTION__, result);
    }

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ITEJPG);
    return result;
}


JPG_ERR
iteJpg_Process(
    HJPG            *pHJpeg,
    JPG_BUF_INFO    *pStreamBufInfo,
    uint32_t        *pJpgSize,
    void            *extraData
    JPG_EXTRA_INFO)
{
    JPG_ERR     result = JPG_ERR_OK;
    JPG_DEV     *pJpgDev = (JPG_DEV*)pHJpeg;

    _jpg_trace_caller(" %s: caller=%s()[#%d]\n", __FUNCTION__, _caller, _line);
    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ITEJPG, "0x%x, 0x%x, 0x%x\n", pHJpeg, pJpgSize, extraData);
    _jpg_verify_handle(JPG_MSG_TYPE_TRACE_ITEJPG, pHJpeg, 0, result);

    if( pJpgDev && pJpgDev->status != JPG_STATUS_FAIL )
    {
        switch( pJpgDev->initParam.codecType )
        {
            case JPG_CODEC_DEC_JPG:
                result = _jpg_dec_fire(pJpgDev);
                break;

            case JPG_CODEC_DEC_JPG_CMD:
            case JPG_CODEC_DEC_MJPG:
                {
                    JPG_PRS_BS_CTRL     *pJPrsBsCtrl = &pJpgDev->jPrsInfo.jPrsBsCtrl;
                    JPG_BUF_INFO        bsBufInfo = {0};

                    if( pStreamBufInfo )
                    {
                        memcpy(&bsBufInfo, pStreamBufInfo, sizeof(JPG_BUF_INFO));
                    }
                    else
                    {
                        bsBufInfo.pBufAddr  = pJPrsBsCtrl->bsPrsBuf + pJPrsBsCtrl->curPos;
                        bsBufInfo.bufLength = pJPrsBsCtrl->realSize - pJPrsBsCtrl->curPos;
                    }

                    if( pJpgDev->initParam.codecType == JPG_CODEC_DEC_JPG_CMD )
                        result = _jpg_dec_cmd_fire(pJpgDev, &bsBufInfo);
                #if (CONFIG_JPG_CODEC_DESC_DEC_MJPG_DESC)
                    else if( pJpgDev->initParam.codecType == JPG_CODEC_DEC_MJPG )
                        result = _jpg_dec_mjpg_fire(pJpgDev, &bsBufInfo);
                #endif
                }
                break;


        #if (CONFIG_JPG_CODEC_DESC_ENCODER_DESC)
            case JPG_CODEC_ENC_JPG:
                if( pJpgSize )
                    result = _jpg_enc_fire(pJpgDev, pStreamBufInfo, pJpgSize);
                break;
        #endif

        #if (CONFIG_JPG_CODEC_DESC_ENC_MJPG_DESC)
            case JPG_CODEC_ENC_MJPG:
                if( pJpgSize )
                    result = _jpg_enc_mjpg_fire(pJpgDev, pStreamBufInfo, pJpgSize);
                break;
        #endif

            default:
                result = JPG_ERR_INVALID_PARAMETER;
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " err, wrong parameters !");
                break;
        }
    }

    if( result != JPG_ERR_OK )
    {
        pJpgDev->status = JPG_STATUS_FAIL;
        jpg_msg_ex(JPG_MSG_TYPE_ERR,"%s() err 0x%x !", __FUNCTION__, result);
    }

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ITEJPG);
    return result;
}

JPG_ERR
iteJpg_SetBaseOutInfo(
    HJPG            *pHJpeg,
    uint32_t        *pWidth,
    uint32_t        *pHeight,
    void            *extraData
    JPG_EXTRA_INFO)
{
    JPG_ERR     result = JPG_ERR_OK;
    JPG_DEV     *pJpgDev = (JPG_DEV*)pHJpeg;

    _jpg_trace_caller(" %s: caller=%s()[#%d]\n", __FUNCTION__, _caller, _line);
    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ITEJPG, "0x%x, 0x%x\n", pHJpeg, extraData);
    _jpg_verify_handle(JPG_MSG_TYPE_TRACE_ITEJPG, pHJpeg, 0, result);

    if( pJpgDev && pJpgDev->status != JPG_STATUS_FAIL )
    {
        JPG_INIT_PARAM      *pInitParam = &pJpgDev->initParam;
        JPG_FRM_COMP        *pJFrmComp = &pJpgDev->jPrsInfo.jFrmComp;
        JPG_DISP_INFO       *pJDispInfo = &pJpgDev->jDispInfo;

        if( pWidth )     pInitParam->width    = *pWidth;
        if( pHeight )    pInitParam->height   = *pHeight;

        // calculate src/dest range
        switch( pJpgDev->initParam.dispMode )
        {
            case JPG_DISP_FIT:
                _genParam_Fit(pJDispInfo,
                              pInitParam->width, pInitParam->height,
                              pJFrmComp->imgWidth, pJFrmComp->imgHeight);
                break;

            case JPG_DISP_CUT:
            case JPG_DISP_CENTER:
            case JPG_DISP_CUT_PART:
                {
                    uint32_t keepPercentage = (pInitParam->dispMode == JPG_DISP_CUT_PART) ?
                                              pInitParam->keepPercentage :
                                                  ((pInitParam->dispMode == JPG_DISP_CENTER) ? 100 : 0);

                    _genParam_CutPart(pJDispInfo,
                                      pInitParam->width, pInitParam->height,
                                      pJFrmComp->imgWidth, pJFrmComp->imgHeight,
                                      keepPercentage);
                }
                break;

            default:
                result = JPG_ERR_INVALID_PARAMETER;
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " err, wrong parameters !");
                break;
        }
    }

    if( result != JPG_ERR_OK )
    {
        pJpgDev->status = JPG_STATUS_FAIL;
        jpg_msg_ex(JPG_MSG_TYPE_ERR,"%s() err 0x%x !", __FUNCTION__, result);
    }

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ITEJPG);
    return result;
}


JPG_ERR
iteJpg_Reset(
    HJPG            *pHJpeg,
    void            *extraData
    JPG_EXTRA_INFO)
{
    JPG_ERR     result = JPG_ERR_OK;
    JPG_DEV     *pJpgDev = (JPG_DEV*)pHJpeg;

    _jpg_trace_caller(" %s: caller=%s()[#%d]\n", __FUNCTION__, _caller, _line);
    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ITEJPG, "0x%x, 0x%x\n", pHJpeg, extraData);
    _jpg_verify_handle(JPG_MSG_TYPE_TRACE_ITEJPG, pHJpeg, 0, result);

    if( pJpgDev )
    {
        JPG_HEAP_TYPE       heapType = JPG_HEAP_DEF;
        JPG_PARSER_INFO     *pJPrsInfo = &pJpgDev->jPrsInfo;
        JPG_STREAM_HANDLE   *pHInJStream = &pJpgDev->hJInStream;
        JPG_STREAM_DESC     *pJInStreamDesc = &pJpgDev->hJInStream.jStreamDesc;
        JCOMM_INIT_PARAM    jCommInitParam = {0};
        int                 i;

        switch( pJpgDev->initParam.codecType )
        {
        #if (CONFIG_JPG_CODEC_DESC_DEC_MJPG_DESC)
            case JPG_CODEC_DEC_MJPG:
        #endif
            case JPG_CODEC_DEC_JPG:
                // jpg parser terminate
                heapType = (pJPrsInfo->jFrmComp.bFindHDT == true) ? JPG_HEAP_DEF : JPG_HEAP_STATIC;
                for(i = 0; i < 2; i++)
                {
                    if( pJPrsInfo->jFrmComp.huff_DC[i].pHuffmanTable )
                    {
                        if( pJInStreamDesc->jFree_mem )
                            pJInStreamDesc->jFree_mem(pHInJStream, heapType, pJPrsInfo->jFrmComp.huff_DC[i].pHuffmanTable);

                        pJPrsInfo->jFrmComp.huff_DC[i].pHuffmanTable = 0;
                    }

                    if( pJPrsInfo->jFrmComp.huff_AC[i].pHuffmanTable )
                    {
                        if( pJInStreamDesc->jFree_mem )
                            pJInStreamDesc->jFree_mem(pHInJStream, heapType, pJPrsInfo->jFrmComp.huff_AC[i].pHuffmanTable);

                        pJPrsInfo->jFrmComp.huff_AC[i].pHuffmanTable = 0;
                    }
                }

                switch( pJpgDev->initParam.codecType )
                {
                    case JPG_CODEC_DEC_JPG:
                        if( pJpgDev->initParam.decType == JPG_DEC_PRIMARY )
                        {
                            result = JPG_ERR_HW_NOT_SUPPORT;
                            break;
                        }
                        else
                            pJpgDev->initParam.decType = JPG_DEC_PRIMARY;

                        if( pJPrsInfo->jPrsBsCtrl.bsPrsBuf &&
                            pJInStreamDesc->jFree_mem )
                        {
                            pJInStreamDesc->jFree_mem(pHInJStream, JPG_HEAP_BS_BUF, pJPrsInfo->jPrsBsCtrl.bsPrsBuf);
                            pJPrsInfo->jPrsBsCtrl.bsPrsBuf = 0;
                        }

                        jComm_deInit(pJpgDev->pHJComm, 0);

                        jCommInitParam.pHInJStream  = &pJpgDev->hJInStream;
                        jCommInitParam.pHOutJStream = &pJpgDev->hJOutStream;

                        if( pJPrsInfo->jAttrib.jBaseLiteInfo[pJpgDev->initParam.decType].bJprog == true )
                            jCommInitParam.codecType = JPG_CODEC_DEC_JPROG;
                        else
                            jCommInitParam.codecType = JPG_CODEC_DEC_JPG;

                        jComm_Init(pJpgDev->pHJComm, &jCommInitParam, 0);

                        if( pJInStreamDesc->jSeek_stream )
                            pJInStreamDesc->jSeek_stream(pHInJStream, 0, JPG_SEEK_SET, 0);

                        pJpgDev->status = JPG_STATUS_IDLE;
                        break;

                    case JPG_CODEC_DEC_MJPG:
                        pJPrsInfo->bSkipHDT = false;
                        pJPrsInfo->jFrmComp.qTable.tableCnt = 0;

						if( pJPrsInfo->jPrsBsCtrl.bsPrsBuf &&  pJInStreamDesc->jFree_mem )
                        {
                            pJInStreamDesc->jFree_mem(pHInJStream, JPG_HEAP_BS_BUF, pJPrsInfo->jPrsBsCtrl.bsPrsBuf);
                            pJPrsInfo->jPrsBsCtrl.bsPrsBuf = 0;
                        }

						#if 0  //Open MotionJpeg should not  turn on/off the power repeatly ,so I mark it, Benson 2016/0412
                        jComm_deInit(pJpgDev->pHJComm, 0);

                        jCommInitParam.pHInJStream  = &pJpgDev->hJInStream;
                        jCommInitParam.pHOutJStream = &pJpgDev->hJOutStream;
                        jCommInitParam.codecType    = JPG_CODEC_DEC_MJPG;
                        jComm_Init(pJpgDev->pHJComm, &jCommInitParam, 0);
						#endif

                        pJpgDev->status = JPG_STATUS_IDLE;
                        break;
                }
                break;

        #if (CONFIG_JPG_CODEC_DESC_ENCODER_DESC)
            case JPG_CODEC_ENC_JPG:
                break;
        #endif

        #if (CONFIG_JPG_CODEC_DESC_ENC_MJPG_DESC)
            case JPG_CODEC_ENC_MJPG:
                break;
        #endif

            default:
                result = JPG_ERR_INVALID_PARAMETER;
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " err, wrong parameters !");
                break;
        }

        // ?? need this ??
    }

    if( result != JPG_ERR_OK )
    {
        jpg_msg_ex(JPG_MSG_TYPE_ERR,"%s() err 0x%x !", __FUNCTION__, result);
    }

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ITEJPG);
    return result;
}

JPG_ERR
iteJpg_WaitIdle(
    HJPG            *pHJpeg,
    void            *extraData
    JPG_EXTRA_INFO)
{
    JPG_ERR     result = JPG_ERR_OK;
    JPG_DEV     *pJpgDev = (JPG_DEV*)pHJpeg;

    _jpg_trace_caller(" %s: caller=%s()[#%d]\n", __FUNCTION__, _caller, _line);
    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ITEJPG, "0x%x, 0x%x\n", pHJpeg, extraData);
    _jpg_verify_handle(JPG_MSG_TYPE_TRACE_ITEJPG, pHJpeg, 0, result);

    if( pJpgDev && pJpgDev->status != JPG_STATUS_FAIL )
    {
        JCOMM_HANDLE        *pHJComm = pJpgDev->pHJComm;

        result = jComm_Control(pHJComm, (uint32_t)JCODEC_CMD_WAIT_IDLE, 0, 0);
        if( result != JPG_ERR_OK )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " error, wait idle time out !! ");
        }

    }

    if( result != JPG_ERR_OK )
    {
        pJpgDev->status = JPG_STATUS_FAIL;
        jpg_msg_ex(JPG_MSG_TYPE_ERR,"%s() err 0x%x !", __FUNCTION__, result);
    }

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ITEJPG);
    return result;
}

JPG_ERR
iteJpg_GetStatus(
    HJPG            *pHJpeg,
    JPG_USER_INFO   *pJpgUserInfo,
    void            *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;
    JPG_DEV     *pJpgDev = (JPG_DEV*)pHJpeg;

    _jpg_verify_handle(JPG_MSG_TYPE_TRACE_ITEJPG, pHJpeg, 0, result);

    if( pJpgDev && pJpgUserInfo )
    {
        JPG_DISP_INFO       *pJDispInfo = &pJpgDev->jDispInfo;
        JPG_FRM_COMP        *pJFrmComp = &pJpgDev->jPrsInfo.jFrmComp;
        JPG_INIT_PARAM      *pJInitParam = &pJpgDev->initParam;

        pJpgUserInfo->status = pJpgDev->status;
        if( pJpgDev->status != JPG_STATUS_FAIL )
        {
            uint16_t             heightUnit;
            uint16_t             widthUnit;
            uint16_t             i;

            pJpgUserInfo->jpgRect.x = pJDispInfo->dstX;
            pJpgUserInfo->jpgRect.y = pJDispInfo->dstY;
            pJpgUserInfo->jpgRect.w = pJDispInfo->dstW;
            pJpgUserInfo->jpgRect.h = pJDispInfo->dstH;


            widthUnit  = (pJFrmComp->jFrmInfo[0].horizonSamp << 3);
            heightUnit = (pJFrmComp->jFrmInfo[0].verticalSamp << 3);

            pJpgUserInfo->real_width  = (pJFrmComp->imgWidth  + (widthUnit - 1)) & ~(widthUnit - 1);
            pJpgUserInfo->real_height = (pJFrmComp->imgHeight + (heightUnit - 1)) & ~(heightUnit - 1);

            pJpgUserInfo->slice_num   = ((pJpgUserInfo->real_height + 0x7) >> 3);

            pJpgUserInfo->imgWidth  = pJFrmComp->imgWidth;
            pJpgUserInfo->imgHeight = pJFrmComp->imgHeight;

            pJpgUserInfo->comp1Pitch = (pJpgUserInfo->real_width + 0x7) & ~0x7; 

			#if (CFG_CHIP_FAMILY != 9920)
            if( (pJInitParam->codecType != JPG_CODEC_ENC_JPG) && (pJInitParam->codecType != JPG_CODEC_ENC_MJPG))
            {
                if (ithGetDeviceId() == 0x9850 && ithGetRevisionId() > 0 &&  ithIsTilingModeOn())
				{
					for(i = 0; i< ((sizeof(PITCH_ARR) / sizeof(PITCH_ARR[0]))- 1); i++)
					{
						if( (PITCH_ARR[i] < pJpgUserInfo->comp1Pitch) && (PITCH_ARR[i+1] >= pJpgUserInfo->comp1Pitch) )
						pJpgUserInfo->comp1Pitch = PITCH_ARR[i+1];
					}  
				}
				else
				{
					if(pJpgUserInfo->comp1Pitch < 2048) pJpgUserInfo->comp1Pitch = 2048;
				}				
            }
			#endif

            if( pJFrmComp->jFrmInfo[0].horizonSamp == 1 &&
                pJFrmComp->jFrmInfo[0].verticalSamp == 1 )
            { // JPG_COLOR_SPACE_YUV444
                pJpgUserInfo->comp23Pitch  = pJpgUserInfo->comp1Pitch;
                pJpgUserInfo->colorFormate = JPG_COLOR_SPACE_YUV444;
            }
            else if( pJFrmComp->jFrmInfo[0].horizonSamp == 1 &&
                     pJFrmComp->jFrmInfo[0].verticalSamp == 2 )
            { // JPG_COLOR_SPACE_YUV422R
                pJpgUserInfo->comp23Pitch = pJpgUserInfo->comp1Pitch;
                pJpgUserInfo->colorFormate = JPG_COLOR_SPACE_YUV422R;
            }
            else
            {
                pJpgUserInfo->comp23Pitch = (pJpgUserInfo->comp1Pitch >> 1);
                if( pJFrmComp->jFrmInfo[0].horizonSamp == 1 &&
                    pJFrmComp->jFrmInfo[0].verticalSamp == 2 )
                {
                    pJpgUserInfo->colorFormate = JPG_COLOR_SPACE_YUV422R;
                }
                else if( pJFrmComp->jFrmInfo[0].horizonSamp == 2 &&
                         pJFrmComp->jFrmInfo[0].verticalSamp == 2 )
                {
                    if( pJFrmComp->jFrmInfo[1].horizonSamp == 1 &&
                        pJFrmComp->jFrmInfo[1].verticalSamp == 2 )
                        pJpgUserInfo->colorFormate = JPG_COLOR_SPACE_YUV422;
                    else
                    {
						#if (CFG_CHIP_FAMILY == 9850)
						pJpgUserInfo->colorFormate = JPG_COLOR_SPACE_YUV422;
						#else
						pJpgUserInfo->colorFormate = JPG_COLOR_SPACE_YUV420;
						#endif  
                    }
                }
                else if( pJFrmComp->jFrmInfo[0].horizonSamp == 4 &&
                         pJFrmComp->jFrmInfo[0].verticalSamp == 1 )
                {
                    pJpgUserInfo->colorFormate = JPG_COLOR_SPACE_YUV411;
                }
                else if( pJFrmComp->jFrmInfo[0].horizonSamp == 2 &&
                         pJFrmComp->jFrmInfo[0].verticalSamp == 1 )
                {
                    pJpgUserInfo->colorFormate = JPG_COLOR_SPACE_YUV422;
                }
            }


			if (ithGetDeviceId() == 0x9850 && ithGetRevisionId() > 0 && ithIsTilingModeOn())
            {
				pJpgUserInfo->colorFormate = JPG_COLOR_SPACE_YUV422;
				pJpgUserInfo->comp23Pitch = pJpgUserInfo->comp1Pitch;   
			}

			switch(pJDispInfo->outColorSpace)
			{
				case JPG_COLOR_SPACE_RGB565:
					pJpgUserInfo->colorFormate = JPG_COLOR_SPACE_RGB565;
					pJpgUserInfo->comp23Pitch = 0;
					break;
					
				case JPG_COLOR_SPACE_ARGB8888:
					pJpgUserInfo->colorFormate = JPG_COLOR_SPACE_ARGB8888;
					pJpgUserInfo->comp23Pitch = pJpgUserInfo->comp1Pitch; // for AlphaPlane pitch.
					break;

				case JPG_COLOR_SPACE_ARGB4444:
					pJpgUserInfo->colorFormate = JPG_COLOR_SPACE_ARGB4444;
					pJpgUserInfo->comp23Pitch = pJpgUserInfo->comp1Pitch; // for AlphaPlane pitch.
					break;
					
				default:
            		break;
			}
				
			//printf("ite_jpg.c pJpgUserInfo->colorFormate = %d\n",pJpgUserInfo->colorFormate);
        }
        else
        {
            if(&pJpgUserInfo->jpgRect)
                memset(&pJpgUserInfo->jpgRect, 0x0, sizeof(JPG_RECT));
        }

    }

    if( result != JPG_ERR_OK )
    {
        jpg_msg_ex(JPG_MSG_TYPE_ERR,"%s() err 0x%x !", __FUNCTION__, result);
    }

    return result;
}


// JPG_ERR
// iteJpg_Template(
//     HJPG            *pHJpeg,
//     void            *extraData)
// {
//     JPG_ERR     result = JPG_ERR_OK;
//     JPG_DEV     *pJpgDev = (JPG_DEV*)pHJpeg;
//
//     _jpg_trace_enter(JPG_MSG_TYPE_TRACE_ITEJPG, "0x%x, 0x%x\n", pHJpeg, extraData);
//     _jpg_verify_handle(JPG_MSG_TYPE_TRACE_ITEJPG, pHJpeg, 0, result);
//
//     if( pJpgDev && pJpgDev->status != JPG_STATUS_FAIL )
//     {
//     }
//
//     if( result != JPG_ERR_OK )
//     {
//         pJpgDev->status = JPG_STATUS_FAIL;
//         jpg_msg_ex(JPG_MSG_TYPE_ERR,"%s() err 0x%x !", __FUNCTION__, result);
//     }
//
//     _jpg_trace_leave(JPG_MSG_TYPE_TRACE_ITEJPG);
//     return result;
// }





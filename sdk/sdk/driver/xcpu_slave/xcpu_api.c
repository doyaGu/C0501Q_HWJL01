#include "xcpu_api.h"
#include "xcpu_msgq.h"
#include "pal/pal.h"
//#include "core_interface.h"

//#if (!defined(WIN32)) && defined(ENABLE_XCPU_MSGQ)

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================
//typedef struct
//{
//    MMP_UINT16              hour;
//    MMP_UINT16              minute;
//} TIME_HM_INFO;
//
//typedef struct
//{
//    MMP_UINT16              year;
//    MMP_UINT8               month;
//    MMP_UINT8               day;
//    MMP_UINT8               hour;
//    MMP_UINT8               minute;
//} TIME_YMDHM_INFO;


//=============================================================================
//                              Extern Reference
//=============================================================================
//=============================================================================
//                              Global Data Definition
//=============================================================================

SMTK_SAVE_CUST_CONFIG gpfSaveCustConfig;
SMTK_LOAD_CUST_CONFIG gpfLoadCustConfig;

//static SYSTEM_CONFIG    gHostAPISystemConfig;
//static MMP_INT32        gCurrentVolume;


//=============================================================================
//                              Private Function Declaration
//=============================================================================

//static void
//_ResetSystemConfig(MMP_BOOL bResetCountryInfo);
//
//static void
//_RestoreSystemConfig(MMP_BOOL bRestoreCountryInfo);


static void
_WCHARConverTo2Byte(
    MMP_UINT16* pOut,
    MMP_WCHAR* pIn,
    MMP_UINT32 length);

//static void
//_MJDBCDConvertToTimeHMInfo(
//    DTV_MJDBCD_TIME mjdbcd,
//    TIME_HM_INFO* pTimeInfo);

static void
_SendReturnMsgToHost(
    MMP_UINT16 msgType,
    MMP_UINT16 msgID,
    MMP_UINT16 result);

static void
_SendBufferDoneMsgToHost(
    MMP_UINT16 msgType,
    MMP_UINT16 msgID,
    MMP_UINT8* pBuffer,
    MMP_BOOL  bIsDataOk);

//=============================================================================
//                              Public Function Definition
//=============================================================================

void
xCpuAPI_GetInputVideoInfo(
    INPUT_VIDEO_INFO* videoInfo)
{
#if 0
    CORE_INPUT_VIDEO_INFO info = {0};

    coreGetInputVideoInfo(&info);

    if (info.src_width == 640 && info.src_height == 480 && info.src_framerate == MMP_CAP_FRAMERATE_60HZ)
        *videoInfo = INPUT_VIDEO_INFO_640X480_60P;
    else if (info.src_width == 720 && info.src_height == 480 && info.src_framerate == MMP_CAP_FRAMERATE_59_94HZ)
    {
        if (info.binterlaced)
            *videoInfo = INPUT_VIDEO_INFO_720X480_59I;
        else
            *videoInfo = INPUT_VIDEO_INFO_720X480_59P;
    }
    else if (info.src_width == 720 && info.src_height == 480 && info.src_framerate == MMP_CAP_FRAMERATE_60HZ)
    {
        if (info.binterlaced)
            *videoInfo = INPUT_VIDEO_INFO_720X480_60I;
        else
            *videoInfo = INPUT_VIDEO_INFO_720X480_60P;
    }
    else if (info.src_width == 720 && info.src_height == 576 && info.src_framerate == MMP_CAP_FRAMERATE_50HZ)
    {
        if (info.binterlaced)
            *videoInfo = INPUT_VIDEO_INFO_720X576_50I;
        else
            *videoInfo = INPUT_VIDEO_INFO_720X576_50P;
    }
    else if (info.src_width == 1280 && info.src_height == 720 && info.src_framerate == MMP_CAP_FRAMERATE_50HZ)
        *videoInfo = INPUT_VIDEO_INFO_1280X720_50P;
    else if (info.src_width == 1280 && info.src_height == 720 && info.src_framerate == MMP_CAP_FRAMERATE_59_94HZ)
        *videoInfo = INPUT_VIDEO_INFO_1280X720_59P;
    else if (info.src_width == 1280 && info.src_height == 720 && info.src_framerate == MMP_CAP_FRAMERATE_60HZ)
        *videoInfo = INPUT_VIDEO_INFO_1280X720_60P;
    else if (info.src_width == 1920 && info.src_height == 1080 && info.src_framerate == MMP_CAP_FRAMERATE_24HZ)
        *videoInfo = INPUT_VIDEO_INFO_1920X1080_24P;
    else if (info.src_width == 1920 && info.src_height == 1080 && info.src_framerate == MMP_CAP_FRAMERATE_50HZ)
    {
        if (info.binterlaced)
            *videoInfo = INPUT_VIDEO_INFO_1920X1080_50I;
        else
            *videoInfo = INPUT_VIDEO_INFO_1920X1080_50P;
    }
    else if (info.src_width == 1920 && info.src_height == 1080 && info.src_framerate == MMP_CAP_FRAMERATE_59_94HZ)
    {
        if (info.binterlaced)
            *videoInfo = INPUT_VIDEO_INFO_1920X1080_59I;
        else
            *videoInfo = INPUT_VIDEO_INFO_1920X1080_59P;
    }
    else if (info.src_width == 1920 && info.src_height == 1080 && info.src_framerate == MMP_CAP_FRAMERATE_60HZ)
    {
        if (info.binterlaced)
            *videoInfo = INPUT_VIDEO_INFO_1920X1080_60I;
        else
            *videoInfo = INPUT_VIDEO_INFO_1920X1080_60P;
    }
    else
        *videoInfo = INPUT_VIDEO_INFO_NONE;
#endif
}

//=============================================================================
//                              Private Function Definition
//=============================================================================
//static
//void
//_ResetSystemConfig(MMP_BOOL bResetCountryInfo)
//{
//    if (bResetCountryInfo==MMP_TRUE)
//    {
//        smtkDtvSetCountryIndex(gHostAPISystemConfig.CountryTableIndex);
//        //smtkDtvSetCountryCode(gHostAPISystemConfig.CountryCode);
//    }
//    else
//    {
//        smtkDtvSetTimeZone(gHostAPISystemConfig.GMTTimeZone);
//        //smtkDtvSetLanguageIndex(gHostAPISystemConfig.Language);
//        smtkDtvSetData(DTV_DATA_LANGUAGE_INDEX, gHostAPISystemConfig.Language);
//        smtkDtvSetLcn((gHostAPISystemConfig.EnableLCN==1)?MMP_TRUE:MMP_FALSE);
//#ifdef ENABLE_MFN
//        //smtkDtvSetMfn((gHostAPISystemConfig.EnableHandOver==1)?MMP_TRUE:MMP_FALSE);
//        if (gHostAPISystemConfig.EnableHandOver==1)
//            smtkDtvSetMfn(DTV_MFN_COMBINATION_MODE, 3, 3);
//        else
//            smtkDtvSetMfn(DTV_MFN_NONE, 0, 0);
//#endif
//
//        if (gHostAPISystemConfig.EnableHandOver==1)
//            smtkDtvSetWeakSignalTimeout(gHostAPISystemConfig.TimeOutforHandover);
//    }
//}

//static
//void
//_RestoreSystemConfig(MMP_BOOL bResetCountryInfo)
//{
//    if (bResetCountryInfo == MMP_TRUE)
//    {
//        //gHostAPISystemConfig.CountryCode=smtkDtvGetCountryCode();
//        gHostAPISystemConfig.CountryTableIndex  = smtkDtvGetCountryIndex();
//    }
//    else
//    {
//        //gHostAPISystemConfig.Language           = smtkDtvGetLanguageIndex();
//        gHostAPISystemConfig.Language           = smtkDtvGetData(DTV_DATA_LANGUAGE_INDEX);
//        gHostAPISystemConfig.EnableLCN          = (MMP_UINT32)smtkDtvGetLcn();
//        gHostAPISystemConfig.GMTTimeZone        = smtkDtvGetTimeZone();
//#ifdef ENABLE_MFN
//        gHostAPISystemConfig.EnableHandOver     = (MMP_UINT32)smtkDtvGetMfn();
//#endif
//        gHostAPISystemConfig.TimeOutforHandover = smtkDtvGetWeakSignalTimeout();
//    }
//}



static
void
_WCHARConverTo2Byte(
    MMP_UINT16* pOut,
    MMP_WCHAR* pIn,
    MMP_UINT32 length)
{
    MMP_UINT32 index=0;
    for(;index<length;++index,++pIn,++pOut)
        *pOut=(*pIn)&0xffff;
}

//static
//void
//_MJDBCDConvertToTimeHMInfo(
//    DTV_MJDBCD_TIME mjdbcd,
//    TIME_HM_INFO* pTimeInfo)
//{
//    pTimeInfo->hour   = ((mjdbcd.low24 & 0xF00000) >> 20)*10+((mjdbcd.low24 & 0x0F0000) >> 16);
//    pTimeInfo->minute = ((mjdbcd.low24 & 0x00F000) >> 12)*10+((mjdbcd.low24 & 0x000F00) >> 8);
//}

static void
_SendReturnMsgToHost(
    MMP_UINT16 msgType,
    MMP_UINT16 msgID,
    MMP_UINT16 result)
{
    XCPU_MSG_OBJ tSToMMsg = {0};
    tSToMMsg.type   = msgType;
    tSToMMsg.id     = msgID;
    tSToMMsg.msg[0] = result;
    xCpuMsgQ_SendMessage(&tSToMMsg);
}

static void
_SendBufferDoneMsgToHost(
    MMP_UINT16 msgType,
    MMP_UINT16 msgID,
    MMP_UINT8* pBuffer ,
    MMP_BOOL  bIsDataOk)
{
    XCPU_MSG_OBJ tSToMMsg = {0};
    tSToMMsg.type   = msgType;
    tSToMMsg.id     = msgID;
    tSToMMsg.msg[0] = (MMP_UINT16)((((MMP_UINT32)pBuffer) & 0xFFFF0000L) >> 16L);
    tSToMMsg.msg[1] = (MMP_UINT16)(((MMP_UINT32)pBuffer) & 0x0000FFFFL);
    tSToMMsg.msg[2] = (MMP_UINT16)bIsDataOk;
    xCpuMsgQ_SendMessage(&tSToMMsg);
}

//#endif


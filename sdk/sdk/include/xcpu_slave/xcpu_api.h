#ifndef XCPU_API_H
#define XCPU_API_H

#include "ite/mmp_types.h"
//#include "core_config.h"

#ifdef __cplusplus
extern "C" {
#endif

//#if (!defined(WIN32)) && defined(ENABLE_XCPU_MSGQ)

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef MMP_BOOL
(*SMTK_SAVE_CUST_CONFIG)(
    void);

typedef MMP_UINT16
(*SMTK_LOAD_CUST_CONFIG)(
    MMP_UINT32 fileSize,
    MMP_UINT8  *pBufferStart);

typedef enum INPUT_VIDEO_INFO_TAG
{
    INPUT_VIDEO_INFO_NONE = 0,
    INPUT_VIDEO_INFO_640X480_60P,
    INPUT_VIDEO_INFO_720X480_59I,
    INPUT_VIDEO_INFO_720X480_59P,
    INPUT_VIDEO_INFO_720X480_60I,
    INPUT_VIDEO_INFO_720X480_60P,
    INPUT_VIDEO_INFO_720X576_50I,
    INPUT_VIDEO_INFO_720X576_50P,
    INPUT_VIDEO_INFO_1280X720_50P,
    INPUT_VIDEO_INFO_1280X720_59P,
    INPUT_VIDEO_INFO_1280X720_60P,
    INPUT_VIDEO_INFO_1920X1080_24P,
    INPUT_VIDEO_INFO_1920X1080_50I,
    INPUT_VIDEO_INFO_1920X1080_50P,
    INPUT_VIDEO_INFO_1920X1080_59I,
    INPUT_VIDEO_INFO_1920X1080_59P,
    INPUT_VIDEO_INFO_1920X1080_60I,
    INPUT_VIDEO_INFO_1920X1080_60P,
} INPUT_VIDEO_INFO;

//=============================================================================
//                              Function Declaration
//=============================================================================

void
xCpuAPI_GetInputVideoInfo(
    INPUT_VIDEO_INFO *videoInfo);

//#endif  // end of #if (!defined(WIN32)) && defined(ENABLE_XCPU_MSGQ)

#ifdef __cplusplus
}
#endif

#endif  // XCPU_API_H
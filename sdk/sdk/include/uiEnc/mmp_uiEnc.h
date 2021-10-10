#ifndef __MMP_UIENC_H_DWIL2GIB_0D3P_9D0N_VV3I_FIB3K0N7BJ21__
#define __MMP_UIENC_H_DWIL2GIB_0D3P_9D0N_VV3I_FIB3K0N7BJ21__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "uiEnc_err.h"
//=============================================================================
//				  Constant Definition
//=============================================================================
#if defined(WIN32)
    #define UIE_API extern //__declspec(dllexport)
#else
    #define UIE_API extern
#endif


/**
 * one symbol unit
 **/
typedef enum UIE_UNIT_TYPE_TAG
{
    UIE_UNIT_2_BYTES = 0,
    UIE_UNIT_4_BYTES = 1,
    
}UIE_UNIT_TYPE;

/**
 * source bytes pre pixel
 **/
typedef enum UIE_BPP_TYPE_TAG
{
    UIE_BPP_2_BYTES = 0,
    UIE_BPP_4_BYTES = 1,
    
}UIE_BPP_TYPE;

/**
 * m bits to record length value
 **/
typedef enum UIE_RL_SIZE_TYPE_TAG
{
    UIE_RL_SIZE_3_BITS = 0,
    UIE_RL_SIZE_4_BITS = 1,
    UIE_RL_SIZE_5_BITS = 2,
    UIE_RL_SIZE_6_BITS = 3,
    UIE_RL_SIZE_7_BITS = 4,
    UIE_RL_SIZE_8_BITS = 5,
    UIE_RL_SIZE_9_BITS = 6,    
    
}UIE_RL_SIZE_TYPE;

/**
 * compress speed delay times
 **/
typedef enum UIE_DELAY_TIMES_TAG
{
    UIE_DELAY_00X    = 0,
    UIE_DELAY_01X    = 1,
    UIE_DELAY_02X    = 2,
    UIE_DELAY_03X    = 3,
    UIE_DELAY_04X    = 4,
    UIE_DELAY_05X    = 5,
    UIE_DELAY_06X    = 6,
    UIE_DELAY_07X    = 7,
    UIE_DELAY_08X    = 8,
    UIE_DELAY_09X    = 9,
    UIE_DELAY_10X    = 10,
    UIE_DELAY_11X    = 11,
    UIE_DELAY_12X    = 12,
    UIE_DELAY_13X    = 13,
    UIE_DELAY_14X    = 14,
    UIE_DELAY_15X    = 15,
    
}UIE_DELAY_TIMES;
//=============================================================================
//				  Macro Definition
//=============================================================================


//=============================================================================
//				  Structure Definition
//=============================================================================
typedef struct _UIE_HANDLE_TAG
{
    // src
    uint8_t           *srcAddr;  // must 64-aligement
    int32_t           srcWidth;
    int32_t           srcHeight;
    int32_t           srcPitch;
    
    // dest
    uint8_t           *destAddr; // must 64-aligement
    int32_t           destLineByte; // like width
    int32_t           destPitch;
    
    // common
    UIE_UNIT_TYPE       unitType;
    UIE_BPP_TYPE        srcBpp;
    UIE_RL_SIZE_TYPE    rlSizeType;
    UIE_DELAY_TIMES     delayTimes;
    
}UIE_HANDLE;

//=============================================================================
//				  Global Data Definition
//=============================================================================


//=============================================================================
//				  Private Function Definition
//=============================================================================


//=============================================================================
//				  Public Function Definition
//=============================================================================
UIE_API UIE_ERR
mmpUiEncCreateHandle(
    UIE_HANDLE      **pHUiE);


UIE_API UIE_ERR
mmpUiEncDestroyHandle(
    UIE_HANDLE      **pHUiE);


UIE_API UIE_ERR
mmpUiEncFire(
    UIE_HANDLE      *hUiE);




#ifdef __cplusplus
}
#endif

#endif


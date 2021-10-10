
#include <stdio.h>
#include "uiEnc_hw.h"
#include "mmp_uiEnc.h"
//=============================================================================
//				  Constant Definition
//=============================================================================


//=============================================================================
//				  Macro Definition
//=============================================================================


//=============================================================================
//				  Structure Definition
//=============================================================================


//=============================================================================
//				  Global Data Definition
//=============================================================================

UIE_UINT32  uieMsgOnFlag = 0x1;
//=============================================================================
//				  Private Function Definition
//=============================================================================


//=============================================================================
//				  Public Function Definition
//=============================================================================
UIE_ERR
mmpUiEncCreateHandle(
    UIE_HANDLE      **pHUiE)
{
    UIE_ERR     result = UIE_ERR_OK;
    UIE_HANDLE  *pUiE = UIE_NULL;
    
    if( *pHUiE )
    {
        uie_msg_ex(UIE_MSG_TYPE_ERR, " error, Exist uiEnc handle !!");
        result = UIE_ERR_INVALID_PARAMETER;
        goto end;
    }
    
    pUiE = (*pHUiE) = uie_Malloc(sizeof(UIE_HANDLE));
    uie_Memset((void*)pUiE, 0x0, sizeof(UIE_HANDLE));

    UIE_PowerUp();

end:    
    return result;
}


UIE_ERR
mmpUiEncDestroyHandle(
    UIE_HANDLE      **pHUiE)
{
    UIE_ERR     result = UIE_ERR_OK;
    
    if( *pHUiE )
    {
        uie_Free(*pHUiE);
        *pHUiE = UIE_NULL;

        UIE_PowerDown();
    }    
    
    return result;
}

UIE_ERR
mmpUiEncFire(
    UIE_HANDLE      *hUiE)
{
    UIE_ERR         result = UIE_ERR_OK;
    UIE_SRC_INFO    srcInfo = {0};
    UIE_DST_INFO    dstInfo = {0};
    
    if( hUiE == UIE_NULL )
    {
        uie_msg_ex(UIE_MSG_TYPE_ERR, " error, Null Pointer !!");
        result = UIE_ERR_NULL_POINTER;
        goto end;
    }

    if( ((UIE_UINT32)hUiE->srcAddr & UIE_MSK_SRC_ADDR_L) & ~UIE_MSK_SRC_ADDR_L )
    {
        uie_msg_ex(UIE_MSG_TYPE_ERR, " err, src addr must 16-aligement !!");
        result = UIE_ERR_INVALID_PARAMETER;
        goto end;
    }

    if( ((UIE_UINT32)hUiE->destAddr & UIE_MSK_DST_ADDR_L) & ~UIE_MSK_DST_ADDR_L )
    {
        uie_msg_ex(UIE_MSG_TYPE_ERR, " err, dest addr must 16-aligement !!");
        result = UIE_ERR_INVALID_PARAMETER;
        goto end;
    }

    srcInfo.src_w     = hUiE->srcWidth;
    srcInfo.src_h     = hUiE->srcHeight;
    srcInfo.src_pitch = hUiE->srcPitch;
    srcInfo.srcAddr_H = (UIE_UINT16)(((UIE_UINT32)hUiE->srcAddr & 0xFFFF0000) >> 16);
    srcInfo.srcAddr_L = (UIE_UINT16)((UIE_UINT32)hUiE->srcAddr & 0x0000FFFF);

    dstInfo.dst_pitch = hUiE->destPitch;
    dstInfo.lineBytes = hUiE->destLineByte;
    dstInfo.dstAddr_H = (UIE_UINT16)(((UIE_UINT32)hUiE->destAddr & 0xFFFF0000) >> 16);
    dstInfo.dstAddr_L = (UIE_UINT16)((UIE_UINT32)hUiE->destAddr & 0x0000FFFF);
/*
    result = UIE_WaitIdle();
    if( result != UIE_ERR_OK )
    {
        uie_msg_ex(UIE_MSG_TYPE_ERR, " err Engine not idle !!");
        goto end;
    }
//*/

    UIE_SetSrcInfo_Reg(&srcInfo);
    UIE_SetDstInfo_Reg(&dstInfo);
    UIE_SetEncParam_1(hUiE->rlSizeType, hUiE->unitType, hUiE->srcBpp);
    UIE_SetEncParam_2(hUiE->delayTimes);
    UIE_Fire_Reg();

    result = UIE_WaitIdle();
    if( result != UIE_ERR_OK )
    {
        uie_msg_ex(UIE_MSG_TYPE_ERR, " err Engine not idle !!");
        goto end;
    }

end:    
    return result;    
}



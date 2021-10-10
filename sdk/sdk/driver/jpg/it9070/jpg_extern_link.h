#ifndef __jpg_extern_link_H_jidmhNLV_tzuY_6odF_UeVG_5z0aH5bzQlIp__
#define __jpg_extern_link_H_jidmhNLV_tzuY_6odF_UeVG_5z0aH5bzQlIp__

#ifdef __cplusplus
extern "C" {
#endif

#include "jpg_defs.h"
#include "ite_jpg.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum JEL_CTRL_CMD_TAG
{
    JEL_CTRL_UNKNOW         = 0,
    JEL_CTRL_ISP_INIT,
    JEL_CTRL_ISP_TERMINATE,
    JEL_CTRL_ISP_SET_DISP_INFO,
    JEL_CTRL_ISP_HW_RESET,
    JEL_CTRL_ISP_WAIT_IDLE,
    JEL_CTRL_ISP_IMG_PROC,
    JEL_CTRL_ISP_POWERDOWN,
    JEL_CTRL_HOST_W_MEM,
    JEL_CTRL_HOST_R_MEM,
    JEL_CTRL_CPU_INVALD_CACHE,

}JEL_CTRL_CMD;

typedef enum JEL_SET_ISP_MODE_TAG
{
    JEL_SET_ISP_UNKNOW      = 0,
    JEL_SET_ISP_SHOW_IMAGE,
    JEL_SET_ISP_COLOR_TRANSFORM,

}JEL_SET_ISP_MODE;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct JPG_MEM_MOVE_INFO_TAG
{
    uint32_t    srcAddr;
    uint32_t    dstAddr;
    uint32_t    sizeByByte;

}JPG_MEM_MOVE_INFO;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
JPG_ERR
Jpg_Ext_Link_Ctrl(
    JEL_CTRL_CMD    cmd,
    uint32_t        *value,
    void            *extraData);



#if (_MSC_VER) && !(ENABLE_JPG_SW_SIMULSTION)
    static void
    _jpg_reflash_vram(
        JPG_STREAM_HANDLE   *pHJStream,
        uint32_t            cmd,
        uint32_t            sysRamAddr,
        uint32_t            length,
        uint8_t             **pVRamAddr)
    {
        uint32_t            realVramAddr = 0;
        JPG_MEM_MOVE_INFO   jMemInfo = {0};

        if( pHJStream->jStreamDesc.jControl )
            pHJStream->jStreamDesc.jControl(pHJStream, cmd, &realVramAddr, 0);

        if( pVRamAddr )  *pVRamAddr = (uint8_t*)realVramAddr;

        // move data to vram
        if( sysRamAddr )
        {
            jMemInfo.dstAddr    = (uint32_t)realVramAddr;
            jMemInfo.srcAddr    = (uint32_t)sysRamAddr;
            jMemInfo.sizeByByte = length;
            Jpg_Ext_Link_Ctrl(JEL_CTRL_HOST_W_MEM, 0, (void*)&jMemInfo);
        }
    }

    static void
    _jpg_dump_vram(
        JPG_STREAM_HANDLE   *pHJStream,
        uint32_t            cmd,
        uint32_t            sysRamAddr,
        uint32_t            length)
    {
        uint32_t            realVramAddr = 0;
        JPG_MEM_MOVE_INFO   jMemInfo = {0};

        if( pHJStream->jStreamDesc.jControl )
            pHJStream->jStreamDesc.jControl(pHJStream, cmd, &realVramAddr, 0);

        // move data to vram
        if( sysRamAddr )
        {
            printf("\n\n\tsysRamAddr=0x%x, realVramAddr=0x%x\n", sysRamAddr, realVramAddr);
            jMemInfo.dstAddr    = (uint32_t)sysRamAddr;
            jMemInfo.srcAddr    = (uint32_t)realVramAddr;
            jMemInfo.sizeByByte = length;
            Jpg_Ext_Link_Ctrl(JEL_CTRL_HOST_R_MEM, 0, (void*)&jMemInfo);
        }
    }
#else
    #define _jpg_reflash_vram(pHJStream, cmd, sysRamAddr, length, vRamAddr)
    #define _jpg_dump_vram(pHJStream, cmd, sysRamAddr, length)
#endif


#ifdef __cplusplus
}
#endif

#endif

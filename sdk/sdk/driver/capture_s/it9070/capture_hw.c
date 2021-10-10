#include <unistd.h>
#include <stdio.h>
#include "capture_s/capture_types.h"
#include "capture_reg.h"
#include "capture_hw.h"

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

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
void
CaptureControllerHardware_Run(void)
{
    ithWriteRegH(CAP_REG_START_CAP, 0x0001);
}

void
CaptureControllerHardware_Stop(void)
{
    ithWriteRegH(CAP_REG_START_CAP, 0x0000);
}

void
CaptureControllerHardware_SetColorFormat(MMP_UINT8 YUV422Format)
{
    //YUV format
    ithWriteRegMaskH(CAP_REG_SET01, (YUV422Format << CAP_SHT_YUVFORMAT), CAP_MSK_YUVFORMAT);
}

void
CaptureControllerHardware_SetInterleaveMode(CAP_INPUT_VIDEO_FORMAT Interleave)
{
    /* Set Interlace or Progressive */
    ithWriteRegMaskH(CAP_REG_SET01, (Interleave << CAP_SHT_INTERLEAVEMODE), CAP_MSK_INTERLEAVEMODE);
}

MMP_UINT16
CaptureControllerHardwareGetBufferIndex(void)
{
    return ithReadRegH(CAP_REG_BUF_WRITEIDX) & CAP_MSK_BUF_WRITEIDX;
}

// TODO: split this function into more meaningful functions
void
CaptureControllerHardware_SetInputDataInfo(CAP_INPUT_INFO *pIninfo)
{
    //Top Field pol;
    ithWriteRegMaskH(CAP_REG_SET01, (pIninfo->TopFieldPol << CAP_SHT_TOPFIELDPOL), CAP_MSK_TOPFIELDPOL);

    /* Set Hsync & Vsync Porlarity */
    ithWriteRegMaskH(CAP_REG_SET01, (pIninfo->HSyncPol << CAP_SHT_HSYNCPOL), CAP_MSK_HSYNCPOL);
    ithWriteRegMaskH(CAP_REG_SET01, (pIninfo->VSyncPol << CAP_SHT_VSYNCPOL), CAP_MSK_VSYNCPOL);

    /* Set YUV pitch */
    ithWriteRegH(CAP_REG_PITCHY, pIninfo->PitchY);
    ithWriteRegH(CAP_REG_PITCHUV, pIninfo->PitchUV);

    /*  Set Active Region  Set CapWidth & Cap Height  */
    ithWriteRegH(CAP_REG_WIDTH, (pIninfo->capwidth) & CAP_MSK_WIDTH);
    ithWriteRegH(CAP_REG_HEIGHT, (pIninfo->capheight) & CAP_MSK_HEIGHT);

    ithWriteRegH(CAP_REG_HSYNC_ACT_START_IDX, pIninfo->HNum1);
    ithWriteRegH(CAP_REG_HSYNC_ACT_END_IDX, pIninfo->HNum2);
    ithWriteRegH(CAP_REG_VSYNC_ACT_TOP_START_IDX, pIninfo->LineNum1);
    ithWriteRegH(CAP_REG_VSYNC_ACT_TOP_END_IDX, pIninfo->LineNum2);
    ithWriteRegH(CAP_REG_VSYNC_ACT_BTM_START_IDX, pIninfo->LineNum3);
    ithWriteRegH(CAP_REG_VSYNC_ACT_BTM_END_IDX, pIninfo->LineNum4);
}

void
CaptureControllerHardware_Reset(void)
{
    ithSetRegBitH(ITH_CAP_CLK_REG, ITH_EN_CAPC_RST_BIT);
    MMP_Sleep(1);
    ithClearRegBitH(ITH_CAP_CLK_REG, ITH_EN_CAPC_RST_BIT);
    MMP_Sleep(1);
}

void
CaptureControllerHardware_DumpAllRegister(void)
{
    uint16_t i;

    for (i = 0; i <= 0x4c; i = i + 2)
    {
        int reg = (REG_CAP_BASE + i);
        printf("reg(0x%x)=0x%x,", reg, ithReadRegH(reg));
    }
    printf("\n");
}

void
CaptureControllerHardware_UseTripleBuffer(MMP_BOOL flag)
{
    //setting Double or Triple buffer
    ithWriteRegMaskH(CAP_REG_SET01, (flag << CAP_SHT_TRIBUF), CAP_MSK_TRIBUF);
}

void
CaptureControllerHardware_SetDataEnable(MMP_BOOL EnDEMode)
{
    ithWriteRegMaskH(CAP_REG_SET01, (EnDEMode << CAP_SHT_DEMODE), CAP_MSK_DEMODE);
}

void
CaptureControllerHardware_SetInputProtocol(CAP_INPUT_PROTOCOL_INFO pInputProtocol)
{
    ithWriteRegMaskH(CAP_REG_SET01, (pInputProtocol << CAP_SHT_CCIRMODE), CAP_MSK_CCIRMODE);
}

void
CaptureControllerHardware_SetBufferAddress(
    MMP_UINT32 *pAddr)
{
    ithWriteRegH(CAP_REG_Y0_BUF_ADDR_L, (pAddr[0] & CAP_MSK_BUF_ADDR_L));
    ithWriteRegH(CAP_REG_Y0_BUF_ADDR_H, ((pAddr[0] >> 16) & CAP_MSK_BUF_ADDR_H));
    ithWriteRegH(CAP_REG_U0_BUF_ADDR_L, (pAddr[1] & CAP_MSK_BUF_ADDR_L));
    ithWriteRegH(CAP_REG_U0_BUF_ADDR_H, ((pAddr[1] >> 16) & CAP_MSK_BUF_ADDR_H));
    ithWriteRegH(CAP_REG_V0_BUF_ADDR_L, (pAddr[2] & CAP_MSK_BUF_ADDR_L));
    ithWriteRegH(CAP_REG_V0_BUF_ADDR_H, ((pAddr[2] >> 16) & CAP_MSK_BUF_ADDR_H));

    ithWriteRegH(CAP_REG_Y1_BUF_ADDR_L, (pAddr[3] & CAP_MSK_BUF_ADDR_L));
    ithWriteRegH(CAP_REG_Y1_BUF_ADDR_H, ((pAddr[3] >> 16) & CAP_MSK_BUF_ADDR_H));
    ithWriteRegH(CAP_REG_U1_BUF_ADDR_L, (pAddr[4] & CAP_MSK_BUF_ADDR_L));
    ithWriteRegH(CAP_REG_U1_BUF_ADDR_H, ((pAddr[4] >> 16) & CAP_MSK_BUF_ADDR_H));
    ithWriteRegH(CAP_REG_V1_BUF_ADDR_L, (pAddr[5] & CAP_MSK_BUF_ADDR_L));
    ithWriteRegH(CAP_REG_V1_BUF_ADDR_H, ((pAddr[5] >> 16) & CAP_MSK_BUF_ADDR_H));

#ifdef TRIPLE_BUFFER
    ithWriteRegH(CAP_REG_Y2_BUF_ADDR_L, (pAddr[6] & CAP_MSK_BUF_ADDR_L));
    ithWriteRegH(CAP_REG_Y2_BUF_ADDR_H, ((pAddr[6] >> 16) & CAP_MSK_BUF_ADDR_H));
    ithWriteRegH(CAP_REG_U2_BUF_ADDR_L, (pAddr[7] & CAP_MSK_BUF_ADDR_L));
    ithWriteRegH(CAP_REG_U2_BUF_ADDR_H, ((pAddr[7] >> 16) & CAP_MSK_BUF_ADDR_H));
    ithWriteRegH(CAP_REG_V2_BUF_ADDR_L, (pAddr[8] & CAP_MSK_BUF_ADDR_L));
    ithWriteRegH(CAP_REG_V2_BUF_ADDR_H, ((pAddr[8] >> 16) & CAP_MSK_BUF_ADDR_H));
#endif
}

void
Cap_EnableClock(
    void)
{
    ithSetRegBitH(ITH_CAP_CLK_REG, ITH_EN_M17CLK_BIT);
    ithSetRegBitH(ITH_SENSOR_CLK_REG, ITH_SENSOR_CLK_DIV_EN_BIT);
}

void
Cap_DisableClock(
    void)
{
    ithClearRegBitH(ITH_CAP_CLK_REG, ITH_EN_M17CLK_BIT);
    ithClearRegBitH(ITH_SENSOR_CLK_REG, ITH_SENSOR_CLK_DIV_EN_BIT);
}
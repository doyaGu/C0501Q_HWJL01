

#include <stdio.h>
#include "uiEnc_hw.h"
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


//=============================================================================
//				  Private Function Definition
//=============================================================================


//=============================================================================
//				  Public Function Definition
//=============================================================================
void 
UIE_PowerUp(
    void)
{
    ////HOST_WriteRegisterMask(0x004A, 0xFFFF, (0x1000 | 0x0002)); // enable clock
    //HOST_WriteRegisterMask(0x004A, 0xFFFF, 0x1000); // reset register
    //HOST_WriteRegisterMask(0x004A, 0x0000, 0x1000); // reset register

    uie_EnginClockOn();
    uie_EnginReset();
}


void 
UIE_PowerDown(
    void)
{
    //HOST_WriteRegisterMask(0x004A, 0xFFFF, 0x1000); // reset register
    //HOST_WriteRegisterMask(0x004A, 0x0000, 0x1000); // reset register
    ////HOST_WriteRegisterMask(0x004A, 0x0000, 0x0002); // disable clock

    uie_EnginClockOff();
}

void
UIE_Fire_Reg(
    void)
{
    uie_WriteHwReg(UIE_REG_FIRE, (UIE_BIT_FIRE << UIE_SHT_FIRE));
}

void 
UIE_SetEncParam_1(
    UIE_INT     runSize,
    UIE_INT     unitSize,
    UIE_INT     srcBpp)
{
    UIE_UINT16  value = 0;
    
    value = ((runSize & UIE_MSK_RUN_SIZE) << UIE_SHT_RUN_SIZE) |
            ((unitSize & UIE_MSK_UNIT_SIZE) << UIE_SHT_UNIT_SIZE) |
            ((srcBpp & UIE_MSK_SRC_BPP) << UIE_SHT_SRC_BPP);
            
    uie_WriteHwReg(UIE_REG_SET_ENC_PARAM_1, value);
    
}

void 
UIE_SetEncParam_2(
    UIE_INT     engDelay)
{
    uie_WriteHwReg(UIE_REG_SET_ENC_PARAM_2, (UIE_UINT16)((engDelay & UIE_MSK_ENC_DELAY) << UIE_SHT_ENC_DELAY));
}

void
UIE_SetSrcInfo_Reg(
    UIE_SRC_INFO    *srcInfo)
{
    // set src width
    uie_WriteHwReg(UIE_REG_SRC_WIDTH, (UIE_UINT16)(srcInfo->src_w & UIE_MSK_SRC_WIDTH));
    
    // set src height
    uie_WriteHwReg(UIE_REG_SRC_HEIGHT, (UIE_UINT16)(srcInfo->src_h & UIE_MSK_SRC_HEIGHT));
    
    // set src pitch
    uie_WriteHwReg(UIE_REG_SRC_PITCH, (UIE_UINT16)(srcInfo->src_pitch & UIE_MSK_SRC_PITCH));
    
    // set src addr low
    uie_WriteHwReg(UIE_REG_SRC_ADDR_L, (UIE_UINT16)(srcInfo->srcAddr_L & UIE_MSK_SRC_ADDR_L));
    // set src addr high
    uie_WriteHwReg(UIE_REG_SRC_ADDR_H, (UIE_UINT16)(srcInfo->srcAddr_H & UIE_MSK_SRC_ADDR_H));

}

void
UIE_SetDstInfo_Reg(
    UIE_DST_INFO    *dstInfo)
{
    // set dest line bytes
    uie_WriteHwReg(UIE_REG_DST_LINE_BYTE, (UIE_UINT16)(dstInfo->lineBytes & UIE_MSK_DST_LINE_BYTE));

    // set dest pitch
    uie_WriteHwReg(UIE_REG_DST_PITCH, (UIE_UINT16)(dstInfo->dst_pitch & UIE_MSK_DST_PITCH));
    
    // set dest addr low
    uie_WriteHwReg(UIE_REG_DST_ADDR_L, (UIE_UINT16)(dstInfo->dstAddr_L & UIE_MSK_DST_ADDR_L));
    // set dest addr high
    uie_WriteHwReg(UIE_REG_DST_ADDR_H, (UIE_UINT16)(dstInfo->dstAddr_H & UIE_MSK_DST_ADDR_H));
}

UIE_UINT16
UIE_GetEngStatus(
    void)
{
    UIE_UINT16      value = 0;
    
    uie_ReadHwReg(UIE_REG_ENG_STATUS, &value);
    
    return ((value & UIE_MSK_ENG_STATUS) >> 1);
}

UIE_ERR
UIE_WaitIdle(
    void)
{
    UIE_ERR     result = UIE_ERR_OK;
    UIE_INT     timeOut = 0;

    while( UIE_GetEngStatus() == UIE_ENG_BUSY )
    {
        uie_sleep(3);
       
        timeOut++;
        if( timeOut > UIE_TIME_OUT_CNT )
        {
            uie_msg_ex(UIE_MSG_TYPE_ERR, " err, Time Out !!");
            result = UIE_ERR_TIME_OUT;
            goto end;
        }
    }

end:
    return result;
}

void
UIE_RegLog(
    void)
{
    UIE_UINT16      value = 0;
    
    uie_ReadHwReg(UIE_REG_FIRE, &value);
    printf("  Reg[0x%x]  0x%04x\n", UIE_REG_FIRE, value);
    uie_ReadHwReg(UIE_REG_SET_ENC_PARAM_1, &value);
    printf("  Reg[0x%x]  0x%04x\n", UIE_REG_SET_ENC_PARAM_1, value);
    uie_ReadHwReg(UIE_REG_SET_ENC_PARAM_2, &value);
    printf("  Reg[0x%x]  0x%04x\n", UIE_REG_SET_ENC_PARAM_2, value);
    
    //src
    uie_ReadHwReg(UIE_REG_SRC_WIDTH, &value);
    printf("  Reg[0x%x]  0x%04x\n", UIE_REG_SRC_WIDTH, value);
    uie_ReadHwReg(UIE_REG_SRC_HEIGHT, &value);
    printf("  Reg[0x%x]  0x%04x\n", UIE_REG_SRC_HEIGHT, value);
    uie_ReadHwReg(UIE_REG_SRC_PITCH, &value);
    printf("  Reg[0x%x]  0x%04x\n", UIE_REG_SRC_PITCH, value);
    uie_ReadHwReg(UIE_REG_SRC_ADDR_L, &value);
    printf("  Reg[0x%x]  0x%04x\n", UIE_REG_SRC_ADDR_L, value);
    uie_ReadHwReg(UIE_REG_SRC_ADDR_H, &value);
    printf("  Reg[0x%x]  0x%04x\n", UIE_REG_SRC_ADDR_H, value);
    
    // dest
    uie_ReadHwReg(UIE_REG_DST_LINE_BYTE, &value);
    printf("  Reg[0x%x]  0x%04x\n", UIE_REG_DST_LINE_BYTE, value);
    uie_ReadHwReg(UIE_REG_DST_PITCH, &value);
    printf("  Reg[0x%x]  0x%04x\n", UIE_REG_DST_PITCH, value);
    uie_ReadHwReg(UIE_REG_DST_ADDR_L, &value);
    printf("  Reg[0x%x]  0x%04x\n", UIE_REG_DST_ADDR_L, value);
    uie_ReadHwReg(UIE_REG_DST_ADDR_H, &value);
    printf("  Reg[0x%x]  0x%04x\n", UIE_REG_DST_ADDR_H, value);
    
    // status
    uie_ReadHwReg(UIE_REG_ENG_STATUS, &value);
    printf("  Reg[0x%x]  0x%04x\n", UIE_REG_ENG_STATUS, value);    
}



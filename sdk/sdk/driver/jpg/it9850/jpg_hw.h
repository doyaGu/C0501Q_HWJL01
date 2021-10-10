#ifndef __jpg_hw_H_q7IQErpq_NjBJ_RnXf_FFeU_vVBgZPrzqrmb__
#define __jpg_hw_H_q7IQErpq_NjBJ_RnXf_FFeU_vVBgZPrzqrmb__

#ifdef __cplusplus
extern "C" {
#endif


#include "jpg_reg.h"
#include "jpg_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define JPG_REG                 uint16_t
#define JPG_TIMEOUT_COUNT       3000 //1000
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
extern uint16_t jpgCompCtrl[5];
extern JPG_YUV_TO_RGB yuv2RgbMatrix;

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
/////////////////////////////////////////////////
// Base Engine API
////////////////////////////////////////////////
#if 1
    void _LogReg(bool bMsgOn);
    #define JPG_LogReg(bMsgOn)    _LogReg(bMsgOn)
	#define JPG_RemapLogReg(bMsgOn)  _LogRemapReg(bMsgOn)
#endif

void
JPG_RegReset(
    void);


void
JPG_PowerUp(
    void);


void
JPG_EncPowerDown(
    void);


void
JPG_DecPowerDown(
    void);

/////////////////////////////////////////////////
// Access Register API
////////////////////////////////////////////////
// 0x00
void
JPG_SetCodecCtrlReg(
    JPG_REG   data);

void
JPG_SetBitstreamReadBytePosReg(
    JPG_REG   data);

// 0x02
void
JPG_SetDriReg(
    JPG_REG   data);

// 0x04
void
JPG_SetTableSpecifyReg(
    const JPG_FRM_COMP      *frmComp);

// 0x06, 0x08, 0xF0~0xFA
void
JPG_SetFrmSizeInfoReg(
    const JPG_FRM_SIZE_INFO     *sizeInfo);

// 0x0A~0x1A
void
JPG_SetLineBufInfoReg(
    const JPG_LINE_BUF_INFO     *lineBufInfo);

// 0x16
void
JPG_SetLineBufSliceUnitReg(
    JPG_REG   data,
    JPG_REG   yVerticalSamp);

// 0x1C
void
JPG_SetLineBufSliceWriteNumReg(
    JPG_REG   data);

// 0x1E, 0x20, 0x22, 0x24
void
JPG_SetBitStreamBufInfoReg(
    const JPG_HW_BS_CTRL   *bsBufInfo);

// 0x26, 0x28
void
JPG_SetBitstreamBufRwSizeReg(
    uint32_t    data);

// 0x2A, 0x2D
void
JPG_SetSamplingFactorReg(
    const JPG_FRM_COMP      *frmComp);

// 0x2E~0xED
// input the Zig-zag order
void
JPG_SetQtableReg(
    JPG_FRM_COMP    *frmComp);

// 0xEE
void
JPG_DropHv(
    JPG_REG   data);

// 0xFC
void
JPG_StartReg(
    void);

// 0xFE
JPG_REG
JPG_GetEngineStatusReg(
    void);

// 0x100
JPG_REG
JPG_GetProcStatusReg( //JPG_GetEngineStatus1Reg(
    void);

// 0x102
void
JPG_SetLineBufCtrlReg(
    JPG_REG   data);

// 0x104
JPG_REG
JPG_GetLineBufValidSliceReg(
    void);

// 0x106
void
JPG_SetBitstreamBufCtrlReg(
    JPG_REG   data);

// 0x108
uint32_t
JPG_GetBitStreamValidSizeReg(
    void);

// 0x10C
void
JPG_SetHuffmanCodeCtrlReg(
    JPG_HUFFMAN_TAB_SEL   tableSel,
    uint8_t               *pCodeLength);

// 0xB0E
void
JPG_SetDcHuffmanValueReg(
    JPG_HUFFMAN_TAB_SEL     tableSel,
    uint8_t                 *pCodeValue,
    uint16_t                totalLength);

// 0x110
// 0x112
void
JPG_SetEncodeAcHuffmanValueReg(
    JPG_HUFFMAN_TAB_SEL     tableSel,
    uint8_t                 *pCodeValue,
    uint16_t                totalLength);

// 0x110
// 0x112
void
JPG_SetDecodeAcHuffmanValueReg(
    JPG_HUFFMAN_TAB_SEL     tableSel,
    uint8_t                 *pCodeValue,
    uint16_t                totalLength);

// jpg output RGB
void
JPG_SetYuv2RgbMatrix(
    JPG_YUV_TO_RGB      *matrix);

void
JPG_SetRgb565DitherKey(
    JPG_DITHER_KEY     *ditherKeyInfo);

void 
JPG_SetTilingMode(
    void);

#ifdef __cplusplus
}
#endif

#endif

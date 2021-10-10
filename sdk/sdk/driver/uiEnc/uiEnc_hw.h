#ifndef __UIENC_HW_H_2KK29U0X_BZA8_1BLL_99J9_4UAOAUDTR44E__
#define __UIENC_HW_H_2KK29U0X_BZA8_1BLL_99J9_4UAOAUDTR44E__

#ifdef __cplusplus
extern "C" {
#endif


#include "uiEnc_types.h"
//=============================================================================
//				  Constant Definition
//=============================================================================
#define UIE_TIME_OUT_CNT    500

//=============================================================================
//				  Register Definition
//=============================================================================
#define REG_UIE_BASE                              (0x1500)        /* Base Register Address */

//====================================================================
/* 
 * 0x1500                                  
 * Fire Register, Frie
 */
//====================================================================
#define UIE_REG_FIRE                        (REG_UIE_BASE + 0x0000)

#define UIE_BIT_FIRE                        0x0001 //0000 0000 0000 0001
#define UIE_SHT_FIRE                        0

//====================================================================
/* 
 * 0x1502                                  
 * Encoder Parameter Register 1
 */
//====================================================================
#define UIE_REG_SET_ENC_PARAM_1             (REG_UIE_BASE + 0x0002)

#define UIE_MSK_RUN_SIZE                    0x0007 //0000 0111 0000 0000
#define UIE_SHT_RUN_SIZE                    8

#define UIE_MSK_UNIT_SIZE                   0x0001 //0000 0000 0000 0010
#define UIE_SHT_UNIT_SIZE                   1

#define UIE_MSK_SRC_BPP                     0x0001 //0000 0000 0000 0001
#define UIE_SHT_SRC_BPP                     0

//====================================================================
/* 
 * 0x1504                                  
 * Encoder Parameter Register 2
 */
//====================================================================
#define UIE_REG_SET_ENC_PARAM_2             (REG_UIE_BASE + 0x0004)

#define UIE_MSK_ENC_DELAY                   0x000F //0000 0000 0000 1111
#define UIE_SHT_ENC_DELAY                   0

//====================================================================
/* 
 * 0x1514 ~ 0x151C                           
 * source register
 */
//====================================================================
#define UIE_REG_SRC_ADDR_L                 (REG_UIE_BASE + 0x0014)
#define UIE_REG_SRC_ADDR_H                 (REG_UIE_BASE + 0x0016)
#define UIE_REG_SRC_WIDTH                  (REG_UIE_BASE + 0x0018)
#define UIE_REG_SRC_HEIGHT                 (REG_UIE_BASE + 0x001A)
#define UIE_REG_SRC_PITCH                  (REG_UIE_BASE + 0x001C)

#define UIE_MSK_SRC_ADDR_L                 0xFFF0 //1111 1111 1111 0000
#define UIE_MSK_SRC_ADDR_H                 0xFFFF //1111 1111 1111 1111
#define UIE_MSK_SRC_WIDTH                  0xFFFF //1111 1111 1111 1111
#define UIE_MSK_SRC_HEIGHT                 0xFFFF //1111 1111 1111 1111
#define UIE_MSK_SRC_PITCH                  0xFFFF //1111 1111 1111 1111

//====================================================================
/* 
 * 0x1520 ~ 0x1526                         
 * destination register
 */
//====================================================================
#define UIE_REG_DST_ADDR_L                 (REG_UIE_BASE + 0x0020)
#define UIE_REG_DST_ADDR_H                 (REG_UIE_BASE + 0x0022)
#define UIE_REG_DST_LINE_BYTE              (REG_UIE_BASE + 0x0024)
#define UIE_REG_DST_PITCH                  (REG_UIE_BASE + 0x0026)

#define UIE_MSK_DST_ADDR_L                 0xFFF0 //1111 1111 1111 0000
#define UIE_MSK_DST_ADDR_H                 0xFFFF //1111 1111 1111 1111
#define UIE_MSK_DST_LINE_BYTE              0xFFFF //1111 1111 1111 1111
#define UIE_MSK_DST_PITCH                  0xFFFF //1111 1111 1111 1111

//====================================================================
/* 
 * 0x1540                         
 * Engine status register
 */
//====================================================================
#define UIE_REG_ENG_STATUS                     (REG_UIE_BASE + 0x0040)

#define UIE_MSK_ENG_STATUS                     0x0002 //0000 0000 0000 0010

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
    void);


void 
UIE_PowerDown(
    void);


void
UIE_Fire_Reg(
    void);
    

void 
UIE_SetEncParam_1(
    UIE_INT     runSize,
    UIE_INT     unitSize,
    UIE_INT     srcBpp);
    

void 
UIE_SetEncParam_2(
    UIE_INT     engDelay);
    

void
UIE_SetSrcInfo_Reg(
    UIE_SRC_INFO    *srcInfo);
    

UIE_UINT16
UIE_GetEngStatus(
    void);

    
void
UIE_SetDstInfo_Reg(
    UIE_DST_INFO    *dstInfo);
    

UIE_ERR
UIE_WaitIdle(
    void);


void
UIE_RegLog(
    void);

    

#ifdef __cplusplus
}
#endif

#endif


#ifndef __ISP_HW_OP_H__
#define __ISP_HW_OP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "isp_types.h"

//=============================================================================
//                        Macro Definition
//=============================================================================

//=============================================================================
//                        Structure Definition
//=============================================================================
typedef struct ISP_HW_OPERATION {
    void        (*ReadReg)(MMP_UINT16, MMP_UINT16 *);
    void        (*WriteReg)(MMP_UINT16, MMP_UINT16);
    void        (*WriteRegMask)(MMP_UINT16 addr, MMP_UINT16 data, MMP_UINT16 mask);
    void        (*WaitQueue)(MMP_UINT32 sizeInByte);
    void        (*FireQueue)(void);
    void        (*FireIspQueue)(void);
    ISP_RESULT  (*WaitIspIdle)(ISP_CONTEXT *ISPctxt);
} ISP_HW_OPERATION;

//=============================================================================
//				          Global Data Definition
//=============================================================================
extern ISP_HW_OPERATION gtIspHwOpIspQ;
extern ISP_HW_OPERATION gtIspHwOpCmdQ;
extern ISP_HW_OPERATION gtIspHwOpMMIO;
extern ISP_HW_OPERATION *gptIspHwOp;

//=============================================================================
//				          Public Function Definition
//=============================================================================
extern void
ISP_CmdSelect(
    ISP_HW_OPERATION *ptIspHwOp);

extern void
ISP_ReadRegister(
    MMP_UINT16 addr,
    MMP_UINT16 *data);

extern void
ISP_WriteRegister(
    MMP_UINT16 addr,
    MMP_UINT16 data);

extern void
ISP_WriteRegisterMask(
    MMP_UINT16 addr,
    MMP_UINT16 data,
    MMP_UINT16 mask);

extern void
ISP_CMD_QUEUE_WAIT(
    MMP_UINT32 sizeInByte);

extern void
ISP_CMD_QUEUE_FIRE(
    void);

extern void
ISP_FireIspQueue(
    void);

extern ISP_RESULT
ISP_WaitIspIdle(
    ISP_CONTEXT *ISPctxt);

#ifdef __cplusplus
}
#endif

#endif
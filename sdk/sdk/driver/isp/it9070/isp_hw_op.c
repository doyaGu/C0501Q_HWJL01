#include "isp_hw_op.h"
#include "isp_hw.h"
#include "isp_queue_type.h"
#include "isp_queue.h"

//=============================================================================
//                        Private Function Declaration
//=============================================================================
static void _dummy_WriteRegisterMask(MMP_UINT16 addr, MMP_UINT16 data, MMP_UINT16 mask);
static void _dummy_WaitQueue(MMP_UINT32 sizeInByte);
static void _dummy_FireQueue(void);
static void _dummy_FireIspQueue(void);
static ISP_RESULT _dummy_WaitIspIdle(ISP_CONTEXT *ISPctxt);

static void _IspQ_ReadRegister(MMP_UINT16 addr, MMP_UINT16 *data);
static void _IspQ_WriteRegister(MMP_UINT16 addr, MMP_UINT16 data);
static void _IspQ_WaitQueue(MMP_UINT32 sizeInByte);
static void _IspQ_FireQueue(void);
static void _IspQ_FireIspQueue(void);

static void _CmdQ_ReadRegister(MMP_UINT16 addr, MMP_UINT16 *data);
static void _CmdQ_WriteRegister(MMP_UINT16 addr, MMP_UINT16 data);
static void _CmdQ_WaitQueue(MMP_UINT32 sizeInByte);
static void _CmdQ_FireQueue(void);

static void _MMIO_ReadRegister(MMP_UINT16 addr, MMP_UINT16 *data);
static void _MMIO_WriteRegister(MMP_UINT16 addr, MMP_UINT16 data);
static void _MMIO_WriteRegisterMask(MMP_UINT16 addr, MMP_UINT16 data, MMP_UINT16 mask);
static ISP_RESULT _MMIO_WaitIspIdle(ISP_CONTEXT *ISPctxt);

//=============================================================================
//                        Global Data Definition
//=============================================================================
ISP_HW_OPERATION gtIspHwOpIspQ =
{
    _IspQ_ReadRegister,
    _IspQ_WriteRegister,
    _dummy_WriteRegisterMask,
    _IspQ_WaitQueue,
    _IspQ_FireQueue,
    _IspQ_FireIspQueue,
    _dummy_WaitIspIdle
};

ISP_HW_OPERATION gtIspHwOpCmdQ =
{
    _CmdQ_ReadRegister,
    _CmdQ_WriteRegister,
    _dummy_WriteRegisterMask,
    _CmdQ_WaitQueue,
    _CmdQ_FireQueue,
    _dummy_FireIspQueue,
    _dummy_WaitIspIdle
};

ISP_HW_OPERATION gtIspHwOpMMIO =
{
    _MMIO_ReadRegister,
    _MMIO_WriteRegister,
    _MMIO_WriteRegisterMask,
    _dummy_WaitQueue,
    _dummy_FireQueue,
    _dummy_FireIspQueue,
    _MMIO_WaitIspIdle
};

ISP_HW_OPERATION *gptIspHwOp = &gtIspHwOpMMIO;

//=============================================================================
//                        Public Function Definition
//=============================================================================
MMP_INLINE void
ISP_CmdSelect(
    ISP_HW_OPERATION *ptIspHwOp)
{
    gptIspHwOp = ptIspHwOp;
}

MMP_INLINE void
ISP_ReadRegister(
    MMP_UINT16 addr,
    MMP_UINT16 *data)
{
    gptIspHwOp->ReadReg(addr, data);
}

MMP_INLINE void
ISP_WriteRegister(
    MMP_UINT16 addr,
    MMP_UINT16 data)
{
    gptIspHwOp->WriteReg(addr, data);
}

MMP_INLINE void
ISP_WriteRegisterMask(
    MMP_UINT16 addr,
    MMP_UINT16 data,
    MMP_UINT16 mask)
{
    gptIspHwOp->WriteRegMask(addr, data, mask);
}

MMP_INLINE void
ISP_CMD_QUEUE_WAIT(
    MMP_UINT32 sizeInByte)
{
    gptIspHwOp->WaitQueue(sizeInByte);
}

MMP_INLINE void
ISP_CMD_QUEUE_FIRE(
    void)
{
    gptIspHwOp->FireQueue();
}

MMP_INLINE void
ISP_FireIspQueue(
    void)
{
    gptIspHwOp->FireIspQueue();
}

MMP_INLINE ISP_RESULT
ISP_WaitIspIdle(
    ISP_CONTEXT *ISPctxt)
{
    return gptIspHwOp->WaitIspIdle(ISPctxt);
}

//=============================================================================
//                        Private Function Definition
//=============================================================================
static void
_dummy_WriteRegisterMask(
    MMP_UINT16 addr,
    MMP_UINT16 data,
    MMP_UINT16 mask)
{
    isp_msg(ISP_MSG_TYPE_ERR, "No Use ISP_WriteRegisterMask!\n");
}

static void _dummy_WaitQueue(MMP_UINT32 sizeInByte) {}
static void _dummy_FireQueue(void) {}
static void _dummy_FireIspQueue(void) {}
static ISP_RESULT _dummy_WaitIspIdle(ISP_CONTEXT *ISPctxt) { return ISP_SUCCESS; }

static void
_IspQ_ReadRegister(
    MMP_UINT16 addr,
    MMP_UINT16 *data)
{
    IspQ_WaitIdle(0x0060, 0x0060);  // D[5][6]: S/W CQ Empty
    isp_ReadHwReg(addr, data);
}

static void
_IspQ_WriteRegister(
    MMP_UINT16 addr,
    MMP_UINT16 data)
{
    IspQ_PackCommand(addr, data);
}

static void
_IspQ_WaitQueue(
    MMP_UINT32 sizeInByte)
{
    IspQ_Lock();    // for lock
    IspQ_WaitSize(sizeInByte);
}

static void
_IspQ_FireQueue(
    void)
{
    IspQ_Fire();
    IspQ_Unlock();  // for unlock
}

static void
_IspQ_FireIspQueue(
    void)
{
    isp_WriteHwReg(ISPQ_REG_FIRE_CMD, (MMP_UINT16)0x1);
}

static void
_CmdQ_ReadRegister(
    MMP_UINT16 addr,
    MMP_UINT16 *data)
{
#if defined(ISSUE_CODE)
    WaitEngineIdle(0x0003, 0x0003);  // D[0][1]: S/W CQ Empty
    isp_ReadHwReg(addr, data);
#endif
}

static void
_CmdQ_WriteRegister(
    MMP_UINT16 addr,
    MMP_UINT16 data)
{
#if defined(ISSUE_CODE)
    CmdQ_PackCommand(addr, data);
#endif
}

static void
_CmdQ_WaitQueue(
    MMP_UINT32 sizeInByte)
{
#if defined(ISSUE_CODE)
    CmdQ_Lock();      // for lock
    CmdQ_WaitSize(sizeInByte);
#endif
}

static void
_CmdQ_FireQueue(
    void)
{
#if defined(ISSUE_CODE)
    CmdQ_Fire();
    CmdQ_Unlock();  // for unlock
#endif
}

static void
_MMIO_ReadRegister(
    MMP_UINT16 addr,
    MMP_UINT16 *data)
{
    isp_ReadHwReg(addr, data);
}

static void
_MMIO_WriteRegister(
    MMP_UINT16 addr,
    MMP_UINT16 data)
{
    isp_WriteHwReg(addr, data);
}

static void
_MMIO_WriteRegisterMask(
    MMP_UINT16 addr,
    MMP_UINT16 data,
    MMP_UINT16 mask)
{
    isp_WriteHwRegMask(addr, data, mask);
}

static ISP_RESULT _MMIO_WaitIspIdle(ISP_CONTEXT *ISPctxt)
{
    ISP_RESULT result = ISP_SUCCESS;
    if (ISPctxt->OutInfo.EnableLcdOnFly == MMP_TRUE)
    {
        result = ISP_WaitISPChangeIdle();
        if (result)
        {
            isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
            goto end;
        }
    }
    else
    {
        result = ISP_WaitEngineIdle();
        if (result)
        {
            isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
            goto end;
        }
    }
end:
    return result;
}
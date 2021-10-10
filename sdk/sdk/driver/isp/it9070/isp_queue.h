#ifndef __ISP_QUEUE_H_0M7DTDRW_Z2JS_U8BZ_EOKP_NTMJ9XHOWJPY__
#define __ISP_QUEUE_H_0M7DTDRW_Z2JS_U8BZ_EOKP_NTMJ9XHOWJPY__

#ifdef __cplusplus
extern "C" {
#endif

#include "isp_defs.h"
#include "isp_queue_type.h"

//=============================================================================
//                Constant Definition
//=============================================================================
//#define ISPQ_DEBUG_MSG
//#define ISPQ_USE_HW_GET_REMAINDER

typedef enum _ISPQ_MODULE_STATUS
{
    ISPQ_MODULE_INTR       = 0x1000,
    ISPQ_MODULE_AHB_EMPTY  = 0x0800,
    ISPQ_MODULE_FLIP_CMD   = 0x0400,
    ISPQ_MODULE_OVG_CMD    = 0x0200,
    ISPQ_MODULE_ISP_CMD    = 0x0100,
    ISPQ_MODULE_FLIP_EMPTY = 0x0080,
    ISPQ_MODULE_ISP_BUSY   = 0x0040,
    ISPQ_MODULE_DPU_BUSY   = 0x0020,
    ISPQ_MODULE_OVG_BUSY   = 0x0010,
    ISPQ_MODULE_CMD_FAIL   = 0x0008,
    ISPQ_MODULE_ALL_IDLE   = 0x0004,
    ISPQ_MODULE_HQ_EMPTY   = 0x0002,
    ISPQ_MODULE_SQ_EMPTY   = 0x0001,
} ISPQ_MODULE_STATUS;

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct _ISP_QUEUE_TAG
{
    MMP_UINT32 bufAddress;
    MMP_UINT32 bufAlignAddr;
    MMP_UINT32 bufLength;

    MMP_UINT32 currPtr;
    MMP_UINT32 readPtr;
    MMP_UINT32 writePtr;

    MMP_INT32  refCount;

#if defined(WIN32)
    #pragma data_seg(".MYSEC")
    void *semaphore;
    #pragma data_seg()

#elif defined(__FREERTOS__)
    void  *semaphore;
#elif defined(__OPENRTOS__)
    sem_t *semaphore;
#endif
} ISP_QUEUE;

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
ISP_API MMP_BOOL
IspQ_Initialize(
    void);

ISP_API void
IspQ_Terminate(
    void);

ISP_API MMP_BOOL
IspQ_Lock(
    void);

ISP_API MMP_BOOL
IspQ_Unlock(
    void);

/**************************************************************************
 * @brief IspQ_WaitSize()
 *
 * Wait enough size for data packing.
 *
 * @param size The number of date to be writed, count in byte.
 * @retval MMP_TRUE If wait enough size success.
 * @retval MMP_FALSE If wait enough size fail.
 *************************************************************************/
ISP_API MMP_BOOL
IspQ_WaitSize(
    MMP_UINT32 size);

ISP_API MMP_BOOL
IspQ_PackCommand(
    MMP_UINT32 addr,
    MMP_UINT32 data);

ISP_API MMP_BOOL
IspQ_BurstCommand(
    MMP_UINT32 addr,
    MMP_UINT32 length,
    void       *command);

ISP_API void
IspQ_Fire(
    void);

ISP_API void
IspQ_WaitIdle(
    MMP_UINT16 val,
    MMP_UINT16 msk);

//=============================================================================
//                Macro Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
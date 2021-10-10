/*
 * Copyright (c) 2013 ITE Technology Corp. All Rights Reserved.
 */
/** @file xcpu_msgq.c
 *
 * @version 0.1
 */

#include "xcpu_msgq.h"
#include "itx.h"
//#include "pal.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

#define MAX_XCPU_MSG_COUNT      8

// XCPU_SLAVE_MSG_ALIGNMENT this definition can't be modified.
#define XCPU_SLAVE_MSG_ALIGNMENT 268

/* MMIO Index */
#define REG_MASTER_WRITE        0x1700
#define REG_MASTER_READ         0x1698

#if (ITX_BUS_TYPE == ITX_BUS_I2C)
#define MEM_SHARED_INFO         0xC0200000
#else
#define MEM_SHARED_INFO         0x1000
#endif

#define MEM_VERSION             (MEM_SHARED_INFO + 0x0)
#define MEM_SUBVERSION          (MEM_SHARED_INFO + 0x18)
#define MEM_SLAVE_WRITE         (MEM_SHARED_INFO + 0x16)
#define MEM_SLAVE_READ          (MEM_SHARED_INFO + 0x14)
#define MEM_MASTER_WRITE        (MEM_SHARED_INFO + 0x1E)
#define MEM_MASTER_READ         (MEM_SHARED_INFO + 0x1C)

//=============================================================================
//                              Macro Definition
//=============================================================================

#define _COUNT_OF_MSG( w, r, max)   ((r <= w) ? (w - r) : (max - (r - w)))
#define _IS_MSGQ_FULL( w, r, max)   (_COUNT_OF_MSG(w, r, max) == max - 1)
#define _IS_MSGQ_EMPTY(w, r, max)   (_COUNT_OF_MSG(w, r, max) == 0)

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct _SHAREINFO {
    MMP_UINT32 version;
    MMP_UINT32 mToSMsgQ_Address;
    MMP_UINT32 mToSMsgQ_Capacity;
    MMP_UINT32 sToMMsgQ_Address;
    MMP_UINT32 sToMMsgQ_Capacity;
    MMP_UINT16 slaveReadIndex;
    MMP_UINT16 slaveWriteIndex;
    MMP_UINT32 subversion;
    MMP_UINT16 masterReadIndex;
    MMP_UINT16 masterWriteIndex;
    MMP_UINT16 standaloneBootFlag;
    MMP_UINT32 cmd_bufaddr;
} SHAREINFO;

/* Message queue structure definition */
typedef struct XCPU_MSGQ_TAG
{
    XCPU_MSG_OBJ tMsg[MAX_XCPU_MSG_COUNT];
} XCPU_MSGQ;

/* Queue manager structure definition */
typedef struct XCPU_MSGQ_MGR_TAG
{
    MMP_UINT32  ptMToSMsgQ;
    MMP_UINT32  ptSToMMsgQ;
} XCPU_MSGQ_MGR;

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

static XCPU_MSGQ_MGR gtMsgQMgr;
static SHAREINFO     shareinfo;

//=============================================================================
//                              Protected Function Declaration
//=============================================================================

//=============================================================================
/**
 * Periodically call this method to receive messages from the slave device and to
 * dispatch the message into the corresponding system message queues according
 * to the types of messages.
 *
 * @return MMP_TRUE -  Received Msg
 *         MMP_FALSE - No any waiting Msg
 */
//=============================================================================
MMP_BOOL
xCpuMsgQ_RouteMessage(
    void);

//=============================================================================
//                              Private Function Declaration
//=============================================================================

static MMP_INLINE MMP_UINT16
_XCPUMSGQ_GetSWIndex(
    void)
{
    xCpuIO_ReadMemoryUInt16(
        (MMP_UINT32)&shareinfo.slaveWriteIndex,
        (MMP_UINT32)MEM_SLAVE_WRITE,
        2);
    return shareinfo.slaveWriteIndex;
}

static MMP_INLINE void
_XCPUMSGQ_SetSWIndex(
    MMP_UINT16 index)
{
    shareinfo.slaveWriteIndex = index;
    xCpuIO_WriteMemoryUInt16(
        (MMP_UINT32)MEM_SLAVE_WRITE,
        (MMP_UINT32)&shareinfo.slaveWriteIndex,
        2);
}

static MMP_INLINE MMP_UINT16
_XCPUMSGQ_GetSRIndex(
    void)
{
    xCpuIO_ReadMemoryUInt16(
        (MMP_UINT32)&shareinfo.slaveReadIndex,
        (MMP_UINT32)MEM_SLAVE_READ,
        2);
    return shareinfo.slaveReadIndex;
}

static MMP_INLINE void
_XCPUMSGQ_SetSRIndex(
    MMP_UINT16 index)
{
    shareinfo.slaveReadIndex = index;
    xCpuIO_WriteMemoryUInt16(
        (MMP_UINT32)MEM_SLAVE_READ,
        (MMP_UINT32)&shareinfo.slaveReadIndex,
        2);
}

static MMP_INLINE MMP_UINT16
_XCPUMSGQ_GetMWIndex(
    void)
{
    //xCpuIO_ReadMemoryUInt16(
    //    (MMP_UINT32)&shareinfo.masterWriteIndex,
    //    (MMP_UINT32)MEM_MASTER_WRITE,
    //    2);
    //return shareinfo.masterWriteIndex;
    return xCpuIO_ReadRegister(REG_MASTER_WRITE);
}

static MMP_INLINE void
_XCPUMSGQ_SetMWIndex(
    MMP_UINT16 index)
{
    shareinfo.masterWriteIndex = index;
    //xCpuIO_WriteMemoryUInt16(
    //    (MMP_UINT32)MEM_MASTER_WRITE,
    //    (MMP_UINT32)&shareinfo.masterWriteIndex,
    //    2);
    xCpuIO_WriteRegister(REG_MASTER_WRITE, index);
}

static MMP_INLINE MMP_UINT16
_XCPUMSGQ_GetMRIndex(
    void)
{
    //xCpuIO_ReadMemoryUInt16(
    //    (MMP_UINT32)&shareinfo.masterReadIndex,
    //    (MMP_UINT32)MEM_MASTER_READ,
    //    2);
    //return shareinfo.masterReadIndex;
    return xCpuIO_ReadRegister(REG_MASTER_READ);
}

static MMP_INLINE void
_XCPUMSGQ_SetMRIndex(
    MMP_UINT16 index)
{
    shareinfo.masterReadIndex = index;
    //xCpuIO_WriteMemoryUInt16(
    //    (MMP_UINT32)MEM_MASTER_READ,
    //    (MMP_UINT32)&shareinfo.masterReadIndex,
    //    2);
    xCpuIO_WriteRegister(REG_MASTER_READ, index);
}

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * This method implement a handshake mechanism with slave device to make sure slave device
 * has successfully booted up and is ready to receive or send cross CPU message.
 * This method will be blocked until slave device announces it is ready. Only call
 * this function once slave device boots.
 *
 * @return  0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
xCpuMsgQ_Init(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;
    MMP_INT timeout = 50;

    while ((0xFFFF != xCpuIO_ReadRegister(REG_MASTER_READ)) && timeout-- > 0)
    {
        PalSleep(100 * 1000);        
    }

    if (timeout > 0)
    {   
        // Get the shared information between two CPUs.
        xCpuIO_ReadMemoryUInt32(
            (MMP_UINT32)&shareinfo,
            (MMP_UINT32)MEM_SHARED_INFO,
            (MMP_UINT32)sizeof(shareinfo));

        printf("version:0x%x mToSMsgQ_Address:0x%x mToSMsgQ_Capacity:0x%x sToMMsgQ_Address:0x%x\n",shareinfo.version,shareinfo.mToSMsgQ_Address,shareinfo.mToSMsgQ_Capacity,shareinfo.sToMMsgQ_Address);
        printf("cmd buf addr: 0x%x\n", shareinfo.cmd_bufaddr);
#if defined(CFG_SYSTEM_TRANWO_IPTV)        
        //if (shareinfo.version != 0x71204)
        //{
        //    result = MMP_RESULT_ERROR;
        //    return result;
        //}
#endif        
        gtMsgQMgr.ptMToSMsgQ = (MMP_UINT32)shareinfo.mToSMsgQ_Address;
        gtMsgQMgr.ptSToMMsgQ = (MMP_UINT32)shareinfo.sToMMsgQ_Address;

        _XCPUMSGQ_SetSWIndex(0x0000);
        _XCPUMSGQ_SetSRIndex(0x0000);
        _XCPUMSGQ_SetMWIndex(0x0000);
        _XCPUMSGQ_SetMRIndex(0x0000);
    }
    else
    {
        result = MMP_RESULT_ERROR;//MMP_RESULT_TIMEOUT
        printf("xCpuMsgQ_Init Time out (%d)\n", timeout);
    }
    return result;
}

//=============================================================================
/**
 * Send a message to the slave device.
 *
 * @param ptMtoSMsg the pointer to an XCPU_MSG_OBJ structure that contains the
 *                  message information to be sent
 * @return  0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
XCPU_MSGQ_ERROR_CODE
xCpuMsgQ_SendMessage(
    XCPU_MSG_OBJ* ptMtoSMsg)
{
    XCPU_MSGQ_ERROR_CODE result = QUEUE_NO_ERROR;
    MMP_UINT16 writeIndex;
    MMP_UINT16 readIndex;

    writeIndex = _XCPUMSGQ_GetMWIndex();
    readIndex  = _XCPUMSGQ_GetSRIndex();

    if (!_IS_MSGQ_FULL(writeIndex, readIndex, MAX_XCPU_MSG_COUNT))
    {
        xCpuIO_WriteMemoryUInt16(
            //(MMP_UINT32)&gtMsgQMgr.ptMToSMsgQ->tMsg[writeIndex],
            gtMsgQMgr.ptMToSMsgQ + XCPU_SLAVE_MSG_ALIGNMENT * writeIndex,
            (MMP_UINT32)ptMtoSMsg,
            sizeof(XCPU_MSG_OBJ));

        if (MAX_XCPU_MSG_COUNT <= ++writeIndex)
            writeIndex = 0;
        _XCPUMSGQ_SetMWIndex(writeIndex);
        
        //printf("xCpuMsgQ_SendMessage ri=%d, wi=%d, m-wi=%d\n", 
        //    readIndex, writeIndex, shareinfo.masterWriteIndex);
    }
    else
    {
        result = QUEUE_IS_FULL;
    }

    return result;
}

//=============================================================================
/**
 * Receive a message from slave device.
 *
 * @param ptStoMMsg Pointer to an XCPU_MSG_OBJ structure that contains the
 *                  message information received from slave device.
 * @return  0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
XCPU_MSGQ_ERROR_CODE
xCpuMsgQ_ReceiveMessage(
    XCPU_MSG_OBJ* ptStoMMsg)
{
    XCPU_MSGQ_ERROR_CODE result = QUEUE_NO_ERROR;
    MMP_UINT16 writeIndex;
    MMP_UINT16 readIndex;

    writeIndex = _XCPUMSGQ_GetSWIndex();
    readIndex  = _XCPUMSGQ_GetMRIndex();

    if (!_IS_MSGQ_EMPTY(writeIndex, readIndex, MAX_XCPU_MSG_COUNT))
    {
        //printf("xCpuMsgQ_ReceiveMessage ri=%d, wi=%d, m-wi=%d\n", 
        //    readIndex, writeIndex, shareinfo.slaveWriteIndex);
            
        xCpuIO_ReadMemoryUInt16(
            (MMP_UINT32)ptStoMMsg,
            gtMsgQMgr.ptSToMMsgQ + XCPU_SLAVE_MSG_ALIGNMENT * readIndex,
            //(MMP_UINT32)&gtMsgQMgr.ptSToMMsgQ->tMsg[readIndex],
            sizeof(XCPU_MSG_OBJ));

        if (MAX_XCPU_MSG_COUNT <= ++readIndex)
            readIndex = 0;
        _XCPUMSGQ_SetMRIndex(readIndex);
    }
    else
    {
        result = QUEUE_IS_EMPTY;
    }

    return result;
}

//=============================================================================
/**
 * Periodically call this method to receive messages from the Caster and to
 * dispatch the message into the corresponding system message queues according
 * to the types of messages.
 *
 * @return MMP_TRUE -  Received Msg
 *         MMP_FALSE - No any waiting Msg
 */
//=============================================================================
MMP_BOOL
xCpuMsgQ_RouteMessage(
    void)
{
    // Polling the XCPU message queue to see if any message is
    // received from the slave CPU.
    XCPU_MSG_OBJ tStoMMsg = {0};

    if (QUEUE_NO_ERROR == xCpuMsgQ_ReceiveMessage(&tStoMMsg))
    {
        switch (tStoMMsg.type)
        {
        case SYS_MSG_TYPE_FILE:
            sysMsgQ_SendMessage(SYS_MSGQ_ID_FILE, &tStoMMsg);
            break;
        case SYS_MSG_TYPE_CMD:
            sysMsgQ_SendMessage(SYS_MSGQ_ID_CMD, &tStoMMsg);
            break;
        }
        return MMP_TRUE;
    }
    else
    {
        return MMP_FALSE;
    }
}

//=============================================================================
/**
 * Release the related resources.
 *
 * @return  none
 */
//=============================================================================
void
xCpuMsgQ_Terminate(
    void)
{

}

MMP_UINT32
xCpuMsgQ_GetVersion(
    void)
{
    xCpuIO_ReadMemoryUInt32(
        (MMP_UINT32)&shareinfo.version,
        (MMP_UINT32)MEM_VERSION,
        4);
    return shareinfo.version;
}

MMP_UINT32
xCpuMsgQ_GetSubversion(
    void)
{
    xCpuIO_ReadMemoryUInt32(
        (MMP_UINT32)&shareinfo.subversion,
        (MMP_UINT32)MEM_SUBVERSION,
        4);
    return shareinfo.subversion;
}

MMP_UINT32
xCpuMsgQ_GetCmdBufAddr(
    void)
{
    printf("xCpuMsgQ_GetCmdBufAddr:0x%x\n",shareinfo.cmd_bufaddr);
    return shareinfo.cmd_bufaddr;
}

//=============================================================================
//                              Private Function Definition
//=============================================================================


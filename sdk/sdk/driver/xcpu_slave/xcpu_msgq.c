/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file xcpu_msgq.c
 *
 * @author I-Chun Lai
 * @version 0.1
 */

#include "xcpu_msgq.h"
//#include "mmp_mpg2.h"
//#include "dtv_dma.h"

//#if (!defined(WIN32)) && defined(ENABLE_XCPU_MSGQ)

//=============================================================================
//                              Constant Definition
//=============================================================================

#define MAX_XCPU_MSG_COUNT      64

/* MMIO Index */
#define MASTER_WRITE            0x169A//0x1700
#define MASTER_READ             0x1698

#define SUBVERSION              1

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
} SHAREINFO;

/* Message queue structure definition */
typedef struct XCPU_MSGQ_TAG
{
    XCPU_MSG_OBJ tMsg[MAX_XCPU_MSG_COUNT];
} XCPU_MSGQ;

/* Queue manager structure definition */
typedef struct XCPU_MSGQ_MGR_TAG
{
    XCPU_MSGQ*  ptMToSMsgQ;
    XCPU_MSGQ*  ptSToMMsgQ;
} XCPU_MSGQ_MGR;

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
static XCPU_MSGQ gtMToSMsgQ;
static XCPU_MSGQ gtSToMMsgQ;
//static void*     gpSemaphore;

static XCPU_MSGQ_MGR gtMsgQMgr = {
    .ptMToSMsgQ = &gtMToSMsgQ,
    .ptSToMMsgQ = &gtSToMMsgQ
};

#if 1
SHAREINFO* shareinfo = NULL;
#else
SHAREINFO shareinfo __attribute__ ((section (".common_var"))) = {
    .version            = 0x9079,
    .mToSMsgQ_Address   = (MMP_UINT32)&gtMToSMsgQ,
    .mToSMsgQ_Capacity  = MAX_XCPU_MSG_COUNT,
    .sToMMsgQ_Address   = (MMP_UINT32)&gtSToMMsgQ,
    .sToMMsgQ_Capacity  = MAX_XCPU_MSG_COUNT,
    .slaveReadIndex     = 0,
    .slaveWriteIndex    = 0,
    .subversion         = 0,
    .masterReadIndex    = 0,
    .masterWriteIndex   = 0,
    .standaloneBootFlag = 0,
};
#endif

#if 0
typedef struct
{
    int version;
	...
} a;


extern int _xcpu_msgq;


int main()
{
	a* ptr = (a*) &_xcpu_msgq;
}
#endif

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//static void*
//_XCPUMSGQ_GetSemaphore(
//    void);

static MMP_INLINE MMP_UINT16
_XCPUMSGQ_GetSWIndex(
    void)
{
    return shareinfo->slaveWriteIndex;
}

static MMP_INLINE void
_XCPUMSGQ_SetSWIndex(
    MMP_UINT16 index)
{
    shareinfo->slaveWriteIndex = index;
}

static MMP_INLINE MMP_UINT16
_XCPUMSGQ_GetSRIndex(
    void)
{
    return shareinfo->slaveReadIndex;
}

static MMP_INLINE void
_XCPUMSGQ_SetSRIndex(
    MMP_UINT16 index)
{
    shareinfo->slaveReadIndex = index;
}

static MMP_INLINE MMP_UINT16
_XCPUMSGQ_GetMWIndex(
    void)
{
    MMP_UINT16 value;

    //if (MMP_NULL == gpSemaphore)
    //    gpSemaphore = _XCPUMSGQ_GetSemaphore();

    //if (gpSemaphore)
    //{
    //    SYS_WaitSemaphore(gpSemaphore);
    //    HOST_ReadRegister(MASTER_WRITE, &value);
    //    SYS_ReleaseSemaphore(gpSemaphore);
    //}
    //else
    {
        HOST_ReadRegister(MASTER_WRITE, &value);
    }

    return value;
}

static MMP_INLINE void
_XCPUMSGQ_SetMWIndex(
    MMP_UINT16 index)
{
    HOST_WriteRegister(MASTER_WRITE, index);
}

static MMP_INLINE MMP_UINT16
_XCPUMSGQ_GetMRIndex(
    void)
{
    MMP_UINT16 value;

    //if (MMP_NULL == gpSemaphore)
    //    gpSemaphore = _XCPUMSGQ_GetSemaphore();

    //if (gpSemaphore)
    //{
    //    SYS_WaitSemaphore(gpSemaphore);
    //    HOST_ReadRegister(MASTER_READ, &value);
    //    SYS_ReleaseSemaphore(gpSemaphore);
    //}
    //else
    {
        HOST_ReadRegister(MASTER_READ, &value);
    }

    return value;
}

static MMP_INLINE void
_XCPUMSGQ_SetMRIndex(
    MMP_UINT16 index)
{
    HOST_WriteRegister(MASTER_READ, index);
}

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * This method implements a handshake mechanism with the host CPU to inform the
 * host CPU that Castor has successfully booted up and is ready to receive or
 * send cross CPU message.
 *
 * @return  none
 */
//=============================================================================
void
xCpuMsgQ_Init(
    void)
{
	extern int _xcpu_msgq;

	shareinfo = (SHAREINFO*)&_xcpu_msgq;
	shareinfo->version            = 0x9079;
    shareinfo->mToSMsgQ_Address   = (MMP_UINT32)&gtMToSMsgQ;
    shareinfo->mToSMsgQ_Capacity  = MAX_XCPU_MSG_COUNT;
    shareinfo->sToMMsgQ_Address   = (MMP_UINT32)&gtSToMMsgQ;
    shareinfo->sToMMsgQ_Capacity  = MAX_XCPU_MSG_COUNT;
    shareinfo->slaveReadIndex     = 0;
    shareinfo->slaveWriteIndex    = 0;
    shareinfo->subversion         = 0;
    shareinfo->masterReadIndex    = 0;
    shareinfo->masterWriteIndex   = 0;
    shareinfo->standaloneBootFlag = 0;
	
    // Do handshake with master CPU
    _XCPUMSGQ_SetMRIndex(0xFFFF);

    while (0x0000 != _XCPUMSGQ_GetMRIndex())
    {
        PalSleep(1);
    }
}

//=============================================================================
/**
 * Send a message to the host CPU.
 *
 * @param ptStoMMsg the pointer to an XCPU_MSG_OBJ structure that contains the
 *                  message information to be sent
 * @return  0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
XCPU_MSGQ_ERROR_CODE
xCpuMsgQ_SendMessage(
    XCPU_MSG_OBJ* ptStoMMsg)
{
    XCPU_MSGQ_ERROR_CODE result = QUEUE_NO_ERROR;
    MMP_UINT16 writeIndex;
    MMP_UINT16 readIndex;

    writeIndex = _XCPUMSGQ_GetSWIndex();
    readIndex  = _XCPUMSGQ_GetMRIndex();

    if (!_IS_MSGQ_FULL(writeIndex, readIndex, MAX_XCPU_MSG_COUNT))
    {
        HOST_ReadBlockMemory(
            (MMP_UINT32)&gtMsgQMgr.ptSToMMsgQ->tMsg[writeIndex],
            (MMP_UINT32)ptStoMMsg,
            sizeof(XCPU_MSG_OBJ));

        if (MAX_XCPU_MSG_COUNT <= ++writeIndex)
            writeIndex = 0;
        _XCPUMSGQ_SetSWIndex(writeIndex);

        dbg_msg(DBG_MSG_TYPE_ERROR, "xCpuMsgQ_SendMessage ri=%d, wi=%d\n", readIndex, writeIndex);
        dbg_msg(DBG_MSG_TYPE_ERROR, "xCpuMsgQ_SendMessage type=%d, id=%d\n", ptStoMMsg->type, ptStoMMsg->id);
    }
    else
    {
        result = QUEUE_IS_FULL;
    }

    return result;
}

//=============================================================================
/**
 * Receive a message from the host CPU.
 *
 * @param ptMtoSMsg Pointer to an XCPU_MSG_OBJ structure that contains the
 *                  message information received from the host CPU.
 * @return  0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
XCPU_MSGQ_ERROR_CODE
xCpuMsgQ_ReceiveMessage(
    XCPU_MSG_OBJ* ptMtoSMsg)
{
    XCPU_MSGQ_ERROR_CODE result = QUEUE_NO_ERROR;
    MMP_UINT16 writeIndex;
    MMP_UINT16 readIndex;

    writeIndex = _XCPUMSGQ_GetMWIndex();
    readIndex  = _XCPUMSGQ_GetSRIndex();

    if (!_IS_MSGQ_EMPTY(writeIndex, readIndex, MAX_XCPU_MSG_COUNT))
    {       
        //INVALIDATE_DCACHE();
        //ithInvalidateDCache();

        //dbg_msg(DBG_MSG_TYPE_ERROR, "xCpuMsgQ_ReceiveMessage ri=%d, wi=%d, m-wi=%d\n", 
        //    readIndex, writeIndex, shareinfo->masterWriteIndex);

        HOST_ReadBlockMemory(
            (MMP_UINT32)ptMtoSMsg,
            (MMP_UINT32)&gtMsgQMgr.ptMToSMsgQ->tMsg[readIndex],
            sizeof(XCPU_MSG_OBJ));

        if (MAX_XCPU_MSG_COUNT <= ++readIndex)
            readIndex = 0;
        _XCPUMSGQ_SetSRIndex(readIndex);

        //dbg_msg(DBG_MSG_TYPE_ERROR, "xCpuMsgQ_ReceiveMessage type=%d, id=%d\n", ptMtoSMsg->type, ptMtoSMsg->id);
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
 * @return none
 */
//=============================================================================
void
xCpuMsgQ_RouteMessage(
    void)
{
    // Polling the XCPU message queue to see if any message is received
    // from the master CPU.
    XCPU_MSG_OBJ tMtoSMsg = {0};

    ithInvalidateDCache();
    while (QUEUE_NO_ERROR == xCpuMsgQ_ReceiveMessage(&tMtoSMsg))
    {
        //dbg_msg(DBG_MSG_TYPE_ERROR, "Received msg_type: %d, msg_id: %d\n", tMtoSMsg.type, tMtoSMsg.id);

        switch (tMtoSMsg.type)
        {
        case SYS_MSG_TYPE_KBD:
            sysMsgQ_SendMessage(SYS_MSGQ_ID_KBD, &tMtoSMsg);
            break;
        case SYS_MSG_TYPE_FILE:
            sysMsgQ_SendMessage(SYS_MSGQ_ID_FILE, &tMtoSMsg);
            break;
        case SYS_MSG_TYPE_CMD:
            sysMsgQ_SendMessage(SYS_MSGQ_ID_CMD, &tMtoSMsg);
            break;
        }
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

void
xCpuMsgQ_SendBootOK(
    void)
{
    shareinfo->subversion = SUBVERSION;
}

MMP_UINT16
xCpuMsgQ_GetStandaloneBootFlag(
    void)
{
    return shareinfo->standaloneBootFlag;
}

void
xCpuMsgQ_InitStandaloneFlag(
    void)
{
	shareinfo->standaloneBootFlag=0;
}


//=============================================================================
//                              Private Function Definition
//=============================================================================

//static void*
//_XCPUMSGQ_GetSemaphore(
//    void)
//{
//    MMP_RESULT  result      = MMP_SUCCESS;
//    void*       pSemaphore  = MMP_NULL;
//
//    result = mmpMpg2DecodeQuery(MPG2_SEMAPHORE, 0, (MMP_UINT32*)(&pSemaphore));
//    if (MMP_SUCCESS == result && pSemaphore)
//        return pSemaphore;
//    return MMP_NULL;
//}

//#endif

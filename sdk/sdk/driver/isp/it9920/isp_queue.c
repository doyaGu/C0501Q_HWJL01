#include "isp_types.h"
#include "isp_queue.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define ISPQ_MSG_ON        0           // show message

#define CACHE_LINESZ       32

#define ISPQ_SLEEP_TIME    1
#define ISPQ_TIMEOUT_COUNT 5000

//=============================================================================
//                Macro Definition
//=============================================================================
#define _IspQ_CurrPtrCheckBoundary()                \
    do                                              \
    {                                               \
        if (IspQueue.currPtr >= IspQueue.bufLength) \
            IspQueue.currPtr = 0;                   \
    } while (0)

#define _IspQ_IncCurrPtr()                          \
    do                                              \
    {                                               \
        IspQueue.currPtr += 4;                      \
        _IspQ_CurrPtrCheckBoundary();               \
    } while (0)

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================
static ISP_QUEUE IspQueue = {0};

//=============================================================================
//                Private Function Definition
//=============================================================================
static void
_IspQ_SetHW(
    ISP_QUEUE *cmdq)
{
    MMP_UINT32 base = (MMP_UINT32)IspQueue.bufAlignAddr - (MMP_UINT32)isp_GetVramBaseAddr();

    isp_WriteHwReg(ISPQ_REG_BASE_ADDR_LO, (MMP_UINT16) ((base >> ISPQ_SHT_BASE_ADDR_LO) & ISPQ_MSK_BASE_ADDR_LO));
    isp_WriteHwReg(ISPQ_REG_BASE_ADDR_HI, (MMP_UINT16) ((base >> ISPQ_SHT_BASE_ADDR_HI) & ISPQ_MSK_BASE_ADDR_HI));
    isp_WriteHwReg(ISPQ_REG_LENGTH,       ISPQ_LENGTH);

    isp_WriteHwReg(ISPQ_REG_WRITE_PTR_HI, 0);
    isp_WriteHwReg(ISPQ_REG_WRITE_PTR_LO, 0);
    //isp_WriteHwRegMask(ISPQ_Reg_Control, 0x1000,0x1000);
}

static MMP_UINT32
_IspQ_GetCurrPtr(
    void)
{
    return IspQueue.currPtr;
}

static MMP_UINT32
_IspQ_GetReadPtr(
    void)
{
    MMP_UINT32 readPtr  = 0;
    MMP_UINT16 readPtrL = 0;
    MMP_UINT16 readPtrH = 0;

    isp_ReadHwReg(ISPQ_REG_READ_PTR_LO, &readPtrL);
    isp_ReadHwReg(ISPQ_REG_READ_PTR_HI, &readPtrH);
    readPtr = ((MMP_UINT32)readPtrH << ISPQ_SHT_READ_PTR_HI) | (MMP_UINT32)readPtrL;

    return readPtr;
}

static MMP_UINT32
_IspQ_GetWritePtr(
    void)
{
    MMP_UINT32 writePtr  = 0;
    MMP_UINT16 writePtrL = 0;
    MMP_UINT16 writePtrH = 0;

    isp_ReadHwReg(ISPQ_REG_WRITE_PTR_LO, &writePtrL);
    isp_ReadHwReg(ISPQ_REG_WRITE_PTR_HI, &writePtrH);
    writePtr = ((MMP_UINT32)writePtrH << ISPQ_SHT_WRITE_PTR_HI) | (MMP_UINT32)writePtrL;

    return writePtr;
}

static MMP_UINT32
_IspQ_GetRemainder(
    void)
{
#ifdef ISPQ_USE_HW_GET_REMAINDER
    // MMP_UINT32  remainder = 0;
    // MMP_UINT16  remainder0   = 0;
    // MMP_UINT16  remainder1   = 0;

    // isp_ReadHwReg(_cmd_reg_remainder0, &remainder0);
    // isp_ReadHwReg(_cmd_reg_remainder1, &remainder1);
    // remainder = (MMP_UINT32)remainder0 | ((MMP_UINT32)remainder1 << _cmd_sht_remainder1);

    // return remainder;
#else
    /**
     *  case 1: readPtr  <= writePtr <= currPtr
     *  case 2: currPtr  <= readPtr  <= writePtr
     *  case 3: writePtr <= currPtr  <= readPtr
     */
    MMP_UINT32 currPtr       = IspQueue.currPtr;
    MMP_UINT32 remainderSize = 0;

    MMP_UINT32 readPtr       = _IspQ_GetReadPtr();
    MMP_UINT32 writePtr      = _IspQ_GetWritePtr();

    /* In case 1 */
    if ((readPtr <= writePtr) && (writePtr <= currPtr))
    {
        remainderSize = (IspQueue.bufLength - currPtr) + readPtr;
    }
    /* In case 2 */
    else if ((currPtr <= readPtr) && (readPtr <= writePtr))
    {
        remainderSize = readPtr - currPtr;
    }
    /* In case 3 */
    else if ((writePtr <= currPtr) && (currPtr <= readPtr))
    {
        remainderSize = readPtr - currPtr;
    }

    return remainderSize;
#endif
}

static MMP_UINT32
_IspQ_FillNullCmd(
    MMP_UINT32 numOfCmd)
{
#ifdef _WIN32
    MMP_UINT32 dummy = 0;
    MMP_UINT32 i     = 0;

    for (i = 0; i < numOfCmd; i++)
    {
        isp_WriteBlkVram(IspQueue.bufAlignAddr + IspQueue.currPtr, (MMP_UINT32)&dummy, 4);
        _IspQ_IncCurrPtr();
    }
#else
    MMP_UINT32 i = 0;

    for (i = 0; i < numOfCmd; i++)
    {
        MMP_UINT32 *pAddr = (MMP_UINT32 *)(IspQueue.bufAlignAddr + IspQueue.currPtr);

        *pAddr = 0;
        _IspQ_IncCurrPtr();
    }
#endif

    return 0;
}

//=============================================================================
//                Public Function Definition
//=============================================================================
MMP_BOOL
IspQ_Initialize(
    void)
{
    MMP_BOOL   result = MMP_FALSE;
    MMP_UINT32 alignISPQSize;

#ifndef ENABLE_ONFLY
    if (!IspQueue.semaphore)
    {
        isp_CreateSemaphore(&IspQueue.semaphore, 1, "ISP_CMDQ");
    }

    isp_WaitSemaphore(IspQueue.semaphore);
#endif

    if (!IspQueue.refCount)
    {
        // aligned to cache line
        alignISPQSize       = ITH_ALIGN_UP(ISPQ_SIZE, CACHE_LINESZ);
        IspQueue.bufAddress = (MMP_UINT32)isp_VramAllocate(alignISPQSize, 4); //MEM_USER_CMDQ); //allocate CACHE_LINESZ more to align later.
        if (IspQueue.bufAddress)
        {
            IspQueue.bufAlignAddr = (MMP_UINT32)((IspQueue.bufAddress + 0x7) & (~0x7)); // 64bit alignment
            IspQueue.bufLength    = ISPQ_SIZE;
            IspQueue.currPtr      = 0;
            IspQueue.readPtr      = 0;
            IspQueue.writePtr     = 0;
            IspQueue.refCount     = 1;
            result                = MMP_TRUE;

            /* Initial comdq HW */
            _IspQ_SetHW(&IspQueue);
        }
    }
    else
    {
        IspQueue.refCount++;
    }

#ifndef ENABLE_ONFLY
    isp_ReleaseSemaphore(IspQueue.semaphore);
#endif

    return result;
}

void
IspQ_Terminate(
    void)
{
    //ISP_ASSERT(IspQueue.refCount);

    if (--IspQueue.refCount == 0)
    {
#ifndef ENABLE_ONFLY
        if (IspQueue.semaphore)
        {
            isp_DeleteSemaphore(IspQueue.semaphore);
        }
#endif
        isp_VramFree((void *)IspQueue.bufAddress);
        isp_Memset(&IspQueue, 0x00, sizeof(ISP_QUEUE));
    }
}

MMP_BOOL
IspQ_Lock(
    void)
{
#ifndef ENABLE_ONFLY
    isp_WaitSemaphore(IspQueue.semaphore);
#endif
    isp_msg(ISPQ_MSG_ON, "[IspQ]: Lock\n");
    return MMP_TRUE;
}

MMP_BOOL
IspQ_Unlock(
    void)
{
    isp_msg(ISPQ_MSG_ON, "[IspQ]: Unlock\n");
#ifndef ENABLE_ONFLY
    isp_ReleaseSemaphore(IspQueue.semaphore);
#endif
    return MMP_TRUE;
}

MMP_BOOL
IspQ_WaitSize(
    MMP_UINT32 sizeInBytes)
{
    MMP_BOOL   result        = MMP_FALSE;
    MMP_UINT32 remainderSize = 0;

    if (sizeInBytes > ISPQ_SIZE)
    {
        return MMP_FALSE;
    }

    remainderSize = _IspQ_GetRemainder();

    while (remainderSize < sizeInBytes)
    {
        remainderSize = _IspQ_GetRemainder();
    }

    isp_msg(ISPQ_MSG_ON, "[IspQ]: Remainder Size = %u, Request Size = %u\n", remainderSize, sizeInBytes);

    return MMP_TRUE;
}

MMP_BOOL
IspQ_PackCommand(
    MMP_UINT32 addr,
    MMP_UINT32 data)
{
    // one pack 64 bits        addr    value
    //                      |---32---|---32---|
    MMP_UINT8 *currAddr = (MMP_UINT8 *)IspQueue.bufAlignAddr;

#ifdef _WIN32

    isp_WriteBlkVram(IspQueue.bufAlignAddr + IspQueue.currPtr, (MMP_UINT32)&addr, 4);
    _IspQ_IncCurrPtr();
    isp_WriteBlkVram(IspQueue.bufAlignAddr + IspQueue.currPtr, (MMP_UINT32)&data, 4);
    _IspQ_IncCurrPtr();

#elif defined(__FREERTOS__)

    {
        MMP_UINT32 *pAddr = 0;

        pAddr  = (MMP_UINT32 *)(currAddr + IspQueue.currPtr);
        *pAddr = addr;
        _IspQ_IncCurrPtr();
        pAddr  = (MMP_UINT32 *)(currAddr + IspQueue.currPtr);
        *pAddr = data;
        _IspQ_IncCurrPtr();
    }
    //(MMP_UINT32*)(currAddr + IspQueue.currPtr) = addr; _IspQ_IncCurrPtr();
    //(MMP_UINT32*)(currAddr + IspQueue.currPtr) = data; _IspQ_IncCurrPtr();
#endif
    return MMP_TRUE;
}

MMP_BOOL
IspQ_BurstCommand(
    MMP_UINT32 addr,
    MMP_UINT32 lengthInBytes,
    void       *command)
{
    MMP_UINT8  *currAddr   = (MMP_UINT8 *)IspQueue.bufAlignAddr;
    MMP_UINT32 alignLength = (lengthInBytes + 0x7) & (~0x7);   // length must align to 64bit
    MMP_UINT32 delta       = 0;
    MMP_UINT32 cmdCount    = lengthInBytes / 4;

#ifdef CMDQ_DEBUG_MSG
    /* lengthInBytes must be  alignment to 4*/
    {
        MMP_UINT32 alignTo4 = (lengthInBytes + 0x3) & (~0x3); // length must align to 32bit
        if (alignTo4 != lengthInBytes)
        {
            isp_msg(ISP_MSG_TYPE_ERR, "[IspQ][ERROR]: lengthInBytes(%u) not alignement to 4byte!\n", lengthInBytes);
        }
    }
#endif

#ifdef _WIN32

    addr |= 0x01;
    isp_WriteBlkVram(IspQueue.bufAlignAddr + IspQueue.currPtr, (MMP_UINT32)&addr, 4);
    _IspQ_IncCurrPtr();
    isp_WriteBlkVram(IspQueue.bufAlignAddr + IspQueue.currPtr, (MMP_UINT32)&cmdCount, 4);
    _IspQ_IncCurrPtr();

    if ( (IspQueue.currPtr + lengthInBytes) >= IspQueue.bufLength)
    {
        delta            = IspQueue.bufLength - IspQueue.currPtr;
        isp_WriteBlkVram((MMP_UINT32)currAddr + IspQueue.currPtr, (MMP_UINT32)command, delta);
        IspQueue.currPtr = 0;
    }

    isp_WriteBlkVram(IspQueue.bufAlignAddr + IspQueue.currPtr, (MMP_UINT32)command + delta, lengthInBytes - delta);

#elif defined(__FREERTOS__)

    {
        MMP_UINT32 *pAddr = 0;

        pAddr  = (MMP_UINT32 *)(currAddr + IspQueue.currPtr);
        *pAddr = (addr | 0x01);
        _IspQ_IncCurrPtr();
        pAddr  = (MMP_UINT32 *)(currAddr + IspQueue.currPtr);
        *pAddr = cmdCount;
        _IspQ_IncCurrPtr();
    }
    //(MMP_UINT32*)(currAddr + IspQueue.currPtr) = (addr | 0x01); _IspQ_IncCurrPtr();
    //(MMP_UINT32*)(currAddr + IspQueue.currPtr) = cmdCount;      _IspQ_IncCurrPtr();
    if ( (IspQueue.currPtr + lengthInBytes) >= IspQueue.bufLength)
    {
        delta            = IspQueue.bufLength - IspQueue.currPtr;
        memcpy(currAddr + IspQueue.currPtr, command, delta);
        IspQueue.currPtr = 0;
    }

    memcpy(currAddr + IspQueue.currPtr, command + delta, lengthInBytes - delta);

#endif

    IspQueue.currPtr += lengthInBytes - delta;
    _IspQ_CurrPtrCheckBoundary();

    if (alignLength != lengthInBytes)
    {
        _IspQ_FillNullCmd(((alignLength - lengthInBytes) >> 2));
    }
    return MMP_TRUE;
}

void
IspQ_Fire(
    void)
{
    ithInvalidateDCacheRange((void *)(&IspQueue),         sizeof(ISP_QUEUE));
    ithInvalidateDCacheRange((void *)IspQueue.bufAddress, (ISPQ_SIZE + 0x8));

    isp_WriteHwReg(ISPQ_REG_WRITE_PTR_HI, (IspQueue.currPtr & 0x00070000) >> 16);
    isp_WriteHwReg(ISPQ_REG_WRITE_PTR_LO, IspQueue.currPtr & 0xFFFF);

    isp_msg(ISPQ_MSG_ON, "[IspQ]: Update write pointer to 0x%08X\n", IspQueue.currPtr);
}

/**
 *  ISPQ_MODULE_INTR        TRUE: Not,              FALSE: Interrupt Occur
    ISPQ_MODULE_AHB_EMPTY   TRUE: Not Empty,        FALSE: Empty
    ISPQ_MODULE_FLIP_CMD    TRUE: no,               FALSE: Flip Command
    ISPQ_MODULE_OVG_CMD     TRUE: no,               FALSE: OVG Command
    ISPQ_MODULE_ISP_CMD     TRUE: no,               FALSE: ISP Command
    ISPQ_MODULE_FLIP_EMPTY  TRUE: queue not empty,  FALSE: queue empty
    ISPQ_MODULE_ISP_BUSY    TRUE: ISP idle,         FALSE: ISP busy
    ISPQ_MODULE_DPU_BUSY    TRUE: DPU idle,         FALSE: DPU busy
    ISPQ_MODULE_OVG_BUSY    TRUE: OVG idle,         FALSE: OVG busy
    ISPQ_MODULE_CMD_FAIL    TRUE: No Fails Occurs., FALSE: Fails occurs more than one time.
    ISPQ_MODULE_ALL_IDLE    TRUE: Not Empty,        FALSE: Idle and Empty
    ISPQ_MODULE_HQ_EMPTY    TRUE: Not Empty,        FALSE: Empty
    ISPQ_MODULE_SQ_EMPTY    TRUE: Not Empty,        FALSE: Empty
 */
MMP_BOOL
IspQ_GetStatus(
    ISPQ_MODULE_STATUS module)
{
    MMP_INT16 regValue = 0;

    isp_ReadHwReg(ISPQ_REG_STATUS, &regValue);

    return (regValue & module);
}

void
IspQ_WaitIdle(
    MMP_UINT16 val,
    MMP_UINT16 msk)
{
    MMP_UINT32 write   = 0;
    MMP_UINT16 idle    = 0;
    MMP_UINT32 TimeOut = 0;

    TimeOut = 0;
    isp_ReadHwReg(ISPQ_REG_STATUS, &idle);
    while ((idle & msk) != val)
    {
#ifndef ENABLE_ONFLY
        isp_sleep(ISPQ_SLEEP_TIME);
#endif
        TimeOut += 1;
        if (TimeOut > ISPQ_TIMEOUT_COUNT)
        {
            isp_msg(ISP_MSG_TYPE_ERR, "ISPQ_WaitIdle_2 timeout\n");
        }
        isp_ReadHwReg(ISPQ_REG_STATUS, &idle);
    }
}
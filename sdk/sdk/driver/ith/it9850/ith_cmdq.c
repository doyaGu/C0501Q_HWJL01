/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL command queue functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"

/* Constant definitions */
#define CMDQ_UNIT_SIZE      1024    // 1k

ITHCmdQ* ithCmdQ;
static uint32_t cmdQBase, currPtr, waitSize;

static uint32_t GetReadPointer(void)
{
    uint32_t readPtr;

    // Soft Queue Read Pointer Register 1 & 2
    readPtr  = ithReadRegH(ITH_CMDQ_RD_LO_REG) & ITH_CMDQ_RD_LO_MASK;
    readPtr |= (uint32_t)(ithReadRegH(ITH_CMDQ_RD_HI_REG) & ITH_CMDQ_RD_HI_MASK) << 16;

    return readPtr;
}

static uint32_t GetWritePointer(void)
{
    uint32_t writePtr;

    // Soft Queue Write Pointer Register 1 & 2
    writePtr  = ithReadRegH(ITH_CMDQ_WR_LO_REG) & ITH_CMDQ_WR_LO_MASK;
    writePtr |= (uint32_t)(ithReadRegH(ITH_CMDQ_WR_HI_REG) & ITH_CMDQ_WR_HI_MASK) << 16;

    return writePtr;
}

static void SetWritePointer(uint32_t ptr)
{
    ASSERT(ITH_IS_ALIGNED(ptr, sizeof (uint64_t)));
    ASSERT(ptr != 0);

    // Soft Queue Write Pointer Register 1 & 2
    ithWriteRegH(ITH_CMDQ_WR_HI_REG, (uint16_t)(ptr >> 16)); // Must write high-bits first
    ithWriteRegH(ITH_CMDQ_WR_LO_REG, (uint16_t)ptr);
}

static void FillNullCommands(void)
{
    uint32_t size, count, i, *ptr;

    size    = ithCmdQ->size - currPtr;
    if (size == 0)
        return;

    count   = size / sizeof (uint32_t);

    ptr = (uint32_t*)(cmdQBase + currPtr);

    for (i = 0; i < count; ++i)
        ptr[i] = 0;
    
#ifdef CFG_CPU_WB
    ithFlushDCacheRange(ptr,size);
    ithFlushMemBuffer();
#endif
}

/*
 * Case 1: readPtr <= writePtr <= currPtr <= ithCmdQ->size
 * Case 2: currPtr <= readPtr <= writePtr <= ithCmdQ->size
 * Case 3: writePtr <= currPtr <= readPtr <= ithCmdQ->size
 */
static int WaitAvailableSize(uint32_t size)
{
    uint32_t timeout;

    if (currPtr + size >= ithCmdQ->size)
    {
        // Cannot be case 2, else locking size > queue size
        ASSERT(GetWritePointer() <= currPtr);

        timeout = ITH_CMDQ_LOOP_TIMEOUT;
        do
        {
            uint32_t readPtr = GetReadPointer();
            
            // Wait read pointer <= current pointer (case 3 -> case 1)
            if (readPtr <= currPtr)
                break;

            LOG_DBG "CMDQ busy1: 0x%X > 0x%X\r\n", readPtr, currPtr LOG_END
            ithTaskYield();

        } while (--timeout);

        if (timeout == 0)
        {
            LOG_ERR "Wait available1 timeout, size: %d\r\n", size LOG_END

        #ifdef CFG_ITH_DBG
                ithCmdQStats();
        #endif

            return __LINE__;
        }

        // Fill null commands in the end of command queue
        FillNullCommands();

        // Reset current pointer to zero
        currPtr = 0;
    }

    // Should currPtr + size < writePtr when in case 2
    ASSERT((GetWritePointer() <= currPtr) || (currPtr + size < GetWritePointer()));

    timeout = ITH_CMDQ_LOOP_TIMEOUT;
    do
    {
        uint32_t readPtr = GetReadPointer();

        // Wait read pointer <= current pointer (case 3 -> case 1) or
        // read pointer - current pointer > required size (case 2 or case 3)
        if (readPtr <= currPtr || readPtr - currPtr >= size)
            return 0;

        LOG_DBG "CMDQ busy2: 0x%X > 0x%X\r\n", readPtr, currPtr LOG_END
        ithTaskYield();

    } while (--timeout);

    LOG_ERR "Wait available2 timeout, size: %d\r\n", size LOG_END

#ifdef CFG_ITH_DBG
    ithCmdQStats();
#endif
    
    return __LINE__;
}

int ithCmdQWaitEmpty(void)
{
    int ret = 0;
    uint32_t timeout;
    ithCmdQLock();

    timeout = ITH_CMDQ_LOOP_TIMEOUT;
    do
    {
        if (ithReadRegH(ITH_CMDQ_SR1_REG) & (0x1 << ITH_CMDQ_ALLIDLE_BIT))
            break;

        usleep(1000);
    } while (--timeout);

    if (timeout == 0)
    {
        LOG_ERR "Wait empty timeout\r\n" LOG_END

    #ifdef CFG_ITH_DBG
        ithCmdQStats();
    #endif

        ret = __LINE__;
    }
    ithCmdQUnlock();
    return ret;
}

void ithCmdQInit(ITHCmdQ* cmdQ)
{
    ASSERT(cmdQ);
    ASSERT(cmdQ->addr);
    ASSERT(cmdQ->size);
    
    ithCmdQ     = cmdQ;
    cmdQBase    = (uint32_t)ithMapVram(cmdQ->addr, cmdQ->size, ITH_VRAM_WRITE);
    currPtr     = 0;
    waitSize    = 0;
}

void ithCmdQExit(void)
{
    ithUnmapVram((void*)cmdQBase, ithCmdQ->size);
    ithCmdQ = NULL;
}

void ithCmdQReset(void)
{
    ASSERT(ithCmdQ);

    // Enable N5CLK
    ithSetRegBitH(ITH_ISP_CLK2_REG, ITH_EN_N5CLK_BIT);

    // Enable command queue clock
    ithSetRegBitH(ITH_CQ_CLK_REG, ITH_EN_M3CLK_BIT);
    ithSetRegBitH(ITH_EN_MMIO_REG, ITH_EN_CQ_MMIO_BIT);
    
    // Reset command queue engine
    ithSetRegBitH(ITH_CQ_CLK_REG, ITH_CQ_RST_BIT);
    ithDelay(1);
    ithClearRegBitH(ITH_CQ_CLK_REG, ITH_CQ_RST_BIT);
    
    // Initialize command queue registers
    ithWriteRegMaskH(ITH_CMDQ_BASE_LO_REG, ithCmdQ->addr, ITH_CMDQ_BASE_LO_MASK);
    ithWriteRegMaskH(ITH_CMDQ_BASE_HI_REG, ithCmdQ->addr >> 16, ITH_CMDQ_BASE_HI_MASK);
    ithWriteRegMaskH(ITH_CMDQ_LEN_REG, ithCmdQ->size / CMDQ_UNIT_SIZE - 1, ITH_CMDQ_LEN_MASK);
    ithWriteRegMaskH(ITH_CMDQ_WR_LO_REG, 0, ITH_CMDQ_WR_LO_MASK);
    ithWriteRegMaskH(ITH_CMDQ_WR_HI_REG, 0, ITH_CMDQ_WR_HI_MASK);
}

uint32_t* ithCmdQWaitSize(uint32_t size)
{
    ASSERT(size > 0);
    ASSERT(ITH_IS_ALIGNED(size, sizeof (uint64_t)));
    ASSERT(ithCmdQ);
    
    currPtr = GetWritePointer();

    // Wait command queue's size is available
    if (WaitAvailableSize(size) != 0)
        return NULL;

    ASSERT(ITH_IS_ALIGNED(currPtr, sizeof (uint64_t)));
    
    waitSize = size;
    
    return (uint32_t*) (cmdQBase + currPtr);
}

void ithCmdQFlush(uint32_t* ptr)
{
    uint32_t cmdsPtr = cmdQBase + currPtr;
    uint32_t cmdsSize = (uint32_t)ptr - cmdsPtr;

#ifdef DEBUG
    if (cmdsSize != waitSize)
        LOG_ERR "CmdQ cmdsSize %d != waitSize %d\r\n", cmdsSize, waitSize LOG_END

#endif // DEBUG
    
    ASSERT(cmdsSize <= waitSize);

    // Flush cache
    ithFlushDCacheRange((void*)cmdsPtr, cmdsSize);
    ithFlushMemBuffer();

    currPtr += cmdsSize;
    SetWritePointer(currPtr);
    waitSize = 0;

    // Check whether decode fail
#ifdef DEBUG
    if (ithReadRegH(ITH_CMDQ_SR1_REG) & (0x1 << ITH_CMDQ_CMQFAIL_BIT))
	{
        LOG_ERR "CmdQ decode fail\r\n" LOG_END
		ithCmdQStats();
		ASSERT(0);
	}
#endif // DEBUG
}

void ithCmdQFlip(unsigned int index)
{
    uint32_t* ptr;

    ithCmdQLock();
    ptr = ithCmdQWaitSize(ITH_CMDQ_SINGLE_CMD_SIZE);
    ITH_CMDQ_SINGLE_CMD(ptr, ITH_HOST_BASE + ITH_CMDQ_FLIPIDX_REG, index);
    ithCmdQFlush(ptr);
    ithCmdQUnlock();
}

void ithCmdQStats(void)
{
    uint32_t baseAddr, writePtr, readPtr, addr;
    char ctrlBits[33], statusBits[33];

    PRINTF("CMDQ SW: addr=0x%X,size=%d,base=0x%X,mutex=0x%X\r\n",
        ithCmdQ->addr, 
        ithCmdQ->size, 
        cmdQBase, 
        (uint32_t)ithCmdQ->mutex);

    baseAddr  = ithReadRegH(ITH_CMDQ_BASE_LO_REG) & ITH_CMDQ_BASE_LO_MASK;
    baseAddr |= (uint32_t)(ithReadRegH(ITH_CMDQ_BASE_HI_REG) & ITH_CMDQ_BASE_HI_MASK) << 16;

    writePtr  = ithReadRegH(ITH_CMDQ_WR_LO_REG) & ITH_CMDQ_WR_LO_MASK;
    writePtr |= (uint32_t)(ithReadRegH(ITH_CMDQ_WR_HI_REG) & ITH_CMDQ_WR_HI_MASK) << 16;

    readPtr   = ithReadRegH(ITH_CMDQ_RD_LO_REG) & ITH_CMDQ_RD_LO_MASK;
    readPtr  |= (uint32_t)(ithReadRegH(ITH_CMDQ_RD_HI_REG) & ITH_CMDQ_RD_HI_MASK) << 16;

    ithUltob(ctrlBits, ithReadRegH(ITH_CMDQ_CR_REG));
    ithUltob(statusBits, ithReadRegH(ITH_CMDQ_SR1_REG));

    PRINTF("CMDQ HW: addr=0x%X,len=%d,writePtr=0x%X,ctl=%s,readPtr=0x%X,sr1=%s\r\n",
        baseAddr,
        (ithReadRegH(ITH_CMDQ_LEN_REG) & ITH_CMDQ_LEN_MASK) >> ITH_CMDQ_LEN_BIT,
        writePtr,
        &ctrlBits[sizeof (uint32_t) * 8 - 16],
        readPtr,
        &statusBits[sizeof (uint32_t) * 8 - 16]);

    ithPrintRegH(ITH_CMDQ_BASE_LO_REG, ITH_CMDQ_SR1_REG - ITH_CMDQ_BASE_LO_REG + sizeof (uint16_t));

#define FORWARD_SIZE 16
    addr = readPtr > FORWARD_SIZE ? readPtr - FORWARD_SIZE : 0;

    ithPrintVram32(baseAddr + addr, FORWARD_SIZE * 2);
}

void ithCmdQEnableClock(void)
{
    // Enable N5CLK
    ithSetRegBitH(ITH_ISP_CLK2_REG, ITH_EN_N5CLK_BIT);

    // Enable command queue clock
    ithSetRegBitH(ITH_CQ_CLK_REG, ITH_EN_M3CLK_BIT);
}

void ithCmdQDisableClock(void)
{
    ithClearRegBitH(ITH_CQ_CLK_REG, ITH_EN_M3CLK_BIT);
    
    if ((ithReadRegH(ITH_ISP_CLK2_REG) & (0x1 << ITH_EN_ICLK_BIT)) == 0)
        ithClearRegBitH(ITH_ISP_CLK2_REG, ITH_EN_N5CLK_BIT);   // disable N5 clock safely
}

void ithCmdQSetTripleBuffer(void)
{
    ithSetRegBitH(ITH_CMDQ_CR_REG, ITH_CMDQ_FLIPBUFMODE_BIT);
}

/**
* Enables specified command queue controls.
*
* @param ctrl the controls to enable.
*/
void ithCmdQCtrlEnable(ITHCmdQCtrl ctrl)
{
    ithSetRegBitH(ITH_CMDQ_CR_REG, ctrl);
}

/**
* Disables specified command queue controls.
*
* @param ctrl the controls to disable.
*/
void ithCmdQCtrlDisable(ITHCmdQCtrl ctrl)
{
    ithClearRegBitH(ITH_CMDQ_CR_REG, ctrl);
}

/**
* Clears the interrupt of command queue.
*/
void ithCmdQClearIntr(void)
{
    ithSetRegBitH(ITH_CMDQ_IR1_REG, ITH_CMDQ_INTCLR_BIT);
}

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
ITHCmdQ* ithCmdQ1;

static uint32_t cmdQBase, currPtr, waitSize;
static uint32_t cmdQBase1, currPtr1, waitSize1;

static uint32_t GetReadPointer(ITHCmdQPortOffset portOffset)
{
    uint32_t readPtr;

    readPtr = (ithReadRegA(ITH_CMDQ_BASE + ITH_CMDQ_RD_REG + portOffset) & ITH_CMDQ_RD_MASK);

    return readPtr;
}

static uint32_t GetWritePointer(ITHCmdQPortOffset portOffset)
{
    uint32_t writePtr;

    writePtr = (ithReadRegA(ITH_CMDQ_BASE + ITH_CMDQ_WR_REG + portOffset) & ITH_CMDQ_WR_MASK);

    return writePtr;
}

static void SetWritePointer(uint32_t ptr, ITHCmdQPortOffset portOffset)
{
    ASSERT(ITH_IS_ALIGNED(ptr, sizeof (uint64_t)));
    ASSERT(ptr != 0);

    ithWriteRegMaskA(ITH_CMDQ_BASE + ITH_CMDQ_WR_REG + portOffset, ptr, ITH_CMDQ_WR_MASK);
}

static void FillNullCommands(ITHCmdQPortOffset portOffset)
{
    uint32_t size, count, i, *ptr;

    if (portOffset == ITH_CMDQ0_OFFSET)
    {
        size = ithCmdQ->size - currPtr;
        if (size == 0)
            return;

        count = size / sizeof (uint32_t);

        ptr = (uint32_t*)(cmdQBase + currPtr);
    }
    else
    {
        size = ithCmdQ1->size - currPtr1;
        if (size == 0)
            return;

        count = size / sizeof (uint32_t);

        ptr = (uint32_t*)(cmdQBase1 + currPtr1);
    }


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
static int WaitAvailableSize(uint32_t size, ITHCmdQPortOffset portOffset)
{
    uint32_t timeout;

    if (portOffset == ITH_CMDQ0_OFFSET) //CMDQ0
    {
        if (currPtr + size >= ithCmdQ->size)
        {
            // Cannot be case 2, else locking size > queue size
            ASSERT(GetWritePointer(portOffset) <= currPtr);

            timeout = ITH_CMDQ_LOOP_TIMEOUT;
            do
            {
                uint32_t readPtr = GetReadPointer(portOffset);

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
            FillNullCommands(portOffset);

            // Reset current pointer to zero
            currPtr = 0;
        }

        // Should currPtr + size < writePtr when in case 2
        ASSERT((GetWritePointer(portOffset) <= currPtr) || (currPtr + size < GetWritePointer(portOffset)));

        timeout = ITH_CMDQ_LOOP_TIMEOUT;
        do
        {
            uint32_t readPtr = GetReadPointer(portOffset);

            // Wait read pointer <= current pointer (case 3 -> case 1) or
            // read pointer - current pointer > required size (case 2 or case 3)
            if (readPtr <= currPtr || readPtr - currPtr >= size)
                return 0;

            LOG_DBG "CMDQ busy2: 0x%X > 0x%X\r\n", readPtr, currPtr LOG_END
                ithTaskYield();

        } while (--timeout);

        LOG_ERR "Wait available2 timeout, size: %d\r\n", size LOG_END
    }
    else //CMDQ1
    {
        if (currPtr1 + size >= ithCmdQ1->size)
        {
            // Cannot be case 2, else locking size > queue size
            ASSERT(GetWritePointer(portOffset) <= currPtr1);

            timeout = ITH_CMDQ_LOOP_TIMEOUT;
            do
            {
                uint32_t readPtr1 = GetReadPointer(portOffset);

                // Wait read pointer <= current pointer (case 3 -> case 1)
                if (readPtr1 <= currPtr1)
                    break;

                LOG_DBG "CMDQ1 busy1: 0x%X > 0x%X\r\n", readPtr1, currPtr1 LOG_END
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
            FillNullCommands(portOffset);

            // Reset current pointer to zero
            currPtr1 = 0;
        }

        // Should currPtr + size < writePtr when in case 2
        ASSERT((GetWritePointer(portOffset) <= currPtr1) || (currPtr1 + size < GetWritePointer(portOffset)));

        timeout = ITH_CMDQ_LOOP_TIMEOUT;
        do
        {
            uint32_t readPtr1 = GetReadPointer(portOffset);

            // Wait read pointer <= current pointer (case 3 -> case 1) or
            // read pointer - current pointer > required size (case 2 or case 3)
            if (readPtr1 <= currPtr1 || readPtr1 - currPtr1 >= size)
                return 0;

            LOG_DBG "CMDQ1 busy2: 0x%X > 0x%X\r\n", readPtr1, currPtr1 LOG_END
                ithTaskYield();

        } while (--timeout);

        LOG_ERR "Wait available2 timeout, size: %d\r\n", size LOG_END
    }


#ifdef CFG_ITH_DBG
        ithCmdQStats();
#endif
    
    return __LINE__;
}

int ithCmdQWaitEmpty(ITHCmdQPortOffset portOffset)
{
    int ret = 0;
    uint32_t timeout;
    ithCmdQLock(portOffset);

    timeout = ITH_CMDQ_LOOP_TIMEOUT;
    do
    {
        if (ithReadRegA(ITH_CMDQ_BASE + ITH_CMDQ_SR1_REG + portOffset) & (0x1 << ITH_CMDQ_ALLIDLE_BIT))
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
    ithCmdQUnlock(portOffset);
    return ret;
}

void ithCmdQInit(ITHCmdQ* cmdQ, ITHCmdQPortOffset portOffset)
{
    if (portOffset == ITH_CMDQ0_OFFSET)
    {
        ASSERT(cmdQ);
        ASSERT(cmdQ->addr);
        ASSERT(cmdQ->size);

        ithCmdQ = cmdQ;
        cmdQBase = (uint32_t)ithMapVram(cmdQ->addr, cmdQ->size, ITH_VRAM_WRITE);
        currPtr = 0;
        waitSize = 0;
    }
    else
    {
        ASSERT(cmdQ);
        ASSERT(cmdQ->addr);
        ASSERT(cmdQ->size);

        ithCmdQ1 = cmdQ;
        cmdQBase1 = (uint32_t)ithMapVram(cmdQ->addr, cmdQ->size, ITH_VRAM_WRITE);
        currPtr1 = 0;
        waitSize1 = 0;
    }

}

void ithCmdQExit(ITHCmdQPortOffset portOffset)
{
    if (portOffset == ITH_CMDQ0_OFFSET)
    {
        ithUnmapVram((void*)cmdQBase, ithCmdQ->size);
        ithCmdQ = NULL;
    }
    else
    {
        ithUnmapVram((void*)cmdQBase1, ithCmdQ1->size);
        ithCmdQ1 = NULL;
    }

}

void ithCmdQReset(ITHCmdQPortOffset portOffset)
{
    if (portOffset == ITH_CMDQ0_OFFSET)
    {
        ASSERT(ithCmdQ);
    }
    else
    {
        ASSERT(ithCmdQ1);
    }

    // Enable N5CLK
    //ithSetRegBitH(ITH_ISP_CLK2_REG, ITH_EN_N5CLK_BIT);

    // Enable command queue clock
    ithSetRegBitA(ITH_HOST_BASE + ITH_CQ_CLK_REG, ITH_EN_M2CLK_BIT);
    ithSetRegBitA(ITH_HOST_BASE + ITH_CQ_CLK_REG, ITH_EN_N2CLK_BIT);
    //ithSetRegBitA(ITH_HOST_BASE + ITH_EN_MMIO_REG, ITH_EN_CQ_MMIO_BIT);

    // Reset command queue engine
    ithSetRegBitA(ITH_HOST_BASE + ITH_CQ_CLK_REG, ITH_CQ_RST_BIT);
    ithDelay(1);
    ithClearRegBitA(ITH_HOST_BASE + ITH_CQ_CLK_REG, ITH_CQ_RST_BIT);

    // Initialize command queue registers
    if (portOffset == ITH_CMDQ0_OFFSET)
    {
        //CMDQ0
        ithWriteRegMaskA(ITH_CMDQ_BASE + ITH_CMDQ_BASE_REG, ithCmdQ->addr << ITH_CMDQ_BASE_BIT, ITH_CMDQ_BASE_MASK);
        ithWriteRegMaskA(ITH_CMDQ_BASE + ITH_CMDQ_LEN_REG, ithCmdQ->size / CMDQ_UNIT_SIZE - 1, ITH_CMDQ_LEN_MASK);
        ithWriteRegMaskA(ITH_CMDQ_BASE + ITH_CMDQ_WR_REG, 0 << ITH_CMDQ_WR_BIT, ITH_CMDQ_WR_MASK);
    }
    else
    {
        //CMDQ1
        ithWriteRegMaskA(ITH_CMDQ_BASE + ITH_CMDQ_BASE_REG + ITH_CMDQ_BASE_OFFSET, ithCmdQ1->addr << ITH_CMDQ_BASE_BIT, ITH_CMDQ_BASE_MASK);
        ithWriteRegMaskA(ITH_CMDQ_BASE + ITH_CMDQ_LEN_REG + ITH_CMDQ_BASE_OFFSET, ithCmdQ1->size / CMDQ_UNIT_SIZE - 1, ITH_CMDQ_LEN_MASK);
        ithWriteRegMaskA(ITH_CMDQ_BASE + ITH_CMDQ_WR_REG + ITH_CMDQ_BASE_OFFSET, 0 << ITH_CMDQ_WR_BIT, ITH_CMDQ_WR_MASK);
    }
}

uint32_t* ithCmdQWaitSize(uint32_t size, ITHCmdQPortOffset portOffset)
{
    ASSERT(size > 0);
    ASSERT(ITH_IS_ALIGNED(size, sizeof (uint64_t)));
   
    if (portOffset == ITH_CMDQ0_OFFSET)
    {
        ASSERT(ithCmdQ);

        currPtr = GetWritePointer(portOffset);

        // Wait command queue's size is available
        if (WaitAvailableSize(size, portOffset) != 0)
            return NULL;

        ASSERT(ITH_IS_ALIGNED(currPtr, sizeof (uint64_t)));

        waitSize = size;

        return (uint32_t*)(cmdQBase + currPtr);
    }
    else
    {
        ASSERT(ithCmdQ1);

        currPtr1 = GetWritePointer(portOffset);

        // Wait command queue's size is available
        if (WaitAvailableSize(size, portOffset) != 0)
            return NULL;

        ASSERT(ITH_IS_ALIGNED(currPtr1, sizeof (uint64_t)));

        waitSize1 = size;

        return (uint32_t*)(cmdQBase1 + currPtr1);
    }

}

void ithCmdQFlush(uint32_t* ptr, ITHCmdQPortOffset portOffset)
{
    if (portOffset == ITH_CMDQ0_OFFSET)
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
        SetWritePointer(currPtr, portOffset);
        waitSize = 0;
    }
    else
    {
        uint32_t cmdsPtr1 = cmdQBase1 + currPtr1;
        uint32_t cmdsSize1 = (uint32_t)ptr - cmdsPtr1;

#ifdef DEBUG
        if (cmdsSize1 != waitSize1)
            LOG_ERR "CmdQ1 cmdsSize1 %d != waitSize1 %d\r\n", cmdsSize1, waitSize1 LOG_END

#endif // DEBUG

        ASSERT(cmdsSize1 <= waitSize1);

        // Flush cache
        ithFlushDCacheRange((void*)cmdsPtr1, cmdsSize1);
        ithFlushMemBuffer();

        currPtr1 += cmdsSize1;
        SetWritePointer(currPtr1, portOffset);
        waitSize1 = 0;
    }

    // Check whether decode fail
#ifdef DEBUG
    if (ithReadRegA(ITH_CMDQ_BASE + ITH_CMDQ_SR1_REG + portOffset) & (0x1 << ITH_CMDQ_CMQFAIL_BIT))
    {
        LOG_ERR "CmdQ decode fail\r\n" LOG_END
            ithCmdQStats();
        ASSERT(0);
    }
#endif // DEBUG
    
}

void ithCmdQFlip(unsigned int index, ITHCmdQPortOffset portOffset)
{
    uint32_t* ptr;

    ithCmdQLock(portOffset);
    ptr = ithCmdQWaitSize(ITH_CMDQ_SINGLE_CMD_SIZE, portOffset);
    ITH_CMDQ_SINGLE_CMD(ptr, ITH_CMDQ_BASE + ITH_CMDQ_FLIPIDX_REG + portOffset, index);
    ithCmdQFlush(ptr, portOffset);
    ithCmdQUnlock(portOffset);
}

void ithCmdQStats(void)
{
    uint32_t baseAddr, writePtr, readPtr, addr;
    uint32_t baseAddr1, writePtr1, readPtr1, addr1;
    char ctrlBits[33], statusBits[33];
    char ctrlBits1[33], statusBits1[33];

    //CMDQ0
    PRINTF("CMDQ 0 SW: addr=0x%X,size=%d,base=0x%X,mutex=0x%X\r\n",
        ithCmdQ->addr, 
        ithCmdQ->size, 
        cmdQBase, 
        (uint32_t)ithCmdQ->mutex);

    baseAddr = (ithReadRegA(ITH_CMDQ_BASE + ITH_CMDQ_BASE_REG) & ITH_CMDQ_BASE_MASK) >> ITH_CMDQ_BASE_BIT;

    writePtr = (ithReadRegA(ITH_CMDQ_BASE + ITH_CMDQ_WR_REG) & ITH_CMDQ_WR_MASK) >> ITH_CMDQ_WR_BIT;

    readPtr = (ithReadRegA(ITH_CMDQ_BASE + ITH_CMDQ_RD_REG) & ITH_CMDQ_RD_MASK) >> ITH_CMDQ_RD_BIT;

    ithUltob(ctrlBits, ithReadRegA(ITH_CMDQ_BASE + ITH_CMDQ_CR_REG));
    ithUltob(statusBits, ithReadRegA(ITH_CMDQ_BASE + ITH_CMDQ_SR1_REG));

    ithPrintf("CMDQ 0 HW: addr=0x%X,len=%d,writePtr=0x%X,ctl=%s,readPtr=0x%X,sr1=%s\r\n",
        baseAddr,
        (ithReadRegA(ITH_CMDQ_BASE + ITH_CMDQ_LEN_REG) & ITH_CMDQ_LEN_MASK) >> ITH_CMDQ_LEN_BIT,
        writePtr,
        &ctrlBits[sizeof(uint32_t) * 8 - 16],
        readPtr,
        &statusBits[sizeof(uint32_t) * 8 - 16]);

    ithPrintRegA(ITH_CMDQ_BASE + ITH_CMDQ_BASE_REG, (0x3C) + sizeof(uint32_t));

#define FORWARD_SIZE 16
    addr = readPtr > FORWARD_SIZE ? readPtr - FORWARD_SIZE : 0;

    ithPrintVram32(baseAddr + addr, FORWARD_SIZE);

    //CMDQ1
    PRINTF("CMDQ 1 SW: addr=0x%X,size=%d,base=0x%X,mutex=0x%X\r\n",
        ithCmdQ1->addr,
        ithCmdQ1->size,
        cmdQBase1,
        (uint32_t)ithCmdQ1->mutex);

    baseAddr1 = (ithReadRegA(ITH_CMDQ_BASE + ITH_CMDQ_BASE_REG + ITH_CMDQ_BASE_OFFSET) & ITH_CMDQ_BASE_MASK) >> ITH_CMDQ_BASE_BIT;

    writePtr1 = (ithReadRegA(ITH_CMDQ_BASE + ITH_CMDQ_WR_REG + ITH_CMDQ_BASE_OFFSET) & ITH_CMDQ_WR_MASK) >> ITH_CMDQ_WR_BIT;

    readPtr1 = (ithReadRegA(ITH_CMDQ_BASE + ITH_CMDQ_RD_REG + ITH_CMDQ_BASE_OFFSET) & ITH_CMDQ_RD_MASK) >> ITH_CMDQ_RD_BIT;

    ithUltob(ctrlBits1, ithReadRegA(ITH_CMDQ_BASE + ITH_CMDQ_CR_REG + ITH_CMDQ_BASE_OFFSET));
    ithUltob(statusBits1, ithReadRegA(ITH_CMDQ_BASE + ITH_CMDQ_SR1_REG + ITH_CMDQ_BASE_OFFSET));

    ithPrintf("CMDQ 1 HW: addr=0x%X,len=%d,writePtr=0x%X,ctl=%s,readPtr=0x%X,sr1=%s\r\n",
        baseAddr1,
        (ithReadRegA(ITH_CMDQ_BASE + ITH_CMDQ_LEN_REG + ITH_CMDQ_BASE_OFFSET) & ITH_CMDQ_LEN_MASK) >> ITH_CMDQ_LEN_BIT,
        writePtr1,
        &ctrlBits1[sizeof(uint32_t)* 8 - 16],
        readPtr1,
        &statusBits1[sizeof(uint32_t)* 8 - 16]);

    ithPrintRegA(ITH_CMDQ_BASE + ITH_CMDQ_BASE_REG + ITH_CMDQ_BASE_OFFSET, 0x3C + sizeof(uint32_t));

#define FORWARD_SIZE 16
    addr1 = readPtr1 > FORWARD_SIZE ? readPtr1 - FORWARD_SIZE : 0;

    ithPrintVram32(baseAddr1 + addr1, FORWARD_SIZE);
}

void ithCmdQEnableClock(void)
{
    // Enable N5CLK
    //ithSetRegBitH(ITH_ISP_CLK2_REG, ITH_EN_N5CLK_BIT);

    // Enable command queue clock
    ithSetRegBitA(ITH_HOST_BASE + ITH_CQ_CLK_REG, ITH_EN_M2CLK_BIT);
}

void ithCmdQDisableClock(void)
{
    ithClearRegBitA(ITH_HOST_BASE + ITH_CQ_CLK_REG, ITH_EN_M2CLK_BIT);

    //if ((ithReadRegH(ITH_ISP_CLK2_REG) & (0x1 << ITH_EN_ICLK_BIT)) == 0)
    //    ithClearRegBitH(ITH_ISP_CLK2_REG, ITH_EN_N5CLK_BIT);   // disable N5 clock safely
}

void ithCmdQSetTripleBuffer(ITHCmdQPortOffset portOffset)
{
    ithSetRegBitA(ITH_CMDQ_BASE + ITH_CMDQ_CR_REG + portOffset, ITH_CMDQ_FLIPBUFMODE_BIT);
}

/**
* Enables specified command queue controls.
*
* @param ctrl the controls to enable.
*/
void ithCmdQCtrlEnable(ITHCmdQCtrl ctrl, ITHCmdQPortOffset portOffset)
{
    ithSetRegBitA(ITH_CMDQ_BASE + ITH_CMDQ_CR_REG + portOffset, ctrl);
}

/**
* Disables specified command queue controls.
*
* @param ctrl the controls to disable.
*/
void ithCmdQCtrlDisable(ITHCmdQCtrl ctrl, ITHCmdQPortOffset portOffset)
{
    ithClearRegBitA(ITH_CMDQ_BASE + ITH_CMDQ_CR_REG + portOffset, ctrl);
}

/**
* Clears the interrupt of command queue.
*/
void ithCmdQClearIntr(ITHCmdQPortOffset portOffset)
{
    ithSetRegBitA(ITH_CMDQ_BASE + ITH_CMDQ_IR1_REG + portOffset, ITH_CMDQ_INTCLR_BIT);
}
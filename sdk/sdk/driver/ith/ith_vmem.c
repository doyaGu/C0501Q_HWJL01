/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * Video memroy management functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"

typedef enum
{
    MCB_UNUSED    = 0x0,
    MCB_FREE      = 0x1,
    MCB_ALLOCATED = 0x2
} McbState;

ITHVmem *ithVmem;

static void MergeBlock(ITHVmemMcb *block)
{
    ITHList    *blockNode = (ITHList *)block;
    ITHVmemMcb *siblingBlock;

    if (blockNode->next != NULL)
    {
        // Merge next block
        siblingBlock = (ITHVmemMcb *)blockNode->next;

        if (siblingBlock->state == MCB_FREE)
        {
            block->size += siblingBlock->size;
            siblingBlock->state = MCB_UNUSED;

            ithListRemove(&ithVmem->usedMcbList, siblingBlock);
            ithVmem->usedMcbCount--;
        }
    }

    if (blockNode->prev != &ithVmem->usedMcbList)
    {
        // Merge previus block
        siblingBlock = (ITHVmemMcb *)blockNode->prev;
        ASSERT(siblingBlock);

        if (siblingBlock->state == MCB_FREE)
        {
            block->addr = siblingBlock->addr;
            block->size += siblingBlock->size;
            siblingBlock->state = MCB_UNUSED;

            ithListRemove(&ithVmem->usedMcbList, siblingBlock);
            ithVmem->usedMcbCount--;
        }
    }
    block->state = MCB_FREE;
}

static ITHVmemMcb *GetNextUnusedMCB(int fromIndex, int *nextIndex)
{
    ITHVmemMcb *mcbs = ithVmem->mcbs;
    int i;

    if (fromIndex < ITH_VMEM_MCB_COUNT)
    {
        for (i = fromIndex; i < ITH_VMEM_MCB_COUNT; ++i)
        {
            if (mcbs[i].state == MCB_UNUSED)
            {
                if (nextIndex)
                    *nextIndex = i + 1;
                return &mcbs[i];
            }
        }
    }
    return NULL;
}

static int SplitBlock(ITHVmemMcb *block, uint32_t alignment, uint32_t size)
{
    int        result;
    uint32_t   aligned_addr;
    int from = 0;

    if (block->size == size) // No need to split
    {
        block->size  = size;
        block->state = MCB_ALLOCATED;
        result       = 0;
        goto end;
    }

    aligned_addr = ITH_ALIGN_UP(block->addr, alignment);
    if (aligned_addr == block->addr)
    {
        ITHVmemMcb *nextBlock = NULL;
        nextBlock = GetNextUnusedMCB(from, &from);
        if (!nextBlock) // Out of mcb
        {
            result = __LINE__;
            LOG_WARN "Out of video mcb\r\n" LOG_END
            goto end;
        }

        nextBlock->addr  = block->addr + size;
        nextBlock->size  = block->size - size;
        nextBlock->state = MCB_FREE;
        block->size      = size;
        block->state     = MCB_ALLOCATED;

        ithListInsertAfter(&ithVmem->usedMcbList, block, nextBlock);
        ithVmem->usedMcbCount++;

        result = 0;
    }
    else
    {
        ITHVmemMcb *prevBlock = NULL, *nextBlock = NULL;
        prevBlock = GetNextUnusedMCB(from, &from);
        nextBlock = GetNextUnusedMCB(from, &from);
        if ((!prevBlock) || (!nextBlock)) // Out of mcb
        {
            result = __LINE__;
            LOG_WARN "Out of video mcb\r\n" LOG_END
            goto end;
        }

        prevBlock->addr  = block->addr;
        prevBlock->size  = aligned_addr - block->addr;
        prevBlock->state = MCB_FREE;
        nextBlock->addr  = aligned_addr + size;
        nextBlock->size  = block->size - size - prevBlock->size;
        nextBlock->state = MCB_FREE;
        block->addr      = aligned_addr;
        block->size      = size;
        block->state     = MCB_ALLOCATED;

        ithListInsertBefore(&ithVmem->usedMcbList, block, prevBlock);
        ithVmem->usedMcbCount++;

        if (nextBlock->size > 0)
        {
            ithListInsertAfter(&ithVmem->usedMcbList, block, nextBlock);
            ithVmem->usedMcbCount++;
        }
        else
            nextBlock->state = MCB_UNUSED;

        MergeBlock(prevBlock);

        result = 0;
    }

end:
    return result;
}

void ithVmemInit(ITHVmem *vmem)
{
    ITHVmemMcb *mcb;

    ASSERT(vmem);
    ASSERT(vmem->totalSize > 0);
    ASSERT(vmem->mutex);

    ithVmem    = vmem;

    // Initialize the first mcb
    mcb        = vmem->mcbs;

    mcb->addr  = vmem->startAddr;
    mcb->size  = vmem->totalSize;
    mcb->state = MCB_FREE;

    ithListPushFront(&vmem->usedMcbList, mcb);

    vmem->freeSize     = vmem->totalSize;
    vmem->usedMcbCount = 1;

    ithVmemStats();
}

uint32_t ithVmemAlignedAlloc(uint32_t alignment, uint32_t size)
{
    uint32_t   addr       = 0;
    ITHVmemMcb *bestBlock = NULL;
    ITHList    *mcbNode;
    int        result;

    ASSERT(ithVmem);
    ASSERT(size > 0);

    // algin to 32 bytes for hw optimize
    size = ITH_ALIGN_UP(size, 32);

    // Out of video memory
    if (size > ithVmem->totalSize)
    {
        LOG_WARN "Out of video memory\r\n" LOG_END
        ithVmemStats();
        return 0;
    }

    ithLockMutex(ithVmem->mutex);

#ifdef ITH_VMEM_BESTFIT
    // Find best mcb
    for (mcbNode = ithVmem->usedMcbList.next; mcbNode; mcbNode = mcbNode->next)
    {
        ITHVmemMcb *block = (ITHVmemMcb *) mcbNode;

        if (block->state == MCB_ALLOCATED)
            continue;

        ASSERT(block->state == MCB_FREE);

        if (block->size >= size + (ITH_ALIGN_UP(block->addr, alignment) - block->addr))
        {
            if (bestBlock == NULL || block->size < bestBlock->size)
            {
                bestBlock = block;
            }
        }
    }
#else
    // Find first suitable mcb
    for (mcbNode = ithVmem->usedMcbList.next; mcbNode; mcbNode = mcbNode->next)
    {
        ITHVmemMcb *block = (ITHVmemMcb *) mcbNode;

        if (block->state == MCB_ALLOCATED)
            continue;

        if (block->state == MCB_FREE && block->size >= size)
        {
            bestBlock = block;
            break;
        }
    }
#endif // ITH_VMEM_BESTFIT

    if (bestBlock)
    {
        result = SplitBlock(bestBlock, alignment, size);
        if (result != 0)
            goto end;

        ithVmem->freeSize -= size;

        addr               = bestBlock->addr;
    }
    else
    {
        // Out of video memory
        LOG_WARN "Out of video memory\r\n" LOG_END
    }

end:
    ithUnlockMutex(ithVmem->mutex);
    return addr;
}

uint32_t ithVmemAlloc(uint32_t size)
{
    return (uint32_t) ithVmemAlignedAlloc(32, ITH_ALIGN_UP(size, 32));
}

void ithVmemFree(uint32_t addr)
{
    ITHList    *mcbNode;
    ITHVmemMcb *mcb = NULL;
    uint32_t   size;

    ASSERT(ithVmem);
    ASSERT(addr != 0);

    ithLockMutex(ithVmem->mutex);

    // Find the mcb node
    for (mcbNode = ithVmem->usedMcbList.next; mcbNode; mcbNode = mcbNode->next)
    {
        mcb = (ITHVmemMcb *) mcbNode;

        if (mcb->addr == addr)
            break;
    }

    if (!mcb)
    {
        LOG_WARN "Bad video address: 0x%X\r\n", addr LOG_END
        goto end;
    }

    ASSERT(mcb->state == MCB_ALLOCATED);

    size               = mcb->size;

    MergeBlock(mcb);
    ithVmem->freeSize += size;

end:
    ithUnlockMutex(ithVmem->mutex);
}

void ithVmemStats(void)
{
    ITHList  *node;
    uint32_t i, usedSize;

    ASSERT(ithVmem);

    ithLockMutex(ithVmem->mutex);

    usedSize = ithVmem->totalSize - ithVmem->freeSize;

    PRINTF("VMEM: usage=%d/%d(%d%%),free=%d,mcb=%d/%d\r\n",
           usedSize,
           ithVmem->totalSize,
           (int)(100.0f * usedSize / ithVmem->totalSize),
           ithVmem->freeSize,
           ithVmem->usedMcbCount,
           ITH_VMEM_MCB_COUNT);

    i = 0;
    for (node = ithVmem->usedMcbList.next; node; node = node->next)
    {
        ITHVmemMcb *block = (ITHVmemMcb *) node;

        PRINTF("  mcb(%d): ptr=0x%X,addr=0x%X,size=%d,state=%d\r\n",
               i++, block, block->addr, block->size, block->state);
    }

    ithUnlockMutex(ithVmem->mutex);
}
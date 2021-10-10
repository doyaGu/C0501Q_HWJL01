/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * ITE RISC header File
 *
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "ite/ith.h"
#include "ite/itp.h"
#include "stdlib.h"
#include "string.h"
#include "ite/ite_risc.h"

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Enumeration Type Definition
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================
#define RISC1_MISC_REG         0x168C
#define RISC1_STALL_BIT_OFFSET 1
#define RISC1_FIRE_BIT_OFFSET  0

#define RISC1_RESET_REG        0x44
#define RISC1_RESET_BIT_OFFSET 12

#define RISC1_PC_LOW           0x16B4
#define RISC1_PC_HIGH          0x16B6

#define RISC2_MISC_REG         0x16CA
#define RISC2_STALL_BIT_OFFSET 1
#define RISC2_FIRE_BIT_OFFSET  0

#define RISC2_RESET_REG        0x44
#define RISC2_RESET_BIT_OFFSET 14

#define RISC2_PC_LOW           0x16D0
#define RISC2_PC_HIGH          0x16D2

#define RISC_REMAP_ADDR_LOW    0x16DE
#define RISC_REMAP_ADDR_HIGH   0x16E0

#define RISC_IMG_START_OFFSET  0x1000

static uint8_t gRiscStratupCode[] = {
    #include "risc_startup.hex"
};

static uint8_t gRiscBuffer[RISC1_IMAGE_SIZE + RISC2_IMAGE_SIZE + AUDIO_MESSAGE_SIZE + SHARE_MEM1_SIZE + SHARE_MEM2_SIZE] __attribute__ ((aligned(32)));
static bool    gbInited = false;

//=============================================================================
//                              Function Declaration
//=============================================================================
int
iteRiscInit(
    void)
{
    //Only need to be inited once.
    if (gbInited == false)
    {
        uint32_t startupAddr = (uint32_t)gRiscBuffer;
        memset(gRiscBuffer, 0x0, sizeof(gRiscBuffer));
        memcpy(gRiscBuffer, gRiscStratupCode, sizeof(gRiscStratupCode));

        ithWriteRegH(RISC_REMAP_ADDR_LOW, (uint16_t)startupAddr);
        ithWriteRegH(RISC_REMAP_ADDR_HIGH, (uint16_t)(startupAddr >> 16));

        ithWriteRegH(RISC1_RESET_REG, 0x7FF);

        gbInited = true;
    }
}
    
int
iteRiscTerminate(
    void)
{
}

uint8_t*
iteRiscGetTargetMemAddress(
    int loadTarget)
{
    switch (loadTarget)
    {
        case RISC1_IMAGE_MEM_TARGET:
            return &gRiscBuffer[RISC_IMG_START_OFFSET];
        case RISC2_IMAGE_MEM_TARGET:
            return &gRiscBuffer[RISC1_IMAGE_SIZE];
        case AUDIO_MESSAGE_MEM_TARGET:
            return &gRiscBuffer[RISC1_IMAGE_SIZE + RISC2_IMAGE_SIZE];
        case SHARE_MEM1_TARGET:
            return &gRiscBuffer[RISC1_IMAGE_SIZE + RISC2_IMAGE_SIZE + AUDIO_MESSAGE_SIZE];
        case SHARE_MEM2_TARGET:
            return &gRiscBuffer[RISC1_IMAGE_SIZE + RISC2_IMAGE_SIZE + AUDIO_MESSAGE_SIZE + SHARE_MEM1_SIZE];
        default:
            return 0;
    }
}

int
iteRiscLoadData(
    int             loadTarget,
    uint8_t*        pData,
    int             dataSize)
{
    switch (loadTarget)
    {
        case RISC1_IMAGE_MEM_TARGET:
            if (dataSize > RISC1_IMAGE_SIZE)
            {
                return INVALID_LOAD_SIZE;
            }
            break;
        case RISC2_IMAGE_MEM_TARGET:
            if (dataSize > RISC2_IMAGE_SIZE)
            {
                return INVALID_LOAD_SIZE;
            }
            break;
        case AUDIO_MESSAGE_MEM_TARGET:
            if (dataSize > AUDIO_MESSAGE_SIZE)
            {
                return INVALID_LOAD_SIZE;
            }
            break;
        case SHARE_MEM1_TARGET:
            if (dataSize > SHARE_MEM1_SIZE)
            {
                return INVALID_LOAD_SIZE;
            }
            break;
        case SHARE_MEM2_TARGET:
            if (dataSize > SHARE_MEM2_SIZE)
            {
                return INVALID_LOAD_SIZE;
            }
            break;
        default:
            return INVALID_MEM_TARGET;
    }
    memcpy(iteRiscGetTargetMemAddress(loadTarget), pData, dataSize);
    
    return ITE_RISC_OK_RESULT;
}

void
iteRiscFireCpu(
    int             targetCpu)
{
    int i;
    switch(targetCpu)
    {
        case RISC1_CPU:
        {
            ithWriteRegMaskH(RISC1_MISC_REG, ((0 << RISC1_STALL_BIT_OFFSET) | (1 << RISC1_FIRE_BIT_OFFSET)), ((1 << RISC1_STALL_BIT_OFFSET) | (1 << RISC1_FIRE_BIT_OFFSET)));
            for (i = 0; i < 10; i++)
                asm ("");
            ithWriteRegMaskH(RISC1_MISC_REG, (0 << RISC1_FIRE_BIT_OFFSET), (1 << RISC1_FIRE_BIT_OFFSET));  
            break;
        }
        case RISC2_CPU:
        {
            ithWriteRegMaskH(RISC2_MISC_REG, ((0 << RISC2_STALL_BIT_OFFSET) | (1 << RISC2_FIRE_BIT_OFFSET)), ((1 << RISC2_STALL_BIT_OFFSET) | (1 << RISC2_FIRE_BIT_OFFSET)));
            for (i = 0; i < 10; i++)
                asm ("");
            ithWriteRegMaskH(RISC2_MISC_REG, (0 << RISC2_FIRE_BIT_OFFSET), (1 << RISC2_FIRE_BIT_OFFSET));          
            break;
        }
        default:
            return;
    }
}

void
iteRiscResetCpu(
    int             targetCpu)
{
    int i;
    switch(targetCpu)
    {
        case RISC1_CPU:
        {
            ithWriteRegMaskH(RISC1_MISC_REG, ((1 << RISC1_STALL_BIT_OFFSET) | (0 << RISC1_FIRE_BIT_OFFSET)), ((1 << RISC1_STALL_BIT_OFFSET) | (1 << RISC1_FIRE_BIT_OFFSET)));
            // reset risc cpu
            ithWriteRegMaskH(RISC1_RESET_REG, 1 << RISC1_RESET_BIT_OFFSET, 1 << RISC1_RESET_BIT_OFFSET);
            for (i = 0; i < 2048; i++)
                asm ("");
            ithWriteRegMaskH(RISC1_RESET_REG, 0 << RISC1_RESET_BIT_OFFSET, 1 << RISC1_RESET_BIT_OFFSET);
            break;
        }
        case RISC2_CPU:
        {
            ithWriteRegMaskH(RISC2_MISC_REG, ((1 << RISC2_STALL_BIT_OFFSET) | (0 << RISC2_FIRE_BIT_OFFSET)), ((1 << RISC2_STALL_BIT_OFFSET) | (1 << RISC2_FIRE_BIT_OFFSET)));
            // reset risc cpu
            ithWriteRegMaskH(RISC2_RESET_REG, 1 << RISC2_RESET_BIT_OFFSET, 1 << RISC2_RESET_BIT_OFFSET);
            for (i = 0; i < 2048; i++)
                asm ("");
            ithWriteRegMaskH(RISC2_RESET_REG, 0 << RISC2_RESET_BIT_OFFSET, 1 << RISC2_RESET_BIT_OFFSET);
            break;
        }
        default:
            return;
    }
}

uint32_t
iteRiscGetProgramCounter(
    int         targetCpu)
{
    uint32_t pc = 0;
    switch(targetCpu)
    {
        case RISC1_CPU:
        {
            pc = ithReadRegH(RISC1_PC_LOW);
            pc |= (((uint32_t) ithReadRegH(RISC1_PC_HIGH)) << 16);
            break;
        }
        case RISC2_CPU:
        {
            pc = ithReadRegH(RISC2_PC_LOW);
            pc |= (((uint32_t) ithReadRegH(RISC2_PC_HIGH)) << 16);
            break;
        }
        default:
            return 0;
    }
    return pc;
}
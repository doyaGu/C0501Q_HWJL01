/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * ITE RISC header File
 *
 */

#ifndef ITE_RISC_H
#define ITE_RISC_H

//=============================================================================
//                              Include Files
//=============================================================================
#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Enumeration Type Definition
//=============================================================================

typedef enum ITE_RISC_ENGINE_TAG
{
    ITE_SW_PERIPHERAL_ENGINE = 1,    /**< SW peripheral engine */
    ITE_RISC_ENGINE_RESERVED
} ITE_RISC_ENGINE;

//=============================================================================
//                              Constant Definition
//=============================================================================

#define RISC_COMMUNICATE_REG      0x16A8
#define RISC_PC_LOW_REG           0x16B4
#define RISC_PC_HIGH_REG          0x16B6

#define RISC1_IMAGE_MEM_TARGET          0
#define RISC2_IMAGE_MEM_TARGET          1
#define AUDIO_MESSAGE_MEM_TARGET        2
#define SHARE_MEM1_TARGET               3
#define SHARE_MEM2_TARGET               4

#define RISC1_CPU                       0
#define RISC2_CPU                       1

#if (CFG_CHIP_FAMILY == 9850)
#define ALT_CPU                         RISC2_CPU
#define ALT_CPU_IMAGE_MEM_TARGET        RISC2_IMAGE_MEM_TARGET
#endif

#if (CFG_CHIP_FAMILY == 9070) // relate to project\codec\codecs_defs.h
#ifdef CFG_AUDIO_CODEC_AACDEC
#define RISC1_IMAGE_SIZE                (900 * 1024)
#elif defined CFG_AUDIO_CODEC_WMADEC
#define RISC1_IMAGE_SIZE                (900 * 1024)
#else
#define RISC1_IMAGE_SIZE                (1000 * 1024)
#endif
#elif (CFG_CHIP_FAMILY == 9910)
#ifdef CFG_AUDIO_CODEC_AACDEC
#define RISC1_IMAGE_SIZE                (900 * 1024)
#elif defined CFG_AUDIO_CODEC_WMADEC
#define RISC1_IMAGE_SIZE                (900 * 1024)
#else
#define RISC1_IMAGE_SIZE                (900 * 1024)
#endif
#else
#ifdef CFG_AUDIO_CODEC_AACDEC
#define RISC1_IMAGE_SIZE                (1000 * 1024)
#elif defined CFG_AUDIO_CODEC_WMADEC
#define RISC1_IMAGE_SIZE                (500 * 1024)
#else
#define RISC1_IMAGE_SIZE                (350 * 1024)
#endif
#endif

#define RISC2_IMAGE_SIZE                (120 * 1024)
#define AUDIO_MESSAGE_SIZE              (32 * 1024)
#define SHARE_MEM1_SIZE                 (16 * 1024)
#define SHARE_MEM2_SIZE                 (16 * 1024)

#define ITE_RISC_OK_RESULT              0
#define INVALID_MEM_TARGET              -1
#define INVALID_LOAD_SIZE               -2

//=============================================================================
//                              Function Declaration
//=============================================================================
int
iteRiscInit(
    void);
    
int
iteRiscTerminate(
    void);
    
int
iteRiscLoadData(
    int             loadTarget,
    uint8_t*        pData,
    int             dataSize);

void
iteRiscFireCpu(
    int             targetCpu);

void
iteRiscResetCpu(
    int             targetCpu);

uint32_t
iteRiscGetProgramCounter(
    int             targetCpu);

uint8_t*
iteRiscGetTargetMemAddress(
    int             loadTarget);


int32_t
iteRiscOpenEngine(
    ITE_RISC_ENGINE engine_type, uint32_t bootmode);

int32_t
iteRiscTerminateEngine(
    void);
#ifdef __cplusplus
}
#endif

#endif
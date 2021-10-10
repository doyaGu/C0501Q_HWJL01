/*
 * Copyright (c) 2004 ITE Technology Corp. All Rights Reserved.
 */
#ifndef NF_CONFIGS_H
#define NF_CONFIGS_H

#define NF_HW_ECC // Undefine this to use software ECC

#if CFG_NAND_PAGE_SIZE == 2048
    #define NF_LARGEBLOCK_2KB
#elif CFG_NAND_PAGE_SIZE == 4096
    #define NF_LARGEBLOCK_4KB
#elif CFG_NAND_PAGE_SIZE == 8192
    #define NF_LARGEBLOCK_8KB
#else
    #define NF_LARGEBLOCK_2KB
#endif

/************************************************************
   define NAND attribution

 ***************************************************************/

#ifdef ENABLE_HW_SCRAMBLER
    #define IsEnHwScrambler      (1)
#else
    #define IsEnHwScrambler      (0)
#endif

#define NF_PITCH_RESERVED_BLOCK_ISSUE

#define MAX_DIE_NUMBER           1

#define ECC_PAGE_SIZE            (512)
#define BCH_ECC_SIZE             (512) //if 4kB/page NAND use BCH 24-bit ECC = (4+((BCH 24-bit)*2))*((page size)/(ECC_PAGE_SIZE)) = (4+48)*(4)= 208

#if defined(CFG_NAND_512B_PAGE)
    #define PAGE_SIZE            (512)
    #define SPARE_SIZE           (6)
    #define SEG_SPR_SIZE         (0)
    #define TOTAL_SPARE_SIZE     (16)
    #define BCH_MODE             (4)
    #define ECC_LENGTH           (10)
    #define BLOCK_SIZE           (16) * PAGE_SIZE
#elif defined(CFG_NAND_2KB_PAGE)
    #define PAGE_SIZE            (2048)
    #define SPARE_SIZE           (24)
    #define SEG_SPR_SIZE         (6)
    #define TOTAL_SPARE_SIZE     (64)
    #define BCH_MODE             (4)
    #define ECC_LENGTH           (10)

    #if defined(CFG_NAND_TYPE_SLC)
        #define BLOCK_SIZE       (64) * PAGE_SIZE
    #else
        #define BLOCK_SIZE       (128) * PAGE_SIZE
    #endif
#elif defined(CFG_NAND_4KB_PAGE)
    #define PAGE_SIZE            (4096)
    #define SPARE_SIZE           (24)
    #define SEG_SPR_SIZE         (0)

    #if defined(CFG_NAND_TYPE_SLC)
        #define BLOCK_SIZE       (64) * PAGE_SIZE
    #else
        #define BLOCK_SIZE       (128) * PAGE_SIZE
    #endif

    #if defined(NAND_SPARE_SIZE_208_BYTE)
        #define TOTAL_SPARE_SIZE 208
        #define BCH_MODE         14
        #define ECC_LENGTH       23
    #else
        #define TOTAL_SPARE_SIZE 128
        #define BCH_MODE         8
        #define ECC_LENGTH       13
    #endif
#elif defined(CFG_NAND_8KB_PAGE)
    #define PAGE_SIZE            (8192)
    #define SPARE_SIZE           (24)
    #define SEG_SPR_SIZE         (0)

    #if defined(CFG_NAND_TYPE_SLC)
        #define BLOCK_SIZE       (64) * PAGE_SIZE
    #else
        #define BLOCK_SIZE       (128) * PAGE_SIZE
    #endif

    #if defined(NAND_SPARE_SIZE_436_BYTE)
        #define TOTAL_SPARE_SIZE 436
        #define BCH_MODE         14
        #define ECC_LENGTH       23
    #else
        #define TOTAL_SPARE_SIZE 376
        #define BCH_MODE         8
        #define ECC_LENGTH       13
    #endif
#else
    #define NF_PAGE_SIZE         2048
    #define NF_BLOCK_SIZE        2048
    #define NF_BLOCK_SIZE        2048
#endif

#define DATA_SIZE                (PAGE_SIZE + SPARE_SIZE)

#define LL_RP_1STHALF            0x01
#define LL_RP_2NDHALF            0x00
#define LL_RP_DATA               0xfe
#define LL_RP_SPARE              0xff

// Log definitions
typedef enum
{
    NAND_ZONE_ERROR   = (0x1 << 0),
    NAND_ZONE_WARNING = (0x1 << 1),
    NAND_ZONE_INFO    = (0x1 << 2),
    NAND_ZONE_DEBUG   = (0x1 << 3),
    NAND_ZONE_ENTER   = (0x1 << 4),
    NAND_ZONE_LEAVE   = (0x1 << 5),
    NAND_ZONE_ALL     = 0xFFFFFFFF
} NANDLogZones;

#ifndef LOG_INFO
    #define NAND_LOG_ZONES (NAND_ZONE_ERROR | NAND_ZONE_WARNING)

    #define LOG_ERROR      ((void) ((NAND_ZONE_ERROR & NAND_LOG_ZONES) ? (printf ("[NAND][ERROR]"
    #define LOG_WARNING    ((void) ((NAND_ZONE_WARNING & NAND_LOG_ZONES) ? (printf ("[NAND][WARNING]"
    #define LOG_INFO       ((void) ((NAND_ZONE_INFO & NAND_LOG_ZONES) ? (printf ("[NAND][INFO]"
    #define LOG_DEBUG      ((void) ((NAND_ZONE_DEBUG & NAND_LOG_ZONES) ? (printf ("[NAND][DEBUG]"
    #define LOG_ENTER      ((void) ((NAND_ZONE_ENTER & NAND_LOG_ZONES) ? (printf ("[NAND][ENTER]"
    #define LOG_LEAVE      ((void) ((NAND_ZONE_LEAVE & NAND_LOG_ZONES) ? (printf ("[NAND][LEAVE]"
    #define LOG_DATA       ((void) ((true) ? (printf (
    #define LOG_INFO2      ((void) ((true) ? (printf (
    #define LOG_END        )), 1 : 0));
#endif

#ifdef WIN32
    #ifdef NF_BUILD_DLL
        #define DLLAPI_EX
        #ifdef DLLAPI_EX
            #define DLLAPI __declspec(dllexport)   // export DLL information
        #else
            #define DLLAPI __declspec(dllimport)   // import DLL information
        #endif
    #else
        #define DLLAPI
    #endif
#elif defined(NUCLEUS_PLUS)

#elif defined(_WIN32_WCE)

#elif defined(__linux)

#elif defined(__OPENRTOS__)

//#define NF_USE_OLD_VERSION
    #define NF_PATCH_READ_10K_LIFE_CYCLE_ISSUE
    #define NF_PATCH_NORBOOT
    #define DLLAPI

    #if defined(NF_NAND_BOOT) && defined(NF_USE_OLD_VERSION)
        #error "NF_NAND_BOOT and NF_USE_OLD_VERSION can not define simultaneously"
    #endif
#else
    #error Unknown target!
#endif // #ifdef WIN32

#ifndef _WIN32_WCE
#endif

#endif
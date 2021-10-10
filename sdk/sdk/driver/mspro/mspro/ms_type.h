/*
 * Copyright (c) 2007 ITE Corp. All Rights Reserved.
 */
/** @file
 *  Use as MS (pro) type header file.
 *
 * @author Irene Lin
 */
#ifndef MS_TYPE_H
#define MS_TYPE_H

//=============================================================================
//                              Include Files
//=============================================================================
#include "mspro/config.h"

extern MMP_UINT32 INT_CED, INT_CMDNK, INT_ERR, INT_BREQ;

//=============================================================================
//                              Constant Definition
//=============================================================================
typedef enum MS_FLAGS_TAG
{
    MS_FLAGS_PARALLEL_INTERFACE        = (0x00000001 << 0),
    MS_FLAGS_ATTRIB_READY              = (0x00000001 << 1),
    MS_FLAGS_WRITE_PROTECT             = (0x00000001 << 2),
    /** Only for MS card */
    MS_FLAGS_USE_EX_CMD_TIMEOUT        = (0x00000001 << 3),
    MS_FLAGS_BOOT_AREA_PROTECTION_FLAG = (0x00000001 << 4),
    MS_FLAGS_FIND_FIRST_BOOT_BLOCK     = (0x00000001 << 5),
    MS_FLAGS_MS_SUPPORT_PARALLEL       = (0x00000001 << 6),
    MS_FLAGS_MS_ROM                    = (0x00000001 << 7),
    MS_FLAGS_BLOCK_HAS_PAGE_NG         = (0x00000001 << 8),
    MS_FLAGS_READ_PROTECT              = (0x00000001 << 9),
    MS_FLAGS_WRITE_DATA_ERROR          = (0x00000001 << 10),

    MS_FLAGS_DMA_ENABLE                = (0x00000001 << 11),
    MS_FLAGS_LAST_IS_READ              = (0x00000001 << 12)
} MS_FLAGS;

typedef enum MS_DEVICE_TYPE_TAG
{
    MS_CARD_ERROR = 0,
    MS_CARD       = 1,
    MS_CARD_PRO   = 2
} MS_DEVICE_TYPE;

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef MMP_INT (*MS_GET_ATTRIB)(MS_PRO_CARD_ATTRIB *attrib);
typedef MMP_INT (*MS_READ_MULTISECTOR)(MMP_UINT32 sector, MMP_UINT16 count, MMP_UINT8 *data);
typedef MMP_INT (*MS_WRITE_MULTISECTOR)(MMP_UINT32 sector, MMP_UINT16 count, MMP_UINT8 *data);
typedef MMP_INT (*MS_READ_PAGEDATA)(MMP_UINT8 *data);
typedef MMP_INT (*MS_WRITE_PAGEDATA)(MMP_UINT8 *data);

typedef struct MS_MEDIA_TYPE_TAG
{
    MMP_UINT8      ms_status;
    MMP_UINT8      ms_type;
    MMP_UINT8      ms_category;
    MMP_UINT8      ms_class;
    MMP_UINT8      ms_parallel;
    MS_DEVICE_TYPE ms_device_type;
} MS_MEDIA_TYPE;

typedef struct MEM_STICK_CARD_STRUCT_TAG
{
#if 0
    MMP_UINT16         chipId;
    MMP_UINT16         chipVersion;
#endif
    MMP_UINT32         flags;
    MS_MEDIA_TYPE      Media_type;
    MMP_UINT32         FIFO_Size;
    MMP_UINT32         FIFO_Width;

    MMP_INT            dmaCh;
    MS_PRO_CARD_ATTRIB attrib;

    MMP_UINT32         gpioPowerEn;
    MMP_UINT32         gpioCardInsert;

    /** Only for Memory Stick Pro card. */
    MMP_UINT8 entryCount;

    /** Only for Memory Stick card. */
    MMP_UINT32 mediaSize;
    MMP_UINT32 pageNum;
    MMP_UINT16 physicalBlockNum;
    MMP_UINT16 logicalBlockNum;
    MMP_UINT8  *tmpAddr;           /** for ms write use */
    MMP_UINT16 segmentNum;
    MMP_UINT16 bootBlockNum;
    MMP_UINT32 bootBlock0Index;
    MMP_UINT32 bootBlock1Index;
    MMP_UINT16 blockSize;
    MMP_UINT16 reserved;
    MMP_UINT32 disabledBlockOffset;
    MMP_UINT32 disabledBlockNum;
    MMP_UINT32 unusedBlockIndex;
    MMP_UINT32 lastUpdateIndex;
    MMP_UINT32 lastUpdateSegment;
    MMP_UINT32 lastLogBlockAddr;
    MMP_UINT32 irqGpio;

#if defined(MS_WORKAROUND)
    MMP_UINT8 sysReg;
    MMP_UINT8 reserved1[3];
#endif
} MEM_STICK_CARD_STRUCT;

#endif
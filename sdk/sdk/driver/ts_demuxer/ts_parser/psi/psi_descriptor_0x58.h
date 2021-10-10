/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file psi_descriptor_0x58.h
 * Application interface for the DVB "local time offset" descriptor
 * decoder and generator. This descriptor's definition can be found in
 * ETSI EN 300 468 section 6.2.19.
 *
 * @author I-Chun Lai
 * @version 0.1
 */

#ifndef PSI_DESCRIPTOR_0X58_H
#define PSI_DESCRIPTOR_0X58_H

#include "ite/mmp_types.h"
#include "psi_descriptor_kit.h"
#include "psi_time.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
#define LOCAL_TIME_OFFSET_DESCRIPTOR_TAG (0x58)
#define MAX_LOCAL_TIME_OFFSET_COUNT      (0x5)

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct PSI_LOCAL_TIME_OFFSET_TAG
{
    MMP_UINT8  country_code[4];
    //MMP_UINT32      country_region_id;
    MMP_UINT32 local_time_offset_polarity;
    MMP_UINT32 local_time_offset;
    //PSI_MJDBCD_TIME time_of_change;
    //MMP_UINT32      next_time_offset;
} PSI_LOCAL_TIME_OFFSET;

/*
 * This structure is used to store a decoded "local time offset"
 * descriptor. (ETSI EN 300 468 section 6.2.19).
 */
typedef struct PSI_LOCAL_TIME_OFFSET_DESCRIPTOR_TAG
{
    MMP_UINT32            totalLocalTimeOffsetCount;
    PSI_LOCAL_TIME_OFFSET tLocalTimeOffset[MAX_LOCAL_TIME_OFFSET_COUNT];
} PSI_LOCAL_TIME_OFFSET_DESCRIPTOR;

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
/**
 * "local time offset" descriptor decoder.
 *
 * @param ptDescriptor  pointer to the descriptor structure
 * @return a pointer to a new "local time offset" descriptor structure
 *         which contains the decoded data.
 */
//=============================================================================
PSI_LOCAL_TIME_OFFSET_DESCRIPTOR *
psiDescriptor_DecodeLocalTimeOffsetDescriptor(
    PSI_DESCRIPTOR *ptDescriptor);

#ifdef __cplusplus
}
#endif

#endif // End of #ifndef PSI_DESCRIPTOR_0X58_H
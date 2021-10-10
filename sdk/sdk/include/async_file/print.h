/*
 * Copyright (c) 2004 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * The print functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef PAL_PRINT_H
#define PAL_PRINT_H

#include "async_file/def.h"

#ifdef __cplusplus
extern "C" {
#endif

void
PalEnablePrintBuffer(
    MMP_BOOL   enable,
    MMP_UINT32 size);

void
PalEnableUartPrint(
    MMP_BOOL   enable,
    MMP_UINT32 gpio_group,
    MMP_UINT32 gpio_pin,
    MMP_UINT32 baud_rate);

MMP_INT
PalPrintf(
    const MMP_CHAR *format,
    ...);

#ifdef __cplusplus
}
#endif

#endif /* PAL_PRINT_H */
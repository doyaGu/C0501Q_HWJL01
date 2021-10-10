/*
 * Copyright (c) 2004 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * PAL configurations.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef CONFIG_H
#define CONFIG_H

//#include "pal/print.h"

/* Log zones definition */
#define LOG_ZONES    (0) //MMP_BIT_ALL & ~MMP_ZONE_ENTER & ~MMP_ZONE_LEAVE)

/* Log fucntions definition */
#define LOG_ERROR   ((void) ((MMP_ZONE_ERROR & LOG_ZONES) ? (PalPrintf("[SMEDIA][PAL][ERROR]"
#define LOG_WARNING ((void) ((MMP_ZONE_WARNING & LOG_ZONES) ? (PalPrintf("[SMEDIA][PAL][WARNING]"
#define LOG_INFO    ((void) ((MMP_ZONE_INFO & LOG_ZONES) ? (PalPrintf("[SMEDIA][PAL][INFO]"
#define LOG_DEBUG   ((void) ((MMP_ZONE_DEBUG & LOG_ZONES) ? (PalPrintf("[SMEDIA][PAL][DEBUG]"
#define LOG_ENTER   ((void) ((MMP_ZONE_ENTER & LOG_ZONES) ? (PalPrintf("[SMEDIA][PAL][ENTER]"
#define LOG_LEAVE   ((void) ((MMP_ZONE_LEAVE & LOG_ZONES) ? (PalPrintf("[SMEDIA][PAL][LEAVE]"

#define LOG_END     )), 1 : 0));

#endif /* CONFIG_H */

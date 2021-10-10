/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file psi_time.h
 * Use to process time conversion and presentation.
 * @author I-Chun Lai
 * @version 0.1
 */
#ifndef PSI_TIME_H
#define PSI_TIME_H

#include "ite/mmp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Macro Definition
//=============================================================================

#define PSI_MJDBCD_TIME_EQUAL(a, b)           (((a).high16 == (b).high16) && ((a).low24 == (b).low24))

#define PSI_MJDBCD_TIME_LESS_OR_EQUAL(a, b)   (((a).high16 < (b).high16) \
                                           || (((a).high16 == (b).high16) && ((a).low24 <= (b).low24)))
#define PSI_MJDBCD_TIME_LESS(a, b)            (((a).high16 < (b).high16) \
                                           || (((a).high16 == (b).high16) && ((a).low24 < (b).low24)))
#define PSI_MJDBCD_TIME_LARGER(a, b)          (((a).high16 > (b).high16) \
                                           || (((a).high16 == (b).high16) && ((a).low24 > (b).low24)))
#define PSI_MJDBCD_TIME_LARGER_OR_EQUAL(a, b) (((a).high16 > (b).high16) \
                                           || (((a).high16 == (b).high16) && ((a).low24 >= (b).low24)))

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct PSI_MJDBCD_TIME_TAG
{
    MMP_UINT32 low24;
    MMP_UINT32 high16;
} PSI_MJDBCD_TIME;

typedef struct PSI_YMDHMS_TIME_TAG
{
    MMP_UINT16 year;
    MMP_UINT8  month;
    MMP_UINT8  day;
    MMP_UINT8  dayOfWeek;

    MMP_INT8   hour;
    MMP_INT8   minute;
    MMP_INT8   second;
} PSI_YMDHMS_TIME;

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
/**
 * Convert Modified Julian Date (MJD) to YMD format
 *
 * @param mjd   Modified Julian Date
 * @return Year/Month/Day (YMD) format
 *      ----------------------------------------
 *      |        Year        |  Month |   Day  |
 *      ----------------------------------------
 *              16-bit          8-bit    8-bit
 * @example
 *      MMP_UINT32 mjd = 53919;
 *      MMP_UINT32 ymd = psiTime_MJDToYMD(mjd);
 *
 *      MMP_INT year  = (ymd & 0xFFFF0000) >> 16;   (== 2006)
 *      MMP_INT month = (ymd & 0x0000FF00) >>  8;   (==    7)
 *      MMP_INT day   = (ymd & 0x000000FF) >>  0;   (==    3)
 */
//=============================================================================
MMP_UINT32
psiTime_MJDToYMD(
    MMP_UINT32 mjd);

PSI_YMDHMS_TIME
psiTime_MjdBcdToYmdHms(
    PSI_MJDBCD_TIME tMjdBcdTime);

PSI_MJDBCD_TIME
psiTime_Add(
    PSI_MJDBCD_TIME tMjdBcdTime,
    MMP_INT32       hms);

PSI_MJDBCD_TIME
psiTime_Sub(
    PSI_MJDBCD_TIME tMjdBcdTime,
    MMP_UINT32      hms);

//=============================================================================
/**
 * Tests the UTC times for equality.
 *
 * @param tMJDBCDTimeA  the time to be compared
 * @param tMJDBCDTimeB  the time to be compared
 * @returns 1) a negative number if time A is earlier than time B
 *          2) zero if the two times are equal
 *          3) a positive number is time A is later than time B.
 */
//=============================================================================
MMP_INT
psiTime_Compare(
    const PSI_MJDBCD_TIME tMJDBCDTimeA,
    const PSI_MJDBCD_TIME tMJDBCDTimeB);

//=============================================================================
/**
 * Convert BCD to minutes (ignore seconds)
 *
 * @param bcd   BCD of the UTC information.
 */
//=============================================================================
MMP_UINT32
psiTime_BcdToMinute(MMP_UINT32 bcd);

//=============================================================================
/**
 * Assuming that tMJDBCDTimeA > tMJDBCDTimeB, this function calculates time distance
 * (tMJDBCDTimeA - tMJDBCDTimeB)in minutes(pixels).
 *
 * @param tMJDBCDTimeA  UTC time
 * @param tMJDBCDTimeB  UTC time
 *
 * @return           time length in pixels.
 */
//=============================================================================
MMP_INT32
psiTime_TimeDiffInMinute(
    PSI_MJDBCD_TIME tMJDBCDTimeA,
    PSI_MJDBCD_TIME tMJDBCDTimeB);

//=============================================================================
/**
 * Check if the UTC time is valid.
 *
 * @return MMP_TRUE if the UTC time is valid.
 */
//=============================================================================
MMP_BOOL
psiTime_IsValidUtcTime(
    PSI_MJDBCD_TIME tTime);

#ifdef __cplusplus
}
#endif

#endif // End of #ifndef PSI_TIME_H
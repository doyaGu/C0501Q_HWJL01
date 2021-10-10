/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file psi_time.h
 * Use to process time conversion and presentation.
 * @author I-Chun Lai
 * @version 0.1
 */

#include "psi_time.h"
#include "bcd.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

#define MIN_MJD_VALID_DATE                      (0x3AE7)
#define MAX_MJD_VALID_DATE                      (0xFFFF)

#define BCD_HOUR(bcdHMS)    ((MMP_UINT8)(((bcdHMS) & 0xFF0000) >> 0x10))
#define BCD_MINU(bcdHMS)    ((MMP_UINT8)(((bcdHMS) & 0x00FF00) >> 0x8))
#define BCD_SEC(bcdHMS)     ((MMP_UINT8)((bcdHMS) & 0x0000FF))

//=============================================================================
//                              Private Function Declaration
//=============================================================================

static MMP_INLINE MMP_INT8
_Bcd2Dec(
     MMP_UINT8 bcd);

static MMP_INLINE MMP_UINT8
_Dec2Bcd(
     MMP_UINT8 dec);

static MMP_INLINE MMP_INT8
_BcdHour(
    MMP_UINT32 bcdHMS);

static MMP_INLINE MMP_INT8
_BcdMin(
    MMP_UINT32 bcdHMS);

static MMP_INLINE MMP_INT8
_BcdSec(
    MMP_UINT32 bcdHMS);

//=============================================================================
//                              Public Function Definition
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
    MMP_UINT32 mjd)
{
    MMP_UINT32 ymd  = 0;
    MMP_INT32 y1    = 0;
    MMP_INT32 m1    = 0;
    MMP_INT32 k     = 0;
    MMP_INT   y     = 0;
    MMP_INT   m     = 0;
    MMP_INT   d     = 0;

    y1 = (mjd * 20 - 301564) / 7305;
    m1 = (mjd * 10000 - 149561000 - ((y1 * 1461) >> 2) * 10000) / 306001;
    k  = (m1 == 14 || m1 == 15) ? 1 : 0;

    y = y1 + k + 1900;
    m = m1 - 1 - k * 12;
    d = mjd - 14956 - ((y1 * 1461) >> 2) - ((m1 * 125338) >> 12);
    ymd = y << 16 | m << 8 | d;

    return ymd;
}

PSI_YMDHMS_TIME
psiTime_MjdBcdToYmdHms(
    PSI_MJDBCD_TIME tMjdBcdTime)
{
    PSI_YMDHMS_TIME tYmdHmsTime = {0};

    tYmdHmsTime.hour   = _Bcd2Dec(_BcdHour(tMjdBcdTime.low24));
    tYmdHmsTime.minute = _Bcd2Dec(_BcdMin( tMjdBcdTime.low24));
    tYmdHmsTime.second = _Bcd2Dec(_BcdSec( tMjdBcdTime.low24));

    if (tYmdHmsTime.hour   < 24
     && tYmdHmsTime.minute < 60
     && tYmdHmsTime.second < 60)
    {
        MMP_UINT32 ymd          = psiTime_MJDToYMD(tMjdBcdTime.high16);
        tYmdHmsTime.year        = (MMP_UINT16)(ymd >> 16);
        tYmdHmsTime.month       = (MMP_UINT8)((ymd & 0xFF00) >>  8);
        tYmdHmsTime.day         = (MMP_UINT8)(ymd & 0x00FF);
        tYmdHmsTime.dayOfWeek   = (MMP_UINT8)((tMjdBcdTime.high16 + 2) % 7 + 1);
    }
    else
    {
        PalMemset(&tYmdHmsTime, 0x0, sizeof(PSI_YMDHMS_TIME));
    }

    return tYmdHmsTime;
}

PSI_MJDBCD_TIME
psiTime_Add(
    PSI_MJDBCD_TIME tMjdBcdTime,
    MMP_INT32 bcdHMS)
{
    if (bcdHMS > 0)
    {
#if 1
        MMP_INT hour = _Bcd2Dec(BCD_HOUR(tMjdBcdTime.low24)) + _Bcd2Dec(BCD_HOUR(bcdHMS));
        MMP_INT min  = _Bcd2Dec(BCD_MINU(tMjdBcdTime.low24)) + _Bcd2Dec(BCD_MINU(bcdHMS));
        MMP_INT sec  = _Bcd2Dec(BCD_SEC( tMjdBcdTime.low24)) + _Bcd2Dec(BCD_SEC( bcdHMS));

        if (60 <= sec)
        {
            sec -= 60;
            ++min;
        }
        if (60 <= min)
        {
            min -= 60;
            ++hour;
        }
        if (24 <= hour)
        {
            hour -= 24;
            ++tMjdBcdTime.high16;
        }

        tMjdBcdTime.low24 =
              (((MMP_UINT32)_Dec2Bcd((MMP_UINT8)hour)) << 0x10)
            + (((MMP_UINT32)_Dec2Bcd((MMP_UINT8)min )) << 0x8)
            + (((MMP_UINT32)_Dec2Bcd((MMP_UINT8)sec )) << 0x0);
#else
        MMP_INT32 bcdHour = bcd_AddBcd(BCD_HOUR(tMjdBcdTime.low24), BCD_HOUR(bcdHMS));
        MMP_INT32 bcdMin  = bcd_AddBcd(BCD_MINU(tMjdBcdTime.low24), BCD_MINU(bcdHMS));
        MMP_INT32 bcdSec  = bcd_AddBcd(BCD_SEC( tMjdBcdTime.low24), BCD_SEC( bcdHMS));

        if (0x60 <= bcdSec)
        {
            bcdSec = bcd_SubBcd(bcdSec, 0x60);
            bcdMin = bcd_AddBcd(bcdMin, 0x1);
        }
        if (0x60 <= bcdMin)
        {
            bcdMin  = bcd_SubBcd(bcdMin, 0x60);
            bcdHour = bcd_AddBcd(bcdHour, 0x1);
        }
        if (0x24 <= bcdHour)
        {
            bcdHour = bcd_SubBcd(bcdHour, 0x24);
            ++tMjdBcdTime.high16;
        }
        tMjdBcdTime.low24 = (bcdHour << 0x10)
            + (bcdMin  << 0x8)
            + (bcdSec  << 0x0);
#endif
    }
    else if (bcdHMS < 0)
    {
        tMjdBcdTime = psiTime_Sub(tMjdBcdTime, -(bcdHMS));
    }

    return tMjdBcdTime;
}

PSI_MJDBCD_TIME
psiTime_Sub(
    PSI_MJDBCD_TIME tMjdBcdTime,
    MMP_UINT32 bcdHMS)
{
#if 1
    MMP_INT hour = _Bcd2Dec(BCD_HOUR(tMjdBcdTime.low24)) - _Bcd2Dec(BCD_HOUR(bcdHMS));
    MMP_INT min  = _Bcd2Dec(BCD_MINU(tMjdBcdTime.low24)) - _Bcd2Dec(BCD_MINU(bcdHMS));
    MMP_INT sec  = _Bcd2Dec(BCD_SEC( tMjdBcdTime.low24)) - _Bcd2Dec(BCD_SEC( bcdHMS));

    if (sec < 0)
    {
        sec += 60;
        --min;
    }
    if (min < 0)
    {
        min += 60;
        --hour;
    }
    if (hour < 0)
    {
        hour += 24;
        --tMjdBcdTime.high16;
    }

    tMjdBcdTime.low24 = (((MMP_UINT32)_Dec2Bcd((MMP_UINT8)hour)) << 0x10)
                      + (((MMP_UINT32)_Dec2Bcd((MMP_UINT8)min )) << 0x8)
                      + (((MMP_UINT32)_Dec2Bcd((MMP_UINT8)sec )) << 0x0);
#else
    MMP_INT32 bcdHour = bcd_SubBcd(BCD_HOUR(tMjdBcdTime.low24), BCD_HOUR(bcdHMS));
    MMP_INT32 bcdMin  = bcd_SubBcd(BCD_MINU(tMjdBcdTime.low24), BCD_MINU(bcdHMS));
    MMP_INT32 bcdSec  = bcd_SubBcd(BCD_SEC( tMjdBcdTime.low24), BCD_SEC( bcdHMS));

    if (bcdSec < 0)
    {
        bcdSec = bcd_AddBcd(bcdSec, 0x60);
        bcdMin = bcd_SubBcd(bcdMin, 0x1);
    }
    if (bcdMin < 0)
    {
        bcdMin = bcd_AddBcd(bcdMin, 0x60);
        bcdHour = bcd_SubBcd(bcdHour, 0x1);
    }
    if (bcdHour < 0)
    {
        bcdHour = bcd_AddBcd(bcdHour, 0x24);
        tMjdBcdTime.high16--;
    }
    tMjdBcdTime.low24 = (bcdHour << 0x10)
                   + (bcdMin  << 0x8)
                   + (bcdSec  << 0x0);
#endif

    return tMjdBcdTime;
}

//=============================================================================
/**
 * Tests the UTC times for equality.
 *
 * @param tMJDBCDTimeA the time to be compared
 * @param tMJDBCDTimeB the time to be compared
 * @returns 1) a negative number if time A is earlier than time B
 *          2) zero if the two times are equal
 *          3) a positive number is time A is later than time B.
 */
//=============================================================================
MMP_INT
psiTime_Compare(
    const PSI_MJDBCD_TIME tTimeA,
    const PSI_MJDBCD_TIME tTimeB)
{
#if 1
    MMP_INT result = (MMP_INT32)tTimeA.high16 - (MMP_INT32)tTimeB.high16;
    if (result == 0)
    {
        result = (MMP_INT32)tTimeA.low24 - (MMP_INT32)tTimeB.low24;
    }
    return result;
#else
    if (tTimeA.high16 < tTimeB.high16)
        return -1;
    else if (tTimeA.high16 > tTimeB.high16)
        return 1;
    else
    {
        if (tTimeA.low24 < tTimeB.low24)
            return -1;
        else if (tTimeA.low24 > tTimeB.low24)
            return 1;
    }

    return 0;
#endif
}

//=============================================================================
/**
 * Convert BCD to minutes (ignore seconds)
 *
 * @param bcd   BCD of the UTC information.
 */
//=============================================================================
MMP_UINT32
psiTime_BcdToMinute(MMP_UINT32 bcd)
{
    return ((bcd & 0xF00000) >> 20) * 600
        +  ((bcd & 0x0F0000) >> 16) * 60
        +  ((bcd & 0x00F000) >> 12) * 10
        +  ((bcd & 0x000F00) >> 8);
}

//=============================================================================
/**
 * Assuming that tTimeA > tTimeB, this function calculates time
 * difference (tTimeA - tTimeB) in minutes.
 *
 * @param tMJDBCDTimeA  time
 * @param tMJDBCDTimeB  time
 *
 * @return  time length in pixels.
 */
//=============================================================================
MMP_INT32
psiTime_TimeDiffInMinute(
    PSI_MJDBCD_TIME tTimeA,
    PSI_MJDBCD_TIME tTimeB)
{
    MMP_INT hourA = _Bcd2Dec(_BcdHour(tTimeA.low24));
    MMP_INT minA  = _Bcd2Dec(_BcdMin( tTimeA.low24));
    MMP_INT hourB = _Bcd2Dec(_BcdHour(tTimeB.low24));
    MMP_INT minB  = _Bcd2Dec(_BcdMin( tTimeB.low24));
    MMP_INT32 timeDiffInMin = 0;

    if (tTimeA.high16 != tTimeB.high16)
    {
        timeDiffInMin += (tTimeA.high16 - tTimeB.high16) * 24 * 60;
    }

    if (tTimeA.low24 != tTimeB.low24)
    {
        timeDiffInMin += (hourA - hourB) * 60 + (minA - minB);
    }

    // return (MMP_UINT32)SMTK_ABS(timeDiffInMin);
    return (MMP_INT32)timeDiffInMin;
}

//=============================================================================
/**
* Check if the UTC time is valid.
*
* @return MMP_TRUE if the UTC time is valid.
*/
//=============================================================================
MMP_BOOL
psiTime_IsValidUtcTime(
    PSI_MJDBCD_TIME tTime)
{
    MMP_BOOL result = MMP_FALSE;

    if (MIN_MJD_VALID_DATE <= tTime.high16 && tTime.high16 <= MAX_MJD_VALID_DATE)
    {
        if (bcd_IsBcd(tTime.low24) && !bcd_IsGreater(tTime.low24, 0x295959))
        {
            result = MMP_TRUE;
        }
    }

    return result;
}

//=============================================================================
//                              Private Function Definition
//=============================================================================

//=============================================================================
/**
 * Converts from BCD to a byte value.
 *
 * @param bcd   The input BCD value.
 * @return The byte value converted from the BCD.
 */
//=============================================================================
MMP_INLINE MMP_INT8
_Bcd2Dec(
     MMP_UINT8 bcd)
{
    return (MMP_UINT8)((bcd >> 4) * 10 + (bcd & 0xF));
}

static MMP_INLINE MMP_UINT8
_Dec2Bcd(
     MMP_UINT8 dec)
{
    MMP_UINT tens  = (dec * 6554) >> 16;
    MMP_UINT units = dec - tens * 10;
    return (MMP_UINT8)(tens << 4) + units;
}

static MMP_INLINE MMP_INT8
_BcdHour(
    MMP_UINT32 bcdHMS)
{
    return (MMP_INT8)((bcdHMS & 0xFF0000) >> 0x10);
}

static MMP_INLINE MMP_INT8
_BcdMin(
    MMP_UINT32 bcdHMS)
{
    return (MMP_INT8)((bcdHMS & 0x00FF00) >> 0x8);
}

static MMP_INLINE MMP_INT8
_BcdSec(
    MMP_UINT32 bcdHMS)
{
    return (MMP_INT8)((bcdHMS & 0x0000FF) >> 0x0);
}


/*
 * Copyright (c) 2004 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * The string functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef PAL_STRING_H
#define PAL_STRING_H

#include "async_file/def.h"

#ifdef __cplusplus
extern "C" {
#endif

void *
PalMemcpy(
    void       *s1,
    const void *s2,
    MMP_SIZE_T n);

void *
PalMemset(
    void       *s,
    MMP_INT    c,
    MMP_SIZE_T n);

MMP_INT
PalMemcmp(
    const void *s1,
    const void *s2,
    MMP_SIZE_T n);

MMP_WCHAR *
PalWcscat(
    MMP_WCHAR       *s1,
    const MMP_WCHAR *s2);

MMP_WCHAR *
PalWcscpy(
    MMP_WCHAR       *s1,
    const MMP_WCHAR *s2);

MMP_INT
PalWcscmp(
    const MMP_WCHAR *s1,
    const MMP_WCHAR *s2);

MMP_SIZE_T
PalWcslen(
    const MMP_WCHAR *s);

MMP_INT
PalWcmpwname(
    const MMP_WCHAR *wname,
    const MMP_WCHAR *name);

MMP_WCHAR *
PalWcsstr(
    const MMP_WCHAR *str,
    const MMP_WCHAR *strSearch);

/***
 * int PalWcsicmp(dst, src) - compare wide-character strings, ignore case
 *
 * Purpose:
 *       PalWcsicmp perform a case-insensitive MMP_WCHAR string comparision.
 *       PalWcsicmp is independent of locale.
 *
 * Entry:
 *       MMP_WCHAR *string1, *src - strings to compare
 *
 * Return:
 *       Returns <0 if string1 < string2
 *       Returns 0  if string1 = string2
 *       Returns >0 if string1 > string2
 *       Returns _NLSCMPERROR is something went wrong
 *
 *******************************************************************************/
MMP_INT
PalWcsicmp(
    const MMP_WCHAR *string1,
    const MMP_WCHAR *string2);

/***
 * MMP_WCHAR *_wcsdup(string) - duplicate string into malloc'd memory
 *
 * Purpose:
 *       Allocates enough storage via malloc() for a copy of the
 *       string, copies the string into the new memory, and returns
 *       a pointer to it (wide-character).
 *
 * Entry:
 *       MMP_WCHAR *string - string to copy into new memory
 *
 * Exit:
 *       returns a pointer to the newly allocated storage with the
 *       string in it.
 *
 *       returns NULL if enough memory could not be allocated, or
 *       string was NULL.
 *
 * Uses:
 *
 * Exceptions:
 *
 *******************************************************************************/
MMP_WCHAR *
PalWcsdup(
    const MMP_WCHAR *string);

MMP_WCHAR *
PalWcsrchr(
    const MMP_WCHAR *str,
    MMP_WCHAR       c);

#ifdef __cplusplus
}
#endif

#endif /* PAL_STRING_H */
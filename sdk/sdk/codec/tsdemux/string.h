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

#include "def.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Copies bytes between buffers.
 *
 * @param s1 New buffer.
 * @param s2 Buffer to copy from.
 * @param n Number of bytes to copy.
 * @return The value of s1.
 */
void*
PalMemcpy(
    void* s1,
    const void* s2,
    MMP_SIZE_T n);

/**
 * Sets buffers to a specified character.
 *
 * @param s Pointer to destination.
 * @param c Character to set.
 * @param n Number of characters.
 * @return The value of s.
 */
void*
PalMemset(
    void* s,
    MMP_INT c,
    MMP_SIZE_T n);

/**
 * Compares characters in two buffers.
 *
 * @param s1 First buffer.
 * @param s2 Second buffer.
 * @param n Number of characters.
 * @return An integer greater than, equal to, or less than zero, according
 * as the object pointed to by s1 is greater than, equal to, or less than the
 * object pointed to by s2.
 */
MMP_INT
PalMemcmp(
    const void* s1,
    const void* s2,
    MMP_SIZE_T n);

/**
 * Copy a string.
 *
 * @param s1 Destination string.
 * @param s2 Null-terminated source string.
 * @return The destination string.
 */
MMP_CHAR*
PalStrcpy(
    MMP_CHAR* s1,
    const MMP_CHAR* s2);

/**
 * Copy characters of one string to another.
 *
 * @param s1 Destination string.
 * @param s2 Source string.
 * @param n Number of characters to be copied.
 * @return The destination string.
 */
MMP_CHAR*
PalStrncpy(
    MMP_CHAR* s1,
    const MMP_CHAR* s2,
    MMP_SIZE_T n);

/**
 * Compare strings.
 *
 * @param s1 Null-terminated strings to compare.
 * @param s2 Null-terminated strings to compare.
 * @return An integer greater than, equal to, or less than zero, according
 *  as the string pointed to by s1 is greater than, equal to, or
 *  less than the string pointed to by s2.
 */
MMP_INT
PalStrcmp(
    const MMP_CHAR* s1,
    const MMP_CHAR* s2);

/**
 * Compare characters of two strings.
 *
 * @param s1 Null-terminated strings to compare.
 * @param s2 Null-terminated strings to compare.
 * @param n Number of characters to compare.
 * @return An integer greater than, equal to, or less than zero, according
 *  as the string pointed to by s1 is greater than, equal to, or
 *  less than the string pointed to by s2.
 */
MMP_INT
PalStrncmp(
    const MMP_CHAR* s1,
    const MMP_CHAR* s2,
    MMP_SIZE_T n);

/**
 * Get the length of a string.
 *
 * @param s Null-terminated string.
 * @return The number of characters in string.
 */
MMP_SIZE_T
PalStrlen(
    const MMP_CHAR* s);

/**
 * Append a string.
 *
 * @param s1 Null-terminated destination string.
 * @param s2 Null-terminated source string.
 * @return The number of characters in string.
 */
MMP_CHAR*
PalStrcat(
    MMP_CHAR* s1,
    const MMP_CHAR* s2);

/**
 * Find a substring.
 *
 * @param s1 Null-terminated string to search.
 * @param s2 Null-terminated string to search for.
 * @return A pointer to the located string, or a null pointer if the string
 *  is not found.
 */
MMP_CHAR*
PalStrstr(
    const MMP_CHAR* s1,
    const MMP_CHAR* s2);

#ifdef __cplusplus
}
#endif

#endif /* PAL_STRING_H */

/*
 * Copyright (c) 2013 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * The platform adaptation layer functions.
 *
 * @version 0.1
 */
#ifndef PAL_PAL_H
#define PAL_PAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ssp/mmp_spi.h"
#include "ssp/ssp_reg.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

/**
 * Boolean quantity.
 */
typedef enum MMP_BOOL_TAG {

    /** Indicates a condition to be untrue */
    MMP_FALSE,

    /** Indicates a condition to be true */
    MMP_TRUE
} MMP_BOOL;

/**
 * Result codes.
 */
typedef enum MMP_RESULT_TAG {

    /** No errors occurred */
    MMP_RESULT_SUCCESS = 0,

    /** Unknown error */
    MMP_RESULT_ERROR = 1,

    /** Time out */
    MMP_RESULT_TIMEOUT = 2,

    /** Invalid arguments for register access */
    MMP_RESULT_INVALID_ARGUMENTS = 3,

    /** CIRTICAL SECTION ERROR */
    MMP_RESULT_ERROR_CRITICAL_SECTION = 4,

    /** LOAD FILE TO CASTOR ERROR */
    MMP_RESULT_LOAD_FILE_ERROR_FILE_SIZE =5,
    MMP_RESULT_LOAD_FILE_ERROR_CHECK_SUM =6,
    MMP_RESULT_LOAD_FILE_ERROR_VERSION =7,

    /** NOT SUPPORTED FUNCTION */
    MMP_RESULT_NOT_SUPPORTED,
    MMP_RESULT_NO_ENOUGH_MEMORY,
    ///////////////////////////////////////

    /** Base for Bus Error */
    MMP_RESULT_BUS_ERROR = 0x70000000,

    MMP_RESULT_ERROR_MAX = 0x7FFFFFFF
} MMP_RESULT;

/**
 * Queue error codes.
 */
//typedef enum QUEUE_ERROR_CODE_TAG
//{
//    QUEUE_INVALID_INPUT = -5,
//    QUEUE_IS_EMPTY      = -4,
//    QUEUE_IS_FULL       = -3,
//    QUEUE_EXIST         = -2,
//    QUEUE_NOT_EXIST     = -1,
//    QUEUE_NO_ERROR      = 0
//} QUEUE_ERROR_CODE;

//=============================================================================
//                              Macro Definition
//=============================================================================

/**
 * 8-bit signed quantity
 */
typedef char MMP_INT8;

/**
 * 8-bit unsigned quantity
 */
typedef unsigned char MMP_UINT8;

/**
 * 16-bit signed quantity
 */
typedef signed short MMP_INT16;

/**
 * 16-bit unsigned quantity
 */
typedef unsigned short MMP_UINT16;

/**
 * 32-bit signed quantity
 */
typedef signed long MMP_INT32;

/**
 * 32-bit unsigned quantity
 */
typedef unsigned long MMP_UINT32;

/**
 * Signed integer type
 */
typedef signed int MMP_INT;

/**
 * Unsigned integer type
 */
typedef unsigned int MMP_UINT;

/**
 * Signed long type
 */
typedef signed long MMP_LONG;

/**
 * Unsigned long type
 */
typedef unsigned long MMP_ULONG;

/**
 * signed double type
 */
typedef double MMP_DOUBLE;

/**
 *  Unicode definition
 */
#define MMP_WCHAR       MMP_UINT16

/**
 * < 4-byte format:0000 0000 0000 0000 RRRR RGGG GGGB BBBB. 
 */
typedef MMP_UINT32 MMP_M2D_COLOR;

/** New definitions in V2 */
#define MMP_SUCCESS         MMP_RESULT_SUCCESS
#define MMP_CHAR            MMP_INT8
#define MMP_NULL            0
#define MMP_INLINE          __inline

typedef unsigned int MMP_SIZE_T;
typedef MMP_UINT32   MMP_HANDLE;

//=============================================================================
//                              Function Declaration
//=============================================================================

///////////////////////////////////////////////////////////////////////////////
// MEMORY
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
/**
 * Copies characters between buffers.
 *
 * @param dest  New buffer
 * @param src   Buffer to copy from
 * @param count Number of characters to copy
 * @return none.
 */
//=============================================================================
void*
PalMemcpy(
    void* dest,
    const void* src,
    MMP_SIZE_T count);

//=============================================================================
/**
 * Allocates memory blocks.
 *
 * @param size  Bytes to allocate
 * @return a void pointer to the allocated space, or NULL if there is
 *         insufficient memory available.
 */
//=============================================================================
void*
PalMalloc(
    MMP_SIZE_T size);

//=============================================================================
/**
 * Deallocates or frees a memory block.
 *
 * @param ptr   Previously allocated memory block to be freed
 * @return none.
 */
//=============================================================================
void
PalFree(
    void* ptr);

//=============================================================================
/**
 * Get string length.
 *
 * @param ptr   string address
 * @return length.
 */
//=============================================================================
MMP_SIZE_T
PalWcslen(
    const MMP_WCHAR* s);

//=============================================================================
/**
 * Reset memory
 *
 * 
 * 
 */
//=============================================================================
void*
PalMemset(
    void* s,
    MMP_INT c,
    MMP_SIZE_T n);

//=============================================================================
/**
 * delay in microseconds.
 *
 * @return none. 
 */
//=============================================================================	
void
PalSleep(
    MMP_UINT32 us);
  
#ifdef __cplusplus
}
#endif

#endif /* PAL_PAL_H */

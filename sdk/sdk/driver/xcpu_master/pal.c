/*
 * Copyright (c) 2013 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * The platform adaptation layer functions.
 *
 * @version 0.1
 */
#include "pal.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Public Function Definition
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
    MMP_SIZE_T count)
{
    return memcpy(dest, src, count);
}

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
    MMP_SIZE_T size)
{
    void* mem;

    assert(size > 0);

    mem = malloc(size);
    assert(mem);

    return mem;
}

//=============================================================================
/**
 * Deallocates or frees a memory block.
 *
 * @param ptr	Previously allocated memory block to be freed
 * @return none. 
 */
//=============================================================================	
void
PalFree(
    void* ptr)
{
    free(ptr);
}

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
    const MMP_WCHAR* s)
{
    return (MMP_SIZE_T)wcslen((wchar_t*)s);
}

void*
PalMemset(
    void* s,
    MMP_INT c,
    MMP_SIZE_T n)
{
    return memset(s, c, n);
}

//=============================================================================
/**
 * delay in microseconds.
 *
 * @return none. 
 */
//=============================================================================	
void
PalSleep(
    MMP_UINT32 us)
{
   /*
     *  ToDo:  Add code here
     *
     *  //Pseudo code
     *  delay(us);
     */
   usleep(us);  
}


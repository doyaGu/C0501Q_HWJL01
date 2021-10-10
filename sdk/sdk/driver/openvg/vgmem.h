#ifndef __VGMEM_H_C4EDAFEF_6B59_4245_8CE2_F1A37BA59285__
#define __VGMEM_H_C4EDAFEF_6B59_4245_8CE2_F1A37BA59285__

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
#define VG_MEM_RESERVE_SIZE		(4 * 1024 * 1024) // bytes
#define VG_MEM_INVALID_OBJID 	0xFFFFFFFF

//=============================================================================
//                              Function Declaration
//=============================================================================
void
vgMemInit(
	unsigned long startAddr,
	unsigned long size);

void*
vgMalloc(
	unsigned long size,
	unsigned long objID);

void*
vgMemalign(
	unsigned long byteAlign,
	unsigned long size,
	unsigned long objID);

void
vgFree(
   void* addr);

#ifdef __cplusplus
}
#endif

#endif // __VGMEM_H_C4EDAFEF_6B59_4245_8CE2_F1A37BA59285__

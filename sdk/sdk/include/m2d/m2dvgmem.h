#ifndef __VGMEM_H_C4EDAFEF_6B59_4245_8CE2_F1A37BA59285__
#define __VGMEM_H_C4EDAFEF_6B59_4245_8CE2_F1A37BA59285__

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================
#define m2dvgMalloc(size, objID)                itpVmemAlloc(size)
#define m2dvgMemalign(byteAlign, size, objID)   itpVmemAlignedAlloc(byteAlign, size)
#define m2dvgFree(addr)                         do { if (addr) itpVmemFree(addr); } while(0)

#ifdef __cplusplus
}
#endif

#endif // __VGMEM_H_C4EDAFEF_6B59_4245_8CE2_F1A37BA59285__

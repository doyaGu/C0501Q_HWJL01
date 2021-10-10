#ifndef __XD_BINARY_HEAP_H__
#define __XD_BINARY_HEAP_H__

#include "xd_type.h"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////
//
//	Constant variable or Marco
//
////////////////////////////////////////////////////////////
#define ElementType int

////////////////////////////////////////////////////////////
//
//	Variable Declaration
//
////////////////////////////////////////////////////////////
typedef struct _XD_BINARYHEAP
{
	MMP_UINT32 Capacity;
	MMP_UINT32 Size;
	ElementType* pElement;
}XD_BINARYHEAP;

////////////////////////////////////////////////////////////
//
// Function prototype
//
////////////////////////////////////////////////////////////
XD_BINARYHEAP* XD_CreateHeap(MMP_UINT32 Size);
MMP_BOOL XD_DeleteHeap(XD_BINARYHEAP* pXd_heap);
MMP_BOOL XD_HeapInsertElement(XD_BINARYHEAP* pXd_heap, ElementType element);
MMP_BOOL XD_HeapPopMinElement(XD_BINARYHEAP* pXd_heap, ElementType* pElement);
MMP_BOOL XD_HeapFindMinElement(XD_BINARYHEAP* pXd_heap, ElementType* pElement);
MMP_BOOL XD_HeapIsEmpty(XD_BINARYHEAP* pXd_heap);
MMP_BOOL XD_HeapIsFull(XD_BINARYHEAP* pXd_heap);
void     XD_HeapMakeEmpty(XD_BINARYHEAP* pXd_heap);

#ifdef __cplusplus
}
#endif


#endif

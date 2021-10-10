#include "xd/xd_binaryheap.h"
#include <stdlib.h>

////////////////////////////////////////////////////////////
//
//	Constant variable or Marco
//
////////////////////////////////////////////////////////////
#define XD_MINVALUE -1

////////////////////////////////////////////////////////////
//
//	Variable Declaration
//
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
//
// Function Implement
//
////////////////////////////////////////////////////////////
XD_BINARYHEAP* XD_CreateHeap(MMP_UINT32 Size)
{
	XD_BINARYHEAP* pNewHeap = (XD_BINARYHEAP*)malloc(sizeof(XD_BINARYHEAP));
	if ( pNewHeap == MMP_NULL )
		return MMP_NULL;

	pNewHeap->pElement = (ElementType*)malloc((Size + 1) * sizeof(ElementType));
	if ( pNewHeap->pElement == MMP_NULL )
	{
		free(pNewHeap);
		pNewHeap = MMP_NULL;
		return MMP_NULL;
	}

	pNewHeap->Capacity    = Size;
	pNewHeap->Size        = 0;
	pNewHeap->pElement[0] = XD_MINVALUE;

	return pNewHeap;
}

MMP_BOOL XD_DeleteHeap(XD_BINARYHEAP* pXd_heap)
{
	if ( pXd_heap == MMP_NULL )
		return MMP_FALSE;
	else
	{
		if ( pXd_heap->pElement != MMP_NULL )
		{
			free(pXd_heap->pElement);
			pXd_heap->pElement = MMP_NULL;
		}
		free(pXd_heap);
		pXd_heap = MMP_NULL;

		return MMP_TRUE;
	}
}

MMP_BOOL XD_HeapInsertElement(XD_BINARYHEAP* pXd_heap, ElementType element)
{
	int i = 0;

	if (   pXd_heap == MMP_NULL 
		|| XD_HeapIsFull(pXd_heap)
		|| element <= XD_MINVALUE)
		return MMP_FALSE;

	for ( i=(++pXd_heap->Size); pXd_heap->pElement[i/2]>element; i/=2 )
	{
		pXd_heap->pElement[i] = pXd_heap->pElement[i/2];
	}
	pXd_heap->pElement[i] = element;

	return MMP_TRUE;
}

MMP_BOOL XD_HeapPopMinElement(XD_BINARYHEAP* pXd_heap, ElementType* pElement)
{
	MMP_UINT32 i, child;
	ElementType minElement, lastElement;

	if (   pXd_heap == MMP_NULL 
		|| XD_HeapIsEmpty(pXd_heap))
		return MMP_FALSE;

	minElement  = pXd_heap->pElement[1];
	lastElement = pXd_heap->pElement[pXd_heap->Size--];

	for ( i=1; (i*2) <= pXd_heap->Size; i = child )
	{
		child = i * 2;
		if (   child != pXd_heap->Size
			&& pXd_heap->pElement[child + 1] < pXd_heap->pElement[child])
		{
			child++;
		}

		if ( lastElement > pXd_heap->pElement[child])
		{
			pXd_heap->pElement[i] = pXd_heap->pElement[child];
		}
		else
		{
			break;
		}
	}
	pXd_heap->pElement[i] = lastElement;

	*pElement = minElement;

	return MMP_TRUE;
}

MMP_BOOL XD_HeapFindMinElement(XD_BINARYHEAP* pXd_heap, ElementType* pElement);

MMP_BOOL XD_HeapIsEmpty(XD_BINARYHEAP* pXd_heap)
{
	return pXd_heap->Size == 0 ? MMP_TRUE : MMP_FALSE;
}

MMP_BOOL XD_HeapIsFull(XD_BINARYHEAP* pXd_heap)
{
	return pXd_heap->Size == pXd_heap->Capacity ? MMP_TRUE : MMP_FALSE;
}

void XD_HeapMakeEmpty(XD_BINARYHEAP* pXd_heap);

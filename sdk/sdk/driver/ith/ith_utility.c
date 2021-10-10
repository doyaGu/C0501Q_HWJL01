/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL utility functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"
#include <string.h>

void ithReadVram(void* dest, uint32_t src, uint32_t size)
{
    void* ptr = ithMapVram(src, size, ITH_VRAM_READ);
    memcpy(dest, ptr, size);
    ithUnmapVram(ptr, size);
}

void ithWriteVram(uint32_t dest, const void* src, uint32_t size)
{
    void* ptr = ithMapVram(dest, size, ITH_VRAM_WRITE);
    memcpy(ptr, src, size);
    ithUnmapVram(ptr, size);
}

void ithCopyVram(uint32_t dest, uint32_t src, uint32_t size)
{
    void* destPtr   = ithMapVram(dest, size, ITH_VRAM_WRITE);
    void* srcPtr    = ithMapVram(src, size, ITH_VRAM_READ);

    memcpy(destPtr, srcPtr, size);

    ithUnmapVram(srcPtr, size);
    ithUnmapVram(destPtr, size);
}

void ithSetVram(uint32_t dest, int c, uint32_t size)
{
    void* ptr = ithMapVram(dest, size, ITH_VRAM_WRITE);
    memset(ptr, c, size);
    ithUnmapVram(ptr, size);
}

void ithListPushFront(ITHList* list, void* node)
{
    ITHList* listNode = (ITHList*) node;

    if (list->next == NULL)
    {
        /* This is the first item we add */
        list->prev      = listNode;
        listNode->next  = NULL;
    }
    else
    {
        /* We already have items in the list */
        listNode->next         = list->next;
        listNode->next->prev   = listNode;
    }
    list->next     = listNode;
    listNode->prev = list;
}

void ithListPushBack(ITHList* list, void* node)
{
    ITHList* listNode = (ITHList*) node;

    if (list->prev == NULL)
    {
        /* This is the first item we add */
        list->next      = listNode;
        listNode->prev  = list;
    }
    else
    {
        /* We already have items in the list */
        listNode->prev         = list->prev;
        listNode->prev->next   = listNode;
    }
    list->prev      = listNode;
    listNode->next  = NULL;
}

void ithListInsertBefore(ITHList* list, void* listNode, void* node)
{
    ITHList* theListNode  = (ITHList*) listNode;
    ITHList* the_node     = (ITHList*) node;

    the_node->prev          = theListNode->prev;
    the_node->next          = theListNode;
    theListNode->prev->next = the_node;
    theListNode->prev       = the_node;
}

void
ithListInsertAfter(ITHList* list, void* listNode, void* node)
{
    ITHList* theListNode  = (ITHList*) listNode;
    ITHList* the_node     = (ITHList*) node;

    the_node->next          = theListNode->next;
    the_node->prev          = theListNode;

    if (theListNode->next)
    {
        theListNode->next->prev = the_node;
    }
    else
    {
        list->prev = the_node;
    }
    theListNode->next = the_node;
}

void ithListRemove(ITHList* list, void* node)
{
    ITHList* listNode = (ITHList*) node;

    if (list->prev == listNode)
        list->prev = (listNode->prev == list) ? NULL : listNode->prev;

	if (listNode->next != NULL)
		listNode->next->prev = listNode->prev;

	listNode->prev->next = listNode->next;
}

void ithListClear(ITHList* list, void (*dtor)(void*))
{
	ITHList *p, *q;

	if (dtor != NULL)
    {
        q = list->next;
		while (q)
		{
			p = q;
			q = q->next;
			dtor(p);
		}
	}
    list->next = list->prev = NULL;
}

char* ithUltob(char* s, unsigned long i)
{
    char* cp        = s;
    unsigned long bitMask   = ~0ul - (~0ul >> 1);

    do
    {
        *cp++ = (i & bitMask) ? '1' : '0';
    } while (bitMask >>= 1);

    *cp = '\0';
    return s;
}

void ithPrintRegH(uint16_t addr, unsigned int size)
{
    uint16_t reg, p;
    unsigned int i, j, count;

    if (size == 0)
        return;

    reg     = ITH_ALIGN_DOWN(addr, sizeof (uint16_t));
    count   = size / sizeof (uint16_t);

    j = 0;
    p = reg;
    for (i = 0; i < count; ++i)
    {
        uint16_t value = ithReadRegH(p);

        if (j == 0)
            PRINTF("0x%04X:", p);

        PRINTF(" %04X", value);

        if (j >= 7)
        {
            PRINTF("\r\n");
            j = 0;
        }
        else
            j++;

        p += sizeof (uint16_t);
    }

    if (j > 0)
        PRINTF("\r\n");
}

void ithPrintRegA(uint32_t addr, unsigned int size)
{
    uint32_t reg, p;
    unsigned int i, j, count;

    if (size == 0)
        return;

    reg     = ITH_ALIGN_DOWN(addr, sizeof (uint32_t));
    count   = size / sizeof (uint32_t);

    j = 0;
    p = reg;
    for (i = 0; i < count; ++i)
    {
        uint32_t value = ithReadRegA(p);

        if (j == 0)
            PRINTF("0x%08X:", p);

        PRINTF(" %08X", value);

        if (j >= 3)
        {
            PRINTF("\r\n");
            j = 0;
        }
        else
            j++;

        p += sizeof (uint32_t);
    }

    if (j > 0)
        PRINTF("\r\n");
}

void ithPrintVram8(uint32_t addr, unsigned int size)
{
    uint8_t *base, *mem;
    unsigned int i, j;

    if (size == 0)
        return;

    base = (uint8_t*)ithMapVram(addr, size, ITH_VRAM_READ);
    mem = base;

    j = 0;
    for (i = 0; i < size; ++i)
    {
        if (j == 0)
            PRINTF("0x%08X:", addr);

        PRINTF(" %02X", *mem);

        if (j >= 15)
        {
            PRINTF("\r\n");
            j = 0;
        }
        else
            j++;

        addr++;
        mem++;
    }

	ithUnmapVram(base, size);

    if (j > 0)
        PRINTF("\r\n");
}

void ithPrintVram16(uint32_t addr, unsigned int size)
{
    uint16_t *base, *mem;
    unsigned int i, j, count;

    if (size == 0)
        return;

    base	= (uint16_t*)ithMapVram(addr, size, ITH_VRAM_READ);
    mem		= base;
    count   = size / sizeof (uint16_t);

    j = 0;
    for (i = 0; i < count; ++i)
    {
        if (j == 0)
            PRINTF("0x%08X:", addr);

        PRINTF(" %04X", *mem);

        if (j >= 7)
        {
            PRINTF("\r\n");
            j = 0;
        }
        else
            j++;

        addr += sizeof (uint16_t);
        mem++;
    }

	ithUnmapVram(base, size);

    if (j > 0)
        PRINTF("\r\n");
}

void ithPrintVram32(uint32_t addr, unsigned int size)
{
    uint32_t *base, *mem;
    unsigned int i, j, count;

    if (size == 0)
        return;

    base	= (uint32_t*)ithMapVram(addr, size, ITH_VRAM_READ);
    mem		= base;
    count   = size / sizeof (uint32_t);

    j = 0;
    for (i = 0; i < count; ++i)
    {
        if (j == 0)
            PRINTF("0x%08X:", addr);

        PRINTF(" %08X", *mem);

        if (j >= 3)
        {
            PRINTF("\r\n");
            j = 0;
        }
        else
            j++;

        addr += sizeof (uint32_t);
        mem++;
    }

	ithUnmapVram(base, size);

    if (j > 0)
        PRINTF("\r\n");
}

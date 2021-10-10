#include "async_file/config.h"
#include "async_file/pal.h"
#include <stdlib.h>

extern void* __heap_start__;
extern unsigned int __heap_end__;

void*
PalHeapAlloc(
    MMP_INT name,
    MMP_SIZE_T size)
{
    void* mem;

    //PalAssert(size > 0);
    //if ((mem = (void*) MEM_DeployEdge(size)) == 0)
        mem = malloc(size);

    //if (0 == mem)
    //{
        //PalPrintf("memory allocation is fail - size: %u bytes, remain size: %u bytes\n",
        //          size,
        //          (MMP_UINT32) ((void*)(&__heap_end__) - __heap_start__));

        // [09282010] vincent: remove to avoid system hang in normal case,
        // for example, load a bigger image carried in mp3 stream
        //while(1);
    //}

    return mem;
}

void
PalHeapFree(
    MMP_INT name,
    void* ptr)
{
    if (ptr != MMP_NULL)
    {
        //if (MEM_ReleaseEdge(ptr) == MMP_FALSE)
            free(ptr);
        ptr = MMP_NULL;
    }
}

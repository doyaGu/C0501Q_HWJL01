#include "ite/itu.h"
#include "itu_cfg.h"

void ituStringSetLoad(ITUStringSet* stringSet, uint32_t base)
{
    int i;

    for (i = 0; i < stringSet->count; i++)
    {
        stringSet->strings[i] = (char*)(base + (uint32_t)stringSet->strings[i]);
    }
}

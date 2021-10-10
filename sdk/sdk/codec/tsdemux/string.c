// #include "config.h"
#include "def.h"
#include <string.h>

void*
PalMemcpy(
    void* s1,
    const void* s2,
    MMP_SIZE_T n)
{
    return memcpy(s1, s2, n);
}

void*
PalMemset(
    void* s,
    MMP_INT c,
    MMP_SIZE_T n)
{
    return memset(s, c, n);
}

int
PalMemcmp(
    const void* s1,
    const void* s2,
    MMP_SIZE_T n)
{
    return memcmp(s1, s2, n);
}



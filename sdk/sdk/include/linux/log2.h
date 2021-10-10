#ifndef _LOG2_H_
#define _LOG2_H_


static int ilog2(uint32_t v)
{
    int l = 0;
    while ((1UL << l) < v)
        l++;
    return l;
}


#endif // _LOG2_H_
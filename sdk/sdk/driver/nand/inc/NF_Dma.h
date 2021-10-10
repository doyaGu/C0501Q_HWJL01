#ifndef NF_DMA_H
#define NF_DMA_H

#include "ite/mmp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

MMP_BOOL NF_InitDma();
MMP_BOOL NF_DmaCopy(void* dest, void* src, MMP_UINT32 length);
MMP_BOOL NF_TerminateDma();


#ifdef __cplusplus
}
#endif

#endif

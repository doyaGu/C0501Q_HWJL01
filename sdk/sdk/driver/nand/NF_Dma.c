#include "mmp_dma.h"
#include "nandflash/nf_dma.h"

//**************************************************************************
//  Global Variable
//**************************************************************************
static MMP_DMA_CONTEXT g_NfDmaContext = MMP_NULL;
static MMP_UINT32 g_DmaAttrib[] =
{
    MMP_DMA_ATTRIB_DMA_TYPE, (MMP_UINT32)MMP_DMA_TYPE_MEM_TO_MEM,
    MMP_DMA_ATTRIB_SRC_ADDR, (MMP_UINT32)0, // src
    MMP_DMA_ATTRIB_DST_ADDR, (MMP_UINT32)0, // dest
    MMP_DMA_ATTRIB_TOTAL_SIZE_IN_BYTE, (MMP_UINT32)0,  // size
    MMP_DMA_ATTRIB_NONE
};

//**************************************************************************
//  Function Implamentation
//**************************************************************************
MMP_BOOL NF_InitDma()
{
    MMP_BOOL initResult = MMP_TRUE;
    
    //printf("NF_InitDma: \n");
    
    mmpDmaInitialize();
    
    if ( g_NfDmaContext == MMP_NULL )
    {
        if ( mmpDmaCreateContext(&g_NfDmaContext) )
            initResult = MMP_FALSE;
    }
    
    return initResult;
}

MMP_BOOL NF_DmaCopy(void* dest, void* src, MMP_UINT32 length)
{
    //printf("NF_DmaCopy: \n");
    
    g_DmaAttrib[3] = (MMP_UINT32)src;
    g_DmaAttrib[5] = (MMP_UINT32)dest;
    g_DmaAttrib[7] = length;
    
    mmpDmaSetAttrib(g_NfDmaContext, g_DmaAttrib);
    mmpDmaFire(g_NfDmaContext);
    
    if ( !mmpDmaWaitIdle(g_NfDmaContext) )
        return MMP_TRUE;
    else
        return MMP_FALSE;
}

MMP_BOOL NF_TerminateDma()
{
    //printf("NF_TerminateDma: \n");
    
    mmpDmaDestroyContext(g_NfDmaContext);
    g_NfDmaContext = MMP_NULL;
}

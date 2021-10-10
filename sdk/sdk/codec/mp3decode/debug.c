#if !defined(WIN32) && !defined(__CYGWIN__)
#error don't include debug.c in non-win32 mode
#endif

#include "mp3_config.h"

#if defined(STAGE_DEBUG)
    int *dbgST_UnpackSF;
    int *dbgST_DecodeHuffman;
    int *dbgST_Dequantize;
    int *dbgST_IMDCT;
    int *dbgST_Synthesis;
#endif

#if defined(ENGINE_DEBUG)
    int dbgENG_cnt;
    int *dbgENG_srcfile;
    int *dbgENG_dstfile0;
    int *dbgENG_dstfile1;
    int *dbgENG_dstfile2;
    int *dbgENG_dstfile3;
#endif

#if defined(OVERFLOW_DEBUG)
    int g_ovflw_indx;
    int g_ovflw_cnt[OVFLW_STAGE];
    double g_ovflw_shift_coef[OVFLW_MAXSHIFT];
#endif

#if defined(LAYERII_DEBUG)
    FILE *dbgLayerII;
#endif

void initDbg()
{
#if defined(OVERFLOW_DEBUG)
    int i;
    g_ovflw_indx = 0;
    for(i=0; i<OVFLW_STAGE; i++){
        g_ovflw_cnt[i]=0;
    }
    for(i=0; i<OVFLW_MAXSHIFT; i++){
        g_ovflw_shift_coef[i] = 1.0/pow(2.0, i);
    }
#endif

#if defined(STAGE_DEBUG)
    dbgST_UnpackSF      = fopen("unpacksf.prb", "w");
    dbgST_DecodeHuffman = fopen("decodehuffman.prb", "w");
    dbgST_Dequantize    = fopen("dequantize.prb", "w");
    dbgST_IMDCT         = fopen("imdct.prb", "w");
    dbgST_Synthesis     = fopen("synthesis.prb", "w");
#endif

#if defined(ENGINE_DEBUG)
    dbgENG_cnt    = 0;
    dbgENG_srcfile  = fopen("dma_input.txt", "w");
    dbgENG_dstfile0 = fopen("dma_wrfifo0.txt","w");
    dbgENG_dstfile1 = fopen("dma_wrfifo1.txt","w");
    dbgENG_dstfile2 = fopen("dma_wrfifo2.txt","w");
    dbgENG_dstfile3 = fopen("dma_wrfifo3.txt","w");
#endif

#if defined(LAYERII_DEBUG)
    dbgLayerII = fopen("layer2.txt","w");
#endif
}

void finishDbg()
{
#if defined(OVERFLOW_DEBUG)
    int i;
    for(i=0; i<OVFLW_STAGE; i++){
        printf("stage%d overflow count=%d\n", i, g_ovflw_cnt[i]);
    }
#endif

#if defined(STAGE_DEBUG)
    fclose(dbgST_UnpackSF     );
    fclose(dbgST_DecodeHuffman);
    fclose(dbgST_Dequantize   );
    fclose(dbgST_IMDCT        );
    fclose(dbgST_Synthesis    );
#endif

#if defined(ENGINE_DEBUG)
    fclose(dbgENG_srcfile );
    fclose(dbgENG_dstfile0);
    fclose(dbgENG_dstfile1);
    fclose(dbgENG_dstfile2);
    fclose(dbgENG_dstfile3);
#endif

#if defined(LAYERII_DEBUG)
    fclose(dbgLayerII);
#endif

}


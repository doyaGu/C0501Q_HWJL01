

#include "simple_draw.h"

//=============================================================================
//                  Constant Definition
//=============================================================================


//=============================================================================
//                  Macro Definition
//=============================================================================


//=============================================================================
//                  Structure Definition
//=============================================================================


//=============================================================================
//                  Global Data Definition
//=============================================================================


//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
Draw_Rect(
    uint8_t     *baseAddr,
    uint32_t    dstPitch,
    BASE_RECT   *rect,
    uint32_t    color)
{
    unsigned char   Bpp = 2; // byte per pixel
    unsigned int    offset = 0;
    unsigned char   *tmpBuf = 0;
    unsigned char   *dmaWriteBuf = NULL;
    unsigned char   *tmpOri = NULL , *tmpOri2=NULL;
    int             row = 0, col = 0;
    int             x, y;

    row = rect->h;
    col = rect->w;

#if 1
    {
        uint16_t* base = ithMapVram((uint32_t) baseAddr, ithLcdGetPitch() * ithLcdGetHeight(), ITH_VRAM_WRITE);
        uint16_t* ptr = base;

        for (y = 0; y < row; y++)
            for (x = 0; x < col; x++)
                *ptr++ = color;

         ithFlushDCacheRange(base, row *col*2);
         ithFlushMemBuffer();
    }
#else
    tmpBuf = malloc((rect->w << Bpp));
    tmpOri  = tmpBuf;
	dmaWriteBuf = (uint8_t *)itpVmemAlloc((rect->w << Bpp));
    tmpOri2 = dmaWriteBuf;

    if( tmpBuf )
    {
        unsigned short   *ptr = (unsigned short*)tmpBuf;
        unsigned char    *ptCur = 0;
        uint8_t* mappedSysRam = NULL;

        printf("color = %d\n",color);
        for (x = 0; x < col; x++)
            *ptr++ = (unsigned short)color;

        offset = rect->y * dstPitch + rect->x * Bpp;
        ptCur = baseAddr + offset;

        printf("run Draw_Rect\n");
        for(y = 0; y < rect->h; y++)
        {
           // _Vram_WriteBlkMem((unsigned int)ptCur, (unsigned int)tmpBuf, rect->w*Bpp);
//
            mappedSysRam = ithMapVram((uint32_t)dmaWriteBuf, (rect->w << Bpp), ITH_VRAM_WRITE);
            memcpy(mappedSysRam, tmpBuf, (rect->w << Bpp));
            ithFlushDCacheRange(mappedSysRam, (rect->w << Bpp));
            ithUnmapVram(mappedSysRam,  (rect->w << Bpp));
            ithFlushMemBuffer();
//
           dmaWriteBuf +=(rect->w << Bpp);
           tmpBuf  +=(rect->w << Bpp);
           //ptCur += (dstPitch);
            
        }
        tmpBuf = tmpOri ;
        dmaWriteBuf  = tmpOri2;
        free(tmpBuf);
    	if (dmaWriteBuf)
    	{
    		itpVmemFree((uint32_t)dmaWriteBuf);
    	}

        ithFlushDCache();
        ithFlushMemBuffer();
    }
#endif

    return 0;
}

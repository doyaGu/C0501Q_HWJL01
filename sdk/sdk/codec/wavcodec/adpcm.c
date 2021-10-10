/*
 * adpcm.c --
 *
 *      Intel/DVI ADPCM coder/decoder.
 */

/***********************************************************
Copyright 1992 by Stichting Mathematisch Centrum, Amsterdam, The
Netherlands.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Stichting Mathematisch
Centrum or CWI not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.

STICHTING MATHEMATISCH CENTRUM DISCLAIMS ALL WARRANTIES WITH REGARD TO
THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS, IN NO EVENT SHALL STICHTING MATHEMATISCH CENTRUM BE LIABLE
FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

/*
** Intel/DVI ADPCM coder/decoder.
**
** The algorithm for this coder was taken from the IMA Compatability Project
** proceedings, Vol 2, Number 2; May 1992.
**
** Version 1.1, 16-Dec-92.
**
** Change log:
** - Fixed a stupid bug, where the delta was computed as
**   stepsize*code/4 in stead of stepsize*(code+0.5)/4. The old behavior can
**   still be gotten by defining STUPID_V1_BUG.
*/

//#include "wave.h"

#include "adpcm.h"
#include "plugin.h"

#define ADPCM_BIT_PERSAMPLE     4

/* Intel ADPCM step variation table */
static int indexTable[16] = {
    -1, -1, -1, -1,     /* +0 - +3, decrease the step size */
     2,  4,  6,  8,     /* +4 - +7, increase the step size */
    -1, -1, -1, -1,     /* -0 - -3, decrease the step size */
     2,  4,  6,  8,     /* -4 - -7, increase the step size */
};

static int stepsizeTable[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};


//static _WaveInfoMode WaveInfo;

#if defined(HAVE_ENCODE)

void adpcm_coder (int len,
	struct adpcm_state *state)
{
    int i;
    unsigned short *inp;               /* Input buffer pointer */
    unsigned char *outp;  /* Output buffer pointer */
    int val;                       /* Current input sample value */
    int sign;                     /* Current adpcm sign bit */
    int delta;                   /* Current adpcm output value */
    int diff;                      /* Difference between val and valprev */
    int step;                    /* Stepsize */
    int valpred;              /* Predicted output value */
    int vpdiff;                 /* Current change to valpred */
    int index;                 /* Current step change index */
    int outputbuffer;     /* place to keep previous 4-bit value */
    int bufferstep;         /* toggle between outputbuffer/output */
    int pcmRdIdx;               /* input buffer read pointer */
    int wavWrIdx;               /* output buffer write pointer */

    int temp;
    int nTemp ;
 
    pcmRdIdx = pcmReadIdx;
    wavWrIdx = wavWriteIdx;
    nTemp = 1;

    temp = wavWriteIdx;
    inp  = (unsigned short*)&pcmWriteBuf[pcmRdIdx];        
    outp = (unsigned char*)&streamBuf[wavWrIdx];

    valpred = state->valprev;
    index = state->index;
    step = stepsizeTable[index];
    
    bufferstep = 1;
//printf(" adpcm enc %d \n",len);
    for ( ; len > 0 ; len-=2 ) 
    {
        val = (short)pcmWriteBuf[pcmRdIdx] ;

        if(pcmRdIdx%2==0)
        {
            pcmRdIdx +=2;
        }
        else
        {
            pcmRdIdx +=1;            
        }
        if (pcmRdIdx >= sizeof(pcmWriteBuf))
        {
            pcmRdIdx = 0;
        }
        

	/* Step 1 - compute difference with previous value */
	diff = val - valpred;
	sign = (diff < 0) ? 8 : 0;
	if ( sign ) diff = (-diff);

	/* Step 2 - Divide and clamp */
	/* Note:
	** This code *approximately* computes:
	**    delta = diff*4/step;
	**    vpdiff = (delta+0.5)*step/4;
	** but in shift step bits are dropped. The net result of this is
	** that even if you have fast mul/div hardware you cannot put it to
	** good use since the fixup would be too expensive.
	*/
	delta = 0;
	vpdiff = (step >> 3);
	
	if ( diff >= step ) 
       {
	    delta = 4;
	    diff -= step;
	    vpdiff += step;
	}
	step >>= 1;
	if ( diff >= step  ) 
       {
	    delta |= 2;
	    diff -= step;
	    vpdiff += step;
	}
	step >>= 1;
	if ( diff >= step ) 
       {
	    delta |= 1;
	    vpdiff += step;
	}

	/* Step 3 - Update previous value */
	if ( sign )
	  valpred -= vpdiff;
	else
	  valpred += vpdiff;

	/* Step 4 - Clamp previous value to 16 bits */
	if ( valpred > 32767 )
	  valpred = 32767;
	else if ( valpred < -32768 )
	  valpred = -32768;

	/* Step 5 - Assemble value, update index and step values */
	delta |= sign;
	
	index += indexTable[delta];
	if ( index < 0 ) index = 0;
	if ( index > 88 ) index = 88;
	step = stepsizeTable[index];

	/* Step 6 - Output value */
	if ( bufferstep ) 
       {
	    outputbuffer = delta & 0x0f;
           if(WaveInfo.nChans ==2)
           {
               outp = (unsigned char*)&streamBuf[wavWrIdx];
               *outp = ((delta << 4) & 0xf0) | outputbuffer;        
               wavWrIdx += 1;        
               //printf(" input %d out put %d %d  %d  \n",val,((delta << 4) & 0xf0),outputbuffer,*outp);
           }
	}
       else
       {
	    //*outp++ = ((delta << 4) & 0xf0) | outputbuffer;
            outp = (unsigned char*)&streamBuf[wavWrIdx];
            *outp = ((delta << 4) & 0xf0) | outputbuffer;        
            wavWrIdx += 1;        
            if (wavWrIdx >= sizeof(streamBuf))
            {
                wavWrIdx = 0;
            }
            
          //  printf(" input %d out put %d %d  %d  \n",val,((delta << 4) & 0xf0),outputbuffer,*outp);
/*
            if( nTemp == 1 )
            {
              //printf(" nTemp ==    \n");
                outp = (unsigned char*)&streamBuf[wavWrIdx];
                *outp = ((delta << 4) & 0xf0) | outputbuffer;       
                wavWrIdx += 1;        
                if (wavWrIdx >= sizeof(streamBuf))
                {
                    wavWrIdx = 0;
                }

                outp = (unsigned char*)&streamBuf[wavWrIdx];
                *outp = ((delta << 4) & 0xf0) | outputbuffer;       
                wavWrIdx += 1;        
                if (wavWrIdx >= sizeof(streamBuf))
                {
                    wavWrIdx = 0;
                }

                outp = (unsigned char*)&streamBuf[wavWrIdx];
                *outp = ((delta << 4) & 0xf0) | outputbuffer;       
                wavWrIdx += 1;        
                if (wavWrIdx >= sizeof(streamBuf))
                {
                    wavWrIdx = 0;
                }

           
            }
            nTemp -=2;
*/
           
	}
       
       if( WaveInfo.nChans ==1)
       {
	    bufferstep = !bufferstep;
       }
       
    }

    /* Output last step, if needed */
    if ( !bufferstep )
    {
      //*outp++ = outputbuffer;
        outp = (unsigned char*)&streamBuf[wavWrIdx];
        *outp = (unsigned char)outputbuffer;     
       
        wavWrIdx += 1;        

        if (wavWrIdx >= sizeof(streamBuf))
            wavWrIdx = 0;

         
    }

//printf(" wav write %d \n", (wavWrIdx > temp)? (wavWrIdx-temp):(sizeof(streamBuf) - (temp-wavWrIdx)));
    
    state->valprev = valpred;
    state->index = index;
}
#endif // defined(HAVE_ENCODE)

void
adpcm_decoder(
    int len,
    struct adpcm_state *state)
{
    int i, n, c;
    int sign;                   /* Current adpcm sign bit */
    int delta;                  /* Current adpcm output value */
    int step[2];                /* Stepsize */
    int valprev[2];             /* virtual previous output value */
    int vpdiff;                 /* Current change to valprev */
    int index[2];               /* Current step change index */
    int inputbuffer = 0;        /* place to keep next 4-bit value */
    int bufferstep;             /* toggle between inputbuffer/input */
    unsigned char  *inp;        /* input buffer pointer */
    unsigned short *outp;       /* output buffer pointer */
    int wavRdIdx;               /* input buffer read pointer */
    int pcmWrIdx;               /* output buffer write pointer */
    int samplecode[8][2];       /* For 4-bit DVI ADPCM each data word
                                   contains eight sample codes */
    int samples;
    int temp;
    pcmWrIdx = pcmWriteIdx;
    wavRdIdx = wavReadIdx;

    temp = pcmWriteIdx;
    if (len <= 4 * WaveInfo.nChans) 
    {
        PRINTF("No enougth ADCPM stream!\n");
        return;
    }

    for(i=0; i<WaveInfo.nChans; i++) 
    {
        outp = (unsigned short*)&pcmWriteBuf[pcmWrIdx];
        valprev[i] = ((int)streamBuf[wavRdIdx] & 0xff); 
        wavRdIdx = ((wavRdIdx+1) == sizeof(streamBuf)) ? 0 : (wavRdIdx + 1);
        valprev[i] += (((int)streamBuf[wavRdIdx] << 8) & 0xff00); 
        wavRdIdx = ((wavRdIdx+1) == sizeof(streamBuf)) ? 0 : (wavRdIdx + 1);
        index[i] = ((int)streamBuf[wavRdIdx] & 0xff);
        wavRdIdx = ((wavRdIdx+2) >= sizeof(streamBuf)) ? ((wavRdIdx+2) - sizeof(streamBuf)) : (wavRdIdx + 2);

        valprev[i] = (valprev[i] & 0x8000) ? valprev[i] - 0x10000 : valprev[i];
        *outp = (unsigned short)valprev[i];
        pcmWrIdx += 2;
        if (pcmWrIdx >= sizeof(pcmWriteBuf))
        {
            pcmWrIdx = 0;
        }

        if (index[i] < 0) 
        {
            index[i] = 0;
        }
        else if (index[i] > 88)
        {
            index[i] = 88;
        }

        step[i] = stepsizeTable[index[i]];
        PRINTF("ADPCM: valprev[%d](%d) index[%d](%d)\n", i, valprev[i], i, index[i]);
    }
    inp  = (unsigned char*)&streamBuf[wavRdIdx];

    // header size per channel is 4 bytes.
    samples = (len - 4 * WaveInfo.nChans) * (8 / ADPCM_BIT_PERSAMPLE);
    //printf(" adpcm dec  len %d samples %d \n",len,samples);
    
    for (i = 0; i < samples / (8 * WaveInfo.nChans); i++) 
    {
        /* Step 1 - get the delta value */
        for (c = 0; c < WaveInfo.nChans; c++) 
        {
            bufferstep = 0;
            for(n = 0; n < 8; n++) 
            {
                if (bufferstep) 
                {
                    samplecode[n][c] = (inputbuffer >> 4) & 0xf;
                }
                else 
                {
                    inputbuffer = *inp;
                    samplecode[n][c] = inputbuffer & 0xf;
                    wavRdIdx ++;
                    if (wavRdIdx >= sizeof(streamBuf)) 
                    {
                        wavRdIdx = 0;
                        #if defined(__OPENRTOS__) && READBUF_SIZE < 8*1024
                        dc_invalidate(); // Flush Data Cache
                        #endif
                    }
                    inp  = (unsigned char*)&streamBuf[wavRdIdx];
                }
                bufferstep = !bufferstep;
            }
        }

        for (n = 0; n < 8; n++) 
        {
            for (c = 0; c < WaveInfo.nChans; c++) 
            {
                /* Step 2 - Find new index value (for later) */
                delta = samplecode[n][c];
                index[c] += indexTable[delta];
                if (index[c] < 0) index[c] = 0;
                else if (index[c] > 88) index[c] = 88;

                /* Step 3 - Separate sign and magnitude */
                sign = delta & 8;
                delta = delta & 7;

                /* Step 4 - update output value */
                vpdiff = 0;
                if (delta & 4) vpdiff += step[c];
                if (delta & 2) vpdiff += (step[c] >> 1);
                if (delta & 1) vpdiff += (step[c] >> 2);
                vpdiff += (step[c] >> 3);

                if (sign)
                    valprev[c] -= vpdiff;
                else
                    valprev[c] += vpdiff;

                /* Step 5 - clamp output value */
                if (valprev[c] > 32767)
                    valprev[c] = 32767;
                else if (valprev[c] < -32768)
                    valprev[c] = -32768;

                /* Step 6 - Update step value */
                step[c] = stepsizeTable[index[c]];

                /* Step 7 - Output value */
                outp = (unsigned short*)&pcmWriteBuf[pcmWrIdx];
                *outp = (unsigned short)valprev[c];

                pcmWrIdx += 2;
                if (pcmWrIdx >= sizeof(pcmWriteBuf)) pcmWrIdx = 0;
            }
        }
    }

    //printf(" pcm write %d \n",(pcmWrIdx>temp)? (pcmWrIdx-temp):(sizeof(pcmWriteBuf)-(temp - pcmWrIdx)) );
}


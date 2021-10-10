/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : g_adapt.c
*      Purpose          : gain adaptation for MR795 gain quantization
*
********************************************************************************
*/

/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "g_adapt.h"
const char g_adapt_id[] = "@(#)$Id $" g_adapt_h;

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
//#include <stdio.h>
//#include <stdlib.h>
#include "typedef.h"
#include "basic_op.h"
//#include "oper_32b.h"
//#include "count.h"
#include "cnst.h"
#include "gmed_n.h"

/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/
#define LTP_GAIN_THR1 2721 /* 2721 Q13 = 0.3322 ~= 1.0 / (10*log10(2)) */
#define LTP_GAIN_THR2 5443 /* 5443 Q13 = 0.6644 ~= 2.0 / (10*log10(2)) */

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
GainAdaptState GainAdaptS;
/*************************************************************************
*
*  Function:   gain_adapt_init
*  Purpose:    Allocates state memory and initializes state memory
*
**************************************************************************
*/
int gain_adapt_init (GainAdaptState **st)
{
    GainAdaptState* s;

//    if (st == (GainAdaptState **) NULL){
//        fprintf(stderr, "gain_adapt_init: invalid parameter\n");
//        return -1;
//    }
//    *st = NULL;

    /* allocate memory */
//    if ((s= (GainAdaptState *) malloc(sizeof(GainAdaptState))) == NULL){
//        fprintf(stderr, "gain_adapt_init: can't malloc state structure\n");
//        return -1;
//    }
    s = &GainAdaptS;
    gain_adapt_reset(s);
    *st = s;

    return 0;
}

/*************************************************************************
*
*  Function:   gain_adapt_reset
*  Purpose:    Initializes state memory to zero
*
**************************************************************************
*/
int gain_adapt_reset (GainAdaptState *st)
{
    //Word16 i;

//    if (st == (GainAdaptState *) NULL){
//        fprintf(stderr, "gain_adapt_reset: invalid parameter\n");
//        return -1;
//    }

    st->onset = 0;
    st->prev_alpha = 0;
    st->prev_gc = 0;

    //for (i = 0; i < LTPG_MEM_SIZE; i++)
    //{
    //    st->ltpg_mem[i] = 0;
    //}
    st->ltpg_mem[0] = 0;
    st->ltpg_mem[1] = 0;
    st->ltpg_mem[2] = 0;
    st->ltpg_mem[3] = 0;
    st->ltpg_mem[4] = 0;

    return 0;
}

/*************************************************************************
*
*  Function:   gain_adapt_exit
*  Purpose:    The memory used for state memory is freed
*
**************************************************************************
*/
//void gain_adapt_exit (GainAdaptState **st)
//{
//    if (st == NULL || *st == NULL)
//        return;

    /* deallocate memory */
//    free(*st);
//    *st = NULL;

//    return;
//}

/*************************************************************************
 *
 *  Function:   gain_adapt()
 *  Purpose:    calculate pitch/codebook gain adaptation factor alpha
 *              (and update the adaptor state)
 *
 **************************************************************************
 */
void gain_adapt(
    GainAdaptState *st,  /* i  : state struct                  */
    Word16 ltpg,         /* i  : ltp coding gain (log2()), Q13 */
    Word16 gain_cod,     /* i  : code gain,                Q1  */
    Word16 *alpha        /* o  : gain adaptation factor,   Q15 */
)
{
    Word16 adapt;      /* adaptdation status; 0, 1, or 2       */
    Word16 result;     /* alpha factor, Q13                    */
    Word16 filt;       /* median-filtered LTP coding gain, Q13 */
    Word16 tmp;
    Word32 i;

    /* basic adaptation */
    if (ltpg <= LTP_GAIN_THR1)
    {
        adapt = 0;
    }
    else
    {
        if (ltpg <= LTP_GAIN_THR2)
        {
            adapt = 1;
        }
        else
        {
            adapt = 2;
        }
    }

    /*
     * // onset indicator
     * if ((cbGain > onFact * cbGainMem[0]) && (cbGain > 100.0))
     *     onset = 8;
     * else
     *     if (onset)
     *         onset--;
     */
    /* tmp = cbGain / onFact; onFact = 2.0; 200 Q1 = 100.0 */
    tmp = (gain_cod+1)>>1;
    if ((tmp > st->prev_gc) && (gain_cod > 200))
    {
        st->onset = 8;
    }
    else
    {
        if (st->onset != 0)
        {
            st->onset --;
        }
    }

    /*
     *  // if onset, increase adaptor state
     *  if (onset && (gainAdapt < 2)) gainAdapt++;
     */
    if ((st->onset != 0) && (adapt < 2))
    {
        adapt ++;
    }

    st->ltpg_mem[0] = ltpg;
    filt = gmed_n (st->ltpg_mem, 5);   /* function result */

    if (adapt == 0)
    {
        if (filt > 5443) /* 5443 Q13 = 0.66443... */
        {
            result = 0;
        }
        else
        {
            if (filt < 0)
            {
                result = 16384;  /* 16384 Q15 = 0.5 */
            }
            else
            {   /* result       =   0.5 - 0.75257499*filt     */
                /* result (Q15) = 16384 - 24660 * (filt << 2) */
                filt <<= 2; /* Q15 */
                result = 16384 - (Word16)((24660*filt)>>15);
            }
        }
    }
    else
    {
        result = 0;
    }
    /*
     *  if (prevAlpha == 0.0) result = 0.5 * (result + prevAlpha);
     */
    if (st->prev_alpha == 0)
    {
        result >>= 1;
    }

    /* store the result */
    *alpha = result;

    /* update adapter state memory */
    st->prev_alpha = result;
    st->prev_gc = gain_cod;

    for (i = 4; i > 0; i--)
    {
        st->ltpg_mem[i] = st->ltpg_mem[i-1];
    }
    //st->ltpg_mem[4] = st->ltpg_mem[3];
    //st->ltpg_mem[3] = st->ltpg_mem[2];
    //st->ltpg_mem[2] = st->ltpg_mem[1];
    //st->ltpg_mem[1] = st->ltpg_mem[0];

    /* mem[0] is just present for convenience in calling the gmed_n[5]
     * function above. The memory depth is really LTPG_MEM_SIZE-1.
     */
}

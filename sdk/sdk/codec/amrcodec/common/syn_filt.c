/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : syn_filt.c
*      Purpose          : Perform synthesis filtering through 1/A(z).
*
********************************************************************************
*/
/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "syn_filt.h"
const char syn_filt_id[] = "@(#)$Id $" syn_filt_h;

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
#include "basic_op.h"
//#include "count.h"
#include "cnst.h"
#include "copy.h"

#ifdef MM365ENGINE
#include "engine.h"
#endif

/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/
/*
*--------------------------------------*
* Constants (defined in cnst.h         *
*--------------------------------------*
*  M         : LPC order               *
*--------------------------------------*
*/

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
void Syn_filt_de (
    Word16 a[],     /* (i)     : a[M+1] prediction coefficients   (M=10)  */
    Word16 x[],     /* (i)     : input signal                             */
    Word16 y[],     /* (o)     : output signal                            */
    Word16 lg,      /* (i)     : size of filtering                        */
    Word16 mem[],   /* (i/o)   : memory associated with this filtering.   */
    Word16 update   /* (i)     : 0=no update, 1=update of memory.         */
)
{
    Word32 i;
    //Word32 j;
    Word32 s;
    Word16 tmp[80];   /* This is usually done by memory allocation (lg+M) */
    Word16 *yy;

#   if !defined(HAVE_MACLC) || !defined(OPRISCASM)
    Word32 mac_hi,mac_lo;
#   endif

    Copy( mem, tmp, M);
    yy = &tmp[M];

    /* Do the filtering. */

    // 2004.09.27 sis3830
    //for (i = 0; i < lg; i++)
    //{
    i = 0;
    do
    {

        // sis3830 L_mult overflow
        //s = L_mult (x[i], a[0]);
        //for (j = 1; j <= M; j++)
        //{
        //    // sis3830 L_msu overflow
        //    s = L_msu (s, a[j], yy[-j]);
        //}

#ifdef OPRISCASM
        asm ("l.mac %0, %1" : : "r"((Word32)x[i]), "r"((Word32)a[0]));
        //for (j=1; j <= M; j++)
        //{
        //j = 1;
        //do
        //{
        //  asm ("l.msb %0, %1" : : "r"((Word32)a[j]), "r"((Word32)yy[-j]));
        //  j++;
        //} while (j <= M );
        asm ("l.msb %0, %1" : : "r"((Word32)a[ 1]), "r"((Word32)yy[ -1]));
        asm ("l.msb %0, %1" : : "r"((Word32)a[ 2]), "r"((Word32)yy[ -2]));
        asm ("l.msb %0, %1" : : "r"((Word32)a[ 3]), "r"((Word32)yy[ -3]));
        asm ("l.msb %0, %1" : : "r"((Word32)a[ 4]), "r"((Word32)yy[ -4]));
        asm ("l.msb %0, %1" : : "r"((Word32)a[ 5]), "r"((Word32)yy[ -5]));
        asm ("l.msb %0, %1" : : "r"((Word32)a[ 6]), "r"((Word32)yy[ -6]));
        asm ("l.msb %0, %1" : : "r"((Word32)a[ 7]), "r"((Word32)yy[ -7]));
        asm ("l.msb %0, %1" : : "r"((Word32)a[ 8]), "r"((Word32)yy[ -8]));
        asm ("l.msb %0, %1" : : "r"((Word32)a[ 9]), "r"((Word32)yy[ -9]));
        asm ("l.msb %0, %1" : : "r"((Word32)a[10]), "r"((Word32)yy[-10]));

        asm ("l.mac %0, %1" : : "r"((Word32)1), "r"((Word32)0x800));

#   ifdef HAVE_MACLC
        asm volatile("l.maclc %0, 4" : "=r" (s));
#   else
        asm volatile("l.mfspr %0, r0, 0x2802" : "=r"(mac_hi));
        asm volatile("l.macrc %0, 0" : "=r" (mac_lo));
#   endif

#else

        Word64 value64 = 0;
        value64 = x[i] * a[0];
        //for (j = 1; j <= M; j++)
        //{
        //   value64 -= ((a[j])*yy[-j]);
        //}
        value64 -= ((a[ 1])*yy[ -1]);
        value64 -= ((a[ 2])*yy[ -2]);
        value64 -= ((a[ 3])*yy[ -3]);
        value64 -= ((a[ 4])*yy[ -4]);
        value64 -= ((a[ 5])*yy[ -5]);
        value64 -= ((a[ 6])*yy[ -6]);
        value64 -= ((a[ 7])*yy[ -7]);
        value64 -= ((a[ 8])*yy[ -8]);
        value64 -= ((a[ 9])*yy[ -9]);
        value64 -= ((a[10])*yy[-10]);

        value64 += 0x800;

        mac_hi = (Word32)(value64 >> 32) ;
        mac_lo = (Word32)value64;

#endif

#if !defined(OPRISCASM) || (defined(OPRISCASM) && !defined(HAVE_MACLC))
        _CheckOverflow(mac_hi, mac_lo, s);

        if ( s > (Word32)0x0fffffffL )
            s = (Word32)0x7fffffffL;
        else if ( s < (Word32)0xf0000000L )
            s = (Word32)0x80000000L;
        else
            s <<= 3;
#endif

        // sis3830 Q
        //*yy++ = round16(s);
        *yy++ = (Word16)( s >>16 );
        i++;
    } while (i < lg);

    Copy( &tmp[M], y, lg);

    /* Update of memory if update==1 */

    if (update != 0)
    {
        Copy( &y[lg-M], mem, M);
    }
    return;
}

void Syn_filt (
    Word16 a[],     /* (i)     : a[M+1] prediction coefficients   (M=10)  */
    Word16 x[],     /* (i)     : input signal                             */
    Word16 y[],     /* (o)     : output signal                            */
    Word16 lg,      /* (i)     : size of filtering                        */
    Word16 mem[],   /* (i/o)   : memory associated with this filtering.   */
    Word16 update   /* (i)     : 0=no update, 1=update of memory.         */
)
{

    Word32 i;
    Word32 s;
    Word16 tmp[80];   /* This is usually done by memory allocation (lg+M) */
    Word16 *yy;

    Copy( mem, tmp, M);
    yy = &tmp[M];

    /* Do the filtering. */

    // 2004.09.27 sis3830
    //for (i = 0; i < lg; i++)
    //{
    i = 0;
    do
    {

        // sis3830 L_mult overflow
        //s = L_mult (x[i], a[0]);
        //for (j = 1; j <= M; j++)
        //{
        //    // sis3830 L_msu overflow
        //    s = L_msu (s, a[j], yy[-j]);
        //}

    //  Word32 mac_hi,mac_lo;
#ifdef SYNFILT_OPRISC
        asm ("l.mac %0, %1" : : "r"((Word32)x[i]), "r"((Word32)a[0]));
        //for (j=1; j <= M; j++)
        //{
        //j = 1;
        //do
        //{
        //  asm ("l.msb %0, %1" : : "r"((Word32)a[j]), "r"((Word32)yy[-j]));
        //  j++;
        //} while (j <= M );
        asm ("l.msb %0, %1" : : "r"((Word32)a[ 1]), "r"((Word32)yy[ -1]));
        asm ("l.msb %0, %1" : : "r"((Word32)a[ 2]), "r"((Word32)yy[ -2]));
        asm ("l.msb %0, %1" : : "r"((Word32)a[ 3]), "r"((Word32)yy[ -3]));
        asm ("l.msb %0, %1" : : "r"((Word32)a[ 4]), "r"((Word32)yy[ -4]));
        asm ("l.msb %0, %1" : : "r"((Word32)a[ 5]), "r"((Word32)yy[ -5]));
        asm ("l.msb %0, %1" : : "r"((Word32)a[ 6]), "r"((Word32)yy[ -6]));
        asm ("l.msb %0, %1" : : "r"((Word32)a[ 7]), "r"((Word32)yy[ -7]));
        asm ("l.msb %0, %1" : : "r"((Word32)a[ 8]), "r"((Word32)yy[ -8]));
        asm ("l.msb %0, %1" : : "r"((Word32)a[ 9]), "r"((Word32)yy[ -9]));
        asm ("l.msb %0, %1" : : "r"((Word32)a[10]), "r"((Word32)yy[-10]));

        asm ("l.mac %0, %1" : : "r"((Word32)1), "r"((Word32)0x800));

        //asm volatile("l.mfspr %0, r0, 0x2802" : "=r"(mac_hi));
        //asm volatile("l.macrc %0, 0" : "=r" (mac_lo));
        asm volatile("l.macrc %0, 0" : "=r" (s));
#else
    s = 0;
    s = x[i] * a[0];
    s -= ((a[ 1])*yy[ -1]);
    s -= ((a[ 2])*yy[ -2]);
    s -= ((a[ 3])*yy[ -3]);
    s -= ((a[ 4])*yy[ -4]);
    s -= ((a[ 5])*yy[ -5]);
    s -= ((a[ 6])*yy[ -6]);
    s -= ((a[ 7])*yy[ -7]);
    s -= ((a[ 8])*yy[ -8]);
    s -= ((a[ 9])*yy[ -9]);
    s -= ((a[10])*yy[-10]);
    s += 0x800;
    //if ( s > (Word32)0x0fffffffL )
    //      s = (Word32)0x7fffffffL;
    //else if ( s < (Word32)0xf0000000L )
    //   s = (Word32)0x80000000L;
    //else

#endif
    s <<= 4;
        // sis3830 Q
        //*yy++ = round (s);
        *yy++ = (Word16)( s >>16 );
        i++;
    } while (i < lg);

    //Copy( &tmp[M], y, lg);
#ifdef SYNFILT_CPY
    memcpy16(y, &tmp[M], lg);
#else
    for(i=0;i<lg;i++)
      y[i]=tmp[M+i];
#endif
//     fprintf(file_synfilt,"%.4x\n",y[i]);

    /* Update of memory if update==1 */

    if (update != 0)
    {
      //Copy( &y[lg-M], mem, M);
      mem[0]=y[30];
      mem[1]=y[31];
      mem[2]=y[32];
      mem[3]=y[33];
      mem[4]=y[34];
      mem[5]=y[35];
      mem[6]=y[36];
      mem[7]=y[37];
      mem[8]=y[38];
      mem[9]=y[39];
    }
    return;
}

/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : s10_8pf.c
*      Purpose          : Searches a 35/31 bit algebraic codebook containing
*                       : 10/8 pulses in a frame of 40 samples.
*
********************************************************************************
*/
/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "s10_8pf.h"
const char s10_8pf_id[] = "@(#)$Id $" s10_8pf_h;
/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
//#include "basic_op.h"
//#include "count.h"
#include "cnst.h"

/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/

/*************************************************************************
 *
 *  FUNCTION  search_10and8i40()
 *
 *  PURPOSE: Search the best codevector; determine positions of the 10/8
 *           pulses in the 40-sample frame.
 *
 *   search_10and8i40 (10,5,5,dn, rr, ipos, pos_max, codvec);   for GSMEFR
 *   search_10and8i40 (8, 4,4,dn, rr, ipos, pos_max, codvec);   for 10.2
 *
 *************************************************************************/

#define _1_2    (Word16)(32768L/2)
#define _1_4    (Word16)(32768L/4)
#define _1_8    (Word16)(32768L/8)
#define _1_16   (Word16)(32768L/16)
#define _1_32   (Word16)(32768L/32)
#define _1_64   (Word16)(32768L/64)
#define _1_128  (Word16)(32768L/128)

void search_10and8i40 (
    Word16 nbPulse,      /* i : nbpulses to find                       */
    Word16 step,         /* i :  stepsize                              */
    Word16 nbTracks,     /* i :  nbTracks                              */
    Word16 dn[],         /* i : correlation between target and h[]     */
    Word16 rr[][L_CODE], /* i : matrix of autocorrelation              */
    Word16 ipos[],       /* i : starting position for each pulse       */
    Word16 pos_max[],    /* i : position of maximum of dn[]            */
    Word16 codvec[]      /* o : algebraic codebook vector              */
)
{
   Word32 i0, i1, i2, i3, i4, i5, i6, i7, i8, i9;
   Word32 i, ia, ib;
   Word32 psk, ps, ps0, ps1, ps2, sq, sq2;
   Word32 alpk, alp, alp_16;
   Word32 rrv[L_CODE];
   Word32 s, alp0, alp1, alp2;
   Word32 gsmefrFlag;
   //Word32 j, k, pos;

   if (nbPulse == 10)
      gsmefrFlag=1;
   else
      gsmefrFlag=0;

   /* fix i0 on maximum of correlation position */
   i0 = pos_max[ipos[0]];

   /*------------------------------------------------------------------*
    * i1 loop:                                                         *
    *------------------------------------------------------------------*/

   /* Default value */
   psk = -1;
   alpk = 1;
   for (i = 0; i < nbPulse; i++)
      codvec[i] = i;

   //for (i = 1; i < nbTracks; i++)
   //{
      i1 = pos_max[ipos[1]];
      ps0 = dn[i0] + dn[i1];
      alp0 = rr[i0][i0] * _1_16;
      alp0 += (rr[i1][i1] * _1_16);
      alp0 += (rr[i0][i1] * _1_8);

      /*----------------------------------------------------------------*
       * i2 and i3 loop:                                                *
       *----------------------------------------------------------------*/

      for (i3 = ipos[3]; i3 < L_CODE; i3 += step)
      {
         s = rr[i3][i3] * _1_8;
         s += (rr[i0][i3] * _1_4);
         s += (rr[i1][i3] * _1_4);
         rrv[i3] = ( (s+0x00004000L)>>15 );
      }

      /* Default value */
      sq = -1;
      alp = 1;
      ps = 0;
      ia = ipos[2];
      ib = ipos[3];

      for (i2 = ipos[2]; i2 < L_CODE; i2 += step)
      {
         /* index increment = step  */
         ps1 = ps0 + dn[i2];

         /* index incr= step+L_CODE */
         alp1 = alp0;
         alp1 += (rr[i2][i2] * _1_16);
            /* index increment = step  */
         alp1 += (rr[i0][i2] * _1_8);
         /* index increment = step  */
         alp1 += (rr[i1][i2] * _1_8);

         for (i3 = ipos[3]; i3 < L_CODE; i3 += step)
         {
            /* index increment = step */
            ps2 = ps1 + dn[i3];

            /* index increment = step */
            alp2 = alp1;
            alp2 += (rrv[i3] * _1_2);
            /* index increment = step */
            alp2 += (rr[i2][i3] * _1_8);

            alp_16 = ( (alp2+0x00004000L)>>15 );

            sq2 = (Word16)( (ps2*ps2)>>15 );

            s = alp * sq2;
            s -= (sq * alp_16);

            if (s > 0)
            {
               sq = sq2;
               ps = ps2;
               alp = alp_16;
               ia = i2;
               ib = i3;
            }
         }
      }
      i2 = ia;
      i3 = ib;

        /*----------------------------------------------------------------*
         * i4 and i5 loop:                                                *
         *----------------------------------------------------------------*/

        ps0 = ps;
        alp0 = alp * _1_2;

        for (i5 = ipos[5]; i5 < L_CODE; i5 += step)
        {
            s = rr[i5][i5] * _1_8;
            s += (rr[i0][i5] * _1_4);
            s += (rr[i1][i5] * _1_4);
            s += (rr[i2][i5] * _1_4);
            s += (rr[i3][i5] * _1_4);
            rrv[i5] = ( (s+0x00004000L)>>15 );
        }

        /* Default value */
        sq = -1;
        alp = 1;
        ps = 0;
        ia = ipos[4];
        ib = ipos[5];

        for (i4 = ipos[4]; i4 < L_CODE; i4 += step)
        {
            ps1 = ps0 + dn[i4];

            alp1 = alp0;
            alp1 += (rr[i4][i4] * _1_32);
            alp1 += (rr[i0][i4] * _1_16);
            alp1 += (rr[i1][i4] * _1_16);
            alp1 += (rr[i2][i4] * _1_16);
            alp1 += (rr[i3][i4] * _1_16);

            for (i5 = ipos[5]; i5 < L_CODE; i5 += step)
            {
                ps2 = ps1 + dn[i5];

                alp2 = alp1;
                alp2 += (rrv[i5] * _1_4);
                alp2 += (rr[i4][i5] * _1_16);

                alp_16 = ( (alp2+0x00004000L)>>15 );

                sq2 = (Word16)( (ps2 * ps2)>>15 );

                s = alp * sq2;
                s -= (sq * alp_16);

                if (s > 0)
                {
                    sq = sq2;
                    ps = ps2;
                    alp = alp_16;
                    ia = i4;
                    ib = i5;
                }
            }
        }
        i4 = ia;
        i5 = ib;

        /*----------------------------------------------------------------*
         * i6 and i7 loop:                                                *
         *----------------------------------------------------------------*/

        ps0 = ps;
        alp0 = alp * _1_2;

        for (i7 = ipos[7]; i7 < L_CODE; i7 += step)
        {
            s = rr[i7][i7] * _1_16;
            s += (rr[i0][i7] * _1_8);
            s += (rr[i1][i7] * _1_8);
            s += (rr[i2][i7] * _1_8);
            s += (rr[i3][i7] * _1_8);
            s += (rr[i4][i7] * _1_8);
            s += (rr[i5][i7] * _1_8);
            rrv[i7] = ( (s+0x00004000L)>>15 );
        }

        /* Default value */
        sq = -1;
        alp = 1;
        ps = 0;
        ia = ipos[6];
        ib = ipos[7];

        for (i6 = ipos[6]; i6 < L_CODE; i6 += step)
        {
            ps1 = ps0 + dn[i6];

            alp1 = alp0;
            alp1 += (rr[i6][i6] * _1_64);
            alp1 += (rr[i0][i6] * _1_32);
            alp1 += (rr[i1][i6] * _1_32);
            alp1 += (rr[i2][i6] * _1_32);
            alp1 += (rr[i3][i6] * _1_32);
            alp1 += (rr[i4][i6] * _1_32);
            alp1 += (rr[i5][i6] * _1_32);

            for (i7 = ipos[7]; i7 < L_CODE; i7 += step)
            {
                ps2 = ps1 + dn[i7];

                alp2 = alp1;
                alp2 += (rrv[i7] * _1_4);
                alp2 += (rr[i6][i7] * _1_32);

                alp_16 = ( (alp2+0x00004000L)>>15 );

                sq2 = (Word16)( (ps2 * ps2)>>15 );

                s = alp * sq2;
                s -= (sq * alp_16);

                if (s > 0)
                {
                    sq = sq2;
                    ps = ps2;
                    alp = alp_16;
                    ia = i6;
                    ib = i7;
                }
            }
        }
        i6 = ia;
        i7 = ib;

        /* now finished searching a set of 8 pulses */

        if(gsmefrFlag != 0){
           /* go on with the two last pulses for GSMEFR                      */
           /*----------------------------------------------------------------*
            * i8 and i9 loop:                                                *
            *----------------------------------------------------------------*/

           ps0 = ps;
           alp0 = alp * _1_2;

           for (i9 = ipos[9]; i9 < L_CODE; i9 += step)
           {
              s = rr[i9][i9] * _1_16;
              s += (rr[i0][i9] * _1_8);
              s += (rr[i1][i9] * _1_8);
              s += (rr[i2][i9] * _1_8);
              s += (rr[i3][i9] * _1_8);
              s += (rr[i4][i9] * _1_8);
              s += (rr[i5][i9] * _1_8);
              s += (rr[i6][i9] * _1_8);
              s += (rr[i7][i9] * _1_8);
              rrv[i9] = ( (s+0x00004000L)>>15 );
           }

           /* Default value */
           sq = -1;
           alp = 1;
           ps = 0;
           ia = ipos[8];
           ib = ipos[9];

           for (i8 = ipos[8]; i8 < L_CODE; i8 += step)
           {
              ps1 = ps0 + dn[i8];

              alp1 = alp0;
              alp1 += (rr[i8][i8] * _1_128);
              alp1 += (rr[i0][i8] * _1_64);
              alp1 += (rr[i1][i8] * _1_64);
              alp1 += (rr[i2][i8] * _1_64);
              alp1 += (rr[i3][i8] * _1_64);
              alp1 += (rr[i4][i8] * _1_64);
              alp1 += (rr[i5][i8] * _1_64);
              alp1 += (rr[i6][i8] * _1_64);
              alp1 += (rr[i7][i8] * _1_64);

              for (i9 = ipos[9]; i9 < L_CODE; i9 += step)
              {
                 ps2 = ps1 + dn[i9];

                 alp2 = alp1;
                 alp2 += (rrv[i9] * _1_8);
                 alp2 += (rr[i8][i9] * _1_64);

                alp_16 = ( (alp2+0x00004000L)>>15 );

                sq2 = (Word16)( (ps2 * ps2)>>15 );

                s = alp * sq2;
                s -= (sq * alp_16);

                 if (s > 0)
                 {
                    sq = sq2;
                    ps = ps2;
                    alp = alp_16;
                    ia = i8;
                    ib = i9;
                 }
              }
           }
        }/* end  gsmefrFlag */

        /*----------------------------------------------------------------  *
         * test and memorise if this combination is better than the last one.*
         *----------------------------------------------------------------*/

        s = alpk * sq;
        s -= (psk * alp);
//        s = L_msu (L_mult (alpk, sq), psk, alp);

        if (s > 0)
        {
            psk = sq;
            alpk = alp;
            codvec[0] = (Word16)i0;
            codvec[1] = (Word16)i1;
            codvec[2] = (Word16)i2;
            codvec[3] = (Word16)i3;
            codvec[4] = (Word16)i4;
            codvec[5] = (Word16)i5;
            codvec[6] = (Word16)i6;
            codvec[7] = (Word16)i7;

            if (gsmefrFlag != 0)
            {
               codvec[8] = (Word16)ia;
               codvec[9] = (Word16)ib;
            }
        }
        /*----------------------------------------------------------------*
         * Cyclic permutation of i1,i2,i3,i4,i5,i6,i7,(i8 and i9).          *
         *----------------------------------------------------------------*/

        //pos = ipos[1];
        //for (j = 1, k = 2; k < nbPulse; j++, k++)
        //{
        //    ipos[j] = ipos[k];
        //}
        //ipos[(nbPulse-1)] = pos;
   //} /* end 1..nbTracks  loop*/
}

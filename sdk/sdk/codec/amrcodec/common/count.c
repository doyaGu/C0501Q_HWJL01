﻿/***********************************************************************
 *
 *   This file contains functions for the automatic complexity calculation
 * $Id $
 *************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "typedef.h"
#include "count.h"
//#include "sys/sys.h"

/* Global counter variable for calculation of complexity weight */

BASIC_OP multiCounter[MAXCOUNTERS];
int currCounter=0; /* Zero equals global counter */

/*BASIC_OP counter;*/
const BASIC_OP op_weight =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 4, 15, 18, 30, 1, 2, 1, 2, 2
};

/* function prototypes */
Word32 TotalWeightedOperation (void);
Word32 DeltaWeightedOperation (void);

/* local variable */
#if WMOPS

/* Counters for separating counting for different objects */
static int maxCounter=0;
static char* objectName[MAXCOUNTERS+1];
static Word16 fwc_corr[MAXCOUNTERS+1];

#define NbFuncMax  1024

static Word16 funcid[MAXCOUNTERS], nbframe[MAXCOUNTERS];
static Word32 glob_wc[MAXCOUNTERS], wc[MAXCOUNTERS][NbFuncMax];
static float total_wmops[MAXCOUNTERS];

static Word32 LastWOper[MAXCOUNTERS];

static char* my_strdup(const char *s)
/*
 * duplicates UNIX function strdup() which is not ANSI standard:
 * -- malloc() memory area big enough to hold the string s
 * -- copy string into new area
 * -- return pointer to new area
 *
 * returns NULL if either s==NULL or malloc() fails
 */
{
    char *dup;

    if (s == NULL)
        return NULL;

    /* allocate memory for copy of ID string (including string terminator) */
    /* NOTE: the ID strings will never be deallocated because there is no
             way to "destroy" a counter that is not longer needed          */
    if ((dup = (char *) SYS_Malloc(strlen(s)+1)) == NULL)
        return NULL;

    return strcpy(dup, s);
}

#endif

int getCounterId(char *objectNameArg)
{
#if WMOPS
  if(maxCounter>=MAXCOUNTERS-1) return 0;
  objectName[++maxCounter]=my_strdup(objectNameArg);
  return maxCounter;
#else
  return 0; /* Dummy */
#endif
}

void setCounter(int counterId)
{
#if WMOPS
  if(counterId>maxCounter || counterId<0)
    {
      currCounter=0;
      return;
    }
  currCounter=counterId;
#endif
}

#if WMOPS
static Word32 WMOPS_frameStat()
/* calculate the WMOPS seen so far and update the global
   per-frame maximum (glob_wc)
 */
{
    Word32 tot;

    tot = TotalWeightedOperation ();
    if (tot > glob_wc[currCounter])
        glob_wc[currCounter] = tot;

    /* check if fwc() was forgotten at end of last frame */
    if (tot > LastWOper[currCounter]) {
        if (!fwc_corr[currCounter]) {
            fprintf(stderr,
                    "count: operations counted after last fwc() for '%s'; "
                    "-> fwc() called\n",
                    objectName[currCounter]?objectName[currCounter]:"");
        }
        fwc();
    }

    return tot;
}

static void WMOPS_clearMultiCounter()
{
    Word16 i;

    Word32 *ptr = (Word32 *) &multiCounter[currCounter];
    for (i = 0; i < (sizeof (multiCounter[currCounter])/ sizeof (Word32)); i++)
    {
        *ptr++ = 0;
    }
}
#endif

Word32 TotalWeightedOperation ()
{
#if WMOPS
    Word16 i;
    Word32 tot, *ptr, *ptr2;

    tot = 0;
    ptr = (Word32 *) &multiCounter[currCounter];
    ptr2 = (Word32 *) &op_weight;
    for (i = 0; i < (sizeof (multiCounter[currCounter])/ sizeof (Word32)); i++)
    {
        tot += ((*ptr++) * (*ptr2++));
    }

    return ((Word32) tot);
#else
    return 0; /* Dummy */
#endif
}

Word32 DeltaWeightedOperation ()
{
#if WMOPS
    Word32 NewWOper, delta;

    NewWOper = TotalWeightedOperation ();
    delta = NewWOper - LastWOper[currCounter];
    LastWOper[currCounter] = NewWOper;
    return (delta);
#else
    return 0; /* Dummy */
#endif
}

void move16 (void)
{
#if WMOPS
    multiCounter[currCounter].DataMove16++;
#endif
}

void move32 (void)
{
#if WMOPS
    multiCounter[currCounter].DataMove32++;
#endif
}

void test (void)
{
#if WMOPS
    multiCounter[currCounter].Test++;
#endif
}

void logic16 (void)
{
#if WMOPS
    multiCounter[currCounter].Logic16++;
#endif
}

void logic32 (void)
{
#if WMOPS
    multiCounter[currCounter].Logic32++;
#endif
}

void Init_WMOPS_counter (void)
{
#if WMOPS
    Word16 i;

    /* reset function weight operation counter variable */

    for (i = 0; i < NbFuncMax; i++)
        wc[currCounter][i] = (Word32) 0;
    glob_wc[currCounter] = 0;
    nbframe[currCounter] = 0;
    total_wmops[currCounter] = 0.0;

    /* initially clear all counters */
    WMOPS_clearMultiCounter();
    LastWOper[currCounter] = 0;
    funcid[currCounter] = 0;
#endif
}

void Reset_WMOPS_counter (void)
{
#if WMOPS
    Word32 tot = WMOPS_frameStat();

    /* increase the frame counter --> a frame is counted WHEN IT BEGINS */
    nbframe[currCounter]++;
    /* add wmops used in last frame to count, then reset counter */
    /* (in first frame, this is a no-op                          */
    total_wmops[currCounter] += ((float) tot) * 0.00005;

    /* clear counter before new frame starts */
    WMOPS_clearMultiCounter();
    LastWOper[currCounter] = 0;
    funcid[currCounter] = 0;           /* new frame, set function id to zero */
#endif
}

Word32 fwc (void)                      /* function worst case */
{
#if WMOPS
    Word32 tot;

    tot = DeltaWeightedOperation ();
    if (tot > wc[currCounter][funcid[currCounter]])
        wc[currCounter][funcid[currCounter]] = tot;

    funcid[currCounter]++;

    return (tot);
#else
    return 0; /* Dummy */
#endif
}

void WMOPS_output (Word16 dtx_mode)
{
#if WMOPS
    Word16 i;
    Word32 tot, tot_wm, tot_wc;

    /* get operations since last reset (or init),
       but do not update the counters (except the glob_wc[] maximum)
       so output CAN be called in each frame without problems.
       The frame counter is NOT updated!
     */
    tot = WMOPS_frameStat();
    tot_wm = total_wmops[currCounter] + ((float) tot) * 0.00005;

    fprintf (stdout, "%10s:WMOPS=%.3f",
         objectName[currCounter]?objectName[currCounter]:"",
         ((float) tot) * 0.00005);

    if (nbframe[currCounter] != 0)
        fprintf (stdout, "  Average=%.3f",
                 tot_wm / (float) nbframe[currCounter]);

    fprintf (stdout, "  WorstCase=%.3f",
             ((float) glob_wc[currCounter]) * 0.00005);

    /* Worst worst case printed only when not in DTX mode */
    if (dtx_mode == 0)
    {
        tot_wc = 0L;
        for (i = 0; i < funcid[currCounter]; i++)
            tot_wc += wc[currCounter][i];
        fprintf (stdout, "  WorstWC=%.3f", ((float) tot_wc) * 0.00005);
    }
    fprintf (stdout, " (%d frames)\n", nbframe[currCounter]);

    // add by PoWei
    /*
    fprintf (stdout, " add => %d\n", multiCounter[currCounter].add );
    fprintf (stdout, " sub => %d\n", multiCounter[currCounter].sub );
    fprintf (stdout, " abs_s => %d\n", multiCounter[currCounter].abs_s );
    fprintf (stdout, " shl => %d\n", multiCounter[currCounter].shl );
    fprintf (stdout, " shr => %d\n", multiCounter[currCounter].shr );
    fprintf (stdout, " extract_h => %d\n", multiCounter[currCounter].extract_h );
    fprintf (stdout, " extract_l => %d\n", multiCounter[currCounter].extract_l );
    fprintf (stdout, " mult => %d\n", multiCounter[currCounter].mult );
    fprintf (stdout, " L_mult => %d\n", multiCounter[currCounter].L_mult );
    fprintf (stdout, " negate => %d\n", multiCounter[currCounter].negate );
    fprintf (stdout, " round => %d\n", multiCounter[currCounter].round );
    fprintf (stdout, " L_mac => %d\n", multiCounter[currCounter].L_mac );
    fprintf (stdout, " L_msu => %d\n", multiCounter[currCounter].L_msu );
    fprintf (stdout, " L_smac => %d\n", multiCounter[currCounter].L_macNs );
    fprintf (stdout, " L_smsu => %d\n", multiCounter[currCounter].L_msuNs );
    fprintf (stdout, " L_smac/L_smsu Num>40 => %d\n", multiCounter[currCounter].mac_r );
    fprintf (stdout, " L_smac/L_smsu Num=40 => %d\n", multiCounter[currCounter].msu_r );
    fprintf (stdout, " L_smac/L_smsu 40>Num>10 => %d\n", multiCounter[currCounter].L_add_c );
    fprintf (stdout, " L_smac/L_smsu Num=10 => %d\n", multiCounter[currCounter].L_sub_c );
    fprintf (stdout, " L_smac/L_smsu Num<10 => %d\n", multiCounter[currCounter].L_sat );
    fprintf (stdout, " L_add => %d\n", multiCounter[currCounter].L_add );
    fprintf (stdout, " L_sub => %d\n", multiCounter[currCounter].L_sub );
    fprintf (stdout, " L_negate => %d\n", multiCounter[currCounter].L_negate );
    fprintf (stdout, " L_shl => %d\n", multiCounter[currCounter].L_shl );
    fprintf (stdout, " L_shr => %d\n", multiCounter[currCounter].L_shr );
    fprintf (stdout, " mult_r => %d\n", multiCounter[currCounter].mult_r );
    fprintf (stdout, " shr_r => %d\n", multiCounter[currCounter].shr_r );
    fprintf (stdout, " shift_r => %d\n", multiCounter[currCounter].shift_r );
    fprintf (stdout, " L_deposit_h => %d\n", multiCounter[currCounter].L_deposit_h );
    fprintf (stdout, " L_deposit_l => %d\n", multiCounter[currCounter].L_deposit_l );
    fprintf (stdout, " L_shr_r => %d\n", multiCounter[currCounter].L_shr_r );
    fprintf (stdout, " L_shift_r => %d\n", multiCounter[currCounter].L_shift_r );
    fprintf (stdout, " L_abs => %d\n", multiCounter[currCounter].L_abs );
    fprintf (stdout, " norm_s => %d\n", multiCounter[currCounter].norm_s );
    fprintf (stdout, " div_s => %d\n", multiCounter[currCounter].div_s );
    fprintf (stdout, " norm_l => %d\n", multiCounter[currCounter].norm_l );
    fprintf (stdout, " DataMove16 => %d\n", multiCounter[currCounter].DataMove16 );
    fprintf (stdout, " DataMove32 => %d\n", multiCounter[currCounter].DataMove32 );
    fprintf (stdout, " Logic16 => %d\n", multiCounter[currCounter].Logic16 );
    fprintf (stdout, " Logic32 => %d\n", multiCounter[currCounter].Logic32 );
    fprintf (stdout, " Test => %d\n", multiCounter[currCounter].Test );
    */

#endif
}

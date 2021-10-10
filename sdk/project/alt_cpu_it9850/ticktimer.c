/*
 * Copyright (c) 2006 SMedia Technology Corp. All Rights Reserved.
 */
/* @file
 * Include file for tick timer
 *
 * @author Kuoping Hsu
 * @date 2006.07.26.
 * @version 1.0
 *
 */

#include "spr_defs.h"

/////////////////////////////////////////////////////////////////
// Timer Functions
//
// The timer function is use to count the ticks in 2.68 seconds
// period maximun (count to 0x0fffffff in 100MHz). It dose not
// raise the tick timer exception to count the larger number
// of ticks.
//
// Usage:
//
//    int ticks;
//    startTimer();
//
//    ..... blah blah .....
//
//    ticks = get_timer();
//
/////////////////////////////////////////////////////////////////
static unsigned long gPrevTick1 = 0;
static unsigned long gPrevTick2 = 0;
static unsigned long gPrevTick1Offset = 0;
static unsigned long gPrevTick2Offset = 0;

__inline unsigned long getCurTimer(int timer)
{
    unsigned long current_ticks = 0;
    unsigned long* pPrevTick = 0;
    unsigned long* pTickOffset = 0;
    unsigned long tickReg = 0;
    unsigned long setReg = 0;
    if (timer)
    {
        tickReg =SPR_TTCR2;
        setReg = SPR_TTMR2;
        pPrevTick = &gPrevTick2;
        pTickOffset = &gPrevTick2Offset;
    }
    else
    {
        tickReg =SPR_TTCR;        
        setReg = SPR_TTMR;
        pPrevTick = &gPrevTick1;
        pTickOffset = &gPrevTick1Offset;
    }
    current_ticks =  mfspr(tickReg) + *pTickOffset;
    //Timer is halt
    if (current_ticks == *pPrevTick)
    {
        //stop
        mtspr(setReg, 0);
        //restart
        mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_TEE);
        mtspr(setReg, 0);
        /* Reset counter */
        mtspr(tickReg, 0);
        /* single run mode and disable interrupt */
        mtspr(setReg, SPR_TTMR_CR | SPR_TTMR_PERIOD);
        current_ticks =  mfspr(tickReg) + *pTickOffset;
    }
    *pPrevTick = current_ticks;
    return current_ticks;

}

__inline unsigned long getTimer(int timer)
{
    unsigned long current_ticks = 0;
    if (timer)
    {
        current_ticks = mfspr(SPR_TTCR2);
        /* reset timer */
        //mtspr(SPR_TTCR2, 0);
    }
    else
    {
        current_ticks = mfspr(SPR_TTCR);
        /* reset timer */
        //mtspr(SPR_TTCR, 0);
    }
    return current_ticks;
}

__inline void resetTimer(int timer)
{
    if (timer)
    {
        mtspr(SPR_TTCR2, 0);
    }
    else
    {
        mtspr(SPR_TTCR, 0);
    }
}

__inline void stopTimer(int timer) 
{
    /* Stop Timer */
    if (timer)
    {
        mtspr(SPR_TTMR2, 0);
    }
    else
    {
        mtspr(SPR_TTMR, 0);
    }
}

__inline void startTimer(int timer) 
{
    /* Disable tick timer exception recognition */
    mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_TEE);
    /* stop timer */
    if (timer)
    {
        mtspr(SPR_TTMR2, 0);
        gPrevTick2 = gPrevTick2Offset = 0;
        /* Reset counter */
        mtspr(SPR_TTCR2, 0);
        /* single run mode and disable interrupt */
        mtspr(SPR_TTMR2, SPR_TTMR_CR | SPR_TTMR_PERIOD);
    }
    else
    {
        mtspr(SPR_TTMR, 0);        
        gPrevTick1 = gPrevTick1Offset = 0;
        /* Reset counter */
        mtspr(SPR_TTCR, 0);
        /* single run mode and disable interrupt */
        mtspr(SPR_TTMR, SPR_TTMR_CR | SPR_TTMR_PERIOD);
    }
}

__inline unsigned long getDuration(int timer, unsigned long previousTick) 
{
    unsigned long curTick = getCurTimer(timer);

    if (curTick >= previousTick)
    {
        return (curTick - previousTick);
    }
    else
    {
        return (curTick + (SPR_TTCR_PERIOD - previousTick));
    }
}

__inline unsigned long getDurationInUs(int timer, unsigned long previousTick, unsigned long tickPerUs) 
{
    unsigned long curTick = getTimer(timer);
    if (curTick >= previousTick)
    {
        return ((curTick - previousTick) / tickPerUs);
    }
    else
    {
        return ((curTick + (SPR_TTCR_PERIOD - previousTick)) / tickPerUs);
    }
}

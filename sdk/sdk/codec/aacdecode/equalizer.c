/**************************************************************************************
 * Filename:    equalizer.c
 *
 * Description: AAC equalizer
 *              equalize => computeGain() and equalize()
 * Creator:     s025
 *
 *
 *
 **************************************************************************************/
#include "assembly.h"
#include "coder.h"

#if defined(WIN32) || defined(__CYGWIN__)
#  include <stdio.h>
#endif

#if defined(EQUALIZER)

#  define Indx0db 12

int db2gain[25]=
{
0x0000404D,0x00004826,0x000050F4,0x00005AD5,0x000065EA,0x00007259,0x0000804D,0x00008FF5,
0x0000A186,0x0000B53B,0x0000CB59,0x0000E429,0x00010000,0x00011F3C,0x00014248,0x0001699C,
0x000195BB,0x0001C73D,0x0001FEC9,0x00023D1C,0x0002830A,0x0002D181,0x0003298B,0x00038C52,
0x0003FB27,
};

#if defined(WIN32) || defined(__CYGWIN__)
EQINFO eqinfo = {
    1, 1,
    {  0,  82, 251, 404, 557, 732, 907, 1098, 1328, 1600, 3300, 6000, 13000, 16000, 25000,    -1},
    {  1,   2,   2,   3,   3,   4,   1,    0,   -4,   -7,   -7,   -7,    -7,    -7,    -7,     0},
};
#else
extern EQINFO eqinfo;
#endif // defined(WIN32) || defined(__CYGWIN__)

static const int windowsize[2] = {128, 1024};
static const int blockgainoffset[2] = {0, 128};
static int nBlockGain[128+1024];

int incrL;
int incrS;

/**************************************************************************************
 * Function:    computeEQGain
 *
 * Description: Get Gain value for equalize
 *
 *
 *
 * Inputs:      audio samplerate,
 *
 * Outputs:     none
 *
 * Return:
 **************************************************************************************/
void computeEQGain(int samplerate)
{
    int band, sample;
    int curr_freq;
    int curr_gain, gain_incr;

    incrL = samplerate/2048;
    incrS = samplerate/256;
    band  = 0;
    curr_freq = eqinfo.bandcenter[band];
    curr_gain = db2gain[Indx0db+eqinfo.dbtab[band]];
    gain_incr = (db2gain[Indx0db+eqinfo.dbtab[band+1]]-db2gain[Indx0db+eqinfo.dbtab[band]]) * incrL /
                (eqinfo.bandcenter[band+1] - eqinfo.bandcenter[band]);

    for(sample=0; sample < 1024; sample++){
        nBlockGain[128+sample] = curr_gain;

        curr_freq += incrL;
        curr_gain += gain_incr;
        if(curr_freq >= eqinfo.bandcenter[band+1]){
            band++;
            curr_gain = db2gain[Indx0db+eqinfo.dbtab[band]];
            gain_incr = (db2gain[Indx0db+eqinfo.dbtab[band+1]]-db2gain[Indx0db+eqinfo.dbtab[band]]) * incrL /
                        (eqinfo.bandcenter[band+1] - eqinfo.bandcenter[band]);
        }
    }

    band=0;
    curr_freq = eqinfo.bandcenter[band];
    curr_gain = db2gain[Indx0db+eqinfo.dbtab[band]];
    gain_incr = (db2gain[Indx0db+eqinfo.dbtab[band+1]]-db2gain[Indx0db+eqinfo.dbtab[band]]) * incrS /
                (eqinfo.bandcenter[band+1] - eqinfo.bandcenter[band]);

    for(sample=0 ; sample < 128; sample++){
        nBlockGain[sample] = curr_gain;

        curr_freq += incrS;
        curr_gain += gain_incr;
        if(curr_freq >= eqinfo.bandcenter[band+1]){
            band++;
            curr_gain = db2gain[Indx0db+eqinfo.dbtab[band]];
            gain_incr = (db2gain[Indx0db+eqinfo.dbtab[band+1]]-db2gain[Indx0db+eqinfo.dbtab[band]]) * incrS /
                        (eqinfo.bandcenter[band+1] - eqinfo.bandcenter[band]);
        }
    }
}

/**************************************************************************************
 * Function:    equalize
 *
 * Description: AAC equalize
 *
 *
 *
 * Inputs:      sampleBuf  // frequncy data array. [1024] for long, [128] for short
 *              blockType  //0: short block, 1: long block.
 *              mixedBlock //0 = regular block (all short or long), 1 = mixed block
 *
 * Outputs:     none
 *
 * Return:
 **************************************************************************************/
 // spec_coef[1024] records frequency data.
void equalize(int *sampleBuf, int blockType)
{
  int i;
  int val;
    int block_limit;//, sbgainidx;

    int *pBlockGain;

  //sbgainidx=0;
    block_limit = windowsize[blockType]; //shortwindow = 128 or longwindow = 1024
    pBlockGain = nBlockGain + blockgainoffset[blockType]; //+0 or +128

    for(i=0; i < block_limit; i++)  {
        val = MULSHIFT(*pBlockGain++, sampleBuf[i], 16);
        // saturation: format = Q26
        /*
        if(val < -0x03ffffff) {
            val = -0x03ffffff;
        } else if(val > 0x03ffffff) {
            val = 0x03ffffff;
        }
        */
        sampleBuf[i] = val;
    }

}
#endif //EQUALIZER


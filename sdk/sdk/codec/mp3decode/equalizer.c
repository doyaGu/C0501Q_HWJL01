#include "mp3_config.h"

#if defined(EQUALIZER)

#   define Indx0db     12
#   define EQSHIFT     16

static int db2gain[25]={
    0x0000404D,0x00004826,0x000050F4,0x00005AD5,0x000065EA,0x00007259,
    0x0000804D,0x00008FF5,0x0000A186,0x0000B53B,0x0000CB59,0x0000E429,
    0x00010000,0x00011F3C,0x00014248,0x0001699C,0x000195BB,0x0001C73D,
    0x0001FEC9,0x00023D1C,0x0002830A,0x0002D181,0x0003298B,0x00038C52,
    0x0003FB27,
};

static param_eq_struct param_eq = {
    0,
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

static int longblockGain[576];
static int shortblockGain[192];

static void computeEqGain(param_eq_struct *p_param)
{
    int band, sample;
    int curr_freq;
    int curr_gain, gain_incr;
    short *dbtab = p_param->dbtab;
    short *bandcenter = p_param->bandcenter;

    int incrL = mp3DecInfo.samprate / 1152;
    int incrS = mp3DecInfo.samprate / 384;

    band=0;
    curr_freq = bandcenter[band];
    curr_gain = db2gain[Indx0db+dbtab[band]];

    gain_incr = (db2gain[Indx0db+dbtab[band+1]]-db2gain[Indx0db+dbtab[band]]) * incrL / (bandcenter[band+1] - bandcenter[band]);

    for(sample=0; sample < 576; sample++){
        longblockGain[sample] = curr_gain;

        curr_freq += incrL;
        curr_gain += gain_incr;
        if(curr_freq >= bandcenter[band+1]){
            band++;
            curr_gain = db2gain[Indx0db+dbtab[band]];
            gain_incr = (db2gain[Indx0db+dbtab[band+1]]-db2gain[Indx0db+dbtab[band]]) * incrL / (bandcenter[band+1] - bandcenter[band]);
        }
    }

    band=0;
    curr_freq = bandcenter[band];
    curr_gain = db2gain[Indx0db+dbtab[band]];
    gain_incr = (db2gain[Indx0db+dbtab[band+1]]-db2gain[Indx0db+dbtab[band]]) * incrS / (bandcenter[band+1] - bandcenter[band]);

    for(sample=0 ; sample < 192; sample++){
        shortblockGain[sample] = curr_gain;

        curr_freq += incrS;
        curr_gain += gain_incr;
        if(curr_freq >= bandcenter[band+1]){
            band++;
            curr_gain = db2gain[Indx0db+dbtab[band]];
            gain_incr = (db2gain[Indx0db+dbtab[band+1]]-db2gain[Indx0db+dbtab[band]]) * incrS / (bandcenter[band+1] - bandcenter[band]);
        }
    }
}

static void check_eq_param(param_eq_struct *p_param)
{
    int i, update=0;
    for(i=0; i<16; i++){
        if((param_eq.dbtab[i] != p_param->dbtab[i]) ||
            (param_eq.bandcenter[i] != p_param->bandcenter[i])){
            update=1;
            break;
        }
    }
    if(update){
        for(i=0; i<16; i++){
            param_eq.dbtab[i] = p_param->dbtab[i];
            param_eq.bandcenter[i] = p_param->bandcenter[i];
        }
        computeEqGain(p_param);
    }
}

void equalization_filter(param_eq_struct *p_param)
{
    int gr, ch, i, k;
    int *sampleBuf;
    SideInfoSub *sis;
    int longblock_limit, sbgainidx;
    int val;

    check_eq_param(p_param);

    for (gr = 0; gr < mp3DecInfo.nGrans; gr++) {
        // dequantize all the samples in each channel
        for (ch = 0; ch < mp3DecInfo.nChans; ch++) {
            sampleBuf = mp3DecInfo.HuffmanInfoPS.huffDecBuf[gr][ch];    //hi->huffDecBuf[gr][ch];
            sis = &mp3DecInfo.SideInfoPS.sis[gr][ch];       //&si->sis[gr][ch];

            sbgainidx=0;
            if (sis->blockType == 2) {
                if (sis->mixedBlock) {
                    longblock_limit = 36;
                    sbgainidx=12;
                } else {
                    longblock_limit = 0;
                }
            } else {
                // long block
                longblock_limit = 576;
            }

            for(i=0; i < longblock_limit; i++){
                val = MULSHIFT(longblockGain[i], sampleBuf[i], EQSHIFT);
                sampleBuf[i] = val;
            }
            for(i=longblock_limit; i < 576; i+=3){
                for(k=0; k<3; k++){
                    val = MULSHIFT(shortblockGain[sbgainidx], sampleBuf[i + k], EQSHIFT);
                    sampleBuf[i + k] = val;
                }
                sbgainidx++;
            }

        }// for (ch...)
    }// for (gr...)
}

#endif
